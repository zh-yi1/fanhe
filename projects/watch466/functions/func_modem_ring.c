#include "include.h"
#include "func.h"
#include "func_bt.h"

enum {
    COMPO_ID_TXT_NUMBER = 0xff,     //避免id被覆盖
    COMPO_ID_BTN_ANSWER,
    COMPO_ID_BTN_REJECT,
};

typedef struct f_modem_call_t_ {

} f_modem_ring_t;

void func_modem_ring_number_update(void)
{
    // if (modem_cb.number_sta) {
        compo_textbox_t *number_txt = compo_getobj_byid(COMPO_ID_TXT_NUMBER);
        compo_textbox_set(number_txt, modem_cb.number);
        printf("num: [%s]", modem_cb.number);
    // }
}

//创建窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_modem_ring_form_create(void)
{
    //printf("%s\n", __func__);
    //新建窗体
    compo_form_t *frm = compo_form_create(true);
    compo_button_t *btn;

    compo_textbox_t *number_txt = compo_textbox_create(frm, 20);
    compo_textbox_set_location(number_txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 50, GUI_SCREEN_WIDTH, 50);
    compo_textbox_set_autosize(number_txt, true);
    compo_setid(number_txt, COMPO_ID_TXT_NUMBER);
    msg_enqueue(EVT_CALL_NUMBER_UPDATE);

    //接听
    btn = compo_button_create_by_image(frm, UI_BUF_CALL_ANSWER_BIN);
    compo_setid(btn, COMPO_ID_BTN_ANSWER);
    compo_button_set_pos(btn, 250, 279);

    //挂断
    btn = compo_button_create_by_image(frm, UI_BUF_CALL_REJECT_BIN);
    compo_setid(btn, COMPO_ID_BTN_REJECT);
    compo_button_set_pos(btn, 70, 279);

    return frm;
}

void func_modem_ring_process(void)
{
    func_process();
    reset_sleep_delay_all();
    // bsp_modem_ring_process();
}

static void func_modem_ring_click(void)
{
#if MODEM_CAT1_EN
    int id = compo_get_button_id();
    switch (id) {
    case COMPO_ID_BTN_ANSWER:
        printf("COMPO_ID_BTN_ANSWER\n");
        modem_call_answer();
        break;

    case COMPO_ID_BTN_REJECT:
        printf("COMPO_ID_BTN_REJECT\n");
        modem_call_hangup();
        break;

    default:
        break;
    }
#endif
}

//蓝牙通话消息处理
static void func_modem_ring_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_modem_ring_click();                         //单击按钮
        break;

    case EVT_CALL_NUMBER_UPDATE:
        func_modem_ring_number_update();
        break;

    case MSG_SYS_500MS:
        reset_guioff_delay();                           //来电不休眠
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_modem_ring_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_modem_ring_t));
    func_cb.frm_main = func_modem_ring_form_create();

    func_cb.mp3_res_play = mp3_res_play;
    // bsp_modem_ring_enter();

    if (sys_cb.gui_sleep_sta) {
        sys_cb.gui_need_wakeup = 1;
    }
}

void func_modem_ring_exit(void)
{
    // bsp_modem_ring_exit();
}

void func_modem_ring(void)
{
    printf("%s\n", __func__);
    func_modem_ring_enter();
    while (func_cb.sta == FUNC_MODEM_RING) {
        func_modem_ring_process();
        func_modem_ring_message(msg_dequeue());
    }
    func_modem_ring_exit();
}
