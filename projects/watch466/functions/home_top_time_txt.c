#include "include.h"
#include "home_top_time_txt.h"

#ifndef UI_BUF_0FONT_FONT_TEST_BIN
#error "UI_BUF_0FONT_FONT_TEST_BIN missing: add font_test.bin to ui.bin then Output/bin/prebuild.bat"
#endif

#define HOME_TOP_TIME_TXT_FONT              UI_BUF_0FONT_FONT_TEST_BIN
#define HOME_TOP_TIME_TXT_W                 120
#define HOME_TOP_TIME_TXT_H                 36

static void home_top_time_txt_parse(tm_t *tm, u8 *hour12, u8 *min, bool *is_pm)
{
    u8 hour = tm->hour;

    *is_pm = false;
    if (hour >= 12) {
        *is_pm = true;
        if (hour > 12) {
            hour -= 12;
        }
    }
    if (hour == 0) {
        hour = 12;
    }
    *hour12 = hour;
    *min = tm->min;
}

static u16 home_top_time_txt_key(tm_t *tm)
{
    u8 hour12;
    u8 min;
    bool is_pm;

    home_top_time_txt_parse(tm, &hour12, &min, &is_pm);
    return (u16)hour12 | ((u16)min << 8) | (is_pm ? 0x8000 : 0);
}

static void home_top_time_txt_format(char *buf, u16 buf_size, tm_t *tm)
{
    u8 hour12;
    u8 min;
    bool is_pm;

    if (buf == NULL || buf_size == 0 || tm == NULL) {
        return;
    }
    home_top_time_txt_parse(tm, &hour12, &min, &is_pm);
    snprintf(buf, buf_size, "%u:%02u%s", (unsigned)hour12, (unsigned)min, is_pm ? "PM" : "AM");
}

static void home_top_time_txt_font_once(home_top_time_txt_t *ui)
{
    if (ui == NULL || ui->txt == NULL || ui->font_ready) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    WDT_CLR();
#endif
    compo_textbox_set_font(ui->txt, HOME_TOP_TIME_TXT_FONT);
    ui->font_ready = true;
}

void home_top_time_txt_create(compo_form_t *frm, u16 id)
{
    compo_textbox_t *txt;

    if (frm == NULL) {
        return;
    }
    txt = compo_textbox_create(frm, 12);
    compo_setid(txt, id);
    compo_textbox_set_wholewrap(txt, false);
#if ELUNCHBOX_PANEL_EN
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_visible(txt, false);
#else
    compo_textbox_set_font(txt, HOME_TOP_TIME_TXT_FONT);
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
#endif
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_pos(txt, HOME_TOP_TIME_X, HOME_TOP_TIME_Y);
    compo_textbox_set_location(txt, HOME_TOP_TIME_X, HOME_TOP_TIME_Y,
                               HOME_TOP_TIME_TXT_W, HOME_TOP_TIME_TXT_H);
}

void home_top_time_txt_bind(home_top_time_txt_t *ui, u16 id)
{
    if (ui == NULL) {
        return;
    }
    ui->txt = compo_getobj_byid(id);
    ui->font_ready = false;
    ui->last_key = 0xffff;
}

bool home_top_time_txt_refresh(home_top_time_txt_t *ui, tm_t *tm)
{
    char buf[12];
    widget_text_t *widget;
    u16 key;

    if (ui == NULL || ui->txt == NULL || tm == NULL) {
        return false;
    }

    key = home_top_time_txt_key(tm);
    if (ui->last_key == key) {
        return false;
    }

    home_top_time_txt_font_once(ui);
#if ELUNCHBOX_PANEL_EN
    WDT_CLR();
#endif

    widget = ui->txt->txt;
    compo_textbox_set_align_center(ui->txt, false);
    widget_set_align_center(widget, false);
    compo_textbox_set_wholewrap(ui->txt, false);
    compo_textbox_set_autosize(ui->txt, false);
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_location(ui->txt, HOME_TOP_TIME_X, HOME_TOP_TIME_Y,
                               HOME_TOP_TIME_TXT_W, HOME_TOP_TIME_TXT_H);
    compo_textbox_set_forecolor(ui->txt, COLOR_BLACK);
    home_top_time_txt_format(buf, sizeof(buf), tm);
#if ELUNCHBOX_PANEL_EN
    widget_text_set(ui->txt->txt, buf);
#else
    compo_textbox_set(ui->txt, buf);
#endif
    compo_textbox_set_visible(ui->txt, true);
    if (widget != NULL) {
        widget_set_top(widget, true);
    }
    ui->last_key = key;
    return true;
}

bool home_top_time_txt_tick(home_top_time_txt_t *ui, u8 *last_min, u8 *last_sec)
{
    tm_t tm;

    if (ui == NULL || last_min == NULL || last_sec == NULL) {
        return false;
    }
    tm = rtc_clock_get();
    if (*last_min == tm.min && *last_sec == tm.sec) {
        return false;
    }
    *last_min = tm.min;
    *last_sec = tm.sec;
    return home_top_time_txt_refresh(ui, &tm);
}

void home_top_time_txt_force(home_top_time_txt_t *ui, u8 *last_min, u8 *last_sec)
{
    tm_t tm;

    if (ui == NULL) {
        return;
    }
    tm = rtc_clock_get();
    if (last_min != NULL) {
        *last_min = tm.min;
    }
    if (last_sec != NULL) {
        *last_sec = tm.sec;
    }
    ui->last_key = 0xffff;
    home_top_time_txt_refresh(ui, &tm);
}

void home_top_time_txt_keep_visible(home_top_time_txt_t *ui)
{
    char buf[12];
    tm_t tm;
    widget_text_t *widget;

    if (ui == NULL || ui->txt == NULL || ui->last_key == 0xffff) {
        return;
    }

    home_top_time_txt_font_once(ui);
    tm = rtc_clock_get();
    home_top_time_txt_format(buf, sizeof(buf), &tm);
    widget = ui->txt->txt;
    compo_textbox_set_align_center(ui->txt, false);
    if (widget != NULL) {
        widget_set_align_center(widget, false);
        widget_text_set_ellipsis(widget, false);
    }
    compo_textbox_set_forecolor(ui->txt, COLOR_BLACK);
    compo_textbox_set_visible(ui->txt, true);
#if ELUNCHBOX_PANEL_EN
    if (widget != NULL) {
        widget_text_set(widget, buf);
        widget_set_top(widget, true);
    }
#else
    compo_textbox_set(ui->txt, buf);
#endif
}
