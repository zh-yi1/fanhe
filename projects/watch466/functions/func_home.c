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
 * Home 位图：tools/gen_home_icons.py -> Output/bin/ui/home/*.bin -> prebuild -> ui.bin
 * 全部 home/*.bin 为 GPU 格式(0x24150)：必须 os_spiflash_read + set_ram，不可 set 直引 Flash。
 *
 * 左上角 RTC：0m.png..9m.png、colonm.png、AMm.png、PMm.png -> 同名 .bin -> ui.bin
 *   home_top_time.c：os_spiflash_read + compo_picturebox_set_ram
 *
 * 中部倒计时 HH:MM（4 位数字 + 冒号）：
 *   PNG 源：Output/bin/ui/home/0.png..9.png、colon.png
 *   Flash：UI_BUF_HOME_0_BIN..9_BIN、UI_BUF_HOME_COLON_BIN（ui.bin）
 *   RAM：home_ui_digit_ram[4]、home_ui_colon_ram -> func_home_clock_update()
 *
 * 右上角状态栏（蓝牙 / 锁 / 电量）：
 *   PNG 源：Output/bin/ui/home/bluetooth.png、lock.png、battery_level.png
 *   Flash：UI_BUF_HOME_BLUETOOTH_BIN、UI_BUF_HOME_LOCK_BIN、UI_BUF_HOME_BATTERY_LEVEL_BIN
 *   RAM：home_ui_shared_status_*_ram -> home_ui_shared_status_init() + func_home_status_icons_apply()
 *
 * 底部导航 Tab（HEAT / MODE / SETUP）：
 *   选中：蓝底 + 白框 + 白底线 + 白图标（*_sel.bin）
 *   未选中：黑底 + 白框 + 蓝底线 + 白图标（*.bin）
 */
#define UI_HOME_ICON_PLACEHOLDER          UI_BUF_ICON_ACTIVITY_BIN

#ifndef UI_BUF_HOME_0_BIN
#error "Run tools/gen_home_icons.py then Output/bin/prebuild.bat to refresh ui.h"
#endif

#ifndef UI_BUF_HOME_COLON_BIN
#error "Missing colon.bin: add ui/home/colon.png and run gen_home_icons.py + prebuild.bat"
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

#ifndef UI_BUF_HOME_HEAT_BIN
#error "Missing heat.bin: add ui/home/heat.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_MODE_BIN
#error "Missing mode.bin: add ui/home/mode.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_SETUP_BIN
#error "Missing setup.bin: add ui/home/setting.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_HEAT_SEL_BIN
#error "Missing heat_sel.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_MODE_SEL_BIN
#error "Missing mode_sel.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_SETUP_SEL_BIN
#error "Missing setup_sel.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_WHILE_LINE_BIN
#error "Missing while_line.bin: add ui/home/while_line.png and run gen_mode_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BLUE_LINE_BIN
#error "Missing blue_line.bin: add ui/home/blue_line.png and run gen_mode_icons.py + prebuild.bat"
#endif

#define HOME_COLOR_BLUE                 make_color(4, 109, 217)

#define HOME_TAB_BTN_W                  118
#define HOME_TAB_BTN_H                  96
#define HOME_TAB_BTN_R                  16
#define HOME_TAB_BTN_R_IN               14
#define HOME_TAB_BTN_Y                  390
#define HOME_TAB_PAD_TOP                12
#define HOME_TAB_PAD_BOTTOM             10
#define HOME_TAB_PAD_MID                6
#define HOME_TAB_FONT_H                 8
#define HOME_TAB_LINE_H                 3
#define HOME_TAB_ICON_Y                 (HOME_TAB_BTN_Y - HOME_TAB_BTN_H / 2 + HOME_TAB_PAD_TOP + HOME_NAV_ICON_MAX_H / 2)
#define HOME_TAB_LABEL_Y                (HOME_TAB_ICON_Y + HOME_NAV_ICON_MAX_H / 2 + HOME_TAB_PAD_MID + HOME_TAB_FONT_H / 2)
#define HOME_TAB_DASH_Y                 (HOME_TAB_BTN_Y + HOME_TAB_BTN_H / 2 - HOME_TAB_PAD_BOTTOM - HOME_TAB_LINE_H / 2)
#define HOME_TAB_DASH_W                 HOME_DASH_RAM_W
#define HOME_TAB_X0                     87
#define HOME_TAB_X1                     233
#define HOME_TAB_X2                     379

#define HOME_STATUS_Y                   48
#define HOME_STATUS_RIGHT_MARGIN        24
#define HOME_STATUS_GAP                 10
#define HOME_STATUS_BAT_X               (GUI_SCREEN_WIDTH - HOME_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define HOME_STATUS_LOCK_X              (HOME_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - HOME_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define HOME_STATUS_BT_X                (HOME_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - HOME_STATUS_GAP - HOME_STATUS_BT_W / 2)
#define HOME_STATUS_H                   36
#define HOME_CLOCK_Y                    ((HOME_STATUS_Y + HOME_STATUS_H + HOME_TAB_BTN_Y - HOME_TAB_BTN_H / 2) / 2)
#define HOME_CLOCK_GAP                  4

enum {
    HOME_CLOCK_IDX_H10 = 0,
    HOME_CLOCK_IDX_H1,
    HOME_CLOCK_IDX_M10,
    HOME_CLOCK_IDX_M1,
    HOME_CLOCK_IDX_CNT,
};

enum {
    HOME_TAB_HEAT = 0,
    HOME_TAB_MODE,
    HOME_TAB_SETUP,
    HOME_TAB_CNT,
};

enum {
    COMPO_ID_PIC_TOP_TIME_H10 = 1,
    COMPO_ID_PIC_TOP_TIME_H1,
    COMPO_ID_PIC_TOP_TIME_COLON,
    COMPO_ID_PIC_TOP_TIME_M10,
    COMPO_ID_PIC_TOP_TIME_M1,
    COMPO_ID_PIC_TOP_TIME_AMPM,
    COMPO_ID_PIC_CLOCK_H10,
    COMPO_ID_PIC_CLOCK_H1,
    COMPO_ID_PIC_CLOCK_COLON,
    COMPO_ID_PIC_CLOCK_M10,
    COMPO_ID_PIC_CLOCK_M1,
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

typedef struct home_tab_ui_t_ {
    compo_shape_t *sel_bg;
    compo_shape_t *border_out;
    compo_shape_t *border_in;
    compo_shape_t *line;
    compo_picturebox_t *pic;
    compo_button_t *btn;
    compo_textbox_t *label;
} home_tab_ui_t;

typedef struct f_home_t_ {
    u8 tab;
    u8 last_top_min;
    u8 last_top_sec;
    u16 last_cd_total_min;
    home_top_time_ui_t top_time;
    compo_picturebox_t *pic_clock[HOME_CLOCK_IDX_CNT];
    compo_picturebox_t *pic_clock_colon;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
    home_tab_ui_t tabs[HOME_TAB_CNT];
} f_home_t;

static u32 home_countdown_remain_sec;
static bool home_countdown_running;

static const u32 tbl_home_digit_addr[10] = {
    UI_BUF_HOME_0_BIN,
    UI_BUF_HOME_1_BIN,
    UI_BUF_HOME_2_BIN,
    UI_BUF_HOME_3_BIN,
    UI_BUF_HOME_4_BIN,
    UI_BUF_HOME_5_BIN,
    UI_BUF_HOME_6_BIN,
    UI_BUF_HOME_7_BIN,
    UI_BUF_HOME_8_BIN,
    UI_BUF_HOME_9_BIN,
};

static const u16 tbl_home_digit_len[10] = {
    UI_LEN_HOME_0_BIN, UI_LEN_HOME_1_BIN, UI_LEN_HOME_2_BIN, UI_LEN_HOME_3_BIN,
    UI_LEN_HOME_4_BIN, UI_LEN_HOME_5_BIN, UI_LEN_HOME_6_BIN, UI_LEN_HOME_7_BIN,
    UI_LEN_HOME_8_BIN, UI_LEN_HOME_9_BIN,
};

static const u32 tbl_home_nav_icon_addr_nor[HOME_TAB_CNT] = {
    UI_BUF_HOME_HEAT_BIN,
    UI_BUF_HOME_MODE_BIN,
    UI_BUF_HOME_SETUP_BIN,
};

static const u32 tbl_home_nav_icon_addr_sel[HOME_TAB_CNT] = {
    UI_BUF_HOME_HEAT_SEL_BIN,
    UI_BUF_HOME_MODE_SEL_BIN,
    UI_BUF_HOME_SETUP_SEL_BIN,
};

static const u16 tbl_home_nav_icon_len[HOME_TAB_CNT] = {
    UI_LEN_HOME_HEAT_BIN,
    UI_LEN_HOME_MODE_BIN,
    UI_LEN_HOME_SETUP_BIN,
};

static const u16 tbl_home_nav_icon_len_sel[HOME_TAB_CNT] = {
    UI_LEN_HOME_HEAT_SEL_BIN,
    UI_LEN_HOME_MODE_SEL_BIN,
    UI_LEN_HOME_SETUP_SEL_BIN,
};

static const u16 tbl_home_nav_icon_w[HOME_TAB_CNT] = {
    HOME_NAV_HEAT_W,
    HOME_NAV_MODE_W,
    HOME_NAV_SETUP_W,
};

static const u16 tbl_home_nav_icon_h[HOME_TAB_CNT] = {
    HOME_NAV_HEAT_H,
    HOME_NAV_MODE_H,
    HOME_NAV_SETUP_H,
};

static const u16 tbl_home_tab_pic_id[HOME_TAB_CNT] = {
    COMPO_ID_TAB0_PIC,
    COMPO_ID_TAB1_PIC,
    COMPO_ID_TAB2_PIC,
};

static const u16 tbl_home_tab_dash_id[HOME_TAB_CNT] = {
    COMPO_ID_TAB0_LINE,
    COMPO_ID_TAB1_LINE,
    COMPO_ID_TAB2_LINE,
};

static const u16 tbl_home_clock_id[HOME_CLOCK_IDX_CNT] = {
    COMPO_ID_PIC_CLOCK_H10,
    COMPO_ID_PIC_CLOCK_H1,
    COMPO_ID_PIC_CLOCK_M10,
    COMPO_ID_PIC_CLOCK_M1,
};


static void func_home_status_icons_init(void)
{
    home_ui_shared_status_init();
}

static void func_home_status_icons_apply(f_home_t *f_home)
{
    func_home_status_icons_init();

    if (f_home->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f_home->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f_home->pic_bt, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
    }
    if (f_home->pic_lock != NULL && gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
        compo_picturebox_set_ram(f_home->pic_lock, home_ui_shared_status_lock_ram);
        compo_picturebox_set_size(f_home->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    }
    if (f_home->pic_bat != NULL && gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        compo_picturebox_set_ram(f_home->pic_bat, home_ui_shared_status_bat_ram);
        compo_picturebox_set_size(f_home->pic_bat, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
    }
}

void func_home_countdown_set(u8 hour, u8 min)
{
    if (hour > 99) {
        hour = 99;
    }
    if (min > 59) {
        min = 59;
    }
    home_countdown_remain_sec = (u32)hour * 3600 + (u32)min * 60;
}

void func_home_countdown_start(void)
{
    home_countdown_running = true;
}

void func_home_countdown_stop(void)
{
    home_countdown_running = false;
}

u32 func_home_countdown_remain_sec(void)
{
    return home_countdown_remain_sec;
}

static void func_home_countdown_get_display(u8 *hour, u8 *min)
{
    u32 sec = home_countdown_remain_sec;
    u32 h = sec / 3600;

    if (h > 99) {
        h = 99;
    }
    *hour = (u8)h;
    *min = (u8)((sec % 3600) / 60);
}

static void func_home_countdown_tick(void)
{
    if (home_countdown_running && home_countdown_remain_sec > 0) {
        home_countdown_remain_sec--;
        if (home_countdown_remain_sec == 0) {
            home_countdown_running = false;
        }
    }
}

static void func_home_clock_layout(f_home_t *f_home, u8 hour, u8 min)
{
    u8 digits[HOME_CLOCK_IDX_CNT] = { hour / 10, hour % 10, min / 10, min % 10 };
    s16 x;
    u16 total;
    u8 i;

    total = HOME_DIGIT_W + HOME_CLOCK_GAP + HOME_DIGIT_W + HOME_CLOCK_GAP
          + HOME_COLON_W + HOME_CLOCK_GAP + HOME_DIGIT_W + HOME_CLOCK_GAP
          + HOME_DIGIT_W;
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2);

    for (i = 0; i < HOME_CLOCK_IDX_CNT; i++) {
        s16 cx = x + (s16)(HOME_DIGIT_W / 2);

        compo_picturebox_set_pos(f_home->pic_clock[i], cx, HOME_CLOCK_Y);
        compo_picturebox_set_size(f_home->pic_clock[i], HOME_DIGIT_W, HOME_DIGIT_H);
        x += HOME_DIGIT_W + HOME_CLOCK_GAP;
        if (i == HOME_CLOCK_IDX_H1) {
            cx = x + (s16)(HOME_COLON_W / 2);
            compo_picturebox_set_pos(f_home->pic_clock_colon, cx, HOME_CLOCK_Y);
            x += HOME_COLON_W + HOME_CLOCK_GAP;
        }
    }
}

static void func_home_clock_update(f_home_t *f_home, u8 hour, u8 min)
{
    u8 digits[HOME_CLOCK_IDX_CNT] = { hour / 10, hour % 10, min / 10, min % 10 };
    u8 i;

    if (f_home->pic_clock[0] == NULL) {
        return;
    }

    os_spiflash_read(home_ui_colon_ram, UI_BUF_HOME_COLON_BIN, UI_LEN_HOME_COLON_BIN);
    if (gui_set_ram_check(home_ui_colon_ram, __func__)) {
        compo_picturebox_set_ram(f_home->pic_clock_colon, home_ui_colon_ram);
    }

    for (i = 0; i < HOME_CLOCK_IDX_CNT; i++) {
        u8 d = digits[i];

        os_spiflash_read(home_ui_digit_ram[i], tbl_home_digit_addr[d], tbl_home_digit_len[d]);
        if (gui_set_ram_check(home_ui_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_home->pic_clock[i], home_ui_digit_ram[i]);
        }
    }

    func_home_clock_layout(f_home, hour, min);
}

static void func_home_tab_icon_update(f_home_t *f_home, u8 idx)
{
    bool selected = (idx == f_home->tab);
    u32 addr = selected ? tbl_home_nav_icon_addr_sel[idx] : tbl_home_nav_icon_addr_nor[idx];
    u16 len = selected ? tbl_home_nav_icon_len_sel[idx] : tbl_home_nav_icon_len[idx];

    if (f_home->tabs[idx].pic == NULL) {
        return;
    }

    os_spiflash_read(home_ui_shared_icon_runtime[idx], addr, len);
    if (gui_set_ram_check(home_ui_shared_icon_runtime[idx], __func__)) {
        compo_picturebox_set_ram(f_home->tabs[idx].pic, home_ui_shared_icon_runtime[idx]);
        compo_picturebox_set_size(f_home->tabs[idx].pic,
                                  tbl_home_nav_icon_w[idx], tbl_home_nav_icon_h[idx]);
    }
}

static void func_home_tab_line_update(f_home_t *f_home, u8 idx)
{
    bool selected = (idx == f_home->tab);

    if (f_home->tabs[idx].line == NULL) {
        return;
    }

    compo_shape_set_color(f_home->tabs[idx].line, selected ? COLOR_WHITE : HOME_COLOR_BLUE);
}

static const char * const tbl_home_tab_label[HOME_TAB_CNT] = {
    "HEAT",
    "MODE",
    "SETUP",
};

static const s16 tbl_home_tab_x[HOME_TAB_CNT] = {
    HOME_TAB_X0,
    HOME_TAB_X1,
    HOME_TAB_X2,
};

static compo_shape_t *func_home_shape_create(compo_form_t *frm, u16 id, s16 x, s16 y,
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

static compo_shape_t *func_home_tab_line_create(compo_form_t *frm, u16 id, s16 x)
{
    compo_shape_t *shape = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(shape, id);
    compo_shape_set_location(shape, x, HOME_TAB_DASH_Y, HOME_TAB_DASH_W, HOME_TAB_LINE_H);
    compo_shape_set_color(shape, HOME_COLOR_BLUE);
    compo_shape_set_radius(shape, 1);
    return shape;
}

static void func_home_tab_create(compo_form_t *frm, u8 idx, u16 id_base, const char *label, s16 x)
{
    compo_picturebox_t *pic;
    compo_button_t *btn;
    compo_textbox_t *txt;

    func_home_shape_create(frm, id_base + 0, x, HOME_TAB_BTN_Y,
                           HOME_TAB_BTN_W, HOME_TAB_BTN_H, HOME_COLOR_BLUE, HOME_TAB_BTN_R);
    func_home_shape_create(frm, id_base + 1, x, HOME_TAB_BTN_Y,
                           HOME_TAB_BTN_W, HOME_TAB_BTN_H, COLOR_WHITE, HOME_TAB_BTN_R);
    func_home_shape_create(frm, id_base + 2, x, HOME_TAB_BTN_Y,
                           HOME_TAB_BTN_W - 4, HOME_TAB_BTN_H - 4, COLOR_BLACK, HOME_TAB_BTN_R_IN);

    pic = compo_picturebox_create(frm, UI_HOME_ICON_PLACEHOLDER);
    compo_setid(pic, tbl_home_tab_pic_id[idx]);
    os_spiflash_read(home_ui_shared_icon_runtime[idx], tbl_home_nav_icon_addr_nor[idx], tbl_home_nav_icon_len[idx]);
    if (gui_set_ram_check(home_ui_shared_icon_runtime[idx], __func__)) {
        compo_picturebox_set_ram(pic, home_ui_shared_icon_runtime[idx]);
    }
    compo_picturebox_set_pos(pic, x, HOME_TAB_ICON_Y);
    compo_picturebox_set_size(pic, tbl_home_nav_icon_w[idx], tbl_home_nav_icon_h[idx]);

    txt = compo_textbox_create(frm, 8);
    compo_setid(txt, id_base + 4);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_pos(txt, x, HOME_TAB_LABEL_Y);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, label);

    func_home_tab_line_create(frm, tbl_home_tab_dash_id[idx], x);

    btn = compo_button_create(frm);
    compo_setid(btn, id_base + 3);
    compo_button_set_location(btn, x, HOME_TAB_BTN_Y, HOME_TAB_BTN_W, HOME_TAB_BTN_H);
}

static void func_home_tab_bind(f_home_t *f_home, u8 idx, u16 id_base)
{
    home_tab_ui_t *tab = &f_home->tabs[idx];

    tab->sel_bg = compo_getobj_byid(id_base + 0);
    tab->border_out = compo_getobj_byid(id_base + 1);
    tab->border_in = compo_getobj_byid(id_base + 2);
    tab->line = compo_getobj_byid(tbl_home_tab_dash_id[idx]);
    tab->pic = compo_getobj_byid(tbl_home_tab_pic_id[idx]);
    tab->btn = compo_getobj_byid(id_base + 3);
    tab->label = compo_getobj_byid(id_base + 4);
}

static void func_home_tab_refresh(f_home_t *f_home)
{
    u8 i;

    for (i = 0; i < HOME_TAB_CNT; i++) {
        home_tab_ui_t *tab = &f_home->tabs[i];
        bool selected = (i == f_home->tab);

        compo_shape_set_visible(tab->sel_bg, selected);
        compo_shape_set_visible(tab->border_out, !selected);
        compo_shape_set_visible(tab->border_in, !selected);
        func_home_tab_icon_update(f_home, i);
        func_home_tab_line_update(f_home, i);
    }
}

static void func_home_status_refresh(f_home_t *f_home)
{
    tm_t tm = rtc_clock_get();

    if (f_home->last_top_min != tm.min || f_home->last_top_sec != tm.sec) {
        f_home->last_top_min = tm.min;
        f_home->last_top_sec = tm.sec;
        home_top_time_refresh(&f_home->top_time, &tm);
        func_home_countdown_tick();
    }

    {
        u8 cd_hour, cd_min;
        u16 cd_total_min;

        func_home_countdown_get_display(&cd_hour, &cd_min);
        cd_total_min = (u16)cd_hour * 60 + cd_min;
        if (f_home->last_cd_total_min != cd_total_min) {
            f_home->last_cd_total_min = cd_total_min;
            func_home_clock_update(f_home, cd_hour, cd_min);
        }
    }
}

static void func_home_button_click(f_home_t *f_home)
{
    int id = compo_get_button_id();

    switch (id) {
    case COMPO_ID_TAB0_BTN:
        if (func_cb.sta == FUNC_HOME) {
            func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        } else {
            f_home->tab = HOME_TAB_HEAT;
            func_home_tab_refresh(f_home);
        }
        break;

    case COMPO_ID_TAB1_BTN:
        if (func_cb.sta == FUNC_HOME) {
            func_switch_to(FUNC_MODE, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        } else {
            f_home->tab = HOME_TAB_MODE;
            func_home_tab_refresh(f_home);
        }
        break;

    case COMPO_ID_TAB2_BTN:
        f_home->tab = HOME_TAB_SETUP;
        func_home_tab_refresh(f_home);
        func_switch_to(FUNC_SETTING, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    default:
        break;
    }
}

compo_form_t *func_home_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_picturebox_t *pic;

    home_top_time_create(frm, UI_HOME_ICON_PLACEHOLDER,
                         COMPO_ID_PIC_TOP_TIME_H10, COMPO_ID_PIC_TOP_TIME_H1,
                         COMPO_ID_PIC_TOP_TIME_COLON, COMPO_ID_PIC_TOP_TIME_M10,
                         COMPO_ID_PIC_TOP_TIME_M1, COMPO_ID_PIC_TOP_TIME_AMPM);

    pic = compo_picturebox_create(frm, UI_HOME_ICON_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, HOME_STATUS_BT_X, HOME_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BT_W, HOME_STATUS_BT_H);

    pic = compo_picturebox_create(frm, UI_HOME_ICON_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_LOCK);
    compo_picturebox_set_pos(pic, HOME_STATUS_LOCK_X, HOME_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);

    pic = compo_picturebox_create(frm, UI_HOME_ICON_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, HOME_STATUS_BAT_X, HOME_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);

    for (u8 i = 0; i < HOME_CLOCK_IDX_CNT; i++) {
        pic = compo_picturebox_create(frm, UI_HOME_ICON_PLACEHOLDER);
        compo_setid(pic, tbl_home_clock_id[i]);
        compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, HOME_CLOCK_Y);
        compo_picturebox_set_size(pic, HOME_DIGIT_MAX_W, HOME_DIGIT_H);
    }

    pic = compo_picturebox_create(frm, UI_HOME_ICON_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_CLOCK_COLON);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, HOME_CLOCK_Y);
    compo_picturebox_set_size(pic, HOME_COLON_W, HOME_COLON_H);

    func_home_tab_create(frm, HOME_TAB_HEAT, COMPO_ID_TAB0_SEL_BG,
                         tbl_home_tab_label[HOME_TAB_HEAT], tbl_home_tab_x[HOME_TAB_HEAT]);
    func_home_tab_create(frm, HOME_TAB_MODE, COMPO_ID_TAB1_SEL_BG,
                         tbl_home_tab_label[HOME_TAB_MODE], tbl_home_tab_x[HOME_TAB_MODE]);
    func_home_tab_create(frm, HOME_TAB_SETUP, COMPO_ID_TAB2_SEL_BG,
                         tbl_home_tab_label[HOME_TAB_SETUP], tbl_home_tab_x[HOME_TAB_SETUP]);

    return frm;
}

void func_home_process(void)
{
    f_home_t *f_home = (f_home_t *)func_cb.f_cb;

    if (f_home != NULL) {
        func_home_status_refresh(f_home);
    }
    func_process();
}

void func_home_message(size_msg_t msg)
{
    f_home_t *f_home = (f_home_t *)func_cb.f_cb;

    switch (msg) {
    case MSG_CTP_CLICK:
        func_home_button_click(f_home);
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_home_enter(void)
{
    f_home_t *f_home;

    func_cb.f_cb = func_zalloc(sizeof(f_home_t));
    func_cb.frm_main = func_home_form_create();

    f_home = (f_home_t *)func_cb.f_cb;
    f_home->tab = HOME_TAB_HEAT;
    f_home->last_top_min = 0xff;
    f_home->last_top_sec = 0xff;
    f_home->last_cd_total_min = 0xffff;

    home_top_time_bind(&f_home->top_time, COMPO_ID_PIC_TOP_TIME_H10, COMPO_ID_PIC_TOP_TIME_H1,
                       COMPO_ID_PIC_TOP_TIME_COLON, COMPO_ID_PIC_TOP_TIME_M10,
                       COMPO_ID_PIC_TOP_TIME_M1, COMPO_ID_PIC_TOP_TIME_AMPM);
    {
        tm_t tm = rtc_clock_get();
        home_top_time_refresh(&f_home->top_time, &tm);
    }
    for (u8 i = 0; i < HOME_CLOCK_IDX_CNT; i++) {
        f_home->pic_clock[i] = compo_getobj_byid(tbl_home_clock_id[i]);
    }
    f_home->pic_clock_colon = compo_getobj_byid(COMPO_ID_PIC_CLOCK_COLON);
    f_home->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f_home->pic_lock = compo_getobj_byid(COMPO_ID_PIC_LOCK);
    f_home->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);
    func_home_tab_bind(f_home, HOME_TAB_HEAT, COMPO_ID_TAB0_SEL_BG);
    func_home_tab_bind(f_home, HOME_TAB_MODE, COMPO_ID_TAB1_SEL_BG);
    func_home_tab_bind(f_home, HOME_TAB_SETUP, COMPO_ID_TAB2_SEL_BG);

    func_home_status_icons_apply(f_home);
    func_home_countdown_set(90, 5);
    func_home_countdown_start();

    func_home_tab_refresh(f_home);
    func_home_status_refresh(f_home);
}

void func_home_exit(void)
{
    func_cb.last = FUNC_HOME;
}

void func_home(void)
{
    printf("%s\n", __func__);
    func_home_enter();
    while (func_cb.sta == FUNC_HOME) {
        func_home_process();
        func_home_message(msg_dequeue());
    }
    func_home_exit();
}
