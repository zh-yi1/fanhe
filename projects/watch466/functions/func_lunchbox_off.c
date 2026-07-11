/**
 * @file    func_lunchbox_off.c
 * @brief   饭盒熄屏控制实现 — 仅关背光+屏供电，不深度休眠
 *
 * 设计思路：
 *   原 func_process 手动关机路径末尾会调用 sleep_process() → sfunc_sleep()
 *   进入深度休眠（关 BLE、关外设时钟、CPU halt）。这在一些场景下过于激进：
 *   CPU 休眠后 tick_get() 停止推进，导致按键长按计时失效；UART RX 仅靠
 *   端口唤醒触发，响应不及时。
 *
 *   本模块提供 lunchbox_display_off() 替代深度休眠：关背光 + 关屏供电，
 *   CPU 继续保持主循环运转，按键扫描/UART 轮询/BLE 事件全部正常工作。
 *   唤醒只需调用 lunchbox_display_on()，无需经过 sleep_proc 的复杂流程。
 *
 *   开关顺序（参考 func_lunchbox_lcd.c）：
 *     熄屏：PWM→0 → 关背光 → 关 VDDLCD（先关背光，避免花屏）
 *     亮屏：开 VDDLCD → 恢复背光（先开屏供电，避免花屏）
 */
#include "include.h"
#include "func_lunchbox_off.h"

void lunchbox_display_off(void)
{
    os_gui_draw_w4_done();          // 等当前帧刷完（原 gui_sleep 也有此步骤）
    lcd_drv_set_brightness(0);      // PWM 占空比置 0（led_pg_off 在 PWM 模式下无效）
    LCD_BL_DIS();                   // 先关背光（避免花屏）
    WriteComm(0x28);                // LCD Display Off（停像素扫描，否则信号线寄生供电→无法彻底断电）
    LCD_POWER_DIS();                // 再关屏幕供电 VDDLCD
}

void lunchbox_display_on(void)
{
    LCD_POWER_EN();                 // 先开 VDDLCD（必须先于背光，否则花屏）
    WriteComm(0x29);                // LCD Display On（恢复像素扫描）
    tft_bglight_force_on();         // 恢复背光（内部调用 LCD_BL_EN + PWM 恢复）
}
