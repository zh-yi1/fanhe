#include "include.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define ANIMATION_TICK_EXPIRE           18                          //对话框动画切换单位时间(ms)
#define ANIMATION_STEP                  (GUI_SCREEN_WIDTH / 8)      //步进
#if !FLASHDB_
#define MSGBOX_MAX_TXT_LEN              140                         //短信的最大长度
#endif

bool fun_message_is_duanxin(void);

//组件ID
enum{
    //按键
    COMPO_ID_BTN_OK = 1,
    COMPO_ID_BTN_CANCEL,
    COMPO_ID_BTN_YES,
    COMPO_ID_BTN_NO,
    COMPO_ID_BTN_DELETE,
};

enum {
    MSG_TYPE_WECHAT = 1,   //微信消息
};

typedef struct msg_cb_t_ {
    compo_form_t *frm;
    int res;
    u32 tick;
    s32 animation_dx;
    bool show;
    bool flag_animation;
    bool flag_entering;
} msg_cb_t;

//创建对话框窗体
static compo_form_t *msgbox_frm_create(char *msg, char *title, int mode, char msg_type)
{
    compo_button_t *btn;
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //创建遮罩层
    compo_shape_t *masklayer = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(masklayer, COLOR_BLACK);
    compo_shape_set_location(masklayer, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_alpha(masklayer, 255);

    //创建文本
    compo_textbox_t *txt_off = compo_textbox_create(frm, MSGBOX_MAX_TXT_LEN);
    compo_textbox_set_location(txt_off, GUI_SCREEN_CENTER_X, 290, GUI_SCREEN_WIDTH, 200);
    compo_textbox_set_multiline(txt_off, true);
    compo_textbox_set(txt_off, msg);

    //消息推送弹框使用，根据消息类型创建对应消息图标
    switch(msg_type) {
        case MSG_TYPE_WECHAT:
            // compo_form_add_image(frm, UI_BUF_SETTING_LIGHT_BIN, 160, 100);  //需要更替为微信图标
            break;
        default:
            break;
    }

    //创建按钮
    switch (mode) {
    case MSGBOX_MODE_BTN_OK:
        //btn = compo_button_create_by_EN
// #define MSGBOX_MAX_TXT_LEN              64
// #elseimage(frm, UI_BUF_ALARM_CLOCK_YES_BIN);
        // compo_setid(btn, COMPO_ID_BTN_OK);
        // compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 350);
        break;

    case MSGBOX_MODE_BTN_OKCANCEL:
        // btn = compo_button_create_by_image(frm, UI_BUF_COMMON_NO_BIN);
        // compo_setid(btn, COMPO_ID_BTN_CANCEL);
        // compo_button_set_pos(btn, 120, 350);

        // btn = compo_button_create_by_image(frm, UI_BUF_ALARM_CLOCK_YES_BIN);
        // compo_setid(btn, COMPO_ID_BTN_OK);
        // compo_button_set_pos(btn, 340, 350);
        break;

    case MSGBOX_MODE_BTN_YESNO:
        // btn = compo_button_create_by_image(frm, UI_BUF_COMMON_NO_BIN);
        // compo_setid(btn, COMPO_ID_BTN_NO);
        // compo_button_set_pos(btn, 120, 350);

        // btn = compo_button_create_by_image(frm, UI_BUF_ALARM_CLOCK_YES_BIN);
        // compo_setid(btn, COMPO_ID_BTN_YES);
        // compo_button_set_pos(btn, 340, 350);
        break;
    case MSGBOX_MODE_BTN_DELETE:
        // btn = compo_button_create_by_image(frm, UI_BUF_COMMON_NO_BIN);  //需更替为删除图标
        // compo_setid(btn, COMPO_ID_BTN_NO);
        // compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 350);
        break;
    default:
        halt(HALT_MSGBOX_MODE);
        break;
    }
    return frm;
}

//单击按钮
static void msgbox_button_click(void)
{
    int id = compo_get_button_id();
    msg_cb_t *msg_cb = func_cb.msg_cb;
    switch (id) {
    case COMPO_ID_BTN_OK:
        msg_cb->res = MSGBOX_RES_OK;
        msg_cb->flag_animation = true;
        msg_cb->flag_entering = false;
        break;

    case COMPO_ID_BTN_CANCEL:
        msg_cb->res = MSGBOX_RES_CANCEL;
        msg_cb->flag_animation = true;
        msg_cb->flag_entering = false;
        break;

    case COMPO_ID_BTN_YES:
        msg_cb->res = MSGBOX_RES_YES;
        msg_cb->flag_animation = true;
        msg_cb->flag_entering = false;
        break;

    case COMPO_ID_BTN_NO:
        msg_cb->res = MSGBOX_RES_NO;
        msg_cb->flag_animation = true;
        msg_cb->flag_entering = false;
        break;
    case COMPO_ID_BTN_DELETE:
        msg_cb->res = MSGBOX_RES_NO;
        msg_cb->flag_animation = true;
        msg_cb->flag_entering = false;
        break;

    default:
        break;
    }
}

//对话框消息处理
static void msgbox_message(size_msg_t msg)
{
    msg_cb_t *msg_cb = func_cb.msg_cb;
    switch (msg) {
    case MSG_CTP_CLICK:
        if (!msg_cb->flag_animation) {
            msgbox_button_click();                         //单击按钮
        }
        break;

#if FLASHDB_EN
    case MSG_CTP_SHORT_LEFT:
        if (fun_message_is_duanxin()) {
            // 快捷回复
            func_cb.sta = FUNC_MESSAGE_REPLY;
            msg_cb->show = false;
        }
        break;
#endif
    case MSG_CTP_SHORT_RIGHT:
    case KU_BACK:
        msg_cb->flag_animation = true;
        msg_cb->flag_entering = false;
        break;

    default:
        evt_message(msg);
    }
}

//对话框事件处理
static void msgbox_process(void)
{
    msg_cb_t *msg_cb = func_cb.msg_cb;
    u32 wid, hei;
    if (msg_cb->flag_animation) {
        if (tick_check_expire(msg_cb->tick, ANIMATION_TICK_EXPIRE)) {
            msg_cb->tick = tick_get();
            if (msg_cb->flag_entering) {
                msg_cb->animation_dx += ANIMATION_STEP;
            } else {
                msg_cb->animation_dx -= ANIMATION_STEP;
            }
            if (msg_cb->animation_dx <= 0) {
                msg_cb->flag_animation = false;
                msg_cb->show = false;
                wid = 1;
                hei = 1;
            } else if (msg_cb->animation_dx >= GUI_SCREEN_WIDTH) {
                msg_cb->flag_animation = false;
                wid = GUI_SCREEN_WIDTH;
                hei = GUI_SCREEN_HEIGHT;
            } else {
                wid = msg_cb->animation_dx;
                hei = muls_shift16(GUI_SCREEN_HEIGHT, (wid << 16) / GUI_SCREEN_WIDTH);
            }
            compo_form_scale_to(msg_cb->frm, wid, hei);
        }
    }
    func_process();
}

//进入对话框
static void msgbox_enter(compo_form_t *frm)
{
    msg_cb_t *msg_cb = func_zalloc(sizeof(msg_cb_t));
    func_cb.msg_cb = msg_cb;
    msg_cb->frm = frm;
    msg_cb->show = true;
    msg_cb->flag_animation = true;
    msg_cb->flag_entering = true;
    msg_cb->tick = tick_get();
    compo_form_scale_to(msg_cb->frm, 1, 1);
}

//退出对话框
static void msgbox_exit(void)
{
    msg_cb_t *msg_cb = func_cb.msg_cb;
    compo_form_destroy(msg_cb->frm);
    func_free(msg_cb);
    func_cb.msg_cb = NULL;
}

//对话框
int msgbox(char *msg, char *title, int mode, char msg_type)
{
    msg_cb_t *msg_cb;
    msgbox_enter(msgbox_frm_create(msg, title, mode, msg_type));
    msg_cb = func_cb.msg_cb;
    while (msg_cb->show) {
        msgbox_message(msg_dequeue());
        msgbox_process();
    }
    int res = msg_cb->res;
    msgbox_exit();                      //这里释放到msg_cb结构体
    return res;
}
