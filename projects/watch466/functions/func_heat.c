#include "include.h"
#include "func.h"
#include "func_lunchbox_uart.h"
#include "func_reservation.h"
#include "home_icon_res.h"
#include "home_ui_ram.h"
#include "home_ui_shared.h"
#include "home_top_time.h"
#include "heat_display_reg.h"
#include "ui_layout_anchor.h"
#if ELUNCHBOX_PANEL_EN
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#include "func_key_lock.h"
#include "func_heat_panel.h"
#include "func_lunchbox_lcd.h"

extern volatile u8 elunchbox_te_block_flag;
#endif
#if USER_PANEL_LED
#include "port_panel_led.h"
#endif

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * Heat 页 UI（320×240 横屏设计图）：
 *   顶栏 Y≈20：左上 RTC，右上 BT/锁/电量
 *   中部 Y≈60：大号倒计时 HH:MM（右上角锚点定位）
 *   下部 Y≈158：温度 XXX°F（右上角锚点定位）
 *   图标 bin 保持 home_icon_res.h 原始尺寸，不缩放
 * PT8028：TCH4 确认 | TCH5 开关/返回 | TCH2/TCH6 减/加 | TCH0 锁键解锁童锁
 */
#define UI_HEAT_PLACEHOLDER               UI_BUF_ICON_ACTIVITY_BIN

#ifndef UI_BUF_HOME_W0X_BIN
#error "Missing w0x.bin: add ui/home/w0x.png..w9x.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_B0X_BIN
#error "Missing b0x.bin: add ui/home/b0x.png..b9x.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_WBX_BIN
#error "Missing wbx.bin: add ui/home/wbx.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BHX_BIN
#error "Missing bhx.bin: add ui/home/bhx.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_WHX_BIN
#define UI_BUF_HOME_WHX_BIN               UI_BUF_HOME_BHX_BIN
#define UI_LEN_HOME_WHX_BIN               UI_LEN_HOME_BHX_BIN
#endif
#ifndef UI_BUF_HOME_WSX_BIN
#define UI_BUF_HOME_WSX_BIN               UI_BUF_HOME_BHX_BIN
#define UI_LEN_HOME_WSX_BIN               UI_LEN_HOME_BHX_BIN
#endif

#ifndef HEAT_WHX_W
#define HEAT_WHX_W                        HEAT_BHX_W
#define HEAT_WHX_H                        HEAT_BHX_H
#define HEAT_WHX_RAM_SIZE                 HEAT_BHX_RAM_SIZE
#endif
#ifndef HEAT_WSX_W
#define HEAT_WSX_W                        0
#define HEAT_WSX_H                        0
#define HEAT_WSX_RAM_SIZE                 0
#endif

#if HEAT_WHX_RAM_SIZE > HEAT_BHX_RAM_SIZE
#define HEAT_TEMP_DEG_RAM_MAX_SIZE        HEAT_WHX_RAM_SIZE
#else
#define HEAT_TEMP_DEG_RAM_MAX_SIZE        HEAT_BHX_RAM_SIZE
#endif

#if HEAT_WSX_RAM_SIZE > HEAT_BHX_RAM_SIZE
#define HEAT_TEMP_SUF_RAM_MAX_SIZE        HEAT_WSX_RAM_SIZE
#else
#define HEAT_TEMP_SUF_RAM_MAX_SIZE        HEAT_BHX_RAM_SIZE
#endif

#ifndef UI_BUF_HOME_BLUETOOTH_BIN
#error "Missing bluetooth.bin: add ui/home/bluetooth.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_LOCK_BIN
#error "Missing lock.bin: add ui/home/lock.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_DL4_BIN
#error "Missing dl4.bin: add res/home/dl1~dl4.png, dl.png and run gen_home_icons.py + prebuild.bat"
#endif

/* 466×466 参考布局；320×240 横屏按设计图固定坐标，图标 bin 保持原始尺寸 */
#define HEAT_REF_W                        466
#define HEAT_REF_H                        466
#define HEAT_SX(v)                        ((s16)((s32)(v) * GUI_SCREEN_WIDTH / HEAT_REF_W))
#define HEAT_SY(v)                        ((s16)((s32)(v) * GUI_SCREEN_HEIGHT / HEAT_REF_H))

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define HEAT_STATUS_Y                     20
#define HEAT_STATUS_RIGHT_MARGIN          10
#define HEAT_STATUS_GAP                   6
#else
#define HEAT_STATUS_Y                     HEAT_SY(48)
#define HEAT_STATUS_RIGHT_MARGIN          HEAT_SX(24)
#define HEAT_STATUS_GAP                   HEAT_SX(10)
#endif

#define HEAT_STATUS_BAT_X                 (GUI_SCREEN_WIDTH - HEAT_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define HEAT_STATUS_LOCK_X                (HEAT_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - HEAT_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define HEAT_STATUS_BT_X                  (HEAT_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - HEAT_STATUS_GAP - HOME_STATUS_BT_W / 2)

#define HEAT_LOCK_MS                      30000
#define HEAT_TEMP_PRESET_CNT              7
#define HEAT_MIN_STEP                     5
#define HEAT_DEFAULT_TEMP_IDX             5       /* 194°F (90°C) */
#define HEAT_MSG_OK                       KU_BACK
#define HEAT_MSG_PLUS                     KU_VOL_UP
#define HEAT_MSG_MINUS                    KU_VOL_DOWN
#define HEAT_MSG_POWER                    (KEY_RIGHT | KEY_SHORT_UP)

enum {
    HEAT_FOCUS_HOUR = 0,
    HEAT_FOCUS_MIN,
    HEAT_FOCUS_TEMP,
};

enum {
    HEAT_UI_SETUP = 0,
    HEAT_UI_HEATING,
    HEAT_UI_FINISHED,
};

enum {
    HEAT_TIMER_IDX_H10 = 0,
    HEAT_TIMER_IDX_H1,
    HEAT_TIMER_IDX_M10,
    HEAT_TIMER_IDX_M1,
    HEAT_TIMER_IDX_CNT,
};

enum {
    HEAT_TEMP_IDX_H = 0,
    HEAT_TEMP_IDX_T10,
    HEAT_TEMP_IDX_T1,
    HEAT_TEMP_IDX_CNT,
};

enum {
    COMPO_ID_PIC_TOP_TIME_H10 = 1,
    COMPO_ID_PIC_TOP_TIME_H1,
    COMPO_ID_PIC_TOP_TIME_COLON,
    COMPO_ID_PIC_TOP_TIME_M10,
    COMPO_ID_PIC_TOP_TIME_M1,
    COMPO_ID_PIC_TOP_TIME_AMPM,
    COMPO_ID_PIC_TIMER_H10,
    COMPO_ID_PIC_TIMER_H1,
    COMPO_ID_PIC_TIMER_COLON,
    COMPO_ID_PIC_TIMER_M10,
    COMPO_ID_PIC_TIMER_M1,
    COMPO_ID_PIC_TEMP_H,
    COMPO_ID_PIC_TEMP_T10,
    COMPO_ID_PIC_TEMP_T1,
    COMPO_ID_PIC_TEMPF,
    COMPO_ID_PIC_TEMP_S,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_LOCK,
    COMPO_ID_PIC_BAT,
};

typedef struct f_heat_t_ {
    u8 focus;
    u8 ui_state;
    u8 set_hour;
    u8 set_min;
    u8 temp_idx;
    u8 proto_mode;              // 协议加热模式 (1=自定义, 2=鸡腿, 3=意面, 5=保温)
    u16 display_temp_f;
    u32 heat_live_remain_min;   /* 加热中：回调推送的剩余分钟 */
    u16 heat_live_temp_f;       /* 加热中：回调推送的实时温度 °F */
    bool heat_live_ready;       /* 是否已收到至少一次回调 */
    u32 heat_total_sec;
    u32 heat_start_tick;
    bool screen_locked;
    u8 last_top_min;
    u8 last_top_sec;
    u16 last_timer_key;
    u16 last_temp_f;
    bool last_h_white;
    bool last_m_white;
    bool last_t_white;
#if ELUNCHBOX_PANEL_EN
    bool key_ready;
#endif
    home_top_time_ui_t top_time;
    compo_picturebox_t *pic_timer[HEAT_TIMER_IDX_CNT];
    compo_picturebox_t *pic_timer_colon;
    compo_picturebox_t *pic_temp[HEAT_TEMP_IDX_CNT];
    compo_picturebox_t *pic_temp_degf;
    compo_picturebox_t *pic_temp_suffix;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
} f_heat_t;

static const u16 tbl_heat_temp_preset[HEAT_TEMP_PRESET_CNT] = {
    104, 122, 140, 158, 176, 194, 212,
};

static u8 heat_temp_digit_ram[HEAT_TEMP_IDX_CNT][HEAT_B_DIGIT_RAM_MAX_SIZE];
static u8 heat_temp_degf_ram[HEAT_TEMP_DEG_RAM_MAX_SIZE];
#if !ELUNCHBOX_PANEL_EN
static u8 heat_temp_suffix_ram[HEAT_TEMP_SUF_RAM_MAX_SIZE];
#endif

static u32 heat_countdown_remain_sec;
static bool heat_countdown_running;

static const u32 tbl_heat_w_digit_addr[10] = {
    UI_BUF_HOME_W0X_BIN, UI_BUF_HOME_W1X_BIN, UI_BUF_HOME_W2X_BIN, UI_BUF_HOME_W3X_BIN,
    UI_BUF_HOME_W4X_BIN, UI_BUF_HOME_W5X_BIN, UI_BUF_HOME_W6X_BIN, UI_BUF_HOME_W7X_BIN,
    UI_BUF_HOME_W8X_BIN, UI_BUF_HOME_W9X_BIN,
};

static const u32 tbl_heat_b_digit_addr[10] = {
    UI_BUF_HOME_B0X_BIN, UI_BUF_HOME_B1X_BIN, UI_BUF_HOME_B2X_BIN, UI_BUF_HOME_B3X_BIN,
    UI_BUF_HOME_B4X_BIN, UI_BUF_HOME_B5X_BIN, UI_BUF_HOME_B6X_BIN, UI_BUF_HOME_B7X_BIN,
    UI_BUF_HOME_B8X_BIN, UI_BUF_HOME_B9X_BIN,
};

static const u16 tbl_heat_w_digit_len[10] = {
    UI_LEN_HOME_W0X_BIN, UI_LEN_HOME_W1X_BIN, UI_LEN_HOME_W2X_BIN, UI_LEN_HOME_W3X_BIN,
    UI_LEN_HOME_W4X_BIN, UI_LEN_HOME_W5X_BIN, UI_LEN_HOME_W6X_BIN, UI_LEN_HOME_W7X_BIN,
    UI_LEN_HOME_W8X_BIN, UI_LEN_HOME_W9X_BIN,
};

static const u16 tbl_heat_b_digit_len[10] = {
    UI_LEN_HOME_B0X_BIN, UI_LEN_HOME_B1X_BIN, UI_LEN_HOME_B2X_BIN, UI_LEN_HOME_B3X_BIN,
    UI_LEN_HOME_B4X_BIN, UI_LEN_HOME_B5X_BIN, UI_LEN_HOME_B6X_BIN, UI_LEN_HOME_B7X_BIN,
    UI_LEN_HOME_B8X_BIN, UI_LEN_HOME_B9X_BIN,
};

static const u16 tbl_heat_w_digit_w[10] = {
    HEAT_W0X_W, HEAT_W1X_W, HEAT_W2X_W, HEAT_W3X_W, HEAT_W4X_W,
    HEAT_W5X_W, HEAT_W6X_W, HEAT_W7X_W, HEAT_W8X_W, HEAT_W9X_W,
};

static const u16 tbl_heat_b_digit_w[10] = {
    HEAT_B0X_W, HEAT_B1X_W, HEAT_B2X_W, HEAT_B3X_W, HEAT_B4X_W,
    HEAT_B5X_W, HEAT_B6X_W, HEAT_B7X_W, HEAT_B8X_W, HEAT_B9X_W,
};

#if !ELUNCHBOX_PANEL_EN
static const u16 tbl_heat_timer_id[HEAT_TIMER_IDX_CNT] = {
    COMPO_ID_PIC_TIMER_H10, COMPO_ID_PIC_TIMER_H1,
    COMPO_ID_PIC_TIMER_M10, COMPO_ID_PIC_TIMER_M1,
};

static const u16 tbl_heat_temp_id[HEAT_TEMP_IDX_CNT] = {
    COMPO_ID_PIC_TEMP_H, COMPO_ID_PIC_TEMP_T10, COMPO_ID_PIC_TEMP_T1,
};
#endif

static void func_heat_display_refresh(f_heat_t *f_heat);
static void func_heat_heating_finish_check(f_heat_t *f_heat);
#if ELUNCHBOX_PANEL_EN
static void func_heat_sync_mcu_snapshot(f_heat_t *f_heat);
#endif
void func_heat_countdown_set(u8 hour, u8 min);
void func_heat_countdown_stop(void);

#if USER_PANEL_LED
static void func_heat_led_sync(bool on)
{
    panel_led_set_heat_latched(on);
}
#else
static void func_heat_led_sync(bool on)
{
    (void)on;
}
#endif

static u16 func_heat_setup_total_min(const f_heat_t *f_heat)
{
    return (u16)f_heat->set_hour * 60 + f_heat->set_min;
}

static void func_heat_setup_apply_total_min(f_heat_t *f_heat, u16 total_min)
{
    if (total_min < LB_HEAT_DURATION_MIN_MIN) {
        total_min = LB_HEAT_DURATION_MIN_MIN;
    }
    if (total_min > LB_HEAT_DURATION_MAX_MIN) {
        total_min = LB_HEAT_DURATION_MAX_MIN;
    }
    f_heat->set_hour = (u8)(total_min / 60);
    f_heat->set_min = (u8)(total_min % 60);
}

static void func_heat_reset_setup(f_heat_t *f_heat)
{
    f_heat->ui_state = HEAT_UI_SETUP;
    f_heat->focus = HEAT_FOCUS_TEMP;
    f_heat->set_hour = 1;
    f_heat->set_min = 0;
    f_heat->temp_idx = HEAT_DEFAULT_TEMP_IDX;
    f_heat->display_temp_f = 0;
    f_heat->heat_live_remain_min = 0;
    f_heat->heat_live_temp_f = 0;
    f_heat->heat_live_ready = false;
    f_heat->heat_total_sec = 0;
    f_heat->heat_start_tick = 0;
    f_heat->screen_locked = false;
    f_heat->last_timer_key = 0xffff;
    f_heat->last_temp_f = 0xffff;
    func_heat_countdown_stop();
    func_heat_countdown_set(f_heat->set_hour, f_heat->set_min);
}

#if !ELUNCHBOX_PANEL_EN
static void func_heat_display_on_info(const heat_display_info_t *info)
{
    printf("func_heat_display_on_info\n");
    printf("info->remain_min: %d, info->temp_f: %d\n", info->remain_min, info->temp_f);
    f_heat_t *f_heat = (f_heat_t *)func_cb.f_cb;

    if (info == NULL || f_heat == NULL || func_cb.sta != FUNC_HEAT) {
        return;
    }
    if (f_heat->ui_state != HEAT_UI_HEATING) {
        return;
    }

    f_heat->heat_live_remain_min = info->remain_min;
    f_heat->heat_live_temp_f = info->temp_f;
    printf("heat_live_remain_min: %d, heat_live_temp_f: %d\n", f_heat->heat_live_remain_min, f_heat->heat_live_temp_f);
    f_heat->heat_live_ready = true;
    f_heat->last_timer_key = 0xffff;
    f_heat->last_temp_f = 0xffff;

    if (info->remain_min == 0) {
        func_heat_heating_finish_check(f_heat);
        return;
    }
    if (f_heat->ui_state == HEAT_UI_HEATING) {
        func_heat_display_refresh(f_heat);
    }
}

static void func_heat_status_icons_init(void)
{
    home_ui_shared_status_init();
}

static void func_heat_status_icons_apply(f_heat_t *f_heat)
{
    func_heat_status_icons_init();

    if (f_heat->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f_heat->pic_bt, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
        compo_picturebox_set_visible(f_heat->pic_bt, true);
    }
    home_ui_shared_status_bind_bat(f_heat->pic_bat);
}
#endif

void func_heat_lock_icon_apply(f_heat_t *f_heat)
{
    if (f_heat == NULL || f_heat->pic_lock == NULL) {
        return;
    }

    if (func_key_lock_show_status_icon(f_heat->screen_locked)) {
        home_ui_shared_status_init();
        compo_picturebox_set_pos(f_heat->pic_lock, HEAT_STATUS_LOCK_X, HEAT_STATUS_Y);
        if (gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
            compo_picturebox_set_ram(f_heat->pic_lock, home_ui_shared_status_lock_ram);
            compo_picturebox_set_size(f_heat->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
            compo_picturebox_set_visible(f_heat->pic_lock, true);
        }
    } else {
        compo_picturebox_set_visible(f_heat->pic_lock, false);
    }
}

static u16 func_heat_get_target_temp_f(const f_heat_t *f_heat)
{
    if (f_heat->temp_idx >= HEAT_TEMP_PRESET_CNT) {
        return tbl_heat_temp_preset[0];
    }
    return tbl_heat_temp_preset[f_heat->temp_idx];
}

/** @brief 将华氏温度映射到预设温度档位索引 (找最接近的) */
static u8 func_heat_temp_f_to_idx(u16 temp_f)
{
    u8 best = 0;
    s16 best_diff = 999;
    for (u8 i = 0; i < HEAT_TEMP_PRESET_CNT; i++) {
        s16 diff = (s16)temp_f - (s16)tbl_heat_temp_preset[i];
        if (diff < 0) diff = -diff;
        if (diff < best_diff) {
            best_diff = diff;
            best = i;
        }
    }
    return best;
}

void func_heat_countdown_set(u8 hour, u8 min)
{
    if (hour > 99) {
        hour = 99;
    }
    if (min > 59) {
        min = 59;
    }
    heat_countdown_remain_sec = (u32)hour * 3600 + (u32)min * 60;
}

void func_heat_countdown_start(void)
{
    heat_countdown_running = true;
}

void func_heat_countdown_stop(void)
{
    heat_countdown_running = false;
}

u32 func_heat_countdown_remain_sec(void)
{
    return heat_countdown_remain_sec;
}

static void func_heat_pic_pos_tr(compo_picturebox_t *pic, s16 tr_x, s16 tr_y, u16 w, u16 h)
{
    if (pic == NULL) {
        return;
    }
    compo_picturebox_set_pos(pic, tr_x - (s16)(w / 2), tr_y + (s16)(h / 2));
}

static void func_heat_temp_layout_ex(f_heat_t *f_heat, u8 digits[HEAT_TEMP_IDX_CNT], bool white)
{
    const u16 *tbl = white ? tbl_heat_w_digit_w : tbl_heat_b_digit_w;
    static const s16 tbl_temp_tr_x[HEAT_TEMP_IDX_CNT] = {
        HEAT_TEMP_TR_H_X, HEAT_TEMP_TR_T10_X, HEAT_TEMP_TR_T1_X,
    };
    u16 digit_h = white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H;
    u8 i;

    for (i = 0; i < HEAT_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];
        u16 w = tbl[d];

        func_heat_pic_pos_tr(f_heat->pic_temp[i], tbl_temp_tr_x[i], HEAT_TEMP_TR_Y, w, digit_h);
        compo_picturebox_set_size(f_heat->pic_temp[i], w, digit_h);
    }

    if (f_heat->pic_temp_degf != NULL) {
        /* 灰/白均用 bhx 布局框 (247,158) 右上角锚点，显示尺寸一致 */
        func_heat_pic_pos_tr(f_heat->pic_temp_degf, HEAT_TEMP_TR_UNIT_X, HEAT_TEMP_TR_UNIT_Y,
                             HEAT_BHX_W, HEAT_BHX_H);
        compo_picturebox_set_size(f_heat->pic_temp_degf, HEAT_BHX_W, HEAT_BHX_H);
        compo_picturebox_set_visible(f_heat->pic_temp_degf, true);
    }
    if (f_heat->pic_temp_suffix != NULL) {
        compo_picturebox_set_visible(f_heat->pic_temp_suffix, false);
    }
}

static void func_heat_temp_update_ex(f_heat_t *f_heat, u16 temp_f, bool white)
{
    u8 digits[HEAT_TEMP_IDX_CNT];
    u32 sym_addr;
    u16 sym_len;
    u8 i;

    if (temp_f > 999) {
        temp_f = 999;
    }

    digits[HEAT_TEMP_IDX_H] = (u8)(temp_f / 100);
    digits[HEAT_TEMP_IDX_T10] = (u8)((temp_f / 10) % 10);
    digits[HEAT_TEMP_IDX_T1] = (u8)(temp_f % 10);

    if (f_heat->last_temp_f == temp_f && f_heat->last_t_white == white) {
        func_heat_temp_layout_ex(f_heat, digits, white);
        return;
    }
    f_heat->last_temp_f = temp_f;
    f_heat->last_t_white = white;

    sym_addr = white ? UI_BUF_HOME_WHX_BIN : UI_BUF_HOME_BHX_BIN;
    sym_len = white ? UI_LEN_HOME_WHX_BIN : UI_LEN_HOME_BHX_BIN;
    os_spiflash_read(heat_temp_degf_ram, sym_addr, sym_len);
    if (f_heat->pic_temp_degf != NULL && gui_set_ram_check(heat_temp_degf_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_temp_degf, heat_temp_degf_ram);
        compo_picturebox_set_visible(f_heat->pic_temp_degf, true);
    }

    if (f_heat->pic_temp_suffix != NULL) {
        compo_picturebox_set_visible(f_heat->pic_temp_suffix, false);
    }

    for (i = 0; i < HEAT_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];
        u32 addr = white ? tbl_heat_w_digit_addr[d] : tbl_heat_b_digit_addr[d];
        u16 len = white ? tbl_heat_w_digit_len[d] : tbl_heat_b_digit_len[d];

        os_spiflash_read(heat_temp_digit_ram[i], addr, len);
        if (gui_set_ram_check(heat_temp_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_heat->pic_temp[i], heat_temp_digit_ram[i]);
        }
    }

    func_heat_temp_layout_ex(f_heat, digits, white);
}

void func_heat_temp_set_f(u16 temp_f)
{
    f_heat_t *f_heat = (f_heat_t *)func_cb.f_cb;

    if (f_heat != NULL) {
        func_heat_temp_update_ex(f_heat, temp_f, true);
    }
}

static void func_heat_lock_check(f_heat_t *f_heat)
{
#if ELUNCHBOX_PANEL_EN
    (void)f_heat;
    /* Panel：加热 30s 自动锁屏由 func_key_lock + lunchbox_heat_start 统一处理 */
#else
    if (f_heat->ui_state != HEAT_UI_HEATING || f_heat->screen_locked) {
        return;
    }
    if (tick_check_expire(f_heat->heat_start_tick, HEAT_LOCK_MS)) {
        f_heat->screen_locked = true;
        func_heat_lock_icon_apply(f_heat);
    }
#endif
}

#if !ELUNCHBOX_PANEL_EN
static void func_heat_heating_finish_check(f_heat_t *f_heat)
{
    if (f_heat->ui_state != HEAT_UI_HEATING) {
        return;
    }
    if (!f_heat->heat_live_ready || f_heat->heat_live_remain_min != 0) {
        return;
    }
    heat_countdown_running = false;
    f_heat->ui_state = HEAT_UI_FINISHED;
    f_heat->display_temp_f = func_heat_get_target_temp_f(f_heat);
    f_heat->screen_locked = false;
    func_heat_lock_icon_apply(f_heat);
    func_heat_led_sync(false);
    func_mode_keep_warm_enter();
}
#endif

static void func_heat_status_refresh(f_heat_t *f_heat)
{
#if ELUNCHBOX_PANEL_EN
    func_heat_lock_check(f_heat);
    return;
#endif
    tm_t tm = rtc_clock_get();

    if (f_heat->last_top_min != tm.min || f_heat->last_top_sec != tm.sec) {
        f_heat->last_top_min = tm.min;
        f_heat->last_top_sec = tm.sec;
        home_top_time_refresh(&f_heat->top_time, &tm);
    }

    func_heat_lock_check(f_heat);
}

#if !ELUNCHBOX_PANEL_EN
static void func_heat_timer_layout_ex(f_heat_t *f_heat, u8 hour, u8 min, bool h_white, bool m_white)
{
    const u16 *htbl = h_white ? tbl_heat_w_digit_w : tbl_heat_b_digit_w;
    const u16 *mtbl = m_white ? tbl_heat_w_digit_w : tbl_heat_b_digit_w;
    u8 h_digits[2] = { hour / 10, hour % 10 };
    u8 m_digits[2] = { min / 10, min % 10 };
    u16 h_digit_h = h_white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H;
    u16 m_digit_h = m_white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H;

    func_heat_pic_pos_tr(f_heat->pic_timer[HEAT_TIMER_IDX_H10],
                         HEAT_TIMER_TR_H10_X, HEAT_TIMER_TR_Y,
                         htbl[h_digits[0]], h_digit_h);
    compo_picturebox_set_size(f_heat->pic_timer[HEAT_TIMER_IDX_H10],
                              htbl[h_digits[0]], h_digit_h);

    func_heat_pic_pos_tr(f_heat->pic_timer[HEAT_TIMER_IDX_H1],
                         HEAT_TIMER_TR_H1_X, HEAT_TIMER_TR_Y,
                         htbl[h_digits[1]], h_digit_h);
    compo_picturebox_set_size(f_heat->pic_timer[HEAT_TIMER_IDX_H1],
                              htbl[h_digits[1]], h_digit_h);

    func_heat_pic_pos_tr(f_heat->pic_timer_colon,
                         HEAT_TIMER_TR_COLON_X, HEAT_TIMER_TR_Y,
                         HEAT_WBX_W, HEAT_WBX_H);
    compo_picturebox_set_size(f_heat->pic_timer_colon, HEAT_WBX_W, HEAT_WBX_H);

    func_heat_pic_pos_tr(f_heat->pic_timer[HEAT_TIMER_IDX_M10],
                         HEAT_TIMER_TR_M10_X, HEAT_TIMER_TR_Y,
                         mtbl[m_digits[0]], m_digit_h);
    compo_picturebox_set_size(f_heat->pic_timer[HEAT_TIMER_IDX_M10],
                              mtbl[m_digits[0]], m_digit_h);

    func_heat_pic_pos_tr(f_heat->pic_timer[HEAT_TIMER_IDX_M1],
                         HEAT_TIMER_TR_M1_X, HEAT_TIMER_TR_Y,
                         mtbl[m_digits[1]], m_digit_h);
    compo_picturebox_set_size(f_heat->pic_timer[HEAT_TIMER_IDX_M1],
                              mtbl[m_digits[1]], m_digit_h);
}
#endif

#if !ELUNCHBOX_PANEL_EN
static void func_heat_timer_update_ex(f_heat_t *f_heat, u8 hour, u8 min, bool h_white, bool m_white)
{
    u8 digits[HEAT_TIMER_IDX_CNT];
    u16 timer_key;
    u8 i;

    digits[HEAT_TIMER_IDX_H10] = hour / 10;
    digits[HEAT_TIMER_IDX_H1] = hour % 10;
    digits[HEAT_TIMER_IDX_M10] = min / 10;
    digits[HEAT_TIMER_IDX_M1] = min % 10;
    timer_key = (u16)hour * 100 + min;
    if (f_heat->last_timer_key == timer_key
        && f_heat->last_h_white == h_white && f_heat->last_m_white == m_white) {
        return;
    }
    f_heat->last_timer_key = timer_key;
    f_heat->last_h_white = h_white;
    f_heat->last_m_white = m_white;

    os_spiflash_read(home_ui_shared_timer_colon_ram, UI_BUF_HOME_WBX_BIN, UI_LEN_HOME_WBX_BIN);
    if (gui_set_ram_check(home_ui_shared_timer_colon_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_timer_colon, home_ui_shared_timer_colon_ram);
    }

    for (i = 0; i < HEAT_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];
        bool white = (i <= HEAT_TIMER_IDX_H1) ? h_white : m_white;
        u32 addr = white ? tbl_heat_w_digit_addr[d] : tbl_heat_b_digit_addr[d];
        u16 len = white ? tbl_heat_w_digit_len[d] : tbl_heat_b_digit_len[d];

        os_spiflash_read(home_ui_shared_timer_digit_ram[i], addr, len);
        if (gui_set_ram_check(home_ui_shared_timer_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_heat->pic_timer[i], home_ui_shared_timer_digit_ram[i]);
        }
    }

    func_heat_timer_layout_ex(f_heat, hour, min, h_white, m_white);
}
#endif

static void func_heat_display_refresh(f_heat_t *f_heat)
{
#if ELUNCHBOX_PANEL_EN
    func_heat_panel_mark_dirty(f_heat);
    return;
#else
    u8 hour;
    u8 min;
    u16 temp;
    bool h_white;
    bool m_white;
    bool t_white;

    if (f_heat->ui_state == HEAT_UI_SETUP) {
        hour = f_heat->set_hour;
        min = f_heat->set_min;
        temp = func_heat_get_target_temp_f(f_heat);
        h_white = (f_heat->focus == HEAT_FOCUS_HOUR);
        m_white = (f_heat->focus == HEAT_FOCUS_MIN);
        t_white = (f_heat->focus == HEAT_FOCUS_TEMP);
    } else if (f_heat->ui_state == HEAT_UI_HEATING) {
        temp = func_heat_get_target_temp_f(f_heat);
        if (f_heat->heat_live_ready) {
            hour = (u8)(f_heat->heat_live_remain_min / 60);
            min = (u8)(f_heat->heat_live_remain_min % 60);
        } else {
            hour = f_heat->set_hour;
            min = f_heat->set_min;
        }
        h_white = true;
        m_white = true;
        t_white = true;
    } else {
        hour = 0;
        min = 0;
        temp = func_heat_get_target_temp_f(f_heat);
        h_white = true;
        m_white = true;
        t_white = true;
    }

    if (hour > 99) {
        hour = 99;
    }
    func_heat_timer_update_ex(f_heat, hour, min, h_white, m_white);
    func_heat_temp_update_ex(f_heat, temp, t_white);
    func_heat_lock_icon_apply(f_heat);
#endif
}

static void func_heat_start_heating(f_heat_t *f_heat)
{
    f_heat->ui_state = HEAT_UI_HEATING;
    f_heat->screen_locked = false;
    f_heat->heat_start_tick = tick_get();
    f_heat->heat_total_sec = (u32)f_heat->set_hour * 3600 + (u32)f_heat->set_min * 60;
    if (f_heat->heat_total_sec == 0) {
        f_heat->heat_total_sec = 60;
    }
    f_heat->heat_live_ready = false;
    f_heat->heat_live_remain_min = 0;
    f_heat->heat_live_temp_f = 0;
    f_heat->last_timer_key = 0xffff;
    f_heat->last_temp_f = 0xffff;
    func_heat_countdown_stop();

#if FUNC_LUNCHBOX_UART_EN
    {
        bool skip_uart = lb_heat_uart_remote_consume();
        u16 target_temp_f = (f_heat->temp_idx < HEAT_TEMP_PRESET_CNT)
                          ? tbl_heat_temp_preset[f_heat->temp_idx]
                          : tbl_heat_temp_preset[0];
        u32 duration_min = f_heat->heat_total_sec / 60;

        if (duration_min < LB_HEAT_DURATION_MIN_MIN) {
            duration_min = LB_HEAT_DURATION_MIN_MIN;
        } else if (duration_min > LB_HEAT_DURATION_MAX_MIN) {
            duration_min = LB_HEAT_DURATION_MAX_MIN;
        }
        if (!skip_uart) {
            printf("lb: heat_start -> UART\n");
            printf("target_temp_f: %d, duration_min: %d, proto_mode: %d\n",
                   lunchbox_temp_f_to_idx(target_temp_f), duration_min, f_heat->proto_mode);
            lunchbox_heat_start(f_heat->proto_mode, lunchbox_temp_f_to_idx(target_temp_f), duration_min);
        } else {
            printf("lb: heat_start skipped UART (MCU remote)\n");
            lunchbox_heat_start_local(f_heat->proto_mode,
                                      lunchbox_temp_f_to_idx(target_temp_f),
                                      duration_min);
        }
    }
#endif

#if ELUNCHBOX_PANEL_EN
    if (!elunchbox_pwr_is_manual_off()) {
        elunchbox_user_activity_reset();
        func_key_lock_on_heating_start();
    }
#endif

    func_heat_led_sync(true);
    func_heat_display_refresh(f_heat);
    printf("start_heating: display_refresh done\n");
}

#if ELUNCHBOX_PANEL_EN
void func_heat_prepare_ble_stop(void)
{
    f_heat_t *f_heat = (f_heat_t *)func_cb.f_cb;

    if (func_cb.sta != FUNC_HEAT || f_heat == NULL) {
        return;
    }
    func_heat_countdown_stop();
    func_heat_reset_setup(f_heat);
}

void func_heat_ble_remote_restart(void)
{
    f_heat_t *f_heat;
    lb_mode_to_heat_preset_t preset;
    u32 total_min;

    if (func_cb.sta != FUNC_HEAT || func_cb.f_cb == NULL) {
        return;
    }
    if (!lb_mode_to_heat_get(&preset)) {
        return;
    }

    f_heat = (f_heat_t *)func_cb.f_cb;
    f_heat->set_hour   = preset.hour;
    f_heat->set_min    = preset.min;
    f_heat->temp_idx   = func_heat_temp_f_to_idx(preset.temp_f);
    f_heat->proto_mode = preset.proto_mode;
    func_heat_setup_apply_total_min(f_heat, func_heat_setup_total_min(f_heat));

    f_heat->ui_state = HEAT_UI_HEATING;
    f_heat->screen_locked = false;
    f_heat->heat_start_tick = tick_get();
    total_min = (u32)f_heat->set_hour * 60 + f_heat->set_min;
    f_heat->heat_total_sec = total_min * 60;
    if (f_heat->heat_total_sec == 0) {
        f_heat->heat_total_sec = 60;
    }
    f_heat->heat_live_ready = false;
    f_heat->heat_live_remain_min = 0;
    f_heat->heat_live_temp_f = 0;
    f_heat->last_timer_key = 0xffff;
    f_heat->last_temp_f = 0xffff;
    func_heat_countdown_set(f_heat->set_hour, f_heat->set_min);
    func_heat_countdown_stop();

#if FUNC_LUNCHBOX_UART_EN && !LB_BRIDGE_MODE
    {
        u16 target_temp_f = (f_heat->temp_idx < HEAT_TEMP_PRESET_CNT)
                          ? tbl_heat_temp_preset[f_heat->temp_idx]
                          : tbl_heat_temp_preset[0];
        u32 duration_min = f_heat->heat_total_sec / 60;

        if (duration_min < LB_HEAT_DURATION_MIN_MIN) {
            duration_min = LB_HEAT_DURATION_MIN_MIN;
        } else if (duration_min > LB_HEAT_DURATION_MAX_MIN) {
            duration_min = LB_HEAT_DURATION_MAX_MIN;
        }
        if (!lb_heat_uart_remote_peek()) {
            lunchbox_heat_start(f_heat->proto_mode, lunchbox_temp_f_to_idx(target_temp_f), duration_min);
        } else {
            (void)lb_heat_uart_remote_consume();
            lunchbox_heat_start_local(f_heat->proto_mode,
                                      lunchbox_temp_f_to_idx(target_temp_f),
                                      duration_min);
        }
    }
#else
    (void)lb_heat_uart_remote_consume();
    if (!elunchbox_pwr_is_manual_off()) {
        elunchbox_user_activity_reset();
        func_key_lock_on_heating_start();
    }
#endif

    func_heat_sync_mcu_snapshot(f_heat);
    func_heat_led_sync(true);
    func_heat_panel_mark_dirty(f_heat);
    {
        u8 was_blocked = elunchbox_te_block_flag;
        if (!was_blocked) {
            elunchbox_te_block_flag = 1;
        }
        home_gpu_wait_idle();
        WDT_CLR();
        func_heat_panel_enter(f_heat);
        home_gpu_wait_idle();
        if (!was_blocked) {
            elunchbox_te_block_flag = 0;
        }
    }
    printf("heat_ble_remote_restart: mode=%u %uh%um temp_idx=%u\n",
           f_heat->proto_mode, f_heat->set_hour, f_heat->set_min, f_heat->temp_idx);
}
#endif

static void func_heat_ok_key(f_heat_t *f_heat)
{
    if (f_heat->ui_state != HEAT_UI_SETUP) {
        return;
    }

    switch (f_heat->focus) {
    case HEAT_FOCUS_HOUR:
        f_heat->focus = HEAT_FOCUS_MIN;
        break;

    case HEAT_FOCUS_MIN:
        f_heat->focus = HEAT_FOCUS_TEMP;
        break;

    case HEAT_FOCUS_TEMP:
        func_heat_start_heating(f_heat);
        return;

    default:
        break;
    }

    f_heat->last_timer_key = 0xffff;
    f_heat->last_temp_f = 0xffff;
    func_heat_display_refresh(f_heat);
}

static void func_heat_power_key(f_heat_t *f_heat)
{
    if (func_key_lock_is_active()) {
        return;
    }
    if (f_heat->ui_state == HEAT_UI_HEATING) {
        func_heat_countdown_stop();
        /* 先注销 LCD 回调再发停止指令，防止 UART RX 在页面切换期间
           触发 func_heat_display_on_info 访问已销毁的 panel widget */
        heat_display_unregister();
#if FUNC_LUNCHBOX_UART_EN
#if ELUNCHBOX_PANEL_EN
        if (home_ui_shared_battery_is_charging()
            || func_elunchbox_warm_from_charging()
            || lb_heat_mcu_nav_active()) {
            lunchbox_heat_clear_local();
            lb_heat_mcu_nav_set(false);
            func_elunchbox_warm_from_charging_set(false);
        } else {
            lunchbox_heat_stop();
        }
#else
        lunchbox_heat_stop();
#endif
#endif
        func_heat_led_sync(false);
        func_heat_reset_setup(f_heat);
        /* 不调 func_switch_to：设 sta 让 while 循环退出，
           由 func_exit() 的安全路径清理 form 和锁键 overlay */
        func_cb.sta = FUNC_HOME;
        return;
    }

    if (f_heat->ui_state == HEAT_UI_FINISHED) {
        func_cb.sta = FUNC_HOME;
        return;
    }

    switch (f_heat->focus) {
    case HEAT_FOCUS_HOUR:
        func_cb.sta = FUNC_HOME;
        break;

    case HEAT_FOCUS_MIN:
        f_heat->focus = HEAT_FOCUS_HOUR;
        f_heat->last_timer_key = 0xffff;
        func_heat_display_refresh(f_heat);
        break;

    case HEAT_FOCUS_TEMP:
        f_heat->focus = HEAT_FOCUS_MIN;
        f_heat->last_timer_key = 0xffff;
        f_heat->last_temp_f = 0xffff;
        func_heat_display_refresh(f_heat);
        break;

    default:
        break;
    }
}

static void func_heat_value_inc(f_heat_t *f_heat)
{
    if (f_heat->ui_state != HEAT_UI_SETUP) {
        return;
    }

    switch (f_heat->focus) {
    case HEAT_FOCUS_HOUR:
        {
            u16 total = func_heat_setup_total_min(f_heat);

            if (total + 60 <= LB_HEAT_DURATION_MAX_MIN) {
                func_heat_setup_apply_total_min(f_heat, total + 60);
            }
        }
        func_heat_countdown_set(f_heat->set_hour, f_heat->set_min);
        break;

    case HEAT_FOCUS_MIN:
        {
            u16 total = func_heat_setup_total_min(f_heat);

            if (total + HEAT_MIN_STEP <= LB_HEAT_DURATION_MAX_MIN) {
                func_heat_setup_apply_total_min(f_heat, total + HEAT_MIN_STEP);
            }
        }
        func_heat_countdown_set(f_heat->set_hour, f_heat->set_min);
        break;

    case HEAT_FOCUS_TEMP:
        if (f_heat->temp_idx + 1 < HEAT_TEMP_PRESET_CNT) {
            f_heat->temp_idx++;
        }
        break;

    default:
        break;
    }

    f_heat->last_timer_key = 0xffff;
    f_heat->last_temp_f = 0xffff;
    func_heat_display_refresh(f_heat);
}

static void func_heat_value_dec(f_heat_t *f_heat)
{
    if (f_heat->ui_state != HEAT_UI_SETUP) {
        return;
    }

    switch (f_heat->focus) {
    case HEAT_FOCUS_HOUR:
        {
            u16 total = func_heat_setup_total_min(f_heat);

            if (total >= LB_HEAT_DURATION_MIN_MIN + 60) {
                func_heat_setup_apply_total_min(f_heat, total - 60);
            } else {
                func_heat_setup_apply_total_min(f_heat, LB_HEAT_DURATION_MIN_MIN);
            }
        }
        func_heat_countdown_set(f_heat->set_hour, f_heat->set_min);
        break;

    case HEAT_FOCUS_MIN:
        {
            u16 total = func_heat_setup_total_min(f_heat);

            if (total >= LB_HEAT_DURATION_MIN_MIN + HEAT_MIN_STEP) {
                func_heat_setup_apply_total_min(f_heat, total - HEAT_MIN_STEP);
            } else {
                func_heat_setup_apply_total_min(f_heat, LB_HEAT_DURATION_MIN_MIN);
            }
        }
        func_heat_countdown_set(f_heat->set_hour, f_heat->set_min);
        break;

    case HEAT_FOCUS_TEMP:
        if (f_heat->temp_idx > 0) {
            f_heat->temp_idx--;
        }
        break;

    default:
        break;
    }

    f_heat->last_timer_key = 0xffff;
    f_heat->last_temp_f = 0xffff;
    func_heat_display_refresh(f_heat);
}

compo_form_t *func_heat_form_create(void)
{
#if ELUNCHBOX_PANEL_EN
    return func_heat_panel_form_create();
#else
    compo_form_t *frm = compo_form_create(true);
    compo_picturebox_t *pic;

    home_top_time_create(frm, UI_HEAT_PLACEHOLDER,
                         COMPO_ID_PIC_TOP_TIME_H10, COMPO_ID_PIC_TOP_TIME_H1,
                         COMPO_ID_PIC_TOP_TIME_COLON, COMPO_ID_PIC_TOP_TIME_M10,
                         COMPO_ID_PIC_TOP_TIME_M1, COMPO_ID_PIC_TOP_TIME_AMPM);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, HEAT_STATUS_BT_X, HEAT_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BT_W, HOME_STATUS_BT_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_LOCK);
    compo_picturebox_set_pos(pic, HEAT_STATUS_LOCK_X, HEAT_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, HEAT_STATUS_BAT_X, HEAT_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_H10);
    func_heat_pic_pos_tr(pic, HEAT_TIMER_TR_H10_X, HEAT_TIMER_TR_Y, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_H1);
    func_heat_pic_pos_tr(pic, HEAT_TIMER_TR_H1_X, HEAT_TIMER_TR_Y, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_M10);
    func_heat_pic_pos_tr(pic, HEAT_TIMER_TR_M10_X, HEAT_TIMER_TR_Y, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_M1);
    func_heat_pic_pos_tr(pic, HEAT_TIMER_TR_M1_X, HEAT_TIMER_TR_Y, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_COLON);
    func_heat_pic_pos_tr(pic, HEAT_TIMER_TR_COLON_X, HEAT_TIMER_TR_Y, HEAT_WBX_W, HEAT_WBX_H);
    compo_picturebox_set_size(pic, HEAT_WBX_W, HEAT_WBX_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TEMP_H);
    func_heat_pic_pos_tr(pic, HEAT_TEMP_TR_H_X, HEAT_TEMP_TR_Y, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TEMP_T10);
    func_heat_pic_pos_tr(pic, HEAT_TEMP_TR_T10_X, HEAT_TEMP_TR_Y, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TEMP_T1);
    func_heat_pic_pos_tr(pic, HEAT_TEMP_TR_T1_X, HEAT_TEMP_TR_Y, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TEMPF);
    func_heat_pic_pos_tr(pic, HEAT_TEMP_TR_UNIT_X, HEAT_TEMP_TR_UNIT_Y, HEAT_BHX_W, HEAT_BHX_H);
    compo_picturebox_set_size(pic, HEAT_BHX_W, HEAT_BHX_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TEMP_S);
    func_heat_pic_pos_tr(pic, HEAT_TEMP_TR_UNIT_X, HEAT_TEMP_TR_UNIT_Y, HEAT_BHX_W, HEAT_BHX_H);
    compo_picturebox_set_size(pic, HEAT_BHX_W, HEAT_BHX_H);
    compo_picturebox_set_visible(pic, false);
    compo_picturebox_set_visible(pic, false);

    return frm;
#endif
}

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static void func_heat_key_debug_press(f_heat_t *f_heat, u8 press_tch)
{
    printf("[HEAT_KEY] take tch=%u ui=%u locked=%u key_lock=%u switching=%u "
           "guioff=%u warm_chg=%u mcu_nav=%u uart_remote=%u te_blk=%u\n",
           press_tch,
           f_heat ? f_heat->ui_state : 0xff,
           (f_heat != NULL && f_heat->screen_locked) ? 1u : 0u,
           func_key_lock_is_active() ? 1u : 0u,
           sys_cb.flag_swithing ? 1u : 0u,
           elunchbox_pwr_gui_off_is_on() ? 1u : 0u,
           func_elunchbox_warm_from_charging() ? 1u : 0u,
           lb_heat_mcu_nav_active() ? 1u : 0u,
           lb_heat_uart_remote_peek() ? 1u : 0u,
           elunchbox_te_block_flag ? 1u : 0u);
}

static void func_heat_pt8028_keys_process(f_heat_t *f_heat)
{
    u8 press_tch;

    if (f_heat == NULL || sys_cb.flag_swithing) {
        return;
    }
    pt8028_key_scan_page();
    if (func_key_lock_is_active()) {
        (void)func_key_lock_press_take_poll();
        return;
    }
    if (func_key_lock_press_take_guarded(&press_tch)) {
        return;
    }
    if (press_tch == 0xff) {
        return;
    }
    func_heat_key_debug_press(f_heat, press_tch);
    if (press_tch <= PT8028_KEY_TCH6 && press_tch != PT8028_KEY_TCH4) {
        elunchbox_user_activity_reset();
    }
    if (press_tch == PT8028_KEY_TCH5) {
        pt8028_defer_key_sound_tch(PT8028_KEY_TCH5);
        func_heat_power_key(f_heat);
    } else if (press_tch == PT8028_KEY_TCH4) {
        pt8028_defer_key_sound_tch(PT8028_KEY_TCH4);
        func_heat_ok_key(f_heat);
    } else if (press_tch == PT8028_KEY_TCH6) {
        pt8028_defer_key_sound_tch(PT8028_KEY_TCH6);
        func_heat_value_dec(f_heat);
    } else if (press_tch == PT8028_KEY_TCH2) {
        pt8028_defer_key_sound_tch(PT8028_KEY_TCH2);
        func_heat_value_inc(f_heat);
    } else if (press_tch == PT8028_KEY_TCH3) {
        if (!sys_cb.flag_swithing) {
            pt8028_defer_key_sound_tch(PT8028_KEY_TCH3);
            func_switch_to(FUNC_NEW_MODE, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
    }
}
#endif

static void func_heat_process(void)
{
    f_heat_t *f_heat = (f_heat_t *)func_cb.f_cb;

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    if (f_heat != NULL && !f_heat->key_ready) {
        func_process();
        func_home_drain_stale_key_msgs();
        pt8028_release_clear();
        (void)pt8028_take_press_tch();
        f_heat->key_ready = true;
        printf("[HEAT_KEY] key_ready=1 warm_chg=%u mcu_nav=%u\n",
               func_elunchbox_warm_from_charging() ? 1u : 0u,
               lb_heat_mcu_nav_active() ? 1u : 0u);
        return;
    }
    func_heat_pt8028_keys_process(f_heat);
#endif
    if (f_heat != NULL) {
#if ELUNCHBOX_PANEL_EN
        func_heat_panel_process(f_heat);
#endif
        func_heat_status_refresh(f_heat);
    }
    func_process();
}

static void func_heat_message(size_msg_t msg)
{
    f_heat_t *f_heat = (f_heat_t *)func_cb.f_cb;

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    if (f_heat != NULL && f_heat->key_ready) {
        switch (msg) {
        case HEAT_MSG_OK:
        case HEAT_MSG_PLUS:
        case HEAT_MSG_MINUS:
        case HEAT_MSG_POWER:
            return;
        default:
            break;
        }
    }
    if (msg != NO_KEY && msg != 0) {
        printf("[HEAT_KEY] ku msg=0x%04x ui=%u locked=%u key_lock=%u switching=%u\n",
               msg,
               f_heat ? f_heat->ui_state : 0xff,
               (f_heat != NULL && f_heat->screen_locked) ? 1u : 0u,
               func_key_lock_is_active() ? 1u : 0u,
               sys_cb.flag_swithing ? 1u : 0u);
    }
#endif

    if (sys_cb.flag_swithing) {
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        if (msg != NO_KEY && msg != 0) {
            printf("[HEAT_KEY] drop ku: switching\n");
        }
#endif
        return;
    }

    if (func_key_lock_ku_blocked(msg)) {
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        if (msg != NO_KEY && msg != 0) {
            printf("[HEAT_KEY] drop ku: key_lock\n");
        }
#endif
        return;
    }

    if (f_heat != NULL && f_heat->screen_locked) {
        if (msg != HEAT_MSG_POWER && msg != HEAT_MSG_OK) {
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
            if (msg != NO_KEY && msg != 0) {
                printf("[HEAT_KEY] drop ku: screen_locked\n");
            }
#endif
            return;
        }
    }

    switch (msg) {
    case HEAT_MSG_OK:
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        printf("[HEAT_KEY] act OK\n");
#endif
        pt8028_defer_key_sound_tch(PT8028_KEY_TCH4);
        func_heat_ok_key(f_heat);
        break;

    case HEAT_MSG_PLUS:
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        printf("[HEAT_KEY] act PLUS\n");
#endif
        pt8028_defer_key_sound_tch(PT8028_KEY_TCH6);
        func_heat_value_dec(f_heat);
        break;

    case HEAT_MSG_MINUS:
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        printf("[HEAT_KEY] act MINUS\n");
#endif
        pt8028_defer_key_sound_tch(PT8028_KEY_TCH2);
        func_heat_value_inc(f_heat);
        break;

    case HEAT_MSG_POWER:
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        printf("[HEAT_KEY] act POWER\n");
#endif
        pt8028_defer_key_sound_tch(PT8028_KEY_TCH5);
        func_heat_power_key(f_heat);
        break;

    case KU_LEFT:
        break;

    case KU_MODE:
#if ELUNCHBOX_PANEL_EN
        /* 模式键：跳转到模式选择页 */
        if (!sys_cb.flag_swithing) {
            pt8028_defer_key_sound_tch(PT8028_KEY_TCH3);
            func_switch_to(FUNC_NEW_MODE, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
        break;
#else
        break;
#endif

    default:
        func_message(msg);
        break;
    }
}

void func_heat_enter(void)
{
    f_heat_t *f_heat;

#if ELUNCHBOX_PANEL_EN
    home_gpu_wait_idle();
    WDT_CLR();
#endif

    func_cb.f_cb = func_zalloc(sizeof(f_heat_t));
    func_cb.frm_main = func_heat_form_create();

    f_heat = (f_heat_t *)func_cb.f_cb;
    f_heat->focus = HEAT_FOCUS_HOUR;

    // 检查是否从模式界面 (鸡腿/意面) 跳转过来
    {
        lb_mode_to_heat_preset_t preset;
        if (lb_mode_to_heat_get(&preset)) {
            // 使用模式界面的预设参数
            f_heat->ui_state   = HEAT_UI_SETUP;
            f_heat->set_hour   = preset.hour;
            f_heat->set_min    = preset.min;
            f_heat->temp_idx   = func_heat_temp_f_to_idx(preset.temp_f);
            f_heat->proto_mode = preset.proto_mode;
        } else {
            // 默认：自定义加热
            f_heat->ui_state   = HEAT_UI_SETUP;
            f_heat->set_hour   = 1;
            f_heat->set_min    = 0;
            f_heat->temp_idx   = 4;    // 176°F (80°C)
            f_heat->proto_mode = 1;
        }
    }
    func_heat_setup_apply_total_min(f_heat, func_heat_setup_total_min(f_heat));

    f_heat->display_temp_f = 0;
    f_heat->heat_live_remain_min = 0;
    f_heat->heat_live_temp_f = 0;
    f_heat->heat_live_ready = false;
    f_heat->heat_total_sec = 0;
    f_heat->heat_start_tick = 0;
    f_heat->screen_locked = false;
    f_heat->last_top_min = 0xff;
    f_heat->last_top_sec = 0xff;
    f_heat->last_timer_key = 0xffff;
    f_heat->last_temp_f = 0xffff;
    f_heat->last_h_white = false;
    f_heat->last_m_white = false;
    f_heat->last_t_white = false;
#if ELUNCHBOX_PANEL_EN
    f_heat->key_ready = false;
#endif

#if ELUNCHBOX_PANEL_EN
    printf("heat_enter: create form done, te_block=%d\n", elunchbox_te_block_flag);
    home_gpu_wait_idle();
    WDT_CLR();

    func_heat_panel_bind(f_heat);
    printf("heat_enter: panel bind done\n");
    func_heat_countdown_set(f_heat->set_hour, f_heat->set_min);
    func_heat_countdown_stop();
    func_heat_panel_mark_dirty(f_heat);
    printf("heat_enter: panel bind done, check autostart\n");

    if (lb_heat_autostart_consume()) {
        printf("heat_enter: autostart -> start_heating\n");
        func_elunchbox_warm_from_charging_set(false);
        lb_heat_mcu_nav_set(false);
        func_heat_start_heating(f_heat);
        func_heat_sync_mcu_snapshot(f_heat);
        printf("heat_enter: start_heating done ui=%u key_lock=%u mcu_nav=%u remote=%u\n",
               f_heat->ui_state,
               func_key_lock_is_active() ? 1u : 0u,
               lb_heat_mcu_nav_active() ? 1u : 0u,
               lb_heat_uart_remote_peek() ? 1u : 0u);
    } else if (f_heat->proto_mode != 1) {
        printf("heat_enter: proto_mode=%d -> start_heating\n", f_heat->proto_mode);
        func_heat_start_heating(f_heat);
        func_heat_sync_mcu_snapshot(f_heat);
        printf("heat_enter: start_heating done\n");
    } else if (f_heat->ui_state == HEAT_UI_SETUP) {
        func_switch_to(FUNC_NEW_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        return;
    }

    home_gpu_wait_idle();
    WDT_CLR();
    {
        u8 was_blocked = elunchbox_te_block_flag;
        if (!was_blocked) {
            elunchbox_te_block_flag = 1;
        }
        func_heat_panel_enter(f_heat);
        home_gpu_wait_idle();
        WDT_CLR();
        if (!was_blocked) {
            elunchbox_te_block_flag = 0;
        }
    }
    printf("heat_enter: panel enter done\n");
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(0);
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif
#else
    home_top_time_bind(&f_heat->top_time, COMPO_ID_PIC_TOP_TIME_H10, COMPO_ID_PIC_TOP_TIME_H1,
                       COMPO_ID_PIC_TOP_TIME_COLON, COMPO_ID_PIC_TOP_TIME_M10,
                       COMPO_ID_PIC_TOP_TIME_M1, COMPO_ID_PIC_TOP_TIME_AMPM);
    {
        tm_t tm = rtc_clock_get();
        home_top_time_refresh(&f_heat->top_time, &tm);
    }
    for (u8 i = 0; i < HEAT_TIMER_IDX_CNT; i++) {
        f_heat->pic_timer[i] = compo_getobj_byid(tbl_heat_timer_id[i]);
    }
    f_heat->pic_timer_colon = compo_getobj_byid(COMPO_ID_PIC_TIMER_COLON);
    for (u8 i = 0; i < HEAT_TEMP_IDX_CNT; i++) {
        f_heat->pic_temp[i] = compo_getobj_byid(tbl_heat_temp_id[i]);
    }
    f_heat->pic_temp_degf = compo_getobj_byid(COMPO_ID_PIC_TEMPF);
    f_heat->pic_temp_suffix = compo_getobj_byid(COMPO_ID_PIC_TEMP_S);
    f_heat->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f_heat->pic_lock = compo_getobj_byid(COMPO_ID_PIC_LOCK);
    f_heat->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);

    home_ui_shared_battery_attach_pic(f_heat->pic_bat);
    func_heat_status_icons_apply(f_heat);
    func_heat_countdown_set(f_heat->set_hour, f_heat->set_min);
    func_heat_countdown_stop();
    func_heat_display_refresh(f_heat);
    func_heat_status_refresh(f_heat);
    heat_display_register(func_heat_display_on_info);

    if (lb_heat_autostart_consume()) {
        func_heat_start_heating(f_heat);
    } else if (f_heat->proto_mode != 1) {
        func_heat_start_heating(f_heat);
    }
#endif
}

void func_heat_exit(void)
{
    f_heat_t *f_heat = (f_heat_t *)func_cb.f_cb;

#if ELUNCHBOX_PANEL_EN
    if (func_cb.sta == FUNC_NEW_WARM) {
        func_heat_panel_exit_to_warm();
    } else if (func_cb.sta != FUNC_HOME) {  //加了这个判断if (func_cb.sta != FUNC_HOME)
        /* 切 HOME 时不在此处做 GPU detach：
         * func_heat_panel_exit() 中的 os_gui_draw_force() 会强制渲染锁键
         * overlay，而 overlay 缓冲区在后续 func_exit() 中才被释放，导致
         * GPU 仍持有对已释放内存的引用 → C241 崩溃。
         * GPU 资源交由 func_exit() 的 compo_form_destroy() + compos_init()
         * 统一清理（与 FUNC_NEW_HEAT 行为一致）。 */
        func_heat_panel_exit();
    }
#else
    (void)f_heat;
#endif
    home_ui_shared_battery_detach_pic();
    if (func_cb.sta != FUNC_NEW_WARM && func_cb.sta != FUNC_HOME) { //加了func_cb.sta != FUNC_HOME
        heat_display_unregister();
    }
#if FUNC_LUNCHBOX_UART_EN
    if (f_heat != NULL && f_heat->ui_state == HEAT_UI_HEATING) {
#if ELUNCHBOX_PANEL_EN
        /* MCU 驱动跳页 / 充电转保温：勿回发 stop */
        if (func_cb.sta == FUNC_NEW_WARM || home_ui_shared_battery_is_charging()
            || lb_heat_mcu_nav_active() || lb_heat_uart_remote_peek()) {
            lunchbox_heat_clear_local();
        } else {
            lunchbox_heat_stop();
        }
#else
        lunchbox_heat_stop();
#endif
    }
#else
    func_heat_countdown_stop();
#endif
    func_heat_led_sync(false);
    func_cb.last = FUNC_HEAT;
}

bool func_heat_ui_is_heating(void)
{
    f_heat_t *f_heat;

    if (func_cb.sta != FUNC_HEAT || func_cb.f_cb == NULL) {
        return false;
    }
    f_heat = (f_heat_t *)func_cb.f_cb;
    return f_heat->ui_state == HEAT_UI_HEATING;
}

/** UART 上报加热结束/remain=0 是否可信（须先收到过 remain>0，避免开局残留 DP 误切保温） */
bool func_heat_uart_finish_ok(void)
{
    f_heat_t *f_heat;

    if (!func_heat_ui_is_heating()) {
        return false;
    }
    f_heat = (f_heat_t *)func_cb.f_cb;
    return f_heat->heat_live_ready;
}

#if ELUNCHBOX_PANEL_EN
u8 func_heat_panel_get_ui_state(const void *f)
{
    const f_heat_t *f_heat = (const f_heat_t *)f;

    if (f_heat == NULL) {
        return 0;
    }
    return f_heat->ui_state;
}

u8 func_heat_panel_get_set_hour(const void *f)
{
    const f_heat_t *f_heat = (const f_heat_t *)f;

    if (f_heat == NULL) {
        return 0;
    }
    return f_heat->set_hour;
}

u8 func_heat_panel_get_set_min(const void *f)
{
    const f_heat_t *f_heat = (const f_heat_t *)f;

    if (f_heat == NULL) {
        return 0;
    }
    return f_heat->set_min;
}

u8 func_heat_panel_get_temp_idx(const void *f)
{
    const f_heat_t *f_heat = (const f_heat_t *)f;

    if (f_heat == NULL) {
        return 0;
    }
    return f_heat->temp_idx;
}

u32 func_heat_panel_get_remain_min(const void *f)
{
    const f_heat_t *f_heat = (const f_heat_t *)f;

    if (f_heat == NULL) {
        return 0;
    }
    return f_heat->heat_live_remain_min;
}

u32 func_heat_panel_get_total_sec(const void *f)
{
    const f_heat_t *f_heat = (const f_heat_t *)f;

    if (f_heat == NULL) {
        return 0;
    }
    return f_heat->heat_total_sec;
}

u32 func_heat_panel_get_start_tick(const void *f)
{
    const f_heat_t *f_heat = (const f_heat_t *)f;

    if (f_heat == NULL) {
        return 0;
    }
    return f_heat->heat_start_tick;
}

bool func_heat_panel_get_live_ready(const void *f)
{
    const f_heat_t *f_heat = (const f_heat_t *)f;

    if (f_heat == NULL) {
        return false;
    }
    return f_heat->heat_live_ready;
}

u16 func_heat_panel_get_live_temp_f(const void *f)
{
    const f_heat_t *f_heat = (const f_heat_t *)f;

    if (f_heat == NULL) {
        return 0;
    }
    return f_heat->heat_live_temp_f;
}

bool func_heat_panel_is_heating(const void *f)
{
    const f_heat_t *f_heat = (const f_heat_t *)f;

    if (f_heat == NULL) {
        return false;
    }
    return f_heat->ui_state == HEAT_UI_HEATING;
}

void func_heat_panel_set_live(struct f_heat_t_ *f_heat, u32 remain_min, u16 temp_f)
{
    f_heat_t *f = (f_heat_t *)f_heat;

    if (f == NULL) {
        return;
    }
    f->heat_live_remain_min = remain_min;
    f->heat_live_temp_f = temp_f;
    f->heat_live_ready = true;
    f->last_timer_key = 0xffff;
    f->last_temp_f = 0xffff;
}

#if ELUNCHBOX_PANEL_EN
static void func_heat_sync_mcu_snapshot(f_heat_t *f_heat)
{
    heat_display_info_t snap;

    if (f_heat == NULL || f_heat->ui_state != HEAT_UI_HEATING) {
        return;
    }
    if (!heat_display_get_last(&snap)) {
        return;
    }
    func_heat_panel_set_live(f_heat, snap.remain_min, snap.temp_f);
    f_heat->temp_idx = func_heat_temp_f_to_idx(snap.temp_f);
    func_heat_panel_ack_mcu_live(snap.remain_min);
    printf("heat_sync_mcu: remain=%umin temp=%uF idx=%u\n",
           snap.remain_min, snap.temp_f, f_heat->temp_idx);
}
#endif

#if ELUNCHBOX_PANEL_EN
static void func_elunchbox_enter_warm_from_heat_body(void);
#endif

void func_heat_panel_heating_finish(struct f_heat_t_ *f_heat)
{
    f_heat_t *f = (f_heat_t *)f_heat;

    if (f == NULL || f->ui_state != HEAT_UI_HEATING) {
        return;
    }
    if (!f->heat_live_ready || f->heat_live_remain_min != 0) {
        return;
    }
    func_elunchbox_enter_warm_from_heat();
}

void func_elunchbox_enter_warm_from_heat(void)
{
    func_elunchbox_warm_from_charging_set(false);
    func_elunchbox_enter_warm_from_heat_body();
}

void func_elunchbox_enter_warm_from_charging(void)
{
    func_elunchbox_warm_from_charging_set(true);
    lb_heat_mcu_nav_set(true);
    printf("[LCD_ROUTE] MCU DP02=5 charge -> warm (sta=%u)\n", func_cb.sta);
    func_elunchbox_enter_warm_from_heat_body();
}

static void func_elunchbox_enter_warm_common_prep(void)
{
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    func_key_lock_on_heating_stop();
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif
    heat_display_unregister();
    heat_display_warm_exit_reset();
#if FUNC_LUNCHBOX_UART_EN
    if (func_elunchbox_warm_from_charging()) {
        lunchbox_warm_mark_active();
    } else if (!lunchbox_keep_warm_is_active()) {
        lunchbox_keep_warm_start();
    }
#endif
}

static void func_elunchbox_enter_warm_from_heat_body(void)
{
    f_heat_t *f;

#if !ELUNCHBOX_PANEL_EN
    func_heat_countdown_stop();
    func_mode_keep_warm_enter();
    return;
#endif

    if (func_cb.sta == FUNC_NEW_WARM) {
        return;
    }

    /* Home/预约等待等：预约到点+充电转保温，尚未在加热页 */
    if (func_cb.sta != FUNC_HEAT || func_cb.f_cb == NULL) {
        func_elunchbox_enter_warm_common_prep();
        func_elunchbox_switch_to_warm_panel();
        return;
    }

    f = (f_heat_t *)func_cb.f_cb;
    if (!func_elunchbox_warm_from_charging() && f->ui_state != HEAT_UI_HEATING) {
        return;
    }

    if (!func_elunchbox_warm_from_charging()) {
        printf("heat_finish -> warm panel\n");
    }
    func_heat_countdown_stop();
    f->ui_state = HEAT_UI_FINISHED;
    f->screen_locked = false;
    func_heat_led_sync(false);
    func_elunchbox_enter_warm_common_prep();
    if (!elunchbox_te_block_flag) {
        elunchbox_te_block_flag = 1;
    }
    func_cb.sta = FUNC_NEW_WARM;
}
#endif

#if ELUNCHBOX_PANEL_EN
static bool func_heat_key_page_ok(void)
{
    switch (func_cb.sta) {
    case FUNC_HOME:
    case FUNC_MODE:
    case FUNC_SETUP:
    case FUNC_HEAT:
    case FUNC_LANGUAGEING:
    case FUNC_TIMEING:
    case FUNC_VERINFO:
    case FUNC_RESERVATION:
        return true;
    default:
        return false;
    }
}

void func_heat_key_poll(void)
{
    static u8 heat_key_lp_tch = PT8028_KEY_NONE;
#if PT8028_HEAT_LONG_MS > 0
    static u32 heat_key_lp_tick;
#endif
    static bool heat_key_lp_wait_rel;

    u8 tch;

#if ELUNCHBOX_PANEL_EN
    /* Home 在 new_home_pt8028_keys_process 内处理 TCH1 */
    if (func_cb.sta == FUNC_HOME) {
        return;
    }
#endif

    if (sys_cb.flag_swithing || func_cb.sta == FUNC_HEAT) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (func_cb.sta == FUNC_NEW_HEAT) {
        return;
    }
#endif
#if FUNC_RESERVATION_UI_EN
    if (func_cb.sta == FUNC_RESERVATION && !func_new_reservation_key_ready()) {
        return;
    }
#endif
    if (!func_heat_key_page_ok()) {
        return;
    }

    if (heat_key_lp_wait_rel) {
        if (pt8028_get_press_tch() != PT8028_KEY_TCH1) {
            heat_key_lp_wait_rel = false;
        }
        return;
    }

    if (func_key_lock_is_active()) {
        heat_key_lp_tch = PT8028_KEY_NONE;
        return;
    }

    tch = pt8028_get_press_tch();
    if (tch == PT8028_KEY_TCH1) {
        if (heat_key_lp_tch != PT8028_KEY_TCH1) {
            heat_key_lp_tch = PT8028_KEY_TCH1;
#if PT8028_HEAT_LONG_MS > 0
            heat_key_lp_tick = tick_get();
#endif
#if PT8028_HEAT_LONG_MS == 0
            func_elunchbox_switch_to_heat();
            heat_key_lp_wait_rel = true;
#endif
        }
#if PT8028_HEAT_LONG_MS > 0
        else if (tick_check_expire(heat_key_lp_tick, PT8028_HEAT_LONG_MS)) {
            func_elunchbox_switch_to_heat();
            heat_key_lp_wait_rel = true;
        }
#endif
    } else {
        heat_key_lp_tch = PT8028_KEY_NONE;
    }
}
#else
void func_heat_key_poll(void)
{
}
#endif

void func_heat(void)
{
    printf("%s\n", __func__);
    func_heat_enter();
    while (func_cb.sta == FUNC_HEAT) {
        func_heat_message(msg_dequeue());
        func_heat_process();
    }
    func_heat_exit();
}
