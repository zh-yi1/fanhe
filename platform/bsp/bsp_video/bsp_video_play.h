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
 * avi���Ź����У�������Դ������Ƶ
 */
void bsp_video_mp3_res_play(u32 addr, u32 len);

/**
 * avi���ų�ʼ��
 */
void bsp_video_play_init(u32 res_addr);

/**
 * video play uninit
 */
void bsp_video_play_uninit(void);

void bsp_video_play_audio_keepalive(void);

#endif
