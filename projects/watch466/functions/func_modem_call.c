#include "include.h"
#include "func.h"
#include "func_bt.h"

void mic_mute(bool en);
enum {
    COMPO_ID_TXT_NUMBER = 0xff,     //避免id被覆盖
    COMPO_ID_TXT_TIME,
    COMPO_ID_BTN_REJECT,
    COMPO_ID_BTN_MIC,
};

typedef struct f_modem_call_t_ {
    bool sta;                   //0:呼出(outgoing); 1:通话(call);

    char call_time_str[10];     //通话计时字符串
    u32 clcc_tick;              //主动查询号码计时
    u32 exit_tick;              //页面退出计时
    u8 mic_mute;
} f_modem_call_t;

static void func_modem_call_back_to(void)
{
    u8 last_func = func_directly_back_to();
    if (last_func == FUNC_MODEM_RING) {
        func_directly_back_to();
    }
}

void modem_incall_time_update(void)
{
    f_modem_call_t *f_modem_call = (f_modem_call_t *)func_cb.f_cb;
    char *call_time_str = f_modem_call->call_time_str;

    u16 call_times = bsp_modem_call_get_times();
    u8 hours   = call_times / 3600;
    u8 minutes = (call_times % 3600) / 60;
    u8 seconds = call_times % 60;
    sprintf(call_time_str, "%02d:%02d:%02d", hours, minutes, seconds);
//    printf("call_time_str: %s\n", call_time_str);

    compo_textbox_t *time_txt = compo_getobj_byid(COMPO_ID_TXT_TIME);
    compo_textbox_set(time_txt, call_time_str);
    compo_textbox_set_visible(time_txt, true);
}

void func_modem_call_number_update(void)
{
    // if (modem_cb.number_sta) {
        compo_textbox_t *number_txt = compo_getobj_byid(COMPO_ID_TXT_NUMBER);
        compo_textbox_set(number_txt, modem_cb.number);
    // }
}

//创建窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_modem_call_form_create(void)
{
    //printf("%s\n", __func__);
    //新建窗体, 通话页面
    compo_form_t *frm = compo_form_create(true);
    compo_button_t *btn;

    compo_textbox_t *number_txt = compo_textbox_create(frm, 20);
    compo_textbox_set_location(number_txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 50, GUI_SCREEN_WIDTH, 50);
    compo_textbox_set_autosize(number_txt, true);
    compo_setid(number_txt, COMPO_ID_TXT_NUMBER);
    msg_enqueue(EVT_CALL_NUMBER_UPDATE);

    compo_textbox_t *time_txt = compo_textbox_create(frm, 10);
    compo_textbox_set_location(time_txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, 50);
    compo_textbox_set_autosize(time_txt, true);
    compo_setid(time_txt, COMPO_ID_TXT_TIME);

    //挂断按钮
    btn = compo_button_create_by_image(frm, UI_BUF_CALL_REJECT_BIN);
    compo_setid(btn, COMPO_ID_BTN_REJECT);
    compo_button_set_pos(btn, 160, 313);

    //mic
    btn = compo_button_create_by_image(frm, UI_BUF_CALL_MUTE_BIN);
    compo_setid(btn, COMPO_ID_BTN_MIC);
    compo_button_set_pos(btn, 49, 313);

    return frm;
}

//创建窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_modem_outgoing_form_create(void)
{
    //新建窗体, 呼出页面
    compo_form_t *frm = compo_form_create(true);
    compo_button_t *btn;

    compo_textbox_t *number_txt = compo_textbox_create(frm, 20);
    compo_textbox_set_location(number_txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 50, GUI_SCREEN_WIDTH, 50);
    compo_textbox_set_autosize(number_txt, true);
    compo_setid(number_txt, COMPO_ID_TXT_NUMBER);
    msg_enqueue(EVT_CALL_NUMBER_UPDATE);

    //挂断按钮
    btn = compo_button_create_by_image(frm, UI_BUF_CALL_REJECT_BIN);
    compo_setid(btn, COMPO_ID_BTN_REJECT);
    compo_button_set_pos(btn, 160, 313);

    return frm;
}

AT(.text.func.bt)
static void func_modem_call_interface(void)
{
    f_modem_call_t *f_modem_call = (f_modem_call_t *)func_cb.f_cb;

    if (modem_cb.disp_status == BT_STA_INCALL && f_modem_call->sta == false) {
        //销毁窗体
        if (func_cb.frm_main != NULL) {
            compo_form_destroy(func_cb.frm_main);
        }

        func_cb.frm_main = func_modem_call_form_create();
        f_modem_call->sta = true;
    }
}

AT(.text.func.bt)
static void func_modem_call_exit_process(void)
{
    f_modem_call_t *f_modem_call = (f_modem_call_t *)func_cb.f_cb;

    // if (!modem_cb.number_sta) {                                                            //号码未通知, 主动查询
    //     if (tick_check_expire(f_modem_call->clcc_tick, 500)) {
    //         f_modem_call->clcc_tick = tick_get();
    //         // modem_call_get_remote_phone_number();
    //     }
    // }

    if (f_modem_call->exit_tick && tick_check_expire(f_modem_call->exit_tick, 1000)) {        //强制退出, 防呆
        f_modem_call->exit_tick = 0;
        func_modem_call_back_to();
    }
}

void func_modem_call_process(void)
{
    func_process();
    func_modem_call_exit_process();
    func_modem_call_interface();
    reset_sleep_delay_all();
}

static void func_modem_call_click(void)
{
#if MODEM_CAT1_EN
    f_modem_call_t *f_modem_call = (f_modem_call_t *)func_cb.f_cb;

    int id = compo_get_button_id();
    switch (id) {

    case COMPO_ID_BTN_REJECT:
        printf("COMPO_ID_BTN_REJECT\n");
        modem_call_hangup();
        f_modem_call->exit_tick = tick_get();
        break;
    case COMPO_ID_BTN_MIC:
        printf("COMPO_ID_BTN_MIC\n");
        if (!f_modem_call->mic_mute) {
            f_modem_call->mic_mute = 1;
            mic_mute(true);
        } else {
            f_modem_call->mic_mute = 0;
            mic_mute(false);
        }
        break;
    default:
        break;
    }
#endif

}

//蓝牙音乐消息处理
static void func_modem_call_message(size_msg_t msg)
{
    f_modem_call_t *f_modem_call = (f_modem_call_t *)func_cb.f_cb;

    switch (msg) {
    case KU_BACK:
#if MODEM_CAT1_EN
        // if(modem_get_call_indicate() == BT_CALL_INCOMING) {
        //     modem_call_answer();                  //接听第2路通话
        // } else {
            modem_call_hangup();                        //挂断当前通话
        // }
#endif
        break;

    case KU_RIGHT:
        break;

    case KU_LEFT:
        break;

    case MSG_SYS_1S:
        func_message(MSG_SYS_1S);
        if (f_modem_call->sta == true) {
            modem_incall_time_update();
        }
        break;

    case MSG_CTP_CLICK:
        func_modem_call_click();                           //单击按钮
        break;

    case EVT_CALL_NUMBER_UPDATE:
        func_modem_call_number_update();
        break;

    case MSG_SYS_500MS:
        reset_guioff_delay();                           //来电不休眠
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_modem_call_mgr(bsp_call_mgr_msg_t msg)
{
    printf("%s %d\n", __func__, msg);
    switch (msg) {
    case CALL_MGR_MODEM_INCOM:
        func_cb.sta          = FUNC_MODEM_RING;
        modem_cb.disp_status = BT_STA_INCOMING;
        break;
    case CALL_MGR_MODEM_OUTGO:

        func_cb.sta          = FUNC_MODEM_CALL;
        modem_cb.disp_status = BT_STA_OUTGOING;
        break;
    case CALL_MGR_MODEM_INCALL:
        modem_cb.flags |= MCALL_FLAG_INCALL;

        func_cb.sta          = FUNC_MODEM_CALL;
        modem_cb.disp_status = BT_STA_INCALL;
        break;
    case CALL_MGR_MODEM_ENDCALL:
        modem_cb.disp_status = BT_STA_CONNECTED;
        memset(modem_cb.number, 0, sizeof(modem_cb.number));
        modem_cb.times = 0;
        modem_cb.flags &= ~MCALL_FLAG_INCALL;
        break;
    default:
        break;
    }
}

void func_modem_call_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_modem_call_t));
    func_cb.frm_main = func_modem_outgoing_form_create();

    func_cb.mp3_res_play = mp3_res_play;

    if (sys_cb.gui_sleep_sta) {
        sys_cb.gui_need_wakeup = 1;
    }
}

void func_modem_call_exit(void)
{
}

void func_modem_call(void)
{
    printf("%s\n", __func__);
    func_modem_call_enter();
    while (func_cb.sta == FUNC_MODEM_CALL) {
        func_modem_call_process();
        func_modem_call_message(msg_dequeue());
    }
    func_modem_call_exit();
}
