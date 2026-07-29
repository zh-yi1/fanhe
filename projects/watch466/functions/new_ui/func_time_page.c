#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"
#include "general_title_ui.h"
#include "lang.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * 时间设置页 — 效果图 TIME
 *   顶栏：i18n[STR_TIME] + 蓝牙/电量
 *   时/分框：new_blue_bj1 / new_gray_bj1
 *   数字：选中白字 new_time_w0..9，未选黑字 new_time_b0..9
 *   上下箭头：new_up / new_down
 *   底部 NO/YES：new_gray_bj2 / new_blue_bj2
 *
 * 按键：UP/DOWN 调值或切换 NO/YES；CONFIRM 推进焦点 / 确认；BACK 回设置
 */
#define TIME_PAGE_BG                    0xEF5D
#define TIME_PAGE_PANEL_Y               142
#define TIME_PAGE_PANEL_W               280
#define TIME_PAGE_PANEL_H               200
#define TIME_PAGE_PANEL_RADIUS          16

#define TIME_PAGE_COL_SPAN              53
/* 与 func_new_timeing 一致：内容整体下移（BOX/箭头/NO·YES） */
#define TIME_PAGE_CONTENT_OFFSET_Y      ((s16)(-16))
#define TIME_PAGE_BOX_Y                 ((s16)(TIME_PAGE_PANEL_Y + TIME_PAGE_CONTENT_OFFSET_Y)) /* 126 */
#define TIME_PAGE_HOUR_X                ((s16)(GUI_SCREEN_CENTER_X - TIME_PAGE_COL_SPAN))
#define TIME_PAGE_MIN_X                 ((s16)(GUI_SCREEN_CENTER_X + TIME_PAGE_COL_SPAN))

/* 时钟数字：右上角锚点（与 func_new_timeing 一致）
 * 320×240：H10(95,110) H1(130,111) M10(205,110) M1(240,111) */
#define TIME_PAGE_CLOCK_TR_H10_X        ((s16)((s32)95 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define TIME_PAGE_CLOCK_TR_H10_Y        ((s16)((s32)110 * GUI_SCREEN_HEIGHT / HEAT_LAYOUT_REF_H))
#define TIME_PAGE_CLOCK_TR_H1_X         ((s16)((s32)125 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define TIME_PAGE_CLOCK_TR_H1_Y         ((s16)((s32)111 * GUI_SCREEN_HEIGHT / HEAT_LAYOUT_REF_H))
#define TIME_PAGE_CLOCK_TR_M10_X        ((s16)((s32)205 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define TIME_PAGE_CLOCK_TR_M10_Y        ((s16)((s32)110 * GUI_SCREEN_HEIGHT / HEAT_LAYOUT_REF_H))
#define TIME_PAGE_CLOCK_TR_M1_X         ((s16)((s32)235 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define TIME_PAGE_CLOCK_TR_M1_Y         ((s16)((s32)111 * GUI_SCREEN_HEIGHT / HEAT_LAYOUT_REF_H))

#define TIME_PAGE_UNIT_Y                ((s16)(TIME_PAGE_BOX_Y + 10))
#define TIME_PAGE_ARROW_UP_Y            ((s16)(TIME_PAGE_BOX_Y - 47))
#define TIME_PAGE_ARROW_DOWN_Y          ((s16)(TIME_PAGE_BOX_Y + 47))

#define TIME_PAGE_BTN_DIST              130
#define TIME_PAGE_BTN_Y                 ((s16)(TIME_PAGE_PANEL_Y + TIME_PAGE_PANEL_H / 2 \
                                               - NEW_TIME_BTN_H / 2 - 12 \
                                               + TIME_PAGE_CONTENT_OFFSET_Y))
#define TIME_PAGE_BTN_NO_X              ((s16)(GUI_SCREEN_CENTER_X - TIME_PAGE_BTN_DIST / 2))
#define TIME_PAGE_BTN_YES_X             ((s16)(GUI_SCREEN_CENTER_X + TIME_PAGE_BTN_DIST / 2))
#define TIME_PAGE_BTN_LABEL_Y           ((s16)(TIME_PAGE_BTN_Y - 1))

#define TIME_PAGE_COLOR_ON              COLOR_WHITE
#define TIME_PAGE_COLOR_OFF             COLOR_BLACK

/* ---- 布局参考尺寸（原 new_time_res.h / ui_layout_anchor.h） ---- */
#define HEAT_LAYOUT_REF_W               240
#define HEAT_LAYOUT_REF_H               284
#define NEW_TIME_BTN_W                  110
#define NEW_TIME_BTN_H                  44
#define NEW_TIME_BOX_W                  60
#define NEW_TIME_BOX_H                  64
#define NEW_TIME_ARROW_W                20
#define NEW_TIME_ARROW_H                16
#define TIME_PAGE_COLOR_UNIT_SEL        COLOR_WHITE
#define TIME_PAGE_COLOR_UNIT_NOR        COLOR_BLACK

enum {
    TIME_PAGE_FOCUS_HOUR = 0,
    TIME_PAGE_FOCUS_MIN,
    TIME_PAGE_FOCUS_BOTTOM,
};

typedef struct
{
    u8 hour;
    u8 min;
    u8 focus;       /* HOUR / MIN / BOTTOM */
    u8 bottom_sel;  /* 0=NO, 1=YES */
    general_title_bar_t tb;
    compo_picturebox_t *pic_hour_bg;
    compo_picturebox_t *pic_min_bg;
    compo_picturebox_t *pic_hour_up;
    compo_picturebox_t *pic_hour_down;
    compo_picturebox_t *pic_min_up;
    compo_picturebox_t *pic_min_down;
    compo_picturebox_t *pic_h10;
    compo_picturebox_t *pic_h1;
    compo_picturebox_t *pic_m10;
    compo_picturebox_t *pic_m1;
    compo_picturebox_t *pic_no_bg;
    compo_picturebox_t *pic_yes_bg;
    compo_textbox_t *txt_colon;
    compo_textbox_t *txt_unit_h;
    compo_textbox_t *txt_unit_min;
    compo_textbox_t *txt_no;
    compo_textbox_t *txt_yes;
} f_time_page_t;

static const u32 g_digit_w_addr[10] = {
    UI_BUF_NEW_UI_NEW_TIME_W0_BIN, UI_BUF_NEW_UI_NEW_TIME_W1_BIN,
    UI_BUF_NEW_UI_NEW_TIME_W2_BIN, UI_BUF_NEW_UI_NEW_TIME_W3_BIN,
    UI_BUF_NEW_UI_NEW_TIME_W4_BIN, UI_BUF_NEW_UI_NEW_TIME_W5_BIN,
    UI_BUF_NEW_UI_NEW_TIME_W6_BIN, UI_BUF_NEW_UI_NEW_TIME_W7_BIN,
    UI_BUF_NEW_UI_NEW_TIME_W8_BIN, UI_BUF_NEW_UI_NEW_TIME_W9_BIN,
};

static const u32 g_digit_b_addr[10] = {
    UI_BUF_NEW_UI_NEW_TIME_B0_BIN, UI_BUF_NEW_UI_NEW_TIME_B1_BIN,
    UI_BUF_NEW_UI_NEW_TIME_B2_BIN, UI_BUF_NEW_UI_NEW_TIME_B3_BIN,
    UI_BUF_NEW_UI_NEW_TIME_B4_BIN, UI_BUF_NEW_UI_NEW_TIME_B5_BIN,
    UI_BUF_NEW_UI_NEW_TIME_B6_BIN, UI_BUF_NEW_UI_NEW_TIME_B7_BIN,
    UI_BUF_NEW_UI_NEW_TIME_B8_BIN, UI_BUF_NEW_UI_NEW_TIME_B9_BIN,
};

/* 与 bin 原图一致，勿拉伸（尤其 1 为 5×30） */
static const u16 g_digit_native_w[10] = {
    19, 5, 19, 19, 19, 20, 19, 19, 19, 20,
};
static const u16 g_digit_native_h[10] = {
    32, 30, 32, 32, 30, 32, 32, 31, 32, 32,
};

/* 设置数字图片并定位（右上角锚点） */
static void time_page_digit_update(compo_picturebox_t *pic, u8 digit, bool white,
                                   s16 tr_x, s16 tr_y)
{
    u16 w, h;

    if (pic == NULL) {
        return;
    }
    if (digit > 9) {
        digit = 9;
    }
    w = g_digit_native_w[digit];
    h = g_digit_native_h[digit];
    compo_picturebox_set(pic, white ? g_digit_w_addr[digit] : g_digit_b_addr[digit]);
    compo_picturebox_set_size(pic, w, h);
    compo_picturebox_set_pos(pic, tr_x - (s16)(w / 2), tr_y + (s16)(h / 2));
    if (pic->img != NULL) {
        widget_set_top(pic->img, true);
    }
}

static void time_page_btn_label_show(compo_textbox_t *txt, s16 x, s16 y,
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

static void time_page_update_display(void)
{
    f_time_page_t *inf = (f_time_page_t *)func_cb.f_cb;
    bool hour_sel, min_sel, no_sel, yes_sel;
    u8 h10, h1, m10, m1;

    if (inf == NULL) {
        return;
    }

    hour_sel = (inf->focus == TIME_PAGE_FOCUS_HOUR);
    min_sel  = (inf->focus == TIME_PAGE_FOCUS_MIN);
    no_sel   = (inf->focus == TIME_PAGE_FOCUS_BOTTOM && inf->bottom_sel == 0);
    yes_sel  = (inf->focus == TIME_PAGE_FOCUS_BOTTOM && inf->bottom_sel != 0);

    /* 时/分底图 */
    if (inf->pic_hour_bg != NULL) {
        compo_picturebox_set(inf->pic_hour_bg,
                             hour_sel ? UI_BUF_NEW_UI_NEW_BLUE_BJ1_BIN
                                      : UI_BUF_NEW_UI_NEW_GRAY_BJ1_BIN);
        compo_picturebox_set_size(inf->pic_hour_bg, NEW_TIME_BOX_W, NEW_TIME_BOX_H);
    }
    if (inf->pic_min_bg != NULL) {
        compo_picturebox_set(inf->pic_min_bg,
                             min_sel ? UI_BUF_NEW_UI_NEW_BLUE_BJ1_BIN
                                     : UI_BUF_NEW_UI_NEW_GRAY_BJ1_BIN);
        compo_picturebox_set_size(inf->pic_min_bg, NEW_TIME_BOX_W, NEW_TIME_BOX_H);
    }

    /* 数字：选中框白字，未选黑字；右上角锚点定位 */
    h10 = (u8)(inf->hour / 10);
    h1  = (u8)(inf->hour % 10);
    m10 = (u8)(inf->min  / 10);
    m1  = (u8)(inf->min  % 10);

    time_page_digit_update(inf->pic_h10, h10, hour_sel,
                           TIME_PAGE_CLOCK_TR_H10_X, TIME_PAGE_CLOCK_TR_H10_Y);
    time_page_digit_update(inf->pic_h1,  h1,  hour_sel,
                           TIME_PAGE_CLOCK_TR_H1_X,  TIME_PAGE_CLOCK_TR_H1_Y);
    time_page_digit_update(inf->pic_m10, m10, min_sel,
                           TIME_PAGE_CLOCK_TR_M10_X, TIME_PAGE_CLOCK_TR_M10_Y);
    time_page_digit_update(inf->pic_m1,  m1,  min_sel,
                           TIME_PAGE_CLOCK_TR_M1_X,  TIME_PAGE_CLOCK_TR_M1_Y);

    /* H / M 单位颜色 */
    if (inf->txt_unit_h != NULL) {
        compo_textbox_set_forecolor(inf->txt_unit_h,
                                    hour_sel ? TIME_PAGE_COLOR_UNIT_SEL : TIME_PAGE_COLOR_UNIT_NOR);
    }
    if (inf->txt_unit_min != NULL) {
        compo_textbox_set_forecolor(inf->txt_unit_min,
                                    min_sel ? TIME_PAGE_COLOR_UNIT_SEL : TIME_PAGE_COLOR_UNIT_NOR);
    }

    /* NO/YES 底图与文字 */
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
    time_page_btn_label_show(inf->txt_no, TIME_PAGE_BTN_NO_X, TIME_PAGE_BTN_LABEL_Y,
                             i18n[STR_BTN_NO], no_sel ? TIME_PAGE_COLOR_ON : TIME_PAGE_COLOR_OFF);
    time_page_btn_label_show(inf->txt_yes, TIME_PAGE_BTN_YES_X, TIME_PAGE_BTN_LABEL_Y,
                             i18n[STR_BTN_YES], yes_sel ? TIME_PAGE_COLOR_ON : TIME_PAGE_COLOR_OFF);
}

static void time_page_save_rtc(f_time_page_t *inf)
{
    tm_t tm_set;

    tm_set = rtc_clock_get();
    tm_set.hour = inf->hour;
    tm_set.min  = inf->min;
    rtc_clock_set(tm_set);
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_time_sync(RTCCNT + LB_RTC_UNIX_OFFSET);
#endif
}

static void time_page_value_inc(f_time_page_t *inf)
{
    if (inf->focus == TIME_PAGE_FOCUS_HOUR) {
        inf->hour = (u8)((inf->hour + 1) % 24);
    } else if (inf->focus == TIME_PAGE_FOCUS_MIN) {
        inf->min = (u8)((inf->min + 1) % 60);
    } else {
        inf->bottom_sel = (u8)((inf->bottom_sel + 1) & 1);
    }
    time_page_update_display();
}

static void time_page_value_dec(f_time_page_t *inf)
{
    if (inf->focus == TIME_PAGE_FOCUS_HOUR) {
        inf->hour = (u8)((inf->hour + 23) % 24);
    } else if (inf->focus == TIME_PAGE_FOCUS_MIN) {
        inf->min = (u8)((inf->min + 59) % 60);
    } else {
        inf->bottom_sel = (u8)((inf->bottom_sel + 1) & 1);
    }
    time_page_update_display();
}

static void time_page_confirm(f_time_page_t *inf)
{
    if (inf->focus == TIME_PAGE_FOCUS_HOUR) {
        inf->focus = TIME_PAGE_FOCUS_MIN;
        time_page_update_display();
    } else if (inf->focus == TIME_PAGE_FOCUS_MIN) {
        inf->focus = TIME_PAGE_FOCUS_BOTTOM;
        inf->bottom_sel = 1; /* 默认落在 YES */
        time_page_update_display();
    } else {
        if (inf->bottom_sel != 0) {
            time_page_save_rtc(inf);
        }
        func_cb.sta = FUNC_NEW_SETUP;
    }
}

compo_form_t *func_time_page_form_create(void)
{
    f_time_page_t *inf = (f_time_page_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_shape_t *shape;
    tm_t tm;

    /* 浅灰背景 */
    widget_set_visible(frm->icon, false);
    shape = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(shape, TIME_PAGE_BG);
    compo_shape_set_location(shape, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部标题栏 */
    general_title_bar_create(frm, &inf->tb, i18n[STR_SETUP_TIME]);

    /* 白色圆角卡片 */
    shape = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(shape, COLOR_WHITE);
    compo_shape_set_location(shape, GUI_SCREEN_CENTER_X, TIME_PAGE_PANEL_Y,
                             TIME_PAGE_PANEL_W, TIME_PAGE_PANEL_H);
    compo_shape_set_radius(shape, TIME_PAGE_PANEL_RADIUS);

    /* 时/分底图 */
    inf->pic_hour_bg = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_BLUE_BJ1_BIN);
    compo_picturebox_set_pos(inf->pic_hour_bg, TIME_PAGE_HOUR_X, TIME_PAGE_BOX_Y);
    compo_picturebox_set_size(inf->pic_hour_bg, NEW_TIME_BOX_W, NEW_TIME_BOX_H);

    inf->pic_min_bg = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_GRAY_BJ1_BIN);
    compo_picturebox_set_pos(inf->pic_min_bg, TIME_PAGE_MIN_X, TIME_PAGE_BOX_Y);
    compo_picturebox_set_size(inf->pic_min_bg, NEW_TIME_BOX_W, NEW_TIME_BOX_H);

    /* 上下箭头 */
    inf->pic_hour_up = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_UP_BIN);
    compo_picturebox_set_pos(inf->pic_hour_up, TIME_PAGE_HOUR_X, TIME_PAGE_ARROW_UP_Y);
    compo_picturebox_set_size(inf->pic_hour_up, NEW_TIME_ARROW_W, NEW_TIME_ARROW_H);

    inf->pic_hour_down = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_DOWN_BIN);
    compo_picturebox_set_pos(inf->pic_hour_down, TIME_PAGE_HOUR_X, TIME_PAGE_ARROW_DOWN_Y);
    compo_picturebox_set_size(inf->pic_hour_down, NEW_TIME_ARROW_W, NEW_TIME_ARROW_H);

    inf->pic_min_up = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_UP_BIN);
    compo_picturebox_set_pos(inf->pic_min_up, TIME_PAGE_MIN_X, TIME_PAGE_ARROW_UP_Y);
    compo_picturebox_set_size(inf->pic_min_up, NEW_TIME_ARROW_W, NEW_TIME_ARROW_H);

    inf->pic_min_down = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_DOWN_BIN);
    compo_picturebox_set_pos(inf->pic_min_down, TIME_PAGE_MIN_X, TIME_PAGE_ARROW_DOWN_Y);
    compo_picturebox_set_size(inf->pic_min_down, NEW_TIME_ARROW_W, NEW_TIME_ARROW_H);

    /* 数字（初始白/黑各一对） */
    inf->pic_h10 = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_TIME_W0_BIN);
    inf->pic_h1  = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_TIME_W0_BIN);
    inf->pic_m10 = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_TIME_B0_BIN);
    inf->pic_m1  = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_TIME_B0_BIN);

    /* 冒号 */
    inf->txt_colon = compo_textbox_create(frm, 4);
    compo_textbox_set_location(inf->txt_colon, GUI_SCREEN_CENTER_X,
                               (s16)(TIME_PAGE_BOX_Y - 8), 0, 0);
    compo_textbox_set_autosize(inf->txt_colon, true);
    compo_textbox_set_align_center(inf->txt_colon, true);
    compo_textbox_set_font(inf->txt_colon, UI_BUF_0FONT_FONT_TEST_18_BIN);
    compo_textbox_set_forecolor(inf->txt_colon, COLOR_BLACK);
    compo_textbox_set(inf->txt_colon, ":");

    /* H / M 单位 */
    inf->txt_unit_h = compo_textbox_create(frm, 4);
    compo_textbox_set_location(inf->txt_unit_h,
                               (s16)(TIME_PAGE_HOUR_X + NEW_TIME_BOX_W / 2 - 12),
                               TIME_PAGE_UNIT_Y, 0, 0);
    compo_textbox_set_autosize(inf->txt_unit_h, true);
    compo_textbox_set_align_center(inf->txt_unit_h, true);
    compo_textbox_set_font(inf->txt_unit_h, UI_BUF_0FONT_FONT_TEST_10_BIN);
    compo_textbox_set(inf->txt_unit_h, "H");

    inf->txt_unit_min = compo_textbox_create(frm, 8);
    compo_textbox_set_location(inf->txt_unit_min,
                               (s16)(TIME_PAGE_MIN_X + NEW_TIME_BOX_W / 2 - 8),
                               TIME_PAGE_UNIT_Y, 0, 0);
    compo_textbox_set_autosize(inf->txt_unit_min, true);
    compo_textbox_set_align_center(inf->txt_unit_min, true);
    compo_textbox_set_font(inf->txt_unit_min, UI_BUF_0FONT_FONT_TEST_10_BIN);
    compo_textbox_set(inf->txt_unit_min, "M");

    /* NO / YES 底图 */
    inf->pic_no_bg = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_GRAY_BJ2_BIN);
    compo_picturebox_set_pos(inf->pic_no_bg, TIME_PAGE_BTN_NO_X, TIME_PAGE_BTN_Y);
    compo_picturebox_set_size(inf->pic_no_bg, NEW_TIME_BTN_W, NEW_TIME_BTN_H);

    inf->pic_yes_bg = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN);
    compo_picturebox_set_pos(inf->pic_yes_bg, TIME_PAGE_BTN_YES_X, TIME_PAGE_BTN_Y);
    compo_picturebox_set_size(inf->pic_yes_bg, NEW_TIME_BTN_W, NEW_TIME_BTN_H);

    /* NO / YES 文字 */
    inf->txt_no = compo_textbox_create(frm, 8);
    compo_textbox_set_font(inf->txt_no, UI_BUF_0FONT_FONT_TEST_14_BIN);
    inf->txt_yes = compo_textbox_create(frm, 8);
    compo_textbox_set_font(inf->txt_yes, UI_BUF_0FONT_FONT_TEST_14_BIN);

    tm = rtc_clock_get();
    inf->hour = tm.hour;
    inf->min  = tm.min;
    inf->focus = TIME_PAGE_FOCUS_HOUR;
    inf->bottom_sel = 1;
    time_page_update_display();

    tft_bglight_force_on();
    return frm;
}

/* ---------- 按键处理 ---------- */

static void func_time_page_handle_keys(void)
{
    f_time_page_t *inf = (f_time_page_t *)func_cb.f_cb;
    func_key_event_t evt;

    while (func_key_get_event(&evt)) {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key) {
        case FUNC_KEY_UP:
            time_page_value_dec(inf);
            break;

        case FUNC_KEY_DOWN:
            time_page_value_inc(inf);
            break;

        case FUNC_KEY_CONFIRM:
            time_page_confirm(inf);
            break;

        case FUNC_KEY_BACK:
            func_cb.sta = FUNC_NEW_SETUP;
            break;

        default:
            break;
        }
    }
}

/* ---------- 生命周期 ---------- */

static void func_time_page_process(void)
{
    func_key_poll();
    func_time_page_handle_keys();
    func_key_lock_poll();

    if (func_key_lock_gui_dirty()) {
        if (func_key_lock_overlay_visible()) {
            func_lock_page_show(func_key_lock_overlay_is_unlock());
        } else {
            func_lock_page_hide();
        }
    }

    func_process();
}

static void func_time_page_message(size_msg_t msg)
{
    func_message(msg);
}

void func_time_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_time_page_t));
    func_key_reset();
    func_cb.frm_main = func_time_page_form_create();
    general_title_bar_attach(&((f_time_page_t *)func_cb.f_cb)->tb);
}

void func_time_page_exit(void)
{
    func_key_flush();
    general_title_bar_detach();
}

void func_time_page(void)
{
    printf("%s\n", __func__);
    func_time_page_enter();
    while (func_cb.sta == FUNC_NEW_TIME) {
        func_time_page_process();
        func_time_page_message(msg_dequeue());
    }
    func_time_page_exit();
}
