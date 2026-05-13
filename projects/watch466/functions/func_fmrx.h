#ifndef _FUNC_FMRX_H
#define _FUNC_FMRX_H

u16 get_ch_freq(u8 ch);
void fmrx_half_seek_start(u8 dir);
void fmrx_switch_channel(u8 dir);
void fmrx_switch_freq(u8 dir);
void func_fmrx_message(u16 msg);
void func_fmrx_mp3_res_play(u32 addr, u32 len);
void func_fmrx_pause_play(void);
void func_fmrx_enter(void);
void func_fmrx_bypass(void);
void func_fmrx_restore(void);

#endif // _FUNC_FMRX_H
