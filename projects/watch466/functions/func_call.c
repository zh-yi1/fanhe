#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define CALL_LIST_CNT                       ((int)(sizeof(tbl_call_list) / sizeof(tbl_call_list[0])))

enum {
    COMPO_ID_LISTBOX = 1,
};

typedef struct f_call_list_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;
} f_call_list_t;

static const compo_listbox_item_t tbl_call_list[] = {
    {STR_CALL_RECENT,      UI_BUF_ICON_CALL_BIN,               .func_sta = FUNC_CALL_SUB_RECORD},
    {STR_CALL_LINK,        UI_BUF_ICON_ADDRESS_BOOK_BIN,       .func_sta = FUNC_CALL_SUB_LINKMAN},
    {STR_CALL_DIAL,        UI_BUF_ICON_MENU_BIN,               .func_sta = FUNC_CALL_SUB_DIAL},
};

//电话页面
compo_form_t *func_call_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //新建列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);
    compo_listbox_set_rect_size(listbox, GUI_SCREEN_WIDTH, 134, 60);
    compo_listbox_set_rect_color(listbox, make_color(29, 29, 29));
    compo_listbox_set_rect_visible(listbox, true);
    compo_listbox_set(listbox, tbl_call_list, CALL_LIST_CNT);
    compo_setid(listbox, COMPO_ID_LISTBOX);

    compo_listbox_set_focus_byidx(listbox, 1);
    compo_listbox_update(listbox);
    return frm;
}

//点进图标进入应用
void func_call_icon_click(void)
{
    int icon_idx;
    f_call_list_t *f_call = (f_call_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_call->listbox;
    u8 func_sta;

    icon_idx = compo_listbox_select(listbox, ctp_get_sxy());
    if (icon_idx < 0 || icon_idx >= CALL_LIST_CNT) {
        return;
    }

    //根据图标索引获取应用ID
    func_sta = tbl_call_list[icon_idx].func_sta;
    //切入应用
    if (func_sta > 0) {
        compo_form_t *frm = func_create_form(func_sta);
        func_switching(FUNC_SWITCH_ZOOM_ENTER | FUNC_SWITCH_AUTO, listbox->sel_icon);
        compo_form_destroy(frm);
        func_cb.sta = func_sta;
    }
}

//电话功能消息处理
static void func_call_list_message(size_msg_t msg)
{
    f_call_list_t *f_call = (f_call_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_call->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }
    switch (msg) {
    case MSG_CTP_CLICK:
        func_call_icon_click();                //单击图标
        break;


    case MSG_CTP_LONG:
        break;

    default:
        func_message(msg);
        break;
    }
}

//电话事件处理
static void func_call_process(void)
{
    f_call_list_t *f_call = (f_call_list_t *)func_cb.f_cb;
    compo_listbox_move(f_call->listbox);
    func_process();
}

//进入电话功能
void func_call_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_call_list_t));
    func_cb.frm_main = func_call_form_create();

    f_call_list_t *f_call = (f_call_list_t *)func_cb.f_cb;
    f_call->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_call->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_call->mcb;        //建立移动控制块，退出时需要释放
    compo_listbox_move_init_modify(listbox, compo_listbox_gety_byidx(listbox, 1), compo_listbox_gety_byidx(listbox, CALL_LIST_CNT - 2));
    func_cb.enter_tick = tick_get();
}

//退出电话功能
void func_call_exit(void)
{
    func_cb.last = FUNC_CALL;
}

//电话功能
void func_call(void)
{
    printf("%s\n", __func__);
    func_call_enter();
    while (func_cb.sta == FUNC_CALL) {
        func_call_process();
        func_call_list_message(msg_dequeue());
    }
    func_call_exit();
}
