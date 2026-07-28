#ifndef _FUNC_LED_H
#define _FUNC_LED_H

#include "include.h"

/*
 * 面板 LED 模块 — 基于 IMB-076_LCD_V1.0 原理图
 *
 * 硬件连接：
 *   MCU AB5790T PB4~PB9 → 面板 LED1~LED6 (白色 0603, 510R 限流)
 *   PT8028S TCH0~TCH7 触摸 → MCU PE1~PE4 (BCD + OUT_FLAG)
 *
 * 行为：按下触摸键 → 对应 LED 亮；松开 → 全灭（单灯模式，同时只亮一个）
 *
 * 调用：func_led_scan() 放在 func_process() 主循环，每帧调用一次
 */

#ifndef USER_PANEL_LED
#define USER_PANEL_LED                  0
#endif
#ifndef PANEL_LED_ACTIVE_HIGH
#define PANEL_LED_ACTIVE_HIGH           1
#endif

/* ---- GPIO 引脚 (AB5790T PB4~PB9) ---- */
#define FUNC_LED1_GPIO                  IO_PB4
#define FUNC_LED2_GPIO                  IO_PB5
#define FUNC_LED3_GPIO                  IO_PB6
#define FUNC_LED4_GPIO                  IO_PB7
#define FUNC_LED5_GPIO                  IO_PB8
#define FUNC_LED6_GPIO                  IO_PB9

#define FUNC_LED_NONE                   0xFF

/*
 * LED ID 枚举 — 与原理图丝印对应：
 *   LED1(开关) LED2(确认) LED3(模式) LED4(预约) LED5(童锁) LED6(加热)
 */
typedef enum {
    FUNC_LED_ID_SWITCH = 1,     /* LED1 PB4 — 开关键背光 */
    FUNC_LED_ID_OK     = 2,     /* LED2 PB5 — 确认键背光 */
    FUNC_LED_ID_MODE   = 3,     /* LED3 PB6 — 模式键背光 */
    FUNC_LED_ID_RES    = 4,     /* LED4 PB7 — 预约键背光 */
    FUNC_LED_ID_LOCK   = 5,     /* LED5 PB8 — 童锁键背光 */
    FUNC_LED_ID_HEAT   = 6,     /* LED6 PB9 — 加热键背光 */
    FUNC_LED_ID_CNT    = 6,
} func_led_id_t;

void func_led_init(void);
void func_led_all_off(void);
void func_led_set(func_led_id_t id, bool on);
void func_led_scan(void);
u8   func_led_get_last_tch(void);

#endif /* _FUNC_LED_H */
