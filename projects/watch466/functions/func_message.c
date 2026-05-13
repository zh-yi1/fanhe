#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define MSG_DEBUG_EN        1

#define MSG_MAX_LIST_CNT    5  //最多保存多少条消息

#define MSG_MAX_CONTENT     64  //消息保存的最大内容

enum {
    COMPO_ID_LISTBOX = 1,
};

enum{
    //数字
    COMPO_ID_NUM_DISP_ONE = 1,
    COMPO_ID_NUM_DISP_TWS,
    COMPO_ID_NUM_DISP_THR,
};

enum {
    MSG_STA_NULL,
    MSG_STA_SHOW_DETAIL,
    MSG_STA_SHOW_LIST,
};

typedef struct f_message_t_ {
    compo_listbox_t *listbox;
    u8 msg_sta;
    u8 date[16];
    u8 msg[MSG_MAX_LIST_CNT][MSG_MAX_CONTENT];
    compo_listbox_move_cb_t mcb;
} f_message_t;

#if MSG_DEBUG_EN
    char test_data[MSG_MAX_LIST_CNT][MSG_MAX_CONTENT] = {
    {"张三:你好！"},
    {"李四:how are you?"},
    {"陈:1244342343294798732897929"},
    {"欧:创建消息窗体,创建消息窗体,创建消息窗体"},
    {"倪:遥遥领先1遥遥领先2遥遥领先3遥遥领先4"},
};
#endif

static const compo_listbox_item_t tbl_message_list[MSG_MAX_LIST_CNT] = {
    {.res_addr = UI_BUF_SETTING_LIGHT_BIN},
    {.res_addr = UI_BUF_SETTING_LIGHT_BIN},
    {.res_addr = UI_BUF_SETTING_LIGHT_BIN},
    {.res_addr = UI_BUF_SETTING_LIGHT_BIN},
    {.res_addr = UI_BUF_SETTING_LIGHT_BIN},
};

compo_listbox_custom_item_t tbl_message_txt_list[MSG_MAX_LIST_CNT] = {0};


//创建消息窗体
compo_form_t *func_message_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_MESSAGE]);

    //设置消息列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);

#if LE_ANCS_CLIENT_EN
    u16 len = 0;
    ble_ancs_msg_cb_t *ancs_msg = ble_app_ancs_get_msg();

    for (u8 i = 0; i < MSG_MAX_LIST_CNT; i++) {
        if (strlen((char *)ancs_msg->msg_content[i]) > LISTBOX_TEXT_LEN) {
            len = LISTBOX_TEXT_LEN;
            memcpy(tbl_message_txt_list[i].str_txt, ancs_msg->msg_content[i], len - 1);
            tbl_message_txt_list[i].str_txt[len - 1] = '\0';
        } else {
            len = strlen((char *)ancs_msg->msg_content[i]);
            memcpy(tbl_message_txt_list[i].str_txt, ancs_msg->msg_content[i], len);
        }
        printf("len:%d, str:%s", len, tbl_message_txt_list[i].str_txt);
    }
#endif

    compo_listbox_set_rect_size(listbox, GUI_SCREEN_WIDTH, 134, 60);
    compo_listbox_set_rect_color(listbox, make_color(29, 29, 29));
    compo_listbox_set_rect_visible(listbox, true);
    compo_listbox_set(listbox, tbl_message_list, MSG_MAX_LIST_CNT);
    compo_setid(listbox, COMPO_ID_LISTBOX);
    compo_listbox_set_text_modify(listbox, tbl_message_txt_list);

    // u8 msg_idx = sys_cb.msg_idx;
    // if (msg_idx < 1) {
    //     msg_idx = 1;
    // }

    compo_listbox_set_focus(listbox, 140);
    compo_listbox_update(listbox);

    return frm;
}

static void func_message_icon_click(void)
{
    int icon_idx = 0;
    f_message_t *f_msg = (f_message_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_msg->listbox;
    icon_idx = compo_listbox_select(listbox, ctp_get_sxy());
    if (icon_idx < 0 || icon_idx >= MSG_MAX_LIST_CNT) {
        return;
    }

    char *msg = NULL;
    char *title = NULL;
#if LE_ANCS_CLIENT_EN
    ble_ancs_msg_cb_t *ancs_msg = ble_app_ancs_get_msg();
    msg = (char *)&ancs_msg->msg_content[icon_idx];
    title = (char *)&ancs_msg->msg_title[icon_idx];
#endif
    int res = msgbox(msg, title, MSGBOX_MODE_BTN_DELETE, MSGBOX_MSG_TYPE_WECHAT);
    if (res == MSGBOX_RES_DELETE) {
        //TODO: delete msg
    }

}

//消息功能事件处理
static void func_message_process(void)
{
    f_message_t *f_msg = (f_message_t *)func_cb.f_cb;
    compo_listbox_move(f_msg->listbox);
    func_process();
}

//消息功能消息处理
static void func_message_message(size_msg_t msg)
{
    f_message_t *f_msg = (f_message_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_msg->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        func_message_icon_click();
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

//进入消息功能
void func_message_enter(void)
{

    func_cb.f_cb = func_zalloc(sizeof(f_message_t));
    func_cb.frm_main = func_message_form_create();

    f_message_t * f_msg = (f_message_t *)func_cb.f_cb;
    f_msg->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_msg->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_msg->mcb;        //建立移动控制块，退出时需要释放
    compo_listbox_move_init_modify(listbox, listbox->ofs_y, compo_listbox_gety_byidx(listbox, MSG_MAX_LIST_CNT - 2));
}

//退出消息功能
void func_message_exit(void)
{
    func_cb.last = FUNC_MESSAGE;
}

//消息功能
void func_message_info(void)
{
    printf("%s\n", __func__);
    func_message_enter();
    while (func_cb.sta == FUNC_MESSAGE) {
        func_message_process();
        func_message_message(msg_dequeue());
    }
    func_message_exit();
}
