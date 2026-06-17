#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_altitude_t_ {

} f_altitude_t;

//创建海拔窗体
compo_form_t *func_altitude_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    // //设置标题栏
    // compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    // compo_form_set_title_center(frm, true);
    // compo_form_set_title(frm, i18n[STR_ALTITUDE]);

	// //创建按键
    // compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_ICON_ALTITUDE_BIN);
    // compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 200);

    return frm;
}

//海拔功能事件处理
static void func_altitude_process(void)
{
    func_process();
}

//海拔功能消息处理
static void func_altitude_message(size_msg_t msg)
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

//进入海拔功能
void func_altitude_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_altitude_t));
    func_cb.frm_main = func_altitude_form_create();
}

//退出海拔功能
void func_altitude_exit(void)
{
    func_cb.last = FUNC_ALTITUDE;
}

//海拔功能
void func_altitude(void)
{
    printf("%s\n", __func__);
    func_altitude_enter();
    while (func_cb.sta == FUNC_ALTITUDE) {
        func_altitude_process();
        func_altitude_message(msg_dequeue());
    }
    func_altitude_exit();
}
