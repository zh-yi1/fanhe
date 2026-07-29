#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"
#include "ui.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/* ---- 滚轮布局 ---- */
#define ROLL_COLS           3       /* 时 / 分 / 秒 */
#define ROLL_ROWS           5
#define ROLL_CENTER_ROW     2

#define TITLE_Y             25
#define PANEL_Y             145
#define PANEL_W             280
#define PANEL_H             185
#define ROW_GAP             35

#define COL_X_H             88      /* 时列 X */
#define COL_X_M             160     /* 分列 X */
#define COL_X_S             232     /* 秒列 X */
#define COLON_X1            124     /* 时:分 冒号 */
#define COLON_X2            196     /* 分:秒 冒号 */

#define COLOR_SEL_FOCUS     0x0AD8
#define COLOR_SEL           COLOR_BLACK
#define COLOR_DIM           0xC618
#define COLOR_COLON         0xDEFB

enum { FOCUS_HOUR = 0, FOCUS_MIN, FOCUS_SEC };

/* ---- 私有状态 ---- */
typedef struct
{
    u8 display_stage;
    u8 hour;
    u8 min;
    u8 sec;
    u8 focus_col;
    u8 prev_sta;        /* 上一个页面状态，退出时根据它决定返回 */
    compo_textbox_t *txt_title;
    compo_textbox_t *txt_roll[ROLL_COLS][ROLL_ROWS];
    compo_textbox_t *txt_colon[2];
} f_appointment_t;

/* ---- 滚轮计算 ---- */
static s16 col_x(u8 col)
{
    static const s16 tbl[ROLL_COLS] = { COL_X_H, COL_X_M, COL_X_S };
    return tbl[col];
}

static s16 row_y(u8 row)
{
    return (s16)(PANEL_Y + (s16)(row - ROLL_CENTER_ROW) * ROW_GAP);
}

static u8 roll_step(u8 col, u8 cur, s8 offset)
{
    if (col == FOCUS_HOUR) {
        s16 v = (s16)cur + offset;
        while (v < 0) v += 24;
        while (v > 23) v -= 24;
        return (u8)v;
    }
    /* 分/秒 5 步进 */
    {
        u8 step = cur / 5;
        s16 ni = (s16)step + offset;
        while (ni < 0) ni += 12;
        while (ni > 11) ni -= 12;
        return (u8)(ni * 5);
    }
}

static u16 cell_color(u8 col, u8 row, u8 focus_col)
{
    if (row != ROLL_CENTER_ROW) return COLOR_DIM;
    return (col == focus_col) ? COLOR_SEL_FOCUS : COLOR_SEL;
}

/* ---- 显示刷新（每按键触发） ---- */
static void appointment_update_display(void)
{
    f_appointment_t *inf = (f_appointment_t *)func_cb.f_cb;
    char buf[8];
    u8 col, row;

    for (col = 0; col < ROLL_COLS; col++) {
        u8 cur;
        if (col == FOCUS_HOUR) cur = inf->hour;
        else if (col == FOCUS_MIN) cur = inf->min;
        else cur = inf->sec;

        for (row = 0; row < ROLL_ROWS; row++) {
            s8 offset = (s8)row - (s8)ROLL_CENTER_ROW;
            u8 show = roll_step(col, cur, offset);

            snprintf(buf, sizeof(buf), "%02u", show);
            compo_textbox_set(inf->txt_roll[col][row], buf);
            compo_textbox_set_forecolor(inf->txt_roll[col][row],
                                        cell_color(col, row, inf->focus_col));
        }
    }
}

/* ---- 生命周期 ---- */
compo_form_t *func_appointment_time_form_create(void)
{
    f_appointment_t *inf = (f_appointment_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    u8 col, row;
    char buf[8];

    /* 米黄背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, 0xEF5D);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 白色面板 */
    compo_shape_t *panel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(panel, COLOR_WHITE);
    compo_shape_set_location(panel, GUI_SCREEN_CENTER_X, PANEL_Y,
                             PANEL_W, PANEL_H);
    compo_shape_set_radius(panel, 16);

    /* 标题 */
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, TITLE_Y,
                               GUI_SCREEN_WIDTH - 16, 28);
    compo_textbox_set(txt, i18n[STR_YUYUE_NAME]);
    inf->txt_title = txt;

    /* 滚轮行（初始值直接设好，全部可见） */
    for (col = 0; col < ROLL_COLS; col++) {
        u8 cur;
        if (col == FOCUS_HOUR) cur = inf->hour;
        else if (col == FOCUS_MIN) cur = inf->min;
        else cur = inf->sec;

        for (row = 0; row < ROLL_ROWS; row++) {
            s8 offset = (s8)row - (s8)ROLL_CENTER_ROW;
            u8 show = roll_step(col, cur, offset);

            txt = compo_textbox_create(frm, 8);
            compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_TEST_BIN);
            compo_textbox_set_autosize(txt, false);
            compo_textbox_set_align_center(txt, true);
            compo_textbox_set_location(txt, col_x(col), row_y(row), 56, 35);
            compo_textbox_set_multiline(txt, false);

            snprintf(buf, sizeof(buf), "%02u", show);
            compo_textbox_set(txt, buf);
            compo_textbox_set_forecolor(txt, cell_color(col, row, FOCUS_HOUR));
            inf->txt_roll[col][row] = txt;
        }
    }

    /* 冒号 ×2 */
    {
        u8 c;
        s16 cx[2] = { COLON_X1, COLON_X2 };
        for (c = 0; c < 2; c++) {
            txt = compo_textbox_create(frm, 8);
            compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_TEST_BIN);
            compo_textbox_set_autosize(txt, false);
            compo_textbox_set_align_center(txt, true);
            compo_textbox_set_forecolor(txt, COLOR_COLON);
            compo_textbox_set_location(txt, cx[c], row_y(ROLL_CENTER_ROW), 20, 35);
            compo_textbox_set_multiline(txt, false);
            compo_textbox_set(txt, ":");
            inf->txt_colon[c] = txt;
        }
    }

    tft_bglight_force_on();
    return frm;
}

/*
 * 按键处理 — 逻辑键由 func_key_map_logical 映射：
 *   TCH6 → UP, TCH2 → DOWN, TCH4 → CONFIRM, TCH5 → BACK
 *
 * 滚轮操作：
 *   UP/DOWN — 当前列（时/分/秒）增减，分秒 5 步进
 *   CONFIRM — 时→分→秒→进入加热设置页
 *   BACK    — 秒→分→时→返回模式页
 */
static void func_appointment_time_handle_keys(void)
{
    f_appointment_t *inf = (f_appointment_t *)func_cb.f_cb;
    func_key_event_t evt;

    while (func_key_get_event(&evt))
    {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key)
        {
        case FUNC_KEY_UP:
        case FUNC_KEY_DOWN:
        {
            s8 dir = (key == FUNC_KEY_UP) ? -1 : 1;

            if (inf->focus_col == FOCUS_HOUR)
                inf->hour = roll_step(FOCUS_HOUR, inf->hour, dir);
            else if (inf->focus_col == FOCUS_MIN)
                inf->min = roll_step(FOCUS_MIN, inf->min, dir);
            else
                inf->sec = roll_step(FOCUS_SEC, inf->sec, dir);
            appointment_update_display();
            break;
        }

        case FUNC_KEY_CONFIRM:
            if (inf->focus_col == FOCUS_HOUR) {
                inf->focus_col = FOCUS_MIN;
                appointment_update_display();
            } else if (inf->focus_col == FOCUS_MIN) {
                inf->focus_col = FOCUS_SEC;
                appointment_update_display();
            } else {
                func_cb.sta = FUNC_NEW_HEAT_SET;
            }
            break;

        case FUNC_KEY_BACK:
            if (inf->focus_col == FOCUS_SEC) {
                inf->focus_col = FOCUS_MIN;
                appointment_update_display();
            } else if (inf->focus_col == FOCUS_MIN) {
                inf->focus_col = FOCUS_HOUR;
                appointment_update_display();
            } else {
                /* 从模式页进入 → 回模式页；其他情况 → 回主页 */
                func_cb.sta = (inf->prev_sta == FUNC_NEW_MODE)
                              ? FUNC_NEW_MODE : FUNC_HOME;
            }
            break;

        /* 直接按键：加热键 → 加热设置页 */
        case FUNC_KEY_HEAT:
            func_cb.sta = FUNC_NEW_HEAT_SET;
            break;

        /* 直接按键：模式键 → 模式页 */
        case FUNC_KEY_MODE:
            func_cb.sta = FUNC_NEW_MODE;
            break;

        /* 直接按键：预约键 → 预约页 */
        case FUNC_KEY_RESERVATION:
            func_cb.sta = FUNC_APPOINTMENT_TIME;
            break;


        default:
            break;
        }
    }
}

/* ---- 每帧处理 ---- */
static void func_appointment_time_process(void)
{
    f_appointment_t *inf = (f_appointment_t *)func_cb.f_cb;

#if ELUNCHBOX_PANEL_EN
    if (inf->display_stage != 0)
    {
        inf->display_stage = 0;
        return;
    }
#endif

    /* 1. 按键处理：扫描 → 长按检测 → 童锁过滤 → 事件入队 */
    func_key_poll();
    func_appointment_time_handle_keys();

    /* 2. 童锁计时器（hint过期/自动锁），不碰 GUI */
    func_key_lock_poll();

    /* 3. 锁标志位 → UI 渲染（页面负责显示，key 模块不管 UI） */
    if (func_key_lock_gui_dirty())
    {
        if (func_key_lock_overlay_visible())
        {
            func_lock_page_show(func_key_lock_overlay_is_unlock());
        }
        else
        {
            func_lock_page_hide();
        }
    }

    func_process();
}

static void func_appointment_time_message(size_msg_t msg)
{
    /* PT8028 按键已由 func_key_poll 统一处理，消息队列仅处理系统事件 */
    func_message(msg);
}

void func_appointment_time_enter(void)
{
    f_appointment_t *inf;

    func_cb.f_cb = func_zalloc(sizeof(f_appointment_t));
    func_key_reset();
    inf = (f_appointment_t *)func_cb.f_cb;

    /* 记录进入前的页面状态，退出时根据它决定返回 */
    inf->prev_sta = func_cb.last;

    /* 先同步系统时间，再创建 form，滚轮初始值即为当前时间 */
    {
        tm_t tm = rtc_clock_get();
        inf->hour = tm.hour;
        inf->min  = tm.min;
    }
    inf->sec = 0;
    inf->focus_col = FOCUS_HOUR;
    inf->display_stage = 1;

    func_cb.frm_main = func_appointment_time_form_create();
}

void func_appointment_time_exit(void)
{
    func_key_flush();
}

void func_appointment_time(void)
{
    printf("%s\n", __func__);
    func_appointment_time_enter();
    while (func_cb.sta == FUNC_APPOINTMENT_TIME)
    {
        func_appointment_time_process();
        func_appointment_time_message(msg_dequeue());
    }
    func_appointment_time_exit();
}
