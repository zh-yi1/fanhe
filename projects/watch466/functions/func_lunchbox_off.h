/**
 * @file    func_lunchbox_off.h
 * @brief   饭盒熄屏控制 — 关背光+屏供电，不进入深度休眠
 *
 * 与手动关机 (elunchbox_pwr_manual_shutdown) 的区别：
 *   - 手动关机：关背光 + 关屏供电 + 断 BLE + 关外设时钟 + CPU 深度休眠
 *   - 本模块：  关背光 + 关屏供电，CPU/BLE/UART/按键扫描全部保持运行
 *
 * 关/开屏顺序（参考 func_lunchbox_lcd.c）：
 *   熄屏：PWM 清零 → 关背光(led_pg_off) → 关 VDDLCD(lcd_pg_off)
 *   亮屏：开 VDDLCD(lcd_pg_on) → 恢复 PWM + 开背光(led_pg_on)
 */
#ifndef __FUNC_LUNCHBOX_OFF_H
#define __FUNC_LUNCHBOX_OFF_H

#include "include.h"

/**
 * @brief 关闭背光 + 关屏供电，CPU 和外设保持运行
 *
 * 与 gui_sleep() 的区别：不设 gui_sleep_sta 标志，不退出 GPU/TFT/CTP。
 * 熄屏顺序：PWM 清零 → 关背光 → 关 VDDLCD（先关背光，避免花屏）
 */
void lunchbox_display_off(void);

/**
 * @brief 开屏供电 + 恢复背光（唤醒显示）
 *
 * 亮屏顺序：开 VDDLCD → 恢复 PWM + 开背光（先开供电，避免花屏）
 */
void lunchbox_display_on(void);

/*
 * ============================================================================
 * func.c 配套改动清单（"关机只关背光"方案）
 * ============================================================================
 *
 *  #  位置 (func.c)            改前                                  改后
 * --- ----------------------- ------------------------------------  ------------------------------------------
 *  1  elunchbox_pwr_manual_    gui_sleep(false)                     //gui_sleep(false)
 *     _shutdown (~L640)                                             lunchbox_display_off()
 *
 *  2  elunchbox_pwr_ble_switch gui_sleep(false)                     //gui_sleep(false)
 *     BLE 关机路径 (~L718)                                          lunchbox_display_off()
 *
 *  3  func_process              sleep_process(bt_is_allow_sleep)    //sleep_process(bt_is_allow_sleep)
 *     手动关机路径 (~L1210)                                         //lunchbox_display_off() (已在#1调用，循环不重复)
 *
 *  4  func_process              sleep_process(bt_is_allow_sleep)    if (!guioff && sleep_process(bt_is_allow_sleep))
 *     通用路径 (~L1342)           无条件调用                         guioff 时跳过，不深度休眠
 *
 *  5  func_process              elunchbox_pwr_manual_shutdown()     //elunchbox_pwr_manual_shutdown()
 *     自动关机路径 (~L1348)                                         gui_off=true + lunchbox_display_off()
 *
 *  6  elunchbox_screen_wake      tft_bglight_force_on()             //tft_bglight_force_on()
 *     唤醒路径 (~L856)                                               lunchbox_display_on() (配对#1-#5)
 *
 * ============================================================================
 * 效果：所有关机路径关背光+关屏供电，TFT/CTP/GPU/BLE/UART/按键扫描保持运行。
 *       唤醒按正确顺序（先供电后背光）恢复，不重初始化显示子系统。
 *
 * 注意：背光为 PWM 控制，led_pg_off() 在 PWM 模式下无效，必须同时调用
 *       lcd_drv_set_brightness(0) 清零 PWM 占空比才能真正熄背光。
 * ============================================================================
 *
 * 2026-07-11 修复1：lunchbox_display_off() 增加 lcd_drv_set_brightness(0)
 *           之前只调 LCD_BL_DIS()→led_pg_off()，PWM 模式下背光不灭
 * 2026-07-11 修复3：增加 VDDLCD 屏供电控制，按正确顺序关/开
 *           lunchbox_display_off: PWM→0 → 关背光 → 关 VDDLCD (避免花屏)
 *           lunchbox_display_on:  开 VDDLCD → 恢复背光 (避免花屏)
 *           同时修改 elunchbox_screen_wake 走 lunchbox_display_on()
 *           (之前直接调 tft_bglight_force_on，醒时无人开 VDDLCD)
 */

#endif // __FUNC_LUNCHBOX_OFF_H
