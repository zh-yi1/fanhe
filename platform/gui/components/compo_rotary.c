#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define ROTARY_SIGHT_DISTANCE           1280                            //视距
#define ROTARY_OVERLOOK                 150                             //默认视角15度
//#define ROTARY_RADIUS                   200                             //转盘半径
//#define ROTARY_HALF_CIRCUM              ((int)(M_PI * ROTARY_RADIUS))   //圆周一半
#define ROTARY_FIX_ANGLE                845                             //从正前顺时针转过多少度后切图

//移动相关控制
#define ANIMATION_TICK_EXPIRE               18                          //动画单位时间Tick(ms)
#define ANIMATION_CNT_ENTERING              15                          //入场动画拍数
#define ANIMATION_CNT_EXITING               15                          //出场动画拍数
#define FOCUS_AUTO_STEP                     3                           //松手后自动对齐焦点单位时间步进
#define FOCUS_AUTO_STEP_DIV                 12
#define DRAG_AUTO_SPEED                     80                          //拖动松手后的速度

/**
 * @brief 创建一个转盘组件
 * @param[in] frm : 窗体指针
 * @param[in] item : 存放转盘相关的信息，图标等
 * @param[in] item_cnt : 存放转盘中信息的数量
 * @return 返回转盘指针
 **/
compo_rotary_t *compo_rotary_create(compo_form_t *frm, compo_rotary_item_t const *item, int item_cnt, u16 radius)
{
    compo_rotary_t *rotary = compo_create(frm, COMPO_TYPE_ROTARY);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    widget_axis3d_t *axis = widget_axis3d_create(page);
    rotary->page = page;
    rotary->item = item;
    rotary->item_cnt = item_cnt;
    rotary->total_angle = ROTARY_ITEM_ANGLE * item_cnt;
    rotary->axis = axis;
    rotary->radius = radius;

    if (item_cnt <= 0) {
        halt(HALT_GUI_COMPO_ROTARY_CREATE);
    }
#if BOX_GUI_ROTATE_DISP
    sph_t sph = {radius, 900 - ROTARY_OVERLOOK, 0, 1800};
    widget_axis3d_set_sph(axis, sph);
#else
    int y = muls_shift16(-rotary->radius, SIN(ROTARY_OVERLOOK));
    widget_axis3d_set_distance(axis, ROTARY_SIGHT_DISTANCE);
    widget_axis3d_set_overlook(axis, ROTARY_OVERLOOK);
    widget_axis3d_set_pos(axis, 0, y, -rotary->radius);
#endif
    for (int i=0; i<ROTARY_ITEM_CNT; i++) {
        widget_image3d_t *img = widget_image3d_create(page, 0);
        rotary->item_img[i] = img;

        widget_image3d_set_axis(img, axis);
        widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
        widget_image3d_set_r(img, rotary->radius);
    }
    widget_text_t *txt = widget_text_create(page, 32);
    widget_text_set_autosize(txt, true);
#if BOX_GUI_ROTATE_DISP
	rotate_widget_set_pos(txt, GUI_GET_SCREEN_CENTER_X, GUI_GET_SCREEN_HEIGHT - widget_text_get_max_height());
#else
    widget_set_pos(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 48);
#endif

    rotary->item_title = txt;
    rotary->img_area = gui_image_get_size(item[0].res_addr);
    return rotary;
}

/**
 * @brief 更新转盘Widget
 * @param[in] rotary : 转盘指针
 **/
void compo_rotary_update(compo_rotary_t *rotary)
{
    int i;
    int sidx;
    int s_angle = rotary->angle - ROTARY_FIX_ANGLE + ROTARY_ITEM_ANGLE;

    if (s_angle < 0) {
        s_angle += rotary->total_angle;
    }
    sidx = s_angle / ROTARY_ITEM_ANGLE;
    s_angle = (sidx + 1) * ROTARY_ITEM_ANGLE - s_angle - ROTARY_FIX_ANGLE;

    for (i=0; i<ROTARY_ITEM_CNT; i++) {
        int idx = sidx + (ROTARY_ITEM_CNT - 1 - i);
        s32 angle = s_angle + (ROTARY_ITEM_CNT - 1 - i) * ROTARY_ITEM_ANGLE;
        while (idx >= rotary->item_cnt) {       //防止当减了之后还是大于item_cnt, 导致显示不正常
            WDT_CLR();
            idx -= rotary->item_cnt;
        }
        widget_image3d_t *img = rotary->item_img[i];
        compo_rotary_item_t const *item = &(rotary->item[idx]);
        widget_image3d_set(img, item->res_addr);
        widget_image3d_set_azimuth(img, angle);
        if (abs(angle) < ROTARY_ITEM_ANGLE / 2) {
            widget_text_set(rotary->item_title, i18n[item->str_idx]);
        }
//        printf("idxl [%d], res [%x]\n",idx, item->res_addr);
    }
}

/**
 * @brief 设置转盘的旋转角度
 * @param[in] rotary : 转盘指针
 * @param[in] angle : 旋转角度
 **/
s32 compo_rotary_set_rotation(compo_rotary_t *rotary, s32 angle)
{
    if (angle < 0) {
        angle = rotary->total_angle - (-angle % rotary->total_angle);
    }
    if (angle >= rotary->total_angle) {
        angle = angle % rotary->total_angle;
    }
    rotary->angle = angle;
    return angle;
}

/**
 * @brief 设置转盘的转盘半径
 * @param[in] rotary : 转盘指针
 * @param[in] radius : 转盘半径
 **/
void compo_rotary_set_radius(compo_rotary_t *rotary, u16 radius)
{
    rotary->radius = radius;
#if !BOX_GUI_ROTATE_DISP
    int y = muls_shift16(-rotary->radius, SIN(ROTARY_OVERLOOK));
    widget_axis3d_set_distance(rotary->axis, ROTARY_SIGHT_DISTANCE);
    widget_axis3d_set_overlook(rotary->axis, ROTARY_OVERLOOK);
    widget_axis3d_set_pos(rotary->axis, 0, y, -rotary->radius);
#endif
    for (int i=0; i<ROTARY_ITEM_CNT; i++) {
        widget_image3d_t *img = widget_image3d_create(rotary->page, 0);
        rotary->item_img[i] = img;

        widget_image3d_set_axis(img, rotary->axis);
        widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
        widget_image3d_set_r(img, rotary->radius);
    }
}


/**
 * @brief 根据idx设置转盘的旋转角度
 * @param[in] rotary : 转盘指针
 * @param[in] idx : 编号
 **/
s32 compo_rotary_set_rotation_byidx(compo_rotary_t *rotary, int idx)
{
    rotary->angle = idx * ROTARY_ITEM_ANGLE;;
    return rotary->angle;
}

static int compo_rotary_calc_idx(compo_rotary_t *rotary)
{
    return ((rotary->angle + ROTARY_ITEM_ANGLE / 2) / ROTARY_ITEM_ANGLE);
}

/**
 * @brief 获取当前idx
 * @param[in] rotary : 转盘指针
 * @return 返回当前idx
 **/
int compo_rotary_get_idx(compo_rotary_t *rotary)
{
    int idx = compo_rotary_calc_idx(rotary);
    if (idx < 0 || idx >= rotary->item_cnt) {
        idx = 0;
    }
    return idx;
}

/**
 * @brief 设置转盘视角
 * @param[in] rotary : 转盘指针
 * @param[in] angle : 视角
 **/
static void compo_rotary_set_overlook(compo_rotary_t *rotary, s16 angle)
{
    widget_axis3d_t *axis = rotary->axis;
#if BOX_GUI_ROTATE_DISP
    widget_axis3d_set_polar(axis, 900 - angle);
#else
    int y = muls_shift16(-rotary->radius, SIN(angle));
    widget_axis3d_set_overlook(axis, angle);
    widget_axis3d_set_pos(axis, 0, y, -rotary->radius);
#endif
}

/**
 * @brief 转盘拖动与移动初始化
 * @param[in] rotary : 转盘指针
 * @return 返回COMPO_ROTARY_STA状态枚举
 **/
u8 compo_rotary_get_sta(compo_rotary_t *rotary)
{
    compo_rotary_move_cb_t *mcb = &rotary->move_cb;
    if (mcb == NULL) {
        return COMPO_ROTARY_STA_IDLE;
    }
    switch (mcb->sta) {
    case COMPO_ROTARY_STA_ENTERING:
        return COMPO_ROTARY_STA_ENTERING;

    case COMPO_ROTARY_STA_EXITING:
        return COMPO_ROTARY_STA_EXITING;

    case COMPO_ROTARY_STA_EXIT:
        return COMPO_ROTARY_STA_EXIT;

    default:
        break;
    }
    if (mcb->flag_drag) {
        return COMPO_ROTARY_STA_DARG;
    } else if (mcb->flag_move_auto) {
        return COMPO_ROTARY_STA_MOVE;
    } else {
        return COMPO_ROTARY_STA_IDLE;
    }
}

/**
 * @brief 转盘拖动与移动处理
 * @param[in] rotary : 转盘指针
 **/
void compo_rotary_move(compo_rotary_t *rotary)
{
    compo_rotary_move_cb_t *mcb = &rotary->move_cb;
    if (mcb == NULL) {
        return;
    }
    switch (mcb->sta) {
    case COMPO_ROTARY_STA_ENTERING:
        if (tick_check_expire(mcb->tick, ANIMATION_TICK_EXPIRE)) {
            s16 cnt, wid, hei, swid, shei, angle;
            mcb->tick = tick_get();
            mcb->animation_cnt++;
            cnt = mcb->animation_cnt;
            angle = ROTARY_OVERLOOK * cnt / ANIMATION_CNT_ENTERING;
            swid = rotary->img_area.wid;
            wid = GUI_SCREEN_WIDTH - (GUI_SCREEN_WIDTH - swid) * cnt / ANIMATION_CNT_ENTERING;
            wid = GUI_SCREEN_WIDTH * wid / swid;
            shei = rotary->img_area.hei;
            hei = GUI_SCREEN_HEIGHT - (GUI_SCREEN_HEIGHT - shei) * cnt / ANIMATION_CNT_ENTERING;
            hei = GUI_SCREEN_HEIGHT * hei / shei;
            compo_rotary_set_overlook(rotary, angle);
            widget_page_scale_to(rotary->page, wid, hei);
            if (mcb->animation_cnt >= ANIMATION_CNT_ENTERING) {
                mcb->sta = COMPO_ROTARY_STA_IDLE;
                widget_set_visible(rotary->item_title, true);
            }
        }
        return;

    case COMPO_ROTARY_STA_EXITING:
        if (tick_check_expire(mcb->tick, ANIMATION_TICK_EXPIRE)) {
            s16 cnt, wid, hei, swid, shei, angle;
            mcb->tick = tick_get();
            mcb->animation_cnt++;
            cnt = mcb->animation_cnt;
            angle = ROTARY_OVERLOOK - ROTARY_OVERLOOK * cnt / ANIMATION_CNT_ENTERING;
            swid = rotary->img_area.wid;
            wid = swid + (GUI_SCREEN_WIDTH - swid) * cnt / ANIMATION_CNT_ENTERING;
            wid = GUI_SCREEN_WIDTH * wid / swid;
            shei = rotary->img_area.hei;
            hei = shei + (GUI_SCREEN_HEIGHT - shei) * cnt / ANIMATION_CNT_ENTERING;
            hei = GUI_SCREEN_HEIGHT * hei / shei;
            compo_rotary_set_overlook(rotary, angle);
            widget_page_scale_to(rotary->page, wid, hei);
            if (mcb->animation_cnt >= ANIMATION_CNT_ENTERING) {
                mcb->sta = COMPO_ROTARY_STA_EXIT;
            }
        }
        return;

    default:
        break;
    }
    if (mcb->flag_drag) {
        s32 dx, dy, da;
        int focus_idx;
        mcb->flag_drag = ctp_get_dxy(&dx, &dy);
        if (mcb->flag_drag) {
            //拖动菜单图标
            da = dx * 1800 / (int)(M_PI * rotary->radius);
            compo_rotary_set_rotation(rotary, mcb->focus_a - da);
            compo_rotary_update(rotary);
        } else {
            //抬手后开始自动移动
            s32 last_dx;
            last_dx = ctp_get_last_dxy().x;
            focus_idx = compo_rotary_calc_idx(rotary);
            mcb->moveto_idx = focus_idx - (last_dx * DRAG_AUTO_SPEED / (int)(M_PI * rotary->radius));
            mcb->focus_a = rotary->angle;
            mcb->flag_move_auto = true;
            mcb->moveto_a = mcb->moveto_idx * ROTARY_ITEM_ANGLE;
            mcb->tick = tick_get();
        }
    }
    if (mcb->flag_move_auto) {
        //自动移动
        if (mcb->focus_a == mcb->moveto_a) {
            mcb->flag_move_auto = false;              //移动完成
            compo_rotary_update(rotary);
        } else if (tick_check_expire(mcb->tick, ANIMATION_TICK_EXPIRE)) {
            s32 da;
            mcb->tick = tick_get();
            da = mcb->moveto_a - mcb->focus_a;
            if (da > 0) {
                if (da > FOCUS_AUTO_STEP * FOCUS_AUTO_STEP_DIV) {
                    da = da / FOCUS_AUTO_STEP_DIV;
                } else if (da > FOCUS_AUTO_STEP) {
                    da = FOCUS_AUTO_STEP;
                } else {
                    da = 1;
                }
            } else {
                if (da < -FOCUS_AUTO_STEP * FOCUS_AUTO_STEP_DIV) {
                    da = da / FOCUS_AUTO_STEP_DIV;
                } else if (da < -FOCUS_AUTO_STEP) {
                    da = -FOCUS_AUTO_STEP;
                } else {
                    da = -1;
                }
            }
            mcb->focus_a += da;
            compo_rotary_set_rotation(rotary, mcb->focus_a);
            compo_rotary_update(rotary);
        }
    }
}

/**
 * @brief 转盘拖动与移动控制
 * @param[in] rotary : 转盘指针
 * @param[in] cmd : 控制命令
 **/
void compo_rotary_move_control(compo_rotary_t *rotary, int cmd)
{
    int wid, hei;
    compo_rotary_move_cb_t *mcb = &rotary->move_cb;
    if (mcb == NULL) {
        return;
    }
    switch (cmd) {
    case COMPO_ROTARY_MOVE_CMD_ENTERING:
        wid = GUI_SCREEN_WIDTH * GUI_SCREEN_WIDTH / rotary->img_area.wid;
        hei = GUI_SCREEN_HEIGHT * GUI_SCREEN_HEIGHT / rotary->img_area.hei;
        widget_page_scale_to(rotary->page, wid, hei);
        widget_set_visible(rotary->item_title, false);
        compo_rotary_set_overlook(rotary, 0);
        mcb->sta = COMPO_ROTARY_STA_ENTERING;
        mcb->animation_cnt = 0;
        mcb->tick = tick_get();
        break;

    case COMPO_ROTARY_MOVE_CMD_EXITING:
        widget_set_visible(rotary->item_title, false);
        mcb->sta = COMPO_ROTARY_STA_EXITING;
        mcb->animation_cnt = 0;
        mcb->tick = tick_get();
        break;

    case COMPO_ROTARY_MOVE_CMD_DRAG:
        //开始拖动
        mcb->flag_drag = true;
        mcb->flag_move_auto = false;
        mcb->focus_a = rotary->angle;
        break;

    case COMPO_ROTARY_MOVE_CMD_FORWARD:
        //向前滚动
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->moveto_idx = compo_rotary_calc_idx(rotary);
            mcb->focus_a = rotary->angle;
        }
        mcb->moveto_idx++;
        mcb->moveto_a = mcb->moveto_idx * ROTARY_ITEM_ANGLE;
        break;

    case COMPO_ROTARY_MOVE_CMD_BACKWARD:
        //向后滚动
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->moveto_idx = compo_rotary_calc_idx(rotary);
            mcb->focus_a = rotary->angle;
        }
        mcb->moveto_idx--;
        mcb->moveto_a = mcb->moveto_idx * ROTARY_ITEM_ANGLE;
        break;

    default:
        halt(HALT_GUI_COMPO_ROTARY_MOVE_CMD);
        break;
    }
}
