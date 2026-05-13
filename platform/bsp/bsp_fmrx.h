#ifndef __FMRX_H
#define __FMRX_H


enum {
    FMRX_IDLE = 0,
    FMRX_SEEK_START,
    FMRX_SEEKING,
    FMRX_SEEK_PLAY,
    FMRX_SEEK_STOP,
    FMRX_SEEKING_HALF,              //半自动搜台
    FMRX_PLAY,
    FMRX_PAUSE,
};

typedef struct {
    u8  pause       :   1,
        rec_en      :   1,
        phasecom_en :   1;
    u32  play_tick;
    u16  freq;
    u8   ch_cur;
    u8   ch_cnt;

    u8   seek_start;
    u8   sta;
    s8   hstep;
    u8   seek_half_reset;

    u16  fcnt;
    u16  buf[13];
} fmrx_cb_t;

extern fmrx_cb_t fmrx_cb;

void fmrx_tmr1ms_isr(void);
void bsp_fmrx_init(void);
void bsp_fmrx_exit(void);
u8 bsp_fmrx_check_freq(u16 freq);
void bsp_fmrx_set_freq(u16 freq);
void bsp_fmrx_logger_out(void);
bool fmrx_is_playing(void);
u8 fmrx_sysclk_config(void);

#if FMRX_OPTIMIZE_TRY   //FM 收台效果尝试优化,可以修改CLK控制等,需要实际样机去测试效果
void fmrx_optimize_try_set(void);
void fmrx_optimize_try_recover(void);
#endif

#if FMRX_TEST_CHANNEL   //FM 固定某些电台测试,可用于对比其它样机,定位到特定的一些台对比声音清晰度
u16 fmrx_test_first_channel_get(void);
void fmrx_test_channel_switch(u8 dir);
#endif
void func_fmrx(void);
#endif // __FMRX_H
