#include "include.h"
#include "func.h"
#include "home_icon_res.h"
#include "home_ui_ram.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * 中部 HH:MM 倒计时（时=白字，分=灰字）；下部温度 xxx°F 用 ui/home 灰色七段 + degf_g.bin。
 * GPU 0x24150：Flash -> RAM -> compo_picturebox_set_ram。
 */
#define UI_HEAT_PLACEHOLDER               UI_BUF_ICON_ACTIVITY_BIN

#ifndef UI_BUF_HOME_0_G_BIN
#error "Run tools/gen_home_icons.py then Output/bin/prebuild.bat to refresh ui.h"
#endif

#ifndef UI_BUF_HOME_DEGF_G_BIN
#error "Run tools/gen_home_icons.py then Output/bin/prebuild.bat to pack degf_g.bin"
#endif

#define HEAT_STATUS_Y                     48
#define HEAT_STATUS_RIGHT_MARGIN          24
#define HEAT_STATUS_GAP                   10
#define HEAT_STATUS_BAT_X                 (GUI_SCREEN_WIDTH - HEAT_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define HEAT_STATUS_LOCK_X                (HEAT_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - HEAT_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define HEAT_STATUS_BT_X                  (HEAT_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - HEAT_STATUS_GAP - HOME_STATUS_BT_W / 2)

#define HEAT_TIMER_Y                      195
#define HEAT_TEMP_Y                       305
#define HEAT_DIGIT_GAP                    4
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
    COMPO_ID_TXT_TOP_TIME = 1,
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
    compo_textbox_t *txt_top_time;
    compo_picturebox_t *pic_timer[HEAT_TIMER_IDX_CNT];
    compo_picturebox_t *pic_timer_colon;
    compo_picturebox_t *pic_temp[HEAT_TEMP_IDX_CNT];
    compo_picturebox_t *pic_temp_degf;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
} f_heat_t;

static u8 heat_status_bt_ram[HOME_STATUS_BT_RAM_SIZE];
static u8 heat_status_lock_ram[HOME_STATUS_LOCK_RAM_SIZE];
static u8 heat_status_bat_ram[HOME_STATUS_BAT_RAM_SIZE];
static u8 heat_temp_digit_ram[HEAT_TEMP_IDX_CNT][HOME_DIGIT_GREY_RAM_MAX_SIZE];
static u8 heat_temp_degf_ram[HOME_TEMPF_RAM_SIZE];
static bool heat_status_icons_inited;

static u32 heat_countdown_remain_sec;
static bool heat_countdown_running;

static const u32 tbl_heat_digit_white_addr[10] = {
    UI_BUF_HOME_0_BIN, UI_BUF_HOME_1_BIN, UI_BUF_HOME_2_BIN, UI_BUF_HOME_3_BIN,
    UI_BUF_HOME_4_BIN, UI_BUF_HOME_5_BIN, UI_BUF_HOME_6_BIN, UI_BUF_HOME_7_BIN,
    UI_BUF_HOME_8_BIN, UI_BUF_HOME_9_BIN,
};

static const u32 tbl_heat_digit_grey_addr[10] = {
    UI_BUF_HOME_0_G_BIN, UI_BUF_HOME_1_G_BIN, UI_BUF_HOME_2_G_BIN, UI_BUF_HOME_3_G_BIN,
    UI_BUF_HOME_4_G_BIN, UI_BUF_HOME_5_G_BIN, UI_BUF_HOME_6_G_BIN, UI_BUF_HOME_7_G_BIN,
    UI_BUF_HOME_8_G_BIN, UI_BUF_HOME_9_G_BIN,
};

static const u16 tbl_heat_digit_white_len[10] = {
    UI_LEN_HOME_0_BIN, UI_LEN_HOME_1_BIN, UI_LEN_HOME_2_BIN, UI_LEN_HOME_3_BIN,
    UI_LEN_HOME_4_BIN, UI_LEN_HOME_5_BIN, UI_LEN_HOME_6_BIN, UI_LEN_HOME_7_BIN,
    UI_LEN_HOME_8_BIN, UI_LEN_HOME_9_BIN,
};

static const u16 tbl_heat_digit_grey_len[10] = {
    UI_LEN_HOME_0_G_BIN, UI_LEN_HOME_1_G_BIN, UI_LEN_HOME_2_G_BIN, UI_LEN_HOME_3_G_BIN,
    UI_LEN_HOME_4_G_BIN, UI_LEN_HOME_5_G_BIN, UI_LEN_HOME_6_G_BIN, UI_LEN_HOME_7_G_BIN,
    UI_LEN_HOME_8_G_BIN, UI_LEN_HOME_9_G_BIN,
};

static const u16 tbl_heat_timer_id[HEAT_TIMER_IDX_CNT] = {
    COMPO_ID_PIC_TIMER_H10, COMPO_ID_PIC_TIMER_H1,
    COMPO_ID_PIC_TIMER_M10, COMPO_ID_PIC_TIMER_M1,
};

static const u16 tbl_heat_temp_id[HEAT_TEMP_IDX_CNT] = {
    COMPO_ID_PIC_TEMP_H, COMPO_ID_PIC_TEMP_T10, COMPO_ID_PIC_TEMP_T1,
};

static void func_heat_status_icons_init(void)
{
    if (heat_status_icons_inited) {
        return;
    }
    os_spiflash_read(heat_status_bt_ram, UI_BUF_HOME_BLUETOOTH_BIN, UI_LEN_HOME_BLUETOOTH_BIN);
    os_spiflash_read(heat_status_lock_ram, UI_BUF_HOME_LOCK_BIN, UI_LEN_HOME_LOCK_BIN);
    os_spiflash_read(heat_status_bat_ram, UI_BUF_HOME_BATTERY_LEVEL_BIN, UI_LEN_HOME_BATTERY_LEVEL_BIN);
    heat_status_icons_inited = true;
}

static void func_heat_status_icons_apply(f_heat_t *f_heat)
{
    func_heat_status_icons_init();

    if (f_heat->pic_bt != NULL && gui_set_ram_check(heat_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_bt, heat_status_bt_ram);
        compo_picturebox_set_size(f_heat->pic_bt, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
    }
    if (f_heat->pic_lock != NULL && gui_set_ram_check(heat_status_lock_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_lock, heat_status_lock_ram);
        compo_picturebox_set_size(f_heat->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    }
    if (f_heat->pic_bat != NULL && gui_set_ram_check(heat_status_bat_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_bat, heat_status_bat_ram);
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

static void func_heat_temp_layout(f_heat_t *f_heat)
{
    u16 total;
    s16 x;
    u8 i;

    total = HOME_DIGIT_W * HEAT_TEMP_IDX_CNT
          + HEAT_TEMP_DIGIT_GAP * (HEAT_TEMP_IDX_CNT - 1)
          + HEAT_TEMP_SYMBOL_GAP + HOME_TEMPF_W;
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2);

    for (i = 0; i < HEAT_TEMP_IDX_CNT; i++) {
        s16 cx = x + (s16)(HOME_DIGIT_W / 2);

        compo_picturebox_set_pos(f_heat->pic_temp[i], cx, HEAT_TEMP_Y);
        compo_picturebox_set_size(f_heat->pic_temp[i], HOME_DIGIT_W, HOME_DIGIT_H);
        x += HOME_DIGIT_W + HEAT_TEMP_DIGIT_GAP;
    }

    if (f_heat->pic_temp_degf != NULL) {
        s16 cx = x + (s16)(HOME_TEMPF_W / 2);

        compo_picturebox_set_pos(f_heat->pic_temp_degf, cx, HEAT_TEMP_Y);
        compo_picturebox_set_size(f_heat->pic_temp_degf, HOME_TEMPF_W, HOME_TEMPF_H);
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

    os_spiflash_read(heat_temp_degf_ram, UI_BUF_HOME_DEGF_G_BIN, UI_LEN_HOME_DEGF_G_BIN);
    if (f_heat->pic_temp_degf != NULL && gui_set_ram_check(heat_temp_degf_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_temp_degf, heat_temp_degf_ram);
    }

    for (i = 0; i < HEAT_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];

        os_spiflash_read(heat_temp_digit_ram[i], tbl_heat_digit_grey_addr[d], tbl_heat_digit_grey_len[d]);
        if (gui_set_ram_check(heat_temp_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_heat->pic_temp[i], heat_temp_digit_ram[i]);
        }
    }

    func_heat_temp_layout(f_heat);
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

static void func_heat_time_str_ampm(char *buf, u16 buf_len, tm_t *tm)
{
    u8 hour = tm->hour;
    const char *ap = "AM";

    if (hour >= 12) {
        ap = "PM";
        if (hour > 12) {
            hour -= 12;
        }
    }
    if (hour == 0) {
        hour = 12;
    }
    snprintf(buf, buf_len, "%d:%02d %s", hour, tm->min, ap);
}

static void func_heat_timer_layout(f_heat_t *f_heat)
{
    u16 total;
    s16 x;
    u8 i;

    total = HOME_DIGIT_W + HEAT_DIGIT_GAP + HOME_DIGIT_W + HEAT_DIGIT_GAP
          + HOME_COLON_W + HEAT_DIGIT_GAP + HOME_DIGIT_W + HEAT_DIGIT_GAP
          + HOME_DIGIT_W;
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2);

    for (i = 0; i < HEAT_TIMER_IDX_CNT; i++) {
        s16 cx = x + (s16)(HOME_DIGIT_W / 2);

        compo_picturebox_set_pos(f_heat->pic_timer[i], cx, HEAT_TIMER_Y);
        compo_picturebox_set_size(f_heat->pic_timer[i], HOME_DIGIT_W, HOME_DIGIT_H);
        x += HOME_DIGIT_W + HEAT_DIGIT_GAP;
        if (i == HEAT_TIMER_IDX_H1) {
            cx = x + (s16)(HOME_COLON_W / 2);
            compo_picturebox_set_pos(f_heat->pic_timer_colon, cx, HEAT_TIMER_Y);
            compo_picturebox_set_size(f_heat->pic_timer_colon, HOME_COLON_W, HOME_COLON_H);
            x += HOME_COLON_W + HEAT_DIGIT_GAP;
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

    os_spiflash_read(home_ui_colon_ram, UI_BUF_HOME_COLON_G_BIN, UI_LEN_HOME_COLON_G_BIN);
    if (gui_set_ram_check(home_ui_colon_ram, __func__)) {
        compo_picturebox_set_ram(f_heat->pic_timer_colon, home_ui_colon_ram);
    }

    for (i = 0; i < HEAT_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];
        bool white = (i <= HEAT_TIMER_IDX_H1);
        u32 addr = white ? tbl_heat_digit_white_addr[d] : tbl_heat_digit_grey_addr[d];
        u16 len = white ? tbl_heat_digit_white_len[d] : tbl_heat_digit_grey_len[d];

        os_spiflash_read(home_ui_digit_ram[i], addr, len);
        if (gui_set_ram_check(home_ui_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_heat->pic_timer[i], home_ui_digit_ram[i]);
        }
    }

    func_heat_timer_layout(f_heat);
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

static void func_heat_status_refresh(f_heat_t *f_heat)
{
    char str[16];
    tm_t tm = rtc_clock_get();

    if (f_heat->last_top_min != tm.min || f_heat->last_top_sec != tm.sec) {
        f_heat->last_top_min = tm.min;
        f_heat->last_top_sec = tm.sec;
        func_heat_time_str_ampm(str, sizeof(str), &tm);
        compo_textbox_set(f_heat->txt_top_time, str);
        func_heat_countdown_tick();
        func_heat_display_refresh(f_heat);
    }
}

compo_form_t *func_heat_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    compo_picturebox_t *pic;
    char str[16];
    tm_t tm = rtc_clock_get();
    u8 i;

    func_heat_time_str_ampm(str, sizeof(str), &tm);
    txt = compo_textbox_create(frm, 16);
    compo_setid(txt, COMPO_ID_TXT_TOP_TIME);
    compo_textbox_set_pos(txt, 78, 48);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, str);

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
        compo_picturebox_set_size(pic, HOME_DIGIT_MAX_W, HOME_DIGIT_H);
    }

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_COLON);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, HEAT_TIMER_Y);
    compo_picturebox_set_size(pic, HOME_COLON_W, HOME_COLON_H);

    for (i = 0; i < HEAT_TEMP_IDX_CNT; i++) {
        pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
        compo_setid(pic, tbl_heat_temp_id[i]);
        compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, HEAT_TEMP_Y);
        compo_picturebox_set_size(pic, HOME_DIGIT_MAX_W, HOME_DIGIT_H);
    }

    pic = compo_picturebox_create(frm, UI_HEAT_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TEMPF);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, HEAT_TEMP_Y);
    compo_picturebox_set_size(pic, HOME_TEMPF_W, HOME_TEMPF_H);

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

    f_heat->txt_top_time = compo_getobj_byid(COMPO_ID_TXT_TOP_TIME);
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
