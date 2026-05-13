#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_password_t_ {

} f_password_t;

//组件ID
enum{
    COMPO_ID_BIN_ON = 1,
    COMPO_ID_BIN_OFF,
    COMPO_ID_BIN_CHANGE,

};

//密码设置页面
compo_form_t *func_set_sub_password_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_SETTING_PASSWORD]);

    //创建文本
    compo_textbox_t *txt_onoff = compo_textbox_create(frm, 20);
    compo_textbox_set_align_center(txt_onoff, false);
    compo_textbox_set_pos(txt_onoff, 40, 122);
    compo_textbox_set(txt_onoff, i18n[STR_PASSWORD_SWI]);

    compo_textbox_t *txt_change = compo_textbox_create(frm, 20);
    compo_textbox_set_align_center(txt_change, false);
    compo_textbox_set_pos(txt_change, 40, 190);
    compo_textbox_set(txt_change, i18n[STR_PASSWORD_CHANGE]);
    compo_textbox_set_visible(txt_change, false);

    if (sys_cb.password_cnt == 4) {
        compo_textbox_set_visible(txt_change, true);
    }

    //创建按钮
    compo_button_t *btn;
    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_SWITCH_BUTTON_OFF_BIN);
    compo_setid(btn, COMPO_ID_BIN_OFF);
    compo_button_set_pos(btn, 388, 140);
    compo_button_set_visible(btn, false);

    if (sys_cb.password_cnt != 4) {
        compo_button_set_visible(btn, true);
    }

    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_SWITCH_BUTTON_ON_BIN);
    compo_setid(btn, COMPO_ID_BIN_ON);
    compo_button_set_pos(btn, 388, 140);
    compo_button_set_visible(btn, false);

    if (sys_cb.password_cnt == 4) {
        compo_button_set_visible(btn, true);
    }

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BIN_CHANGE);
    compo_button_set_location(btn, 390, 210, 60, 60);
    // compo_button_set_visible(btn, false);

    if (sys_cb.password_cnt == 4) {
        compo_button_set_visible(btn, true);
        compo_form_add_image(frm, UI_BUF_SETTING_PASSWORD_OPEN_BIN, 390, 210);
    }

    return frm;
}

//密码设置功能事件处理
static void func_set_sub_password_process(void)
{
    func_process();
}

//单击按钮
static void func_password_button_click(void)
{
    int id = compo_get_button_id();

    switch (id) {
    case COMPO_ID_BIN_OFF:
        func_cb.sta = FUNC_PASSWORD_SUB_DISP;
        break;

    case COMPO_ID_BIN_CHANGE:
        sys_cb.password_change = 1;
        func_cb.sta = FUNC_PASSWORD_SUB_DISP;
        break;

    default:
        break;
    }
}

//密码设置功能消息处理
static void func_set_sub_password_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_password_button_click();
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入密码设置功能
void func_set_sub_password_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_password_t));
    func_cb.frm_main = func_set_sub_password_form_create();
    if(sys_cb.password_change) {
        sys_cb.password_change = 0;
    }
}

//退出密码设置功能
void func_set_sub_password_exit(void)
{
    func_cb.last = FUNC_SET_SUB_PASSWORD;
}

//密码设置功能
void func_set_sub_password(void)
{
    printf("%s\n", __func__);
    func_set_sub_password_enter();
    while (func_cb.sta == FUNC_SET_SUB_PASSWORD) {
        func_set_sub_password_process();
        func_set_sub_password_message(msg_dequeue());
    }
    func_set_sub_password_exit();
}
