#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

extern bool func_switching_flag;
#define ADDRESS_BOOK_LIST_CNT			5 //显示的联系人个数

enum {
    COMPO_ID_LISTBOX = 1,
};

typedef struct f_address_book_list_t_ {
    compo_listbox_t *listbox;
    //临时使用pbap的数据，后续有app使用contact_info_t格式
    pbap_pb_buf_t pb_list[ADDRESS_BOOK_LIST_CNT];
    char num[16];
    u8 kv_idx;
} f_address_book_list_t;

static const compo_listbox_item_t tbl_call_list[ADDRESS_BOOK_LIST_CNT] = {0};
compo_listbox_custom_item_t tbl_call_txt_list[ADDRESS_BOOK_LIST_CNT] = {0};

static bool kvdb_cb(char *name, void *arg)
{
    f_address_book_list_t *f_book = (f_address_book_list_t *)arg;
    contacts_kvdb_get_number(name, f_book->num, sizeof(f_book->num));
    // printf("-- %s %s\n", name, f_book->num);
    sprintf(tbl_call_txt_list[f_book->kv_idx++].str_txt, "%s:%s", name, f_book->num);

    return false;
}

//创建电话簿窗体
compo_form_t *func_address_book_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_CALL_LINK]);

    f_address_book_list_t *f_book = (f_address_book_list_t *)func_cb.f_cb;
    f_book->kv_idx = 0;
    contacts_kvdb_iterate(kvdb_cb, f_book);

    //新建列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);
    compo_listbox_set(listbox, tbl_call_list, ADDRESS_BOOK_LIST_CNT);
    compo_listbox_set_bgimg(listbox, UI_BUF_COMMON_BG_BIN);
    compo_setid(listbox, COMPO_ID_LISTBOX);
    compo_listbox_set_text_modify(listbox, tbl_call_txt_list);

    compo_listbox_set_focus_byidx(listbox, 1);
    compo_listbox_update(listbox);

    return frm;
}

static void func_address_book_sync_pb_callback(void *info, u16 count)
{
    if (info == NULL || count == 0) {
        printf("[%s]:error!",__func__);
        return;
    }

    f_address_book_list_t *f_book = (f_address_book_list_t *)func_cb.f_cb;
    f_book->kv_idx = 0;
    contacts_kvdb_iterate(kvdb_cb, f_book);

	if (func_cb.sta == FUNC_ADDRESS_BOOK && !func_switching_flag) {
        compo_listbox_t *listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
        compo_listbox_set(listbox, tbl_call_list, ADDRESS_BOOK_LIST_CNT);
        compo_listbox_set_text_modify(listbox, tbl_call_txt_list);
        compo_listbox_set_focus_byidx(listbox, 1);
        compo_listbox_update(listbox);
	}
}

//点进图标进入应用
void func_address_book_icon_click(void)
{
    int icon_idx;
	u8 i = 0;
    f_address_book_list_t *f_book = (f_address_book_list_t *)func_cb.f_cb;
    pbap_pb_buf_t *pb_list = f_book->pb_list;
    compo_listbox_t *listbox = f_book->listbox;
    icon_idx = compo_listbox_select(listbox, ctp_get_sxy());
    if (icon_idx < 0 || icon_idx >= ADDRESS_BOOK_LIST_CNT) {
        return;
    }

	memset(sys_cb.outgoing_number, 0, 16);
	for (i = 0; i < 16; i++) {
		sys_cb.outgoing_number[i] = pb_list[icon_idx].num[i];
	}
	bt_call_redial_number();
}

//电话簿功能事件处理
static void func_address_book_process(void)
{
    f_address_book_list_t *f_book = (f_address_book_list_t *)func_cb.f_cb;
    compo_listbox_move(f_book->listbox);
    func_process();
}

//电话簿功能消息处理
static void func_address_book_message(size_msg_t msg)
{
    f_address_book_list_t *f_book = (f_address_book_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_book->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }
    switch (msg) {
    case MSG_CTP_CLICK:
       	func_address_book_icon_click();
        break;

    case MSG_CTP_LONG:
        break;

    case KU_DELAY_BACK:
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {

        }
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入电话簿功能
static void func_address_book_enter(void)
{
    contacts_kvdb_init();
    func_cb.f_cb = func_zalloc(sizeof(f_address_book_list_t));
    func_cb.frm_main = func_address_book_form_create();

    f_address_book_list_t *f_book = (f_address_book_list_t *)func_cb.f_cb;
    f_book->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_book->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = func_zalloc(sizeof(compo_listbox_move_cb_t));        //建立移动控制块，退出时需要释放

    compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, ADDRESS_BOOK_LIST_CNT - 2));
    func_cb.enter_tick = tick_get();

#if MODEM_CAT1_EN
    modem_mtcas_get_contains(1, 5);
#endif
    printf("%s:%d\n", __func__ , bt_pbap_is_syncing());
    //目前无app,暂时从pbap里拿数据
    if (!bt_pbap_is_syncing()) {
        bt_pbap_sync_start(PBAP_OBJECT_PB, f_book->pb_list,
                           ADDRESS_BOOK_LIST_CNT, func_address_book_sync_pb_callback);
	}
}

//退出电话簿功能
static void func_address_book_exit(void)
{
    f_address_book_list_t *f_book = (f_address_book_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_book->listbox;
    bt_pbap_sync_stop();
    func_free(listbox->mcb);                                            //释放移动控制块
    func_cb.last = FUNC_ADDRESS_BOOK;
}

//电话簿功能
void func_address_book(void)
{
    printf("%s\n", __func__);
    func_address_book_enter();
    while (func_cb.sta == FUNC_ADDRESS_BOOK) {
        func_address_book_process();
        func_address_book_message(msg_dequeue());
    }
    func_address_book_exit();
}
