#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"

#ifndef NEW_UI_DIDIAN_W
#define NEW_UI_DIDIAN_W  48
#define NEW_UI_DIDIAN_H  48
#endif

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/* 低电页私有状态 */
typedef struct
{
    compo_picturebox_t *pic_icon;
    bool icon_shown;
} f_lowbat_page_t;

compo_form_t *func_lowbat_page_form_create(void)
{
    f_lowbat_page_t *inf = (f_lowbat_page_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);

    /* 白色背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 低电图标（居中显示） */
    inf->pic_icon = compo_picturebox_create(frm, UI_BUF_NEW_UI_DIDIAN_BIN);
    compo_picturebox_set_pos(inf->pic_icon, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_size(inf->pic_icon, NEW_UI_DIDIAN_W, NEW_UI_DIDIAN_H);

    inf->icon_shown = true;

    tft_bglight_force_on();
    return frm;
}

/*
 * 按键处理
 */
static void func_lowbat_page_handle_keys(void)
{
    func_key_event_t evt;

    while (func_key_get_event(&evt))
    {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key)
        {
        case FUNC_KEY_CONFIRM:
        case FUNC_KEY_BACK:
            func_cb.sta = FUNC_HOME;
            break;

        default:
            break;
        }
    }
}

static void func_lowbat_page_process(void)
{
    /* 1. 按键处理 */
    func_key_poll();
    func_lowbat_page_handle_keys();

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

    /* 4. 刷新 UI（不调 func_process，避免 elunchbox_lowbat_poll 循环退出） */
    if (func_cb.frm_main != NULL) {
        compo_update();
#if ELUNCHBOX_PANEL_EN && LE_EN
        home_ui_shared_ble_status_poll();
#endif
#if ELUNCHBOX_PANEL_EN
        home_ui_shared_battery_chg_poll();
#endif
        gui_process();
    }
}

static void func_lowbat_page_message(size_msg_t msg)
{
    func_message(msg);
}

void func_lowbat_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_lowbat_page_t));
    func_key_reset();
    func_cb.frm_main = func_lowbat_page_form_create();
}

void func_lowbat_page_exit(void)
{
    func_key_flush();
}

void func_lowbat_page(void)
{
    printf("%s\n", __func__);
    func_lowbat_page_enter();
    while (func_cb.sta == FUNC_LOWBAT)
    {
        func_lowbat_page_process();
        func_lowbat_page_message(msg_dequeue());
    }
    func_lowbat_page_exit();
}
