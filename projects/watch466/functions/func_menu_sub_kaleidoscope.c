#include "include.h"
#include "func.h"
#include "func_menu.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define MENU_KALE_CNT                            ((int)(sizeof(tbl_menu_kaleidoscope) / sizeof(tbl_menu_kaleidoscope[0])))
#define KALE_EDGE_SPACE                          2                                                              //边缘距离
#define KALE_ICON_SPACE                          2                                                              //图标距离
#define KALE_ICON_OUTER_SIZE                     (gui_image_get_size(tbl_menu_kaleidoscope[0].res_addr).wid)    //外圈图标大小
#define KALE_ICON_INNER_SIZE                     (KALE_ICON_OUTER_SIZE / 5 * 2)                                 //内圈图标大小
#define KALE_REFRASH_EXPIRE                      4                                                              //刷新间隔 uint：ms
#define KALE_SWITCH_DR                           2                                                              //单次变更R值
#define KALE_ROTATE_ANGLE                        25                                                             //单次旋转角度 范围：1~KALE_CIR_ANGLE_COM

enum {
    COMPO_ID_KALEIDOSCOPE = 1,
};

typedef struct f_menu_kale_t_ {
    compo_kaleidoscope_t *kale;
    bool flag_drag;                 //开始拖动
    bool flag_move_auto;            //自动移到坐标
    s32 dx;
    s32 dy;
    size_msg_t msg;
} f_menu_kale_t;

//万花筒图标列表及顺序
static const menu_kale_item_t tbl_menu_kaleidoscope[] = {
    {UI_BUF_ICON_CLOCK_BG_BIN,                  FUNC_CLOCK},
    {UI_BUF_ICON_HEART_RATE_BIN,                FUNC_HEARTRATE},
    {UI_BUF_ICON_ACTIVITY_BIN,                  FUNC_ACTIVITY},
    {UI_BUF_ICON_BLOOD_OXYGEN_BIN,              FUNC_BLOOD_OXYGEN},
    {UI_BUF_ICON_BLOODSUGAR_BIN,                FUNC_BLOODSUGAR},
    {UI_BUF_ICON_BREATHE_BIN,                   FUNC_BREATHE},
    {UI_BUF_ICON_MUSIC_BIN,                     FUNC_BT},
    {UI_BUF_ICON_CALL_BIN,                      FUNC_CALL},

    {UI_BUF_ICON_MENU_BIN,                      FUNC_STYLE},
    {UI_BUF_ICON_FLASHLIGHT_BIN,                FUNC_FLASHLIGHT},
    {UI_BUF_ICON_FINDPHONE_BIN,                 FUNC_FINDPHONE},
    {UI_BUF_ICON_GAME_BIN,                      FUNC_GAME},
    {UI_BUF_ICON_ALTITUDE_BIN,                  FUNC_ALTITUDE},
    {UI_BUF_ICON_ALARM_CLOCK_BIN,               FUNC_ALARM_CLOCK},
    {UI_BUF_ICON_RESTORE_FACTORY_BIN,           FUNC_RSTFY},
    {UI_BUF_ICON_LANGUAGE_BIN,                  FUNC_LANGUAGE},

    {UI_BUF_ICON_LIGHT_BIN,                     FUNC_LIGHT},
    {UI_BUF_ICON_MAP_BIN,                       FUNC_MAP},
    {UI_BUF_ICON_MESSAGE_BIN,                   FUNC_MESSAGE},
    {UI_BUF_ICON_CALCULATOR_BIN,                FUNC_CALCULATOR},
    {UI_BUF_ICON_OFF_BIN,                       FUNC_OFF},
    {UI_BUF_ICON_RESTART_BIN,                   FUNC_RESTART},
    {UI_BUF_ICON_CALENDAR_BIN,                  FUNC_CALENDAER},
    {UI_BUF_ICON_SCAN_BIN,                      FUNC_SCAN},

    {UI_BUF_ICON_SETTING_BIN,                   FUNC_SETTING},
    {UI_BUF_ICON_SLEEP_BIN,                     FUNC_SLEEP},
    {UI_BUF_ICON_VOLUME_BIN,                    FUNC_VOLUME},
    {UI_BUF_ICON_SPORT_BIN,                     FUNC_SPORT},
    {UI_BUF_ICON_STOPWATCH_BIN,                 FUNC_STOPWATCH},
    {UI_BUF_ICON_CAMERA_BIN,                    FUNC_CAMERA},
    {UI_BUF_ICON_TIMER_BIN,                     FUNC_TIMER},
    {UI_BUF_ICON_ADDRESS_BOOK_BIN,              FUNC_ADDRESS_BOOK},
};

//创建主菜单窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_menu_sub_kale_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(false);       //菜单一般创建在底层

    //新建菜单
    compo_kaleidoscope_t *kale = compo_kaleidoscope_create(frm);
    compo_kaleidoscope_init(kale, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT, KALE_ICON_OUTER_SIZE, KALE_ICON_INNER_SIZE, KALE_EDGE_SPACE, KALE_ICON_SPACE);
    compo_kaleidoscope_icon_add(kale, tbl_menu_kaleidoscope, MENU_KALE_CNT);

	compo_kale_add_time(kale, COMPO_RINGS_TIME_TYPE_HOUR, UI_BUF_ICON_CLOCK_H_BIN, 1, 2, KALE_ICON_OUTER_SIZE);
    compo_kale_add_time(kale, COMPO_RINGS_TIME_TYPE_MIN, UI_BUF_ICON_CLOCK_M_BIN, 1, 2, KALE_ICON_OUTER_SIZE);
    compo_kale_add_time(kale, COMPO_RINGS_TIME_TYPE_SEC, UI_BUF_ICON_CLOCK_S_BIN, 9, 2, KALE_ICON_OUTER_SIZE);
    compo_kale_set_start_angle(kale, 900);

    compo_setid(kale, COMPO_ID_KALEIDOSCOPE);
    compo_kale_set_focus_byidx(kale, func_cb.menu_idx);
    compo_kale_update(kale);

    return frm;
}

//点进图标进入应用
static void func_menu_sub_disk_icon_click(void)
{
    int icon_idx;
    f_menu_kale_t *f_menu = (f_menu_kale_t *)func_cb.f_cb;
    compo_kaleidoscope_t *kale = f_menu->kale;
    u8 func_sta;

    icon_idx = compo_kale_select(kale, ctp_get_sxy().x, ctp_get_sxy().y);
    if (icon_idx < 0 || icon_idx >= MENU_KALE_CNT) {
        return;
    }

    //根据图标索引获取应用ID
    func_sta = tbl_menu_kaleidoscope[icon_idx].func_sta;

    //切入应用
    if (func_sta > 0) {
        compo_form_t *frm = func_create_form(func_sta);
        func_switching(FUNC_SWITCH_ZOOM_ENTER | FUNC_SWITCH_AUTO, kale->sel_icon);
        compo_form_destroy(frm);
        func_cb.sta = func_sta;
        func_cb.menu_idx = icon_idx;                //记住当前编号
    }
}

//切换到时钟
static void func_menu_sub_kale_switch_to_clock(void)
{
    f_menu_kale_t *f_menu = (f_menu_kale_t *)func_cb.f_cb;
    compo_kaleidoscope_t *kale = f_menu->kale;
    widget_icon_t *icon = compo_kale_select_byidx(kale, 0);
    u8 func_sta = FUNC_CLOCK;
    compo_form_t *frm = func_create_form(func_sta);
    func_switching(FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO, icon);
    compo_form_destroy(frm);
    func_cb.sta = func_sta;
    func_cb.menu_idx = 0;
}

//主菜单功能事件处理
static void func_menu_sub_kale_process(void)
{
    f_menu_kale_t *f_menu = (f_menu_kale_t *)func_cb.f_cb;
    compo_kaleidoscope_t *kale = f_menu->kale;

    if (f_menu->flag_drag) {
        f_menu->flag_drag = ctp_get_dxy(&f_menu->dx, &f_menu->dy);
        if (!f_menu->flag_drag) {
            compo_kaleidoscope_ctl_switch_kick(kale, f_menu->msg);
        }
    }

    compo_kaleidoscope_switch_animation(kale, KALE_REFRASH_EXPIRE, KALE_SWITCH_DR, KALE_ROTATE_ANGLE);
    func_process();
}

//拖动过程中，只响应部分消息
static void func_menu_sub_kale_drag_message(size_msg_t msg)
{
    switch (msg) {
    default:
        evt_message(msg);
        break;
    }
}

//正常模式下
static void func_menu_sub_kale_normal_message(size_msg_t msg)
{
    f_menu_kale_t *f_menu = (f_menu_kale_t *)func_cb.f_cb;
//   compo_kaleidoscope_t *kale = f_menu->kale;

    switch (msg) {
    case MSG_CTP_CLICK:
        func_menu_sub_disk_icon_click();                //单击图标
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_DOWN:
    case MSG_QDEC_FORWARD:                              //向前滚动菜单
    case MSG_QDEC_BACKWARD:                             //向后滚动菜单
        f_menu->flag_drag = true;                       //任何方向短划，开启拖动
        f_menu->msg = msg;
        break;

    case KU_DELAY_BACK:
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_menu_sub_kale_switch_to_clock();       //返回时钟表盘界面
        }
        break;

    default:
        func_menu_sub_message(msg);
        break;
    }
}

//主菜单功能消息处理
static void func_menu_sub_kale_message(size_msg_t msg)
{
    f_menu_kale_t *f_menu = (f_menu_kale_t *)func_cb.f_cb;
    if (f_menu->flag_drag) {
        //拖动过程中，只响应部分消息
        func_menu_sub_kale_drag_message(msg);
    } else if (f_menu->flag_move_auto) {
        // //自动移动中，只响应部分消息
        // func_menu_sub_kale_move_auto_message(msg);
    } else {
        //正常模式下
        func_menu_sub_kale_normal_message(msg);
    }
}

//进入主菜单功能
static void func_menu_sub_kale_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_menu_kale_t));
    func_cb.frm_main = func_menu_sub_kale_form_create();

    f_menu_kale_t *f_menu = (f_menu_kale_t *)func_cb.f_cb;
    f_menu->kale = compo_getobj_byid(COMPO_ID_KALEIDOSCOPE);
    compo_kaleidoscope_t *kale = f_menu->kale;

    compo_kaleidoscope_adm_animation_kick(kale);//入场动画KICK

    if (kale->type != COMPO_TYPE_KALEIDOSCOPE) {
        halt(HALT_GUI_COMPO_ICONLIST_TYPE);
    }

    func_cb.enter_tick = tick_get();
}

//主菜单功能
void func_menu_sub_kale(void)
{
    printf("%s\n", __func__);
    func_menu_sub_kale_enter();
    while (func_cb.sta == FUNC_MENU && func_cb.menu_style == MENU_STYLE_KALE) {
        func_menu_sub_kale_process();
        func_menu_sub_kale_message(msg_dequeue());
    }

    func_menu_sub_exit();
}
