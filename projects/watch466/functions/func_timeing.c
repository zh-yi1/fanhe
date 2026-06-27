#include "include.h"
#include "func.h"
#include "home_icon_res.h"
#include "home_ui_shared.h"
#include "ui_layout_anchor.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * Time 设置页 UI（320×240 / 466×466）：返回 + 标题 + 状态栏；时/分 + AM/PM；底部 NO/YES。
 * 图标 bin 保持原始尺寸，不缩放。
 *
 * 320×240 设计图布局（严格按效果图坐标）：
 *   顶栏 0~44：Y=22 — 返回 + "Time" + BT/锁/电量；分隔线 Y=44
 *   时/分框中心 Y=118，框 68×42；箭头 Y=71/165；冒号 X=129
 *   时列 X=76，分列 X=182，AM/PM X=270（AM Y=100，PM Y=136）
 *   H/Min 为文字（与 AMm 同高 10px：H 7×10，Min 19×10）；底部 NO/YES 118×44，中心 (95,219)/(225,219)
 * PT8028：OK 逐步切换 时→分→AM/PM→底部；模式键在 AM/PM 与 NO/YES 间循环；+/- 调节时/分。
 */
#define UI_TIMEING_PLACEHOLDER            UI_BUF_ICON_ACTIVITY_BIN

#ifndef UI_BUF_HOME_LEFT_BIN
#error "Missing left.bin: run tools/convert_ui_home.bat + Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BLUETOOTH_BIN
#error "Missing bluetooth.bin: run tools/gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_LOCK_BIN
#error "Missing lock.bin: run tools/gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_DL4_BIN
#error "Missing dl4.bin: run tools/gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_UP_BIN
#error "Missing up.bin: run tools/convert_ui_home.bat + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_DOWN_BIN
#error "Missing down.bin: run tools/convert_ui_home.bat + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_WCM_BIN
#error "Missing wcm.bin: run tools/convert_ui_home.bat + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_HM_BIN
#error "Missing Hm.bin: run tools/gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_MINM_BIN
#error "Missing Minm.bin: run tools/gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BLUE_AM_BIN
#error "Missing blue_am.bin: run tools/convert_ui_home.bat + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BLACK_AM_BIN
#error "Missing black_am.bin: run tools/convert_ui_home.bat + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BLUE_PM_BIN
#error "Missing blue_pm.bin: run tools/convert_ui_home.bat + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BLACK_PM_BIN
#error "Missing black_pm.bin: run tools/convert_ui_home.bat + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_W0M_BIN
#error "Missing w0m.bin: run tools/convert_ui_home.bat + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_W5M_BIN
#define UI_BUF_HOME_W5M_BIN               UI_BUF_HOME_W5_BIN
#define UI_LEN_HOME_W5M_BIN               UI_LEN_HOME_W5_BIN
#endif

#define TIMEING_COLOR_DIVIDER             make_color(60, 60, 60)
#define TIMEING_COLOR_MIN_BORDER          make_color(60, 60, 60)
#define TIMEING_COLOR_YES                 make_color(4, 109, 217)

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define TIMEING_COLOR_ROW_BG              make_color(34, 34, 34)
#define TIMEING_COLOR_DIVIDER_LINE        make_color(51, 51, 51)
#else
#define TIMEING_COLOR_ROW_BG              make_color(29, 29, 29)
#define TIMEING_COLOR_DIVIDER_LINE        TIMEING_COLOR_DIVIDER
#endif

#define TIMEING_LEFT_W                    11
#define TIMEING_LEFT_H                    17
#define TIMEING_LEFT_RAM_SIZE             (8 + TIMEING_LEFT_W * TIMEING_LEFT_H * 2)

#define TIMEING_ARROW_W                   18
#define TIMEING_ARROW_H                   12
#define TIMEING_ARROW_RAM_SIZE            (8 + TIMEING_ARROW_W * TIMEING_ARROW_H * 2)

#define TIMEING_COLON_W                   5
#define TIMEING_COLON_H                   29
#define TIMEING_COLON_RAM_SIZE            (8 + TIMEING_COLON_W * TIMEING_COLON_H * 2)

#define TIMEING_AMPM_W                    43
#define TIMEING_AMPM_H                    27
#define TIMEING_AMPM_RAM_SIZE             (8 + TIMEING_AMPM_W * TIMEING_AMPM_H * 2)

#define TIMEING_DIGIT_RAM_MAX_SIZE        1436
#define TIMEING_DIGIT_SLOT_CNT            4

#define TIMEING_REF_W                     466
#define TIMEING_REF_H                     466
#define TIMEING_SX(v)                     ((s16)((s32)(v) * GUI_SCREEN_WIDTH / TIMEING_REF_W))
#define TIMEING_SY(v)                     ((s16)((s32)(v) * GUI_SCREEN_HEIGHT / TIMEING_REF_H))

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define TIMEING_HEADER_Y                  22
#define TIMEING_STATUS_Y                  22
#define TIMEING_STATUS_RIGHT_MARGIN       10
#define TIMEING_STATUS_GAP                6
#define TIMEING_BACK_X                    16
#define TIMEING_BACK_BTN_X                6
#define TIMEING_BACK_BTN_Y                4
#define TIMEING_BACK_BTN_W                48
#define TIMEING_BACK_BTN_H                38
#define TIMEING_TITLE_LEFT                28
#define TIMEING_TITLE_Y                   22
#define TIMEING_TITLE_H                   36
#define TIMEING_TITLE_TOP                 (TIMEING_TITLE_Y - TIMEING_TITLE_H / 2)
#define TIMEING_DIVIDER_Y                 44
#define TIMEING_DIVIDER_H                 1
#define TIMEING_HOUR_COL_X                76
#define TIMEING_MIN_COL_X                 182
#define TIMEING_COLON_X                   129
#define TIMEING_AMPM_COL_X                270
#define TIMEING_BOX_Y                     118
#define TIMEING_BOX_W                     78
#define TIMEING_BOX_H                     44
#define TIMEING_BOX_INSET                 3
#define TIMEING_BOX_RADIUS                8
#define TIMEING_ARROW_UP_Y                71
#define TIMEING_ARROW_DOWN_Y              165
#define TIMEING_AM_Y                      100
#define TIMEING_PM_Y                      136
#define TIMEING_DIGIT_GAP                 2
#define TIMEING_ARROW_BTN_DX              18
#define TIMEING_ARROW_BTN_DY              12
#define TIMEING_ARROW_BTN_W               40
#define TIMEING_ARROW_BTN_H               24
#define TIMEING_AMPM_BTN_DX               22
#define TIMEING_AMPM_BTN_DY               14
#define TIMEING_AMPM_BTN_W                48
#define TIMEING_AMPM_BTN_H                28
#define TIMEING_BTN_W                     118
#define TIMEING_BTN_H                     44
#define TIMEING_BTN_RADIUS                8
#define TIMEING_BTN_NO_X                  95
#define TIMEING_BTN_YES_X                 225
#define TIMEING_BTN_BOTTOM_Y              219
#else
#define TIMEING_HEADER_Y                  TIMEING_SY(48)
#define TIMEING_STATUS_Y                  TIMEING_SY(48)
#define TIMEING_STATUS_RIGHT_MARGIN       TIMEING_SX(24)
#define TIMEING_STATUS_GAP                TIMEING_SX(10)
#define TIMEING_BACK_X                    TIMEING_SX(36)
#define TIMEING_BACK_BTN_X                TIMEING_SX(16)
#define TIMEING_BACK_BTN_Y                TIMEING_SY(28)
#define TIMEING_BACK_BTN_W                TIMEING_SX(56)
#define TIMEING_BACK_BTN_H                TIMEING_SY(40)
#define TIMEING_TITLE_LEFT                TIMEING_SX(58)
#define TIMEING_TITLE_Y                   TIMEING_SY(48)
#define TIMEING_TITLE_H                   TIMEING_SY(36)
#define TIMEING_TITLE_TOP                 (TIMEING_TITLE_Y - TIMEING_TITLE_H / 2)
#define TIMEING_DIVIDER_Y                 TIMEING_SY(90)
#define TIMEING_DIVIDER_H                 1
#define TIMEING_HOUR_COL_X                TIMEING_SX(108)
#define TIMEING_MIN_COL_X                 TIMEING_SX(268)
#define TIMEING_COLON_X                   TIMEING_SX(188)
#define TIMEING_AMPM_COL_X                TIMEING_SX(396)
#define TIMEING_BOX_Y                     TIMEING_SY(210)
#define TIMEING_BOX_W                     TIMEING_SX(96)
#define TIMEING_BOX_H                     TIMEING_SY(78)
#define TIMEING_BOX_INSET                 TIMEING_SX(4)
#define TIMEING_BOX_RADIUS                TIMEING_SX(8)
#define TIMEING_ARROW_UP_Y                TIMEING_SY(118)
#define TIMEING_ARROW_DOWN_Y              TIMEING_SY(302)
#define TIMEING_AM_Y                      TIMEING_SY(178)
#define TIMEING_PM_Y                      TIMEING_SY(248)
#define TIMEING_DIGIT_GAP                 TIMEING_SX(10)
#define TIMEING_ARROW_BTN_DX              TIMEING_SX(24)
#define TIMEING_ARROW_BTN_DY              TIMEING_SY(16)
#define TIMEING_ARROW_BTN_W               TIMEING_SX(48)
#define TIMEING_ARROW_BTN_H               TIMEING_SY(32)
#define TIMEING_AMPM_BTN_DX               TIMEING_SX(28)
#define TIMEING_AMPM_BTN_DY               TIMEING_SY(16)
#define TIMEING_AMPM_BTN_W                TIMEING_SX(56)
#define TIMEING_AMPM_BTN_H                TIMEING_SY(32)
#define TIMEING_BTN_W                     TIMEING_SX(145)
#define TIMEING_BTN_H                     TIMEING_SY(48)
#define TIMEING_BTN_RADIUS                TIMEING_SX(8)
#define TIMEING_BTN_NO_X                  TIMEING_SX(118)
#define TIMEING_BTN_YES_X                 TIMEING_SX(348)
#define TIMEING_BTN_BOTTOM_Y              TIMEING_SY(418)
#define TIMEING_TITLE_W                   TIMEING_SX(220)
#endif

/* Time 页时钟：右上角锚点 (320×240) */
#define TIMEING_CLOCK_TR_Y                ((s16)((s32)101 * GUI_SCREEN_HEIGHT / HEAT_LAYOUT_REF_H))
#define TIMEING_CLOCK_TR_H10_X            ((s16)((s32)65 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define TIMEING_CLOCK_TR_H1_X             ((s16)((s32)90 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define TIMEING_CLOCK_TR_COLON_X          ((s16)((s32)125 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define TIMEING_CLOCK_TR_M10_X            ((s16)((s32)165 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define TIMEING_CLOCK_TR_M1_X             ((s16)((s32)190 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))

/* Time 页 H/Min 点阵后缀：右上角锚点 (320×240)，Hm.bin / Minm.bin 8px 高 */
#define TIMEING_SUFFIX_TR_Y               ((s16)((s32)130 * GUI_SCREEN_HEIGHT / HEAT_LAYOUT_REF_H))
#define TIMEING_SUFFIX_H_TR_X             ((s16)((s32)100 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define TIMEING_SUFFIX_MIN_TR_X           ((s16)((s32)210 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))

#define TIMEING_STATUS_BAT_X              (GUI_SCREEN_WIDTH - TIMEING_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define TIMEING_STATUS_LOCK_X             (TIMEING_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - TIMEING_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define TIMEING_STATUS_BT_X               (TIMEING_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - TIMEING_STATUS_GAP - HOME_STATUS_BT_W / 2)

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define TIMEING_TITLE_W                   (TIMEING_STATUS_BT_X - HOME_STATUS_BT_W / 2 - TIMEING_STATUS_GAP - TIMEING_TITLE_LEFT)
#endif

enum {
    TIMEING_FOCUS_HOUR = 0,
    TIMEING_FOCUS_MIN,
    TIMEING_FOCUS_AMPM,
    TIMEING_FOCUS_BOTTOM,
};

#define TIMEING_MSG_OK                      KU_BACK
#define TIMEING_MSG_POWER                   (KEY_RIGHT | KEY_SHORT_UP)

enum {
    COMPO_ID_PIC_BACK = 1,
    COMPO_ID_BTN_BACK,
    COMPO_ID_TITLE,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_LOCK,
    COMPO_ID_PIC_BAT,
    COMPO_ID_HEADER_DIVIDER,
    COMPO_ID_SHAPE_HOUR_BG,
    COMPO_ID_SHAPE_MIN_BORDER,
    COMPO_ID_SHAPE_MIN_BG,
    COMPO_ID_PIC_HOUR_UP,
    COMPO_ID_BTN_HOUR_UP,
    COMPO_ID_PIC_HOUR_DOWN,
    COMPO_ID_BTN_HOUR_DOWN,
    COMPO_ID_PIC_MIN_UP,
    COMPO_ID_BTN_MIN_UP,
    COMPO_ID_PIC_MIN_DOWN,
    COMPO_ID_BTN_MIN_DOWN,
    COMPO_ID_PIC_H10,
    COMPO_ID_PIC_H1,
    COMPO_ID_PIC_COLON,
    COMPO_ID_PIC_M10,
    COMPO_ID_PIC_M1,
    COMPO_ID_PIC_H_SUFFIX,
    COMPO_ID_PIC_MIN_SUFFIX,
    COMPO_ID_PIC_AM,
    COMPO_ID_BTN_AM,
    COMPO_ID_PIC_PM,
    COMPO_ID_BTN_PM,
    COMPO_ID_SHAPE_NO_BG,
    COMPO_ID_SHAPE_YES_BG,
    COMPO_ID_TXT_NO,
    COMPO_ID_TXT_YES,
    COMPO_ID_BTN_NO,
    COMPO_ID_BTN_YES,
};

typedef struct f_timeing_t_ {
    u8 disp_h;
    u8 min;
    u8 focus;
    u8 bottom_sel;
    bool is_pm;
    bool screen_locked;
    compo_shape_t *shape_hour_bg;
    compo_shape_t *shape_min_border;
    compo_shape_t *shape_min_bg;
    compo_shape_t *shape_no_bg;
    compo_shape_t *shape_yes_bg;
    compo_picturebox_t *pic_back;
    compo_button_t *btn_back;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_hour_up;
    compo_picturebox_t *pic_hour_down;
    compo_picturebox_t *pic_min_up;
    compo_picturebox_t *pic_min_down;
    compo_picturebox_t *pic_h10;
    compo_picturebox_t *pic_h1;
    compo_picturebox_t *pic_colon;
    compo_picturebox_t *pic_m10;
    compo_picturebox_t *pic_m1;
    compo_picturebox_t *pic_h_suffix;
    compo_picturebox_t *pic_min_suffix;
    compo_picturebox_t *pic_am;
    compo_picturebox_t *pic_pm;
} f_timeing_t;

static u8 timeing_left_ram[TIMEING_LEFT_RAM_SIZE];
static u8 timeing_arrow_ram[4][TIMEING_ARROW_RAM_SIZE];
static u8 timeing_colon_ram[TIMEING_COLON_RAM_SIZE];
static u8 timeing_ampm_ram[2][TIMEING_AMPM_RAM_SIZE];
static u8 timeing_hm_ram[HOME_TIMEING_SUFFIX_RAM_MAX_SIZE];
static u8 timeing_minm_ram[HOME_TIMEING_SUFFIX_RAM_MAX_SIZE];
static u8 timeing_digit_ram[TIMEING_DIGIT_SLOT_CNT][TIMEING_DIGIT_RAM_MAX_SIZE];

static const u32 tbl_timeing_digit_addr[10] = {
    UI_BUF_HOME_W0M_BIN, UI_BUF_HOME_W1M_BIN, UI_BUF_HOME_W2M_BIN, UI_BUF_HOME_W3M_BIN,
    UI_BUF_HOME_W4M_BIN, UI_BUF_HOME_W5M_BIN, UI_BUF_HOME_W6M_BIN, UI_BUF_HOME_W7M_BIN,
    UI_BUF_HOME_W8M_BIN, UI_BUF_HOME_W9M_BIN,
};

static const u16 tbl_timeing_digit_len[10] = {
    UI_LEN_HOME_W0M_BIN, UI_LEN_HOME_W1M_BIN, UI_LEN_HOME_W2M_BIN, UI_LEN_HOME_W3M_BIN,
    UI_LEN_HOME_W4M_BIN, UI_LEN_HOME_W5M_BIN, UI_LEN_HOME_W6M_BIN, UI_LEN_HOME_W7M_BIN,
    UI_LEN_HOME_W8M_BIN, UI_LEN_HOME_W9M_BIN,
};

static const u16 tbl_timeing_digit_w[10] = {
    20, 5, 20, 19, 20, 20, 21, 19, 20, 20,
};

static const u16 tbl_timeing_digit_h[10] = {
    34, 32, 34, 34, 32, 34, 34, 33, 34, 34,
};

static void func_timeing_pic_pos_tr(compo_picturebox_t *pic, s16 tr_x, s16 tr_y, u16 w, u16 h)
{
    if (pic == NULL) {
        return;
    }
    compo_picturebox_set_pos(pic, tr_x - (s16)(w / 2), tr_y + (s16)(h / 2));
}

static bool func_timeing_gpu_ram_set(u8 *ram, u16 buf_size, u16 data_len, compo_picturebox_t *pic)
{
    u32 magic;
    u16 w;
    u16 h;
    u16 need;

    if (ram == NULL || data_len < 8) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    magic = GET_LE32(&ram[0]);
    w = GET_LE16(&ram[4]);
    h = GET_LE16(&ram[6]);
    need = (u16)(8 + (u32)w * h * 2);
    if (magic != 0x24150 || w == 0 || h == 0 || need != data_len || data_len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    if (pic != NULL) {
        compo_picturebox_set_ram(pic, ram);
        compo_picturebox_set_size(pic, w, h);
        compo_picturebox_set_visible(pic, true);
    }
    return true;
}

static bool func_timeing_gpu_flash_to_ram(u8 *ram, u16 buf_size, u32 flash_addr, u16 flash_len,
                                          compo_picturebox_t *pic)
{
    if (ram == NULL || flash_len == 0 || flash_len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    os_spiflash_read(ram, flash_addr, flash_len);
    return func_timeing_gpu_ram_set(ram, buf_size, flash_len, pic);
}

static void func_timeing_suffix_apply(f_timeing_t *f_timeing)
{
    if (f_timeing == NULL) {
        return;
    }

    if (func_timeing_gpu_flash_to_ram(timeing_hm_ram, HOME_TIMEING_SUFFIX_RAM_MAX_SIZE,
                                      UI_BUF_HOME_HM_BIN, UI_LEN_HOME_HM_BIN,
                                      f_timeing->pic_h_suffix)) {
        func_timeing_pic_pos_tr(f_timeing->pic_h_suffix, TIMEING_SUFFIX_H_TR_X, TIMEING_SUFFIX_TR_Y,
                                HOME_TIMEING_HM_W, HOME_TIMEING_HM_H);
    }

    if (func_timeing_gpu_flash_to_ram(timeing_minm_ram, HOME_TIMEING_SUFFIX_RAM_MAX_SIZE,
                                      UI_BUF_HOME_MINM_BIN, UI_LEN_HOME_MINM_BIN,
                                      f_timeing->pic_min_suffix)) {
        func_timeing_pic_pos_tr(f_timeing->pic_min_suffix, TIMEING_SUFFIX_MIN_TR_X, TIMEING_SUFFIX_TR_Y,
                                HOME_TIMEING_MINM_W, HOME_TIMEING_MINM_H);
    }
}

static compo_shape_t *func_timeing_shape_create(compo_form_t *frm, u16 id, s16 x, s16 y,
                                                s16 w, s16 h, u16 color, u16 radius)
{
    compo_shape_t *shape = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(shape, id);
    compo_shape_set_location(shape, x, y, w, h);
    compo_shape_set_color(shape, color);
    compo_shape_set_radius(shape, radius);
    return shape;
}

static void func_timeing_config_title(compo_textbox_t *txt)
{
    widget_text_t *widget = txt->txt;
    rect_t rect;
    area_t text_area;

    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_align_center(txt, false);
    widget_set_align_center(widget, false);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_location(txt, TIMEING_TITLE_LEFT, TIMEING_TITLE_TOP,
                               TIMEING_TITLE_W, TIMEING_TITLE_H);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, "Time");

    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
}

static void func_timeing_config_center_label(compo_textbox_t *txt, const char *text,
                                             s16 cx, s16 cy, s16 w, s16 h)
{
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_align_center(txt, true);
    widget_set_align_center(txt->txt, true);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(txt->txt, false);
    compo_textbox_set_location(txt, cx, cy, w, h);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, text);
}

static void func_timeing_parse_rtc(u8 *disp_h, u8 *min, bool *is_pm)
{
    tm_t tm = rtc_clock_get();
    u8 hour = tm.hour;

    *is_pm = false;
    if (hour >= 12) {
        *is_pm = true;
    }
    *disp_h = (u8)(hour % 12);
    *min = tm.min;
}

static u8 func_timeing_to_hour24(u8 disp_h, bool is_pm)
{
    if (is_pm) {
        return (disp_h == 0) ? 12 : (u8)(disp_h + 12);
    }
    return disp_h;
}

static bool func_timeing_load_digit(u8 slot, u8 digit, compo_picturebox_t *pic)
{
    if (digit > 9 || slot >= TIMEING_DIGIT_SLOT_CNT) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    if (func_timeing_gpu_flash_to_ram(timeing_digit_ram[slot], TIMEING_DIGIT_RAM_MAX_SIZE,
                                      tbl_timeing_digit_addr[digit],
                                      tbl_timeing_digit_len[digit], pic)) {
        return true;
    }
    return false;
}

static void func_timeing_clock_digit_pos_tr(compo_picturebox_t *pic, u8 digit, s16 tr_x, s16 tr_y)
{
    u16 w;
    u16 h;

    if (digit > 9 || pic == NULL) {
        return;
    }

    w = tbl_timeing_digit_w[digit];
    h = tbl_timeing_digit_h[digit];
    func_timeing_pic_pos_tr(pic, tr_x, tr_y, w, h);
    compo_picturebox_set_size(pic, w, h);
    compo_picturebox_set_visible(pic, true);
}

static void func_timeing_ampm_apply(f_timeing_t *f_timeing)
{
    u32 am_addr;
    u16 am_len;
    u32 pm_addr;
    u16 pm_len;

    if (f_timeing->is_pm) {
        am_addr = UI_BUF_HOME_BLACK_AM_BIN;
        am_len = UI_LEN_HOME_BLACK_AM_BIN;
        pm_addr = UI_BUF_HOME_BLUE_PM_BIN;
        pm_len = UI_LEN_HOME_BLUE_PM_BIN;
    } else {
        am_addr = UI_BUF_HOME_BLUE_AM_BIN;
        am_len = UI_LEN_HOME_BLUE_AM_BIN;
        pm_addr = UI_BUF_HOME_BLACK_PM_BIN;
        pm_len = UI_LEN_HOME_BLACK_PM_BIN;
    }

    func_timeing_gpu_flash_to_ram(timeing_ampm_ram[0], TIMEING_AMPM_RAM_SIZE,
                                  am_addr, am_len, f_timeing->pic_am);
    func_timeing_gpu_flash_to_ram(timeing_ampm_ram[1], TIMEING_AMPM_RAM_SIZE,
                                  pm_addr, pm_len, f_timeing->pic_pm);
}

static void func_timeing_digits_apply(f_timeing_t *f_timeing)
{
    u8 h10 = (u8)(f_timeing->disp_h / 10);
    u8 h1 = (u8)(f_timeing->disp_h % 10);
    u8 m10 = (u8)(f_timeing->min / 10);
    u8 m1 = (u8)(f_timeing->min % 10);

    if (f_timeing == NULL) {
        return;
    }

    if (func_timeing_load_digit(0, h10, f_timeing->pic_h10)) {
        func_timeing_clock_digit_pos_tr(f_timeing->pic_h10, h10,
                                        TIMEING_CLOCK_TR_H10_X, TIMEING_CLOCK_TR_Y);
    }

    if (func_timeing_load_digit(1, h1, f_timeing->pic_h1)) {
        func_timeing_clock_digit_pos_tr(f_timeing->pic_h1, h1,
                                        TIMEING_CLOCK_TR_H1_X, TIMEING_CLOCK_TR_Y);
    }

    if (f_timeing->pic_colon != NULL) {
        func_timeing_pic_pos_tr(f_timeing->pic_colon, TIMEING_CLOCK_TR_COLON_X, TIMEING_CLOCK_TR_Y,
                                TIMEING_COLON_W, TIMEING_COLON_H);
        compo_picturebox_set_size(f_timeing->pic_colon, TIMEING_COLON_W, TIMEING_COLON_H);
        compo_picturebox_set_visible(f_timeing->pic_colon, true);
    }

    if (func_timeing_load_digit(2, m10, f_timeing->pic_m10)) {
        func_timeing_clock_digit_pos_tr(f_timeing->pic_m10, m10,
                                        TIMEING_CLOCK_TR_M10_X, TIMEING_CLOCK_TR_Y);
    }

    if (func_timeing_load_digit(3, m1, f_timeing->pic_m1)) {
        func_timeing_clock_digit_pos_tr(f_timeing->pic_m1, m1,
                                        TIMEING_CLOCK_TR_M1_X, TIMEING_CLOCK_TR_Y);
    }

    func_timeing_suffix_apply(f_timeing);

    func_timeing_ampm_apply(f_timeing);
}

void func_timeing_lock_icon_apply(f_timeing_t *f_timeing);

static void func_timeing_status_icons_apply(f_timeing_t *f_timeing)
{
    home_ui_shared_status_init();

    if (f_timeing->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f_timeing->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f_timeing->pic_bt, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
        compo_picturebox_set_visible(f_timeing->pic_bt, true);
    }
    if (f_timeing->pic_lock != NULL && gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
        compo_picturebox_set_ram(f_timeing->pic_lock, home_ui_shared_status_lock_ram);
        compo_picturebox_set_size(f_timeing->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    }
    home_ui_shared_status_bind_bat(f_timeing->pic_bat);

    func_timeing_lock_icon_apply(f_timeing);
}

void func_timeing_lock_icon_apply(f_timeing_t *f_timeing)
{
    if (f_timeing == NULL || f_timeing->pic_lock == NULL) {
        return;
    }
    if (func_key_lock_show_status_icon(f_timeing->screen_locked)) {
        home_ui_shared_status_init();
        compo_picturebox_set_pos(f_timeing->pic_lock, TIMEING_STATUS_LOCK_X, TIMEING_STATUS_Y);
        if (gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
            compo_picturebox_set_ram(f_timeing->pic_lock, home_ui_shared_status_lock_ram);
            compo_picturebox_set_size(f_timeing->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
            compo_picturebox_set_visible(f_timeing->pic_lock, true);
        }
    } else {
        compo_picturebox_set_visible(f_timeing->pic_lock, false);
    }
}

static void func_timeing_bottom_btn_refresh(f_timeing_t *f_timeing)
{
    if (f_timeing->shape_no_bg == NULL || f_timeing->shape_yes_bg == NULL) {
        return;
    }

    if (f_timeing->focus == TIMEING_FOCUS_BOTTOM) {
        if (f_timeing->bottom_sel == 0) {
            compo_shape_set_color(f_timeing->shape_no_bg, TIMEING_COLOR_MIN_BORDER);
            compo_shape_set_color(f_timeing->shape_yes_bg, TIMEING_COLOR_ROW_BG);
        } else {
            compo_shape_set_color(f_timeing->shape_no_bg, TIMEING_COLOR_ROW_BG);
            compo_shape_set_color(f_timeing->shape_yes_bg, TIMEING_COLOR_YES);
        }
    } else {
        compo_shape_set_color(f_timeing->shape_no_bg, TIMEING_COLOR_ROW_BG);
        compo_shape_set_color(f_timeing->shape_yes_bg, TIMEING_COLOR_YES);
    }
}

static void func_timeing_focus_refresh(f_timeing_t *f_timeing)
{
    bool hour_sel;
    bool min_sel;

    if (f_timeing == NULL) {
        return;
    }

    hour_sel = (f_timeing->focus == TIMEING_FOCUS_HOUR);
    min_sel = (f_timeing->focus == TIMEING_FOCUS_MIN);

    if (f_timeing->shape_hour_bg != NULL) {
        compo_shape_set_visible(f_timeing->shape_hour_bg, hour_sel);
    }
    if (f_timeing->shape_min_border != NULL) {
        compo_shape_set_visible(f_timeing->shape_min_border, true);
    }
    if (f_timeing->shape_min_bg != NULL) {
        compo_shape_set_color(f_timeing->shape_min_bg,
                              min_sel ? TIMEING_COLOR_ROW_BG : COLOR_BLACK);
    }

    func_timeing_bottom_btn_refresh(f_timeing);
}

static void func_timeing_save_rtc(f_timeing_t *f_timeing)
{
    tm_t tm_set = rtc_clock_get();

    tm_set.hour = func_timeing_to_hour24(f_timeing->disp_h, f_timeing->is_pm);
    tm_set.min = f_timeing->min;
    rtc_clock_set(tm_set);
}

static void func_timeing_ok_key(f_timeing_t *f_timeing)
{
    if (f_timeing == NULL) {
        return;
    }

    switch (f_timeing->focus) {
    case TIMEING_FOCUS_HOUR:
        f_timeing->focus = TIMEING_FOCUS_MIN;
        break;

    case TIMEING_FOCUS_MIN:
        f_timeing->focus = TIMEING_FOCUS_AMPM;
        break;

    case TIMEING_FOCUS_AMPM:
        f_timeing->focus = TIMEING_FOCUS_BOTTOM;
        f_timeing->bottom_sel = 0;
        break;

    case TIMEING_FOCUS_BOTTOM:
        if (f_timeing->bottom_sel != 0) {
            func_timeing_save_rtc(f_timeing);
        }
        func_switch_to(FUNC_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        return;

    default:
        break;
    }

    func_timeing_focus_refresh(f_timeing);
}

static void func_timeing_mode_key(f_timeing_t *f_timeing)
{
    if (f_timeing == NULL) {
        return;
    }

    switch (f_timeing->focus) {
    case TIMEING_FOCUS_AMPM:
        f_timeing->is_pm = !f_timeing->is_pm;
        func_timeing_ampm_apply(f_timeing);
        break;

    case TIMEING_FOCUS_BOTTOM:
        f_timeing->bottom_sel = (u8)((f_timeing->bottom_sel + 1) & 1);
        func_timeing_bottom_btn_refresh(f_timeing);
        break;

    default:
        break;
    }
}

static void func_timeing_power_key(f_timeing_t *f_timeing)
{
    if (f_timeing == NULL) {
        return;
    }

    switch (f_timeing->focus) {
    case TIMEING_FOCUS_HOUR:
        func_switch_to(FUNC_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    case TIMEING_FOCUS_MIN:
        f_timeing->focus = TIMEING_FOCUS_HOUR;
        func_timeing_focus_refresh(f_timeing);
        break;

    case TIMEING_FOCUS_AMPM:
        f_timeing->focus = TIMEING_FOCUS_MIN;
        func_timeing_focus_refresh(f_timeing);
        break;

    case TIMEING_FOCUS_BOTTOM:
        f_timeing->focus = TIMEING_FOCUS_AMPM;
        func_timeing_focus_refresh(f_timeing);
        break;

    default:
        break;
    }
}

static void func_timeing_value_inc(f_timeing_t *f_timeing)
{
    if (f_timeing == NULL) {
        return;
    }

    switch (f_timeing->focus) {
    case TIMEING_FOCUS_HOUR:
        f_timeing->disp_h = (u8)((f_timeing->disp_h + 1) % 12);
        func_timeing_digits_apply(f_timeing);
        break;

    case TIMEING_FOCUS_MIN:
        f_timeing->min = (u8)((f_timeing->min + 1) % 60);
        func_timeing_digits_apply(f_timeing);
        break;

    default:
        break;
    }
}

static void func_timeing_value_dec(f_timeing_t *f_timeing)
{
    if (f_timeing == NULL) {
        return;
    }

    switch (f_timeing->focus) {
    case TIMEING_FOCUS_HOUR:
        f_timeing->disp_h = (u8)((f_timeing->disp_h + 11) % 12);
        func_timeing_digits_apply(f_timeing);
        break;

    case TIMEING_FOCUS_MIN:
        f_timeing->min = (u8)((f_timeing->min + 59) % 60);
        func_timeing_digits_apply(f_timeing);
        break;

    default:
        break;
    }
}

static void func_timeing_button_click(void)
{
    int id = compo_get_button_id();
    f_timeing_t *f_timeing = (f_timeing_t *)func_cb.f_cb;

    if (f_timeing == NULL) {
        return;
    }

    switch (id) {
    case COMPO_ID_BTN_BACK:
    case COMPO_ID_BTN_NO:
        func_switch_to(FUNC_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    case COMPO_ID_BTN_YES:
        func_timeing_save_rtc(f_timeing);
        func_switch_to(FUNC_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    case COMPO_ID_BTN_HOUR_UP:
        f_timeing->disp_h = (u8)((f_timeing->disp_h + 1) % 12);
        func_timeing_digits_apply(f_timeing);
        break;

    case COMPO_ID_BTN_HOUR_DOWN:
        f_timeing->disp_h = (u8)((f_timeing->disp_h + 11) % 12);
        func_timeing_digits_apply(f_timeing);
        break;

    case COMPO_ID_BTN_MIN_UP:
        f_timeing->min = (u8)((f_timeing->min + 1) % 60);
        func_timeing_digits_apply(f_timeing);
        break;

    case COMPO_ID_BTN_MIN_DOWN:
        f_timeing->min = (u8)((f_timeing->min + 59) % 60);
        func_timeing_digits_apply(f_timeing);
        break;

    case COMPO_ID_BTN_AM:
        f_timeing->is_pm = false;
        func_timeing_ampm_apply(f_timeing);
        break;

    case COMPO_ID_BTN_PM:
        f_timeing->is_pm = true;
        func_timeing_ampm_apply(f_timeing);
        break;

    default:
        break;
    }
}

compo_form_t *func_timeing_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_picturebox_t *pic;
    compo_textbox_t *txt;
    compo_button_t *btn;

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BACK);
    compo_picturebox_set_pos(pic, TIMEING_BACK_X, TIMEING_HEADER_Y);
    compo_picturebox_set_size(pic, TIMEING_LEFT_W, TIMEING_LEFT_H);
    compo_picturebox_set_visible(pic, false);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_BACK);
    compo_button_set_location(btn, TIMEING_BACK_BTN_X, TIMEING_BACK_BTN_Y,
                              TIMEING_BACK_BTN_W, TIMEING_BACK_BTN_H);

    txt = compo_textbox_create(frm, 8);
    compo_setid(txt, COMPO_ID_TITLE);
    func_timeing_config_title(txt);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, TIMEING_STATUS_BT_X, TIMEING_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_LOCK);
    compo_picturebox_set_pos(pic, TIMEING_STATUS_LOCK_X, TIMEING_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, TIMEING_STATUS_BAT_X, TIMEING_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
    compo_picturebox_set_visible(pic, false);

    func_timeing_shape_create(frm, COMPO_ID_HEADER_DIVIDER, GUI_SCREEN_CENTER_X, TIMEING_DIVIDER_Y,
                              GUI_SCREEN_WIDTH, TIMEING_DIVIDER_H, TIMEING_COLOR_DIVIDER_LINE, 0);

    func_timeing_shape_create(frm, COMPO_ID_SHAPE_HOUR_BG, TIMEING_HOUR_COL_X, TIMEING_BOX_Y,
                              TIMEING_BOX_W, TIMEING_BOX_H, TIMEING_COLOR_ROW_BG, TIMEING_BOX_RADIUS);

    func_timeing_shape_create(frm, COMPO_ID_SHAPE_MIN_BORDER, TIMEING_MIN_COL_X, TIMEING_BOX_Y,
                              TIMEING_BOX_W, TIMEING_BOX_H, TIMEING_COLOR_MIN_BORDER, TIMEING_BOX_RADIUS);
    func_timeing_shape_create(frm, COMPO_ID_SHAPE_MIN_BG, TIMEING_MIN_COL_X, TIMEING_BOX_Y,
                              TIMEING_BOX_W - 2, TIMEING_BOX_H - 2, COLOR_BLACK, TIMEING_BOX_RADIUS - 1);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_HOUR_UP);
    compo_picturebox_set_pos(pic, TIMEING_HOUR_COL_X, TIMEING_ARROW_UP_Y);
    compo_picturebox_set_size(pic, TIMEING_ARROW_W, TIMEING_ARROW_H);
    compo_picturebox_set_visible(pic, false);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_HOUR_UP);
    compo_button_set_location(btn, TIMEING_HOUR_COL_X - TIMEING_ARROW_BTN_DX,
                              TIMEING_ARROW_UP_Y - TIMEING_ARROW_BTN_DY,
                              TIMEING_ARROW_BTN_W, TIMEING_ARROW_BTN_H);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_HOUR_DOWN);
    compo_picturebox_set_pos(pic, TIMEING_HOUR_COL_X, TIMEING_ARROW_DOWN_Y);
    compo_picturebox_set_size(pic, TIMEING_ARROW_W, TIMEING_ARROW_H);
    compo_picturebox_set_visible(pic, false);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_HOUR_DOWN);
    compo_button_set_location(btn, TIMEING_HOUR_COL_X - TIMEING_ARROW_BTN_DX,
                              TIMEING_ARROW_DOWN_Y - TIMEING_ARROW_BTN_DY,
                              TIMEING_ARROW_BTN_W, TIMEING_ARROW_BTN_H);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_MIN_UP);
    compo_picturebox_set_pos(pic, TIMEING_MIN_COL_X, TIMEING_ARROW_UP_Y);
    compo_picturebox_set_size(pic, TIMEING_ARROW_W, TIMEING_ARROW_H);
    compo_picturebox_set_visible(pic, false);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_MIN_UP);
    compo_button_set_location(btn, TIMEING_MIN_COL_X - TIMEING_ARROW_BTN_DX,
                              TIMEING_ARROW_UP_Y - TIMEING_ARROW_BTN_DY,
                              TIMEING_ARROW_BTN_W, TIMEING_ARROW_BTN_H);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_MIN_DOWN);
    compo_picturebox_set_pos(pic, TIMEING_MIN_COL_X, TIMEING_ARROW_DOWN_Y);
    compo_picturebox_set_size(pic, TIMEING_ARROW_W, TIMEING_ARROW_H);
    compo_picturebox_set_visible(pic, false);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_MIN_DOWN);
    compo_button_set_location(btn, TIMEING_MIN_COL_X - TIMEING_ARROW_BTN_DX,
                              TIMEING_ARROW_DOWN_Y - TIMEING_ARROW_BTN_DY,
                              TIMEING_ARROW_BTN_W, TIMEING_ARROW_BTN_H);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_H10);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_H1);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_COLON);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_M10);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_M1);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_AM);
    compo_picturebox_set_pos(pic, TIMEING_AMPM_COL_X, TIMEING_AM_Y);
    compo_picturebox_set_size(pic, TIMEING_AMPM_W, TIMEING_AMPM_H);
    compo_picturebox_set_visible(pic, false);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_AM);
    compo_button_set_location(btn, TIMEING_AMPM_COL_X - TIMEING_AMPM_BTN_DX,
                              TIMEING_AM_Y - TIMEING_AMPM_BTN_DY,
                              TIMEING_AMPM_BTN_W, TIMEING_AMPM_BTN_H);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_PM);
    compo_picturebox_set_pos(pic, TIMEING_AMPM_COL_X, TIMEING_PM_Y);
    compo_picturebox_set_size(pic, TIMEING_AMPM_W, TIMEING_AMPM_H);
    compo_picturebox_set_visible(pic, false);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_PM);
    compo_button_set_location(btn, TIMEING_AMPM_COL_X - TIMEING_AMPM_BTN_DX,
                              TIMEING_PM_Y - TIMEING_AMPM_BTN_DY,
                              TIMEING_AMPM_BTN_W, TIMEING_AMPM_BTN_H);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_H_SUFFIX);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_TIMEING_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_MIN_SUFFIX);
    compo_picturebox_set_visible(pic, false);

    func_timeing_shape_create(frm, COMPO_ID_SHAPE_NO_BG, TIMEING_BTN_NO_X, TIMEING_BTN_BOTTOM_Y,
                              TIMEING_BTN_W, TIMEING_BTN_H, TIMEING_COLOR_ROW_BG, TIMEING_BTN_RADIUS);
    func_timeing_shape_create(frm, COMPO_ID_SHAPE_YES_BG, TIMEING_BTN_YES_X, TIMEING_BTN_BOTTOM_Y,
                              TIMEING_BTN_W, TIMEING_BTN_H, TIMEING_COLOR_YES, TIMEING_BTN_RADIUS);

    txt = compo_textbox_create(frm, 4);
    compo_setid(txt, COMPO_ID_TXT_NO);
    func_timeing_config_center_label(txt, "NO", TIMEING_BTN_NO_X, TIMEING_BTN_BOTTOM_Y,
                                     TIMEING_BTN_W, TIMEING_BTN_H);

    txt = compo_textbox_create(frm, 4);
    compo_setid(txt, COMPO_ID_TXT_YES);
    func_timeing_config_center_label(txt, "YES", TIMEING_BTN_YES_X, TIMEING_BTN_BOTTOM_Y,
                                      TIMEING_BTN_W, TIMEING_BTN_H);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_NO);
    compo_button_set_location(btn, TIMEING_BTN_NO_X - TIMEING_BTN_W / 2,
                            TIMEING_BTN_BOTTOM_Y - TIMEING_BTN_H / 2,
                            TIMEING_BTN_W, TIMEING_BTN_H);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_YES);
    compo_button_set_location(btn, TIMEING_BTN_YES_X - TIMEING_BTN_W / 2,
                            TIMEING_BTN_BOTTOM_Y - TIMEING_BTN_H / 2,
                            TIMEING_BTN_W, TIMEING_BTN_H);

    return frm;
}

static void func_timeing_process(void)
{
    func_process();
}

static void func_timeing_message(size_msg_t msg)
{
    f_timeing_t *f_timeing = (f_timeing_t *)func_cb.f_cb;

    if (func_key_lock_ku_blocked(msg)) {
        return;
    }

    if (f_timeing != NULL && f_timeing->screen_locked) {
        if (msg != TIMEING_MSG_POWER && msg != TIMEING_MSG_OK) {
            return;
        }
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        func_timeing_button_click();
        break;

    case KU_LEFT:
        break;

    case KU_MODE:
        func_timeing_mode_key(f_timeing);
        break;

    case TIMEING_MSG_OK:
        func_timeing_ok_key(f_timeing);
        break;

    case TIMEING_MSG_POWER:
        func_timeing_power_key(f_timeing);
        break;

    case KU_VOL_UP:
        func_timeing_value_inc(f_timeing);
        break;

    case KU_VOL_DOWN:
        func_timeing_value_dec(f_timeing);
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_timeing_enter(void)
{
    f_timeing_t *f_timeing;

    func_cb.f_cb = func_zalloc(sizeof(f_timeing_t));
    func_cb.frm_main = func_timeing_form_create();

    f_timeing = (f_timeing_t *)func_cb.f_cb;
    f_timeing->focus = TIMEING_FOCUS_HOUR;
    f_timeing->bottom_sel = 0;
    f_timeing->screen_locked = false;
    f_timeing->shape_hour_bg = compo_getobj_byid(COMPO_ID_SHAPE_HOUR_BG);
    f_timeing->shape_min_border = compo_getobj_byid(COMPO_ID_SHAPE_MIN_BORDER);
    f_timeing->shape_min_bg = compo_getobj_byid(COMPO_ID_SHAPE_MIN_BG);
    f_timeing->shape_no_bg = compo_getobj_byid(COMPO_ID_SHAPE_NO_BG);
    f_timeing->shape_yes_bg = compo_getobj_byid(COMPO_ID_SHAPE_YES_BG);
    f_timeing->pic_back = compo_getobj_byid(COMPO_ID_PIC_BACK);
    f_timeing->btn_back = compo_getobj_byid(COMPO_ID_BTN_BACK);
    f_timeing->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f_timeing->pic_lock = compo_getobj_byid(COMPO_ID_PIC_LOCK);
    f_timeing->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);
    home_ui_shared_battery_attach_pic(f_timeing->pic_bat);
    f_timeing->pic_hour_up = compo_getobj_byid(COMPO_ID_PIC_HOUR_UP);
    f_timeing->pic_hour_down = compo_getobj_byid(COMPO_ID_PIC_HOUR_DOWN);
    f_timeing->pic_min_up = compo_getobj_byid(COMPO_ID_PIC_MIN_UP);
    f_timeing->pic_min_down = compo_getobj_byid(COMPO_ID_PIC_MIN_DOWN);
    f_timeing->pic_h10 = compo_getobj_byid(COMPO_ID_PIC_H10);
    f_timeing->pic_h1 = compo_getobj_byid(COMPO_ID_PIC_H1);
    f_timeing->pic_colon = compo_getobj_byid(COMPO_ID_PIC_COLON);
    f_timeing->pic_m10 = compo_getobj_byid(COMPO_ID_PIC_M10);
    f_timeing->pic_m1 = compo_getobj_byid(COMPO_ID_PIC_M1);
    f_timeing->pic_am = compo_getobj_byid(COMPO_ID_PIC_AM);
    f_timeing->pic_pm = compo_getobj_byid(COMPO_ID_PIC_PM);
    f_timeing->pic_h_suffix = compo_getobj_byid(COMPO_ID_PIC_H_SUFFIX);
    f_timeing->pic_min_suffix = compo_getobj_byid(COMPO_ID_PIC_MIN_SUFFIX);

    func_timeing_parse_rtc(&f_timeing->disp_h, &f_timeing->min, &f_timeing->is_pm);

    func_timeing_gpu_flash_to_ram(timeing_left_ram, TIMEING_LEFT_RAM_SIZE,
                                  UI_BUF_HOME_LEFT_BIN, UI_LEN_HOME_LEFT_BIN,
                                  f_timeing->pic_back);
    func_timeing_gpu_flash_to_ram(timeing_arrow_ram[0], TIMEING_ARROW_RAM_SIZE,
                                  UI_BUF_HOME_UP_BIN, UI_LEN_HOME_UP_BIN,
                                  f_timeing->pic_hour_up);
    func_timeing_gpu_flash_to_ram(timeing_arrow_ram[1], TIMEING_ARROW_RAM_SIZE,
                                  UI_BUF_HOME_DOWN_BIN, UI_LEN_HOME_DOWN_BIN,
                                  f_timeing->pic_hour_down);
    func_timeing_gpu_flash_to_ram(timeing_arrow_ram[2], TIMEING_ARROW_RAM_SIZE,
                                  UI_BUF_HOME_UP_BIN, UI_LEN_HOME_UP_BIN,
                                  f_timeing->pic_min_up);
    func_timeing_gpu_flash_to_ram(timeing_arrow_ram[3], TIMEING_ARROW_RAM_SIZE,
                                  UI_BUF_HOME_DOWN_BIN, UI_LEN_HOME_DOWN_BIN,
                                  f_timeing->pic_min_down);
    func_timeing_gpu_flash_to_ram(timeing_colon_ram, TIMEING_COLON_RAM_SIZE,
                                  UI_BUF_HOME_WCM_BIN, UI_LEN_HOME_WCM_BIN,
                                  f_timeing->pic_colon);

    func_timeing_status_icons_apply(f_timeing);
    func_timeing_digits_apply(f_timeing);
    func_timeing_focus_refresh(f_timeing);
}

void func_timeing_exit(void)
{
    home_ui_shared_battery_detach_pic();
    func_cb.last = FUNC_TIMEING;
}

void func_timeing(void)
{
    printf("%s\n", __func__);
    func_timeing_enter();
    while (func_cb.sta == FUNC_TIMEING) {
        func_timeing_process();
        func_timeing_message(msg_dequeue());
    }
    func_timeing_exit();
}
