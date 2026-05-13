#include "include.h"

#if USER_MULTI_PRESS_EN
///key_return(短按抬键)是否支持按键多击检测
AT(.com_text.bsp.key)
bool key_multi_press_check_cb(u16 key_return)
{
    if (key_return == KU_BACK) {    //仅PWRKEY的BACK按键需要双击功能
        return true;
    }
    return false;
}
#endif

#if USER_ADKEY
AT(.com_rodata.adkey.table)
const adkey_tbl_t adkey_table[] = {
    {0x0A, KEY_BACK},               //BACK          0R
    {0x20, KEY_LEFT},               //LEFT          1K
    {0x35, KEY_RIGHT},              //RIGHT         2K
    {0xFF, NO_KEY},                 //              END
};
#endif

#if USER_PWRKEY
///最多支持5个按键。数组元数总数请保持不变。不需要的按键改为NO_KEY
AT(.com_rodata.pwrkey.table)
const adkey_tbl_t pwrkey_table[6] = {
    {0x08, KEY_LEFT},               //LEFT          0R
    {0x3A, KEY_RIGHT},              //RIGHT         12K
    {0x7B, KEY_BACK},               //BACK          47K
    {0xFF, NO_KEY},
};
#endif // USER_PWRKEY

#if USER_IOKEY
AT(.text.key.init)
void io_key_init(void)
{
    GPIOFDE  |= BIT(1) | BIT(2);
    GPIOFDIR |= BIT(1) | BIT(2);
    GPIOFPU  |= BIT(1) | BIT(2);
}

AT(.com_text.port.key)
u8 get_iokey(void)
{
    u8 key_val = NO_KEY;

    if (!(GPIOF & BIT(1))) {
        key_val = KEY_BACK;                 //KEY0
    } else if (!(GPIOF & BIT(2))) {
        key_val = KEY_LEFT;                 //KEY1
    } else {
        GPIOFDIR &= ~BIT(1);
        GPIOFCLR = BIT(1);
        delay_us(6);
        if (!(GPIOF & BIT(2))) {
            key_val = KEY_RIGHT;            //两IO的中间键KEY2
        }
        GPIOFDIR |= BIT(1);
    }
    return key_val;
}
#endif // USER_IOKEY

