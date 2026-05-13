#include "include.h"

typedef struct {
    volatile u8 quality;                 //JPEG ENC Quality 0~100;
    volatile u8 jpeg_format;             //0:H1V1 1:H2V1(support, line:16) 2:H1V2 3:H2V2(support, line:32)
    volatile u8 jpeg_yuv_grb;            //0:YUV422, 1:RGB565
    volatile u16 photo_width;
    volatile u16 photo_height;
    volatile u8 *jpeg_buff;
    volatile u32 jpeg_buff_size;
    volatile u8 *jpeg_out_buff;
    volatile u32 jpeg_out_size;
    volatile u8 *jpeg_scale_buff;
    volatile bool enc_is_start;
    volatile bool take_photo_is_start;
    volatile u32 take_photo_GPU2CPU_tick;
    volatile bool is_continue;
}photo_jpeg_share_t;

photo_jpeg_share_t dvp_photo_jpeg_share AT(.share_photo_jpeg.buff);

#if VIDEO_RECODE_TAKE_PHOTO_EN

/** 底层调用**/
/**
 *  bsp avi recode
 */
#define F_NAME   "PIC"

static void photo_index2name(WORD index, char *name)
{
    memcpy(name, F_NAME, 3);
    index = index%10000;

    name[3] = index/10000 + '0';
    name[4] = index%10000/1000 + '0';
    name[5] = index%1000/100 + '0';
    name[6] = index%100/10 + '0';
    name[7] = index%10 + '0';
    memcpy(&name[8], ".jpg\0", 5);
    printf("index:%d -> %s\n", index, name);
}

static bool photo_name2index(char *name, WORD *index)
{
    if((memcmp(name, F_NAME, 3) != 0) || (memcmp(name+8, ".jpg", 4) != 0)){
        return false;
    }

    *index = 0;
    for(u8 i=3; i<=7; i++){
        if((name[i] < '0') || (name[i] > '9')){
            return false;
        }
        *index *= 10;
        *index += name[i] - '0';
    }

    //printf("name:%s -> %d\n", name, *index);
    return true;
}


FRESULT take_photo_file_create(u32 size)
{
    FRESULT res = FR_DISK_ERR;
    if (bsp_sd_mount_state()) {
        if (fs_getfree("B:") > size) {
            CIRCST *photo_circst = (CIRCST *)fs_circ_container();
            memset(photo_circst, 0, fs_circ_container_size());
            strcpy(photo_circst->path, "B:\\PIC\0");    //sd
            strcpy(photo_circst->ext, "*.jpg\0");
            photo_circst->index2name = photo_index2name;
            photo_circst->name2index = photo_name2index;
            photo_circst->index_max  = 9999;
            photo_circst->rfile_limit = -1;
            photo_circst->rfile_type = 0;
            photo_circst->swap_hook  = NULL;
            photo_circst->temp_file_is_creat = false;
            photo_circst->del_clust_size = 256;
            res  = f_circ_open(photo_circst, 0);
            printf("take photo save disk => SD FATFS <%d>\n", res);
        }
    } else if (bsp_flash_mount_state()) {
        if (bsp_flash_disk_get_free() > size) {
            CIRCST *photo_circst = (CIRCST *)fs_circ_container();
            memset(photo_circst, 0, fs_circ_container_size());
            strcpy(photo_circst->path, "A:\\PIC\0");    //FLASH DISK
            strcpy(photo_circst->ext, "*.jpg\0");
            photo_circst->index2name = photo_index2name;
            photo_circst->name2index = photo_name2index;
            photo_circst->index_max  = 9999;
            photo_circst->rfile_limit = -1;
            photo_circst->rfile_type = 0;
            photo_circst->swap_hook  = NULL;
            photo_circst->temp_file_is_creat = false;
            photo_circst->del_clust_size = 256;
            res  = f_circ_open(photo_circst, 0);
            printf("take photo save disk => FLASH FATFS <%d>\n", res);
        }
    } else {            //other
        printf("take photo save disk => OTHER\n");
        res = FR_OK;
    }
    return res;
}

FRESULT take_photo_file_write(const void* buff, UINT btw)
{

    if (sys_cb.camera_take_photo_ble_send) {
        sys_cb.camera_take_photo_ble_send((u8*)buff, btw);
    }

    if (bsp_sd_mount_state()) {
        CIRCST *photo_circst = (CIRCST *)fs_circ_container();
        return f_circ_write(photo_circst, buff, btw, 0);
    } else if (bsp_flash_mount_state()) {
        CIRCST *photo_circst = (CIRCST *)fs_circ_container();
        return f_circ_write(photo_circst, buff, btw, 0);
    }

    return FR_DISK_ERR;
}

void take_photo_file_close(void)
{
    if (bsp_sd_mount_state()) {
        CIRCST *photo_circst = (CIRCST *)fs_circ_container();
        f_circ_close(photo_circst);
    } else if (bsp_flash_mount_state()) {
        CIRCST *photo_circst = (CIRCST *)fs_circ_container();
        f_circ_close(photo_circst);
    } else {

    }
}


typedef struct {
    u16 photo_width;
    u16 photo_height;
    u8 *jpeg_buff;
    u32 jpeg_buff_size;
    u8 *jpeg_scale_buff;
    u8 resolut_type;
    bool take_photo_is_running;
}take_photo_param_t;

extern void bsp_watermark_set(bool is_visible, u16 x_ofs, u16 y_ofs);

#if PSRAM_SIZE == 0
static u8 take_mjpeg_buff[0x10000] AT(.take_photo_buf.buf);
#endif

#if PSRAM_SIZE
//static u8 take_psram_jpeg_buff[840*1024] AT(.psram_take_photo.buf);
static u8 take_psram_jpeg_buff[600*1024] AT(.psram_take_photo.buf);
static u8 jpeg_psram_scale_buff[256*1024] AT(.psram_take_photo.buf);
#endif

static take_photo_param_t take_photo_param;

/**
 * jpeg encode output
 */
u32 photo_encode_jpeg_size_get(void)
{
    return dvp_photo_jpeg_share.jpeg_out_size;
}

u8* photo_encode_jpeg_frame_get(void)
{
    return (u8 *)dvp_photo_jpeg_share.jpeg_out_buff;
}
#if PSRAM_SIZE == 0
static bool take_photo_jpeg_write(u8 *buff, u32 len)
{
    FRESULT res;

//    CIRCST *photo_circst = (CIRCST *)fs_circ_container();
//    res = f_circ_write(photo_circst, buff, len, 0);

    res = take_photo_file_write(buff, len);

    return (res == FR_OK);
}
#endif
/**
 *  take photo callback
 */
void video_take_photo_callback(u8 *buff, u32 size)
{

    printf("take photo size:%d\n", size);
#if PSRAM_SIZE == 0
    u8 ret = 0;

    if(take_photo_param.photo_width == bsp_image_sensor_width_get()){
        if (take_photo_file_create(size) == FR_OK) {
            take_photo_file_write(buff, size);
            take_photo_file_close();
        }
    }else{
        printf("api_photo_recode_write\n");
        dvp_photo_jpeg_share.take_photo_GPU2CPU_tick = 20 * 30; // 20s
        memcpy(take_mjpeg_buff, buff, size);
        bsp_jpgdec_init(1);
        if (take_photo_file_create(size) == FR_OK) {
            ret = api_photo_recode_write(take_mjpeg_buff, size,
                                         dvp_photo_jpeg_share.photo_width, dvp_photo_jpeg_share.photo_height,
                                         bsp_video_jpeg_buff_get(), IMAGE_SENSOR_WIDTH*2*16 + 8,
                                         dvp_disp_get_buff() + 8, DVP_DISP_WIDTH * DVP_DISP_HIGHT * 2,
                                         bsp_video_jpeg_buff_get() + IMAGE_SENSOR_WIDTH*2*16 + 8, IMAGE_SENSOR_WIDTH*100,
                                         take_photo_jpeg_write);
            take_photo_file_close();
        }
    }

    dvp_photo_jpeg_share.take_photo_GPU2CPU_tick = 0;
    dvp_photo_jpeg_share.take_photo_is_start = false;
    take_photo_param.take_photo_is_running   = false;

    printf("photo complete:%d\n", ret);
#else
	bool is_continue = dvp_photo_jpeg_share.is_continue;
    dcache_invalidate_region ((void*)(((u32)buff) | BIT(28)), size);
    if (take_photo_file_create(size) == FR_OK) {
        take_photo_file_write(buff, size);
        take_photo_file_close();
    }

    if(is_continue == 0){
//        take_photo_file_close();
        dvp_photo_jpeg_share.take_photo_GPU2CPU_tick = 0;
        dvp_photo_jpeg_share.take_photo_is_start = false;
        take_photo_param.take_photo_is_running   = false;
        printf("photo complete\n");
    }
#endif
}

/**
 * take photo
 */
void bsp_video_take_photo(void)
{
    static u32 ticks = 0;
//    u8 res;

    printf("bsp_video_take_photo\n");
    if(bsp_video_recode_is_start() || (api_video_play_sta_get() != AVI_STA_STOP) || (bsp_image_sensor_is_init() == false)){
        return;
    }

    if(take_photo_param.take_photo_is_running == true){       //避免极端下，GPU2CPU没收到信息，导致这里一直为true
        if(tick_check_expire(ticks, 12000)){
            take_photo_param.take_photo_is_running = false;
        }else{
            return;
        }
    }

    ticks = tick_get();
#if USB_JPEG_CAMERA
    if(uhs_is_usb_uvc()){
         dvp_photo_jpeg_share.take_photo_is_start = true;
    }
#endif
//    res = take_photo_file_create();

//    if(res == FR_OK){
        jpeg_hw_init();

        dvp_photo_jpeg_share.quality        = 80;       //JPEG ENC Quality 0~100;
        dvp_photo_jpeg_share.jpeg_format    = 1;        //0:H1V1 1:H2V1(support, line:16) 2:H1V2 3:H2V2(support, line:32)
        dvp_photo_jpeg_share.jpeg_yuv_grb   = 1;        //0:YUV422, 1:RGB565
        dvp_photo_jpeg_share.photo_width    = take_photo_param.photo_width;
        dvp_photo_jpeg_share.photo_height   = take_photo_param.photo_height;
        dvp_photo_jpeg_share.enc_is_start   = 0;
        dvp_photo_jpeg_share.jpeg_buff      = take_photo_param.jpeg_buff;
        dvp_photo_jpeg_share.jpeg_buff_size = take_photo_param.jpeg_buff_size;
        dvp_photo_jpeg_share.jpeg_scale_buff= take_photo_param.jpeg_scale_buff;
        bsp_watermark_set(sys_cb.datelabel, 10, take_photo_param.photo_height - 40);
        bsp_photo_recode_watermark_print(true);

#if PSRAM_SIZE
        if(dvp_photo_jpeg_share.jpeg_buff == NULL){
            printf("ERR take photo jpeg_buff NULL\n");
            return;
        }
#endif
        os_gui_draw_w4_done();
        take_photo_param.take_photo_is_running   = true;
        dvp_photo_jpeg_share.take_photo_GPU2CPU_tick = 0;
        dvp_photo_jpeg_share.take_photo_is_start = true;
//    }
//    printf("res:%d\n", res);
}

/**
 * 像素选择
 * [0]:宽 [1]:高 [2]:存储需要的buff大小
 */
static const u32 photo_resolut_table[][2] =
{
#if PSRAM_SIZE
    [PHOTO_RESOLUT_VGA] = {IMAGE_SENSOR_WIDTH, IMAGE_SENSOR_HEIGHT},
    [PHOTO_RESOLUT_2M] = {1600, 1200},
    [PHOTO_RESOLUT_3M] = {2048, 1536},
    [PHOTO_RESOLUT_5M] = {2592, 1944},
    [PHOTO_RESOLUT_8M] = {3264, 2448},
    [PHOTO_RESOLUT_10M] = {3648, 2376},
    [PHOTO_RESOLUT_12M] = {4000, 3000},
    [PHOTO_RESOLUT_15M] = {4800, 3600},
    [PHOTO_RESOLUT_20M] = {5120, 3840},
    [PHOTO_RESOLUT_30M] = {6400, 4800},
    [PHOTO_RESOLUT_32M] = {7680, 5760},
    [PHOTO_RESOLUT_48M] = {8000, 6000},
#else
    [PHOTO_RESOLUT_VGA] = {IMAGE_SENSOR_WIDTH, IMAGE_SENSOR_HEIGHT},
    [PHOTO_RESOLUT_2M] = {IMAGE_SENSOR_WIDTH*2, IMAGE_SENSOR_HEIGHT*2},
    [PHOTO_RESOLUT_3M] = {IMAGE_SENSOR_WIDTH*3, IMAGE_SENSOR_HEIGHT*3},
    [PHOTO_RESOLUT_5M] = {IMAGE_SENSOR_WIDTH*4, IMAGE_SENSOR_HEIGHT*4},
    [PHOTO_RESOLUT_8M] = {IMAGE_SENSOR_WIDTH*5, IMAGE_SENSOR_HEIGHT*5},
    [PHOTO_RESOLUT_10M] = {IMAGE_SENSOR_WIDTH*6, IMAGE_SENSOR_HEIGHT*6},       //最大到这里
    [PHOTO_RESOLUT_12M] = {IMAGE_SENSOR_WIDTH*6, IMAGE_SENSOR_HEIGHT*6},
    [PHOTO_RESOLUT_15M] = {IMAGE_SENSOR_WIDTH*6, IMAGE_SENSOR_HEIGHT*6},
    [PHOTO_RESOLUT_20M] = {IMAGE_SENSOR_WIDTH*6, IMAGE_SENSOR_HEIGHT*6},
    [PHOTO_RESOLUT_30M] = {IMAGE_SENSOR_WIDTH*6, IMAGE_SENSOR_HEIGHT*6},
    [PHOTO_RESOLUT_32M] = {IMAGE_SENSOR_WIDTH*6, IMAGE_SENSOR_HEIGHT*6},
    [PHOTO_RESOLUT_48M] = {IMAGE_SENSOR_WIDTH*6, IMAGE_SENSOR_HEIGHT*6},
#endif
};

void bsp_take_photo_set_resolut(PHOTO_RESOLUT_TYPE resolut)
{
    u32 width, height;

    if(bsp_video_recode_is_start() == true){
        return;
    }
    if(take_photo_param.take_photo_is_running == true){
        return;
    }
#if PSRAM_SIZE == 0
    if(resolut >= PHOTO_RESOLUT_10M){
        resolut = PHOTO_RESOLUT_10M;
    }
#endif

#if DVP_DISP_ROTATE_90 == 0
    width  = photo_resolut_table[resolut][0];
    height = photo_resolut_table[resolut][1];
#else
    width  = photo_resolut_table[resolut][1];
    height = photo_resolut_table[resolut][0];
#endif

    take_photo_param.resolut_type     = resolut;
    take_photo_param.photo_width      = width;
    take_photo_param.photo_height     = height;
#if PSRAM_SIZE
    if(take_photo_param.jpeg_buff == NULL){
        take_photo_param.jpeg_buff      = (u8*)psram_switch_cache(take_psram_jpeg_buff); //1.5M
        take_photo_param.jpeg_buff_size = sizeof(take_psram_jpeg_buff);
        if(take_photo_param.jpeg_buff == NULL){
            printf("ERR1: no more ram\n");
        }
    }

    if(take_photo_param.jpeg_scale_buff == NULL){
        take_photo_param.jpeg_scale_buff = (u8*)psram_switch_cache(jpeg_psram_scale_buff);
        if(take_photo_param.jpeg_scale_buff == NULL){
            printf("ERR2: no more ram\n");
        }
    }

#else
    //take_photo_param.jpeg_buff      = take_mjpeg_buff;
    //take_photo_param.jpeg_buff_size = 0x10000;
#endif

}

PHOTO_RESOLUT_TYPE bsp_take_photo_resolut_get_type(void)
{
    return take_photo_param.resolut_type;
}

void bsp_take_photo_exit(void)
{
    take_photo_param.jpeg_scale_buff = NULL;
    take_photo_param.jpeg_buff      = NULL;
    take_photo_param.jpeg_buff_size = 0;
    take_photo_param.take_photo_is_running = 0;
}


void bsp_take_photo_wait_complete(void)
{
    while(take_photo_param.take_photo_is_running){
        WDT_CLR();
        vusb4s_reset_clr_cnt();
    }
}


bool bsp_take_photo_is_busy(void)
{
    return take_photo_param.take_photo_is_running;
}



/**
 * 水印添加
 */
void bsp_photo_recode_watermark_print(bool is_frist)
{
#if WATER_MARK_EN
    static u32 rtccnt_bk = 0;
    char tm_buff[32];
    //tm_t tm;

    if((is_frist) || (rtccnt_bk != RTCCNT)){
        if(dvp_photo_jpeg_share.enc_is_start == 0){
            rtccnt_bk = RTCCNT;
            //tm = rtc_clock_get();
            snprintf(tm_buff, 32, "%04d/%02d/%02d  %02d:%02d:%02d",
                    compo_cb.tm.year, compo_cb.tm.mon, compo_cb.tm.day, compo_cb.tm.hour, compo_cb.tm.min, compo_cb.tm.sec);
            watermark_put_str((u8*)tm_buff);
        }
    }
#endif // WATER_MARK_EN
}

#else

void video_take_photo_callback(u8 *buff, u32 size) {}

#endif // VIDEO_RECODE_TAKE_PHOTO_EN
