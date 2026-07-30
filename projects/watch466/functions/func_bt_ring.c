#include "include.h"
#include "func.h"
#include "func_bt.h"

enum {
    COMPO_ID_TXT_NUMBER = 0xff,     //避免id被覆盖
    COMPO_ID_BTN_ANSWER,
    COMPO_ID_BTN_REJECT,
    COMPO_ID_BTN_MSG,
};

typedef struct f_bt_call_t_ {

} f_bt_ring_t;

void func_bt_ring_number_update(void)
{
    if (bt_cb.number_sta) {
        compo_textbox_t *number_txt = compo_getobj_byid(COMPO_ID_TXT_NUMBER);
        compo_textbox_set(number_txt, hfp_get_last_call_number(0));
    }
}

//创建窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_bt_ring_form_create(void)
{
    //printf("%s\n", __func__);
    //新建窗体
    compo_form_t *frm = compo_form_create(true);
    compo_button_t *btn;

    compo_textbox_t *number_txt = compo_textbox_create(frm, 20);
    compo_textbox_set_location(number_txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 100, GUI_SCREEN_WIDTH, 50);
    compo_textbox_set_autosize(number_txt, true);
    compo_setid(number_txt, COMPO_ID_TXT_NUMBER);
    msg_enqueue(EVT_CALL_NUMBER_UPDATE);

    //接听
   // btn = compo_button_create_by_image(frm, UI_BUF_CALL_ANSWER_BIN);
    compo_setid(btn, COMPO_ID_BTN_ANSWER);
    compo_button_set_pos(btn, 344, 337);

    //挂断
    //btn = compo_button_create_by_image(frm, UI_BUF_CALL_REJECT_BIN);
    compo_setid(btn, COMPO_ID_BTN_REJECT);
    compo_button_set_pos(btn, 122, 337);

    //信息按钮
    //btn = compo_button_create_by_image(frm, UI_BUF_CALL_MES_BIN);
    compo_setid(btn, COMPO_ID_BTN_MSG);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 377);
    compo_button_set_alpha(btn, UI_BTN_CLICK_EFFECT_ALPHA1); //// 180跟原来的更接近

    //创建消息文本
    compo_textbox_t *txt = compo_textbox_create(frm, 4);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 410);
    // compo_textbox_set(txt, i18n[STR_INFO]); /* STR_INFO 已删 */
    compo_textbox_set(txt, "");

    return frm;
}

void func_bt_ring_process(void)
{
    bsp_bt_ring_process();

#if !CALL_MGR_EN
    if(bt_cb.disp_status != BT_STA_INCOMING) {//退出来电页面
        func_directly_back_to();
        if (bt_cb.disp_status == BT_STA_OUTGOING || bt_cb.disp_status == BT_STA_INCALL) {
            func_cb.sta = FUNC_BT_CALL;
        }
    }
#endif
}

static void func_bt_ring_click(void)
{
    int id = compo_get_button_id();
    switch (id) {
    case COMPO_ID_BTN_ANSWER:
        printf("COMPO_ID_BTN_ANSWER\n");
        bt_call_answer_incoming();
        break;

    case COMPO_ID_BTN_REJECT:
        printf("COMPO_ID_BTN_REJECT\n");
        bt_call_terminate();
        break;

    default:
        break;
    }

}

//蓝牙通话消息处理
static void func_bt_ring_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_bt_ring_click();                         //单击按钮
        break;

    case EVT_CALL_NUMBER_UPDATE:
        func_bt_ring_number_update();
        break;

    case MSG_SYS_500MS:
        reset_sleep_delay_all();                    //来电不休眠
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_UP:
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_bt_ring_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_bt_ring_t));
    func_cb.frm_main = func_bt_ring_form_create();

    func_cb.mp3_res_play = func_bt_mp3_res_play;
    bsp_bt_ring_enter();

    if (sys_cb.gui_sleep_sta) {
        sys_cb.gui_need_wakeup = 1;
    }
}

void func_bt_ring_exit(void)
{
    bsp_bt_ring_exit();
}

void func_bt_ring(void)
{
    u16 interval = 0, latency = 0, tout = 0;

    printf("%s\n", __func__);

    // Adjust BLE connection parameter when incoming
    if (ble_is_connect() && (ble_get_conn_interval() < 400)) {
        interval = ble_get_conn_interval();
        latency = ble_get_conn_latency();
        tout = ble_get_conn_timeout();
        ble_update_conn_param(430, 0, 500);
    }

    func_bt_ring_enter();
    while (func_cb.sta == FUNC_BT_RING) {
        func_bt_ring_process();
        func_bt_ring_message(msg_dequeue());
    }
    func_bt_ring_exit();

    // Restore BLE connection parameter when call stop
    if (bt_cb.disp_status != BT_STA_INCALL) {
        if (interval | latency | tout) {
            ble_update_conn_param(interval, latency, tout);
        }
    }
}
