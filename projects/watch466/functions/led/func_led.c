#include "include.h"
#include "func_led.h"

#ifndef USER_PANEL_LED
#define USER_PANEL_LED                  0
#endif

#if USER_PANEL_LED

#include "bsp_pt8028_key.h"

/*
 * ============================================================================
 *  IMB-076_LCD_V1.0 原理图 — 面板 LED 映射
 * ============================================================================
 *
 *  PT8028S (U3) 8 通道电容触摸 → AB5790T (U2) PE1~PE4 BCD 接口
 *  面板 LED1~LED6 (白色 0603) → AB5790T PB0/PB1/PB2/PB5/PB6/PB7 GPIO 直驱
 *
 *  原理图接线：
 *    TCH0 锁键   → LED5 (PB6)     TCH1 加热   → LED6 (PB7)
 *    TCH2 减号   → 无 LED          TCH3 模式   → LED3 (PB2)
 *    TCH4 确认   → LED2 (PB1)     TCH5 开关   → LED1 (PB0)
 *    TCH6 加号   → 无 LED          TCH7 预约   → LED4 (PB5)
 *
 *  行为：单灯模式 — 按下哪个键亮哪个灯，松开全灭，同时只亮一盏。
 * ============================================================================
 */

/* ---- TCH → LED ID 映射表 (PT8028_KEY_TCH0 ~ TCH7) ---- */
static const u8 tbl_tch_to_led[8] = {
    FUNC_LED_ID_LOCK,       /* TCH0 锁键   → LED5 (PB6) */
    FUNC_LED_ID_HEAT,       /* TCH1 加热   → LED6 (PB7) */
    FUNC_LED_NONE,          /* TCH2 减号   → 无 LED      */
    FUNC_LED_ID_MODE,       /* TCH3 模式   → LED3 (PB2) */
    FUNC_LED_ID_OK,         /* TCH4 确认   → LED2 (PB1) */
    FUNC_LED_ID_SWITCH,     /* TCH5 开关   → LED1 (PB0) */
    FUNC_LED_NONE,          /* TCH6 加号   → 无 LED      */
    FUNC_LED_ID_RES,        /* TCH7 预约   → LED4 (PB5) */
};

/* ---- LED ID → GPIO 表 (索引 = id - 1) ---- */
static const u8 tbl_led_gpio[FUNC_LED_ID_CNT] = {
    FUNC_LED1_GPIO,         /* LED1 PB0 — 开关 */
    FUNC_LED2_GPIO,         /* LED2 PB1 — 确认 */
    FUNC_LED3_GPIO,         /* LED3 PB2 — 模式 */
    FUNC_LED4_GPIO,         /* LED4 PB5 — 预约 */
    FUNC_LED5_GPIO,         /* LED5 PB6 — 童锁 */
    FUNC_LED6_GPIO,         /* LED6 PB7 — 加热 */
};

static u8 led_last_tch;     /* 上次点亮的 TCH，去抖用。初始化为哨兵值避免与 PT8028_KEY_NONE(0xFF) 碰撞导致首帧跳过 */

/* ---- 底层 GPIO 操作 ---- */
static void led_gpio_set(u8 gpio, bool on)
{
    bool level;

    if (gpio == IO_NONE) {
        return;
    }
#if PANEL_LED_ACTIVE_HIGH
    level = on;
#else
    level = !on;
#endif
    port_gpio_set_out(gpio, level);
}

#define LED_TCH_SENTINEL               0xFE    /* 不等于任何有效 TCH(0~7) 也不同于 NONE(0xFF) */

/* ---- 公开 API ---- */

void func_led_init(void)
{
    u8 i;

    led_last_tch = LED_TCH_SENTINEL;
    for (i = 0; i < FUNC_LED_ID_CNT; i++) {
        led_gpio_set(tbl_led_gpio[i], false);
    }
}

void func_led_all_off(void)
{
    u8 i;

    for (i = 0; i < FUNC_LED_ID_CNT; i++) {
        led_gpio_set(tbl_led_gpio[i], false);
    }
}

void func_led_set(func_led_id_t id, bool on)
{
    if (id < FUNC_LED_ID_SWITCH || id > FUNC_LED_ID_HEAT) {
        return;
    }
    led_gpio_set(tbl_led_gpio[id - 1], on);
}

/*
 * 按 TCH 点亮对应 LED（先全灭再亮单灯）
 */
static void func_led_show_tch(u8 tch)
{
    u8 led_id;

    func_led_all_off();
    if (tch > PT8028_KEY_TCH7) {
        return;
    }
    led_id = tbl_tch_to_led[tch];
    if (led_id != FUNC_LED_NONE) {
        func_led_set((func_led_id_t)led_id, true);
    }
}

/*
 * 每帧调用 — 读取 PT8028 当前按下的 TCH，点亮对应 LED。
 * 无按键时全灭。有去抖：同 TCH 连续帧不重复操作 GPIO。
 * 主线程调用，勿放中断（GPIO 操作可能干扰 LCD 刷新）。
 */
void func_led_scan(void)
{
    u8 tch = pt8028_get_led_tch();

    if (tch == led_last_tch) {
        return;                         /* 同键，跳过 */
    }
    led_last_tch = tch;

    if (tch <= PT8028_KEY_TCH7) {
        func_led_show_tch(tch);         /* 按下 → 亮对应灯 */
    } else {
        func_led_all_off();             /* 松开 → 全灭 */
    }
}

u8 func_led_get_last_tch(void)
{
    return led_last_tch;
}

/*
 * 兼容旧 API — 平台 bsp_key.c 仍调用 panel_led_init / panel_led_scan
 */
void panel_led_init(void)       { func_led_init(); }
void panel_led_scan(void)       { func_led_scan(); }

#endif /* USER_PANEL_LED */
