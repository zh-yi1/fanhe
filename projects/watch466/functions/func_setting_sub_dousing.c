#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define DOUSING_LIST_CNT                       ((int)(sizeof(tbl_dousing_list) / sizeof(tbl_dousing_list[0])))

enum {
    COMPO_ID_BTN_NUM0 = 1,
    COMPO_ID_BTN_NUM1,
    COMPO_ID_BTN_NUM2,
    COMPO_ID_BTN_NUM3,
    COMPO_ID_BTN_NUM4,
    COMPO_ID_BTN_NUM5,
    COMPO_ID_BTN_NUM6,
    COMPO_ID_BTN_NUM7,
    COMPO_ID_BTN_OK,
    COMPO_ID_LISTBOX,
};

typedef struct f_dousing_list_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;
    u32 sleep_time;
} f_dousing_list_t;

static const compo_listbox_item_t tbl_dousing_list[] = {
    {STR_FIVE_SEC,      .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 0},
    {STR_TEN_SEC,       .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 1},
    {STR_TWENTY_SEC,    .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 2},
    {STR_THIRTY_SEC,    .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 3},
    {STR_ONE_MIN,       .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 4},
    {STR_FIVE_MIN,      .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 5},
    {STR_NEVER,         .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 6},
};

u8 dousing_ctl_bits;   //控制位

uint func_set_sub_dousing_get_sleep_id()
{
    uint id = 0;
    if (sys_cb.sleep_time == 50) {
        id = 0;
    } else if (sys_cb.sleep_time == 100) {
        id = 1;
    } else if (sys_cb.sleep_time == 200) {
        id = 2;
    } else if (sys_cb.sleep_time == 300) {
        id = 3;
    } else if (sys_cb.sleep_time == 600) {
        id = 4;
    } else if (sys_cb.sleep_time == 3000) {
        id = 5;
    } else if (sys_cb.sleep_time == -1) {
        id = 6;
    }
    return id;
}

//设置控制位
void func_set_sub_dousing_set_bit(uint n, u8 v)
{
    if (n >= DOUSING_LIST_CNT) {
        halt(HALT_BSP_SYS_CTLBITS);
    }
    int bi = n % 8;
    dousing_ctl_bits &= BIT(bi);   //因为是单选，所以其他控制位都置0
    dousing_ctl_bits = (dousing_ctl_bits & ~BIT(bi)) | (v << bi);
}

//获取控制位
u8 func_set_sub_dousing_get_bit(uint n)
{
    if (n >= DOUSING_LIST_CNT) {
        halt(HALT_BSP_SYS_CTLBITS);
    }
    int bi = n % 8;
    return ((dousing_ctl_bits >> bi) & 1);
}

//反转系统控制位
void func_set_sub_dousing_reverse_bit(uint n)
{
    if (n >= DOUSING_LIST_CNT) {
        halt(HALT_BSP_SYS_CTLBITS);
    }
    int bi = n % 8;
    dousing_ctl_bits &= BIT(bi);   //因为是单选，所以其他控制位都置0
    dousing_ctl_bits ^= BIT(bi);
}

//熄屏设置页面
compo_form_t *func_set_sub_dousing_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_SETTING_DOUSING]);

    //新建列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);
    compo_listbox_set_rect_size(listbox, GUI_SCREEN_WIDTH, 90, 60);
    compo_listbox_set_rect_color(listbox, make_color(29, 29, 29));
    compo_listbox_set_rect_visible(listbox, true);
    compo_listbox_set(listbox, tbl_dousing_list, DOUSING_LIST_CNT);
    compo_listbox_set_sta_icon(listbox, UI_BUF_COMMON_HOOK_BIN, 0);
    compo_listbox_set_sta_icon_pos(listbox, listbox->item_width - 70, 0);
    compo_listbox_set_bithook(listbox, func_set_sub_dousing_get_bit);
    compo_listbox_set_sta_icon_top(listbox, false);

    compo_setid(listbox, COMPO_ID_LISTBOX);

    compo_listbox_set_item_text(listbox, 80, 25, 380, 40, false);
    compo_listbox_set_focus_byidx(listbox, 1);
    compo_listbox_update(listbox);

    func_set_sub_dousing_set_bit(func_set_sub_dousing_get_sleep_id(), 1);

    return frm;
}

//点进图标进入应用
void func_set_sub_dousing_list_icon_click(void)
{
    f_dousing_list_t *f_set = (f_dousing_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;
    int icon_idx = compo_listbox_select(listbox, ctp_get_sxy());

    if (icon_idx < 0 || icon_idx >= DOUSING_LIST_CNT) {
        return;
    }

    //切入应用
    switch(icon_idx) {
    case  COMPO_ID_BTN_NUM0...COMPO_ID_BTN_NUM6:
        if (icon_idx==COMPO_ID_BTN_NUM0) {
            sys_cb.sleep_time = 50;
        } else if (icon_idx==COMPO_ID_BTN_NUM1) {
            sys_cb.sleep_time = 100;
        } else if (icon_idx==COMPO_ID_BTN_NUM2) {
            sys_cb.sleep_time = 200;
        } else if (icon_idx==COMPO_ID_BTN_NUM3) {
            sys_cb.sleep_time = 300;
        } else if (icon_idx==COMPO_ID_BTN_NUM4) {
            sys_cb.sleep_time = 600;
        } else if (icon_idx==COMPO_ID_BTN_NUM5) {
            sys_cb.sleep_time = 3000;
        } else if (icon_idx==COMPO_ID_BTN_NUM6) {
            sys_cb.sleep_time = -1;
        }
//        u8 func_sta = task_stack_pop();
//        func_switch_to(func_sta, func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO);
        break;

    default:
        break;
    }
    func_set_sub_dousing_reverse_bit(tbl_dousing_list[icon_idx].vidx);
    compo_listbox_update(listbox);
}

//熄屏时长功能事件处理
static void func_set_sub_dousing_list_process(void)
{
    f_dousing_list_t *f_set = (f_dousing_list_t *)func_cb.f_cb;
    compo_listbox_move(f_set->listbox);
    func_process();
}

//熄屏时长功能消息处理
static void func_set_sub_dousing_list_message(size_msg_t msg)
{
    f_dousing_list_t *f_set = (f_dousing_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }
    switch (msg) {
    case MSG_CTP_CLICK:
        func_set_sub_dousing_list_icon_click();                //单击图标
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

//进入设置功能
void func_set_sub_dousing_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_dousing_list_t));
    func_cb.frm_main = func_set_sub_dousing_form_create();

    f_dousing_list_t *f_set = (f_dousing_list_t *)func_cb.f_cb;
    f_set->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_set->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_set->mcb;//func_zalloc(sizeof(compo_listbox_move_cb_t));        //建立移动控制块，退出时需要释放
    //compo_listbox_move_init(listbox);

    compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, DOUSING_LIST_CNT - 2));
    func_cb.enter_tick = tick_get();
}

//退出设置功能
void func_set_sub_dousing_exit(void)
{
//    f_dousing_list_t *f_set = (f_dousing_list_t *)func_cb.f_cb;
//    compo_listbox_t *listbox = f_set->listbox;
//    func_free(listbox->mcb);                                            //释放移动控制块
    func_cb.last = FUNC_SET_SUB_DOUSING;
}

//设置功能
void func_set_sub_dousing(void)
{
    printf("%s\n", __func__);
    func_set_sub_dousing_enter();
    while (func_cb.sta == FUNC_SET_SUB_DOUSING) {
        func_set_sub_dousing_list_process();
        func_set_sub_dousing_list_message(msg_dequeue());
    }
    func_set_sub_dousing_exit();
}
