#ifndef __BSP_VIDEO_RECODE_H__
#define __BSP_VIDEO_RECODE_H__


typedef enum {
    VIDEO_RESOLUT_VGA = 0,
    VIDEO_RESOLUT_720P,
    VIDEO_RESOLUT_1080P,
}VIDEO_RESOLUT_TYPE;

typedef struct {
    volatile u8 quality;                 //JPEG ENC Quality 0~100;
    volatile u8 jpeg_format;             //0:H1V1 1:H2V1(support, line:16) 2:H1V2 3:H2V2(support, line:32)
    volatile u8 jpeg_yuv_grb;            //0:YUV422, 1:RGB565
    volatile u8 *jpeg_buff;
    volatile u32 jpeg_buff_size;
    volatile u8 *jpeg_out_buff;
    volatile u32 jpeg_out_size;
    volatile u16 video_width;
    volatile u16 video_height;
    volatile u8 curr_quality;
    volatile u16 watermark_x;
    volatile u16 watermark_y;
    volatile u16 watermark_color;
    volatile u8 water_mark_height;
    volatile u8 water_mark_width_bytes;
    volatile u8 *watermark_buff;
    volatile u8 jpeg_encode_lock;
    volatile bool enc_is_start;
    volatile bool video_recode_is_start;
    volatile bool is_auto_quality;
}video_jpeg_share_t;

extern video_jpeg_share_t dvp_video_jpeg_share;



/**
 * 开始录像
 */
void bsp_video_recode_start(void);

/**
 * 停止录像
 */
void bsp_video_recode_stop(void);

/**
 * 判断是否在录像
 */
bool bsp_video_recode_is_start(void);

/**
 * 水印添加
 */
void bsp_video_recode_watermark_print(bool is_frist);

/**
 * 设置分辨率
 *  640*480
 *  1280*720
 */
void bsp_video_recode_set_resolut(VIDEO_RESOLUT_TYPE resolut);

/**
 * 获取分辨率类型
 */
VIDEO_RESOLUT_TYPE bsp_video_resolut_get_type(void);







#endif




