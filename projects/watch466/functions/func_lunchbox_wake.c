/**
 * @file    func_lunchbox_wake.c
 * @brief   饭盒亮屏唤醒实现 — 恢复屏供电+背光
 *
 * 亮屏顺序（参考 func_lunchbox_lcd.c lunchbox_power_on）：
 *   开 VDDLCD → WriteComm(0x29) Display On → 恢复 PWM + 开背光
 *   VDDLCD 必须先于背光打开，否则花屏
 */
#include "include.h"
#include "func_lunchbox_wake.h"

void lunchbox_display_on(void)
{
    printf("elunchbox: [DBG] lunchbox_display_on: VDDLCD+0x29+backlight\n");
    LCD_POWER_EN();                 // 先开 VDDLCD（必须先于背光，否则花屏）
    WriteComm(0x29);                // LCD Display On，恢复像素扫描
    tft_bglight_force_on();         // 恢复背光（内部调用 LCD_BL_EN + PWM 恢复）
}
