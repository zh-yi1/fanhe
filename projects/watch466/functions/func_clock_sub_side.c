#if 0
#include "include.h"
#include "func.h"
#include "func_clock.h"

//创建边菜单
static void func_clock_sub_side_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_form_set_location(frm, 0, 0, GUI_SIDE_MENU_WIDTH, GUI_SCREEN_HEIGHT);

    //创建遮罩层
    compo_shape_t *masklayer = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(masklayer, COLOR_BLACK);
    compo_shape_set_location(masklayer, GUI_SCREEN_CENTER_X / 2, GUI_SCREEN_CENTER_Y, GUI_SIDE_MENU_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_alpha(masklayer, 140);

    //创建按钮
    compo_button_t *btn_test = compo_button_create_by_image(frm, UI_BUF_ICON_GAME_BIN);
    compo_button_set_pos(btn_test, 60, 150);
    compo_button_set_alpha(btn_test, 200);

    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    f_clk->sub_frm = frm;
    f_clk->masklayer = masklayer;
}

//时钟表盘主要事件流程处理
static void func_clock_sub_side_process(void)
{
    func_clock_sub_process();
}

//时钟表盘边菜单功能消息处理
static void func_clock_sub_side_message(size_msg_t msg)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_SHORT_LEFT:
        if (func_switching(FUNC_SWITCH_MENU_SIDE_BACK, NULL)) {
            f_clk->sta = FUNC_CLOCK_MAIN;                   //左滑返回到时钟主界面
        }
        break;

    case MSG_CTP_CLICK:
        if (ctp_get_sxy().x > GUI_SIDE_MENU_WIDTH) {
            func_switching(FUNC_SWITCH_MENU_SIDE_BACK | FUNC_SWITCH_AUTO, NULL);
            f_clk->sta = FUNC_CLOCK_MAIN;                   //单击边栏右边空白返回到时钟主界面
        }
        break;

    case KU_BACK:
        func_switching(FUNC_SWITCH_MENU_SIDE_BACK | FUNC_SWITCH_AUTO, NULL);
        f_clk->sta = FUNC_CLOCK_MAIN;                       //单击BACK键返回到时钟主界面
        break;

    default:
        func_clock_sub_message(msg);
        break;
    }
}

//时钟表盘边菜单进入处理
static void func_clock_sub_side_enter(void)
{
    func_clock_sub_side_form_create();
    if (!func_switching(FUNC_SWITCH_MENU_SIDE_POP, NULL)) {
        return;                                             //下拉到一半取消
    }
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    f_clk->sta = FUNC_CLOCK_SUB_SIDE;                       //进入到边菜单
}

//时钟表盘边菜单退出处理
static void func_clock_sub_side_exit(void)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    compo_form_destroy(f_clk->sub_frm);
    f_clk->sub_frm = NULL;
}

//时钟表盘边菜单
void func_clock_sub_side(void)
{
    func_clock_sub_side_enter();
    while (func_cb.sta == FUNC_CLOCK && ((f_clock_t *)func_cb.f_cb)->sta == FUNC_CLOCK_SUB_SIDE) {
        func_clock_sub_side_process();
        func_clock_sub_side_message(msg_dequeue());
    }
    func_clock_sub_side_exit();
}
#endif
