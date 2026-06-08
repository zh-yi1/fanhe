#include "include.h"
#include "func.h"
#include "home_icon_res.h"
#include "home_ui_ram.h"
#include "home_ui_shared.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * Mode 页：顶栏时间 + 绿色温度 + 状态图标；中部白色 HH:MM 倒计时；底部 Pasta/Chicken/Warm Tab。
 * Tab 预设：Pasta 194°F/1:00，Chicken 212°F/1:00，Warm 120°F/1:00。
 * 全部 ui/home/*.bin 为 GPU 0x24150：Flash->RAM->set_ram。
 */
#define UI_MODE_PLACEHOLDER               UI_BUF_ICON_ACTIVITY_BIN
#define MODE_COLOR_BLUE                   make_color(4, 109, 217)

#ifndef UI_BUF_HOME_PASTA_SEL_BIN
#error "Run tools/gen_home_icons.py then Output/bin/prebuild.bat to refresh ui.h"
#endif


#define MODE_TAB_BTN_W                    118
#define MODE_TAB_BTN_H                    96
#define MODE_TAB_BTN_R                    18
#define MODE_TAB_BTN_R_IN                 16
#define MODE_TAB_BTN_Y                    390
#define MODE_TAB_ICON_SIZE                40
#define MODE_TAB_PAD_TOP                  12
#define MODE_TAB_PAD_BOTTOM               10
#define MODE_TAB_PAD_MID                  8
#define MODE_TAB_FONT_H                   11
#define MODE_TAB_ICON_Y                   (MODE_TAB_BTN_Y - MODE_TAB_BTN_H / 2 + MODE_TAB_PAD_TOP + MODE_TAB_ICON_SIZE / 2)
#define MODE_TAB_LABEL_Y                  (MODE_TAB_ICON_Y + MODE_TAB_ICON_SIZE / 2 + MODE_TAB_PAD_MID + MODE_TAB_FONT_H / 2)
#define MODE_TAB_DASH_Y                   (MODE_TAB_BTN_Y + MODE_TAB_BTN_H / 2 - MODE_TAB_PAD_BOTTOM - HOME_DASH_RAM_H / 2)
#define MODE_TAB_X0                       87
#define MODE_TAB_X1                       233
#define MODE_TAB_X2                       379

#define MODE_STATUS_Y                     48
#define MODE_STATUS_TIME_X                78
#define MODE_STATUS_RIGHT_MARGIN          24
#define MODE_STATUS_GAP                   10
#define MODE_STATUS_BAT_X                 (GUI_SCREEN_WIDTH - MODE_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define MODE_STATUS_LOCK_X                (MODE_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - MODE_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define MODE_STATUS_BT_X                  (MODE_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - MODE_STATUS_GAP - HOME_STATUS_BT_W / 2)

#define MODE_TIMER_Y                      195
#define MODE_DIGIT_GAP                    4
#define MODE_STATUS_TEMP_Y                48
#define MODE_STATUS_TEMP_GAP              2
#define MODE_STATUS_TEMP_SYMBOL_GAP       2

enum {
    MODE_TIMER_IDX_H10 = 0,
    MODE_TIMER_IDX_H1,
    MODE_TIMER_IDX_M10,
    MODE_TIMER_IDX_M1,
    MODE_TIMER_IDX_CNT,
};

enum {
    MODE_TEMP_IDX_H = 0,
    MODE_TEMP_IDX_T10,
    MODE_TEMP_IDX_T1,
    MODE_TEMP_IDX_CNT,
};

enum {
    MODE_TAB_PASTA = 0,
    MODE_TAB_CHICKEN,
    MODE_TAB_WARM,
    MODE_TAB_CNT,
};

enum {
    COMPO_ID_TXT_TOP_TIME = 1,
    COMPO_ID_PIC_STATUS_TEMP_H,
    COMPO_ID_PIC_STATUS_TEMP_T10,
    COMPO_ID_PIC_STATUS_TEMP_T1,
    COMPO_ID_PIC_STATUS_TEMPF,
    COMPO_ID_PIC_TIMER_H10,
    COMPO_ID_PIC_TIMER_H1,
    COMPO_ID_PIC_TIMER_COLON,
    COMPO_ID_PIC_TIMER_M10,
    COMPO_ID_PIC_TIMER_M1,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_LOCK,
    COMPO_ID_PIC_BAT,

    COMPO_ID_TAB0_SEL_BG,
    COMPO_ID_TAB0_BORDER_OUT,
    COMPO_ID_TAB0_BORDER_IN,
    COMPO_ID_TAB0_BTN,
    COMPO_ID_TAB0_LABEL,
    COMPO_ID_TAB0_LINE,
    COMPO_ID_TAB0_PIC,

    COMPO_ID_TAB1_SEL_BG,
    COMPO_ID_TAB1_BORDER_OUT,
    COMPO_ID_TAB1_BORDER_IN,
    COMPO_ID_TAB1_BTN,
    COMPO_ID_TAB1_LABEL,
    COMPO_ID_TAB1_LINE,
    COMPO_ID_TAB1_PIC,

    COMPO_ID_TAB2_SEL_BG,
    COMPO_ID_TAB2_BORDER_OUT,
    COMPO_ID_TAB2_BORDER_IN,
    COMPO_ID_TAB2_BTN,
    COMPO_ID_TAB2_LABEL,
    COMPO_ID_TAB2_LINE,
    COMPO_ID_TAB2_PIC,
};

typedef struct mode_tab_ui_t_ {
    compo_shape_t *sel_bg;
    compo_shape_t *border_out;
    compo_shape_t *border_in;
    compo_picturebox_t *pic;
    compo_picturebox_t *pic_dash;
    compo_button_t *btn;
    compo_textbox_t *label;
} mode_tab_ui_t;

typedef struct f_mode_t_ {
    u8 tab;
    u8 last_top_min;
    u8 last_top_sec;
    u16 last_timer_key;
    u16 last_temp_f;
    compo_textbox_t *txt_top_time;
    compo_picturebox_t *pic_status_temp[MODE_TEMP_IDX_CNT];
    compo_picturebox_t *pic_status_temp_degf;
    compo_picturebox_t *pic_timer[MODE_TIMER_IDX_CNT];
    compo_picturebox_t *pic_timer_colon;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
    mode_tab_ui_t tabs[MODE_TAB_CNT];
} f_mode_t;

static u8 mode_status_temp_digit_ram[MODE_TEMP_IDX_CNT][HOME_DIGIT_GREEN_T_RAM_MAX_SIZE];
static u8 mode_status_degf_gr_ram[HOME_STATUS_TEMPF_GR_T_RAM_SIZE];

static u32 mode_countdown_remain_sec;
static bool mode_countdown_running;

static const u32 tbl_mode_digit_white_addr[10] = {
    UI_BUF_HOME_0_BIN, UI_BUF_HOME_1_BIN, UI_BUF_HOME_2_BIN, UI_BUF_HOME_3_BIN,
    UI_BUF_HOME_4_BIN, UI_BUF_HOME_5_BIN, UI_BUF_HOME_6_BIN, UI_BUF_HOME_7_BIN,
    UI_BUF_HOME_8_BIN, UI_BUF_HOME_9_BIN,
};



static const u16 tbl_mode_digit_white_len[10] = {
    UI_LEN_HOME_0_BIN, UI_LEN_HOME_1_BIN, UI_LEN_HOME_2_BIN, UI_LEN_HOME_3_BIN,
    UI_LEN_HOME_4_BIN, UI_LEN_HOME_5_BIN, UI_LEN_HOME_6_BIN, UI_LEN_HOME_7_BIN,
    UI_LEN_HOME_8_BIN, UI_LEN_HOME_9_BIN,
};



static const u32 tbl_mode_icon_sel_addr[MODE_TAB_CNT] = {
    UI_BUF_HOME_PASTA_SEL_BIN,
    UI_BUF_HOME_CHICKEN_SEL_BIN,
    UI_BUF_HOME_WARM_SEL_BIN,
};

static const u32 tbl_mode_icon_nor_addr[MODE_TAB_CNT] = {
    UI_BUF_HOME_PASTA_NOR_BIN,
    UI_BUF_HOME_CHICKEN_NOR_BIN,
    UI_BUF_HOME_WARM_NOR_BIN,
};

static const u16 tbl_mode_icon_sel_len[MODE_TAB_CNT] = {
    UI_LEN_HOME_PASTA_SEL_BIN,
    UI_LEN_HOME_CHICKEN_SEL_BIN,
    UI_LEN_HOME_WARM_SEL_BIN,
};

static const u16 tbl_mode_icon_nor_len[MODE_TAB_CNT] = {
    UI_LEN_HOME_PASTA_NOR_BIN,
    UI_LEN_HOME_CHICKEN_NOR_BIN,
    UI_LEN_HOME_WARM_NOR_BIN,
};

static const u16 tbl_mode_tab_pic_id[MODE_TAB_CNT] = {
    COMPO_ID_TAB0_PIC,
    COMPO_ID_TAB1_PIC,
    COMPO_ID_TAB2_PIC,
};

static const u16 tbl_mode_tab_dash_id[MODE_TAB_CNT] = {
    COMPO_ID_TAB0_LINE,
    COMPO_ID_TAB1_LINE,
    COMPO_ID_TAB2_LINE,
};

static const u16 tbl_mode_timer_id[MODE_TIMER_IDX_CNT] = {
    COMPO_ID_PIC_TIMER_H10, COMPO_ID_PIC_TIMER_H1,
    COMPO_ID_PIC_TIMER_M10, COMPO_ID_PIC_TIMER_M1,
};

static const u16 tbl_mode_status_temp_id[MODE_TEMP_IDX_CNT] = {
    COMPO_ID_PIC_STATUS_TEMP_H,
    COMPO_ID_PIC_STATUS_TEMP_T10,
    COMPO_ID_PIC_STATUS_TEMP_T1,
};

static const char * const tbl_mode_tab_label[MODE_TAB_CNT] = {
    "Pasta",
    "Chicken",
    "Warm",
};

static const s16 tbl_mode_tab_x[MODE_TAB_CNT] = {
    MODE_TAB_X0,
    MODE_TAB_X1,
    MODE_TAB_X2,
};

typedef struct {
    u16 temp_f;
    u8 hour;
    u8 min;
} mode_tab_preset_t;

/* 底部 Tab 选中时对应的固定温度(°F)与倒计时 */
static const mode_tab_preset_t tbl_mode_tab_preset[MODE_TAB_CNT] = {
    {194, 1, 0},   /* Pasta   */
    {212, 1, 0},   /* Chicken */
    {120, 1, 0},   /* Warm    */
};

static void func_mode_status_temp_update(f_mode_t *f_mode, u16 temp_f);
static void func_mode_timer_update(f_mode_t *f_mode, u8 hour, u8 min);
static void func_mode_display_refresh(f_mode_t *f_mode);
static void func_mode_tab_preset_apply(f_mode_t *f_mode, u8 tab);

static void func_mode_dash_runtime_init(void)
{
    home_ui_shared_dash_init();
}

static void func_mode_status_icons_init(void)
{
    home_ui_shared_status_init();
}

static void func_mode_status_icons_apply(f_mode_t *f_mode)
{
    func_mode_status_icons_init();

    if (f_mode->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f_mode->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f_mode->pic_bt, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
    }
    if (f_mode->pic_lock != NULL && gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
        compo_picturebox_set_ram(f_mode->pic_lock, home_ui_shared_status_lock_ram);
        compo_picturebox_set_size(f_mode->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    }
    if (f_mode->pic_bat != NULL && gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        compo_picturebox_set_ram(f_mode->pic_bat, home_ui_shared_status_bat_ram);
        compo_picturebox_set_size(f_mode->pic_bat, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
    }
}

void func_mode_countdown_set(u8 hour, u8 min)
{
    if (hour > 99) {
        hour = 99;
    }
    if (min > 59) {
        min = 59;
    }
    mode_countdown_remain_sec = (u32)hour * 3600 + (u32)min * 60;
}

void func_mode_countdown_start(void)
{
    mode_countdown_running = true;
}

void func_mode_countdown_stop(void)
{
    mode_countdown_running = false;
}

void func_mode_temp_set_f(u16 temp_f)
{
    f_mode_t *f_mode = (f_mode_t *)func_cb.f_cb;

    if (f_mode != NULL) {
        if (temp_f > 999) {
            temp_f = 999;
        }
        if (f_mode->last_temp_f != temp_f) {
            f_mode->last_temp_f = temp_f;
            func_mode_status_temp_update(f_mode, temp_f);
        }
    }
}

static void func_mode_countdown_tick(void)
{
    if (mode_countdown_running && mode_countdown_remain_sec > 0) {
        mode_countdown_remain_sec--;
        if (mode_countdown_remain_sec == 0) {
            mode_countdown_running = false;
        }
    }
}

static void func_mode_time_str_ampm(char *buf, u16 buf_len, tm_t *tm)
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

static void func_mode_status_temp_layout(f_mode_t *f_mode)
{
    u16 total;
    s16 x;
    u8 i;

    total = MODE_STATUS_TEMP_DIGIT_W * MODE_TEMP_IDX_CNT
          + MODE_STATUS_TEMP_GAP * (MODE_TEMP_IDX_CNT - 1)
          + MODE_STATUS_TEMP_SYMBOL_GAP + MODE_STATUS_TEMPF_W;
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2) - 20;

    for (i = 0; i < MODE_TEMP_IDX_CNT; i++) {
        s16 cx = x + (s16)(MODE_STATUS_TEMP_DIGIT_W / 2);

        compo_picturebox_set_pos(f_mode->pic_status_temp[i], cx, MODE_STATUS_TEMP_Y);
        compo_picturebox_set_size(f_mode->pic_status_temp[i],
                                 MODE_STATUS_TEMP_DIGIT_W, MODE_STATUS_TEMP_DIGIT_H);
        x += MODE_STATUS_TEMP_DIGIT_W + MODE_STATUS_TEMP_GAP;
    }

    if (f_mode->pic_status_temp_degf != NULL) {
        s16 cx = x + (s16)(MODE_STATUS_TEMPF_W / 2);

        compo_picturebox_set_pos(f_mode->pic_status_temp_degf, cx, MODE_STATUS_TEMP_Y);
        compo_picturebox_set_size(f_mode->pic_status_temp_degf,
                                  MODE_STATUS_TEMPF_W, MODE_STATUS_TEMPF_H);
    }
}

static void func_mode_status_temp_update(f_mode_t *f_mode, u16 temp_f)
{
    u8 digits[MODE_TEMP_IDX_CNT];
    u8 i;

    if (temp_f > 999) {
        temp_f = 999;
    }

    digits[MODE_TEMP_IDX_H] = (u8)(temp_f / 100);
    digits[MODE_TEMP_IDX_T10] = (u8)((temp_f / 10) % 10);
    digits[MODE_TEMP_IDX_T1] = (u8)(temp_f % 10);

    os_spiflash_read(mode_status_degf_gr_ram, UI_BUF_HOME_DEGF_GR_T_BIN, UI_LEN_HOME_DEGF_GR_T_BIN);
    if (f_mode->pic_status_temp_degf != NULL && gui_set_ram_check(mode_status_degf_gr_ram, __func__)) {
        compo_picturebox_set_ram(f_mode->pic_status_temp_degf, mode_status_degf_gr_ram);
    }

    for (i = 0; i < MODE_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];

        // os_spiflash_read(mode_status_temp_digit_ram[i],
        //                  tbl_mode_digit_green_addr[d], tbl_mode_digit_green_len[d]);
        if (gui_set_ram_check(mode_status_temp_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_mode->pic_status_temp[i], mode_status_temp_digit_ram[i]);
        }
    }

    func_mode_status_temp_layout(f_mode);
}

static void func_mode_timer_layout(f_mode_t *f_mode)
{
    u16 total;
    s16 x;
    u8 i;

    total = HOME_DIGIT_W + MODE_DIGIT_GAP + HOME_DIGIT_W + MODE_DIGIT_GAP
          + HOME_COLON_W + MODE_DIGIT_GAP + HOME_DIGIT_W + MODE_DIGIT_GAP
          + HOME_DIGIT_W;
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2);

    for (i = 0; i < MODE_TIMER_IDX_CNT; i++) {
        s16 cx = x + (s16)(HOME_DIGIT_W / 2);

        compo_picturebox_set_pos(f_mode->pic_timer[i], cx, MODE_TIMER_Y);
        compo_picturebox_set_size(f_mode->pic_timer[i], HOME_DIGIT_W, HOME_DIGIT_H);
        x += HOME_DIGIT_W + MODE_DIGIT_GAP;
        if (i == MODE_TIMER_IDX_H1) {
            cx = x + (s16)(HOME_COLON_W / 2);
            compo_picturebox_set_pos(f_mode->pic_timer_colon, cx, MODE_TIMER_Y);
            compo_picturebox_set_size(f_mode->pic_timer_colon, HOME_COLON_W, HOME_COLON_H);
            x += HOME_COLON_W + MODE_DIGIT_GAP;
        }
    }
}

static void func_mode_timer_update(f_mode_t *f_mode, u8 hour, u8 min)
{
    u8 digits[MODE_TIMER_IDX_CNT];
    u16 timer_key;
    u8 i;

    digits[MODE_TIMER_IDX_H10] = hour / 10;
    digits[MODE_TIMER_IDX_H1] = hour % 10;
    digits[MODE_TIMER_IDX_M10] = min / 10;
    digits[MODE_TIMER_IDX_M1] = min % 10;
    timer_key = (u16)hour * 100 + min;
    if (f_mode->last_timer_key == timer_key) {
        return;
    }
    f_mode->last_timer_key = timer_key;

    os_spiflash_read(home_ui_colon_ram, UI_BUF_HOME_COLON_BIN, UI_LEN_HOME_COLON_BIN);
    if (gui_set_ram_check(home_ui_colon_ram, __func__)) {
        compo_picturebox_set_ram(f_mode->pic_timer_colon, home_ui_colon_ram);
    }

    for (i = 0; i < MODE_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];

        os_spiflash_read(home_ui_digit_ram[i], tbl_mode_digit_white_addr[d], tbl_mode_digit_white_len[d]);
        if (gui_set_ram_check(home_ui_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_mode->pic_timer[i], home_ui_digit_ram[i]);
        }
    }

    func_mode_timer_layout(f_mode);
}

static void func_mode_display_refresh(f_mode_t *f_mode)
{
    u8 hour = (u8)(mode_countdown_remain_sec / 3600);
    u8 min = (u8)((mode_countdown_remain_sec % 3600) / 60);

    if (hour > 99) {
        hour = 99;
    }
    func_mode_timer_update(f_mode, hour, min);
}

static void func_mode_tab_preset_apply(f_mode_t *f_mode, u8 tab)
{
    const mode_tab_preset_t *preset;

    if (f_mode == NULL || tab >= MODE_TAB_CNT) {
        return;
    }

    preset = &tbl_mode_tab_preset[tab];
    func_mode_countdown_set(preset->hour, preset->min);
    func_mode_countdown_start();

    f_mode->last_timer_key = 0xffff;
    func_mode_display_refresh(f_mode);

    if (f_mode->last_temp_f != preset->temp_f) {
        f_mode->last_temp_f = preset->temp_f;
        func_mode_status_temp_update(f_mode, preset->temp_f);
    }
}

static compo_shape_t *func_mode_shape_create(compo_form_t *frm, u16 id, s16 x, s16 y,
                                               s16 w, s16 h, u16 color, u16 radius)
{
    compo_shape_t *shape = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(shape, id);
    compo_shape_set_location(shape, x, y, w, h);
    compo_shape_set_color(shape, color);
    compo_shape_set_radius(shape, radius);
    compo_shape_set_visible(shape, false);
    return shape;
}

static void func_mode_tab_icon_update(f_mode_t *f_mode, u8 idx)
{
    bool selected = (idx == f_mode->tab);
    u32 addr = selected ? tbl_mode_icon_sel_addr[idx] : tbl_mode_icon_nor_addr[idx];
    u16 len = selected ? tbl_mode_icon_sel_len[idx] : tbl_mode_icon_nor_len[idx];

    if (f_mode->tabs[idx].pic == NULL) {
        return;
    }

    os_spiflash_read(home_ui_shared_icon_runtime[idx], addr, len);
    if (gui_set_ram_check(home_ui_shared_icon_runtime[idx], __func__)) {
        compo_picturebox_set_ram(f_mode->tabs[idx].pic, home_ui_shared_icon_runtime[idx]);
    }
}

static void func_mode_tab_dash_update(f_mode_t *f_mode, u8 idx)
{
    bool selected = (idx == f_mode->tab);
    const u8 *src = selected ? home_ui_shared_dash_runtime_sel : home_ui_shared_dash_runtime_nor;

    if (f_mode->tabs[idx].pic_dash == NULL) {
        return;
    }

    if (gui_set_ram_check((void *)src, __func__)) {
        compo_picturebox_set_ram(f_mode->tabs[idx].pic_dash, src);
    }
}

static void func_mode_tab_create(compo_form_t *frm, u8 idx, u16 id_base, const char *label, s16 x)
{
    compo_shape_t *sel_bg;
    compo_shape_t *border_out;
    compo_shape_t *border_in;
    compo_picturebox_t *pic;
    compo_picturebox_t *pic_dash;
    compo_button_t *btn;
    compo_textbox_t *txt;

    sel_bg = func_mode_shape_create(frm, id_base + 0, x, MODE_TAB_BTN_Y,
                                    MODE_TAB_BTN_W, MODE_TAB_BTN_H, MODE_COLOR_BLUE, MODE_TAB_BTN_R);
    border_out = func_mode_shape_create(frm, id_base + 1, x, MODE_TAB_BTN_Y,
                                        MODE_TAB_BTN_W, MODE_TAB_BTN_H, COLOR_WHITE, MODE_TAB_BTN_R);
    border_in = func_mode_shape_create(frm, id_base + 2, x, MODE_TAB_BTN_Y,
                                       MODE_TAB_BTN_W - 4, MODE_TAB_BTN_H - 4, COLOR_BLACK, MODE_TAB_BTN_R_IN);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, tbl_mode_tab_pic_id[idx]);
    os_spiflash_read(home_ui_shared_icon_runtime[idx], tbl_mode_icon_nor_addr[idx], tbl_mode_icon_nor_len[idx]);
    if (gui_set_ram_check(home_ui_shared_icon_runtime[idx], __func__)) {
        compo_picturebox_set_ram(pic, home_ui_shared_icon_runtime[idx]);
    }
    compo_picturebox_set_pos(pic, x, MODE_TAB_ICON_Y);
    compo_picturebox_set_size(pic, MODE_TAB_ICON_SIZE, MODE_TAB_ICON_SIZE);

    txt = compo_textbox_create(frm, 8);
    compo_setid(txt, id_base + 4);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_pos(txt, x, MODE_TAB_LABEL_Y);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, label);

    pic_dash = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic_dash, tbl_mode_tab_dash_id[idx]);
    func_mode_dash_runtime_init();
    if (gui_set_ram_check(home_ui_shared_dash_runtime_nor, __func__)) {
        compo_picturebox_set_ram(pic_dash, home_ui_shared_dash_runtime_nor);
    }
    compo_picturebox_set_pos(pic_dash, x, MODE_TAB_DASH_Y);
    compo_picturebox_set_size(pic_dash, HOME_DASH_RAM_W, HOME_DASH_RAM_H);

    btn = compo_button_create(frm);
    compo_setid(btn, id_base + 3);
    compo_button_set_location(btn, x, MODE_TAB_BTN_Y, MODE_TAB_BTN_W, MODE_TAB_BTN_H);
}

static void func_mode_tab_bind(f_mode_t *f_mode, u8 idx, u16 id_base)
{
    mode_tab_ui_t *tab = &f_mode->tabs[idx];

    tab->sel_bg = compo_getobj_byid(id_base + 0);
    tab->border_out = compo_getobj_byid(id_base + 1);
    tab->border_in = compo_getobj_byid(id_base + 2);
    tab->pic = compo_getobj_byid(tbl_mode_tab_pic_id[idx]);
    tab->pic_dash = compo_getobj_byid(tbl_mode_tab_dash_id[idx]);
    tab->btn = compo_getobj_byid(id_base + 3);
    tab->label = compo_getobj_byid(id_base + 4);
}

static void func_mode_tab_refresh(f_mode_t *f_mode)
{
    u8 i;

    for (i = 0; i < MODE_TAB_CNT; i++) {
        mode_tab_ui_t *tab = &f_mode->tabs[i];
        bool selected = (i == f_mode->tab);

        compo_shape_set_visible(tab->sel_bg, selected);
        compo_shape_set_visible(tab->border_out, !selected);
        compo_shape_set_visible(tab->border_in, !selected);
        func_mode_tab_icon_update(f_mode, i);
        func_mode_tab_dash_update(f_mode, i);
    }
}

static void func_mode_status_refresh(f_mode_t *f_mode)
{
    char str[16];
    tm_t tm = rtc_clock_get();

    if (f_mode->last_top_min != tm.min || f_mode->last_top_sec != tm.sec) {
        f_mode->last_top_min = tm.min;
        f_mode->last_top_sec = tm.sec;
        func_mode_time_str_ampm(str, sizeof(str), &tm);
        compo_textbox_set(f_mode->txt_top_time, str);
        func_mode_countdown_tick();
        func_mode_display_refresh(f_mode);
    }
}

static void func_mode_button_click(f_mode_t *f_mode)
{
    int id = compo_get_button_id();

    switch (id) {
    case COMPO_ID_TAB0_BTN:
        f_mode->tab = MODE_TAB_PASTA;
        func_mode_tab_preset_apply(f_mode, MODE_TAB_PASTA);
        func_mode_tab_refresh(f_mode);
        break;

    case COMPO_ID_TAB1_BTN:
        f_mode->tab = MODE_TAB_CHICKEN;
        func_mode_tab_preset_apply(f_mode, MODE_TAB_CHICKEN);
        func_mode_tab_refresh(f_mode);
        break;

    case COMPO_ID_TAB2_BTN:
        f_mode->tab = MODE_TAB_WARM;
        func_mode_tab_preset_apply(f_mode, MODE_TAB_WARM);
        func_mode_tab_refresh(f_mode);
        break;

    default:
        break;
    }
}

compo_form_t *func_mode_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    compo_picturebox_t *pic;
    char str[16];
    tm_t tm = rtc_clock_get();
    u8 i;

    func_mode_time_str_ampm(str, sizeof(str), &tm);
    txt = compo_textbox_create(frm, 16);
    compo_setid(txt, COMPO_ID_TXT_TOP_TIME);
    compo_textbox_set_pos(txt, MODE_STATUS_TIME_X, MODE_STATUS_Y);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, str);

    for (i = 0; i < MODE_TEMP_IDX_CNT; i++) {
        pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
        compo_setid(pic, tbl_mode_status_temp_id[i]);
        compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, MODE_STATUS_TEMP_Y);
        compo_picturebox_set_size(pic, MODE_STATUS_TEMP_DIGIT_W, MODE_STATUS_TEMP_DIGIT_H);
    }

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_STATUS_TEMPF);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, MODE_STATUS_TEMP_Y);
    compo_picturebox_set_size(pic, MODE_STATUS_TEMPF_W, MODE_STATUS_TEMPF_H);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, MODE_STATUS_BT_X, MODE_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BT_W, HOME_STATUS_BT_H);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_LOCK);
    compo_picturebox_set_pos(pic, MODE_STATUS_LOCK_X, MODE_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, MODE_STATUS_BAT_X, MODE_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);

    for (i = 0; i < MODE_TIMER_IDX_CNT; i++) {
        pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
        compo_setid(pic, tbl_mode_timer_id[i]);
        compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, MODE_TIMER_Y);
        compo_picturebox_set_size(pic, HOME_DIGIT_MAX_W, HOME_DIGIT_H);
    }

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_COLON);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, MODE_TIMER_Y);
    compo_picturebox_set_size(pic, HOME_COLON_W, HOME_COLON_H);

    func_mode_tab_create(frm, MODE_TAB_PASTA, COMPO_ID_TAB0_SEL_BG,
                         tbl_mode_tab_label[MODE_TAB_PASTA], tbl_mode_tab_x[MODE_TAB_PASTA]);
    func_mode_tab_create(frm, MODE_TAB_CHICKEN, COMPO_ID_TAB1_SEL_BG,
                         tbl_mode_tab_label[MODE_TAB_CHICKEN], tbl_mode_tab_x[MODE_TAB_CHICKEN]);
    func_mode_tab_create(frm, MODE_TAB_WARM, COMPO_ID_TAB2_SEL_BG,
                         tbl_mode_tab_label[MODE_TAB_WARM], tbl_mode_tab_x[MODE_TAB_WARM]);

    return frm;
}

static void func_mode_process(void)
{
    f_mode_t *f_mode = (f_mode_t *)func_cb.f_cb;

    if (f_mode != NULL) {
        func_mode_status_refresh(f_mode);
    }
    func_process();
}

static void func_mode_message(size_msg_t msg)
{
    f_mode_t *f_mode = (f_mode_t *)func_cb.f_cb;

    switch (msg) {
    case MSG_CTP_CLICK:
        func_mode_button_click(f_mode);
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_mode_enter(void)
{
    f_mode_t *f_mode;

    func_cb.f_cb = func_zalloc(sizeof(f_mode_t));
    func_cb.frm_main = func_mode_form_create();

    f_mode = (f_mode_t *)func_cb.f_cb;
    f_mode->tab = MODE_TAB_PASTA;
    f_mode->last_top_min = 0xff;
    f_mode->last_top_sec = 0xff;
    f_mode->last_timer_key = 0xffff;
    f_mode->last_temp_f = 0xffff;

    f_mode->txt_top_time = compo_getobj_byid(COMPO_ID_TXT_TOP_TIME);
    for (u8 i = 0; i < MODE_TEMP_IDX_CNT; i++) {
        f_mode->pic_status_temp[i] = compo_getobj_byid(tbl_mode_status_temp_id[i]);
    }
    f_mode->pic_status_temp_degf = compo_getobj_byid(COMPO_ID_PIC_STATUS_TEMPF);
    for (u8 i = 0; i < MODE_TIMER_IDX_CNT; i++) {
        f_mode->pic_timer[i] = compo_getobj_byid(tbl_mode_timer_id[i]);
    }
    f_mode->pic_timer_colon = compo_getobj_byid(COMPO_ID_PIC_TIMER_COLON);
    f_mode->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f_mode->pic_lock = compo_getobj_byid(COMPO_ID_PIC_LOCK);
    f_mode->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);

    func_mode_tab_bind(f_mode, MODE_TAB_PASTA, COMPO_ID_TAB0_SEL_BG);
    func_mode_tab_bind(f_mode, MODE_TAB_CHICKEN, COMPO_ID_TAB1_SEL_BG);
    func_mode_tab_bind(f_mode, MODE_TAB_WARM, COMPO_ID_TAB2_SEL_BG);

    func_mode_status_icons_apply(f_mode);
    func_mode_tab_preset_apply(f_mode, f_mode->tab);
    func_mode_tab_refresh(f_mode);
    func_mode_status_refresh(f_mode);
}

void func_mode_exit(void)
{
    func_cb.last = FUNC_MODE;
}

void func_mode(void)
{
    printf("%s\n", __func__);
    func_mode_enter();
    while (func_cb.sta == FUNC_MODE) {
        func_mode_process();
        func_mode_message(msg_dequeue());
    }
    func_mode_exit();
}
