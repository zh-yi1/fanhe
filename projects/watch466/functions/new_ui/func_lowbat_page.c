#include "include.h"
#include "func.h"
#include "func_key.h"

#ifndef NEW_UI_DIDIAN_W
#define NEW_UI_DIDIAN_W  320
#define NEW_UI_DIDIAN_H  240
#endif

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

compo_form_t *func_lowbat_page_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_picturebox_t *pic;

    /* 白色背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 低电图标（居中显示） */
    pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_DIDIAN_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_size(pic, NEW_UI_DIDIAN_W, NEW_UI_DIDIAN_H);

    tft_bglight_force_on();
    return frm;
}

void func_lowbat_page_enter(void)
{
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
        WDT_CLR();
        /* 只刷新 UI，不处理按键、不加任何退出逻辑 */
        compo_update();
        if (tft_te_frame_gate()) {
            gui_process();
        }
        msg_dequeue(); /* 丢弃所有消息 */
    }
    func_lowbat_page_exit();
}
