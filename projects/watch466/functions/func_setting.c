#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define SET_LIST_CNT                       ((int)(sizeof(tbl_setting_list) / sizeof(tbl_setting_list[0])))

enum {
    COMPO_ID_LISTBOX = 1,
    COMPO_ID_SCROLLBAR,
};

typedef struct f_set_list_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;

} f_set_list_t;

static const compo_listbox_item_t tbl_setting_list[] = {
#if AVI_DVP_DEMOLIST
    {STR_VIDEO_MODE,                    UI_BUF_SETTING_ABOUT_BIN,                       .func_sta = FUNC_VIDEO_SHOWLIST,            .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //3D效果内容演示
#endif // AVI_DVP_DEMOLIST
    {STR_SETTING_LIGHT,                  UI_BUF_SETTING_LIGHT_BIN,                      .func_sta = FUNC_LIGHT,                     .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //亮度调节
    {STR_SETTING_MENU_NAVGAITON,         UI_BUF_SETTING_ABOUT_BIN,                      .func_sta = FUNC_SET_SUB_MENU_NAVIGATION,   .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //亮度调节
    //{STR_SETTING_DOUSING,                UI_BUF_SETTING_BREATHING_SCREEN_CLICK_BIN,     .func_sta = FUNC_SET_SUB_DOUSING,         .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //息屏时长
    {STR_SETTING_DOUSING,                UI_BUF_SETTING_BREATHING_SCREEN_BIN,           .func_sta = FUNC_SET_SUB_DOUSING,           .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //息屏时长
    {STR_SETTING_UP,                     UI_BUF_SETTING_WRIST_BIN,                      .func_sta = FUNC_SET_SUB_WRIST,             .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //抬腕亮屏
    //{STR_SETTING_DISTURD,                UI_BUF_SETTING_DISTURB_CLICK_BIN,              .func_sta = FUNC_SET_SUB_DISTURD,         .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //勿扰模式
    {STR_SETTING_DISTURD,                UI_BUF_SETTING_DISTURB_BIN,                    .func_sta = FUNC_SET_SUB_DISTURD,           .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //勿扰模式
    {STR_SETTING_SAV,                    UI_BUF_SETTING_SOUND_AND_VIBRATION_BIN,        .func_sta = FUNC_SET_SUB_SAV,               .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //声音与振动
    {STR_SETTING_LANGUAGE,               UI_BUF_SETTING_LANGUAGE_BIN,                   .func_sta = FUNC_SET_SUB_LANGUAGE,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //语言设置
    {STR_SETTING_TIME,                   UI_BUF_SETTING_TIME_BIN,                       .func_sta = FUNC_SET_SUB_TIME,              .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //时间设置
    {STR_SETTING_CALENDAR,               UI_BUF_SETTING_CALENDAR_BIN,                   .func_sta = FUNC_CALENDAER,                 .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //日期设置
    {STR_SETTING_PASSWORD,               UI_BUF_SETTING_PASSWORD_BIN,                   .func_sta = FUNC_SET_SUB_PASSWORD,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //密码锁
    {STR_SETTING_ABOUT,                  UI_BUF_SETTING_ABOUT_BIN,                      .func_sta = FUNC_SET_SUB_ABOUT,             .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //关于
    {STR_SETTING_4G,                     UI_BUF_SETTING_ABOUT_BIN,                      .func_sta = FUNC_SET_SUB_4G,                .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //4G
    {STR_SETTING_RESTART,                UI_BUF_SETTING_RESTART_BIN,                    .func_sta = FUNC_SET_SUB_RESTART,           .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //重启
    {STR_SETTING_RSTFY,                  UI_BUF_SETTING_RESTORE_FACTORY_BIN,            .func_sta = FUNC_SET_SUB_RSTFY,             .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //恢复出厂
    {STR_SETTING_OFF,                    UI_BUF_SETTING_OFF_BIN,                        .func_sta = FUNC_SET_SUB_OFF,               .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //关机
#if FUNC_MUSIC_EN
    {STR_SD_MUSIC,                      UI_BUF_SETTING_ABOUT_BIN,                       .func_sta = FUNC_MUSIC,                     .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},              //3D效果内容演示
#endif // FUNC_MUSIC_EN
#if FUNC_RECORDER_EN
    {STR_MIC_RECORD,                    UI_BUF_SETTING_ABOUT_BIN,                       .func_sta = FUNC_RECORDER,                  .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH,},
#endif // FUNC_RECORDER_EN
};

u8 func_setting_get_bit(uint n)
{
    return 0;
}

//创建主菜单窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_set_sub_list_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //新建菜单列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_MENU_CIRCLE);
    compo_listbox_set_rect_size(listbox, GUI_SCREEN_WIDTH, 134, 70);
    compo_listbox_set_rect_color(listbox, make_color(29, 29, 29));
    compo_listbox_set_rect_visible(listbox, true);
    compo_listbox_set_sta_icon_res(listbox, UI_BUF_COMMON_OPEN_BIN, UI_BUF_COMMON_OPEN_BIN);
    compo_listbox_set(listbox, tbl_setting_list, SET_LIST_CNT);
    compo_listbox_set_sta_icon_pos(listbox, listbox->item_width - 30, 0);
    compo_listbox_set_bithook(listbox, func_setting_get_bit);
    compo_setid(listbox, COMPO_ID_LISTBOX);

    u8 set_idx = sys_cb.set_idx;
    if (set_idx < 1) {
        set_idx = 1;
    }

    compo_listbox_set_focus_byidx(listbox, set_idx);
    compo_listbox_update(listbox);

    // compo_scroll_t* scroll = compo_scroll_create(frm, SCROLL_TYPE_ARC);
    // compo_setid(scroll, COMPO_ID_SCROLLBAR);
    // compo_scroll_set_w_r(scroll, 6, GUI_SCREEN_CENTER_X);
    // compo_scroll_set_color(scroll, make_color(29,29,29), COLOR_BLACK, COLOR_BLACK);
    // compo_scroll_set_range(scroll, 600, 1200);
    // compo_scroll_set_pos(scroll, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    // compo_scroll_set_value(scroll, 0);
    // compo_scroll_set_edge_circle(scroll, true);

    return frm;
}

//点进图标进入应用
static void func_set_sub_list_icon_click(void)
{
    int icon_idx;
    f_set_list_t *f_set = (f_set_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;
    u8 func_sta;

    icon_idx = compo_listbox_select(listbox, ctp_get_sxy());
    if (icon_idx < 0 || icon_idx >= SET_LIST_CNT) {
        return;
    }

    //根据图标索引获取应用ID
    func_sta = tbl_setting_list[icon_idx].func_sta;
    //切入应用
    if (func_sta > 0) {
        if (sys_cb.nav_index == NAV_DRIFT) {
            func_switch_to(func_sta, func_get_switching_mode_byidx(sys_cb.nav_index, false) | FUNC_SWITCH_AUTO);
        } else {
            func_switch_to(func_sta, func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO);
        }
        sys_cb.set_idx = listbox->focus_icon_idx;
    }
}

//切换到设置菜单页面
static void func_set_sub_list_switch_to_clock(void)
{
    u8 func_sta = FUNC_CLOCK;
    f_set_list_t *f_set = (f_set_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;
    widget_icon_t *icon = compo_listbox_select_byidx(listbox, 0);
    compo_form_t *frm = func_create_form(func_sta);
    func_switching(FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO, icon);
    compo_form_destroy(frm);
    func_cb.sta = func_sta;
    sys_cb.set_idx = 0;
}

//主菜单功能事件处理
static void func_set_sub_list_process(void)
{
    f_set_list_t *f_set = (f_set_list_t *)func_cb.f_cb;
    // int value = (f_set->listbox->ofs_y - f_set->listbox->mcb->first_y) * 1000 / abs_s(f_set->listbox->mcb->last_y - f_set->listbox->mcb->first_y);
    // //printf("value = %d, ofs_y=%d, first_y=%d, last_y=%d\n", value, f_menu->listbox->ofs_y,
    // //                f_menu->listbox->mcb->first_y, f_menu->listbox->mcb->last_y);
    // compo_scroll_t* scroll = compo_getobj_byid(COMPO_ID_SCROLLBAR);
    // compo_scroll_set_value(scroll, value);
    compo_listbox_move(f_set->listbox);
    func_process();
}

//进入设置主菜单页面
void func_set_sub_list_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_set_list_t));
    func_cb.frm_main = func_set_sub_list_form_create();

    f_set_list_t *f_set = (f_set_list_t *)func_cb.f_cb;
    f_set->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_set->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_set->mcb   ;//func_zalloc(sizeof(compo_listbox_move_cb_t));        //建立移动控制块，退出时需要释放
    printf("func_set_sub_enter:%x\r\n", listbox->mcb);

    compo_listbox_move_init_modify(listbox, compo_listbox_gety_byidx(listbox, 1), compo_listbox_gety_byidx(listbox, SET_LIST_CNT - 2));
    func_cb.enter_tick = tick_get();
}

//主菜单功能消息处理
static void func_set_sub_list_message(size_msg_t msg)
{
    f_set_list_t *f_set = (f_set_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }
    switch (msg) {
    case MSG_CTP_CLICK:
        func_set_sub_list_icon_click();                //单击图标
        break;

    case MSG_CTP_LONG:
        break;

    case MSG_CTP_SHORT_RIGHT:
        func_message(msg);
        sys_cb.set_idx = 0;
        break;

    case KU_DELAY_BACK:
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_set_sub_list_switch_to_clock();       //返回设置界面
        }
        break;

    default:
        func_message(msg);
        break;
    }
}

//退出菜单样式
void func_set_sub_exit(void)
{

//    f_set_list_t *f_set = (f_set_list_t *)func_cb.f_cb;

//    compo_listbox_t *listbox = f_set->listbox;
//    printf("func_set_sub_exit:%x\r\n", listbox->mcb);
//    func_free(listbox->mcb);                                            //释放移动控制块
    func_cb.last = FUNC_SETTING;
}

//主菜单功能
void func_set_sub_list(void)
{
    func_set_sub_list_enter();
    while (func_cb.sta == FUNC_SETTING) {
        func_set_sub_list_process();
        func_set_sub_list_message(msg_dequeue());
    }
    func_set_sub_exit();
}
