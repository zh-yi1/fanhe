#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define BUTTERFLY_RADIUS                 140                                 //蝴蝶半径
#define BUTTERFLY_HALF_CIRCUM            ((int)(M_PI * BUTTERFLY_RADIUS))     //圆周一半

#define BUTTERFLY_ITEM_ANGLE             3600
#define BUTTERFLY_ITEM_FORWARD           100                            //点击向前滚动的距离

#define BUTTERFLY_MIN_WING                  -600                        //蝴蝶的最大最小翅膀角
#define BUTTERFLY_MAX_WING                  600
#define BUTTERFLY_MAX_FLAP                  (BUTTERFLY_MAX_WING - BUTTERFLY_MIN_WING)

//移动相关控制
#define ANIMATION_TICK_EXPIRE               18                          //动画单位时间Tick(ms)
#define ANIMATION_CNT_ENTERING              15                          //入场动画拍数
#define ANIMATION_CNT_EXITING               15                          //出场动画拍数
#define FOCUS_AUTO_STEP                     3                           //松手后自动对齐焦点单位时间步进
#define FOCUS_AUTO_STEP_DIV                 16
#define DRAG_AUTO_SPEED                     (BUTTERFLY_ITEM_ANGLE * 80)  //拖动松手后的速度

enum{
    BUTTERFLY_STA_FLY_FORWARD = 1,
    BUTTERFLY_STA_FLY_BACKWARD,
};
/**
 * @brief 创建一个蝴蝶组件
 * @param[in] frm : 窗体指针
 * @param[in] res : 蝴蝶图像资源
 * @return 返回蝴蝶组件指针
 **/
compo_butterfly_t *compo_butterfly_create(compo_form_t *frm, compo_butterfly_res_t const *res)
{
    compo_butterfly_t *btfly = compo_create(frm, COMPO_TYPE_BUTTERFLY);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    widget_image3d_t *img;
    widget_axis3d_t *axis = widget_axis3d_create(page);
    area_t area;
    btfly->page = page;
    btfly->flag_need_update = true;

    btfly->axis = axis;
    btfly->overlook = 300;
    widget_axis3d_set_overlook(axis, btfly->overlook);
    widget_axis3d_set_distance(axis, 600);
    widget_axis3d_set_polar(axis, 0);
    widget_axis3d_set_azimuth(axis, 0);
    widget_axis3d_set_rotation(axis, 0);

    widget_axis3d_set_pos(axis, 0, 0, 0);

    img = widget_image3d_create(page, res->res_body);
    widget_image3d_set_axis(img, axis);
    btfly->img_body = img;
    widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    widget_image3d_set_rotation_center(img, 12, 63);
    widget_image3d_set_polar(img, 900);
    widget_image3d_set_azimuth(img, 900);
    widget_image3d_set_rotation(img, 1800);

    area = gui_image_get_size(res->res_wing_up);
    img = widget_image3d_create(page, res->res_wing_up);
    widget_image3d_set_axis(img, axis);
    btfly->img_wing_ul = img;
    widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    widget_image3d_set_rotation_center(img, 0, area.hei);
    widget_image3d_set_polar(img, 900);
    widget_image3d_set_azimuth(img, 2700);
    widget_image3d_set_rotation(img, 1800);

    img = widget_image3d_create(page, res->res_wing_up);
    widget_image3d_set_axis(img, axis);
    btfly->img_wing_ur = img;
    widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    widget_image3d_set_rotation_center(img, 0, area.hei);
    widget_image3d_set_polar(img, 900);
    widget_image3d_set_azimuth(img, 900);
    widget_image3d_set_rotation(img, 1800);

    img = widget_image3d_create(page, res->res_wing_down);
    widget_image3d_set_axis(img, axis);
    btfly->img_wing_dl = img;
    widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    widget_image3d_set_rotation_center(img, 0, 0);
    widget_image3d_set_polar(img, 900);
    widget_image3d_set_azimuth(img, 2700);
    widget_image3d_set_rotation(img, 1800);

    img = widget_image3d_create(page, res->res_wing_down);
    widget_image3d_set_axis(img, axis);
    btfly->img_wing_dr = img;
    widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    widget_image3d_set_rotation_center(img, 0, 0);
    widget_image3d_set_polar(img, 900);
    widget_image3d_set_azimuth(img, 900);
    widget_image3d_set_rotation(img, 1800);

    btfly->wing_dir = 1;                //先往下

    compo_butterfly_update(btfly);
    return btfly;
}

/**
 * @brief 更新蝴蝶Widget
 * @param[in] btfly : 蝴蝶指针
 **/
void compo_butterfly_update(compo_butterfly_t *btfly)
{
    int wing_up_angle = btfly->wing_up_angle;
    int wing_down_angle = btfly->wing_down_angle;
    widget_image3d_t *img;
    if (!btfly->flag_need_update) {
        return;
    }
    btfly->flag_need_update = false;

    int wing_up_polar = (wing_up_angle / 3);
    int wing_down_polar = -(wing_down_angle / 4);
    bool flag_top_body = false;
    bool flag_top_ul = false;
    bool flag_top_ur = false;
    bool flag_top_dl = false;
    bool flag_top_dr = false;
    int rotation = btfly->rotation;
    static int rotation_bak;

    widget_axis3d_t *axis = btfly->axis;

    if (rotation_bak != rotation) {
        rotation_bak = rotation;
        if ((rotation > 0 && rotation <= 900) || (rotation > 2700 && rotation < 3600)) {
            btfly->z -= 1;
            btfly->overlook -= 1;
        } else if (rotation > 900 && rotation < 2700){
            btfly->z += 1;
            btfly->overlook += 1;
        }

    }

    widget_axis3d_set_overlook(axis, btfly->overlook);
    widget_axis3d_set_pos(btfly->axis, 0, 0, btfly->z);
    if (rotation >= 450 && rotation <= 1350) {
        flag_top_body = true;
        flag_top_ur = true;
        flag_top_dr = true;
    } else if (rotation > 1350 && rotation < 2250) {
        flag_top_ul = true;
        flag_top_ur = true;
    } else if (rotation >= 2250 && rotation <= 3150) {
        flag_top_body = true;
        flag_top_ul = true;
        flag_top_dl = true;
    }

    widget_axis3d_set_polar(axis, -rotation);


    img = btfly->img_body;
    widget_set_top(img, flag_top_body);

    img = btfly->img_wing_ul;
    widget_image3d_set_azimuth(img, 2700 - wing_up_angle);
    widget_image3d_set_polar(img, 900 - wing_up_polar);
    widget_set_top(img, flag_top_ul);

    img = btfly->img_wing_ur;
    widget_image3d_set_azimuth(img, 900 + wing_up_angle);
    widget_image3d_set_polar(img, 900 + wing_up_polar);
    widget_set_top(img, flag_top_ur);

    img = btfly->img_wing_dl;
    widget_image3d_set_azimuth(img, 2700 - wing_down_angle);
    widget_image3d_set_polar(img, 900 - wing_down_polar);
    widget_set_top(img, flag_top_dl);

    img = btfly->img_wing_dr;
    widget_image3d_set_azimuth(img, 900 + wing_down_angle);
    widget_image3d_set_polar(img, 900 + wing_down_polar);
    widget_set_top(img, flag_top_dr);
}

/**
 * @brief 设置蝴蝶的旋转角度
 * @param[in] btfly : 蝴蝶指针
 * @param[in] angle : 旋转角度
 **/
s32 compo_butterfly_set_rotation(compo_butterfly_t *btfly, s32 angle)
{
    if (angle < 0) {
        angle = 3600 - (-angle % 3600);
    }
    if (angle >= 3600) {
        angle = angle % 3600;
    }
    if (btfly->rotation != angle) {
        btfly->rotation = angle;
        btfly->flag_need_update = true;
    }
    return angle;
}

/**
 * @brief 设置蝴蝶的球Z坐标
 * @param[in] btfly : 蝴蝶指针
 * @param[in] angle : 旋转角度
 **/
s16 compo_butterfly_set_overlook(compo_butterfly_t *btfly, s16 overlook)
{
    if (btfly->overlook != overlook) {
        btfly->overlook = overlook;
        btfly->flag_need_update = true;
    }
    return overlook;
}

/**
 * @brief 设置蝴蝶的球z坐标
 * @param[in] btfly : 蝴蝶指针
 * @param[in] angle : 旋转角度
 **/
s16 compo_butterfly_set_z(compo_butterfly_t *btfly, s16 z)
{
    if (btfly->z != z) {
        btfly->z = z;
        btfly->flag_need_update = true;
    }
    return z;
}

/**
 * @brief 设置蝴蝶的旋转角度
 * @param[in] btfly : 蝴蝶指针
 * @param[in] angle : 旋转角度
 **/
s32 compo_butterfly_get_rotation(compo_butterfly_t *btfly)
{
    return btfly->rotation;
}

/**
 * @brief 获取蝴蝶的球Z坐标
 * @param[in] btfly : 蝴蝶指针
 * @param[in] angle : 旋转角度
 **/
s16 compo_butterfly_get_z(compo_butterfly_t *btfly)
{
    return btfly->z;
}

/**
 * @brief 扇动蝴蝶的翅膀
 * @param[in] btfly : 蝴蝶指针
 * @param[in] angle : 翅膀角度
 **/
void compo_butterfly_flap(compo_butterfly_t *btfly, s16 angle)
{
    if (angle >= BUTTERFLY_MAX_FLAP) {
        return;
    }
    int wing_up_angle = btfly->wing_up_angle;
    int wing_down_angle = btfly->wing_down_angle;
    int wing_up_step = angle;
    int wing_down_step1 = angle * 3 / 4;
    int wing_down_step2 = angle * 7 / 8;
    int wing_delta_step = wing_down_step1 / 2;
    if (btfly->wing_dir) {
        if (wing_up_angle >= wing_down_angle) {
            wing_up_angle += wing_up_step;
            wing_down_angle += wing_down_step1;
        } else {
            wing_down_angle -= wing_down_step2;
            if (wing_down_angle <= BUTTERFLY_MIN_WING) {
                wing_down_angle = BUTTERFLY_MIN_WING;
            }
            wing_up_angle += wing_delta_step;
        }
        if (wing_up_angle >= BUTTERFLY_MAX_WING) {
            wing_up_angle = BUTTERFLY_MAX_WING;
            btfly->wing_dir = 0;
        }
    } else {
        if (wing_up_angle <= wing_down_angle) {
            wing_up_angle -= wing_up_step;
            wing_down_angle -= wing_down_step2;
        } else {
            wing_down_angle += wing_down_step1;
            if (wing_down_angle >= BUTTERFLY_MAX_WING) {
                wing_down_angle = BUTTERFLY_MAX_WING;
            }
            wing_up_angle -= wing_delta_step;
        }
        if (wing_up_angle <= BUTTERFLY_MIN_WING) {
            wing_up_angle = BUTTERFLY_MIN_WING;
            btfly->wing_dir = 1;
        }
    }
    btfly->wing_up_angle = wing_up_angle;
    btfly->wing_down_angle = wing_down_angle;
    btfly->flag_need_update = true;
}

/**
 * @brief 蝴蝶拖动与移动初始化
 * @param[in] btfly : 蝴蝶指针
 * @return 返回COMPO_BUTTERFLY_STA状态枚举
 **/
u8 compo_butterfly_get_sta(compo_butterfly_t *btfly)
{
    compo_butterfly_move_cb_t *mcb = &btfly->move_cb;
    if (mcb == NULL) {
        return COMPO_BUTTERFLY_STA_IDLE;
    }
    if (mcb->flag_drag) {
        return COMPO_BUTTERFLY_STA_DARG;
    } else if (mcb->flag_move_auto || mcb->flag_move_z_auto == BUTTERFLY_STA_FLY_FORWARD) {
        return COMPO_BUTTERFLY_STA_MOVE;
    } else {
        return COMPO_BUTTERFLY_STA_IDLE;
    }
}

/**
 * @brief 蝴蝶拖动与移动处理
 * @param[in] btfly : 蝴蝶指针
 **/
void compo_butterfly_move(compo_butterfly_t *btfly)
{
    compo_butterfly_move_cb_t *mcb = &btfly->move_cb;
    if (mcb == NULL) {
        return;
    }
    if (mcb->flag_drag) {
//        s32 dx, dy, da, di;
//        mcb->flag_drag = ctp_get_dxy(&dx, &dy);
        if (mcb->flag_drag) {
            //拖动菜单图标
//            da = dx * 1800 / BUTTERFLY_HALF_CIRCUM;
//            di = dy * 1800 / BUTTERFLY_HALF_CIRCUM;
            //compo_butterfly_set_rotation(btfly, mcb->focus_a - da);
            compo_butterfly_update(btfly);
        } else {
            //抬手后开始自动移动
//            s32 last_dx;
//            last_dx = ctp_get_last_dxy().x;
//            mcb->focus_a = btfly->rotation;
//            mcb->flag_move_auto = true;
//            mcb->moveto_a = mcb->focus_a - (last_dx * DRAG_AUTO_SPEED / BUTTERFLY_HALF_CIRCUM);
            mcb->tick = tick_get();
        }
    }
    if (mcb->flag_move_auto) {
        //自动移动
        if (mcb->focus_a == mcb->moveto_a) {
            mcb->flag_move_auto = false;              //移动完成
            compo_butterfly_update(btfly);
        } else if (tick_check_expire(mcb->tick, ANIMATION_TICK_EXPIRE)) {
            s32 da = 0;
            mcb->tick = tick_get();
            da = mcb->moveto_a - mcb->focus_a;
			da = da > 0 ? 10 : -10;
            mcb->focus_a += da;
            compo_butterfly_set_rotation(btfly, mcb->focus_a);
            compo_butterfly_update(btfly);
        }
    } else {

        if (mcb->flag_move_z_auto) {
            //自动移动
            if (mcb->focus_z == mcb->moveto_z) {
                compo_butterfly_update(btfly);
                mcb->moveto_z = 0;
                if (mcb->flag_move_z_auto == 1) {
                    mcb->flag_move_z_auto = 2;
                } else if (mcb->flag_move_z_auto == 2) {
                    mcb->flag_move_z_auto = 0;              //移动完成
                }
            } else if (tick_check_expire(mcb->tick, ANIMATION_TICK_EXPIRE)) {
                s32 da = 0;
                mcb->tick = tick_get();
                da = mcb->moveto_z - mcb->focus_z;
                da = da > 0 ? 1 : -1;
                mcb->focus_z += da;
                compo_butterfly_set_overlook(btfly, btfly->overlook + da);
                compo_butterfly_set_z(btfly, mcb->focus_z);
                compo_butterfly_update(btfly);
            }
        }
    }
}

/**
 * @brief 蝴蝶拖动与移动控制
 * @param[in] btfly : 蝴蝶指针
 * @param[in] cmd : 控制命令
 **/
void compo_butterfly_move_control(compo_butterfly_t *btfly, int cmd)
{
    compo_butterfly_move_cb_t *mcb = &btfly->move_cb;
    if (mcb == NULL) {
        return;
    }
    switch (cmd) {
    case COMPO_BUTTERFLY_MOVE_CMD_DRAG:
        //开始拖动
//        mcb->flag_drag = true;
//        mcb->flag_move_auto = false;
//        mcb->focus_a = btfly->rotation;
        break;

    case COMPO_BUTTERFLY_MOVE_CMD_LEFT:
        //向左滚动
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->focus_a = btfly->rotation;
            mcb->moveto_a = mcb->focus_a;
            mcb->moveto_a = -BUTTERFLY_ITEM_ANGLE;
        }

        break;

    case COMPO_BUTTERFLY_MOVE_CMD_LOOP:
        //向前滚动

        if (!mcb->flag_move_z_auto) {
            mcb->flag_move_z_auto = BUTTERFLY_STA_FLY_FORWARD;
            mcb->focus_z = 0;
            mcb->moveto_z = mcb->focus_z;
            mcb->moveto_z = -BUTTERFLY_ITEM_FORWARD;
        }


        break;

    case COMPO_BUTTERFLY_MOVE_CMD_RIGHT:
        //向右滚动
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->focus_a = btfly->rotation;
            mcb->moveto_a = mcb->focus_a;
            mcb->moveto_a = BUTTERFLY_ITEM_ANGLE;
        }

        break;

    default:
        halt(HALT_GUI_COMPO_BUTTERFLY_MOVE_CMD);
        break;
    }
}

/**
 * @brief 设置蝴蝶的坐标及大小
          注意：蝴蝶设置的坐标是以默认的中心点作为参考
 * @param[in] butterfly : 蝴蝶指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 宽度
 * @param[in] height : 高度
 **/
void compo_butterfly_set_location(compo_butterfly_t *btfly, s16 x, s16 y, s16 width, s16 height)
{
    widget_set_location(btfly->page, x, y, width, height);
}

/**
 * @brief 设置蝴蝶的坐标
          注意：蝴蝶设置的坐标是以默认的中心点作为参考
 * @param[in] label : 蝴蝶指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_butterfly_set_pos(compo_butterfly_t *btfly, s16 x, s16 y)
{
    widget_set_pos(btfly->page, x, y);
}


