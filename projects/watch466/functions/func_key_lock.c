#include "include.h"
#include "func_key_lock.h"

#if ELUNCHBOX_PANEL_EN

#include "home_ui_lock_overlay.h"
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#include "func.h"
#include "func_lunchbox_lcd.h"
#include "heat_display_reg.h"
#if USER_PANEL_LED
#include "port_panel_led.h"
#endif

/*
 * 进入锁定（30s 自动 / 长按 3s）：LED5 亮 + 锁图标 3s → 消失后保持静默
 * 静默后用户按其它键：锁图标 3s；长按锁键(TCH0) 3s 解锁：LED5 灭 + 解锁图标 1.5s 后消失
 * 锁定态：TCH0 长按 3s 解锁；TCH5 长按 3s 关机停加热；两键短按蜂鸣+锁图标
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
static bool key_lock_ignore_ku_left_once;
static bool key_lock_need_key_rel;
static u32 key_lock_hint_cooldown_tick;
static bool key_lock_hint_gui_refresh;
static bool key_lock_entry_hint_settled;  /* 进入锁定的首次 3s 提示已结束 */
static bool key_lock_pwr_sound_sent;      /* TCH5 按下沿已登记蜂鸣 */
static u32 key_lock_pwr_hold_tick;        /* TCH5 本次按下时刻 */

static void func_key_lock_enter(bool from_long_press);
static void func_key_lock_exit(void);
static void func_key_lock_hint_hide(void);
static void func_key_lock_hint_show(key_lock_hint_mode_t mode);
static void func_key_lock_lp_reset(void);
static void func_key_lock_on_locked_enter(void);
static void func_key_lock_need_key_rel_poll(void);

static void func_key_lock_lp_reset(void)
{
    key_lock_lp_tch = PT8028_KEY_NONE;
    key_lock_lp_tick = 0;
}

static u32 func_key_lock_hint_duration_ms(key_lock_hint_mode_t mode)
{
    return (mode == KEY_LOCK_HINT_UNLOCK) ? KEY_UNLOCK_HINT_MS : KEY_LOCK_HINT_MS;
}

static bool func_key_lock_hint_elapsed(u32 dur_ms)
{
    if (key_lock_hint_show_tick == 0) {
        return false;
    }
    return tick_check_expire(key_lock_hint_show_tick, dur_ms);
}

static bool func_key_lock_hint_in_cooldown(void)
{
    if (key_lock_hint_cooldown_tick == 0) {
        return false;
    }
    return !tick_check_expire(key_lock_hint_cooldown_tick, KEY_LOCK_HINT_QUIET_MS);
}

static void func_key_lock_on_locked_enter(void)
{
    key_lock_need_key_rel = true;
    key_lock_hint_cooldown_tick = 0;
    key_lock_entry_hint_settled = false;
    pt8028_release_clear();
}

static void func_key_lock_need_key_rel_poll(void)
{
    if (!key_lock_need_key_rel) {
        return;
    }
    if (pt8028_get_press_tch() == PT8028_KEY_NONE && !pt8028_is_press_active()) {
        key_lock_need_key_rel = false;
        key_lock_ignore_ku_left_once = true;
    }
}

static void func_key_lock_hint_show(key_lock_hint_mode_t mode)
{
    if (key_lock_hint_on && key_lock_hint_mode == mode) {
        return;
    }
    key_lock_hint_mode = mode;
    key_lock_hint_on = true;
    key_lock_hint_show_tick = tick_get();
    key_lock_hint_min_polls = 3;
    key_lock_hint_gui_refresh = true;
    home_ui_lock_overlay_show(mode == KEY_LOCK_HINT_UNLOCK);
#if ELUNCHBOX_PANEL_EN
    if (func_cb.sta == FUNC_HOME) {
        func_home_gui_mark_dirty();
    }
#endif
}

static void func_key_lock_hint_hide(void)
{
    bool was_entry_lock;

    if (!key_lock_hint_on) {
        return;
    }
    was_entry_lock = (key_lock_hint_mode == KEY_LOCK_HINT_LOCK && !key_lock_entry_hint_settled);
    key_lock_hint_on = false;
    key_lock_hint_mode = KEY_LOCK_HINT_NONE;
    key_lock_hint_show_tick = 0;
    key_lock_hint_min_polls = 0;
    key_lock_hint_cooldown_tick = tick_get();
    key_lock_hint_gui_refresh = true;
    home_ui_lock_overlay_hide();
    if (was_entry_lock) {
        key_lock_entry_hint_settled = true;
        key_lock_ignore_ku_left_once = true;
    }
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
        func_key_lock_enter(false);
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

void func_key_lock_notify_blocked_tch(u8 tch)
{
#if FUNC_LUNCHBOX_UART_EN
    u8 key_val;

    if (tch > PT8028_KEY_TCH7) {
        return;
    }
    key_val = pt8028_tch_to_lunchbox_key(tch);
    if (key_val != 0) {
        lunchbox_key_notify(key_val);
    }
#else
    (void)tch;
#endif
}

void func_key_lock_notify_blocked(void)
{
    func_key_lock_notify_blocked_tch(PT8028_KEY_TCH7);
}

static void func_key_lock_enter(bool from_long_press)
{
    if (key_lock_active) {
        return;
    }
    key_lock_active = true;
    func_key_lock_on_locked_enter();
#if USER_PANEL_LED
    panel_led_set_lock_latched(true);
    panel_led_scan();
#endif
    func_key_lock_hint_show(KEY_LOCK_HINT_LOCK);
    if (from_long_press) {
        key_lock_lp_wait_rel = true;
        func_key_lock_lp_reset();
    }
}

static void func_key_lock_exit(void)
{
    if (!key_lock_active) {
        return;
    }
    key_lock_active = false;
    key_lock_need_key_rel = false;
    key_lock_entry_hint_settled = false;
#if USER_PANEL_LED
    panel_led_set_lock_latched(false);
    panel_led_scan();
#endif
    func_key_lock_hint_show(KEY_LOCK_HINT_UNLOCK);
}

static void func_key_lock_lp_toggle(void)
{
    if (key_lock_active) {
        func_key_lock_exit();
        key_lock_lp_wait_rel = true;
        func_key_lock_lp_reset();
    } else {
        func_key_lock_enter(true);
    }
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

void func_key_lock_on_manual_shutdown(void)
{
    key_lock_hint_on = false;
    key_lock_hint_mode = KEY_LOCK_HINT_NONE;
    key_lock_hint_show_tick = 0;
    key_lock_hint_min_polls = 0;
    key_lock_hint_gui_refresh = false;
    home_ui_lock_overlay_shutdown();
}

static void func_key_lock_blocked_hint(void)
{
    if (key_lock_entry_hint_settled && !func_key_lock_hint_in_cooldown()) {
        func_key_lock_hint_show(KEY_LOCK_HINT_LOCK);
    }
}

bool func_key_lock_press_take_poll(void)
{
    u8 press_tch;

    if (!key_lock_active) {
        return false;
    }
    press_tch = pt8028_peek_press_tch();
    if (press_tch > PT8028_KEY_TCH7) {
        return false;
    }
    /* TCH0 留给长按 3s 解锁；TCH5 留给长按 3s 关机 */
    if (press_tch == PT8028_KEY_TCH0 || press_tch == PT8028_KEY_TCH5) {
        return false;
    }
    press_tch = pt8028_take_press_tch();
    if (press_tch > PT8028_KEY_TCH7 || press_tch == PT8028_KEY_TCH0
        || press_tch == PT8028_KEY_TCH5) {
        return false;
    }
    func_key_lock_blocked_hint();
    return true;
}

bool func_key_lock_press_take_guarded(u8 *out_tch)
{
    u8 tch;

    if (out_tch != NULL) {
        *out_tch = 0xff;
    }
    if (!key_lock_active) {
        tch = pt8028_take_press_tch();
        if (out_tch != NULL) {
            *out_tch = tch;
        }
        return false;
    }
    /* TCH0 长按解锁 / TCH5 长按关机：不吞键、不弹锁定提示 */
    if (pt8028_peek_press_tch() == PT8028_KEY_TCH0
        || pt8028_peek_press_tch() == PT8028_KEY_TCH5) {
        return false;
    }
    if (func_key_lock_press_take_poll()) {
        return true;
    }
    tch = pt8028_take_press_tch();
    if (out_tch != NULL) {
        *out_tch = tch;
    }
    return false;
}

bool func_key_lock_filter_tch(u8 tch)
{
    if (!key_lock_active) {
        return false;
    }
    /* 锁键/开关键留给各自长按逻辑 */
    if (tch == PT8028_KEY_TCH0 || tch == PT8028_KEY_TCH5) {
        return false;
    }
    return true;
}

bool func_key_lock_ku_blocked(u16 msg)
{
    if (!key_lock_active) {
        return false;
    }
    if (msg == MSG_CTP_CLICK) {
        return true;
    }
    /* 锁定态下所有 KU 消息全部拦截（电源键 TCH5 走 touch 路径，不经过 KU） */
    return true;
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
        if (key_lock_active && key_lock_lp_tch == PT8028_KEY_TCH0) {
            pt8028_defer_key_sound_tch(PT8028_KEY_TCH0);
            func_key_lock_blocked_hint();
        }
        func_key_lock_lp_reset();
    }
}

static bool func_key_lock_pwr_long_hold(void)
{
    u8 tch = pt8028_get_press_tch();

    return (tch == PT8028_KEY_TCH5);
}

static bool func_key_lock_pwr_long_gui_hold(void)
{
    if (!key_lock_active || !func_key_lock_pwr_long_hold()) {
        return false;
    }
    if (key_lock_pwr_hold_tick == 0) {
        return false;
    }
    return tick_check_expire(key_lock_pwr_hold_tick, KEY_LOCK_PWR_LONG_GUI_MS);
}

static void func_key_lock_pwr_sound_poll(void)
{
    u8 tch;

    if (!key_lock_active) {
        key_lock_pwr_sound_sent = false;
        key_lock_pwr_hold_tick = 0;
        return;
    }
    tch = pt8028_get_press_tch();
    if (tch == PT8028_KEY_TCH5) {
        if (!key_lock_pwr_sound_sent) {
            key_lock_pwr_sound_sent = true;
            key_lock_pwr_hold_tick = tick_get();
            pt8028_defer_key_sound_tch(PT8028_KEY_TCH5);
            func_key_lock_blocked_hint();
        }
    } else {
        key_lock_pwr_sound_sent = false;
        key_lock_pwr_hold_tick = 0;
    }
}

static void func_key_lock_dismiss_overlay_for_pwr_hold(void)
{
    if (!key_lock_hint_on && !home_ui_lock_overlay_is_visible()) {
        return;
    }
    key_lock_hint_on = false;
    key_lock_hint_mode = KEY_LOCK_HINT_NONE;
    key_lock_hint_show_tick = 0;
    key_lock_hint_min_polls = 0;
    key_lock_hint_gui_refresh = false;
    home_ui_lock_overlay_shutdown();
}

bool func_key_lock_pre_gui_poll(void)
{
    if (!func_key_lock_pwr_long_gui_hold()) {
        return false;
    }
    func_key_lock_dismiss_overlay_for_pwr_hold();
    return true;
}

void func_key_lock_poll(void)
{
    if (elunchbox_pwr_is_manual_off()) {
        return;
    }

    func_key_lock_need_key_rel_poll();
    func_key_lock_pwr_sound_poll();
    func_key_lock_lp_poll();
    func_key_lock_heat_auto_poll();

#if USER_PT8028_KEY && SOFT_POWER_ON_OFF
    /* 童锁进入时 release_clear 可能打断 session_tch，此处兜底补提交长按关机 */
    if (key_lock_active) {
        pt8028_try_commit_pwr_long();
    }
#endif

    if (func_key_lock_pwr_long_gui_hold()) {
        return;
    }

    func_key_lock_hint_expire_poll();
    (void)func_key_lock_press_take_poll();

    if (!key_lock_hint_on && home_ui_lock_overlay_is_visible()) {
        home_ui_lock_overlay_hide();
        key_lock_hint_gui_refresh = true;
    }

    if (key_lock_hint_on && key_lock_hint_show_tick != 0) {
        u32 max_ms = func_key_lock_hint_duration_ms(key_lock_hint_mode) + 1000;
        if (tick_check_expire(key_lock_hint_show_tick, max_ms)) {
            func_key_lock_hint_hide();
        }
    }

    if (key_lock_hint_gui_refresh && func_cb.frm_main != NULL && !sys_cb.flag_swithing) {
        key_lock_hint_gui_refresh = false;
#if ELUNCHBOX_PANEL_EN
        if (func_cb.sta == FUNC_HOME) {
            func_home_gui_mark_dirty();
        } else {
            compo_update();
            gui_process();
        }
#endif
    }
}

#endif
