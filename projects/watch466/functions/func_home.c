#include "include.h"
#include "func.h"
#include "home_icon_res.h"
#include "home_ui_ram.h"
#include "home_ui_shared.h"
#include "home_top_time.h"
#include "home_tab_label.h"
#include "func_reservation.h"

#if USER_PANEL_LED
#include "port_panel_led.h"
#endif

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

#if ELUNCHBOX_PANEL_EN
#define HOME_DBG(...)           printf(__VA_ARGS__)
#else
#define HOME_DBG(...)           printf(__VA_ARGS__)
#endif

#if ELUNCHBOX_PANEL_EN
#undef TRACE_EN
#define TRACE_EN                0
#endif

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
 * 底部导航 Tab（HEAT / MODE / SETUP）：320×240 横屏按 UI 效果图排版。
 *   选中：蓝底 + 白框 + 白底线 + 白图标（*_sel.bin）
 *   未选中：黑底 + 白框 + 蓝底线 + 白图标（*.bin）
 *   PT8028（原理图 TCH0~TCH7）：
 *     TCH0 锁键(KU_LEFT) | TCH1 加热(KU_PREV) | TCH2 减(KU_VOL_DOWN) | TCH3 模式(KU_MODE)
 *     TCH4 确认(KU_BACK) | TCH5 开关(KU_RIGHT) | TCH6 加(KU_VOL_UP) | TCH7 预约(KU_NEXT)
 *   Home：TCH3(011) Tab 切换；TCH4(100) 进入 Tab 子页（表2 按下编码，释放 Hold）
 */
#define UI_HOME_ICON_PLACEHOLDER UI_BUF_ICON_ACTIVITY_BIN         

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

/* 466×466 参考布局；320×240 横屏使用紧凑布局 */
#define HOME_REF_W                      466
#define HOME_REF_H                      466
#define HOME_SX(v)                      ((s16)((s32)(v) * GUI_SCREEN_WIDTH / HOME_REF_W))
#define HOME_SY(v)                      ((s16)((s32)(v) * GUI_SCREEN_HEIGHT / HOME_REF_H))
#define HOME_TAB_INSET                  HOME_SX(4)

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
/* 320×240 横屏：底部 Tab 80×65，三等分居中 */
#define HOME_TAB_SIDE_MARGIN            20
#define HOME_TAB_GAP                    20
#define HOME_TAB_BTN_W                  80
#define HOME_TAB_BTN_H                  65
#define HOME_TAB_BTN_R                  8
#define HOME_TAB_BTN_R_IN               7
#define HOME_TAB_BTN_Y                  200
#define HOME_TAB_PAD_TOP                6
#define HOME_TAB_PAD_BOTTOM             5
#define HOME_TAB_PAD_MID                2
#define HOME_TAB_LINE_H                 2
#define HOME_TAB_ICON_DISP_H            18
#define HOME_TAB_X0                     60
#define HOME_TAB_X1                     160
#define HOME_TAB_X2                     260
#define HOME_STATUS_Y                   20
#define HOME_STATUS_RIGHT_MARGIN        10
#define HOME_STATUS_GAP                 6
#define HOME_STATUS_H                   18
#define HOME_CLOCK_GAP                  2
#define HOME_MAIN_DIGIT_W               42
#define HOME_MAIN_DIGIT_H               72
#define HOME_MAIN_COLON_W               21
#define HOME_MAIN_COLON_H               72
#define HOME_TAB_DASH_W_FIXED           50
#elif (GUI_SCREEN_HEIGHT <= 240)
#define HOME_TAB_SIDE_MARGIN            HOME_SX(8)
#define HOME_TAB_BTN_W                  ((GUI_SCREEN_WIDTH - HOME_TAB_SIDE_MARGIN * 2 - HOME_SX(8)) / 3)
#define HOME_TAB_BTN_H                  42
#define HOME_TAB_BTN_R                  8
#define HOME_TAB_BTN_R_IN               7
#define HOME_TAB_BTN_Y                  (GUI_SCREEN_HEIGHT - 14)
#define HOME_TAB_PAD_TOP                3
#define HOME_TAB_PAD_BOTTOM             3
#define HOME_TAB_PAD_MID                2
#define HOME_TAB_LINE_H                 2
#define HOME_TAB_ICON_DISP_H            20
#define HOME_TAB_STEP                   (HOME_TAB_BTN_W + HOME_SX(4))
#define HOME_TAB_X0                     (HOME_TAB_SIDE_MARGIN + HOME_TAB_BTN_W / 2)
#define HOME_TAB_X1                     (HOME_TAB_X0 + HOME_TAB_STEP)
#define HOME_TAB_X2                     (HOME_TAB_X1 + HOME_TAB_STEP)
#define HOME_STATUS_Y                   16
#define HOME_STATUS_RIGHT_MARGIN        HOME_SX(8)
#define HOME_STATUS_GAP                 HOME_SX(4)
#define HOME_STATUS_H                   18
#define HOME_CLOCK_GAP                  2
#define HOME_MAIN_DIGIT_W               32
#define HOME_MAIN_DIGIT_H               38
#define HOME_MAIN_COLON_W               14
#define HOME_MAIN_COLON_H               38
#define HOME_TAB_DASH_W_FIXED           HOME_DASH_RAM_W
#else
#define HOME_TAB_BTN_W                  HOME_SX(118)
#define HOME_TAB_BTN_H                  HOME_SY(96)
#define HOME_TAB_BTN_R                  HOME_SX(16)
#define HOME_TAB_BTN_R_IN               HOME_SX(14)
#define HOME_TAB_BTN_Y                  HOME_SY(390)
#define HOME_TAB_PAD_TOP                HOME_SY(12)
#define HOME_TAB_PAD_BOTTOM             HOME_SY(10)
#define HOME_TAB_PAD_MID                HOME_SY(6)
#define HOME_TAB_FONT_H                 HOME_SY(8)
#define HOME_TAB_LINE_H                 HOME_SY(3)
#define HOME_TAB_ICON_DISP_H            HOME_NAV_ICON_MAX_H
#define HOME_TAB_X0                     HOME_SX(87)
#define HOME_TAB_X1                     HOME_SX(233)
#define HOME_TAB_X2                     HOME_SX(379)
#define HOME_STATUS_Y                   HOME_SY(48)
#define HOME_STATUS_RIGHT_MARGIN        HOME_SX(24)
#define HOME_STATUS_GAP                 HOME_SX(10)
#define HOME_STATUS_H                   HOME_SY(36)
#define HOME_CLOCK_GAP                  HOME_SX(4)
#define HOME_MAIN_DIGIT_W               HOME_SX(48)
#define HOME_MAIN_DIGIT_H               HOME_SY(82)
#define HOME_MAIN_COLON_W               HOME_SX(24)
#define HOME_MAIN_COLON_H               HOME_SY(82)
#define HOME_TAB_DASH_W_FIXED           HOME_DASH_RAM_W
#endif

#define HOME_TAB_ICON_Y                 (HOME_TAB_BTN_Y - HOME_TAB_BTN_H / 2 + HOME_TAB_PAD_TOP + HOME_TAB_ICON_DISP_H / 2)
#define HOME_TAB_LABEL_W                (HOME_TAB_BTN_W - 6)
#define HOME_TAB_LABEL_H                (HOME_TAB_BTN_H - HOME_TAB_PAD_TOP - HOME_TAB_ICON_DISP_H \
                                         - HOME_TAB_PAD_MID - HOME_TAB_PAD_BOTTOM - HOME_TAB_LINE_H)
#define HOME_TAB_LABEL_ZONE_TOP         (HOME_TAB_BTN_Y - HOME_TAB_BTN_H / 2 + HOME_TAB_PAD_TOP \
                                         + HOME_TAB_ICON_DISP_H + HOME_TAB_PAD_MID)
#define HOME_TAB_LABEL_Y                (HOME_TAB_LABEL_ZONE_TOP + HOME_TAB_LABEL_H / 2)
#define HOME_TAB_DASH_Y                 (HOME_TAB_BTN_Y + HOME_TAB_BTN_H / 2 - HOME_TAB_PAD_BOTTOM - HOME_TAB_LINE_H / 2)
#define HOME_TAB_DASH_W                 HOME_TAB_DASH_W_FIXED

#define HOME_STATUS_BAT_X               (GUI_SCREEN_WIDTH - HOME_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define HOME_STATUS_LOCK_X              (HOME_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - HOME_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define HOME_STATUS_BT_X                (HOME_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - HOME_STATUS_GAP - HOME_STATUS_BT_W / 2)
#define HOME_CLOCK_Y                    ((HOME_STATUS_Y + HOME_STATUS_H + HOME_TAB_BTN_Y - HOME_TAB_BTN_H / 2) / 2)
#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define HOME_RES_MARQUEE_Y              55
#define HOME_RES_MARQUEE_W              296
#else
#define HOME_RES_MARQUEE_Y              HOME_SY(80)
#define HOME_RES_MARQUEE_W              (GUI_SCREEN_WIDTH - HOME_SX(32))
#endif

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
    COMPO_ID_TXT_RES_MARQUEE,
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
    compo_picturebox_t *label;
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
    compo_textbox_t *txt_res_marquee;
    home_tab_ui_t tabs[HOME_TAB_CNT];
} f_home_t;

static u32 home_countdown_remain_sec;
static bool home_countdown_running;

#if ELUNCHBOX_PANEL_EN
static u8 home_gui_dirty = 1;

void func_home_gui_mark_dirty(void)
{
    home_gui_dirty = 1;
}

bool func_home_gui_need_refresh(void)
{
    if (!home_gui_dirty) {
        return false;
    }
    home_gui_dirty = 0;
    return true;
}
#endif

static u8 *home_tab_label_ram_ptr[HOME_TAB_CNT];

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

static bool func_home_tab_label_ram_alloc(void)
{
    u8 i;

    for (i = 0; i < HOME_TAB_CNT; i++) {
        u16 sz = home_tab_label_ram_size(tbl_home_tab_label[i]);

        if (sz == 0) {
            return false;
        }
        home_tab_label_ram_ptr[i] = func_zalloc(sz);
        if (home_tab_label_ram_ptr[i] == NULL) {
            return false;
        }
    }
    return true;
}

static void func_home_tab_label_ram_free(void)
{
    u8 i;

    for (i = 0; i < HOME_TAB_CNT; i++) {
        if (home_tab_label_ram_ptr[i] != NULL) {
            func_free(home_tab_label_ram_ptr[i]);
            home_tab_label_ram_ptr[i] = NULL;
        }
    }
}

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

    total = HOME_MAIN_DIGIT_W + HOME_CLOCK_GAP + HOME_MAIN_DIGIT_W + HOME_CLOCK_GAP
          + HOME_MAIN_COLON_W + HOME_CLOCK_GAP + HOME_MAIN_DIGIT_W + HOME_CLOCK_GAP
          + HOME_MAIN_DIGIT_W;
    x = GUI_SCREEN_CENTER_X - (s16)(total / 2);

    for (i = 0; i < HOME_CLOCK_IDX_CNT; i++) {
        s16 cx = x + (s16)(HOME_MAIN_DIGIT_W / 2);

        compo_picturebox_set_pos(f_home->pic_clock[i], cx, HOME_CLOCK_Y);
        compo_picturebox_set_size(f_home->pic_clock[i], HOME_MAIN_DIGIT_W, HOME_MAIN_DIGIT_H);
        x += HOME_MAIN_DIGIT_W + HOME_CLOCK_GAP;
        if (i == HOME_CLOCK_IDX_H1) {
            cx = x + (s16)(HOME_MAIN_COLON_W / 2);
            compo_picturebox_set_pos(f_home->pic_clock_colon, cx, HOME_CLOCK_Y);
            compo_picturebox_set_size(f_home->pic_clock_colon, HOME_MAIN_COLON_W, HOME_MAIN_COLON_H);
            x += HOME_MAIN_COLON_W + HOME_CLOCK_GAP;
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

    home_gpu_wait_idle();

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
#if ELUNCHBOX_PANEL_EN
    func_home_gui_mark_dirty();
#endif
}

static void func_home_countdown_adjust_min(f_home_t *f_home, s16 delta_min)
{
    u32 sec = home_countdown_remain_sec;
    u32 max_sec = (u32)99 * 3600 + (u32)59 * 60;

    if (delta_min > 0) {
        sec += (u32)delta_min * 60;
        if (sec > max_sec) {
            sec = max_sec;
        }
    } else if (delta_min < 0) {
        u32 sub = (u32)(-delta_min) * 60;

        sec = (sec > sub) ? sec - sub : 0;
    }
    home_countdown_remain_sec = sec;

    if (f_home != NULL) {
        u8 cd_hour, cd_min;

        func_home_countdown_get_display(&cd_hour, &cd_min);
        f_home->last_cd_total_min = 0xffff;
        func_home_clock_update(f_home, cd_hour, cd_min);
    }
}

static void func_home_tab_icon_size(u8 idx, u16 *out_w, u16 *out_h)
{
    u16 src_w = tbl_home_nav_icon_w[idx];
    u16 src_h = tbl_home_nav_icon_h[idx];

    if (src_h == 0) {
        *out_w = 0;
        *out_h = 0;
        return;
    }

    *out_h = HOME_TAB_ICON_DISP_H;
    *out_w = (u16)((u32)src_w * HOME_TAB_ICON_DISP_H / src_h);
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
        u16 icon_w;
        u16 icon_h;

        func_home_tab_icon_size(idx, &icon_w, &icon_h);
        compo_picturebox_set_ram(f_home->tabs[idx].pic, home_ui_shared_icon_runtime[idx]);
        compo_picturebox_set_size(f_home->tabs[idx].pic, icon_w, icon_h);
    }
}

static void func_home_tab_line_update(f_home_t *f_home, u8 idx)
{
    bool selected = (idx == f_home->tab);

    if (f_home->tabs[idx].line == NULL) {
        return;
    }

    compo_shape_set_visible(f_home->tabs[idx].line, true);
    compo_shape_set_color(f_home->tabs[idx].line, selected ? COLOR_WHITE : HOME_COLOR_BLUE);
}

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

static void func_home_tab_label_update(f_home_t *f_home, u8 idx)
{
    bool selected = (idx == f_home->tab);
    u16 buf_size = home_tab_label_ram_size(tbl_home_tab_label[idx]);

    if (home_tab_label_ram_ptr[idx] == NULL || buf_size == 0) {
        return;
    }
    home_tab_label_apply(f_home->tabs[idx].label, home_tab_label_ram_ptr[idx], buf_size,
                         tbl_home_tab_label[idx], tbl_home_tab_x[idx], HOME_TAB_LABEL_Y,
                         selected, HOME_COLOR_BLUE);
}

static void func_home_tab_create(compo_form_t *frm, u8 idx, u16 id_base, const char *label, s16 x)
{
    compo_picturebox_t *pic;
    compo_picturebox_t *pic_label;
    compo_button_t *btn;
    u16 label_buf_size = home_tab_label_ram_size(label);

    func_home_shape_create(frm, id_base + 0, x, HOME_TAB_BTN_Y,
                           HOME_TAB_BTN_W, HOME_TAB_BTN_H, HOME_COLOR_BLUE, HOME_TAB_BTN_R);
    func_home_shape_create(frm, id_base + 1, x, HOME_TAB_BTN_Y,
                           HOME_TAB_BTN_W, HOME_TAB_BTN_H, COLOR_WHITE, HOME_TAB_BTN_R);
    func_home_shape_create(frm, id_base + 2, x, HOME_TAB_BTN_Y,
                           HOME_TAB_BTN_W - HOME_TAB_INSET, HOME_TAB_BTN_H - HOME_TAB_INSET,
                           COLOR_BLACK, HOME_TAB_BTN_R_IN);

    pic = compo_picturebox_create(frm, UI_HOME_ICON_PLACEHOLDER);
    compo_setid(pic, tbl_home_tab_pic_id[idx]);
    os_spiflash_read(home_ui_shared_icon_runtime[idx], tbl_home_nav_icon_addr_nor[idx], tbl_home_nav_icon_len[idx]);
    if (gui_set_ram_check(home_ui_shared_icon_runtime[idx], __func__)) {
        compo_picturebox_set_ram(pic, home_ui_shared_icon_runtime[idx]);
    }
    {
        u16 icon_w;
        u16 icon_h;

        func_home_tab_icon_size(idx, &icon_w, &icon_h);
        compo_picturebox_set_pos(pic, x, HOME_TAB_ICON_Y);
        compo_picturebox_set_size(pic, icon_w, icon_h);
    }

    pic_label = compo_picturebox_create(frm, UI_HOME_ICON_PLACEHOLDER);
    compo_setid(pic_label, id_base + 4);
    compo_picturebox_set_visible(pic_label, false);
    if (home_tab_label_ram_ptr[idx] != NULL && label_buf_size > 0) {
        home_tab_label_apply(pic_label, home_tab_label_ram_ptr[idx], label_buf_size,
                             label, x, HOME_TAB_LABEL_Y, (idx == HOME_TAB_HEAT), HOME_COLOR_BLUE);
    }

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

    home_gpu_wait_idle();

    for (i = 0; i < HOME_TAB_CNT; i++) {
        home_tab_ui_t *tab = &f_home->tabs[i];
        bool selected = (i == f_home->tab);

        compo_shape_set_visible(tab->sel_bg, selected);
        compo_shape_set_visible(tab->border_out, !selected);
        compo_shape_set_visible(tab->border_in, !selected);
        func_home_tab_icon_update(f_home, i);
        func_home_tab_line_update(f_home, i);
        func_home_tab_label_update(f_home, i);
    }
#if ELUNCHBOX_PANEL_EN
    func_home_gui_mark_dirty();
#endif
}

static void func_home_tab_select(f_home_t *f_home, u8 tab)
{
    if (f_home == NULL || tab >= HOME_TAB_CNT) {
        return;
    }
    f_home->tab = tab;
    func_home_tab_refresh(f_home);
}

static void func_home_tab_select_next(f_home_t *f_home)
{
    if (f_home == NULL) {
        return;
    }
    func_home_tab_select(f_home, (u8)((f_home->tab + 1) % HOME_TAB_CNT));
}

void func_home_mode_key(void)
{
    f_home_t *f_home = (f_home_t *)func_cb.f_cb;
    static u32 last_tick;
    static const char * const tab_name[HOME_TAB_CNT] = { "加热", "模式", "设置" };

    if (func_cb.sta != FUNC_HOME || f_home == NULL) {
        return;
    }
    if (!tick_check_expire(last_tick, 80)) {
        return;
    }
    last_tick = tick_get();
    func_home_tab_select_next(f_home);
    HOME_DBG("Home Tab -> %s\n", tab_name[f_home->tab]);
}

static void func_home_tab_enter(f_home_t *f_home)
{
    if (f_home == NULL) {
        return;
    }

    home_gpu_wait_idle();

    switch (f_home->tab) {
    case HOME_TAB_HEAT:
        HOME_DBG("func_home_tab_enter: HOME_TAB_HEAT\n");
        func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    case HOME_TAB_MODE:
        HOME_DBG("func_home_tab_enter: HOME_TAB_MODE\n");
        func_switch_to(FUNC_MODE, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    case HOME_TAB_SETUP:
        HOME_DBG("func_home_tab_enter: HOME_TAB_SETUP\n");
        func_switch_to(FUNC_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    default:
        break;
    }
}

static void func_home_res_marquee_refresh(f_home_t *f_home)
{
    char buf[48];

    if (f_home->txt_res_marquee == NULL) {
        return;
    }

    if (!func_reservation_is_waiting()) {
        compo_textbox_set_visible(f_home->txt_res_marquee, false);
#if ELUNCHBOX_PANEL_EN
        func_home_gui_mark_dirty();
#endif
        return;
    }

    func_reservation_marquee_text(buf, sizeof(buf));
    compo_textbox_set(f_home->txt_res_marquee, buf);
    compo_textbox_set_visible(f_home->txt_res_marquee, true);
#if ELUNCHBOX_PANEL_EN
    func_home_gui_mark_dirty();
#endif
}

static void func_home_status_refresh(f_home_t *f_home)
{
    tm_t tm = rtc_clock_get();

    if (f_home->last_top_min != tm.min || f_home->last_top_sec != tm.sec) {
        f_home->last_top_min = tm.min;
        f_home->last_top_sec = tm.sec;
        home_top_time_refresh(&f_home->top_time, &tm);
        func_home_countdown_tick();
        func_home_res_marquee_refresh(f_home);
#if ELUNCHBOX_PANEL_EN
        func_home_gui_mark_dirty();
#endif
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
        func_home_tab_select(f_home, HOME_TAB_HEAT);
        func_home_tab_enter(f_home);
        break;

    case COMPO_ID_TAB1_BTN:
        func_home_tab_select(f_home, HOME_TAB_MODE);
        func_home_tab_enter(f_home);
        break;

    case COMPO_ID_TAB2_BTN:
        func_home_tab_select(f_home, HOME_TAB_SETUP);
        func_home_tab_enter(f_home);
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

    {
        compo_textbox_t *txt = compo_textbox_create(frm, 48);

        compo_setid(txt, COMPO_ID_TXT_RES_MARQUEE);
        compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, HOME_RES_MARQUEE_Y);
        compo_textbox_set_autoroll(txt, true);
        compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_SROLL_CIRC);
        compo_textbox_set_visible(txt, false);
    }

    for (u8 i = 0; i < HOME_CLOCK_IDX_CNT; i++) {
        pic = compo_picturebox_create(frm, UI_HOME_ICON_PLACEHOLDER);
        compo_setid(pic, tbl_home_clock_id[i]);
        compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, HOME_CLOCK_Y);
        compo_picturebox_set_size(pic, HOME_MAIN_DIGIT_W, HOME_MAIN_DIGIT_H);
    }

    pic = compo_picturebox_create(frm, UI_HOME_ICON_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_CLOCK_COLON);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, HOME_CLOCK_Y);
    compo_picturebox_set_size(pic, HOME_MAIN_COLON_W, HOME_MAIN_COLON_H);

    func_home_tab_create(frm, HOME_TAB_HEAT, COMPO_ID_TAB0_SEL_BG,
                         tbl_home_tab_label[HOME_TAB_HEAT], tbl_home_tab_x[HOME_TAB_HEAT]);
    func_home_tab_create(frm, HOME_TAB_MODE, COMPO_ID_TAB1_SEL_BG,
                         tbl_home_tab_label[HOME_TAB_MODE], tbl_home_tab_x[HOME_TAB_MODE]);
    func_home_tab_create(frm, HOME_TAB_SETUP, COMPO_ID_TAB2_SEL_BG,
                         tbl_home_tab_label[HOME_TAB_SETUP], tbl_home_tab_x[HOME_TAB_SETUP]);

    return frm;
}

#if ELUNCHBOX_PANEL_EN
u8 func_res_allow_switch;
#endif

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static void func_home_drain_stale_key_msgs(void)
{
    msg_queue_detach(KU_NEXT, 0);
    msg_queue_detach(KU_MODE, 0);
    msg_queue_detach(KU_BACK, 0);
    msg_queue_detach(KU_PREV, 0);
    msg_queue_detach(KU_LEFT, 0);
    msg_queue_detach(KU_RIGHT, 0);
    msg_queue_detach(KU_VOL_UP, 0);
    msg_queue_detach(KU_VOL_DOWN, 0);
}

void func_home_switch_to_reservation(void)
{
#if !FUNC_RESERVATION_UI_EN
    return;
#endif
    func_res_allow_switch = 1;
    func_switch_to(FUNC_RESERVATION, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
    func_res_allow_switch = 0;
}

/* Home：模式/确认仅在释放沿处理一次（pt8028_take_home_action） */
static void func_home_pt8028_do_confirm(f_home_t *f_home)
{
    if (f_home == NULL) {
        return;
    }
    HOME_DBG("Home: 确认 -> 进入 Tab 子页 (tab=%d)\n", f_home->tab);
    func_home_tab_enter(f_home);
}

static void func_home_pt8028_handle_press(f_home_t *f_home, u8 tch)
{
    static u32 last_ms;
    static u8 last_tch = 0xff;

    if (f_home == NULL || tch > PT8028_KEY_TCH6) {
        return;
    }
    if (tch == PT8028_KEY_TCH3 || tch == PT8028_KEY_TCH4) {
        return;
    }
    if (tch == last_tch && !tick_check_expire(last_ms, 80)) {
        return;
    }
    last_ms = tick_get();
    last_tch = tch;

    switch (tch) {
    case PT8028_KEY_TCH1:
        HOME_DBG("Home: TCH1 加热键按下\n");
        home_gpu_wait_idle();
        func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    case PT8028_KEY_TCH2:
        HOME_DBG("Home: TCH2 减号按下\n");
        func_home_countdown_adjust_min(f_home, -1);
        break;

    case PT8028_KEY_TCH6:
        HOME_DBG("Home: TCH6 加号按下\n");
        func_home_countdown_adjust_min(f_home, 1);
        break;

    case PT8028_KEY_TCH0:
        HOME_DBG("Home: TCH0 锁键按下\n");
        break;

    case PT8028_KEY_TCH5:
        HOME_DBG("Home: TCH5 开关键按下\n");
        break;

    default:
        break;
    }
}

static void func_home_pt8028_handle_release(f_home_t *f_home, u8 tch)
{
    static u32 last_ms;
    static u8 last_tch = 0xff;

    if (f_home == NULL || tch > PT8028_KEY_TCH6) {
        return;
    }
    if (tch == PT8028_KEY_TCH3 || tch == PT8028_KEY_TCH4) {
        return;
    }
    if (tch == last_tch && !tick_check_expire(last_ms, 80)) {
        return;
    }
    last_ms = tick_get();
    last_tch = tch;

    switch (tch) {
    default:
        break;
    }
}
#endif

void func_home_process(void)
{
    f_home_t *f_home = (f_home_t *)func_cb.f_cb;

    func_process();

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    if (f_home != NULL) {
        u8 act = pt8028_take_home_action();
        u8 press_tch = pt8028_take_press_tch();
        u8 release_tch = pt8028_take_release_tch();

        func_home_drain_stale_key_msgs();

        if (act == PT8028_HOME_ACT_MODE) {
            HOME_DBG("Home: 模式键 -> Tab 切换\n");
            func_home_mode_key();
        } else if (act == PT8028_HOME_ACT_CONFIRM) {
            func_home_pt8028_do_confirm(f_home);
        }

        if (press_tch <= PT8028_KEY_TCH6 &&
            press_tch != PT8028_KEY_TCH3 && press_tch != PT8028_KEY_TCH4) {
            u16 kd = (u16)(tbl_pt8028_bcd_to_key[press_tch] | KEY_SHORT);

            msg_queue_detach(kd, 0);
            func_home_pt8028_handle_press(f_home, press_tch);
        }
        if (release_tch <= PT8028_KEY_TCH6 &&
            release_tch != PT8028_KEY_TCH3 && release_tch != PT8028_KEY_TCH4) {
            u16 ku = (u16)(tbl_pt8028_bcd_to_key[release_tch] | KEY_SHORT_UP);

            msg_queue_detach(ku, 0);
            func_home_pt8028_handle_release(f_home, release_tch);
        }
    }
#endif

    if (f_home != NULL) {
        func_home_status_refresh(f_home);
    }
}

void func_home_message(size_msg_t msg)
{
    f_home_t *f_home = (f_home_t *)func_cb.f_cb;

    if (msg == NO_MSG) {
        return;
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        func_home_button_click(f_home);
        break;

    case KU_LEFT:
#if ELUNCHBOX_PANEL_EN
        break;
#else
        HOME_DBG("Home: 锁键\n");
#endif
        break;

    case KU_PREV:
        HOME_DBG("Home: 加热键 -> 跳转加热页\n");
        home_gpu_wait_idle();
        func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    case KU_MODE:
    case K_MODE:
#if ELUNCHBOX_PANEL_EN
        break;
#else
        HOME_DBG("Home msg: 模式键 KU_MODE\n");
        func_home_mode_key();
#endif
        break;

    case KU_BACK:
#if ELUNCHBOX_PANEL_EN
        break;
#else
        HOME_DBG("Home msg: 确认键 KU_BACK\n");
        func_home_tab_enter(f_home);
#endif
        break;

    case KU_RIGHT:
        HOME_DBG("Home: 开关键\n");
        break;

    case KU_NEXT:
        /* 预约键走 pt8028_take_res_key_pending()，此处忽略残留 KU_NEXT */
        break;

    case KU_VOL_UP:
        HOME_DBG("Home msg: 加号\n");
        home_gpu_wait_idle();
        func_home_countdown_adjust_min(f_home, 1);
        break;

    case KU_VOL_DOWN:
        HOME_DBG("Home msg: 减号\n");
        home_gpu_wait_idle();
        func_home_countdown_adjust_min(f_home, -1);
        break;

    default:
#if ELUNCHBOX_PANEL_EN
        /* 不把未识别按键交给 func_message，避免误跳预约页等全局副作用 */
        break;
#else
        func_message(msg);
        break;
#endif
    }
}

void func_home_enter(void)
{
    f_home_t *f_home;

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif
    if (!func_home_tab_label_ram_alloc()) {
        HOME_DBG("func_home: tab label ram alloc fail\n");
    }
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
    f_home->txt_res_marquee = compo_getobj_byid(COMPO_ID_TXT_RES_MARQUEE);
    func_home_tab_bind(f_home, HOME_TAB_HEAT, COMPO_ID_TAB0_SEL_BG);
    func_home_tab_bind(f_home, HOME_TAB_MODE, COMPO_ID_TAB1_SEL_BG);
    func_home_tab_bind(f_home, HOME_TAB_SETUP, COMPO_ID_TAB2_SEL_BG);

    func_home_status_icons_apply(f_home);
    func_home_countdown_set(90, 5);
    func_home_countdown_start();

    func_home_tab_refresh(f_home);
    func_home_status_refresh(f_home);
    func_home_res_marquee_refresh(f_home);

    tft_bglight_frist_set_check();
    os_gui_draw_force();
#if ELUNCHBOX_PANEL_EN
    func_home_gui_mark_dirty();     /* 首帧进主循环仍需 gui_process 一次 */
    pt8028_release_clear();
#endif
}

void func_home_exit(void)
{
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(0);
    pt8028_release_clear();
#endif
#if USER_PANEL_LED
    panel_led_all_off();
#endif
    func_home_tab_label_ram_free();
    func_cb.last = FUNC_HOME;
}

void func_home(void)
{
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(1);
    pt8028_release_clear();
#endif
#if !ELUNCHBOX_PANEL_EN
    printf("%s\n", __func__);
#endif
    func_home_enter();
    while (func_cb.sta == FUNC_HOME) {
        func_home_process();
        func_home_message(msg_dequeue());
    }
    func_home_exit();
}
