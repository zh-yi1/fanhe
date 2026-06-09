#include "include.h"
#include "bsp_pt8028_key.h"

#ifndef USER_PT8028_KEY
#define USER_PT8028_KEY                 0
#endif
#ifndef PT8028_KEY_DEBUG
#define PT8028_KEY_DEBUG                0
#endif

#if USER_PT8028_KEY
#include "port_pt8028_key.h"

#if PT8028_KEY_DEBUG
AT(.com_rodata.bsp.pt8028)
const char pt8028_dbg_fmt[] = "pt8028 flag=%d bcd=%d key=%02x\n";
#endif

AT(.com_text.bsp.pt8028)
static u8 pt8028_read_bcd(void)
{
    u8 d0 = bsp_gpio_get_sta(PT8028_GPIO_D0) ? 1 : 0;
    u8 d1 = bsp_gpio_get_sta(PT8028_GPIO_D1) ? 1 : 0;
    u8 d2 = bsp_gpio_get_sta(PT8028_GPIO_D2) ? 1 : 0;

    return (d2 << 2) | (d1 << 1) | d0;
}

AT(.com_text.bsp.pt8028)
static pt8028_key_id_t pt8028_get_tch_id(void)
{
    u8 flag = bsp_gpio_get_sta(PT8028_GPIO_OUT_FLAG) ? 1 : 0;

    if (flag) {
        return PT8028_KEY_NONE;
    }
    return (pt8028_key_id_t)pt8028_read_bcd();
}

AT(.text.key.init)
void pt8028_key_init(void)
{
    pt8028_port_gpio_init();
}

AT(.com_text.bsp.pt8028)
u8 get_pt8028_key(void)
{
    pt8028_key_id_t tch = pt8028_get_tch_id();
    u8 key_val = NO_KEY;

    if (tch <= PT8028_KEY_TCH7) {
        key_val = tbl_pt8028_bcd_to_key[tch];
    }

#if PT8028_KEY_DEBUG
    {
        static u8 last_dbg_key = NO_KEY;
        u8 flag = bsp_gpio_get_sta(PT8028_GPIO_OUT_FLAG) ? 1 : 0;
        u8 bcd = pt8028_read_bcd();

        if (key_val != last_dbg_key) {
            last_dbg_key = key_val;
            printf(pt8028_dbg_fmt, flag, bcd, key_val);
        }
    }
#endif

    return key_val;
}

#endif // USER_PT8028_KEY
