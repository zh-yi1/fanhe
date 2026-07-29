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

/* 温度挡位 */
static const u16 TEMP_VALUES[] = { 140, 158, 176, 194, 212 };
#define TEMP_CNT  5
#define TEMP_DEFAULT  0

/* 温度进度条图片（对应 5 个挡位） */
static const u32 TMP_SCHEDULE_PICS[TEMP_CNT] = {
    UI_BUF_NEW_UI_TMP_SCHEDULE_0_BIN,
    UI_BUF_NEW_UI_TMP_SCHEDULE_1_BIN,
    UI_BUF_NEW_UI_TMP_SCHEDULE_2_BIN,
    UI_BUF_NEW_UI_TMP_SCHEDULE_3_BIN,
    UI_BUF_NEW_UI_TMP_SCHEDULE_4_BIN,
};

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

/* 时间 5 分钟步进：1H00Min ~ 2H00Min */
#define TIME_MIN_MINUTES  60
#define TIME_MAX_MINUTES  120
#define TIME_STEP         5
#define TIME_CNT          (((TIME_MAX_MINUTES - TIME_MIN_MINUTES) / TIME_STEP) + 1)
#define TIME_DEFAULT      TIME_MIN_MINUTES

/* 模式 */
#define MODE_TEMP  0
#define MODE_TIME  1

/* 加热设置页私有状态 */
typedef struct
{
    u8 display_stage;   // 0 = 正常运行, 1 = 首帧跳过 func_process
    u8 mode;            // MODE_TEMP 或 MODE_TIME
    u8 temp_index;      // 0..TEMP_CNT-1
    u8 time_min;        // TIME_MIN_MINUTES..TIME_MAX_MINUTES, step 5
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

} f_heat_set_t;

/* ---- 显示刷新 ---- */
static void heat_set_update_display(void)
{
    f_heat_set_t *inf = (f_heat_set_t *)func_cb.f_cb;
    char buf[8];
    bool sel_temp = (inf->mode == MODE_TEMP);
    bool sel_time = (inf->mode == MODE_TIME);

    /* 温度行：label 不变，value + icon + 进度条随选中态切换 */
    snprintf(buf, sizeof(buf), "%uF", TEMP_VALUES[inf->temp_index]);
    compo_textbox_set(inf->txt_heat_value, buf);
    compo_textbox_set_forecolor(inf->txt_heat_value, sel_temp ? COLOR_WHITE : COLOR_BLUE);
    compo_picturebox_set(inf->TMP,
                         sel_temp ? UI_BUF_NEW_UI_NEW_BLUE_TIME_BIN
                                  : UI_BUF_NEW_UI_NEW_WHITE_TIME_BIN);
    compo_picturebox_set(inf->tmp_pic, TMP_SCHEDULE_PICS[inf->temp_index]);

    /* 时间行：label 不变，value + icon 随选中态切换 */
    if (inf->time_min % 60 == 0) {
        snprintf(buf, sizeof(buf), "%uH", inf->time_min / 60);
    } else {
        snprintf(buf, sizeof(buf), "%uH%02uMin",
                 inf->time_min / 60, inf->time_min % 60);
    }
    compo_textbox_set(inf->txt_time_value, buf);
    compo_textbox_set_forecolor(inf->txt_time_value, sel_time ? COLOR_WHITE : COLOR_BLUE);
    compo_picturebox_set(inf->TIME,
                         sel_time ? UI_BUF_NEW_UI_NEW_BLUE_TIME_BIN
                                  : UI_BUF_NEW_UI_NEW_WHITE_TIME_BIN);
    compo_picturebox_set(inf->time_pic,
                         TIME_SCHEDULE_PICS[(inf->time_min - TIME_MIN_MINUTES) / TIME_STEP]);
}

/* ---- 按键处理 ---- */
static void func_heat_set_page_handle_keys(void)
{
    f_heat_set_t *inf = (f_heat_set_t *)func_cb.f_cb;
    func_key_event_t evt;

    while (func_key_get_event(&evt))
    {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key)
        {
        case FUNC_KEY_UP:
        case FUNC_KEY_DOWN:
            if (inf->mode == MODE_TEMP) {
                /* 温度挡位循环 */
                if (key == FUNC_KEY_UP) {
                    inf->temp_index++;
                    if (inf->temp_index >= TEMP_CNT) inf->temp_index = 0;
                } else {
                    if (inf->temp_index == 0) inf->temp_index = TEMP_CNT - 1;
                    else inf->temp_index--;
                }
            } else {
                /* 时间 5 分钟步进 */
                if (key == FUNC_KEY_UP) {
                    inf->time_min += TIME_STEP;
                    if (inf->time_min > TIME_MAX_MINUTES)
                        inf->time_min = TIME_MIN_MINUTES;
                } else {
                    if (inf->time_min <= TIME_MIN_MINUTES)
                        inf->time_min = TIME_MAX_MINUTES;
                    else
                        inf->time_min -= TIME_STEP;
                }
            }
            heat_set_update_display();
            break;

        case FUNC_KEY_CONFIRM:
            if (inf->mode == MODE_TEMP) {
                /* 温度选好 → 切到时间选择 */
                inf->mode = MODE_TIME;
                heat_set_update_display();
            } else {
                /* 时间选好 → 跳到加热页 */
                func_cb.sta = FUNC_NEW_HEAT_PAGE;
            }
            break;

        case FUNC_KEY_BACK:
            if (inf->mode == MODE_TIME) {
                /* 时间模式 → 退回温度选择 */
                inf->mode = MODE_TEMP;
                heat_set_update_display();
            } else {
                /* 温度模式 → 返回主页 */
                func_cb.sta = FUNC_HOME_PAGE;
            }
            break;

        default:
            break;
        }
    }
}

/* ---- 每帧处理 ---- */
static void func_heat_set_page_process(void)
{
    f_heat_set_t *inf = (f_heat_set_t *)func_cb.f_cb;

#if ELUNCHBOX_PANEL_EN
    if (inf->display_stage != 0)
    {
        inf->display_stage = 0;
        return;
    }
#endif

    func_key_poll();
    func_heat_set_page_handle_keys();
    func_key_lock_poll();

    if (func_key_lock_gui_dirty())
    {
        if (func_key_lock_overlay_visible())
            func_lock_page_show(func_key_lock_overlay_is_unlock());
        else
            func_lock_page_hide();
    }

    func_process();
    general_status_bar_tick(&inf->sb);
}

static void func_heat_set_page_message(size_msg_t msg)
{
    func_message(msg);
}

/* ---- 生命周期 ---- */
compo_form_t *func_heat_set_page_form_create(void)
{
    f_heat_set_t *inf = (f_heat_set_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);

    /* 白色背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部状态栏 */
    general_status_bar_create(frm, &inf->sb, NULL, &g_ui_sys);

    /* ---- 温度行（默认选中） ---- */
    inf->txt_heat = compo_textbox_create(frm, 12);
    compo_textbox_set_location(inf->txt_heat, GUI_SCREEN_CENTER_X - 80,
                               GUI_SCREEN_CENTER_Y - 50, 0, 0);
    compo_textbox_set_autosize(inf->txt_heat, true);
    compo_textbox_set_align_center(inf->txt_heat, true);
    compo_textbox_set_font(inf->txt_heat, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_heat, COLOR_BLUE);
    compo_textbox_set(inf->txt_heat, i18n[STR_HEAT_LAB]);

    inf->TMP = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_BLUE_TIME_BIN);
    compo_picturebox_set_pos(inf->TMP, GUI_SCREEN_CENTER_X + 100,
                             GUI_SCREEN_CENTER_Y - 50);

    inf->txt_heat_value = compo_textbox_create(frm, 12);
    compo_textbox_set_location(inf->txt_heat_value, GUI_SCREEN_CENTER_X + 100,
                               GUI_SCREEN_CENTER_Y - 51, 0, 0);
    compo_textbox_set_autosize(inf->txt_heat_value, true);
    compo_textbox_set_align_center(inf->txt_heat_value, true);
    compo_textbox_set_font(inf->txt_heat_value, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_heat_value, COLOR_WHITE); // 选中白字
    compo_textbox_set(inf->txt_heat_value, "140F");

    //温度图片进度条
    inf->tmp_pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_TMP_SCHEDULE_0_BIN);
    compo_picturebox_set_pos(inf->tmp_pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 15);

    //温度刻度(固定不变的)
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

    /* ---- 时间行（默认未选中） ---- */
    inf->txt_time = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_time, GUI_SCREEN_CENTER_X - 67,
                               GUI_SCREEN_CENTER_Y + 40, 0, 0);
    compo_textbox_set_autosize(inf->txt_time, true);
    compo_textbox_set_align_center(inf->txt_time, true);
    compo_textbox_set_font(inf->txt_time, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_time, COLOR_BLUE);       // 未选中蓝字
    compo_textbox_set(inf->txt_time, i18n[STR_TIME_LAB]);

    inf->TIME = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_WHITE_TIME_BIN);
    compo_picturebox_set_pos(inf->TIME, GUI_SCREEN_CENTER_X + 100,
                             GUI_SCREEN_CENTER_Y + 40);

    inf->txt_time_value = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_time_value, GUI_SCREEN_CENTER_X + 100,
                               GUI_SCREEN_CENTER_Y + 38, 0, 0);
    compo_textbox_set_autosize(inf->txt_time_value, true);
    compo_textbox_set_align_center(inf->txt_time_value, true);
    compo_textbox_set_font(inf->txt_time_value, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_time_value, COLOR_BLUE);  // 未选中蓝字
    compo_textbox_set(inf->txt_time_value, "1H");

    //时间图片进度条
    inf->time_pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_TIME_SCHEDULE_0_BIN);
    compo_picturebox_set_pos(inf->time_pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 75);

    //时间刻度(固定不变的)
    inf->txt_time_scale1 = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_time_scale1, GUI_SCREEN_CENTER_X - 115,
                               GUI_SCREEN_CENTER_Y + 100, 0, 0);
    compo_textbox_set_autosize(inf->txt_time_scale1, true);
    compo_textbox_set_align_center(inf->txt_time_scale1, true);
    compo_textbox_set_font(inf->txt_time_scale1, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_time_scale1, COLOR_GRAY); // 未选中蓝字
    compo_textbox_set(inf->txt_time_scale1, "1H");

    inf->txt_time_scale2 = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_time_scale2, GUI_SCREEN_CENTER_X,
                               GUI_SCREEN_CENTER_Y + 100, 0, 0);
    compo_textbox_set_autosize(inf->txt_time_scale2, true);
    compo_textbox_set_align_center(inf->txt_time_scale2, true);
    compo_textbox_set_font(inf->txt_time_scale2, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_time_scale2, COLOR_GRAY); // 未选中蓝字
    compo_textbox_set(inf->txt_time_scale2, "1H30Min");

    inf->txt_time_scale3 = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_time_scale3, GUI_SCREEN_CENTER_X + 115,
                               GUI_SCREEN_CENTER_Y + 100, 0, 0);
    compo_textbox_set_autosize(inf->txt_time_scale3, true);
    compo_textbox_set_align_center(inf->txt_time_scale3, true);
    compo_textbox_set_font(inf->txt_time_scale3, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(inf->txt_time_scale3, COLOR_GRAY); // 未选中蓝字
    compo_textbox_set(inf->txt_time_scale3, "2H");

    tft_bglight_force_on();
    return frm;
}

void func_heat_set_page_enter(void)
{
    f_heat_set_t *inf;

    func_cb.f_cb = func_zalloc(sizeof(f_heat_set_t));
    func_key_reset();
    func_cb.frm_main = func_heat_set_page_form_create();
    inf = (f_heat_set_t *)func_cb.f_cb;
    inf->display_stage = 1;
    inf->mode = MODE_TEMP;
    inf->temp_index = TEMP_DEFAULT;
    inf->time_min = TIME_DEFAULT;

    home_gpu_wait_idle();
    WDT_CLR();

    general_status_bar_attach(&inf->sb);
}

void func_heat_set_page_exit(void)
{
    printf("%s\n", __func__);
    func_key_flush();
    general_status_bar_detach();
}

void func_heat_set_page(void)
{
    printf("%s\n", __func__);
    func_heat_set_page_enter();
    while (func_cb.sta == FUNC_NEW_HEAT_SET)
    {
        func_heat_set_page_process();
        func_heat_set_page_message(msg_dequeue());
    }
    func_heat_set_page_exit();
}
