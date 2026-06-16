#include "include.h"
#include "func.h"
#include "func_reservation.h"
#include "home_icon_res.h"
#include "home_top_time.h"
#include "home_ui_ram.h"
#include "home_ui_shared.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * 预约页（320×240 横屏，单页完成全部流程）：
 *   图一（预约时间）：
 *     左上 RTC：0m..9m.bin + colonm.bin + AMm/PMm
 *     中部时间：0..9.bin + colon.bin
 *   图二（加热/温度）：w0x/b0x + whx/bhx + colonm.bin
 * PT8028：TCH4 确认 | TCH5 开关/返回 | TCH2/TCH6 减/加 | TCH0 锁键解锁童锁
 */
#define UI_RES_PLACEHOLDER                UI_BUF_ICON_ACTIVITY_BIN

#ifndef UI_BUF_HOME_0_BIN
#error "Missing 0.bin: run tools/gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_COLON_BIN
#error "Missing colon.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_0M_BIN
#error "Missing 0m.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_W0X_BIN
#error "Missing w0x.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_B0X_BIN
#error "Missing b0x.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BHX_BIN
#error "Missing bhx.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_COLONM_BIN
#error "Missing colonm.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BLUETOOTH_BIN
#error "Missing bluetooth.bin"
#endif

#ifndef UI_BUF_HOME_WHX_BIN
#define UI_BUF_HOME_WHX_BIN               UI_BUF_HOME_BHX_BIN
#define UI_LEN_HOME_WHX_BIN               UI_LEN_HOME_BHX_BIN
#define RES_WHX_W                         HEAT_BHX_W
#define RES_WHX_H                         HEAT_BHX_H
#else
#define RES_WHX_W                         HEAT_BHX_W
#define RES_WHX_H                         HEAT_BHX_H
#endif

#ifndef UI_BUF_HOME_WSX_BIN
#define UI_BUF_HOME_WSX_BIN               UI_BUF_HOME_BHX_BIN
#define UI_LEN_HOME_WSX_BIN               UI_LEN_HOME_BHX_BIN
#define RES_WSX_W                         0
#define RES_WSX_H                         0
#else
#define RES_WSX_W                         0
#define RES_WSX_H                         0
#endif

#ifndef UI_BUF_HOME_BSX_BIN
#define UI_BUF_HOME_BSX_BIN               UI_BUF_HOME_BHX_BIN
#define UI_LEN_HOME_BSX_BIN               UI_LEN_HOME_BHX_BIN
#define RES_BSX_W                         0
#define RES_BSX_H                         0
#else
#define RES_BSX_W                         0
#define RES_BSX_H                         0
#endif

#ifndef UI_BUF_HOME_AMM_BIN
#error "Missing AMm.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_PMM_BIN
#error "Missing PMm.bin: run gen_home_icons.py + prebuild.bat"
#endif

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define RES_STATUS_Y                      20
#define RES_STATUS_RIGHT_MARGIN           10
#define RES_STATUS_GAP                    6
#define RES_INFO_TEXT_X                   12
#define RES_INFO_TEXT_Y                   20
#define RES_INFO_TEXT_W                   180
#define RES_APPT_TIME_Y                   120
#define RES_HEAT_TIMER_Y                  100
#define RES_HEAT_TEMP_Y                   190
#define RES_APPT_DIGIT_W                  HOME_DIGIT_W
#define RES_APPT_DIGIT_H                  HOME_DIGIT_H
#define RES_APPT_COLON_W                  HOME_COLON_W
#define RES_APPT_COLON_H                  HOME_COLON_H
#define RES_APPT_GAP                      2
#else
#define RES_STATUS_Y                      48
#define RES_STATUS_RIGHT_MARGIN           24
#define RES_STATUS_GAP                    10
#define RES_INFO_TEXT_X                   78
#define RES_INFO_TEXT_Y                   48
#define RES_INFO_TEXT_W                   280
#define RES_APPT_TIME_Y                   195
#define RES_HEAT_TIMER_Y                  195
#define RES_HEAT_TEMP_Y                   305
#define RES_APPT_DIGIT_W                  HOME_DIGIT_W
#define RES_APPT_DIGIT_H                  HOME_DIGIT_H
#define RES_APPT_COLON_W                  HOME_COLON_W
#define RES_APPT_COLON_H                  HOME_COLON_H
#define RES_APPT_GAP                      4
#endif

#define RES_STATUS_BAT_X                  (GUI_SCREEN_WIDTH - RES_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define RES_STATUS_LOCK_X                 (RES_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - RES_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define RES_STATUS_BT_X                   (RES_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - RES_STATUS_GAP - HOME_STATUS_BT_W / 2)

#define RES_HEAT_PAIR_GAP                 10
#define RES_HEAT_PAIR_NARROW_EXTRA        8
#define RES_HEAT_COLON_GAP                10
#define RES_HEAT_TEMP_DIGIT_GAP           4
#define RES_HEAT_TEMP_SYMBOL_GAP          6
#define RES_HEAT_LOCK_MS                  30000
#define RES_TEMP_PRESET_CNT               5
#define RES_MSG_OK                        KU_BACK
#define RES_MSG_PLUS                      KU_VOL_UP
#define RES_MSG_MINUS                     KU_VOL_DOWN
#define RES_MSG_POWER                     (KEY_RIGHT | KEY_SHORT_UP)

enum {
    RES_UI_APPT_TIME = 0,
    RES_UI_HEAT_SETUP,
    RES_UI_HEATING,
    RES_UI_FINISHED,
};

enum {
    RES_FOCUS_APPT_HOUR = 0,
    RES_FOCUS_APPT_MIN,
    RES_FOCUS_HEAT_HOUR,
    RES_FOCUS_HEAT_MIN,
    RES_FOCUS_HEAT_TEMP,
};

enum {
    RES_TIMER_IDX_H10 = 0,
    RES_TIMER_IDX_H1,
    RES_TIMER_IDX_M10,
    RES_TIMER_IDX_M1,
    RES_TIMER_IDX_CNT,
};

enum {
    RES_TEMP_IDX_H = 0,
    RES_TEMP_IDX_T10,
    RES_TEMP_IDX_T1,
    RES_TEMP_IDX_CNT,
};

enum {
    COMPO_ID_PIC_TOP_H10 = 1,
    COMPO_ID_PIC_TOP_H1,
    COMPO_ID_PIC_TOP_COLON,
    COMPO_ID_PIC_TOP_M10,
    COMPO_ID_PIC_TOP_M1,
    COMPO_ID_PIC_TOP_AMPM,
    COMPO_ID_PIC_APPT_H10,
    COMPO_ID_PIC_APPT_H1,
    COMPO_ID_PIC_APPT_COLON,
    COMPO_ID_PIC_APPT_M10,
    COMPO_ID_PIC_APPT_M1,
    COMPO_ID_PIC_HEAT_H10,
    COMPO_ID_PIC_HEAT_H1,
    COMPO_ID_PIC_HEAT_COLON,
    COMPO_ID_PIC_HEAT_M10,
    COMPO_ID_PIC_HEAT_M1,
    COMPO_ID_PIC_TEMP_H,
    COMPO_ID_PIC_TEMP_T10,
    COMPO_ID_PIC_TEMP_T1,
    COMPO_ID_PIC_TEMP_SYM,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_LOCK,
    COMPO_ID_PIC_BAT,
    COMPO_ID_TXT_INFO,
};

typedef struct f_reservation_t_ {
    u8 ui;
    u8 focus;
    u8 appt_hour;
    u8 appt_min;
    u8 heat_hour;
    u8 heat_min;
    u8 temp_idx;
    u16 display_temp_f;
    u32 heat_total_sec;
    u32 heat_remain_sec;
    u32 heat_start_tick;
    bool screen_locked;
    bool heating_paused;
    u8 last_top_min;
    u8 last_top_sec;
    u16 last_appt_key;
    u16 last_heat_timer_key;
    u16 last_temp_f;
    bool last_h_white;
    bool last_m_white;
    bool last_t_white;
    u16 last_info_sec;
    home_top_time_ui_t top_time;
    compo_picturebox_t *pic_appt[RES_TIMER_IDX_CNT];
    compo_picturebox_t *pic_appt_colon;
    compo_picturebox_t *pic_heat[RES_TIMER_IDX_CNT];
    compo_picturebox_t *pic_heat_colon;
    compo_picturebox_t *pic_temp[RES_TEMP_IDX_CNT];
    compo_picturebox_t *pic_temp_sym;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
    compo_textbox_t *txt_info;
} f_reservation_t;

typedef struct reservation_global_t_ {
    bool setup_done;
    reservation_phase_t phase;
    u8 appt_hour;
    u8 appt_min;
    u8 heat_hour;
    u8 heat_min;
    u8 temp_idx;
    bool appt_triggered_today;
    u8 last_poll_min;
} reservation_global_t;

static reservation_global_t g_res;

static u8 res_heat_colon_ram[HOME_TOP_TIME_COLONM_RAM_SIZE];
static u8 res_heat_colon_w_ram[HEAT_WBX_RAM_SIZE];
static u8 res_temp_digit_ram[RES_TEMP_IDX_CNT][HEAT_B_DIGIT_RAM_MAX_SIZE];
static u8 res_temp_sym_ram[HEAT_BHX_RAM_SIZE];

static u16 func_res_gpu_pic_w(const u8 *ram)
{
    if (ram == NULL) {
        return 0;
    }
    return GET_LE16(&ram[4]);
}

static u16 func_res_gpu_pic_h(const u8 *ram)
{
    if (ram == NULL) {
        return 0;
    }
    return GET_LE16(&ram[6]);
}

static const u16 tbl_res_temp_preset[RES_TEMP_PRESET_CNT] = {
    140, 158, 176, 190, 212,
};

static const u32 tbl_res_appt_digit_addr[10] = {
    UI_BUF_HOME_0_BIN, UI_BUF_HOME_1_BIN, UI_BUF_HOME_2_BIN, UI_BUF_HOME_3_BIN,
    UI_BUF_HOME_4_BIN, UI_BUF_HOME_5_BIN, UI_BUF_HOME_6_BIN, UI_BUF_HOME_7_BIN,
    UI_BUF_HOME_8_BIN, UI_BUF_HOME_9_BIN,
};

static const u16 tbl_res_appt_digit_len[10] = {
    UI_LEN_HOME_0_BIN, UI_LEN_HOME_1_BIN, UI_LEN_HOME_2_BIN, UI_LEN_HOME_3_BIN,
    UI_LEN_HOME_4_BIN, UI_LEN_HOME_5_BIN, UI_LEN_HOME_6_BIN, UI_LEN_HOME_7_BIN,
    UI_LEN_HOME_8_BIN, UI_LEN_HOME_9_BIN,
};

static const u32 tbl_res_w_digit_addr[10] = {
    UI_BUF_HOME_W0X_BIN, UI_BUF_HOME_W1X_BIN, UI_BUF_HOME_W2X_BIN, UI_BUF_HOME_W3X_BIN,
    UI_BUF_HOME_W4X_BIN, UI_BUF_HOME_W5X_BIN, UI_BUF_HOME_W6X_BIN, UI_BUF_HOME_W7X_BIN,
    UI_BUF_HOME_W8X_BIN, UI_BUF_HOME_W9X_BIN,
};

static const u32 tbl_res_b_digit_addr[10] = {
    UI_BUF_HOME_B0X_BIN, UI_BUF_HOME_B1X_BIN, UI_BUF_HOME_B2X_BIN, UI_BUF_HOME_B3X_BIN,
    UI_BUF_HOME_B4X_BIN, UI_BUF_HOME_B5X_BIN, UI_BUF_HOME_B6X_BIN, UI_BUF_HOME_B7X_BIN,
    UI_BUF_HOME_B8X_BIN, UI_BUF_HOME_B9X_BIN,
};

static const u16 tbl_res_w_digit_len[10] = {
    UI_LEN_HOME_W0X_BIN, UI_LEN_HOME_W1X_BIN, UI_LEN_HOME_W2X_BIN, UI_LEN_HOME_W3X_BIN,
    UI_LEN_HOME_W4X_BIN, UI_LEN_HOME_W5X_BIN, UI_LEN_HOME_W6X_BIN, UI_LEN_HOME_W7X_BIN,
    UI_LEN_HOME_W8X_BIN, UI_LEN_HOME_W9X_BIN,
};

static const u16 tbl_res_b_digit_len[10] = {
    UI_LEN_HOME_B0X_BIN, UI_LEN_HOME_B1X_BIN, UI_LEN_HOME_B2X_BIN, UI_LEN_HOME_B3X_BIN,
    UI_LEN_HOME_B4X_BIN, UI_LEN_HOME_B5X_BIN, UI_LEN_HOME_B6X_BIN, UI_LEN_HOME_B7X_BIN,
    UI_LEN_HOME_B8X_BIN, UI_LEN_HOME_B9X_BIN,
};

static const u16 tbl_res_w_digit_w[10] = {
    HEAT_W0X_W, HEAT_W1X_W, HEAT_W2X_W, HEAT_W3X_W, HEAT_W4X_W,
    HEAT_W5X_W, HEAT_W6X_W, HEAT_W7X_W, HEAT_W8X_W, HEAT_W9X_W,
};

static const u16 tbl_res_b_digit_w[10] = {
    HEAT_B0X_W, HEAT_B1X_W, HEAT_B2X_W, HEAT_B3X_W, HEAT_B4X_W,
    HEAT_B5X_W, HEAT_B6X_W, HEAT_B7X_W, HEAT_B8X_W, HEAT_B9X_W,
};

static const u16 tbl_res_heat_timer_id[RES_TIMER_IDX_CNT] = {
    COMPO_ID_PIC_HEAT_H10, COMPO_ID_PIC_HEAT_H1,
    COMPO_ID_PIC_HEAT_M10, COMPO_ID_PIC_HEAT_M1,
};

static const u16 tbl_res_appt_timer_id[RES_TIMER_IDX_CNT] = {
    COMPO_ID_PIC_APPT_H10, COMPO_ID_PIC_APPT_H1,
    COMPO_ID_PIC_APPT_M10, COMPO_ID_PIC_APPT_M1,
};

static const u16 tbl_res_temp_id[RES_TEMP_IDX_CNT] = {
    COMPO_ID_PIC_TEMP_H, COMPO_ID_PIC_TEMP_T10, COMPO_ID_PIC_TEMP_T1,
};

/* 禁止用 ICON_ACTIVITY 占位图创建（会显示圆环），用 0 创建后仅 set_ram 显示 bin */
static compo_picturebox_t *func_res_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = compo_picturebox_create(frm, 0);

    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

static void func_res_hide_appt_ui(f_reservation_t *f_res)
{
    u8 i;

    if (f_res == NULL) {
        return;
    }

    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        if (f_res->pic_appt[i] != NULL) {
            compo_picturebox_set_visible(f_res->pic_appt[i], false);
        }
    }
    if (f_res->pic_appt_colon != NULL) {
        compo_picturebox_set_visible(f_res->pic_appt_colon, false);
    }
}

static void func_res_appt_bind_digits(f_reservation_t *f_res)
{
    u8 i;

    if (f_res == NULL) {
        return;
    }

    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        f_res->pic_appt[i] = compo_getobj_byid(tbl_res_appt_timer_id[i]);
    }
    f_res->pic_appt_colon = compo_getobj_byid(COMPO_ID_PIC_APPT_COLON);
}

static void func_res_hide_heat_temp_ui(f_reservation_t *f_res)
{
    u8 i;

    if (f_res == NULL) {
        return;
    }

    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        if (f_res->pic_heat[i] != NULL) {
            compo_picturebox_set_visible(f_res->pic_heat[i], false);
        }
    }
    if (f_res->pic_heat_colon != NULL) {
        compo_picturebox_set_visible(f_res->pic_heat_colon, false);
    }
    for (i = 0; i < RES_TEMP_IDX_CNT; i++) {
        if (f_res->pic_temp[i] != NULL) {
            compo_picturebox_set_visible(f_res->pic_temp[i], false);
        }
    }
    if (f_res->pic_temp_sym != NULL) {
        compo_picturebox_set_visible(f_res->pic_temp_sym, false);
    }
}

static bool func_res_gpu_ram(u8 *ram, u16 buf_size, u32 addr, u16 len, compo_picturebox_t *pic)
{
    u16 need;

    if (ram == NULL || len == 0 || len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    os_spiflash_read(ram, addr, len);
    if (!gui_set_ram_check(ram, __func__)) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    need = (u16)(8 + (u32)GET_LE16(&ram[4]) * GET_LE16(&ram[6]) * 2);
    if (need > len || need > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    if (pic != NULL) {
        compo_picturebox_set_ram(pic, ram);
        compo_picturebox_set_visible(pic, true);
    }
    return true;
}

reservation_phase_t func_reservation_get_phase(void)
{
    return g_res.phase;
}

bool func_reservation_is_waiting(void)
{
    return g_res.setup_done && (g_res.phase == RES_PHASE_WAITING);
}

static u16 func_res_get_target_temp_f(u8 temp_idx)
{
    if (temp_idx >= RES_TEMP_PRESET_CNT) {
        return tbl_res_temp_preset[0];
    }
    return tbl_res_temp_preset[temp_idx];
}

static u32 func_res_seconds_until_appt(void)
{
    tm_t tm = rtc_clock_get();
    u32 now = (u32)tm.hour * 3600 + (u32)tm.min * 60 + (u32)tm.sec;
    u32 appt = (u32)g_res.appt_hour * 3600 + (u32)g_res.appt_min * 60;

    if (appt <= now) {
        appt += 24 * 3600;
    }
    return appt - now;
}

void func_reservation_marquee_text(char *buf, u16 buf_len)
{
    u32 sec;
    u32 hours;
    u32 mins;

    if (buf == NULL || buf_len == 0) {
        return;
    }

    if (!func_reservation_is_waiting()) {
        buf[0] = '\0';
        return;
    }

    sec = func_res_seconds_until_appt();
    hours = sec / 3600;
    mins = (sec % 3600) / 60;

    if (hours > 0 && mins > 0) {
        snprintf(buf, buf_len, "Starts in %lu hours %lu min", (unsigned long)hours, (unsigned long)mins);
    } else if (hours > 0) {
        snprintf(buf, buf_len, "Starts in %lu hour%s", (unsigned long)hours, (hours > 1) ? "s" : "");
    } else if (mins > 0) {
        snprintf(buf, buf_len, "Starts in %lu min", (unsigned long)mins);
    } else {
        snprintf(buf, buf_len, "Starts soon");
    }
}

static void func_res_info_text_update(f_reservation_t *f_res)
{
    char buf[48];

    if (f_res->txt_info == NULL) {
        return;
    }

    if (f_res->ui != RES_UI_HEAT_SETUP) {
        compo_textbox_set_visible(f_res->txt_info, false);
        return;
    }

    func_reservation_marquee_text(buf, sizeof(buf));
    compo_textbox_set(f_res->txt_info, buf);
    compo_textbox_set_visible(f_res->txt_info, true);
}

static void func_res_status_icons_apply(f_reservation_t *f_res)
{
    home_ui_shared_status_init();

    if (f_res->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_pos(f_res->pic_bt, RES_STATUS_BT_X, RES_STATUS_Y);
        compo_picturebox_set_ram(f_res->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f_res->pic_bt, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
        compo_picturebox_set_visible(f_res->pic_bt, true);
    }
    if (f_res->pic_bat != NULL && gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        compo_picturebox_set_pos(f_res->pic_bat, RES_STATUS_BAT_X, RES_STATUS_Y);
        compo_picturebox_set_ram(f_res->pic_bat, home_ui_shared_status_bat_ram);
        compo_picturebox_set_size(f_res->pic_bat, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
        compo_picturebox_set_visible(f_res->pic_bat, true);
    }
}

static void func_res_lock_icon_apply(f_reservation_t *f_res)
{
    if (f_res->pic_lock == NULL) {
        return;
    }

    if ((f_res->ui == RES_UI_HEATING) && f_res->screen_locked
        && gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
        compo_picturebox_set_pos(f_res->pic_lock, RES_STATUS_LOCK_X, RES_STATUS_Y);
        compo_picturebox_set_ram(f_res->pic_lock, home_ui_shared_status_lock_ram);
        compo_picturebox_set_size(f_res->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
        compo_picturebox_set_visible(f_res->pic_lock, true);
    } else {
        compo_picturebox_set_visible(f_res->pic_lock, false);
    }
}

static void func_res_top_time_hide(home_top_time_ui_t *ui)
{
    if (ui == NULL) {
        return;
    }
    if (ui->pic_h10 != NULL) {
        compo_picturebox_set_visible(ui->pic_h10, false);
    }
    if (ui->pic_h1 != NULL) {
        compo_picturebox_set_visible(ui->pic_h1, false);
    }
    if (ui->pic_colon != NULL) {
        compo_picturebox_set_visible(ui->pic_colon, false);
    }
    if (ui->pic_m10 != NULL) {
        compo_picturebox_set_visible(ui->pic_m10, false);
    }
    if (ui->pic_m1 != NULL) {
        compo_picturebox_set_visible(ui->pic_m1, false);
    }
    if (ui->pic_ampm != NULL) {
        compo_picturebox_set_visible(ui->pic_ampm, false);
    }
}

static void func_res_top_time_refresh(f_reservation_t *f_res, tm_t *tm)
{
    if (f_res == NULL || tm == NULL) {
        return;
    }

    if (f_res->ui != RES_UI_APPT_TIME) {
        func_res_top_time_hide(&f_res->top_time);
        f_res->top_time.last_key = 0;
        return;
    }

    home_top_time_refresh(&f_res->top_time, tm);
}

static void func_res_appt_layout(f_reservation_t *f_res, u8 hour, u8 min)
{
    u16 total;
    s16 x;
    u8 i;

    (void)hour;
    (void)min;

    total = RES_APPT_DIGIT_W + RES_APPT_GAP + RES_APPT_DIGIT_W + RES_APPT_GAP
          + RES_APPT_COLON_W + RES_APPT_GAP + RES_APPT_DIGIT_W + RES_APPT_GAP
          + RES_APPT_DIGIT_W;
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2);

    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        s16 cx = x + (s16)(RES_APPT_DIGIT_W / 2);

        compo_picturebox_set_pos(f_res->pic_appt[i], cx, RES_APPT_TIME_Y);
        compo_picturebox_set_size(f_res->pic_appt[i], RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);
        x += RES_APPT_DIGIT_W + RES_APPT_GAP;
        if (i == RES_TIMER_IDX_H1) {
            cx = x + (s16)(RES_APPT_COLON_W / 2);
            compo_picturebox_set_pos(f_res->pic_appt_colon, cx, RES_APPT_TIME_Y);
            compo_picturebox_set_size(f_res->pic_appt_colon, RES_APPT_COLON_W, RES_APPT_COLON_H);
            x += RES_APPT_COLON_W + RES_APPT_GAP;
        }
    }
}

static void func_res_appt_update(f_reservation_t *f_res)
{
    u8 digits[RES_TIMER_IDX_CNT];
    u16 key;
    u8 i;

    if (f_res == NULL || f_res->ui != RES_UI_APPT_TIME) {
        return;
    }

    digits[RES_TIMER_IDX_H10] = f_res->appt_hour / 10;
    digits[RES_TIMER_IDX_H1] = f_res->appt_hour % 10;
    digits[RES_TIMER_IDX_M10] = f_res->appt_min / 10;
    digits[RES_TIMER_IDX_M1] = f_res->appt_min % 10;
    key = (u16)f_res->appt_hour * 100 + f_res->appt_min;

    /* 中部大号时间冒号：colon.bin（与 func_home / func_mode 相同） */
    os_spiflash_read(home_ui_colon_ram, UI_BUF_HOME_COLON_BIN, UI_LEN_HOME_COLON_BIN);
    if (f_res->pic_appt_colon != NULL && gui_set_ram_check(home_ui_colon_ram, __func__)) {
        compo_picturebox_set_ram(f_res->pic_appt_colon, home_ui_colon_ram);
        compo_picturebox_set_visible(f_res->pic_appt_colon, true);
    } else if (f_res->pic_appt_colon != NULL) {
        compo_picturebox_set_visible(f_res->pic_appt_colon, false);
    }

    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];

        os_spiflash_read(home_ui_digit_ram[i], tbl_res_appt_digit_addr[d], tbl_res_appt_digit_len[d]);
        if (gui_set_ram_check(home_ui_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_res->pic_appt[i], home_ui_digit_ram[i]);
            compo_picturebox_set_visible(f_res->pic_appt[i], true);
        } else if (f_res->pic_appt[i] != NULL) {
            compo_picturebox_set_visible(f_res->pic_appt[i], false);
        }
    }

    if (f_res->last_appt_key != key) {
        f_res->last_appt_key = key;
    }

    func_res_appt_layout(f_res, f_res->appt_hour, f_res->appt_min);
}

static u16 func_res_heat_pair_gap(const u16 *tbl_w, u8 d0, u8 d1)
{
    u16 gap = RES_HEAT_PAIR_GAP;

    if (d0 == 1 || d1 == 1 || tbl_w[d0] <= HEAT_W1X_W || tbl_w[d1] <= HEAT_W1X_W) {
        gap += RES_HEAT_PAIR_NARROW_EXTRA;
    }
    return gap;
}

static u16 func_res_heat_timer_total_w(u8 hour, u8 min, bool h_white, bool m_white)
{
    const u16 *htbl = h_white ? tbl_res_w_digit_w : tbl_res_b_digit_w;
    const u16 *mtbl = m_white ? tbl_res_w_digit_w : tbl_res_b_digit_w;
    u8 hd[2] = { hour / 10, hour % 10 };
    u8 md[2] = { min / 10, min % 10 };

    return htbl[hd[0]] + func_res_heat_pair_gap(htbl, hd[0], hd[1]) + htbl[hd[1]]
         + RES_HEAT_COLON_GAP + HOME_TOP_TIME_COLONM_W + RES_HEAT_COLON_GAP
         + mtbl[md[0]] + func_res_heat_pair_gap(mtbl, md[0], md[1]) + mtbl[md[1]];
}

static void func_res_heat_timer_layout(f_reservation_t *f_res, u8 hour, u8 min,
                                       bool h_white, bool m_white)
{
    const u16 *htbl = h_white ? tbl_res_w_digit_w : tbl_res_b_digit_w;
    const u16 *mtbl = m_white ? tbl_res_w_digit_w : tbl_res_b_digit_w;
    u8 hd[2] = { hour / 10, hour % 10 };
    u8 md[2] = { min / 10, min % 10 };
    u16 total;
    s16 x;
    u8 i;

    total = func_res_heat_timer_total_w(hour, min, h_white, m_white);
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2);

    for (i = 0; i < 2; i++) {
        u8 d = hd[i];
        u16 w = htbl[d];

        compo_picturebox_set_pos(f_res->pic_heat[i], x + (s16)(w / 2), RES_HEAT_TIMER_Y);
        compo_picturebox_set_size(f_res->pic_heat[i], w,
                                  h_white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H);
        x += w + ((i == 0) ? func_res_heat_pair_gap(htbl, hd[0], hd[1]) : RES_HEAT_COLON_GAP);
    }

    compo_picturebox_set_pos(f_res->pic_heat_colon, x + (s16)(HOME_TOP_TIME_COLONM_W / 2), RES_HEAT_TIMER_Y);
    compo_picturebox_set_size(f_res->pic_heat_colon, HOME_TOP_TIME_COLONM_W, HOME_TOP_TIME_COLONM_H);
    x += HOME_TOP_TIME_COLONM_W + RES_HEAT_COLON_GAP;

    for (i = 0; i < 2; i++) {
        u8 d = md[i];
        u16 w = mtbl[d];

        compo_picturebox_set_pos(f_res->pic_heat[i + 2], x + (s16)(w / 2), RES_HEAT_TIMER_Y);
        compo_picturebox_set_size(f_res->pic_heat[i + 2], w,
                                  m_white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H);
        x += w;
        if (i == 0) {
            x += func_res_heat_pair_gap(mtbl, md[0], md[1]);
        }
    }
}

static void func_res_heat_timer_update(f_reservation_t *f_res, u8 hour, u8 min,
                                       bool h_white, bool m_white, bool use_w_colon)
{
    u8 digits[RES_TIMER_IDX_CNT];
    u16 key;
    u8 i;

    if (f_res->ui == RES_UI_APPT_TIME) {
        for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
            if (f_res->pic_heat[i] != NULL) {
                compo_picturebox_set_visible(f_res->pic_heat[i], false);
            }
        }
        if (f_res->pic_heat_colon != NULL) {
            compo_picturebox_set_visible(f_res->pic_heat_colon, false);
        }
        return;
    }

    digits[RES_TIMER_IDX_H10] = hour / 10;
    digits[RES_TIMER_IDX_H1] = hour % 10;
    digits[RES_TIMER_IDX_M10] = min / 10;
    digits[RES_TIMER_IDX_M1] = min % 10;
    key = (u16)hour * 100 + min;
    if (f_res->last_heat_timer_key == key
        && f_res->last_h_white == h_white && f_res->last_m_white == m_white) {
        return;
    }
    f_res->last_heat_timer_key = key;
    f_res->last_h_white = h_white;
    f_res->last_m_white = m_white;

    if (use_w_colon) {
#ifndef UI_BUF_HOME_WBX_BIN
        func_res_gpu_ram(res_heat_colon_w_ram, sizeof(res_heat_colon_w_ram),
                         UI_BUF_HOME_COLONM_BIN, UI_LEN_HOME_COLONM_BIN, f_res->pic_heat_colon);
#else
        func_res_gpu_ram(res_heat_colon_w_ram, sizeof(res_heat_colon_w_ram),
                         UI_BUF_HOME_WBX_BIN, UI_LEN_HOME_WBX_BIN, f_res->pic_heat_colon);
#endif
    } else {
        func_res_gpu_ram(res_heat_colon_ram, sizeof(res_heat_colon_ram),
                         UI_BUF_HOME_COLONM_BIN, UI_LEN_HOME_COLONM_BIN, f_res->pic_heat_colon);
    }

    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];
        bool white = (i <= RES_TIMER_IDX_H1) ? h_white : m_white;
        u32 addr = white ? tbl_res_w_digit_addr[d] : tbl_res_b_digit_addr[d];
        u16 len = white ? tbl_res_w_digit_len[d] : tbl_res_b_digit_len[d];

        os_spiflash_read(home_ui_digit_ram[i], addr, len);
        if (gui_set_ram_check(home_ui_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_res->pic_heat[i], home_ui_digit_ram[i]);
            compo_picturebox_set_visible(f_res->pic_heat[i], true);
        }
    }

    func_res_heat_timer_layout(f_res, hour, min, h_white, m_white);
}

static u16 func_res_temp_total_w(u8 digits[RES_TEMP_IDX_CNT], bool white)
{
    const u16 *tbl = white ? tbl_res_w_digit_w : tbl_res_b_digit_w;
    u16 total = 0;
    u8 i;

    for (i = 0; i < RES_TEMP_IDX_CNT; i++) {
        total += tbl[digits[i]];
        if (i + 1 < RES_TEMP_IDX_CNT) {
            total += RES_HEAT_TEMP_DIGIT_GAP;
        }
    }
    total += RES_HEAT_TEMP_SYMBOL_GAP + HEAT_BHX_W;
    if (white && RES_WSX_W > 0) {
        total += RES_WSX_W;
    }
    return total;
}

static void func_res_temp_layout(f_reservation_t *f_res, u8 digits[RES_TEMP_IDX_CNT], bool white)
{
    const u16 *tbl = white ? tbl_res_w_digit_w : tbl_res_b_digit_w;
    u16 total;
    s16 x;
    u8 i;

    total = func_res_temp_total_w(digits, white);
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2);

    for (i = 0; i < RES_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];
        u16 w = tbl[d];

        compo_picturebox_set_pos(f_res->pic_temp[i], x + (s16)(w / 2), RES_HEAT_TEMP_Y);
        compo_picturebox_set_size(f_res->pic_temp[i], w,
                                  white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H);
        x += w + RES_HEAT_TEMP_DIGIT_GAP;
    }

    if (f_res->pic_temp_sym != NULL) {
        compo_picturebox_set_pos(f_res->pic_temp_sym, x + (s16)(HEAT_BHX_W / 2), RES_HEAT_TEMP_Y);
        compo_picturebox_set_size(f_res->pic_temp_sym, HEAT_BHX_W, HEAT_BHX_H);
    }
}

static void func_res_temp_update(f_reservation_t *f_res, u16 temp_f, bool white)
{
    u8 digits[RES_TEMP_IDX_CNT];
    u32 sym_addr;
    u16 sym_len;
    u8 i;

    if (f_res->ui == RES_UI_APPT_TIME) {
        for (i = 0; i < RES_TEMP_IDX_CNT; i++) {
            if (f_res->pic_temp[i] != NULL) {
                compo_picturebox_set_visible(f_res->pic_temp[i], false);
            }
        }
        if (f_res->pic_temp_sym != NULL) {
            compo_picturebox_set_visible(f_res->pic_temp_sym, false);
        }
        return;
    }

    if (temp_f > 999) {
        temp_f = 999;
    }
    if (f_res->last_temp_f == temp_f && f_res->last_t_white == white) {
        return;
    }
    f_res->last_temp_f = temp_f;
    f_res->last_t_white = white;

    digits[RES_TEMP_IDX_H] = (u8)(temp_f / 100);
    digits[RES_TEMP_IDX_T10] = (u8)((temp_f / 10) % 10);
    digits[RES_TEMP_IDX_T1] = (u8)(temp_f % 10);

    sym_addr = white ? UI_BUF_HOME_WHX_BIN : UI_BUF_HOME_BHX_BIN;
    sym_len = white ? UI_LEN_HOME_WHX_BIN : UI_LEN_HOME_BHX_BIN;
    func_res_gpu_ram(res_temp_sym_ram, sizeof(res_temp_sym_ram), sym_addr, sym_len, f_res->pic_temp_sym);

    for (i = 0; i < RES_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];
        u32 addr = white ? tbl_res_w_digit_addr[d] : tbl_res_b_digit_addr[d];
        u16 len = white ? tbl_res_w_digit_len[d] : tbl_res_b_digit_len[d];

        os_spiflash_read(res_temp_digit_ram[i], addr, len);
        if (gui_set_ram_check(res_temp_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_res->pic_temp[i], res_temp_digit_ram[i]);
            compo_picturebox_set_visible(f_res->pic_temp[i], true);
        }
    }

    func_res_temp_layout(f_res, digits, white);
}

static void func_res_display_refresh(f_reservation_t *f_res)
{
    u8 hour;
    u8 min;
    u16 temp;
    bool h_white;
    bool m_white;
    bool t_white;
    bool w_colon;

    if (f_res->ui == RES_UI_APPT_TIME) {
        tm_t tm = rtc_clock_get();

        func_res_hide_heat_temp_ui(f_res);
        f_res->top_time.last_key = 0;
        func_res_top_time_refresh(f_res, &tm);
        func_res_appt_update(f_res);
        func_res_info_text_update(f_res);
        func_res_lock_icon_apply(f_res);
        return;
    }

    func_res_hide_appt_ui(f_res);

    if (f_res->ui == RES_UI_HEAT_SETUP) {
        hour = f_res->heat_hour;
        min = f_res->heat_min;
        temp = func_res_get_target_temp_f(f_res->temp_idx);
        h_white = (f_res->focus == RES_FOCUS_HEAT_HOUR);
        m_white = (f_res->focus == RES_FOCUS_HEAT_MIN);
        t_white = (f_res->focus == RES_FOCUS_HEAT_TEMP);
        w_colon = false;
    } else {
        hour = (u8)(f_res->heat_remain_sec / 3600);
        min = (u8)((f_res->heat_remain_sec % 3600) / 60);
        temp = f_res->display_temp_f;
        h_white = true;
        m_white = true;
        t_white = true;
        w_colon = true;
    }

    if (hour > 99) {
        hour = 99;
    }

    func_res_heat_timer_update(f_res, hour, min, h_white, m_white, w_colon);
    func_res_temp_update(f_res, temp, t_white);
    func_res_info_text_update(f_res);
    func_res_lock_icon_apply(f_res);
}

static void func_res_start_heating(f_reservation_t *f_res)
{
    f_res->ui = RES_UI_HEATING;
    f_res->screen_locked = false;
    f_res->heating_paused = false;
    f_res->heat_start_tick = tick_get();
    f_res->heat_total_sec = (u32)f_res->heat_hour * 3600 + (u32)f_res->heat_min * 60;
    if (f_res->heat_total_sec == 0) {
        f_res->heat_total_sec = 60;
    }
    f_res->heat_remain_sec = f_res->heat_total_sec;
    f_res->display_temp_f = 0;
    f_res->last_heat_timer_key = 0xffff;
    f_res->last_temp_f = 0xffff;
    g_res.phase = RES_PHASE_HEATING;
    func_res_display_refresh(f_res);
}

void func_reservation_force_heating_enter(void)
{
    f_reservation_t *f_res = (f_reservation_t *)func_cb.f_cb;

    g_res.phase = RES_PHASE_HEATING;
    if (f_res != NULL && func_cb.sta == FUNC_RESERVATION) {
        f_res->heat_hour = g_res.heat_hour;
        f_res->heat_min = g_res.heat_min;
        f_res->temp_idx = g_res.temp_idx;
        func_res_start_heating(f_res);
    }
}

static void func_res_save_and_go_home(f_reservation_t *f_res)
{
    g_res.setup_done = true;
    g_res.phase = RES_PHASE_WAITING;
    g_res.appt_hour = f_res->appt_hour;
    g_res.appt_min = f_res->appt_min;
    g_res.heat_hour = f_res->heat_hour;
    g_res.heat_min = f_res->heat_min;
    g_res.temp_idx = f_res->temp_idx;
    g_res.appt_triggered_today = false;
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

static void func_res_ok_key(f_reservation_t *f_res)
{
    switch (f_res->ui) {
    case RES_UI_APPT_TIME:
        if (f_res->focus == RES_FOCUS_APPT_HOUR) {
            f_res->focus = RES_FOCUS_APPT_MIN;
        } else {
            f_res->ui = RES_UI_HEAT_SETUP;
            f_res->focus = RES_FOCUS_HEAT_HOUR;
            f_res->last_appt_key = 0xffff;
            f_res->last_heat_timer_key = 0xffff;
            f_res->last_temp_f = 0xffff;
        }
        break;

    case RES_UI_HEAT_SETUP:
        switch (f_res->focus) {
        case RES_FOCUS_HEAT_HOUR:
            f_res->focus = RES_FOCUS_HEAT_MIN;
            break;
        case RES_FOCUS_HEAT_MIN:
            f_res->focus = RES_FOCUS_HEAT_TEMP;
            break;
        case RES_FOCUS_HEAT_TEMP:
            func_res_save_and_go_home(f_res);
            return;
        default:
            break;
        }
        f_res->last_heat_timer_key = 0xffff;
        f_res->last_temp_f = 0xffff;
        break;

    case RES_UI_FINISHED:
        func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    default:
        break;
    }

    func_res_display_refresh(f_res);
}

static void func_res_power_key(f_reservation_t *f_res)
{
    if (f_res->ui == RES_UI_HEATING) {
        if (f_res->screen_locked) {
            f_res->heating_paused = !f_res->heating_paused;
        } else {
            func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
        return;
    }

    if (f_res->ui == RES_UI_FINISHED) {
        g_res.phase = RES_PHASE_NONE;
        g_res.setup_done = false;
        func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        return;
    }

    switch (f_res->ui) {
    case RES_UI_APPT_TIME:
        if (f_res->focus == RES_FOCUS_APPT_MIN) {
            f_res->focus = RES_FOCUS_APPT_HOUR;
            f_res->last_appt_key = 0xffff;
        } else {
            func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
        break;

    case RES_UI_HEAT_SETUP:
        switch (f_res->focus) {
        case RES_FOCUS_HEAT_HOUR:
            f_res->ui = RES_UI_APPT_TIME;
            f_res->focus = RES_FOCUS_APPT_MIN;
            f_res->last_appt_key = 0xffff;
            f_res->last_heat_timer_key = 0xffff;
            break;
        case RES_FOCUS_HEAT_MIN:
            f_res->focus = RES_FOCUS_HEAT_HOUR;
            f_res->last_heat_timer_key = 0xffff;
            break;
        case RES_FOCUS_HEAT_TEMP:
            f_res->focus = RES_FOCUS_HEAT_MIN;
            f_res->last_heat_timer_key = 0xffff;
            f_res->last_temp_f = 0xffff;
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }

    func_res_display_refresh(f_res);
}

static void func_res_value_inc(f_reservation_t *f_res)
{
    if (f_res->ui == RES_UI_HEATING || f_res->ui == RES_UI_FINISHED) {
        return;
    }

    switch (f_res->focus) {
    case RES_FOCUS_APPT_HOUR:
        if (f_res->appt_hour < 23) {
            f_res->appt_hour++;
        } else {
            f_res->appt_hour = 0;
        }
        f_res->last_appt_key = 0xffff;
        break;

    case RES_FOCUS_APPT_MIN:
        if (f_res->appt_min < 59) {
            f_res->appt_min++;
        } else {
            f_res->appt_min = 0;
            if (f_res->appt_hour < 23) {
                f_res->appt_hour++;
            } else {
                f_res->appt_hour = 0;
            }
        }
        f_res->last_appt_key = 0xffff;
        break;

    case RES_FOCUS_HEAT_HOUR:
        if (f_res->heat_hour < 99) {
            f_res->heat_hour++;
        }
        f_res->last_heat_timer_key = 0xffff;
        break;

    case RES_FOCUS_HEAT_MIN:
        if (f_res->heat_min < 59) {
            f_res->heat_min++;
        } else {
            f_res->heat_min = 0;
            if (f_res->heat_hour < 99) {
                f_res->heat_hour++;
            }
        }
        f_res->last_heat_timer_key = 0xffff;
        break;

    case RES_FOCUS_HEAT_TEMP:
        if (f_res->temp_idx + 1 < RES_TEMP_PRESET_CNT) {
            f_res->temp_idx++;
        }
        f_res->last_temp_f = 0xffff;
        break;

    default:
        break;
    }

    func_res_display_refresh(f_res);
}

static void func_res_value_dec(f_reservation_t *f_res)
{
    if (f_res->ui == RES_UI_HEATING || f_res->ui == RES_UI_FINISHED) {
        return;
    }

    switch (f_res->focus) {
    case RES_FOCUS_APPT_HOUR:
        if (f_res->appt_hour > 0) {
            f_res->appt_hour--;
        } else {
            f_res->appt_hour = 23;
        }
        f_res->last_appt_key = 0xffff;
        break;

    case RES_FOCUS_APPT_MIN:
        if (f_res->appt_min > 0) {
            f_res->appt_min--;
        } else {
            f_res->appt_min = 59;
            if (f_res->appt_hour > 0) {
                f_res->appt_hour--;
            } else {
                f_res->appt_hour = 23;
            }
        }
        f_res->last_appt_key = 0xffff;
        break;

    case RES_FOCUS_HEAT_HOUR:
        if (f_res->heat_hour > 0) {
            f_res->heat_hour--;
        }
        f_res->last_heat_timer_key = 0xffff;
        break;

    case RES_FOCUS_HEAT_MIN:
        if (f_res->heat_min > 0) {
            f_res->heat_min--;
        } else if (f_res->heat_hour > 0) {
            f_res->heat_min = 59;
            f_res->heat_hour--;
        }
        f_res->last_heat_timer_key = 0xffff;
        break;

    case RES_FOCUS_HEAT_TEMP:
        if (f_res->temp_idx > 0) {
            f_res->temp_idx--;
        }
        f_res->last_temp_f = 0xffff;
        break;

    default:
        break;
    }

    func_res_display_refresh(f_res);
}

static void func_res_heating_tick(f_reservation_t *f_res)
{
    u16 target;

    if (f_res->ui != RES_UI_HEATING || f_res->heating_paused) {
        return;
    }

    if (f_res->heat_remain_sec > 0) {
        f_res->heat_remain_sec--;
    }

    target = func_res_get_target_temp_f(f_res->temp_idx);
    if (f_res->heat_total_sec > 0) {
        u32 elapsed = f_res->heat_total_sec - f_res->heat_remain_sec;

        f_res->display_temp_f = (u16)((u32)target * elapsed / f_res->heat_total_sec);
    } else {
        f_res->display_temp_f = target;
    }

    if (f_res->heat_remain_sec == 0) {
        f_res->ui = RES_UI_FINISHED;
        f_res->display_temp_f = target;
        f_res->screen_locked = false;
        g_res.phase = RES_PHASE_FINISHED;
        f_res->last_heat_timer_key = 0xffff;
        f_res->last_temp_f = 0xffff;
    }

    func_res_display_refresh(f_res);
}

static void func_res_lock_check(f_reservation_t *f_res)
{
    if (f_res->ui != RES_UI_HEATING || f_res->screen_locked) {
        return;
    }
    if (tick_check_expire(f_res->heat_start_tick, RES_HEAT_LOCK_MS)) {
        f_res->screen_locked = true;
        func_res_lock_icon_apply(f_res);
    }
}

static void func_res_status_refresh(f_reservation_t *f_res)
{
    tm_t tm = rtc_clock_get();
    bool sec_changed = false;

    if (f_res->last_top_min != tm.min || f_res->last_top_sec != tm.sec) {
        f_res->last_top_min = tm.min;
        f_res->last_top_sec = tm.sec;
        func_res_top_time_refresh(f_res, &tm);
        sec_changed = true;
    }

    if (sec_changed && (f_res->ui == RES_UI_HEATING) && !f_res->heating_paused) {
        func_res_heating_tick(f_res);
    }

    if (sec_changed && (f_res->ui == RES_UI_HEAT_SETUP)) {
        if (f_res->last_info_sec != (u16)(func_res_seconds_until_appt() / 60)) {
            f_res->last_info_sec = (u16)(func_res_seconds_until_appt() / 60);
            func_res_info_text_update(f_res);
        }
    }

    func_res_lock_check(f_res);
}

void func_reservation_poll(void)
{
    tm_t tm;

#if ELUNCHBOX_PANEL_EN
    if (func_cb.sta == FUNC_HOME) {
        return;
    }
#endif
    if (!g_res.setup_done || g_res.phase != RES_PHASE_WAITING) {
        return;
    }

    tm = rtc_clock_get();
    if (tm.min == g_res.last_poll_min) {
        return;
    }
    g_res.last_poll_min = tm.min;

    if (tm.hour == g_res.appt_hour && tm.min == g_res.appt_min) {
        g_res.phase = RES_PHASE_HEATING;
#if FUNC_RESERVATION_UI_EN
#if ELUNCHBOX_PANEL_EN
        /* 到点仅在已在预约页时开加热，不从其它页强跳预约 UI */
        if (func_cb.sta == FUNC_RESERVATION) {
            f_reservation_t *f_res = (f_reservation_t *)func_cb.f_cb;

            if (f_res != NULL) {
                f_res->heat_hour = g_res.heat_hour;
                f_res->heat_min = g_res.heat_min;
                f_res->temp_idx = g_res.temp_idx;
                func_res_start_heating(f_res);
            }
        }
#else
        if (func_cb.sta != FUNC_RESERVATION) {
            func_switch_to(FUNC_RESERVATION, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        } else {
            f_reservation_t *f_res = (f_reservation_t *)func_cb.f_cb;

            if (f_res != NULL) {
                f_res->heat_hour = g_res.heat_hour;
                f_res->heat_min = g_res.heat_min;
                f_res->temp_idx = g_res.temp_idx;
                func_res_start_heating(f_res);
            }
        }
#endif
#endif
    }
}

compo_form_t *func_reservation_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    u8 i;

    home_top_time_create(frm, 0, COMPO_ID_PIC_TOP_H10, COMPO_ID_PIC_TOP_H1,
                         COMPO_ID_PIC_TOP_COLON, COMPO_ID_PIC_TOP_M10,
                         COMPO_ID_PIC_TOP_M1, COMPO_ID_PIC_TOP_AMPM);

    func_res_pic_create_hidden(frm, COMPO_ID_PIC_APPT_H10);
    func_res_pic_create_hidden(frm, COMPO_ID_PIC_APPT_H1);
    func_res_pic_create_hidden(frm, COMPO_ID_PIC_APPT_COLON);
    func_res_pic_create_hidden(frm, COMPO_ID_PIC_APPT_M10);
    func_res_pic_create_hidden(frm, COMPO_ID_PIC_APPT_M1);

    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        func_res_pic_create_hidden(frm, tbl_res_heat_timer_id[i]);
    }
    func_res_pic_create_hidden(frm, COMPO_ID_PIC_HEAT_COLON);

    for (i = 0; i < RES_TEMP_IDX_CNT; i++) {
        func_res_pic_create_hidden(frm, tbl_res_temp_id[i]);
    }
    func_res_pic_create_hidden(frm, COMPO_ID_PIC_TEMP_SYM);

    func_res_pic_create_hidden(frm, COMPO_ID_PIC_BT);
    func_res_pic_create_hidden(frm, COMPO_ID_PIC_LOCK);
    func_res_pic_create_hidden(frm, COMPO_ID_PIC_BAT);

    txt = compo_textbox_create(frm, 48);
    compo_setid(txt, COMPO_ID_TXT_INFO);
    compo_textbox_set_pos(txt, RES_INFO_TEXT_X + RES_INFO_TEXT_W / 2, RES_INFO_TEXT_Y);
    compo_textbox_set_autoroll(txt, true);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_SROLL_CIRC);
    compo_textbox_set_visible(txt, false);

    return frm;
}

static void func_reservation_process(void)
{
    f_reservation_t *f_res = (f_reservation_t *)func_cb.f_cb;

    if (f_res != NULL) {
        func_res_status_refresh(f_res);
    }
    func_process();
}

static void func_reservation_message(size_msg_t msg)
{
    f_reservation_t *f_res = (f_reservation_t *)func_cb.f_cb;

    switch (msg) {
    case RES_MSG_OK:
        func_res_ok_key(f_res);
        break;

    case RES_MSG_PLUS:
        func_res_value_inc(f_res);
        break;

    case RES_MSG_MINUS:
        func_res_value_dec(f_res);
        break;

    case RES_MSG_POWER:
        func_res_power_key(f_res);
        break;

    case KU_LEFT:
        if (f_res != NULL && f_res->ui == RES_UI_HEATING && f_res->screen_locked) {
            f_res->screen_locked = false;
            func_res_lock_icon_apply(f_res);
        }
        break;

#if ELUNCHBOX_PANEL_EN
    case KU_NEXT:
        break;
#endif

    default:
        func_message(msg);
        break;
    }
}

void func_reservation_enter(void)
{
    f_reservation_t *f_res;
    tm_t tm;

    func_cb.f_cb = func_zalloc(sizeof(f_reservation_t));
    func_cb.frm_main = func_reservation_form_create();
    f_res = (f_reservation_t *)func_cb.f_cb;

    home_top_time_bind(&f_res->top_time, COMPO_ID_PIC_TOP_H10, COMPO_ID_PIC_TOP_H1,
                       COMPO_ID_PIC_TOP_COLON, COMPO_ID_PIC_TOP_M10,
                       COMPO_ID_PIC_TOP_M1, COMPO_ID_PIC_TOP_AMPM);
    func_res_appt_bind_digits(f_res);
    for (u8 i = 0; i < RES_TIMER_IDX_CNT; i++) {
        f_res->pic_heat[i] = compo_getobj_byid(tbl_res_heat_timer_id[i]);
    }
    f_res->pic_heat_colon = compo_getobj_byid(COMPO_ID_PIC_HEAT_COLON);
    for (u8 i = 0; i < RES_TEMP_IDX_CNT; i++) {
        f_res->pic_temp[i] = compo_getobj_byid(tbl_res_temp_id[i]);
    }
    f_res->pic_temp_sym = compo_getobj_byid(COMPO_ID_PIC_TEMP_SYM);
    f_res->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f_res->pic_lock = compo_getobj_byid(COMPO_ID_PIC_LOCK);
    f_res->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);
    f_res->txt_info = compo_getobj_byid(COMPO_ID_TXT_INFO);

    f_res->last_top_min = 0xff;
    f_res->last_top_sec = 0xff;
    f_res->last_appt_key = 0xffff;
    f_res->last_heat_timer_key = 0xffff;
    f_res->last_temp_f = 0xffff;
    f_res->last_info_sec = 0xffff;

    if (g_res.phase == RES_PHASE_HEATING || g_res.phase == RES_PHASE_FINISHED) {
        f_res->ui = (g_res.phase == RES_PHASE_FINISHED) ? RES_UI_FINISHED : RES_UI_HEATING;
        f_res->appt_hour = g_res.appt_hour;
        f_res->appt_min = g_res.appt_min;
        f_res->heat_hour = g_res.heat_hour;
        f_res->heat_min = g_res.heat_min;
        f_res->temp_idx = g_res.temp_idx;
        if (f_res->ui == RES_UI_HEATING) {
            func_res_start_heating(f_res);
        } else {
            f_res->display_temp_f = func_res_get_target_temp_f(f_res->temp_idx);
        }
    } else if (g_res.setup_done) {
        f_res->ui = RES_UI_HEAT_SETUP;
        f_res->focus = RES_FOCUS_HEAT_HOUR;
        f_res->appt_hour = g_res.appt_hour;
        f_res->appt_min = g_res.appt_min;
        f_res->heat_hour = g_res.heat_hour;
        f_res->heat_min = g_res.heat_min;
        f_res->temp_idx = g_res.temp_idx;
    } else {
        f_res->ui = RES_UI_APPT_TIME;
        f_res->focus = RES_FOCUS_APPT_HOUR;
        f_res->appt_hour = 3;
        f_res->appt_min = 0;
        f_res->heat_hour = 1;
        f_res->heat_min = 0;
        f_res->temp_idx = 3;
    }

    tm = rtc_clock_get();
    g_res.last_poll_min = tm.min;

    func_res_hide_heat_temp_ui(f_res);
    func_res_hide_appt_ui(f_res);
    func_res_status_icons_apply(f_res);
    func_res_top_time_refresh(f_res, &tm);
    func_res_display_refresh(f_res);
}

void func_reservation_exit(void)
{
    func_cb.last = FUNC_RESERVATION;
}

void func_reservation(void)
{
    printf("%s\n", __func__);
    func_reservation_enter();
    while (func_cb.sta == FUNC_RESERVATION) {
        func_reservation_process();
        func_reservation_message(msg_dequeue());
    }
    func_reservation_exit();
}
