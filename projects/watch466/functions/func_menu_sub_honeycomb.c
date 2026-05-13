#include "include.h"
#include "func.h"
#include "func_menu.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define FOCUS_AUTO_TICK_EXPIRE              18                  //松手后自动对齐焦点单位时间(ms)
#define FOCUS_AUTO_STEP                     12                  //松手后自动对齐焦点单位时间步进
#define FOCUS_AUTO_STEP_DIV                 8

#define MENU_HC_ZOOMOUT_TICK_EXPIRE         18                  //缩放单位时间
#define MENU_HC_ZOOMOUT_STEP                2                   //缩放步进
#define MENU_HC_ZOOMOUT_STEP_DIV            6
#define MENU_HC_ZOOM_CANCEL_TICK_EXPIRE     300                 //超时取消缩放

#define DRAG_AUTO_SPEED                     16                  //拖动松手后的速度(惯性)

#define MENU_HC_CNT                         ((int)((sizeof(tbl_menu_honeycomb) / sizeof(tbl_menu_honeycomb[0]))))

enum {
    COMPO_ID_ICONLIST = 1,
};

typedef struct f_menu_hc_t_ {
    compo_iconlist_t *iconlist;
    bool flag_drag;                 //开始拖动
    bool flag_move_auto;            //自动移到坐标
    u32 tick;
    s32 focus_x;
    s32 focus_y;
    s32 focus_dx;
    s32 focus_dy;
    s32 focus_xstep;
    s32 focus_ystep;
    int moveto_ix;                  //设定自动移到的目标菜单ix
    int moveto_iy;                  //设置自动移到的目标菜单iy
    point_t moveto;                 //设定自动移到的坐标
    u16 icon_size;
    s16 zoom_cur;
    s16 zoom_target;
    s16 zoom_min;
    s16 zoomin_threshold;
    s16 zoomout_threshold;
    s16 zoom_out;
    bool flag_zoomout;
    bool flag_zooming;
} f_menu_hc_t;

typedef struct menu_hc_item_t_ {
    u32 res_addr;
    u8 func_sta;
} menu_hc_item_t;

//蜂窝图标列表及顺序
const menu_hc_item_t tbl_menu_honeycomb[] = {
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
    {UI_BUF_ICON_VOICE_BIN,                     FUNC_VOICE},
    {UI_BUF_ICON_WEATHER_BIN,                   FUNC_WEATHER},
    {UI_BUF_ICON_BLOOD_PRESSURE_BIN,            FUNC_BLOOD_PRESSURE},
#if SECURITY_PAY_EN
    {UI_BUF_ICON_ALIPAY_BIN,                    FUNC_ALIPAY},
#endif // SECURITY_PAY_EN
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
};


//创建主菜单窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_menu_sub_honeycomb_form_create(void)
{
    compo_iconlist_t *iconlist = NULL;
    //新建窗体
    compo_form_t *frm = compo_form_create(false);       //菜单一般创建在底层

    //新建蜂窝效果
    if (func_cb.menu_style == MENU_STYLE_HONEYCOMB) {
        iconlist = compo_iconlist_create(frm, ICONLIST_STYLE_HONEYCOMB);
    } else {
        iconlist = compo_iconlist_create(frm, ICONLIST_STYLE_CUM_HONEYGRID);
    }

    compo_setid(iconlist, COMPO_ID_ICONLIST);
    for (int i=0; i<MENU_HC_CNT; i++) {
        compo_iconlist_add(iconlist, tbl_menu_honeycomb[i].res_addr);
    }
    compo_iconlist_add_time(iconlist, COMPO_ICONLIST_TIME_TYPE_HOUR, UI_BUF_ICON_CLOCK_H_BIN, 1, 2);
    compo_iconlist_add_time(iconlist, COMPO_ICONLIST_TIME_TYPE_MIN, UI_BUF_ICON_CLOCK_M_BIN, 1, 2);
    compo_iconlist_add_time(iconlist, COMPO_ICONLIST_TIME_TYPE_SEC, UI_BUF_ICON_CLOCK_S_BIN, 9, 2);
    compo_iconlist_set_start_angle(iconlist, 900);

    compo_iconlist_set_focus_byidx(iconlist, func_cb.menu_idx);
    compo_iconlist_update(iconlist);
    return frm;
}

//点进图标进入应用
static void func_menu_sub_honeycomb_icon_click(void)
{
    point_t pt;
    int icon_idx;
    f_menu_hc_t *f_menu = (f_menu_hc_t *)func_cb.f_cb;
    compo_iconlist_t *iconlist = f_menu->iconlist;
    u8 func_sta;

    pt = ctp_get_sxy();
    icon_idx = compo_iconlist_select(iconlist, pt.x, pt.y);
    if (icon_idx < 0 || icon_idx >= MENU_HC_CNT) {
        return;
    }

    if (f_menu->flag_zoomout) {
        compo_iconlist_set_focus_byidx(iconlist, icon_idx);
        compo_iconlist_update(iconlist);
        f_menu->flag_zooming = true;
        f_menu->zoom_target = 0;
    } else {
        //根据图标索引获取应用ID
        func_sta = tbl_menu_honeycomb[icon_idx].func_sta;

        //切入应用
        if (func_sta > 0) {
            compo_form_t *frm = func_create_form(func_sta);
            printf("func_sta:%d\n", func_sta);
#if SECURITY_PAY_EN
            if (func_sta == FUNC_ALIPAY) {
                func_switching(FUNC_SWITCH_DIRECT, iconlist->sel_icon);
            } else
#endif
            {
                func_switching(FUNC_SWITCH_ZOOM_ENTER | FUNC_SWITCH_AUTO, iconlist->sel_icon);
            }
            compo_form_destroy(frm);
            func_cb.sta = func_sta;
            func_cb.menu_idx = icon_idx;                //记住当前编号
        }
    }
}

//切换进入应用
static void func_menu_sub_honeycomb_switching_in(void)
{
    f_menu_hc_t *f_menu = (f_menu_hc_t *)func_cb.f_cb;
    compo_iconlist_t *iconlist = f_menu->iconlist;
    int icon_idx = iconlist->focus_icon_idx;
    if (icon_idx < 0 || icon_idx >= MENU_HC_CNT) {
        return;
    }

    //根据图标索引获取应用ID
    u8 func_sta = tbl_menu_honeycomb[icon_idx].func_sta;

    //切入应用
    if (func_sta > 0) {
        compo_form_t *frm = func_create_form(func_sta);
        bool res = func_switching(FUNC_SWITCH_ZOOM_ENTER, iconlist->focus_icon);
        compo_form_destroy(frm);
        if (res) {
            func_cb.sta = func_sta;
            func_cb.menu_idx = icon_idx;            //记住当前编号
        }
    }
}

//图标放大回到普通模式
static void func_menu_sub_honeycomb_zooming_in()
{
    f_menu_hc_t *f_menu = (f_menu_hc_t *)func_cb.f_cb;
    f_menu->flag_zooming = true;
    f_menu->zoom_target += MENU_HC_ZOOMOUT_STEP * MENU_HC_ZOOMOUT_STEP_DIV;
    if (f_menu->zoom_target > 0) {
        f_menu->zoom_target = 0;
        f_menu->tick = tick_get();
    }
}

//图标缩小到平铺模式
static void func_menu_sub_honeycomb_zooming_out()
{
    f_menu_hc_t *f_menu = (f_menu_hc_t *)func_cb.f_cb;
    f_menu->flag_zooming = true;
    f_menu->zoom_target -= MENU_HC_ZOOMOUT_STEP * MENU_HC_ZOOMOUT_STEP_DIV;
    if (f_menu->zoom_target < f_menu->zoom_min) {
        f_menu->zoom_target = f_menu->zoom_min;
        f_menu->tick = tick_get();
    }
}

//切换到时钟
static void func_menu_sub_honeycomb_switch_to_clock(void)
{
    f_menu_hc_t *f_menu = (f_menu_hc_t *)func_cb.f_cb;
    compo_iconlist_t *iconlist = f_menu->iconlist;
    widget_icon_t *icon = compo_iconlist_select_byidx(iconlist, 0);
    rect_t rect = widget_get_location(icon);
    u8 func_sta = FUNC_CLOCK;
    int switch_mode = FUNC_SWITCH_ZOOM_ENTER | FUNC_SWITCH_AUTO;
    compo_form_t *frm = func_create_form(func_sta);
    if (rect.wid < 10) {
        switch_mode = FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO;
    }
    func_switching(switch_mode, icon);
    compo_form_destroy(frm);
    func_cb.sta = func_sta;
    func_cb.menu_idx = 0;
}

//主菜单功能事件处理
static void func_menu_sub_honeycomb_process(void)
{
    f_menu_hc_t *f_menu = (f_menu_hc_t *)func_cb.f_cb;
    compo_iconlist_t *iconlist = f_menu->iconlist;
    if (f_menu->flag_drag) {
        s32 dx = f_menu->focus_dx;
        s32 dy = f_menu->focus_dy;
        f_menu->flag_drag = ctp_get_dxy(&f_menu->focus_dx, &f_menu->focus_dy);
        if (f_menu->flag_drag) {
            //拖动菜单图标
            f_menu->focus_xstep = f_menu->focus_dx - dx;
            f_menu->focus_ystep = f_menu->focus_dy - dy;
            compo_iconlist_set_focus(iconlist, f_menu->focus_x - f_menu->focus_dx, f_menu->focus_y - f_menu->focus_dy);
            compo_iconlist_update(iconlist);
        } else if (f_menu->flag_zoomout) {
            //平铺视图下抬手后开始自动居中
            f_menu->focus_x = iconlist->ofs_x;
            f_menu->focus_y = iconlist->ofs_y;
            f_menu->flag_move_auto = true;
            f_menu->moveto.x = 0;
            f_menu->moveto.y = 0;
            f_menu->tick = tick_get();
        } else {
            //抬手后开始自动移动
            point_t last_dxy = ctp_get_last_dxy();
            s32 to_x, to_y, r;
            f_menu->focus_x = iconlist->ofs_x;
            f_menu->focus_y = iconlist->ofs_y;
            f_menu->flag_move_auto = true;
            to_x = f_menu->focus_x - (last_dxy.x * DRAG_AUTO_SPEED);
            to_y = f_menu->focus_y - (last_dxy.y * DRAG_AUTO_SPEED);
            r = sqrt64(to_x * to_x + to_y * to_y);
            if (r > iconlist->max_r) {
                to_x = to_x * iconlist->max_r / r;
                to_y = to_y * iconlist->max_r / r;
            }
            f_menu->moveto = compo_iconlist_align_xy(iconlist, to_x, to_y);
            f_menu->tick = tick_get();
        }
    } else if (f_menu->flag_move_auto) {
        //自动移动
        if (f_menu->focus_x == f_menu->moveto.x && f_menu->focus_y == f_menu->moveto.y) {
            f_menu->flag_move_auto = false;            //移动完成
        } else if (tick_check_expire(f_menu->tick, FOCUS_AUTO_TICK_EXPIRE)) {
            s32 dx, dy;
            f_menu->tick = tick_get();
            dx = f_menu->moveto.x - f_menu->focus_x;
            dy = f_menu->moveto.y - f_menu->focus_y;
            if (dx > 0) {
                if (dx > FOCUS_AUTO_STEP * FOCUS_AUTO_STEP_DIV) {
                    dx = dx / FOCUS_AUTO_STEP_DIV;
                } else if (dx > FOCUS_AUTO_STEP) {
                    dx = FOCUS_AUTO_STEP;
                } else {
                    dx = 1;
                }
            } else if (dx < 0) {
                if (dx < -FOCUS_AUTO_STEP * FOCUS_AUTO_STEP_DIV) {
                    dx = dx / FOCUS_AUTO_STEP_DIV;
                } else if (dx < -FOCUS_AUTO_STEP) {
                    dx = -FOCUS_AUTO_STEP;
                } else {
                    dx = -1;
                }
            }
            if (dy > 0) {
                if (dy > FOCUS_AUTO_STEP * FOCUS_AUTO_STEP_DIV) {
                    dy = dy / FOCUS_AUTO_STEP_DIV;
                } else if (dy > FOCUS_AUTO_STEP) {
                    dy = FOCUS_AUTO_STEP;
                } else {
                    dy = 1;
                }
            } else if (dy < 0) {
                if (dy < -FOCUS_AUTO_STEP * FOCUS_AUTO_STEP_DIV) {
                    dy = dy / FOCUS_AUTO_STEP_DIV;
                } else if (dy < -FOCUS_AUTO_STEP) {
                    dy = -FOCUS_AUTO_STEP;
                } else {
                    dy = -1;
                }
            }
            f_menu->focus_x += dx;
            f_menu->focus_y += dy;
            compo_iconlist_set_focus(iconlist, f_menu->focus_x, f_menu->focus_y);
            compo_iconlist_update(iconlist);
        }
    } else if (f_menu->flag_zooming) {
        if (f_menu->zoom_cur == f_menu->zoom_target) {
            if (f_menu->zoom_cur == 0) {
                f_menu->flag_zooming = false;
                f_menu->flag_zoomout = false;

                f_menu->focus_x = iconlist->ofs_x;
                f_menu->focus_y = iconlist->ofs_y;
            } else if (f_menu->zoom_cur == f_menu->zoom_out) {
                f_menu->flag_zooming = false;
                f_menu->flag_zoomout = true;

                //居中
                f_menu->focus_x = iconlist->ofs_x;
                f_menu->focus_y = iconlist->ofs_y;
                f_menu->flag_move_auto = true;
                f_menu->moveto.x = 0;
                f_menu->moveto.y = 0;
            } else if (f_menu->flag_zoomout) {
                if (f_menu->zoom_cur > f_menu->zoomin_threshold) {
                    f_menu->zoom_target = 0;
                } else if (tick_check_expire(f_menu->tick, MENU_HC_ZOOM_CANCEL_TICK_EXPIRE)) {
                    f_menu->zoom_target = f_menu->zoom_out;
                }
            } else {
                if (f_menu->zoom_cur < f_menu->zoomout_threshold) {
                    f_menu->zoom_target = f_menu->zoom_out;
                } else if (tick_check_expire(f_menu->tick, MENU_HC_ZOOM_CANCEL_TICK_EXPIRE)) {
                    f_menu->zoom_target = 0;
                }
            }
        } else if (tick_check_expire(f_menu->tick, MENU_HC_ZOOMOUT_TICK_EXPIRE)) {
            int dzoom = f_menu->zoom_target - f_menu->zoom_cur;
            int dw;
            f_menu->tick = tick_get();
            if (dzoom > MENU_HC_ZOOMOUT_STEP) {
                dzoom /= MENU_HC_ZOOMOUT_STEP_DIV;
                if (dzoom < MENU_HC_ZOOMOUT_STEP) {
                    dzoom = MENU_HC_ZOOMOUT_STEP;
                }
            } else if (dzoom < -MENU_HC_ZOOMOUT_STEP) {
                dzoom /= MENU_HC_ZOOMOUT_STEP_DIV;
                if (dzoom > -MENU_HC_ZOOMOUT_STEP) {
                    dzoom = -MENU_HC_ZOOMOUT_STEP;
                }
            }
            f_menu->zoom_cur += dzoom;
            dw = -f_menu->zoom_cur * 8;
            compo_iconlist_set_size(iconlist, GUI_SCREEN_WIDTH + dw, GUI_SCREEN_HEIGHT + dw);
            compo_iconlist_set_iconsize(iconlist, f_menu->icon_size + f_menu->zoom_cur);
            compo_iconlist_set_focus_byidx(iconlist, iconlist->focus_icon_idx);     //改变iconlist_size时，需要重置一下中心点
            compo_iconlist_update(iconlist);
        }
    }
    func_process();
}

//拖动过程中，只响应部分消息
static void func_menu_sub_honeycomb_drag_message(size_msg_t msg)
{
    switch (msg) {
    default:
        evt_message(msg);
        break;
    }
}

//自动移动中，只响应部分消息
static void func_menu_sub_honeycomb_move_auto_message(size_msg_t msg)
{
    f_menu_hc_t *f_menu = (f_menu_hc_t *)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_TOUCH:
        f_menu->flag_drag = true;                               //移动过程中，触屏停止
        f_menu->flag_move_auto = false;
        break;

    default:
        evt_message(msg);
        break;
    }
}

//处理缩放过程中，只响应部分消息
static void func_menu_sub_honeycomb_zooming_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_QDEC_FORWARD:
        func_menu_sub_honeycomb_zooming_in();
        break;

    case MSG_QDEC_BACKWARD:
        func_menu_sub_honeycomb_zooming_out();
        break;

    default:
        evt_message(msg);
        break;
    }
}

//全局视图下的消息处理
static void func_menu_sub_honeycomb_zoomout_message(size_msg_t msg)
{
    f_menu_hc_t *f_menu = (f_menu_hc_t *)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_CLICK:
        func_menu_sub_honeycomb_icon_click();           //单击图标
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_DOWN:
        f_menu->flag_drag = true;                       //任何方向短划，开启拖动
        f_menu->flag_move_auto = false;
        break;

    case MSG_QDEC_FORWARD:
        func_menu_sub_honeycomb_zooming_in();
        break;

    case MSG_QDEC_BACKWARD:
        func_menu_sub_honeycomb_zooming_out();
        break;

    case KU_DELAY_BACK:
        f_menu->flag_zooming = true;
        f_menu->zoom_target = 0;
        break;

    default:
        func_menu_sub_message(msg);
        break;
    }
}

//正常模式下
static void func_menu_sub_honeycomb_normal_message(size_msg_t msg)
{
    f_menu_hc_t *f_menu = (f_menu_hc_t *)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_CLICK:
        func_menu_sub_honeycomb_icon_click();           //单击图标
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_DOWN:
        f_menu->flag_drag = true;                       //任何方向短划，开启拖动
        f_menu->flag_move_auto = false;
        break;

    case MSG_QDEC_FORWARD:                              //向前滚动菜单
        func_menu_sub_honeycomb_switching_in();
        break;

    case MSG_QDEC_BACKWARD:
        func_menu_sub_honeycomb_zooming_out();
        break;

    case MSG_CTP_LONG:
        break;

    case KU_DELAY_BACK:
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_menu_sub_honeycomb_switch_to_clock();      //返回时钟表盘界面
        }
        break;

    default:
        func_menu_sub_message(msg);
        break;
    }
}

//主菜单功能消息处理
static void func_menu_sub_honeycomb_message(size_msg_t msg)
{
    f_menu_hc_t *f_menu = (f_menu_hc_t *)func_cb.f_cb;
    if (f_menu->flag_drag) {
        //拖动过程中，只响应部分消息
        func_menu_sub_honeycomb_drag_message(msg);
    } else if (f_menu->flag_move_auto) {
        //自动移动中，只响应部分消息
        func_menu_sub_honeycomb_move_auto_message(msg);
    } else if (f_menu->flag_zooming) {
        //处理缩放过程中，只响应部分消息
        func_menu_sub_honeycomb_zooming_message(msg);
    } else if (f_menu->flag_zoomout) {
        //全局视图下的消息处理
        func_menu_sub_honeycomb_zoomout_message(msg);
    } else {
        //正常模式下
        func_menu_sub_honeycomb_normal_message(msg);
    }
}

//进入主菜单功能
static void func_menu_sub_honeycomb_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_menu_hc_t));
    func_cb.frm_main = func_menu_sub_honeycomb_form_create();

    f_menu_hc_t *f_menu = (f_menu_hc_t *)func_cb.f_cb;
    f_menu->iconlist = compo_getobj_byid(COMPO_ID_ICONLIST);
    compo_iconlist_t *iconlist = f_menu->iconlist;
    if (iconlist->type != COMPO_TYPE_ICONLIST) {
        halt(HALT_GUI_COMPO_ICONLIST_TYPE);
    }
    f_menu->focus_x = iconlist->ofs_x;
    f_menu->focus_y = iconlist->ofs_y;
    f_menu->icon_size = iconlist->icon_org_size;
    f_menu->zoom_out = iconlist->icon_zoomout_size - f_menu->icon_size;
    f_menu->zoom_min = (iconlist->icon_zoomout_size >> 1) - f_menu->icon_size;
    f_menu->zoomout_threshold = f_menu->zoom_out * 2 / 3;
    f_menu->zoomin_threshold = f_menu->zoom_out / 3;
    func_cb.enter_tick = tick_get();
    if (func_cb.flag_animation) {
        func_cb.flag_animation = 0;
    }
}

//主菜单功能
void func_menu_sub_honeycomb(void)
{
    printf("%s\n", __func__);
    func_menu_sub_honeycomb_enter();
    while (func_cb.sta == FUNC_MENU
           && (func_cb.menu_style == MENU_STYLE_HONEYCOMB || func_cb.menu_style == MENU_STYLE_CUM_HONEYGRID)) {
        func_menu_sub_honeycomb_process();
        func_menu_sub_honeycomb_message(msg_dequeue());
    }
    func_menu_sub_exit();
}
