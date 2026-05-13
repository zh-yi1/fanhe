#if 0
#include "include.h"
#include "func.h"
#include "func_clock.h"

//创建上拉菜单
static void func_clock_sub_pullup_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);

    //创建遮罩层
    compo_shape_t *masklayer = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(masklayer, COLOR_BLACK);
    compo_shape_set_location(masklayer, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_alpha(masklayer, 160);

    //创建文本
    compo_textbox_t *txt1 = compo_textbox_create(frm, 100);
    compo_textbox_set_multiline(txt1, false);
    compo_textbox_set_location(txt1, GUI_SCREEN_CENTER_X, 80, 240, 31);
    compo_textbox_set_autosize(txt1, false);
    compo_textbox_set(txt1, "遥遥领先1遥遥领先2遥遥领先3遥遥领先4遥遥领先5遥遥领先6遥遥领先7遥遥领先8遥遥领先9遥遥领先0");
    compo_textbox_set_autoroll_mode(title, TEXT_AUTOROLL_MODE_SROLL_CIRC);

    compo_textbox_t *txt2 = compo_textbox_create(frm, 100);
    compo_textbox_set_multiline(txt2, true);
    compo_textbox_set_location(txt2, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 240, 120);
    compo_textbox_set_autosize(txt2, false);
    compo_textbox_set(txt2, "遥遥领先1遥遥领先2遥遥领先3遥遥领先4遥遥领先5遥遥领先6遥遥领先7遥遥领先8");
    compo_textbox_set_autoroll_mode(title, TEXT_AUTOROLL_MODE_SROLL_CIRC);

    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    f_clk->sub_frm = frm;
    f_clk->masklayer = masklayer;
}

//时钟表盘主要事件流程处理
static void func_clock_sub_pullup_process(void)
{
    func_clock_sub_process();
}

//时钟表盘上拉菜单功能消息处理
static void func_clock_sub_pullup_message(size_msg_t msg)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_SHORT_DOWN:
        if (func_switching(FUNC_SWITCH_MENU_PULLUP_DOWN, NULL)) {
            f_clk->sta = FUNC_CLOCK_MAIN;                   //下滑返回到时钟主界面
        }
        break;

    case KU_BACK:
        func_switching(FUNC_SWITCH_MENU_PULLUP_DOWN | FUNC_SWITCH_AUTO, NULL);
        f_clk->sta = FUNC_CLOCK_MAIN;                       //单击BACK键返回到时钟主界面
        break;

    default:
        func_clock_sub_message(msg);
        break;
    }
}

//时钟表盘上拉菜单进入处理
static void func_clock_sub_pullup_enter(void)
{
    func_clock_sub_pullup_form_create();
    if (!func_switching(FUNC_SWITCH_MENU_PULLUP_UP, NULL)) {
        return;                                             //上拉到一半取消
    }
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    f_clk->sta = FUNC_CLOCK_SUB_PULLUP;                     //进入到上拉菜单
}

//时钟表盘上拉菜单退出处理
static void func_clock_sub_pullup_exit(void)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    compo_form_destroy(f_clk->sub_frm);
    f_clk->sub_frm = NULL;
}

//时钟表盘上拉菜单
void func_clock_sub_pullup(void)
{
    func_clock_sub_pullup_enter();
    while (func_cb.sta == FUNC_CLOCK && ((f_clock_t *)func_cb.f_cb)->sta == FUNC_CLOCK_SUB_PULLUP) {
        func_clock_sub_pullup_process();
        func_clock_sub_pullup_message(msg_dequeue());
    }
    func_clock_sub_pullup_exit();
}
#endif // 0
