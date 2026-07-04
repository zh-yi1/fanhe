#include "include.h"
#include "func.h"
#include "new_home_icon_res.h"
#include "home_ui_shared.h"
#include "home_ui_gpu_detach.h"
#include "home_ui_ram.h"
#include "func_key_lock.h"

#if ELUNCHBOX_PANEL_EN
extern volatile u8 elunchbox_te_block_flag;
#endif

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

#ifndef UI_BUF_0FONT_FONT_TEST_BIN
#error "UI_BUF_0FONT_FONT_TEST_BIN missing: add font_test.bin to ui.bin then Output/bin/prebuild.bat"
#endif
#define NEW_HEAT_FONT                       UI_BUF_0FONT_FONT_TEST_BIN

#ifndef VERINFO_VERSION_STR
#define VERINFO_VERSION_STR               "3E 610317-V1.0"
#endif

/*
 * 版本信息页 — 效果图 Ver. Info.
 *   顶栏：VER. INFO.（左对齐）+ 蓝牙/电量，无返回图标
 *   中部：版本号（浅蓝，居中）
 *   电源键：回设置页
 */
#define NEW_VERINFO_STATUS_Y               20
#define NEW_VERINFO_STATUS_RIGHT_MARGIN    10
#define NEW_VERINFO_STATUS_GAP             6
#define NEW_VERINFO_TITLE_X                10
#define NEW_VERINFO_TITLE_Y                5
#define NEW_VERINFO_TITLE_W                200
#define NEW_VERINFO_TITLE_H                36
#define NEW_VERINFO_VERSION_Y              GUI_SCREEN_CENTER_Y
#define NEW_VERINFO_VERSION_W              320
#define NEW_VERINFO_VERSION_H              40

#define NEW_VERINFO_COLOR_TITLE            COLOR_BLACK
#define NEW_VERINFO_COLOR_VERSION          0x2BF4
#define NEW_VERINFO_PAGE_BG                COLOR_WHITE

enum {
    COMPO_ID_SHAPE_BG = 1,
    COMPO_ID_TXT_TITLE,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_BAT,
    COMPO_ID_TXT_VERSION,
};

typedef struct {
    bool display_pending;
#if ELUNCHBOX_PANEL_EN
    bool key_ready;
    bool text_pending;
#endif
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_bat;
    compo_textbox_t *txt_title;
    compo_textbox_t *txt_version;
} f_new_verinfo_t;

static void new_verinfo_title_txt_prepare(compo_textbox_t *txt);
static void new_verinfo_title_txt_show(compo_textbox_t *txt);
static void new_verinfo_version_txt_prepare(compo_textbox_t *txt);
static void new_verinfo_version_txt_show(compo_textbox_t *txt);

#if ELUNCHBOX_PANEL_EN
static bool new_verinfo_font_ready;

static void new_verinfo_font_bind_txt(compo_textbox_t *txt)
{
    if (txt != NULL) {
        compo_textbox_set_font(txt, NEW_HEAT_FONT);
    }
}

static void new_verinfo_font_apply_once(f_new_verinfo_t *f)
{
    if (new_verinfo_font_ready || f == NULL) {
        return;
    }

    WDT_CLR();
    new_verinfo_font_bind_txt(f->txt_title);
    WDT_CLR();
    new_verinfo_font_bind_txt(f->txt_version);
    new_verinfo_font_ready = true;
}
#endif

#if ELUNCHBOX_PANEL_EN
static compo_picturebox_t *new_verinfo_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);

    pic->img = (void *)widget_image_create(frm->page_body, 0);
    pic->radix = 1;
    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

static void new_verinfo_gpu_detach_before_leave(f_new_verinfo_t *f)
{
    if (f == NULL) {
        return;
    }
    WDT_CLR();
    home_ui_gpu_pic_detach_light(f->pic_bt);
    home_ui_gpu_pic_detach_light(f->pic_bat);
    home_ui_shared_battery_detach_pic();
    WDT_CLR();
}
#else
static compo_picturebox_t *new_verinfo_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);

    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}
#endif

static compo_textbox_t *new_verinfo_txt_create(compo_form_t *frm, u16 id, u16 buf_size,
                                                s16 x, s16 y, u16 w, u16 h, u16 color,
                                                bool center)
{
    compo_textbox_t *txt = compo_textbox_create(frm, buf_size);

    compo_setid(txt, id);
    compo_textbox_set_wholewrap(txt, false);
#if ELUNCHBOX_PANEL_EN
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_visible(txt, false);
#else
    compo_textbox_set_font(txt, NEW_HEAT_FONT);
#endif
    compo_textbox_set_align_center(txt, center);
    compo_textbox_set_pos(txt, x, y);
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set_location(txt, x, y, w, h);
    return txt;
}

static void new_verinfo_title_txt_prepare(compo_textbox_t *txt)
{
    widget_text_t *widget;

    if (txt == NULL) {
        return;
    }
    widget = txt->txt;
    compo_textbox_set_align_center(txt, false);
    widget_set_align_center(widget, false);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_location(txt, NEW_VERINFO_TITLE_X, NEW_VERINFO_TITLE_Y,
                               NEW_VERINFO_TITLE_W, NEW_VERINFO_TITLE_H);
    compo_textbox_set_forecolor(txt, NEW_VERINFO_COLOR_TITLE);
    compo_textbox_set_visible(txt, false);
}

static void new_verinfo_title_txt_show(compo_textbox_t *txt)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL) {
        return;
    }
    new_verinfo_title_txt_prepare(txt);
    widget = txt->txt;
    compo_textbox_set(txt, "VER. INFO.");
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);

    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
    compo_textbox_set_visible(txt, true);
}

static void new_verinfo_version_txt_prepare(compo_textbox_t *txt)
{
    widget_text_t *widget;

    if (txt == NULL) {
        return;
    }
    widget = txt->txt;
    compo_textbox_set_align_center(txt, true);
    widget_set_align_center(widget, true);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, NEW_VERINFO_VERSION_Y,
                               NEW_VERINFO_VERSION_W, NEW_VERINFO_VERSION_H);
    compo_textbox_set_forecolor(txt, NEW_VERINFO_COLOR_VERSION);
    compo_textbox_set_visible(txt, false);
}

static void new_verinfo_version_txt_show(compo_textbox_t *txt)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL) {
        return;
    }
    new_verinfo_version_txt_prepare(txt);
    widget = txt->txt;
    compo_textbox_set(txt, VERINFO_VERSION_STR);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);

    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
    compo_textbox_set_visible(txt, true);
}

static void new_verinfo_bg_create(compo_form_t *frm)
{
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, COMPO_ID_SHAPE_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(bg, NEW_VERINFO_PAGE_BG);
    compo_shape_set_radius(bg, 0);
}

static void new_verinfo_bind_objects(f_new_verinfo_t *f)
{
    if (f == NULL) {
        return;
    }
    f->txt_title = compo_getobj_byid(COMPO_ID_TXT_TITLE);
    f->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);
    f->txt_version = compo_getobj_byid(COMPO_ID_TXT_VERSION);
}

static void new_verinfo_status_refresh(f_new_verinfo_t *f)
{
    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (f->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f->pic_bt, NEW_HOME_BT_W, NEW_HOME_BT_H);
        compo_picturebox_set_visible(f->pic_bt, true);
    }
    if (f->pic_bat != NULL) {
        home_ui_shared_battery_attach_pic(f->pic_bat);
    }
#else
    home_ui_shared_status_init();
    if (f->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f->pic_bt, NEW_HOME_BT_W, NEW_HOME_BT_H);
        compo_picturebox_set_visible(f->pic_bt, true);
    }
    home_ui_shared_status_bind_bat(f->pic_bat);
#endif
}

static void new_verinfo_ui_apply(f_new_verinfo_t *f)
{
    if (f == NULL) {
        return;
    }
    new_verinfo_status_refresh(f);
    new_verinfo_title_txt_show(f->txt_title);
    new_verinfo_version_txt_show(f->txt_version);
}

#if ELUNCHBOX_PANEL_EN
static void new_verinfo_icons_apply(f_new_verinfo_t *f)
{
    if (f == NULL) {
        return;
    }
    new_verinfo_status_refresh(f);
}

static void new_verinfo_text_apply(f_new_verinfo_t *f)
{
    if (f == NULL) {
        return;
    }
    new_verinfo_font_apply_once(f);
    new_verinfo_title_txt_show(f->txt_title);
    new_verinfo_version_txt_show(f->txt_version);
}
#endif

static void new_verinfo_power_key(void)
{
    if (sys_cb.flag_swithing) {
        return;
    }
    func_switch_to(FUNC_NEW_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

compo_form_t *func_new_verinfo_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    compo_picturebox_t *pic;
    s16 bat_x;
    s16 bt_x;

    new_verinfo_bg_create(frm);

    txt = new_verinfo_txt_create(frm, COMPO_ID_TXT_TITLE, 16,
                                 NEW_VERINFO_TITLE_X, NEW_VERINFO_TITLE_Y,
                                 NEW_VERINFO_TITLE_W, NEW_VERINFO_TITLE_H,
                                 NEW_VERINFO_COLOR_TITLE, false);
    new_verinfo_title_txt_prepare(txt);

    txt = new_verinfo_txt_create(frm, COMPO_ID_TXT_VERSION, 24,
                                 GUI_SCREEN_CENTER_X, NEW_VERINFO_VERSION_Y,
                                 NEW_VERINFO_VERSION_W, NEW_VERINFO_VERSION_H,
                                 NEW_VERINFO_COLOR_VERSION, true);
    new_verinfo_version_txt_prepare(txt);

    bat_x = (s16)(GUI_SCREEN_WIDTH - NEW_VERINFO_STATUS_RIGHT_MARGIN - NEW_HOME_BAT_W / 2);
    bt_x = (s16)(bat_x - NEW_HOME_BAT_W / 2 - NEW_VERINFO_STATUS_GAP - NEW_HOME_BT_W / 2);

    pic = new_verinfo_pic_create_hidden(frm, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, bt_x, NEW_VERINFO_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);

    pic = new_verinfo_pic_create_hidden(frm, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, bat_x, NEW_VERINFO_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);

    return frm;
}

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static void new_verinfo_pt8028_keys_process(f_new_verinfo_t *f)
{
    u8 press_tch;

    if (f == NULL || !f->key_ready) {
        return;
    }
    press_tch = pt8028_take_press_tch();
    if (press_tch == 0xff) {
        return;
    }
    if (press_tch <= PT8028_KEY_TCH6) {
        elunchbox_user_activity_reset();
    }
    if (func_key_lock_filter_tch(press_tch)) {
        return;
    }
    if (press_tch == PT8028_KEY_TCH4 || press_tch == PT8028_KEY_TCH5) {
        new_verinfo_power_key();
    }
}

static void new_verinfo_keys_poll(f_new_verinfo_t *f)
{
    if (f == NULL || !f->key_ready) {
        return;
    }
    new_verinfo_pt8028_keys_process(f);
}
#endif

static void func_new_verinfo_message(size_msg_t msg)
{
    f_new_verinfo_t *f = (f_new_verinfo_t *)func_cb.f_cb;

    if (sys_cb.flag_swithing) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (f != NULL && !f->key_ready) {
        return;
    }
#endif
    if (func_key_lock_ku_blocked(msg)) {
        return;
    }
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    switch (msg) {
    case KU_BACK:
    case KEY_RIGHT | KEY_SHORT_UP:
        return;
    default:
        break;
    }
#endif
    switch (msg) {
    case KU_BACK:
    case KEY_RIGHT | KEY_SHORT_UP:
        new_verinfo_power_key();
        break;
    default:
        func_message(msg);
        break;
    }
}

static void func_new_verinfo_process(void)
{
    f_new_verinfo_t *f = (f_new_verinfo_t *)func_cb.f_cb;

    if (f == NULL) {
        func_process();
        return;
    }

#if ELUNCHBOX_PANEL_EN
    if (!f->key_ready) {
        if (f->display_pending) {
            home_gpu_wait_idle();
            WDT_CLR();
            new_verinfo_icons_apply(f);
            f->display_pending = false;
            f->text_pending = true;
        } else if (f->text_pending) {
            WDT_CLR();
            new_verinfo_text_apply(f);
            f->text_pending = false;
        }
        func_process();
        func_home_drain_stale_key_msgs();
        pt8028_release_clear();
        (void)pt8028_take_press_tch();
        if (!f->display_pending && !f->text_pending) {
            f->key_ready = true;
        }
        return;
    }
#endif

    if (f->display_pending) {
        new_verinfo_ui_apply(f);
        f->display_pending = false;
    }

    func_process();
#if ELUNCHBOX_PANEL_EN
    if (!elunchbox_ui_is_live()) {
        return;
    }
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    new_verinfo_keys_poll(f);
#endif
}

void func_new_verinfo_enter(void)
{
    f_new_verinfo_t *f;

    printf("func_new_verinfo_enter\n");

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(1);
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif

#if ELUNCHBOX_PANEL_EN
    WDT_CLR();
#endif

    msg_queue_detach(KU_BACK, 0);
    msg_queue_detach(KU_RIGHT, 0);

    func_cb.f_cb = func_zalloc(sizeof(f_new_verinfo_t));
    f = (f_new_verinfo_t *)func_cb.f_cb;
#if ELUNCHBOX_PANEL_EN
    new_verinfo_font_ready = false;
    f->key_ready = false;
    f->text_pending = false;
#endif
    f->display_pending = true;

    func_cb.frm_main = func_new_verinfo_form_create();
    new_verinfo_bind_objects(f);

#if ELUNCHBOX_PANEL_EN
    home_ui_shared_status_init();
    WDT_CLR();
    home_gpu_wait_idle();
    WDT_CLR();
    elunchbox_te_block_flag = 0;
    tft_bglight_force_on();
#else
    new_verinfo_ui_apply(f);
    f->display_pending = false;
#endif
}

void func_new_verinfo_exit(void)
{
#if ELUNCHBOX_PANEL_EN
    f_new_verinfo_t *f = (f_new_verinfo_t *)func_cb.f_cb;

    if (f != NULL) {
        new_verinfo_gpu_detach_before_leave(f);
    } else {
        home_ui_shared_battery_detach_pic();
    }
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(0);
    pt8028_release_clear();
#endif
    func_cb.last = FUNC_NEW_VERINFO;
    printf("func_new_verinfo_exit\n");
}

void func_new_verinfo(void)
{
    func_new_verinfo_enter();
    while (func_cb.sta == FUNC_NEW_VERINFO) {
        func_new_verinfo_process();
        func_new_verinfo_message(msg_dequeue());
    }
    func_new_verinfo_exit();
}
