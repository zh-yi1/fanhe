/**
 * @file    func_lunchbox_off.h
 * @brief   饭盒熄屏控制 — 关闭背光+屏供电，不进入深度休眠
 *
 * 关屏顺序（参考 func_lunchbox_lcd.c）：
 *   等待帧完成 → PWM 清零 → 关背光(led_pg_off) → 关 VDDLCD(lcd_pg_off)
 */
#ifndef __FUNC_LUNCHBOX_OFF_H
#define __FUNC_LUNCHBOX_OFF_H

#include "include.h"

void lunchbox_display_off(void);

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
 *     手动关机路径 (~L1210)                                         //lunchbox_display_off() (已在#1调用)
 *
 *  4  func_process              sleep_process(bt_is_allow_sleep)    if (!guioff && sleep_process(bt_is_allow_sleep))
 *     通用路径 (~L1342)           无条件调用                         guioff 时跳过，不深度休眠
 *
 *  5  func_process              elunchbox_pwr_manual_shutdown()     //elunchbox_pwr_manual_shutdown()
 *     自动关机路径 (~L1348)                                         gui_off=true + lunchbox_display_off()
 *
 *  6  elunchbox_pwr_manual_      // elunchbox_saved_clkgat0 =       elunchbox_saved_clkgat0 = CLKGAT0
 *     _shutdown (~L660)           CLKGAT0; (注释掉了保存)             恢复保存，防止唤醒时 CLKGAT0=0→8001蓝屏
 *
 *  7  func_process              无充电唤醒检查                       heat_display_charge_wake_pending()
 *     自动熄屏路径 (~L1308)                                         充电时唤醒屏幕
 *
 * ============================================================================
 * 效果：所有关机路径关背光+关屏供电，TFT/CTP/GPU/BLE/UART/按键扫描保持运行。
 *
 * 注意：背光为 PWM 控制，led_pg_off() 在 PWM 模式下无效，必须同时调用
 *       lcd_drv_set_brightness(0) 清零 PWM 占空比才能真正熄背光。
 * ============================================================================
 *
 * 2026-07-11 初始版本：仅关背光+屏供电，替代 gui_sleep/sleep_process 深度休眠
 *           修复历程详见 func.c 内注释及上方改动清单
 * ============================================================================
 */

#endif // __FUNC_LUNCHBOX_OFF_H
