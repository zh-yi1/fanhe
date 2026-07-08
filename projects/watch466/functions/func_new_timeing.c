#include "include.h"
#include "func.h"
#include "new_time_res.h"
#include "new_timeing_res.h"
#include "new_home_icon_res.h"
#include "new_home_tab_res.h"
#include "home_ui_shared.h"
#include "home_ui_gpu_detach.h"
#include "home_ui_ram.h"
#include "func_key_lock.h"
#include "func_lunchbox_uart.h"

#if ELUNCHBOX_PANEL_EN
extern volatile u8 elunchbox_te_block_flag;
#endif

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

#ifndef UI_BUF_NEW_UI_NEW_BLUE_BJ1_BIN
#error "Run tools/gen_new_time_icons.py then Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_NEW_UI_NEW_B0_BIN
#error "Run tools/gen_new_timeing_icons.py then Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_0FONT_FONT_TEST_BIN
#error "UI_BUF_0FONT_FONT_TEST_BIN missing: add font_test.bin to ui.bin then Output/bin/prebuild.bat"
#endif
#define NEW_TIMEING_FONT                    UI_BUF_0FONT_FONT_TEST_BIN

#if (NEW_TIME_NEW_BLUE_BJ1_RAM_SIZE > NEW_HOME_TAB_RAM_SIZE)
#error "timeing box icon exceeds home_ui_shared_icon_runtime slot"
#endif
#if ((NEW_TIME_NEW_GRAY_BJ2_RAM_SIZE + NEW_TIME_NEW_BLUE_BJ2_RAM_SIZE) > NEW_HOME_TAB_RAM_SIZE)
#error "timeing btn icons exceed home_ui_shared_icon_runtime slot"
#endif
#if (NEW_TIME_NEW_UP_RAM_SIZE > HEAT_WBX_RAM_SIZE)
#error "timeing up arrow exceeds timer colon pool"
#endif
#if (NEW_TIME_NEW_DOWN_RAM_SIZE > HEAT_B_DIGIT_RAM_MAX_SIZE)
#error "timeing down arrow exceeds timer digit pool"
#endif

/*
 * 时间设置页 — 效果图 TIME
 *   时/分框：new_blue_bj1 / new_gray_bj1；数字 new_w* / new_b*
 *   底部 NO/YES：new_gray_bj2 / new_blue_bj2
 *   加减键调节当前列；确认键切换焦点；电源键返回/取消
 */
#define NEW_TIMEING_REF_W                   466
#define NEW_TIMEING_REF_H                   466
#define NEW_TIMEING_SX(v)                   ((s16)((s32)(v) * GUI_SCREEN_WIDTH / NEW_TIMEING_REF_W))
#define NEW_TIMEING_SY(v)                   ((s16)((s32)(v) * GUI_SCREEN_HEIGHT / NEW_TIMEING_REF_H))
#define NEW_TIMEING_DIGIT_GAP               NEW_TIMEING_SX(4)

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define NEW_TIMEING_STATUS_Y                22
#define NEW_TIMEING_STATUS_RIGHT_MARGIN     10
#define NEW_TIMEING_STATUS_GAP              6
#define NEW_TIMEING_TITLE_LEFT              10
#define NEW_TIMEING_TITLE_Y                 9
#define NEW_TIMEING_TITLE_H                 36
#define NEW_TIMEING_TITLE_W                 120
#define NEW_TIMEING_PANEL_Y                 142
#define NEW_TIMEING_PANEL_W                 280
#define NEW_TIMEING_PANEL_H                 210
#define NEW_TIMEING_PAGE_BG                 0xEF5D
#define NEW_TIMEING_CONTENT_OFFSET_Y        ((s16)(-16))
#define NEW_TIMEING_COL_HALF_SPAN           ((s16)53)
#define NEW_TIMEING_BOX_Y                   ((s16)(NEW_TIMEING_PANEL_Y + NEW_TIMEING_CONTENT_OFFSET_Y))
#define NEW_TIMEING_ARROW_UP_Y              ((s16)(NEW_TIMEING_BOX_Y - 47))
#define NEW_TIMEING_ARROW_DOWN_Y            ((s16)(NEW_TIMEING_BOX_Y + 47))
#define NEW_TIMEING_BTN_CENTER_DIST         ((s16)130)
#define NEW_TIMEING_BTN_BOTTOM_Y            ((s16)(NEW_TIMEING_PANEL_Y + NEW_TIMEING_PANEL_H / 2 - NEW_TIME_BTN_H / 2 - 12 + NEW_TIMEING_CONTENT_OFFSET_Y))
#else
#define NEW_TIMEING_STATUS_Y                NEW_TIMEING_SY(48)
#define NEW_TIMEING_STATUS_RIGHT_MARGIN     NEW_TIMEING_SX(24)
#define NEW_TIMEING_STATUS_GAP              NEW_TIMEING_SX(10)
#define NEW_TIMEING_TITLE_LEFT              NEW_TIMEING_SX(10)
#define NEW_TIMEING_TITLE_Y                 NEW_TIMEING_SY(34)
#define NEW_TIMEING_TITLE_H                 NEW_TIMEING_SY(36)
#define NEW_TIMEING_TITLE_W                 NEW_TIMEING_SX(220)
#define NEW_TIMEING_PANEL_Y                 NEW_TIMEING_SY(144)
#define NEW_TIMEING_PANEL_W                 NEW_TIMEING_SX(280)
#define NEW_TIMEING_PANEL_H                 NEW_TIMEING_SY(240)
#define NEW_TIMEING_PAGE_BG                 0xEF5D
#define NEW_TIMEING_CONTENT_OFFSET_Y        NEW_TIMEING_SY(-24)
#define NEW_TIMEING_COL_HALF_SPAN           NEW_TIMEING_SX(80)
#define NEW_TIMEING_BOX_Y                   ((s16)(NEW_TIMEING_PANEL_Y + NEW_TIMEING_CONTENT_OFFSET_Y))
#define NEW_TIMEING_ARROW_UP_Y              ((s16)(NEW_TIMEING_BOX_Y - NEW_TIMEING_SY(92)))
#define NEW_TIMEING_ARROW_DOWN_Y            ((s16)(NEW_TIMEING_BOX_Y + NEW_TIMEING_SY(92)))
#define NEW_TIMEING_BTN_CENTER_DIST         NEW_TIMEING_SX(230)
#define NEW_TIMEING_BTN_BOTTOM_Y            ((s16)(NEW_TIMEING_PANEL_Y + NEW_TIMEING_PANEL_H / 2 - NEW_TIME_BTN_H / 2 - NEW_TIMEING_SY(16) + NEW_TIMEING_CONTENT_OFFSET_Y))
#endif

/*
 * new_blue_bj1 / new_gray_bj1 固定左上角（320×240 当前屏位；蓝/灰切换仅换图）
 */
#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define NEW_TIMEING_HOUR_BOX_X              107
#define NEW_TIMEING_HOUR_BOX_Y              126
#define NEW_TIMEING_MIN_BOX_X               213
#define NEW_TIMEING_MIN_BOX_Y               126
#else
#define NEW_TIMEING_HOUR_BOX_X              ((s16)((s32)107 * GUI_SCREEN_WIDTH / 320))
#define NEW_TIMEING_HOUR_BOX_Y              ((s16)((s32)126 * GUI_SCREEN_HEIGHT / 240))
#define NEW_TIMEING_MIN_BOX_X               ((s16)((s32)213 * GUI_SCREEN_WIDTH / 320))
#define NEW_TIMEING_MIN_BOX_Y               ((s16)((s32)126 * GUI_SCREEN_HEIGHT / 240))
#endif

/* 时/分/按钮区相对白色卡片水平居中 */
#define NEW_TIMEING_HOUR_COL_X              ((s16)(GUI_SCREEN_CENTER_X - NEW_TIMEING_COL_HALF_SPAN))
#define NEW_TIMEING_MIN_COL_X               ((s16)(GUI_SCREEN_CENTER_X + NEW_TIMEING_COL_HALF_SPAN))
#define NEW_TIMEING_COLON_X                 GUI_SCREEN_CENTER_X
#define NEW_TIMEING_BTN_NO_X                ((s16)(GUI_SCREEN_CENTER_X - NEW_TIMEING_BTN_CENTER_DIST / 2))
#define NEW_TIMEING_BTN_YES_X               ((s16)(GUI_SCREEN_CENTER_X + NEW_TIMEING_BTN_CENTER_DIST / 2))

#define NEW_TIMEING_STATUS_BAT_X            (GUI_SCREEN_WIDTH - NEW_TIMEING_STATUS_RIGHT_MARGIN - NEW_HOME_BAT_W / 2)
#define NEW_TIMEING_STATUS_BT_X             (NEW_TIMEING_STATUS_BAT_X - NEW_HOME_BAT_W / 2 - NEW_TIMEING_STATUS_GAP - NEW_HOME_BT_W / 2)

#define NEW_TIMEING_COLOR_TITLE             COLOR_BLACK
#define NEW_TIMEING_COLOR_ON                COLOR_WHITE
#define NEW_TIMEING_COLOR_OFF               COLOR_BLACK

enum {
    NEW_TIMEING_FOCUS_HOUR = 0,
    NEW_TIMEING_FOCUS_MIN,
    NEW_TIMEING_FOCUS_BOTTOM,
};

enum {
    COMPO_ID_SHAPE_BG = 1,
    COMPO_ID_SHAPE_PANEL,
    COMPO_ID_TXT_TITLE,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_BAT,
    COMPO_ID_PIC_HOUR_BG,
    COMPO_ID_PIC_MIN_BG,
    COMPO_ID_PIC_HOUR_UP,
    COMPO_ID_PIC_HOUR_DOWN,
    COMPO_ID_PIC_MIN_UP,
    COMPO_ID_PIC_MIN_DOWN,
    COMPO_ID_PIC_H10,
    COMPO_ID_PIC_H1,
    COMPO_ID_TXT_COLON,
    COMPO_ID_PIC_M10,
    COMPO_ID_PIC_M1,
    COMPO_ID_PIC_NO_BG,
    COMPO_ID_PIC_YES_BG,
    COMPO_ID_TXT_NO,
    COMPO_ID_TXT_YES,
};

enum {
    NEW_TIMEING_LOAD_STATUS = 0,
    NEW_TIMEING_LOAD_BOXES,
    NEW_TIMEING_LOAD_ARROWS,
    NEW_TIMEING_LOAD_BTNS,
    NEW_TIMEING_LOAD_DIGITS,
    NEW_TIMEING_LOAD_TEXT,
    NEW_TIMEING_LOAD_DONE,
};

typedef struct {
    u8 hour;
    u8 min;
    u8 focus;
    u8 bottom_sel;
    bool display_pending;
#if ELUNCHBOX_PANEL_EN
    u8 load_stage;
    bool key_ready;
#endif
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_hour_bg;
    compo_picturebox_t *pic_min_bg;
    compo_picturebox_t *pic_hour_up;
    compo_picturebox_t *pic_hour_down;
    compo_picturebox_t *pic_min_up;
    compo_picturebox_t *pic_min_down;
    compo_picturebox_t *pic_h10;
    compo_picturebox_t *pic_h1;
    compo_textbox_t *txt_colon;
    compo_picturebox_t *pic_m10;
    compo_picturebox_t *pic_m1;
    compo_picturebox_t *pic_no_bg;
    compo_picturebox_t *pic_yes_bg;
    compo_textbox_t *txt_title;
    compo_textbox_t *txt_no;
    compo_textbox_t *txt_yes;
} f_new_timeing_t;

#if ELUNCHBOX_PANEL_EN
/* 复用 Home Tab / 倒计时共享池，避免新增 ~33KB BSS 导致上电无法启动 */
#define NEW_TIMEING_HOUR_BOX_RAM            (home_ui_shared_icon_runtime[0])
#define NEW_TIMEING_MIN_BOX_RAM             (home_ui_shared_icon_runtime[1])
#define NEW_TIMEING_NO_BTN_RAM              (home_ui_shared_icon_runtime[2])
#define NEW_TIMEING_YES_BTN_RAM             (home_ui_shared_icon_runtime[2] + NEW_TIME_NEW_GRAY_BJ2_RAM_SIZE)
#define NEW_TIMEING_ARROW_UP_RAM            (home_ui_shared_timer_colon_ram)
#define NEW_TIMEING_ARROW_DOWN_RAM          (home_ui_shared_timer_digit_ram[0])

static bool new_timeing_res_arrow_up_ready;
static bool new_timeing_res_arrow_down_ready;
static bool new_timeing_font_ready;

static const u32 tbl_timeing_b_digit_addr[10] = {
    UI_BUF_NEW_UI_NEW_B0_BIN, UI_BUF_NEW_UI_NEW_B1_BIN, UI_BUF_NEW_UI_NEW_B2_BIN,
    UI_BUF_NEW_UI_NEW_B3_BIN, UI_BUF_NEW_UI_NEW_B4_BIN, UI_BUF_NEW_UI_NEW_B5_BIN,
    UI_BUF_NEW_UI_NEW_B6_BIN, UI_BUF_NEW_UI_NEW_B7_BIN, UI_BUF_NEW_UI_NEW_B8_BIN,
    UI_BUF_NEW_UI_NEW_B9_BIN,
};
static const u16 tbl_timeing_b_digit_len[10] = {
    UI_LEN_NEW_UI_NEW_B0_BIN, UI_LEN_NEW_UI_NEW_B1_BIN, UI_LEN_NEW_UI_NEW_B2_BIN,
    UI_LEN_NEW_UI_NEW_B3_BIN, UI_LEN_NEW_UI_NEW_B4_BIN, UI_LEN_NEW_UI_NEW_B5_BIN,
    UI_LEN_NEW_UI_NEW_B6_BIN, UI_LEN_NEW_UI_NEW_B7_BIN, UI_LEN_NEW_UI_NEW_B8_BIN,
    UI_LEN_NEW_UI_NEW_B9_BIN,
};
static const u16 tbl_timeing_b_digit_w[10] = {
    NEW_TIMEING_NEW_B0_W, NEW_TIMEING_NEW_B1_W, NEW_TIMEING_NEW_B2_W,
    NEW_TIMEING_NEW_B3_W, NEW_TIMEING_NEW_B4_W, NEW_TIMEING_NEW_B5_W,
    NEW_TIMEING_NEW_B6_W, NEW_TIMEING_NEW_B7_W, NEW_TIMEING_NEW_B8_W,
    NEW_TIMEING_NEW_B9_W,
};
static const u16 tbl_timeing_b_digit_h[10] = {
    NEW_TIMEING_NEW_B0_H, NEW_TIMEING_NEW_B1_H, NEW_TIMEING_NEW_B2_H,
    NEW_TIMEING_NEW_B3_H, NEW_TIMEING_NEW_B4_H, NEW_TIMEING_NEW_B5_H,
    NEW_TIMEING_NEW_B6_H, NEW_TIMEING_NEW_B7_H, NEW_TIMEING_NEW_B8_H,
    NEW_TIMEING_NEW_B9_H,
};

static const u32 tbl_timeing_w_digit_addr[10] = {
    UI_BUF_NEW_UI_NEW_W0_BIN, UI_BUF_NEW_UI_NEW_W1_BIN, UI_BUF_NEW_UI_NEW_W2_BIN,
    UI_BUF_NEW_UI_NEW_W3_BIN, UI_BUF_NEW_UI_NEW_W4_BIN, UI_BUF_NEW_UI_NEW_W5_BIN,
    UI_BUF_NEW_UI_NEW_W6_BIN, UI_BUF_NEW_UI_NEW_W7_BIN, UI_BUF_NEW_UI_NEW_W8_BIN,
    UI_BUF_NEW_UI_NEW_W9_BIN,
};
static const u16 tbl_timeing_w_digit_len[10] = {
    UI_LEN_NEW_UI_NEW_W0_BIN, UI_LEN_NEW_UI_NEW_W1_BIN, UI_LEN_NEW_UI_NEW_W2_BIN,
    UI_LEN_NEW_UI_NEW_W3_BIN, UI_LEN_NEW_UI_NEW_W4_BIN, UI_LEN_NEW_UI_NEW_W5_BIN,
    UI_LEN_NEW_UI_NEW_W6_BIN, UI_LEN_NEW_UI_NEW_W7_BIN, UI_LEN_NEW_UI_NEW_W8_BIN,
    UI_LEN_NEW_UI_NEW_W9_BIN,
};
static const u16 tbl_timeing_w_digit_w[10] = {
    NEW_TIMEING_NEW_W0_W, NEW_TIMEING_NEW_W1_W, NEW_TIMEING_NEW_W2_W,
    NEW_TIMEING_NEW_W3_W, NEW_TIMEING_NEW_W4_W, NEW_TIMEING_NEW_W5_W,
    NEW_TIMEING_NEW_W6_W, NEW_TIMEING_NEW_W7_W, NEW_TIMEING_NEW_W8_W,
    NEW_TIMEING_NEW_W9_W,
};
static const u16 tbl_timeing_w_digit_h[10] = {
    NEW_TIMEING_NEW_W0_H, NEW_TIMEING_NEW_W1_H, NEW_TIMEING_NEW_W2_H,
    NEW_TIMEING_NEW_W3_H, NEW_TIMEING_NEW_W4_H, NEW_TIMEING_NEW_W5_H,
    NEW_TIMEING_NEW_W6_H, NEW_TIMEING_NEW_W7_H, NEW_TIMEING_NEW_W8_H,
    NEW_TIMEING_NEW_W9_H,
};

static void new_timeing_font_bind_txt(compo_textbox_t *txt)
{
    if (txt != NULL) {
        compo_textbox_set_font(txt, NEW_TIMEING_FONT);
    }
}

static void new_timeing_font_apply_once(f_new_timeing_t *f)
{
    if (new_timeing_font_ready || f == NULL) {
        return;
    }
    WDT_CLR();
    new_timeing_font_bind_txt(f->txt_title);
    new_timeing_font_bind_txt(f->txt_colon);
    new_timeing_font_bind_txt(f->txt_no);
    new_timeing_font_bind_txt(f->txt_yes);
    new_timeing_font_ready = true;
}

static bool new_timeing_gpu_ram_bind(u8 *ram, u16 buf_size, u32 addr, u16 len,
                                     compo_picturebox_t *pic, u16 w, u16 h, s16 x, s16 y)
{
    u16 need;

    if (pic == NULL || addr == 0 || len == 0 || ram == NULL || len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }
    WDT_CLR();
    os_spiflash_read(ram, addr, len);
    if (!gui_set_ram_check(ram, __func__)) {
        compo_picturebox_set_visible(pic, false);
        return false;
    }
    need = (u16)(8 + (u32)GET_LE16(&ram[4]) * GET_LE16(&ram[6]) * 2);
    if (need > len || need > buf_size) {
        compo_picturebox_set_visible(pic, false);
        return false;
    }
    compo_picturebox_set_ram(pic, ram);
    compo_picturebox_set_size(pic, w, h);
    compo_picturebox_set_pos(pic, x, y);
    compo_picturebox_set_visible(pic, true);
    return true;
}

static void new_timeing_pic_pos_tr(compo_picturebox_t *pic, s16 tr_x, s16 tr_y, u16 w, u16 h)
{
    if (pic == NULL) {
        return;
    }
    compo_picturebox_set_pos(pic, (s16)(tr_x - w / 2), (s16)(tr_y - h / 2));
}

static void new_timeing_box_bg_pos_fix(compo_picturebox_t *pic, s16 x, s16 y)
{
    if (pic == NULL) {
        return;
    }
    compo_picturebox_set_pos(pic, x, y);
    compo_picturebox_set_size(pic, NEW_TIME_BOX_W, NEW_TIME_BOX_H);
    compo_picturebox_set_visible(pic, true);
}

static bool new_timeing_box_bg_texture_bind(u8 *ram, u16 buf_size, u32 addr, u16 len,
                                            compo_picturebox_t *pic)
{
    u16 need;

    if (pic == NULL || addr == 0 || len == 0 || ram == NULL || len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }
    WDT_CLR();
    os_spiflash_read(ram, addr, len);
    if (!gui_set_ram_check(ram, __func__)) {
        compo_picturebox_set_visible(pic, false);
        return false;
    }
    need = (u16)(8 + (u32)GET_LE16(&ram[4]) * GET_LE16(&ram[6]) * 2);
    if (need > len || need > buf_size) {
        compo_picturebox_set_visible(pic, false);
        return false;
    }
    compo_picturebox_set_ram(pic, ram);
    compo_picturebox_set_size(pic, NEW_TIME_BOX_W, NEW_TIME_BOX_H);
    return true;
}
#endif

static void new_timeing_txt_pos_center(compo_textbox_t *txt, s16 cx, s16 cy, s16 w, s16 h)
{
    if (txt == NULL) {
        return;
    }
    compo_textbox_set_location(txt, (s16)(cx - w / 2), (s16)(cy - h / 2), w, h);
}

static void new_timeing_parse_rtc(u8 *hour, u8 *min)
{
    tm_t tm = rtc_clock_get();

    *hour = tm.hour;
    *min = tm.min;
}

static void new_timeing_save_rtc(f_new_timeing_t *f)
{
    tm_t tm_set;

    if (f == NULL) {
        return;
    }
    tm_set = rtc_clock_get();
    tm_set.hour = f->hour;
    tm_set.min = f->min;
    rtc_clock_set(tm_set);
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_time_sync(RTCCNT + LB_RTC_UNIX_OFFSET);
#endif
}

#if ELUNCHBOX_PANEL_EN
static s16 new_timeing_digit_pair_offset(u16 w10, u16 w1)
{
    return (s16)((s16)w10 / 2 + NEW_TIMEING_DIGIT_GAP + (s16)w1 / 2);
}

static bool new_timeing_load_digit(u8 slot, u8 digit, bool white_set,
                                   compo_picturebox_t *pic)
{
    u32 addr;
    u16 len;
    u16 w;
    u16 h;

    if (pic == NULL || digit > 9 || slot >= HOME_UI_DIGIT_SLOTS) {
        return false;
    }
    if (white_set) {
        addr = tbl_timeing_w_digit_addr[digit];
        len = tbl_timeing_w_digit_len[digit];
        w = tbl_timeing_w_digit_w[digit];
        h = tbl_timeing_w_digit_h[digit];
    } else {
        addr = tbl_timeing_b_digit_addr[digit];
        len = tbl_timeing_b_digit_len[digit];
        w = tbl_timeing_b_digit_w[digit];
        h = tbl_timeing_b_digit_h[digit];
    }
    if (len > NEW_TIMEING_DIGIT_RAM_SIZE || len > HOME_DIGIT_RAM_MAX_SIZE) {
        return false;
    }
    return new_timeing_gpu_ram_bind(home_ui_digit_ram[slot], HOME_DIGIT_RAM_MAX_SIZE,
                                    addr, len, pic, w, h, 0, 0);
}

static void new_timeing_clock_digit_pos_pair(u8 d10, u8 d1, bool white_set,
                                             s16 center_x, s16 center_y,
                                             compo_picturebox_t *pic10,
                                             compo_picturebox_t *pic1)
{
    u16 w10;
    u16 w1;
    u16 h10;
    u16 h1;
    s16 off;

    w10 = white_set ? tbl_timeing_w_digit_w[d10] : tbl_timeing_b_digit_w[d10];
    w1 = white_set ? tbl_timeing_w_digit_w[d1] : tbl_timeing_b_digit_w[d1];
    h10 = white_set ? tbl_timeing_w_digit_h[d10] : tbl_timeing_b_digit_h[d10];
    h1 = white_set ? tbl_timeing_w_digit_h[d1] : tbl_timeing_b_digit_h[d1];
    off = new_timeing_digit_pair_offset(w10, w1);
    new_timeing_pic_pos_tr(pic10, (s16)(center_x - off / 2), center_y, w10, h10);
    new_timeing_pic_pos_tr(pic1, (s16)(center_x + off / 2), center_y, w1, h1);
}

static void new_timeing_boxes_apply(f_new_timeing_t *f)
{
    bool hour_sel;
    bool min_sel;
    u32 hour_addr;
    u32 min_addr;
    u16 hour_len;
    u16 min_len;

    if (f == NULL) {
        return;
    }
    hour_sel = (f->focus == NEW_TIMEING_FOCUS_HOUR);
    min_sel = (f->focus == NEW_TIMEING_FOCUS_MIN);
    hour_addr = hour_sel ? UI_BUF_NEW_UI_NEW_BLUE_BJ1_BIN : UI_BUF_NEW_UI_NEW_GRAY_BJ1_BIN;
    hour_len = hour_sel ? UI_LEN_NEW_UI_NEW_BLUE_BJ1_BIN : UI_LEN_NEW_UI_NEW_GRAY_BJ1_BIN;
    min_addr = min_sel ? UI_BUF_NEW_UI_NEW_BLUE_BJ1_BIN : UI_BUF_NEW_UI_NEW_GRAY_BJ1_BIN;
    min_len = min_sel ? UI_LEN_NEW_UI_NEW_BLUE_BJ1_BIN : UI_LEN_NEW_UI_NEW_GRAY_BJ1_BIN;

    (void)new_timeing_box_bg_texture_bind(NEW_TIMEING_HOUR_BOX_RAM, NEW_HOME_TAB_RAM_SIZE,
                                          hour_addr, hour_len, f->pic_hour_bg);
    new_timeing_box_bg_pos_fix(f->pic_hour_bg, NEW_TIMEING_HOUR_BOX_X, NEW_TIMEING_HOUR_BOX_Y);
    (void)new_timeing_box_bg_texture_bind(NEW_TIMEING_MIN_BOX_RAM, NEW_HOME_TAB_RAM_SIZE,
                                          min_addr, min_len, f->pic_min_bg);
    new_timeing_box_bg_pos_fix(f->pic_min_bg, NEW_TIMEING_MIN_BOX_X, NEW_TIMEING_MIN_BOX_Y);
}

static void new_timeing_arrows_apply(f_new_timeing_t *f)
{
    if (f == NULL) {
        return;
    }
    if (!new_timeing_res_arrow_up_ready) {
        os_spiflash_read(NEW_TIMEING_ARROW_UP_RAM, UI_BUF_NEW_UI_NEW_UP_BIN, UI_LEN_NEW_UI_NEW_UP_BIN);
        if (gui_set_ram_check(NEW_TIMEING_ARROW_UP_RAM, __func__)) {
            new_timeing_res_arrow_up_ready = true;
        }
    }
    if (!new_timeing_res_arrow_down_ready) {
        os_spiflash_read(NEW_TIMEING_ARROW_DOWN_RAM, UI_BUF_NEW_UI_NEW_DOWN_BIN, UI_LEN_NEW_UI_NEW_DOWN_BIN);
        if (gui_set_ram_check(NEW_TIMEING_ARROW_DOWN_RAM, __func__)) {
            new_timeing_res_arrow_down_ready = true;
        }
    }
    if (new_timeing_res_arrow_up_ready) {
        (void)new_timeing_gpu_ram_bind(NEW_TIMEING_ARROW_UP_RAM, HEAT_WBX_RAM_SIZE,
                                       UI_BUF_NEW_UI_NEW_UP_BIN, UI_LEN_NEW_UI_NEW_UP_BIN,
                                       f->pic_hour_up, NEW_TIME_ARROW_W, NEW_TIME_ARROW_H,
                                       NEW_TIMEING_HOUR_COL_X, NEW_TIMEING_ARROW_UP_Y);
        (void)new_timeing_gpu_ram_bind(NEW_TIMEING_ARROW_UP_RAM, HEAT_WBX_RAM_SIZE,
                                       UI_BUF_NEW_UI_NEW_UP_BIN, UI_LEN_NEW_UI_NEW_UP_BIN,
                                       f->pic_min_up, NEW_TIME_ARROW_W, NEW_TIME_ARROW_H,
                                       NEW_TIMEING_MIN_COL_X, NEW_TIMEING_ARROW_UP_Y);
    }
    if (new_timeing_res_arrow_down_ready) {
        (void)new_timeing_gpu_ram_bind(NEW_TIMEING_ARROW_DOWN_RAM, HEAT_B_DIGIT_RAM_MAX_SIZE,
                                       UI_BUF_NEW_UI_NEW_DOWN_BIN, UI_LEN_NEW_UI_NEW_DOWN_BIN,
                                       f->pic_hour_down, NEW_TIME_ARROW_W, NEW_TIME_ARROW_H,
                                       NEW_TIMEING_HOUR_COL_X, NEW_TIMEING_ARROW_DOWN_Y);
        (void)new_timeing_gpu_ram_bind(NEW_TIMEING_ARROW_DOWN_RAM, HEAT_B_DIGIT_RAM_MAX_SIZE,
                                       UI_BUF_NEW_UI_NEW_DOWN_BIN, UI_LEN_NEW_UI_NEW_DOWN_BIN,
                                       f->pic_min_down, NEW_TIME_ARROW_W, NEW_TIME_ARROW_H,
                                       NEW_TIMEING_MIN_COL_X, NEW_TIMEING_ARROW_DOWN_Y);
    }
}

static void new_timeing_btns_apply(f_new_timeing_t *f)
{
    u32 no_addr;
    u32 yes_addr;
    u16 no_len;
    u16 yes_len;

    if (f == NULL) {
        return;
    }
    if (f->focus == NEW_TIMEING_FOCUS_BOTTOM && f->bottom_sel == 0) {
        no_addr = UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN;
        no_len = UI_LEN_NEW_UI_NEW_BLUE_BJ2_BIN;
        yes_addr = UI_BUF_NEW_UI_NEW_GRAY_BJ2_BIN;
        yes_len = UI_LEN_NEW_UI_NEW_GRAY_BJ2_BIN;
    } else if (f->focus == NEW_TIMEING_FOCUS_BOTTOM && f->bottom_sel != 0) {
        no_addr = UI_BUF_NEW_UI_NEW_GRAY_BJ2_BIN;
        no_len = UI_LEN_NEW_UI_NEW_GRAY_BJ2_BIN;
        yes_addr = UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN;
        yes_len = UI_LEN_NEW_UI_NEW_BLUE_BJ2_BIN;
    } else {
        no_addr = UI_BUF_NEW_UI_NEW_GRAY_BJ2_BIN;
        no_len = UI_LEN_NEW_UI_NEW_GRAY_BJ2_BIN;
        yes_addr = UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN;
        yes_len = UI_LEN_NEW_UI_NEW_BLUE_BJ2_BIN;
    }
    (void)new_timeing_gpu_ram_bind(NEW_TIMEING_NO_BTN_RAM, NEW_TIME_NEW_GRAY_BJ2_RAM_SIZE,
                                   no_addr, no_len, f->pic_no_bg,
                                   NEW_TIME_BTN_W, NEW_TIME_BTN_H,
                                   NEW_TIMEING_BTN_NO_X, NEW_TIMEING_BTN_BOTTOM_Y);
    (void)new_timeing_gpu_ram_bind(NEW_TIMEING_YES_BTN_RAM,
                                   (u16)(NEW_HOME_TAB_RAM_SIZE - NEW_TIME_NEW_GRAY_BJ2_RAM_SIZE),
                                   yes_addr, yes_len, f->pic_yes_bg,
                                   NEW_TIME_BTN_W, NEW_TIME_BTN_H,
                                   NEW_TIMEING_BTN_YES_X, NEW_TIMEING_BTN_BOTTOM_Y);
}

static void new_timeing_title_txt_show(compo_textbox_t *txt)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

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
    compo_textbox_set_location(txt, NEW_TIMEING_TITLE_LEFT, NEW_TIMEING_TITLE_Y,
                               NEW_TIMEING_TITLE_W, NEW_TIMEING_TITLE_H);
    compo_textbox_set_forecolor(txt, NEW_TIMEING_COLOR_TITLE);
    compo_textbox_set(txt, "TIME");
    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
    compo_textbox_set_visible(txt, true);
}

static void new_timeing_digits_apply(f_new_timeing_t *f)
{
    u8 h10;
    u8 h1;
    u8 m10;
    u8 m1;
    bool hour_white;
    bool min_white;

    if (f == NULL) {
        return;
    }
    h10 = (u8)(f->hour / 10);
    h1 = (u8)(f->hour % 10);
    m10 = (u8)(f->min / 10);
    m1 = (u8)(f->min % 10);
    hour_white = (f->focus == NEW_TIMEING_FOCUS_HOUR);
    min_white = (f->focus == NEW_TIMEING_FOCUS_MIN);

    if (new_timeing_load_digit(0, h10, hour_white, f->pic_h10) &&
        new_timeing_load_digit(1, h1, hour_white, f->pic_h1)) {
        new_timeing_clock_digit_pos_pair(h10, h1, hour_white,
                                         NEW_TIMEING_HOUR_COL_X, NEW_TIMEING_BOX_Y,
                                         f->pic_h10, f->pic_h1);
    }
    if (new_timeing_load_digit(2, m10, min_white, f->pic_m10) &&
        new_timeing_load_digit(3, m1, min_white, f->pic_m1)) {
        new_timeing_clock_digit_pos_pair(m10, m1, min_white,
                                         NEW_TIMEING_MIN_COL_X, NEW_TIMEING_BOX_Y,
                                         f->pic_m10, f->pic_m1);
    }
}

static void new_timeing_btn_label_show(compo_textbox_t *txt, s16 cx, s16 cy,
                                       const char *label, u16 color)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL) {
        return;
    }
    widget = txt->txt;
    compo_textbox_set_location(txt, cx, cy, NEW_TIME_BTN_W, NEW_TIME_BTN_H);
    compo_textbox_set_align_center(txt, true);
    if (widget != NULL) {
        widget_set_align_center(widget, true);
        widget_text_set_ellipsis(widget, false);
    }
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set(txt, label);
    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
    compo_textbox_set_visible(txt, true);
    if (widget != NULL) {
        widget_set_top(widget, true);
    }
}

static void new_timeing_label_show(compo_textbox_t *txt, const char *label, u16 color)
{
    if (txt == NULL) {
        return;
    }
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set(txt, label);
    compo_textbox_set_visible(txt, true);
}

static void new_timeing_text_apply(f_new_timeing_t *f)
{
    u16 no_color;
    u16 yes_color;

    if (f == NULL) {
        return;
    }
    new_timeing_font_apply_once(f);
    new_timeing_title_txt_show(f->txt_title);
    new_timeing_txt_pos_center(f->txt_colon, NEW_TIMEING_COLON_X, NEW_TIMEING_BOX_Y, 12, 24);
    new_timeing_label_show(f->txt_colon, ":", NEW_TIMEING_COLOR_OFF);
    if (f->focus == NEW_TIMEING_FOCUS_BOTTOM && f->bottom_sel == 0) {
        no_color = NEW_TIMEING_COLOR_ON;
        yes_color = NEW_TIMEING_COLOR_OFF;
    } else if (f->focus == NEW_TIMEING_FOCUS_BOTTOM && f->bottom_sel != 0) {
        no_color = NEW_TIMEING_COLOR_OFF;
        yes_color = NEW_TIMEING_COLOR_ON;
    } else {
        no_color = NEW_TIMEING_COLOR_OFF;
        yes_color = NEW_TIMEING_COLOR_ON;
    }
    new_timeing_btn_label_show(f->txt_no, NEW_TIMEING_BTN_NO_X, NEW_TIMEING_BTN_BOTTOM_Y,
                               "NO", no_color);
    new_timeing_btn_label_show(f->txt_yes, NEW_TIMEING_BTN_YES_X, NEW_TIMEING_BTN_BOTTOM_Y,
                               "YES", yes_color);
}

static void new_timeing_status_refresh(f_new_timeing_t *f)
{
    if (f == NULL) {
        return;
    }
    if (f->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_pos(f->pic_bt, NEW_TIMEING_STATUS_BT_X, NEW_TIMEING_STATUS_Y);
        home_ui_shared_status_refresh_bt(f->pic_bt);
    }
    if (f->pic_bat != NULL) {
        compo_picturebox_set_pos(f->pic_bat, NEW_TIMEING_STATUS_BAT_X, NEW_TIMEING_STATUS_Y);
        home_ui_shared_battery_attach_pic(f->pic_bat);
    }
}

static void new_timeing_ui_apply(f_new_timeing_t *f)
{
    if (f == NULL) {
        return;
    }
    new_timeing_status_refresh(f);
    new_timeing_boxes_apply(f);
    new_timeing_arrows_apply(f);
    new_timeing_btns_apply(f);
    new_timeing_digits_apply(f);
    new_timeing_text_apply(f);
}

static void new_timeing_focus_refresh(f_new_timeing_t *f)
{
    if (f == NULL) {
        return;
    }
    new_timeing_boxes_apply(f);
    new_timeing_btns_apply(f);
    new_timeing_digits_apply(f);
    new_timeing_text_apply(f);
}

static void new_timeing_gpu_detach_before_leave(f_new_timeing_t *f)
{
    (void)f;
    home_ui_shared_battery_detach_pic();
}
#endif

static void new_timeing_value_inc(f_new_timeing_t *f)
{
    if (f == NULL) {
        return;
    }
    switch (f->focus) {
    case NEW_TIMEING_FOCUS_HOUR:
        f->hour = (u8)((f->hour + 1) % 24);
#if ELUNCHBOX_PANEL_EN
        new_timeing_focus_refresh(f);
#endif
        break;
    case NEW_TIMEING_FOCUS_MIN:
        f->min = (u8)((f->min + 1) % 60);
#if ELUNCHBOX_PANEL_EN
        new_timeing_focus_refresh(f);
#endif
        break;
    case NEW_TIMEING_FOCUS_BOTTOM:
        f->bottom_sel = (u8)((f->bottom_sel + 1) & 1);
#if ELUNCHBOX_PANEL_EN
        new_timeing_focus_refresh(f);
#endif
        break;
    default:
        break;
    }
}

static void new_timeing_value_dec(f_new_timeing_t *f)
{
    if (f == NULL) {
        return;
    }
    switch (f->focus) {
    case NEW_TIMEING_FOCUS_HOUR:
        f->hour = (u8)((f->hour + 23) % 24);
#if ELUNCHBOX_PANEL_EN
        new_timeing_focus_refresh(f);
#endif
        break;
    case NEW_TIMEING_FOCUS_MIN:
        f->min = (u8)((f->min + 59) % 60);
#if ELUNCHBOX_PANEL_EN
        new_timeing_focus_refresh(f);
#endif
        break;
    case NEW_TIMEING_FOCUS_BOTTOM:
        f->bottom_sel = (u8)((f->bottom_sel + 1) & 1);
#if ELUNCHBOX_PANEL_EN
        new_timeing_focus_refresh(f);
#endif
        break;
    default:
        break;
    }
}

static void new_timeing_ok_key(f_new_timeing_t *f)
{
    if (f == NULL) {
        return;
    }
    switch (f->focus) {
    case NEW_TIMEING_FOCUS_HOUR:
        f->focus = NEW_TIMEING_FOCUS_MIN;
        break;
    case NEW_TIMEING_FOCUS_MIN:
        f->focus = NEW_TIMEING_FOCUS_BOTTOM;
        f->bottom_sel = 0;
        break;
    case NEW_TIMEING_FOCUS_BOTTOM:
        if (f->bottom_sel != 0) {
            new_timeing_save_rtc(f);
        }
        func_switch_to(FUNC_NEW_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        return;
    default:
        break;
    }
#if ELUNCHBOX_PANEL_EN
    new_timeing_focus_refresh(f);
#endif
}

static void new_timeing_mode_key(f_new_timeing_t *f)
{
    (void)f;
    if (sys_cb.flag_swithing) {
        return;
    }
    func_switch_to(FUNC_NEW_MODE, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

static void new_timeing_power_key(f_new_timeing_t *f)
{
    if (f == NULL) {
        return;
    }
    switch (f->focus) {
    case NEW_TIMEING_FOCUS_HOUR:
        func_switch_to(FUNC_NEW_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;
    case NEW_TIMEING_FOCUS_MIN:
        f->focus = NEW_TIMEING_FOCUS_HOUR;
#if ELUNCHBOX_PANEL_EN
        new_timeing_focus_refresh(f);
#endif
        break;
    case NEW_TIMEING_FOCUS_BOTTOM:
        f->focus = NEW_TIMEING_FOCUS_MIN;
#if ELUNCHBOX_PANEL_EN
        new_timeing_focus_refresh(f);
#endif
        break;
    default:
        break;
    }
}

static compo_picturebox_t *new_timeing_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);

    if (pic == NULL) {
        return NULL;
    }
    pic->img = (void *)widget_image_create(frm->page_body, 0);
    pic->radix = 1;
    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

static compo_textbox_t *new_timeing_txt_create(compo_form_t *frm, u16 id, u16 buf_size,
                                               s16 x, s16 y, s16 w, s16 h, u16 color)
{
    compo_textbox_t *txt = compo_textbox_create(frm, buf_size);

    compo_setid(txt, id);
    new_timeing_txt_pos_center(txt, x, y, w, h);
    compo_textbox_set_visible(txt, false);
    (void)color;
    return txt;
}

static void new_timeing_bind_objects(f_new_timeing_t *f)
{
    if (f == NULL) {
        return;
    }
    f->txt_title = compo_getobj_byid(COMPO_ID_TXT_TITLE);
    f->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);
    f->pic_hour_bg = compo_getobj_byid(COMPO_ID_PIC_HOUR_BG);
    f->pic_min_bg = compo_getobj_byid(COMPO_ID_PIC_MIN_BG);
    f->pic_hour_up = compo_getobj_byid(COMPO_ID_PIC_HOUR_UP);
    f->pic_hour_down = compo_getobj_byid(COMPO_ID_PIC_HOUR_DOWN);
    f->pic_min_up = compo_getobj_byid(COMPO_ID_PIC_MIN_UP);
    f->pic_min_down = compo_getobj_byid(COMPO_ID_PIC_MIN_DOWN);
    f->pic_h10 = compo_getobj_byid(COMPO_ID_PIC_H10);
    f->pic_h1 = compo_getobj_byid(COMPO_ID_PIC_H1);
    f->txt_colon = compo_getobj_byid(COMPO_ID_TXT_COLON);
    f->pic_m10 = compo_getobj_byid(COMPO_ID_PIC_M10);
    f->pic_m1 = compo_getobj_byid(COMPO_ID_PIC_M1);
    f->pic_no_bg = compo_getobj_byid(COMPO_ID_PIC_NO_BG);
    f->pic_yes_bg = compo_getobj_byid(COMPO_ID_PIC_YES_BG);
    f->txt_no = compo_getobj_byid(COMPO_ID_TXT_NO);
    f->txt_yes = compo_getobj_byid(COMPO_ID_TXT_YES);
}

compo_form_t *func_new_timeing_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, COMPO_ID_SHAPE_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(bg, NEW_TIMEING_PAGE_BG);
    compo_shape_set_radius(bg, 0);

    {
        compo_shape_t *panel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

        compo_setid(panel, COMPO_ID_SHAPE_PANEL);
        compo_shape_set_location(panel, GUI_SCREEN_CENTER_X, NEW_TIMEING_PANEL_Y,
                                 NEW_TIMEING_PANEL_W, NEW_TIMEING_PANEL_H);
        compo_shape_set_color(panel, COLOR_WHITE);
        compo_shape_set_radius(panel, 16);
    }

    {
        compo_textbox_t *txt = compo_textbox_create(frm, 8);

        compo_setid(txt, COMPO_ID_TXT_TITLE);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_location(txt, NEW_TIMEING_TITLE_LEFT, NEW_TIMEING_TITLE_Y,
                                   NEW_TIMEING_TITLE_W, NEW_TIMEING_TITLE_H);
        compo_textbox_set_visible(txt, false);
    }
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(compo_getobj_byid(COMPO_ID_PIC_BT),
                             NEW_TIMEING_STATUS_BT_X, NEW_TIMEING_STATUS_Y);
    compo_picturebox_set_size(compo_getobj_byid(COMPO_ID_PIC_BT),
                             NEW_HOME_BT_W, NEW_HOME_BT_H);
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(compo_getobj_byid(COMPO_ID_PIC_BAT),
                             NEW_TIMEING_STATUS_BAT_X, NEW_TIMEING_STATUS_Y);
    compo_picturebox_set_size(compo_getobj_byid(COMPO_ID_PIC_BAT),
                             NEW_HOME_BAT_W, NEW_HOME_BAT_H);

    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_HOUR_BG);
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_MIN_BG);
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_HOUR_UP);
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_HOUR_DOWN);
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_MIN_UP);
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_MIN_DOWN);
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_H10);
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_H1);
    (void)new_timeing_txt_create(frm, COMPO_ID_TXT_COLON, 2,
                                 NEW_TIMEING_COLON_X, NEW_TIMEING_BOX_Y, 12, 24,
                                 NEW_TIMEING_COLOR_OFF);
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_M10);
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_M1);
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_NO_BG);
    (void)new_timeing_pic_create_hidden(frm, COMPO_ID_PIC_YES_BG);
    {
        compo_textbox_t *txt;

        txt = compo_textbox_create(frm, 4);
        compo_setid(txt, COMPO_ID_TXT_NO);
        compo_textbox_set_location(txt, NEW_TIMEING_BTN_NO_X, NEW_TIMEING_BTN_BOTTOM_Y,
                                   NEW_TIME_BTN_W, NEW_TIME_BTN_H);
        compo_textbox_set_visible(txt, false);

        txt = compo_textbox_create(frm, 4);
        compo_setid(txt, COMPO_ID_TXT_YES);
        compo_textbox_set_location(txt, NEW_TIMEING_BTN_YES_X, NEW_TIMEING_BTN_BOTTOM_Y,
                                   NEW_TIME_BTN_W, NEW_TIME_BTN_H);
        compo_textbox_set_visible(txt, false);
    }
    return frm;
}

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static void new_timeing_pt8028_keys_process(f_new_timeing_t *f)
{
    u8 press_tch;

    if (f == NULL || !f->key_ready) {
        return;
    }
    press_tch = pt8028_take_press_tch();
    if (press_tch == PT8028_KEY_TCH2) {
        new_timeing_value_dec(f);
    } else if (press_tch == PT8028_KEY_TCH6) {
        new_timeing_value_inc(f);
    } else if (press_tch == PT8028_KEY_TCH3) {
        new_timeing_mode_key(f);
    } else if (press_tch == PT8028_KEY_TCH4) {
        new_timeing_ok_key(f);
    } else if (press_tch == PT8028_KEY_TCH5) {
        new_timeing_power_key(f);
    }
}

static void new_timeing_keys_poll(f_new_timeing_t *f)
{
    if (f == NULL || !f->key_ready) {
        return;
    }
    new_timeing_pt8028_keys_process(f);
}
#endif

static void func_new_timeing_message(size_msg_t msg)
{
    f_new_timeing_t *f = (f_new_timeing_t *)func_cb.f_cb;

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
        new_timeing_mode_key(f);
        break;
    case KU_BACK:
        new_timeing_ok_key(f);
        break;
    case KU_VOL_UP:
        new_timeing_value_inc(f);
        break;
    case KU_VOL_DOWN:
        new_timeing_value_dec(f);
        break;
    case KEY_RIGHT | KEY_SHORT_UP:
        new_timeing_power_key(f);
        break;
    default:
        func_message(msg);
        break;
    }
}

static void func_new_timeing_process(void)
{
    f_new_timeing_t *f = (f_new_timeing_t *)func_cb.f_cb;

    if (f == NULL) {
        func_process();
        return;
    }

#if ELUNCHBOX_PANEL_EN
    if (!f->key_ready) {
        WDT_CLR();
        switch (f->load_stage) {
        case NEW_TIMEING_LOAD_STATUS:
            new_timeing_status_refresh(f);
            f->load_stage = NEW_TIMEING_LOAD_BOXES;
            break;
        case NEW_TIMEING_LOAD_BOXES:
            new_timeing_boxes_apply(f);
            f->load_stage = NEW_TIMEING_LOAD_ARROWS;
            break;
        case NEW_TIMEING_LOAD_ARROWS:
            home_gpu_wait_idle();
            WDT_CLR();
            {
                u8 was_blocked = elunchbox_te_block_flag;
                if (!was_blocked) {
                    elunchbox_te_block_flag = 1;
                }
                new_timeing_arrows_apply(f);
                home_gpu_wait_idle();
                if (!was_blocked) {
                    elunchbox_te_block_flag = 0;
                }
            }
            f->load_stage = NEW_TIMEING_LOAD_BTNS;
            break;
        case NEW_TIMEING_LOAD_BTNS:
            new_timeing_btns_apply(f);
            f->load_stage = NEW_TIMEING_LOAD_DIGITS;
            break;
        case NEW_TIMEING_LOAD_DIGITS:
            new_timeing_digits_apply(f);
            f->load_stage = NEW_TIMEING_LOAD_TEXT;
            break;
        case NEW_TIMEING_LOAD_TEXT:
            new_timeing_text_apply(f);
            f->load_stage = NEW_TIMEING_LOAD_DONE;
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
#if ELUNCHBOX_PANEL_EN
        new_timeing_ui_apply(f);
#endif
        f->display_pending = false;
    }

    func_process();
#if ELUNCHBOX_PANEL_EN
    if (!elunchbox_ui_is_live()) {
        return;
    }
    new_timeing_status_refresh(f);
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    new_timeing_keys_poll(f);
#endif
}

void func_new_timeing_enter(void)
{
    f_new_timeing_t *f;

    printf("func_new_timeing_enter\n");

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

    func_cb.f_cb = func_zalloc(sizeof(f_new_timeing_t));
    f = (f_new_timeing_t *)func_cb.f_cb;
    f->focus = NEW_TIMEING_FOCUS_HOUR;
    f->bottom_sel = 0;
#if ELUNCHBOX_PANEL_EN
    f->load_stage = NEW_TIMEING_LOAD_STATUS;
    f->key_ready = false;
    home_ui_digit_pool_reset();
    new_timeing_res_arrow_up_ready = false;
    new_timeing_res_arrow_down_ready = false;
    new_timeing_font_ready = false;
#endif
    f->display_pending = false;

    func_cb.frm_main = func_new_timeing_form_create();
    new_timeing_bind_objects(f);
    new_timeing_parse_rtc(&f->hour, &f->min);

#if ELUNCHBOX_PANEL_EN
    home_ui_shared_status_init();
    WDT_CLR();
    home_gpu_wait_idle();
    os_gui_draw_force();
    home_gpu_wait_idle();
    WDT_CLR();
    elunchbox_te_block_flag = 0;
    tft_bglight_force_on();
#endif
}

void func_new_timeing_exit(void)
{
#if ELUNCHBOX_PANEL_EN
    f_new_timeing_t *f = (f_new_timeing_t *)func_cb.f_cb;

    if (f != NULL) {
        new_timeing_gpu_detach_before_leave(f);
    } else {
        home_ui_shared_battery_detach_pic();
    }
    new_timeing_res_arrow_up_ready = false;
    new_timeing_res_arrow_down_ready = false;
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(0);
    pt8028_release_clear();
#endif
    func_cb.last = FUNC_NEW_TIME;
    printf("func_new_timeing_exit\n");
}

void func_new_timeing(void)
{
    func_new_timeing_enter();
    while (func_cb.sta == FUNC_NEW_TIME) {
        func_new_timeing_process();
        func_new_timeing_message(msg_dequeue());
    }
    func_new_timeing_exit();
}
