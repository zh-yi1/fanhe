#ifndef _FUNC_KEY_LOCK_H
#define _FUNC_KEY_LOCK_H

#include "include.h"
#include "home_ui_lock_overlay.h"

#if ELUNCHBOX_PANEL_EN

#ifndef PT8028_LOCK_LONG_MS
#define PT8028_LOCK_LONG_MS             3000
#endif
#ifndef KEY_LOCK_HINT_MS
#define KEY_LOCK_HINT_MS                3000
#endif
#ifndef KEY_UNLOCK_HINT_MS
#define KEY_UNLOCK_HINT_MS              1500        //解锁图标显示(ms)
#endif
#ifndef HEAT_AUTO_LOCK_MS
#define HEAT_AUTO_LOCK_MS               30000       //加热开始后自动锁屏(ms)
#endif

bool func_key_lock_is_active(void);
bool func_key_lock_show_status_icon(bool page_local_locked);
void func_key_lock_poll(void);
bool func_key_lock_filter_tch(u8 tch);
bool func_key_lock_ku_blocked(u16 msg);
void func_key_lock_on_page_change(void);
void func_key_lock_on_form_destroy(void);
void func_key_lock_notify_blocked(void);
void func_key_lock_on_heating_start(void);
void func_key_lock_on_heating_stop(void);
/** 锁/解锁图标是否正在显示 */
bool func_key_lock_hint_is_on(void);
void func_key_lock_overlay_to_front(void);

#else

static inline bool func_key_lock_is_active(void) { return false; }
static inline bool func_key_lock_show_status_icon(bool page_local_locked)
{
    (void)page_local_locked;
    return false;
}
static inline void func_key_lock_poll(void) {}
static inline bool func_key_lock_filter_tch(u8 tch) { (void)tch; return false; }
static inline bool func_key_lock_ku_blocked(u16 msg) { (void)msg; return false; }
static inline void func_key_lock_on_page_change(void) {}
static inline void func_key_lock_on_form_destroy(void) {}
static inline void func_key_lock_notify_blocked(void) {}
static inline void func_key_lock_on_heating_start(void) {}
static inline void func_key_lock_on_heating_stop(void) {}
static inline bool func_key_lock_hint_is_on(void) { return false; }
static inline void func_key_lock_overlay_to_front(void) {}

#endif

#endif
