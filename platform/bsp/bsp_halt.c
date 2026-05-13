#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
AT(.com_rodata.halt)
const char str_halt[] = "<<< Halt >>>: %X\n";
AT(.com_rodata.halt)
const char str_res_halt[] = "ERROR ======> resource halt:%X\n";
#else
#define TRACE(...)
#endif

//资源错误代码
AT(.com_rodata.halt)
const u32 res_halt_no_res[19] = {
    0xC003, 0xC241, 0xC242, 0xC261, 0xC262, 0xC281, 0xC282, 0xC401,
    0xC402, 0xC471, 0xC472, 0xC481, 0xC482, 0xC491, 0xC492, 0xC4A1,
    0xC4A2, 0x5301, 0x5302
};

//蓝屏异常处理跳转,默认跳转到idle界面
AT(.com_text.halt)
bool halt_do(u32 halt_no)
{
//    for(u8 i=0;i<sizeof(res_halt_no_res)/sizeof(res_halt_no_res[0]);i++) {
//        if (halt_no == res_halt_no_res[i]) {
//            sys_cb.flag_halt = true;
//            printf(str_res_halt, halt_no);
//            msg_enqueue(EVT_HALT);
//            return true;
//        }
//    }
    printf(str_res_halt, halt_no);
    return false;
}

AT(.com_text.halt)
void halt(u32 halt_no)
{
    if (sys_cb.flag_halt) {
        return;
    }

    if(halt_no == 0x8101){
        return;
    }

    if (sys_cb.gui_sleep_sta) {
        gui_wakeup();
        LCD_BL_EN();
    }

    PICCON = 0;
    halt_err_set(halt_no);
    gui_halt(halt_no);
    if (!halt_do(halt_no)) {
        int reset_cnt = 15000000;
        TRACE(str_halt, halt_no);
        while (1) {
            WDT_CLR();
            if (reset_cnt-- <= 0) {
                WDT_RST_DELAY();
            }
        }
    }
}
