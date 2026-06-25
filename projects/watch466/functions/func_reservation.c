#include "include.h"
#include "func.h"
#include "func_lunchbox_uart.h"
#include "func_reservation.h"
#include "heat_display_reg.h"
#include "ui_layout_anchor.h"
#include "home_icon_res.h"
#include "home_top_time.h"
#include "home_ui_ram.h"
#include "home_ui_shared.h"
#include "home_ui_gpu_detach.h"
#include "ui_layout_anchor.h"

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
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
#define RES_APPT_TIMER_TR_H1_X            ((s16)((s32)141 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define RES_APPT_TIMER_TR_COLON_X         ((s16)((s32)172 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
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

static u8 res_heat_colon_ram[HEAT_WBX_RAM_SIZE];
static u8 res_heat_timer_digit_ram[RES_TIMER_IDX_CNT][HEAT_B_DIGIT_RAM_MAX_SIZE];
static u8 res_temp_digit_ram[RES_TEMP_IDX_CNT][HEAT_B_DIGIT_RAM_MAX_SIZE];
static u8 res_temp_degf_ram[RES_TEMP_DEG_RAM_MAX_SIZE];
static u8 res_temp_suffix_ram[RES_TEMP_SUF_RAM_MAX_SIZE];

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
    140, 158, 176, 194, 212,
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
    if (buf == NULL || buf_len == 0) {
        return;
    }

    if (!func_reservation_is_waiting()) {
        buf[0] = '\0';
        return;
    }

    /* 显示预约界面设置的预约时间（小时+分钟），默认1小时，最高23小时 */
    snprintf(buf, buf_len, "Start in %02u H", g_res.appt_hour);
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

static void func_res_lock_icon_apply(f_reservation_t *f_res);

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
    if (f_res->pic_bat != NULL && gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        compo_picturebox_set_ram(f_res->pic_bat, home_ui_shared_status_bat_ram);
        compo_picturebox_set_size(f_res->pic_bat, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
        compo_picturebox_set_visible(f_res->pic_bat, true);
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

static void func_res_lock_icon_apply(f_reservation_t *f_res)
{
    if (f_res == NULL || f_res->pic_lock == NULL) {
        return;
    }

    if (f_res->screen_locked) {
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

static void func_res_appt_layout(f_reservation_t *f_res, u8 hour, u8 min)
{
    (void)hour;
    (void)min;

    func_res_pic_pos_tr(f_res->pic_appt[RES_TIMER_IDX_H10],
                        RES_APPT_TIMER_TR_H10_X, RES_APPT_TIMER_TR_Y,
                        RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);
    compo_picturebox_set_size(f_res->pic_appt[RES_TIMER_IDX_H10],
                              RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);

    func_res_pic_pos_tr(f_res->pic_appt[RES_TIMER_IDX_H1],
                        RES_APPT_TIMER_TR_H1_X, RES_APPT_TIMER_TR_Y,
                        RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);
    compo_picturebox_set_size(f_res->pic_appt[RES_TIMER_IDX_H1],
                              RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);

    func_res_pic_pos_tr(f_res->pic_appt_colon,
                        RES_APPT_TIMER_TR_COLON_X, RES_APPT_TIMER_TR_Y,
                        RES_APPT_COLON_W, RES_APPT_COLON_H);
    compo_picturebox_set_size(f_res->pic_appt_colon, RES_APPT_COLON_W, RES_APPT_COLON_H);

    func_res_pic_pos_tr(f_res->pic_appt[RES_TIMER_IDX_M10],
                        RES_APPT_TIMER_TR_M10_X, RES_APPT_TIMER_TR_Y,
                        RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);
    compo_picturebox_set_size(f_res->pic_appt[RES_TIMER_IDX_M10],
                              RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);

    func_res_pic_pos_tr(f_res->pic_appt[RES_TIMER_IDX_M1],
                        RES_APPT_TIMER_TR_M1_X, RES_APPT_TIMER_TR_Y,
                        RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);
    compo_picturebox_set_size(f_res->pic_appt[RES_TIMER_IDX_M1],
                              RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);
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

    if (f_res->last_appt_key == key) {
        func_res_appt_layout(f_res, f_res->appt_hour, f_res->appt_min);
        return;
    }
    f_res->last_appt_key = key;

#if ELUNCHBOX_PANEL_EN
    /* 预约页与 Home 互斥：复用 home_ui_digit_ram / home_ui_colon_ram（已在 .disp） */
    func_res_gpu_ram(home_ui_colon_ram, HOME_COLON_RAM_SIZE,
                     UI_BUF_HOME_COLON_BIN, UI_LEN_HOME_COLON_BIN, f_res->pic_appt_colon);
    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];

        func_res_gpu_ram(home_ui_digit_ram[i], HOME_DIGIT_RAM_MAX_SIZE,
                         tbl_res_appt_digit_addr[d], tbl_res_appt_digit_len[d],
                         f_res->pic_appt[i]);
    }
#else
    home_ui_pic_set_flash(f_res->pic_appt_colon, UI_BUF_HOME_COLON_BIN,
                          RES_APPT_COLON_W, RES_APPT_COLON_H);

    for (i = 0; i < RES_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];

        home_ui_pic_set_flash(f_res->pic_appt[i], tbl_res_appt_digit_addr[d],
                              RES_APPT_DIGIT_W, RES_APPT_DIGIT_H);
    }
#endif

    func_res_appt_layout(f_res, f_res->appt_hour, f_res->appt_min);
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
        u16 sym_w = white ? HEAT_WHX_W : HEAT_BHX_W;
        u16 sym_h = white ? HEAT_WHX_H : HEAT_BHX_H;

        func_res_pic_pos_tr(f_res->pic_temp_degf, HEAT_TEMP_TR_UNIT_X, HEAT_TEMP_TR_Y,
                            sym_w, sym_h);
        compo_picturebox_set_size(f_res->pic_temp_degf, sym_w, sym_h);
    }

    if (f_res->pic_temp_suffix != NULL) {
        if (white && HEAT_WSX_W > 0) {
            func_res_pic_pos_tr(f_res->pic_temp_suffix,
                                HEAT_TEMP_TR_UNIT_X + (s16)HEAT_WHX_W, HEAT_TEMP_TR_Y,
                                HEAT_WSX_W, HEAT_WSX_H);
            compo_picturebox_set_size(f_res->pic_temp_suffix, HEAT_WSX_W, HEAT_WSX_H);
            compo_picturebox_set_visible(f_res->pic_temp_suffix, true);
        } else {
            compo_picturebox_set_visible(f_res->pic_temp_suffix, false);
        }
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
        if (white && HEAT_WSX_W > 0) {
            func_res_gpu_ram(res_temp_suffix_ram, RES_TEMP_SUF_RAM_MAX_SIZE,
                             UI_BUF_HOME_WSX_BIN, UI_LEN_HOME_WSX_BIN, f_res->pic_temp_suffix);
        } else {
            compo_picturebox_set_visible(f_res->pic_temp_suffix, false);
        }
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
                          white ? HEAT_WHX_W : HEAT_BHX_W,
                          white ? HEAT_WHX_H : HEAT_BHX_H);

    if (f_res->pic_temp_suffix != NULL) {
        if (white && HEAT_WSX_W > 0) {
            home_ui_pic_set_flash(f_res->pic_temp_suffix, UI_BUF_HOME_WSX_BIN,
                                  HEAT_WSX_W, HEAT_WSX_H);
        } else {
            compo_picturebox_set_visible(f_res->pic_temp_suffix, false);
        }
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

    /* schedule_min > 0 才视为有效的预约数据（heat_display_show 不填充此字段，
     * 仅 heat_display_show_schedule / show_all 显式推送时才生效） */
    if (schedule_changed && info->schedule_min > 0) {
        f_res->live_schedule_min = info->schedule_min;
        f_res->live_schedule_ready = true;
        f_res->last_info_sec = 0xffff;
    }
    f_res->live_remain_min = info->remain_min;
    f_res->live_temp_f = info->temp_f;
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
    } else {
#if FUNC_LUNCHBOX_UART_EN
        if (f_res->live_heat_ready) {
            hour = (u8)(f_res->live_remain_min / 60);
            min = (u8)(f_res->live_remain_min % 60);
            temp = f_res->live_temp_f;
        } else {
            hour = (u8)(f_res->heat_remain_sec / 3600);
            min = (u8)((f_res->heat_remain_sec % 3600) / 60);
            temp = f_res->display_temp_f;
        }
#else
        hour = (u8)(f_res->heat_remain_sec / 3600);
        min = (u8)((f_res->heat_remain_sec % 3600) / 60);
        temp = f_res->display_temp_f;
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
        f_res->ui = RES_UI_FINISHED;
        f_res->display_temp_f = func_res_get_target_temp_f(f_res->temp_idx);
        f_res->screen_locked = false;
        g_res.phase = RES_PHASE_FINISHED;
        /* 不在此处 unregister：由 func_reservation_exit 统一注销 */
    }

    if (f_res->ui != RES_UI_FINISHED) {
        func_res_display_refresh(f_res);
    }
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

    /* 统一回调 func_res_display_on_info 已在 enter 注册，覆盖 HEAT_SETUP + HEATING 双状态 */

#if FUNC_LUNCHBOX_UART_EN
    {
        u16 temp_f = (f_res->temp_idx < RES_TEMP_PRESET_CNT)
                   ? tbl_res_temp_preset[f_res->temp_idx]
                   : tbl_res_temp_preset[0];
        u32 duration_min = f_res->heat_total_sec / 60;

        if (duration_min == 0) {
            duration_min = 1;
        }
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

#if FUNC_LUNCHBOX_UART_EN
    {
        u32 now = RTCCNT + LB_RTC_UNIX_OFFSET;  // RTCCNT 从2020起算, +offset 转Unix时间戳
        u32 today_midnight = now - (now % 86400);
        u32 target_sec = (u32)f_res->appt_hour * 3600 + (u32)f_res->appt_min * 60;
        u32 unix_time = today_midnight + target_sec;
        u16 temp_f = (f_res->temp_idx < RES_TEMP_PRESET_CNT)
                   ? tbl_res_temp_preset[f_res->temp_idx]
                   : tbl_res_temp_preset[0];
        u8 duration = (u8)((u32)f_res->heat_hour * 60 + (u32)f_res->heat_min);

        if (target_sec <= (now % 86400)) {
            unix_time += 86400;
        }
        if (duration == 0) {
            duration = 1;
        }
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

static void func_res_ok_key(f_reservation_t *f_res)
{
    if (f_res == NULL || func_cb.sta != FUNC_RESERVATION) {
        return;
    }

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
        if (f_res->screen_locked) {
            f_res->heating_paused = !f_res->heating_paused;
            func_res_display_refresh(f_res);
        } else {
#if FUNC_LUNCHBOX_UART_EN
            lunchbox_heat_stop();
#endif
            if (func_res_switch_home()) {
                return;
            }
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
        if (f_res->focus == RES_FOCUS_APPT_MIN) {
            f_res->focus = RES_FOCUS_APPT_HOUR;
            f_res->last_appt_key = 0xffff;
        } else if (func_res_switch_home()) {
            return;
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

    switch (f_res->focus) {
    case RES_FOCUS_APPT_HOUR:
        if (f_res->appt_hour < 23) {
            f_res->appt_hour++;
        }
        /* 最高23小时，超过不增加 */
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
                f_res->appt_min = 59;  /* 最高23小时：到达23:59后继续加分钟保持在23:59 */
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
        if (f_res->appt_hour > 1) {
            f_res->appt_hour--;
        }
        /* 默认最低1小时，减到1后不再减少 */
        f_res->last_appt_key = 0xffff;
        break;

    case RES_FOCUS_APPT_MIN:
        if (f_res->appt_min > 0) {
            f_res->appt_min--;
        } else if (f_res->appt_hour > 1) {
            f_res->appt_min = 59;
            f_res->appt_hour--;
        } else {
            /* 最低1小时：到达 01:00 后，继续减分钟保持在 01:00 */
            f_res->appt_min = 0;
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

static void func_res_heating_finish_check(f_reservation_t *f_res)
{
    u16 target;

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

    target = func_res_get_target_temp_f(f_res->temp_idx);
    f_res->ui = RES_UI_FINISHED;
    f_res->display_temp_f = target;
    f_res->screen_locked = false;
    g_res.phase = RES_PHASE_FINISHED;
    f_res->last_heat_timer_key = 0xffff;
    f_res->last_temp_f = 0xffff;
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_heat_stop();
#endif
    func_res_display_refresh(f_res);
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
        heat_display_unregister();
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_heat_stop();
#endif
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

    /* 推送本地预约倒计时到 LCD 回调（LCD 开发方提供 show_schedule / show / show_all 三个接口） */
    {
        u32 now_sec = (u32)tm.hour * 3600 + (u32)tm.min * 60 + (u32)tm.sec;
        u32 appt_sec = (u32)g_res.appt_hour * 3600 + (u32)g_res.appt_min * 60;
        u32 remain;

        if (appt_sec <= now_sec) {
            appt_sec += 24 * 3600;
        }
        remain = (appt_sec - now_sec) / 60;
        if (remain > 0) {
            heat_display_info_t last;
            if (heat_display_get_last(&last)) {
                heat_display_show_all(remain, last.remain_min, last.temp_f);
            } else {
                heat_display_show_schedule(remain);
            }
        }
    }

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
    func_res_pic_pos_tr(pic, HEAT_TEMP_TR_UNIT_X, HEAT_TEMP_TR_Y, HEAT_BHX_W, HEAT_BHX_H);
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

    if (f_res != NULL && f_res->screen_locked) {
        if (msg != RES_MSG_POWER && msg != KU_LEFT) {
            return;
        }
    }

    switch (msg) {
    case RES_MSG_OK:
        func_res_ok_key(f_res);
        break;

    case RES_MSG_PLUS:
        if (f_res != NULL && func_cb.sta == FUNC_RESERVATION) {
            func_res_value_inc(f_res);
        }
        break;

    case RES_MSG_MINUS:
        if (f_res != NULL && func_cb.sta == FUNC_RESERVATION) {
            func_res_value_dec(f_res);
        }
        break;

    case RES_MSG_POWER:
        func_res_power_key(f_res);
        break;

    case KU_LEFT:
        if (f_res != NULL) {
            f_res->screen_locked = !f_res->screen_locked;
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
            /* 统一回调 func_res_display_on_info 已在末尾注册，覆盖所有 UI 状态（含 HEATING） */
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
        f_res->temp_idx = 3;
    }

    /* 确保预约时间范围：默认1小时，最高23小时（进入时夹紧，防止旧数据或外部设置导致越界） */
    if (f_res->appt_hour < 1) f_res->appt_hour = 1;
    if (f_res->appt_hour > 23) f_res->appt_hour = 23;

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
    f_reservation_t *f_res = (f_reservation_t *)func_cb.f_cb;
    u8 i;

    /* 注销加热显示回调（避免切换到其他页面后仍收到推送） */
    heat_display_unregister();

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
