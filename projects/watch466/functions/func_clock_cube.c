#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif



//立方体
#define CUBE_RADIUS                 75                                                             //切图正方形一半
#define KALE_EDGE_SPACE             2                                                              //边缘距离
#define KALE_ICON_SPACE             2                                                              //图标距离
#define KALE_ICON_OUTER_SIZE        (gui_image_get_size(tbl_menu_cube[0].res_addr).wid)    		   //外圈图标大小
#define KALE_ICON_INNER_SIZE        (KALE_ICON_OUTER_SIZE / 5 * 2)                                 //内圈图标大小
#define KALE_REFRASH_EXPIRE         4                                                              //刷新间隔 uint：ms
#define KALE_SWITCH_DR              2                                                              //单次变更R值
#define KALE_ROTATE_ANGLE           25


//立方体图标列表(固定6项)
static const compo_cube_item_t tbl_menu_cube[] = {
    {UI_BUF_DIALPLATE_CUBE_CALL_BIN,             FUNC_CALL},
    {UI_BUF_DIALPLATE_CUBE_HEART_RATE_BIN,       FUNC_HEARTRATE},
    {UI_BUF_DIALPLATE_CUBE_MUSIC_BIN,            FUNC_BT},
    {UI_BUF_DIALPLATE_CUBE_SLEEP_BIN,            FUNC_SLEEP},
    {UI_BUF_DIALPLATE_CUBE_SPORT_BIN,            FUNC_SPORT},
    {UI_BUF_DIALPLATE_CUBE_STEP_BIN,             FUNC_SLEEP},
};

enum{
    COMPO_ID_CUBE = 1,
    COMPO_ID_TIME_DOT,
};

u16 func_clock_preview_get_type(void);


//点进图标进入应用
static void func_clock_cube_disk_icon_click(void)
{
//    if (sys_cb.dialplate_index != func_clock_get_dialplate_cube_idx()) {
//        return;
//    }

    compo_cube_t *cube = compo_getobj_byid(COMPO_ID_CUBE);
    point_t pt = ctp_get_sxy();

    int icon_idx = compo_cube_get_idx(cube, pt.x, pt.y);
    u8 func_sta;
    if (icon_idx < 0 || icon_idx >= CUBE_ITEM_CNT) {
        return;
    }
    //根据图标索引获取应用ID
    func_sta = tbl_menu_cube[icon_idx].func_sta;

    //切入应用
    if (func_sta > 0) {
//        func_switching(FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO, NULL);
        func_switch_to(func_sta, FUNC_SWITCH_LR_ZOOM_LEFT | FUNC_SWITCH_AUTO);
        func_cb.sta = func_sta;
        func_cb.menu_idx = icon_idx;                //记住当前编号
    }
}


//立方体表盘
compo_form_t *func_clock_cube_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);       //菜单一般创建在底层

    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_CUBE_BG_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 20);

    //创建立方体菜单
    compo_cube_t *cube = compo_cube_create(frm, CUBE_RADIUS, tbl_menu_cube, CUBE_ITEM_CNT);
    compo_cube_set_pos(cube, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 20);
    compo_setid(cube, COMPO_ID_CUBE);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 2);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X - 50, GUI_SCREEN_CENTER_Y - 180, 300, 70);
    compo_bonddata(txt, COMPO_BOND_HOUR);


    txt = compo_textbox_create(frm, 2);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X + 50, GUI_SCREEN_CENTER_Y - 180, 300, 70);
    compo_bonddata(txt, COMPO_BOND_MINUTE);

    txt = compo_textbox_create(frm, 10);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 180, 300, 70);
    compo_bonddata(txt, COMPO_BOND_DATE);

    txt = compo_textbox_create(frm, 1);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 180, 300, 70);
    compo_textbox_set(txt, ":");
    compo_setid(txt, COMPO_ID_TIME_DOT);

    //compo_cube_update(cube);
    return frm;
}


//地图功能事件处理
void func_clock_cube_process(void)
{

    compo_cube_t *cube = compo_getobj_byid(COMPO_ID_CUBE);
    compo_cube_move(cube);

//    func_process();
}

//地图功能消息处理
void func_clock_cube_message(size_msg_t msg)
{
    point_t pt = ctp_get_sxy();
    s16 cube_limit_x = (GUI_SCREEN_WIDTH - 240) / 2;
    s16 cube_limit_y = (GUI_SCREEN_HEIGHT - 240) / 2;
    bool flag_cube_touch_x = (pt.x >= cube_limit_x) && (pt.x <= (cube_limit_x + 240));
    bool flag_cube_touch_y = (pt.y >= cube_limit_y) && (pt.y <= (cube_limit_y + 240));

    if (sys_cb.dialplate_index == func_clock_get_dialplate_cube_idx() && flag_cube_touch_x && flag_cube_touch_y) {
        if (msg == MSG_CTP_TOUCH) {
            compo_cube_t *cube = compo_getobj_byid(COMPO_ID_CUBE);
            //移动过程中，触屏停止。重新进入到开始拖动模式
            compo_cube_move_control(cube, COMPO_CUBE_MOVE_CMD_DRAG);
        } else if (msg >= MSG_CTP_SHORT_LEFT && msg <= MSG_CTP_SHORT_DOWN){
            return;
        }

    }
    static bool time_visible = 0;
    switch (msg) {
    case MSG_CTP_CLICK:
        func_clock_cube_disk_icon_click();                //单击图标
        break;

    case MSG_CTP_SHORT_UP:
//        func_clock_sub_pullup();                //上拉菜单
        func_switch_to(FUNC_CARD, FUNC_SWITCH_MENU_PULLUP_UP | FUNC_SWITCH_DOWN_BG_BLUR);  //上拉卡片界面
        break;

    case MSG_CTP_SHORT_RIGHT:
//        func_clock_sub_side();                  //右拉边菜单
        func_switch_to(FUNC_SIDEBAR, func_get_switching_mode_byidx(sys_cb.nav_index, false));  //右滑界面
        break;

    case MSG_CTP_SHORT_DOWN:
        func_clock_sub_dropdown();              //下拉菜单
        break;

    case MSG_CTP_LONG:
        if (func_clock_preview_get_type() == PREVIEW_ROTARY_STYLE) {
            func_cb.sta = FUNC_CLOCK_PREVIEW;
        } else {
            func_switch_to(FUNC_CLOCK_PREVIEW, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);                    //切换回主时钟
        }
        break;

    case MSG_QDEC_FORWARD:                              //向前滚动菜单
        {
            compo_cube_t *cube = compo_getobj_byid(COMPO_ID_CUBE);
            compo_cube_move_control(cube, COMPO_CUBE_MOVE_CMD_FORWARD);
        }
        break;

    case MSG_QDEC_BACKWARD:                             //向后滚动菜单
        {
            compo_cube_t *cube = compo_getobj_byid(COMPO_ID_CUBE);
            compo_cube_move_control(cube, COMPO_CUBE_MOVE_CMD_BACKWARD);
        }
        break;

    case MSG_SYS_500MS:
    //秒跳动处理
        {
            {
                compo_textbox_t *txt = compo_getobj_byid(COMPO_ID_TIME_DOT);
                compo_textbox_set_visible(txt, time_visible);
                time_visible ^= 1;
            }
        }

        break;

    default:
        func_message(msg);
        break;
    }
}
