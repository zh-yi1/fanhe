#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define MENU_NAVIGATION_LIST_CNT                       ((int)(sizeof(tbl_menu_navigation_list) / sizeof(tbl_menu_navigation_list[0])))

enum {
    COMPO_ID_NULL = 0,
    COMPO_ID_LISTBOX,
};

typedef struct f_menu_navigation_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;

} f_menu_navigation_t;


static const compo_listbox_item_t tbl_menu_navigation_list[] = {
    {STR_NAV_ZOOM,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 0},
    {STR_NAV_FADE,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 1},
    {STR_NAV_SHIFT,         .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 2},
    {STR_NAV_FLIP,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 3},
    {STR_NAV_PLATE_FLIP,    .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 4},
    {STR_NAV_CUBE,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 5},
    {STR_NAV_ICUBE,         .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 6},
    {STR_NAV_ROTA,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 7},
    {STR_NAV_FOLD,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 8},
    {STR_NAV_DRIFT,         .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 9},
    {STR_NAV_NONE,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,    .vidx = 10},
};

u16 menu_navigation_ctl_bits;   //控制位

//设置控制位
void func_set_sub_menu_navigation_set_bit(uint n, u8 v)
{
    if (n >= MENU_NAVIGATION_LIST_CNT) {
        halt(HALT_BSP_SYS_CTLBITS);
    }
    int bi = n % 16;
    menu_navigation_ctl_bits &= BIT(bi);   //因为是单选，所以其他控制位都置0
    menu_navigation_ctl_bits = (menu_navigation_ctl_bits & ~BIT(bi)) | (v << bi);
}

//获取控制位
u8 func_set_sub_menu_navigation_get_bit(uint n)
{
    if (n >= MENU_NAVIGATION_LIST_CNT) {
        halt(HALT_BSP_SYS_CTLBITS);
    }
    int bi = n % 16;
    return ((menu_navigation_ctl_bits >> bi) & 1);
}

//反转系统控制位
void func_set_sub_menu_navigation_reverse_bit(uint n)
{
    if (n >= MENU_NAVIGATION_LIST_CNT) {
        halt(HALT_BSP_SYS_CTLBITS);
    }
    int bi = n % 16;
    menu_navigation_ctl_bits &= BIT(bi);   //因为是单选，所以其他控制位都置0
    menu_navigation_ctl_bits ^= BIT(bi);
}

//熄屏设置页面
compo_form_t *func_set_sub_menu_navigation_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_SETTING_MENU_NAVGAITON]);

    //新建菜单列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);
    compo_listbox_set_rect_size(listbox, GUI_SCREEN_WIDTH, 90, 60);
    compo_listbox_set_rect_color(listbox, make_color(29, 29, 29));
    compo_listbox_set_rect_visible(listbox, true);
    compo_listbox_set(listbox, tbl_menu_navigation_list, MENU_NAVIGATION_LIST_CNT);
    compo_listbox_set_item_text(listbox, GUI_SCREEN_WIDTH/2, 40, GUI_SCREEN_WIDTH, 40, true);
    compo_setid(listbox, COMPO_ID_LISTBOX);
    compo_listbox_set_sta_icon(listbox, UI_BUF_COMMON_HOOK_BIN, 0);
    compo_listbox_set_sta_icon_pos(listbox, listbox->item_width - 70, 0);
    compo_listbox_set_bithook(listbox, func_set_sub_menu_navigation_get_bit);
    compo_listbox_set_sta_icon_top(listbox, false);

    compo_listbox_set_item_text(listbox, 80, 25, 380, 40, false);
    compo_listbox_set_focus_byidx(listbox, 1);
    compo_listbox_update(listbox);

    func_set_sub_menu_navigation_set_bit(sys_cb.nav_index, 1);
    return frm;
}

//点进图标进入应用
void func_set_sub_menu_navigation_icon_click(void)
{
    int icon_idx;
    f_menu_navigation_t *f_menu_navigation = (f_menu_navigation_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_menu_navigation->listbox;
//    compo_form_t *frm = NULL;
//    bool res = false;

    icon_idx = compo_listbox_select(listbox, ctp_get_sxy());
    if (icon_idx < 0 || icon_idx >= MENU_NAVIGATION_LIST_CNT) {
        return;
    }

    printf("icon_idx:%d\n", icon_idx);
    sys_cb.nav_index = icon_idx;
    func_set_sub_menu_navigation_reverse_bit(tbl_menu_navigation_list[icon_idx].vidx);
    compo_listbox_update(listbox);
    // u8 func_sta = task_stack_pop();
    // func_switch_to(func_sta, func_get_switching_mode_byidx(sys_cb.nav_index, false) | FUNC_SWITCH_AUTO);
}

//熄屏时长功能事件处理
static void func_set_sub_menu_navigation_process(void)
{
    f_menu_navigation_t *f_set = (f_menu_navigation_t *)func_cb.f_cb;
    compo_listbox_move(f_set->listbox);
    func_process();
}

//熄屏时长功能消息处理
static void func_set_sub_menu_navigation_message(size_msg_t msg)
{
    f_menu_navigation_t *f_set = (f_menu_navigation_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }
    switch (msg) {
    case MSG_CTP_CLICK:
        func_set_sub_menu_navigation_icon_click();                //单击图标
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
void func_set_sub_menu_navigation_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_menu_navigation_t));
    func_cb.frm_main = func_set_sub_menu_navigation_form_create();

    f_menu_navigation_t *f_set = (f_menu_navigation_t *)func_cb.f_cb;
    f_set->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_set->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_set->mcb;//func_zalloc(sizeof(compo_listbox_move_cb_t));        //建立移动控制块，退出时需要释放
    //compo_listbox_move_init(listbox);

    compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, MENU_NAVIGATION_LIST_CNT - 2));
    func_cb.enter_tick = tick_get();
}

//退出设置功能
void func_set_sub_menu_navigation_exit(void)
{
//    f_menu_navigation_t *f_set = (f_menu_navigation_t *)func_cb.f_cb;
//    compo_listbox_t *listbox = f_set->listbox;
//    func_free(listbox->mcb);                                            //释放移动控制块
    func_cb.last = FUNC_SET_SUB_MENU_NAVIGATION;
}

//设置功能
void func_set_sub_menu_navigation(void)
{
    printf("%s\n", __func__);
    func_set_sub_menu_navigation_enter();
    while (func_cb.sta == FUNC_SET_SUB_MENU_NAVIGATION) {
        func_set_sub_menu_navigation_process();
        func_set_sub_menu_navigation_message(msg_dequeue());
    }
    func_set_sub_menu_navigation_exit();
}
