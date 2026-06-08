#include "include.h"
#include "func.h"
#include "home_icon_res.h"
#include "home_ui_ram.h"
#include "home_ui_shared.h"
#include "home_top_time.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * Heat 页 UI（参照设计图）：
 *   左上：RTC 图标（0m..9m + colonm + AMm/PMm -> ui.bin，home_top_time.c）
 *   右上：bluetooth.bin / lock.bin / battery_level.bin
 *   中部倒计时：w0x.bin..w9x.bin（白字时）+ wbx.bin（冒号）+ b0x.bin..b9x.bin（灰字分）
 *   下部温度：b0x.bin..b9x.bin（灰字）+ bhx.bin（灰 °F）
 * PNG 放 Output/bin/ui/home/，运行 gen_home_icons.py -> prebuild -> ui.bin
 * GPU 0x24150：os_spiflash_read + compo_picturebox_set_ram
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

#ifndef UI_BUF_HOME_BLUETOOTH_BIN
#error "Missing bluetooth.bin: add ui/home/bluetooth.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_LOCK_BIN
#error "Missing lock.bin: add ui/home/lock.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BATTERY_LEVEL_BIN
#error "Missing battery_level.bin: add ui/home/battery_level.png and run gen_home_icons.py + prebuild.bat"
#endif

#define HEAT_STATUS_Y                     48
#define HEAT_STATUS_RIGHT_MARGIN          24
#define HEAT_STATUS_GAP                   10
#define HEAT_STATUS_BAT_X                 (GUI_SCREEN_WIDTH - HEAT_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define HEAT_STATUS_LOCK_X                (HEAT_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - HEAT_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define HEAT_STATUS_BT_X                  (HEAT_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - HEAT_STATUS_GAP - HOME_STATUS_BT_W / 2)

#define HEAT_TIMER_Y                      195
#define HEAT_TEMP_Y                       305
#define HEAT_TIMER_PAIR_GAP               10  /* 时/分各位数字之间 */
#define HEAT_TIMER_PAIR_NARROW_EXTRA      8   /* 含数字 1 等窄字时加宽（如 01） */
#define HEAT_TIMER_COLON_GAP              10  /* 时与分之间（冒号两侧） */
#define HEAT_TEMP_DIGIT_GAP               4
#define HEAT_TEMP_SYMBOL_GAP              6

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
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_LOCK,
    COMPO_ID_PIC_BAT,
};

typedef struct f_heat_t_ {
    u8 last_top_min;
    u8 last_top_sec;
    u16 last_timer_key;
    u16 last_temp_f;
    home_top_time_ui_t top_time;
    compo_picturebox_t *pic_timer[HEAT_TIMER_IDX_CNT];
    compo_picturebox_t *pic_timer_colon;
    compo_picturebox_t *pic_temp[HEAT_TEMP_IDX_CNT];
    compo_picturebox_t *pic_temp_degf;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
} f_heat_t;

static u8 heat_colon_ram[HEAT_WBX_RAM_SIZE];
static u8 heat_temp_digit_ram[HEAT_TEMP_IDX_CNT][HEAT_B_DIGIT_RAM_MAX_SIZE];
static u8 heat_temp_degf_ram[HEAT_BHX_RAM_SIZE];

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
    }
    if (f_heat->pic_lock != NULL && gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_lock, home_ui_shared_status_lock_ram);
        compo_picturebox_set_size(f_heat->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    }
    if (f_heat->pic_bat != NULL && gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_bat, home_ui_shared_status_bat_ram);
        compo_picturebox_set_size(f_heat->pic_bat, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
    }
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

static u16 func_heat_timer_pair_gap(const u16 *tbl_w, u8 d0, u8 d1)
{
    u16 gap = HEAT_TIMER_PAIR_GAP;

    if (d0 == 1 || d1 == 1 || tbl_w[d0] <= HEAT_W1X_W || tbl_w[d1] <= HEAT_W1X_W) {
        gap += HEAT_TIMER_PAIR_NARROW_EXTRA;
    }
    return gap;
}

static u16 func_heat_timer_total_w(u8 hour, u8 min)
{
    u8 w_digits[2] = { hour / 10, hour % 10 };
    u8 b_digits[2] = { min / 10, min % 10 };
    u16 h_pair_gap = func_heat_timer_pair_gap(tbl_heat_w_digit_w, w_digits[0], w_digits[1]);
    u16 m_pair_gap = func_heat_timer_pair_gap(tbl_heat_b_digit_w, b_digits[0], b_digits[1]);

    return tbl_heat_w_digit_w[w_digits[0]] + h_pair_gap
         + tbl_heat_w_digit_w[w_digits[1]] + HEAT_TIMER_COLON_GAP
         + HEAT_WBX_W + HEAT_TIMER_COLON_GAP
         + tbl_heat_b_digit_w[b_digits[0]] + m_pair_gap
         + tbl_heat_b_digit_w[b_digits[1]];
}

static u16 func_heat_temp_total_w(u8 digits[HEAT_TEMP_IDX_CNT])
{
    u16 total = 0;
    u8 i;

    for (i = 0; i < HEAT_TEMP_IDX_CNT; i++) {
        total += tbl_heat_b_digit_w[digits[i]];
        if (i + 1 < HEAT_TEMP_IDX_CNT) {
            total += HEAT_TEMP_DIGIT_GAP;
        }
    }
    total += HEAT_TEMP_SYMBOL_GAP + HEAT_BHX_W;
    return total;
}

static void func_heat_temp_layout(f_heat_t *f_heat, u8 digits[HEAT_TEMP_IDX_CNT])
{
    u16 total;
    s16 x;
    u8 i;

    total = func_heat_temp_total_w(digits);
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2);

    for (i = 0; i < HEAT_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];
        u16 w = tbl_heat_b_digit_w[d];
        s16 cx = x + (s16)(w / 2);

        compo_picturebox_set_pos(f_heat->pic_temp[i], cx, HEAT_TEMP_Y);
        compo_picturebox_set_size(f_heat->pic_temp[i], w, HEAT_B_DIGIT_MAX_H);
        x += w + HEAT_TEMP_DIGIT_GAP;
    }

    if (f_heat->pic_temp_degf != NULL) {
        s16 cx = x + HEAT_TEMP_SYMBOL_GAP + (s16)(HEAT_BHX_W / 2);

        compo_picturebox_set_pos(f_heat->pic_temp_degf, cx, HEAT_TEMP_Y);
        compo_picturebox_set_size(f_heat->pic_temp_degf, HEAT_BHX_W, HEAT_BHX_H);
    }
}

static void func_heat_temp_update(f_heat_t *f_heat, u16 temp_f)
{
    u8 digits[HEAT_TEMP_IDX_CNT];
    u8 i;

    if (temp_f > 999) {
        temp_f = 999;
    }
    if (f_heat->last_temp_f == temp_f) {
        return;
    }
    f_heat->last_temp_f = temp_f;

    digits[HEAT_TEMP_IDX_H] = (u8)(temp_f / 100);
    digits[HEAT_TEMP_IDX_T10] = (u8)((temp_f / 10) % 10);
    digits[HEAT_TEMP_IDX_T1] = (u8)(temp_f % 10);

    os_spiflash_read(heat_temp_degf_ram, UI_BUF_HOME_BHX_BIN, UI_LEN_HOME_BHX_BIN);
    if (f_heat->pic_temp_degf != NULL && gui_set_ram_check(heat_temp_degf_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_temp_degf, heat_temp_degf_ram);
    }

    for (i = 0; i < HEAT_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];

        os_spiflash_read(heat_temp_digit_ram[i], tbl_heat_b_digit_addr[d], tbl_heat_b_digit_len[d]);
        if (gui_set_ram_check(heat_temp_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_heat->pic_temp[i], heat_temp_digit_ram[i]);
        }
    }

    func_heat_temp_layout(f_heat, digits);
}

void func_heat_temp_set_f(u16 temp_f)
{
    f_heat_t *f_heat = (f_heat_t *)func_cb.f_cb;

    if (f_heat != NULL) {
        func_heat_temp_update(f_heat, temp_f);
    }
}

static void func_heat_countdown_tick(void)
{
    if (heat_countdown_running && heat_countdown_remain_sec > 0) {
        heat_countdown_remain_sec--;
        if (heat_countdown_remain_sec == 0) {
            heat_countdown_running = false;
        }
    }
}

static void func_heat_status_refresh(f_heat_t *f_heat)
{
    tm_t tm = rtc_clock_get();

    if (f_heat->last_top_min != tm.min || f_heat->last_top_sec != tm.sec) {
        f_heat->last_top_min = tm.min;
        f_heat->last_top_sec = tm.sec;
        home_top_time_refresh(&f_heat->top_time, &tm);
        func_heat_countdown_tick();
        func_heat_display_refresh(f_heat);
    }
}

static void func_heat_timer_layout(f_heat_t *f_heat, u8 hour, u8 min)
{
    u8 w_digits[2] = { hour / 10, hour % 10 };
    u8 b_digits[2] = { min / 10, min % 10 };
    u16 h_pair_gap = func_heat_timer_pair_gap(tbl_heat_w_digit_w, w_digits[0], w_digits[1]);
    u16 m_pair_gap = func_heat_timer_pair_gap(tbl_heat_b_digit_w, b_digits[0], b_digits[1]);
    u16 total;
    s16 x;
    u8 i;

    total = func_heat_timer_total_w(hour, min);
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2);

    for (i = 0; i < 2; i++) {
        u8 d = w_digits[i];
        u16 w = tbl_heat_w_digit_w[d];
        s16 cx = x + (s16)(w / 2);

        compo_picturebox_set_pos(f_heat->pic_timer[i], cx, HEAT_TIMER_Y);
        compo_picturebox_set_size(f_heat->pic_timer[i], w, HEAT_W_DIGIT_MAX_H);
        x += w + ((i == 0) ? h_pair_gap : HEAT_TIMER_COLON_GAP);
    }

    {
        s16 cx = x + (s16)(HEAT_WBX_W / 2);

        compo_picturebox_set_pos(f_heat->pic_timer_colon, cx, HEAT_TIMER_Y);
        compo_picturebox_set_size(f_heat->pic_timer_colon, HEAT_WBX_W, HEAT_WBX_H);
        x += HEAT_WBX_W + HEAT_TIMER_COLON_GAP;
    }

    for (i = 0; i < 2; i++) {
        u8 d = b_digits[i];
        u16 w = tbl_heat_b_digit_w[d];
        s16 cx = x + (s16)(w / 2);

        compo_picturebox_set_pos(f_heat->pic_timer[i + 2], cx, HEAT_TIMER_Y);
        compo_picturebox_set_size(f_heat->pic_timer[i + 2], w, HEAT_B_DIGIT_MAX_H);
        x += w;
        if (i == 0) {
            x += m_pair_gap;
        }
    }
}

static void func_heat_timer_update(f_heat_t *f_heat, u8 hour, u8 min)
{
    u8 digits[HEAT_TIMER_IDX_CNT];
    u16 timer_key;
    u8 i;

    digits[HEAT_TIMER_IDX_H10] = hour / 10;
    digits[HEAT_TIMER_IDX_H1] = hour % 10;
    digits[HEAT_TIMER_IDX_M10] = min / 10;
    digits[HEAT_TIMER_IDX_M1] = min % 10;
    timer_key = (u16)hour * 100 + min;
    if (f_heat->last_timer_key == timer_key) {
        return;
    }
    f_heat->last_timer_key = timer_key;

    os_spiflash_read(heat_colon_ram, UI_BUF_HOME_WBX_BIN, UI_LEN_HOME_WBX_BIN);
    if (gui_set_ram_check(heat_colon_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_timer_colon, heat_colon_ram);
    }

    for (i = 0; i < HEAT_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];
        bool white = (i <= HEAT_TIMER_IDX_H1);
        u32 addr = white ? tbl_heat_w_digit_addr[d] : tbl_heat_b_digit_addr[d];
        u16 len = white ? tbl_heat_w_digit_len[d] : tbl_heat_b_digit_len[d];

        os_spiflash_read(home_ui_digit_ram[i], addr, len);
        if (gui_set_ram_check(home_ui_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_heat->pic_timer[i], home_ui_digit_ram[i]);
        }
    }

    func_heat_timer_layout(f_heat, hour, min);
}

static void func_heat_display_refresh(f_heat_t *f_heat)
{
    u8 hour = (u8)(heat_countdown_remain_sec / 3600);
    u8 min = (u8)((heat_countdown_remain_sec % 3600) / 60);

    if (hour > 99) {
        hour = 99;
    }
    func_heat_timer_update(f_heat, hour, min);
    func_heat_temp_update(f_heat, f_heat->last_temp_f);
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
    switch (msg) {
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
    f_heat->last_top_min = 0xff;
    f_heat->last_top_sec = 0xff;
    f_heat->last_timer_key = 0xffff;
    f_heat->last_temp_f = 0xffff;

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
    f_heat->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f_heat->pic_lock = compo_getobj_byid(COMPO_ID_PIC_LOCK);
    f_heat->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);

    func_heat_status_icons_apply(f_heat);
    func_heat_countdown_set(2, 0);
    func_heat_countdown_start();
    func_heat_temp_set_f(190);
    func_heat_display_refresh(f_heat);
    func_heat_status_refresh(f_heat);
}

void func_heat_exit(void)
{
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
