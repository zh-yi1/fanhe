#include "include.h"
#include "func.h"
#include "new_home_icon_res.h"
#include "new_heat_res.h"
#include "home_ui_shared.h"
#include "home_ui_gpu_detach.h"
#include "home_ui_ram.h"
#include "func_lunchbox_uart.h"
#include "func_key_lock.h"

#if ELUNCHBOX_PANEL_EN
extern volatile u8 elunchbox_te_block_flag;
#endif

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

#ifndef UI_BUF_NEW_UI_NEW_TEMP_1_BIN
#error "Run tools/gen_new_heat_icons.py then Output/bin/prebuild.bat"
#endif

/*
 * 新加热设置页（320×240 白底）：
 *   顶栏：蓝牙 + 电量（new_dl, new_dl1..new_dl4，按原逻辑显示）
 *   温度条 new_temp_1..5，时长条 new_time_1..13，圆点 new_point
 *   默认 140°F / 1 小时；先调温度，确认后调时长，再确认开始加热
 * PT8028：TCH4 确认 | TCH5 返回 | TCH2/TCH6 减/加
 */
#define NEW_HEAT_STATUS_Y                 20
#define NEW_HEAT_STATUS_RIGHT_MARGIN      10
#define NEW_HEAT_STATUS_GAP               6

#define NEW_HEAT_TEMP_LABEL_Y             58
#define NEW_HEAT_TEMP_SLIDER_Y            82
#define NEW_HEAT_TEMP_SCALE_Y             100

#define NEW_HEAT_TIME_LABEL_Y             142
#define NEW_HEAT_TIME_SLIDER_Y            166
#define NEW_HEAT_TIME_SCALE_Y             184

#define NEW_HEAT_LABEL_X                  20
#define NEW_HEAT_BADGE_X                  ((s16)(GUI_SCREEN_WIDTH - NEW_HEAT_STATUS_RIGHT_MARGIN - NEW_HEAT_BADGE_W / 2))
#define NEW_HEAT_SLIDER_SLOT_X            ((s16)((GUI_SCREEN_WIDTH - NEW_HEAT_SLIDER_W) / 2))

#define NEW_HEAT_COLOR_LABEL              0x0AD8
#define NEW_HEAT_COLOR_SCALE              0xC618
#define NEW_HEAT_COLOR_ON_BADGE           COLOR_WHITE
#define NEW_HEAT_COLOR_OFF_BADGE          0x0AD8

#define NEW_HEAT_MSG_OK                   KU_BACK
#define NEW_HEAT_MSG_PLUS                 KU_VOL_UP
#define NEW_HEAT_MSG_MINUS                KU_VOL_DOWN
#define NEW_HEAT_MSG_POWER                (KEY_RIGHT | KEY_SHORT_UP)

enum {
    COMPO_ID_SHAPE_BG = 1,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_BAT,
    COMPO_ID_PIC_TEMP_TRACK,
    COMPO_ID_PIC_TIME_TRACK,
    COMPO_ID_PIC_TEMP_POINT,
    COMPO_ID_PIC_TIME_POINT,
    COMPO_ID_PIC_TEMP_BADGE,
    COMPO_ID_PIC_TIME_BADGE,
    COMPO_ID_TXT_TEMP_LABEL,
    COMPO_ID_TXT_TIME_LABEL,
    COMPO_ID_TXT_TEMP_VAL,
    COMPO_ID_TXT_TIME_VAL,
    COMPO_ID_TXT_TEMP_SCALE0,
    COMPO_ID_TXT_TEMP_SCALE4 = COMPO_ID_TXT_TEMP_SCALE0 + 4,
    COMPO_ID_TXT_TIME_SCALE0,
    COMPO_ID_TXT_TIME_SCALE2 = COMPO_ID_TXT_TIME_SCALE0 + 2,
};

enum {
    NEW_HEAT_FOCUS_TEMP = 0,
    NEW_HEAT_FOCUS_TIME,
};

typedef struct {
    u8 focus;
    u8 temp_idx;
    u8 time_idx;
    bool display_pending;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_temp_track;
    compo_picturebox_t *pic_time_track;
    compo_picturebox_t *pic_temp_point;
    compo_picturebox_t *pic_time_point;
    compo_picturebox_t *pic_temp_badge;
    compo_picturebox_t *pic_time_badge;
    compo_textbox_t *txt_temp_label;
    compo_textbox_t *txt_time_label;
    compo_textbox_t *txt_temp_val;
    compo_textbox_t *txt_time_val;
    compo_textbox_t *txt_temp_scale[NEW_HEAT_TEMP_CNT];
    compo_textbox_t *txt_time_scale[3];
} f_new_heat_t;

/* 温度值：140°F, 158°F, 176°F, 194°F, 212°F */
static const u16 tbl_new_heat_temp_f[NEW_HEAT_TEMP_CNT] = {
    140, 158, 176, 194, 212,
};

/* 时长（分钟）：60min(1H) ~ 120min(2H)，每档+5min */
static const u16 tbl_new_heat_time_min[NEW_HEAT_TIME_CNT] = {
    60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120,
};

/* 时间刻度尺显示三档：索引 0(1H)、6(1H30min)、12(2H) */
static const u8 tbl_new_heat_time_scale_idx[3] = { 0, 6, 12 };

static void new_heat_white_bg_create(compo_form_t *frm)
{
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, COMPO_ID_SHAPE_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_radius(bg, 0);
}

/* placeholder：widget_icon_create(addr=0) 不读 flash，避免 C281；勿 compo_picturebox_create。 */
static compo_picturebox_t *new_heat_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);

    /* 必须用 widget_image_create 而非 widget_icon_create：
     * - widget_icon_create(page, 0) 在 GPU 扫描时触发 C482
     * - widget_image_create(page, 0) 能安全处理 addr=0（如 compo_rowbox 所用）
     * - picturebox->img 的类型是 widget_image_t *，与 widget_icon_t 不兼容 */
    pic->img = (void *)widget_image_create(frm->page_body, 0);
    pic->radix = 1;
    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

#if ELUNCHBOX_PANEL_EN
#define NEW_HEAT_RAM_TEMP_TRACK         home_ui_digit_ram[0]
#define NEW_HEAT_RAM_TIME_TRACK         home_ui_digit_ram[1]
#define NEW_HEAT_RAM_TEMP_BADGE         home_ui_digit_ram[2]
#define NEW_HEAT_RAM_TIME_BADGE         home_ui_digit_ram[3]
#define NEW_HEAT_RAM_POINT              home_ui_colon_ram
#define NEW_HEAT_RAM_TRACK_CAP          HOME_DIGIT_RAM_MAX_SIZE
#define NEW_HEAT_RAM_BADGE_CAP          HOME_DIGIT_RAM_MAX_SIZE
#define NEW_HEAT_RAM_POINT_CAP          HOME_COLON_RAM_SIZE

static bool new_heat_gpu_ram_bind(u8 *ram, u16 buf_size, u32 addr, u16 len,
                                  compo_picturebox_t *pic, u16 w, u16 h, s16 x, s16 y)
{
    u16 need;

    if (pic == NULL || addr == 0 || len == 0) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }
    if (ram == NULL || len > buf_size) {
        compo_picturebox_set_visible(pic, false);
        return false;
    }

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

static void new_heat_pic_apply(compo_picturebox_t *pic, u32 addr, u16 len,
                               u8 *ram, u16 ram_cap, u16 w, u16 h, s16 x, s16 y)
{
    if (pic == NULL || addr == 0) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return;
    }
    new_heat_gpu_ram_bind(ram, ram_cap, addr, len, pic, w, h, x, y);
}
#else
#define NEW_HEAT_RAM_TEMP_TRACK         NULL
#define NEW_HEAT_RAM_TIME_TRACK         NULL
#define NEW_HEAT_RAM_TEMP_BADGE         NULL
#define NEW_HEAT_RAM_TIME_BADGE         NULL
#define NEW_HEAT_RAM_POINT              NULL
#define NEW_HEAT_RAM_TRACK_CAP          0
#define NEW_HEAT_RAM_BADGE_CAP          0
#define NEW_HEAT_RAM_POINT_CAP          0

static void new_heat_pic_apply(compo_picturebox_t *pic, u32 addr, u16 len,
                               u8 *ram, u16 ram_cap, u16 w, u16 h, s16 x, s16 y)
{
    if (pic == NULL || addr == 0) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return;
    }
    home_gpu_wait_idle();
    compo_picturebox_set(pic, addr);
    compo_picturebox_set_size(pic, w, h);
    compo_picturebox_set_pos(pic, x, y);
    compo_picturebox_set_visible(pic, true);
    (void)len;
    (void)ram;
    (void)ram_cap;
}
#endif

static u32 new_heat_temp_track_addr(u8 idx)
{
    static const u32 tbl[NEW_HEAT_TEMP_CNT] = {
        UI_BUF_NEW_UI_NEW_TEMP_1_BIN,
        UI_BUF_NEW_UI_NEW_TEMP_2_BIN,
        UI_BUF_NEW_UI_NEW_TEMP_3_BIN,
        UI_BUF_NEW_UI_NEW_TEMP_4_BIN,
        UI_BUF_NEW_UI_NEW_TEMP_5_BIN,
    };

    if (idx >= NEW_HEAT_TEMP_CNT) {
        return 0;
    }
    return tbl[idx];
}

static u16 new_heat_temp_track_w(u8 idx)
{
    static const u16 tbl_w[NEW_HEAT_TEMP_CNT] = {
        NEW_HEAT_NEW_TEMP_1_W,
        NEW_HEAT_NEW_TEMP_2_W,
        NEW_HEAT_NEW_TEMP_3_W,
        NEW_HEAT_NEW_TEMP_4_W,
        NEW_HEAT_NEW_TEMP_5_W,
    };

    if (idx >= NEW_HEAT_TEMP_CNT) {
        return NEW_HEAT_SLIDER_W;
    }
    return tbl_w[idx];
}

static u32 new_heat_time_track_addr(u8 idx)
{
    static const u32 tbl[NEW_HEAT_TIME_CNT] = {
        UI_BUF_NEW_UI_NEW_TIME_1_BIN,
        UI_BUF_NEW_UI_NEW_TIME_2_BIN,
        UI_BUF_NEW_UI_NEW_TIME_3_BIN,
        UI_BUF_NEW_UI_NEW_TIME_4_BIN,
        UI_BUF_NEW_UI_NEW_TIME_5_BIN,
        UI_BUF_NEW_UI_NEW_TIME_6_BIN,
        UI_BUF_NEW_UI_NEW_TIME_7_BIN,
        UI_BUF_NEW_UI_NEW_TIME_8_BIN,
        UI_BUF_NEW_UI_NEW_TIME_9_BIN,
        UI_BUF_NEW_UI_NEW_TIME_10_BIN,
        UI_BUF_NEW_UI_NEW_TIME_11_BIN,
        UI_BUF_NEW_UI_NEW_TIME_12_BIN,
        UI_BUF_NEW_UI_NEW_TIME_13_BIN,
    };

    if (idx >= NEW_HEAT_TIME_CNT) {
        return 0;
    }
    return tbl[idx];
}

static u16 new_heat_temp_track_len(u8 idx)
{
    static const u16 tbl[NEW_HEAT_TEMP_CNT] = {
        UI_LEN_NEW_UI_NEW_TEMP_1_BIN,
        UI_LEN_NEW_UI_NEW_TEMP_2_BIN,
        UI_LEN_NEW_UI_NEW_TEMP_3_BIN,
        UI_LEN_NEW_UI_NEW_TEMP_4_BIN,
        UI_LEN_NEW_UI_NEW_TEMP_5_BIN,
    };

    if (idx >= NEW_HEAT_TEMP_CNT) {
        return 0;
    }
    return tbl[idx];
}

static u16 new_heat_time_track_len(u8 idx)
{
    static const u16 tbl[NEW_HEAT_TIME_CNT] = {
        UI_LEN_NEW_UI_NEW_TIME_1_BIN,
        UI_LEN_NEW_UI_NEW_TIME_2_BIN,
        UI_LEN_NEW_UI_NEW_TIME_3_BIN,
        UI_LEN_NEW_UI_NEW_TIME_4_BIN,
        UI_LEN_NEW_UI_NEW_TIME_5_BIN,
        UI_LEN_NEW_UI_NEW_TIME_6_BIN,
        UI_LEN_NEW_UI_NEW_TIME_7_BIN,
        UI_LEN_NEW_UI_NEW_TIME_8_BIN,
        UI_LEN_NEW_UI_NEW_TIME_9_BIN,
        UI_LEN_NEW_UI_NEW_TIME_10_BIN,
        UI_LEN_NEW_UI_NEW_TIME_11_BIN,
        UI_LEN_NEW_UI_NEW_TIME_12_BIN,
        UI_LEN_NEW_UI_NEW_TIME_13_BIN,
    };

    if (idx >= NEW_HEAT_TIME_CNT) {
        return 0;
    }
    return tbl[idx];
}

static u16 new_heat_time_track_w(u8 idx)
{
    /* 所有时间进度条均为 245px 宽 */
    (void)idx;
    return NEW_HEAT_NEW_TIME_1_W;
}

/* 温度与时间轨道高度相同（均为 7px） */
#define NEW_HEAT_TRACK_H                  NEW_HEAT_NEW_TEMP_1_H

static s16 new_heat_track_left(u16 track_w)
{
    return (s16)(NEW_HEAT_SLIDER_SLOT_X + (NEW_HEAT_SLIDER_W - track_w) / 2);
}

static s16 new_heat_point_x(u8 idx, u8 max_idx, u16 track_w)
{
    u16 span;
    s16 left;

    if (max_idx == 0) {
        return (s16)(NEW_HEAT_SLIDER_SLOT_X + NEW_HEAT_SLIDER_W / 2);
    }
    left = new_heat_track_left(track_w);
    span = track_w > NEW_HEAT_POINT_W ? (u16)(track_w - NEW_HEAT_POINT_W) : 0;
    return (s16)(left + NEW_HEAT_POINT_W / 2 + (u32)idx * span / max_idx);
}

static void new_heat_format_temp(char *buf, u16 temp_f)
{
    sprintf(buf, "%uF", temp_f);
}

static void new_heat_format_duration(char *buf, u16 total_min)
{
    u8 hour = (u8)(total_min / 60);
    u8 min = (u8)(total_min % 60);

    if (min == 0) {
        sprintf(buf, "%uH", hour);
    } else if (min == 30) {
        sprintf(buf, "%uH30min", hour);
    } else {
        sprintf(buf, "%uH%umin", hour, min);
    }
}

static void new_heat_status_icons_apply(f_new_heat_t *f)
{
    if (f == NULL) {
        return;
    }
    home_ui_shared_status_init();
    if (f->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f->pic_bt, NEW_HOME_BT_W, NEW_HOME_BT_H);
        compo_picturebox_set_visible(f->pic_bt, true);
    }
    if (f->pic_bat != NULL && gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        home_ui_shared_battery_attach_pic(f->pic_bat);
    }
}

static void new_heat_track_apply(compo_picturebox_t *pic, u32 addr, u16 len,
                                 u8 *ram, u16 ram_cap, u16 track_w, s16 y)
{
    if (pic == NULL) {
        return;
    }
    if (addr == 0 || len == 0) {
        compo_picturebox_set_visible(pic, false);
        return;
    }
    new_heat_pic_apply(pic, addr, len, ram, ram_cap, track_w, NEW_HEAT_TRACK_H,
                       (s16)(new_heat_track_left(track_w) + track_w / 2), y);
}

static void new_heat_point_apply(compo_picturebox_t *pic, u8 idx, u8 max_idx, u16 track_w, s16 y, bool visible)
{
    if (pic == NULL) {
        return;
    }
    if (!visible) {
        compo_picturebox_set_visible(pic, false);
        return;
    }
    new_heat_pic_apply(pic, UI_BUF_NEW_UI_NEW_POINT_BIN, UI_LEN_NEW_UI_NEW_POINT_BIN,
                       NEW_HEAT_RAM_POINT, NEW_HEAT_RAM_POINT_CAP,
                       NEW_HEAT_POINT_W, NEW_HEAT_POINT_H,
                       new_heat_point_x(idx, max_idx, track_w), y);
}

static void new_heat_temp_track_apply(f_new_heat_t *f)
{
    if (f == NULL) {
        return;
    }
    new_heat_track_apply(f->pic_temp_track, new_heat_temp_track_addr(f->temp_idx),
                         new_heat_temp_track_len(f->temp_idx),
                         NEW_HEAT_RAM_TEMP_TRACK, NEW_HEAT_RAM_TRACK_CAP,
                         new_heat_temp_track_w(f->temp_idx), NEW_HEAT_TEMP_SLIDER_Y);
}

static void new_heat_time_track_apply(f_new_heat_t *f)
{
    if (f == NULL) {
        return;
    }
    new_heat_track_apply(f->pic_time_track, new_heat_time_track_addr(f->time_idx),
                         new_heat_time_track_len(f->time_idx),
                         NEW_HEAT_RAM_TIME_TRACK, NEW_HEAT_RAM_TRACK_CAP,
                         new_heat_time_track_w(f->time_idx), NEW_HEAT_TIME_SLIDER_Y);
}

static void new_heat_temp_point_apply(f_new_heat_t *f)
{
    u16 temp_w;

    if (f == NULL) {
        return;
    }
    temp_w = new_heat_temp_track_w(f->temp_idx);
    new_heat_point_apply(f->pic_temp_point, f->temp_idx, NEW_HEAT_TEMP_CNT - 1, temp_w,
                         NEW_HEAT_TEMP_SLIDER_Y, f->focus == NEW_HEAT_FOCUS_TEMP);
}

static void new_heat_time_point_apply(f_new_heat_t *f)
{
    u16 time_w;

    if (f == NULL) {
        return;
    }
    time_w = new_heat_time_track_w(f->time_idx);
    new_heat_point_apply(f->pic_time_point, f->time_idx, NEW_HEAT_TIME_CNT - 1, time_w,
                         NEW_HEAT_TIME_SLIDER_Y, f->focus == NEW_HEAT_FOCUS_TIME);
}

static void new_heat_temp_badge_apply(f_new_heat_t *f)
{
    u32 addr;
    u16 len;

    if (f == NULL || f->pic_temp_badge == NULL) {
        return;
    }
    addr = (f->focus == NEW_HEAT_FOCUS_TEMP)
        ? UI_BUF_NEW_UI_NEW_BLUE_TIME_BIN : UI_BUF_NEW_UI_NEW_WHITE_TIME_BIN;
    len = (f->focus == NEW_HEAT_FOCUS_TEMP)
        ? UI_LEN_NEW_UI_NEW_BLUE_TIME_BIN : UI_LEN_NEW_UI_NEW_WHITE_TIME_BIN;
    new_heat_pic_apply(f->pic_temp_badge, addr, len,
                       NEW_HEAT_RAM_TEMP_BADGE, NEW_HEAT_RAM_BADGE_CAP,
                       NEW_HEAT_BADGE_W, NEW_HEAT_BADGE_H,
                       NEW_HEAT_BADGE_X, NEW_HEAT_TEMP_LABEL_Y);
}

static void new_heat_time_badge_apply(f_new_heat_t *f)
{
    u32 addr;
    u16 len;

    if (f == NULL || f->pic_time_badge == NULL) {
        return;
    }
    addr = (f->focus == NEW_HEAT_FOCUS_TIME)
        ? UI_BUF_NEW_UI_NEW_BLUE_TIME_BIN : UI_BUF_NEW_UI_NEW_WHITE_TIME_BIN;
    len = (f->focus == NEW_HEAT_FOCUS_TIME)
        ? UI_LEN_NEW_UI_NEW_BLUE_TIME_BIN : UI_LEN_NEW_UI_NEW_WHITE_TIME_BIN;
    new_heat_pic_apply(f->pic_time_badge, addr, len,
                       NEW_HEAT_RAM_TIME_BADGE, NEW_HEAT_RAM_BADGE_CAP,
                       NEW_HEAT_BADGE_W, NEW_HEAT_BADGE_H,
                       NEW_HEAT_BADGE_X, NEW_HEAT_TIME_LABEL_Y);
}

static void new_heat_tracks_apply(f_new_heat_t *f)
{
    if (f == NULL) {
        return;
    }
    new_heat_temp_track_apply(f);
    new_heat_time_track_apply(f);
}

static void new_heat_badges_apply(f_new_heat_t *f)
{
    if (f == NULL) {
        return;
    }
    new_heat_temp_point_apply(f);
    new_heat_time_point_apply(f);
    new_heat_temp_badge_apply(f);
    new_heat_time_badge_apply(f);
}

#define NEW_HEAT_GPU_SETTLE_FRAMES      5

#if ELUNCHBOX_PANEL_EN
static void new_heat_text_apply_main(f_new_heat_t *f);
static void new_heat_text_apply_scales(f_new_heat_t *f);
#endif

static void new_heat_text_apply(f_new_heat_t *f)
{
    char buf[24];
    u16 temp_w;
    u16 time_w;
    u8 i;

    if (f == NULL) {
        return;
    }

#if ELUNCHBOX_PANEL_EN
    new_heat_text_apply_main(f);
    new_heat_text_apply_scales(f);
    return;
#endif

    temp_w = new_heat_temp_track_w(f->temp_idx);
    time_w = new_heat_time_track_w(f->time_idx);

    if (f->txt_temp_label != NULL) {
        compo_textbox_set(f->txt_temp_label, "Heating Temp");
        compo_textbox_set_visible(f->txt_temp_label, true);
    }
    if (f->txt_time_label != NULL) {
        compo_textbox_set(f->txt_time_label, "Heating Duration");
        compo_textbox_set_visible(f->txt_time_label, true);
    }
    if (f->txt_temp_val != NULL) {
        new_heat_format_temp(buf, tbl_new_heat_temp_f[f->temp_idx]);
        compo_textbox_set(f->txt_temp_val, buf);
        compo_textbox_set_forecolor(f->txt_temp_val,
            f->focus == NEW_HEAT_FOCUS_TEMP ? NEW_HEAT_COLOR_ON_BADGE : NEW_HEAT_COLOR_OFF_BADGE);
        compo_textbox_set_align_center(f->txt_temp_val, true);
        compo_textbox_set_visible(f->txt_temp_val, true);
    }
    if (f->txt_time_val != NULL) {
        new_heat_format_duration(buf, tbl_new_heat_time_min[f->time_idx]);
        compo_textbox_set(f->txt_time_val, buf);
        compo_textbox_set_forecolor(f->txt_time_val,
            f->focus == NEW_HEAT_FOCUS_TIME ? NEW_HEAT_COLOR_ON_BADGE : NEW_HEAT_COLOR_OFF_BADGE);
        compo_textbox_set_align_center(f->txt_time_val, true);
        compo_textbox_set_visible(f->txt_time_val, true);
    }

    for (i = 0; i < NEW_HEAT_TEMP_CNT; i++) {
        if (f->txt_temp_scale[i] != NULL) {
            new_heat_format_temp(buf, tbl_new_heat_temp_f[i]);
            compo_textbox_set(f->txt_temp_scale[i], buf);
            compo_textbox_set_pos(f->txt_temp_scale[i],
                                  new_heat_point_x(i, NEW_HEAT_TEMP_CNT - 1, temp_w),
                                  NEW_HEAT_TEMP_SCALE_Y);
            compo_textbox_set_visible(f->txt_temp_scale[i], true);
        }
    }

    for (i = 0; i < 3; i++) {
        u8 tidx = tbl_new_heat_time_scale_idx[i];

        if (f->txt_time_scale[i] != NULL) {
            new_heat_format_duration(buf, tbl_new_heat_time_min[tidx]);
            compo_textbox_set(f->txt_time_scale[i], buf);
            compo_textbox_set_pos(f->txt_time_scale[i],
                                  new_heat_point_x(tidx, NEW_HEAT_TIME_CNT - 1, time_w),
                                  NEW_HEAT_TIME_SCALE_Y);
            compo_textbox_set_visible(f->txt_time_scale[i], true);
        }
    }
}

#if ELUNCHBOX_PANEL_EN
static void new_heat_text_apply_main(f_new_heat_t *f)
{
    char buf[24];

    if (f == NULL) {
        return;
    }
    if (f->txt_temp_label == NULL) {
        compo_textbox_t *t = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_TEMP_LABEL);
        if (t == NULL && func_cb.frm_main != NULL) {
            t = compo_textbox_create(func_cb.frm_main, 24);
            compo_setid(t, COMPO_ID_TXT_TEMP_LABEL);
            compo_textbox_set_wholewrap(t, false);
            compo_textbox_set_pos(t, NEW_HEAT_LABEL_X, NEW_HEAT_TEMP_LABEL_Y);
            compo_textbox_set_forecolor(t, NEW_HEAT_COLOR_LABEL);
        }
        f->txt_temp_label = t;
    }
    if (f->txt_time_label == NULL) {
        compo_textbox_t *t = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_TIME_LABEL);
        if (t == NULL && func_cb.frm_main != NULL) {
            t = compo_textbox_create(func_cb.frm_main, 24);
            compo_setid(t, COMPO_ID_TXT_TIME_LABEL);
            compo_textbox_set_wholewrap(t, false);
            compo_textbox_set_pos(t, NEW_HEAT_LABEL_X, NEW_HEAT_TIME_LABEL_Y);
            compo_textbox_set_forecolor(t, NEW_HEAT_COLOR_LABEL);
        }
        f->txt_time_label = t;
    }
    if (f->txt_temp_val == NULL) {
        compo_textbox_t *t = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_TEMP_VAL);
        if (t == NULL && func_cb.frm_main != NULL) {
            t = compo_textbox_create(func_cb.frm_main, 24);
            compo_setid(t, COMPO_ID_TXT_TEMP_VAL);
            compo_textbox_set_wholewrap(t, false);
            compo_textbox_set_pos(t, NEW_HEAT_BADGE_X, NEW_HEAT_TEMP_LABEL_Y);
            compo_textbox_set_forecolor(t, NEW_HEAT_COLOR_ON_BADGE);
        }
        f->txt_temp_val = t;
    }
    if (f->txt_time_val == NULL) {
        compo_textbox_t *t = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_TIME_VAL);
        if (t == NULL && func_cb.frm_main != NULL) {
            t = compo_textbox_create(func_cb.frm_main, 24);
            compo_setid(t, COMPO_ID_TXT_TIME_VAL);
            compo_textbox_set_wholewrap(t, false);
            compo_textbox_set_pos(t, NEW_HEAT_BADGE_X, NEW_HEAT_TIME_LABEL_Y);
            compo_textbox_set_forecolor(t, NEW_HEAT_COLOR_OFF_BADGE);
        }
        f->txt_time_val = t;
    }
    if (f->txt_temp_label != NULL) {
        /* 用默认系统字体 → 不读 Flash，避免 C281 */
        compo_textbox_set_autosize(f->txt_temp_label, true);
        compo_textbox_set(f->txt_temp_label, "Heating Temp");
        compo_textbox_set_visible(f->txt_temp_label, true);
    }
    if (f->txt_time_label != NULL) {
        compo_textbox_set_autosize(f->txt_time_label, true);
        compo_textbox_set(f->txt_time_label, "Heating Duration");
        compo_textbox_set_visible(f->txt_time_label, true);
    }
    if (f->txt_temp_val != NULL) {
        compo_textbox_set_autosize(f->txt_temp_val, true);
        compo_textbox_set_align_center(f->txt_temp_val, true);
        new_heat_format_temp(buf, tbl_new_heat_temp_f[f->temp_idx]);
        compo_textbox_set(f->txt_temp_val, buf);
        compo_textbox_set_forecolor(f->txt_temp_val,
            f->focus == NEW_HEAT_FOCUS_TEMP ? NEW_HEAT_COLOR_ON_BADGE : NEW_HEAT_COLOR_OFF_BADGE);
        compo_textbox_set_visible(f->txt_temp_val, true);
    }
    if (f->txt_time_val != NULL) {
        compo_textbox_set_autosize(f->txt_time_val, true);
        compo_textbox_set_align_center(f->txt_time_val, true);
        new_heat_format_duration(buf, tbl_new_heat_time_min[f->time_idx]);
        compo_textbox_set(f->txt_time_val, buf);
        compo_textbox_set_forecolor(f->txt_time_val,
            f->focus == NEW_HEAT_FOCUS_TIME ? NEW_HEAT_COLOR_ON_BADGE : NEW_HEAT_COLOR_OFF_BADGE);
        compo_textbox_set_visible(f->txt_time_val, true);
    }
}

static void new_heat_text_apply_scales(f_new_heat_t *f)
{
    char buf[24];
    u16 temp_w;
    u16 time_w;
    u8 i;

    if (f == NULL) {
        return;
    }
    for (i = 0; i < NEW_HEAT_TEMP_CNT; i++) {
        if (f->txt_temp_scale[i] == NULL) {
            compo_textbox_t *t = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_TEMP_SCALE0 + i);
            if (t == NULL && func_cb.frm_main != NULL) {
                t = compo_textbox_create(func_cb.frm_main, 24);
                compo_setid(t, COMPO_ID_TXT_TEMP_SCALE0 + i);
                compo_textbox_set_wholewrap(t, false);
                compo_textbox_set_pos(t, NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TEMP_SCALE_Y);
                compo_textbox_set_forecolor(t, NEW_HEAT_COLOR_SCALE);
            }
            f->txt_temp_scale[i] = t;
        }
    }
    for (i = 0; i < 3; i++) {
        if (f->txt_time_scale[i] == NULL) {
            compo_textbox_t *t = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_TIME_SCALE0 + i);
            if (t == NULL && func_cb.frm_main != NULL) {
                t = compo_textbox_create(func_cb.frm_main, 24);
                compo_setid(t, COMPO_ID_TXT_TIME_SCALE0 + i);
                compo_textbox_set_wholewrap(t, false);
                compo_textbox_set_pos(t, NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TIME_SCALE_Y);
                compo_textbox_set_forecolor(t, NEW_HEAT_COLOR_SCALE);
            }
            f->txt_time_scale[i] = t;
        }
    }
    for (i = 0; i < NEW_HEAT_TEMP_CNT; i++) {
        if (f->txt_temp_scale[i] != NULL) {
            compo_textbox_set_autosize(f->txt_temp_scale[i], true);
            compo_textbox_set_align_center(f->txt_temp_scale[i], true);
        }
    }
    for (i = 0; i < 3; i++) {
        if (f->txt_time_scale[i] != NULL) {
            compo_textbox_set_autosize(f->txt_time_scale[i], true);
            compo_textbox_set_align_center(f->txt_time_scale[i], true);
        }
    }
    temp_w = new_heat_temp_track_w(f->temp_idx);
    time_w = new_heat_time_track_w(f->time_idx);
    for (i = 0; i < NEW_HEAT_TEMP_CNT; i++) {
        if (f->txt_temp_scale[i] != NULL) {
            new_heat_format_temp(buf, tbl_new_heat_temp_f[i]);
            compo_textbox_set(f->txt_temp_scale[i], buf);
            compo_textbox_set_pos(f->txt_temp_scale[i],
                                  new_heat_point_x(i, NEW_HEAT_TEMP_CNT - 1, temp_w),
                                  NEW_HEAT_TEMP_SCALE_Y);
            compo_textbox_set_visible(f->txt_temp_scale[i], true);
        }
    }
    for (i = 0; i < 3; i++) {
        u8 tidx = tbl_new_heat_time_scale_idx[i];

        if (f->txt_time_scale[i] != NULL) {
            new_heat_format_duration(buf, tbl_new_heat_time_min[tidx]);
            compo_textbox_set(f->txt_time_scale[i], buf);
            compo_textbox_set_pos(f->txt_time_scale[i],
                                  new_heat_point_x(tidx, NEW_HEAT_TIME_CNT - 1, time_w),
                                  NEW_HEAT_TIME_SCALE_Y);
            compo_textbox_set_visible(f->txt_time_scale[i], true);
        }
    }
}
#endif

static void new_heat_ui_refresh(f_new_heat_t *f)
{
    if (f == NULL) {
        return;
    }
    WDT_CLR();
#if !ELUNCHBOX_PANEL_EN
    home_gpu_wait_idle();
#endif
    new_heat_tracks_apply(f);
    new_heat_badges_apply(f);
    new_heat_text_apply(f);
}

#define new_heat_sliders_apply(f)       new_heat_ui_refresh(f)

static void new_heat_bind_objects(f_new_heat_t *f)
{
    u8 i;

    f->pic_bt = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_BT);
    f->pic_bat = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_BAT);
    f->pic_temp_track = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_TEMP_TRACK);
    f->pic_time_track = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_TIME_TRACK);
    f->pic_temp_point = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_TEMP_POINT);
    f->pic_time_point = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_TIME_POINT);
    f->pic_temp_badge = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_TEMP_BADGE);
    f->pic_time_badge = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_TIME_BADGE);
    f->txt_temp_label = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_TEMP_LABEL);
    f->txt_time_label = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_TIME_LABEL);
    f->txt_temp_val = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_TEMP_VAL);
    f->txt_time_val = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_TIME_VAL);
    for (i = 0; i < NEW_HEAT_TEMP_CNT; i++) {
        f->txt_temp_scale[i] = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_TEMP_SCALE0 + i);
    }
    for (i = 0; i < 3; i++) {
        f->txt_time_scale[i] = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_TIME_SCALE0 + i);
    }
}

/* 加减键：在温度/时间进度条上移动圆形图标（new_point） */
static void new_heat_value_inc(f_new_heat_t *f)
{
    if (f == NULL) {
        return;
    }
    if (f->focus == NEW_HEAT_FOCUS_TEMP) {
        if (f->temp_idx + 1 < NEW_HEAT_TEMP_CNT) {
            f->temp_idx++;
        }
    } else if (f->time_idx + 1 < NEW_HEAT_TIME_CNT) {
        f->time_idx++;
    }
    new_heat_ui_refresh(f);
}

static void new_heat_value_dec(f_new_heat_t *f)
{
    if (f == NULL) {
        return;
    }
    if (f->focus == NEW_HEAT_FOCUS_TEMP) {
        if (f->temp_idx > 0) {
            f->temp_idx--;
        }
    } else if (f->time_idx > 0) {
        f->time_idx--;
    }
    new_heat_ui_refresh(f);
}

/* 确认键：先确认温度（切换到时长设置），再确认时长（开始加热） */
static void new_heat_ok_key(f_new_heat_t *f)
{
    u16 temp_f;
    u16 total_min;
    u8 hour;
    u8 min;

    if (f == NULL) {
        return;
    }
    if (f->focus == NEW_HEAT_FOCUS_TEMP) {
        /* 温度已确认，切换到时长设置 */
        f->focus = NEW_HEAT_FOCUS_TIME;
        new_heat_ui_refresh(f);
        return;
    }

    /* 时长已确认，调用加热函数开始加热 */
    temp_f = tbl_new_heat_temp_f[f->temp_idx];
    total_min = tbl_new_heat_time_min[f->time_idx];
    hour = (u8)(total_min / 60);
    min = (u8)(total_min % 60);

    lb_mode_to_heat_set(1, temp_f, hour, min);
    lb_heat_autostart_set(true);
    func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

/* 返回/电源键：时长设置中返回温度设置，温度设置中返回主页 */
static void new_heat_power_key(f_new_heat_t *f)
{
    if (f == NULL) {
        return;
    }
    if (f->focus == NEW_HEAT_FOCUS_TIME) {
        f->focus = NEW_HEAT_FOCUS_TEMP;
        new_heat_ui_refresh(f);
        return;
    }
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

/* 定期刷新状态栏（蓝牙、电量电池图标按原始逻辑显示） */
static void new_heat_status_refresh(f_new_heat_t *f)
{
    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    /* 电量图标通过 home_ui_shared_battery_attach_pic 绑定后自动刷新 */
    /* 无需额外操作，UART DP 数据到达时会通过 home_ui_shared_battery_feed_dp 自动更新 */
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

static compo_textbox_t *new_heat_txt_create(compo_form_t *frm, u16 id, u32 font_addr,
                                            s16 x, s16 y, u16 color, bool center)
{
    compo_textbox_t *txt = compo_textbox_create(frm, 24);

    compo_setid(txt, id);
    compo_textbox_set_wholewrap(txt, false);
#if ELUNCHBOX_PANEL_EN
    /* ELUNCHBOX：不能在此 setup 字体（set_font/autosize/align_center 会读取
     * flash 字体数据→触发 GPU guard C281）。移到 new_heat_text_apply
     * （enter 安全上下文）中统一设置。 */
    compo_textbox_set_visible(txt, false);
#else
    compo_textbox_set_font(txt, font_addr);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, center);
#endif
    compo_textbox_set_pos(txt, x, y);
    compo_textbox_set_forecolor(txt, color);
    return txt;
}

compo_form_t *func_new_heat_form_create(void)
{
    compo_form_t *frm = compo_form_create(false);
    compo_picturebox_t *pic;
    s16 bat_x;
    s16 bt_x;

    new_heat_white_bg_create(frm);

    bat_x = (s16)(GUI_SCREEN_WIDTH - NEW_HEAT_STATUS_RIGHT_MARGIN - NEW_HOME_BAT_W / 2);
    bt_x = (s16)(bat_x - NEW_HOME_BAT_W / 2 - NEW_HEAT_STATUS_GAP - NEW_HOME_BT_W / 2);

    pic = new_heat_pic_create_hidden(frm, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, bt_x, NEW_HEAT_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);

    pic = new_heat_pic_create_hidden(frm, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, bat_x, NEW_HEAT_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);

    new_heat_txt_create(frm, COMPO_ID_TXT_TEMP_LABEL, UI_BUF_0FONT_FONT_ASC_BIN,
                        NEW_HEAT_LABEL_X, NEW_HEAT_TEMP_LABEL_Y, NEW_HEAT_COLOR_LABEL, false);
    new_heat_txt_create(frm, COMPO_ID_TXT_TIME_LABEL, UI_BUF_0FONT_FONT_ASC_BIN,
                        NEW_HEAT_LABEL_X, NEW_HEAT_TIME_LABEL_Y, NEW_HEAT_COLOR_LABEL, false);

    pic = new_heat_pic_create_hidden(frm, COMPO_ID_PIC_TEMP_BADGE);
    compo_picturebox_set_pos(pic, NEW_HEAT_BADGE_X, NEW_HEAT_TEMP_LABEL_Y);
    compo_picturebox_set_size(pic, NEW_HEAT_BADGE_W, NEW_HEAT_BADGE_H);

    pic = new_heat_pic_create_hidden(frm, COMPO_ID_PIC_TIME_BADGE);
    compo_picturebox_set_pos(pic, NEW_HEAT_BADGE_X, NEW_HEAT_TIME_LABEL_Y);
    compo_picturebox_set_size(pic, NEW_HEAT_BADGE_W, NEW_HEAT_BADGE_H);

    new_heat_txt_create(frm, COMPO_ID_TXT_TEMP_VAL, UI_BUF_0FONT_FONT_ASC_12_BIN,
                        NEW_HEAT_BADGE_X, NEW_HEAT_TEMP_LABEL_Y, NEW_HEAT_COLOR_ON_BADGE, true);
    new_heat_txt_create(frm, COMPO_ID_TXT_TIME_VAL, UI_BUF_0FONT_FONT_ASC_12_BIN,
                        NEW_HEAT_BADGE_X, NEW_HEAT_TIME_LABEL_Y, NEW_HEAT_COLOR_OFF_BADGE, true);

    pic = new_heat_pic_create_hidden(frm, COMPO_ID_PIC_TEMP_TRACK);
    compo_picturebox_set_pos(pic, NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TEMP_SLIDER_Y);
    compo_picturebox_set_size(pic, NEW_HEAT_SLIDER_W, NEW_HEAT_TRACK_H);

    pic = new_heat_pic_create_hidden(frm, COMPO_ID_PIC_TIME_TRACK);
    compo_picturebox_set_pos(pic, NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TIME_SLIDER_Y);
    compo_picturebox_set_size(pic, NEW_HEAT_SLIDER_W, NEW_HEAT_TRACK_H);

    pic = new_heat_pic_create_hidden(frm, COMPO_ID_PIC_TEMP_POINT);
    compo_picturebox_set_pos(pic, NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TEMP_SLIDER_Y);
    compo_picturebox_set_size(pic, NEW_HEAT_POINT_W, NEW_HEAT_POINT_H);

    pic = new_heat_pic_create_hidden(frm, COMPO_ID_PIC_TIME_POINT);
    compo_picturebox_set_pos(pic, NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TIME_SLIDER_Y);
    compo_picturebox_set_size(pic, NEW_HEAT_POINT_W, NEW_HEAT_POINT_H);

    for (u8 i = 0; i < NEW_HEAT_TEMP_CNT; i++) {
        new_heat_txt_create(frm, COMPO_ID_TXT_TEMP_SCALE0 + i, UI_BUF_0FONT_FONT_ASC_12_BIN,
                            NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TEMP_SCALE_Y,
                            NEW_HEAT_COLOR_SCALE, true);
    }
    for (u8 j = 0; j < 3; j++) {
        new_heat_txt_create(frm, COMPO_ID_TXT_TIME_SCALE0 + j, UI_BUF_0FONT_FONT_ASC_12_BIN,
                            NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TIME_SCALE_Y,
                            NEW_HEAT_COLOR_SCALE, true);
    }

    return frm;
}

static void func_new_heat_message(size_msg_t msg)
{
    f_new_heat_t *f = (f_new_heat_t *)func_cb.f_cb;

    if (sys_cb.flag_swithing) {
        return;
    }
    if (func_key_lock_ku_blocked(msg)) {
        return;
    }
    /* 吞掉 Home 确认键残留的 KU_BACK，否则会 func_back_to() 立刻退回 Home */
    switch (msg) {
    case KU_BACK:
    case KU_MODE:
    case KU_VOL_UP:
    case KU_VOL_DOWN:
    case KU_RIGHT:
        return;
    default:
        break;
    }

    switch (msg) {
    case NEW_HEAT_MSG_OK:
        new_heat_ok_key(f);
        break;
    case NEW_HEAT_MSG_PLUS:
        new_heat_value_inc(f);
        break;
    case NEW_HEAT_MSG_MINUS:
        new_heat_value_dec(f);
        break;
    case NEW_HEAT_MSG_POWER:
        new_heat_power_key(f);
        break;
    default:
        func_message(msg);
        break;
    }
}

static void func_new_heat_process(void)
{
    f_new_heat_t *f = (f_new_heat_t *)func_cb.f_cb;

    if (f == NULL) {
        func_process();
        return;
    }

    new_heat_status_refresh(f);
    func_process();
}

void func_new_heat_enter(void)
{
    f_new_heat_t *f;

    printf("func_new_heat_enter\n");
    printf("eh_a\n");

#if ELUNCHBOX_PANEL_EN
    printf("eh_b\n");
    home_gpu_wait_idle();
    printf("eh_c\n");
    WDT_CLR();
    printf("eh_d\n");
#endif

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    printf("eh_e\n");
    func_home_drain_stale_key_msgs();
    printf("eh_f\n");
    pt8028_release_clear();
    printf("eh_g\n");
#endif
    msg_queue_detach(KU_BACK, 0);
    printf("eh_h\n");
    msg_queue_detach(KU_VOL_UP, 0);
    printf("eh_i\n");
    msg_queue_detach(KU_VOL_DOWN, 0);
    printf("eh_j\n");
    msg_queue_detach(KU_RIGHT, 0);
    printf("eh_k\n");

    func_cb.f_cb = func_zalloc(sizeof(f_new_heat_t));
    printf("eh_l\n");
    WDT_CLR();
    printf("eh_m\n");

    f = (f_new_heat_t *)func_cb.f_cb;
    printf("eh_n\n");
    f->focus = NEW_HEAT_FOCUS_TEMP;
    printf("eh_o\n");
    f->temp_idx = 0;
    printf("eh_p\n");
    f->time_idx = 0;
    printf("eh_q\n");
    f->display_pending = false;
    printf("eh_r\n");

    /* flag_top=false → 新页面不设为顶层，GPU 看不到它 */
    func_cb.frm_main = func_new_heat_form_create();
    printf("eh_s\n");
    WDT_CLR();
    printf("eh_t\n");
    new_heat_bind_objects(f);
    printf("eh_u\n");
    WDT_CLR();

    /* 预加载共享状态数据到 RAM */
    home_ui_shared_status_init();
    printf("eh_v3\n");

    /* 页面非顶层，读 Flash 设字体/绑定 RAM 图片 → 不会触发 C281/C482 */
    new_heat_status_icons_apply(f);
    printf("eh_y\n");
    WDT_CLR();
    new_heat_ui_refresh(f);
    printf("eh_za\n");
    WDT_CLR();

    /* 所有资源已就绪，现在才激活页面（GPU 第一眼看到的都是有有效数据的 widget）*/
    home_gpu_wait_idle();
    printf("eh_zc\n");
    WDT_CLR();
    compo_pool_set_top(func_cb.frm_main);
    printf("eh_set_top\n");
    home_gpu_wait_idle();
    printf("eh_set_top2\n");

    /* 全部就绪，恢复 TE 渲染 */
    elunchbox_te_block_flag = 0;
    printf("eh_ze te=0\n");
    tft_bglight_force_on();
    printf("eh_zf\n");
}

void func_new_heat_exit(void)
{
    f_new_heat_t *f = (f_new_heat_t *)func_cb.f_cb;

    if (f != NULL) {
        compo_picturebox_t *pics[8];
        u8 n = 0;
        u8 i;

        if (f->pic_bt) pics[n++] = f->pic_bt;
        if (f->pic_bat) pics[n++] = f->pic_bat;
        if (f->pic_temp_track) pics[n++] = f->pic_temp_track;
        if (f->pic_time_track) pics[n++] = f->pic_time_track;
        if (f->pic_temp_point) pics[n++] = f->pic_temp_point;
        if (f->pic_time_point) pics[n++] = f->pic_time_point;
        if (f->pic_temp_badge) pics[n++] = f->pic_temp_badge;
        if (f->pic_time_badge) pics[n++] = f->pic_time_badge;
        for (i = 0; i < n; i++) {
            home_ui_gpu_pic_detach_light(pics[i]);
        }
    }

    home_ui_shared_battery_detach_pic();
    func_cb.last = FUNC_NEW_HEAT;
    printf("func_new_heat_exit\n");
}

void func_new_heat(void)
{
    printf("func_new_heat run\n");
    func_new_heat_enter();
    while (func_cb.sta == FUNC_NEW_HEAT) {
        func_new_heat_process();
        func_new_heat_message(msg_dequeue());
    }
    func_new_heat_exit();
}
