#include "include.h"
#include "func_key_lock.h"

#if ELUNCHBOX_PANEL_EN

#include "home_ui_lock_overlay.h"
#include "bsp_pt8028_key.h"
#include "func.h"
#include "func_lunchbox_lcd.h"
#include "heat_display_reg.h"
#if USER_PANEL_LED
#include "port_panel_led.h"
#endif

/*
 * 按键锁定需求：
 * 1. 长按锁键 3s → 锁定 + LED5 亮 + 显示锁图标 3s 后自动消失
 * 2. 锁定态除电源键(TCH5)外无效；按任意其它键 → 弹出锁图标 3s 后消失
 * 3. 锁定态长按锁键 3s → 解锁 + LED5 灭 + 解锁图标 1.5s 后消失
 * 全页面统一：func_key_lock_press_take_poll / func_key_lock_poll / func_key_lock_ku_blocked
 */

typedef enum {
    KEY_LOCK_HINT_NONE = 0,
    KEY_LOCK_HINT_LOCK,
    KEY_LOCK_HINT_UNLOCK,
} key_lock_hint_mode_t;

static bool key_lock_active;
static bool key_lock_hint_on;
static key_lock_hint_mode_t key_lock_hint_mode;
static u32 key_lock_hint_show_tick;
static u8 key_lock_hint_min_polls;
static u8 key_lock_lp_tch;
static u32 key_lock_lp_tick;
static bool key_lock_lp_wait_rel;
static u32 key_lock_heat_arm_tick;
static u8 key_lock_press_edge_tch;
static bool key_lock_ignore_ku_left_once;

static void func_key_lock_set(bool locked);
static void func_key_lock_hint_hide(void);
static void func_key_lock_hint_show(key_lock_hint_mode_t mode, bool force);
static void func_key_lock_hint_user_key(void);
static void func_key_lock_lp_reset(void);

static void func_key_lock_lp_reset(void)
{
    key_lock_lp_tch = PT8028_KEY_NONE;
    key_lock_lp_tick = 0;
}

static void func_key_lock_lp_toggle(void)
{
    func_key_lock_set(!key_lock_active);
    key_lock_lp_wait_rel = true;
    func_key_lock_lp_reset();
}

static u32 func_key_lock_hint_duration_ms(key_lock_hint_mode_t mode)
{
    return (mode == KEY_LOCK_HINT_UNLOCK) ? KEY_UNLOCK_HINT_MS : KEY_LOCK_HINT_MS;
}

static bool func_key_lock_hint_elapsed(u32 dur_ms)
{
    u32 now;

    if (key_lock_hint_show_tick == 0) {
        return false;
    }
    now = tick_get();
    return (u32)(now - key_lock_hint_show_tick) >= dur_ms;
}

static void func_key_lock_hint_show(key_lock_hint_mode_t mode, bool force)
{
    /* 已在显示同类图标时不重置计时，避免杂散 KU/UART 事件导致 3s 永不到期 */
    if (!force && key_lock_hint_on && key_lock_hint_mode == mode) {
        return;
    }
    key_lock_hint_mode = mode;
    key_lock_hint_on = true;
    key_lock_hint_show_tick = tick_get();
    key_lock_hint_min_polls = 3;
    home_ui_lock_overlay_show(mode == KEY_LOCK_HINT_UNLOCK);
    func_key_lock_overlay_to_front();
#if ELUNCHBOX_PANEL_EN
    if (func_cb.sta == FUNC_HOME) {
        func_home_gui_mark_dirty();
    }
    gui_widget_refresh();
#endif
}

static void func_key_lock_hint_user_key(void)
{
    if (!key_lock_active || key_lock_lp_wait_rel) {
        return;
    }
    /* 图标已显示时不重置计时，避免同一次按键按下/抬起/多路径重复触发 */
    if (key_lock_hint_on && key_lock_hint_mode == KEY_LOCK_HINT_LOCK) {
        return;
    }
    func_key_lock_hint_show(KEY_LOCK_HINT_LOCK, true);
}

static void func_key_lock_hint_hide(void)
{
    if (!key_lock_hint_on) {
        return;
    }
    key_lock_hint_on = false;
    key_lock_hint_mode = KEY_LOCK_HINT_NONE;
    key_lock_hint_show_tick = 0;
    key_lock_hint_min_polls = 0;
    home_ui_lock_overlay_hide();
#if ELUNCHBOX_PANEL_EN
    if (func_cb.sta == FUNC_HOME) {
        func_home_gui_mark_dirty();
    }
    gui_widget_refresh();
#endif
}

static void func_key_lock_hint_expire_poll(void)
{
    u32 dur;

    if (!key_lock_hint_on) {
        return;
    }
    if (key_lock_hint_min_polls > 0) {
        key_lock_hint_min_polls--;
        return;
    }
    dur = func_key_lock_hint_duration_ms(key_lock_hint_mode);
    if (!func_key_lock_hint_elapsed(dur)) {
        return;
    }
    func_key_lock_hint_hide();
}

static bool func_key_lock_heating_active(void)
{
#if FUNC_LUNCHBOX_UART_EN
    if (lunchbox_heating_task_active()) {
        return true;
    }
#endif
    if (heat_display_heating_active()) {
        return true;
    }
    if (func_cb.sta == FUNC_HEAT && func_heat_ui_is_heating()) {
        return true;
    }
    return false;
}

void func_key_lock_on_heating_start(void)
{
    if (key_lock_active) {
        key_lock_heat_arm_tick = 0;
        return;
    }
    if (key_lock_heat_arm_tick != 0) {
        return;
    }
    key_lock_heat_arm_tick = tick_get();
}

void func_key_lock_on_heating_stop(void)
{
    key_lock_heat_arm_tick = 0;
}

static void func_key_lock_heat_auto_poll(void)
{
    if (key_lock_heat_arm_tick == 0) {
        return;
    }
    if (key_lock_active) {
        key_lock_heat_arm_tick = 0;
        return;
    }
    if (!func_key_lock_heating_active()) {
        key_lock_heat_arm_tick = 0;
        return;
    }
    if (tick_check_expire(key_lock_heat_arm_tick, HEAT_AUTO_LOCK_MS)) {
        key_lock_heat_arm_tick = 0;
        func_key_lock_set(true);
    }
}

bool func_key_lock_is_active(void)
{
    return key_lock_active;
}

bool func_key_lock_show_status_icon(bool page_local_locked)
{
    return page_local_locked;
}

void func_key_lock_notify_blocked(void)
{
    /* 仅表示按键被拦截；锁图标只由 func_key_lock_set / hint_user_key 弹出，
       避免 UART/KU 杂散事件在 3s 消失后再次自动弹出。 */
}

static void func_key_lock_set(bool locked)
{
    key_lock_press_edge_tch = PT8028_KEY_NONE;

    if (locked) {
        if (key_lock_active) {
            return;
        }
        key_lock_active = true;
#if USER_PANEL_LED
        panel_led_set_lock_latched(true);
        panel_led_scan();
#endif
        func_key_lock_hint_show(KEY_LOCK_HINT_LOCK, true);
        return;
    }

    if (!key_lock_active) {
        return;
    }
    key_lock_active = false;
#if USER_PANEL_LED
    panel_led_set_lock_latched(false);
    panel_led_scan();
#endif
    func_key_lock_hint_show(KEY_LOCK_HINT_UNLOCK, true);
}

bool func_key_lock_hint_is_on(void)
{
    return key_lock_hint_on;
}

void func_key_lock_overlay_to_front(void)
{
    if (key_lock_hint_on) {
        home_ui_lock_overlay_bring_front();
    }
}

void func_key_lock_on_page_change(void)
{
    home_ui_lock_overlay_reset();
    if (key_lock_hint_on) {
        home_ui_lock_overlay_show(key_lock_hint_mode == KEY_LOCK_HINT_UNLOCK);
    }
}

void func_key_lock_on_form_destroy(void)
{
    home_ui_lock_overlay_reset();
}

bool func_key_lock_press_take_poll(void)
{
    u8 press_tch;

    if (!key_lock_active) {
        return false;
    }
    press_tch = pt8028_peek_press_tch();
    if (press_tch > PT8028_KEY_TCH7 || press_tch == PT8028_KEY_TCH5) {
        return false;
    }
    press_tch = pt8028_take_press_tch();
    if (press_tch > PT8028_KEY_TCH7) {
        return false;
    }
    (void)func_key_lock_filter_tch(press_tch);
    return true;
}

bool func_key_lock_filter_tch(u8 tch)
{
    if (!key_lock_active) {
        return false;
    }
    if (tch == PT8028_KEY_TCH5) {
        return false;
    }
    if (tch == PT8028_KEY_TCH0) {
        /* 锁键短按：弹出锁图标；长按 3s 解锁由 lp_poll 处理 */
        if (!key_lock_lp_wait_rel) {
            func_key_lock_hint_user_key();
        }
        return true;
    }
    if (tch <= PT8028_KEY_TCH7) {
        func_key_lock_hint_user_key();
    }
    return true;
}

bool func_key_lock_ku_blocked(u16 msg)
{
    u8 usage;

    if (!key_lock_active) {
        return false;
    }
    if (msg == MSG_CTP_CLICK) {
        return true;
    }
    usage = (u8)(msg & KEY_USAGE_MASK);
    if (usage == KEY_RIGHT) {
        return false;
    }
    if (usage == KEY_LEFT) {
        if (key_lock_ignore_ku_left_once) {
            key_lock_ignore_ku_left_once = false;
            return true;
        }
        /* 锁键短按/松手：用户主动按键，弹出锁图标 */
        func_key_lock_hint_user_key();
    }
    return true;
}

static void func_key_lock_active_press_poll(void)
{
    u8 tch;

    if (!key_lock_active || key_lock_lp_wait_rel) {
        key_lock_press_edge_tch = PT8028_KEY_NONE;
        return;
    }

    tch = pt8028_get_press_tch();
    if (tch == PT8028_KEY_NONE || tch == PT8028_KEY_TCH5) {
        key_lock_press_edge_tch = PT8028_KEY_NONE;
        return;
    }
    if (tch == PT8028_KEY_TCH0 && key_lock_lp_wait_rel) {
        key_lock_press_edge_tch = PT8028_KEY_NONE;
        return;
    }

    if (key_lock_press_edge_tch != tch) {
        key_lock_press_edge_tch = tch;
        func_key_lock_hint_user_key();
    }
}

static void func_key_lock_lp_poll(void)
{
    u8 tch;

    if (key_lock_lp_wait_rel) {
        if (pt8028_get_press_tch() != PT8028_KEY_TCH0) {
            key_lock_lp_wait_rel = false;
            key_lock_ignore_ku_left_once = true;
            func_key_lock_lp_reset();
        }
        return;
    }

    tch = pt8028_get_press_tch();
    if (tch == PT8028_KEY_TCH0) {
        if (key_lock_lp_tch != PT8028_KEY_TCH0) {
            key_lock_lp_tch = PT8028_KEY_TCH0;
            key_lock_lp_tick = tick_get();
        } else if (tick_check_expire(key_lock_lp_tick, PT8028_LOCK_LONG_MS)) {
            func_key_lock_lp_toggle();
        }
    } else {
        func_key_lock_lp_reset();
    }
}

void func_key_lock_poll(void)
{
    (void)func_key_lock_press_take_poll();
    func_key_lock_lp_poll();
    func_key_lock_heat_auto_poll();
    func_key_lock_active_press_poll();
    func_key_lock_hint_expire_poll();

    if (!key_lock_hint_on && home_ui_lock_overlay_is_visible()) {
        home_ui_lock_overlay_hide();
#if ELUNCHBOX_PANEL_EN
        gui_widget_refresh();
#endif
    }
}

#endif
