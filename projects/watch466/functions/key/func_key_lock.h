#ifndef _FUNC_KEY_LOCK_H
#define _FUNC_KEY_LOCK_H

#include "include.h"

#if ELUNCHBOX_PANEL_EN

#ifndef PT8028_LOCK_LONG_MS
#define PT8028_LOCK_LONG_MS             3000
#endif
#ifndef KEY_LOCK_HINT_MS
#define KEY_LOCK_HINT_MS                3000
#endif
#ifndef KEY_UNLOCK_HINT_MS
#define KEY_UNLOCK_HINT_MS              1500
#endif
#ifndef HEAT_AUTO_LOCK_MS
#define HEAT_AUTO_LOCK_MS               30000
#endif
#ifndef KEY_LOCK_HINT_QUIET_MS
#define KEY_LOCK_HINT_QUIET_MS          300
#endif
#ifndef KEY_LOCK_PWR_LONG_GUI_MS
#define KEY_LOCK_PWR_LONG_GUI_MS        400
#endif

/*===========================================================================
 * 状态标志位 — 页面读取这些来决定 UI 显示
 *
 * 用法（在页面 process 中）：
 *   func_key_lock_poll();
 *   if (func_key_lock_gui_dirty()) {
 *       if (func_key_lock_overlay_visible())
 *           home_ui_lock_overlay_show(func_key_lock_overlay_is_unlock());
 *       else
 *           home_ui_lock_overlay_hide();
 *       func_home_gui_mark_dirty();  // 或其他刷新方式
 *   }
 *===========================================================================*/

bool func_key_lock_is_active(void);          /* 锁定中 */
bool func_key_lock_overlay_visible(void);    /* 锁图标overlay该不该显示 */
bool func_key_lock_overlay_is_unlock(void);  /* true=解锁图标, false=锁定图标 */
bool func_key_lock_gui_dirty(void);          /* GUI需刷新（读后自动清零） */

/*===========================================================================
 * 回调 — 由 func_key_poll 调用，页面勿调
 *===========================================================================*/

void func_key_lock_toggle(void);
void func_key_lock_on_blocked_key(u8 tch);
void func_key_lock_on_pwr_key_in_lock(void);
void func_key_lock_on_pwr_key_release(void);
void func_key_lock_on_all_keys_released(void);

/*===========================================================================
 * 每帧轮询 — 只跑计时器，不做任何 GUI 操作
 *===========================================================================*/

void func_key_lock_poll(void);

/** 关机长按预备中该不该跳过本帧 gui_process */
bool func_key_lock_pre_gui_poll(void);

/*===========================================================================
 * 生命周期
 *===========================================================================*/

void func_key_lock_on_page_change(void);
void func_key_lock_on_form_destroy(void);
void func_key_lock_on_manual_shutdown(void);
void func_key_lock_overlay_to_front(void);
void func_key_lock_on_heating_start(void);
void func_key_lock_on_heating_stop(void);

/*===========================================================================
 * 兼容旧接口（逐步废弃）
 *===========================================================================*/

bool func_key_lock_show_status_icon(bool page_local_locked);
bool func_key_lock_filter_tch(u8 tch);
bool func_key_lock_ku_blocked(u16 msg);
void func_key_lock_notify_blocked(void);
void func_key_lock_notify_blocked_tch(u8 tch);
bool func_key_lock_hint_is_on(void);
bool func_key_lock_press_take_poll(void);
bool func_key_lock_press_take_guarded(u8 *out_tch);

#else

/* Stubs */
static inline bool func_key_lock_is_active(void) { return false; }
static inline bool func_key_lock_overlay_visible(void) { return false; }
static inline bool func_key_lock_overlay_is_unlock(void) { return false; }
static inline bool func_key_lock_gui_dirty(void) { return false; }
static inline void func_key_lock_toggle(void) {}
static inline void func_key_lock_on_blocked_key(u8 tch) { (void)tch; }
static inline void func_key_lock_on_pwr_key_in_lock(void) {}
static inline void func_key_lock_on_pwr_key_release(void) {}
static inline void func_key_lock_on_all_keys_released(void) {}
static inline void func_key_lock_poll(void) {}
static inline bool func_key_lock_pre_gui_poll(void) { return false; }
static inline void func_key_lock_on_page_change(void) {}
static inline void func_key_lock_on_form_destroy(void) {}
static inline void func_key_lock_on_manual_shutdown(void) {}
static inline void func_key_lock_overlay_to_front(void) {}
static inline void func_key_lock_on_heating_start(void) {}
static inline void func_key_lock_on_heating_stop(void) {}
static inline bool func_key_lock_show_status_icon(bool l) { return l; }
static inline bool func_key_lock_filter_tch(u8 tch) { (void)tch; return false; }
static inline bool func_key_lock_ku_blocked(u16 msg) { (void)msg; return false; }
static inline void func_key_lock_notify_blocked(void) {}
static inline void func_key_lock_notify_blocked_tch(u8 tch) { (void)tch; }
static inline bool func_key_lock_hint_is_on(void) { return false; }
static inline bool func_key_lock_press_take_poll(void) { return false; }
static inline bool func_key_lock_press_take_guarded(u8 *out_tch)
{
    if (out_tch) *out_tch = 0xff;
    return false;
}

#endif

#endif
