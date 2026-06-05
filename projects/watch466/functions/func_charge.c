#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif
#define TEST_EN 1

#if TEST_EN
static u16 test_value = 0;
#define BAT_PERCENT_VALUE       test_value  //电量百分比数值
#else
#define BAT_PERCENT_VALUE       sys_cb.vbat_percent  //电量百分比数值
#endif // TEST_EN

#define CHARGE_CNT                     5               //电池类型数量
enum {
	//数字
   COMPO_ID_NUM_PERCENT,
   COMPO_ID_NUM_CHARGE_PERCENT_1,   //电量0%
   COMPO_ID_NUM_CHARGE_PERCENT_2,//电量20%
   COMPO_ID_NUM_CHARGE_PERCENT_3,//电量40%
   COMPO_ID_NUM_CHARGE_PERCENT_4,//电量60%
   COMPO_ID_NUM_CHARGE_PERCENT_5,//电量80%
   COMPO_ID_NUM_CHARGE_PERCENT_FULL,//电量100%
};

typedef struct f_charge_t_ {
   u8 percent_bkp;
} f_charge_t;

////根据百分比计算四分之一圈旋转的角度(0-2700)
//static u16 func_charge_percent_to_deg(u16 percent)
//{
//   percent = MIN(100, MAX(25, percent));
//   return (2700 * (percent - 25) / (100 - 25));
//}

//创建充电窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_charge_form_create(void)
{
   //新建窗体和背景
   compo_form_t *frm = compo_form_create(true);

//    //新建图像
//    	compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_CHARGE_CHARGE_H140_BIN);
// 	compo_picturebox_cut(pic, sys_cb.charge_idx, CHARGE_CNT);
// 	compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
// 	compo_setid(pic, COMPO_ID_NUM_CHARGE_PERCENT_1);

//     pic = compo_picturebox_create(frm, UI_BUF_CHARGE_CHARGE_H140_BIN);
// 	compo_picturebox_cut(pic, sys_cb.charge_idx + 1, CHARGE_CNT);
// 	compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
//     compo_picturebox_set_visible(pic, BAT_PERCENT_VALUE >= 20);//LOWPWR_PERCENT
// 	compo_setid(pic, COMPO_ID_NUM_CHARGE_PERCENT_2);

//     pic = compo_picturebox_create(frm, UI_BUF_CHARGE_CHARGE_H140_BIN);
// 	compo_picturebox_cut(pic, sys_cb.charge_idx + 2, CHARGE_CNT);
// 	compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
//      compo_picturebox_set_visible(pic, BAT_PERCENT_VALUE >= 40);//LOWPWR_PERCENT
// 	compo_setid(pic, COMPO_ID_NUM_CHARGE_PERCENT_3);

//     pic = compo_picturebox_create(frm, UI_BUF_CHARGE_CHARGE_H140_BIN);
// 	compo_picturebox_cut(pic, sys_cb.charge_idx + 3, CHARGE_CNT);
// 	compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
//     compo_picturebox_set_visible(pic, BAT_PERCENT_VALUE >= 60);//LOWPWR_PERCENT
// 	compo_setid(pic, COMPO_ID_NUM_CHARGE_PERCENT_4);

//     pic = compo_picturebox_create(frm, UI_BUF_CHARGE_CHARGE_H140_BIN);
// 	compo_picturebox_cut(pic, sys_cb.charge_idx + 4, CHARGE_CNT);
// 	compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
//     compo_picturebox_set_visible(pic, BAT_PERCENT_VALUE >= 80);//LOWPWR_PERCENT
// 	compo_setid(pic, COMPO_ID_NUM_CHARGE_PERCENT_5);

//     pic = compo_picturebox_create(frm, UI_BUF_CHARGE_CHARGE_FULL_BIN);
// 	compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
//     compo_picturebox_set_visible(pic, BAT_PERCENT_VALUE >= 100);//LOWPWR_PERCENT
// 	compo_setid(pic, COMPO_ID_NUM_CHARGE_PERCENT_FULL);

   return frm;
}

//充电功能事件处理
static void func_charge_process(void)
{
   f_charge_t *f_charge = (f_charge_t *)func_cb.f_cb;
    compo_picturebox_t *pic;
//    compo_textbox_t *txt;
//    char str_buff[8];

#if TEST_EN
    static u32 tick = 0;
    if (tick_check_expire(func_cb.enter_tick, 3000) && tick_check_expire(tick, 300)) {
        tick = tick_get();
        test_value += test_value >= 100 ? 0 : 1;
        printf("test_value [%d]\n", test_value);
    }
#endif // TEST_EN

    if (f_charge->percent_bkp != BAT_PERCENT_VALUE) {
        pic = compo_getobj_byid(COMPO_ID_NUM_CHARGE_PERCENT_2);
        compo_picturebox_set_visible(pic, BAT_PERCENT_VALUE > 20);//LOWPWR_PERCENT
        pic = compo_getobj_byid(COMPO_ID_NUM_CHARGE_PERCENT_3);
        compo_picturebox_set_visible(pic, BAT_PERCENT_VALUE > 40);//LOWPWR_PERCENT
        pic = compo_getobj_byid(COMPO_ID_NUM_CHARGE_PERCENT_4);
        compo_picturebox_set_visible(pic, BAT_PERCENT_VALUE > 60);//LOWPWR_PERCENT
        pic = compo_getobj_byid(COMPO_ID_NUM_CHARGE_PERCENT_5);
        compo_picturebox_set_visible(pic, BAT_PERCENT_VALUE > 80);//LOWPWR_PERCENT
        pic = compo_getobj_byid(COMPO_ID_NUM_CHARGE_PERCENT_FULL);
        compo_picturebox_set_visible(pic, BAT_PERCENT_VALUE >= 100);
    }

   func_process();
}

//充电功能消息处理
static void func_charge_message(size_msg_t msg)
{
   switch (msg) {
#if TEST_EN
   case MSG_CTP_CLICK:
        test_value = 0;
        TRACE("test_value:%d\n", test_value);
        compo_picturebox_t *pic = compo_getobj_byid(COMPO_ID_NUM_CHARGE_PERCENT_FULL);
        compo_picturebox_set_visible(pic, BAT_PERCENT_VALUE >= 100);
       break;
#endif // TEST_EN

   case MSG_CTP_SHORT_UP:
   case MSG_CTP_SHORT_DOWN:
   case MSG_CTP_SHORT_LEFT:
//    case MSG_CTP_SHORT_RIGHT:
   case MSG_CTP_LONG:
       break;

   case MSG_QDEC_FORWARD:
   case MSG_QDEC_BACKWARD:
       break;

   default:
       func_message(msg);
       break;
   }
}

void func_charge_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_charge_t));
    func_cb.frm_main = func_charge_form_create();
}

//退出地图功能
void func_charge_exit(void)
{
    func_cb.last = FUNC_CHARGE;
}

//地图功能
void func_charge(void)
{
    printf("%s\n", __func__);
    func_charge_enter();
    while (func_cb.sta == FUNC_CHARGE) {
        func_charge_process();
        func_charge_message(msg_dequeue());
    }
    func_charge_exit();
}



#if 0
//立方体
#define CUBE_RADIUS_MAX             60                                                             //切图正方形一倍
#define CUBE_RADIUS_MIN             45                                                             //切图正方形一半
#define KALE_EDGE_SPACE             2                                                              //边缘距离
#define KALE_ICON_SPACE             2                                                              //图标距离
#define KALE_ICON_OUTER_SIZE        (gui_image_get_size(tbl_menu_cube[0].res_addr).wid)    		   //外圈图标大小
#define KALE_ICON_INNER_SIZE        (KALE_ICON_OUTER_SIZE / 5 * 2)                                 //内圈图标大小
#define KALE_REFRASH_EXPIRE         4                                                              //刷新间隔 uint：ms
#define KALE_SWITCH_DR              2                                                              //单次变更R值
#define KALE_ROTATE_ANGLE           25

#define CUBE_ELE_CNT                5

#define AUTO_MOVE_MODE              1


//立方体图标列表(固定6项)
static const compo_cube_item_t tbl_menu_cube[] = {
    {UI_BUF_CHARGE_ICON_BIN,             FUNC_NULL},
    {UI_BUF_CHARGE_ICON_BIN,             FUNC_NULL},
    {UI_BUF_CHARGE_ICON_BIN,             FUNC_NULL},
    {UI_BUF_CHARGE_ICON_BIN,             FUNC_NULL},
    {UI_BUF_CHARGE_ICON_BIN,             FUNC_NULL},
    {UI_BUF_CHARGE_ICON_BIN,             FUNC_NULL},
};

typedef struct f_charge_t_ {
    bool click_sta;
} f_charge_t;


enum{
    COMPO_ID_CUBE = 1,
};

//立方体表盘
compo_form_t *func_charge_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);       //菜单一般创建在底层

    //创建立方体菜单
    compo_cube_t *cube = compo_cube_create(frm, CUBE_RADIUS_MAX, tbl_menu_cube, CUBE_ITEM_CNT);
    compo_cube_set_type(cube, COMPO_CUBE_TYPE_POWER);
    compo_cube_add_element(cube, 0, UI_BUF_CHARGE_LIGHT_BIN, CUBE_ELE_CNT);

    compo_cube_set_pos(cube, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 20);
    compo_setid(cube, COMPO_ID_CUBE);
    return frm;
}


enum {
    BATTER_3D_CLICK_STA_NONE,   //None
    BATTER_3D_CLICK_STA_DEC,    //缩小
    BATTER_3D_CLICK_STA_INC,    //放大
    BATTER_3D_CLICK_STA_END,    //结束
    BATTER_3D_CLICK_STA_STOP,   //停顿
};

enum {
    BATTER_3D_ANIM_STA_NONE,
    BATTER_3D_ANIM_STA_DEC,
    BATTER_3D_ANIM_STA_INC,
    BATTER_3D_ANIM_STA_END,
    BATTER_3D_ANIM_STA_POS,
};

void func_charge_process_do(void)
{
    f_charge_t *f_charge = (f_charge_t *)func_cb.f_cb;
    compo_cube_t *cube = compo_getobj_byid(COMPO_ID_CUBE);
    compo_cube_move(cube);

    static u32 tick1, tick2;
    u32 width_max = cube->ele[0].location.wid;//gui_image_get_size(tbl_menu_ele[0].res_addr).wid / 2;
    static u32 width;


    static u8 click_sta = BATTER_3D_CLICK_STA_NONE; //0:None 1:开始缩小; 2:开始放大; 3:结束; 4:停顿
    static u8 stop_cnt = 0;

    static u8 img3d_index[CUBE_ELE_CNT/2];
    static u8 ele_anim_sta = BATTER_3D_ANIM_STA_NONE;     //0:None; 1:光衰弱; 2:光增强; 3:结束; 4:改光位置
    static s16 alph = 0;


    //极角 与 方位角 组合
    //极角:450(固定);       方位角: 450 到 -450;    旋转角: 0(固定);
    //方位角:450(固定);     极角: 450 到 -450;      旋转角: 0(固定);
    //极角:1350(固定);      方位角: 450 到 -450;    旋转角: 0(固定);
    //方位角:1350(固定);    极角: 450 到 -450;      旋转角: 0(固定);
    //极角:2250(固定);      方位角: 900(固定);      旋转角: 450 到 -450;
    //极角:1350(固定);      方位角: 900(固定);      旋转角: 450 到 -450;
    static const u16 tbl_cube_ele_polar[] = {
        450, 0, 1350, 0, 2250, 1350,
    };

    static const u16 tbl_cube_ele_azimuth[] = {
        0, 450, 0, 1350, 900, 900,
    };

    if (tick_check_expire(tick1, 20)) {
        tick1 = tick_get();
        //静态元素动画
        switch (ele_anim_sta) {
        case BATTER_3D_ANIM_STA_NONE: {

            for (int k=0; k<sizeof(img3d_index)/sizeof(img3d_index[0]); k++) {
                int n = CUBE_ELE_CNT > CUBE_ITEM_ELE_CNT ? (CUBE_ITEM_ELE_CNT-1) : (CUBE_ELE_CNT-1);
                int range =  n + 1;
                int buckets = 3600 / range;
                int max_random = buckets * range;
                int random_num;
                do {
                    random_num = get_random(3600);
                } while (random_num >= max_random);

                random_num = (random_num % range);
                img3d_index[k] = random_num;
//                img3d[k] = cube->ele[random_num].img3d;
//                printf("img3d[%d,%d,%x]\n", k, random_num, img3d[k]);
            }

            alph = 255;
            ele_anim_sta = BATTER_3D_ANIM_STA_DEC;

        } break;

        case BATTER_3D_ANIM_STA_DEC:
            alph-=5;
            if (alph <= 0) {
                alph = 0;
                ele_anim_sta = BATTER_3D_ANIM_STA_POS;
            }

            for (int k=0; k<sizeof(img3d_index)/sizeof(img3d_index[0]); k++) {
                if (cube->ele[img3d_index[k]].img3d != NULL) {
                    widget_set_alpha(cube->ele[img3d_index[k]].img3d, alph);
                }
            }
            break;

        case BATTER_3D_ANIM_STA_POS:
            for (int k=0; k<sizeof(img3d_index)/sizeof(img3d_index[0]); k++) {
                if (cube->ele[img3d_index[k]].img3d != NULL) {
                    // 更均匀的分布方法（适用于大n值）
                    int n = 250;
                    int range = 2 * n + 1;
                    int buckets = 3600 / range;
                    int max_random = buckets * range;

                    int random_num;
                    do {
                        random_num = get_random(3600);
                    } while (random_num >= max_random);

                    random_num = (random_num % range) - n;
                    s16 polar = tbl_cube_ele_polar[cube->ele[img3d_index[k]].sta] != 0 ? tbl_cube_ele_polar[cube->ele[img3d_index[k]].sta] : random_num;
                    s16 azimuth = tbl_cube_ele_azimuth[cube->ele[img3d_index[k]].sta] != 0 ? tbl_cube_ele_azimuth[cube->ele[img3d_index[k]].sta] : random_num;
                    s16 rotation = tbl_cube_ele_azimuth[cube->ele[img3d_index[k]].sta] == 900 ? random_num : 0;
                    widget_image3d_set_polar(cube->ele[img3d_index[k]].img3d, polar);
                    widget_image3d_set_azimuth(cube->ele[img3d_index[k]].img3d, azimuth);
                    widget_image3d_set_rotation_center(cube->ele[img3d_index[k]].img3d, cube->ele[img3d_index[k]].location.wid/2, cube->ele[img3d_index[k]].location.hei/2);
                    widget_image3d_set_rotation(cube->ele[img3d_index[k]].img3d, rotation);
//                    printf("%s-> img[%d, %d, %x] polar[%d], azimuth[%d] rotation[%d]\n", __func__,  i, j, img3d[k], polar, azimuth, rotation);
                }
            }
            ele_anim_sta = BATTER_3D_ANIM_STA_INC;
            break;

        case BATTER_3D_ANIM_STA_INC:
            alph+=5;

            if (alph >= 255) {
                alph = 255;
                ele_anim_sta = BATTER_3D_ANIM_STA_END;
            }

            for (int k=0; k<sizeof(img3d_index)/sizeof(img3d_index[0]); k++) {
                if (cube->ele[img3d_index[k]].img3d != NULL) {
                    widget_set_alpha(cube->ele[img3d_index[k]].img3d, alph);
                }
            }
            break;

        case BATTER_3D_ANIM_STA_END:
            ele_anim_sta = BATTER_3D_ANIM_STA_NONE;
            break;
        }
    }


    if (tick_check_expire(tick2, 100)) {
        tick2 = tick_get();
        //点击元素动画
        if (f_charge->click_sta) {
            switch (click_sta) {
            case BATTER_3D_CLICK_STA_NONE:
                width = width_max;
                if (cube->radius >= CUBE_RADIUS_MAX) {
                    click_sta = BATTER_3D_CLICK_STA_DEC;
                }
                break;

            case BATTER_3D_CLICK_STA_DEC:
                cube->radius--;
                //1:x = CUBE_RADIUS_MIN * 1414/1000 : (s32)cube->radius * 1414 / 1000;
                if ((((s32)cube->radius * 1414 / 1000) / (CUBE_RADIUS_MIN * 1414/1000)) <= width_max) {
                    width--;
                    if (width < 0) {
                        width = 0;
                    }
                }
                if (cube->radius <= CUBE_RADIUS_MIN) {
                    width = 0;
                    click_sta = BATTER_3D_CLICK_STA_STOP;
                }
                break;

            case BATTER_3D_CLICK_STA_INC:
                cube->radius++;

                if ((((s32)cube->radius * 1414 / 1000) / (CUBE_RADIUS_MIN * 1414/1000)) <= width_max) {
                    width++;
                    if (width > width_max) {
                        width = width_max;
                    }
                }

                if (cube->radius >= CUBE_RADIUS_MAX) {
                    width = width_max;
                    click_sta = BATTER_3D_CLICK_STA_END;
                }
                break;

            case BATTER_3D_CLICK_STA_END:
                f_charge->click_sta = false;
                click_sta = BATTER_3D_CLICK_STA_NONE;
                break;

            case BATTER_3D_CLICK_STA_STOP:
                stop_cnt++;
                if (stop_cnt > 5) {
                    click_sta = BATTER_3D_CLICK_STA_INC;
                    stop_cnt = 0;
                }
                break;
            }

            for (int i=0; i<CUBE_ITEM_CNT; i++) {
                widget_image3d_set_r(cube->item_img[i], cube->radius);
            }

            for (int i=0; i<CUBE_ITEM_ELE_CNT; i++) {
                if (cube->ele[i].img3d != NULL) {
                    widget_set_size(cube->ele[i].img3d, width, cube->ele[i].location.hei);
                }
            }
        }
    }
}

void func_charge_process(void)
{
    func_charge_process_do();
    func_process();
}

void func_charge_click(void)
{
    f_charge_t *f_charge = (f_charge_t *)func_cb.f_cb;
    if (f_charge->click_sta == false) {
        f_charge->click_sta = true;
    }
}

void func_charge_message(size_msg_t msg)
{
    compo_cube_t *cube = compo_getobj_byid(COMPO_ID_CUBE);

    point_t pt = ctp_get_sxy();
    s16 cube_limit_x = (GUI_SCREEN_WIDTH - 240) / 2;
    s16 cube_limit_y = (GUI_SCREEN_HEIGHT - 240) / 2;
    bool flag_cube_touch_x = (pt.x >= cube_limit_x) && (pt.x <= (cube_limit_x + 240));
    bool flag_cube_touch_y = (pt.y >= cube_limit_y) && (pt.y <= (cube_limit_y + 240));

    if (flag_cube_touch_x && flag_cube_touch_y) {
        if (msg == MSG_CTP_TOUCH) {
            compo_cube_t *cube = compo_getobj_byid(COMPO_ID_CUBE);
            //移动过程中，触屏停止。重新进入到开始拖动模式
            compo_cube_move_control(cube, COMPO_CUBE_MOVE_CMD_DRAG);
        } else if (msg >= MSG_CTP_SHORT_LEFT && msg <= MSG_CTP_SHORT_DOWN){
            return;
        }

    }
//    static bool time_visible = 0;
    switch (msg) {
    case MSG_CTP_CLICK:
        func_charge_click();
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
        //自动滚动处理
//        if (click == false) {
//            compo_cube_t *cube = compo_getobj_byid(COMPO_ID_CUBE);
//            compo_cube_move_control(cube, COMPO_CUBE_MOVE_CMD_FORWARD);
//        }

//    //秒跳动处理
//        {
//            {
//                compo_textbox_t *txt = compo_getobj_byid(COMPO_ID_TIME_DOT);
//                compo_textbox_set_visible(txt, time_visible);
//                time_visible ^= 1;
//            }
//        }
//
        break;

    default:
#if AUTO_MOVE_MODE
        compo_cube_move_control(cube, COMPO_CUBE_MOVE_CMD_AUTO);
#endif
        func_message(msg);
        break;
    }
}

void func_charge_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_charge_t));
    func_cb.frm_main = func_charge_form_create();
}

//退出地图功能
void func_charge_exit(void)
{
    func_cb.last = FUNC_CHARGE;
}

//地图功能
void func_charge(void)
{
    printf("%s\n", __func__);
    func_charge_enter();
    while (func_cb.sta == FUNC_CHARGE) {
        func_charge_process();
        func_charge_message(msg_dequeue());
    }
    func_charge_exit();
}

#endif


