#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define STOPWATCH_LIST_CNT                       ((int)(sizeof(tbl_stopwatch_list) / sizeof(tbl_stopwatch_list[0])))

enum {
    COMPO_ID_LISTBOX = 1,
};

typedef struct f_stopwatch_sub_record_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;
} f_stopwatch_sub_record_t;

static const compo_listbox_item_t tbl_stopwatch_list[STOPWATCH_REC_NUM_MAX];
compo_listbox_custom_item_t tbl_stopwatch_txt_list[STOPWATCH_REC_NUM_MAX];

//创建秒表记录窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_stopwatch_sub_record_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    // compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    // compo_form_set_title(frm, i18n[STR_STOP_WATCH_RECORD]);

    // //新建列表
    // compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);
    // compo_listbox_set(listbox, tbl_stopwatch_list, sys_cb.stopwatch_rec_cnt);
    // compo_setid(listbox, COMPO_ID_LISTBOX);

    // compo_listbox_set_bgimg(listbox, UI_BUF_COMMON_BG_BIN);
    // compo_listbox_set_text_modify(listbox, tbl_stopwatch_txt_list);

	// u8 min = 0;
	// u8 sec = 0;
	// u16 msec = 0;
    // for (u8 i = 0; i < sys_cb.stopwatch_rec_cnt; i++) {
    //     min = ((sys_cb.stopwatch_rec_view[i] / 1000) % 3600) / 60;
    //     sec = (sys_cb.stopwatch_rec_view[i] / 1000) % 60;
    //     msec = sys_cb.stopwatch_rec_view[i] % 1000;
    //     sprintf(tbl_stopwatch_txt_list[i].str_txt, "%2d. %02d:%02d.%02d", sys_cb.stopwatch_rec_cnt - i, min, sec, msec / 10);
    // }
    // compo_listbox_set_focus_byidx(listbox, 1);
    // compo_listbox_update(listbox);

    return frm;
}

//秒表记录功能事件处理
static void func_stopwatch_sub_record_process(void)
{
    f_stopwatch_sub_record_t *f_stopwatch = (f_stopwatch_sub_record_t *)func_cb.f_cb;
    compo_listbox_move(f_stopwatch->listbox);
    func_process();
}

//秒表记录功能消息处理
static void func_stopwatch_sub_record_message(size_msg_t msg)
{
    f_stopwatch_sub_record_t *f_stopwatch = (f_stopwatch_sub_record_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_stopwatch->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }

    switch (msg) {
    case MSG_CTP_CLICK:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_LONG:
        break;

    case MSG_CTP_SHORT_RIGHT:
        func_backing_to();
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入秒表记录功能
void func_stopwatch_sub_record_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_stopwatch_sub_record_t));
    func_cb.frm_main = func_stopwatch_sub_record_form_create();

    f_stopwatch_sub_record_t *f_stopwatch = (f_stopwatch_sub_record_t *)func_cb.f_cb;
    f_stopwatch->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_stopwatch->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_stopwatch->mcb;        //建立移动控制块，退出时需要释放
    compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, STOPWATCH_LIST_CNT - 2));
    func_cb.enter_tick = tick_get();
}

//退出秒表记录功能
void func_stopwatch_sub_record_exit(void)
{
    func_cb.last = FUNC_STOPWATCH_SUB_RECORD;
}

//秒表记录功能
void func_stopwatch_sub_record(void)
{
    printf("%s\n", __func__);
    func_stopwatch_sub_record_enter();
    while (func_cb.sta == FUNC_STOPWATCH_SUB_RECORD) {
        func_stopwatch_sub_record_process();
        func_stopwatch_sub_record_message(msg_dequeue());
    }
    func_stopwatch_sub_record_exit();
}
