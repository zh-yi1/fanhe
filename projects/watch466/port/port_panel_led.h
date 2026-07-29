#ifndef _PORT_PANEL_LED_H
#define _PORT_PANEL_LED_H

#include "include.h"

/*
 * 兼容旧头 — 面板 LED 已迁移到 functions/led/func_led.{h,c}
 * 平台 bsp_key.c 仍 include 此头，此处声明转发到新 API。
 */

void panel_led_init(void);      /* → func_led_init()   */
void panel_led_scan(void);      /* → func_led_scan()   */

#endif /* _PORT_PANEL_LED_H */
