#ifndef _MUSIC_RES_H_
#define _MUSIC_RES_H_

void mp3_res_play(u32 addr, u32 len);
void mp3_res_play_w4_done(void);
void wav_res_play(u32 addr, u32 len);

/**
 * @brief 检查mp3资源是否播放完成, 若完成自动执行结束动作
 * @return 0:已完成; 1:未完成;
 **/
u8 mp3_res_process(void);

#endif // _MUSIC_RES_H_
