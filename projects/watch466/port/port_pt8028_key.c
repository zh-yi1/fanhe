#include "include.h"

#include "bsp_pt8028_key.h"

#include "port_pt8028_key.h"



#ifndef USER_PT8028_KEY

#define USER_PT8028_KEY                 0

#endif



#if USER_PT8028_KEY



#define PT8028_PE_GPIO_MASK             (BIT(1) | BIT(2) | BIT(3) | BIT(4))



/*

 * PT8028S TCH0~TCH7 -> BCD(D2:D1:D0)，与表2一致：

 * TCH0 锁键(000) | TCH1 加热(001) | TCH2 减号(010) | TCH3 模式(011)

 * TCH4 确认(100) | TCH5 开关(101) | TCH6 加号(110) | TCH7 预约(111, 仅 OUT_FLAG=0)

 * 上电/空闲：OUT_FLAG=1 且 D=111，不是按键

 */

AT(.com_rodata.port.pt8028)

const u8 tbl_pt8028_bcd_to_key[8] = {

    KEY_LEFT,       /* TCH0 BCD0 锁键   */

    KEY_PREV,       /* TCH1 BCD1 加热   */

    KEY_VOL_DOWN,   /* TCH2 BCD2 减号   */

    KEY_MODE,       /* TCH3 BCD3 模式   */

    KEY_BACK,       /* TCH4 BCD4 确认   */

    KEY_RIGHT,      /* TCH5 BCD5 开关   */

    KEY_VOL_UP,     /* TCH6 BCD6 加号   */

    KEY_NEXT,       /* TCH7 BCD7 预约   */

};

/* TCH -> lunchbox_key_notify(key_val) */
const u8 tbl_pt8028_tch_to_lunchbox_key[8] = {
    6,  /* TCH0 锁键   */
    5,  /* TCH1 加热   */
    4,  /* TCH2 减号   */
    3,  /* TCH3 模式   */
    2,  /* TCH4 确认   */
    1,  /* TCH5 开关   */
    7,  /* TCH6 加号   */
    8,  /* TCH7 预约   */
};

u8 pt8028_tch_to_lunchbox_key(u8 tch)
{
    if (tch > PT8028_KEY_TCH7) {
        return 0;
    }
    return tbl_pt8028_tch_to_lunchbox_key[tch];
}



/*

 * 表2 严格映射：BCD(D2:D1:D0) 即 TCH，不做 remap/长按短按。

 */

AT(.com_text.port.pt8028)

u8 pt8028_elunchbox_bcd_remap(u8 bcd, u8 out_flag)

{

    (void)out_flag;

    return bcd;

}



AT(.com_rodata.port.pt8028)

static const char * const tbl_pt8028_key_name[8] = {

    "锁键", "加热", "减号", "模式", "确认", "开关", "加号", "预约",

};



AT(.text.port.pt8028)

void pt8028_key_edge_print(u8 edge_up, u8 bcd, u8 out_flag)

{

#if PT8028_KEY_DEBUG

    u8 d0 = bcd & 1;

    u8 d1 = (bcd >> 1) & 1;

    u8 d2 = (bcd >> 2) & 1;



    printf("PT8028 %s: FLAG,D2,D1,D0=%d,%d,%d,%d TCH%d BCD=%d\n",
           edge_up ? "上升沿" : "下降沿",
           out_flag, d2, d1, d0, bcd, bcd);

#else

    (void)edge_up;

    (void)bcd;

    (void)out_flag;

#endif

}



AT(.text.port.pt8028)

void pt8028_key_name_print(u8 tch, u8 key, u16 ku, u8 bcd, u8 out_flag)

{

#if PT8028_KEY_DEBUG

    const char *name = (tch <= PT8028_KEY_TCH7) ? tbl_pt8028_key_name[tch] : "?";



    printf("PT8028 [%s] TCH%d BCD=%d OUT_FLAG=%d key=0x%02X msg=0x%04X\n",

           name, tch, bcd, out_flag, key, ku);

#else

    (void)tch;

    (void)key;

    (void)ku;

    (void)bcd;

    (void)out_flag;

#endif

}



AT(.text.port.pt8028)

void pt8028_key_reject_print(u8 hold_bcd, u8 latch_bcd, u8 hist_bcd, u8 out_flag)

{

#if PT8028_KEY_DEBUG

    printf("PT8028 无效键: Hold=%d Latch=%d Hist=%d OUT_FLAG=%d\n",

           hold_bcd, latch_bcd, hist_bcd, out_flag);

#endif

}



/*

 * 寄存器级 reclaim：解除 SD1 G3 / 模拟 / 输出占用，强制 PE1~4 为数字输入。

 * PE2 勿作摄像头 LDO 输出，否则 D0 永远读 1。

 */

AT(.com_text.port.pt8028)

void pt8028_pe_reclaim_fast(void)

{

#if ELUNCHBOX_PANEL_EN

    u32 m = PT8028_PE_GPIO_MASK;



    FUNCMCON3 = (FUNCMCON3 & ~0xF) | SD1MAP_NONE;

    GPIOEFEN &= ~m;

    GPIOEDE  |= m;

    GPIOEDIR |= m;

    GPIOEPU     &= ~m;

    GPIOEPD     &= ~m;

    GPIOEPU200K &= ~m;

    GPIOEPD200K &= ~m;

    GPIOEPU300  &= ~m;

    GPIOEPD300  &= ~m;

#if PT8028_GPIO_FLAG_PULL == GPIOxPU300

    GPIOEPU300 |= BIT(1);

#elif PT8028_GPIO_FLAG_PULL == GPIOxPU200K

    GPIOEPU200K |= BIT(1);

#elif PT8028_GPIO_FLAG_PULL == GPIOxPU

    GPIOEPU |= BIT(1);

#endif

#if PT8028_GPIO_BCD_PULL == GPIOxPU300

    GPIOEPU300 |= (BIT(2) | BIT(3) | BIT(4));

#elif PT8028_GPIO_BCD_PULL == GPIOxPU200K

    GPIOEPU200K |= (BIT(2) | BIT(3) | BIT(4));

#elif PT8028_GPIO_BCD_PULL == GPIOxPU

    GPIOEPU |= (BIT(2) | BIT(3) | BIT(4));

#endif

#endif

}



AT(.com_text.port.pt8028)

bool pt8028_pe_gpio_ok(void)

{

#if ELUNCHBOX_PANEL_EN

    u32 m = PT8028_PE_GPIO_MASK;



    if ((GPIOEDE & m) != m) {

        return false;

    }

    if ((GPIOEFEN & m) != 0) {

        return false;

    }

    if ((GPIOEDIR & m) != m) {

        return false;

    }

    if ((FUNCMCON3 & 0xF) != SD1MAP_NONE) {

        return false;

    }

#endif

    return true;

}



/*

 * PE1~4 数字 GPIO 输入

 */

AT(.text.key.init)

static void pt8028_gpio_input_init(u8 io, u8 pull)

{

    if ((io == IO_NONE) || (io > IO_MAX_NUM)) {

        return;

    }

    bsp_gpio_de_en(io);

    port_gpio_set_in(io, pull);

}



AT(.text.key.init)

void pt8028_port_gpio_init(void)

{
#if 1
    pt8028_gpio_invalidate();

#if ELUNCHBOX_PANEL_EN

    FUNCMCON0 = (FUNCMCON0 & ~0xF) | SD0MAP_NONE;

    FUNCMCON3 = (FUNCMCON3 & ~0xF) | SD1MAP_NONE;

#endif

    pt8028_gpio_input_init(PT8028_GPIO_OUT_FLAG, PT8028_GPIO_FLAG_PULL);

    pt8028_gpio_input_init(PT8028_GPIO_D0, PT8028_GPIO_BCD_PULL);

    pt8028_gpio_input_init(PT8028_GPIO_D1, PT8028_GPIO_BCD_PULL);

    pt8028_gpio_input_init(PT8028_GPIO_D2, PT8028_GPIO_BCD_PULL);

    pt8028_pe_reclaim_fast();

    pt8028_gpio_mark_configured();
#else
    bsp_gpio_de_en(PT8028_GPIO_OUT_FLAG);
    bsp_gpio_de_en(PT8028_GPIO_D0);
    bsp_gpio_de_en(PT8028_GPIO_D1);
    bsp_gpio_de_en(PT8028_GPIO_D2);
    bsp_gpio_pu_en(PT8028_GPIO_OUT_FLAG, GPIOxPU200K);
    bsp_gpio_pu_en(PT8028_GPIO_D0, GPIOxPU200K);
    bsp_gpio_pu_en(PT8028_GPIO_D1, GPIOxPU200K);
    bsp_gpio_pu_en(PT8028_GPIO_D2, GPIOxPU200K);
#endif
}



AT(.text.key.init)

void pt8028_port_gpio_dump(void)

{

    u32 pe;



    pt8028_pe_reclaim_fast();

    pe = GPIOE;

    printf("pt8028 idle: FLAG,D2,D1,D0=%d,%d,%d,%d PE=0x%02x EDE=0x%02x DIR=0x%02x FC3=0x%x\n",

           (int)((pe >> 1) & 1), (int)((pe >> 4) & 1), (int)((pe >> 3) & 1),

           (int)((pe >> 2) & 1), (u8)(pe & 0x1f), (u8)(GPIOEDE & 0x1f),

           (u8)(GPIOEDIR & 0x1f), (u8)(FUNCMCON3 & 0x0f));

}



#endif // USER_PT8028_KEY


