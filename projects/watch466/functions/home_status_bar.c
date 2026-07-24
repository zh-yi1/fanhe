#include "include.h"
#include "home_status_bar.h"
#include "home_ui_shared.h"
#include "home_ui_ram.h"
#include "func_reservation.h"
#include "func_key_lock.h"

static struct {
    home_top_time_txt_t top_time;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
    compo_textbox_t *txt_marquee;
    u8 last_min;
    u8 last_sec;
} sb;

void home_status_bar_time_create(compo_form_t *frm)
{
    home_top_time_txt_create(frm, HOME_STATUS_BAR_ID_TOP_TIME);
    home_top_time_txt_bind(&sb.top_time, HOME_STATUS_BAR_ID_TOP_TIME);
}

void home_status_bar_bind(compo_picturebox_t *bt, compo_picturebox_t *lock,
                          compo_picturebox_t *bat, compo_textbox_t *marquee)
{
    sb.pic_bt        = bt;
    sb.pic_lock      = lock;
    sb.pic_bat       = bat;
    sb.txt_marquee   = marquee;
    sb.last_min      = 0xff;
    sb.last_sec      = 0xff;
}

void home_status_bar_enter(void)
{
    home_ui_shared_status_init();
    if (sb.pic_bt && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        home_ui_shared_status_refresh_bt(sb.pic_bt);
    }
    home_ui_shared_status_lock_bind(sb.pic_lock);
    home_ui_shared_status_bind_bat(sb.pic_bat);
    home_ui_shared_battery_attach_pic(sb.pic_bat);
}

void home_status_bar_tick(bool screen_locked)
{
    /* 时间（仅分钟/秒变化时刷新） */
    home_top_time_txt_tick(&sb.top_time, &sb.last_min, &sb.last_sec);

    /* 跑马灯（预约回 Home 时 RTC 秒不一定变，每帧检查） */
    if (sb.txt_marquee) {
        if (!func_reservation_is_waiting()) {
            compo_textbox_set_visible(sb.txt_marquee, false);
        } else {
            char buf[48];
            func_reservation_marquee_text(buf, sizeof(buf));
            compo_textbox_set(sb.txt_marquee, buf);
            compo_textbox_set_visible(sb.txt_marquee, true);
        }
    }

    /* 蓝牙图标由 home_ui_shared_ble_status_poll() 在 func_process 中按需刷新，
     * 不每帧调 refresh_bt 避免 GPU 重绑挤占 TE 间隔 → gui thread miss */

    /* 锁图标 */
    home_ui_shared_status_lock_set_visible(sb.pic_lock, screen_locked);
}

void home_status_bar_exit(void)
{
    home_ui_shared_battery_detach_pic();
}

void home_status_bar_wake(void)
{
    home_ui_shared_status_inited = false;
    home_ui_shared_status_lock_preloaded = false;

    home_ui_shared_status_init();
    home_ui_shared_bt_icon_wake_reset();
    if (sb.pic_bt && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        home_ui_shared_status_refresh_bt(sb.pic_bt);
    }
    home_ui_shared_status_lock_bind(sb.pic_lock);
    home_ui_shared_status_bind_bat(sb.pic_bat);
}

void home_status_bar_bring_front(void)
{
    home_top_time_txt_bring_front(&sb.top_time);
}

void home_status_bar_time_force(void)
{
    home_top_time_txt_force(&sb.top_time, &sb.last_min, &sb.last_sec);
}
