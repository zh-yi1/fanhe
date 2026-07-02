#include "include.h"
#include "func_key_lock.h"

#if ELUNCHBOX_PANEL_EN

#include "bsp_pt8028_key.h"
#if USER_PANEL_LED
#include "port_panel_led.h"
#endif

typedef struct f_home_t_ f_home_t;
typedef struct f_heat_t_ f_heat_t;
typedef struct f_mode_t_ f_mode_t;
typedef struct f_setup_t_ f_setup_t;
typedef struct f_timeing_t_ f_timeing_t;
typedef struct f_languageing_t_ f_languageing_t;
typedef struct f_verinfo_t_ f_verinfo_t;
typedef struct f_reservation_t_ f_reservation_t;

void func_home_lock_icon_apply(f_home_t *f_home);
void func_heat_lock_icon_apply(f_heat_t *f_heat);
void func_mode_lock_icon_apply(f_mode_t *f_mode);
void func_setup_lock_icon_apply(f_setup_t *f_setup);
void func_timeing_lock_icon_apply(f_timeing_t *f_timeing);
void func_languageing_lock_icon_apply(f_languageing_t *f_lang);
void func_verinfo_lock_icon_apply(f_verinfo_t *f_verinfo);
void func_res_lock_icon_apply(f_reservation_t *f_res);

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

bool func_key_lock_is_active(void)
{
    return key_lock_active;
}

bool func_key_lock_show_status_icon(bool page_local_locked)
{
    return page_local_locked || key_lock_hint_on;
}

static void func_key_lock_refresh_ui(void)
{
    if (func_cb.f_cb == NULL) {
        return;
    }
    switch (func_cb.sta) {
    case FUNC_HOME:
        func_home_lock_icon_apply((f_home_t *)func_cb.f_cb);
        break;
    case FUNC_HEAT:
        func_heat_lock_icon_apply((f_heat_t *)func_cb.f_cb);
        break;
    case FUNC_MODE:
        func_mode_lock_icon_apply((f_mode_t *)func_cb.f_cb);
        break;
    case FUNC_SETUP:
        func_setup_lock_icon_apply((f_setup_t *)func_cb.f_cb);
        break;
    case FUNC_TIMEING:
        func_timeing_lock_icon_apply((f_timeing_t *)func_cb.f_cb);
        break;
    case FUNC_LANGUAGEING:
        func_languageing_lock_icon_apply((f_languageing_t *)func_cb.f_cb);
        break;
    case FUNC_VERINFO:
        func_verinfo_lock_icon_apply((f_verinfo_t *)func_cb.f_cb);
        break;
    case FUNC_RESERVATION:
#if !ELUNCHBOX_PANEL_EN
        func_res_lock_icon_apply((f_reservation_t *)func_cb.f_cb);
#endif
        break;
    default:
        break;
    }
#if ELUNCHBOX_PANEL_EN
    os_gui_draw_force();
#endif
}

static void func_key_lock_hint_show(key_lock_hint_mode_t mode)
{
    key_lock_hint_mode = mode;
    key_lock_hint_on = true;
    key_lock_hint_start = tick_get();
    func_key_lock_refresh_ui();
}

static void func_key_lock_hint_hide(void)
{
    if (!key_lock_hint_on) {
        return;
    }
    key_lock_hint_on = false;
    key_lock_hint_mode = KEY_LOCK_HINT_NONE;
    func_key_lock_refresh_ui();
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
    if (key_lock_hint_on) {
        func_key_lock_refresh_ui();
    }
}

bool func_key_lock_filter_tch(u8 tch)
{
    if (!key_lock_active) {
        return false;
    }
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

    if (key_lock_hint_on &&
        tick_check_expire(key_lock_hint_start, func_key_lock_hint_duration_ms())) {
        func_key_lock_hint_hide();
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
