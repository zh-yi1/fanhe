/**
 * elunchbox_lp.c — 饭盒低功耗状态机
 *
 * 管理: 空闲计时 → 关屏 → 息屏倒计时 → 深睡 → 唤醒亮屏
 *
 * 关屏顺序: 等帧完成 → PWM清零 → 关背光 → 0x28 Display Off → 关VDDLCD
 * 亮屏顺序: 开VDDLCD → 0x29 Display On → 恢复PWM+背光
 */

#include "elunchbox_lp.h"
#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#endif

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
static bool elunchbox_manual_off;               /* manual_off 模式标志 */
static bool elunchbox_intentional_wake;         /* 有意的唤醒 (gui_wakeup 可用) */
static bool elunchbox_manual_wake_pending;      /* PE1 下降沿 pending */
static bool elunchbox_guioff_sleep_mode;        /* guioff sleep mode 标志 */
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

    /* 退出 manual_off 模式，重置 PowerSwitch 发送标记 */
    if (elunchbox_manual_off) {
        elunchbox_manual_off = false;
        elunchbox_pwroff_sent_reset();
    }

    if (sys_cb.gui_sleep_sta) {
        gui_wakeup();           /* 深睡后恢复 GPU */
    }
    lunchbox_display_on();      /* 物理屏: VDDLCD→0x29→背光 */
    elunchbox_user_activity_reset();
    printf("elunchbox: screen wake\n");
}

/* ============================================================
 * manual_off 深度休眠管理
 * ============================================================ */

bool elunchbox_pwr_is_manual_off(void)           { return elunchbox_manual_off; }
void elunchbox_pwr_manual_off_set(void)          { elunchbox_manual_off = true; }
void elunchbox_pwr_manual_off_clr(void)          { elunchbox_manual_off = false; }

void elunchbox_pwr_intentional_wake_set(bool v)  { elunchbox_intentional_wake = v; }
bool elunchbox_pwr_manual_off_gui_wake_ok(void)  { return elunchbox_intentional_wake; }

/* PE1 FLAG software edge detect (manual_off deep sleep 轮询用) */
/* PE1 FLAG 直接读 GPIO 电平: 1=HIGH(无键) 0=LOW(有键) */
static u8 elunchbox_read_pe1_flag(void)
{
    return (GPIOE >> 1) & 1;
}

void elunchbox_manual_off_sleep_poll(void)
{
    static u8 last_flag;
    u8 flag = elunchbox_read_pe1_flag();
    if (last_flag != 0 && flag == 0) {
        elunchbox_manual_wake_pending = true;   /* HIGH→LOW 下降沿 */
    }
    last_flag = flag;
}

bool elunchbox_manual_wake_pending_peek(void)    { return elunchbox_manual_wake_pending; }
void elunchbox_manual_wake_pending_take(void)    { elunchbox_manual_wake_pending = false; }

/* guioff sleep mode 管理 */
bool elunchbox_guioff_in_sleep_mode(void)        { return elunchbox_guioff_sleep_mode; }
void elunchbox_guioff_sleep_mode_enter(void)     { elunchbox_guioff_sleep_mode = true; }
void elunchbox_guioff_sleep_service(void)        { /* 预约管理在 deep sleep 内的 tick */ }

void elunchbox_guioff_sleep_post_wake(bool wkp)
{
    elunchbox_guioff_sleep_mode = false;
    elunchbox_guioff_sleep_delay_reset();
    if (wkp) {
        CLKGAT0 = elunchbox_saved_clkgat0;
        elunchbox_manual_off = false;
        elunchbox_pwr_gui_off = false;
        elunchbox_pwroff_sent_reset();
        /* 唤醒后强制回主界面 */
        extern func_cb_t func_cb;
        if (func_cb.sta != FUNC_HOME) {
            func_cb.sta = FUNC_HOME;
        }
#if USER_PT8028_KEY
        pt8028_gpio_ensure_periodic();
        pt8028_key_scan();
#endif
    }
}

void elunchbox_pwr_gui_wake_reason(const char *reason)
{
    printf("elunchbox: wake reason=%s\n", reason);
    elunchbox_pwr_gui_wake();
}

/* 加热进行中 → 不关屏/不深睡 (当前版本返回 false, 后续由 func_heat 模块对接) */
bool elunchbox_heating_blocks_idle(void)
{
    return false;
}

/* manual_off 中按键刚按下 → 保持唤醒等长按计时器 */
bool elunchbox_pwr_manual_off_should_stay_awake(void)
{
#if USER_PT8028_KEY
    /* TCH5 正在按下 → 保持唤醒等长按计时器 */
    if (pt8028_get_press_tch() == 5) {
        return true;
    }
#endif
    return false;
}

/* UART TX block stub —— 真实实现在 comm/lb_uart_app.c, 这里只补串口关掉时的空壳。
 * 少了这个 #if 会和 lb_uart_app.c 撞出 multiple definition。 */
#if !FUNC_LUNCHBOX_UART_EN
void lb_uart_tx_block(bool block)
{
    (void)block;
}
#endif
