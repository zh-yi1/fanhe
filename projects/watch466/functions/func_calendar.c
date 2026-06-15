#include "include.h"
#include "func.h"
#include "compo_shape.h"

typedef struct f_calendar_t_ {
    u16 today_year;
    u8 today_mon;
    u8 today_day;
    u16 update_year;
    u8 update_mon;
    u16 last_rtc_year;
    u8 last_rtc_mon;
    u8 last_rtc_day;
} f_calendar_t;

#define CALE_GRID_COLS              7
#define CALE_GRID_ROWS              6
#define CALE_CELL_MAX               (CALE_GRID_COLS * CALE_GRID_ROWS)

#define CALE_MARGIN_X               24
#define CALE_GAP_X                  6
#define CALE_CELL_W                 ((s16)((GUI_SCREEN_WIDTH - 2 * CALE_MARGIN_X - (CALE_GRID_COLS - 1) * CALE_GAP_X) / CALE_GRID_COLS))
#define CALE_CELL_H                 30
#define CALE_ROW_GAP                8
#define CALE_HIGHLIGHT_WH           36

#define CALE_HEADER_Y               72
#define CALE_WEEK_Y                 118
#define CALE_GRID_Y0                152

#define IS_LEAP_YEAR(year)          (!((year) % 400) || (((year) % 100) && !((year) % 4)))

enum {
    COMPO_ID_HEADER_DATE = 1,
    COMPO_ID_TODAY_HIGHLIGHT,           /* 仅 1 个，刷新时移到当天格 */
    COMPO_ID_DATE_PIC_START = 3,
};

static char calendar_header_str[16];

static const u16 tbl_calendar_week_str[CALE_GRID_COLS] = {
    STR_MONDAY, STR_TUESDAY, STR_WEDNESDAY, STR_THURSDAY,
    STR_FRIDAY, STR_SATURDAY, STR_SUNDAY,
};

static u8 cal_last_month(u8 cur_month)
{
    return (cur_month > 1) ? (cur_month - 1) : 12;
}

static u8 cal_max_of_days_per_month(u16 year, u8 month)
{
    switch (month) {
    case 1: case 3: case 5: case 7: case 8: case 10: case 12:
        return 31;
    case 4: case 6: case 9: case 11:
        return 30;
    case 2:
        return IS_LEAP_YEAR(year) ? 29 : 28;
    default:
        return 30;
    }
}

/* Zeller：返回 0=周一 … 6=周日 */
static u8 cal_weekday_monday_first(u16 year, u8 month, u8 day)
{
    u16 y = year;
    u8 m = month;

    if (m < 3) {
        m += 12;
        y--;
    }
    {
        u16 k = y % 100;
        u16 j = y / 100;
        int h = (day + 13 * (m + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;

        return (u8)((h + 5) % 7);
    }
}

static void func_calendar_rtc_read(f_calendar_t *f)
{
    tm_t tm = rtc_clock_get();

    f->today_year = tm.year;
    f->today_mon = tm.mon;
    f->today_day = tm.day;
}

/* 月历始终对准 RTC 当天所在年月 */
static void func_calendar_sync_today_view(f_calendar_t *f)
{
    func_calendar_rtc_read(f);
    f->update_year = f->today_year;
    f->update_mon = f->today_mon;
}

static void func_calendar_header_refresh(f_calendar_t *f)
{
    compo_label_t *header = compo_getobj_byid(COMPO_ID_HEADER_DATE);

    snprintf(calendar_header_str, sizeof(calendar_header_str), "%04u/%u/%u",
             (unsigned)f->today_year, (unsigned)f->today_mon, (unsigned)f->today_day);
    if (header != NULL) {
        compo_label_set(header, calendar_header_str);
    }
}

static s16 func_calendar_cell_x(u8 col)
{
    return (s16)(CALE_MARGIN_X + CALE_CELL_W / 2 + col * (CALE_CELL_W + CALE_GAP_X));
}

static s16 func_calendar_cell_y(u8 row)
{
    return (s16)(CALE_GRID_Y0 + CALE_CELL_H / 2 + row * (CALE_CELL_H + CALE_ROW_GAP));
}

static void func_calendar_grid_refresh(f_calendar_t *f)
{
    u16 year = f->update_year;
    u8 month = f->update_mon;
    u8 first_col = cal_weekday_monday_first(year, month, 1);
    u8 day_max = cal_max_of_days_per_month(year, month);
    u8 last_day_max = cal_max_of_days_per_month(year, cal_last_month(month));
    u8 day = 1;
    u8 i;
    bool today_found = false;
    s16 today_x = 0;
    s16 today_y = 0;
    compo_shape_t *hl = compo_getobj_byid(COMPO_ID_TODAY_HIGHLIGHT);

    for (i = 0; i < CALE_CELL_MAX; i++) {
        compo_picturebox_t *pic = compo_getobj_byid(COMPO_ID_DATE_PIC_START + i);
        bool in_month;
        bool is_today;
        u8 show_day;

        if (pic == NULL) {
            continue;
        }

        in_month = false;
        is_today = false;
        show_day = 0;

        if (i < first_col) {
            show_day = (u8)(last_day_max - first_col + i + 1);
        } else if (day <= day_max) {
            show_day = day;
            in_month = true;
            if (f->today_year == year && f->today_mon == month && f->today_day == day) {
                is_today = true;
                today_found = true;
                today_x = func_calendar_cell_x(i % CALE_GRID_COLS);
                today_y = func_calendar_cell_y(i / CALE_GRID_COLS);
            }
            day++;
        } else {
            show_day = (u8)(day - day_max);
            day++;
        }

        compo_picturebox_set_color(pic, is_today ? COLOR_BLACK : (in_month ? COLOR_WHITE : COLOR_GRAY));
        compo_picturebox_cut(pic, show_day, 32);
    }

    if (hl != NULL) {
        if (today_found) {
            compo_shape_set_location(hl, today_x, today_y, CALE_HIGHLIGHT_WH, CALE_HIGHLIGHT_WH);
            compo_shape_set_visible(hl, true);
        } else {
            compo_shape_set_visible(hl, false);
        }
    }
}

static void func_calender_refresh(f_calendar_t *f)
{
    func_calendar_header_refresh(f);
    func_calendar_grid_refresh(f);
}

compo_form_t *func_calender_form_create(void)
{
    u8 i;
    compo_form_t *frm = compo_form_create(true);

    compo_label_t *header = compo_label_create(frm, 16);
    compo_label_set_pos(header, GUI_SCREEN_CENTER_X, CALE_HEADER_Y);
    compo_label_set_align_center(header, true);
    compo_label_set_forecolor(header, COLOR_WHITE);
    compo_setid(header, COMPO_ID_HEADER_DATE);

    {
        s16 x_pos = func_calendar_cell_x(0);

        for (i = 0; i < CALE_GRID_COLS; i++) {
            compo_label_t *lbl = compo_label_create(frm, 4);
            compo_label_set_pos(lbl, x_pos, CALE_WEEK_Y);
            compo_label_set_align_center(lbl, true);
            compo_label_set_forecolor(lbl, COLOR_WHITE);
            /* 英文 Mon/Tue 用窗体时间小字库；中文仍用系统字库 */
            if (sys_cb.lang_id == LANG_EN) {
                compo_label_set_font(lbl, UI_BUF_FONT_FORM_TIME);
            } else {
                compo_label_set_font(lbl, UI_BUF_FONT_SYS);
            }
            compo_label_set(lbl, i18n[tbl_calendar_week_str[i]]);
            x_pos += (CALE_CELL_W + CALE_GAP_X);
        }
    }

    {
        compo_shape_t *hl = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
        compo_shape_set_color(hl, COLOR_WHITE);
        compo_shape_set_visible(hl, false);
        compo_setid(hl, COMPO_ID_TODAY_HIGHLIGHT);
    }

    for (i = 0; i < CALE_CELL_MAX; i++) {
        u8 row = i / CALE_GRID_COLS;
        u8 col = i % CALE_GRID_COLS;
        s16 x_pos = func_calendar_cell_x(col);
        s16 y_pos = func_calendar_cell_y(row);
        //compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_COMMON_NUM_30_26_BIN);

       // compo_picturebox_cut(pic, 1, 32);
       // compo_picturebox_set_pos(pic, x_pos, y_pos);
       // compo_setid(pic, COMPO_ID_DATE_PIC_START + i);
    }

    return frm;
}

static void func_calendar_rtc_tick(void)
{
    f_calendar_t *f = (f_calendar_t *)func_cb.f_cb;

    if (f == NULL) {
        return;
    }

    func_calendar_rtc_read(f);
    if (f->last_rtc_year != f->today_year || f->last_rtc_mon != f->today_mon
        || f->last_rtc_day != f->today_day) {
        f->last_rtc_year = f->today_year;
        f->last_rtc_mon = f->today_mon;
        f->last_rtc_day = f->today_day;

        f->update_year = f->today_year;
        f->update_mon = f->today_mon;
        func_calender_refresh(f);
    }
}

static void func_calendar_comm_process(void)
{
    func_calendar_rtc_tick();
    func_process();
}

static void func_calendar_message(size_msg_t msg)
{
    switch (msg) {
    case KU_RIGHT:                                      /* KEY2 返回上一级 */
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_back_to();
        }
        break;

    case KU_BACK:
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_back_to();
        }
        break;

    /* 禁止滑动退出（不交给 func_message） */
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_LONG:
    case MSG_CTP_LONG_UP:
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_calendar_enter(void)
{
    f_calendar_t *f;

    func_cb.f_cb = func_zalloc(sizeof(f_calendar_t));
    f = (f_calendar_t *)func_cb.f_cb;

    func_calendar_sync_today_view(f);
    f->last_rtc_year = f->today_year;
    f->last_rtc_mon = f->today_mon;
    f->last_rtc_day = f->today_day;

    func_cb.frm_main = func_calender_form_create();
    func_calender_refresh(f);
    func_cb.enter_tick = tick_get();
}

void func_calendar_exit(void)
{
    func_cb.last = FUNC_CALENDAER;
}

void func_calendar(void)
{
    printf("%s\n", __func__);
    func_calendar_enter();
    while (func_cb.sta == FUNC_CALENDAER) {
        func_calendar_comm_process();
        func_calendar_message(msg_dequeue());
    }
    func_calendar_exit();
}
