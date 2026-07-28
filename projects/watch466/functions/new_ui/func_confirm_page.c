#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"
#include "general_ui.h"
#include "new_time_res.h"
#include "lang.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * 开盖确认弹窗 — 效果图 CONFIRM
 *   底层：首页内容（半透明显示）
 *   中层：半透明遮罩
 *   上层：白色圆角对话框 + 多语言文案 + NO/YES（bj2）
 *
 * 按键：UP/DOWN 切换 NO/YES；CONFIRM 确认后回首页；BACK 回首页
 */
#define CONFIRM_DIM_ALPHA               160

#define CONFIRM_PANEL_W                 280
#define CONFIRM_PANEL_H                 160
#define CONFIRM_PANEL_Y                 GUI_SCREEN_CENTER_Y
#define CONFIRM_PANEL_RADIUS            16

#define CONFIRM_MSG_W                   240
#define CONFIRM_MSG_H                   48
#define CONFIRM_MSG_Y                   ((s16)(CONFIRM_PANEL_Y - 38))

#define CONFIRM_SUB_W                   240
#define CONFIRM_SUB_H                   28
#define CONFIRM_SUB_Y                   ((s16)(CONFIRM_PANEL_Y + 10))

#define CONFIRM_BTN_DIST                130
#define CONFIRM_BTN_Y                   ((s16)(CONFIRM_PANEL_Y + CONFIRM_PANEL_H / 2 \
                                               - NEW_TIME_BTN_H / 2 - 14))
#define CONFIRM_BTN_NO_X                ((s16)(GUI_SCREEN_CENTER_X - CONFIRM_BTN_DIST / 2))
#define CONFIRM_BTN_YES_X               ((s16)(GUI_SCREEN_CENTER_X + CONFIRM_BTN_DIST / 2))
#define CONFIRM_BTN_LABEL_Y             ((s16)(CONFIRM_BTN_Y - 4))

#define CONFIRM_COLOR_MSG               0x0AD8
#define CONFIRM_COLOR_SUB               0x9CD3
#define CONFIRM_COLOR_ON                COLOR_WHITE
#define CONFIRM_COLOR_OFF               COLOR_BLACK

#ifndef UI_BUF_0FONT_FONT_TEST_10_BIN
#define CONFIRM_MSG_FONT                UI_BUF_0FONT_FONT_TEST_14_BIN
#else
#define CONFIRM_MSG_FONT                UI_BUF_0FONT_FONT_TEST_10_BIN
#endif

enum {
    CONFIRM_SEL_NO = 0,
    CONFIRM_SEL_YES,
};

typedef struct {
    u8 sel; /* 0=NO, 1=YES */
    general_status_bar_t sb;
    compo_picturebox_t *pic_no_bg;
    compo_picturebox_t *pic_yes_bg;
    compo_textbox_t *txt_msg;
    compo_textbox_t *txt_sub;
    compo_textbox_t *txt_no;
    compo_textbox_t *txt_yes;
} f_confirm_page_t;

static void confirm_btn_label_show(compo_textbox_t *txt, s16 x, s16 y,
                                   const char *label, u16 color)
{
    if (txt == NULL) {
        return;
    }
    compo_textbox_set_location(txt, x, y, NEW_TIME_BTN_W, NEW_TIME_BTN_H);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set(txt, label);
    compo_textbox_set_visible(txt, true);
}

static void confirm_msg_show(compo_textbox_t *txt, const char *str)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL || str == NULL) {
        return;
    }
    widget = txt->txt;
    compo_textbox_set_multiline(txt, true);
    compo_textbox_set_align_center(txt, true);
    if (widget != NULL) {
        widget_set_align_center(widget, true);
        widget_text_set_ellipsis(widget, false);
    }
    compo_textbox_set_wholewrap(txt, true);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, CONFIRM_MSG_Y,
                               CONFIRM_MSG_W, CONFIRM_MSG_H);
    compo_textbox_set_font(txt, CONFIRM_MSG_FONT);
    compo_textbox_set_forecolor(txt, CONFIRM_COLOR_MSG);
    compo_textbox_set(txt, str);
    if (widget != NULL) {
        rect = widget_get_location(widget);
        text_area = widget_text_get_area(widget);
        if (rect.hei > text_area.hei) {
            widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
        } else {
            widget_text_set_client(widget, 0, 0);
        }
    }
    compo_textbox_set_visible(txt, true);
}

static void confirm_sub_show(compo_textbox_t *txt, const char *str)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL || str == NULL) {
        return;
    }
    widget = txt->txt;
    compo_textbox_set_align_center(txt, true);
    if (widget != NULL) {
        widget_set_align_center(widget, true);
        widget_text_set_ellipsis(widget, false);
    }
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, CONFIRM_SUB_Y,
                               CONFIRM_SUB_W, CONFIRM_SUB_H);
    compo_textbox_set_font(txt, CONFIRM_MSG_FONT);
    compo_textbox_set_forecolor(txt, CONFIRM_COLOR_SUB);
    compo_textbox_set(txt, str);
    if (widget != NULL) {
        rect = widget_get_location(widget);
        text_area = widget_text_get_area(widget);
        if (rect.hei > text_area.hei) {
            widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
        } else {
            widget_text_set_client(widget, 0, 0);
        }
    }
    compo_textbox_set_visible(txt, true);
}

static void confirm_update_display(void)
{
    f_confirm_page_t *inf = (f_confirm_page_t *)func_cb.f_cb;
    bool no_sel;
    bool yes_sel;

    if (inf == NULL) {
        return;
    }

    no_sel  = (inf->sel == CONFIRM_SEL_NO);
    yes_sel = (inf->sel == CONFIRM_SEL_YES);

    if (inf->pic_no_bg != NULL) {
        compo_picturebox_set(inf->pic_no_bg,
                             no_sel ? UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN
                                    : UI_BUF_NEW_UI_NEW_GRAY_BJ2_BIN);
        compo_picturebox_set_size(inf->pic_no_bg, NEW_TIME_BTN_W, NEW_TIME_BTN_H);
    }
    if (inf->pic_yes_bg != NULL) {
        compo_picturebox_set(inf->pic_yes_bg,
                             yes_sel ? UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN
                                     : UI_BUF_NEW_UI_NEW_GRAY_BJ2_BIN);
        compo_picturebox_set_size(inf->pic_yes_bg, NEW_TIME_BTN_W, NEW_TIME_BTN_H);
    }

    confirm_msg_show(inf->txt_msg, i18n[STR_CONFIRM_LID_OPEN]);
    confirm_sub_show(inf->txt_sub, i18n[STR_CONFIRM_CONTINUE_HEAT]);
    confirm_btn_label_show(inf->txt_no, CONFIRM_BTN_NO_X, CONFIRM_BTN_LABEL_Y,
                           i18n[STR_BTN_NO], no_sel ? CONFIRM_COLOR_ON : CONFIRM_COLOR_OFF);
    confirm_btn_label_show(inf->txt_yes, CONFIRM_BTN_YES_X, CONFIRM_BTN_LABEL_Y,
                           i18n[STR_BTN_YES], yes_sel ? CONFIRM_COLOR_ON : CONFIRM_COLOR_OFF);
}

/** 底层首页内容（弹窗外可见） */
static void confirm_home_backdrop_create(compo_form_t *frm)
{
    compo_shape_t *bg;
    compo_picturebox_t *pic;
    compo_textbox_t *txt;

    bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_LOGO_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 50);

    pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_HEAT_0_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X - 100, GUI_SCREEN_CENTER_Y + 45);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X - 98, GUI_SCREEN_CENTER_Y + 60, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(txt, COLOR_BLUE);
    compo_textbox_set(txt, i18n[STR_HEAT]);

    pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_MODE_0_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 45);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 60, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(txt, COLOR_BLUE);
    compo_textbox_set(txt, i18n[STR_MODE]);

    pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_SETUP_0_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X + 98, GUI_SCREEN_CENTER_Y + 45);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X + 98, GUI_SCREEN_CENTER_Y + 60, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(txt, COLOR_BLUE);
    compo_textbox_set(txt, i18n[STR_SETUP]);
}

compo_form_t *func_confirm_page_form_create(void)
{
    f_confirm_page_t *inf = (f_confirm_page_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_shape_t *dim;
    compo_shape_t *panel;

    widget_set_visible(frm->icon, false);

    /* 底层首页 + 顶部状态栏 */
    confirm_home_backdrop_create(frm);
    general_status_bar_create(frm, &inf->sb, NULL);

    /* 半透明遮罩：周围透出首页，中间对话框区域不透明白底 */
    dim = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(dim, COLOR_WHITE);
    compo_shape_set_location(dim, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_alpha(dim, CONFIRM_DIM_ALPHA);

    panel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(panel, COLOR_WHITE);
    compo_shape_set_location(panel, GUI_SCREEN_CENTER_X, CONFIRM_PANEL_Y,
                             CONFIRM_PANEL_W, CONFIRM_PANEL_H);
    compo_shape_set_radius(panel, CONFIRM_PANEL_RADIUS);

    /* 主/副标题 */
    inf->txt_msg = compo_textbox_create(frm, 80);
    compo_textbox_set_font(inf->txt_msg, CONFIRM_MSG_FONT);
    inf->txt_sub = compo_textbox_create(frm, 64);
    compo_textbox_set_font(inf->txt_sub, CONFIRM_MSG_FONT);

    /* NO / YES */
    inf->pic_no_bg = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_GRAY_BJ2_BIN);
    compo_picturebox_set_pos(inf->pic_no_bg, CONFIRM_BTN_NO_X, CONFIRM_BTN_Y);
    compo_picturebox_set_size(inf->pic_no_bg, NEW_TIME_BTN_W, NEW_TIME_BTN_H);

    inf->pic_yes_bg = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN);
    compo_picturebox_set_pos(inf->pic_yes_bg, CONFIRM_BTN_YES_X, CONFIRM_BTN_Y);
    compo_picturebox_set_size(inf->pic_yes_bg, NEW_TIME_BTN_W, NEW_TIME_BTN_H);

    inf->txt_no = compo_textbox_create(frm, 8);
    compo_textbox_set_font(inf->txt_no, UI_BUF_0FONT_FONT_TEST_14_BIN);
    inf->txt_yes = compo_textbox_create(frm, 8);
    compo_textbox_set_font(inf->txt_yes, UI_BUF_0FONT_FONT_TEST_14_BIN);

    inf->sel = CONFIRM_SEL_YES;
    confirm_update_display();

    tft_bglight_force_on();
    return frm;
}

static void confirm_finish(f_confirm_page_t *inf)
{
    (void)inf;
    func_cb.sta = FUNC_HOME;
}

static void func_confirm_page_handle_keys(void)
{
    f_confirm_page_t *inf = (f_confirm_page_t *)func_cb.f_cb;
    func_key_event_t evt;

    while (func_key_get_event(&evt)) {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key) {
        case FUNC_KEY_UP:
        case FUNC_KEY_DOWN:
            inf->sel = (u8)((inf->sel == CONFIRM_SEL_NO)
                            ? CONFIRM_SEL_YES : CONFIRM_SEL_NO);
            confirm_update_display();
            break;

        case FUNC_KEY_CONFIRM:
            confirm_finish(inf);
            break;

        case FUNC_KEY_BACK:
            func_cb.sta = FUNC_HOME;
            break;

        default:
            break;
        }
    }
}

static void func_confirm_page_process(void)
{
    f_confirm_page_t *inf = (f_confirm_page_t *)func_cb.f_cb;

    func_key_poll();
    func_confirm_page_handle_keys();

    func_key_lock_poll();

    if (func_key_lock_gui_dirty()) {
        if (func_key_lock_overlay_visible()) {
            func_lock_page_show(func_key_lock_overlay_is_unlock());
        } else {
            func_lock_page_hide();
        }
    }

    func_process();
    general_status_bar_tick(&inf->sb);
}

static void func_confirm_page_message(size_msg_t msg)
{
    func_message(msg);
}

void func_confirm_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_confirm_page_t));
    func_key_reset();
    func_cb.frm_main = func_confirm_page_form_create();
    general_status_bar_attach(&((f_confirm_page_t *)func_cb.f_cb)->sb);
}

void func_confirm_page_exit(void)
{
    func_key_flush();
    general_status_bar_detach();
    func_cb.last = FUNC_NEW_CONFIRM;
}

void func_confirm_page(void)
{
    printf("%s\n", __func__);
    func_confirm_page_enter();
    while (func_cb.sta == FUNC_NEW_CONFIRM) {
        func_confirm_page_process();
        func_confirm_page_message(msg_dequeue());
    }
    func_confirm_page_exit();
}
