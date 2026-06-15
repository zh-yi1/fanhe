#include "include.h"
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"

#ifndef USER_PT8028_KEY
#define USER_PT8028_KEY                 0
#endif

#if USER_PT8028_KEY

/*
 * PT8028S TCH0~TCH7 -> BCD(D2:D1:D0)，与原理图一致：
 * TCH0 锁键 | TCH1 加热 | TCH2 减号 | TCH3 模式
 * TCH4 确认 | TCH5 开关 | TCH6 加号 | TCH7 预约
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
           edge_up ? "释放" : "按下",
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
    pt8028_gpio_input_init(PT8028_GPIO_OUT_FLAG, PT8028_GPIO_FLAG_PULL);
    pt8028_gpio_input_init(PT8028_GPIO_D0, PT8028_GPIO_BCD_PULL);
    pt8028_gpio_input_init(PT8028_GPIO_D1, PT8028_GPIO_BCD_PULL);
    pt8028_gpio_input_init(PT8028_GPIO_D2, PT8028_GPIO_BCD_PULL);
}

AT(.text.key.init)
void pt8028_port_gpio_dump(void)
{
    u8 d0 = bsp_gpio_get_sta(PT8028_GPIO_D0) ? 1 : 0;
    u8 d1 = bsp_gpio_get_sta(PT8028_GPIO_D1) ? 1 : 0;
    u8 d2 = bsp_gpio_get_sta(PT8028_GPIO_D2) ? 1 : 0;
    u8 flag = bsp_gpio_get_sta(PT8028_GPIO_OUT_FLAG) ? 1 : 0;

    printf("pt8028 idle: FLAG,D2,D1,D0=%d,%d,%d,%d (expect 1,1,1,1) [PE1~4]\n",
           flag, d2, d1, d0);
}

#endif // USER_PT8028_KEY

