#include "include.h"
#include "func.h"
#include "func_menu.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define FOOTBALL_RADIUS                          195
#define MENU_FOOTBALL_ITEM_CNT                   ((int)(sizeof(tbl_menu_football) / sizeof(tbl_menu_football[0])))
#define KALE_EDGE_SPACE                          2                                                              //边缘距离
#define KALE_ICON_SPACE                          2                                                              //图标距离
#define KALE_ICON_OUTER_SIZE                     (gui_image_get_size(tbl_menu_football[0].res_addr).wid)    //外圈图标大小
#define KALE_ICON_INNER_SIZE                     (KALE_ICON_OUTER_SIZE / 5 * 2)                                 //内圈图标大小
#define KALE_REFRASH_EXPIRE                      4                                                              //刷新间隔 uint：ms
#define KALE_SWITCH_DR                           2                                                              //单次变更R值
#define KALE_ROTATE_ANGLE                        25                                                             //单次旋转角度 范围：1~KALE_CIR_ANGLE_COM

enum {
    COMPO_ID_FOOTBALL = 1,
};

typedef struct f_menu_football_t_ {
    compo_football_t *ball;
} f_menu_football_t;

//足球图标列表(固定20项)
static const compo_football_item_t tbl_menu_football[] = {
    // {UI_BUF_ICON_FOOTBALL_BATTERY_LEVEL_BIN,       FUNC_FLASHLIGHT},
    // {UI_BUF_ICON_FOOTBALL_BLUETOOTH_BIN,              FUNC_MAP},
    // {UI_BUF_ICON_FOOTBALL_BRIGHTNESS__ADJUSTMENT_BIN,         FUNC_BRIGHTNESS},//
    // {UI_BUF_ICON_FOOTBALL_BRIGHTNESS_TIME_BIN,      FUNC_ALARM_CLOCK},
    // {UI_BUF_ICON_FOOTBALL_CALENDAR_BIN,           FUNC_CALENDAER},//

    // {UI_BUF_ICON_FOOTBALL_DELETE_BIN,            FUNC_BT},
    // {UI_BUF_ICON_FOOTBALL_DOWNLOAD_BIN,             FUNC_GAME},
    // {UI_BUF_ICON_FOOTBALL_FLASHLIGHT_BIN,     FUNC_FLASHLIGHT},//
    // {UI_BUF_ICON_FOOTBALL_PERSONAL_STATUS_BIN,         FUNC_CALENDAER},
    // {UI_BUF_ICON_FOOTBALL_SETTING_BIN,           FUNC_NULL},

    // {UI_BUF_ICON_FOOTBALL_TIME_BIN,             FUNC_CALL},
    // {UI_BUF_ICON_FOOTBALL_VOLUME_BIN,          FUNC_ADDRESS_BOOK},
    // {UI_BUF_ICON_FOOTBALL_WEATHER_BIN,            FUNC_SLEEP},

    // {UI_BUF_ICON_FOOTBALL_BATTERY_LEVEL_BIN,       FUNC_CALCULATOR},
    // {UI_BUF_ICON_FOOTBALL_BLUETOOTH_BIN,          FUNC_COMPASS},

    // {UI_BUF_ICON_FOOTBALL_BRIGHTNESS__ADJUSTMENT_BIN,            FUNC_BRIGHTNESS},//
    // {UI_BUF_ICON_FOOTBALL_BRIGHTNESS_TIME_BIN,        FUNC_STOPWATCH},
    // {UI_BUF_ICON_FOOTBALL_CALENDAR_BIN,            FUNC_CALENDAER},//
    // {UI_BUF_ICON_FOOTBALL_DELETE_BIN,       FUNC_HEARTRATE},
    // {UI_BUF_ICON_FOOTBALL_DOWNLOAD_BIN,          FUNC_SETTING},
};

//创建主菜单窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_menu_sub_football_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(false);       //菜单一般创建在底层

//    //创建背景
//    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_ICON_FOOTBALL_BG_BIN);
//    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);

    //创建足球菜单
    compo_football_t *ball = compo_football_create(frm, FOOTBALL_RADIUS, tbl_menu_football, MENU_FOOTBALL_ITEM_CNT);

    compo_setid(ball, COMPO_ID_FOOTBALL);
    //compo_football_update(ball);
    return frm;
}

//点进图标进入应用
static void func_menu_sub_disk_icon_click(void)
{
    f_menu_football_t *f_menu = (f_menu_football_t *)func_cb.f_cb;
    compo_football_t *ball = f_menu->ball;
    point_t pt = ctp_get_sxy();

    int icon_idx = compo_football_get_idx(ball, pt.x, pt.y);
    u8 func_sta;
    if (icon_idx < 0 || icon_idx >= MENU_FOOTBALL_ITEM_CNT) {
        return;
    }
    //根据图标索引获取应用ID
    func_sta = tbl_menu_football[icon_idx].func_sta;

    //切入应用
    if (func_sta > 0) {
        func_switching(FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO, NULL);
        func_cb.sta = func_sta;
        func_cb.menu_idx = icon_idx;                //记住当前编号
    }
}

//切换到时钟
static void func_menu_sub_football_switch_to_clock(void)
{
    u8 func_sta = FUNC_CLOCK;
    func_switching(FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO, NULL);
    func_cb.sta = func_sta;
    func_cb.menu_idx = 0;
    func_cb.flag_animation = true;
}

//主菜单功能事件处理
static void func_menu_sub_football_process(void)
{
    f_menu_football_t *f_menu = (f_menu_football_t *)func_cb.f_cb;
    compo_football_t *ball = f_menu->ball;
    compo_football_move(ball);
    func_process();
}

//拖动过程中，只响应部分消息
static void func_menu_sub_football_drag_message(size_msg_t msg)
{
    switch (msg) {
    case KU_RIGHT:                                      //KEY2 返回表盘
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_menu_sub_football_switch_to_clock();
        }
        break;

    default:
        evt_message(msg);
        break;
    }
}

//自动移动中，只响应部分消息
static void func_menu_sub_football_move_message(size_msg_t msg)
{
    f_menu_football_t *f_menu = (f_menu_football_t *)func_cb.f_cb;
    compo_football_t *ball = f_menu->ball;

    switch (msg) {
    case MSG_CTP_TOUCH:
        //移动过程中，触屏停止。重新进入到开始拖动模式
        compo_football_move_control(ball, COMPO_FOOTBALL_MOVE_CMD_DRAG);
        break;

    case MSG_QDEC_FORWARD:                              //向前滚动菜单
        compo_football_move_control(ball, COMPO_FOOTBALL_MOVE_CMD_FORWARD);
        break;

    case MSG_QDEC_BACKWARD:                             //向后滚动菜单
        compo_football_move_control(ball, COMPO_FOOTBALL_MOVE_CMD_BACKWARD);
        break;

    case KU_RIGHT:                                      //KEY2 返回表盘
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_menu_sub_football_switch_to_clock();
        }
        break;

    default:
        func_menu_sub_message(msg);
        break;
    }
}

//正常模式下
static void func_menu_sub_football_normal_message(size_msg_t msg)
{
    f_menu_football_t *f_menu = (f_menu_football_t *)func_cb.f_cb;
    compo_football_t *ball = f_menu->ball;

    switch (msg) {
    case MSG_CTP_CLICK:
        func_menu_sub_disk_icon_click();                //单击图标
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_DOWN:
        compo_football_move_control(ball, COMPO_FOOTBALL_MOVE_CMD_DRAG);
        break;

    case MSG_QDEC_FORWARD:                              //向前滚动菜单
        compo_football_move_control(ball, COMPO_FOOTBALL_MOVE_CMD_FORWARD);
        break;

    case MSG_QDEC_BACKWARD:                             //向后滚动菜单
        compo_football_move_control(ball, COMPO_FOOTBALL_MOVE_CMD_BACKWARD);
        break;

    case KU_DELAY_BACK:
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_menu_sub_football_switch_to_clock();       //返回时钟表盘界面
        }
        break;

    case KU_RIGHT:                                      //KEY2 返回表盘
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_menu_sub_football_switch_to_clock();
        }
        break;

    default:
        func_menu_sub_message(msg);
        break;
    }
}

//主菜单功能消息处理
static void func_menu_sub_football_message(size_msg_t msg)
{
    f_menu_football_t *f_menu = (f_menu_football_t *)func_cb.f_cb;
    compo_football_t *ball = f_menu->ball;
    u8 sta = compo_football_get_sta(ball);
    switch (sta) {
    case COMPO_FOOTBALL_STA_IDLE:
        //正常模式下
        func_menu_sub_football_normal_message(msg);
        break;

    case COMPO_FOOTBALL_STA_DARG:
        //拖动过程中，只响应部分消息
        func_menu_sub_football_drag_message(msg);
        break;

    case COMPO_FOOTBALL_STA_MOVE:
        //自动移动中，只响应部分消息
        func_menu_sub_football_move_message(msg);
        break;

    default:
        halt(HALT_GUI_COMPO_FOOTBALL_MOVE_STA);
        break;
    }
}

//进入主菜单功能
static void func_menu_sub_football_enter(void)
{

    func_cb.f_cb = func_zalloc(sizeof(f_menu_football_t));
    func_cb.frm_main = func_menu_sub_football_form_create();
    //tft_set_temode(0);
    f_menu_football_t *f_menu = (f_menu_football_t *)func_cb.f_cb;
    f_menu->ball = compo_getobj_byid(COMPO_ID_FOOTBALL);
    compo_football_t *ball = f_menu->ball;
    compo_football_move_control(ball, COMPO_FOOTBALL_MOVE_CMD_ENTER);
    if (ball->type != COMPO_TYPE_FOOTBALL) {
        halt(HALT_GUI_COMPO_FOOTBALL_TYPE);
    }
    func_cb.enter_tick = tick_get();
}

//主菜单功能
void func_menu_sub_football(void)
{
    printf("%s\n", __func__);
    func_menu_sub_football_enter();
    while (func_cb.sta == FUNC_MENU && func_cb.menu_style == MENU_STYLE_FOOTBALL) {
        func_menu_sub_football_process();
        func_menu_sub_football_message(msg_dequeue());
    }
    func_menu_sub_exit();
}
