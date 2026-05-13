#ifndef _LIB_ET_UI_IO_H__
#define _LIB_ET_UI_IO_H__

#if (ASR_SELECT == ASR_YJ)

typedef struct __ET_UI_IO_T {
    volatile u8 sound_en;
    #if ET_USER_UART0_EN
    volatile u8 need_uart_send;
    #endif // ET_USER_UART0_EN
#if ET_A_WAKE_WORD_EN
    volatile u8 et_wake_kws_en;
    volatile u8 et_wake_kws_t_cnt;
    volatile u8 et_wake_kws_busy;
#endif // ET_A_WAKE_WORD_EN
}ET_UI_IO_T;

extern 	ET_UI_IO_T  t_pwn;

#endif //ASR_SELECT

#endif
