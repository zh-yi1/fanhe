#ifndef _PORT_PANEL_LED_H
#define _PORT_PANEL_LED_H

#include "include.h"

/*
 * 面板 LED：按下亮、松开灭；panel_led_scan 在 func_process 主线程调用
 */
#ifndef USER_PANEL_LED
#define USER_PANEL_LED                  0
#endif
#ifndef PANEL_LED_ACTIVE_HIGH
#define PANEL_LED_ACTIVE_HIGH           1
#endif

#define PANEL_LED1_GPIO                 IO_PB4
#define PANEL_LED2_GPIO                 IO_PB5
#define PANEL_LED3_GPIO                 IO_PB6
#define PANEL_LED4_GPIO                 IO_PB7
#define PANEL_LED5_GPIO                 IO_PB8
#define PANEL_LED6_GPIO                 IO_PB9

#define PANEL_LED_NONE                  0xFF

typedef enum {
    PANEL_LED_ID_SWITCH = 1,
    PANEL_LED_ID_OK     = 2,
    PANEL_LED_ID_MODE   = 3,
    PANEL_LED_ID_RES    = 4,
    PANEL_LED_ID_LOCK   = 5,
    PANEL_LED_ID_HEAT   = 6,
    PANEL_LED_ID_CNT    = 6,
} panel_led_id_t;

void panel_led_init(void);
void panel_led_all_off(void);
void panel_led_set(panel_led_id_t id, bool on);
void panel_led_scan(void);
u8 panel_led_get_last_tch(void);

#endif // _PORT_PANEL_LED_H
