#include "include.h"
#include "func.h"
#include "func_lunchbox_uart.h"
#include "func_reservation.h"
#include "heat_display_reg.h"
#include "home_icon_res.h"
#include "home_top_time.h"
#include "home_ui_ram.h"
#include "home_ui_shared.h"
#include "home_ui_gpu_detach.h"
#include "ui_layout_anchor.h"

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
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
 * 预约页（320×240 横屏，单页完成全部流程）：
 *   图一（预约时长）：时钟右上角锚点 83/141/172/225/283,90（0..9.bin + colon.bin）
 *   图二（加热/温度）：时钟 83/141/172/225/283,60；温度 89/139/185/247,158
 * PT8028：TCH4 确认 | TCH5 开关/返回 | TCH2/TCH6 减/加 | TCH0 锁键解锁童锁
 * 图一：加减键仅调预约小时；确认键直接进入图二（加热/温度设置）
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

#ifndef UI_BUF_HOME_WBX_BIN
#error "Missing wbx.bin: add ui/home/wbx.png and run gen_home_icons.py + prebuild.bat"
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
#define RES_TEMP_DEG_RAM_MAX_SIZE         HEAT_WHX_RAM_SIZE
#else
#define RES_TEMP_DEG_RAM_MAX_SIZE         HEAT_BHX_RAM_SIZE
#endif

#if HEAT_WSX_RAM_SIZE > HEAT_BHX_RAM_SIZE
#define RES_TEMP_SUF_RAM_MAX_SIZE         HEAT_WSX_RAM_SIZE
#else
#define RES_TEMP_SUF_RAM_MAX_SIZE         HEAT_BHX_RAM_SIZE
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
#define RES_APPT_DIGIT_W                  HOME_DIGIT_W
#define RES_APPT_DIGIT_H                  HOME_DIGIT_H
#define RES_APPT_COLON_W                  HOME_COLON_W
#define RES_APPT_COLON_H                  HOME_COLON_H
#else
#define RES_STATUS_Y                      48
#define RES_STATUS_RIGHT_MARGIN           24
#define RES_STATUS_GAP                    10
#define RES_INFO_TEXT_X                   78
#define RES_INFO_TEXT_Y                   48
#define RES_INFO_TEXT_W                   280
#define RES_APPT_DIGIT_W                  HOME_DIGIT_W
#define RES_APPT_DIGIT_H                  HOME_DIGIT_H
#define RES_APPT_COLON_W                  HOME_COLON_W
#define RES_APPT_COLON_H                  HOME_COLON_H
#endif

#define RES_STATUS_BAT_X                  (GUI_SCREEN_WIDTH - RES_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define RES_STATUS_LOCK_X                 (RES_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - RES_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define RES_STATUS_BT_X                   (RES_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - RES_STATUS_GAP - HOME_STATUS_BT_W / 2)

/* 图一：预约时长时钟右上角锚点 (320×240) */
#define RES_APPT_TIMER_TR_Y               ((s16)((s32)90 * GUI_SCREEN_HEIGHT / HEAT_LAYOUT_REF_H))
#define RES_APPT_TIMER_TR_H10_X           ((s16)((s32)83 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define RES_APPT_TIMER_TR_H1_X            ((s16)((s32)132 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define RES_APPT_TIMER_TR_COLON_X         ((s16)((s32)175 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define RES_APPT_TIMER_TR_M10_X           ((s16)((s32)225 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define RES_APPT_TIMER_TR_M1_X            ((s16)((s32)283 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))

/* 图二：加热时长时钟（Y=60, H10=83）与温度（同 Heat 页 HEAT_TEMP_TR_*） */
#define RES_HEAT_TIMER_TR_Y               ((s16)((s32)60 * GUI_SCREEN_HEIGHT / HEAT_LAYOUT_REF_H))
#define RES_HEAT_TIMER_TR_H10_X           ((s16)((s32)100 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define RES_HEAT_TIMER_TR_H1_X            ((s16)((s32)141 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define RES_HEAT_TIMER_TR_COLON_X         ((s16)((s32)172 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define RES_HEAT_TIMER_TR_M10_X           ((s16)((s32)225 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define RES_HEAT_TIMER_TR_M1_X            ((s16)((s32)283 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))

#define RES_HEAT_LOCK_MS                  30000
#define RES_TEMP_PRESET_CNT               7
#define RES_MIN_STEP                      5
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
    COMPO_ID_PIC_TEMPF,
    COMPO_ID_PIC_TEMP_S,
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
    u32 live_schedule_min;      /* 串口回调：预约剩余分钟 */
    u32 live_remain_min;        /* 串口回调：加热剩余分钟 */
    u16 live_temp_f;            /* 串口回调：实时温度 °F */
    bool live_schedule_ready;
    bool live_heat_ready;
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
    bool display_pending;
    home_top_time_ui_t top_time;
    compo_picturebox_t *pic_appt[RES_TIMER_IDX_CNT];
    compo_picturebox_t *pic_appt_colon;
    compo_picturebox_t *pic_heat[RES_TIMER_IDX_CNT];
    compo_picturebox_t *pic_heat_colon;
    compo_picturebox_t *pic_temp[RES_TEMP_IDX_CNT];
    compo_picturebox_t *pic_temp_degf;
    compo_picturebox_t *pic_temp_suffix;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
    compo_textbox_t *txt_info;
} f_reservation_t;

reservation_global_t g_res;
bool g_res_heat_pending = false;

static u8 res_heat_colon_ram[HEAT_WBX_RAM_SIZE];
static u8 res_heat_timer_digit_ram[RES_TIMER_IDX_CNT][HEAT_B_DIGIT_RAM_MAX_SIZE];
static u8 res_temp_digit_ram[RES_TEMP_IDX_CNT][HEAT_B_DIGIT_RAM_MAX_SIZE];
static u8 res_temp_degf_ram[RES_TEMP_DEG_RAM_MAX_SIZE];
#if 0
static u8 res_temp_suffix_ram[RES_TEMP_SUF_RAM_MAX_SIZE];
#endif

#if 0
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
#endif

static const u16 tbl_res_temp_preset[RES_TEMP_PRESET_CNT] = {
    104, 122, 140, 158, 176, 194, 212,
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

/* 用合法 placeholder 创建，默认 visible=false；ELUNCHBOX 下用 set_ram 绑定贴图 */
static compo_picturebox_t *func_res_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_RES_PLACEHOLDER);

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

    /* 仅隐藏，不做 set_ram(0)。本页使用 flash 直绑（compo_picturebox_set + ui.bin），
     * set_ram(0) 会把 pic 切到无效 ram@0 模式，导致后续 draw（即使 visible=false 的 pic 被遍历）时 C241。
     * 真正释放用 detach 只在 exit 里做（frm 即将销毁）。
     */
    home_gpu_wait_idle();
    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        if (f_res->pic_appt[i]) compo_picturebox_set_visible(f_res->pic_appt[i], false);
    }
    if (f_res->pic_appt_colon) compo_picturebox_set_visible(f_res->pic_appt_colon, false);
    home_gpu_wait_idle();
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

    home_gpu_wait_idle();
    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        if (f_res->pic_heat[i]) compo_picturebox_set_visible(f_res->pic_heat[i], false);
    }
    if (f_res->pic_heat_colon) compo_picturebox_set_visible(f_res->pic_heat_colon, false);
    for (i = 0; i < RES_TEMP_IDX_CNT; i++) {
        if (f_res->pic_temp[i]) compo_picturebox_set_visible(f_res->pic_temp[i], false);
    }
    if (f_res->pic_temp_degf) compo_picturebox_set_visible(f_res->pic_temp_degf, false);
    if (f_res->pic_temp_suffix) compo_picturebox_set_visible(f_res->pic_temp_suffix, false);
    home_gpu_wait_idle();
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

    home_gpu_wait_idle();
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

bool func_reservation_is_active(void)
{
    return g_res.setup_done &&
           (g_res.phase == RES_PHASE_WAITING || g_res.phase == RES_PHASE_HEATING);
}

bool func_reservation_is_heating(void)
{
    return g_res.setup_done && (g_res.phase == RES_PHASE_HEATING);
}

#if USER_PANEL_LED
static void func_reservation_led_sync(void)
{
    panel_led_set_res_latched(func_reservation_is_active());
}
#endif

static u16 func_res_get_target_temp_f(u8 temp_idx)
{
    if (temp_idx >= RES_TEMP_PRESET_CNT) {
        return tbl_res_temp_preset[0];
    }
    return tbl_res_temp_preset[temp_idx];
}

/** RTC 日历 tm → Unix 秒（tm_to_time 为 2020 纪元，+LB_RTC_UNIX_OFFSET 转 Unix） */
static u32 func_res_tm_to_unix(tm_t tm)
{
    return tm_to_time(tm) + LB_RTC_UNIX_OFFSET;
}

/** 当前 RTC 实际年月日时分秒 → Unix 秒 */
static u32 func_res_now_unix(void)
{
    return func_res_tm_to_unix(rtc_clock_get());
}

/** 当前实际日期 + 预约时:分:00 → Unix 秒；若今天该时刻已过则取明天 */
static u32 func_res_appt_unix_from_setting(u8 appt_hour, u8 appt_min)
{
    tm_t appt = rtc_clock_get();
    printf("appt: %d %d %d\n", appt.year, appt.mon, appt.day);
    u32 appt_rtc;

    appt.hour = appt_hour;
    appt.min = appt_min;
    appt.sec = 0;

    appt_rtc = tm_to_time(appt);
    // if (appt_rtc <= now_rtc) {
    //     appt_rtc = tm_to_time(time_to_tm(appt_rtc + 86400));
    // }
    return appt_rtc + LB_RTC_UNIX_OFFSET;
}

static u32 func_res_seconds_until_appt(void)
{
    u32 now = func_res_now_unix();

    if (g_res.setup_done && g_res.appt_unix > now) {
        return g_res.appt_unix - now;
    }
    return (u32)g_res.appt_hour * 3600 + (u32)g_res.appt_min * 60;
}

void func_reservation_marquee_text(char *buf, u16 buf_len)
{
    if (buf == NULL || buf_len == 0) {
        return;
    }

    if (!func_reservation_is_waiting()) {
        buf[0] = '\0';
        return;
    }

    {
        u32 sec = func_res_seconds_until_appt();
        u32 hours = sec / 3600;

        if (hours > 99) {
            hours = 99;
        }
        snprintf(buf, buf_len, "Start in %02u H", (unsigned)hours);
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

#if FUNC_LUNCHBOX_UART_EN
    if (f_res->live_schedule_ready) {
        u32 hours = f_res->live_schedule_min / 60;

        if (hours > 99) {
            hours = 99;
        }
        snprintf(buf, sizeof(buf), "Start in %02u H", (unsigned)hours);
        compo_textbox_set(f_res->txt_info, buf);
        compo_textbox_set_visible(f_res->txt_info, true);
        return;
    }
#endif

    func_reservation_marquee_text(buf, sizeof(buf));
    compo_textbox_set(f_res->txt_info, buf);
    compo_textbox_set_visible(f_res->txt_info, true);
}

void func_res_lock_icon_apply(f_reservation_t *f_res);

static void func_res_status_icons_apply(f_reservation_t *f_res)
{
#if ELUNCHBOX_PANEL_EN
    home_ui_shared_status_init();
    if (f_res->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f_res->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f_res->pic_bt, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
        compo_picturebox_set_visible(f_res->pic_bt, true);
        compo_picturebox_set_pos(f_res->pic_bt, RES_STATUS_BT_X, RES_STATUS_Y);
    }
    home_ui_shared_status_bind_bat(f_res->pic_bat);
    if (f_res->pic_bat != NULL) {
        compo_picturebox_set_pos(f_res->pic_bat, RES_STATUS_BAT_X, RES_STATUS_Y);
    }
#else
    home_ui_status_apply_flash(f_res->pic_bt, f_res->pic_lock, f_res->pic_bat, false);
    if (f_res->pic_bt != NULL) {
        compo_picturebox_set_pos(f_res->pic_bt, RES_STATUS_BT_X, RES_STATUS_Y);
    }
    if (f_res->pic_bat != NULL) {
        compo_picturebox_set_pos(f_res->pic_bat, RES_STATUS_BAT_X, RES_STATUS_Y);
    }
#endif
    func_res_lock_icon_apply(f_res);
}

void func_res_lock_icon_apply(f_reservation_t *f_res)
{
    if (f_res == NULL || f_res->pic_lock == NULL) {
        return;
    }

    if (func_key_lock_show_status_icon(f_res->screen_locked)) {
        compo_picturebox_set_pos(f_res->pic_lock, RES_STATUS_LOCK_X, RES_STATUS_Y);
#if ELUNCHBOX_PANEL_EN
        home_ui_shared_status_init();
        if (gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
            compo_picturebox_set_ram(f_res->pic_lock, home_ui_shared_status_lock_ram);
            compo_picturebox_set_size(f_res->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
            compo_picturebox_set_visible(f_res->pic_lock, true);
        }
#else
        home_ui_pic_set_flash(f_res->pic_lock, UI_BUF_HOME_LOCK_BIN,
                              HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
        compo_picturebox_set_visible(f_res->pic_lock, true);
#endif
    } else {
        compo_picturebox_set_visible(f_res->pic_lock, false);
    }
}

static void func_res_top_time_hide(home_top_time_ui_t *ui)
{
    if (ui == NULL) {
        return;
    }
    /* 轻量隐藏：只 visible=false + 失效缓存。不要调用完整 gpu_detach（它会 set_ram(0) 毒害 flash 直绑的 pic）。
     * 完整 detach（含 set_ram(0)）只在 exit 做。
     */
    home_gpu_wait_idle();
    if (ui->pic_h10) compo_picturebox_set_visible(ui->pic_h10, false);
    if (ui->pic_h1) compo_picturebox_set_visible(ui->pic_h1, false);
    if (ui->pic_colon) compo_picturebox_set_visible(ui->pic_colon, false);
    if (ui->pic_m10) compo_picturebox_set_visible(ui->pic_m10, false);
    if (ui->pic_m1) compo_picturebox_set_visible(ui->pic_m1, false);
    if (ui->pic_ampm) compo_picturebox_set_visible(ui->pic_ampm, false);
    ui->last_key = 0;
    home_gpu_wait_idle();
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

#if ELUNCHBOX_PANEL_EN
    home_top_time_refresh(&f_res->top_time, tm);
#else
    home_top_time_refresh_flash(&f_res->top_time, tm);
#endif
}

static void func_res_pic_pos_tr(compo_picturebox_t *pic, s16 tr_x, s16 tr_y, u16 w, u16 h)
{
    if (pic == NULL) {
        return;
    }
    compo_picturebox_set_pos(pic, tr_x - (s16)(w / 2), tr_y + (s16)(h / 2));
}

static void func_res_appt_layout(f_reservation_t *f_res, u8 hour, u8 min,
                               bool h_white, bool m_white)
{
    const u16 *htbl = h_white ? tbl_res_w_digit_w : tbl_res_b_digit_w;
    const u16 *mtbl = m_white ? tbl_res_w_digit_w : tbl_res_b_digit_w;
    u8 hd[2] = { hour / 10, hour % 10 };
    u8 md[2] = { min / 10, min % 10 };
    u16 h_digit_h = h_white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H;
    u16 m_digit_h = m_white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H;

    (void)hour;
    (void)min;

    func_res_pic_pos_tr(f_res->pic_appt[RES_TIMER_IDX_H10],
                        RES_APPT_TIMER_TR_H10_X, RES_APPT_TIMER_TR_Y,
                        htbl[hd[0]], h_digit_h);
    compo_picturebox_set_size(f_res->pic_appt[RES_TIMER_IDX_H10], htbl[hd[0]], h_digit_h);

    func_res_pic_pos_tr(f_res->pic_appt[RES_TIMER_IDX_H1],
                        RES_APPT_TIMER_TR_H1_X, RES_APPT_TIMER_TR_Y,
                        htbl[hd[1]], h_digit_h);
    compo_picturebox_set_size(f_res->pic_appt[RES_TIMER_IDX_H1], htbl[hd[1]], h_digit_h);

    func_res_pic_pos_tr(f_res->pic_appt_colon,
                        RES_APPT_TIMER_TR_COLON_X, RES_APPT_TIMER_TR_Y,
                        RES_APPT_COLON_W, RES_APPT_COLON_H);
    compo_picturebox_set_size(f_res->pic_appt_colon, RES_APPT_COLON_W, RES_APPT_COLON_H);

    func_res_pic_pos_tr(f_res->pic_appt[RES_TIMER_IDX_M10],
                        RES_APPT_TIMER_TR_M10_X, RES_APPT_TIMER_TR_Y,
                        mtbl[md[0]], m_digit_h);
    compo_picturebox_set_size(f_res->pic_appt[RES_TIMER_IDX_M10], mtbl[md[0]], m_digit_h);

    func_res_pic_pos_tr(f_res->pic_appt[RES_TIMER_IDX_M1],
                        RES_APPT_TIMER_TR_M1_X, RES_APPT_TIMER_TR_Y,
                        mtbl[md[1]], m_digit_h);
    compo_picturebox_set_size(f_res->pic_appt[RES_TIMER_IDX_M1], mtbl[md[1]], m_digit_h);
}

static u16 func_res_appt_total_min(const f_reservation_t *f_res)
{
    return (u16)f_res->appt_hour * 60 + f_res->appt_min;
}

static void func_res_appt_apply_total_min(f_reservation_t *f_res, u16 total_min)
{
    f_res->appt_hour = (u8)(total_min / 60);
    f_res->appt_min = (u8)(total_min % 60);
}

static u16 func_res_heat_setup_total_min(const f_reservation_t *f_res)
{
    return (u16)f_res->heat_hour * 60 + f_res->heat_min;
}

static void func_res_heat_setup_apply_total_min(f_reservation_t *f_res, u16 total_min)
{
    if (total_min < LB_HEAT_DURATION_MIN_MIN) {
        total_min = LB_HEAT_DURATION_MIN_MIN;
    }
    if (total_min > LB_HEAT_DURATION_MAX_MIN) {
        total_min = LB_HEAT_DURATION_MAX_MIN;
    }
    f_res->heat_hour = (u8)(total_min / 60);
    f_res->heat_min = (u8)(total_min % 60);
}

static void func_res_appt_update(f_reservation_t *f_res, bool h_white, bool m_white)
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

    if (f_res->last_appt_key == key
        && f_res->last_h_white == h_white && f_res->last_m_white == m_white) {
        func_res_appt_layout(f_res, f_res->appt_hour, f_res->appt_min, h_white, m_white);
        return;
    }
    f_res->last_appt_key = key;
    f_res->last_h_white = h_white;
    f_res->last_m_white = m_white;

#if ELUNCHBOX_PANEL_EN
    /* 预约页与 Home 互斥：复用 home_ui_digit_ram / home_ui_colon_ram（已在 .disp） */
    func_res_gpu_ram(home_ui_colon_ram, HOME_COLON_RAM_SIZE,
                     UI_BUF_HOME_COLON_BIN, UI_LEN_HOME_COLON_BIN, f_res->pic_appt_colon);
    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];
        bool white = (i <= RES_TIMER_IDX_H1) ? h_white : m_white;
        u32 addr = white ? tbl_res_w_digit_addr[d] : tbl_res_b_digit_addr[d];
        u16 len = white ? tbl_res_w_digit_len[d] : tbl_res_b_digit_len[d];

        func_res_gpu_ram(home_ui_digit_ram[i], HOME_DIGIT_RAM_MAX_SIZE,
                         addr, len, f_res->pic_appt[i]);
    }
#else
    home_ui_pic_set_flash(f_res->pic_appt_colon, UI_BUF_HOME_COLON_BIN,
                          RES_APPT_COLON_W, RES_APPT_COLON_H);

    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];
        bool white = (i <= RES_TIMER_IDX_H1) ? h_white : m_white;
        u32 addr = white ? tbl_res_w_digit_addr[d] : tbl_res_b_digit_addr[d];
        u16 w = white ? tbl_res_w_digit_w[d] : tbl_res_b_digit_w[d];

        home_ui_pic_set_flash(f_res->pic_appt[i], addr, w,
                              white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H);
    }
#endif

    func_res_appt_layout(f_res, f_res->appt_hour, f_res->appt_min, h_white, m_white);
}

static void func_res_heat_timer_layout(f_reservation_t *f_res, u8 hour, u8 min,
                                       bool h_white, bool m_white)
{
    const u16 *htbl = h_white ? tbl_res_w_digit_w : tbl_res_b_digit_w;
    const u16 *mtbl = m_white ? tbl_res_w_digit_w : tbl_res_b_digit_w;
    u8 hd[2] = { hour / 10, hour % 10 };
    u8 md[2] = { min / 10, min % 10 };
    u16 h_digit_h = h_white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H;
    u16 m_digit_h = m_white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H;

    func_res_pic_pos_tr(f_res->pic_heat[RES_TIMER_IDX_H10],
                        RES_HEAT_TIMER_TR_H10_X, RES_HEAT_TIMER_TR_Y,
                        htbl[hd[0]], h_digit_h);
    compo_picturebox_set_size(f_res->pic_heat[RES_TIMER_IDX_H10], htbl[hd[0]], h_digit_h);

    func_res_pic_pos_tr(f_res->pic_heat[RES_TIMER_IDX_H1],
                        RES_HEAT_TIMER_TR_H1_X, RES_HEAT_TIMER_TR_Y,
                        htbl[hd[1]], h_digit_h);
    compo_picturebox_set_size(f_res->pic_heat[RES_TIMER_IDX_H1], htbl[hd[1]], h_digit_h);

    func_res_pic_pos_tr(f_res->pic_heat_colon,
                        RES_HEAT_TIMER_TR_COLON_X, RES_HEAT_TIMER_TR_Y,
                        HEAT_WBX_W, HEAT_WBX_H);
    compo_picturebox_set_size(f_res->pic_heat_colon, HEAT_WBX_W, HEAT_WBX_H);

    func_res_pic_pos_tr(f_res->pic_heat[RES_TIMER_IDX_M10],
                        RES_HEAT_TIMER_TR_M10_X, RES_HEAT_TIMER_TR_Y,
                        mtbl[md[0]], m_digit_h);
    compo_picturebox_set_size(f_res->pic_heat[RES_TIMER_IDX_M10], mtbl[md[0]], m_digit_h);

    func_res_pic_pos_tr(f_res->pic_heat[RES_TIMER_IDX_M1],
                        RES_HEAT_TIMER_TR_M1_X, RES_HEAT_TIMER_TR_Y,
                        mtbl[md[1]], m_digit_h);
    compo_picturebox_set_size(f_res->pic_heat[RES_TIMER_IDX_M1], mtbl[md[1]], m_digit_h);
}

static void func_res_heat_timer_update(f_reservation_t *f_res, u8 hour, u8 min,
                                       bool h_white, bool m_white)
{
    u8 digits[RES_TIMER_IDX_CNT];
    u16 key;
    u8 i;

    if (f_res->ui == RES_UI_APPT_TIME) {
        home_gpu_wait_idle();
        for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
            if (f_res->pic_heat[i]) compo_picturebox_set_visible(f_res->pic_heat[i], false);
        }
        if (f_res->pic_heat_colon) compo_picturebox_set_visible(f_res->pic_heat_colon, false);
        home_gpu_wait_idle();
        return;
    }

    digits[RES_TIMER_IDX_H10] = hour / 10;
    digits[RES_TIMER_IDX_H1] = hour % 10;
    digits[RES_TIMER_IDX_M10] = min / 10;
    digits[RES_TIMER_IDX_M1] = min % 10;
    key = (u16)hour * 100 + min;
    if (f_res->last_heat_timer_key == key
        && f_res->last_h_white == h_white && f_res->last_m_white == m_white) {
        func_res_heat_timer_layout(f_res, hour, min, h_white, m_white);
        return;
    }
    f_res->last_heat_timer_key = key;
    f_res->last_h_white = h_white;
    f_res->last_m_white = m_white;

#if ELUNCHBOX_PANEL_EN
    func_res_gpu_ram(res_heat_colon_ram, HEAT_WBX_RAM_SIZE,
                     UI_BUF_HOME_WBX_BIN, UI_LEN_HOME_WBX_BIN, f_res->pic_heat_colon);

    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];
        bool white = (i <= RES_TIMER_IDX_H1) ? h_white : m_white;
        u32 addr = white ? tbl_res_w_digit_addr[d] : tbl_res_b_digit_addr[d];
        u16 len = white ? tbl_res_w_digit_len[d] : tbl_res_b_digit_len[d];

        func_res_gpu_ram(res_heat_timer_digit_ram[i], HEAT_B_DIGIT_RAM_MAX_SIZE,
                         addr, len, f_res->pic_heat[i]);
    }
#else
    home_ui_pic_set_flash(f_res->pic_heat_colon, UI_BUF_HOME_WBX_BIN, HEAT_WBX_W, HEAT_WBX_H);

    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];
        bool white = (i <= RES_TIMER_IDX_H1) ? h_white : m_white;
        u32 addr = white ? tbl_res_w_digit_addr[d] : tbl_res_b_digit_addr[d];
        u16 w = white ? tbl_res_w_digit_w[d] : tbl_res_b_digit_w[d];

        home_ui_pic_set_flash(f_res->pic_heat[i], addr, w,
                              white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H);
    }
#endif

    func_res_heat_timer_layout(f_res, hour, min, h_white, m_white);
}

static void func_res_temp_layout(f_reservation_t *f_res, u8 digits[RES_TEMP_IDX_CNT], bool white)
{
    const u16 *tbl = white ? tbl_res_w_digit_w : tbl_res_b_digit_w;
    static const s16 tbl_temp_tr_x[RES_TEMP_IDX_CNT] = {
        HEAT_TEMP_TR_H_X, HEAT_TEMP_TR_T10_X, HEAT_TEMP_TR_T1_X,
    };
    u16 digit_h = white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H;
    u8 i;

    for (i = 0; i < RES_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];
        u16 w = tbl[d];

        func_res_pic_pos_tr(f_res->pic_temp[i], tbl_temp_tr_x[i], HEAT_TEMP_TR_Y, w, digit_h);
        compo_picturebox_set_size(f_res->pic_temp[i], w, digit_h);
    }

    if (f_res->pic_temp_degf != NULL) {
        func_res_pic_pos_tr(f_res->pic_temp_degf, HEAT_TEMP_TR_UNIT_X, HEAT_TEMP_TR_UNIT_Y,
                            HEAT_BHX_W, HEAT_BHX_H);
        compo_picturebox_set_size(f_res->pic_temp_degf, HEAT_BHX_W, HEAT_BHX_H);
        compo_picturebox_set_visible(f_res->pic_temp_degf, true);
    }
    if (f_res->pic_temp_suffix != NULL) {
        compo_picturebox_set_visible(f_res->pic_temp_suffix, false);
    }
}

static void func_res_temp_update(f_reservation_t *f_res, u16 temp_f, bool white)
{
    u8 digits[RES_TEMP_IDX_CNT];
    u8 i;

    if (f_res->ui == RES_UI_APPT_TIME) {
        home_gpu_wait_idle();
        for (i = 0; i < RES_TEMP_IDX_CNT; i++) {
            if (f_res->pic_temp[i]) compo_picturebox_set_visible(f_res->pic_temp[i], false);
        }
        if (f_res->pic_temp_degf) compo_picturebox_set_visible(f_res->pic_temp_degf, false);
        if (f_res->pic_temp_suffix) compo_picturebox_set_visible(f_res->pic_temp_suffix, false);
        home_gpu_wait_idle();
        return;
    }

    if (temp_f > 999) {
        temp_f = 999;
    }

    digits[RES_TEMP_IDX_H] = (u8)(temp_f / 100);
    digits[RES_TEMP_IDX_T10] = (u8)((temp_f / 10) % 10);
    digits[RES_TEMP_IDX_T1] = (u8)(temp_f % 10);

    if (f_res->last_temp_f == temp_f && f_res->last_t_white == white) {
        func_res_temp_layout(f_res, digits, white);
        return;
    }
    f_res->last_temp_f = temp_f;
    f_res->last_t_white = white;

#if ELUNCHBOX_PANEL_EN
    {
        u32 sym_addr = white ? UI_BUF_HOME_WHX_BIN : UI_BUF_HOME_BHX_BIN;
        u16 sym_len = white ? UI_LEN_HOME_WHX_BIN : UI_LEN_HOME_BHX_BIN;

        func_res_gpu_ram(res_temp_degf_ram, RES_TEMP_DEG_RAM_MAX_SIZE,
                         sym_addr, sym_len, f_res->pic_temp_degf);
    }

    if (f_res->pic_temp_suffix != NULL) {
        compo_picturebox_set_visible(f_res->pic_temp_suffix, false);
    }

    for (i = 0; i < RES_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];
        u32 addr = white ? tbl_res_w_digit_addr[d] : tbl_res_b_digit_addr[d];
        u16 len = white ? tbl_res_w_digit_len[d] : tbl_res_b_digit_len[d];

        func_res_gpu_ram(res_temp_digit_ram[i], HEAT_B_DIGIT_RAM_MAX_SIZE,
                         addr, len, f_res->pic_temp[i]);
    }
#else
    home_ui_pic_set_flash(f_res->pic_temp_degf, white ? UI_BUF_HOME_WHX_BIN : UI_BUF_HOME_BHX_BIN,
                          HEAT_BHX_W, HEAT_BHX_H);

    if (f_res->pic_temp_suffix != NULL) {
        compo_picturebox_set_visible(f_res->pic_temp_suffix, false);
    }

    for (i = 0; i < RES_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];
        u32 addr = white ? tbl_res_w_digit_addr[d] : tbl_res_b_digit_addr[d];
        u16 w = white ? tbl_res_w_digit_w[d] : tbl_res_b_digit_w[d];

        home_ui_pic_set_flash(f_res->pic_temp[i], addr, w,
                              white ? HEAT_W_DIGIT_MAX_H : HEAT_B_DIGIT_MAX_H);
    }
#endif

    func_res_temp_layout(f_res, digits, white);
}

static void func_res_display_refresh(f_reservation_t *f_res);
static void func_res_heating_finish_check(f_reservation_t *f_res);

static void func_res_display_on_info(const heat_display_info_t *info)
{
    f_reservation_t *f_res = (f_reservation_t *)func_cb.f_cb;
    bool schedule_changed;
    bool heat_changed;

    if (info == NULL || f_res == NULL || func_cb.sta != FUNC_RESERVATION) {
        return;
    }

    schedule_changed = (!f_res->live_schedule_ready
                        || f_res->live_schedule_min != info->schedule_min);
    heat_changed = (!f_res->live_heat_ready
                    || f_res->live_remain_min != info->remain_min
                    || f_res->live_temp_f != info->temp_f);

    f_res->live_schedule_min = info->schedule_min;
    f_res->live_remain_min = info->remain_min;
    f_res->live_temp_f = info->temp_f;
    if (schedule_changed) {
        f_res->live_schedule_ready = true;
        f_res->last_info_sec = 0xffff;
    }
    if (heat_changed) {
        f_res->live_heat_ready = true;
        f_res->last_heat_timer_key = 0xffff;
        f_res->last_temp_f = 0xffff;
    }

    if (f_res->ui == RES_UI_HEATING && f_res->live_heat_ready && info->remain_min == 0) {
        func_res_heating_finish_check(f_res);
    }

    if (f_res->ui == RES_UI_HEAT_SETUP || f_res->ui == RES_UI_HEATING) {
        func_res_display_refresh(f_res);
    }
}

static void func_res_display_refresh(f_reservation_t *f_res)
{
    u8 hour;
    u8 min;
    u16 temp;
    bool h_white;
    bool m_white;
    bool t_white;

    if (f_res->ui == RES_UI_APPT_TIME) {
        tm_t tm = rtc_clock_get();
        bool h_white = (f_res->focus == RES_FOCUS_APPT_HOUR);
        bool m_white = (f_res->focus == RES_FOCUS_APPT_MIN);

        func_res_hide_heat_temp_ui(f_res);
        f_res->top_time.last_key = 0;
        func_res_top_time_refresh(f_res, &tm);
        func_res_appt_update(f_res, h_white, m_white);
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
    } else {
        temp = func_res_get_target_temp_f(f_res->temp_idx);
#if FUNC_LUNCHBOX_UART_EN
        if (f_res->live_heat_ready) {
            hour = (u8)(f_res->live_remain_min / 60);
            min = (u8)(f_res->live_remain_min % 60);
        } else {
            hour = (u8)(f_res->heat_remain_sec / 3600);
            min = (u8)((f_res->heat_remain_sec % 3600) / 60);
        }
#else
        hour = (u8)(f_res->heat_remain_sec / 3600);
        min = (u8)((f_res->heat_remain_sec % 3600) / 60);
#endif
        h_white = true;
        m_white = true;
        t_white = true;
    }

    if (hour > 99) {
        hour = 99;
    }

    func_res_heat_timer_update(f_res, hour, min, h_white, m_white);
    func_res_temp_update(f_res, temp, t_white);
    func_res_info_text_update(f_res);
    func_res_lock_icon_apply(f_res);
}

/* 加热模块实时推送的显示回调 (UART 0x01 / BLE 0x04 DataPoints → LCD) */
static void func_reservation_heat_display_on_info(const heat_display_info_t *info)
{
    f_reservation_t *f_res = (f_reservation_t *)func_cb.f_cb;

    if (info == NULL || f_res == NULL || func_cb.sta != FUNC_RESERVATION) {
        return;
    }
    if (f_res->ui != RES_UI_HEATING) {
        return;
    }

    f_res->heat_remain_sec = info->remain_min * 60;
    f_res->display_temp_f = info->temp_f;
    f_res->last_heat_timer_key = 0xffff;
    f_res->last_temp_f = 0xffff;

    /* 加热完成检测 */
    if (info->remain_min == 0 && f_res->heat_remain_sec == 0) {
        f_res->live_heat_ready = true;
        f_res->live_remain_min = 0;
        func_res_heating_finish_check(f_res);
        return;
    }

    func_res_display_refresh(f_res);
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
    f_res->live_schedule_ready = false;
    f_res->live_heat_ready = false;
    f_res->live_schedule_min = 0;
    f_res->live_remain_min = 0;
    f_res->live_temp_f = 0;
    f_res->display_temp_f = 0;
    f_res->last_heat_timer_key = 0xffff;
    f_res->last_temp_f = 0xffff;
    g_res.phase = RES_PHASE_HEATING;

    /* 注册加热显示回调：接收 UART/BLE 推送的实时剩余时间+温度 */
    heat_display_register(func_reservation_heat_display_on_info);

#if FUNC_LUNCHBOX_UART_EN
    {
        u16 temp_f = (f_res->temp_idx < RES_TEMP_PRESET_CNT)
                   ? tbl_res_temp_preset[f_res->temp_idx]
                   : tbl_res_temp_preset[0];
        u32 duration_min = f_res->heat_total_sec / 60;

        if (duration_min == 0) {
            duration_min = LB_HEAT_DURATION_MIN_MIN;
        }
        printf("target_temp_f reservation: %d, duration_min: %d, proto_mode: %d\n",
               lunchbox_temp_f_to_idx(temp_f), duration_min, 4);
        lunchbox_heat_start(4, lunchbox_temp_f_to_idx(temp_f), duration_min);
    }
#endif

#if ELUNCHBOX_PANEL_EN
    /* 留给进入路径末尾的两阶段统一刷，避免过早绑资源 */
#else
    func_res_display_refresh(f_res);
#endif
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

static void func_res_clear_switch_keys(void)
{
    msg_queue_detach(RES_MSG_POWER, 0);
    msg_queue_detach(RES_MSG_OK, 0);
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    while (pt8028_take_res_key_pending()) {
    }
#endif
}

static bool func_res_switch_home(void)
{
    u8 prev_sta;

    if (sys_cb.flag_swithing || func_cb.sta != FUNC_RESERVATION) {
        return false;
    }

    /* GPU exit 预清理：切换前把 GPU 彻底排空，避免 fade 期间与 Home 资源重叠导致 C241 */
    home_gpu_wait_idle();
    os_gui_draw_force();
    home_gpu_wait_idle();

    func_res_clear_switch_keys();
    prev_sta = func_cb.sta;
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
    return (func_cb.sta == FUNC_HOME && func_cb.sta != prev_sta);
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

#if USER_PANEL_LED
    func_reservation_led_sync();
#endif

    g_res.appt_unix = func_res_appt_unix_from_setting(f_res->appt_hour, f_res->appt_min);

#if FUNC_LUNCHBOX_UART_EN
    {
        u32 unix_time = g_res.appt_unix;
        u16 temp_f = (f_res->temp_idx < RES_TEMP_PRESET_CNT)
                   ? tbl_res_temp_preset[f_res->temp_idx]
                   : tbl_res_temp_preset[0];
        u8 duration = (u8)((u32)f_res->heat_hour * 60 + (u32)f_res->heat_min);

        if (duration < LB_HEAT_DURATION_MIN_MIN) {
            duration = LB_HEAT_DURATION_MIN_MIN;
        } else if (duration > LB_HEAT_DURATION_MAX_MIN) {
            duration = LB_HEAT_DURATION_MAX_MIN;
        }
        printf("reservation send: unix_time=%u now=%u duration=%u\n",
               unix_time, func_res_now_unix(), duration);
        lunchbox_reservation_send(1, 0, NULL, unix_time,
                                  lunchbox_temp_f_to_idx(temp_f),
                                  duration, 1, 0xff);
    }
#endif

    if (sys_cb.flag_swithing) {
        return;
    }
    func_res_clear_switch_keys();
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

u8 func_reservation_new_ui_load_hour(void)
{
    u8 hour;
    u8 min;
    u8 sec;

    func_reservation_new_ui_load_time(&hour, &min, &sec);
    return hour;
}

void func_reservation_new_ui_load_time(u8 *hour, u8 *min, u8 *sec)
{
    if (hour == NULL || min == NULL || sec == NULL) {
        return;
    }
    if (g_res.setup_done && g_res.phase == RES_PHASE_WAITING) {
        *hour = g_res.appt_hour;
        if (*hour > FUNC_RES_APPT_HOUR_MAX) {
            *hour = FUNC_RES_APPT_HOUR_MAX;
        }
        *min = g_res.appt_min - (g_res.appt_min % 5);
        *sec = 0;
        return;
    }
    {
        tm_t tm = rtc_clock_get();

        *hour = (u8)((tm.hour + 1) % 24);
        *min = (u8)((tm.min / 5) * 5);
        *sec = 0;
    }
}

void func_reservation_new_ui_submit_time(u8 hour, u8 min, u8 sec)
{
    u16 temp_f;
    u8 duration;
    u32 now;

    (void)sec;
    if (hour > FUNC_RES_APPT_HOUR_MAX) {
        hour = FUNC_RES_APPT_HOUR_MAX;
    }
    min = min - (min % 5);
    if (min > 55) {
        min = 55;
    }

    g_res.setup_done = true;
    g_res.phase = RES_PHASE_WAITING;
    g_res.appt_hour = hour;
    g_res.appt_min = min;
    g_res.heat_hour = 1;
    g_res.heat_min = 0;
    g_res.temp_idx = 5;
    g_res.appt_triggered_today = false;
    g_res.appt_unix = func_res_appt_unix_from_setting(hour, min);
    now = func_res_now_unix();
    if (g_res.appt_unix <= now) {
        g_res.appt_unix += 86400;
    }
    g_res.last_poll_min = 0xff;

#if USER_PANEL_LED
    func_reservation_led_sync();
#endif

#if FUNC_LUNCHBOX_UART_EN
    temp_f = tbl_res_temp_preset[g_res.temp_idx];
    duration = (u8)((u32)g_res.heat_hour * 60 + (u32)g_res.heat_min);
    if (duration < LB_HEAT_DURATION_MIN_MIN) {
        duration = LB_HEAT_DURATION_MIN_MIN;
    } else if (duration > LB_HEAT_DURATION_MAX_MIN) {
        duration = LB_HEAT_DURATION_MAX_MIN;
    }
    printf("new_res submit: %02u:%02u unix=%u duration=%u\n",
           hour, min, g_res.appt_unix, duration);
    lunchbox_reservation_send(1, 0, NULL, g_res.appt_unix,
                              lunchbox_temp_f_to_idx(temp_f),
                              duration, 1, 0xff);
#endif

    if (sys_cb.flag_swithing) {
        return;
    }
    func_res_clear_switch_keys();
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

void func_reservation_new_ui_do_submit(void)
{
    u16 temp_f;
    u8 duration;
    u32 now;

    g_res.phase = RES_PHASE_WAITING;
    g_res.appt_triggered_today = false;
    g_res.appt_unix = func_res_appt_unix_from_setting(g_res.appt_hour, g_res.appt_min);
    now = func_res_now_unix();
    if (g_res.appt_unix <= now) {
        g_res.appt_unix += 86400;
    }
    g_res.last_poll_min = 0xff;
    g_res_heat_pending = false;

#if USER_PANEL_LED
    func_reservation_led_sync();
#endif

#if FUNC_LUNCHBOX_UART_EN
    temp_f = tbl_res_temp_preset[g_res.temp_idx];
    duration = (u8)((u32)g_res.heat_hour * 60 + (u32)g_res.heat_min);
    if (duration < LB_HEAT_DURATION_MIN_MIN) {
        duration = LB_HEAT_DURATION_MIN_MIN;
    } else if (duration > LB_HEAT_DURATION_MAX_MIN) {
        duration = LB_HEAT_DURATION_MAX_MIN;
    }
    printf("new_res submit(appt=%02u:%02u heat=%uh%um idx=%u): unix=%u duration=%u\n",
           g_res.appt_hour, g_res.appt_min,
           g_res.heat_hour, g_res.heat_min, g_res.temp_idx,
           g_res.appt_unix, duration);
    lunchbox_reservation_send(1, 0, NULL, g_res.appt_unix,
                              lunchbox_temp_f_to_idx(temp_f),
                              duration, 1, 0xff);
#endif

    if (sys_cb.flag_swithing) {
        return;
    }
    func_res_clear_switch_keys();
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

void func_reservation_new_ui_submit(u8 appt_hour)
{
    func_reservation_new_ui_submit_time(appt_hour, 0, 0);
}

bool func_reservation_new_ui_go_home(void)
{
    return func_res_switch_home();
}

static void func_res_ok_key(f_reservation_t *f_res)
{
    if (f_res == NULL || func_cb.sta != FUNC_RESERVATION) {
        return;
    }

    switch (f_res->ui) {
    case RES_UI_APPT_TIME:
        switch (f_res->focus) {
        case RES_FOCUS_APPT_HOUR:
            f_res->focus = RES_FOCUS_APPT_MIN;
            break;
        case RES_FOCUS_APPT_MIN:
            f_res->ui = RES_UI_HEAT_SETUP;
            f_res->focus = RES_FOCUS_HEAT_HOUR;
            break;
        default:
            break;
        }
        f_res->last_appt_key = 0xffff;
        f_res->last_heat_timer_key = 0xffff;
        f_res->last_temp_f = 0xffff;
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
        if (func_res_switch_home()) {
            return;
        }
        break;

    default:
        break;
    }

    if (func_cb.sta != FUNC_RESERVATION || f_res == NULL) {
        return;
    }
    func_res_display_refresh(f_res);
}

static void func_res_power_key(f_reservation_t *f_res)
{
    if (f_res == NULL || func_cb.sta != FUNC_RESERVATION) {
        return;
    }

    if (f_res->ui == RES_UI_HEATING) {
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_heat_stop();
#endif
        g_res.phase = RES_PHASE_NONE;
        g_res.setup_done = false;
        if (func_res_switch_home()) {
            return;
        }
        return;
    }

    if (f_res->ui == RES_UI_FINISHED) {
        g_res.phase = RES_PHASE_NONE;
        g_res.setup_done = false;
        if (func_res_switch_home()) {
            return;
        }
        return;
    }

    switch (f_res->ui) {
    case RES_UI_APPT_TIME:
        switch (f_res->focus) {
        case RES_FOCUS_APPT_HOUR:
            if (func_res_switch_home()) {
                return;
            }
            break;
        case RES_FOCUS_APPT_MIN:
            f_res->focus = RES_FOCUS_APPT_HOUR;
            f_res->last_appt_key = 0xffff;
            break;
        default:
            break;
        }
        break;

    case RES_UI_HEAT_SETUP:
        switch (f_res->focus) {
        case RES_FOCUS_HEAT_HOUR:
            f_res->ui = RES_UI_APPT_TIME;
            f_res->focus = RES_FOCUS_APPT_HOUR;
            f_res->last_appt_key = 0xffff;
            f_res->last_heat_timer_key = 0xffff;
            break;
        case RES_FOCUS_HEAT_MIN:
            f_res->focus = RES_FOCUS_HEAT_HOUR;
            f_res->last_heat_timer_key = 0xffff;
            break;
        case RES_FOCUS_HEAT_TEMP:
            /* 设置完预约时间 + 加热时间 + 温度后，按开关键提交预约并返回主界面，显示跑马灯 */
            func_res_save_and_go_home(f_res);
            return;
        default:
            break;
        }
        break;

    default:
        break;
    }

    if (func_cb.sta != FUNC_RESERVATION) {
        return;
    }
    func_res_display_refresh(f_res);
}

static void func_res_value_inc(f_reservation_t *f_res)
{
    if (f_res->ui == RES_UI_HEATING || f_res->ui == RES_UI_FINISHED) {
        return;
    }

    if (f_res->ui == RES_UI_APPT_TIME || f_res->ui == RES_UI_HEAT_SETUP) {
        switch (f_res->focus) {
        case RES_FOCUS_APPT_HOUR:
            if (f_res->appt_hour < FUNC_RES_APPT_HOUR_MAX) {
                f_res->appt_hour++;
            }
            f_res->last_appt_key = 0xffff;
            break;

        case RES_FOCUS_APPT_MIN:
            func_res_appt_apply_total_min(f_res,
                                          func_res_appt_total_min(f_res) + RES_MIN_STEP);
            f_res->last_appt_key = 0xffff;
            break;

        case RES_FOCUS_HEAT_HOUR:
            {
                u16 total = func_res_heat_setup_total_min(f_res);

                if (total + 60 <= LB_HEAT_DURATION_MAX_MIN) {
                    func_res_heat_setup_apply_total_min(f_res, total + 60);
                }
            }
            f_res->last_heat_timer_key = 0xffff;
            break;

        case RES_FOCUS_HEAT_MIN:
            {
                u16 total = func_res_heat_setup_total_min(f_res);

                if (total + RES_MIN_STEP <= LB_HEAT_DURATION_MAX_MIN) {
                    func_res_heat_setup_apply_total_min(f_res, total + RES_MIN_STEP);
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
}

static void func_res_value_dec(f_reservation_t *f_res)
{
    if (f_res->ui == RES_UI_HEATING || f_res->ui == RES_UI_FINISHED) {
        return;
    }

    if (f_res->ui == RES_UI_APPT_TIME || f_res->ui == RES_UI_HEAT_SETUP) {
        switch (f_res->focus) {
        case RES_FOCUS_APPT_HOUR:
            if (f_res->appt_hour > 0) {
                f_res->appt_hour--;
            }
            f_res->last_appt_key = 0xffff;
            break;

        case RES_FOCUS_APPT_MIN:
            {
                u16 total = func_res_appt_total_min(f_res);

                if (total >= RES_MIN_STEP) {
                    total -= RES_MIN_STEP;
                } else {
                    total = 0;
                }
                func_res_appt_apply_total_min(f_res, total);
            }
            f_res->last_appt_key = 0xffff;
            break;

        case RES_FOCUS_HEAT_HOUR:
            {
                u16 total = func_res_heat_setup_total_min(f_res);

                if (total >= LB_HEAT_DURATION_MIN_MIN + 60) {
                    func_res_heat_setup_apply_total_min(f_res, total - 60);
                } else {
                    func_res_heat_setup_apply_total_min(f_res, LB_HEAT_DURATION_MIN_MIN);
                }
            }
            f_res->last_heat_timer_key = 0xffff;
            break;

        case RES_FOCUS_HEAT_MIN:
            {
                u16 total = func_res_heat_setup_total_min(f_res);

                if (total >= LB_HEAT_DURATION_MIN_MIN + RES_MIN_STEP) {
                    func_res_heat_setup_apply_total_min(f_res, total - RES_MIN_STEP);
                } else {
                    func_res_heat_setup_apply_total_min(f_res, LB_HEAT_DURATION_MIN_MIN);
                }
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
}

static void func_res_heating_finish_check(f_reservation_t *f_res)
{
    if (f_res == NULL || f_res->ui != RES_UI_HEATING) {
        return;
    }
#if FUNC_LUNCHBOX_UART_EN
    if (!f_res->live_heat_ready || f_res->live_remain_min != 0) {
        return;
    }
#else
    if (f_res->heat_remain_sec != 0) {
        return;
    }
#endif

    f_res->screen_locked = false;
    g_res.phase = RES_PHASE_FINISHED;
    f_res->last_heat_timer_key = 0xffff;
    f_res->last_temp_f = 0xffff;
    heat_display_unregister();
    func_mode_keep_warm_enter();
}

static void func_res_heating_tick(f_reservation_t *f_res)
{
#if FUNC_LUNCHBOX_UART_EN
    (void)f_res;
    return;
#else
    u16 target;

    if (f_res->ui != RES_UI_HEATING || f_res->heating_paused) {
        return;
    }

    if (f_res->heat_remain_sec > 0) {
        f_res->heat_remain_sec--;
    }

    target = func_res_get_target_temp_f(f_res->temp_idx);
    f_res->display_temp_f = target;

    if (f_res->heat_remain_sec == 0) {
        f_res->live_heat_ready = true;
        f_res->live_remain_min = 0;
        f_res->screen_locked = false;
        g_res.phase = RES_PHASE_FINISHED;
        f_res->last_heat_timer_key = 0xffff;
        f_res->last_temp_f = 0xffff;
        heat_display_unregister();
        func_mode_keep_warm_enter();
        return;
    }

    func_res_display_refresh(f_res);
#endif
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
#if FUNC_LUNCHBOX_UART_EN
        if (f_res->live_schedule_ready) {
            if (f_res->last_info_sec != (u16)f_res->live_schedule_min) {
                f_res->last_info_sec = (u16)f_res->live_schedule_min;
                func_res_info_text_update(f_res);
            }
        } else
#endif
        if (f_res->last_info_sec != (u16)(func_res_seconds_until_appt() / 60)) {
            f_res->last_info_sec = (u16)(func_res_seconds_until_appt() / 60);
            func_res_info_text_update(f_res);
        }
    }

    func_res_lock_check(f_res);
}

#if ELUNCHBOX_PANEL_EN && FUNC_LUNCHBOX_UART_EN
/** Home/息屏到点：不跳预约 UI，直接 UART 开加热 */
static void func_res_trigger_heating_uart_from_global(void)
{
    u16 temp_f;
    u32 duration_min;

    temp_f = (g_res.temp_idx < RES_TEMP_PRESET_CNT)
           ? tbl_res_temp_preset[g_res.temp_idx]
           : tbl_res_temp_preset[0];
    duration_min = (u32)g_res.heat_hour * 60 + (u32)g_res.heat_min;
    if (duration_min == 0) {
        duration_min = LB_HEAT_DURATION_MIN_MIN;
    }
    if (duration_min < LB_HEAT_DURATION_MIN_MIN) {
        duration_min = LB_HEAT_DURATION_MIN_MIN;
    } else if (duration_min > LB_HEAT_DURATION_MAX_MIN) {
        duration_min = LB_HEAT_DURATION_MAX_MIN;
    }
    /* 设置加热参数 + 自动启动标记，使 func_heat_enter 能直接进入加热面板 */
    lb_mode_to_heat_set(4, temp_f, (u8)(duration_min / 60), (u8)(duration_min % 60));
    lb_heat_autostart_set(true);
    lunchbox_heat_start(4, lunchbox_temp_f_to_idx(temp_f), duration_min);
}
#endif

void func_reservation_on_manual_shutdown(void)
{
    if (!g_res.setup_done) {
        return;
    }
    if (g_res.phase == RES_PHASE_HEATING || g_res.phase == RES_PHASE_FINISHED) {
        g_res.phase = RES_PHASE_WAITING;
    }
#if USER_PANEL_LED
    func_reservation_led_sync();
#endif
#if ELUNCHBOX_PANEL_EN
    func_reservation_sleep_wake_arm();
#endif
}

#if ELUNCHBOX_PANEL_EN
void func_reservation_sleep_wake_arm(void)
{
    u32 until;
    u32 sec;

    if (!func_reservation_is_waiting()) {
        return;
    }
    until = func_res_seconds_until_appt();
    if (until == 0) {
        sec = 1;
    } else if (until > 3600) {
        sec = 3600;
    } else {
        sec = until;
    }
    if (sec > 60) {
        sec = 60;
    }
    if (sec < 1) {
        sec = 1;
    }
    rtc_set_alarm_wakeup(sec);
}
#endif

#if ELUNCHBOX_PANEL_EN && FUNC_RESERVATION_UI_EN
static void func_reservation_fire_heating_at_appt(void)
{
    g_res.phase = RES_PHASE_HEATING;
#if USER_PANEL_LED
    func_reservation_led_sync();
#endif
#if ELUNCHBOX_PANEL_EN
    if (func_cb.sta == FUNC_RESERVATION) {
        func_res_trigger_heating_uart_from_global();
        if (func_cb.sta != FUNC_HEAT) {
            func_res_allow_switch = 1;
            func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
            func_res_allow_switch = 0;
        }
    } else {
#if FUNC_LUNCHBOX_UART_EN
        func_res_trigger_heating_uart_from_global();
#endif
        if (elunchbox_pwr_is_manual_off()
            || elunchbox_pwr_gui_off_is_on()
            || sys_cb.gui_sleep_sta) {
            elunchbox_pwr_gui_wake_reason("reservation heat");
        }
        if (func_cb.sta != FUNC_HEAT) {
            func_res_allow_switch = 1;
            func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
            func_res_allow_switch = 0;
        }
    }
#endif
}
#endif

void func_reservation_poll(void)
{
    tm_t tm;
    bool guioff_deep;

#if USER_PANEL_LED
    func_reservation_led_sync();
#endif

    if (!g_res.setup_done || g_res.phase != RES_PHASE_WAITING) {
        return;
    }

    guioff_deep = elunchbox_pwr_is_manual_off()
               || elunchbox_pwr_gui_off_is_on()
               || sys_cb.gui_sleep_sta;

    tm = rtc_clock_get();
    if (!guioff_deep && tm.min == g_res.last_poll_min) {
        return;
    }
    g_res.last_poll_min = tm.min;

    {
        u32 now = func_res_now_unix();

        if (g_res.appt_unix > 0 && now >= g_res.appt_unix) {
#if ELUNCHBOX_PANEL_EN && FUNC_RESERVATION_UI_EN
            func_reservation_fire_heating_at_appt();
#else
            g_res.phase = RES_PHASE_HEATING;
#if USER_PANEL_LED
            func_reservation_led_sync();
#endif
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
        }
    }

#if ELUNCHBOX_PANEL_EN
    if (func_reservation_is_waiting() && elunchbox_pwr_is_manual_off()) {
        func_reservation_sleep_wake_arm();
    }
#endif
}

compo_form_t *func_reservation_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    compo_picturebox_t *pic;

    home_top_time_create(frm, UI_RES_PLACEHOLDER, COMPO_ID_PIC_TOP_H10, COMPO_ID_PIC_TOP_H1,
                         COMPO_ID_PIC_TOP_COLON, COMPO_ID_PIC_TOP_M10,
                         COMPO_ID_PIC_TOP_M1, COMPO_ID_PIC_TOP_AMPM);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_APPT_H10);
    func_res_pic_pos_tr(pic, RES_APPT_TIMER_TR_H10_X, RES_APPT_TIMER_TR_Y,
                        RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);
    compo_picturebox_set_size(pic, RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_APPT_H1);
    func_res_pic_pos_tr(pic, RES_APPT_TIMER_TR_H1_X, RES_APPT_TIMER_TR_Y,
                        RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);
    compo_picturebox_set_size(pic, RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_APPT_M10);
    func_res_pic_pos_tr(pic, RES_APPT_TIMER_TR_M10_X, RES_APPT_TIMER_TR_Y,
                        RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);
    compo_picturebox_set_size(pic, RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_APPT_M1);
    func_res_pic_pos_tr(pic, RES_APPT_TIMER_TR_M1_X, RES_APPT_TIMER_TR_Y,
                        RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);
    compo_picturebox_set_size(pic, RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_APPT_COLON);
    func_res_pic_pos_tr(pic, RES_APPT_TIMER_TR_COLON_X, RES_APPT_TIMER_TR_Y,
                        RES_APPT_COLON_W, RES_APPT_COLON_H);
    compo_picturebox_set_size(pic, RES_APPT_COLON_W, RES_APPT_COLON_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_HEAT_H10);
    func_res_pic_pos_tr(pic, RES_HEAT_TIMER_TR_H10_X, RES_HEAT_TIMER_TR_Y,
                        HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_HEAT_H1);
    func_res_pic_pos_tr(pic, RES_HEAT_TIMER_TR_H1_X, RES_HEAT_TIMER_TR_Y,
                        HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_HEAT_M10);
    func_res_pic_pos_tr(pic, RES_HEAT_TIMER_TR_M10_X, RES_HEAT_TIMER_TR_Y,
                        HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_HEAT_M1);
    func_res_pic_pos_tr(pic, RES_HEAT_TIMER_TR_M1_X, RES_HEAT_TIMER_TR_Y,
                        HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_HEAT_COLON);
    func_res_pic_pos_tr(pic, RES_HEAT_TIMER_TR_COLON_X, RES_HEAT_TIMER_TR_Y,
                        HEAT_WBX_W, HEAT_WBX_H);
    compo_picturebox_set_size(pic, HEAT_WBX_W, HEAT_WBX_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_TEMP_H);
    func_res_pic_pos_tr(pic, HEAT_TEMP_TR_H_X, HEAT_TEMP_TR_Y, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_TEMP_T10);
    func_res_pic_pos_tr(pic, HEAT_TEMP_TR_T10_X, HEAT_TEMP_TR_Y, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_TEMP_T1);
    func_res_pic_pos_tr(pic, HEAT_TEMP_TR_T1_X, HEAT_TEMP_TR_Y, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_B0X_W, HEAT_B_DIGIT_MAX_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_TEMPF);
    func_res_pic_pos_tr(pic, HEAT_TEMP_TR_UNIT_X, HEAT_TEMP_TR_UNIT_Y, HEAT_BHX_W, HEAT_BHX_H);
    compo_picturebox_set_size(pic, HEAT_BHX_W, HEAT_BHX_H);

    func_res_pic_create_hidden(frm, COMPO_ID_PIC_TEMP_S);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, RES_STATUS_BT_X, RES_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BT_W, HOME_STATUS_BT_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_LOCK);
    compo_picturebox_set_pos(pic, RES_STATUS_LOCK_X, RES_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);

    pic = func_res_pic_create_hidden(frm, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, RES_STATUS_BAT_X, RES_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);

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
#if ELUNCHBOX_PANEL_EN
        if (f_res->display_pending) {
            f_res->display_pending = false;
            home_gpu_wait_idle();
            func_res_display_refresh(f_res);
        }
#endif
        func_res_status_refresh(f_res);
    }
    func_process();
}

static void func_reservation_message(size_msg_t msg)
{
    f_reservation_t *f_res = (f_reservation_t *)func_cb.f_cb;

    if (sys_cb.flag_swithing) {
        return;
    }

    if (func_key_lock_ku_blocked(msg)) {
        return;
    }

    if (f_res != NULL && f_res->screen_locked) {
        if (msg != RES_MSG_POWER) {
            return;
        }
    }

    switch (msg) {
    case RES_MSG_OK:
        func_res_ok_key(f_res);
        break;

    case RES_MSG_PLUS:
        if (f_res != NULL && func_cb.sta == FUNC_RESERVATION) {
            func_res_value_dec(f_res);
        }
        break;

    case RES_MSG_MINUS:
        if (f_res != NULL && func_cb.sta == FUNC_RESERVATION) {
            func_res_value_inc(f_res);
        }
        break;

    case RES_MSG_POWER:
        func_res_power_key(f_res);
        break;

    case KU_LEFT:
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

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif

    func_cb.f_cb = func_zalloc(sizeof(f_reservation_t));
    home_gpu_wait_idle();
    WDT_CLR();
    func_cb.frm_main = func_reservation_form_create();
    f_res = (f_reservation_t *)func_cb.f_cb;
#if ELUNCHBOX_PANEL_EN
    tft_bglight_force_on();
#endif
    WDT_CLR();

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
    f_res->pic_temp_degf = compo_getobj_byid(COMPO_ID_PIC_TEMPF);
    f_res->pic_temp_suffix = compo_getobj_byid(COMPO_ID_PIC_TEMP_S);
    f_res->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f_res->pic_lock = compo_getobj_byid(COMPO_ID_PIC_LOCK);
    f_res->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);
    home_ui_shared_battery_attach_pic(f_res->pic_bat);
    f_res->txt_info = compo_getobj_byid(COMPO_ID_TXT_INFO);

    f_res->last_top_min = 0xff;
    f_res->last_top_sec = 0xff;
    f_res->last_appt_key = 0xffff;
    f_res->last_heat_timer_key = 0xffff;
    f_res->last_temp_f = 0xffff;
    f_res->last_info_sec = 0xffff;
    f_res->live_schedule_min = 0;
    f_res->live_remain_min = 0;
    f_res->live_temp_f = 0;
    f_res->live_schedule_ready = false;
    f_res->live_heat_ready = false;

    if (g_res.phase == RES_PHASE_HEATING || g_res.phase == RES_PHASE_FINISHED) {
        f_res->ui = (g_res.phase == RES_PHASE_FINISHED) ? RES_UI_FINISHED : RES_UI_HEATING;
        f_res->appt_hour = g_res.appt_hour;
        f_res->appt_min = g_res.appt_min;
        f_res->heat_hour = g_res.heat_hour;
        f_res->heat_min = g_res.heat_min;
        f_res->temp_idx = g_res.temp_idx;
        if (f_res->ui == RES_UI_HEATING) {
            /* 只设状态，显示绑定留给进入路径末尾的”空白首帧 + 内容 apply”两阶段统一做，
             * 避免在 form_create 后、受保护的 os_gui_draw_force 之前就调用 display_refresh 绑资源。
             */
            f_res->screen_locked = false;
            f_res->heating_paused = false;
            f_res->heat_start_tick = tick_get();
            f_res->heat_total_sec = (u32)f_res->heat_hour * 3600 + (u32)f_res->heat_min * 60;
            if (f_res->heat_total_sec == 0) f_res->heat_total_sec = 60;
            f_res->heat_remain_sec = f_res->heat_total_sec;
            f_res->display_temp_f = 0;
            f_res->last_heat_timer_key = 0xffff;
            f_res->last_temp_f = 0xffff;
            g_res.phase = RES_PHASE_HEATING;
            /* 重新进入加热中页面时，注册显示回调接收实时推送 */
            heat_display_register(func_reservation_heat_display_on_info);
            /* 不要在这里 display_refresh；两阶段末尾会根据当前 ui 调一次 */
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
        f_res->appt_hour = 1;   /* 默认1小时 */
        f_res->appt_min = 0;
        f_res->heat_hour = 1;
        f_res->heat_min = 0;
        f_res->temp_idx = 5;    // 194°F (90°C)
    }

    func_res_heat_setup_apply_total_min(f_res, func_res_heat_setup_total_min(f_res));

    tm = rtc_clock_get();
    g_res.last_poll_min = tm.min;

    home_ui_digit_pool_reset();

#if ELUNCHBOX_PANEL_EN
    f_res->display_pending = true;
    func_res_status_icons_apply(f_res);
    tft_bglight_force_on();
#else
    func_res_status_icons_apply(f_res);
    func_res_display_refresh(f_res);
#endif
    heat_display_register(func_res_display_on_info);
}

void func_reservation_exit(void)
{
    home_ui_shared_battery_detach_pic();
    heat_display_unregister();
    f_reservation_t *f_res = (f_reservation_t *)func_cb.f_cb;
    u8 i;

    if (f_res != NULL) {
        /* GPU exit：彻底释放 reservation 持有的所有 GPU 资源（top_time / 预约/加热计时器 / 温度 / 状态图标）。
         * 多轮 wait + force + detach，避免切回 Home 时与 Home clock/status 共享资源冲突导致 C241/C245。
         */
        home_gpu_wait_idle();
        os_gui_draw_force();
        home_gpu_wait_idle();

        home_top_time_gpu_detach(&f_res->top_time);
        home_gpu_wait_idle();

        for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
            home_ui_gpu_pic_detach(f_res->pic_appt[i]);
            home_ui_gpu_pic_detach(f_res->pic_heat[i]);
        }
        home_ui_gpu_pic_detach(f_res->pic_appt_colon);
        home_ui_gpu_pic_detach(f_res->pic_heat_colon);
        home_gpu_wait_idle();

        for (i = 0; i < RES_TEMP_IDX_CNT; i++) {
            home_ui_gpu_pic_detach(f_res->pic_temp[i]);
        }
        home_ui_gpu_pic_detach(f_res->pic_temp_degf);
        home_ui_gpu_pic_detach(f_res->pic_temp_suffix);
        home_gpu_wait_idle();

        home_ui_gpu_pic_detach(f_res->pic_bt);
        home_ui_gpu_pic_detach(f_res->pic_lock);
        home_ui_gpu_pic_detach(f_res->pic_bat);

        /* 额外多轮同步，确保 detach 完全生效 */
        home_gpu_wait_idle();
        os_gui_draw_force();
        home_gpu_wait_idle();
        os_gui_draw_force();
        home_gpu_wait_idle();
    }

    home_ui_digit_pool_reset();
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
