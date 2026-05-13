#include "include.h"



video_jpeg_share_t dvp_video_jpeg_share AT(.share_video_jpeg.buff);

#if VIDEO_RECODE_TAKE_PHOTO_EN
extern void avi_frame_init(u8 *buff, u32 buff_size);
extern bool avi_put_frame(u8 *buf, u16 len);
extern u32 avi_get_frame(u8 *rx_buff, u16 len);
extern u32 avi_frame_stored_get(void);
extern u32 avi_frame_max_get(void);

typedef struct {
    u16 video_width;
    u16 video_height;
    u8 *mjpeg_buff;
    u32 mjpeg_buff_size;
    u16 watermark_x_ofs;
    u16 watermark_y_ofs;
    u8 resolut_type;
    u8 quality;
}video_recode_param_t;
#define VIDEO_RECODE_AUDIO_LEN  8192

static u8 recode_sdadc_buff[VIDEO_RECODE_AUDIO_LEN] AT(.video_recode.buf);
static u8 recode_pcm_buff[VIDEO_RECODE_AUDIO_LEN + 32] AT(.video_recode.buf);
#if PSRAM_SIZE == 0
ALIGNED(4) static uint8_t jpeg_buff[0x15000] AT(.gpu_buf.buff);
static u8 mjpeg_buff[0x10000] AT(.jpeg_buf.buf);
#endif
static video_recode_param_t video_recode_param;
#if PSRAM_SIZE

static u8 mjpeg_psram_buff[300*1024] AT(.psram_video_recode.buf);
static u8 dvp_jpeg_psram_buff[300*1024*2] AT(.psram_video_recode.buf);
#endif


#if PSRAM_SIZE == 0
u8 *bsp_video_jpeg_buff_get(void)
{
    return jpeg_buff;
}
#endif
/**
 *  bsp avi recode
 */
#define F_NAME   "MOVI"

void fcirc_index2name(WORD index, char *name)
{
    memcpy(name, F_NAME, 4);
    index = index%10000;

    name[4] = index/1000 + '0';
    name[5] = index%1000/100 + '0';
    name[6] = index%100/10 + '0';
    name[7] = index%10 + '0';
    memcpy(&name[8], ".avi\0", 5);
    sys_cb.start_vedio_flag = true;
    printf("index:%d -> %s\n", index, name);
}

bool fcirc_name2index(char *name, WORD *index)
{
    if((memcmp(name, F_NAME, 4) != 0) || (memcmp(name+8, ".avi", 4) != 0)){
        return false;
    }

    *index = 0;
    for(u8 i=4; i<=7; i++){
        if((name[i] < '0') || (name[i] > '9')){
            return false;
        }
        *index *= 10;
        *index += name[i] - '0';
    }

    //printf("name:%s -> %d\n", name, *index);

    return true;
}

void jpeg_hw_init(void)
{
    printf("jpeg_hw_init\n");
    CLKDIVCON3 = (CLKDIVCON3 & ~(BIT(2)*0x3)) | BIT(2) * 1;
    RSTCON0 &= ~BIT(23);
    CLKGAT3 |=  BIT(7);
    RSTCON0 |=  BIT(23);

    JENCCON = 0;
    JENCDRICON = 0;
    JENCMCUCON = 0;
    JENCCON = 0
        | BIT(8) * 0b111100
        | BIT(4) * 0b110
        | BIT(0)
        ;
    JENCPEND = BIT(1)|BIT(2)|BIT(0);
}

void bsp_watermark_set(bool is_visible, u16 x_ofs, u16 y_ofs)
{
#if WATER_MARK_EN
    if (is_visible) {
            dvp_video_jpeg_share.watermark_x                   = x_ofs;
    } else {
            dvp_video_jpeg_share.watermark_x                   = 60000;
    }
    dvp_video_jpeg_share.watermark_y                   = y_ofs;
    dvp_video_jpeg_share.watermark_color           = COLOR_YELLOW;
    dvp_video_jpeg_share.water_mark_height           = watermark_get_height();
    dvp_video_jpeg_share.water_mark_width_bytes= watermark_get_width_bytes();
    dvp_video_jpeg_share.watermark_buff            = (u8 *)watermark_get_buff();
#endif // WATER_MARK_EN
}

void bsp_video_recode_start(void)
{
    AVI_RECODE_PARAM recode_param;
    u8 res = -1;

    if((dvp_video_jpeg_share.video_recode_is_start == true) || (bsp_image_sensor_is_init() == false)){
        return;
    }
    printf("bsp_video_recode_start\n");
    strcpy(recode_param.path, "B:\\DCIM");
    strcpy(recode_param.ext, "*.avi");
    recode_param.index2name = fcirc_index2name;
    recode_param.name2index = fcirc_name2index;
    recode_param.index_max  = 9999;
    recode_param.rfile_limit = sys_cb.rfile_limit;//180; //sec
    recode_param.rfile_type  = 0;
    recode_param.free_per    = 0;

    recode_param.width  = video_recode_param.video_width;
    recode_param.height = video_recode_param.video_height;
    memcpy(recode_param.compress, "MJPG", 4);
    recode_param.rate   = 30;

    recode_param.audio_samples = 16000;
    recode_param.chl           = 1;

    recode_param.del_clust_size = 256;

#if PSRAM_SIZE == 0
    recode_param.mjpeg_buff      = mjpeg_buff;
    recode_param.mjpeg_buff_size = sizeof(mjpeg_buff);
#else
    video_recode_param.mjpeg_buff = (u8*)psram_switch_cache(mjpeg_psram_buff);
    video_recode_param.mjpeg_buff_size = sizeof(mjpeg_psram_buff);
    recode_param.mjpeg_buff      = video_recode_param.mjpeg_buff;
    recode_param.mjpeg_buff_size = video_recode_param.mjpeg_buff_size;
#endif

    recode_param.pcm_buff = recode_pcm_buff;
    recode_param.pcm_buff_size = sizeof(recode_pcm_buff);

    res = api_video_recode_start(&recode_param);
    if(res == 0){
        jpeg_hw_init();
        dvp_video_jpeg_share.quality               = video_recode_param.quality;
        dvp_video_jpeg_share.enc_is_start          = 0;
        dvp_video_jpeg_share.jpeg_format           = 1;   //H2V1
        dvp_video_jpeg_share.jpeg_yuv_grb          = 1;   //0:YUV422, 1:RGB565
        dvp_video_jpeg_share.jpeg_encode_lock      = 0;
#if PSRAM_SIZE == 0
        dvp_video_jpeg_share.jpeg_buff             = jpeg_buff;
        dvp_video_jpeg_share.jpeg_buff_size        = sizeof(jpeg_buff);
#else
        dvp_video_jpeg_share.jpeg_buff             = (u8*)psram_switch_cache(dvp_jpeg_psram_buff);
        dvp_video_jpeg_share.jpeg_buff_size        = sizeof(dvp_jpeg_psram_buff);
#endif
        dvp_video_jpeg_share.is_auto_quality       = true;
        if(DVP_DMA_LINE_NUM < 32){
            dvp_video_jpeg_share.jpeg_format       = 1;   //H2V2
        }

        bsp_watermark_set(sys_cb.datelabel, video_recode_param.watermark_x_ofs, video_recode_param.watermark_y_ofs);
        // if (sys_cb.datelabel) {
        //     dvp_video_jpeg_share.watermark_x           = video_recode_param.watermark_x_ofs;
        // } else {
        //     dvp_video_jpeg_share.watermark_x           = 10000;
        // }
        // dvp_video_jpeg_share.watermark_y           = video_recode_param.watermark_y_ofs;
        // dvp_video_jpeg_share.watermark_color       = COLOR_YELLOW;
        // dvp_video_jpeg_share.water_mark_height     = watermark_get_height();
        // dvp_video_jpeg_share.water_mark_width_bytes= watermark_get_width_bytes();
        // dvp_video_jpeg_share.watermark_buff        = (u8 *)watermark_get_buff();

        //分辨率支持VGA 720P
        dvp_video_jpeg_share.video_width           = video_recode_param.video_width;
        dvp_video_jpeg_share.video_height          = video_recode_param.video_height;

        dvp_video_jpeg_share.curr_quality          = dvp_video_jpeg_share.quality;
        bsp_video_recode_watermark_print(true);
        dvp_video_jpeg_share.video_recode_is_start = true;
    }else{
        video_recode_param.mjpeg_buff = NULL;
    }

    printf("res:%d\n", res);
    bsp_sys_unmute();
}

void bsp_video_recode_stop(void)
{
    printf("bsp_video_recode_stop\n");
    dvp_video_jpeg_share.video_recode_is_start = false;
    api_video_recode_stop(0);
    delay_5ms(20);

    dvp_video_jpeg_share.jpeg_buff = NULL;
    video_recode_param.mjpeg_buff = NULL;

    sys_clk_free(INDEX_VIDEO);
    bsp_sys_mute();
}


bool bsp_video_recode_is_start(void)
{
    return dvp_video_jpeg_share.video_recode_is_start;
}

/**
 * 像素选择
 */
static const u16 video_resolut_table[3][3] =
{
    [VIDEO_RESOLUT_VGA]   = {IMAGE_SENSOR_WIDTH, IMAGE_SENSOR_HEIGHT, 60},
    [VIDEO_RESOLUT_720P]  = {1280, 720, 50},
    [VIDEO_RESOLUT_1080P] = {1920, 1080, 40},
};
void bsp_video_recode_set_resolut(VIDEO_RESOLUT_TYPE resolut)
{
    u16 width, height;
    if(bsp_video_recode_is_start() == true){
        return;
    }

	if(resolut > VIDEO_RESOLUT_1080P){
		resolut =  VIDEO_RESOLUT_VGA;
	}

#if DVP_DISP_ROTATE_90 == 0
    width  = video_resolut_table[resolut][0];
    height = video_resolut_table[resolut][1];
#else
    width  = video_resolut_table[resolut][1];
    height = video_resolut_table[resolut][0];
#endif
    // usb 摄像头出来的数据就是 640*480 没有经过jpegdecode 转换
#if USB_JPEG_CAMERA
    if (uhs_is_usb_uvc()) {
        width = usb_uvc.width;
        height = usb_uvc.height;

    }
#endif
    video_recode_param.resolut_type     = resolut;
    video_recode_param.watermark_x_ofs  = 10;                //水印位置
    video_recode_param.watermark_y_ofs  = height - 40;
    video_recode_param.video_width      = width;
    video_recode_param.video_height     = height;
    video_recode_param.quality          = video_resolut_table[resolut][2];

    dvp_video_jpeg_share.video_width    = width;
    dvp_video_jpeg_share.video_height   = height;

}

VIDEO_RESOLUT_TYPE bsp_video_resolut_get_type(void)
{
    return video_recode_param.resolut_type;
}

/**
 *  avi 开始录制回调用
 */
void avi_recode_init_hook(void)
{
    avi_frame_init(recode_sdadc_buff, sizeof(recode_sdadc_buff));
    audio_path_init(AUDIO_PATH_VIDEO_REC);
    audio_path_start(AUDIO_PATH_VIDEO_REC);
    sys_clk_req(INDEX_VIDEO, VIDEO_CLK_SEL);
}

/**
 *  avi 退出录制回调
 */
void avi_recode_exit_hook(void)
{
    printf("avi_recode_exit_hook\n");
    dvp_video_jpeg_share.video_recode_is_start = false;
#if PSRAM_SIZE
    if((dvp_video_jpeg_share.jpeg_buff != NULL) || (video_recode_param.mjpeg_buff != NULL)){
        delay_5ms(20);
    }
    dvp_video_jpeg_share.jpeg_buff = NULL;
    video_recode_param.mjpeg_buff = NULL;
#endif
    audio_path_exit(AUDIO_PATH_VIDEO_REC);
    sys_clk_free(INDEX_VIDEO);
}

AT(.com_text.video)
void jpeg_encode_lock_clean(void)
{
    dvp_video_jpeg_share.jpeg_encode_lock = 0;
}

/**
 * jpeg encode output
 */
AT(.com_text.avi)
u32 video_encode_jpeg_size_get(void)
{
    return dvp_video_jpeg_share.jpeg_out_size;
}
AT(.com_text.avi)
u8* video_encode_jpeg_frame_get(void)
{
    if((u32)dvp_video_jpeg_share.jpeg_out_buff >= 0x30000000){
        dcache_invalidate_region ((void*)dvp_video_jpeg_share.jpeg_out_buff, dvp_video_jpeg_share.jpeg_out_size);
    }
    return (u8 *)dvp_video_jpeg_share.jpeg_out_buff;
}

/**
 * 录影音频部分
 */
AT(.com_text.video)
void video_recode_sdadc_proecess(u8 *ptr, u32 samples, u32 pcm_mode)
{
    avi_put_frame(ptr, samples * 2 * (pcm_mode & PCM_CHMASK));
    if(dvp_video_jpeg_share.jpeg_encode_lock){
        dvp_video_jpeg_share.jpeg_encode_lock--;
    }
}

u32 avi_recode_get_audio_len(void)
{
    return avi_frame_stored_get();
}

void avi_recode_get_audio_buff(u8 *frame_buff, u32  len)
{
    avi_get_frame(frame_buff, len);
}

u32 avi_recode_get_audio_max_len(void)
{
    return avi_frame_max_get();
}

/**
 * 水印添加
 */
void bsp_video_recode_watermark_print(bool is_frist)
{
#if WATER_MARK_EN
    static u32 rtccnt_bk = 0;
    char tm_buff[32];
    //tm_t tm;

    if((is_frist) || (rtccnt_bk != RTCCNT)){
        if(dvp_video_jpeg_share.enc_is_start == 0){
            rtccnt_bk = RTCCNT;
            //tm = rtc_clock_get();
            snprintf(tm_buff, 32, "%04d/%02d/%02d  %02d:%02d:%02d",
                    compo_cb.tm.year, compo_cb.tm.mon, compo_cb.tm.day, compo_cb.tm.hour, compo_cb.tm.min, compo_cb.tm.sec);
            watermark_put_str((u8*)tm_buff);
        }
    }
#endif // WATER_MARK_EN
}
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
