#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_about_t_ {
    u8 value;
} f_about_t;

enum {
    COMPO_ID_BIN_4G = 1,
};

//关于页面
compo_form_t *func_set_sub_4g_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    // //设置标题栏
    // compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    // compo_form_set_title_center(frm, true);
    // compo_form_set_title(frm, i18n[STR_SETTING_4G]);

    // compo_textbox_t *txt_shake = compo_textbox_create(frm, 20);
    // compo_textbox_set_align_center(txt_shake, false);
    // compo_textbox_set_pos(txt_shake, 30, 194);
    // compo_textbox_set(txt_shake, i18n[STR_SETTING_4G_SWI]);

    // compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_COMMON_SWITCH_BUTTON_OFF_BIN);
    // compo_setid(btn, COMPO_ID_BIN_4G);
    // compo_button_set_pos(btn, 380, 214);

    return frm;
}

//更新显示界面
static void func_set_sub_4g_disp(void)
{
    // f_about_t *about = (f_about_t *)func_cb.f_cb;
    // compo_button_t *btn = compo_getobj_byid(COMPO_ID_BIN_4G);

    // if (about->value) {
    //     compo_button_set_bgimg(btn, UI_BUF_COMMON_SWITCH_BUTTON_ON_BIN);
    // } else {
    //     compo_button_set_bgimg(btn, UI_BUF_COMMON_SWITCH_BUTTON_OFF_BIN);
    // }
}

//单击按钮
static void func_wrist_button_click(void)
{
    f_about_t *about = (f_about_t *)func_cb.f_cb;
    int id = compo_get_button_id();

    switch(id) {
    case COMPO_ID_BIN_4G:
        about->value ^= 1;
        break;

    default:
        break;
    }
    func_set_sub_4g_disp();
}

//关于功能事件处理
static void func_set_sub_4g_process(void)
{
    func_process();
}

//关于功能消息处理
static void func_set_sub_4g_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_wrist_button_click();
        break;

    case KU_DELAY_BACK:
        break;

    default:
		func_message(msg);
        break;
    }
}

//进入关于功能
void func_set_sub_4g_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_about_t));
    func_cb.frm_main = func_set_sub_4g_form_create();

    f_about_t *about = (f_about_t *)func_cb.f_cb;
    printf("4g sta: %d\n", bsp_modem_get_init_flag());
    if (bsp_modem_get_init_flag()) {
        about->value = 1;
    } else {
        about->value = 0;
    }
    func_set_sub_4g_disp();
}

//退出关于功能
void func_set_sub_4g_exit(void)
{
   // func_cb.last = FUNC_SET_SUB_4G;
}

//关于功能
void func_set_sub_4g(void)
{
    printf("%s\n", __func__);
    func_set_sub_4g_enter();
    while (func_cb.sta == FUNC_SET_SUB_4G) {
        func_set_sub_4g_process();
        func_set_sub_4g_message(msg_dequeue());
    }
    func_set_sub_4g_exit();
}
