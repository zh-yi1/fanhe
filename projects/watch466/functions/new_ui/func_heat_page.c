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

/* 圆环进度图 13 张，每 5 分钟切换 */
static const u32 ANNULUS_PICS[13] = {
    UI_BUF_NEW_UI_ANNULUS_0_BIN,
    UI_BUF_NEW_UI_ANNULUS_1_BIN,
    UI_BUF_NEW_UI_ANNULUS_2_BIN,
    UI_BUF_NEW_UI_ANNULUS_3_BIN,
    UI_BUF_NEW_UI_ANNULUS_4_BIN,
    UI_BUF_NEW_UI_ANNULUS_5_BIN,
    UI_BUF_NEW_UI_ANNULUS_6_BIN,
    UI_BUF_NEW_UI_ANNULUS_7_BIN,
    UI_BUF_NEW_UI_ANNULUS_8_BIN,
    UI_BUF_NEW_UI_ANNULUS_9_BIN,
    UI_BUF_NEW_UI_ANNULUS_10_BIN,
    UI_BUF_NEW_UI_ANNULUS_11_BIN,
    UI_BUF_NEW_UI_ANNULUS_12_BIN,
};

/* 加热页私有状态 */
typedef struct
{
    u8 display_stage; // 0 = 正常运行, 1 = 首帧跳过 func_process
    u32 total_sec;      // 总秒数
    u32 start_tick;     // 进入时的 tick
    u8  last_idx;       // 上次圆环索引，避免重复刷新
    general_status_bar_t sb;
    compo_picturebox_t *state_pic;
    compo_picturebox_t *schedule_pic;
    compo_textbox_t *temp_text;
    compo_textbox_t *temp_text1;
    compo_textbox_t *time_text;
    compo_textbox_t *time_text1;
    compo_textbox_t *residue_time_text;
    compo_textbox_t *residue_time_text1;
} f_heat_page_t;

compo_form_t *func_heat_page_form_create(void)
{
    f_heat_page_t *inf = (f_heat_page_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);

    /* 白色背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部状态栏 */
    general_status_bar_create(frm, &inf->sb, NULL, &g_ui_sys);

    /* ---- 加热圆环进度条 ---- */
    inf->schedule_pic = compo_picturebox_create(frm, ANNULUS_PICS[0]);
    compo_picturebox_set_pos(inf->schedule_pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 15);

    /* ---- 加热圆环进度条内的文本 ---- */
    inf->residue_time_text = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->residue_time_text, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 10, 0, 0);
    compo_textbox_set_autosize(inf->residue_time_text, true);
    compo_textbox_set_align_center(inf->residue_time_text, true);
    compo_textbox_set_font(inf->residue_time_text, UI_BUF_0FONT_FONT_TEST_BIN);
    compo_textbox_set_multiline(inf->residue_time_text, false);
    compo_textbox_set_forecolor(inf->residue_time_text, COLOR_BLUE);
    {
        char buf[8];
        if (g_ui_sys.time_min % 60 == 0) {
            snprintf(buf, sizeof(buf), "%uH", g_ui_sys.time_min / 60);
        } else {
            snprintf(buf, sizeof(buf), "%uH%02uMin",
                     g_ui_sys.time_min / 60, g_ui_sys.time_min % 60);
        }
        compo_textbox_set(inf->residue_time_text, buf);
    }

    inf->residue_time_text1 = compo_textbox_create(frm, 21);
    compo_textbox_set_location(inf->residue_time_text1, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 20, 0, 0);
    compo_textbox_set_autosize(inf->residue_time_text1, true);
    compo_textbox_set_align_center(inf->residue_time_text1, true);
    compo_textbox_set_font(inf->residue_time_text1, UI_BUF_0FONT_FONT_TEST_10_BIN);
    compo_textbox_set_multiline(inf->residue_time_text1, false);
    compo_textbox_set_forecolor(inf->residue_time_text1, COLOR_GRAY);
    compo_textbox_set(inf->residue_time_text1, i18n[STR_RESIDUE_LAB]);

    /* 加热温度和时间的参数显示 */
    //图片
    inf->state_pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_SHOW_BIN);
    compo_picturebox_set_pos(inf->state_pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 85);

    //温度文本
    inf->temp_text = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->temp_text, GUI_SCREEN_CENTER_X - 80, GUI_SCREEN_CENTER_Y + 75, 0, 0);
    compo_textbox_set_autosize(inf->temp_text, true);
    compo_textbox_set_align_center(inf->temp_text, true);
    compo_textbox_set_font(inf->temp_text, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_multiline(inf->temp_text, false);
    compo_textbox_set_forecolor(inf->temp_text, COLOR_BLUE);
    {
        char buf[8];
        snprintf(buf, sizeof(buf), "%uF", g_ui_sys.temp);
        compo_textbox_set(inf->temp_text, buf);
    }

    inf->temp_text1 = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->temp_text1, GUI_SCREEN_CENTER_X - 80, GUI_SCREEN_CENTER_Y + 95, 0, 0);
    compo_textbox_set_autosize(inf->temp_text1, true);
    compo_textbox_set_align_center(inf->temp_text1, true);
    compo_textbox_set_font(inf->temp_text1, UI_BUF_0FONT_FONT_TEST_10_BIN);
    compo_textbox_set_multiline(inf->temp_text1, false);
    compo_textbox_set_forecolor(inf->temp_text1, COLOR_GRAY);
    compo_textbox_set(inf->temp_text1, i18n[STR_HEAT_LAB]);

    //时间文本
    inf->time_text = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->time_text, GUI_SCREEN_CENTER_X + 80, GUI_SCREEN_CENTER_Y + 75, 0, 0);
    compo_textbox_set_autosize(inf->time_text, true);
    compo_textbox_set_align_center(inf->time_text, true);
    compo_textbox_set_font(inf->time_text, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_multiline(inf->time_text, false);
    compo_textbox_set_forecolor(inf->time_text, COLOR_BLUE);
    {
        char buf[8];
        if (g_ui_sys.time_min % 60 == 0) {
            snprintf(buf, sizeof(buf), "%uH", g_ui_sys.time_min / 60);
        } else {
            snprintf(buf, sizeof(buf), "%uH%02uMin",
                     g_ui_sys.time_min / 60, g_ui_sys.time_min % 60);
        }
        compo_textbox_set(inf->time_text, buf);
    }

    inf->time_text1 = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->time_text1, GUI_SCREEN_CENTER_X + 70, GUI_SCREEN_CENTER_Y + 95, 0, 0);
    compo_textbox_set_autosize(inf->time_text1, true);
    compo_textbox_set_align_center(inf->time_text1, true);
    compo_textbox_set_font(inf->time_text1, UI_BUF_0FONT_FONT_TEST_10_BIN);
    compo_textbox_set_multiline(inf->time_text1, false);
    compo_textbox_set_forecolor(inf->time_text1, COLOR_GRAY);
    compo_textbox_set(inf->time_text1, i18n[STR_TIME_LAB]);

    tft_bglight_force_on();
    return frm;
}

static void func_heat_page_handle_keys(void)
{
    func_key_event_t evt;

    while (func_key_get_event(&evt)) {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key) {
        case FUNC_KEY_BACK:
            func_cb.sta = FUNC_HOME_PAGE;
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

static void func_heat_page_process(void)
{
    f_heat_page_t *inf = (f_heat_page_t *)func_cb.f_cb;

#if ELUNCHBOX_PANEL_EN
    if (inf->display_stage != 0) {
        inf->display_stage = 0;
        return;
    }
#endif

    func_key_poll();
    func_heat_page_handle_keys();
    func_key_lock_poll();

    if (func_key_lock_gui_dirty()) {
        if (func_key_lock_overlay_visible())
            func_lock_page_show(func_key_lock_overlay_is_unlock());
        else
            func_lock_page_hide();
    }

    /* 倒计时：根据启动时刻计算剩余秒数，只在圆环索引变化时刷新 */
    {
        u32 elapsed = (tick_get() - inf->start_tick) / 1000;
        u32 remain;

        if (elapsed >= inf->total_sec) {
            remain = 0;
        } else {
            remain = inf->total_sec - elapsed;
        }

        {
            u8 idx = (u8)(12 - remain * 12 / inf->total_sec);

            if (idx != inf->last_idx) {
                inf->last_idx = idx;
                compo_picturebox_set(inf->schedule_pic, ANNULUS_PICS[idx]);

                /* 更新圆环内的时间文本 */
                {
                    char buf[16];
                    u16 rm = (u16)(remain / 60);
                    if (remain % 60 == 0) {
                        snprintf(buf, sizeof(buf), "%uH", rm / 60);
                    } else {
                        snprintf(buf, sizeof(buf), "%uH%02uMin",
                                 rm / 60, rm % 60);
                    }
                    compo_textbox_set(inf->residue_time_text, buf);
                }
            }
        }
    }

    func_process();
    general_status_bar_tick(&inf->sb);
}

static void func_heat_page_message(size_msg_t msg)
{
    func_message(msg);
}

void func_heat_page_enter(void)
{
    f_heat_page_t *inf;

    func_cb.f_cb = func_zalloc(sizeof(f_heat_page_t));
    func_key_reset();
    func_cb.frm_main = func_heat_page_form_create();
    inf = (f_heat_page_t *)func_cb.f_cb;
    inf->display_stage = 1;
    inf->total_sec  = (u32)g_ui_sys.time_min * 60;
    inf->start_tick = tick_get();
    inf->last_idx   = 0;

    /* 初始圆环图：满环 */
    compo_picturebox_set(inf->schedule_pic, ANNULUS_PICS[0]);

    home_gpu_wait_idle();
    WDT_CLR();

    general_status_bar_attach(&inf->sb);
}

void func_heat_page_exit(void)
{
    func_key_flush();
    general_status_bar_detach();
}

void func_heat_page(void)
{
    printf("%s\n", __func__);
    func_heat_page_enter();
    while (func_cb.sta == FUNC_NEW_HEAT_PAGE)
    {
        func_heat_page_process();
        func_heat_page_message(msg_dequeue());
    }
    func_heat_page_exit();
}
