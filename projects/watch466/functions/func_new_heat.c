#include "include.h"
#include "func.h"
#include "new_home_icon_res.h"
#include "new_heat_res.h"
#include "home_top_time_txt.h"
#include "home_ui_shared.h"
#include "home_ui_gpu_detach.h"
#include "home_ui_ram.h"
#include "func_lunchbox_uart.h"
#include "func_key_lock.h"
#include "func_reservation.h"

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

#ifndef UI_BUF_0FONT_FONT_TEST_BIN
#error "UI_BUF_0FONT_FONT_TEST_BIN missing: add font_test.bin to ui.bin then Output/bin/prebuild.bat"
#endif
#define NEW_HEAT_FONT                       UI_BUF_0FONT_FONT_TEST_BIN   /* 全局统一使用 18px 字体（font_test_18.bin 覆盖） */

/*
 * 新加热设置页（320×240 白底）：
 *   顶栏：蓝牙 + 电量 + 模式名称（Chicken / Pasta）
 *   温度条 new_temp_1..5，时长条 new_time_1..13，圆点 new_point
 *   默认 140°F / 1 小时；先调温度，确认后调时长，再确认开始加热
 * PT8028：TCH4 确认 | TCH5 返回 | TCH2/TCH6 减/加
 *
 * 模式页（func_new_mode.c）进入时通过全局变量传递模式名称和默认参数：
 *   g_new_heat_mode_name  — 顶部显示的模式名称（NULL 则显示 "Heating Temp"）
 *   g_new_heat_proto_mode — 启动时使用的协议模式值
 *   g_new_heat_temp_idx    — 默认温度索引
 *   g_new_heat_time_idx    — 默认时长索引
 */
#define NEW_HEAT_STATUS_Y                 20
#define NEW_HEAT_STATUS_RIGHT_MARGIN      10
#define NEW_HEAT_STATUS_GAP               6

#define NEW_HEAT_MODE_TITLE_Y             20
#define NEW_HEAT_MODE_TITLE_H             36
#define NEW_HEAT_MODE_TITLE_W             120

#define NEW_HEAT_TEMP_LABEL_Y             52
#define NEW_HEAT_TEMP_LABEL_TEXT_Y        46
#define NEW_HEAT_TEMP_SLIDER_Y            82
#define NEW_HEAT_TEMP_SCALE_Y             100

#define NEW_HEAT_TIME_LABEL_Y             137
#define NEW_HEAT_TIME_LABEL_TEXT_Y        131
#define NEW_HEAT_TIME_SLIDER_Y            166
#define NEW_HEAT_TIME_SCALE_Y             184

#define NEW_HEAT_BADGE_TXT_W              80

#define NEW_HEAT_BADGE_X                  ((s16)(GUI_SCREEN_WIDTH - NEW_HEAT_STATUS_RIGHT_MARGIN - NEW_HEAT_BADGE_W / 2))
#define NEW_HEAT_BADGE_TXT_X              ((s16)(NEW_HEAT_BADGE_X - (NEW_HEAT_BADGE_TXT_W - NEW_HEAT_BADGE_W) / 2 +12))
#define NEW_HEAT_SLIDER_SLOT_X            ((s16)((GUI_SCREEN_WIDTH - NEW_HEAT_SLIDER_W) / 2))
#define NEW_HEAT_LABEL_X                  ((s16)(NEW_HEAT_SLIDER_SLOT_X - 27))

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
    COMPO_ID_TXT_TOP_TIME,
    COMPO_ID_TXT_MODE_TITLE,
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
    u8 last_top_min;
    u8 last_top_sec;
    bool display_pending;
#if ELUNCHBOX_PANEL_EN
    bool key_ready;
    bool slider_only_pending;   /* 加减键 pending：只刷 track/位置/文字，不重读 badge 和 scales */
#endif
    home_top_time_txt_t top_time;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_temp_track;
    compo_picturebox_t *pic_time_track;
    compo_picturebox_t *pic_temp_point;
    compo_picturebox_t *pic_time_point;
    compo_picturebox_t *pic_temp_badge;
    compo_picturebox_t *pic_time_badge;
    compo_textbox_t *txt_mode_title;
    compo_textbox_t *txt_temp_label;
    compo_textbox_t *txt_time_label;
    compo_textbox_t *txt_temp_val;
    compo_textbox_t *txt_time_val;
    compo_textbox_t *txt_temp_scale[NEW_HEAT_TEMP_CNT];
    compo_textbox_t *txt_time_scale[3];
} f_new_heat_t;

/* 温度值 协议 ID=7: 40/50/60/70/80/90/100°C → 104~212°F */
static const u16 tbl_new_heat_temp_f[NEW_HEAT_TEMP_CNT] = {
     140, 158, 176, 194, 212,
};

/* 时长（分钟）：60min(1H) ~ 120min(2H)，每档 +5min */
static const u16 tbl_new_heat_time_min[NEW_HEAT_TIME_CNT] = {
    60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120,
};

/* 时间刻度尺显示三档：索引 0(1H)、6(1H30min)、12(2H) */
static const u8 tbl_new_heat_time_scale_idx[3] = {
    NEW_HEAT_TIME_IDX_MIN, 6, NEW_HEAT_TIME_IDX_MAX,
};

/* 模式页（func_new_mode.c）进入时通过以下全局变量传递模式名称和默认参数 */
const char *g_new_heat_mode_name = NULL;
u8 g_new_heat_temp_idx = 0;
u8 g_new_heat_time_idx = NEW_HEAT_TIME_IDX_1H;
u8 g_new_heat_proto_mode = 1;

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

/* 温度刻度文字与轨道刻度线对齐：固定满刻度宽，不随当前 temp_idx 变化 */
static s16 new_heat_temp_tick_x(u8 idx)
{
    u8 max_idx = NEW_HEAT_TEMP_CNT - 1;
    u16 track_w = NEW_HEAT_SLIDER_W;
    s16 left = new_heat_track_left(track_w);

    if (max_idx == 0) {
        return (s16)(left + track_w / 2);
    }
    return (s16)(left + (u32)idx * track_w / max_idx);
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

    if (total_min < 60) {
        sprintf(buf, "%umin", total_min);
    } else if (min == 0) {
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
    home_top_time_txt_force(&f->top_time, &f->last_top_min, &f->last_top_sec);
    home_ui_shared_status_init();
    if (f->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        home_ui_shared_status_refresh_bt(f->pic_bt);
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

/* 圆点图已在 RAM：加减键只改位置，勿重复读 Flash */
static void new_heat_point_repos(compo_picturebox_t *pic, u8 idx, u8 max_idx,
                                 u16 track_w, s16 y, bool visible)
{
    if (pic == NULL) {
        return;
    }
    if (!visible) {
        compo_picturebox_set_visible(pic, false);
        return;
    }
    compo_picturebox_set_pos(pic, new_heat_point_x(idx, max_idx, track_w), y);
    compo_picturebox_set_visible(pic, true);
}

static void new_heat_temp_point_repos(compo_picturebox_t *pic, u8 idx, s16 y, bool visible)
{
    if (pic == NULL) {
        return;
    }
    if (!visible) {
        compo_picturebox_set_visible(pic, false);
        return;
    }
    compo_picturebox_set_pos(pic, new_heat_temp_tick_x(idx), y);
    compo_picturebox_set_visible(pic, true);
}

static void new_heat_temp_track_apply(f_new_heat_t *f);
static void new_heat_time_track_apply(f_new_heat_t *f);

static void new_heat_slider_focus_apply(f_new_heat_t *f)
{
    char buf[24];

    if (f == NULL) {
        return;
    }
    elunchbox_te_block_flag = 1;
    if (f->focus == NEW_HEAT_FOCUS_TEMP) {
        new_heat_temp_track_apply(f);
        new_heat_temp_point_repos(f->pic_temp_point, f->temp_idx, NEW_HEAT_TEMP_SLIDER_Y, true);
        if (f->txt_temp_val != NULL) {
            new_heat_format_temp(buf, tbl_new_heat_temp_f[f->temp_idx]);
            compo_textbox_set(f->txt_temp_val, buf);
        }
    } else {
        new_heat_time_track_apply(f);
        new_heat_point_repos(f->pic_time_point, f->time_idx, NEW_HEAT_TIME_CNT - 1,
                             new_heat_time_track_w(f->time_idx), NEW_HEAT_TIME_SLIDER_Y,
                             true);
        if (f->txt_time_val != NULL) {
            new_heat_format_duration(buf, tbl_new_heat_time_min[f->time_idx]);
            compo_textbox_set(f->txt_time_val, buf);
        }
    }
    elunchbox_te_block_flag = 0;
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
    if (f == NULL) {
        return;
    }
    if (f->focus != NEW_HEAT_FOCUS_TEMP) {
        new_heat_temp_point_repos(f->pic_temp_point, f->temp_idx, NEW_HEAT_TEMP_SLIDER_Y, false);
        return;
    }
    new_heat_pic_apply(f->pic_temp_point, UI_BUF_NEW_UI_NEW_POINT_BIN, UI_LEN_NEW_UI_NEW_POINT_BIN,
                       NEW_HEAT_RAM_POINT, NEW_HEAT_RAM_POINT_CAP,
                       NEW_HEAT_POINT_W, NEW_HEAT_POINT_H,
                       new_heat_temp_tick_x(f->temp_idx), NEW_HEAT_TEMP_SLIDER_Y);
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
static bool new_heat_font_ready;

static void new_heat_font_bind_txt(compo_textbox_t *txt)
{
    if (txt != NULL) {
        compo_textbox_set_font(txt, NEW_HEAT_FONT);
    }
}

static void new_heat_font_apply_once(f_new_heat_t *f)
{
    u8 i;

    if (new_heat_font_ready || f == NULL) {
        return;
    }

    WDT_CLR();
    new_heat_font_bind_txt(f->txt_mode_title);
    new_heat_font_bind_txt(f->txt_temp_label);
    new_heat_font_bind_txt(f->txt_time_label);
    new_heat_font_bind_txt(f->txt_temp_val);
    new_heat_font_bind_txt(f->txt_time_val);
    for (i = 0; i < NEW_HEAT_TEMP_CNT; i++) {
        WDT_CLR();
        new_heat_font_bind_txt(f->txt_temp_scale[i]);
    }
    for (i = 0; i < 3; i++) {
        WDT_CLR();
        new_heat_font_bind_txt(f->txt_time_scale[i]);
    }
    new_heat_font_ready = true;

    /* 方式 3：覆盖 "Heating Temp" / "Heating Duration" 标签为小字体 */
    compo_textbox_set_font(f->txt_temp_label, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_font(f->txt_time_label, UI_BUF_0FONT_FONT_TEST_14_BIN);
}

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
    new_heat_font_apply_once(f);
    new_heat_text_apply_main(f);
    new_heat_text_apply_scales(f);
    return;
#endif

    temp_w = new_heat_temp_track_w(f->temp_idx);
    time_w = new_heat_time_track_w(f->time_idx);
    (void)temp_w;

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
            widget_text_set_client(f->txt_temp_scale[i]->txt, 0, 5);
            compo_textbox_set_align_center(f->txt_temp_scale[i], true);
            compo_textbox_set_pos(f->txt_temp_scale[i],
                                  new_heat_temp_tick_x(i),
                                  NEW_HEAT_TEMP_SCALE_Y);
            compo_textbox_set_visible(f->txt_temp_scale[i], true);
        }
    }

    for (i = 0; i < 3; i++) {
        u8 tidx = tbl_new_heat_time_scale_idx[i];

        if (f->txt_time_scale[i] != NULL) {
            new_heat_format_duration(buf, tbl_new_heat_time_min[tidx]);
            compo_textbox_set(f->txt_time_scale[i], buf);
            widget_text_set_client(f->txt_time_scale[i]->txt, 0, 5);
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
            compo_textbox_set_autosize(t, false);
            compo_textbox_set_pos(t, NEW_HEAT_LABEL_X, NEW_HEAT_TEMP_LABEL_TEXT_Y);
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
            compo_textbox_set_autosize(t, false);
            compo_textbox_set_pos(t, NEW_HEAT_LABEL_X, NEW_HEAT_TIME_LABEL_TEXT_Y);
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
            compo_textbox_set_autosize(t, false);
            compo_textbox_set_pos(t, NEW_HEAT_BADGE_TXT_X, NEW_HEAT_TEMP_LABEL_Y);
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
            compo_textbox_set_autosize(t, false);
            compo_textbox_set_pos(t, NEW_HEAT_BADGE_TXT_X, NEW_HEAT_TIME_LABEL_Y);
            compo_textbox_set_forecolor(t, NEW_HEAT_COLOR_OFF_BADGE);
        }
        f->txt_time_val = t;
    }
    /* 显示模式名称（Chicken / Pasta），来自模式页全局变量 */
    if (f->txt_mode_title != NULL && g_new_heat_mode_name != NULL) {
        compo_textbox_set(f->txt_mode_title, g_new_heat_mode_name);
        compo_textbox_set_visible(f->txt_mode_title, true);
    } else if (f->txt_mode_title != NULL) {
        compo_textbox_set_visible(f->txt_mode_title, false);
    }
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
        compo_textbox_set_visible(f->txt_temp_val, true);
    }
    if (f->txt_time_val != NULL) {
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
                compo_textbox_set_autosize(t, false);
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
                compo_textbox_set_autosize(t, false);
                compo_textbox_set_pos(t, NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TIME_SCALE_Y);
                compo_textbox_set_forecolor(t, NEW_HEAT_COLOR_SCALE);
            }
            f->txt_time_scale[i] = t;
        }
    }
    temp_w = new_heat_temp_track_w(f->temp_idx);
    time_w = new_heat_time_track_w(f->time_idx);
    (void)temp_w;
    for (i = 0; i < NEW_HEAT_TEMP_CNT; i++) {
        if (f->txt_temp_scale[i] != NULL) {
            new_heat_format_temp(buf, tbl_new_heat_temp_f[i]);
            compo_textbox_set(f->txt_temp_scale[i], buf);
            widget_text_set_client(f->txt_temp_scale[i]->txt, 0, 5);
            compo_textbox_set_align_center(f->txt_temp_scale[i], true);
            compo_textbox_set_pos(f->txt_temp_scale[i],
                                  new_heat_temp_tick_x(i),
                                  NEW_HEAT_TEMP_SCALE_Y);
            compo_textbox_set_visible(f->txt_temp_scale[i], true);
        }
    }

    for (i = 0; i < 3; i++) {
        u8 tidx = tbl_new_heat_time_scale_idx[i];

        if (f->txt_time_scale[i] != NULL) {
            new_heat_format_duration(buf, tbl_new_heat_time_min[tidx]);
            compo_textbox_set(f->txt_time_scale[i], buf);
            widget_text_set_client(f->txt_time_scale[i]->txt, 0, 5);
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

    if (f == NULL) {
        return;
    }
    home_top_time_txt_bind(&f->top_time, COMPO_ID_TXT_TOP_TIME);
    f->pic_bt = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_BT);
    f->pic_bat = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_BAT);
    f->txt_mode_title = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_MODE_TITLE);
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
#if ELUNCHBOX_PANEL_EN
        /* 鸡腿/意面模式温度固定，不可调节 */
        if (g_new_heat_proto_mode != 1) {
            return;
        }
#endif
        if (f->temp_idx + 1 < NEW_HEAT_TEMP_CNT) {
            f->temp_idx++;
#if ELUNCHBOX_PANEL_EN
            f->display_pending = true;
            f->slider_only_pending = true;
#else
            new_heat_ui_refresh(f);
#endif
        }
    } else if (f->time_idx + 1 < NEW_HEAT_TIME_CNT) {
        f->time_idx++;
#if ELUNCHBOX_PANEL_EN
        f->display_pending = true;
        f->slider_only_pending = true;
#else
        new_heat_ui_refresh(f);
#endif
    }
}

static void new_heat_value_dec(f_new_heat_t *f)
{
    if (f == NULL) {
        return;
    }
    if (f->focus == NEW_HEAT_FOCUS_TEMP) {
#if ELUNCHBOX_PANEL_EN
        /* 鸡腿/意面模式温度固定，不可调节 */
        if (g_new_heat_proto_mode != 1) {
            return;
        }
#endif
        if (f->temp_idx > 0) {
            f->temp_idx--;
#if ELUNCHBOX_PANEL_EN
            f->display_pending = true;
            f->slider_only_pending = true;
#else
            new_heat_ui_refresh(f);
#endif
        }
    } else if (f->time_idx > 0) {
        f->time_idx--;
#if ELUNCHBOX_PANEL_EN
        f->display_pending = true;
        f->slider_only_pending = true;
#else
        new_heat_ui_refresh(f);
#endif
    }
}

/* 确认键：先确认温度（切换到时长设置），再确认时长（开始加热） */
static void new_heat_ok_key(f_new_heat_t *f)
{
    u16 temp_f;
    u16 total_min;

    if (f == NULL) {
        return;
    }
    if (f->focus == NEW_HEAT_FOCUS_TEMP) {
        /* 温度已确认，切换到时长设置 */
        f->focus = NEW_HEAT_FOCUS_TIME;
#if ELUNCHBOX_PANEL_EN
        f->display_pending = true;
        f->slider_only_pending = false;   /* 焦点切换需完整刷新 */
#else
        new_heat_ui_refresh(f);
#endif
        return;
    }

    /* 时长已确认，启动加热后切换到 func_heat 显示加热面板。
     * 不封锁 TE 做 form_create（避免 tmr thread miss 和 GPU 硬件状态问题），
     * 仅封锁 func_heat_panel_enter 确保资源绑定期间不被 TE 打断。 */
    temp_f = tbl_new_heat_temp_f[f->temp_idx];
    total_min = tbl_new_heat_time_min[f->time_idx];
    if (total_min < LB_HEAT_DURATION_MIN_MIN) {
        total_min = LB_HEAT_DURATION_MIN_MIN;
    } else if (total_min > LB_HEAT_DURATION_MAX_MIN) {
        total_min = LB_HEAT_DURATION_MAX_MIN;
    }

#if ELUNCHBOX_PANEL_EN
    if (g_res_heat_pending) {
        /* 预约模式：保存加热参数，提交预约串口指令 */
        g_res.heat_hour = (u8)(total_min / 60);
        g_res.heat_min = (u8)(total_min % 60);
        g_res.temp_idx = lunchbox_temp_f_to_idx(temp_f);
        /* pt8028_release_clear 会清零 key_notify_pending，须先发送 lunchbox 按键通知 */
        {
            u8 lunchbox_key = pt8028_tch_to_lunchbox_key(PT8028_KEY_TCH4);
            if (lunchbox_key != 0) {
                lunchbox_key_notify(lunchbox_key);
            }
        }
        /* 预约时间直接使用用户设定的时间，不再减去加热时长 */
        func_reservation_new_ui_do_submit();
        return;
    }
#endif

    printf("new_heat_ok: set preset + autostart, direct sta\n");
    lb_mode_to_heat_set(g_new_heat_proto_mode, temp_f,
                        (u8)(total_min / 60), (u8)(total_min % 60));
#if ELUNCHBOX_PANEL_EN
    lb_heat_user_uart_tx_force_set(true);
    lb_heat_mcu_nav_set(false);
    func_elunchbox_warm_from_charging_set(false);
#endif
    lb_heat_autostart_set(true);
    g_new_heat_mode_name = NULL;
    g_new_heat_temp_idx = 0;
    g_new_heat_time_idx = NEW_HEAT_TIME_IDX_1H;
    g_new_heat_proto_mode = 1;
    func_cb.sta = FUNC_HEAT;
}

/* 返回/电源键：时长设置中返回温度设置，温度设置中返回主页 */
static void new_heat_power_key(f_new_heat_t *f)
{
    if (f == NULL) {
        return;
    }
    if (f->focus == NEW_HEAT_FOCUS_TIME) {
#if ELUNCHBOX_PANEL_EN
        /* 鸡腿/意面模式温度固定，从时间返回直接回到主页 */
        if (g_new_heat_proto_mode != 1) {
            goto do_home;
        }
#endif
        f->focus = NEW_HEAT_FOCUS_TEMP;
#if ELUNCHBOX_PANEL_EN
        f->display_pending = true;
        f->slider_only_pending = false;   /* 焦点切换需完整刷新 */
#else
        new_heat_ui_refresh(f);
#endif
        return;
    }
do_home:
    g_new_heat_mode_name = NULL;
    g_new_heat_temp_idx = 0;
    g_new_heat_time_idx = NEW_HEAT_TIME_IDX_1H;
    g_new_heat_proto_mode = 1;
    g_res_heat_pending = false;  /* 退出预约温度设置，清除预约 pending 标记 */
    /* 不调 func_switch_to：让 func_exit() 安全路径做清理 */
    func_cb.sta = FUNC_HOME;
}

/* 模式键：跳转到模式选择页 */
static void new_heat_mode_key(void)
{
    if (sys_cb.flag_swithing) {
        return;
    }
    func_switch_to(FUNC_NEW_MODE, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

/* 定期刷新状态栏（蓝牙、电量电池图标按原始逻辑显示） */
#if !ELUNCHBOX_PANEL_EN
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
#endif

static compo_textbox_t *new_heat_txt_create(compo_form_t *frm, u16 id, u32 font_addr,
                                            s16 x, s16 y, u16 color, bool center)
{
    compo_textbox_t *txt = compo_textbox_create(frm, 24);

    compo_setid(txt, id);
    compo_textbox_set_wholewrap(txt, false);
#if ELUNCHBOX_PANEL_EN
    /* 禁用 autosize → 后续 compo_textbox_set() 不读字体 Flash，避免 C281 */
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_align_center(txt, center);
    compo_textbox_set_visible(txt, false);
#else
    compo_textbox_set_font(txt, font_addr ? font_addr : NEW_HEAT_FONT);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, center);
#endif
    compo_textbox_set_pos(txt, x, y);
    compo_textbox_set_forecolor(txt, color);
    return txt;
}

compo_form_t *func_new_heat_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    compo_picturebox_t *pic;
    s16 bat_x;
    s16 bt_x;

    new_heat_white_bg_create(frm);

    home_top_time_txt_create(frm, COMPO_ID_TXT_TOP_TIME);

    bat_x = (s16)(GUI_SCREEN_WIDTH - NEW_HEAT_STATUS_RIGHT_MARGIN - NEW_HOME_BAT_W / 2);
    bt_x = (s16)(bat_x - NEW_HOME_BAT_W / 2 - NEW_HEAT_STATUS_GAP - NEW_HOME_BT_W / 2);

    pic = new_heat_pic_create_hidden(frm, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, bt_x, NEW_HEAT_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);

    pic = new_heat_pic_create_hidden(frm, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, bat_x, NEW_HEAT_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);

    /* 模式名称标题：显示 Chicken / Pasta（从模式页进入时），默认隐藏 */
    txt = new_heat_txt_create(frm, COMPO_ID_TXT_MODE_TITLE, NEW_HEAT_FONT,
                              GUI_SCREEN_CENTER_X, NEW_HEAT_MODE_TITLE_Y,
                              COLOR_BLACK, true);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, NEW_HEAT_MODE_TITLE_Y,
                               NEW_HEAT_MODE_TITLE_W, NEW_HEAT_MODE_TITLE_H);

    txt = new_heat_txt_create(frm, COMPO_ID_TXT_TEMP_LABEL, NEW_HEAT_FONT,
                              NEW_HEAT_LABEL_X, NEW_HEAT_TEMP_LABEL_TEXT_Y, NEW_HEAT_COLOR_LABEL, false);
    compo_textbox_set_location(txt, NEW_HEAT_LABEL_X, NEW_HEAT_TEMP_LABEL_TEXT_Y, 300, 30);
    txt = new_heat_txt_create(frm, COMPO_ID_TXT_TIME_LABEL, NEW_HEAT_FONT,
                              NEW_HEAT_LABEL_X, NEW_HEAT_TIME_LABEL_TEXT_Y, NEW_HEAT_COLOR_LABEL, false);
    compo_textbox_set_location(txt, NEW_HEAT_LABEL_X, NEW_HEAT_TIME_LABEL_TEXT_Y, 300, 30);

    pic = new_heat_pic_create_hidden(frm, COMPO_ID_PIC_TEMP_BADGE);
    compo_picturebox_set_pos(pic, NEW_HEAT_BADGE_X, NEW_HEAT_TEMP_LABEL_Y);
    compo_picturebox_set_size(pic, NEW_HEAT_BADGE_W, NEW_HEAT_BADGE_H);

    pic = new_heat_pic_create_hidden(frm, COMPO_ID_PIC_TIME_BADGE);
    compo_picturebox_set_pos(pic, NEW_HEAT_BADGE_X, NEW_HEAT_TIME_LABEL_Y);
    compo_picturebox_set_size(pic, NEW_HEAT_BADGE_W, NEW_HEAT_BADGE_H);

    txt = new_heat_txt_create(frm, COMPO_ID_TXT_TEMP_VAL, NEW_HEAT_FONT,
                              NEW_HEAT_BADGE_TXT_X, NEW_HEAT_TEMP_LABEL_Y, NEW_HEAT_COLOR_ON_BADGE, true);
    compo_textbox_set_location(txt, NEW_HEAT_BADGE_TXT_X, NEW_HEAT_TEMP_LABEL_Y, NEW_HEAT_BADGE_TXT_W, NEW_HEAT_BADGE_H);
    txt = new_heat_txt_create(frm, COMPO_ID_TXT_TIME_VAL, NEW_HEAT_FONT,
                              NEW_HEAT_BADGE_TXT_X, NEW_HEAT_TIME_LABEL_Y, NEW_HEAT_COLOR_OFF_BADGE, true);
    compo_textbox_set_location(txt, NEW_HEAT_BADGE_TXT_X, NEW_HEAT_TIME_LABEL_Y, NEW_HEAT_BADGE_TXT_W, NEW_HEAT_BADGE_H);

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
        txt = new_heat_txt_create(frm, COMPO_ID_TXT_TEMP_SCALE0 + i, NEW_HEAT_FONT,
                            NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TEMP_SCALE_Y,
                            NEW_HEAT_COLOR_SCALE, true);
        compo_textbox_set_location(txt, NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TEMP_SCALE_Y, 85, 40);
    }
    for (u8 j = 0; j < 3; j++) {
        txt = new_heat_txt_create(frm, COMPO_ID_TXT_TIME_SCALE0 + j, NEW_HEAT_FONT,
                            NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TIME_SCALE_Y,
                            NEW_HEAT_COLOR_SCALE, true);
        compo_textbox_set_location(txt, NEW_HEAT_SLIDER_SLOT_X, NEW_HEAT_TIME_SCALE_Y, 140, 40);
    }

    return frm;
}

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static void new_heat_pt8028_keys_process(f_new_heat_t *f)
{
    u8 press_tch;

    if (f == NULL || !f->key_ready) {
        return;
    }
    if (func_key_lock_press_take_guarded(&press_tch)) {
        return;
    }
    if (press_tch == 0xff) {
        return;
    }
    if (press_tch <= PT8028_KEY_TCH6 && press_tch != PT8028_KEY_TCH4) {
        elunchbox_user_activity_reset();
    }
    if (press_tch == PT8028_KEY_TCH4) {
        new_heat_ok_key(f);
    } else if (press_tch == PT8028_KEY_TCH5) {
        new_heat_power_key(f);
    } else if (press_tch == PT8028_KEY_TCH2) {
        new_heat_value_inc(f);
    } else if (press_tch == PT8028_KEY_TCH6) {
        new_heat_value_dec(f);
    } else if (press_tch == PT8028_KEY_TCH3) {
        new_heat_mode_key();
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

static void new_heat_keys_poll(f_new_heat_t *f)
{
    if (f == NULL || !f->key_ready) {
        return;
    }
    pt8028_key_scan_page();
    new_heat_pt8028_keys_process(f);
}
#endif

static void func_new_heat_message(size_msg_t msg)
{
    f_new_heat_t *f = (f_new_heat_t *)func_cb.f_cb;

    if (msg == NO_MSG || func_cb.sta != FUNC_NEW_HEAT) {
        return;
    }
    if (sys_cb.flag_swithing) {    //正在切换动画中，不处理
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (f != NULL && !f->key_ready) {  //UI还没准备好，不处理
        return;
    }
#endif
    if (func_key_lock_ku_blocked(msg)) {  //摁键被锁，不处理
        return;
    }
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    /* PT8028 走 new_heat_pt8028_keys_process，勿重复处理消息队列 */
    switch (msg) {
    case KU_BACK:
    case KU_VOL_UP:
    case KU_VOL_DOWN:
    case KU_MODE:
    case KEY_RIGHT | KEY_SHORT_UP:
        return;
    default:
        break;
    }
#endif
    switch (msg) {
    case KU_BACK:       /* NEW_HEAT_MSG_OK = 确认键 */
        new_heat_ok_key(f);
        break;
    case KU_VOL_UP:     /* NEW_HEAT_MSG_PLUS → 减键 */
        new_heat_value_dec(f);
        break;
    case KU_VOL_DOWN:   /* NEW_HEAT_MSG_MINUS → 加键 */
        new_heat_value_inc(f);
        break;
    case KEY_RIGHT | KEY_SHORT_UP:  /* NEW_HEAT_MSG_POWER = 电源/返回键 */
        new_heat_power_key(f);
        break;
    case KU_MODE:       /* 模式键：跳转到模式页 */
        new_heat_mode_key();
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

#if ELUNCHBOX_PANEL_EN
    if (!f->key_ready) {
        if (f->display_pending) {
            /* 首次初始显示：大量 SPI flash 读取，需 te_block=1 保护 */
            home_gpu_wait_idle();
            WDT_CLR();
            elunchbox_te_block_flag = 1;
            new_heat_status_icons_apply(f);
            new_heat_tracks_apply(f);
            new_heat_badges_apply(f);
            new_heat_text_apply(f);
            elunchbox_te_block_flag = 0;
            f->display_pending = false;
            gui_widget_refresh();  /* 非阻塞请求重绘 */
        }
        func_process();
        func_home_drain_stale_key_msgs();
        pt8028_release_clear();
        (void)pt8028_take_press_tch();
        f->key_ready = true;
        return;
    }

    /* 先扫键再刷新：避免 SPI/GPU 阻塞期间按键被积压；同帧尽量合并连按 */
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    if (elunchbox_ui_is_live()) {
        u8 k;

        for (k = 0; k < 4; k++) {
            u8 t_before = f->temp_idx;
            u8 tm_before = f->time_idx;

            new_heat_keys_poll(f);
            if (f->temp_idx == t_before && f->time_idx == tm_before) {
                break;
            }
        }
    }
#endif

    if (f->display_pending) {
        bool slider_refreshed = false;

        WDT_CLR();

        if (f->slider_only_pending) {
            /* 加减键：只刷新当前焦点滑条，勿 wait_idle（易 gui thread miss） */
            new_heat_slider_focus_apply(f);
            slider_refreshed = true;
        } else {
            /* 焦点切换(确认/返回键)：完整 UI 刷新 */
            home_gpu_wait_idle();
            elunchbox_te_block_flag = 1;
            new_heat_ui_refresh(f);
            elunchbox_te_block_flag = 0;
        }

        f->display_pending = false;
        f->slider_only_pending = false;
        WDT_CLR();
        gui_widget_refresh();

        home_top_time_txt_tick(&f->top_time, &f->last_top_min, &f->last_top_sec);
        /* 加减帧跳过 BT 图标 Flash 重读，减轻卡顿 */
        if (!slider_refreshed) {
            u8 was_blocked = elunchbox_te_block_flag;

            if (!was_blocked) {
                elunchbox_te_block_flag = 1;
            }
            home_ui_shared_status_refresh_bt(f->pic_bt);
            if (!was_blocked) {
                elunchbox_te_block_flag = 0;
            }
        }
    } else {
        home_top_time_txt_tick(&f->top_time, &f->last_top_min, &f->last_top_sec);
        {
            u8 was_blocked = elunchbox_te_block_flag;

            if (!was_blocked) {
                elunchbox_te_block_flag = 1;
            }
            home_ui_shared_status_refresh_bt(f->pic_bt);
            if (!was_blocked) {
                elunchbox_te_block_flag = 0;
            }
        }
    }
#endif
    func_process();
#if ELUNCHBOX_PANEL_EN
    if (!elunchbox_ui_is_live()) {
        return;
    }
#endif
#if !ELUNCHBOX_PANEL_EN
    new_heat_text_apply(f);
    new_heat_status_refresh(f);
#endif
}

void func_new_heat_enter(void)
{
    f_new_heat_t *f;

    printf("func_new_heat_enter\n");

#if ELUNCHBOX_PANEL_EN
    lb_heat_autostart_set(false);
#endif

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(1);
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif

    msg_queue_detach(KU_BACK, 0);
    msg_queue_detach(KU_VOL_UP, 0);
    msg_queue_detach(KU_VOL_DOWN, 0);
    msg_queue_detach(KU_RIGHT, 0);

    func_cb.f_cb = func_zalloc(sizeof(f_new_heat_t));
    WDT_CLR();

    f = (f_new_heat_t *)func_cb.f_cb;
    f->focus = (g_new_heat_proto_mode != 1) ? NEW_HEAT_FOCUS_TIME : NEW_HEAT_FOCUS_TEMP;
    f->temp_idx = g_new_heat_temp_idx;
    f->time_idx = g_new_heat_time_idx;
    f->last_top_min = 0xff;
    f->last_top_sec = 0xff;
#if ELUNCHBOX_PANEL_EN
    new_heat_font_ready = false;
    f->key_ready = false;
    f->display_pending = true;
    home_ui_digit_pool_reset();
#endif

    func_cb.frm_main = func_new_heat_form_create();
    new_heat_bind_objects(f);

#if ELUNCHBOX_PANEL_EN
    /* 预热锁定计时在加热实际启动时由 func_lunchbox_lcd.c 触发，
     * 此处仅进入设置页，不启动 30s 自动锁键。 */
    home_ui_shared_status_init();
    home_gpu_wait_idle();
    WDT_CLR();
    elunchbox_te_block_flag = 0;
    tft_bglight_force_on();
    printf("func_new_heat_enter: ok te_block=0 pending=1\n");
#else
    home_ui_shared_status_init();
    new_heat_status_icons_apply(f);
    new_heat_tracks_apply(f);
    new_heat_badges_apply(f);
    new_heat_text_apply(f);
    f->display_pending = false;
#endif
}

void func_new_heat_exit(void)
{
    g_new_heat_mode_name = NULL;
    g_new_heat_temp_idx = 0;
    g_new_heat_time_idx = NEW_HEAT_TIME_IDX_1H;
    g_new_heat_proto_mode = 1;
    /* GPU 资源由 func_exit() 的 compo_form_destroy() + compos_init() 统一清理。
     * 此处不做 GPU detach，避免：
     *   - 切 HOME 路径：func_switch_to 已 destroy form，frm_main=NULL，detach 是 use-after-free
     *   - 切 HEAT 路径：func_exit 后续做完整 destroy，此处 detach 多余且触发 C245
     * 仅 detach battery pic 全局引用，避免 dirty 指针遗留。 */
    home_ui_shared_battery_detach_pic();
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(0);
    pt8028_release_clear();
#endif
    func_cb.last = FUNC_NEW_HEAT;
    printf("func_new_heat_exit\n");
}

void func_new_heat(void)
{
    printf("func_new_heat run\n");
    func_new_heat_enter();                        //init
    while (func_cb.sta == FUNC_NEW_HEAT) {
        func_new_heat_process();                  //刷新UI
        func_new_heat_message(msg_dequeue());     //处理摁键消息
    }
    func_new_heat_exit();
}
