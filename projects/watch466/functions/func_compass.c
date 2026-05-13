#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_compass_t_ {

} f_compass_t;

//创建指南针窗体
compo_form_t *func_compass_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_COMPASS]);

    //创建按键
    compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_ICON_COMPASS_BIN);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 200);

    return frm;
}

//指南针功能事件处理
static void func_compass_process(void)
{
    func_process();
}

//指南针功能消息处理
static void func_compass_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        break;

    case MSG_CTP_SHORT_UP:
        break;

    case MSG_CTP_SHORT_DOWN:
        break;

    case MSG_CTP_LONG:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入指南针功能
void func_compass_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_compass_t));
    func_cb.frm_main = func_compass_form_create();
}

//退出指南针功能
void func_compass_exit(void)
{
    func_cb.last = FUNC_COMPASS;
}

//指南针功能
void func_compass(void)
{
    printf("%s\n", __func__);
    func_compass_enter();
    while (func_cb.sta == FUNC_COMPASS) {
        func_compass_process();
        func_compass_message(msg_dequeue());
    }
    func_compass_exit();
}
