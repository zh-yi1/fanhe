#include "include.h"
#include "func.h"

#if BT_EMIT_EN

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

enum {
    //按键
	COMPO_ID_BTN_LIST = 1,
	COMPO_ID_BTN_SELECT,
//	COMPO_ID_BTN_WATCH,
//	COMPO_ID_BTN_EARPHONE,
};

static const compo_listbox_item_t tbl_msc_src_list[] = {
    {STR_MUSIC_SRC_BT,          .item_mode = COMPO_LISTBOX_ITEM_MODE_NORMAL},
    {STR_MUSIC_SRC_WATCH,       .item_mode = COMPO_LISTBOX_ITEM_MODE_NORMAL},
    {STR_MUSIC_SRC_EARPHONE,    .item_mode = COMPO_LISTBOX_ITEM_MODE_NORMAL},
};

typedef struct f_msc_src_t_ {

} f_msc_src_t;

//创建地图窗体
compo_form_t *func_music_src_form_create(void)
{
    printf("%s\n", __func__);
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_MENU_NORMAL);
    compo_listbox_set(listbox, tbl_msc_src_list, 3);

    compo_listbox_set_bgimg(listbox, UI_BUF_COMMON_BG_BIN);
    compo_listbox_set_focus_byidx(listbox, 1);
    compo_setid(listbox, COMPO_ID_BTN_LIST);
    compo_listbox_update(listbox);

    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_COMMON_SELECT_ON_BIN);
    compo_setid(pic, COMPO_ID_BTN_SELECT);
    compo_picturebox_set_pos(pic, 280, 103 + 90 * sys_cb.music_src);

    return frm;
}

//地图功能事件处理
static void func_music_src_process(void)
{
    func_process();
}

static void func_music_src_button_click(void)
{
    compo_listbox_t *listbox = compo_getobj_byid(COMPO_ID_BTN_LIST);

    int num = compo_listbox_select(listbox, ctp_get_sxy());
    printf("num:%d\n", num);

    if (num < MUSIC_SRC_BT || num > MUSIC_SRC_EARPHONE) {
        return;
    }

    compo_picturebox_t *pic = compo_getobj_byid(COMPO_ID_BTN_SELECT);
    compo_picturebox_set_pos(pic, 280, 103 + 90 * num);
    sys_cb.music_src = num;

    switch (sys_cb.music_src) {
    case MUSIC_SRC_BT:
        func_back_to();
        break;

    case MUSIC_SRC_WATCH:
        func_switch_to(FUNC_MUSIC, FUNC_SWITCH_LR_ZOOM_RIGHT | FUNC_SWITCH_AUTO);         //进入本地音乐界面
        break;

    case MUSIC_SRC_EARPHONE:
        func_switch_to(FUNC_EMIT_LIST, FUNC_SWITCH_LR_ZOOM_LEFT | FUNC_SWITCH_AUTO);       //进入搜索界面
        break;

    default:
        break;
    }

}

//地图功能消息处理
static void func_music_src_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_music_src_button_click();
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

//进入地图功能
void func_music_src_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_msc_src_t));
    func_cb.frm_main = func_music_src_form_create();
}

//退出地图功能
void func_music_src_exit(void)
{
    func_cb.last = FUNC_MUSIC_SRC;
}

//地图功能
void func_music_src(void)
{
    printf("%s\n", __func__);
    func_music_src_enter();
    while (func_cb.sta == FUNC_MUSIC_SRC) {
        func_music_src_process();
        func_music_src_message(msg_dequeue());
    }
    func_music_src_exit();
}
#endif

