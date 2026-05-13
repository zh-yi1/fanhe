#ifndef _BSP_GIF_H
#define _BSP_GIF_H


void bsp_gif_play(u8 *buf_out, u8 *buf_in, u32 buf_len);
bool bsp_gif_play_process(u8 *buf_out, u8 *buf_in, u32 buf_len);
void bsp_gif_stop(void);



#endif // _BSP_GIF_H
