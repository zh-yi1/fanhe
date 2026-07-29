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

/* 模式页私有状态 */
typedef struct
{
    u8 selection; // 0~3 对应 4 张模式图片
    general_status_bar_t sb;
    compo_picturebox_t *pic_mode;
    compo_textbox_t *txt_label[4];
} f_mode_t;

static const u32 g_mode_change_bins[] = {
    UI_BUF_NEW_UI_NEW_MODE_CHANGE_1_BIN,
    UI_BUF_NEW_UI_NEW_MODE_CHANGE_2_BIN,
    UI_BUF_NEW_UI_NEW_MODE_CHANGE_3_BIN,
    UI_BUF_NEW_UI_NEW_MODE_CHANGE_4_BIN,
};
#define MODE_CHANGE_CNT  (sizeof(g_mode_change_bins) / sizeof(g_mode_change_bins[0]))

static void mode_update_display(void)
{
    f_mode_t *inf = (f_mode_t *)func_cb.f_cb;
    u8 i;

    compo_picturebox_set(inf->pic_mode, g_mode_change_bins[inf->selection]);
    for (i = 0; i < 4; i++)
    {
        compo_textbox_set_forecolor(inf->txt_label[i], (i == inf->selection) ? COLOR_BLUE : COLOR_BLACK);
    }
}

compo_form_t *func_mode_page_form_create(void)
{
    f_mode_t *inf = (f_mode_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);

    /* 灰色背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, 0xEF5D);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部状态栏（左上角时间） */
    general_status_bar_create(frm, &inf->sb, i18n[STR_MODE], &g_ui_sys);

    /* 模式主图（初始第 0 张） */
    inf->selection = 0;
    inf->pic_mode = compo_picturebox_create(frm, g_mode_change_bins[0]);
    compo_picturebox_set_pos(inf->pic_mode, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 20);


        /* 模式文字标签（从上到下：Order / Chicken / Pasta / Warm） */
        {
            static const s16 label_x[] = { 110, 117, 110, 110 };
            static const s16 label_y[] = { 80, 120, 160, 200 };
            u8 i;
    
            for (i = 0; i < 4; i++)
            {
                inf->txt_label[i] = compo_textbox_create(frm, 16);
                compo_textbox_set_location(inf->txt_label[i], label_x[i], label_y[i], 0, 0);
                compo_textbox_set_autosize(inf->txt_label[i], true);
                compo_textbox_set_font(inf->txt_label[i], UI_BUF_0FONT_FONT_TEST_14_BIN);
                compo_textbox_set(inf->txt_label[i], i18n[
    (i == 0) ? STR_ORDER :
    (i == 1) ? STR_CHICKEN :
    (i == 2) ? STR_PASTA : STR_WARM
                ]);
                compo_textbox_set_forecolor(inf->txt_label[i], (i == 0) ? COLOR_BLUE : COLOR_BLACK);
            }
        }
    
    tft_bglight_force_on();
    return frm;
}

/*
 * 按键处理 — 逻辑键由 func_key_map_logical 映射：
 *   TCH4 → CONFIRM, TCH5 → BACK
 */
static void func_mode_page_handle_keys(void)
{
    f_mode_t *inf = (f_mode_t *)func_cb.f_cb;
    func_key_event_t evt;

    while (func_key_get_event(&evt))
    {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key)
        {
        case FUNC_KEY_UP:
            inf->selection = (inf->selection == 0) ? (MODE_CHANGE_CNT - 1) : (inf->selection - 1);
            mode_update_display();
            break;

        case FUNC_KEY_DOWN:
            inf->selection = (inf->selection + 1) % MODE_CHANGE_CNT;
            mode_update_display();
            break;

        case FUNC_KEY_CONFIRM:
            if (inf->selection == 0)
                func_cb.sta = FUNC_APPOINTMENT_TIME;
            else if (inf->selection == 1)
                func_cb.sta = FUNC_NEW_HEAT_CHICKEN;
            else if (inf->selection == 2)
                func_cb.sta = FUNC_NEW_HEAT_PASTA;
            else
                func_cb.sta = FUNC_NEW_WARM_PAGE;
            break;

        case FUNC_KEY_BACK:
            func_cb.sta = FUNC_HOME;
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

static void func_mode_page_process(void)
{
    f_mode_t *inf = (f_mode_t *)func_cb.f_cb;

    /* 1. 按键处理 */
    func_key_poll();
    func_mode_page_handle_keys();

    /* 2. 童锁 */
    func_key_lock_poll();

    /* 3. 锁标志位 → UI 渲染 */
    if (func_key_lock_gui_dirty())
    {
        if (func_key_lock_overlay_visible())
        {
            func_lock_page_show(func_key_lock_overlay_is_unlock());
        }
        else
        {
            func_lock_page_hide();
        }
    }

    func_process();

    /* 4. 状态栏时间刷新 */
    general_status_bar_tick(&inf->sb);
}

static void func_mode_page_message(size_msg_t msg)
{
    func_message(msg);
}

void func_mode_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_mode_t));
    func_key_reset();
    func_cb.frm_main = func_mode_page_form_create();
    general_status_bar_attach(&((f_mode_t *)func_cb.f_cb)->sb);
}

void func_mode_page_exit(void)
{
    func_key_flush();
    general_status_bar_detach();
}

void func_mode_page(void)
{
    printf("%s\n", __func__);
    func_mode_page_enter();
    while (func_cb.sta == FUNC_NEW_MODE)
    {
        func_mode_page_process();
        func_mode_page_message(msg_dequeue());
    }
    func_mode_page_exit();
}
