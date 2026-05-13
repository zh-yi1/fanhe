#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define FOOTBALL_ROLL360_MODE           1                                   //是否采用任意角度滚动模式
#define FOOTBALL_RADIUS                 195                                 //足球半径

#define FOOTBALL_HALF_CIRCUM            ((int)(M_PI * FOOTBALL_RADIUS))     //圆周一半
#define FOOTBALL_MIN_POLAR              450                                 //足球的最大最小极角
#define FOOTBALL_MAX_POLAR              1350
#define FOOTBALL_ITEM_ANGLE             360
#define FOOTBALL_ENTER_ANGLE            360*6

//移动相关控制
#define ANIMATION_TICK_EXPIRE               18                          //动画单位时间Tick(ms)
#define ANIMATION_CNT_ENTERING              15                          //入场动画拍数
#define ANIMATION_CNT_EXITING               15                          //出场动画拍数
#define FOCUS_AUTO_STEP                     12                           //松手后自动对齐焦点单位时间步进
#define FOCUS_AUTO_STEP_DIV                 16
#define DRAG_AUTO_SPEED                     (FOOTBALL_ITEM_ANGLE * 100)  //拖动松手后的速度

//极角
static const u16 tbl_ball_polar[] = {
    372, 372, 372, 372, 372, 792, 792, 792, 792, 792,
    1008, 1008, 1008, 1008, 1008, 1428, 1428, 1428, 1428, 1428,
};

//方位角
static const u16 tbl_ball_azimuth[] = {
    0, 720, 1440, 2160, 2880, 0, 720, 1440, 2160, 2880,
    360, 1080, 1800, 2520, 3240, 360, 1080, 1800, 2520, 3240,
};

/**
 * @brief 创建一个足球菜单组件
 * @param[in] frm : 窗体指针
 * @param[in] item : 足球菜单项列表
 * @param[in] item_cnt : 足球菜单项列表数量，必须是20项
 * @return 返回足球菜单组件指针
 **/
compo_football_t *compo_football_create(compo_form_t *frm, s16 radius, compo_football_item_t const *item, int item_cnt)
{
    if (item_cnt != FOOTBALL_ITEM_CNT) {
        halt(HALT_GUI_COMPO_FOOTBALL_CREATE);
    }
    compo_football_t *ball = compo_create(frm, COMPO_TYPE_FOOTBALL);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    int i;
    widget_axis3d_t *axis = widget_axis3d_create(page);
    ball->page = page;
    ball->item = item;
    ball->flag_need_update = true;
    ball->axis = axis;
    for (i=0; i<FOOTBALL_ITEM_CNT; i++) {
        widget_image3d_t *img = widget_image3d_create(page, item[i].res_addr);
        ball->item_img[i] = img;

        widget_image3d_set_axis(img, axis);
        widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
        widget_image3d_set_r(img, radius);
        widget_image3d_set_polar(img, tbl_ball_polar[i]);
        widget_image3d_set_azimuth(img, tbl_ball_azimuth[i]);
    }
    ball->img_area = gui_image_get_size(item[0].res_addr);
    ball->sph.rotation = 900;
    ball->sph.polar = 900;
    ball->sph.azimuth = 900;
    compo_football_update(ball);
    return ball;
}

/**
 * @brief 更新足球菜单Widget
 * @param[in] ball : 足球菜单指针
 **/
void compo_football_update(compo_football_t *ball)
{
    int i;
    if (!ball->flag_need_update) {
        return;
    }
    widget_axis3d_t *axis = ball->axis;
    ball->flag_need_update = false;
    //printf("ball: %d %d %d\n", ball->sph.polar, ball->sph.azimuth, ball->sph.rotation);
    widget_axis3d_set_sph(axis, ball->sph);
    for (i=0; i<FOOTBALL_ITEM_CNT; i++) {
        widget_image3d_t *img = ball->item_img[i];
        widget_set_visible(img, widget_image3d_is_front(img));
    }
}

/**
 * @brief 设置足球菜单的旋转角度
 * @param[in] ball : 足球菜单指针
 * @param[in] angle : 旋转角度
 **/
s32 compo_football_set_rotation(compo_football_t *ball, s32 angle)
{
    if (angle < 0) {
        angle = 3600 - (-angle % 3600);
    }
    if (angle >= 3600) {
        angle = angle % 3600;
    }
    if (ball->sph.rotation != angle) {
        ball->sph.rotation = angle;
        ball->flag_need_update = true;
    }
    return angle;
}

/**
 * @brief 足球菜单滚动
 * @param[in] ball : 足球菜单指针
 * @param[in] angle : 旋转角度
 **/
void compo_football_roll(compo_football_t *ball, s16 roll_polar)
{
    compo_football_move_cb_t *mcb = &ball->move_cb;
    ball->sph = widget_axis3d_roll(ball->axis, roll_polar, mcb->roll_azimuth);
    ball->flag_need_update = true;
}

/**
 * @brief 足球菜单滚动
 * @param[in] ball : 足球菜单指针
 * @param[in] angle : 旋转角度
 **/
void compo_football_roll_from(compo_football_t *ball, s16 roll_polar, s16 roll_azimuth)
{
    compo_football_move_cb_t *mcb = &ball->move_cb;
    ball->sph = widget_axis3d_roll_from(ball->axis, mcb->focus_sph, roll_polar, roll_azimuth);
    ball->flag_need_update = true;
}

/**
 * @brief 设置足球菜单的极角
 * @param[in] ball : 足球菜单指针
 * @param[in] angle : 极角
 **/
s32 compo_football_set_polar(compo_football_t *ball, s32 angle)
{
    if (angle < FOOTBALL_MIN_POLAR) {
        angle = FOOTBALL_MIN_POLAR;
    } else if (angle > FOOTBALL_MAX_POLAR) {
        angle = FOOTBALL_MAX_POLAR;
    }
    if (ball->sph.polar != angle) {
        ball->sph.polar = angle;
        ball->flag_need_update = true;
    }
    return angle;
}

/**
 * @brief 足球菜单拖动与移动初始化
 * @param[in] ball : 足球菜单指针
 * @return 返回COMPO_FOOTBALL_STA状态枚举
 **/
u8 compo_football_get_sta(compo_football_t *ball)
{
    compo_football_move_cb_t *mcb = &ball->move_cb;
    if (mcb == NULL) {
        return COMPO_FOOTBALL_STA_IDLE;
    }
    if (mcb->flag_drag) {
        return COMPO_FOOTBALL_STA_DARG;
    } else if (mcb->flag_move_auto) {
        return COMPO_FOOTBALL_STA_MOVE;
    } else {
        return COMPO_FOOTBALL_STA_IDLE;
    }
}

/**
 * @brief 获取足球菜单项编号
 * @param[in] ball : 足球菜单指针
 * @param[in] x:屏幕坐标x
 * @param[in] y:屏幕坐标y
 * @return 返回菜单编号
 **/
int compo_football_get_idx(compo_football_t *ball, s16 x, s16 y)
{
    int i;
    for (i=0; i<FOOTBALL_ITEM_CNT; i++) {
        widget_image3d_t *img = ball->item_img[i];
        if (widget_image3d_is_front(img) && widget_image3d_contains(img, x, y)) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 足球菜单拖动与移动处理
 * @param[in] ball : 足球菜单指针
 **/
__attribute__((optimize("-O3")))
void compo_football_move(compo_football_t *ball)
{
    compo_football_move_cb_t *mcb = &ball->move_cb;
    if (mcb == NULL) {
        return;
    }
    if (mcb->flag_drag) {
        s32 dx, dy, ax, ay;
        mcb->flag_drag = ctp_get_dxy(&dx, &dy);
        if (mcb->flag_drag) {
            //拖动菜单图标
            ax = dx * 1800 / FOOTBALL_HALF_CIRCUM;
            ay = dy * 1800 / FOOTBALL_HALF_CIRCUM;
#if FOOTBALL_ROLL360_MODE
            int rp = sqrt64(ax * ax + ay * ay);
            int ra = ARCTAN2(-ay, ax);
            compo_football_roll_from(ball, rp, ra);
#else
            compo_football_set_polar(ball, mcb->focus_sph.polar - ay);
            compo_football_set_rotation(ball, mcb->focus_sph.rotation - ax);
#endif
            compo_football_update(ball);
        } else {
            //抬手后开始自动移动
            point_t last_dxy = ctp_get_last_dxy();
            int da;
#if FOOTBALL_ROLL360_MODE
            mcb->roll_azimuth = ARCTAN2(-last_dxy.y, last_dxy.x);
            da = sqrt64(last_dxy.x * last_dxy.x + last_dxy.y * last_dxy.y);
#else
            da = last_dxy.x;
#endif
            mcb->moveto_a = da * DRAG_AUTO_SPEED / FOOTBALL_HALF_CIRCUM;
            mcb->flag_move_auto = true;
            mcb->start_a = 0;
            mcb->tick = tick_get();
        }
    }
    if (mcb->flag_move_auto) {
        //自动移动
        if (mcb->start_a == mcb->moveto_a) {
            mcb->flag_move_auto = false;              //移动完成
            compo_football_update(ball);
        } else if (tick_check_expire(mcb->tick, ANIMATION_TICK_EXPIRE)) {
            s32 da;
            mcb->tick = tick_get();
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
#if FOOTBALL_ROLL360_MODE
            compo_football_roll(ball, da);
#else
            compo_football_set_rotation(ball, ball->sph.rotation - da);
#endif
            compo_football_update(ball);
        }
    }
}

/**
 * @brief 足球菜单拖动与移动控制
 * @param[in] ball : 足球菜单指针
 * @param[in] cmd : 控制命令
 **/
void compo_football_move_control(compo_football_t *ball, int cmd)
{
    compo_football_move_cb_t *mcb = &ball->move_cb;
    if (mcb == NULL) {
        return;
    }
    switch (cmd) {
    case COMPO_FOOTBALL_MOVE_CMD_DRAG:
        //开始拖动
        mcb->flag_drag = true;
        mcb->flag_move_auto = false;
        mcb->focus_sph = ball->sph;
        break;

    case COMPO_FOOTBALL_MOVE_CMD_FORWARD:
        //向前滚动
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->start_a = 0;
            mcb->moveto_a = 0;
            mcb->roll_azimuth = 0;
        }
        mcb->moveto_a += FOOTBALL_ITEM_ANGLE;
        break;

    case COMPO_FOOTBALL_MOVE_CMD_BACKWARD:
        //向后滚动
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->start_a = 0;
            mcb->moveto_a = 0;
            mcb->roll_azimuth = 0;
        }
        mcb->moveto_a -= FOOTBALL_ITEM_ANGLE;
        break;

    case COMPO_FOOTBALL_MOVE_CMD_ENTER:
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->start_a = 0;
            mcb->moveto_a = 0;
            mcb->roll_azimuth = 0;
        }
        mcb->moveto_a += FOOTBALL_ENTER_ANGLE;
        break;

    default:
        halt(HALT_GUI_COMPO_FOOTBALL_MOVE_CMD);
        break;
    }
}

