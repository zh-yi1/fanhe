#ifndef __BSP_AVI_H__
#define __BSP_AVI_H__


/**
 * bsp avi play
 */
void *bsp_video_play(FIL* fp);

/**
 * bsp avi play stop
 */
void bsp_video_stop(void);

/**
 * avi播放过程中，播放资源区的音频
 */
void bsp_video_mp3_res_play(u32 addr, u32 len);

/**
 * avi播放初始化
 */
void bsp_video_play_init(u32 res_addr);

/**
 * video play uninit
 */
void bsp_video_play_uninit(void);

#endif
