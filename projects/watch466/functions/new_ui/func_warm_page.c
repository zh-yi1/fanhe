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

/* 圆环进度图 13 张（和加热页共用） */
static const u32 ANNULUS_PICS[13] = {
    UI_BUF_NEW_UI_ANNULUS_12_BIN,
    UI_BUF_NEW_UI_ANNULUS_11_BIN,
    UI_BUF_NEW_UI_ANNULUS_10_BIN,
    UI_BUF_NEW_UI_ANNULUS_9_BIN,
    UI_BUF_NEW_UI_ANNULUS_8_BIN,
    UI_BUF_NEW_UI_ANNULUS_7_BIN,
    UI_BUF_NEW_UI_ANNULUS_6_BIN,
    UI_BUF_NEW_UI_ANNULUS_5_BIN,
    UI_BUF_NEW_UI_ANNULUS_4_BIN,
    UI_BUF_NEW_UI_ANNULUS_3_BIN,
    UI_BUF_NEW_UI_ANNULUS_2_BIN,
    UI_BUF_NEW_UI_ANNULUS_1_BIN,
    UI_BUF_NEW_UI_ANNULUS_0_BIN,
};

/* 加热页私有状态 */
typedef struct
{
    u8 display_stage; // 0 = 正常运行, 1 = 首帧跳过 func_process
    u32 total_sec;      // 总保温时长（秒）
    u32 start_tick;     // 进入时的 tick
    u8  last_idx;       // 上次圆环索引
    general_status_bar_t sb;
    compo_picturebox_t *state_pic;
    compo_picturebox_t *schedule_pic;
    compo_textbox_t *temp_text;
    compo_textbox_t *temp_text1;
    compo_textbox_t *time_text;
    compo_textbox_t *time_text1;
    compo_textbox_t *residue_time_text;
    compo_textbox_t *residue_time_text1;
} f_warm_page_t;

compo_form_t *func_warm_page_form_create(void)
{
    f_warm_page_t *inf = (f_warm_page_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);

    /* 白色背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部状态栏 */
    general_status_bar_create(frm, &inf->sb, i18n[STR_WARM1], &g_ui_sys);

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
    compo_textbox_set(inf->residue_time_text, "0Min");

    inf->residue_time_text1 = compo_textbox_create(frm, 21);
    compo_textbox_set_location(inf->residue_time_text1, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 20, 0, 0);
    compo_textbox_set_autosize(inf->residue_time_text1, true);
    compo_textbox_set_align_center(inf->residue_time_text1, true);
    compo_textbox_set_font(inf->residue_time_text1, UI_BUF_0FONT_FONT_TEST_10_BIN);
    compo_textbox_set_multiline(inf->residue_time_text1, false);
    compo_textbox_set_forecolor(inf->residue_time_text1, COLOR_GRAY);
    compo_textbox_set(inf->residue_time_text1, i18n[STR_WARM_LAB]);

    /* 加热温度和时间的参数显示 */
    //图片
    inf->state_pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_SHOW_BIN);
    compo_picturebox_set_pos(inf->state_pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 85);

    //温度文本
    inf->temp_text = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->temp_text, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 75, 0, 0);
    compo_textbox_set_autosize(inf->temp_text, true);
    compo_textbox_set_align_center(inf->temp_text, true);
    compo_textbox_set_font(inf->temp_text, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_multiline(inf->temp_text, false);
    compo_textbox_set_forecolor(inf->temp_text, COLOR_BLUE);
    compo_textbox_set(inf->temp_text, "194F");

    inf->temp_text1 = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->temp_text1, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 95, 0, 0);
    compo_textbox_set_autosize(inf->temp_text1, true);
    compo_textbox_set_align_center(inf->temp_text1, true);
    compo_textbox_set_font(inf->temp_text1, UI_BUF_0FONT_FONT_TEST_10_BIN);
    compo_textbox_set_multiline(inf->temp_text1, false);
    compo_textbox_set_forecolor(inf->temp_text1, COLOR_GRAY);
    compo_textbox_set(inf->temp_text1, i18n[STR_HEAT_LAB]);

    tft_bglight_force_on();
    return frm;
}

static void func_warm_page_handle_keys(void)
{
    func_key_event_t evt;

    while (func_key_get_event(&evt)) {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key) {
        case FUNC_KEY_BACK:
            func_cb.sta = FUNC_HOME_PAGE;
            break;

        default:
            break;
        }
    }
}

static void func_warm_page_process(void)
{
    f_warm_page_t *inf = (f_warm_page_t *)func_cb.f_cb;

#if ELUNCHBOX_PANEL_EN
    if (inf->display_stage != 0) {
        inf->display_stage = 0;
        return;
    }
#endif

    func_key_poll();
    func_warm_page_handle_keys();
    func_key_lock_poll();

    if (func_key_lock_gui_dirty()) {
        if (func_key_lock_overlay_visible())
            func_lock_page_show(func_key_lock_overlay_is_unlock());
        else
            func_lock_page_hide();
    }

    /* 保温计时：从 0 往上数，圆环从空到满 */
    {
        u32 elapsed = (tick_get() - inf->start_tick) / 1000;
        u32 remain = (elapsed >= inf->total_sec) ? inf->total_sec : elapsed;
        u8 idx = (u8)(remain * 12 / inf->total_sec);

        if (idx > 12) idx = 12;
        if (idx != inf->last_idx) {
            inf->last_idx = idx;
            compo_picturebox_set(inf->schedule_pic, ANNULUS_PICS[idx]);

            {
                char buf[16];
                u16 m = (u16)(remain / 60);
                if (remain % 60 == 0) {
                    snprintf(buf, sizeof(buf), "%uH", m / 60);
                } else {
                    snprintf(buf, sizeof(buf), "%uH%02uMin", m / 60, m % 60);
                }
                compo_textbox_set(inf->residue_time_text, buf);
            }
        }
    }

    func_process();
    general_status_bar_tick(&inf->sb);
}

static void func_warm_page_message(size_msg_t msg)
{
    func_message(msg);
}

void func_warm_page_enter(void)
{
    f_warm_page_t *inf;

    func_cb.f_cb = func_zalloc(sizeof(f_warm_page_t));
    func_key_reset();
    func_cb.frm_main = func_warm_page_form_create();
    inf = (f_warm_page_t *)func_cb.f_cb;
    inf->display_stage = 1;
    inf->total_sec  = (u32)g_ui_sys.keep_warm_min * 60;
    inf->start_tick = tick_get();
    inf->last_idx   = 0;

    home_gpu_wait_idle();
    WDT_CLR();

    general_status_bar_attach(&inf->sb);
}

void func_warm_page_exit(void)
{
    func_key_flush();
    general_status_bar_detach();
}

void func_warm_page(void)
{
    printf("%s\n", __func__);
    func_warm_page_enter();
    while (func_cb.sta == FUNC_NEW_WARM_PAGE)
    {
        func_warm_page_process();
        func_warm_page_message(msg_dequeue());
    }
    func_warm_page_exit();
}
