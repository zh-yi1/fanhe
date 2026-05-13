#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//组件ID
enum {
    //按键
	COMPO_ID_BTN_FLASHON = 1,
	COMPO_ID_BTN_FLASHOFF,

    COMPO_ID_PIC_FLASHON,
	COMPO_ID_PIC_FLASHOFF,
};

typedef struct f_flashlight_t_ {
    u8 flashlight_flag;
} f_flashlight_t;

//创建手电筒窗体
compo_form_t *func_flashlight_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_FLASHLIGHT]);

    //新建按钮
    compo_button_t *btn;
    btn = compo_button_create_by_image(frm, UI_BUF_ICON_FLASHLIGHT_BIN);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 181);


    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BUTTON2_BIN);
    compo_setid(btn, COMPO_ID_BTN_FLASHON);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 418);

	//创建文本
	compo_textbox_t *txt = compo_textbox_create(frm, 10);
	compo_setid(txt, COMPO_ID_PIC_FLASHON);
	compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 418);
    compo_textbox_set(txt, i18n[STR_START]);
    compo_textbox_set_visible(txt, false);

    txt = compo_textbox_create(frm, 10);
    compo_setid(txt, COMPO_ID_PIC_FLASHOFF);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 418);
    compo_textbox_set(txt, i18n[STR_CLOSE]);
    compo_textbox_set_visible(txt, true);

    return frm;
}

//手电筒功能事件处理
static void func_flashlight_process(void)
{
    func_process();
}

//单击按钮
static void func_flashlight_button_click(void)
{
    int id = compo_get_button_id();
    compo_textbox_t *txt_off = compo_getobj_byid(COMPO_ID_PIC_FLASHOFF);
    compo_textbox_t *txt_on  = compo_getobj_byid(COMPO_ID_PIC_FLASHON);
    f_flashlight_t *fst = (f_flashlight_t *)func_cb.f_cb;
    switch(id) {
    case COMPO_ID_BTN_FLASHON:
    if (fst->flashlight_flag) {
        fst->flashlight_flag = 0;
    } else {
        fst->flashlight_flag = 1;
    }
    break;

    default:
    break;
    }
        if (fst->flashlight_flag) {
         compo_textbox_set_visible(txt_on, true);
         compo_textbox_set_visible(txt_off, false);
        } else {
         compo_textbox_set_visible(txt_on, false);
         compo_textbox_set_visible(txt_off, true);
    }
}

//手电筒功能消息处理
static void func_flashlight_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_flashlight_button_click();
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

//进入手电筒控制功能
void func_flashlight_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_flashlight_t));
    func_cb.frm_main = func_flashlight_form_create();
}

//退出手电筒功能
void func_flashlight_exit(void)
{
    func_cb.last = FUNC_FLASHLIGHT;
}

//手电筒功能
void func_flashlight(void)
{
    printf("%s\n", __func__);
    func_flashlight_enter();
    while (func_cb.sta == FUNC_FLASHLIGHT) {
        func_flashlight_process();
        func_flashlight_message(msg_dequeue());
    }
    func_flashlight_exit();
}
