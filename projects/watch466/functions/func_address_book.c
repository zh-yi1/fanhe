#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define ADDRESS_BOOK_LIST_CNT			5 //显示的联系人个数

enum {
    COMPO_ID_LISTBOX = 1,
};

typedef struct f_address_book_list_t_ {
    compo_listbox_t *listbox;
    //临时使用pbap的数据，后续有app使用contact_info_t格式
    pbap_pb_buf_t pb_list[ADDRESS_BOOK_LIST_CNT];
    compo_listbox_move_cb_t mcb;
} f_address_book_list_t;

//static const compo_listbox_item_t tbl_call_list[ADDRESS_BOOK_LIST_CNT] = {0};
compo_listbox_custom_item_t tbl_call_txt_list[ADDRESS_BOOK_LIST_CNT] = {0};

static const compo_listbox_item_t tbl_call_list[ADDRESS_BOOK_LIST_CNT] = {
    {STR_CALL_LINK_ONE},
    {STR_CALL_LINK_TWS},
    {STR_CALL_LINK_THR},
    {STR_CALL_LINK_FOUR},
    {STR_CALL_LINK_FIVE},
};

//创建电话簿窗体
compo_form_t *func_address_book_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_CALL_LINK]);

    //新建列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);
    compo_listbox_set_rect_size(listbox, GUI_SCREEN_WIDTH, 90, 60);
    compo_listbox_set_rect_color(listbox, make_color(29, 29, 29));
    compo_listbox_set_rect_visible(listbox, true);
    compo_listbox_set(listbox, tbl_call_list, ADDRESS_BOOK_LIST_CNT);
    compo_setid(listbox, COMPO_ID_LISTBOX);
    compo_listbox_set_item_text(listbox, GUI_SCREEN_CENTER_X, 40, GUI_SCREEN_WIDTH, 40, true);

    compo_listbox_set_focus_byidx(listbox, 1);
    compo_listbox_update(listbox);

    return frm;
}

#if BT_PBAP_EN
static void func_address_book_sync_pb_callback(void *info, u16 count)
{
    if (info == NULL || count == 0) {
        printf("[%s]:error!",__func__);
        return;
    }

    pbap_pb_buf_t *pb_buf = (pbap_pb_buf_t *)info;

    for (u16 i = 0; i < count; i++) {
        if (strlen(pb_buf[i].name) + strlen(pb_buf[i].num) > LISTBOX_TEXT_LEN - 2) {
            printf("[%s]:str len exceeds the max!\n", __func__);
			continue;
		}
		if (i >= ADDRESS_BOOK_LIST_CNT) {
            break;
		}
        sprintf(tbl_call_txt_list[i].str_txt, "%s:%s", pb_buf[i].name, pb_buf[i].num);
	}
	if (func_cb.sta == FUNC_ADDRESS_BOOK && !sys_cb.flag_swithing) {
        compo_listbox_t *listbox = compo_getobj_byid(COMPO_ID_LISTBOX);

        u16 count_tmp = count;
        if (count_tmp > ADDRESS_BOOK_LIST_CNT) {
            count_tmp = ADDRESS_BOOK_LIST_CNT;
        }
        if (count_tmp < 2 && count_tmp > 0) {
            count_tmp = 2;
        }
        if (count) {
            compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, count_tmp - 2));
            compo_listbox_set(listbox, tbl_call_list, count);
        }

        compo_listbox_set_text_modify(listbox, tbl_call_txt_list);
        compo_listbox_set_focus_byidx(listbox, 1);
        compo_listbox_update(listbox);
	}
}
#endif

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
//    case MSG_CTP_SHORT_LEFT:
//    case MSG_CTP_SHORT_RIGHT:
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
void func_address_book_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_address_book_list_t));
    func_cb.frm_main = func_address_book_form_create();

    f_address_book_list_t *f_book = (f_address_book_list_t *)func_cb.f_cb;
    f_book->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_book->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_book->mcb;
    compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, ADDRESS_BOOK_LIST_CNT - 2));
    func_cb.enter_tick = tick_get();

#if BT_PBAP_EN
    printf("%s:%d\n", __func__ , bt_pbap_is_syncing());
    //目前无app,暂时从pbap里拿数据
    if (!bt_pbap_is_syncing()) {
        bt_pbap_sync_start(PBAP_OBJECT_PB, f_book->pb_list,
                           ADDRESS_BOOK_LIST_CNT, func_address_book_sync_pb_callback);
	}
#endif
}

//退出电话簿功能
void func_address_book_exit(void)
{
#if BT_PBAP_EN
    bt_pbap_sync_stop();
#endif
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
