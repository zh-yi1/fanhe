#include "include.h"
#include "func.h"
#include "func_reservation.h"
#include "func_key_lock.h"
#include "home_top_time_txt.h"
#include "home_ui_shared.h"

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

/*
 * 预约页 — 效果图「Set Time (Heating Finish Time)」
 *   三列滚轮：时(0~23) / 分(5min步进) / 秒(5s步进)
 *   加/减：调节当前列；确认：切换列(时→分→秒→提交)
 *   电源：取消回 Home
 */
#define NEW_RES_ROLL_COLS               3
#define NEW_RES_ROLL_ROWS               5
#define NEW_RES_ROLL_CENTER_ROW         2

#define NEW_RES_TITLE_Y                 25
#define NEW_RES_PANEL_Y                 145
#define NEW_RES_PANEL_W                 280
#define NEW_RES_PANEL_H                 185
#define NEW_RES_ROW_GAP                 35

#define NEW_RES_COL_X_H                 88
#define NEW_RES_COL_X_M                 160
#define NEW_RES_COL_X_S                 232
#define NEW_RES_COLON_X0                124
#define NEW_RES_COLON_X1                196

#define NEW_RES_COLOR_TITLE             COLOR_BLACK
#define NEW_RES_COLOR_SEL_FOCUS         0x0AD8
#define NEW_RES_COLOR_SEL               COLOR_BLACK
#define NEW_RES_COLOR_DIM               0xC618
#define NEW_RES_COLOR_COLON             0xDEFB
#define NEW_RES_PANEL_BG                0xEF5D

#define NEW_RES_MSG_OK                  KU_BACK
#define NEW_RES_MSG_PLUS                KU_VOL_UP
#define NEW_RES_MSG_MINUS               KU_VOL_DOWN
#define NEW_RES_MSG_POWER               (KEY_RIGHT | KEY_SHORT_UP)

enum {
    COMPO_ID_SHAPE_BG = 1,
    COMPO_ID_TXT_TOP_TIME,
    COMPO_ID_SHAPE_PANEL,
    COMPO_ID_TXT_TITLE,
    COMPO_ID_TXT_ROLL_BASE = 10,
    COMPO_ID_TXT_COLON0 = 25,
    COMPO_ID_TXT_COLON1 = 26,
};

enum {
    NEW_RES_FOCUS_HOUR = 0,
    NEW_RES_FOCUS_MIN,
    NEW_RES_FOCUS_SEC,
    NEW_RES_FOCUS_CNT,
};

typedef struct {
    u8 appt_hour;
    u8 appt_min;
    u8 appt_sec;
    u8 focus_col;
    u8 last_top_min;
    u8 last_top_sec;
    bool display_pending;
    home_top_time_txt_t top_time;
    compo_textbox_t *txt_title;
    compo_textbox_t *txt_roll[NEW_RES_ROLL_COLS][NEW_RES_ROLL_ROWS];
    compo_textbox_t *txt_colon[2];
} f_new_reservation_t;

#if ELUNCHBOX_PANEL_EN
static bool new_res_font_ready;

static void new_res_font_bind_txt(compo_textbox_t *txt)
{
    if (txt != NULL) {
        compo_textbox_set_font(txt, NEW_HEAT_FONT);
    }
}

static void new_res_font_apply_once(f_new_reservation_t *f)
{
    u8 col;
    u8 row;

    if (new_res_font_ready || f == NULL) {
        return;
    }

    WDT_CLR();
    new_res_font_bind_txt(f->txt_title);
    for (col = 0; col < NEW_RES_ROLL_COLS; col++) {
        for (row = 0; row < NEW_RES_ROLL_ROWS; row++) {
            WDT_CLR();
            new_res_font_bind_txt(f->txt_roll[col][row]);
        }
    }
    WDT_CLR();
    new_res_font_bind_txt(f->txt_colon[0]);
    new_res_font_bind_txt(f->txt_colon[1]);
    new_res_font_ready = true;
}
#endif

static s16 new_res_col_x(u8 col)
{
    static const s16 tbl[NEW_RES_ROLL_COLS] = {
        NEW_RES_COL_X_H, NEW_RES_COL_X_M, NEW_RES_COL_X_S,
    };

    if (col >= NEW_RES_ROLL_COLS) {
        return NEW_RES_COL_X_H;
    }
    return tbl[col];
}

static s16 new_res_row_y(u8 row)
{
    return (s16)(NEW_RES_PANEL_Y + (s16)(row - NEW_RES_ROLL_CENTER_ROW) * NEW_RES_ROW_GAP);
}

static u8 new_res_col_value(const f_new_reservation_t *f, u8 col)
{
    if (f == NULL) {
        return 0;
    }
    switch (col) {
    case NEW_RES_FOCUS_HOUR:
        return f->appt_hour;
    case NEW_RES_FOCUS_MIN:
        return f->appt_min;
    case NEW_RES_FOCUS_SEC:
        return f->appt_sec;
    default:
        return 0;
    }
}

static void new_res_col_value_set(f_new_reservation_t *f, u8 col, u8 val)
{
    if (f == NULL) {
        return;
    }
    switch (col) {
    case NEW_RES_FOCUS_HOUR:
        f->appt_hour = val;
        break;
    case NEW_RES_FOCUS_MIN:
        f->appt_min = val;
        break;
    case NEW_RES_FOCUS_SEC:
        f->appt_sec = val;
        break;
    default:
        break;
    }
}

static u8 new_res_roll_at(u8 col, u8 cur, s8 offset)
{
    if (col == NEW_RES_FOCUS_HOUR) {
        s16 v = (s16)cur + offset;

        while (v < 0) {
            v += 24;
        }
        while (v > FUNC_RES_APPT_HOUR_MAX) {
            v -= 24;
        }
        return (u8)v;
    }

    {
        u8 idx = cur / 5;
        s16 ni = (s16)idx + offset;

        while (ni < 0) {
            ni += 12;
        }
        while (ni > 11) {
            ni -= 12;
        }
        return (u8)(ni * 5);
    }
}

static void new_res_col_inc(f_new_reservation_t *f, u8 col)
{
    u8 val;

    if (f == NULL) {
        return;
    }
    val = new_res_col_value(f, col);
    val = new_res_roll_at(col, val, 1);
    new_res_col_value_set(f, col, val);
    f->display_pending = true;
}

static void new_res_col_dec(f_new_reservation_t *f, u8 col)
{
    u8 val;

    if (f == NULL) {
        return;
    }
    val = new_res_col_value(f, col);
    val = new_res_roll_at(col, val, -1);
    new_res_col_value_set(f, col, val);
    f->display_pending = true;
}

static u16 new_res_cell_color(u8 col, u8 row, u8 focus_col)
{
    if (row != NEW_RES_ROLL_CENTER_ROW) {
        return NEW_RES_COLOR_DIM;
    }
    if (col == focus_col) {
        return NEW_RES_COLOR_SEL_FOCUS;
    }
    return NEW_RES_COLOR_SEL;
}

static compo_textbox_t *new_res_txt_create(compo_form_t *frm, u16 id, u16 buf_size,
                                           s16 x, s16 y, u16 w, u16 h, u16 color)
{
    compo_textbox_t *txt = compo_textbox_create(frm, buf_size);

    compo_setid(txt, id);
    compo_textbox_set_wholewrap(txt, false);
#if ELUNCHBOX_PANEL_EN
    /* 禁用 autosize → 后续 compo_textbox_set() 不读字体 Flash，避免 C281 */
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_visible(txt, false);
#else
    compo_textbox_set_font(txt, NEW_HEAT_FONT);
#endif
    compo_textbox_set_pos(txt, x, y);
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set_location(txt, x, y, w, h);
    return txt;
}

static void new_res_white_bg_create(compo_form_t *frm)
{
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, COMPO_ID_SHAPE_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(bg, NEW_RES_PANEL_BG);
    compo_shape_set_radius(bg, 0);
}

static void new_res_panel_create(compo_form_t *frm)
{
    compo_shape_t *panel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(panel, COMPO_ID_SHAPE_PANEL);
    compo_shape_set_location(panel, GUI_SCREEN_CENTER_X, NEW_RES_PANEL_Y,
                             NEW_RES_PANEL_W, NEW_RES_PANEL_H);
    compo_shape_set_color(panel, COLOR_WHITE);
    compo_shape_set_radius(panel, 16);
}

static void new_res_bind_objects(f_new_reservation_t *f)
{
    u8 col;
    u8 row;

    if (f == NULL) {
        return;
    }
    home_top_time_txt_bind(&f->top_time, COMPO_ID_TXT_TOP_TIME);
    f->txt_title = compo_getobj_byid(COMPO_ID_TXT_TITLE);
    for (col = 0; col < NEW_RES_ROLL_COLS; col++) {
        for (row = 0; row < NEW_RES_ROLL_ROWS; row++) {
            f->txt_roll[col][row] = compo_getobj_byid(COMPO_ID_TXT_ROLL_BASE + col * NEW_RES_ROLL_ROWS + row);
        }
    }
    f->txt_colon[0] = compo_getobj_byid(COMPO_ID_TXT_COLON0);
    f->txt_colon[1] = compo_getobj_byid(COMPO_ID_TXT_COLON1);
}

static void new_res_roller_apply(f_new_reservation_t *f)
{
    char buf[8];
    u8 col;
    u8 row;

    if (f == NULL) {
        return;
    }

#if ELUNCHBOX_PANEL_EN
    new_res_font_apply_once(f);
#endif

    if (f->txt_title != NULL) {
        widget_text_t *widget = f->txt_title->txt;
        rect_t rect;
        area_t text_area;

        compo_textbox_set_align_center(f->txt_title, true);
        compo_textbox_set_wholewrap(f->txt_title, false);
        compo_textbox_set_autoroll(f->txt_title, false);
        compo_textbox_set_autoroll_mode(f->txt_title, TEXT_AUTOROLL_MODE_NULL);
#if ELUNCHBOX_PANEL_EN
        compo_textbox_set_autosize(f->txt_title, false);
#endif
        widget_text_set_ellipsis(widget, false);
        widget_set_location(widget,
                            GUI_SCREEN_CENTER_X, NEW_RES_TITLE_Y,
                            GUI_SCREEN_WIDTH - 16, 40);
        compo_textbox_set(f->txt_title, "Set Time (Heating Finish Time)");
        rect = widget_get_location(widget);
        text_area = widget_text_get_area(widget);
        if (rect.hei > text_area.hei) {
            widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
        } else {
            widget_text_set_client(widget, 0, 0);
        }
        compo_textbox_set_visible(f->txt_title, true);
    }

    for (col = 0; col < NEW_RES_ROLL_COLS; col++) {
        u8 cur = new_res_col_value(f, col);

        for (row = 0; row < NEW_RES_ROLL_ROWS; row++) {
            compo_textbox_t *txt = f->txt_roll[col][row];
            s8 offset = (s8)row - (s8)NEW_RES_ROLL_CENTER_ROW;
            u8 show_val = new_res_roll_at(col, cur, offset);

            if (txt == NULL) {
                continue;
            }
            snprintf(buf, sizeof(buf), "%02u", show_val);
            compo_textbox_set(txt, buf);
            compo_textbox_set_forecolor(txt, new_res_cell_color(col, row, f->focus_col));
            compo_textbox_set_pos(txt, new_res_col_x(col), new_res_row_y(row));
            compo_textbox_set_visible(txt, true);
        }
    }

    for (col = 0; col < 2; col++) {
        if (f->txt_colon[col] != NULL) {
            compo_textbox_set(f->txt_colon[col], ":");
            compo_textbox_set_forecolor(f->txt_colon[col], NEW_RES_COLOR_COLON);
            compo_textbox_set_pos(f->txt_colon[col],
                                  col == 0 ? NEW_RES_COLON_X0 : NEW_RES_COLON_X1,
                                  new_res_row_y(NEW_RES_ROLL_CENTER_ROW));
            compo_textbox_set_visible(f->txt_colon[col], true);
        }
    }
}

static void new_res_ui_refresh(f_new_reservation_t *f)
{
    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    home_gpu_wait_idle();
    WDT_CLR();
#endif
    home_top_time_txt_force(&f->top_time, &f->last_top_min, &f->last_top_sec);
    new_res_roller_apply(f);
    f->display_pending = false;
}

static void new_res_value_inc(f_new_reservation_t *f)
{
    if (f == NULL) {
        return;
    }
    new_res_col_inc(f, f->focus_col);
}

static void new_res_value_dec(f_new_reservation_t *f)
{
    if (f == NULL) {
        return;
    }
    new_res_col_dec(f, f->focus_col);
}

static void new_res_ok_key(f_new_reservation_t *f)
{
    if (f == NULL || sys_cb.flag_swithing) {
        return;
    }
    if (f->focus_col + 1 < NEW_RES_FOCUS_CNT) {
        f->focus_col++;
        f->display_pending = true;
        return;
    }
    /* 全部列已确认：保存预约时间到 g_res，跳转到加热参数设置页 */
    g_res.setup_done = true;
    g_res.appt_hour = f->appt_hour;
    g_res.appt_min = f->appt_min;
    g_res.heat_hour = 0;
    g_res.heat_min = 0;
    g_res.temp_idx = 0;
    g_res_heat_pending = true;
    func_switch_to(FUNC_NEW_HEAT, FUNC_SWITCH_DIRECT | FUNC_SWITCH_AUTO);
}

static void new_res_power_key(void)
{
    if (sys_cb.flag_swithing) {
        return;
    }
    func_reservation_new_ui_go_home();
}

compo_form_t *func_new_reservation_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    u8 col;
    u8 row;

    new_res_white_bg_create(frm);

    home_top_time_txt_create(frm, COMPO_ID_TXT_TOP_TIME);

    new_res_panel_create(frm);

    /* 标题 "Set Time (Heating Finish Time)" = 32 字符，预置 32buf */
    txt = new_res_txt_create(frm, COMPO_ID_TXT_TITLE, 32,
                             GUI_SCREEN_CENTER_X, NEW_RES_TITLE_Y, GUI_SCREEN_WIDTH - 16, 28,
                             NEW_RES_COLOR_TITLE);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, NEW_RES_TITLE_Y,
                               GUI_SCREEN_WIDTH - 16, 28);

    for (col = 0; col < NEW_RES_ROLL_COLS; col++) {
        for (row = 0; row < NEW_RES_ROLL_ROWS; row++) {
            (void)new_res_txt_create(frm,
                                     (u16)(COMPO_ID_TXT_ROLL_BASE + col * NEW_RES_ROLL_ROWS + row),
                                     8,
                                     new_res_col_x(col), new_res_row_y(row), 56, 35,
                                     NEW_RES_COLOR_DIM);
        }
    }

    (void)new_res_txt_create(frm, COMPO_ID_TXT_COLON0, 8, NEW_RES_COLON_X0,
                             new_res_row_y(NEW_RES_ROLL_CENTER_ROW), 20, 32, NEW_RES_COLOR_COLON);
    (void)new_res_txt_create(frm, COMPO_ID_TXT_COLON1, 8, NEW_RES_COLON_X1,
                             new_res_row_y(NEW_RES_ROLL_CENTER_ROW), 20, 32, NEW_RES_COLOR_COLON);

    return frm;
}

static void func_new_reservation_message(size_msg_t msg)
{
    f_new_reservation_t *f = (f_new_reservation_t *)func_cb.f_cb;

    if (sys_cb.flag_swithing) {
        return;
    }
    if (func_key_lock_ku_blocked(msg)) {
        return;
    }
    switch (msg) {
    case KU_BACK:
        new_res_ok_key(f);
        break;
    case KU_VOL_UP:
        new_res_value_inc(f);
        break;
    case KU_VOL_DOWN:
        new_res_value_dec(f);
        break;
    case KEY_RIGHT | KEY_SHORT_UP:
        new_res_power_key();
        break;
    default:
        func_message(msg);
        break;
    }
}

static void func_new_reservation_process(void)
{
    f_new_reservation_t *f = (f_new_reservation_t *)func_cb.f_cb;

    if (f == NULL) {
        func_process();
        return;
    }
    if (f->display_pending) {
        new_res_ui_refresh(f);
    }
    home_top_time_txt_tick(&f->top_time, &f->last_top_min, &f->last_top_sec);
    func_process();
}

void func_new_reservation_enter(void)
{
    f_new_reservation_t *f;

    printf("func_new_reservation_enter\n");

#if ELUNCHBOX_PANEL_EN
    new_res_font_ready = false;
    home_gpu_wait_idle();
    WDT_CLR();
#endif

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif

    msg_queue_detach(KU_BACK, 0);
    msg_queue_detach(KU_VOL_UP, 0);
    msg_queue_detach(KU_VOL_DOWN, 0);
    msg_queue_detach(KU_RIGHT, 0);

    func_cb.f_cb = func_zalloc(sizeof(f_new_reservation_t));
    f = (f_new_reservation_t *)func_cb.f_cb;
    func_reservation_new_ui_load_time(&f->appt_hour, &f->appt_min, &f->appt_sec);
    f->focus_col = NEW_RES_FOCUS_HOUR;
    f->last_top_min = 0xff;
    f->last_top_sec = 0xff;

    func_cb.frm_main = func_new_reservation_form_create();
    new_res_bind_objects(f);

    /* 首帧由 process() 做 new_res_ui_refresh */
    f->display_pending = true;

#if ELUNCHBOX_PANEL_EN
    home_gpu_wait_idle();
    WDT_CLR();
    elunchbox_te_block_flag = 0;
    tft_bglight_force_on();
#endif
}

void func_new_reservation_exit(void)
{
    func_cb.last = FUNC_RESERVATION;
    printf("func_new_reservation_exit\n");
}

void func_new_reservation(void)
{
    printf("func_new_reservation run\n");
    func_new_reservation_enter();
    while (func_cb.sta == FUNC_RESERVATION) {
        func_new_reservation_process();
        func_new_reservation_message(msg_dequeue());
    }
    func_new_reservation_exit();
}
