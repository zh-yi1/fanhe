#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define CUBE_ROLL360_MODE                   1                                   //是否采用任意角度滚动模式

#define CUBE_HALF_CIRCUM(x)                 ((int)(M_PI * x))         //圆周一半
#define CUBE_MIN_POLAR                      0                                   //立方体的最大最小极角
#define CUBE_MAX_POLAR                      3600
#define CUBE_ITEM_ANGLE                     360

//移动相关控制
#define ANIMATION_TICK_EXPIRE               18                                  //动画单位时间Tick(ms)
#define ANIMATION_CNT_ENTERING              15                                  //入场动画拍数
#define ANIMATION_CNT_EXITING               15                                  //出场动画拍数
#define FOCUS_AUTO_STEP                     5                                   //松手后自动对齐焦点单位时间步进
#define FOCUS_AUTO_STEP_DIV                 16
#define DRAG_AUTO_SPEED                     (CUBE_ITEM_ANGLE * 80)              //拖动松手后的速度

#define MOVE_AUTO_STEP                      20
#define MOVE_AUTO_SPEED                     MOVE_AUTO_STEP*5


//    {UI_BUF_ICON_CUBE_CALL_BIN,             FUNC_CALL},
//    {UI_BUF_ICON_CUBE_HEART_RATE_BIN,       FUNC_HEARTRATE},
//    {UI_BUF_ICON_CUBE_MUSIC_BIN,            FUNC_BT},
////    {UI_BUF_ICON_CUBE_SLEEP_BIN,            FUNC_SLEEP},
////    {UI_BUF_ICON_CUBE_SPORT_BIN,            FUNC_SPORT},
////    {UI_BUF_ICON_CUBE_STEP_BIN,             FUNC_SLEEP},

//极角
static const u16 tbl_cube_polar[] = {
//    372, 372, 372, 372, 372, 792, 792, 792, 792, 792,
//    1008, 1008, 1008, 1008, 1008, 1428, 1428, 1428, 1428, 1428,
    0, 900, 900, 900, 900, 1799
};

//方位角
static const u16 tbl_cube_azimuth[] = {
//    0, 720, 1440, 2160, 2880, 0, 720, 1440, 2160, 2880,
//    360, 1080, 1800, 2520, 3240, 360, 1080, 1800, 2520, 3240,
    1800, 900, 1800, 2700, 0, 0
};


/**
 * 设置compo_cube的类型
 * @param cube 指向compo_cube结构体的指针，用于指定哪个立方体的类型需要被设置
 * @param type 要设置的立方体类型，使用uint8_t表示
 * 此函数用于更新compo_cube结构体中的类型信息如果传入的指针为NULL，将调用halt函数
 * 停止程序运行，以避免潜在的程序崩溃或未定义行为
 */
void compo_cube_set_type(compo_cube_t *cube, uint8_t type)
{
    if (cube == NULL) {             // 检查传入的指针是否为NULL，如果为NULL则停止程序运行
        halt(HALT_GUI_COMPO_CUBE);
        return;
    }
    cube->cube_type = type;              // 更新立方体的类型
    cube->flag_need_update = true;
    compo_cube_update(cube);
}

void compo_cube_add_element(compo_cube_t *cube, s16 radius, u32 ele_res, int ele_cnt)
{

    if (cube->cube_type == COMPO_CUBE_TYPE_NORMAL) {
        halt(HALT_GUI_COMPO_CUBE);
        return;
    }

    if (ele_cnt > sizeof(cube->ele)/sizeof(cube->ele[0])) {
        ele_cnt = sizeof(cube->ele)/sizeof(cube->ele[0]);
    }

//    cube->ele = item_ele;
    for(int i=0; i<ele_cnt; i++) {
        cube->ele[i].radius = radius;
        cube->ele[i].res = ele_res;
        widget_image3d_t *img = widget_image3d_create(cube->page, ele_res);
//        cube->item_ele[i] = img;
        cube->ele[i].img3d = img;

        widget_image3d_set_axis(img, cube->axis);

        cube->ele[i].location.x = GUI_SCREEN_CENTER_X;
        cube->ele[i].location.y = GUI_SCREEN_CENTER_Y;
        cube->ele[i].location.wid = gui_image_get_size(ele_res).wid / 2;
        cube->ele[i].location.hei = gui_image_get_size(ele_res).hei;

        widget_set_location(img, cube->ele[i].location.x, cube->ele[i].location.y, cube->ele[i].location.wid, cube->ele[i].location.hei);
        widget_image3d_set_r(img, 0);

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


        //每个角度3个元素
        static int j = 0;
        if (i % ((ele_cnt/6) > 0 ? (ele_cnt/6) : 1) == 0 && i != 0) {
            j++;
            if (j >= sizeof(tbl_cube_ele_polar) / sizeof(tbl_cube_ele_polar[0])) {
                j = 0;
            }
        }

        cube->ele[i].sta = j;


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

        s16 polar = tbl_cube_ele_polar[j] != 0 ? tbl_cube_ele_polar[j] : random_num;
        s16 azimuth = tbl_cube_ele_azimuth[j] != 0 ? tbl_cube_ele_azimuth[j] : random_num;
        s16 rotation = tbl_cube_ele_azimuth[j] == 900 ? random_num : 0;
        widget_image3d_set_polar(img, polar);
        widget_image3d_set_azimuth(img, azimuth);
        widget_image3d_set_rotation_center(img, gui_image_get_size(ele_res).wid/2, gui_image_get_size(ele_res).hei/2);
        widget_image3d_set_rotation(img, rotation);
//        printf("%s-> img[%d, %d, %x] polar[%d], azimuth[%d] rotation[%d]\n", __func__,  i, j, img, polar, azimuth, rotation);
    }

    cube->flag_need_update = true;
    compo_cube_update(cube);
}

/**
 * @brief 创建一个立方体菜单组件
 * @param[in] frm : 窗体指针
 * @param[in] item : 立方体菜单项列表
 * @param[in] item_cnt : 立方体菜单项列表数量，必须是20项
 * @return 返回立方体菜单组件指针
 **/
compo_cube_t *compo_cube_create(compo_form_t *frm, s16 radius, compo_cube_item_t const *item, int item_cnt)
{
    if (item_cnt != CUBE_ITEM_CNT) {
        halt(HALT_GUI_COMPO_CUBE_CREATE);
    }
    compo_cube_t *cube = compo_create(frm, COMPO_TYPE_CUBE);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    int i;
    widget_axis3d_t *axis = widget_axis3d_create(page);
    if (cube->cube_type == COMPO_CUBE_TYPE_NONE) {
        cube->cube_type = COMPO_CUBE_TYPE_NORMAL;
    }
    cube->page = page;
    cube->item = item;
    cube->flag_need_update = true;
    cube->axis = axis;
    cube->radius = radius;
    widget_axis3d_set_distance(axis, 600);
    for (i=0; i<CUBE_ITEM_CNT; i++) {
        widget_image3d_t *img = widget_image3d_create(page, item[i].res_addr);
        cube->item_img[i] = img;

        widget_image3d_set_axis(img, axis);
        widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
        widget_image3d_set_r(img, radius);
        widget_image3d_set_polar(img, tbl_cube_polar[i]);
        widget_image3d_set_azimuth(img, tbl_cube_azimuth[i]);
    }
    cube->img_area = gui_image_get_size(item[0].res_addr);

    //初始角度
    cube->sph.rotation = 2250;
    cube->sph.polar = 550;
    cube->sph.azimuth = 920;
    compo_cube_update(cube);
    return cube;
}

/**
 * @brief 更新立方体菜单Widget
 * @param[in] cube : 立方体菜单指针
 **/
void compo_cube_update(compo_cube_t *cube)
{
    int i;
    if (!cube->flag_need_update) {
        return;
    }
    widget_axis3d_t *axis = cube->axis;
    cube->flag_need_update = false;
//    printf("cube: %d %d %d\n", cube->sph.polar, cube->sph.azimuth, cube->sph.rotation);
    widget_axis3d_set_sph(axis, cube->sph);
    for (i=0; i<CUBE_ITEM_CNT; i++) {
        widget_image3d_t *img = cube->item_img[i];
        if (cube->cube_type == COMPO_CUBE_TYPE_NORMAL) {
            widget_set_top(img, widget_image3d_is_front(img));
        } else if (cube->cube_type == COMPO_CUBE_TYPE_POWER) {
            widget_set_visible(img, true);
            widget_set_top(img, widget_image3d_is_front(img));
        }
    }
}

/**
 * @brief 设置立方体菜单的旋转角度
 * @param[in] cube : 立方体菜单指针
 * @param[in] angle : 旋转角度
 **/
s32 compo_cube_set_rotation(compo_cube_t *cube, s32 angle)
{
    if (angle < 0) {
        angle = 3600 - (-angle % 3600);
    }
    if (angle >= 3600) {
        angle = angle % 3600;
    }
    if (cube->sph.rotation != angle) {
        cube->sph.rotation = angle;
        cube->flag_need_update = true;
    }
    return angle;
}

/**
 * @brief 立方体菜单滚动
 * @param[in] cube : 立方体菜单指针
 * @param[in] angle : 旋转角度
 **/
void compo_cube_roll(compo_cube_t *cube, s16 roll_polar)
{
    compo_cube_move_cb_t *mcb = &cube->move_cb;
    cube->sph = widget_axis3d_roll(cube->axis, roll_polar, mcb->roll_azimuth);
    cube->flag_need_update = true;
}

/**
 * @brief 立方体菜单滚动
 * @param[in] cube : 立方体菜单指针
 * @param[in] angle : 旋转角度
 **/
void compo_cube_roll_from(compo_cube_t *cube, s16 roll_polar, s16 roll_azimuth)
{
    compo_cube_move_cb_t *mcb = &cube->move_cb;
    cube->sph = widget_axis3d_roll_from(cube->axis, mcb->focus_sph, roll_polar, roll_azimuth);
    cube->flag_need_update = true;
}

/**
 * @brief 设置立方体菜单的极角
 * @param[in] cube : 立方体菜单指针
 * @param[in] angle : 极角
 **/
s32 compo_cube_set_polar(compo_cube_t *cube, s32 angle)
{
    if (angle < CUBE_MIN_POLAR) {
        angle = CUBE_MIN_POLAR;
    } else if (angle > CUBE_MAX_POLAR) {
        angle = CUBE_MAX_POLAR;
    }
    if (cube->sph.polar != angle) {
        cube->sph.polar = angle;
        cube->flag_need_update = true;
    }
    return angle;
}

/**
 * @brief 立方体菜单拖动与移动初始化
 * @param[in] cube : 立方体菜单指针
 * @return 返回COMPO_CUBE_STA状态枚举
 **/
u8 compo_cube_get_sta(compo_cube_t *cube)
{
    compo_cube_move_cb_t *mcb = &cube->move_cb;
    if (mcb == NULL) {
        return COMPO_CUBE_STA_IDLE;
    }
    if (mcb->flag_drag) {
        return COMPO_CUBE_STA_DARG;
    } else if (mcb->flag_move_auto) {
        return COMPO_CUBE_STA_MOVE;
    } else {
        return COMPO_CUBE_STA_IDLE;
    }
}

/**
 * @brief 获取立方体菜单项编号
 * @param[in] cube : 立方体菜单指针
 * @param[in] x:屏幕坐标x
 * @param[in] y:屏幕坐标y
 * @return 返回菜单编号
 **/
int compo_cube_get_idx(compo_cube_t *cube, s16 x, s16 y)
{
    int i;
    for (i=0; i<CUBE_ITEM_CNT; i++) {
        widget_image3d_t *img = cube->item_img[i];
        if (widget_image3d_is_front(img) && widget_image3d_contains(img, x, y)) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 立方体菜单拖动与移动处理
 * @param[in] cube : 立方体菜单指针
 **/
void compo_cube_move(compo_cube_t *cube)
{
    compo_cube_move_cb_t *mcb = &cube->move_cb;
    if (mcb == NULL) {
        return;
    }
    if (mcb->flag_drag) {
        s32 dx, dy, ax, ay;
        mcb->flag_drag = ctp_get_dxy(&dx, &dy);
        if (mcb->flag_drag) {
            //拖动菜单图标
            ax = dx * 1800 / CUBE_HALF_CIRCUM(cube->radius);
            ay = dy * 1800 / CUBE_HALF_CIRCUM(cube->radius);
#if CUBE_ROLL360_MODE
            int rp = sqrt64(ax * ax + ay * ay);
            int ra = ARCTAN2(-ay, ax);
            compo_cube_roll_from(cube, rp, ra);
#else
            compo_cube_set_polar(cube, mcb->focus_sph.polar - ay);
            compo_cube_set_rotation(cube, mcb->focus_sph.rotation - ax);
#endif
            compo_cube_update(cube);
        } else {
            //抬手后开始自动移动
            point_t last_dxy = ctp_get_last_dxy();
            int da;
#if CUBE_ROLL360_MODE
            mcb->roll_azimuth = ARCTAN2(-last_dxy.y, last_dxy.x);
            da = sqrt64(last_dxy.x * last_dxy.x + last_dxy.y * last_dxy.y);
#else
            da = last_dxy.x;
#endif
            mcb->moveto_a = da * DRAG_AUTO_SPEED / CUBE_HALF_CIRCUM(cube->radius);
            mcb->flag_move_auto = true;
            mcb->start_a = 0;
            mcb->tick = tick_get();
        }
    }
    if (mcb->flag_move_auto) {
        //自动移动
        if (mcb->start_a == mcb->moveto_a) {
            if (mcb->cmd != COMPO_CUBE_MOVE_CMD_AUTO) {
                mcb->flag_intertia = false;
            }
            mcb->flag_move_auto = false;              //移动完成
            compo_cube_update(cube);
        } else if (tick_check_expire(mcb->tick, ANIMATION_TICK_EXPIRE)) {
            s32 da;
            mcb->tick = tick_get();

            if (mcb->cmd == COMPO_CUBE_MOVE_CMD_AUTO) {

                if (mcb->moveto_a - mcb->start_a > 0) {
                    da = MOVE_AUTO_STEP;
                } else {
                    da = -MOVE_AUTO_STEP;
                }
                mcb->start_a += da;
                if (abs(mcb->start_a - mcb->moveto_a) < MOVE_AUTO_STEP) {
                    mcb->start_a = mcb->moveto_a;
                }
            } else {
                da = mcb->moveto_a - mcb->start_a;
                if (da > 0) {
                    if (da > FOCUS_AUTO_STEP * FOCUS_AUTO_STEP_DIV) {
                        da = da / FOCUS_AUTO_STEP_DIV;
                    } else if (da > FOCUS_AUTO_STEP) {
                        da = FOCUS_AUTO_STEP;
                    }
                } else {
                    if (da < -FOCUS_AUTO_STEP * FOCUS_AUTO_STEP_DIV) {
                        da = da / FOCUS_AUTO_STEP_DIV;
                    } else if (da < -FOCUS_AUTO_STEP) {
                        da = -FOCUS_AUTO_STEP;
                    }
                }
                mcb->start_a += da;
            }

#if CUBE_ROLL360_MODE
            compo_cube_roll(cube, da);
#else
            compo_cube_set_rotation(cube, cube->sph.rotation - da);
#endif
            compo_cube_update(cube);
        }
    }
}

/**
 * @brief 立方体菜单拖动与移动控制
 * @param[in] cube : 立方体菜单指针
 * @param[in] cmd : 控制命令
 **/
void compo_cube_move_control(compo_cube_t *cube, int cmd)
{
    compo_cube_move_cb_t *mcb = &cube->move_cb;
    if (mcb == NULL) {
        return;
    }
    switch (cmd) {
    case COMPO_CUBE_MOVE_CMD_DRAG:
        //开始拖动
        mcb->flag_drag = true;
        mcb->flag_move_auto = false;
        mcb->focus_sph = cube->sph;
        mcb->flag_intertia = true;
        mcb->cmd = cmd;
        break;

    case COMPO_CUBE_MOVE_CMD_FORWARD:
        //向前滚动
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->start_a = 0;
            mcb->moveto_a = 0;
            mcb->roll_azimuth = 0;
            mcb->flag_intertia = true;
        }
        mcb->moveto_a += CUBE_ITEM_ANGLE;
        mcb->cmd = cmd;
        break;

    case COMPO_CUBE_MOVE_CMD_BACKWARD:
        //向后滚动
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->start_a = 0;
            mcb->moveto_a = 0;
            mcb->roll_azimuth = 0;
            mcb->flag_intertia = true;
        }
        mcb->moveto_a -= CUBE_ITEM_ANGLE;
        mcb->cmd = cmd;
        break;

    case COMPO_CUBE_MOVE_CMD_AUTO:
        if (!mcb->flag_move_auto && !mcb->flag_intertia) {
            mcb->flag_move_auto = true;
            mcb->start_a = 0;
            mcb->moveto_a = 0;
//            mcb->roll_azimuth = 0;
            mcb->moveto_a += CUBE_ITEM_ANGLE;
            mcb->cmd = cmd;
        }

        break;

    default:
        halt(HALT_GUI_COMPO_CUBE_MOVE_CMD);
        break;
    }
}

void compo_cube_set_pos(compo_cube_t *cube, s16 x, s16 y)
{
    widget_set_pos(cube->page, x, y);
}


