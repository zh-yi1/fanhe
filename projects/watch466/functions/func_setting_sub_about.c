#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

extern void func_debug_info_check();

typedef struct f_about_t_ {

} f_about_t;

//关于页面
compo_form_t *func_set_sub_about_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_SETTING_ABOUT]);

    return frm;
}

//关于功能事件处理
static void func_set_sub_about_process(void)
{
    func_debug_info_check();
    func_process();
}

//关于功能消息处理
static void func_set_sub_about_message(size_msg_t msg)
{
    switch (msg) {

    case KU_DELAY_BACK:
        break;

    default:
		func_message(msg);
        break;
    }
}

//进入关于功能
void func_set_sub_about_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_about_t));
    func_cb.frm_main = func_set_sub_about_form_create();
}

//退出关于功能
void func_set_sub_about_exit(void)
{
    func_cb.last = FUNC_SET_SUB_ABOUT;
}

//关于功能
void func_set_sub_about(void)
{
    printf("%s\n", __func__);
    func_set_sub_about_enter();
    while (func_cb.sta == FUNC_SET_SUB_ABOUT) {
        func_set_sub_about_process();
        func_set_sub_about_message(msg_dequeue());
    }
    func_set_sub_about_exit();
}
