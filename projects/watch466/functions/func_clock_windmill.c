#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//泡泡
#define FUNC_BUBBLES_NUM            10                      //最大可存在泡泡个数

//图标列表
///平面立体风车 （这个效果好一点）
static const compo_windmill_item_t tbl_menu_windmill_2d[] = {
    //风车
    {UI_BUF_DIALPLATE_WINDMILL_RED1_BIN},            //外部平行边界
    {UI_BUF_DIALPLATE_WINDMILL_YELLOW1_BIN},
    {UI_BUF_DIALPLATE_WINDMILL_GREEN1_BIN},
    {UI_BUF_DIALPLATE_WINDMILL_BLUE1_BIN},

    {UI_BUF_DIALPLATE_WINDMILL_YELLOW1_BIN},         //内部平行边界
    {UI_BUF_DIALPLATE_WINDMILL_GREEN1_BIN},
    {UI_BUF_DIALPLATE_WINDMILL_BLUE1_BIN},
    {UI_BUF_DIALPLATE_WINDMILL_RED1_BIN},

    {UI_BUF_DIALPLATE_WINDMILL_RED1_BIN},            //红色斜边
    {UI_BUF_DIALPLATE_WINDMILL_YELLOW1_BIN},         //黄色斜边
    {UI_BUF_DIALPLATE_WINDMILL_GREEN1_BIN},          //绿色斜边
    {UI_BUF_DIALPLATE_WINDMILL_BLUE1_BIN},           //蓝色斜边

    {UI_BUF_DIALPLATE_WINDMILL_WIND_TOP_BIN},        //大图
    {UI_BUF_DIALPLATE_WINDMILL_WIND_BOTTOM_BIN},
};

///斜面立体风车
//static const compo_windmill_item_t tbl_menu_windmill_3d[] = {
////    UI_BUF_3D_WINDMILL_WIND_TOP_BIN,     //基准图，辅助画图
//    UI_BUF_DIALPLATE_WINDMILL_TRI_RED_BIN,         //三角
//    UI_BUF_DIALPLATE_WINDMILL_TRI_RED_BIN,
//    UI_BUF_DIALPLATE_WINDMILL_TRI_RED1_BIN,
//    UI_BUF_DIALPLATE_WINDMILL_TRI_RED1_BIN,
//
//    UI_BUF_DIALPLATE_WINDMILL_TRI_YELLOW_BIN,
//    UI_BUF_DIALPLATE_WINDMILL_TRI_YELLOW_BIN,         //三角
//    UI_BUF_DIALPLATE_WINDMILL_TRI_YELLOW1_BIN,
//    UI_BUF_DIALPLATE_WINDMILL_TRI_YELLOW1_BIN,
//
//    UI_BUF_DIALPLATE_WINDMILL_TRI_GREEN_BIN,         //三角
//    UI_BUF_DIALPLATE_WINDMILL_TRI_GREEN_BIN,
//    UI_BUF_DIALPLATE_WINDMILL_TRI_GREEN1_BIN,
//    UI_BUF_DIALPLATE_WINDMILL_TRI_GREEN1_BIN,
//
//    UI_BUF_DIALPLATE_WINDMILL_TRI_BLUE_BIN,         //三角
//    UI_BUF_DIALPLATE_WINDMILL_TRI_BLUE_BIN,
//    UI_BUF_DIALPLATE_WINDMILL_TRI_BLUE1_BIN,
//    UI_BUF_DIALPLATE_WINDMILL_TRI_BLUE1_BIN,
//
//    UI_BUF_DIALPLATE_WINDMILL_TRI_RED2_BIN,         //三角
//    UI_BUF_DIALPLATE_WINDMILL_TRI_YELLOW2_BIN,         //三角
//    UI_BUF_DIALPLATE_WINDMILL_TRI_GREEN2_BIN,         //三角
//    UI_BUF_DIALPLATE_WINDMILL_TRI_BLUE2_BIN,         //三角
//
//    UI_BUF_DIALPLATE_WINDMILL_TRI_YELLOW2_BIN,
//    UI_BUF_DIALPLATE_WINDMILL_TRI_GREEN2_BIN,
//    UI_BUF_DIALPLATE_WINDMILL_TRI_BLUE2_BIN,
//    UI_BUF_DIALPLATE_WINDMILL_TRI_RED2_BIN,
//
//};

enum{
    COMPO_ID_WINDMILL = 1,                                              //风车ID
    COMPO_ID_TIME_DOT,                                                  //表盘时间的冒号

    COMPO_ID_BUBBLE_1,                                                  //泡泡ID
    COMPO_ID_BUBBLE_n = COMPO_ID_BUBBLE_1 + FUNC_BUBBLES_NUM,           //第n个泡泡ID
};

enum {                      ///泡泡状态共用体
    BUBBLE_STA_NONE,        //none;
    BUBBLE_STA_START,       //开始，确定泡泡初始位置
    BUBBLE_STA_MOVE,        //移动
    BUBBLE_STA_END,         //结束
};

typedef struct bubble_t_ {  ///泡泡信息结构体
    u8 sta;                 //泡泡状态
    bool visible;           //泡泡是否可以显示
    u32 res;                //泡泡图片资源
    rect_t location;        //泡泡位置信息
    s8 wind_dir;            //泡泡受到风吹的方向，1:往左; -1往右; 0:没有风
} bubble_t;

//bool flag_create_pbubbles = false;      //泡泡信息结构体指针是否创建（申请内存），true:已申请内存; false:未申请内存
static bubble_t *bubbles;                      //泡泡信息结构体指针
u16 func_clock_preview_get_type(void);

/**
 * @brief       泡泡信息结构体指针初始化
 * @param[in]   void
 * @return[out] void
 */
void func_clock_windmill_pbubbles_create(void)
{
    bubbles = ab_malloc(sizeof(bubble_t) * FUNC_BUBBLES_NUM);
}

/**
 * @brief       泡泡信息结构体指针内存释放
 * @param[in]   void
 * @return[out] void
 */
void func_clock_windmill_pbubbles_destory(void)
{
    ab_free(bubbles);
}

/**
 * @brief       风车2.5D UI 窗体创建
 * @param[in]   void
 * @return[out] compo_form_t*
 *              返回创建窗体
 */
compo_form_t *func_clock_windmill_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);       //菜单一般创建在底层

    //创建背景图
    compo_picturebox_t* pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_WINDMILL_BG_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_size(pic, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT); //原图适配320的，这里拉伸下

    //创建泡泡
    if (func_cb.sta == FUNC_CLOCK) {
        func_clock_windmill_pbubbles_create();
        for (int i=0; i<FUNC_BUBBLES_NUM; i++) {
            bubbles[i].res = UI_BUF_DIALPLATE_WINDMILL_BUBBLE_BIN;
            bubbles[i].sta = BUBBLE_STA_NONE;
            bubbles[i].visible = false;

            compo_picturebox_t* bubble = compo_picturebox_create(frm, UI_BUF_DIALPLATE_WINDMILL_BUBBLE_BIN);
            compo_setid(bubble, COMPO_ID_BUBBLE_1+i);
            compo_picturebox_set_visible(bubble, false);
        }
    }

    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_WINDMILL_POLE_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - gui_image_get_size(UI_BUF_DIALPLATE_WINDMILL_POLE_BIN).hei + gui_image_get_size(UI_BUF_DIALPLATE_WINDMILL_POLE_BIN).hei/2);

    //创建立方体菜单
    compo_windmill_t *windmill = compo_windmill_create(frm, tbl_menu_windmill_2d, sizeof(tbl_menu_windmill_2d)/sizeof(tbl_menu_windmill_2d[0]));
//    compo_windmill_t *windmill = compo_windmill_create(frm, tbl_menu_windmill_3d, sizeof(tbl_menu_windmill_3d)/sizeof(tbl_menu_windmill_3d[0]));
//    widget_set_size(windmill->page, 300, 365);
    compo_windmill_set_pos(windmill, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - gui_image_get_size(UI_BUF_DIALPLATE_WINDMILL_POLE_BIN).hei - 50);
    widget_page_scale_to(windmill->page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_setid(windmill, COMPO_ID_WINDMILL);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 2);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X - 50, GUI_SCREEN_CENTER_Y - 160, 300, 70);
    compo_bonddata(txt, COMPO_BOND_HOUR);

    txt = compo_textbox_create(frm, 2);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X + 50, GUI_SCREEN_CENTER_Y - 160, 300, 70);
    compo_bonddata(txt, COMPO_BOND_MINUTE);

    txt = compo_textbox_create(frm, 10);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 90, 300, 70);
    compo_bonddata(txt, COMPO_BOND_DATE);

    txt = compo_textbox_create(frm, 1);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 160, 300, 70);
    compo_textbox_set(txt, ":");
    compo_setid(txt, COMPO_ID_TIME_DOT);

    return frm;
}


/**
 * @brief       风车 2.5D UI 动画效果处理
 * @param[in]   void
 * @return[out] void
 */
void func_clock_windmill_process(void)
{
    compo_windmill_t *windmill = compo_getobj_byid(COMPO_ID_WINDMILL);
    static u32 ticks = 0;
    if (tick_check_expire(ticks, 50)) {
        ticks = tick_get();
        compo_windmill_move_control(windmill, COMPO_WINDMILL_MOVE_CMD_AUTO);
    }


    compo_windmill_move(windmill);

    //泡泡3d效果处理
    static u32 tick1 = 0;
    static u32 tick2 = 0;
//    static u8 bubble_generate_cnt = 0;
    //随机生成泡泡个数
    if (tick_check_expire(tick2, 1000)) {
        tick2 = tick_get();
        for (int i=0; i<FUNC_BUBBLES_NUM; i++) {
            compo_picturebox_t* bubble = compo_getobj_byid(COMPO_ID_BUBBLE_1+i);
            if (compo_picturebox_get_visible(bubble) == false) {
                bubbles[i].sta = BUBBLE_STA_START;
                break;
            }
        }
    }


    //遍历每个泡泡
    if (tick_check_expire(tick1, 10)) {
        tick1 = tick_get();
        for (int i=0; i<FUNC_BUBBLES_NUM; i++) {
            compo_picturebox_t* bubble = compo_getobj_byid(COMPO_ID_BUBBLE_1+i);
            switch (bubbles[i].sta) {
            case BUBBLE_STA_NONE:
                //none状态，泡泡处于隐藏状态
                compo_picturebox_set_visible(bubble, false);
                compo_picturebox_set_pos(bubble, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT);
                compo_picturebox_set_size(bubble, gui_image_get_size(bubbles[i].res).wid,
                                          gui_image_get_size(bubbles[i].res).hei);
                bubbles[i].visible = false;
                bubbles[i].wind_dir = 0;
//                bubbles[i].sta = BUBBLE_STA_START;
                break;

            case BUBBLE_STA_START:
                compo_picturebox_set_visible(bubble, true);
                bubbles[i].visible = true;


                //随机生成泡泡位置
                int min = GUI_SCREEN_WIDTH/5;
                int max = GUI_SCREEN_WIDTH*4/5;
                int random_num = get_random(max);
                while (random_num < min) {
                    random_num += get_random(max - random_num);
                }

//                printf("rand_num = %d\n", random_num);
                compo_picturebox_set_pos(bubble, random_num, GUI_SCREEN_HEIGHT);
                bubbles[i].sta = BUBBLE_STA_MOVE;
                break;

            case BUBBLE_STA_MOVE: {
                rect_t location = compo_picturebox_get_location(bubble);
                location.y--;
                if (windmill->move_cb.flag_move_auto && windmill->mode != COMPO_WINDMILL_MOVE_CMD_AUTO) {
                    bubbles[i].wind_dir = windmill->wind_grade > 0 ? 1 : -1;
                }
                if (bubbles[i].wind_dir != 0) {
                    location.x += bubbles[i].wind_dir;
                }
//                printf("--->wind[%d]\n", windmill->wind_grade);
                location.hei = location.wid = gui_image_get_size(bubbles[i].res).wid * location.y / GUI_SCREEN_HEIGHT;

                compo_picturebox_set_pos(bubble, location.x, location.y);
                compo_picturebox_set_size(bubble, location.wid, location.hei);
//                printf("bubble [%d] location[%d,%d,%d,%d]\n", i, location.x, location.y, location.wid, location.hei);

                if ((location.y <= -gui_image_get_size(bubbles[i].res).hei/2) ||
                    (location.wid <=1 || location.hei <=1)) {
                    bubbles[i].sta = BUBBLE_STA_END;
                }
            } break;

            case BUBBLE_STA_END:
                bubbles[i].sta = BUBBLE_STA_NONE;
                break;

            default:
                break;
            }
        }
    }



}

/**
 * @brief       风车 2.5D UI 消息处理
 * @param[in]   size_msg_t  传入消息
 * @return[out] void
 */
void func_clock_windmill_message(size_msg_t msg)
{
    point_t pt = ctp_get_sxy();
    static bool time_visible = 0;
    s16 windmill_limit_x = GUI_SCREEN_WIDTH / 20;
    s16 windmill_limit_y = GUI_SCREEN_HEIGHT / 20;
    bool flag_windmill_touch_x = (pt.x >= windmill_limit_x) && (pt.x <= (windmill_limit_x + GUI_SCREEN_WIDTH/10*9));
    bool flag_windmill_touch_y = (pt.y >= windmill_limit_y) && (pt.y <= (windmill_limit_y + GUI_SCREEN_WIDTH/10*9));

    if (flag_windmill_touch_x && flag_windmill_touch_y) {
        if (msg == MSG_CTP_TOUCH) {
            compo_windmill_t *windmill = compo_getobj_byid(COMPO_ID_WINDMILL);
            //移动过程中，触屏停止。重新进入到开始拖动模式
//             printf("compo_windmill_move_control\n");
            compo_windmill_move_control(windmill, COMPO_WINDMILL_MOVE_CMD_DRAG);

        } else if (msg >= MSG_CTP_SHORT_LEFT && msg <= MSG_CTP_SHORT_DOWN){
            return;
        }

    }
//    static bool time_visible = 0;
    switch (msg) {
    case MSG_CTP_CLICK:
//        func_windmill_disk_icon_click();                //单击图标
        break;

    case MSG_CTP_LONG_UP:
        func_clock_swipe_up_to_football_menu();         //自底部上长滑进入足球菜单
        break;

    case MSG_CTP_SHORT_RIGHT:
//        func_clock_sub_side();                  //右拉边菜单
        func_switch_to(FUNC_SIDEBAR, func_get_switching_mode_byidx(sys_cb.nav_index, false));  //右滑界面
        break;

    case MSG_CTP_SHORT_DOWN:
        func_clock_sub_dropdown();              //下拉菜单
        break;

    case MSG_QDEC_FORWARD:                              //向前滚动菜单
        {
            compo_windmill_t *windmill = compo_getobj_byid(COMPO_ID_WINDMILL);
            compo_windmill_move_control(windmill, COMPO_WINDMILL_MOVE_CMD_FORWARD);
        }
        break;

    case MSG_QDEC_BACKWARD:                             //向后滚动菜单
        {
            compo_windmill_t *windmill = compo_getobj_byid(COMPO_ID_WINDMILL);
            compo_windmill_move_control(windmill, COMPO_WINDMILL_MOVE_CMD_BACKWARD);
        }
        break;

    case MSG_CTP_LONG:
        if (func_clock_preview_get_type() == PREVIEW_ROTARY_STYLE) {
            func_cb.sta = FUNC_CLOCK_PREVIEW;
        } else {
            func_switch_to(FUNC_CLOCK_PREVIEW, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);                    //切换回主时钟
        }
        break;

    case MSG_SYS_500MS:
    //秒跳动处理
        {
            compo_textbox_t *txt = compo_getobj_byid(COMPO_ID_TIME_DOT);
            compo_textbox_set_visible(txt, time_visible);
            time_visible ^= 1;
        }

        break;

    default:
        func_message(msg);
        break;
    }
}
