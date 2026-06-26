#include "include.h"
#include "port_panel_led.h"

#ifndef USER_PANEL_LED
#define USER_PANEL_LED                  0
#endif

#if USER_PANEL_LED

#include "bsp_pt8028_key.h"

/*
 * 原理图 LED 与按键（510Ω 到 GND，GPIO 高电平点亮）：
 *   LED1(PB0) 开关 TCH5 | LED2(PB1) 确认 TCH4 | LED3(PB2) 模式 TCH3
 *   LED4(PB5) 预约 TCH7 | LED5(PB6) 锁键 TCH0 | LED6(PB7) 加热 TCH1
 *   TCH2 减号 / TCH6 加号 无 LED
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
static bool panel_led_lock_latched AT(.buf.panel_led);
static bool panel_led_res_latched AT(.buf.panel_led);
static bool panel_led_heat_latched AT(.buf.panel_led);

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
void panel_led_set_lock_latched(bool on)
{
    panel_led_lock_latched = on;
}

AT(.com_text.port.panel_led)
void panel_led_set_res_latched(bool on)
{
    panel_led_res_latched = on;
}

void panel_led_set_heat_latched(bool on)
{
    panel_led_heat_latched = on;
}

AT(.com_text.port.panel_led)
void panel_led_scan(void)
{
    u8 tch;
    u8 led_id;

    panel_led_all_off();

    if (panel_led_lock_latched) {
        panel_led_set(PANEL_LED_ID_LOCK, true);
    }
    if (panel_led_res_latched) {
        panel_led_set(PANEL_LED_ID_RES, true);
    }
    if (panel_led_heat_latched) {
        panel_led_set(PANEL_LED_ID_HEAT, true);
    }

    tch = pt8028_get_led_tch();
    if (tch <= PT8028_KEY_TCH7) {
        led_id = tbl_tch_to_led[tch];
        if (led_id != PANEL_LED_NONE) {
            panel_led_set((panel_led_id_t)led_id, true);
        }
    }
    panel_led_last_tch = tch;
}

AT(.com_text.port.panel_led)
u8 panel_led_get_last_tch(void)
{
    return panel_led_last_tch;
}

#endif // USER_PANEL_LED
