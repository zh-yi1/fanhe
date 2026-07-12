/**
 * @file    func_lunchbox_off.c
 * @brief   饭盒熄屏控制 — 关闭背光+屏供电，不深度休眠
 *
 * 设计思路：
 *   原 func_process 手动关机路径末尾调用 sleep_process() → sfunc_sleep()
 *   进入深度休眠（关 BLE、关外设时钟、CPU halt），过于激进。
 *
 *   本模块提供 lunchbox_display_off() 替代：仅关背光+屏供电，
 *   CPU 保持主循环运转，按键扫描/UART 轮询/BLE 事件正常工作。
 *
 *   关屏顺序（参考 func_lunchbox_lcd.c lunchbox_power_off）：
 *     等待帧完成 → PWM 清零 → 关背光 → WriteComm(0x28) Display Off → 关 VDDLCD
 *
 *   注意：TFT 信号线持续翻转会通过 ESD 二极管寄生供电，仅 lcd_pg_off() 不足以
 *         彻底断电。须先发 WriteComm(0x28) 让 LCD 停止像素扫描，再关 VDDLCD。
 */
#include "include.h"
#include "func_lunchbox_off.h"

void lunchbox_display_off(void)
{
    os_gui_draw_w4_done();          // 等当前帧刷完（原 gui_sleep 也有此步骤）
    lunchbox_display_off_fast();
}

void lunchbox_display_off_fast(void)
{
    lcd_drv_set_brightness(0);      // PWM 占空比置 0（led_pg_off 在 PWM 模式下无效）
    LCD_BL_DIS();                   // 先关背光（避免花屏）
    WriteComm(0x28);                // LCD Display Off，停像素扫描（否则信号线寄生供电→无法彻底断电）
    LCD_POWER_DIS();                // 再关屏幕供电 VDDLCD
}
