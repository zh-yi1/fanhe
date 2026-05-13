#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//组件ID
enum{
    //按键
    COMPO_ID_BTN_NUM1 = 1,
    COMPO_ID_BTN_NUM2,
    COMPO_ID_BTN_NUM3,
    COMPO_ID_BTN_NUM4,
    COMPO_ID_BTN_NUM5,
    COMPO_ID_BTN_NUM6,
    COMPO_ID_BTN_NUM7,
    COMPO_ID_BTN_NUM8,
    COMPO_ID_BTN_NUM9,
    COMPO_ID_BTN_NULL,
    COMPO_ID_BTN_NUM0,
    COMPO_ID_BTN_DEL_CLICK,

    //图像
    COMPO_ID_PIC_PASSWORD_ZERO,
    COMPO_ID_PIC_PASSWORD_ONE,
    COMPO_ID_PIC_PASSWORD_TWS,
    COMPO_ID_PIC_PASSWORD_THR,

    //文本
    COMPO_ID_TXT_NEWPASSWORD,

    //数字
	COMPO_ID_NUM_DISP_ZERO,
	COMPO_ID_NUM_DISP_ONE,
	COMPO_ID_NUM_DISP_TWS,
	COMPO_ID_NUM_DISP_THR,
};

typedef struct f_password_sub_disp_t_ {
   u8 value[4];
   u8 cnt;
} f_password_sub_disp_t;

#define PASSWORD_NUM_ITEM_CNT           1
#define PASSWORD_DISP_BTN_ITEM_CNT      11
#define PASSWORD_BTN_WIDTH              94
#define PASSWORD_BTN_HEI                68
#define PASSWORD_Y_POS                  75
#define PASSWORD_DISP_PIC_ITEM_CNT    ((int)(sizeof(tbl_password_disp_pic_item) / sizeof(tbl_password_disp_pic_item[0])))

typedef struct password_disp_pic_item_t_ {
    u32 res_addr;
    u16 pic_id;
    s16 x;
    s16 y;
    bool visible_en;
} password_disp_pic_item_t;

//图片item，创建时遍历一下
static const password_disp_pic_item_t tbl_password_disp_pic_item[] = {
    {UI_BUF_SETTING_PASSWORD_NUM_BIN,     COMPO_ID_PIC_PASSWORD_ZERO,        199,    PASSWORD_Y_POS,    false},
    {UI_BUF_SETTING_PASSWORD_NUM_BIN,     COMPO_ID_PIC_PASSWORD_ONE,         216,    PASSWORD_Y_POS,    false},
    {UI_BUF_SETTING_PASSWORD_NUM_BIN,     COMPO_ID_PIC_PASSWORD_TWS,         233,    PASSWORD_Y_POS,    false},
    {UI_BUF_SETTING_PASSWORD_NUM_BIN,     COMPO_ID_PIC_PASSWORD_THR,         250,    PASSWORD_Y_POS,    false},
};


typedef struct password_num_item_t_ {
    u32 res_addr;
    int num_cnt;
    u16 num_id;
    int val;
    s16 x;
    s16 y;
    bool zfill_en;
    bool visible_en;
} password_num_item_t;


//创建密码--开启密码锁显示窗体
compo_form_t *func_password_sub_disp_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);
    compo_form_add_image(frm, UI_BUF_SETTING_PASSWORD_BG_BIN, GUI_SCREEN_CENTER_X, 210 + PASSWORD_Y_POS);

    //创建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 20);
    compo_setid(txt, COMPO_ID_TXT_NEWPASSWORD);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, PASSWORD_Y_POS);
    compo_textbox_set(txt, i18n[STR_CUR_PASSWORD]);
    compo_textbox_set_visible(txt, true);

    if(sys_cb.password_cnt == 4 && sys_cb.password_change) {
        compo_textbox_set(txt, i18n[STR_OLD_PASSWORD]);
    }else if(sys_cb.password_cnt == 0 && !sys_cb.password_change){
        compo_textbox_set(txt, i18n[STR_NEW_PASSWORD]);
    }

    //新建图像
    compo_picturebox_t *pic_click;
    for (u8 idx = 0; idx < PASSWORD_DISP_PIC_ITEM_CNT; idx++) {
        pic_click = compo_picturebox_create(frm, tbl_password_disp_pic_item[idx].res_addr);
        compo_setid(pic_click, tbl_password_disp_pic_item[idx].pic_id);
        compo_picturebox_set_pos(pic_click, tbl_password_disp_pic_item[idx].x, tbl_password_disp_pic_item[idx].y);
        compo_picturebox_set_visible(pic_click, tbl_password_disp_pic_item[idx].visible_en);
    }

    //创建按钮
    compo_button_t *btn;
    for (u8 idx_btn = 0; idx_btn < PASSWORD_DISP_BTN_ITEM_CNT + 1; idx_btn++) {
        btn = compo_button_create(frm);
        if (idx_btn == 9) {
            continue;
        } else {
            compo_setid(btn, COMPO_ID_BTN_NUM1 + idx_btn);
        }
        compo_button_set_location(btn, 131 + (idx_btn%3) * (PASSWORD_BTN_WIDTH+8), PASSWORD_Y_POS + 120 + (idx_btn/3) * (PASSWORD_BTN_HEI+8), PASSWORD_BTN_WIDTH, PASSWORD_BTN_HEI);
    }

    //创建数字
	char buf[13];
    compo_textbox_t *txt_num;
    for (u8 idx = 0; idx < PASSWORD_NUM_ITEM_CNT; idx++) {
        txt_num = compo_textbox_create(frm, 7);
		compo_textbox_set_font(txt_num, UI_BUF_0FONT_FONT_NUM_38_BIN);
        compo_setid(txt_num, COMPO_ID_NUM_DISP_ZERO + idx);
        compo_textbox_set_pos(txt_num, GUI_SCREEN_CENTER_X, PASSWORD_Y_POS);
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "%d%d%d%d", sys_cb.password_value[idx], sys_cb.password_value[idx+1], sys_cb.password_value[idx+2], sys_cb.password_value[idx+3]);
        compo_textbox_set(txt_num, buf);
        compo_textbox_set_visible(txt_num, false);
    }

    return frm;
}

//开启密码锁功能事件处理
static void func_password_sub_disp_process(void)
{
    func_process();
}

//单击按钮
static void func_password_sub_disp_button_click(void)
{
    int id = compo_get_button_id();
    f_password_sub_disp_t *password = (f_password_sub_disp_t *)func_cb.f_cb;
    u8 psd_ctn = 0;
    char buf[13];
    //获取数字组件的地址
    compo_textbox_t *txt_num  = compo_getobj_byid(COMPO_ID_NUM_DISP_ZERO);
    //获取图片组件的地址
    compo_picturebox_t *pic_zer = compo_getobj_byid(COMPO_ID_PIC_PASSWORD_ZERO);
    compo_picturebox_t *pic_one = compo_getobj_byid(COMPO_ID_PIC_PASSWORD_ONE);
    compo_picturebox_t *pic_tws = compo_getobj_byid(COMPO_ID_PIC_PASSWORD_TWS);
    compo_picturebox_t *pic_thr = compo_getobj_byid(COMPO_ID_PIC_PASSWORD_THR);
    compo_textbox_t *txt  = compo_getobj_byid(COMPO_ID_TXT_NEWPASSWORD);

    switch (id) {
    case COMPO_ID_BTN_NUM1...COMPO_ID_BTN_NUM9:
        compo_textbox_set_visible(txt, false);
        if(password->cnt < 4) {
            password->value[password->cnt++] = id;
        }
        compo_textbox_set_visible(txt_num, true);
        break;

    case COMPO_ID_BTN_NUM0:
        compo_textbox_set_visible(txt, false);
        if(password->cnt < 4) {
            password->value[password->cnt++] = 0;
        }
        compo_textbox_set_visible(txt_num, true);
        break;

    case COMPO_ID_BTN_DEL_CLICK:
        compo_textbox_set_visible(txt, false);
        if(password->cnt > 0) {
            password->cnt--;
        }
        break;

    default:
        compo_textbox_set_visible(txt, false);
        break;
    }

    if(password->cnt == 1) {
        compo_picturebox_set_visible(pic_zer, false);
        compo_picturebox_set_visible(pic_one, true);
        compo_picturebox_set_visible(pic_tws, true);
        compo_picturebox_set_visible(pic_thr, true);

        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "%d", password->value[0]);
        compo_textbox_set(txt_num, buf);
    }
    else if(password->cnt == 2) {
        compo_picturebox_set_visible(pic_zer, false);
        compo_picturebox_set_visible(pic_one, false);
        compo_picturebox_set_visible(pic_tws, true);
        compo_picturebox_set_visible(pic_thr, true);

        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "%d%d", password->value[0], password->value[1]);
        compo_textbox_set(txt_num, buf);
    }
    else if(password->cnt == 3) {
        compo_picturebox_set_visible(pic_zer, false);
        compo_picturebox_set_visible(pic_one, false);
        compo_picturebox_set_visible(pic_tws, false);
        compo_picturebox_set_visible(pic_thr, true);

        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "%d%d%d", password->value[0], password->value[1], password->value[2]);
        compo_textbox_set(txt_num, buf);
    }
    else if(password->cnt == 4) {
        compo_picturebox_set_visible(pic_zer, false);
        compo_picturebox_set_visible(pic_one, false);
        compo_picturebox_set_visible(pic_tws, false);
        compo_picturebox_set_visible(pic_thr, false);

        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "%d%d%d%d", password->value[0], password->value[1], password->value[2], password->value[3]);
        compo_textbox_set(txt_num, buf);

        if(sys_cb.password_cnt == 0 && password->cnt == 4 && !sys_cb.password_change) {
            sys_cb.password_cnt = password->cnt;
            for(int i = 0; i < password->cnt; i++) {
                sys_cb.password_value[i] = password->value[i];
            }
            func_cb.sta = FUNC_PASSWORD_SUB_SELECT;
        }
        else if(sys_cb.password_cnt == 4 && password->cnt == 4 && !sys_cb.password_change) {
            for(int j = 0; j <password->cnt;j++)  {
                if(sys_cb.password_value[j] == password->value[j]) {
                    psd_ctn++;
                }
                if(psd_ctn == 4) {
                    sys_cb.password_cnt = 0;
                    func_cb.sta = FUNC_SET_SUB_PASSWORD;
                }
            }
        }
        else if(sys_cb.password_cnt == 0 && password->cnt == 4 && sys_cb.password_change) {
            sys_cb.password_cnt = password->cnt;
            for(int i = 0; i < password->cnt; i++) {
                sys_cb.password_value[i] = password->value[i];
            }
            func_cb.sta = FUNC_PASSWORD_SUB_SELECT;
        }
        else if(sys_cb.password_cnt == 4 && password->cnt == 4 && sys_cb.password_change) {
            for(int j = 0; j <password->cnt;j++)  {
                if(sys_cb.password_value[j] == password->value[j]) {
                    psd_ctn++;
                }
            }
        }
            compo_picturebox_set_visible(pic_zer, false);
            compo_picturebox_set_visible(pic_one, false);
            compo_picturebox_set_visible(pic_tws, false);
            compo_picturebox_set_visible(pic_thr, false);

            compo_textbox_set(txt_num, "");

            if(psd_ctn == 4) {
                sys_cb.password_cnt = 0;
                password->cnt = 0;
                compo_textbox_set(txt, i18n[STR_NEW_PASSWORD]);
                compo_textbox_set_visible(txt, true);
            }else {
                compo_textbox_set(txt, i18n[STR_PASSWORD_ERR]);
                compo_textbox_set_visible(txt, true);
                password->cnt = 0;
            }
    }
    else{
        compo_picturebox_set_visible(pic_zer, true);
        compo_picturebox_set_visible(pic_one, true);
        compo_picturebox_set_visible(pic_tws, true);
        compo_picturebox_set_visible(pic_thr, true);

        compo_textbox_set(txt_num, "");
    }
}

//开启密码锁功能消息处理
static void func_password_sub_disp_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_password_sub_disp_button_click();
        break;

    case MSG_CTP_SHORT_RIGHT:
        if(sys_cb.password_change && sys_cb.password_cnt == 0) {
            sys_cb.password_cnt = 4;
        }
        func_message(msg);
        break;

    case MSG_QDEC_FORWARD:
    case MSG_QDEC_BACKWARD:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入开启密码锁功能
void func_password_sub_disp_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_password_sub_disp_t));
    func_cb.frm_main = func_password_sub_disp_form_create();
}

//退出开启密码锁功能
void func_password_sub_disp_exit(void)
{
    f_password_sub_disp_t *password = (f_password_sub_disp_t *)func_cb.f_cb;
    func_cb.last = FUNC_PASSWORD_SUB_DISP;
    password->cnt = 0;
}

//开启密码锁功能
void func_password_sub_disp(void)
{
    printf("%s\n", __func__);
    func_password_sub_disp_enter();
    while (func_cb.sta == FUNC_PASSWORD_SUB_DISP) {
        func_password_sub_disp_process();
        func_password_sub_disp_message(msg_dequeue());
    }
    func_password_sub_disp_exit();
}
