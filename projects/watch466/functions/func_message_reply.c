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

char *fun_message_get_reply_number(void);

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
    u16 reply_ucs2[15];
} f_message_t;

static const compo_listbox_item_t tbl_message_list[MSG_MAX_LIST_CNT] = {
    {.res_addr = UI_BUF_SETTING_LIGHT_BIN},
    {.res_addr = UI_BUF_SETTING_LIGHT_BIN},
    {.res_addr = UI_BUF_SETTING_LIGHT_BIN},
    {.res_addr = UI_BUF_SETTING_LIGHT_BIN},
};

compo_listbox_custom_item_t tbl_message_reply_txt_list[MSG_MAX_LIST_CNT] = {0};

static const char *default_reply[MSG_MAX_LIST_CNT] = {
    "在忙1",
    "稍后给您回电话",
    "短信测试a",
    "短信测试",
    "短信测试",
};

//创建消息窗体
compo_form_t *func_message_reply_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_MESSAGE]);

    //设置消息列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);

    compo_listbox_set(listbox, tbl_message_list, MSG_MAX_LIST_CNT);
    compo_listbox_set_bgimg(listbox, UI_BUF_COMMON_BG_BIN);
    compo_setid(listbox, COMPO_ID_LISTBOX);
    compo_listbox_set_text_modify(listbox, tbl_message_reply_txt_list);

    for (u8 i = 0; i < MSG_MAX_LIST_CNT; i++) {
        memcpy(tbl_message_reply_txt_list[i].str_txt, default_reply[i], strlen(default_reply[i]) + 1);
    }

    compo_listbox_set_focus_byidx(listbox, COMPO_ID_NUM_DISP_ONE);
    compo_listbox_update(listbox);

    return frm;
}

int utf8_to_unicode_be(u16 *u, const char *p, int max_word_cnt);
static void func_message_reply_icon_click(void)
{
    int icon_idx = 0;
    f_message_t *f_msg = (f_message_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_msg->listbox;
    icon_idx = compo_listbox_select(listbox, ctp_get_sxy());
    if (icon_idx < 0 || icon_idx >= MSG_MAX_LIST_CNT) {
        return;
    }
    char *num = fun_message_get_reply_number();
    int ret = utf8_to_unicode_be(f_msg->reply_ucs2, default_reply[icon_idx], sizeof(f_msg->reply_ucs2));
    printf("--> %s %s %d\n", num, default_reply[icon_idx], ret);
    // print_r(f_msg->reply_ucs2, 30);
    modem_sms_submit_ucs2(num, (const u8 *)f_msg->reply_ucs2, ret * 2);
}

//消息功能事件处理
static void func_message_reply_process(void)
{
    f_message_t *f_msg = (f_message_t *)func_cb.f_cb;
    compo_listbox_move(f_msg->listbox);
    func_process();
}

//消息功能消息处理
static void func_message_reply_message(size_msg_t msg)
{
    f_message_t *f_msg = (f_message_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_msg->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        func_message_reply_icon_click();
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
static void func_message_reply_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_message_t));
    func_cb.frm_main = func_message_reply_form_create();

    f_message_t * f_msg = (f_message_t *)func_cb.f_cb;
    f_msg->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_msg->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = func_zalloc(sizeof(compo_listbox_move_cb_t));        //建立移动控制块，退出时需要释放
    compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, MSG_MAX_LIST_CNT - 2));
}

//退出消息功能
static void func_message_reply_exit(void)
{
    f_message_t *f_msg = (f_message_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_msg->listbox;
    func_free(listbox->mcb);
    func_cb.last = FUNC_MESSAGE_REPLY;
}

//消息功能
void func_message_reply_info(void)
{
    printf("%s\n", __func__);
    func_message_reply_enter();
    while (func_cb.sta == FUNC_MESSAGE_REPLY) {
        func_message_reply_process();
        func_message_reply_message(msg_dequeue());
    }
    func_message_reply_exit();
}
