#include "include.h"
#include "func.h"
#include "func_menu.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define GRID_AUTO_STEP                  12               //手松后对其步距, 惯性系数越大, 移动速度越快
#define GRID_AUTO_SPEED                 16               //惯性速度, 越大, 最终移动距离越远
#define GRID_FOCUS_AUTO_TICK_EXPIRE     18              //自动对其图标时间阈值
#define GRID_AUTO_STEP_DIV              8

#define GRID_ZOOM_STEP                  2               //缩放界面所有图标的缩放步距值
#define GRID_ZOOM_AUTO_TICK_EXPIRE      18              //点击图标放大时自动对其焦点时间
#define GRID_ZOOM_STEP_DIV              6

#define GRID_ZOOM_CANCEL_TICK_EXPIRE    300

#define GRID_ENTERING_ANIMATION_CNT     1
#define GRID_ANIMATION_TICK_EXPIRE      18

#define MENU_GRID_CNT                         ((int)((sizeof(tbl_menu_grid) / sizeof(tbl_menu_grid[0]))))

//iconlist菜单对应组件ID
enum {
    COMPO_ID_GRIDBOX = 1,
};

typedef struct menu_hc_item_t_ {
    u32 res_addr;
    u8 func_sta;
} menu_hc_item_t;

//全部图标资源tbl, 最大可以显示81个图标
static const menu_hc_item_t tbl_menu_grid[] = {
    {UI_BUF_ICON_CLOCK_BG_BIN,                  FUNC_CLOCK},
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
    {UI_BUF_ICON_HEART_RATE_BIN,                FUNC_HEARTRATE},
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
    {UI_BUF_ICON_BLOOD_PRESSURE_BIN,            FUNC_BLOODSUGAR},
#if SECURITY_PAY_EN
    {UI_BUF_ICON_ALIPAY_BIN,                    FUNC_ALIPAY},
#endif // SECURITY_PAY_EN
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},

    //测试图标
    /*
    {UI_BUF_ICON_ADDRESS_BOOK_BIN,              FUNC_ADDRESS_BOOK},
    {UI_BUF_ICON_VOICE_BIN,                     FUNC_VOICE},
    {UI_BUF_ICON_WEATHER_BIN,                   FUNC_WEATHER},
    {UI_BUF_ICON_BLOOD_PRESSURE_BIN,            FUNC_BLOODSUGAR},
    {UI_BUF_ICON_ALIPAY_BIN,                    FUNC_ALIPAY},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_ADDRESS_BOOK_BIN,              FUNC_ADDRESS_BOOK},
    {UI_BUF_ICON_VOICE_BIN,                     FUNC_VOICE},
    {UI_BUF_ICON_WEATHER_BIN,                   FUNC_WEATHER},
    {UI_BUF_ICON_BLOOD_PRESSURE_BIN,            FUNC_BLOODSUGAR},
    {UI_BUF_ICON_ALIPAY_BIN,                    FUNC_ALIPAY},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    */
};

//gridbox菜单控制结构体
typedef struct f_menu_grid_t_ {
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
    point_t moveto;

    u16 icon_size;
    s16 zoom_cur;
    s16 zoom_target;
    s16 zoom_out;
    bool flag_zoomout;
    bool flag_zooming;

    s16 zoom_min;
    s16 zoomin_threshold;
    s16 zoomout_threshold;

    s32 animation_cnt;

}f_menu_grid_t;

//创建grid风格主菜单窗体
compo_form_t* func_menu_sub_grid_form_create(void)
{
    compo_form_t *frm = compo_form_create(false);
    compo_iconlist_t *iconlist;
    if (func_cb.menu_style == MENU_STYLE_GRID) {
        iconlist = compo_iconlist_create(frm, ICONLIST_STYLE_GRID);
    } else {
        iconlist = compo_iconlist_create(frm, ICONLIST_STYLE_CUM_GRID);
    }
    compo_setid(iconlist, COMPO_ID_GRIDBOX);
    for (int i=0; i<MENU_GRID_CNT; i++) {
        compo_iconlist_add(iconlist, tbl_menu_grid[i].res_addr);
    }
    compo_iconlist_add_time(iconlist, COMPO_ICONLIST_TIME_TYPE_HOUR, UI_BUF_ICON_CLOCK_H_BIN, 1, 2);
    compo_iconlist_add_time(iconlist, COMPO_ICONLIST_TIME_TYPE_MIN, UI_BUF_ICON_CLOCK_M_BIN, 1, 2);
    compo_iconlist_add_time(iconlist, COMPO_ICONLIST_TIME_TYPE_SEC, UI_BUF_ICON_CLOCK_S_BIN, 9, 2);
    compo_iconlist_set_start_angle(iconlist, 900);

    compo_iconlist_set_focus_byidx(iconlist, func_cb.menu_idx);
    compo_iconlist_update(iconlist);
    return frm;
}

//点击图标
static void func_menu_grid_icon_click(void)
{
    point_t pt;
    u8 func_sta;
    f_menu_grid_t *f_menu = (f_menu_grid_t*)func_cb.f_cb;
    compo_iconlist_t *iconlist = f_menu->iconlist;
    pt = ctp_get_sxy();

    //获取点击图标id
    int icon_idx;
    icon_idx = compo_iconlist_select(iconlist,pt.x,pt.y);
    //printf("click [%d]\n", icon_idx);
    if (icon_idx < 0 || icon_idx >= MENU_GRID_CNT) {
        return;
    }

    //平铺模式, 聚焦到点击图标, 退出平铺模式
    if (f_menu->flag_zoomout) {
        //放大图标
        compo_iconlist_set_focus_byidx(iconlist, icon_idx);
        compo_iconlist_update(iconlist);
        f_menu->flag_zooming = true;
        f_menu->zoom_target = 0;
    } else {
        //切入应用程序
        func_sta = tbl_menu_grid[icon_idx].func_sta;
        if (func_sta > 0) {
            compo_form_t *frm = func_create_form(func_sta);
            func_switching(FUNC_SWITCH_ZOOM_ENTER | FUNC_SWITCH_AUTO, iconlist->sel_icon);
            compo_form_destroy(frm);
            func_cb.sta = func_sta;
            func_cb.menu_idx = icon_idx;
        }
    }
}

//切换进入应用
static void func_menu_sub_grid_switching_in(void)
{
    f_menu_grid_t *f_menu = (f_menu_grid_t*)func_cb.f_cb;
    compo_iconlist_t *iconlist = f_menu->iconlist;
    int icon_idx;
    icon_idx = compo_iconlist_select(iconlist,GUI_SCREEN_CENTER_X,GUI_SCREEN_CENTER_Y);
    //printf("click [%d]\n", icon_idx);
    if (icon_idx < 0 || icon_idx >= MENU_GRID_CNT) {
        return;
    }

    //根据图标索引获取应用ID
    u8 func_sta = tbl_menu_grid[icon_idx].func_sta;

    //切入应用
    if (func_sta > 0) {
        compo_form_t *frm = func_create_form(func_sta);
        bool res = func_switching(FUNC_SWITCH_ZOOM_ENTER, iconlist->sel_icon);
        compo_form_destroy(frm);
        if (res) {
            func_cb.sta = func_sta;
            func_cb.menu_idx = icon_idx;            //记住当前编号
        }
    }
}

//切换到时钟
static void func_menu_sub_gridbox_switch_to_clock(void)
{
    f_menu_grid_t *f_menu = (f_menu_grid_t*)func_cb.f_cb;
    compo_iconlist_t *iconlist = f_menu->iconlist;

    widget_icon_t *icon = compo_iconlist_select_byidx(iconlist, iconlist->focus_icon_idx);
    u8 func_sta = FUNC_CLOCK;
    compo_form_t *frm = func_create_form(func_sta);
    func_switching(FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO, icon);
    compo_form_destroy(frm);
    func_cb.sta = func_sta;
    //func_cb.menu_idx = iconlist->focus_icon_idx;
    func_cb.menu_idx = 0;
    func_cb.flag_animation = true;
}

//入场动画
static void func_menu_sub_grid_entering(void)
{
    f_menu_grid_t *f_menu = (f_menu_grid_t*)func_cb.f_cb;
    compo_iconlist_t *iconlist = f_menu->iconlist;
    //compo_iconlist_set_focus_byidx(iconlist, 21);
    compo_iconlist_update(iconlist);
    f_menu->focus_x = iconlist->ofs_x;
    f_menu->focus_y = iconlist->ofs_y;
}

//主菜单功能事件处理
static void func_menu_sub_grid_process(void)
{
    f_menu_grid_t *f_menu = (f_menu_grid_t*)func_cb.f_cb;
    compo_iconlist_t *iconlist = f_menu->iconlist;

    if (f_menu->animation_cnt > 0) {
        //入场动画
        if (tick_check_expire(f_menu->tick, GRID_ANIMATION_TICK_EXPIRE)) {
            f_menu->tick = tick_get();
            f_menu->animation_cnt--;
            func_menu_sub_grid_entering();
        }
    } else if (f_menu->flag_drag) {
        //s32 dx = f_menu->focus_dx;
        //s32 dy = f_menu->focus_dy;
        f_menu->flag_drag = ctp_get_dxy(&f_menu->focus_dx, &f_menu->focus_dy);
        if (f_menu->flag_drag) {
            //拖动菜单图标
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
            //抬手后开始自动移动(惯性)
            point_t last_dxy = ctp_get_last_dxy();
            s32 to_x, to_y;
            f_menu->focus_x = iconlist->ofs_x;
            f_menu->focus_y = iconlist->ofs_y;

            f_menu->flag_move_auto = true;
            to_x = f_menu->focus_x - (last_dxy.x * GRID_AUTO_SPEED);
            to_y = f_menu->focus_y - (last_dxy.y * GRID_AUTO_SPEED);
            f_menu->moveto = compo_iconlist_align_xy(iconlist, to_x, to_y);
            f_menu->tick = tick_get();
        }

    } else if (f_menu->flag_move_auto) {
        //自动移动
        if (f_menu->focus_x == f_menu->moveto.x && f_menu->focus_y == f_menu->moveto.y) {
            //int idx = compo_iconlist_select(iconlist, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
            //if (idx >= 0) {
            //    iconlist->focus_idx = idx;
            //}
            //printf("f_menu->flag_move_auto_over[%d]\n",iconlist->focus_idx);
            f_menu->flag_move_auto = false;            //移动完成
        } else if (tick_check_expire(f_menu->tick, GRID_FOCUS_AUTO_TICK_EXPIRE)) {
            s32 dx, dy;
            f_menu->tick = tick_get();
            dx = f_menu->moveto.x - f_menu->focus_x;
            dy = f_menu->moveto.y - f_menu->focus_y;


            if (abs_s(dx) > GRID_AUTO_STEP * GRID_AUTO_STEP_DIV) {
                f_menu->focus_xstep = abs_s(dx) / GRID_AUTO_STEP_DIV;
            } else if (abs_s(dx) > GRID_AUTO_STEP) {
                f_menu->focus_xstep = GRID_AUTO_STEP;
            } else {
                f_menu->focus_xstep = 1;
            }


            if (abs_s(dy) > GRID_AUTO_STEP * GRID_AUTO_STEP_DIV) {
                f_menu->focus_ystep = abs_s(dy) / GRID_AUTO_STEP_DIV;
            } else if (abs_s(dy) > GRID_AUTO_STEP) {
                f_menu->focus_ystep = GRID_AUTO_STEP;
            } else {
                f_menu->focus_ystep = 1;
            }

            if (dx < 0) {
                f_menu->focus_xstep *= -1;
            }

            if (dy < 0) {
                f_menu->focus_ystep *= -1;
            }

            if (f_menu->focus_x != f_menu->moveto.x) {
                f_menu->focus_x += f_menu->focus_xstep;
            }
            if (f_menu->focus_y != f_menu->moveto.y) {
                f_menu->focus_y += f_menu->focus_ystep;
            }
            //printf("xy [%d,%d] toxy[%d,%d]\n",f_menu->focus_x, f_menu->focus_y, f_menu->moveto.x, f_menu->moveto.y);
            compo_iconlist_set_focus(iconlist, f_menu->focus_x, f_menu->focus_y);
            compo_iconlist_update(iconlist);
        }
    } else if (f_menu->flag_zooming) {
        //缩放
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
                } else if (tick_check_expire(f_menu->tick, GRID_ZOOM_CANCEL_TICK_EXPIRE)) {
                    f_menu->zoom_target = f_menu->zoom_out;
                }
            } else {
                if (f_menu->zoom_cur < f_menu->zoomout_threshold) {
                    f_menu->zoom_target = f_menu->zoom_out;
                } else if (tick_check_expire(f_menu->tick, GRID_ZOOM_CANCEL_TICK_EXPIRE)) {
                    f_menu->zoom_target = 0;
                }
            }
        } else if (tick_check_expire(f_menu->tick, GRID_ZOOM_AUTO_TICK_EXPIRE)) {

            int dzoom = f_menu->zoom_target - f_menu->zoom_cur;

            f_menu->tick = tick_get();

            if (dzoom > GRID_ZOOM_STEP) {
                dzoom /= GRID_ZOOM_STEP_DIV;
                if (dzoom < GRID_ZOOM_STEP) {
                    dzoom = GRID_ZOOM_STEP;
                }
            } else if (dzoom < -GRID_ZOOM_STEP) {
                dzoom /= GRID_ZOOM_STEP_DIV;
                if (dzoom > -GRID_ZOOM_STEP) {
                    dzoom = -GRID_ZOOM_STEP;
                }
            }

            f_menu->zoom_cur += dzoom;

            compo_iconlist_set_iconsize(iconlist, f_menu->icon_size + f_menu->zoom_cur);
            compo_iconlist_set_focus_byidx(iconlist,iconlist->focus_icon_idx);
            compo_iconlist_update(iconlist);
        }

    }

    func_process();
}

static void func_menu_sub_grid_entering_message(size_msg_t msg)
{
    switch (msg) {
    default:
        evt_message(msg);
        break;
    }
}

//拖动过程中，只响应部分消息
static void func_menu_sub_grid_drag_message(size_msg_t msg)
{
    switch (msg) {
    default:
        evt_message(msg);
        break;
    }
}

//自动移动中，只响应部分消息
static void func_menu_sub_grid_move_auto_message(size_msg_t msg)
{
    f_menu_grid_t *f_menu = (f_menu_grid_t*)func_cb.f_cb;
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

//图标放大回到普通模式
static void func_menu_sub_grid_zooming_in()
{
    f_menu_grid_t *f_menu = (f_menu_grid_t*)func_cb.f_cb;
    f_menu->flag_zooming = true;
    f_menu->zoom_target += GRID_ZOOM_STEP * GRID_ZOOM_STEP_DIV;
    if (f_menu->zoom_target > 0) {
        f_menu->zoom_target = 0;
        f_menu->tick = tick_get();
    }
}

//图标缩小到平铺模式
static void func_menu_sub_grid_zooming_out()
{
    f_menu_grid_t *f_menu = (f_menu_grid_t*)func_cb.f_cb;
    f_menu->flag_zooming = true;
    f_menu->zoom_target -= GRID_ZOOM_STEP * GRID_ZOOM_STEP_DIV;;
    if (f_menu->zoom_target < f_menu->zoom_min) {
        f_menu->zoom_target = f_menu->zoom_min;
        f_menu->tick = tick_get();
    }
}

//处理缩放过程中，只响应部分消息
static void func_menu_sub_grid_zooming_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_QDEC_FORWARD:
        func_menu_sub_grid_zooming_in();
        break;

    case MSG_QDEC_BACKWARD:
        func_menu_sub_grid_zooming_out();
        break;

    default:
        evt_message(msg);
        break;
    }
}

//正常模式消息处理
static void func_menu_sub_grid_normal_message(size_msg_t msg)
{
    f_menu_grid_t *f_menu = (f_menu_grid_t*)func_cb.f_cb;
    //compo_iconlist_t *iconlist = f_menu->iconlist;

    switch (msg) {
    case MSG_CTP_CLICK:
        func_menu_grid_icon_click();                    //单击图标
        break;

    case MSG_CTP_SHORT_LEFT:                            //任意方向滑动
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_DOWN:
        //iconlist->status = GRIDBOX_PAGE_DRAG;            //滑动,在process处理
        f_menu->flag_drag = true;
        f_menu->flag_move_auto = false;
        //f_menu->flag_zooming = false;
        break;

    case MSG_QDEC_FORWARD:                              //向前滚动菜单     图标放大回初始状态
        //iconlist->status = GRIDBOX_PAGE_ZOOM_DOWN;

        //func_menu_sub_grid_zooming_in();
        func_menu_sub_grid_switching_in();
        break;

    case MSG_QDEC_BACKWARD:                              //向后滚动菜单     图标缩小到平铺状态
        func_menu_sub_grid_zooming_out();
        break;

    case MSG_CTP_LONG:
        break;

    case KU_DELAY_BACK:
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_menu_sub_gridbox_switch_to_clock();      //返回时钟表盘界面
        }
        break;

    default:
        func_menu_sub_message(msg);
        break;

    }

}

//全局视图下的消息处理
static void func_menu_sub_grid_zoomout_message(size_msg_t msg)
{
    f_menu_grid_t *f_menu = (f_menu_grid_t*)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_CLICK:
        func_menu_grid_icon_click();             //单击图标
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_DOWN:
        f_menu->flag_drag = true;                       //任何方向短划，开启拖动
        f_menu->flag_move_auto = false;
        break;

    case MSG_QDEC_FORWARD:
        //func_menu_sub_honeycomb_zooming_in();
        func_menu_sub_grid_zooming_in();
        break;

    case MSG_QDEC_BACKWARD:
            //func_menu_sub_honeycomb_zooming_out();
        func_menu_sub_grid_zooming_out();
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

//进入grid风格主菜单消息处理
static void func_menu_sub_grid_message(size_msg_t msg)
{
    f_menu_grid_t *f_menu = (f_menu_grid_t*)func_cb.f_cb;
    //compo_iconlist_t *iconlist = f_menu->iconlist;
    if (f_menu->animation_cnt > 0) { //入场动画过程中，只响应部分消息
        func_menu_sub_grid_entering_message(msg);
    } else if (f_menu->flag_drag) { //拖动只响应部分消息
        func_menu_sub_grid_drag_message(msg);
    } else if (f_menu->flag_move_auto) {
        func_menu_sub_grid_move_auto_message(msg);
    } else if(f_menu->flag_zooming) {
        func_menu_sub_grid_zooming_message(msg);
    } else if (f_menu->flag_zoomout) {
        func_menu_sub_grid_zoomout_message(msg);
    } else {
        func_menu_sub_grid_normal_message(msg); //正常模式
    }
}

//进入grid风格主菜单
static void func_menu_sub_grid_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_menu_grid_t));
    func_cb.frm_main = func_menu_sub_grid_form_create();
    f_menu_grid_t *f_menu = (f_menu_grid_t*)func_cb.f_cb;
    f_menu->iconlist = compo_getobj_byid(COMPO_ID_GRIDBOX);
    compo_iconlist_t *iconlist = f_menu->iconlist;

    f_menu->zoom_target = 0;
    f_menu->flag_zoomout = 0;
    f_menu->flag_zooming = 0;
    f_menu->flag_move_auto = 0;
    f_menu->flag_drag = 0;
    f_menu->zoom_cur = 0;

    f_menu->focus_x = iconlist->ofs_x;
    f_menu->focus_y = iconlist->ofs_y;
    f_menu->icon_size = iconlist->icon_org_size;
    f_menu->zoom_min = iconlist->icon_zoomout_size / 2 - f_menu->icon_size;
    f_menu->zoom_out = iconlist->icon_zoomout_size - f_menu->icon_size;
    f_menu->zoomout_threshold = f_menu->zoom_out * 2 / 3;
    f_menu->zoomin_threshold = f_menu->zoom_out / 3;

    func_cb.enter_tick = tick_get();

    f_menu->animation_cnt = 0;
    if (func_cb.flag_animation) {
        func_cb.flag_animation = false;
        f_menu->animation_cnt = GRID_ENTERING_ANIMATION_CNT;
        f_menu->tick = tick_get();
    }

}

static void func_menu_sub_grid_exit(void)
{
    func_menu_sub_exit();
}

void func_menu_sub_grid(void)
{
    printf("%s-->style:%d\n", __func__, func_cb.menu_style);
    func_menu_sub_grid_enter();
    while (func_cb.sta == FUNC_MENU && (func_cb.menu_style == MENU_STYLE_GRID || func_cb.menu_style == MENU_STYLE_CUM_GRID)) {
        func_menu_sub_grid_process();
        func_menu_sub_grid_message(msg_dequeue());
    }
    func_menu_sub_grid_exit();
}
