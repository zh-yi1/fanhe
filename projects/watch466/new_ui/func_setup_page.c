#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"
#include "general_title_ui.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/* 设置页私有状态 */
typedef struct
{
    u8 selection; // 0~2 对应 3 张设置图片
    general_title_bar_t tb;
    compo_picturebox_t *pic_setup;
    compo_textbox_t *txt_label[3];
} f_setup_t;

static const u32 g_setup_change_bins[] = {
    UI_BUF_NEW_UI_NEW_SETUP_CHANGE_1_BIN,
    UI_BUF_NEW_UI_NEW_SETUP_CHANGE_2_BIN,
    UI_BUF_NEW_UI_NEW_SETUP_CHANGE_3_BIN,
};
#define SETUP_CHANGE_CNT  (sizeof(g_setup_change_bins) / sizeof(g_setup_change_bins[0]))

static void setup_update_display(void)
{
    f_setup_t *inf = (f_setup_t *)func_cb.f_cb;
    u8 i;

    compo_picturebox_set(inf->pic_setup, g_setup_change_bins[inf->selection]);
    for (i = 0; i < 3; i++)
    {
        compo_textbox_set_forecolor(inf->txt_label[i], (i == inf->selection) ? COLOR_BLUE : COLOR_BLACK);
    }
}

compo_form_t *func_setup_page_form_create(void)
{
    f_setup_t *inf = (f_setup_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);

    /* 灰色背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, 0xEF5D);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部标题栏（左上角标题 + 右上角蓝牙/电量） */
    general_title_bar_create(frm, &inf->tb, i18n[STR_SETUP]);

    /* 设置主图（初始第 0 张） */
    inf->selection = 0;
    inf->pic_setup = compo_picturebox_create(frm, g_setup_change_bins[0]);
    compo_picturebox_set_pos(inf->pic_setup, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 20);

        /* 设置文字标签（从上到下：Time / Language / Ver. Info.） */
        {
            static const s16 label_x[] = { 110, 130, 125 };
            static const s16 label_y[] = { 80, 120, 160 };
            u8 i;

            for (i = 0; i < 3; i++)
            {
                inf->txt_label[i] = compo_textbox_create(frm, 16);
                compo_textbox_set_location(inf->txt_label[i], label_x[i], label_y[i], 0, 0);
                compo_textbox_set_autosize(inf->txt_label[i], true);
                compo_textbox_set_font(inf->txt_label[i], UI_BUF_0FONT_FONT_TEST_14_BIN);
                compo_textbox_set(inf->txt_label[i], i18n[
                    (i == 0) ? STR_SETUP_TIME :
                    (i == 1) ? STR_LANGUAGE : STR_VER_INFO
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
static void func_setup_page_handle_keys(void)
{
    f_setup_t *inf = (f_setup_t *)func_cb.f_cb;
    func_key_event_t evt;

    while (func_key_get_event(&evt))
    {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key)
        {
        case FUNC_KEY_UP:
            inf->selection = (inf->selection == 0) ? (SETUP_CHANGE_CNT - 1) : (inf->selection - 1);
            setup_update_display();
            break;

        case FUNC_KEY_DOWN:
            inf->selection = (inf->selection + 1) % SETUP_CHANGE_CNT;
            setup_update_display();
            break;

        case FUNC_KEY_CONFIRM:
            if (inf->selection == 0)
                func_cb.sta = FUNC_NEW_TIME;
            else if (inf->selection == 1)
                func_cb.sta = FUNC_NEW_LANGUAGE;
            else
                func_cb.sta = FUNC_NEW_VERINFO;
            break;

        case FUNC_KEY_BACK:
            func_cb.sta = FUNC_HOME;
            break;

        default:
            break;
        }
    }
}

static void func_setup_page_process(void)
{
    /* 1. 按键处理 */
    func_key_poll();
    func_setup_page_handle_keys();

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
}

static void func_setup_page_message(size_msg_t msg)
{
    func_message(msg);
}

void func_setup_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_setup_t));
    func_key_reset();
    func_cb.frm_main = func_setup_page_form_create();
    general_title_bar_attach(&((f_setup_t *)func_cb.f_cb)->tb);
}

void func_setup_page_exit(void)
{
    func_key_flush();
    general_title_bar_detach();
}

void func_setup_page(void)
{
    printf("%s\n", __func__);
    func_setup_page_enter();
    while (func_cb.sta == FUNC_NEW_SETUP)
    {
        func_setup_page_process();
        func_setup_page_message(msg_dequeue());
    }
    func_setup_page_exit();
}
