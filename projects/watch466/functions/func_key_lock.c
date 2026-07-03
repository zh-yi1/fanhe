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

typedef enum {
    KEY_LOCK_HINT_NONE = 0,
    KEY_LOCK_HINT_LOCK,
    KEY_LOCK_HINT_UNLOCK,
} key_lock_hint_mode_t;

static bool key_lock_active;
static bool key_lock_hint_on;
static key_lock_hint_mode_t key_lock_hint_mode;
static u32 key_lock_hint_start;
static u8 key_lock_lp_tch;
static u32 key_lock_lp_tick;
static bool key_lock_lp_wait_rel;
static u32 key_lock_heat_arm_tick;

static void func_key_lock_set(bool locked);

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
    /* 全局按键锁用全屏 new_lock/new_unlock overlay，右上角小锁仅用于页内 screen_locked */
    return page_local_locked;
}

static void func_key_lock_overlay_apply(void)
{
    if (key_lock_hint_on) {
        home_ui_lock_overlay_show(key_lock_hint_mode == KEY_LOCK_HINT_UNLOCK);
    } else {
        home_ui_lock_overlay_hide();
    }
}

static void func_key_lock_hint_show(key_lock_hint_mode_t mode)
{
    key_lock_hint_mode = mode;
    key_lock_hint_on = true;
    key_lock_hint_start = tick_get();
    func_key_lock_overlay_apply();
}

static void func_key_lock_hint_hide(void)
{
    if (!key_lock_hint_on) {
        return;
    }
    key_lock_hint_on = false;
    key_lock_hint_mode = KEY_LOCK_HINT_NONE;
    home_ui_lock_overlay_hide();
}

static u32 func_key_lock_hint_duration_ms(void)
{
    if (key_lock_hint_mode == KEY_LOCK_HINT_UNLOCK) {
        return KEY_UNLOCK_HINT_MS;
    }
    return KEY_LOCK_HINT_MS;
}

void func_key_lock_notify_blocked(void)
{
    if (!key_lock_active) {
        return;
    }
    func_key_lock_hint_show(KEY_LOCK_HINT_LOCK);
}

static void func_key_lock_set(bool locked)
{
    key_lock_active = locked;
#if USER_PANEL_LED
    panel_led_set_lock_latched(locked);
#endif
    if (locked) {
        func_key_lock_hint_show(KEY_LOCK_HINT_LOCK);
    } else {
        func_key_lock_hint_show(KEY_LOCK_HINT_UNLOCK);
    }
}

void func_key_lock_on_page_change(void)
{
    home_ui_lock_overlay_reset();
    if (key_lock_hint_on) {
        func_key_lock_overlay_apply();
    }
}

void func_key_lock_on_form_destroy(void)
{
    home_ui_lock_overlay_reset();
}

bool func_key_lock_filter_tch(u8 tch)
{
    if (!key_lock_active) {
        return false;
    }
    /* 锁定态仅电源键 TCH5 有效 */
    if (tch == PT8028_KEY_TCH5) {
        return false;
    }
    if (tch <= PT8028_KEY_TCH7) {
        func_key_lock_notify_blocked();
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
        func_key_lock_notify_blocked();
        return true;
    }
    usage = (u8)(msg & KEY_USAGE_MASK);
    if (usage == KEY_RIGHT) {
        return false;
    }
    func_key_lock_notify_blocked();
    return true;
}

void func_key_lock_poll(void)
{
    u8 tch;

    func_key_lock_heat_auto_poll();

    if (key_lock_hint_on) {
        if (tick_check_expire(key_lock_hint_start, func_key_lock_hint_duration_ms())) {
            func_key_lock_hint_hide();
        } else {
            home_ui_lock_overlay_bring_front();
        }
    }

    if (key_lock_lp_wait_rel) {
        tch = pt8028_get_press_tch();
        if (tch != PT8028_KEY_TCH0) {
            key_lock_lp_wait_rel = false;
        }
        return;
    }

    tch = pt8028_get_press_tch();
    if (tch == PT8028_KEY_TCH0) {
        if (key_lock_lp_tch != PT8028_KEY_TCH0) {
            key_lock_lp_tch = PT8028_KEY_TCH0;
            key_lock_lp_tick = tick_get();
        } else if (tick_check_expire(key_lock_lp_tick, PT8028_LOCK_LONG_MS)) {
            func_key_lock_set(!key_lock_active);
            key_lock_lp_wait_rel = true;
        }
    } else {
        key_lock_lp_tch = PT8028_KEY_NONE;
    }
}

#endif
