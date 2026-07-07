#include "include.h"
#include "func.h"
#include "new_time_res.h"
#include "new_home_icon_res.h"
#include "home_icon_res.h"
#include "home_ui_shared.h"
#include "func_key_lock.h"
#include "func_lunchbox_uart.h"
#include "ui_layout_anchor.h"

#if ELUNCHBOX_PANEL_EN
extern volatile u8 elunchbox_te_block_flag;
#endif

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

#ifndef UI_BUF_NEW_UI_NEW_UP_BIN
#error "Run tools/gen_new_time_icons.py then Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_0FONT_FONT_TEST_BIN
#error "UI_BUF_0FONT_FONT_TEST_BIN missing: add font_test.bin to ui.bin then Output/bin/prebuild.bat"
#endif
#define NEW_HEAT_FONT                       UI_BUF_0FONT_FONT_TEST_BIN

/*
 * 时间设置页 — 浅色底；时钟数字/冒号/H/Min 用 textbox（无 bitmap 黑底）
 * PT8028：TCH4 确认 | TCH5 开关 | TCH2/TCH6 减/加 | TCH3 模式(AM/PM/NO·YES)
 */
#define NEW_TIME_COLON_W                   8
#define NEW_TIME_CLOCK_TXT_W               24
#define NEW_TIME_DIGIT_GAP                 NEW_TIME_SX(4)

#define NEW_TIME_REF_W                     466
#define NEW_TIME_REF_H                     466
#define NEW_TIME_SX(v)                     ((s16)((s32)(v) * GUI_SCREEN_WIDTH / NEW_TIME_REF_W))
#define NEW_TIME_SY(v)                     ((s16)((s32)(v) * GUI_SCREEN_HEIGHT / NEW_TIME_REF_H))

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define NEW_TIME_STATUS_Y                  22
#define NEW_TIME_STATUS_RIGHT_MARGIN       10
#define NEW_TIME_STATUS_GAP                6
#define NEW_TIME_TITLE_LEFT                28
#define NEW_TIME_TITLE_Y                   15
#define NEW_TIME_TITLE_H                   36
#define NEW_TIME_TITLE_TOP                 (NEW_TIME_TITLE_Y - NEW_TIME_TITLE_H / 2)
#define NEW_TIME_DIVIDER_Y                 44
#define NEW_TIME_DIVIDER_H                 1
#define NEW_TIME_HOUR_COL_X                76
#define NEW_TIME_MIN_COL_X                 182
#define NEW_TIME_COLON_X                   129
#define NEW_TIME_AMPM_COL_X                270
#define NEW_TIME_BOX_Y                     118
#define NEW_TIME_BOX_W                     78
#define NEW_TIME_BOX_H                     75
#define NEW_TIME_BOX_RADIUS                8
#define NEW_TIME_ARROW_UP_Y                71
#define NEW_TIME_ARROW_DOWN_Y              165
#define NEW_TIME_AM_Y                      100
#define NEW_TIME_PM_Y                      136
#define NEW_TIME_BTN_W                     118
#define NEW_TIME_BTN_H                     44
#define NEW_TIME_BTN_RADIUS                8
#define NEW_TIME_BTN_NO_X                  95
#define NEW_TIME_BTN_YES_X                 225
#define NEW_TIME_BTN_BOTTOM_Y              219
#define NEW_TIME_AMPM_W                    48
#define NEW_TIME_AMPM_H                    28
#define NEW_TIME_TITLE_W                   (NEW_TIME_STATUS_BT_X - HOME_STATUS_BT_W / 2 - NEW_TIME_STATUS_GAP - NEW_TIME_TITLE_LEFT)
#define NEW_TIME_PANEL_Y                   142
#define NEW_TIME_PANEL_W                   280
#define NEW_TIME_PANEL_H                   210
#else
#define NEW_TIME_STATUS_Y                  NEW_TIME_SY(48)
#define NEW_TIME_STATUS_RIGHT_MARGIN       NEW_TIME_SX(24)
#define NEW_TIME_STATUS_GAP                NEW_TIME_SX(10)
#define NEW_TIME_TITLE_LEFT                NEW_TIME_SX(58)
#define NEW_TIME_TITLE_Y                   NEW_TIME_SY(42)
#define NEW_TIME_TITLE_H                   NEW_TIME_SY(36)
#define NEW_TIME_TITLE_TOP                 (NEW_TIME_TITLE_Y - NEW_TIME_TITLE_H / 2)
#define NEW_TIME_DIVIDER_Y                 NEW_TIME_SY(90)
#define NEW_TIME_DIVIDER_H                 1
#define NEW_TIME_HOUR_COL_X                NEW_TIME_SX(108)
#define NEW_TIME_MIN_COL_X                 NEW_TIME_SX(268)
#define NEW_TIME_COLON_X                   NEW_TIME_SX(188)
#define NEW_TIME_AMPM_COL_X                NEW_TIME_SX(396)
#define NEW_TIME_BOX_Y                     NEW_TIME_SY(210)
#define NEW_TIME_BOX_W                     NEW_TIME_SX(96)
#define NEW_TIME_BOX_H                     NEW_TIME_SY(92)
#define NEW_TIME_BOX_RADIUS                NEW_TIME_SX(8)
#define NEW_TIME_ARROW_UP_Y                NEW_TIME_SY(118)
#define NEW_TIME_ARROW_DOWN_Y              NEW_TIME_SY(302)
#define NEW_TIME_AM_Y                      NEW_TIME_SY(178)
#define NEW_TIME_PM_Y                      NEW_TIME_SY(248)
#define NEW_TIME_BTN_W                     NEW_TIME_SX(145)
#define NEW_TIME_BTN_H                     NEW_TIME_SY(48)
#define NEW_TIME_BTN_RADIUS                NEW_TIME_SX(8)
#define NEW_TIME_BTN_NO_X                  NEW_TIME_SX(118)
#define NEW_TIME_BTN_YES_X                 NEW_TIME_SX(348)
#define NEW_TIME_BTN_BOTTOM_Y              NEW_TIME_SY(418)
#define NEW_TIME_AMPM_W                    NEW_TIME_SX(56)
#define NEW_TIME_AMPM_H                    NEW_TIME_SY(32)
#define NEW_TIME_TITLE_W                   NEW_TIME_SX(220)
#define NEW_TIME_PANEL_Y                   NEW_TIME_SY(144)
#define NEW_TIME_PANEL_W                   NEW_TIME_SX(280)
#define NEW_TIME_PANEL_H                   NEW_TIME_SY(240)
#endif

#define NEW_TIME_SUFFIX_TR_Y               ((s16)((s32)130 * GUI_SCREEN_HEIGHT / HEAT_LAYOUT_REF_H))
#define NEW_TIME_SUFFIX_H_TR_X             ((s16)((s32)100 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define NEW_TIME_SUFFIX_MIN_TR_X           ((s16)((s32)210 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))

#define NEW_TIME_STATUS_BAT_X              (GUI_SCREEN_WIDTH - NEW_TIME_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define NEW_TIME_STATUS_BT_X               (NEW_TIME_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - NEW_TIME_STATUS_GAP - NEW_HOME_BT_W / 2)

#define NEW_TIME_PANEL_BG                  0xEF5D
#define NEW_TIME_COLOR_TITLE               COLOR_BLACK
#define NEW_TIME_COLOR_ON                  COLOR_WHITE
#define NEW_TIME_COLOR_OFF                 COLOR_BLACK
#define NEW_TIME_COLOR_SEL                 0x0AD8
#define NEW_TIME_COLOR_BOX_NOR             0xCFDF
#ifndef NEW_TIME_BOX_RADIUS
#define NEW_TIME_BOX_RADIUS                12
#endif
#define NEW_TIME_BTN_RADIUS                8
#define NEW_TIME_AMPM_RADIUS               6

enum {
    NEW_TIME_FOCUS_HOUR = 0,
    NEW_TIME_FOCUS_MIN,
    NEW_TIME_FOCUS_AMPM,
    NEW_TIME_FOCUS_BOTTOM,
};

enum {
    COMPO_ID_SHAPE_BG = 1,
    COMPO_ID_SHAPE_PANEL,
    COMPO_ID_TXT_TITLE,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_BAT,
    COMPO_ID_SHAPE_HOUR_BG,
    COMPO_ID_SHAPE_MIN_BORDER,
    COMPO_ID_SHAPE_MIN_BG,
    COMPO_ID_PIC_HOUR_UP,
    COMPO_ID_PIC_HOUR_DOWN,
    COMPO_ID_PIC_MIN_UP,
    COMPO_ID_PIC_MIN_DOWN,
    COMPO_ID_TXT_H10,
    COMPO_ID_TXT_H1,
    COMPO_ID_TXT_COLON,
    COMPO_ID_TXT_M10,
    COMPO_ID_TXT_M1,
    COMPO_ID_SHAPE_AM,
    COMPO_ID_SHAPE_PM,
    COMPO_ID_SHAPE_NO,
    COMPO_ID_SHAPE_YES,
    COMPO_ID_TXT_H_SUFFIX,
    COMPO_ID_TXT_MIN_SUFFIX,
    COMPO_ID_TXT_AM,
    COMPO_ID_TXT_PM,
    COMPO_ID_TXT_NO,
    COMPO_ID_TXT_YES,
};

enum {
    NEW_TIME_LOAD_STATUS = 0,
    NEW_TIME_LOAD_SHAPES,
    NEW_TIME_LOAD_ARROWS,
    NEW_TIME_LOAD_CLOCK,
    NEW_TIME_LOAD_TEXT,
    NEW_TIME_LOAD_DONE,
};

typedef struct {
    u8 disp_h;
    u8 min;
    u8 focus;
    u8 bottom_sel;
    bool is_pm;
    bool display_pending;
#if ELUNCHBOX_PANEL_EN
    u8 load_stage;
    bool key_ready;
#endif
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_bat;
    compo_shape_t *shape_hour_bg;
    compo_shape_t *shape_min_border;
    compo_shape_t *shape_min_bg;
    compo_shape_t *shape_am;
    compo_shape_t *shape_pm;
    compo_shape_t *shape_no;
    compo_shape_t *shape_yes;
    compo_picturebox_t *pic_hour_up;
    compo_picturebox_t *pic_hour_down;
    compo_picturebox_t *pic_min_up;
    compo_picturebox_t *pic_min_down;
    compo_textbox_t *txt_title;
    compo_textbox_t *txt_h10;
    compo_textbox_t *txt_h1;
    compo_textbox_t *txt_colon;
    compo_textbox_t *txt_m10;
    compo_textbox_t *txt_m1;
    compo_textbox_t *txt_h_suffix;
    compo_textbox_t *txt_min_suffix;
    compo_textbox_t *txt_am;
    compo_textbox_t *txt_pm;
    compo_textbox_t *txt_no;
    compo_textbox_t *txt_yes;
} f_new_time_t;

#if ELUNCHBOX_PANEL_EN
/* 仅箭头用小缓冲 */
static u8 new_time_arrow_up_ram[NEW_TIME_NEW_UP_RAM_SIZE];
static u8 new_time_arrow_down_ram[NEW_TIME_NEW_DOWN_RAM_SIZE];
static bool new_time_res_up_ready;
static bool new_time_res_down_ready;
static bool new_time_font_ready;

static void new_time_font_bind_txt(compo_textbox_t *txt)
{
    if (txt != NULL) {
        compo_textbox_set_font(txt, NEW_HEAT_FONT);
    }
}

static void new_time_font_apply_once(f_new_time_t *f)
{
    if (new_time_font_ready || f == NULL) {
        return;
    }

    WDT_CLR();
    new_time_font_bind_txt(f->txt_title);
    WDT_CLR();
    new_time_font_bind_txt(f->txt_h10);
    WDT_CLR();
    new_time_font_bind_txt(f->txt_h1);
    WDT_CLR();
    new_time_font_bind_txt(f->txt_colon);
    WDT_CLR();
    new_time_font_bind_txt(f->txt_m10);
    WDT_CLR();
    new_time_font_bind_txt(f->txt_m1);
    WDT_CLR();
    new_time_font_bind_txt(f->txt_h_suffix);
    WDT_CLR();
    new_time_font_bind_txt(f->txt_min_suffix);
    WDT_CLR();
    new_time_font_bind_txt(f->txt_am);
    WDT_CLR();
    new_time_font_bind_txt(f->txt_pm);
    WDT_CLR();
    new_time_font_bind_txt(f->txt_no);
    WDT_CLR();
    new_time_font_bind_txt(f->txt_yes);
    new_time_font_ready = true;
}

static const char *new_time_stage_name(u8 stage)
{
    switch (stage) {
    case NEW_TIME_LOAD_STATUS:   return "status";
    case NEW_TIME_LOAD_SHAPES:   return "shapes";
    case NEW_TIME_LOAD_ARROWS:   return "arrows";
    case NEW_TIME_LOAD_CLOCK:    return "clock";
    case NEW_TIME_LOAD_TEXT:     return "text";
    case NEW_TIME_LOAD_DONE:     return "done";
    default:                     return "?";
    }
}
#endif

static void new_time_parse_rtc(u8 *disp_h, u8 *min, bool *is_pm)
{
    tm_t tm = rtc_clock_get();
    u8 hour = tm.hour;

    *is_pm = (hour >= 12);
    *disp_h = (u8)(hour % 12);
    if (*disp_h == 0) {
        *disp_h = 12;
    }
    *min = tm.min;
}

static u8 new_time_to_hour24(u8 disp_h, bool is_pm)
{
    if (is_pm) {
        return (disp_h == 12) ? 12 : (u8)(disp_h + 12);
    }
    return (disp_h == 12) ? 0 : disp_h;
}

static void new_time_save_rtc(f_new_time_t *f)
{
    tm_t tm_set;

    if (f == NULL) {
        return;
    }
    tm_set = rtc_clock_get();
    tm_set.hour = new_time_to_hour24(f->disp_h, f->is_pm);
    tm_set.min = f->min;
    rtc_clock_set(tm_set);
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_time_sync(RTCCNT + LB_RTC_UNIX_OFFSET);
#endif
}

static void new_time_pic_pos_tr(compo_picturebox_t *pic, s16 tr_x, s16 tr_y, u16 w, u16 h)
{
    if (pic == NULL) {
        return;
    }
    compo_picturebox_set_pos(pic, tr_x - (s16)(w / 2), tr_y + (s16)(h / 2));
}

static void new_time_txt_pos_center(compo_textbox_t *txt, s16 cx, s16 cy, s16 w, s16 h)
{
    if (txt == NULL) {
        return;
    }
    compo_textbox_set_location(txt, cx, cy, w, h);
}

static s16 new_time_digit_pair_offset(void)
{
    return (s16)((NEW_TIME_CLOCK_TXT_W + NEW_TIME_DIGIT_GAP) / 2);
}

static void new_time_label_show(compo_textbox_t *txt, const char *label, u16 color);

static void new_time_clock_digit_show(compo_textbox_t *txt, u8 digit,
                                      s16 center_x, s16 center_y, u16 color)
{
    char buf[2];
    widget_text_t *widget;

    if (txt == NULL || digit > 9) {
        return;
    }
    buf[0] = (char)('0' + digit);
    buf[1] = '\0';
    widget = txt->txt;
    new_time_txt_pos_center(txt, center_x, center_y,
                            NEW_TIME_CLOCK_TXT_W, NEW_TIME_BOX_H);
    compo_textbox_set_align_center(txt, true);
    if (widget != NULL) {
        widget_set_align_center(widget, true);
        widget_text_set_ellipsis(widget, false);
    }
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set(txt, buf);
    compo_textbox_set_visible(txt, true);
    if (widget != NULL) {
        widget_set_top(widget, true);
    }
}

#if ELUNCHBOX_PANEL_EN
static compo_picturebox_t *new_time_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);

    pic->img = (void *)widget_image_create(frm->page_body, 0);
    pic->radix = 1;
    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

static compo_shape_t *new_time_shape_create(compo_form_t *frm, u16 id,
                                            s16 x, s16 y, s16 w, s16 h,
                                            u16 color, u16 radius)
{
    compo_shape_t *shape = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(shape, id);
    compo_shape_set_location(shape, x, y, w, h);
    compo_shape_set_color(shape, color);
    compo_shape_set_radius(shape, radius);
    compo_shape_set_visible(shape, false);
    return shape;
}

#endif

static compo_textbox_t *new_time_txt_create(compo_form_t *frm, u16 id, u16 buf_size,
                                            s16 x, s16 y, u16 w, u16 h, u16 color, bool center)
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

static void new_time_title_show(compo_textbox_t *txt)
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
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_location(txt, NEW_TIME_TITLE_LEFT, NEW_TIME_TITLE_TOP,
                               NEW_TIME_TITLE_W, NEW_TIME_TITLE_H);
    compo_textbox_set_forecolor(txt, NEW_TIME_COLOR_TITLE);
    compo_textbox_set(txt, "TIME");
    compo_textbox_set_visible(txt, true);
}

static void new_time_label_show(compo_textbox_t *txt, const char *label, u16 color)
{
    widget_text_t *widget;

    if (txt == NULL) {
        return;
    }
    widget = txt->txt;
    compo_textbox_set_align_center(txt, true);
    if (widget != NULL) {
        widget_set_align_center(widget, true);
    }
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    if (widget != NULL) {
        widget_text_set_ellipsis(widget, false);
    }
    compo_textbox_set(txt, label);
    compo_textbox_set_visible(txt, true);
    if (widget != NULL) {
        widget_set_top(widget, true);
    }
}

static void new_time_shapes_apply(f_new_time_t *f)
{
    bool hour_sel;
    bool min_sel;

    if (f == NULL) {
        return;
    }
    hour_sel = (f->focus == NEW_TIME_FOCUS_HOUR);
    min_sel = (f->focus == NEW_TIME_FOCUS_MIN);

#if ELUNCHBOX_PANEL_EN
    if (f->shape_hour_bg != NULL) {
        compo_shape_set_color(f->shape_hour_bg, NEW_TIME_COLOR_SEL);
        compo_shape_set_visible(f->shape_hour_bg, hour_sel);
    }
    if (f->shape_min_border != NULL) {
        compo_shape_set_color(f->shape_min_border, NEW_TIME_COLOR_BOX_NOR);
        compo_shape_set_visible(f->shape_min_border, true);
    }
    if (f->shape_min_bg != NULL) {
        compo_shape_set_color(f->shape_min_bg,
                              min_sel ? NEW_TIME_COLOR_SEL : NEW_TIME_COLOR_BOX_NOR);
        compo_shape_set_visible(f->shape_min_bg, true);
    }
    if (f->shape_am != NULL) {
        compo_shape_set_color(f->shape_am,
                              (!f->is_pm) ? NEW_TIME_COLOR_SEL : NEW_TIME_COLOR_BOX_NOR);
        compo_shape_set_visible(f->shape_am, true);
    }
    if (f->shape_pm != NULL) {
        compo_shape_set_color(f->shape_pm,
                              f->is_pm ? NEW_TIME_COLOR_SEL : NEW_TIME_COLOR_BOX_NOR);
        compo_shape_set_visible(f->shape_pm, true);
    }
    if (f->focus == NEW_TIME_FOCUS_BOTTOM && f->bottom_sel == 0) {
        if (f->shape_no != NULL) {
            compo_shape_set_color(f->shape_no, NEW_TIME_COLOR_SEL);
            compo_shape_set_visible(f->shape_no, true);
        }
        if (f->shape_yes != NULL) {
            compo_shape_set_color(f->shape_yes, NEW_TIME_COLOR_BOX_NOR);
            compo_shape_set_visible(f->shape_yes, true);
        }
    } else if (f->focus == NEW_TIME_FOCUS_BOTTOM && f->bottom_sel != 0) {
        if (f->shape_no != NULL) {
            compo_shape_set_color(f->shape_no, NEW_TIME_COLOR_BOX_NOR);
            compo_shape_set_visible(f->shape_no, true);
        }
        if (f->shape_yes != NULL) {
            compo_shape_set_color(f->shape_yes, NEW_TIME_COLOR_SEL);
            compo_shape_set_visible(f->shape_yes, true);
        }
    } else {
        if (f->shape_no != NULL) {
            compo_shape_set_color(f->shape_no, NEW_TIME_COLOR_BOX_NOR);
            compo_shape_set_visible(f->shape_no, true);
        }
        if (f->shape_yes != NULL) {
            compo_shape_set_color(f->shape_yes, NEW_TIME_COLOR_SEL);
            compo_shape_set_visible(f->shape_yes, true);
        }
    }
#endif
}

static void new_time_arrow_pic_bind(compo_picturebox_t *pic, u8 *ram, s16 x, s16 y, u16 w, u16 h)
{
    if (pic == NULL || ram == NULL) {
        return;
    }
    compo_picturebox_set_ram(pic, ram);
    compo_picturebox_set_size(pic, w, h);
    compo_picturebox_set_pos(pic, x, y);
    compo_picturebox_set_visible(pic, true);
}

static void new_time_arrows_apply(f_new_time_t *f)
{
    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (!new_time_res_up_ready) {
        os_spiflash_read(new_time_arrow_up_ram, UI_BUF_NEW_UI_NEW_UP_BIN, UI_LEN_NEW_UI_NEW_UP_BIN);
        if (gui_set_ram_check(new_time_arrow_up_ram, __func__)) {
            new_time_res_up_ready = true;
            printf("nt_arr: up ok len=%u\n", (unsigned)UI_LEN_NEW_UI_NEW_UP_BIN);
        }
    }
    if (!new_time_res_down_ready) {
        os_spiflash_read(new_time_arrow_down_ram, UI_BUF_NEW_UI_NEW_DOWN_BIN, UI_LEN_NEW_UI_NEW_DOWN_BIN);
        if (gui_set_ram_check(new_time_arrow_down_ram, __func__)) {
            new_time_res_down_ready = true;
            printf("nt_arr: down ok len=%u\n", (unsigned)UI_LEN_NEW_UI_NEW_DOWN_BIN);
        }
    }
    if (new_time_res_up_ready) {
        new_time_arrow_pic_bind(f->pic_hour_up, new_time_arrow_up_ram,
                                NEW_TIME_HOUR_COL_X, NEW_TIME_ARROW_UP_Y,
                                NEW_TIME_NEW_UP_W, NEW_TIME_NEW_UP_H);
        new_time_arrow_pic_bind(f->pic_min_up, new_time_arrow_up_ram,
                                NEW_TIME_MIN_COL_X, NEW_TIME_ARROW_UP_Y,
                                NEW_TIME_NEW_UP_W, NEW_TIME_NEW_UP_H);
    }
    if (new_time_res_down_ready) {
        new_time_arrow_pic_bind(f->pic_hour_down, new_time_arrow_down_ram,
                                NEW_TIME_HOUR_COL_X, NEW_TIME_ARROW_DOWN_Y,
                                NEW_TIME_NEW_DOWN_W, NEW_TIME_NEW_DOWN_H);
        new_time_arrow_pic_bind(f->pic_min_down, new_time_arrow_down_ram,
                                NEW_TIME_MIN_COL_X, NEW_TIME_ARROW_DOWN_Y,
                                NEW_TIME_NEW_DOWN_W, NEW_TIME_NEW_DOWN_H);
    }
#endif
}

static void new_time_clock_apply(f_new_time_t *f)
{
    u8 h10;
    u8 h1;
    u8 m10;
    u8 m1;
    u16 hour_color;
    u16 min_color;

    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    new_time_font_apply_once(f);
#endif
    h10 = (u8)(f->disp_h / 10);
    h1 = (u8)(f->disp_h % 10);
    m10 = (u8)(f->min / 10);
    m1 = (u8)(f->min % 10);
    hour_color = (f->focus == NEW_TIME_FOCUS_HOUR) ? NEW_TIME_COLOR_ON : NEW_TIME_COLOR_OFF;
    min_color = (f->focus == NEW_TIME_FOCUS_MIN) ? NEW_TIME_COLOR_ON : NEW_TIME_COLOR_OFF;

    {
        s16 pair_off = new_time_digit_pair_offset();

        new_time_clock_digit_show(f->txt_h10, h10,
                                  (s16)(NEW_TIME_HOUR_COL_X - pair_off), NEW_TIME_BOX_Y, hour_color);
        new_time_clock_digit_show(f->txt_h1, h1,
                                  (s16)(NEW_TIME_HOUR_COL_X + pair_off), NEW_TIME_BOX_Y, hour_color);
        new_time_txt_pos_center(f->txt_colon, NEW_TIME_COLON_X, NEW_TIME_BOX_Y,
                                NEW_TIME_COLON_W, NEW_TIME_BOX_H);
        new_time_label_show(f->txt_colon, ":", NEW_TIME_COLOR_OFF);
        new_time_clock_digit_show(f->txt_m10, m10,
                                  (s16)(NEW_TIME_MIN_COL_X - pair_off), NEW_TIME_BOX_Y, min_color);
        new_time_clock_digit_show(f->txt_m1, m1,
                                  (s16)(NEW_TIME_MIN_COL_X + pair_off), NEW_TIME_BOX_Y, min_color);
    }
    new_time_txt_pos_center(f->txt_h_suffix, NEW_TIME_SUFFIX_H_TR_X, NEW_TIME_SUFFIX_TR_Y,
                            HOME_TIMEING_HM_W, HOME_TIMEING_HM_H);
    new_time_label_show(f->txt_h_suffix, "H", NEW_TIME_COLOR_OFF);
    new_time_txt_pos_center(f->txt_min_suffix, NEW_TIME_SUFFIX_MIN_TR_X, NEW_TIME_SUFFIX_TR_Y,
                            HOME_TIMEING_MINM_W, HOME_TIMEING_MINM_H);
    new_time_label_show(f->txt_min_suffix, "min", NEW_TIME_COLOR_OFF);
}

static void new_time_digits_apply(f_new_time_t *f)
{
    new_time_clock_apply(f);
}

static void new_time_text_apply(f_new_time_t *f)
{
    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    new_time_font_apply_once(f);
#endif
    new_time_title_show(f->txt_title);
    new_time_label_show(f->txt_am, "AM", f->is_pm ? NEW_TIME_COLOR_OFF : NEW_TIME_COLOR_ON);
    new_time_label_show(f->txt_pm, "PM", f->is_pm ? NEW_TIME_COLOR_ON : NEW_TIME_COLOR_OFF);
    if (f->focus == NEW_TIME_FOCUS_BOTTOM && f->bottom_sel == 0) {
        new_time_label_show(f->txt_no, "NO", NEW_TIME_COLOR_ON);
        new_time_label_show(f->txt_yes, "YES", NEW_TIME_COLOR_OFF);
    } else if (f->focus == NEW_TIME_FOCUS_BOTTOM && f->bottom_sel != 0) {
        new_time_label_show(f->txt_no, "NO", NEW_TIME_COLOR_OFF);
        new_time_label_show(f->txt_yes, "YES", NEW_TIME_COLOR_ON);
    } else {
        new_time_label_show(f->txt_no, "NO", NEW_TIME_COLOR_OFF);
        new_time_label_show(f->txt_yes, "YES", NEW_TIME_COLOR_ON);
    }
}

static void new_time_status_refresh(f_new_time_t *f)
{
    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (f->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_pos(f->pic_bt, NEW_TIME_STATUS_BT_X, NEW_TIME_STATUS_Y);
        home_ui_shared_status_refresh_bt(f->pic_bt);
    }
    if (f->pic_bat != NULL) {
        compo_picturebox_set_pos(f->pic_bat, NEW_TIME_STATUS_BAT_X, NEW_TIME_STATUS_Y);
        home_ui_shared_battery_attach_pic(f->pic_bat);
    }
#endif
}

static void new_time_ui_apply(f_new_time_t *f)
{
    if (f == NULL) {
        return;
    }
    new_time_status_refresh(f);
    new_time_shapes_apply(f);
    new_time_arrows_apply(f);
    new_time_digits_apply(f);
    new_time_text_apply(f);
}

static void new_time_focus_refresh(f_new_time_t *f)
{
    if (f == NULL) {
        return;
    }
    new_time_shapes_apply(f);
    new_time_digits_apply(f);
    new_time_text_apply(f);
}

#if ELUNCHBOX_PANEL_EN
static void new_time_gpu_detach_before_leave(f_new_time_t *f)
{
    (void)f;
    home_ui_shared_battery_detach_pic();
}
#endif

static void new_time_value_inc(f_new_time_t *f)
{
    if (f == NULL) {
        return;
    }
    switch (f->focus) {
    case NEW_TIME_FOCUS_HOUR:
        f->disp_h = (u8)(f->disp_h % 12) + 1;
        new_time_focus_refresh(f);
        break;
    case NEW_TIME_FOCUS_MIN:
        f->min = (u8)((f->min + 1) % 60);
        new_time_focus_refresh(f);
        break;
    case NEW_TIME_FOCUS_AMPM:
        f->is_pm = !f->is_pm;
        new_time_shapes_apply(f);
        new_time_text_apply(f);
        break;
    case NEW_TIME_FOCUS_BOTTOM:
        f->bottom_sel = (u8)((f->bottom_sel + 1) & 1);
        new_time_focus_refresh(f);
        break;
    default:
        break;
    }
}

static void new_time_value_dec(f_new_time_t *f)
{
    if (f == NULL) {
        return;
    }
    switch (f->focus) {
    case NEW_TIME_FOCUS_HOUR:
        f->disp_h = (f->disp_h == 1) ? 12 : (u8)(f->disp_h - 1);
        new_time_focus_refresh(f);
        break;
    case NEW_TIME_FOCUS_MIN:
        f->min = (u8)((f->min + 59) % 60);
        new_time_focus_refresh(f);
        break;
    case NEW_TIME_FOCUS_AMPM:
        f->is_pm = !f->is_pm;
        new_time_shapes_apply(f);
        new_time_text_apply(f);
        break;
    case NEW_TIME_FOCUS_BOTTOM:
        f->bottom_sel = (u8)((f->bottom_sel + 1) & 1);
        new_time_focus_refresh(f);
        break;
    default:
        break;
    }
}

static void new_time_ok_key(f_new_time_t *f)
{
    if (f == NULL) {
        return;
    }
    switch (f->focus) {
    case NEW_TIME_FOCUS_HOUR:
        f->focus = NEW_TIME_FOCUS_MIN;
        break;
    case NEW_TIME_FOCUS_MIN:
        f->focus = NEW_TIME_FOCUS_AMPM;
        break;
    case NEW_TIME_FOCUS_AMPM:
        f->focus = NEW_TIME_FOCUS_BOTTOM;
        f->bottom_sel = 0;
        break;
    case NEW_TIME_FOCUS_BOTTOM:
        if (f->bottom_sel != 0) {
            new_time_save_rtc(f);
        }
        func_switch_to(FUNC_NEW_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        return;
    default:
        break;
    }
    new_time_focus_refresh(f);
}

static void new_time_mode_key(f_new_time_t *f)
{
    (void)f;
    /* 模式键：跳转到模式选择页 */
    if (sys_cb.flag_swithing) {
        return;
    }
    func_switch_to(FUNC_NEW_MODE, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

static void new_time_power_key(f_new_time_t *f)
{
    if (f == NULL) {
        return;
    }
    switch (f->focus) {
    case NEW_TIME_FOCUS_HOUR:
        func_switch_to(FUNC_NEW_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;
    case NEW_TIME_FOCUS_MIN:
        f->focus = NEW_TIME_FOCUS_HOUR;
        new_time_focus_refresh(f);
        break;
    case NEW_TIME_FOCUS_AMPM:
        f->focus = NEW_TIME_FOCUS_MIN;
        new_time_focus_refresh(f);
        break;
    case NEW_TIME_FOCUS_BOTTOM:
        f->focus = NEW_TIME_FOCUS_AMPM;
        new_time_focus_refresh(f);
        break;
    default:
        break;
    }
}

static void new_time_bg_create(compo_form_t *frm)
{
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, COMPO_ID_SHAPE_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(bg, NEW_TIME_PANEL_BG);
    compo_shape_set_radius(bg, 0);
}

static void new_time_panel_create(compo_form_t *frm)
{
    compo_shape_t *panel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(panel, COMPO_ID_SHAPE_PANEL);
    compo_shape_set_location(panel, GUI_SCREEN_CENTER_X, NEW_TIME_PANEL_Y,
                             NEW_TIME_PANEL_W, NEW_TIME_PANEL_H);
    compo_shape_set_color(panel, COLOR_WHITE);
    compo_shape_set_radius(panel, 16);
}

static void new_time_bind_objects(f_new_time_t *f)
{
    if (f == NULL) {
        return;
    }
    f->txt_title = compo_getobj_byid(COMPO_ID_TXT_TITLE);
    f->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);
    f->shape_hour_bg = compo_getobj_byid(COMPO_ID_SHAPE_HOUR_BG);
    f->shape_min_border = compo_getobj_byid(COMPO_ID_SHAPE_MIN_BORDER);
    f->shape_min_bg = compo_getobj_byid(COMPO_ID_SHAPE_MIN_BG);
    f->shape_am = compo_getobj_byid(COMPO_ID_SHAPE_AM);
    f->shape_pm = compo_getobj_byid(COMPO_ID_SHAPE_PM);
    f->shape_no = compo_getobj_byid(COMPO_ID_SHAPE_NO);
    f->shape_yes = compo_getobj_byid(COMPO_ID_SHAPE_YES);
    f->pic_hour_up = compo_getobj_byid(COMPO_ID_PIC_HOUR_UP);
    f->pic_hour_down = compo_getobj_byid(COMPO_ID_PIC_HOUR_DOWN);
    f->pic_min_up = compo_getobj_byid(COMPO_ID_PIC_MIN_UP);
    f->pic_min_down = compo_getobj_byid(COMPO_ID_PIC_MIN_DOWN);
    f->txt_h10 = compo_getobj_byid(COMPO_ID_TXT_H10);
    f->txt_h1 = compo_getobj_byid(COMPO_ID_TXT_H1);
    f->txt_colon = compo_getobj_byid(COMPO_ID_TXT_COLON);
    f->txt_m10 = compo_getobj_byid(COMPO_ID_TXT_M10);
    f->txt_m1 = compo_getobj_byid(COMPO_ID_TXT_M1);
    f->txt_h_suffix = compo_getobj_byid(COMPO_ID_TXT_H_SUFFIX);
    f->txt_min_suffix = compo_getobj_byid(COMPO_ID_TXT_MIN_SUFFIX);
    f->txt_am = compo_getobj_byid(COMPO_ID_TXT_AM);
    f->txt_pm = compo_getobj_byid(COMPO_ID_TXT_PM);
    f->txt_no = compo_getobj_byid(COMPO_ID_TXT_NO);
    f->txt_yes = compo_getobj_byid(COMPO_ID_TXT_YES);
}

compo_form_t *func_new_time_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);

    new_time_bg_create(frm);
    new_time_panel_create(frm);

    (void)new_time_txt_create(frm, COMPO_ID_TXT_TITLE, 8,
                              NEW_TIME_TITLE_LEFT, NEW_TIME_TITLE_Y,
                              NEW_TIME_TITLE_W, NEW_TIME_TITLE_H,
                              NEW_TIME_COLOR_TITLE, false);

    (void)new_time_pic_create_hidden(frm, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(compo_getobj_byid(COMPO_ID_PIC_BT),
                             NEW_TIME_STATUS_BT_X, NEW_TIME_STATUS_Y);
    compo_picturebox_set_size(compo_getobj_byid(COMPO_ID_PIC_BT),
                              NEW_HOME_BT_W, NEW_HOME_BT_H);

    (void)new_time_pic_create_hidden(frm, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(compo_getobj_byid(COMPO_ID_PIC_BAT),
                             NEW_TIME_STATUS_BAT_X, NEW_TIME_STATUS_Y);
    compo_picturebox_set_size(compo_getobj_byid(COMPO_ID_PIC_BAT),
                              NEW_HOME_BAT_W, NEW_HOME_BAT_H);

    (void)new_time_shape_create(frm, COMPO_ID_SHAPE_HOUR_BG,
                                NEW_TIME_HOUR_COL_X, NEW_TIME_BOX_Y,
                                NEW_TIME_BOX_W, NEW_TIME_BOX_H,
                                NEW_TIME_COLOR_BOX_NOR, NEW_TIME_BOX_RADIUS);
    (void)new_time_shape_create(frm, COMPO_ID_SHAPE_MIN_BORDER,
                                NEW_TIME_MIN_COL_X, NEW_TIME_BOX_Y,
                                NEW_TIME_BOX_W, NEW_TIME_BOX_H,
                                NEW_TIME_COLOR_BOX_NOR, NEW_TIME_BOX_RADIUS);
    (void)new_time_shape_create(frm, COMPO_ID_SHAPE_MIN_BG,
                                NEW_TIME_MIN_COL_X, NEW_TIME_BOX_Y,
                                (s16)(NEW_TIME_BOX_W - 2), (s16)(NEW_TIME_BOX_H - 2),
                                NEW_TIME_COLOR_BOX_NOR, (u16)(NEW_TIME_BOX_RADIUS - 1));

    (void)new_time_pic_create_hidden(frm, COMPO_ID_PIC_HOUR_UP);
    (void)new_time_pic_create_hidden(frm, COMPO_ID_PIC_HOUR_DOWN);
    (void)new_time_pic_create_hidden(frm, COMPO_ID_PIC_MIN_UP);
    (void)new_time_pic_create_hidden(frm, COMPO_ID_PIC_MIN_DOWN);

    (void)new_time_txt_create(frm, COMPO_ID_TXT_H10, 2,
                              NEW_TIME_HOUR_COL_X, NEW_TIME_BOX_Y,
                              NEW_TIME_CLOCK_TXT_W, NEW_TIME_BOX_H,
                              NEW_TIME_COLOR_OFF, true);
    (void)new_time_txt_create(frm, COMPO_ID_TXT_H1, 2,
                              NEW_TIME_HOUR_COL_X, NEW_TIME_BOX_Y,
                              NEW_TIME_CLOCK_TXT_W, NEW_TIME_BOX_H,
                              NEW_TIME_COLOR_OFF, true);
    (void)new_time_txt_create(frm, COMPO_ID_TXT_COLON, 2,
                              NEW_TIME_COLON_X, NEW_TIME_BOX_Y,
                              NEW_TIME_COLON_W, NEW_TIME_BOX_H,
                              NEW_TIME_COLOR_OFF, true);
    (void)new_time_txt_create(frm, COMPO_ID_TXT_M10, 2,
                              NEW_TIME_MIN_COL_X, NEW_TIME_BOX_Y,
                              NEW_TIME_CLOCK_TXT_W, NEW_TIME_BOX_H,
                              NEW_TIME_COLOR_OFF, true);
    (void)new_time_txt_create(frm, COMPO_ID_TXT_M1, 2,
                              NEW_TIME_MIN_COL_X, NEW_TIME_BOX_Y,
                              NEW_TIME_CLOCK_TXT_W, NEW_TIME_BOX_H,
                              NEW_TIME_COLOR_OFF, true);
    (void)new_time_txt_create(frm, COMPO_ID_TXT_H_SUFFIX, 4,
                              NEW_TIME_SUFFIX_H_TR_X, NEW_TIME_SUFFIX_TR_Y,
                              HOME_TIMEING_HM_W, HOME_TIMEING_HM_H,
                              NEW_TIME_COLOR_OFF, true);
    (void)new_time_txt_create(frm, COMPO_ID_TXT_MIN_SUFFIX, 8,
                              NEW_TIME_SUFFIX_MIN_TR_X, NEW_TIME_SUFFIX_TR_Y,
                              HOME_TIMEING_MINM_W, HOME_TIMEING_MINM_H,
                              NEW_TIME_COLOR_OFF, true);

    (void)new_time_shape_create(frm, COMPO_ID_SHAPE_AM,
                                NEW_TIME_AMPM_COL_X, NEW_TIME_AM_Y,
                                NEW_TIME_AMPM_W, NEW_TIME_AMPM_H,
                                NEW_TIME_COLOR_BOX_NOR, NEW_TIME_AMPM_RADIUS);
    (void)new_time_shape_create(frm, COMPO_ID_SHAPE_PM,
                                NEW_TIME_AMPM_COL_X, NEW_TIME_PM_Y,
                                NEW_TIME_AMPM_W, NEW_TIME_AMPM_H,
                                NEW_TIME_COLOR_BOX_NOR, NEW_TIME_AMPM_RADIUS);
    (void)new_time_shape_create(frm, COMPO_ID_SHAPE_NO,
                                NEW_TIME_BTN_NO_X, NEW_TIME_BTN_BOTTOM_Y,
                                NEW_TIME_BTN_W, NEW_TIME_BTN_H,
                                NEW_TIME_COLOR_BOX_NOR, NEW_TIME_BTN_RADIUS);
    (void)new_time_shape_create(frm, COMPO_ID_SHAPE_YES,
                                NEW_TIME_BTN_YES_X, NEW_TIME_BTN_BOTTOM_Y,
                                NEW_TIME_BTN_W, NEW_TIME_BTN_H,
                                NEW_TIME_COLOR_SEL, NEW_TIME_BTN_RADIUS);

    (void)new_time_txt_create(frm, COMPO_ID_TXT_AM, 4,
                              NEW_TIME_AMPM_COL_X, NEW_TIME_AM_Y,
                              NEW_TIME_AMPM_W, NEW_TIME_AMPM_H,
                              NEW_TIME_COLOR_ON, true);
    (void)new_time_txt_create(frm, COMPO_ID_TXT_PM, 4,
                              NEW_TIME_AMPM_COL_X, NEW_TIME_PM_Y,
                              NEW_TIME_AMPM_W, NEW_TIME_AMPM_H,
                              NEW_TIME_COLOR_OFF, true);
    (void)new_time_txt_create(frm, COMPO_ID_TXT_NO, 4,
                              NEW_TIME_BTN_NO_X, NEW_TIME_BTN_BOTTOM_Y,
                              NEW_TIME_BTN_W, NEW_TIME_BTN_H,
                              NEW_TIME_COLOR_OFF, true);
    (void)new_time_txt_create(frm, COMPO_ID_TXT_YES, 4,
                              NEW_TIME_BTN_YES_X, NEW_TIME_BTN_BOTTOM_Y,
                              NEW_TIME_BTN_W, NEW_TIME_BTN_H,
                              NEW_TIME_COLOR_ON, true);

    return frm;
}

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static void new_time_pt8028_keys_process(f_new_time_t *f)
{
    u8 press_tch;

    if (f == NULL || !f->key_ready) {
        return;
    }
    if (func_key_lock_press_take_poll()) {
        return;
    }
    press_tch = pt8028_take_press_tch();
    if (press_tch == 0xff) {
        return;
    }
    if (press_tch <= PT8028_KEY_TCH6 && press_tch != PT8028_KEY_TCH4) {
        elunchbox_user_activity_reset();
    }
    if (press_tch == PT8028_KEY_TCH6) {
        new_time_value_inc(f);
    } else if (press_tch == PT8028_KEY_TCH2) {
        new_time_value_dec(f);
    } else if (press_tch == PT8028_KEY_TCH3) {
        new_time_mode_key(f);
    } else if (press_tch == PT8028_KEY_TCH4) {
        new_time_ok_key(f);
    } else if (press_tch == PT8028_KEY_TCH5) {
        new_time_power_key(f);
    } else if (press_tch == PT8028_KEY_TCH1) {
        /* 加热键：跳转到加热设置页 */
        if (!sys_cb.flag_swithing) {
            func_switch_to(FUNC_NEW_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
    } else if (press_tch == PT8028_KEY_TCH7) {
        /* 预约键：跳转到预约设置页 */
        if (!sys_cb.flag_swithing) {
            func_switch_to(FUNC_RESERVATION, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
    }
}

static void new_time_keys_poll(f_new_time_t *f)
{
    if (f == NULL || !f->key_ready) {
        return;
    }
    new_time_pt8028_keys_process(f);
}
#endif

static void func_new_time_message(size_msg_t msg)
{
    f_new_time_t *f = (f_new_time_t *)func_cb.f_cb;

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
    case KU_MODE:
    case KU_BACK:
    case KU_VOL_UP:
    case KU_VOL_DOWN:
    case KEY_RIGHT | KEY_SHORT_UP:
        return;
    default:
        break;
    }
#endif
    switch (msg) {
    case KU_MODE:
        new_time_mode_key(f);
        break;
    case KU_BACK:
        new_time_ok_key(f);
        break;
    case KU_VOL_UP:
        new_time_value_inc(f);
        break;
    case KU_VOL_DOWN:
        new_time_value_dec(f);
        break;
    case KEY_RIGHT | KEY_SHORT_UP:
        new_time_power_key(f);
        break;
    default:
        func_message(msg);
        break;
    }
}

static void func_new_time_process(void)
{
    f_new_time_t *f = (f_new_time_t *)func_cb.f_cb;

    if (f == NULL) {
        func_process();
        return;
    }

#if ELUNCHBOX_PANEL_EN
    if (!f->key_ready) {
        WDT_CLR();
        switch (f->load_stage) {
        case NEW_TIME_LOAD_STATUS:
            new_time_status_refresh(f);
            f->load_stage = NEW_TIME_LOAD_SHAPES;
            break;
        case NEW_TIME_LOAD_SHAPES:
            new_time_shapes_apply(f);
            f->load_stage = NEW_TIME_LOAD_ARROWS;
            break;
        case NEW_TIME_LOAD_ARROWS:
            elunchbox_te_block_flag = 1;
            home_gpu_wait_idle();
            new_time_arrows_apply(f);
            home_gpu_wait_idle();
            elunchbox_te_block_flag = 0;
            f->load_stage = NEW_TIME_LOAD_CLOCK;
            break;
        case NEW_TIME_LOAD_CLOCK:
            new_time_clock_apply(f);
            f->load_stage = NEW_TIME_LOAD_TEXT;
            break;
        case NEW_TIME_LOAD_TEXT:
            new_time_text_apply(f);
            f->load_stage = NEW_TIME_LOAD_DONE;
            f->key_ready = true;
            break;
        default:
            f->key_ready = true;
            break;
        }
        func_process();
        func_home_drain_stale_key_msgs();
        pt8028_release_clear();
        (void)pt8028_take_press_tch();
        return;
    }
#endif

    if (f->display_pending) {
        new_time_ui_apply(f);
        f->display_pending = false;
    }

    func_process();
#if ELUNCHBOX_PANEL_EN
    if (!elunchbox_ui_is_live()) {
        return;
    }
    new_time_status_refresh(f);
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    new_time_keys_poll(f);
#endif
}

void func_new_time_enter(void)
{
    f_new_time_t *f;

    printf("func_new_time_enter\n");

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(1);
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif

#if ELUNCHBOX_PANEL_EN
    WDT_CLR();
#endif

    msg_queue_detach(KU_BACK, 0);
    msg_queue_detach(KU_MODE, 0);
    msg_queue_detach(KU_VOL_UP, 0);
    msg_queue_detach(KU_VOL_DOWN, 0);
    msg_queue_detach(KU_RIGHT, 0);

    func_cb.f_cb = func_zalloc(sizeof(f_new_time_t));
    f = (f_new_time_t *)func_cb.f_cb;
    f->focus = NEW_TIME_FOCUS_HOUR;
    f->bottom_sel = 0;
#if ELUNCHBOX_PANEL_EN
    f->load_stage = NEW_TIME_LOAD_STATUS;
    f->key_ready = false;
#endif
    f->display_pending = false;

    func_cb.frm_main = func_new_time_form_create();
    new_time_bind_objects(f);
    new_time_parse_rtc(&f->disp_h, &f->min, &f->is_pm);
    printf("nt_enter: rtc disp_h=%u min=%u is_pm=%u\n", f->disp_h, f->min, f->is_pm);

#if ELUNCHBOX_PANEL_EN
    new_time_res_up_ready = false;
    new_time_res_down_ready = false;
    new_time_font_ready = false;
    home_ui_shared_status_init();
    WDT_CLR();
    home_gpu_wait_idle();
    os_gui_draw_force();
    home_gpu_wait_idle();
    WDT_CLR();
    elunchbox_te_block_flag = 0;
    tft_bglight_force_on();
    printf("nt_enter: ok stage=%s te=0\n", new_time_stage_name(f->load_stage));
#else
    new_time_ui_apply(f);
    f->display_pending = false;
#endif
}

void func_new_time_exit(void)
{
#if ELUNCHBOX_PANEL_EN
    f_new_time_t *f = (f_new_time_t *)func_cb.f_cb;

    /* 与 func_new_heat_exit 相同：form destroy 统一清理 GPU，此处仅解绑 battery */
    if (f != NULL) {
        new_time_gpu_detach_before_leave(f);
    } else {
        home_ui_shared_battery_detach_pic();
    }
    new_time_res_up_ready = false;
    new_time_res_down_ready = false;
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(0);
    pt8028_release_clear();
#endif
    func_cb.last = FUNC_NEW_TIME;
    printf("func_new_time_exit\n");
}

void func_new_time(void)
{
    func_new_time_enter();
    while (func_cb.sta == FUNC_NEW_TIME) {
        func_new_time_process();
        func_new_time_message(msg_dequeue());
    }
    func_new_time_exit();
}
