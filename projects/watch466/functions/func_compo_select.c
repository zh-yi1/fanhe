#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_compo_select_t_ {

} f_compo_select_t;


enum {
    COMPO_ID_BG_BTN = 1,
};

//创建组件选择窗体
compo_form_t *func_compo_select_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    // //设置标题栏
    // compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    // compo_form_set_title(frm, i18n[STR_COMPO_SELECT]);

    //创建按键
    // compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_COMMON_ADD_BIN);
    // compo_setid(btn, COMPO_ID_BG_BTN);
    // compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);

    return frm;
}

//组件选择功能事件处理
static void func_compo_select_process(void)
{
    func_process();
}

//单击按钮
static void func_compo_select_click(void)
{
    u16 btn_id = compo_get_button_id();
    if (COMPO_ID_BG_BTN == btn_id) {
        func_cb.flag_sort = false;
        func_switch_to(FUNC_COMPO_SELECT_SUB, FUNC_SWITCH_DIRECT);
    }
}

//组件选择功能消息处理
static void func_compo_select_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_compo_select_click();
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

//进入组件选择功能
void func_compo_select_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_compo_select_t));
    func_cb.frm_main = func_compo_select_form_create();
    func_cb.flag_sort = true;
}

//退出组件选择功能
void func_compo_select_exit(void)
{
    func_cb.last = FUNC_COMPO_SELECT;
}

//组件选择功能
void func_compo_select(void)
{
    printf("%s\n", __func__);
    func_compo_select_enter();
    while (func_cb.sta == FUNC_COMPO_SELECT) {
        func_compo_select_process();
        func_compo_select_message(msg_dequeue());
    }
    func_compo_select_exit();
}
