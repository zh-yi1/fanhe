#include "include.h"

#if LOUDSPEAKER_MUTE_EN
static gpio_t mute_gpio;
void amp_abd_pluse_ctrl(gpio_t *p);

void loudspeaker_mute_init(void)
{
    u8 io_num = LOUDSPEAKER_MUTE_PORT;
    gpio_t *p = &mute_gpio;
    bsp_gpio_cfg_init(p, io_num);
    if (p->sfr == NULL) {
        return;
    }
    p->sfr[GPIOxDE] |= BIT(p->num);
    p->sfr[GPIOxDIR] &= ~BIT(p->num);
#if LOUDSPEAKER_HIGH_MUTE
    p->sfr[GPIOxSET] = BIT(p->num);
#else
    p->sfr[GPIOxCLR] = BIT(p->num);
#endif
}

AT(.com_text.mute)
void loudspeaker_mute(void)
{
    gpio_t *p = &mute_gpio;
    if (p->sfr == NULL) {
        return;
    }
#if LOUDSPEAKER_HIGH_MUTE
    p->sfr[GPIOxSET] = BIT(p->num);
#else
    p->sfr[GPIOxCLR] = BIT(p->num);
#endif

#if AMP_CTRL_AB_D_TYPE
    amp_abd_pluse_ctrl();
#endif
}

AT(.com_text.mute)
void loudspeaker_unmute(void)
{
    gpio_t *p = &mute_gpio;
    if (p->sfr == NULL) {
        return;
    }
#if LOUDSPEAKER_HIGH_MUTE
    p->sfr[GPIOxCLR] = BIT(p->num);
#else
    p->sfr[GPIOxSET] = BIT(p->num);
#endif
}

AT(.text.mute)
void loudspeaker_disable(void)
{
    gpio_t *p = &mute_gpio;
    if (p->sfr == NULL) {
        return;
    }
    p->sfr[GPIOxDIR] |= BIT(p->num);
}
#endif // LOUDSPEAKER_MUTE_EN

#if AMP_CTRL_AB_D_EN

static gpio_t amp_gpio;
static u8 amp_conf_type = 0;

#if AMP_CTRL_AB_D_TYPE
AT(.com_text.mute)
void amp_abd_pluse_ctrl(gpio_t *p)
{
    if (!xcfg_cb.high_mute) {
        delay_5ms(4);           //先拉高20ms
        if (amp_conf_type) {    //AB类脉冲控制
            for (u32 i = 0; i < 4; i++) {
                p->sfr[GPIOxCLR] = BIT(p->num);
                delay_us(75);
                p->sfr[GPIOxSET] = BIT(p->num);
                delay_us(75);
            }
        } else {                //暂时只支持D类防破音关闭
            for (u32 i = 0; i < 3; i++) {
                p->sfr[GPIOxCLR] = BIT(p->num);
                delay_us(75);
                p->sfr[GPIOxSET] = BIT(p->num);
                delay_us(75);
            }
        }
    }
}
#endif

AT(.text.amplifier)
void amp_sel_cfg_d(void)
{
    amp_conf_type = 0;
    gpio_t *p = &amp_gpio;
    if (p->sfr == NULL) {
        return;
    }
    p->sfr[GPIOxDE] |= BIT(p->num);
    p->sfr[GPIOxDIR] &= ~BIT(p->num);
    p->sfr[GPIOxSET] = BIT(p->num);
}

AT(.text.amplifier)
void amp_sel_cfg_init(u8 io_num)
{
    gpio_t *p = &amp_gpio;
    bsp_gpio_cfg_init(p, io_num);
    amp_sel_cfg_d();
}

AT(.text.amplifier)
void amp_sel_cfg_ab(void)
{
    amp_conf_type = 1;
    gpio_t *p = &amp_gpio;
    if (p->sfr == NULL) {
        return;
    }
    p->sfr[GPIOxDE] |= BIT(p->num);
    p->sfr[GPIOxDIR] &= ~BIT(p->num);
    p->sfr[GPIOxCLR] = BIT(p->num);
}

AT(.text.amplifier)
void amp_sel_cfg_dis(void)
{
    gpio_t *p = &amp_gpio;
    if (p->sfr == NULL) {
        return;
    }
    p->sfr[GPIOxDE] |= BIT(p->num);
    p->sfr[GPIOxDIR] |= BIT(p->num);
}
#endif // AMP_CTRL_AB_D_EN
