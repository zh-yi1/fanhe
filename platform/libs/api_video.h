#ifndef __API_VIDEO_H__
#define __API_VIDEO_H__
#include "include.h"
typedef enum {
    AVI_STA_PLAYING,
    AVI_STA_PAUSE,
    AVI_STA_END,
    AVI_STA_STOP
}AVISTA;


typedef struct {
    char path[32];                                  //路径，指直接存放的路径
    char ext[8];                                    //需要存储的文件后缀比如"avi"
    void (*index2name)(u16 index, char *name);      //客户实现index到name的转换，这样方便客户可以自定义名字规则。可以参考示例
    bool (*name2index)(char *name, u16 *index);     //客户实现name到index的转换，这样方便客户可以自定义名字规则。可以参考示例
    u16 index_max;                                  //一个循环最大index号，比如9999
    bool  rfile_type;                               //0：按时间分文件存储秒单位 1：按大小分文件存储 bytes单位
    u32 rfile_limit;                                //一个文件最大值。 这个和rfile_type有关，如果      rfile_type为0， rfile_limit表示秒时间；如果为1，表示大小；
    u8  free_per;                                   //磁盘剩余多少容量开始删除旧文件；如果设置0，表示取消循环录影。
    u8 *mjpeg_buff;
    u32 mjpeg_buff_size;
    u8 *pcm_buff;
    u32 pcm_buff_size;

    /*video param*/
    u16 width;
    u16 height;
    char compress[4];   //压缩方式
    u16 rate;           //帧率

    /*audio param*/
    u16 audio_samples;         //采样率；其他默认，PCM格式
    u8 chl;                    //声道，

    /*auto del*/
    u16 del_clust_size;        //循环录影一次删除多少个簇，默认256
}AVI_RECODE_PARAM;

enum {
    VIDEO_PLAY_FATFS,
    VIDEO_PLAY_FLASH,
};

typedef struct {
    u8 *avi_pcm_buff;
    u32 avi_pcm_buff_size;
    u8 *lseek_buff;
    u32 lseek_buff_size;
    u8 *avi_frame_buff;
    u32 avi_frame_buff_size;
    u8 *disp_buff;
    u32 disp_buff_size;
    u16 disp_width;
    u16 disp_height;
    u8 *avi_jpeg_fifo_buff;
    u32 avi_jpeg_fifo_buff_size;
    u32 res_addr;
    u32 seek_ofs;
    u8  type;
}video_play_t;

void bsp_jpgdec_init (u8 div);
void bsp_jpgdec_exit (void);

/**
 *  avi play
 *
 */
void* api_video_play_start(FIL* fp, video_play_t *video_play_p);

/**
 * @brief 停止AVI播放
 **/
void api_video_play_stop(void);

/**
 *  avi play get timestamp
 *  return ms;
 */
u32 api_video_play_get_times(void);

/**
 *  avi play get total timestamp
 *  return ms;
 */
u32 api_video_play_get_total_times(void);

/**
 *  avi play set timestamp
 *  return ms;
 */
bool api_video_play_set_times(u32 ms);

/**
 *  avi play info
 *
 */
bool api_video_play_info(u32 *total_ms, u32 *width, u32 *height, u32 *audio_samples);

/**
 *  avi play pp
 */
void api_video_play_pause(void);

/**
 *  avi play pp
 * 1: pause 0:playing
 */
void api_video_play_set_pp(bool is_pause);

/**
 *  avi play sta
 *
 */
AVISTA api_video_play_sta_get(void);


/**
 *  avi play frame is ready
 *
 */
bool api_video_play_frame_is_ready(void);

/**
 *  avi play frame is ready vaule
 *
 */
bool api_video_play_frame_read_value(void);


/**
    avi recode start
*/
u8 api_video_recode_start(AVI_RECODE_PARAM *param);

/**
    avi recode stop
*/
void api_video_recode_stop(u8 res);

/**
 *  avi play get timestamp
 *  return ms;
 */
u32 api_video_recode_get_times(void);

/**
 * avi recode api
 * 1:recoding 0:idle
 */
bool api_avi_recode_is_start(void);

/**
 *  video recode lock current file
 *  return FRESULT;
 */
FRESULT api_video_recode_lock(void);

/**
 *  video recode unlock current file
 *  return FRESULT;
 */
FRESULT api_video_recode_unlock(void);

/**
 *  video exsit audio
 *  return have;
 */
bool avi_audio_exsit_get(void);

void api_video_play(void);
void api_video_pause(void);

#endif

