/**
 * elunchbox_lp.c — 饭盒低功耗状态机
 *
 * 管理: 空闲计时 → 关屏 → 息屏倒计时 → 深睡 → 唤醒亮屏
 *
 * 关屏顺序: 等帧完成 → PWM清零 → 关背光 → 0x28 Display Off → 关VDDLCD
 * 亮屏顺序: 开VDDLCD → 0x29 Display On → 恢复PWM+背光
 */

#include "elunchbox_lp.h"

/* ============================================================
 * 物理屏幕控制 (替代 gui_sleep/gui_wakeup，仅关物理屏，保持GPU链路)
 * ============================================================ */

void lunchbox_display_off(void)
{
    os_gui_draw_w4_done();          /* 等当前帧刷完 */
    lcd_drv_set_brightness(0);      /* PWM 占空比置 0 */
    LCD_BL_DIS();                   /* 先关背光（避免花屏） */
    WriteComm(0x28);                /* LCD Display Off，停像素扫描 */
    LCD_POWER_DIS();                /* 再关屏幕供电 VDDLCD */
}

void lunchbox_display_on(void)
{
    LCD_POWER_EN();                 /* 先开 VDDLCD（必须先于背光，否则花屏） */
    WriteComm(0x29);                /* LCD Display On，恢复像素扫描 */
    tft_bglight_force_on();         /* 恢复背光 */
}

/* --- 状态变量 --- */
static bool elunchbox_pwr_gui_off;              /* 息屏标志 */
static s32  elunchbox_guioff_sleep_delay = -1L; /* 息屏→深睡倒计时 (100ms) */
static u32  elunchbox_idle_tmr = (u32)ELUNCHBOX_GUIOFF_TIME_SEC * 10; /* 开机即开始空闲计时 */
u32  elunchbox_saved_clkgat0;                   /* 息屏前保存 CLKGAT0 */

/* --- 状态查询 (tick 在 tmr 5ms 路径调用, 须放 com_text 避免 Flash 抢占 miss) --- */
AT(.com_text.sleep)
bool elunchbox_is_guioff(void)                  { return sys_cb.gui_sleep_sta || elunchbox_pwr_gui_off; }
AT(.com_text.sleep)
bool elunchbox_pwr_gui_off_is_on(void)           { return elunchbox_pwr_gui_off; }
bool elunchbox_guioff_sleep_ready(void)          { return elunchbox_guioff_sleep_delay == 0; }
bool elunchbox_guioff_idle_expired(void)         { return elunchbox_idle_tmr == 0; }

/* --- 息屏倒计时 --- */
void elunchbox_guioff_sleep_delay_reset(void) {
    elunchbox_guioff_sleep_delay = (s32)ELUNCHBOX_GUIOFF_SLEEP_DELAY_SEC * 10;
}
void elunchbox_guioff_sleep_arm_immediate(void) {
    elunchbox_guioff_sleep_delay = 0;       /* 与 lowpower manual_off 一致：立刻进深睡 */
}
AT(.com_text.sleep)
void elunchbox_guioff_sleep_delay_tick(void) {
    if (elunchbox_guioff_sleep_delay > 0) elunchbox_guioff_sleep_delay--;
}

/* --- 空闲计时 --- */
void elunchbox_lp_user_activity_reset(void) {
    if (elunchbox_is_guioff()) return;
    elunchbox_idle_tmr = (u32)ELUNCHBOX_GUIOFF_TIME_SEC * 10;
}
AT(.com_text.sleep)
void elunchbox_guioff_idle_tick(void) {
    if (sys_cb.gui_sleep_sta || elunchbox_pwr_gui_off) {
        elunchbox_idle_tmr = (u32)ELUNCHBOX_GUIOFF_TIME_SEC * 10;
        return;
    }
    if (elunchbox_idle_tmr > 0) elunchbox_idle_tmr--;
}

/* --- 关屏 --- */
void elunchbox_screen_off(void)
{
    if (elunchbox_pwr_gui_off && sys_cb.gui_sleep_sta) return;

    elunchbox_pwr_gui_off = true;
    elunchbox_guioff_sleep_delay_reset();   /* 30s 倒计时后再进深睡, 给 BT 栈时间切状态 */
    elunchbox_saved_clkgat0 = CLKGAT0;

    WDT_CLR();
    lunchbox_display_off();         /* 等帧完成→关背光→关VDDLCD，GPU链路保持 */
    /* 不设 gui_sleep_sta：GPU 链路保持，gui_sleep(true) 从未调用，无需 gui_wakeup */
    WDT_CLR();
    printf("elunchbox: screen off\n");
}

/* --- 亮屏唤醒 --- */
void elunchbox_pwr_gui_wake(void)
{
    if (!elunchbox_is_guioff()) return;

    elunchbox_pwr_gui_off = false;
    elunchbox_guioff_sleep_delay_reset();
    CLKGAT0 = elunchbox_saved_clkgat0;

    if (sys_cb.gui_sleep_sta) {
        gui_wakeup();           /* 深睡后恢复 GPU */
    }
    lunchbox_display_on();      /* 物理屏: VDDLCD→0x29→背光 */
    elunchbox_user_activity_reset();
    printf("elunchbox: screen wake\n");
}
