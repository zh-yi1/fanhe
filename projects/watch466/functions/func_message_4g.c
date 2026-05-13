#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define MSG_DEBUG_EN        0

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
    u32 msg_query[MSG_MAX_LIST_CNT * 2];
    u8 msg[140]; // 一条最大 140
    struct msg_ctrl msg_header;
    char msg_name[8];
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
};

compo_listbox_custom_item_t tbl_message_txt_list[MSG_MAX_LIST_CNT] = {0};
static char reply_number[16];

char *fun_message_get_reply_number(void)
{
    return reply_number;
}

bool fun_message_is_duanxin(void)
{
    f_message_t *f_msg = (f_message_t *)func_cb.f_cb;
    if ((func_cb.sta == FUNC_MESSAGE) && (strncmp(f_msg->msg_name, "DuanXin", 7) == 0)) {
        return true;
    }
    return false;
}

//创建消息窗体
compo_form_t *func_message_form_create(void)
{
    f_message_t *f_msg = (f_message_t *)func_cb.f_cb;
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_MESSAGE]);

    //设置消息列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);

    u16 msg_len = msg_tsdb_query(f_msg->msg_query, MSG_MAX_LIST_CNT, false);
    #undef min
    for (u8 i = 0; i < msg_len; i++) {
        tm_t tm = time_to_tm(f_msg->msg_query[2 * i]);
        sprintf(tbl_message_txt_list[i].str_txt, "%d/%d-%d:%d:%d\n", tm.mon,tm.day, tm.hour, tm.min, tm.sec);
    }

    compo_listbox_set(listbox, tbl_message_list, MSG_MAX_LIST_CNT);
    compo_listbox_set_bgimg(listbox, UI_BUF_COMMON_BG_BIN);
    compo_setid(listbox, COMPO_ID_LISTBOX);
    compo_listbox_set_text_modify(listbox, tbl_message_txt_list);

    compo_listbox_set_focus_byidx(listbox, COMPO_ID_NUM_DISP_ONE);
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

    char *title = NULL;

    u16 msg_num = msg_tsdb_query(f_msg->msg_query, MSG_MAX_LIST_CNT, true);
    if (msg_num >= (icon_idx + 1)) {
        title = tbl_message_txt_list[icon_idx].str_txt;
        int ret = msg_tsdb_read_init(f_msg->msg_query[2 * icon_idx + 1], &f_msg->msg_header);
        if (ret == 0) {
            msg_tsdb_read_name(&f_msg->msg_header, f_msg->msg_name, 8);
            msg_tsdb_read_message(&f_msg->msg_header, f_msg->msg, sizeof(f_msg->msg));
            if (strncmp(f_msg->msg_name, "DuanXin", 8) == 0) {
                msg_tsdb_read_title(&f_msg->msg_header, reply_number, sizeof(reply_number));
            }
            printf(">> %s %s %s\n", f_msg->msg_name, reply_number, f_msg->msg);
        }
    }

    int res = msgbox((char *)f_msg->msg, title, MSGBOX_MODE_BTN_DELETE, MSGBOX_MSG_TYPE_WECHAT);
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
static void func_message_enter(void)
{

    func_cb.f_cb = func_zalloc(sizeof(f_message_t));
    func_cb.frm_main = func_message_form_create();

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
static void func_message_exit(void)
{
    f_message_t *f_msg = (f_message_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_msg->listbox;
    func_free(listbox->mcb);
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
