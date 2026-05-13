#include "include.h"


#define ZOOM_MAX                        16



dvp_share_t dvp_gpu_share AT(.share_dvp.buff);

#if AVI_DVP_USE_CAMERA

typedef struct{
    u8 *disp_buff;
    bool is_ready;
    bool is_init;
}img_dvp_cb_t;

static img_dvp_cb_t img_dvp;

#if PSRAM_SIZE
ALIGNED(4) static uint8_t dvp_buff[IMAGE_SENSOR_WIDTH*2*DVP_DMA_LINE_NUM] AT(.psram_imgage_sensor.dvp);//AT(.gpu_buf.buff);
ALIGNED(4) static uint8_t temp_buff[0x15000] AT(.gpu_buf.buff);// AT(.psram_imgage_sensor.temp);
#if VIDEO_RECOED_TAKE_PHOTO_SCALE_EN
ALIGNED(4) static uint8_t dvp_psram_full_buff[IMAGE_SENSOR_WIDTH*IMAGE_SENSOR_HEIGHT*2*3] AT(.psram_imgage_sensor.buff);
#else
ALIGNED(4) static uint8_t dvp_psram_full_buff[IMAGE_SENSOR_WIDTH*IMAGE_SENSOR_HEIGHT*2*1] AT(.psram_imgage_sensor.buff);
#endif // VIDEO_RECOED_TAKE_PHOTO_SCALE_EN
ALIGNED(4) static uint8_t dvp_disp_buff[DVP_DISP_WIDTH * DVP_DISP_HIGHT * 2 + 8] AT(.psram_imgage_sensor_disp.buff);
#else
ALIGNED(4) static uint8_t dvp_buff[IMAGE_SENSOR_WIDTH*2*DVP_DMA_LINE_NUM] AT(.gpu_buf.buff);
#endif

AT(.com_text.dvp)
u32 dvp_gpu2cpu_get(void)
{
    return dvp_gpu_share.gpu2cpu_bitmap;
}

AT(.com_text.dvp)
void dvp_gpu2cpu_clear(void)
{
    dvp_gpu_share.gpu2cpu_bitmap = 0;
}

AT(.com_text.dvp)
void dvp_gpu2disp_ready(void)
{
    if(img_dvp.is_init == false){
        img_dvp.is_ready = 0;
        return;
    }
    img_dvp.is_ready = 1;
    if (dvp_gpu_share.disp_buff != NULL) {
        img_dvp.disp_buff = (u8 *)dvp_gpu_share.disp_buff - 8;
    }
}


AT(.com_text.dvp)
bool dvp_disp_is_ready_and_clean(void)
{
    bool ret = img_dvp.is_ready;
    img_dvp.is_ready = false;

    return ret;
}


AT(.com_text.dvp)
u8* dvp_disp_get_buff(void)
{
    if (GET_LE32(&img_dvp.disp_buff[0]) != 0x24150 || GET_LE16(&img_dvp.disp_buff[4]) == 0 || GET_LE16(&img_dvp.disp_buff[6]) == 0) {
        printf("dvp_disp_get_buff<0x%x>:%d*%d\n", GET_LE32(&img_dvp.disp_buff[0]), GET_LE16(&img_dvp.disp_buff[4]), GET_LE16(&img_dvp.disp_buff[6]));
        printf("show err\n");
        return NULL;
//            halt(HALT_MALLOC);
    }

    return img_dvp.disp_buff;
//    return (u8*)dvp_gpu_share.disp_buff;
}


//拍照或录像模式，0录像，1拍照
void bsp_image_sensor_mode(bool video_or_photo)
{
    dvp_gpu_share.is_video_or_photo_mode = video_or_photo;
}


/**
 * bsp_image_sensor_init
 */
void bsp_image_sensor_init(bool is_reset)
{
    float ratio = 0;
    float ratio_w = 0;
    float ratio_h = 0;
    img_dvp.is_ready = 0;
    if(is_reset == true){
        memset(&dvp_gpu_share, 0, sizeof(dvp_share_t));
    }

//    if(dvp_gpu_share.disp_buff != NULL) {
        sys_clk_req(INDEX_DVP, DVP_CLK_SEL);

        ///摄像头初始化
        if(image_sensor_drv_register(sys_cb.image_sensor_idx) == false){
            sys_clk_free(INDEX_DVP);
            if (dvp_gpu_share.disp_buff != NULL) {
#if !PSRAM_SIZE
                ab_free((void *)dvp_gpu_share.disp_buff);
#endif
                dvp_gpu_share.disp_buff = NULL;
            }
            return;
        }

        ///ratio 缩放
        if (DVP_DISP_WIDTH && DVP_DISP_HIGHT) {
            if (img_drv_t->image_height >= DVP_DISP_HIGHT) {
            #if DVP_DISP_HIGHT
                ratio_h = (img_drv_t->image_height / DVP_DISP_HIGHT);
            #endif // DVP_DISP_HIGHT
                printf("img sensor ratio1:%d\n", ratio_h);

            }

            if (img_drv_t->image_width >= DVP_DISP_WIDTH) {
            #if DVP_DISP_HIGHT
                ratio_w = (img_drv_t->image_width / DVP_DISP_WIDTH);
            #endif // DVP_DISP_HIGHT
                printf("img sensor ratio2:%d\n", ratio_w);

            }

            ratio = (ratio_h < ratio_w) ? ratio_h:ratio_w;
            printf("img sensor ratio:%d\n", ratio);
            if (ratio == 0) {
                ratio = 1;
                printf("img sensor ratio check:%d\n", ratio);
            }

#if PSRAM_SIZE
            dvp_gpu_share.disp_buff = (u8*)psram_switch_cache(dvp_disp_buff);
            printf("image sensor disp psram buf(%d*%d):%d KB\n", DVP_DISP_WIDTH, DVP_DISP_HIGHT, (DVP_DISP_WIDTH * DVP_DISP_HIGHT * 2 + 8)/1024);
#else
        	void customer_heap_init(void);
        	customer_heap_init();
            dvp_gpu_share.disp_buff = ab_malloc((img_drv_t->image_width/ratio) * (img_drv_t->image_height/ratio) * 2 + 8);
            printf("image sensor disp buf malloc(%d*%d):%d KB\n", img_drv_t->image_width/ratio, img_drv_t->image_height/ratio,
                    (img_drv_t->image_width/ratio * img_drv_t->image_height/ratio * 2 + 8)/1024);
#endif // PSRAM_SIZE

        } else {
            dvp_gpu_share.disp_buff = NULL;
        }

        bsp_dvp_set_dbuf(dvp_buff, img_drv_t->image_width * 2, DVP_DMA_LINE_NUM);

        dvp_gpu_share.drive_type    = image_sensor_get_drv_type();
        dvp_gpu_share.dvp_buff      = (u8*)psram_switch_cache(dvp_buff);
        dvp_gpu_share.dvp_fifo_line = DVP_DMA_LINE_NUM;
        dvp_gpu_share.image_width   = img_drv_t->image_width;
        dvp_gpu_share.image_hight   = img_drv_t->image_height;

        printf("bsp_image_sensor_init:0x%x, type:%d\n", dvp_gpu_share.disp_buff, dvp_gpu_share.drive_type);


        if (DVP_DISP_WIDTH && DVP_DISP_HIGHT && dvp_gpu_share.disp_buff) {

            PUT_LE32(dvp_gpu_share.disp_buff, 0x24150);
            if (dvp_gpu_share.image_width < DVP_DISP_WIDTH) {
                PUT_LE16(&dvp_gpu_share.disp_buff[4], dvp_gpu_share.image_width);
            } else {
                PUT_LE16(&dvp_gpu_share.disp_buff[4], dvp_gpu_share.image_width/ratio);
            }
            if (dvp_gpu_share.image_hight < DVP_DISP_HIGHT) {
                PUT_LE16(&dvp_gpu_share.disp_buff[6], dvp_gpu_share.image_hight);
            } else {
                PUT_LE16(&dvp_gpu_share.disp_buff[6], dvp_gpu_share.image_hight/ratio);
            }
            if((u32)dvp_gpu_share.disp_buff >= 0x30000000){
                dcache_writeback_region ((void*)dvp_gpu_share.disp_buff, 8);
            }

            dvp_gpu_share.disp_buff     += 8;
            dvp_gpu_share.disp_is_ready  = true;
            if (img_drv_t->image_height < DVP_DISP_HIGHT && img_drv_t->image_width < DVP_DISP_WIDTH) {
                dvp_gpu_share.disp_width    = dvp_gpu_share.image_width;
                dvp_gpu_share.disp_hight    = dvp_gpu_share.image_hight;
                dvp_gpu_share.disp_fifo_line = DVP_DMA_LINE_NUM;
            } else {

                dvp_gpu_share.disp_width    = dvp_gpu_share.image_width / ratio;
                dvp_gpu_share.disp_hight    = dvp_gpu_share.image_hight / ratio;
                dvp_gpu_share.disp_fifo_line = DVP_DMA_LINE_NUM/(img_drv_t->image_height/dvp_gpu_share.disp_hight);
            }
            printf("image sensor disp buf(%d*%d):%d KB\n", dvp_gpu_share.disp_width, dvp_gpu_share.disp_hight,
                   (dvp_gpu_share.disp_width * dvp_gpu_share.disp_hight * 2 + 8)/1024);
        }
        dvp_gpu_share.is_rotate_90_cw = DVP_DISP_ROTATE_90;
        dvp_gpu_share.zoom_div      = 1;
        dvp_gpu_share.is_recode_full_screen = VIDEO_RECODE_FULL_SCREEN;

#if PSRAM_SIZE
        dvp_gpu_share.dvp_full_buff = (u8*)psram_switch_cache(dvp_psram_full_buff);
        dvp_gpu_share.zoom_buff = dvp_gpu_share.dvp_full_buff + dvp_gpu_share.image_width * dvp_gpu_share.image_hight * 2 * 2;

        if(dvp_gpu_share.is_rotate_90_cw){
            dvp_gpu_share.dvp_full_width  = dvp_gpu_share.image_hight;
            dvp_gpu_share.dvp_full_height = dvp_gpu_share.image_width;
        }else{
            dvp_gpu_share.dvp_full_width  = dvp_gpu_share.image_width;
            dvp_gpu_share.dvp_full_height = dvp_gpu_share.image_hight;
        }
        dvp_gpu_share.temp_buff     = temp_buff;
        dvp_gpu_share.temp_buff_size= sizeof(temp_buff);
#if VIDEO_RECOED_TAKE_PHOTO_SCALE_EN
        dvp_gpu_share.frame_buff_max    = 2;
#else
        dvp_gpu_share.frame_buff_max    = 1;
#endif // VIDEO_RECOED_TAKE_PHOTO_SCALE_EN
        dvp_gpu_share.frame_buff_count  = 0;
        dvp_gpu_share.frame_rd          = 0;
        dvp_gpu_share.frame_wr          = 0;

#endif
        img_dvp.is_init = true;
        dvp_gpu_share.is_drv_ready = true;
        bsp_dvp_dma_start();
//    }
}

AT(.com_text.dvp)
void bsp_image_sensor_set_state(bool state)
{
    img_dvp.is_init = state;
}
/**
 * bsp_image_sensor_uinit
 */
void bsp_image_sensor_uninit(void)
{
    printf("bsp_image_sensor_uninit\n");
    dvp_gpu_share.is_drv_ready = false;
    image_sensor_drv_unregister();
    delay_5ms(40);
    dvp_gpu_share.disp_is_ready  = false;
    img_dvp.is_init = false;

    if(dvp_gpu_share.disp_buff != NULL) {
#if PSRAM_SIZE==0
        printf("disp_buff free start:%x\n", (u8 *)dvp_gpu_share.disp_buff - 8);
//        ab_free((u8 *)dvp_gpu_share.disp_buff - 8);
        void customer_heap_init(void);
        customer_heap_init();
#endif
        dvp_gpu_share.disp_buff = NULL;
        printf("disp_buff free end..\n");
    }
    dvp_gpu_share.dvp_full_buff = NULL;
    sys_clk_free(INDEX_DVP);
}


bool bsp_image_sensor_is_init(void)
{
    return img_dvp.is_init;
}


/**
 * bsp_image_sensor_pwdn
 */
void bsp_image_sensor_pwdn(void)
{
    if(dvp_gpu_share.is_drv_ready){
        image_sensor_drv_unregister();
        delay_5ms(10);
        dvp_gpu_share.disp_is_ready  = false;
        img_dvp.is_init = false;

        if(dvp_gpu_share.disp_buff != NULL) {
#if !PSRAM_SIZE
            ab_free((u8 *)dvp_gpu_share.disp_buff - 8);
#endif
            dvp_gpu_share.disp_buff = NULL;
        }
        dvp_gpu_share.dvp_full_buff = NULL;
    }
}

/**
 * bsp_image_sensor_wkup
 */
void bsp_image_sensor_wkup(void)
{
    if(dvp_gpu_share.is_drv_ready){
        bsp_image_sensor_init(true);
    }
}


/**
 * bsp_image_sensor_change
 */
void bsp_image_sensor_change(void)
{
    if((!dvp_gpu_share.is_drv_ready) || bsp_video_recode_is_start() || bsp_take_photo_is_busy()
    ) {
        return;
    }
    os_gui_draw_w4_done();
    bsp_image_sensor_uninit();
    image_sensor_drv_change();
    bsp_image_sensor_init(false);
}


/**
 * bsp_image_sensor_width_get
 */
u16 bsp_image_sensor_width_get(void)
{
    return dvp_gpu_share.image_width;
}


/**
 * bsp_image_sensor_change
 */
u16 bsp_image_sensor_hight_get(void)
{
    return dvp_gpu_share.image_hight;
}



static void sensor_zoom_updata(void)
{
    printf("zoom:%d\n", dvp_gpu_share.zoom_div);

    dvp_gpu_share.zoom_width = (dvp_gpu_share.dvp_full_width * 10) / (dvp_gpu_share.zoom_div + 9);
    dvp_gpu_share.zoom_height = (dvp_gpu_share.dvp_full_height* 10) / (dvp_gpu_share.zoom_div + 9);
    dvp_gpu_share.zoom_x0 = (dvp_gpu_share.dvp_full_width - dvp_gpu_share.zoom_width) / 2;
    dvp_gpu_share.zoom_y0 = (dvp_gpu_share.dvp_full_height - dvp_gpu_share.zoom_height) / 2;
}

/**
 *数码变焦拉进镜头
 */
void bsp_sensor_zoom_add(void)
{
    if(dvp_gpu_share.zoom_div < ZOOM_MAX){
        dvp_gpu_share.zoom_div++;
        sensor_zoom_updata();
    }
}

/**
 *数码变焦拉远镜头
 */
void bsp_sensor_zoom_sub(void)
{
    if(dvp_gpu_share.zoom_div > 1){
        dvp_gpu_share.zoom_div--;
        sensor_zoom_updata();
    }
}

/**
 *获取当前倍数
 */
u8 bsp_sensor_zoom_get(void)
{
    return dvp_gpu_share.zoom_div;
}


/**
 * 大头贴
 */
static u8 sticker_cache[0x3400] AT(.img_sticker_buff.buff);
#if PSRAM_SIZE
ALIGNED(4) static uint8_t dvp_psram_sticker_buff[DVP_DISP_WIDTH*DVP_DISP_HIGHT*4] AT(.psram_imgage_sensor.buff);
#endif // PSRAM_SIZE
void bsp_image_sticker(u16 x0, u16 y0, u32 addr, u32 len, u8 div)
{
    bool ret;
    dvp_gpu_share.stick_is_ready = false;
    delay_ms(30);
    if(div > 2){
        div = 2;
    }

    if(addr != 0){
#if PSRAM_SIZE
        dvp_gpu_share.stick_img = (u8*)psram_switch_cache(dvp_psram_sticker_buff);
#else
        dvp_gpu_share.stick_img = NULL;
#endif // PSRAM_SIZE
        if (dvp_gpu_share.stick_img == NULL) {
            return;
        }
        ret = api_stick_fill(addr, len, sticker_cache, (u8 *)dvp_gpu_share.stick_img, (void *)&dvp_gpu_share.stick_width, (void *)&dvp_gpu_share.stick_height, 1);
        if(ret == true){
#if PSRAM_SIZE
            dcache_writeback_region((void *)dvp_gpu_share.stick_img, sizeof(dvp_psram_sticker_buff));
#endif // PSRAM_SIZE
            dvp_gpu_share.stick_x0 = x0;
            dvp_gpu_share.stick_y0 = y0;
            dvp_gpu_share.stick_is_ready = true;
            dvp_gpu_share.stick_div = div;
        }
    }
}

/**
 * 切换大头贴
 */
static const u32 sticker_pic[][5] = {
    //     图片                            大小                         显示位置      放大显示倍数
    {0,  0,  0,   0,  0},
    {UI_BUF_STICKER_STICKER1_BIN,  UI_LEN_STICKER_STICKER1_BIN,     0,     0,    2},
    {UI_BUF_STICKER_STICKER2_BIN,  UI_LEN_STICKER_STICKER2_BIN,     0,     0,    2},
    {UI_BUF_STICKER_STICKER1_BIN,  UI_BUF_STICKER_STICKER1_BIN,     201,   300,  2},
    {UI_BUF_STICKER_STICKER2_BIN,  UI_LEN_STICKER_STICKER2_BIN,     205,   255,  2},
    {UI_BUF_STICKER_STICKER1_BIN,  UI_LEN_STICKER_STICKER1_BIN,     183,   64,   2},
    {UI_BUF_STICKER_STICKER2_BIN,  UI_LEN_STICKER_STICKER2_BIN,     230,   20,   2},
    {UI_BUF_STICKER_STICKER1_BIN,  UI_LEN_STICKER_STICKER1_BIN,     200,   200,  2},
};

void bsp_image_sticker_add(void)
{
    sys_cb.sticker_idx++;
    if(sys_cb.sticker_idx >= sizeof(sticker_pic)/20){
        sys_cb.sticker_idx = 0;
    }

    bsp_image_sticker(sticker_pic[sys_cb.sticker_idx][2], sticker_pic[sys_cb.sticker_idx][3],
                      sticker_pic[sys_cb.sticker_idx][0], sticker_pic[sys_cb.sticker_idx][1],
                      sticker_pic[sys_cb.sticker_idx][4]);
}

void bsp_image_sticker_reduce(void)
{
    if(sys_cb.sticker_idx){
        sys_cb.sticker_idx--;
    } else {
        sys_cb.sticker_idx = sizeof(sticker_pic)/20 - 1;
    }


    printf("sticker:%d\n", sys_cb.sticker_idx);
    bsp_image_sticker(sticker_pic[sys_cb.sticker_idx][2], sticker_pic[sys_cb.sticker_idx][3],
                      sticker_pic[sys_cb.sticker_idx][0], sticker_pic[sys_cb.sticker_idx][1],
                      sticker_pic[sys_cb.sticker_idx][4]);
}

#endif // AVI_DVP_USE_CAMERA
