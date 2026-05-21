#include "include.h"
#include "func.h"


#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define LANGUAGE_LIST_CNT                       ((int)(sizeof(tbl_language_list) / sizeof(tbl_language_list[0])))

enum {
    COMPO_ID_LISTBOX = 1,
    COMPO_ID_BTN_OK,
};

typedef struct f_language_list_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;

} f_language_list_t;

static const compo_listbox_item_t tbl_language_list[] = {
    {STR_LANGUAGE_ENG,  .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 0},
    {STR_LANGUAGE_CN,   .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 1},
    {STR_LANGUAGE_FN,   .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 2},
    {STR_LANGUAGE_RU,   .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 3},
    {STR_LANGUAGE_AT,   .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 4},
    {STR_LANGUAGE_JP,   .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 5},

};

u8 lang_ctl_bits;   //语言设置控制位

//设置控制位
void func_set_sub_language_set_bit(uint n, u8 v)
{
    if (n >= LANGUAGE_LIST_CNT) {
        halt(HALT_BSP_SYS_CTLBITS);
    }
    int bi = n % 8;
    lang_ctl_bits &= BIT(bi);   //因为是单选，所以其他控制位都置0
    lang_ctl_bits = (lang_ctl_bits & ~BIT(bi)) | (v << bi);
}

//获取控制位
u8 func_set_sub_language_get_bit(uint n)
{
    if (n >= LANGUAGE_LIST_CNT) {
        halt(HALT_BSP_SYS_CTLBITS);
    }
    int bi = n % 8;
    return ((lang_ctl_bits >> bi) & 1);
}

//反转系统控制位
void func_set_sub_language_reverse_bit(uint n)
{
    if (n >= LANGUAGE_LIST_CNT) {
        halt(HALT_BSP_SYS_CTLBITS);
    }
    int bi = n % 8;
    lang_ctl_bits &= BIT(bi);   //因为是单选，所以其他控制位都置0
    lang_ctl_bits ^= BIT(bi);
}

//语言设置页面
compo_form_t *func_set_sub_language_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_SETTING_LANGUAGE]);

    //新建列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_MENU_CIRCLE);
    compo_listbox_set_item_width(listbox, 412);
    compo_listbox_set_item_height(listbox, 82);
    compo_listbox_set(listbox, tbl_language_list, LANGUAGE_LIST_CNT);
    compo_listbox_set_sta_icon(listbox, UI_BUF_COMMON_SELECT_YES_BIN, UI_BUF_COMMON_SELECT_NO_BIN);
    compo_listbox_set_bithook(listbox, func_set_sub_language_get_bit);
    compo_listbox_set_sta_icon_top(listbox, false);
    compo_setid(listbox, COMPO_ID_LISTBOX);

    compo_listbox_set_focus_byidx(listbox, 1);
    compo_listbox_update(listbox);

    func_set_sub_language_set_bit(sys_cb.lang_id, 1);

    //新建按钮
	compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BUTTON1_BIN);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 418);
    compo_setid(btn, COMPO_ID_BTN_OK);

	//创建文本
	compo_textbox_t *txt_start = compo_textbox_create(frm, 10);
	compo_textbox_set_pos(txt_start, GUI_SCREEN_CENTER_X, 418);
    compo_textbox_set(txt_start, "OK");

    return frm;
}

//点进图标
void func_set_sub_language_list_icon_click(void)
{
    f_language_list_t *f_set = (f_language_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;
    int icon_idx = 0;
    point_t point = ctp_get_sxy();
    int id = compo_get_button_id();

    if (point.y < 360) {
        icon_idx = compo_listbox_select(listbox, point);
    }

    if (id == COMPO_ID_BTN_OK) {
        if (!lang_ctl_bits) {
            sys_cb.lang_id = 0; //如果什么都没有选，默认选0
        }
        param_lang_id_write();
        lang_select(sys_cb.lang_id);
        u8 func_sta = task_stack_pop();
        if (func_sta == FUNC_MENU) {
            func_switch_to(func_sta, FUNC_SWITCH_ZOOM_EXIT | FUNC_SWITCH_AUTO);
        } else {
            func_switch_to(func_sta, func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO);
        }
        return;
    }
    if (icon_idx >= 0) {
        sys_cb.lang_id = icon_idx;
        func_set_sub_language_reverse_bit(tbl_language_list[icon_idx].vidx);
        compo_listbox_update(listbox);
    }
}

//语言设置功能事件处理
static void func_set_sub_language_list_process(void)
{
    f_language_list_t *f_set = (f_language_list_t *)func_cb.f_cb;
    compo_listbox_move(f_set->listbox);
    func_process();
}

//语言设置功能消息处理
static void func_set_sub_language_list_message(size_msg_t msg)
{
    f_language_list_t *f_set = (f_language_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        func_set_sub_language_list_icon_click();                //单击图标
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

//进入语言设置功能
void func_set_sub_language_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_language_list_t));
    func_cb.frm_main = func_set_sub_language_form_create();

    f_language_list_t *f_set = (f_language_list_t *)func_cb.f_cb;
    f_set->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_set->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_set->mcb;//func_zalloc(sizeof(compo_listbox_move_cb_t));        //建立移动控制块，退出时需要释放
   // compo_listbox_move_init(listbox);

    compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, LANGUAGE_LIST_CNT - 2));
    func_cb.enter_tick = tick_get();

}

//退出语言设置功能
void func_set_sub_language_exit(void)
{
//    f_language_list_t *f_set = (f_language_list_t *)func_cb.f_cb;
//    compo_listbox_t *listbox = f_set->listbox;
//    func_free(listbox->mcb);                                            //释放移动控制块
    func_cb.last = FUNC_SET_SUB_LANGUAGE;
}

//语言设置功能
void func_set_sub_language(void)
{
    printf("%s\n", __func__);
    func_set_sub_language_enter();
    while (func_cb.sta == FUNC_SET_SUB_LANGUAGE) {
        func_set_sub_language_list_process();
        func_set_sub_language_list_message(msg_dequeue());
    }
    func_set_sub_language_exit();
}
