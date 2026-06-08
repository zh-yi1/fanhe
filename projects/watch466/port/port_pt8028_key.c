#include "include.h"
#include "port_pt8028_key.h"

#ifndef USER_PT8028_KEY
#define USER_PT8028_KEY                 0
#endif

#if USER_PT8028_KEY

AT(.com_rodata.port.pt8028)
const u8 tbl_pt8028_bcd_to_key[8] = {
    KEY_VOL_UP,     /* TCH0 加号   BCD 0 */
    KEY_NEXT,       /* TCH1 预约   BCD 1 */
    KEY_VOL_DOWN,   /* TCH2 减号   BCD 2 */
    KEY_MODE,       /* TCH3 模式   BCD 3 */
    KEY_BACK,       /* TCH4 确认   BCD 4 */
    KEY_RIGHT,      /* TCH5 开关   BCD 5 */
    KEY_LEFT,       /* TCH6 锁键   BCD 6 */
    KEY_PREV,       /* TCH7 加热   BCD 7 */
};

AT(.text.key.init)
void pt8028_port_gpio_init(void)
{
    port_gpio_set_in(PT8028_GPIO_D0, GPIOxPU);
    port_gpio_set_in(PT8028_GPIO_D1, GPIOxPU);
    port_gpio_set_in(PT8028_GPIO_D2, GPIOxPU);
    port_gpio_set_in(PT8028_GPIO_OUT_FLAG, GPIOxPU);
}

#endif // USER_PT8028_KEY
