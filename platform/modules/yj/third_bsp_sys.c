#include "include.h"
#include "third_bsp_sys.h"
#include "third_kws_event.h"

#if (ASR_SELECT == ASR_YJ)

#if THIRD_UI_LIGHT_EN
#include <lib_et_ui_io.h>
#include "third_func.h"

void third_light_init()
{
    memset(&t_pwn,0,sizeof(ET_UI_IO_T));
}


void third_after_bsp_sys_init()
{

#if ET_USER_UART0_EN
//    udet_init(115200);
    udet_init(9600);
	register_uart0_isr_init(uart0_isr);
	//my_printf_init(et_bsp_empty_putchar_byte);//�����д�ӡprintfӳ�䵽�պ���
#endif

    //kaiji
    #if ET_USER_UART0_EN
    t_pwn.need_uart_send = 1;
    #endif // ET_USER_UART0_EN
    t_pwn.sound_en = 1;

//    t_pwn.time_set_cnt = TIME_30_M_2_S;
    #if 0//ET_A_WAKE_WORD_EN
    t_pwn.et_wake_kws_t_cnt = ET_WAKE_TIME_DEFAULT;
    t_pwn.et_wake_kws_en = 1;
    #if ET_VAD_WAKE_WORD_HOLD_EN && ET_ASR_VAD_EN
    extern void set_et_vad_need_hold(u8 flag);
    set_et_vad_need_hold(1);
    #endif // ET_VAD_WAKE_WORD_HOLD_EN
    #endif // ET_A_WAKE_WORD_EN

}

void third_light_exit()
{

}

#endif //THIRD_UI_LIGHT_EN

#endif //ASR_SELECT