#include "include.h"
#include "func_key_lock.h"

#if ELUNCHBOX_PANEL_EN

#include "bsp_pt8028_key.h"       /* PT8028_KEY_TCH* 枚举值 */
#include "func_key.h"             /* func_key_tch_to_lunchbox_val */
#include "func.h"
#include "port_pt8028_key.h"
#if USER_PANEL_LED
#include "func_led.h"
#endif

/*
 * 童锁模块 — 纯状态标志位。
 *
 * 不包含任何 GUI 调用（不调 home_ui_lock_overlay_* / compo_update / gui_process）。
 * 页面读取标志位后自行渲染 overlay。
 *
 * 标志位：
 *   is_active()          锁定中
 *   overlay_visible()    是否该显示锁图标 overlay
 *   overlay_is_unlock()  true=解锁图标, false=锁定图标
 *   gui_dirty()          GUI 需要刷新（读后自清）
 */

/* 锁状态 */
static bool key_lock_active;

/* overlay 标志位 */
static bool key_lock_overlay_visible;
static bool key_lock_overlay_is_unlock;

/* 计时 */
static u32 key_lock_hint_show_tick;
static u8  key_lock_hint_min_polls;
static u32 key_lock_hint_cooldown_tick;
static bool key_lock_entry_hint_settled;

/* GUI 脏标志 */
static bool key_lock_gui_dirty;

/* 加热自动锁 */
static u32 key_lock_heat_arm_tick;

/* TCH5 关机计时 */
static bool key_lock_pwr_sound_sent;
static u32 key_lock_pwr_hold_tick;

/* need_key_rel */
static bool key_lock_need_key_rel;
static bool key_lock_ignore_ku_left_once;

/*===========================================================================
 * 内部 helpers
 *===========================================================================*/

static u32 hint_duration_ms(void)
{
    return key_lock_overlay_is_unlock ? KEY_UNLOCK_HINT_MS : KEY_LOCK_HINT_MS;
}

static bool hint_elapsed(void)
{
    if (key_lock_hint_show_tick == 0) return false;
    return tick_check_expire(key_lock_hint_show_tick, hint_duration_ms());
}

static bool hint_in_cooldown(void)
{
    if (key_lock_hint_cooldown_tick == 0) return false;
    return !tick_check_expire(key_lock_hint_cooldown_tick, KEY_LOCK_HINT_QUIET_MS);
}

static void hint_show(bool is_unlock)
{
    key_lock_overlay_is_unlock = is_unlock;
    key_lock_overlay_visible = true;
    key_lock_hint_show_tick = tick_get();
    key_lock_hint_min_polls = 3;
    key_lock_gui_dirty = true;
}

static void hint_hide(void)
{
    bool was_entry_lock;

    if (!key_lock_overlay_visible) return;

    was_entry_lock = (!key_lock_overlay_is_unlock && !key_lock_entry_hint_settled);
    key_lock_overlay_visible = false;
    key_lock_hint_show_tick = 0;
    key_lock_hint_min_polls = 0;
    key_lock_hint_cooldown_tick = tick_get();
    key_lock_gui_dirty = true;

    if (was_entry_lock) {
        key_lock_entry_hint_settled = true;
        key_lock_ignore_ku_left_once = true;
    }
}

static void hint_expire_poll(void)
{
    if (!key_lock_overlay_visible) return;
    if (key_lock_hint_min_polls > 0) { key_lock_hint_min_polls--; return; }
    if (!hint_elapsed()) return;
    hint_hide();
}

static void blocked_hint(void)
{
    if (key_lock_entry_hint_settled && !hint_in_cooldown()) {
        hint_show(false);  /* 锁定图标 */
    }
}

/*===========================================================================
 * 状态查询
 *===========================================================================*/

bool func_key_lock_is_active(void)
{
    return key_lock_active;
}

bool func_key_lock_overlay_visible(void)
{
    return key_lock_overlay_visible;
}

bool func_key_lock_overlay_is_unlock(void)
{
    return key_lock_overlay_is_unlock;
}

bool func_key_lock_gui_dirty(void)
{
    bool d = key_lock_gui_dirty;
    key_lock_gui_dirty = false;
    return d;
}

bool func_key_lock_hint_is_on(void)
{
    return key_lock_overlay_visible;
}

bool func_key_lock_show_status_icon(bool page_local_locked)
{
    return page_local_locked;
}

/*===========================================================================
 * 锁状态切换
 *===========================================================================*/

static void lock_enter(void)
{
    if (key_lock_active) return;
    key_lock_active = true;
    key_lock_need_key_rel = true;
    key_lock_hint_cooldown_tick = 0;
    key_lock_entry_hint_settled = false;
#if USER_PANEL_LED
    func_led_scan();
#endif
    hint_show(false);  /* 锁定图标 */
}

static void lock_exit(void)
{
    if (!key_lock_active) return;
    key_lock_active = false;
    key_lock_need_key_rel = false;
    key_lock_entry_hint_settled = false;
#if USER_PANEL_LED
    func_led_scan();
#endif
    hint_show(true);   /* 解锁图标 */
}

void func_key_lock_toggle(void)
{
    if (key_lock_active) lock_exit();
    else lock_enter();
}

/*===========================================================================
 * 回调 — 由 func_key_poll 调用
 *===========================================================================*/

void func_key_lock_on_blocked_key(u8 tch)
{
#if FUNC_LUNCHBOX_UART_EN
    if (tch <= PT8028_KEY_TCH7) {
        u8 key_val = func_key_tch_to_lunchbox_val(tch);
        if (key_val != 0) lunchbox_key_notify(key_val);
    }
#else
    (void)tch;
#endif
    blocked_hint();
}

void func_key_lock_on_pwr_key_in_lock(void)
{
    if (!key_lock_active) return;
    if (!key_lock_pwr_sound_sent) {
        key_lock_pwr_sound_sent = true;
        key_lock_pwr_hold_tick = tick_get();
#if FUNC_LUNCHBOX_UART_EN
        {
            u8 key_val = func_key_tch_to_lunchbox_val(PT8028_KEY_TCH5);
            if (key_val != 0) lunchbox_key_notify(key_val);
        }
#endif
        blocked_hint();
    }
}

void func_key_lock_on_pwr_key_release(void)
{
    key_lock_pwr_sound_sent = false;
    key_lock_pwr_hold_tick = 0;
}

void func_key_lock_on_all_keys_released(void)
{
    if (key_lock_need_key_rel) {
        key_lock_need_key_rel = false;
        key_lock_ignore_ku_left_once = true;
    }
}

/*===========================================================================
 * 兼容旧接口（旧页面未迁移到 func_key_poll 前使用）
 *===========================================================================*/

bool func_key_lock_press_take_poll(void)
{
    u8 press_tch;

    if (!key_lock_active) return false;
    if (!func_key_peek_raw_press(&press_tch)) return false;
    if (press_tch == PT8028_KEY_TCH0 || press_tch == PT8028_KEY_TCH5) return false;
    if (!func_key_take_raw_press(&press_tch)) return false;
    if (press_tch == PT8028_KEY_TCH0 || press_tch == PT8028_KEY_TCH5) return false;
    blocked_hint();
    return true;
}

bool func_key_lock_filter_tch(u8 tch)
{
    if (!key_lock_active) return false;
    if (tch == PT8028_KEY_TCH0 || tch == PT8028_KEY_TCH5) return false;
    return true;
}

bool func_key_lock_ku_blocked(u16 msg)
{
    if (!key_lock_active) return false;
    if (msg == MSG_CTP_CLICK) return true;
    return true;
}

void func_key_lock_notify_blocked_tch(u8 tch)
{
#if FUNC_LUNCHBOX_UART_EN
    if (tch > PT8028_KEY_TCH7) return;
    {
        u8 key_val = func_key_tch_to_lunchbox_val(tch);
        if (key_val != 0) lunchbox_key_notify(key_val);
    }
#else
    (void)tch;
#endif
}

void func_key_lock_notify_blocked(void)
{
    func_key_lock_notify_blocked_tch(PT8028_KEY_TCH7);
}

bool func_key_lock_press_take_guarded(u8 *out_tch)
{
    u8 tch;
    if (out_tch) *out_tch = 0xff;

    if (!key_lock_active) {
        if (func_key_take_raw_press(&tch) && out_tch) *out_tch = tch;
        return false;
    }
    {
        u8 peek;
        if (func_key_peek_raw_press(&peek)
            && (peek == PT8028_KEY_TCH0 || peek == PT8028_KEY_TCH5)) {
            return false;
        }
    }
    if (func_key_take_raw_press(&tch)) {
        if (tch != PT8028_KEY_TCH0 && tch != PT8028_KEY_TCH5) {
            func_key_lock_on_blocked_key(tch);
            return true;
        }
        if (out_tch) *out_tch = tch;
    }
    return false;
}

/*===========================================================================
 * 加热自动锁
 *===========================================================================*/

static bool heating_active(void)
{
#if FUNC_LUNCHBOX_UART_EN
    if (lunchbox_heating_task_active()) return true;
#endif
    if (heat_display_heating_active()) return true;
    if (func_cb.sta == FUNC_HEAT && func_heat_ui_is_heating()) return true;
    return false;
}

void func_key_lock_on_heating_start(void)
{
    if (key_lock_active) { key_lock_heat_arm_tick = 0; return; }
    if (key_lock_heat_arm_tick != 0) return;
    key_lock_heat_arm_tick = tick_get();
}

void func_key_lock_on_heating_stop(void)
{
    key_lock_heat_arm_tick = 0;
}

static void heat_auto_poll(void)
{
    if (key_lock_heat_arm_tick == 0) return;
    if (key_lock_active) { key_lock_heat_arm_tick = 0; return; }
    if (!heating_active()) { key_lock_heat_arm_tick = 0; return; }
    if (tick_check_expire(key_lock_heat_arm_tick, HEAT_AUTO_LOCK_MS)) {
        key_lock_heat_arm_tick = 0;
        lock_enter();
    }
}

/*===========================================================================
 * 生命周期
 *===========================================================================*/

void func_key_lock_overlay_to_front(void)
{
    /* 页面自行处理 overlay z-order，此处仅标记 */
    if (key_lock_overlay_visible) key_lock_gui_dirty = true;
}

void func_key_lock_on_page_change(void)
{
    /* overlay 由新页面根据标志位重建，此处仅重置内部状态 */
    key_lock_gui_dirty = true;
}

void func_key_lock_on_form_destroy(void)
{
    /* 页面销毁时不做额外操作，标志位由后续页面读取 */
}

void func_key_lock_on_manual_shutdown(void)
{
    key_lock_overlay_visible = false;
    key_lock_hint_show_tick = 0;
    key_lock_hint_min_polls = 0;
    key_lock_gui_dirty = false;
}

/*===========================================================================
 * 每帧轮询 — 只跑计时器，不做 GUI 操作
 *===========================================================================*/

bool func_key_lock_pre_gui_poll(void)
{
    if (!key_lock_active || !key_lock_pwr_sound_sent) return false;
    if (!tick_check_expire(key_lock_pwr_hold_tick, KEY_LOCK_PWR_LONG_GUI_MS)) {
        return false;
    }
    /* TCH5 长按预备关机：隐藏 overlay 标志 */
    key_lock_overlay_visible = false;
    key_lock_hint_show_tick = 0;
    key_lock_hint_min_polls = 0;
    key_lock_gui_dirty = false;
    return true;
}

void func_key_lock_poll(void)
{
    if (elunchbox_pwr_is_manual_off()) return;

    heat_auto_poll();

    /* TCH5 长按关机预备中 → 跳过 hint 过期检查 */
    if (key_lock_active && key_lock_pwr_sound_sent
        && tick_check_expire(key_lock_pwr_hold_tick, KEY_LOCK_PWR_LONG_GUI_MS)) {
        return;
    }

    hint_expire_poll();

    /* 超时兜底隐藏 */
    if (key_lock_overlay_visible && key_lock_hint_show_tick != 0) {
        u32 max_ms = hint_duration_ms() + 1000;
        if (tick_check_expire(key_lock_hint_show_tick, max_ms)) {
            hint_hide();
        }
    }
}

#endif
