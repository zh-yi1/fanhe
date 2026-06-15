#include "include.h"
#include "port_panel_led.h"

#ifndef USER_PANEL_LED
#define USER_PANEL_LED                  0
#endif

#if USER_PANEL_LED

#include "bsp_pt8028_key.h"

/*
 * 原理图 PT8028 TCH -> LED，按下亮、松开灭
 */
AT(.com_rodata.port.panel_led)
static const u8 tbl_tch_to_led[8] = {
    PANEL_LED_ID_LOCK,      /* TCH0 -> LED5 */
    PANEL_LED_ID_HEAT,      /* TCH1 -> LED6 */
    PANEL_LED_NONE,         /* TCH2 减号 */
    PANEL_LED_ID_MODE,      /* TCH3 -> LED3 */
    PANEL_LED_ID_OK,        /* TCH4 -> LED2 */
    PANEL_LED_ID_SWITCH,    /* TCH5 -> LED1 */
    PANEL_LED_NONE,         /* TCH6 加号 */
    PANEL_LED_ID_RES,       /* TCH7 -> LED4 */
};

AT(.com_rodata.port.panel_led)
static const u8 tbl_panel_led_gpio[PANEL_LED_ID_CNT] = {
    PANEL_LED1_GPIO,
    PANEL_LED2_GPIO,
    PANEL_LED3_GPIO,
    PANEL_LED4_GPIO,
    PANEL_LED5_GPIO,
    PANEL_LED6_GPIO,
};

static u8 panel_led_last_tch AT(.buf.panel_led);

AT(.com_text.port.panel_led)
static void panel_led_gpio_set(u8 gpio, bool on)
{
    bool level;

    if (gpio == IO_NONE) {
        return;
    }
#if PANEL_LED_ACTIVE_HIGH
    level = on;
#else
    level = !on;
#endif
    port_gpio_set_out(gpio, level);
}

AT(.text.key.init)
void panel_led_init(void)
{
    u8 i;

    panel_led_last_tch = PT8028_KEY_NONE;
    for (i = 0; i < PANEL_LED_ID_CNT; i++) {
        panel_led_gpio_set(tbl_panel_led_gpio[i], false);
    }
}

AT(.com_text.port.panel_led)
void panel_led_all_off(void)
{
    u8 i;

    for (i = 0; i < PANEL_LED_ID_CNT; i++) {
        panel_led_gpio_set(tbl_panel_led_gpio[i], false);
    }
}

AT(.com_text.port.panel_led)
void panel_led_set(panel_led_id_t id, bool on)
{
    if (id < PANEL_LED_ID_SWITCH || id > PANEL_LED_ID_HEAT) {
        return;
    }
    panel_led_gpio_set(tbl_panel_led_gpio[id - 1], on);
}

AT(.com_text.port.panel_led)
static void panel_led_show_tch(u8 tch)
{
    u8 led_id;

    panel_led_all_off();
    if (tch > PT8028_KEY_TCH7) {
        return;
    }
    led_id = tbl_tch_to_led[tch];
    if (led_id != PANEL_LED_NONE) {
        panel_led_set((panel_led_id_t)led_id, true);
    }
}

AT(.com_text.port.panel_led)
void panel_led_scan(void)
{
    u8 tch = pt8028_get_led_tch();

    if (tch == panel_led_last_tch) {
        return;
    }
    panel_led_last_tch = tch;
    if (tch <= PT8028_KEY_TCH7) {
        panel_led_show_tch(tch);
    } else {
        panel_led_all_off();
    }
}

#endif // USER_PANEL_LED
