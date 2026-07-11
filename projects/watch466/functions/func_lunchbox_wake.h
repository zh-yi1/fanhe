/**
 * @file    func_lunchbox_wake.h
 * @brief   饭盒亮屏唤醒 — 恢复屏供电+背光
 *
 * 亮屏顺序（参考 func_lunchbox_lcd.c）：
 *   开 VDDLCD(lcd_pg_on) → 恢复 PWM + 开背光(led_pg_on)
 *
 * 与 gui_wakeup() 的区别：
 *   - 不重初始化 GPU/TFT/CTP（因为熄屏时未退出）
 *   - 仅恢复供电和背光，速度快、无闪烁
 *
 * 配套文件：
 *   func_lunchbox_off.h — 熄屏函数 lunchbox_display_off()
 *   func.c ~L856           — elunchbox_screen_wake 调用本函数
 */
#ifndef __FUNC_LUNCHBOX_WAKE_H
#define __FUNC_LUNCHBOX_WAKE_H

#include "include.h"

/**
 * @brief 开屏供电 + 恢复背光（唤醒显示）
 *
 * 亮屏顺序：开 VDDLCD → 恢复 PWM + 开背光
 * 注意：VDDLCD 必须先于背光打开，否则花屏
 */
void lunchbox_display_on(void);

#endif // __FUNC_LUNCHBOX_WAKE_H
