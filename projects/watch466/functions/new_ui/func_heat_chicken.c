#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"
#include "general_ui.h"
#include "ui.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/* 时间 5 分钟步进：1H ~ 2H */
#define TIME_MIN_MINUTES  60
#define TIME_MAX_MINUTES  120
#define TIME_STEP         5
#define TIME_DEFAULT      TIME_MIN_MINUTES

/* 时间进度条图片（13 挡位，对应 60~120min，5min 步进） */
static const u32 TIME_SCHEDULE_PICS[13] = {
    UI_BUF_NEW_UI_TIME_SCHEDULE_0_BIN,
    UI_BUF_NEW_UI_TIME_SCHEDULE_1_BIN,
    UI_BUF_NEW_UI_TIME_SCHEDULE_2_BIN,
    UI_BUF_NEW_UI_TIME_SCHEDULE_3_BIN,
    UI_BUF_NEW_UI_TIME_SCHEDULE_4_BIN,
    UI_BUF_NEW_UI_TIME_SCHEDULE_5_BIN,
    UI_BUF_NEW_UI_TIME_SCHEDULE_6_BIN,
    UI_BUF_NEW_UI_TIME_SCHEDULE_7_BIN,
    UI_BUF_NEW_UI_TIME_SCHEDULE_8_BIN,
    UI_BUF_NEW_UI_TIME_SCHEDULE_9_BIN,
    UI_BUF_NEW_UI_TIME_SCHEDULE_10_BIN,
    UI_BUF_NEW_UI_TIME_SCHEDULE_11_BIN,
    UI_BUF_NEW_UI_TIME_SCHEDULE_12_BIN,
};

/* 温度进度条图片（5 挡位，对应 140/158/176/194/212F = 协议档位 2~6） */
static const u32 TMP_SCHEDULE_PICS[5] = {
    UI_BUF_NEW_UI_TMP_SCHEDULE_0_BIN,
    UI_BUF_NEW_UI_TMP_SCHEDULE_1_BIN,
    UI_BUF_NEW_UI_TMP_SCHEDULE_2_BIN,
    UI_BUF_NEW_UI_TMP_SCHEDULE_3_BIN,
    UI_BUF_NEW_UI_TMP_SCHEDULE_4_BIN,
};

/* 协议温度档位(0~6) → 温度进度条图片索引(0~4)，档位 2 = 140F 为第一格 */
static u8 chicken_temp_pic_idx(u8 temp_idx)
{
    if (temp_idx < 2) return 0;
    if (temp_idx > 6) return 4;
    return (u8)(temp_idx - 2);
}

/* 本模式的预设时长 → 页面可选范围内的初值（UI 只有 60~120min / 5min 步进，
 * 预设表若给了范围外的值必须夹紧，否则进度条图片索引会越界） */
static u8 chicken_preset_time_min(void)
{
    u32 d = lunchbox_mode_get_duration(LB_MODE_CHICKEN);

    if (d < TIME_MIN_MINUTES) return TIME_MIN_MINUTES;
    if (d > TIME_MAX_MINUTES) return TIME_MAX_MINUTES;
    d -= (d - TIME_MIN_MINUTES) % TIME_STEP;      /* 对齐 5 分钟步进 */
    return (u8)d;
}

/* 加热鸡肉页私有状态 */
typedef struct
{
    u8 display_stage;
    u8 time_min;
    general_status_bar_t sb;
    compo_textbox_t *txt_heat;
    compo_textbox_t *txt_time;
    compo_textbox_t *txt_heat_value;
    compo_textbox_t *txt_time_value;
    compo_picturebox_t *TMP;
    compo_picturebox_t *TIME;
    compo_textbox_t *txt_tmp_scale1;
    compo_textbox_t *txt_tmp_scale2;
    compo_textbox_t *txt_tmp_scale3;
    compo_textbox_t *txt_tmp_scale4;
    compo_textbox_t *txt_tmp_scale5;
    compo_textbox_t *txt_time_scale1;
    compo_textbox_t *txt_time_scale2;
    compo_textbox_t *txt_time_scale3;
    compo_picturebox_t *tmp_pic;
    compo_picturebox_t *time_pic;
} f_heat_chicken_t;

/* ---- 显示刷新 ---- */
static void heat_chicken_update_display(void)
{
    f_heat_chicken_t *inf = (f_heat_chicken_t *)func_cb.f_cb;
    char buf[16];

    if (inf->time_min % 60 == 0) {
        snprintf(buf, sizeof(buf), "%uH", inf->time_min / 60);
    } else {
        snprintf(buf, sizeof(buf), "%uH%02uMin",
                 inf->time_min / 60, inf->time_min % 60);
    }
    compo_textbox_set(inf->txt_time_value, buf);
    compo_picturebox_set(inf->time_pic,
                         TIME_SCHEDULE_PICS[(inf->time_min - TIME_MIN_MINUTES) / TIME_STEP]);
}

/* ---- 按键处理 ---- */
static void func_heat_chicken_handle_keys(void)
{
    f_heat_chicken_t *inf = (f_heat_chicken_t *)func_cb.f_cb;
    func_key_event_t evt;

    while (func_key_get_event(&evt)) {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key) {
        case FUNC_KEY_UP:
            if (inf->time_min <= TIME_MIN_MINUTES)
                inf->time_min = TIME_MAX_MINUTES;
            else
                inf->time_min -= TIME_STEP;
            heat_chicken_update_display();
            break;

        case FUNC_KEY_DOWN:
            inf->time_min += TIME_STEP;
            if (inf->time_min > TIME_MAX_MINUTES)
                inf->time_min = TIME_MIN_MINUTES;
            heat_chicken_update_display();
            break;

        case FUNC_KEY_CONFIRM:
            /* 下发鸡腿模式加热: 温度取预设表(不可改), 时长用页面选的 */
#if FUNC_LUNCHBOX_UART_EN
            {
                u8 t_idx = lunchbox_mode_get_temp(LB_MODE_CHICKEN);
                lb_heat_cmd_start(LB_MODE_CHICKEN, t_idx, inf->time_min);
                printf("chicken: start %uF %umin\n",
                       (unsigned)lunchbox_temp_idx_to_f(t_idx), (unsigned)inf->time_min);
            }
#endif
            func_cb.sta = FUNC_NEW_HEAT_PAGE;
            break;

        case FUNC_KEY_BACK:
            func_cb.sta = FUNC_NEW_MODE;
            break;

        /* 直接按键：加热键 → 加热设置页 */
        case FUNC_KEY_HEAT:
            func_cb.sta = FUNC_NEW_HEAT_SET;
            break;

        /* 直接按键：模式键 → 模式页 */
        case FUNC_KEY_MODE:
            func_cb.sta = FUNC_NEW_MODE;
            break;

        /* 直接按键：预约键 → 预约页 */
        case FUNC_KEY_RESERVATION:
            func_cb.sta = FUNC_APPOINTMENT_TIME;
            break;


        default:
            break;
        }
    }
}

/* ---- 每帧处理 ---- */
static void func_heat_chicken_process(void)
{
    f_heat_chicken_t *inf = (f_heat_chicken_t *)func_cb.f_cb;

#if ELUNCHBOX_PANEL_EN
    if (inf->display_stage != 0) {
        inf->display_stage = 0;
        return;
    }
#endif

    func_key_poll();
    func_heat_chicken_handle_keys();
    func_key_lock_poll();

    if (func_key_lock_gui_dirty()) {
        if (func_key_lock_overlay_visible())
            func_lock_page_show(func_key_lock_overlay_is_unlock());
        else
            func_lock_page_hide();
    }

    func_process();
    general_status_bar_tick(&inf->sb);
}

static void func_heat_chicken_message(size_msg_t msg)
{
    func_message(msg);
}

/* ---- 生命周期 ---- */
compo_form_t *func_heat_chicken_form_create(void)
{
    f_heat_chicken_t *inf = (f_heat_chicken_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);

    /* 白色背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部状态栏 */
    general_status_bar_create(frm, &inf->sb, i18n[STR_CHICKEN_MODE], &g_ui_sys);

    /* ---- 温度行（值取自预设表，不可调，未选中态） ---- */
    inf->txt_heat = compo_textbox_create(frm, 12);
    compo_textbox_set_location(inf->txt_heat, GUI_SCREEN_CENTER_X - 80,
                               GUI_SCREEN_CENTER_Y - 50, 0, 0);
    compo_textbox_set_autosize(inf->txt_heat, true);
    compo_textbox_set_align_center(inf->txt_heat, true);
    compo_textbox_set_font(inf->txt_heat, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_heat, COLOR_BLUE);
    compo_textbox_set(inf->txt_heat, i18n[STR_HEAT_LAB]);

    inf->TMP = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_WHITE_TIME_BIN);
    compo_picturebox_set_pos(inf->TMP, GUI_SCREEN_CENTER_X + 100,
                             GUI_SCREEN_CENTER_Y - 50);

    inf->txt_heat_value = compo_textbox_create(frm, 12);
    compo_textbox_set_location(inf->txt_heat_value, GUI_SCREEN_CENTER_X + 100,
                               GUI_SCREEN_CENTER_Y - 51, 0, 0);
    compo_textbox_set_autosize(inf->txt_heat_value, true);
    compo_textbox_set_align_center(inf->txt_heat_value, true);
    compo_textbox_set_font(inf->txt_heat_value, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_heat_value, COLOR_BLUE);
    {
        char tbuf[8];
        snprintf(tbuf, sizeof(tbuf), "%%uF",
                 (unsigned)lunchbox_temp_idx_to_f(lunchbox_mode_get_temp(LB_MODE_CHICKEN)));
        compo_textbox_set(inf->txt_heat_value, tbuf);
    }

    /* 温度进度条图片: 按预设表的温度档位取图, 温度不可调 */
    inf->tmp_pic = compo_picturebox_create(frm,
            TMP_SCHEDULE_PICS[chicken_temp_pic_idx(lunchbox_mode_get_temp(LB_MODE_CHICKEN))]);
    compo_picturebox_set_pos(inf->tmp_pic, GUI_SCREEN_CENTER_X,
                             GUI_SCREEN_CENTER_Y - 15);

    /* 温度刻度(固定) */
    inf->txt_tmp_scale1 = compo_textbox_create(frm, 12);
    compo_textbox_set_location(inf->txt_tmp_scale1, GUI_SCREEN_CENTER_X - 115,
                               GUI_SCREEN_CENTER_Y + 10, 0, 0);
    compo_textbox_set_autosize(inf->txt_tmp_scale1, true);
    compo_textbox_set_align_center(inf->txt_tmp_scale1, true);
    compo_textbox_set_font(inf->txt_tmp_scale1, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_tmp_scale1, COLOR_GRAY);
    compo_textbox_set(inf->txt_tmp_scale1, "140F");

    inf->txt_tmp_scale2 = compo_textbox_create(frm, 12);
    compo_textbox_set_location(inf->txt_tmp_scale2, GUI_SCREEN_CENTER_X - 57,
                               GUI_SCREEN_CENTER_Y + 10, 0, 0);
    compo_textbox_set_autosize(inf->txt_tmp_scale2, true);
    compo_textbox_set_align_center(inf->txt_tmp_scale2, true);
    compo_textbox_set_font(inf->txt_tmp_scale2, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_tmp_scale2, COLOR_GRAY);
    compo_textbox_set(inf->txt_tmp_scale2, "158F");

    inf->txt_tmp_scale3 = compo_textbox_create(frm, 12);
    compo_textbox_set_location(inf->txt_tmp_scale3, GUI_SCREEN_CENTER_X,
                               GUI_SCREEN_CENTER_Y + 10, 0, 0);
    compo_textbox_set_autosize(inf->txt_tmp_scale3, true);
    compo_textbox_set_align_center(inf->txt_tmp_scale3, true);
    compo_textbox_set_font(inf->txt_tmp_scale3, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_tmp_scale3, COLOR_GRAY);
    compo_textbox_set(inf->txt_tmp_scale3, "176F");

    inf->txt_tmp_scale4 = compo_textbox_create(frm, 12);
    compo_textbox_set_location(inf->txt_tmp_scale4, GUI_SCREEN_CENTER_X + 57,
                               GUI_SCREEN_CENTER_Y + 10, 0, 0);
    compo_textbox_set_autosize(inf->txt_tmp_scale4, true);
    compo_textbox_set_align_center(inf->txt_tmp_scale4, true);
    compo_textbox_set_font(inf->txt_tmp_scale4, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_tmp_scale4, COLOR_GRAY);
    compo_textbox_set(inf->txt_tmp_scale4, "194F");

    inf->txt_tmp_scale5 = compo_textbox_create(frm, 12);
    compo_textbox_set_location(inf->txt_tmp_scale5, GUI_SCREEN_CENTER_X + 115,
                               GUI_SCREEN_CENTER_Y + 10, 0, 0);
    compo_textbox_set_autosize(inf->txt_tmp_scale5, true);
    compo_textbox_set_align_center(inf->txt_tmp_scale5, true);
    compo_textbox_set_font(inf->txt_tmp_scale5, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_tmp_scale5, COLOR_GRAY);
    compo_textbox_set(inf->txt_tmp_scale5, "212F");

    /* ---- 时间行（可调节，选中态白字蓝图标） ---- */
    inf->txt_time = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_time, GUI_SCREEN_CENTER_X - 67,
                               GUI_SCREEN_CENTER_Y + 40, 0, 0);
    compo_textbox_set_autosize(inf->txt_time, true);
    compo_textbox_set_align_center(inf->txt_time, true);
    compo_textbox_set_font(inf->txt_time, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_time, COLOR_BLUE);
    compo_textbox_set(inf->txt_time, i18n[STR_TIME_LAB]);

    inf->TIME = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_BLUE_TIME_BIN);
    compo_picturebox_set_pos(inf->TIME, GUI_SCREEN_CENTER_X + 100,
                             GUI_SCREEN_CENTER_Y + 40);

    inf->txt_time_value = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_time_value, GUI_SCREEN_CENTER_X + 100,
                               GUI_SCREEN_CENTER_Y + 38, 0, 0);
    compo_textbox_set_autosize(inf->txt_time_value, true);
    compo_textbox_set_align_center(inf->txt_time_value, true);
    compo_textbox_set_font(inf->txt_time_value, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_time_value, COLOR_WHITE);
    compo_textbox_set(inf->txt_time_value, "1H");

    /* 时间进度条图片 */
    inf->time_pic = compo_picturebox_create(frm, TIME_SCHEDULE_PICS[0]);
    compo_picturebox_set_pos(inf->time_pic, GUI_SCREEN_CENTER_X,
                             GUI_SCREEN_CENTER_Y + 75);

    /* 时间刻度(固定) */
    inf->txt_time_scale1 = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_time_scale1, GUI_SCREEN_CENTER_X - 115,
                               GUI_SCREEN_CENTER_Y + 100, 0, 0);
    compo_textbox_set_autosize(inf->txt_time_scale1, true);
    compo_textbox_set_align_center(inf->txt_time_scale1, true);
    compo_textbox_set_font(inf->txt_time_scale1, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_time_scale1, COLOR_GRAY);
    compo_textbox_set(inf->txt_time_scale1, "1H");

    inf->txt_time_scale2 = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_time_scale2, GUI_SCREEN_CENTER_X,
                               GUI_SCREEN_CENTER_Y + 100, 0, 0);
    compo_textbox_set_autosize(inf->txt_time_scale2, true);
    compo_textbox_set_align_center(inf->txt_time_scale2, true);
    compo_textbox_set_font(inf->txt_time_scale2, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_time_scale2, COLOR_GRAY);
    compo_textbox_set(inf->txt_time_scale2, "1H30Min");

    inf->txt_time_scale3 = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_time_scale3, GUI_SCREEN_CENTER_X + 115,
                               GUI_SCREEN_CENTER_Y + 100, 0, 0);
    compo_textbox_set_autosize(inf->txt_time_scale3, true);
    compo_textbox_set_align_center(inf->txt_time_scale3, true);
    compo_textbox_set_font(inf->txt_time_scale3, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_time_scale3, COLOR_GRAY);
    compo_textbox_set(inf->txt_time_scale3, "2H");

    tft_bglight_force_on();
    return frm;
}

void func_heat_chicken_enter(void)
{
    f_heat_chicken_t *inf;

    func_cb.f_cb = func_zalloc(sizeof(f_heat_chicken_t));
    func_key_reset();
    func_cb.frm_main = func_heat_chicken_form_create();
    inf = (f_heat_chicken_t *)func_cb.f_cb;
    inf->display_stage = 1;
    inf->time_min = chicken_preset_time_min();   /* 预设时长, APP 可经 0x0a 修改 */
    heat_chicken_update_display();

    home_gpu_wait_idle();
    WDT_CLR();

    general_status_bar_attach(&inf->sb);
}

void func_heat_chicken_exit(void)
{
    func_key_flush();
    general_status_bar_detach();
}

void func_heat_chicken(void)
{
    printf("%s\n", __func__);
    func_heat_chicken_enter();
    while (func_cb.sta == FUNC_NEW_HEAT_CHICKEN)
    {
        func_heat_chicken_process();
        func_heat_chicken_message(msg_dequeue());
    }
    func_heat_chicken_exit();
}
