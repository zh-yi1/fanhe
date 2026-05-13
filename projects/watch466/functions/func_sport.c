#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define MENU_LIST_CNT                       ((int)(sizeof(tbl_sport_list) / sizeof(tbl_sport_list[0])))

enum {
    COMPO_ID_LISTBOX = 1,
};

typedef struct f_sport_list_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;
} f_sport_list_t;

static const compo_listbox_item_t tbl_sport_list[] = {
    {STR_INDOOR_RUN,            UI_BUF_SPORT_BEFORE_EXERCISE_INDOOR_RUNNING_BIN,     .func_sta = FUNC_SPORT_SWITCH},                 //运动
    {STR_RUN,                   UI_BUF_SPORT_BEFORE_EXERCISE_RUN_BIN,                .func_sta = FUNC_SPORT_SWITCH},
    {STR_FOOTBALL,              UI_BUF_SPORT_BEFORE_EXERCISE_FOOTBALL_BIN,           .func_sta = FUNC_SPORT_SWITCH},                 //运动
    {STR_INDOOR_CYCLING,        UI_BUF_SPORT_BEFORE_EXERCISE_INDOOR_CYCLING_BIN,     .func_sta = FUNC_SPORT_SWITCH},                 //运动
    {STR_OUTDOOR_CYCLING,       UI_BUF_SPORT_BEFORE_EXERCISE_OUTDOOR_CYCLING_BIN,    .func_sta = FUNC_SPORT_SWITCH},                 //运动
    {STR_SWIM,                  UI_BUF_SPORT_BEFORE_EXERCISE_SWIM_BIN,               .func_sta = FUNC_SPORT_SWITCH},                 //运动
    {STR_WALK,                  UI_BUF_SPORT_BEFORE_EXERCISE_WALK_BIN,               .func_sta = FUNC_SPORT_SWITCH},
};

u8 func_sport_get_str_idx()
{
    return tbl_sport_list[sys_cb.sport_idx].str_idx;
}

//创建运动窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_sport_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_SPORTS]);

    //新建菜单列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_MENU_NORMAL);
    area_t area = gui_image_get_size(UI_BUF_SPORT_BEFORE_EXERCISE_BG_BIN);
    compo_listbox_set_item_width(listbox, area.wid);
    compo_listbox_set(listbox, tbl_sport_list, MENU_LIST_CNT);
    compo_listbox_set_bgimg(listbox, UI_BUF_SPORT_BEFORE_EXERCISE_BG_BIN);
    compo_setid(listbox, COMPO_ID_LISTBOX);

    compo_listbox_set_focus(listbox, 140);
    compo_listbox_update(listbox);

    return frm;
}

//点进图标进入应用
static void func_sport_list_icon_click(void)
{
    int icon_idx;
    f_sport_list_t *f_sport = (f_sport_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_sport->listbox;
    u8 func_sta;

    icon_idx = compo_listbox_select(listbox, ctp_get_sxy());
    if (icon_idx < 0 || icon_idx >= MENU_LIST_CNT) {
        return;
    }

    //根据图标索引获取应用ID
    func_sta = tbl_sport_list[icon_idx].func_sta;

    //切入应用
    if (func_sta > 0 && !listbox->flag_area) {
        compo_form_t *frm = func_create_form(func_sta);
        func_switching(func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO, listbox->sel_icon);
        compo_form_destroy(frm);
        func_cb.sta = func_sta;
        sys_cb.sport_idx = listbox->focus_icon_idx;                //记住当前编号
        printf("===>> sys_cb.sport_idx=%d\n", sys_cb.sport_idx);
    }

    if (listbox->flag_area) {
        func_cb.sta = FUNC_SPORT_CONFIG;
        listbox->flag_area = 0;
    }
}

//运动功能事件处理
static void func_sport_process(void)
{
    f_sport_list_t *f_sport = (f_sport_list_t *)func_cb.f_cb;
    compo_listbox_move(f_sport->listbox);
    func_process();
}

//运动功能消息处理
static void func_sport_message(size_msg_t msg)
{
    f_sport_list_t *f_sport = (f_sport_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_sport->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }
    switch (msg) {
    case MSG_CTP_CLICK:
        func_sport_list_icon_click();                //单击图标
        break;

    case MSG_CTP_LONG:
        break;

    case MSG_CTP_SHORT_RIGHT:
        func_message(msg);
        sys_cb.sport_idx = 0;
        break;

    case KU_DELAY_BACK:
//        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
//            func_menu_sub_list_switch_to_clock();       //返回时钟表盘界面
//        }
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入运动功能
void func_sport_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_sport_list_t));
    func_cb.frm_main = func_sport_form_create();

    f_sport_list_t *f_sport = (f_sport_list_t *)func_cb.f_cb;
    f_sport->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_sport->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_sport->mcb;
    compo_listbox_move_init_modify(listbox, listbox->ofs_y, compo_listbox_gety_byidx(listbox, MENU_LIST_CNT - 2));
    func_cb.enter_tick = tick_get();
}

//退出运动功能
void func_sport_exit(void)
{
    func_cb.last = FUNC_SPORT;
}

//运动功能
void func_sport(void)
{
    printf("%s\n", __func__);
    func_sport_enter();
    while (func_cb.sta == FUNC_SPORT) {
        func_sport_process();
        func_sport_message(msg_dequeue());
    }
    func_sport_exit();
}
