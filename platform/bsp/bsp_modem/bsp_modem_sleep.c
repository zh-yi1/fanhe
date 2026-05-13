#include "include.h"

#if MODEM_CAT1_EN

static u8 sleep_state = 0;
static u32 sleep_time_bak = -1;
#define SENDER_STA_INIT()                                                      \
    do {                                                                       \
        GPIOFFEN &= ~(BIT(1));                                                 \
        GPIOFDE |= (BIT(1));                                                   \
        GPIOFDIR |= (BIT(1));                                                  \
        GPIOFFEN &= ~(BIT(2));                                                 \
        GPIOFDE |= (BIT(2));                                                   \
        GPIOFDIR &= ~(BIT(2));                                                 \
        GPIOFDRV |= BIT(2);                                                    \
    } while (0)

AT(.com_text.modem.sleep)
void bsp_modem_sta_rise_isr(void)
{
    modem_send_sta_rise_msg();
}

AT(.com_text.modem.sleep)
void bsp_modem_sta_fall_isr(void)
{
    modem_send_sta_fall_msg();
}

void bsp_modem_sta_init(void)
{
    SENDER_STA_INIT();

    extab_user_isr_set(IO_PF1, RISE_EDGE, IOUD_SEL_PD, bsp_modem_sta_rise_isr);
    extab_user_isr_mode_set(IO_PF1, MODE_BOTH_AWAKEN_SLEEP_PWK);
}

void bsp_modem_sleep_enter(void)
{
    if (bsp_modem_get_init_flag()) {
        // close uart
        modem_huart_sleep();
        sleep_state = 1;
    }
}

void bsp_modem_sleep_exit(void)
{
    if (sleep_state) {
        sleep_state = 0;
        modem_huart_wakeup();
        reset_sleep_delay();
        // bsp_modem_sta_init();
    }
}

void modem_pre_send_cmd(void)
{
    SENDER_STA_UP();
    delay_ms(6);
}

void modem_aft_send_cmd(void)
{
    SENDER_STA_DOWN();
}

void modem_sta_rise_isr(void)
{
    // open uart
    bsp_modem_sleep_exit();

    extab_user_isr_clr(IO_PF1);
    extab_user_isr_set(IO_PF1, FALL_EDGE, IOUD_SEL_PU, bsp_modem_sta_fall_isr);
    extab_user_isr_mode_set(IO_PF1, MODE_BOTH_AWAKEN_SLEEP_PWK);

    // keep working
    sleep_time_bak = sys_cb.sleep_time;
    sys_cb.sleep_time = -1;
    printf("%s\n", __func__);
}

void modem_sta_fall_isr(void)
{
    sys_cb.sleep_time = sleep_time_bak;
    reset_sleep_delay_all();

    extab_user_isr_clr(IO_PF1);
    extab_user_isr_set(IO_PF1, RISE_EDGE, IOUD_SEL_PD, bsp_modem_sta_rise_isr);
    extab_user_isr_mode_set(IO_PF1, MODE_BOTH_AWAKEN_SLEEP_PWK);
    printf("%s\n", __func__);
}

u32 modem_sta_get_value(void)
{
    return (GPIOF & BIT(1));
}

#else
void modem_pre_send_cmd(void) {}
void bsp_modem_sleep_enter(void) {}
void modem_recv_sleep_pkg(void) {}
#endif
