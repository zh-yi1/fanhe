#include "include.h"
#include "func.h"
#include "home_icon_res.h"
#include "home_ui_ram.h"
#include "home_ui_shared.h"
#include "home_top_time.h"

#if FUNC_LUNCHBOX_UART_EN
#include "func_lunchbox_uart.h"
#endif

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * Heat 页 UI（320×240 横屏设计图）：
 *   顶栏 Y≈20：左上 RTC，右上 BT/锁/电量
 *   中部 Y≈100：大号倒计时 HH:MM（选中白字 / 未选中灰字）
 *   下部 Y≈190：温度 XXX°F（选中/加热 whx+wsx 白字，未选中 bhx 灰字）
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

#ifndef UI_BUF_HOME_BATTERY_LEVEL_BIN
#error "Missing battery_level.bin: add ui/home/battery_level.png and run gen_home_icons.py + prebuild.bat"
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
#define HEAT_TIMER_Y                      100   /* 倒计时区中心（设计 Y≈60~140） */
#define HEAT_TEMP_Y                       190   /* 温度区中心（设计 Y≈160~220） */
#else
#define HEAT_STATUS_Y                     HEAT_SY(48)
#define HEAT_STATUS_RIGHT_MARGIN          HEAT_SX(24)
#define HEAT_STATUS_GAP                   HEAT_SX(10)
#define HEAT_TIMER_Y                      HEAT_SY(195)
#define HEAT_TEMP_Y                       HEAT_SY(305)
#endif

#define HEAT_STATUS_BAT_X                 (GUI_SCREEN_WIDTH - HEAT_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define HEAT_STATUS_LOCK_X                (HEAT_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - HEAT_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define HEAT_STATUS_BT_X                  (HEAT_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - HEAT_STATUS_GAP - HOME_STATUS_BT_W / 2)
#define HEAT_TIMER_PAIR_GAP               10  /* 时/分各位数字之间 */
#define HEAT_TIMER_PAIR_NARROW_EXTRA      8   /* 含数字 1 等窄字时加宽（如 01） */
#define HEAT_TIMER_COLON_GAP              18  /* 时末位与冒号 */
#define HEAT_TIMER_COLON_GAP_AFTER        18  /* 冒号与分（与左侧对称） */
#define HEAT_TEMP_DIGIT_GAP               14  /* 温度各位基础间距 */
#define HEAT_TEMP_DIGIT_BEFORE_ONE_EXTRA  20  /* 窄字 1 前额外加宽（如 212 的 21） */
#define HEAT_TEMP_DIGIT_ZERO_PAIR_EXTRA   10  /* 相邻 00 额外加宽（加热初显 000°F） */
#ifndef HEAT_TIMER_COLON_GAP_AFTER
#define HEAT_TIMER_COLON_GAP_AFTER        HEAT_TIMER_COLON_GAP
#endif
#define HEAT_TEMP_SYMBOL_GAP              15  /* 末位数字与 °F 单位之间 */
/* whx/wsx 由 bhx 裁切时已含 ° 与 F 之间的空白，不再额外加间距 */
#define HEAT_TEMP_SYM_INNER_GAP           0
#define HEAT_LOCK_MS                      30000
#define HEAT_TEMP_PRESET_CNT              5
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
    u16 display_temp_f;
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
    140, 158, 176, 194, 212,
};

static u8 heat_temp_digit_ram[HEAT_TEMP_IDX_CNT][HEAT_B_DIGIT_RAM_MAX_SIZE];
static u8 heat_temp_degf_ram[HEAT_TEMP_DEG_RAM_MAX_SIZE];
static u8 heat_temp_suffix_ram[HEAT_TEMP_SUF_RAM_MAX_SIZE];

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

static const u16 tbl_heat_timer_id[HEAT_TIMER_IDX_CNT] = {
    COMPO_ID_PIC_TIMER_H10, COMPO_ID_PIC_TIMER_H1,
    COMPO_ID_PIC_TIMER_M10, COMPO_ID_PIC_TIMER_M1,
};

static const u16 tbl_heat_temp_id[HEAT_TEMP_IDX_CNT] = {
    COMPO_ID_PIC_TEMP_H, COMPO_ID_PIC_TEMP_T10, COMPO_ID_PIC_TEMP_T1,
};

static void func_heat_display_refresh(f_heat_t *f_heat);

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
    if (f_heat->pic_bat != NULL && gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_bat, home_ui_shared_status_bat_ram);
        compo_picturebox_set_size(f_heat->pic_bat, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
        compo_picturebox_set_visible(f_heat->pic_bat, true);
    }
}

static void func_heat_lock_icon_apply(f_heat_t *f_heat)
{
    if (f_heat == NULL || f_heat->pic_lock == NULL) {
        return;
    }

    if (f_heat->screen_locked && gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_lock, home_ui_shared_status_lock_ram);
        compo_picturebox_set_size(f_heat->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
        compo_picturebox_set_visible(f_heat->pic_lock, true);
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

static u16 func_heat_timer_pair_gap_ex(const u16 *tbl_w, u8 d0, u8 d1)
{
    u16 gap = HEAT_TIMER_PAIR_GAP;

    if (d0 == 1 || d1 == 1 || tbl_w[d0] <= HEAT_W1X_W || tbl_w[d1] <= HEAT_W1X_W) {
        gap += HEAT_TIMER_PAIR_NARROW_EXTRA;
    }
    return gap;
}

static u16 func_heat_timer_total_w_ex(u8 hour, u8 min, bool h_white, bool m_white)
{
    const u16 *htbl = h_white ? tbl_heat_w_digit_w : tbl_heat_b_digit_w;
    const u16 *mtbl = m_white ? tbl_heat_w_digit_w : tbl_heat_b_digit_w;
    u8 h_digits[2] = { hour / 10, hour % 10 };
    u8 m_digits[2] = { min / 10, min % 10 };
    u16 h_pair_gap = func_heat_timer_pair_gap_ex(htbl, h_digits[0], h_digits[1]);
    u16 m_pair_gap = func_heat_timer_pair_gap_ex(mtbl, m_digits[0], m_digits[1]);

    return htbl[h_digits[0]] + h_pair_gap + htbl[h_digits[1]] + HEAT_TIMER_COLON_GAP
         + HEAT_WBX_W + HEAT_TIMER_COLON_GAP_AFTER
         + mtbl[m_digits[0]] + m_pair_gap + mtbl[m_digits[1]];
}

static u16 func_heat_temp_digit_gap(u8 d_left, u8 d_right)
{
    u16 gap = HEAT_TEMP_DIGIT_GAP;

    if (d_right == 1) {
        gap += HEAT_TEMP_DIGIT_BEFORE_ONE_EXTRA;
    }
    if (d_left == 0 && d_right == 0) {
        gap += HEAT_TEMP_DIGIT_ZERO_PAIR_EXTRA;
    }
    return gap;
}

static u16 func_heat_temp_total_w_ex(u8 digits[HEAT_TEMP_IDX_CNT], bool white)
{
    const u16 *tbl = white ? tbl_heat_w_digit_w : tbl_heat_b_digit_w;
    u16 total = 0;
    u8 i;

    for (i = 0; i < HEAT_TEMP_IDX_CNT; i++) {
        total += tbl[digits[i]];
        if (i + 1 < HEAT_TEMP_IDX_CNT) {
            total += func_heat_temp_digit_gap(digits[i], digits[i + 1]);
        }
    }
    total += HEAT_TEMP_SYMBOL_GAP;
    if (white) {
        total += HEAT_WHX_W;
        if (HEAT_WSX_W > 0) {
            total += HEAT_TEMP_SYM_INNER_GAP + HEAT_WSX_W;
        }
    } else {
        total += HEAT_BHX_W;
    }
    return total;
}

static void func_heat_temp_layout_ex(f_heat_t *f_heat, u8 digits[HEAT_TEMP_IDX_CNT], bool white)
{
    const u16 *tbl = white ? tbl_heat_w_digit_w : tbl_heat_b_digit_w;
    u16 total;
    s16 x;
    u8 i;
    u16 digit_h = white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H;

    total = func_heat_temp_total_w_ex(digits, white);
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2);

    for (i = 0; i < HEAT_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];
        u16 w = tbl[d];
        s16 cx = x + (s16)(w / 2);

        compo_picturebox_set_pos(f_heat->pic_temp[i], cx, HEAT_TEMP_Y);
        compo_picturebox_set_size(f_heat->pic_temp[i], w, digit_h);
        x += w;
        if (i + 1 < HEAT_TEMP_IDX_CNT) {
            x += func_heat_temp_digit_gap(d, digits[i + 1]);
        }
    }

    if (f_heat->pic_temp_degf != NULL) {
        u16 sym_w = white ? HEAT_WHX_W : HEAT_BHX_W;
        u16 sym_h = white ? HEAT_WHX_H : HEAT_BHX_H;
        s16 cx = x + HEAT_TEMP_SYMBOL_GAP + (s16)(sym_w / 2);

        compo_picturebox_set_pos(f_heat->pic_temp_degf, cx, HEAT_TEMP_Y);
        compo_picturebox_set_size(f_heat->pic_temp_degf, sym_w, sym_h);
        x += HEAT_TEMP_SYMBOL_GAP + sym_w;
    }

    if (f_heat->pic_temp_suffix != NULL) {
        if (white && HEAT_WSX_W > 0) {
            s16 cx = x + HEAT_TEMP_SYM_INNER_GAP + (s16)(HEAT_WSX_W / 2);

            compo_picturebox_set_pos(f_heat->pic_temp_suffix, cx, HEAT_TEMP_Y);
            compo_picturebox_set_size(f_heat->pic_temp_suffix, HEAT_WSX_W, HEAT_WSX_H);
            compo_picturebox_set_visible(f_heat->pic_temp_suffix, true);
        } else {
            compo_picturebox_set_visible(f_heat->pic_temp_suffix, false);
        }
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
        if (white && HEAT_WSX_W > 0) {
            os_spiflash_read(heat_temp_suffix_ram, UI_BUF_HOME_WSX_BIN, UI_LEN_HOME_WSX_BIN);
            if (gui_set_ram_check(heat_temp_suffix_ram, __func__)) {
                compo_picturebox_set_ram(f_heat->pic_temp_suffix, heat_temp_suffix_ram);
            }
        } else {
            compo_picturebox_set_visible(f_heat->pic_temp_suffix, false);
        }
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

static void func_heat_countdown_tick(void)
{
    if (heat_countdown_running && heat_countdown_remain_sec > 0) {
        heat_countdown_remain_sec--;
    }
}

static void func_heat_lock_check(f_heat_t *f_heat)
{
    if (f_heat->ui_state != HEAT_UI_HEATING || f_heat->screen_locked) {
        return;
    }
    if (tick_check_expire(f_heat->heat_start_tick, HEAT_LOCK_MS)) {
        f_heat->screen_locked = true;
        func_heat_lock_icon_apply(f_heat);
    }
}

static void func_heat_heating_progress(f_heat_t *f_heat)
{
    u16 target = func_heat_get_target_temp_f(f_heat);

    if (f_heat->heat_total_sec == 0) {
        f_heat->display_temp_f = target;
        return;
    }

    {
        u32 elapsed = f_heat->heat_total_sec - heat_countdown_remain_sec;

        f_heat->display_temp_f = (u16)((u32)target * elapsed / f_heat->heat_total_sec);
    }
}

static void func_heat_heating_finish_check(f_heat_t *f_heat)
{
    if (f_heat->ui_state != HEAT_UI_HEATING) {
        return;
    }
    if (heat_countdown_remain_sec == 0) {
        heat_countdown_running = false;
        f_heat->ui_state = HEAT_UI_FINISHED;
        f_heat->display_temp_f = func_heat_get_target_temp_f(f_heat);
        f_heat->screen_locked = false;
        func_heat_lock_icon_apply(f_heat);
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_heat_stop();
#endif
    }
}

static void func_heat_status_refresh(f_heat_t *f_heat)
{
    tm_t tm = rtc_clock_get();
    bool sec_changed = false;

    if (f_heat->last_top_min != tm.min || f_heat->last_top_sec != tm.sec) {
        f_heat->last_top_min = tm.min;
        f_heat->last_top_sec = tm.sec;
        home_top_time_refresh(&f_heat->top_time, &tm);
        sec_changed = true;
    }

    if (f_heat->ui_state == HEAT_UI_HEATING && sec_changed) {
        func_heat_countdown_tick();
        func_heat_heating_progress(f_heat);
        func_heat_heating_finish_check(f_heat);
        func_heat_display_refresh(f_heat);
    }

    func_heat_lock_check(f_heat);
}

static void func_heat_timer_layout_ex(f_heat_t *f_heat, u8 hour, u8 min, bool h_white, bool m_white)
{
    const u16 *htbl = h_white ? tbl_heat_w_digit_w : tbl_heat_b_digit_w;
    const u16 *mtbl = m_white ? tbl_heat_w_digit_w : tbl_heat_b_digit_w;
    u8 h_digits[2] = { hour / 10, hour % 10 };
    u8 m_digits[2] = { min / 10, min % 10 };
    u16 h_pair_gap = func_heat_timer_pair_gap_ex(htbl, h_digits[0], h_digits[1]);
    u16 m_pair_gap = func_heat_timer_pair_gap_ex(mtbl, m_digits[0], m_digits[1]);
    u16 total;
    s16 x;
    u8 i;

    total = func_heat_timer_total_w_ex(hour, min, h_white, m_white);
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2);

    for (i = 0; i < 2; i++) {
        u8 d = h_digits[i];
        u16 w = htbl[d];
        s16 cx = x + (s16)(w / 2);

        compo_picturebox_set_pos(f_heat->pic_timer[i], cx, HEAT_TIMER_Y);
        compo_picturebox_set_size(f_heat->pic_timer[i], w,
                                  h_white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H);
        x += w + ((i == 0) ? h_pair_gap : HEAT_TIMER_COLON_GAP);
    }

    {
        s16 cx = x + (s16)(HEAT_WBX_W / 2);

        compo_picturebox_set_pos(f_heat->pic_timer_colon, cx, HEAT_TIMER_Y);
        compo_picturebox_set_size(f_heat->pic_timer_colon, HEAT_WBX_W, HEAT_WBX_H);
        x += HEAT_WBX_W + HEAT_TIMER_COLON_GAP_AFTER;
    }

    for (i = 0; i < 2; i++) {
        u8 d = m_digits[i];
        u16 w = mtbl[d];
        s16 cx = x + (s16)(w / 2);

        compo_picturebox_set_pos(f_heat->pic_timer[i + 2], cx, HEAT_TIMER_Y);
        compo_picturebox_set_size(f_heat->pic_timer[i + 2], w,
                                  m_white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H);
        x += w;
        if (i == 0) {
            x += m_pair_gap;
        }
    }
}

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

static void func_heat_display_refresh(f_heat_t *f_heat)
{
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
    } else {
        hour = (u8)(heat_countdown_remain_sec / 3600);
        min = (u8)((heat_countdown_remain_sec % 3600) / 60);
        temp = f_heat->display_temp_f;
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
    f_heat->display_temp_f = 0;
    f_heat->last_timer_key = 0xffff;
    f_heat->last_temp_f = 0xffff;
    heat_countdown_remain_sec = f_heat->heat_total_sec;
    func_heat_countdown_start();

#if FUNC_LUNCHBOX_UART_EN
    printf("lb: heat_start -> UART\n");
    lunchbox_heat_start();
#endif

    func_heat_display_refresh(f_heat);
}

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
    if (f_heat->ui_state == HEAT_UI_HEATING) {
        func_heat_countdown_stop();
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_heat_stop();
#endif
        f_heat->screen_locked = false;
        func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        return;
    }

    if (f_heat->ui_state == HEAT_UI_FINISHED) {
        func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        return;
    }

    switch (f_heat->focus) {
    case HEAT_FOCUS_HOUR:
        func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
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
        if (f_heat->set_hour < 99) {
            f_heat->set_hour++;
        }
        func_heat_countdown_set(f_heat->set_hour, f_heat->set_min);
        break;

    case HEAT_FOCUS_MIN:
        if (f_heat->set_min < 59) {
            f_heat->set_min++;
        } else {
            f_heat->set_min = 0;
            if (f_heat->set_hour < 99) {
                f_heat->set_hour++;
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
        if (f_heat->set_hour > 0) {
            f_heat->set_hour--;
        }
        func_heat_countdown_set(f_heat->set_hour, f_heat->set_min);
        break;

    case HEAT_FOCUS_MIN:
        if (f_heat->set_min > 0) {
            f_heat->set_min--;
        } else if (f_heat->set_hour > 0) {
            f_heat->set_min = 59;
            f_heat->set_hour--;
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
    compo_form_t *frm = compo_form_create(true);
    compo_picturebox_t *pic;
    u8 i;

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

    for (i = 0; i < HEAT_TIMER_IDX_CNT; i++) {
        pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
        compo_setid(pic, tbl_heat_timer_id[i]);
        compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, HEAT_TIMER_Y);
        compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    }

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_COLON);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, HEAT_TIMER_Y);
    compo_picturebox_set_size(pic, HEAT_WBX_W, HEAT_WBX_H);

    for (i = 0; i < HEAT_TEMP_IDX_CNT; i++) {
        pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
        compo_setid(pic, tbl_heat_temp_id[i]);
        compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, HEAT_TEMP_Y);
        compo_picturebox_set_size(pic, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);
    }

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TEMPF);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, HEAT_TEMP_Y);
    compo_picturebox_set_size(pic, HEAT_BHX_W, HEAT_BHX_H);

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TEMP_S);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, HEAT_TEMP_Y);
    compo_picturebox_set_size(pic, HEAT_WSX_W > 0 ? HEAT_WSX_W : HEAT_BHX_W,
                              HEAT_WSX_W > 0 ? HEAT_WSX_H : HEAT_BHX_H);
    compo_picturebox_set_visible(pic, false);

    return frm;
}

static void func_heat_process(void)
{
    f_heat_t *f_heat = (f_heat_t *)func_cb.f_cb;

    if (f_heat != NULL) {
        func_heat_status_refresh(f_heat);
    }
    func_process();
}

static void func_heat_message(size_msg_t msg)
{
    f_heat_t *f_heat = (f_heat_t *)func_cb.f_cb;

    if (sys_cb.flag_swithing) {
        return;
    }

    if (f_heat != NULL && f_heat->screen_locked) {
        // 锁屏仅允许确认、开关、锁键
        if (msg != HEAT_MSG_POWER && msg != HEAT_MSG_OK && msg != KU_LEFT) {
            return;
        }
    }

    switch (msg) {
    case HEAT_MSG_OK:
        func_heat_ok_key(f_heat);
        break;

    case HEAT_MSG_PLUS:
        func_heat_value_inc(f_heat);
        break;

    case HEAT_MSG_MINUS:
        func_heat_value_dec(f_heat);
        break;

    case HEAT_MSG_POWER:
        func_heat_power_key(f_heat);
        break;

    case KU_LEFT:
        if (f_heat != NULL) {
            f_heat->screen_locked = !f_heat->screen_locked;
            func_heat_lock_icon_apply(f_heat);
        }
        break;

    case KU_MODE:
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_heat_enter(void)
{
    f_heat_t *f_heat;

    func_cb.f_cb = func_zalloc(sizeof(f_heat_t));
    func_cb.frm_main = func_heat_form_create();

    f_heat = (f_heat_t *)func_cb.f_cb;
    f_heat->focus = HEAT_FOCUS_HOUR;
    f_heat->ui_state = HEAT_UI_SETUP;
    f_heat->set_hour = 1;
    f_heat->set_min = 0;
    f_heat->temp_idx = 2;
    f_heat->display_temp_f = 0;
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

    func_heat_status_icons_apply(f_heat);
    func_heat_countdown_set(f_heat->set_hour, f_heat->set_min);
    func_heat_countdown_stop();
    func_heat_display_refresh(f_heat);
    func_heat_status_refresh(f_heat);
}

void func_heat_exit(void)
{
    func_heat_countdown_stop();
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_heat_stop();
#endif
    func_cb.last = FUNC_HEAT;
}

void func_heat(void)
{
    printf("%s\n", __func__);
    func_heat_enter();
    while (func_cb.sta == FUNC_HEAT) {
        func_heat_process();
        func_heat_message(msg_dequeue());
    }
    func_heat_exit();
}
