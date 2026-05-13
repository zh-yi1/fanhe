#include "include.h"
#include "func.h"

enum {
	COMPO_ID_BTN_NUM1 = 1,  //按顺序开始
	COMPO_ID_BTN_NUM2,
	COMPO_ID_BTN_NUM3,
	COMPO_ID_BTN_NUM4,
	COMPO_ID_BTN_NUM5,
	COMPO_ID_BTN_NUM6,
	COMPO_ID_BTN_NUM7,
	COMPO_ID_BTN_NUM8,
	COMPO_ID_BTN_NUM9,
    COMPO_ID_BTN_CALL,
    COMPO_ID_BTN_NUM0,
	COMPO_ID_BTN_DEL,       //按顺序结束
    COMPO_ID_NUM_DISP_ZERO,
	COMPO_ID_NUM_DISP_ONE,
	COMPO_ID_NUM_DISP_TWS,
	COMPO_ID_NUM_DISP_THR,
	COMPO_ID_NUM_DISP_FOUR,
	COMPO_ID_NUM_DISP_FIVE,
	COMPO_ID_NUM_DISP_SIX,
	COMPO_ID_NUM_DISP_SEV,
	COMPO_ID_NUM_DISP_EIG,
	COMPO_ID_NUM_DISP_NIN,
	COMPO_ID_NUM_DISP_TEN,
	COMPO_ID_NUM_DISP_ELE,
	COMPO_ID_BTN_ANSWER,
	COMPO_ID_BTN_REJECT,
	COMPO_ID_BTN_MSG,
    COMPO_ID_BTN_MUTE,

	COMPO_ID_PIC_NUM,
};

#define CALL_DISP_BTN_ITEM_CNT      12
#define CALL_BTN_WIDTH              94
#define CALL_BTN_HEI                68

typedef struct f_call_sub_dial_t_ {
    u8 phone_number_cnt;
    u8 phone_number[11];
} f_call_sub_dial_t;

//打电话页面
compo_form_t *func_call_sub_dial_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_CALL_BG_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, 264);

    //创建按钮
    compo_button_t *btn;
    for (u8 idx_btn = 0; idx_btn < CALL_DISP_BTN_ITEM_CNT; idx_btn++) {
        btn = compo_button_create(frm); 
        compo_setid(btn, COMPO_ID_BTN_NUM1 + idx_btn);
        compo_button_set_location(btn, 131 + (idx_btn%3) * (CALL_BTN_WIDTH+8), 150 + (idx_btn/3) * (CALL_BTN_HEI+8), CALL_BTN_WIDTH, CALL_BTN_HEI);
    }

    //创建文本
    compo_textbox_t *txt_title = compo_textbox_create(frm, 30);
    compo_setid(txt_title, COMPO_ID_PIC_NUM);
    compo_textbox_set_location(txt_title, GUI_SCREEN_CENTER_X, 60, 400, 50);
    compo_textbox_set(txt_title, i18n[STR_INPUT_PHONE]);

    //创建数字
    compo_textbox_t *txt_num;
    txt_num = compo_textbox_create(frm, 11);
    compo_textbox_set_font(txt_num, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_setid(txt_num, COMPO_ID_NUM_DISP_ZERO);
    compo_textbox_set_align_center(txt_num, true);
    compo_textbox_set_pos(txt_num, GUI_SCREEN_CENTER_X, 60);
    compo_textbox_set(txt_num, "");

    return frm;
}

//创建来电窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_call_answer_form_create(void)
{
    compo_button_t *btn;

    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //来电按钮
    btn = compo_button_create_by_image(frm, UI_BUF_CALL_ANSWER_BIN);
    compo_setid(btn, COMPO_ID_BTN_ANSWER);
    compo_button_set_pos(btn, 344, 337);
    compo_button_set_alpha(btn, UI_BTN_CLICK_EFFECT_ALPHA1); // 180跟原来的更接近

    //挂电话按钮
    btn = compo_button_create_by_image(frm, UI_BUF_CALL_REJECT_BIN);
    compo_setid(btn, COMPO_ID_BTN_REJECT);
    compo_button_set_pos(btn, 122, 337);
    compo_button_set_alpha(btn, UI_BTN_CLICK_EFFECT_ALPHA1); //// 180跟原来的更接近

    //信息按钮
    btn = compo_button_create_by_image(frm, UI_BUF_CALL_MES_BIN);
    compo_setid(btn, COMPO_ID_BTN_MSG);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 377);
    compo_button_set_alpha(btn, UI_BTN_CLICK_EFFECT_ALPHA1); //// 180跟原来的更接近

    //创建消息文本
    compo_textbox_t *txt = compo_textbox_create(frm, 4);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 420);
    compo_textbox_set(txt, i18n[STR_INFO]);
    return frm;
}

//创建接通窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_calling_form_create(void)
{
    compo_button_t *btn;

    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //新建按钮
    btn = compo_button_create_by_image(frm, UI_BUF_CALL_MUTE_CLICK_BIN);  // UI_BUF_CALL_MUTE_BIN  两个图片不能用透明度进行合一  UI_BUF_CALL_MUTE_BIN 更改透明度之后直接是黑色的 UI_BUF_CALL_MUTE_CLICK_BIN 带有灰色圆
    compo_setid(btn, COMPO_ID_BTN_MUTE);
    compo_button_set_pos(btn, 109, 352);

    btn = compo_button_create_by_image(frm, UI_BUF_CALL_REJECT_BIN);
    compo_setid(btn, COMPO_ID_BTN_REJECT);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 387);
    compo_button_set_alpha(btn, UI_BTN_CLICK_EFFECT_ALPHA1); //// 180跟原来的更接近

    return frm;
}


//进入打电话界面
void func_call_sub_dial_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_call_sub_dial_t));
    func_cb.frm_main = func_call_sub_dial_form_create();
}

//电话功能事件处理
static void func_call_sub_dial_process(void)
{
    func_process();                                  //刷新UI
}

//退出电话表盘功能
void func_call_sub_dial_exit(void)
{
    func_cb.last = FUNC_CALL_SUB_DIAL;
}

//计算前八位的数字显示。
u32 func_calculate_number(u8 cnt, u8 *data)
{
    if(cnt == 8) {
        return data[0]*10000000+data[1]*1000000+data[2]*100000+data[3]*10000+data[4]*1000+data[5]*100+data[6]*10+data[7]*1;
    }else if(cnt == 7) {
        return data[0]*1000000+data[1]*100000+data[2]*10000+data[3]*1000+data[4]*100+data[5]*10+data[6]*1;
    }else if(cnt == 6) {
        return data[0]*100000+data[1]*10000+data[2]*1000+data[3]*100+data[4]*10+data[5]*1;
    }else if(cnt == 5) {
        return data[0]*10000+data[1]*1000+data[2]*100+data[3]*10+data[4]*1;
    }else if(cnt == 4) {
        return data[0]*1000+data[1]*100+data[2]*10+data[3]*1;
    }else if(cnt == 3) {
        return data[0]*100+data[1]*10+data[2]*1;
    }else if(cnt == 2) {
        return data[0]*10+data[1]*1;
    }else if(cnt == 1) {
        return data[0]*1;
    }
    return 0;
}

static void func_call_sub_dial_button_click(void)
{
    int id = compo_get_button_id();
    //获取文本组件的地址
    compo_textbox_t *txt = compo_getobj_byid(COMPO_ID_PIC_NUM);

    //获取数字组件地址
    char buf[34];
    compo_textbox_t *txt_num = compo_getobj_byid(COMPO_ID_NUM_DISP_ZERO);

    f_call_sub_dial_t *call = (f_call_sub_dial_t *)func_cb.f_cb;

    switch (id) {
    case COMPO_ID_BTN_NUM1...COMPO_ID_BTN_NUM9:
        compo_textbox_set_visible(txt, false);
        if(call->phone_number_cnt < 11) {
            call->phone_number[call->phone_number_cnt++] = id;
        }
        break;

    case COMPO_ID_BTN_NUM0:
        compo_textbox_set_visible(txt, false);
        if(call->phone_number_cnt < 11) {
            call->phone_number[call->phone_number_cnt++] = 0;
        }
        break;

    case COMPO_ID_BTN_CALL:
        memset(sys_cb.outgoing_number, 0, 16);
        for(int i=0;i<call->phone_number_cnt;i++) {
            sys_cb.outgoing_number[i] = call->phone_number[i] + 0x30;
        }
#if MODEM_CAT1_EN
        if (bsp_modem_get_init_flag()) {
            modem_call_dial(sys_cb.outgoing_number);
            bsp_modem_call_set_number(sys_cb.outgoing_number);
        } else
#endif
        {
            bt_call_redial_number();
        }
        break;

    case COMPO_ID_BTN_DEL:
        compo_textbox_set_visible(txt, false);
        if(call->phone_number_cnt > 0) {
           call->phone_number_cnt--;
        }
        break;

    default:
        break;
    }

    memset(buf, 0, sizeof(buf));
    if(call->phone_number_cnt == 1) {
        snprintf(buf, sizeof(buf), "%d", call->phone_number[0]);
    }else if(call->phone_number_cnt == 2) {
        snprintf(buf, sizeof(buf), "%d%d", call->phone_number[0], call->phone_number[1]);
    }else if(call->phone_number_cnt == 3) {
        snprintf(buf, sizeof(buf), "%d%d%d", call->phone_number[0], call->phone_number[1], call->phone_number[2]);
    }else if(call->phone_number_cnt == 4) {
        snprintf(buf, sizeof(buf), "%d%d%d%d", call->phone_number[0], call->phone_number[1], call->phone_number[2], call->phone_number[3]);
    }else if(call->phone_number_cnt == 5) {
        snprintf(buf, sizeof(buf), "%d%d%d%d%d", call->phone_number[0], call->phone_number[1], call->phone_number[2], call->phone_number[3], call->phone_number[4]);
    }else if(call->phone_number_cnt == 6) {
        snprintf(buf, sizeof(buf), "%d%d%d%d%d%d", call->phone_number[0], call->phone_number[1], call->phone_number[2], call->phone_number[3], call->phone_number[4], call->phone_number[5]);
    }else if(call->phone_number_cnt == 7) {
        snprintf(buf, sizeof(buf), "%d%d%d%d%d%d%d", call->phone_number[0], call->phone_number[1], call->phone_number[2], call->phone_number[3],
                 call->phone_number[4], call->phone_number[5], call->phone_number[6]);
    }else if(call->phone_number_cnt == 8) {
        snprintf(buf, sizeof(buf), "%d%d%d%d%d%d%d%d", call->phone_number[0], call->phone_number[1], call->phone_number[2], call->phone_number[3],
                 call->phone_number[4], call->phone_number[5], call->phone_number[6], call->phone_number[7]);
    }else if(call->phone_number_cnt == 9) {
        snprintf(buf, sizeof(buf), "%d%d%d%d%d%d%d%d%d", call->phone_number[0], call->phone_number[1], call->phone_number[2], call->phone_number[3],
                 call->phone_number[4], call->phone_number[5], call->phone_number[6], call->phone_number[7], call->phone_number[8]);
    }else if(call->phone_number_cnt == 10) {
        snprintf(buf, sizeof(buf), "%d%d%d%d%d%d%d%d%d%d", call->phone_number[0], call->phone_number[1], call->phone_number[2], call->phone_number[3],
                 call->phone_number[4], call->phone_number[5], call->phone_number[6], call->phone_number[7], call->phone_number[8], call->phone_number[9]);
    }else if(call->phone_number_cnt == 11) {
        snprintf(buf, sizeof(buf), "%d%d%d%d%d%d%d%d%d%d%d", call->phone_number[0], call->phone_number[1], call->phone_number[2], call->phone_number[3],
                 call->phone_number[4], call->phone_number[5], call->phone_number[6], call->phone_number[7], call->phone_number[8], call->phone_number[9], call->phone_number[10]);
    }else {
    }
    compo_textbox_set(txt_num, buf);
}

//电话消息处理
static void func_call_sub_dial_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_call_sub_dial_button_click();
        break;

    case MSG_QDEC_FORWARD:
    case MSG_QDEC_BACKWARD:
    break;

    default:
        func_message(msg);
        break;
    }
}

//电话显示界面
void func_call_sub_dial(void)
{
    printf("%s\n", __func__);
    func_call_sub_dial_enter();
    while (func_cb.sta == FUNC_CALL_SUB_DIAL) {
        func_call_sub_dial_process();
        func_call_sub_dial_message(msg_dequeue());
    }
    func_call_sub_dial_exit();
}
