#include "include.h"
#include "func.h"
#include "func_lunchbox_uart.h"
#include "heat_display_reg.h"
#include "home_icon_res.h"
#include "home_ui_ram.h"
#include "home_ui_shared.h"
#include "home_top_time.h"
#include "home_tab_label.h"
#include "home_ui_gpu_detach.h"
#include "ui_layout_anchor.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * Mode 页 UI（320×240 横屏设计图）：
 *   顶栏 Y≈14：绿色温度 (102/116/130/146,14) + 右上 BT/锁/电量
 *   中部 Y≈60：白色 HH:MM（右上角锚点 100/141/172/225/283,60，与 Heat 页一致）
 *   底部 Tab 80×65：Pasta / Chicken / Warm（图标 bin 原始尺寸，文字 5×7 点阵）
 * PT8028：TCH3 模式循环 Tab；TCH4 确认开始加热；TCH5 开关回 Home；TCH0 锁键解锁童锁
 */
#define UI_MODE_PLACEHOLDER               UI_BUF_ICON_ACTIVITY_BIN
#define MODE_COLOR_BLUE                   make_color(4, 109, 217)
#define MODE_TAB_ICON_BG_BLACK            0x0000

#ifndef UI_BUF_HOME_PASTA_BIN
#error "Missing pasta.bin: add ui/home/pasta.png and run gen_mode_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_CHICKEN_BIN
#error "Missing chicken.bin: add ui/home/chicken.png and run gen_mode_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_INSULATION_BIN
#error "Missing insulation.bin: add ui/home/insulation.png and run gen_mode_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_WHILE_LINE_BIN
#error "Missing while_line.bin: add ui/home/while_line.png and run gen_mode_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BLUE_LINE_BIN
#error "Missing blue_line.bin: add ui/home/blue_line.png and run gen_mode_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_G0_BIN
#error "Missing g0.bin: add ui/home/g0.png..g9.png, gh.png and run gen_mode_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_GH_BIN
#error "Missing gh.bin: add ui/home/gh.png and run gen_mode_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_0_BIN
#error "Missing 0.bin: add res/home/0.png..9.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_COLON_BIN
#error "Missing colon.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BLUETOOTH_BIN
#error "Missing bluetooth.bin: add res/home/bluetooth.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_LOCK_BIN
#error "Missing lock.bin: add res/home/lock.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BATTERY_LEVEL_BIN
#error "Missing battery_level.bin: add res/home/battery_level.png and run gen_home_icons.py + prebuild.bat"
#endif

/* 466×466 参考布局；320×240 横屏按设计图固定坐标，图标 bin 保持原始尺寸 */
#define MODE_REF_W                        466
#define MODE_REF_H                        466
#define MODE_SX(v)                        ((s16)((s32)(v) * GUI_SCREEN_WIDTH / MODE_REF_W))
#define MODE_SY(v)                        ((s16)((s32)(v) * GUI_SCREEN_HEIGHT / MODE_REF_H))

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define MODE_STATUS_Y                     20
#define MODE_STATUS_RIGHT_MARGIN          10
#define MODE_STATUS_GAP                   6
#define MODE_STATUS_H                     18
#define MODE_TAB_BTN_W                    80
#define MODE_TAB_BTN_H                    65
#define MODE_TAB_BTN_R                    8
#define MODE_TAB_BTN_R_IN                 7
#define MODE_TAB_BTN_Y                    200
#define MODE_TAB_PAD_TOP                  6
#define MODE_TAB_PAD_BOTTOM               5
#define MODE_TAB_PAD_MID                  2
#define MODE_TAB_X0                       60
#define MODE_TAB_X1                       160
#define MODE_TAB_X2                       260
#else
#define MODE_STATUS_Y                     MODE_SY(48)
#define MODE_STATUS_RIGHT_MARGIN          MODE_SX(24)
#define MODE_STATUS_GAP                   MODE_SX(10)
#define MODE_STATUS_H                     MODE_SY(36)
#define MODE_TAB_BTN_W                    MODE_SX(118)
#define MODE_TAB_BTN_H                    MODE_SY(96)
#define MODE_TAB_BTN_R                    MODE_SX(18)
#define MODE_TAB_BTN_R_IN                 MODE_SX(16)
#define MODE_TAB_BTN_Y                    MODE_SY(390)
#define MODE_TAB_PAD_TOP                  MODE_SY(12)
#define MODE_TAB_PAD_BOTTOM               MODE_SY(10)
#define MODE_TAB_PAD_MID                  MODE_SY(8)
#define MODE_TAB_X0                       MODE_SX(87)
#define MODE_TAB_X1                       MODE_SX(233)
#define MODE_TAB_X2                       MODE_SX(379)
#endif

#define MODE_TAB_ICON_Y                   (MODE_TAB_BTN_Y - MODE_TAB_BTN_H / 2 + MODE_TAB_PAD_TOP + MODE_TAB_ICON_MAX_H / 2)
#define MODE_TAB_LABEL_W                  (MODE_TAB_BTN_W - 6)
#define MODE_TAB_LABEL_H                  (MODE_TAB_BTN_H - MODE_TAB_PAD_TOP - MODE_TAB_ICON_MAX_H \
                                           - MODE_TAB_PAD_MID - MODE_TAB_PAD_BOTTOM - MODE_TAB_LINE_H)
#define MODE_TAB_LABEL_ZONE_TOP           (MODE_TAB_BTN_Y - MODE_TAB_BTN_H / 2 + MODE_TAB_PAD_TOP \
                                           + MODE_TAB_ICON_MAX_H + MODE_TAB_PAD_MID)
#define MODE_TAB_LABEL_Y                  (MODE_TAB_LABEL_ZONE_TOP + MODE_TAB_LABEL_H / 2)
#define MODE_TAB_DASH_Y                   (MODE_TAB_BTN_Y + MODE_TAB_BTN_H / 2 - MODE_TAB_PAD_BOTTOM - MODE_TAB_LINE_H / 2)

#define MODE_STATUS_BAT_X                 (GUI_SCREEN_WIDTH - MODE_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define MODE_STATUS_LOCK_X                (MODE_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - MODE_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define MODE_STATUS_BT_X                  (MODE_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - MODE_STATUS_GAP - HOME_STATUS_BT_W / 2)

/* Mode 页顶栏绿色温度：右上角锚点 (320×240) */
#define MODE_STATUS_TEMP_TR_Y             ((s16)((s32)14 * GUI_SCREEN_HEIGHT / HEAT_LAYOUT_REF_H))
#define MODE_STATUS_TEMP_TR_H_X           ((s16)((s32)102 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define MODE_STATUS_TEMP_TR_T10_X         ((s16)((s32)116 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define MODE_STATUS_TEMP_TR_T1_X          ((s16)((s32)130 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define MODE_STATUS_TEMP_TR_UNIT_X        ((s16)((s32)146 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))

#define MODE_LOCK_MS                      30000
#define MODE_MSG_OK                       KU_BACK
#define MODE_MSG_POWER                    (KEY_RIGHT | KEY_SHORT_UP)

enum {
    MODE_UI_IDLE = 0,
    MODE_UI_HEATING,
    MODE_UI_FINISHED,
};

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
    MODE_TAB_INSULATION,
    MODE_TAB_CNT,
};

enum {
    COMPO_ID_PIC_TOP_TIME_H10 = 1,
    COMPO_ID_PIC_TOP_TIME_H1,
    COMPO_ID_PIC_TOP_TIME_COLON,
    COMPO_ID_PIC_TOP_TIME_M10,
    COMPO_ID_PIC_TOP_TIME_M1,
    COMPO_ID_PIC_TOP_TIME_AMPM,
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
    compo_shape_t *line;
    compo_button_t *btn;
    compo_picturebox_t *label;
} mode_tab_ui_t;

/* 仅在 Mode 页生命周期内分配，避免常驻 BSS 挤占 ab_malloc 动态堆 */
static u8 *mode_tab_label_ram_ptr[MODE_TAB_CNT];

static const char * const tbl_mode_tab_label[MODE_TAB_CNT] = {
    "Pasta",
    "Chicken",
    "Warm",
};

static void func_mode_tab_label_ram_free(void);

static bool func_mode_tab_label_ram_alloc(void)
{
    u8 i;

    for (i = 0; i < MODE_TAB_CNT; i++) {
        u16 sz = home_tab_label_ram_size(tbl_mode_tab_label[i]);

        if (sz == 0) {
            func_mode_tab_label_ram_free();
            return false;
        }
        mode_tab_label_ram_ptr[i] = func_zalloc(sz);
        if (mode_tab_label_ram_ptr[i] == NULL) {
            func_mode_tab_label_ram_free();
            return false;
        }
    }
    return true;
}

static void func_mode_tab_label_ram_free(void)
{
    u8 i;

    for (i = 0; i < MODE_TAB_CNT; i++) {
        if (mode_tab_label_ram_ptr[i] != NULL) {
            func_free(mode_tab_label_ram_ptr[i]);
            mode_tab_label_ram_ptr[i] = NULL;
        }
    }
}

typedef struct f_mode_t_ {
    u8 tab;
    u8 ui_state;
    bool screen_locked;
    u32 heat_start_tick;
    u32 heat_total_sec;
    u32 heat_live_remain_min;   /* 加热中：串口回调推送的剩余分钟 */
    u16 heat_live_temp_f;       /* 加热中：串口回调推送的实时温度 °F */
    bool heat_live_ready;       /* 是否已收到至少一次回调 */
    u8 last_top_min;
    u8 last_top_sec;
    u16 last_timer_key;
    u16 last_temp_f;
#if ELUNCHBOX_PANEL_EN
    u8 display_stage;   /* 0=就绪；1=顶栏；2=计时；3=温度；4=Tab */
#endif
    home_top_time_ui_t top_time;
    compo_picturebox_t *pic_status_temp[MODE_TEMP_IDX_CNT];
    compo_picturebox_t *pic_status_temp_degf;
    compo_picturebox_t *pic_timer[MODE_TIMER_IDX_CNT];
    compo_picturebox_t *pic_timer_colon;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
    mode_tab_ui_t tabs[MODE_TAB_CNT];
} f_mode_t;

static u8 mode_status_temp_digit_ram[MODE_TEMP_IDX_CNT][MODE_G_DIGIT_RAM_MAX_SIZE];
static u8 mode_status_gh_ram[MODE_GH_RAM_SIZE];
static u8 mode_tab_icon_ram[MODE_TAB_CNT][HOME_ICON_RAM_SIZE];

static u32 mode_countdown_remain_sec;
static bool mode_countdown_running;

static const u32 tbl_mode_g_digit_addr[10] = {
    UI_BUF_HOME_G0_BIN, UI_BUF_HOME_G1_BIN, UI_BUF_HOME_G2_BIN, UI_BUF_HOME_G3_BIN,
    UI_BUF_HOME_G4_BIN, UI_BUF_HOME_G5_BIN, UI_BUF_HOME_G6_BIN, UI_BUF_HOME_G7_BIN,
    UI_BUF_HOME_G8_BIN, UI_BUF_HOME_G9_BIN,
};

static const u16 tbl_mode_g_digit_len[10] = {
    UI_LEN_HOME_G0_BIN, UI_LEN_HOME_G1_BIN, UI_LEN_HOME_G2_BIN, UI_LEN_HOME_G3_BIN,
    UI_LEN_HOME_G4_BIN, UI_LEN_HOME_G5_BIN, UI_LEN_HOME_G6_BIN, UI_LEN_HOME_G7_BIN,
    UI_LEN_HOME_G8_BIN, UI_LEN_HOME_G9_BIN,
};

static const u16 tbl_mode_g_digit_w[10] = {
    MODE_G0_W, MODE_G1_W, MODE_G2_W, MODE_G3_W, MODE_G4_W,
    MODE_G5_W, MODE_G6_W, MODE_G7_W, MODE_G8_W, MODE_G9_W,
};

static const u16 tbl_mode_g_digit_h[10] = {
    MODE_G0_H, MODE_G1_H, MODE_G2_H, MODE_G3_H, MODE_G4_H,
    MODE_G5_H, MODE_G6_H, MODE_G7_H, MODE_G8_H, MODE_G9_H,
};

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

/* Mode 中部倒计时使用 w0x（白字，与 Heat 共享 RAM，不占用 home_ui_digit_ram） */
static const u32 tbl_mode_w_digit_addr[10] = {
    UI_BUF_HOME_W0X_BIN, UI_BUF_HOME_W1X_BIN, UI_BUF_HOME_W2X_BIN, UI_BUF_HOME_W3X_BIN,
    UI_BUF_HOME_W4X_BIN, UI_BUF_HOME_W5X_BIN, UI_BUF_HOME_W6X_BIN, UI_BUF_HOME_W7X_BIN,
    UI_BUF_HOME_W8X_BIN, UI_BUF_HOME_W9X_BIN,
};

static const u16 tbl_mode_w_digit_len[10] = {
    UI_LEN_HOME_W0X_BIN, UI_LEN_HOME_W1X_BIN, UI_LEN_HOME_W2X_BIN, UI_LEN_HOME_W3X_BIN,
    UI_LEN_HOME_W4X_BIN, UI_LEN_HOME_W5X_BIN, UI_LEN_HOME_W6X_BIN, UI_LEN_HOME_W7X_BIN,
    UI_LEN_HOME_W8X_BIN, UI_LEN_HOME_W9X_BIN,
};

static const u16 tbl_mode_w_digit_w[10] = {
    HEAT_W0X_W, HEAT_W1X_W, HEAT_W2X_W, HEAT_W3X_W, HEAT_W4X_W,
    HEAT_W5X_W, HEAT_W6X_W, HEAT_W7X_W, HEAT_W8X_W, HEAT_W9X_W,
};

static const u32 tbl_mode_icon_addr[MODE_TAB_CNT] = {
    UI_BUF_HOME_PASTA_BIN,
    UI_BUF_HOME_CHICKEN_BIN,
    UI_BUF_HOME_INSULATION_BIN,
};

static const u16 tbl_mode_icon_len[MODE_TAB_CNT] = {
    UI_LEN_HOME_PASTA_BIN,
    UI_LEN_HOME_CHICKEN_BIN,
    UI_LEN_HOME_INSULATION_BIN,
};

static const u16 tbl_mode_icon_w[MODE_TAB_CNT] = {
    MODE_TAB_PASTA_W,
    MODE_TAB_CHICKEN_W,
    MODE_TAB_INSULATION_W,
};

static const u16 tbl_mode_icon_h[MODE_TAB_CNT] = {
    MODE_TAB_PASTA_H,
    MODE_TAB_CHICKEN_H,
    MODE_TAB_INSULATION_H,
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

/* 底部 Tab 选中时的固定温度(°F)与倒计时 */
static const mode_tab_preset_t tbl_mode_tab_preset[MODE_TAB_CNT] = {
    {194, 1, 0},   /* Pasta       */
    {212, 1, 0},   /* Chicken     */
    {120, 1, 0},   /* Insulation  */
};

/* Tab → 协议模式值: Pasta=3(意面), Chicken=2(鸡腿), Insulation=5(保温) */
static const u8 tbl_mode_tab_to_proto[MODE_TAB_CNT] = { 3, 2, 5 };

static void func_mode_status_temp_update(f_mode_t *f_mode, u16 temp_f);
static void func_mode_timer_update(f_mode_t *f_mode, u8 hour, u8 min);
static void func_mode_display_refresh(f_mode_t *f_mode);
static void func_mode_status_icons_apply(f_mode_t *f_mode);
static void func_mode_tab_preview_timer(f_mode_t *f_mode, u8 tab);
static void func_mode_tab_preview_temp(f_mode_t *f_mode, u8 tab);
static void func_mode_tab_preview_apply(f_mode_t *f_mode, u8 tab);
static void func_mode_tab_refresh(f_mode_t *f_mode);
static void func_mode_tab_bind(f_mode_t *f_mode, u8 idx, u16 id_base);
static void func_mode_tab_select_next(f_mode_t *f_mode);
static void func_mode_start_heating(f_mode_t *f_mode);
static void func_mode_power_key(f_mode_t *f_mode);
static void func_mode_lock_icon_apply(f_mode_t *f_mode);
static void func_mode_lock_check(f_mode_t *f_mode);
static void func_mode_heating_finish_check(f_mode_t *f_mode);
static bool func_mode_key_allowed(f_mode_t *f_mode, size_msg_t msg);

static void func_mode_display_on_info(const heat_display_info_t *info)
{
    f_mode_t *f_mode = (f_mode_t *)func_cb.f_cb;

    if (info == NULL || f_mode == NULL || func_cb.sta != FUNC_MODE) {
        return;
    }
    if (f_mode->ui_state != MODE_UI_HEATING) {
        return;
    }

    f_mode->heat_live_remain_min = info->remain_min;
    f_mode->heat_live_temp_f = info->temp_f;
    f_mode->heat_live_ready = true;
    f_mode->last_timer_key = 0xffff;
    f_mode->last_temp_f = 0xffff;

    if (info->remain_min == 0) {
        func_mode_heating_finish_check(f_mode);
    }
    if (f_mode->ui_state != MODE_UI_FINISHED) {
        func_mode_display_refresh(f_mode);
    }
}

static void func_mode_form_bind_core(f_mode_t *f_mode)
{
    if (f_mode == NULL) {
        return;
    }

    home_top_time_bind(&f_mode->top_time, COMPO_ID_PIC_TOP_TIME_H10, COMPO_ID_PIC_TOP_TIME_H1,
                       COMPO_ID_PIC_TOP_TIME_COLON, COMPO_ID_PIC_TOP_TIME_M10,
                       COMPO_ID_PIC_TOP_TIME_M1, COMPO_ID_PIC_TOP_TIME_AMPM);
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
}

static void func_mode_form_bind_tabs(f_mode_t *f_mode)
{
    if (f_mode == NULL) {
        return;
    }

    func_mode_tab_bind(f_mode, MODE_TAB_PASTA, COMPO_ID_TAB0_SEL_BG);
    func_mode_tab_bind(f_mode, MODE_TAB_CHICKEN, COMPO_ID_TAB1_SEL_BG);
    func_mode_tab_bind(f_mode, MODE_TAB_INSULATION, COMPO_ID_TAB2_SEL_BG);
}

static bool func_mode_gpu_ram_set(u8 *ram, u16 buf_size, u16 data_len, compo_picturebox_t *pic)
{
    u16 need;

    if (ram == NULL || data_len == 0 || data_len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    if (!gui_set_ram_check(ram, __func__)) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    need = (u16)(8 + (u32)GET_LE16(&ram[4]) * GET_LE16(&ram[6]) * 2);
    if (need > data_len || need > buf_size) {
        printf("%s: len mismatch need %u data %u buf %u\n", __func__, need, data_len, buf_size);
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

static bool func_mode_gpu_flash_to_ram(u8 *ram, u16 buf_size, u32 flash_addr, u16 flash_len,
                                       compo_picturebox_t *pic)
{
    if (ram == NULL || flash_len == 0 || flash_len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    os_spiflash_read(ram, flash_addr, flash_len);
    return func_mode_gpu_ram_set(ram, buf_size, flash_len, pic);
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
        compo_picturebox_set_visible(f_mode->pic_bt, true);
    } else if (f_mode->pic_bt != NULL) {
        compo_picturebox_set_visible(f_mode->pic_bt, false);
    }
    if (f_mode->pic_bat != NULL && gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        compo_picturebox_set_ram(f_mode->pic_bat, home_ui_shared_status_bat_ram);
        compo_picturebox_set_size(f_mode->pic_bat, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
        compo_picturebox_set_visible(f_mode->pic_bat, true);
    } else if (f_mode->pic_bat != NULL) {
        compo_picturebox_set_visible(f_mode->pic_bat, false);
    }
    func_mode_lock_icon_apply(f_mode);
}

static void func_mode_lock_icon_apply(f_mode_t *f_mode)
{
    if (f_mode == NULL || f_mode->pic_lock == NULL) {
        return;
    }

    if (f_mode->screen_locked) {
        home_ui_shared_status_init();
        compo_picturebox_set_pos(f_mode->pic_lock, MODE_STATUS_LOCK_X, MODE_STATUS_Y);
        if (gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
            compo_picturebox_set_ram(f_mode->pic_lock, home_ui_shared_status_lock_ram);
            compo_picturebox_set_size(f_mode->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
            compo_picturebox_set_visible(f_mode->pic_lock, true);
        }
    } else {
        compo_picturebox_set_visible(f_mode->pic_lock, false);
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
    }
}

static void func_mode_lock_check(f_mode_t *f_mode)
{
    if (f_mode->ui_state != MODE_UI_HEATING || f_mode->screen_locked) {
        return;
    }
    if (tick_check_expire(f_mode->heat_start_tick, MODE_LOCK_MS)) {
        f_mode->screen_locked = true;
        func_mode_lock_icon_apply(f_mode);
    }
}

static void func_mode_heating_finish_check(f_mode_t *f_mode)
{
    if (f_mode->ui_state != MODE_UI_HEATING) {
        return;
    }
#if FUNC_LUNCHBOX_UART_EN
    if (!f_mode->heat_live_ready || f_mode->heat_live_remain_min != 0) {
        return;
    }
#else
    if (mode_countdown_remain_sec != 0) {
        return;
    }
#endif
    mode_countdown_running = false;
    f_mode->ui_state = MODE_UI_FINISHED;
    f_mode->screen_locked = false;
    func_mode_lock_icon_apply(f_mode);
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_heat_stop();
#endif
}

static void func_mode_status_refresh(f_mode_t *f_mode)
{
    tm_t tm = rtc_clock_get();
    bool sec_changed = false;

    if (f_mode->last_top_min != tm.min || f_mode->last_top_sec != tm.sec) {
        f_mode->last_top_min = tm.min;
        f_mode->last_top_sec = tm.sec;
        home_top_time_refresh(&f_mode->top_time, &tm);
        sec_changed = true;
    }

    if (f_mode->ui_state == MODE_UI_HEATING && sec_changed) {
#if !FUNC_LUNCHBOX_UART_EN
        func_mode_countdown_tick();
        func_mode_heating_finish_check(f_mode);
        func_mode_display_refresh(f_mode);
#endif
    }

    func_mode_lock_check(f_mode);
}

static const s16 tbl_mode_status_temp_tr_x[MODE_TEMP_IDX_CNT] = {
    MODE_STATUS_TEMP_TR_H_X, MODE_STATUS_TEMP_TR_T10_X, MODE_STATUS_TEMP_TR_T1_X,
};

static void func_mode_pic_pos_tr(compo_picturebox_t *pic, s16 tr_x, s16 tr_y, u16 w, u16 h)
{
    if (pic == NULL) {
        return;
    }
    compo_picturebox_set_pos(pic, tr_x - (s16)(w / 2), tr_y + (s16)(h / 2));
}

static void func_mode_status_temp_layout(f_mode_t *f_mode, u16 temp_f)
{
    u8 digits[MODE_TEMP_IDX_CNT];
    u8 i;

    if (temp_f > 999) {
        temp_f = 999;
    }

    digits[MODE_TEMP_IDX_H] = (u8)(temp_f / 100);
    digits[MODE_TEMP_IDX_T10] = (u8)((temp_f / 10) % 10);
    digits[MODE_TEMP_IDX_T1] = (u8)(temp_f % 10);

    for (i = 0; i < MODE_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];
        u16 w = tbl_mode_g_digit_w[d];

        func_mode_pic_pos_tr(f_mode->pic_status_temp[i], tbl_mode_status_temp_tr_x[i],
                             MODE_STATUS_TEMP_TR_Y, w, MODE_G_DIGIT_MAX_H);
        compo_picturebox_set_size(f_mode->pic_status_temp[i], w, MODE_G_DIGIT_MAX_H);
    }

    if (f_mode->pic_status_temp_degf != NULL) {
        func_mode_pic_pos_tr(f_mode->pic_status_temp_degf, MODE_STATUS_TEMP_TR_UNIT_X,
                             MODE_STATUS_TEMP_TR_Y, MODE_GH_W, MODE_GH_H);
        compo_picturebox_set_size(f_mode->pic_status_temp_degf, MODE_GH_W, MODE_GH_H);
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

    if (f_mode->pic_status_temp_degf != NULL) {
        if (func_mode_gpu_flash_to_ram(mode_status_gh_ram, MODE_GH_RAM_SIZE,
                                        UI_BUF_HOME_GH_BIN, UI_LEN_HOME_GH_BIN,
                                        f_mode->pic_status_temp_degf)) {
            compo_picturebox_set_size(f_mode->pic_status_temp_degf, MODE_GH_W, MODE_GH_H);
        }
    }

    for (i = 0; i < MODE_TEMP_IDX_CNT; i++) {
        u8 d = digits[i];

        if (func_mode_gpu_flash_to_ram(mode_status_temp_digit_ram[i], MODE_G_DIGIT_RAM_MAX_SIZE,
                                       tbl_mode_g_digit_addr[d], tbl_mode_g_digit_len[d],
                                       f_mode->pic_status_temp[i])) {
            compo_picturebox_set_size(f_mode->pic_status_temp[i],
                                      tbl_mode_g_digit_w[d], MODE_G_DIGIT_MAX_H);
        }
    }

    func_mode_status_temp_layout(f_mode, temp_f);
}

static void func_mode_timer_layout(f_mode_t *f_mode, u8 hour, u8 min)
{
    u8 h_digits[2] = { hour / 10, hour % 10 };
    u8 m_digits[2] = { min / 10, min % 10 };

    func_mode_pic_pos_tr(f_mode->pic_timer[MODE_TIMER_IDX_H10],
                         HEAT_TIMER_TR_H10_X, HEAT_TIMER_TR_Y,
                         tbl_mode_w_digit_w[h_digits[0]], HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(f_mode->pic_timer[MODE_TIMER_IDX_H10],
                              tbl_mode_w_digit_w[h_digits[0]], HEAT_W_DIGIT_MAX_H);

    func_mode_pic_pos_tr(f_mode->pic_timer[MODE_TIMER_IDX_H1],
                         HEAT_TIMER_TR_H1_X, HEAT_TIMER_TR_Y,
                         tbl_mode_w_digit_w[h_digits[1]], HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(f_mode->pic_timer[MODE_TIMER_IDX_H1],
                              tbl_mode_w_digit_w[h_digits[1]], HEAT_W_DIGIT_MAX_H);

    func_mode_pic_pos_tr(f_mode->pic_timer_colon,
                         HEAT_TIMER_TR_COLON_X, HEAT_TIMER_TR_Y,
                         HEAT_WBX_W, HEAT_WBX_H);
    compo_picturebox_set_size(f_mode->pic_timer_colon, HEAT_WBX_W, HEAT_WBX_H);

    func_mode_pic_pos_tr(f_mode->pic_timer[MODE_TIMER_IDX_M10],
                         HEAT_TIMER_TR_M10_X, HEAT_TIMER_TR_Y,
                         tbl_mode_w_digit_w[m_digits[0]], HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(f_mode->pic_timer[MODE_TIMER_IDX_M10],
                              tbl_mode_w_digit_w[m_digits[0]], HEAT_W_DIGIT_MAX_H);

    func_mode_pic_pos_tr(f_mode->pic_timer[MODE_TIMER_IDX_M1],
                         HEAT_TIMER_TR_M1_X, HEAT_TIMER_TR_Y,
                         tbl_mode_w_digit_w[m_digits[1]], HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(f_mode->pic_timer[MODE_TIMER_IDX_M1],
                              tbl_mode_w_digit_w[m_digits[1]], HEAT_W_DIGIT_MAX_H);
}

static void func_mode_timer_update(f_mode_t *f_mode, u8 hour, u8 min)
{
    u8 digits[MODE_TIMER_IDX_CNT];
    u16 timer_key;
    u8 i;

    if (f_mode == NULL || sys_cb.flag_swithing) {
        return;
    }

    digits[MODE_TIMER_IDX_H10] = hour / 10;
    digits[MODE_TIMER_IDX_H1] = hour % 10;
    digits[MODE_TIMER_IDX_M10] = min / 10;
    digits[MODE_TIMER_IDX_M1] = min % 10;
    timer_key = (u16)hour * 100 + min;
    if (f_mode->last_timer_key == timer_key) {
        func_mode_timer_layout(f_mode, hour, min);
        return;
    }
    f_mode->last_timer_key = timer_key;

    /* 使用 Heat/Mode 共享的 timer RAM，不碰 home_ui_digit_ram，彻底避免与 Home clock 的 C241 */
    os_spiflash_read(home_ui_shared_timer_colon_ram, UI_BUF_HOME_WBX_BIN, UI_LEN_HOME_WBX_BIN);
    if (gui_set_ram_check(home_ui_shared_timer_colon_ram, __func__)) {
        compo_picturebox_set_ram(f_mode->pic_timer_colon, home_ui_shared_timer_colon_ram);
        compo_picturebox_set_visible(f_mode->pic_timer_colon, true);
    }

    for (i = 0; i < MODE_TIMER_IDX_CNT; i++) {
        u8 d = digits[i];

        os_spiflash_read(home_ui_shared_timer_digit_ram[i], tbl_mode_w_digit_addr[d], tbl_mode_w_digit_len[d]);
        if (gui_set_ram_check(home_ui_shared_timer_digit_ram[i], __func__)) {
            compo_picturebox_set_ram(f_mode->pic_timer[i], home_ui_shared_timer_digit_ram[i]);
            compo_picturebox_set_visible(f_mode->pic_timer[i], true);
        }
    }

    func_mode_timer_layout(f_mode, hour, min);
}

static void func_mode_display_refresh(f_mode_t *f_mode)
{
    u8 hour;
    u8 min;
    u16 temp_f;

    if (f_mode == NULL) {
        return;
    }

    if (f_mode->ui_state == MODE_UI_HEATING) {
#if FUNC_LUNCHBOX_UART_EN
        if (f_mode->heat_live_ready) {
            hour = (u8)(f_mode->heat_live_remain_min / 60);
            min = (u8)(f_mode->heat_live_remain_min % 60);
            temp_f = f_mode->heat_live_temp_f;
        } else {
            const mode_tab_preset_t *preset = &tbl_mode_tab_preset[f_mode->tab];

            hour = preset->hour;
            min = preset->min;
            temp_f = preset->temp_f;
        }
#else
        hour = (u8)(mode_countdown_remain_sec / 3600);
        min = (u8)((mode_countdown_remain_sec % 3600) / 60);
        temp_f = tbl_mode_tab_preset[f_mode->tab].temp_f;
#endif
        if (hour > 99) {
            hour = 99;
        }
        func_mode_timer_update(f_mode, hour, min);
#if FUNC_LUNCHBOX_UART_EN
        func_mode_status_temp_update(f_mode, temp_f);
#endif
        return;
    }

    hour = (u8)(mode_countdown_remain_sec / 3600);
    min = (u8)((mode_countdown_remain_sec % 3600) / 60);
    if (hour > 99) {
        hour = 99;
    }
    func_mode_timer_update(f_mode, hour, min);
}

static void func_mode_tab_preview_timer(f_mode_t *f_mode, u8 tab)
{
    const mode_tab_preset_t *preset;

    if (f_mode == NULL || tab >= MODE_TAB_CNT) {
        return;
    }

    preset = &tbl_mode_tab_preset[tab];
    func_mode_countdown_set(preset->hour, preset->min);
    func_mode_countdown_stop();
    f_mode->last_timer_key = 0xffff;
    func_mode_display_refresh(f_mode);
}

static void func_mode_tab_preview_temp(f_mode_t *f_mode, u8 tab)
{
    const mode_tab_preset_t *preset;

    if (f_mode == NULL || tab >= MODE_TAB_CNT) {
        return;
    }

    preset = &tbl_mode_tab_preset[tab];
    if (f_mode->last_temp_f != preset->temp_f) {
        f_mode->last_temp_f = preset->temp_f;
        func_mode_status_temp_update(f_mode, preset->temp_f);
    }
}

static void func_mode_tab_preview_apply(f_mode_t *f_mode, u8 tab)
{
    func_mode_tab_preview_timer(f_mode, tab);
    func_mode_tab_preview_temp(f_mode, tab);
}

static void func_mode_tab_select(f_mode_t *f_mode, u8 tab)
{
    if (f_mode == NULL || tab >= MODE_TAB_CNT) {
        return;
    }
    f_mode->tab = tab;
    func_mode_tab_preview_apply(f_mode, tab);
    func_mode_tab_refresh(f_mode);
}

static void func_mode_tab_select_next(f_mode_t *f_mode)
{
    if (f_mode == NULL) {
        return;
    }
    func_mode_tab_select(f_mode, (u8)((f_mode->tab + 1) % MODE_TAB_CNT));
}

static void func_mode_start_heating(f_mode_t *f_mode)
{
    const mode_tab_preset_t *preset;

    if (f_mode == NULL || f_mode->ui_state != MODE_UI_IDLE) {
        return;
    }

    preset = &tbl_mode_tab_preset[f_mode->tab];

    // 鸡腿/意面模式：跳转到加热界面并自动开始加热
    if (f_mode->tab == MODE_TAB_PASTA || f_mode->tab == MODE_TAB_CHICKEN) {
        u32 duration_min = (u32)preset->hour * 60 + preset->min;
        if (duration_min == 0) duration_min = 1;
        lb_mode_to_heat_set(tbl_mode_tab_to_proto[f_mode->tab],
                            preset->temp_f, preset->hour, preset->min);
        func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        return;
    }

    // 保温模式：原地加热（保持原有逻辑）
    f_mode->ui_state = MODE_UI_HEATING;
    f_mode->screen_locked = false;
    f_mode->heat_start_tick = tick_get();
    f_mode->heat_total_sec = (u32)preset->hour * 3600 + (u32)preset->min * 60;
    if (f_mode->heat_total_sec == 0) {
        f_mode->heat_total_sec = 60;
    }
    mode_countdown_remain_sec = f_mode->heat_total_sec;
    f_mode->heat_live_ready = false;
    f_mode->heat_live_remain_min = 0;
    f_mode->heat_live_temp_f = 0;
    f_mode->last_timer_key = 0xffff;
    f_mode->last_temp_f = 0xffff;
#if !FUNC_LUNCHBOX_UART_EN
    func_mode_countdown_start();
#endif
    func_mode_display_refresh(f_mode);
    func_mode_lock_icon_apply(f_mode);

#if FUNC_LUNCHBOX_UART_EN
    {
        u32 duration_min = f_mode->heat_total_sec / 60;

        if (duration_min == 0) {
            duration_min = 1;
        }
        lunchbox_heat_start(tbl_mode_tab_to_proto[f_mode->tab],
                            lunchbox_temp_f_to_idx(preset->temp_f),
                            duration_min);
    }
#endif
}

static void func_mode_timer_gpu_detach(f_mode_t *f_mode)
{
    u8 i;

    if (f_mode == NULL) {
        return;
    }

    home_gpu_wait_idle();
    for (i = 0; i < MODE_TIMER_IDX_CNT; i++) {
        home_ui_gpu_pic_detach(f_mode->pic_timer[i]);
    }
    home_ui_gpu_pic_detach(f_mode->pic_timer_colon);
    f_mode->last_timer_key = 0xffff;
    home_gpu_wait_idle();
}

static void func_mode_switch_home(f_mode_t *f_mode)
{
    if (f_mode == NULL || func_cb.sta != FUNC_MODE || sys_cb.flag_swithing) {
        return;
    }

    func_mode_timer_gpu_detach(f_mode);
    WDT_CLR();
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

static void func_mode_power_key(f_mode_t *f_mode)
{
    if (f_mode == NULL) {
        return;
    }

    if (f_mode->ui_state == MODE_UI_HEATING) {
        func_mode_countdown_stop();
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_heat_stop();
#endif
        f_mode->screen_locked = false;
    }

    /* 任何状态下按开关键均返回 Home（含 IDLE 选择页） */
    func_mode_switch_home(f_mode);
}

static bool func_mode_key_allowed(f_mode_t *f_mode, size_msg_t msg)
{
    if (f_mode == NULL) {
        return true;
    }
#if ELUNCHBOX_PANEL_EN
    if (f_mode->display_stage != 0 && msg != MODE_MSG_POWER) {
        return false;
    }
#endif
    if (msg == MODE_MSG_POWER || msg == MODE_MSG_OK || msg == KU_LEFT) {
        return true;
    }
    if (f_mode->screen_locked) {
        return false;
    }
    if (f_mode->ui_state != MODE_UI_IDLE) {
        return false;
    }
    return true;
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

static void func_mode_tab_icon_ram_apply(u8 *ram, u16 len, bool selected)
{
    u16 i;
    u16 pix_cnt;

    if (!selected || len <= 8) {
        return;
    }

    pix_cnt = (len - 8) / 2;
    for (i = 0; i < pix_cnt; i++) {
        u8 *px = &ram[8 + i * 2];

        if (GET_LE16(px) == MODE_TAB_ICON_BG_BLACK) {
            PUT_LE16(px, MODE_COLOR_BLUE);
        }
    }
}

static void func_mode_tab_icon_update(f_mode_t *f_mode, u8 idx)
{
    bool selected = (idx == f_mode->tab);

    if (f_mode->tabs[idx].pic == NULL) {
        return;
    }

    os_spiflash_read(mode_tab_icon_ram[idx], tbl_mode_icon_addr[idx], tbl_mode_icon_len[idx]);
    func_mode_tab_icon_ram_apply(mode_tab_icon_ram[idx], tbl_mode_icon_len[idx], selected);
    if (func_mode_gpu_ram_set(mode_tab_icon_ram[idx], HOME_ICON_RAM_SIZE,
                              tbl_mode_icon_len[idx], f_mode->tabs[idx].pic)) {
        compo_picturebox_set_size(f_mode->tabs[idx].pic,
                                  tbl_mode_icon_w[idx], tbl_mode_icon_h[idx]);
    }
}

static void func_mode_tab_line_update(f_mode_t *f_mode, u8 idx)
{
    bool selected = (idx == f_mode->tab);

    if (f_mode->tabs[idx].line == NULL) {
        return;
    }

    compo_shape_set_visible(f_mode->tabs[idx].line, true);
    compo_shape_set_color(f_mode->tabs[idx].line, selected ? COLOR_WHITE : MODE_COLOR_BLUE);
}

static void func_mode_tab_label_apply(compo_picturebox_t *pic, u8 *ram, u16 buf_size,
                                      const char *label, s16 cx, bool selected)
{
    home_tab_label_apply(pic, ram, buf_size, label, cx, MODE_TAB_LABEL_Y, selected, MODE_COLOR_BLUE);
}

static void func_mode_tab_label_update(f_mode_t *f_mode, u8 idx)
{
    bool selected = (idx == f_mode->tab);

    if (mode_tab_label_ram_ptr[idx] == NULL) {
        return;
    }
    func_mode_tab_label_apply(f_mode->tabs[idx].label, mode_tab_label_ram_ptr[idx],
                              home_tab_label_ram_size(tbl_mode_tab_label[idx]),
                              tbl_mode_tab_label[idx], tbl_mode_tab_x[idx], selected);
}

static compo_shape_t *func_mode_tab_line_create(compo_form_t *frm, u16 id, s16 x)
{
    compo_shape_t *shape = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(shape, id);
    compo_shape_set_location(shape, x, MODE_TAB_DASH_Y, MODE_TAB_LINE_W, MODE_TAB_LINE_H);
    compo_shape_set_color(shape, MODE_COLOR_BLUE);
    compo_shape_set_radius(shape, 1);
    compo_shape_set_visible(shape, false);
    return shape;
}

static void func_mode_tab_create(compo_form_t *frm, u8 idx, u16 id_base, const char *label, s16 x)
{
    compo_shape_t *sel_bg;
    compo_shape_t *border_out;
    compo_shape_t *border_in;
    compo_picturebox_t *pic;
    compo_picturebox_t *pic_label;
    compo_button_t *btn;

    sel_bg = func_mode_shape_create(frm, id_base + 0, x, MODE_TAB_BTN_Y,
                                    MODE_TAB_BTN_W, MODE_TAB_BTN_H, MODE_COLOR_BLUE, MODE_TAB_BTN_R);
    border_out = func_mode_shape_create(frm, id_base + 1, x, MODE_TAB_BTN_Y,
                                        MODE_TAB_BTN_W, MODE_TAB_BTN_H, COLOR_WHITE, MODE_TAB_BTN_R);
    border_in = func_mode_shape_create(frm, id_base + 2, x, MODE_TAB_BTN_Y,
                                       MODE_TAB_BTN_W - 4, MODE_TAB_BTN_H - 4, COLOR_BLACK, MODE_TAB_BTN_R_IN);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, tbl_mode_tab_pic_id[idx]);
    compo_picturebox_set_visible(pic, false);
    compo_picturebox_set_pos(pic, x, MODE_TAB_ICON_Y);

    pic_label = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic_label, id_base + 4);
    compo_picturebox_set_visible(pic_label, false);

    func_mode_tab_line_create(frm, tbl_mode_tab_dash_id[idx], x);

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
    tab->line = compo_getobj_byid(tbl_mode_tab_dash_id[idx]);
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
        func_mode_tab_line_update(f_mode, i);
        func_mode_tab_label_update(f_mode, i);
    }
}

#if ELUNCHBOX_PANEL_EN
static u8 mode_tab_icon_defer;

static void func_mode_tab_refresh_enter(f_mode_t *f_mode)
{
    u8 i;

    mode_tab_icon_defer = 0;
    for (i = 0; i < MODE_TAB_CNT; i++) {
        mode_tab_ui_t *tab = &f_mode->tabs[i];
        bool selected = (i == f_mode->tab);

        compo_shape_set_visible(tab->sel_bg, selected);
        compo_shape_set_visible(tab->border_out, !selected);
        compo_shape_set_visible(tab->border_in, !selected);
        func_mode_tab_line_update(f_mode, i);
        func_mode_tab_label_update(f_mode, i);
        if (selected) {
            func_mode_tab_icon_update(f_mode, i);
        } else {
            mode_tab_icon_defer |= (u8)(1u << i);
        }
    }
}
#endif

static void func_mode_button_click(f_mode_t *f_mode)
{
    int id = compo_get_button_id();

    if (f_mode == NULL || f_mode->ui_state != MODE_UI_IDLE){
        return;
    }

    switch (id) {
    case COMPO_ID_TAB0_BTN:
        func_mode_tab_select(f_mode, MODE_TAB_PASTA);
        break;

    case COMPO_ID_TAB1_BTN:
        func_mode_tab_select(f_mode, MODE_TAB_CHICKEN);
        break;

    case COMPO_ID_TAB2_BTN:
        func_mode_tab_select(f_mode, MODE_TAB_INSULATION);
        break;

    default:
        break;
    }
}

compo_form_t *func_mode_form_create_core(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_picturebox_t *pic;
    u8 i;

    home_top_time_create(frm, UI_MODE_PLACEHOLDER,
                         COMPO_ID_PIC_TOP_TIME_H10, COMPO_ID_PIC_TOP_TIME_H1,
                         COMPO_ID_PIC_TOP_TIME_COLON, COMPO_ID_PIC_TOP_TIME_M10,
                         COMPO_ID_PIC_TOP_TIME_M1, COMPO_ID_PIC_TOP_TIME_AMPM);

    for (i = 0; i < MODE_TEMP_IDX_CNT; i++) {
        pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
        compo_setid(pic, tbl_mode_status_temp_id[i]);
        compo_picturebox_set_visible(pic, false);
        func_mode_pic_pos_tr(pic, tbl_mode_status_temp_tr_x[i], MODE_STATUS_TEMP_TR_Y,
                             MODE_G0_W, MODE_G_DIGIT_MAX_H);
        compo_picturebox_set_size(pic, MODE_G0_W, MODE_G_DIGIT_MAX_H);
    }

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_STATUS_TEMPF);
    compo_picturebox_set_visible(pic, false);
    func_mode_pic_pos_tr(pic, MODE_STATUS_TEMP_TR_UNIT_X, MODE_STATUS_TEMP_TR_Y,
                         MODE_GH_W, MODE_GH_H);
    compo_picturebox_set_size(pic, MODE_GH_W, MODE_GH_H);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BT);
    compo_picturebox_set_visible(pic, false);
    compo_picturebox_set_pos(pic, MODE_STATUS_BT_X, MODE_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BT_W, HOME_STATUS_BT_H);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_LOCK);
    compo_picturebox_set_visible(pic, false);
    compo_picturebox_set_pos(pic, MODE_STATUS_LOCK_X, MODE_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BAT);
    compo_picturebox_set_visible(pic, false);
    compo_picturebox_set_pos(pic, MODE_STATUS_BAT_X, MODE_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_H10);
    compo_picturebox_set_visible(pic, false);
    func_mode_pic_pos_tr(pic, HEAT_TIMER_TR_H10_X, HEAT_TIMER_TR_Y, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_H1);
    compo_picturebox_set_visible(pic, false);
    func_mode_pic_pos_tr(pic, HEAT_TIMER_TR_H1_X, HEAT_TIMER_TR_Y, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_M10);
    compo_picturebox_set_visible(pic, false);
    func_mode_pic_pos_tr(pic, HEAT_TIMER_TR_M10_X, HEAT_TIMER_TR_Y, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_M1);
    compo_picturebox_set_visible(pic, false);
    func_mode_pic_pos_tr(pic, HEAT_TIMER_TR_M1_X, HEAT_TIMER_TR_Y, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);
    compo_picturebox_set_size(pic, HEAT_W0X_W, HEAT_W_DIGIT_MAX_H);

    pic = compo_picturebox_create(frm, UI_MODE_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_TIMER_COLON);
    compo_picturebox_set_visible(pic, false);
    func_mode_pic_pos_tr(pic, HEAT_TIMER_TR_COLON_X, HEAT_TIMER_TR_Y, HEAT_WBX_W, HEAT_WBX_H);
    compo_picturebox_set_size(pic, HEAT_WBX_W, HEAT_WBX_H);

    return frm;
}

static void func_mode_form_add_tabs(compo_form_t *frm)
{
    if (frm == NULL) {
        return;
    }

    func_mode_tab_create(frm, MODE_TAB_PASTA, COMPO_ID_TAB0_SEL_BG,
                         tbl_mode_tab_label[MODE_TAB_PASTA], tbl_mode_tab_x[MODE_TAB_PASTA]);
    func_mode_tab_create(frm, MODE_TAB_CHICKEN, COMPO_ID_TAB1_SEL_BG,
                         tbl_mode_tab_label[MODE_TAB_CHICKEN], tbl_mode_tab_x[MODE_TAB_CHICKEN]);
    func_mode_tab_create(frm, MODE_TAB_INSULATION, COMPO_ID_TAB2_SEL_BG,
                         tbl_mode_tab_label[MODE_TAB_INSULATION], tbl_mode_tab_x[MODE_TAB_INSULATION]);
}

compo_form_t *func_mode_form_create(void)
{
    compo_form_t *frm = func_mode_form_create_core();

    func_mode_form_add_tabs(frm);
    return frm;
}

static void func_mode_process(void)
{
    f_mode_t *f_mode = (f_mode_t *)func_cb.f_cb;

    if (f_mode != NULL) {
#if ELUNCHBOX_PANEL_EN
        if (f_mode->display_stage != 0) {
            switch (f_mode->display_stage) {
            case 1: {
                tm_t tm = rtc_clock_get();
                home_top_time_refresh(&f_mode->top_time, &tm);
                f_mode->display_stage = 2;
                break;
            }
            case 2:
                func_mode_tab_preview_timer(f_mode, f_mode->tab);
                f_mode->display_stage = 3;
                break;
            case 3:
                func_mode_tab_preview_temp(f_mode, f_mode->tab);
                f_mode->display_stage = 4;
                break;
            case 4:
                func_mode_tab_refresh_enter(f_mode);
                f_mode->display_stage = 0;
                break;
            default:
                f_mode->display_stage = 0;
                break;
            }
            WDT_CLR();
            func_process();
            return;
        }
        if (mode_tab_icon_defer) {
            u8 i;

            for (i = 0; i < MODE_TAB_CNT; i++) {
                if (mode_tab_icon_defer & (1u << i)) {
                    func_mode_tab_icon_update(f_mode, i);
                    mode_tab_icon_defer &= (u8)~(1u << i);
                    break;
                }
            }
        }
#endif
        func_mode_status_refresh(f_mode);
    }
    func_process();
}

static void func_mode_message(size_msg_t msg)
{
    f_mode_t *f_mode = (f_mode_t *)func_cb.f_cb;

    switch (msg) {
    case MSG_CTP_CLICK:
        if (f_mode != NULL &&
#if ELUNCHBOX_PANEL_EN
            f_mode->display_stage == 0 &&
#endif
            f_mode->ui_state == MODE_UI_IDLE && !f_mode->screen_locked) {
            func_mode_button_click(f_mode);
        }
        break;

    case KU_MODE:
        if (f_mode != NULL &&
#if ELUNCHBOX_PANEL_EN
            f_mode->display_stage == 0 &&
#endif
            f_mode->ui_state == MODE_UI_IDLE && !f_mode->screen_locked) {
            func_mode_tab_select_next(f_mode);
        }
        break;

    case MODE_MSG_OK:
#if ELUNCHBOX_PANEL_EN
        if (f_mode == NULL || f_mode->display_stage != 0) {
            break;
        }
#endif
        func_mode_start_heating(f_mode);
        break;

    case MODE_MSG_POWER:
        func_mode_power_key(f_mode);
        break;

    case KU_LEFT:
        if (f_mode != NULL) {
            f_mode->screen_locked = !f_mode->screen_locked;
            func_mode_lock_icon_apply(f_mode);
        }
        break;

    default:
        if (func_mode_key_allowed(f_mode, msg)) {
            func_message(msg);
        }
        break;
    }
}

void func_mode_enter(void)
{
    f_mode_t *f_mode;

    printf("func_mode_enter\n");
    WDT_CLR();
    if (!func_mode_tab_label_ram_alloc()) {
        printf("func_mode: tab label ram alloc fail\n");
    }
    func_cb.f_cb = func_zalloc(sizeof(f_mode_t));

    f_mode = (f_mode_t *)func_cb.f_cb;
    f_mode->tab = MODE_TAB_PASTA;
    f_mode->ui_state = MODE_UI_IDLE;
    f_mode->screen_locked = false;
    f_mode->heat_start_tick = 0;
    f_mode->heat_total_sec = 0;
    f_mode->heat_live_remain_min = 0;
    f_mode->heat_live_temp_f = 0;
    f_mode->heat_live_ready = false;
    f_mode->last_top_min = 0xff;
    f_mode->last_top_sec = 0xff;
    f_mode->last_timer_key = 0xffff;
    f_mode->last_temp_f = 0xffff;

#if ELUNCHBOX_PANEL_EN
    home_gpu_wait_idle();
    WDT_CLR();
#endif
    func_cb.frm_main = func_mode_form_create();
    WDT_CLR();
    func_mode_form_bind_core(f_mode);
    func_mode_form_bind_tabs(f_mode);
    WDT_CLR();

    home_top_time_bind(&f_mode->top_time, COMPO_ID_PIC_TOP_TIME_H10, COMPO_ID_PIC_TOP_TIME_H1,
                       COMPO_ID_PIC_TOP_TIME_COLON, COMPO_ID_PIC_TOP_TIME_M10,
                       COMPO_ID_PIC_TOP_TIME_M1, COMPO_ID_PIC_TOP_TIME_AMPM);
#if ELUNCHBOX_PANEL_EN
    home_ui_digit_pool_reset();
    f_mode->display_stage = 1;
    func_mode_status_icons_apply(f_mode);
    tft_bglight_force_on();
    printf("func_mode_enter: ok stage=%u\n", f_mode->display_stage);
#else
    {
        tm_t tm = rtc_clock_get();
        home_top_time_refresh(&f_mode->top_time, &tm);
    }
    func_mode_status_icons_apply(f_mode);
    func_mode_tab_preview_apply(f_mode, f_mode->tab);
    func_mode_tab_refresh(f_mode);
    func_mode_status_refresh(f_mode);
#endif
    heat_display_register(func_mode_display_on_info);
}

void func_mode_exit(void)
{
    heat_display_unregister();
    func_mode_countdown_stop();
    func_mode_tab_label_ram_free();
#if ELUNCHBOX_PANEL_EN
    mode_tab_icon_defer = 0;
#endif
    func_cb.last = FUNC_MODE;
}

#if ELUNCHBOX_PANEL_EN
static u8 mode_idle_preload_step;

void func_mode_idle_preload_reset(void)
{
    mode_idle_preload_step = 0;
}

void func_mode_idle_preload_step(void)
{
    static const u8 timer_digits[4] = {0, 1, 0, 0};
    u8 idx;

    if (mode_idle_preload_step >= 7) {
        return;
    }

    switch (mode_idle_preload_step) {
    case 0:
        os_spiflash_read(mode_tab_icon_ram[MODE_TAB_PASTA],
                         tbl_mode_icon_addr[MODE_TAB_PASTA],
                         tbl_mode_icon_len[MODE_TAB_PASTA]);
        break;
    case 1:
        os_spiflash_read(home_ui_shared_timer_colon_ram,
                         UI_BUF_HOME_WBX_BIN, UI_LEN_HOME_WBX_BIN);
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        idx = (u8)(mode_idle_preload_step - 2);
        os_spiflash_read(home_ui_shared_timer_digit_ram[idx],
                         tbl_mode_w_digit_addr[timer_digits[idx]],
                         tbl_mode_w_digit_len[timer_digits[idx]]);
        break;
    case 6:
        home_ui_shared_dash_init();
        break;
    default:
        break;
    }
    mode_idle_preload_step++;
}
#endif

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
