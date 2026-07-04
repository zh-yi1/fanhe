#include "include.h"
#include "func.h"
#include "func_new_home.h"
#include "home_top_time_txt.h"
#include "new_home_icon_res.h"
#include "new_home_tab_res.h"
#include "home_ui_shared.h"
#include "home_ui_gpu_detach.h"
#include "home_ui_ram.h"
#include "func_reservation.h"
#include "func_key_lock.h"

#if USER_PANEL_LED
#include "port_panel_led.h"
#endif

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

#ifndef UI_BUF_NEW_UI_NEW_LOGO_BIN
#ifndef UI_BUF_HOME_HEAT_BIN
#error "Run tools/gen_new_ui_icons.py then Output/bin/prebuild.bat"
#endif
#endif

#define NEW_HOME_LOGO_X                 (GUI_SCREEN_WIDTH / 2)
#define NEW_HOME_LOGO_Y                 80
#define NEW_HOME_STATUS_Y               20
#define NEW_HOME_STATUS_RIGHT_MARGIN    10
#define NEW_HOME_STATUS_GAP             6
#define NEW_HOME_RES_MARQUEE_Y          (HOME_TOP_TIME_Y + 5)
#define NEW_HOME_RES_MARQUEE_W          150
#define NEW_HOME_RES_MARQUEE_H          34

/* 底部 Tab 布局（79×88，Flash→home_ui_shared_icon_runtime，勿用堆/Flash 直读） */
#define NEW_HOME_TAB_Y                  ((s16)(GUI_SCREEN_HEIGHT - NEW_HOME_TAB_ICON_MAX_H / 2 - 25))
#define NEW_HOME_TAB_GAP                ((s16)((s32)(GUI_SCREEN_WIDTH - NEW_HOME_TAB_ICON_MAX_W * 3) / 4))
#define NEW_HOME_TAB_HEAT_X             (NEW_HOME_TAB_GAP + NEW_HOME_TAB_HEAT_W / 2)
#define NEW_HOME_TAB_MODE_X             (NEW_HOME_TAB_GAP * 2 + NEW_HOME_TAB_ICON_MAX_W + NEW_HOME_TAB_MODE_W / 2)
#define NEW_HOME_TAB_SETUP_X            (NEW_HOME_TAB_GAP * 3 + NEW_HOME_TAB_ICON_MAX_W * 2 + NEW_HOME_TAB_SETUP_W / 2)

enum {
    COMPO_ID_SHAPE_BG = 1,
    COMPO_ID_TXT_TOP_TIME,
    COMPO_ID_PIC_LOGO,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_LOCK,
    COMPO_ID_PIC_BAT,
    COMPO_ID_PIC_TAB_HEAT,
    COMPO_ID_PIC_TAB_MODE,
    COMPO_ID_PIC_TAB_SETUP,
    COMPO_ID_TXT_RES_MARQUEE,
};

static u8 new_home_logo_ram[NEW_HOME_LOGO_RAM_SIZE];
static bool new_home_logo_loaded;

#if ELUNCHBOX_PANEL_EN
static u8 new_home_gui_dirty = 1;
#endif

#if ELUNCHBOX_PANEL_EN
static void new_home_draw_now(void)
{
    home_gpu_wait_idle();
    os_gui_draw_force();
    home_gpu_wait_idle();
}

void func_home_gui_mark_dirty(void)
{
    new_home_gui_dirty = 1;
}

bool func_home_gui_need_refresh(void)
{
    if (!new_home_gui_dirty) {
        return false;
    }
    new_home_gui_dirty = 0;
    return true;
}
#endif

static u32 new_home_tab_flash_addr(u8 tab, bool selected)
{
    switch (tab) {
    case NEW_HOME_TAB_HEAT:
        return selected ? UI_BUF_NEW_UI_HEAT_1_BIN : UI_BUF_NEW_UI_HEAT_0_BIN;
    case NEW_HOME_TAB_MODE:
        return selected ? UI_BUF_NEW_UI_MODE_1_BIN : UI_BUF_NEW_UI_MODE_0_BIN;
    case NEW_HOME_TAB_SETUP:
        return selected ? UI_BUF_NEW_UI_SETUP_1_BIN : UI_BUF_NEW_UI_SETUP_0_BIN;
    default:
        return 0;
    }
}

static u16 new_home_tab_flash_len(u8 tab, bool selected)
{
    switch (tab) {
    case NEW_HOME_TAB_HEAT:
        return selected ? UI_LEN_NEW_UI_HEAT_1_BIN : UI_LEN_NEW_UI_HEAT_0_BIN;
    case NEW_HOME_TAB_MODE:
        return selected ? UI_LEN_NEW_UI_MODE_1_BIN : UI_LEN_NEW_UI_MODE_0_BIN;
    case NEW_HOME_TAB_SETUP:
        return selected ? UI_LEN_NEW_UI_SETUP_1_BIN : UI_LEN_NEW_UI_SETUP_0_BIN;
    default:
        return 0;
    }
}

static bool new_home_tab_gpu_ram_set(u8 *ram, u16 buf_size, u16 flash_len, compo_picturebox_t *pic,
                                     u16 draw_w, u16 draw_h)
{
    u16 need;

    if (ram == NULL || flash_len == 0 || flash_len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }
    if (!gui_set_ram_check(ram, __func__)) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    need = (u16)(8 + (u32)GET_LE16(&ram[4]) * GET_LE16(&ram[6]) * 2);
    if (need > flash_len || need > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    if (pic != NULL) {
        compo_picturebox_set_ram(pic, ram);
        compo_picturebox_set_size(pic, draw_w, draw_h);
        compo_picturebox_set_visible(pic, true);
    }
    return true;
}

static void new_home_tab_apply(f_new_home_t *f)
{
    compo_picturebox_t *tabs[3];
    u8 i;

    if (f == NULL) {
        return;
    }

    tabs[0] = f->pic_tab_heat;
    tabs[1] = f->pic_tab_mode;
    tabs[2] = f->pic_tab_setup;

    home_gpu_wait_idle();
    for (i = 0; i < NEW_HOME_TAB_CNT; i++) {
        u8 *ram = home_ui_shared_icon_runtime[i];
        u32 addr = new_home_tab_flash_addr(i, (i == f->cur_tab));
        u16 len = new_home_tab_flash_len(i, (i == f->cur_tab));

        if (tabs[i] == NULL || addr == 0 || len == 0 || len > NEW_HOME_TAB_RAM_SIZE) {
            continue;
        }
        os_spiflash_read(ram, addr, len);
        new_home_tab_gpu_ram_set(ram, NEW_HOME_TAB_RAM_SIZE, len, tabs[i],
                                 NEW_HOME_TAB_ICON_MAX_W, NEW_HOME_TAB_ICON_MAX_H);
    }
    home_gpu_wait_idle();
}

static void new_home_white_bg_create(compo_form_t *frm)
{
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, COMPO_ID_SHAPE_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_radius(bg, 0);
}

static void new_home_logo_load(void)
{
    if (new_home_logo_loaded) {
        return;
    }
    os_spiflash_read(new_home_logo_ram, UI_BUF_NEW_UI_NEW_LOGO_BIN, UI_LEN_NEW_UI_NEW_LOGO_BIN);
    new_home_logo_loaded = true;
}

static void new_home_logo_apply(f_new_home_t *f)
{
    if (f == NULL || f->pic_logo == NULL) {
        return;
    }
    new_home_logo_load();
    if (gui_set_ram_check(new_home_logo_ram, __func__)) {
        compo_picturebox_set_ram(f->pic_logo, new_home_logo_ram);
        compo_picturebox_set_pos(f->pic_logo, NEW_HOME_LOGO_X, NEW_HOME_LOGO_Y);
        compo_picturebox_set_size(f->pic_logo, NEW_HOME_LOGO_W, NEW_HOME_LOGO_H);
        compo_picturebox_set_visible(f->pic_logo, true);
    }
}

static void new_home_status_icons_apply(f_new_home_t *f)
{
    if (f == NULL) {
        return;
    }
    home_ui_shared_status_init();
    if (f->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f->pic_bt, NEW_HOME_BT_W, NEW_HOME_BT_H);
        compo_picturebox_set_visible(f->pic_bt, true);
    }
    if (f->pic_lock != NULL && gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
        compo_picturebox_set_ram(f->pic_lock, home_ui_shared_status_lock_ram);
        compo_picturebox_set_size(f->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    }
    home_ui_shared_status_bind_bat(f->pic_bat);
    func_home_lock_icon_apply(f);
}

static void new_home_lock_icon_prepare(f_new_home_t *f)
{
    if (f == NULL || f->pic_lock == NULL) {
        return;
    }
    home_ui_shared_status_lock_preload();
    if (gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
        compo_picturebox_set_ram(f->pic_lock, home_ui_shared_status_lock_ram);
        compo_picturebox_set_size(f->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
        compo_picturebox_set_visible(f->pic_lock, false);
    }
}

void func_home_lock_icon_apply(f_home_t *f_home)
{
    if (f_home == NULL || f_home->pic_lock == NULL) {
        return;
    }
    if (func_key_lock_show_status_icon(f_home->screen_locked)) {
        compo_picturebox_set_visible(f_home->pic_lock, true);
    } else {
        compo_picturebox_set_visible(f_home->pic_lock, false);
    }
}

static void new_home_bind_objects(f_new_home_t *f)
{
    f->pic_logo = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_LOGO);
    f->pic_bt = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_BT);
    f->pic_lock = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_LOCK);
    f->pic_bat = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_BAT);
    f->pic_tab_heat = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_TAB_HEAT);
    f->pic_tab_mode = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_TAB_MODE);
    f->pic_tab_setup = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_TAB_SETUP);
    f->txt_res_marquee = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_RES_MARQUEE);
    home_top_time_txt_bind(&f->top_time, COMPO_ID_TXT_TOP_TIME);
}

static void new_home_res_marquee_refresh(f_new_home_t *f)
{
    char buf[48];

    if (f->txt_res_marquee == NULL) {
        return;
    }
    if (!func_reservation_is_waiting()) {
        compo_textbox_set_visible(f->txt_res_marquee, false);
        return;
    }
    func_reservation_marquee_text(buf, sizeof(buf));
    compo_textbox_set(f->txt_res_marquee, buf);
    compo_textbox_set_visible(f->txt_res_marquee, true);
}

static void new_home_status_refresh(f_new_home_t *f)
{
    if (func_cb.sta != FUNC_HOME || sys_cb.flag_swithing || f == NULL) {
        return;
    }
    home_top_time_txt_tick(&f->top_time, &f->last_top_min, &f->last_top_sec);
    /* 预约提交回 Home 时 RTC 秒未必变化，跑马灯须每帧检查 */
    new_home_res_marquee_refresh(f);
}

#if ELUNCHBOX_PANEL_EN
void func_home_force_ui_refresh_after_wake(void)
{
    f_new_home_t *f;

    if (func_cb.sta != FUNC_HOME || func_cb.f_cb == NULL) {
        return;
    }
    f = (f_new_home_t *)func_cb.f_cb;
    new_home_bind_objects(f);

    new_home_logo_loaded = false;
    home_ui_shared_status_inited = false;
    home_ui_shared_status_lock_preloaded = false;

    new_home_status_icons_apply(f);
    new_home_logo_apply(f);
    new_home_tab_apply(f);

    {
        home_top_time_txt_force(&f->top_time, &f->last_top_min, &f->last_top_sec);
    }
    new_home_res_marquee_refresh(f);
    func_home_gui_mark_dirty();
    home_gpu_wait_idle();
    f->display_stage = 0;
}
#endif

compo_form_t *func_home_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_picturebox_t *pic;
    compo_textbox_t *txt;
    s16 bat_x;
    s16 lock_x;
    s16 bt_x;

    new_home_white_bg_create(frm);

    home_top_time_txt_create(frm, COMPO_ID_TXT_TOP_TIME);

    bat_x = (s16)(GUI_SCREEN_WIDTH - NEW_HOME_STATUS_RIGHT_MARGIN - NEW_HOME_BAT_W / 2);
    lock_x = (s16)(bat_x - NEW_HOME_BAT_W / 2 - NEW_HOME_STATUS_GAP - HOME_STATUS_LOCK_W / 2);
    bt_x = (s16)(lock_x - HOME_STATUS_LOCK_W / 2 - NEW_HOME_STATUS_GAP - NEW_HOME_BT_W / 2);

    pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_setid(pic, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, bt_x, NEW_HOME_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);

    pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_setid(pic, COMPO_ID_PIC_LOCK);
    compo_picturebox_set_pos(pic, lock_x, NEW_HOME_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);

    pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_setid(pic, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, bat_x, NEW_HOME_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);

    txt = compo_textbox_create(frm, 48);
    compo_setid(txt, COMPO_ID_TXT_RES_MARQUEE);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_BIN);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, NEW_HOME_RES_MARQUEE_Y,
                               NEW_HOME_RES_MARQUEE_W, NEW_HOME_RES_MARQUEE_H);
    compo_textbox_set_autoroll(txt, true);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_SROLL_CIRC);
    compo_textbox_set_visible(txt, false);

    pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_setid(pic, COMPO_ID_PIC_LOGO);
    compo_picturebox_set_pos(pic, NEW_HOME_LOGO_X, NEW_HOME_LOGO_Y);
    compo_picturebox_set_size(pic, NEW_HOME_LOGO_W, NEW_HOME_LOGO_H);
    compo_picturebox_set_visible(pic, false);

    /* 底部 Tab：HEAT、MODE、SETUP（79×88，Flash→home_ui_shared_icon_runtime） */
    pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_setid(pic, COMPO_ID_PIC_TAB_HEAT);
    compo_picturebox_set_pos(pic, NEW_HOME_TAB_HEAT_X, NEW_HOME_TAB_Y);
    compo_picturebox_set_size(pic, NEW_HOME_TAB_ICON_MAX_W, NEW_HOME_TAB_ICON_MAX_H);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_setid(pic, COMPO_ID_PIC_TAB_MODE);
    compo_picturebox_set_pos(pic, NEW_HOME_TAB_MODE_X, NEW_HOME_TAB_Y);
    compo_picturebox_set_size(pic, NEW_HOME_TAB_ICON_MAX_W, NEW_HOME_TAB_ICON_MAX_H);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_setid(pic, COMPO_ID_PIC_TAB_SETUP);
    compo_picturebox_set_pos(pic, NEW_HOME_TAB_SETUP_X, NEW_HOME_TAB_Y);
    compo_picturebox_set_size(pic, NEW_HOME_TAB_ICON_MAX_W, NEW_HOME_TAB_ICON_MAX_H);
    compo_picturebox_set_visible(pic, false);

    return frm;
}

#if ELUNCHBOX_PANEL_EN
u8 func_res_allow_switch;

void func_home_gpu_detach_before_leave(f_new_home_t *f)
{
    compo_picturebox_t *pics[7];
    u8 n = 0;
    u8 i;

    if (f == NULL) {
        return;
    }

    /* light detach：完整 detach 会长时间 wait_idle → gui thread miss */
    WDT_CLR();

    if (f->pic_bt) pics[n++] = f->pic_bt;
    if (f->pic_lock) pics[n++] = f->pic_lock;
    if (f->pic_bat) pics[n++] = f->pic_bat;
    if (f->pic_tab_heat) pics[n++] = f->pic_tab_heat;
    if (f->pic_tab_mode) pics[n++] = f->pic_tab_mode;
    if (f->pic_tab_setup) pics[n++] = f->pic_tab_setup;
    if (f->pic_logo) pics[n++] = f->pic_logo;

    for (i = 0; i < n; i++) {
        home_ui_gpu_pic_detach_light(pics[i]);
    }
    home_ui_shared_battery_detach_pic();
    WDT_CLR();
}
#endif

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
void func_home_drain_stale_key_msgs(void)
{
    msg_queue_detach(KU_NEXT, 0);
    msg_queue_detach(KU_MODE, 0);
    msg_queue_detach(KU_BACK, 0);
    msg_queue_detach(KU_PREV, 0);
    msg_queue_detach(KU_LEFT, 0);
    msg_queue_detach(KU_RIGHT, 0);
    msg_queue_detach(KU_VOL_UP, 0);
    msg_queue_detach(KU_VOL_DOWN, 0);
}

void func_home_switch_to_reservation(void)
{
    func_elunchbox_switch_to_reservation();
}

void func_home_mode_key(void)
{
    f_new_home_t *f;

    if (func_cb.sta != FUNC_HOME) {
        return;
    }
    f = (f_new_home_t *)func_cb.f_cb;
    if (f == NULL) {
        return;
    }
    if (f->screen_locked) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (f->display_stage != 0) {
        return;
    }
#endif
    /* 模式键循环切换 Tab */
    f->cur_tab++;
    if (f->cur_tab >= NEW_HOME_TAB_CNT) {
        f->cur_tab = 0;
    }
    new_home_tab_apply(f);
#if ELUNCHBOX_PANEL_EN
    new_home_draw_now();
    func_home_gui_mark_dirty();
#endif
}

#if ELUNCHBOX_PANEL_EN
static void func_home_pending_switch_exec(f_new_home_t *f)
{
    u8 target;
    u8 sta_before;

    if (f == NULL || f->pending_switch_sta == 0) {
        return;
    }
    target = f->pending_switch_sta;
    f->pending_switch_sta = 0;

    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
    WDT_CLR();
    /* 与 func_elunchbox_switch_to_heat / 预约页一致：FADE_OUT + GPU recycle，
     * 勿直接改 sta（func_exit 在 te_block 下 wait_idle 易卡死，子页首帧不刷新）。 */
    sta_before = func_cb.sta;
    func_switch_to(target, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
    printf("home confirm: switch sta %u -> %u (func_switch_to)\n", sta_before, func_cb.sta);
}
#endif

void func_home_confirm_key(void)
{
    f_new_home_t *f;
    u8 target;

    if (func_cb.sta != FUNC_HOME) {
        printf("home confirm: skip sta=%u\n", func_cb.sta);
        return;
    }
    f = (f_new_home_t *)func_cb.f_cb;
    if (f == NULL) {
        printf("home confirm: f_cb null\n");
        return;
    }
    if (f->screen_locked) {
        printf("home confirm: locked\n");
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (f->display_stage != 0) {
        printf("home confirm: display_stage=%u\n", f->display_stage);
        return;
    }
    if (f->pending_switch_sta != 0) {
        return;
    }
#endif
    switch (f->cur_tab) {
    case NEW_HOME_TAB_HEAT:
        target = FUNC_NEW_HEAT;
        break;
    case NEW_HOME_TAB_MODE:
        target = FUNC_NEW_MODE;
        break;
    case NEW_HOME_TAB_SETUP:
        target = FUNC_NEW_SETUP;
        break;
    default:
        printf("home confirm: bad tab=%u\n", f->cur_tab);
        return;
    }
    printf("home confirm: tab=%u target=%u (defer)\n", f->cur_tab, target);
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
    f->pending_switch_sta = target;
}
#endif /* USER_PT8028_KEY && ELUNCHBOX_PANEL_EN */

void new_home_pt8028_keys_process(f_new_home_t *f)
{
    u8 press_tch;

    if (f == NULL) {
        return;
    }
    func_home_drain_stale_key_msgs();
    press_tch = pt8028_take_press_tch();
    if (press_tch <= PT8028_KEY_TCH6 && press_tch != PT8028_KEY_TCH4) {
        elunchbox_user_activity_reset();
    }
    if (func_key_lock_filter_tch(press_tch)) {
        return;
    }
    if (press_tch == PT8028_KEY_TCH3) {
        func_home_mode_key();
    } else if (press_tch == PT8028_KEY_TCH1) {
        f->cur_tab = NEW_HOME_TAB_HEAT;
        func_home_confirm_key();
    } else if (press_tch == PT8028_KEY_TCH4) {
        func_home_confirm_key();
    }
}

void func_home_process(void)
{
    f_new_home_t *f = (f_new_home_t *)func_cb.f_cb;

#if ELUNCHBOX_PANEL_EN
    if (sys_cb.gui_sleep_sta || elunchbox_pwr_gui_off_is_on()) {
        func_process();
        return;
    }
#endif

#if ELUNCHBOX_PANEL_EN
    /* 首帧 UI 就绪后再扫键，避免 display_stage!=0 时确认键被静默丢弃 */
    if (f != NULL && f->display_stage != 0) {
        WDT_CLR();
        if (f->display_stage == 1) {
            new_home_status_icons_apply(f);
            new_home_logo_apply(f);
            new_home_tab_apply(f);
            home_top_time_txt_force(&f->top_time, &f->last_top_min, &f->last_top_sec);
            new_home_res_marquee_refresh(f);
            f->display_stage = 0;
            func_home_gui_mark_dirty();
            tft_bglight_force_on();
        }
        if (f->display_stage != 0) {
            func_process();
            return;
        }
    }
#endif

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_gpio_ensure_periodic();
    pt8028_key_scan();
    new_home_pt8028_keys_process(f);
#if USER_PANEL_LED
    panel_led_scan();
#endif
#endif

    if (func_cb.sta != FUNC_HOME || f == NULL) {
        return;
    }
    new_home_status_refresh(f);
    func_process();
#if ELUNCHBOX_PANEL_EN
    func_home_pending_switch_exec(f);
#endif
}

void func_home_message(size_msg_t msg)
{
    if (msg == NO_MSG || func_cb.sta != FUNC_HOME) {
        return;
    }
    if (func_key_lock_ku_blocked(msg)) {
        return;
    }
    switch (msg) {
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    case KU_MODE:
        func_home_mode_key();
        break;
#endif
    case KU_BACK:
        printf("home msg KU_BACK confirm\n");
        func_home_confirm_key();
        break;
    default:
        break;
    }
}

void func_home_enter(void)
{
    f_new_home_t *f;

#if ELUNCHBOX_PANEL_EN
    if (sys_cb.gui_sleep_sta) {
        gui_wakeup();
    }
    elunchbox_panel_boot_power_on();
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif

    func_cb.f_cb = func_zalloc(sizeof(f_new_home_t));
    func_cb.frm_main = func_home_form_create();
    f = (f_new_home_t *)func_cb.f_cb;
    f->screen_locked = false;
    f->cur_tab = NEW_HOME_TAB_HEAT;    /* 默认选中 Heat Tab */
    new_home_bind_objects(f);

#if ELUNCHBOX_PANEL_EN
    home_ui_shared_battery_attach_pic(f->pic_bat);
#endif
    new_home_lock_icon_prepare(f);

#if ELUNCHBOX_PANEL_EN
    f->display_stage = 1;
    f->pending_switch_sta = 0;
    func_home_gui_mark_dirty();
#else
    new_home_status_icons_apply(f);
    new_home_logo_apply(f);
    new_home_tab_apply(f);
    home_top_time_txt_force(&f->top_time, &f->last_top_min, &f->last_top_sec);
    new_home_res_marquee_refresh(f);
#endif

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(1);
#endif
#if USER_PANEL_LED
    panel_led_all_off();
#endif
}

void func_home_exit(void)
{
#if ELUNCHBOX_PANEL_EN
    f_new_home_t *f = (f_new_home_t *)func_cb.f_cb;

    if (f != NULL) {
        func_home_gpu_detach_before_leave(f);
    } else {
        home_ui_shared_battery_detach_pic();
    }
#else
    (void)func_cb.f_cb;
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_release_clear();
    pt8028_set_home_msg_block(0);
#endif
#if USER_PANEL_LED
    panel_led_all_off();
#endif
    func_cb.last = FUNC_HOME;
}

void func_home(void)
{
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(1);
    pt8028_release_clear();
#endif
    func_home_enter();
    while (func_cb.sta == FUNC_HOME) {
        func_home_process();
        func_home_message(msg_dequeue());
    }
    func_home_exit();
}
