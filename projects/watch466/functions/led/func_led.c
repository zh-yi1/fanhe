#include "include.h"
#include "func_led.h"

#ifndef USER_PANEL_LED
#define USER_PANEL_LED                  0
#endif

#if USER_PANEL_LED

#include "bsp_pt8028_key.h"
#include "func_key_lock.h"
#if FUNC_LUNCHBOX_UART_EN
#include "lb_ui_state.h"    /* 预约灯: lunchbox_reservation_pending() */
#endif

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
 *  行为：
 *    - 未锁：按下哪个键亮哪个灯，松开全灭
 *    - 童锁中：锁键灯(LED5)常亮；其它键按下时仍可亮对应灯，且锁灯不灭
 *    - 有待执行预约：预约灯(LED4)常亮；预约到点开始执行 → 灭
 *    - 加热中(模式1~4)：加热灯(LED6)常亮；任务结束 → 灭。保温不算
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
static bool led_last_locked;
static bool led_last_res;   /* 预约灯常亮态跟踪 */

/*
 * 预约灯该不该常亮：列表镜像里有启用的预约, 且预约没在执行中。
 *
 * "执行中"两层判据互为兜底 (模块执行预约时 DP2 回 4 还是回预约存的模式未实测):
 *   - DP2==预约(4) 且加热使能 → 直接判执行中, 灯灭
 *   - 加热使能 0→1 时通信层会自动重查列表 (lb_ui_state_feed_dp), 单次预约被
 *     模块消费后从列表消失 → pending 变假, 灯也会灭
 */
static bool led_last_heat;  /* 加热灯常亮态跟踪 */

/* 加热灯该不该常亮：模块在加热 (模式1~4, 含预约到点执行)。保温(5)不算 */
static bool led_heat_active(void)
{
#if FUNC_LUNCHBOX_UART_EN
    lb_ui_state_t *st = lb_ui_state_get();

    return st->valid && st->heat_enable
        && st->heat_mode >= LB_MODE_CUSTOM && st->heat_mode <= LB_MODE_RESERVE;
#else
    return false;
#endif
}

static bool led_res_pending(void)
{
#if FUNC_LUNCHBOX_UART_EN
    lb_ui_state_t *st = lb_ui_state_get();

    if (st->valid && st->heat_enable && st->heat_mode == LB_MODE_RESERVE) {
        return false;                   /* 预约执行中 → 灯灭 */
    }
    return lunchbox_reservation_pending();
#else
    return false;
#endif
}

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
    led_last_locked = false;
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
 * 童锁激活时 LED5 常亮；有待执行预约时 LED4 常亮；加热中 LED6 常亮；
 * 松开其它键时常亮灯不灭。
 * 主线程调用，勿放中断（GPIO 操作可能干扰 LCD 刷新）。
 */
void func_led_scan(void)
{
    u8 tch = pt8028_get_led_tch();
    bool locked = func_key_lock_is_active();
    bool res = led_res_pending();
    bool heat = led_heat_active();

    if (tch == led_last_tch && locked == led_last_locked
        && res == led_last_res && heat == led_last_heat) {
        return;                         /* 同态，跳过 */
    }
    led_last_tch = tch;
    led_last_locked = locked;
    led_last_res = res;
    led_last_heat = heat;

    if (tch <= PT8028_KEY_TCH7) {
        func_led_show_tch(tch);         /* 按下 → 亮对应灯 */
        if (locked) {
            func_led_set(FUNC_LED_ID_LOCK, true); /* 童锁态锁灯常亮 */
        }
    } else if (locked) {
        func_led_all_off();
        func_led_set(FUNC_LED_ID_LOCK, true);
    } else {
        func_led_all_off();             /* 松开且未锁 → 全灭 */
    }

    if (res) {
        func_led_set(FUNC_LED_ID_RES, true);      /* 预约灯常亮, 盖在最后 */
    }
    if (heat) {
        func_led_set(FUNC_LED_ID_HEAT, true);     /* 加热灯常亮, 同上 */
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
