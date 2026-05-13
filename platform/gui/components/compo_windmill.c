#include "include.h"

#define TRACE_EN    0
#if TRACE_EN
#define TRACE(...)  printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif // TRACE_EN

#define WINDMILL_ROLL360_MODE                   0                                   //是否采用任意角度滚动模式

#define WINDMILL_HALF_CIRCUM(x)                 ((int)(M_PI * x))                   //圆周一半
#define WINDMILL_MIN_POLAR                      -600//0                                   //立方体的最大最小极角
#define WINDMILL_MAX_POLAR                      600//3600
#define WINDMILL_ITEM_ANGLE                     360

//移动相关控制
#define ANIMATION_TICK_EXPIRE               18                                  //动画单位时间Tick(ms)
#define ANIMATION_CNT_ENTERING              15                                  //入场动画拍数
#define ANIMATION_CNT_EXITING               15                                  //出场动画拍数
#define FOCUS_AUTO_STEP                     5                                   //松手后自动对齐焦点单位时间步进
#define FOCUS_AUTO_STEP_DIV                 16
#define DRAG_AUTO_SPEED                     (WINDMILL_ITEM_ANGLE * 80)

#define USE_WINDMILL_3D                     0                                   //0:平面立体风车; 1:斜面立体风车
#define WINDMILL_2D_SPACE                   10                                  //平面立体风车的厚度
#define WINDMILL_3D_SPACE                   100                                 //斜面立体风车的厚度

//极角
static const s16 tbl_windmill_polar[] = {

#if USE_WINDMILL_3D
//    0,      //基准图
    //红
    3600 - WINDMILL_3D_SPACE, WINDMILL_3D_SPACE,
    3600 - WINDMILL_3D_SPACE, WINDMILL_3D_SPACE,


    //黄
    WINDMILL_3D_SPACE, 3600 -WINDMILL_3D_SPACE,
    WINDMILL_3D_SPACE, 3600 - WINDMILL_3D_SPACE,


    //绿
    3600 - WINDMILL_3D_SPACE, WINDMILL_3D_SPACE,
    3600 - WINDMILL_3D_SPACE, WINDMILL_3D_SPACE,


    //蓝
    WINDMILL_3D_SPACE, 3600 - WINDMILL_3D_SPACE,
    WINDMILL_3D_SPACE, 3600 - WINDMILL_3D_SPACE,

    900, 900,
    900, 900,
    900, 900,
    900, 900,

#else
    900, 900, 900, 900,

    900, 900, 900, 900,

    //底层
//    900, 900, 900, 900,
    900, 900, 900, 900,

    0, 1799,
#endif // USE_WINDMILL_3D
};

//方位角
static const s16 tbl_windmill_azimuth[] = {
#if USE_WINDMILL_3D
//    1800,       //基准图

    //红
    2700, 2700,
    2700, 2700,

    //黄
    3599, 3599,
    3599, 3599,

    //绿
    2700, 2700,
    2700, 2700,

    //蓝
    3599, 3599,
    3599, 3599,

    1500,
    600,
    -300,
    -1200,

    910,
    0,
    -900,
    1800,

#else
    900, 1800, 2700, 0,

    900, 1800, 2700, 0,

    //底层1
//    900, 1800, 2700, 0,
    450, 1350, 2250, -450,

    1800, 0,
#endif // USE_WINDMILL_3D
};

//自旋角
static const s16 tbl_windmill_rotation[] = {
#if USE_WINDMILL_3D
//    -900,       //基准图

    //红
    3600-1350, 3600-1350,
    3600-910, 3600-910,

    //黄
    450, 450,
    910, 910,

    //绿
    450, 450,
    910, 910,

    //蓝
    3600-1350, 3600-1350,
    3600-910, 3600-910,

    900, 900,
    900, 900,
    900, 900,
    900, 900,


#else
    900, 900, 900, 900,

    -900, -900, -900, -900,

    //底层
    900, 900, 900, 900,

    -900, 900,
#endif // USE_WINDMILL_3D
};


typedef struct rect_3d_t_ {
    s16 x,y,wid,hei,r;
    bool top;
    s16 rx,ry;      //旋转中心
} rect_3d_t;

//图片位置长宽信息
#define COMPO_WIND_SIZE     226
static const rect_3d_t tbl_windmill_img_rect[] = {

#if USE_WINDMILL_3D
//    {GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, COMPO_WIND_SIZE, COMPO_WIND_SIZE, 0, false, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2},

    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 * 1.45, COMPO_WIND_SIZE/4 * 1.45,  2, true, 0, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 * 1.45, COMPO_WIND_SIZE/4 * 1.45,  2, true, 0, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 + 5,    COMPO_WIND_SIZE/4 + 5,     2, true, 0, COMPO_WIND_SIZE/4},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 + 5,    COMPO_WIND_SIZE/4 + 5,     2, true, 0, COMPO_WIND_SIZE/4},


    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 * 1.45, COMPO_WIND_SIZE/4 * 1.45,  0, true, 0, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 * 1.45, COMPO_WIND_SIZE/4 * 1.45,  0, true, 0, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 + 5,    COMPO_WIND_SIZE/4 + 5,     0, true, 0, COMPO_WIND_SIZE/4},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 + 5,    COMPO_WIND_SIZE/4 + 5,     0, true, 0, COMPO_WIND_SIZE/4},


    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 * 1.45, COMPO_WIND_SIZE/4 * 1.45,  0, true, 0, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 * 1.45, COMPO_WIND_SIZE/4 * 1.45,  0, true, 0, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 + 5,    COMPO_WIND_SIZE/4 + 5,     0, true, 0, COMPO_WIND_SIZE/4},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 + 5,    COMPO_WIND_SIZE/4 + 5,     0, true, 0, COMPO_WIND_SIZE/4},


    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 * 1.45, COMPO_WIND_SIZE/4 * 1.45,  0, true, 0, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 * 1.45, COMPO_WIND_SIZE/4 * 1.45,  0, true, 0, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 + 5,    COMPO_WIND_SIZE/4 + 5,     0, true, 0, COMPO_WIND_SIZE/4},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4 + 5,    COMPO_WIND_SIZE/4 + 5,     0, true, 0, COMPO_WIND_SIZE/4},

    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, 50, COMPO_WIND_SIZE-40, COMPO_WIND_SIZE/4, true, 85/2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, 50, COMPO_WIND_SIZE-40, COMPO_WIND_SIZE/4, true, 85/2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, 50, COMPO_WIND_SIZE-40, COMPO_WIND_SIZE/4, true, 85/2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, 50, COMPO_WIND_SIZE-40, COMPO_WIND_SIZE/4, true, 85/2, 0},

    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, 20, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4, true, 85/2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, 20, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4, true, 85/2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, 20, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4, true, 85/2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, 20, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/4, true, 85/2, 0},

#else

    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, WINDMILL_2D_SPACE*2, COMPO_WIND_SIZE/4, COMPO_WIND_SIZE/4, false, 2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, WINDMILL_2D_SPACE*2, COMPO_WIND_SIZE/4, COMPO_WIND_SIZE/4, false, 2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, WINDMILL_2D_SPACE*2, COMPO_WIND_SIZE/4, COMPO_WIND_SIZE/4, false, 2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, WINDMILL_2D_SPACE*2, COMPO_WIND_SIZE/4, COMPO_WIND_SIZE/4, false, 2, 0},

    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, WINDMILL_2D_SPACE*2, COMPO_WIND_SIZE/4, COMPO_WIND_SIZE/4, false, 2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, WINDMILL_2D_SPACE*2, COMPO_WIND_SIZE/4, COMPO_WIND_SIZE/4, false, 2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, WINDMILL_2D_SPACE*2, COMPO_WIND_SIZE/4, COMPO_WIND_SIZE/4, false, 2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, WINDMILL_2D_SPACE*2, COMPO_WIND_SIZE/4, COMPO_WIND_SIZE/4, false, 2, 0},

    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, WINDMILL_2D_SPACE*2, COMPO_WIND_SIZE/4+20, 80, true, 2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, WINDMILL_2D_SPACE*2, COMPO_WIND_SIZE/4+20, 80, true, 2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, WINDMILL_2D_SPACE*2, COMPO_WIND_SIZE/4+20, 80, true, 2, 0},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, WINDMILL_2D_SPACE*2, COMPO_WIND_SIZE/4+20, 80, true, 2, 0},

    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE, COMPO_WIND_SIZE, WINDMILL_2D_SPACE, true, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2},
    {COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE, COMPO_WIND_SIZE, WINDMILL_2D_SPACE, true, COMPO_WIND_SIZE/2, COMPO_WIND_SIZE/2},
#endif // USE_WINDMILL_3D

};

//创建风车组件
compo_windmill_t* compo_windmill_create(compo_form_t* frm, compo_windmill_item_t const *item, u16 item_cnt)
{
//    printf("%s\n", __func__);
    compo_windmill_t *windmill = compo_create(frm, COMPO_TYPE_WINDMILL);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, COMPO_WIND_SIZE, COMPO_WIND_SIZE);

    //建立aixs(坐标系)
    widget_axis3d_t *axis = widget_axis3d_create(page);

    windmill->page = page;
    windmill->item = item;
    windmill->flag_need_update = true;
    windmill->axis = axis;

    if (item_cnt > WINDMILL_ITEM_CNT) {
        item_cnt = WINDMILL_ITEM_CNT;
    }
    windmill->item_cnt = item_cnt;

    //建立图片
    for (int i=0; i<item_cnt; i++) {

        widget_image3d_t* img = widget_image3d_create(page, item[i].res_addr);
        windmill->item_img[i] = img;

        widget_image3d_set_axis(img, axis);
        widget_set_location(img, tbl_windmill_img_rect[i].x, tbl_windmill_img_rect[i].y, tbl_windmill_img_rect[i].wid, tbl_windmill_img_rect[i].hei);
        widget_image3d_set_rotation_center(img, tbl_windmill_img_rect[i].rx, tbl_windmill_img_rect[i].ry);
        widget_set_top(img, tbl_windmill_img_rect[i].top);
        widget_image3d_set_r(img, tbl_windmill_img_rect[i].r);
        widget_image3d_set_polar(img, tbl_windmill_polar[i]);
        widget_image3d_set_azimuth(img, tbl_windmill_azimuth[i]);
        widget_image3d_set_rotation(img, tbl_windmill_rotation[i]);

//        printf("%s->[%d]rpa[%d,%d,%d]\n", __func__, i, tbl_windmill_rotation[i],tbl_windmill_polar[i], tbl_windmill_azimuth[i]);
    }

    //axis初始角度
    windmill->sph.r = 60;
    windmill->sph.rotation = 0;
    windmill->sph.polar = 0;
    windmill->sph.azimuth = 0;
    compo_windmill_update(windmill);

    return windmill;
}


void compo_windmill_update(compo_windmill_t* windmill)
{
    if (!windmill->flag_need_update) {
        return;
    }
    widget_axis3d_t* axis = windmill->axis;
    windmill->flag_need_update = false;

    widget_axis3d_set_sph(axis, windmill->sph);
    for (int i=0; i<windmill->item_cnt; i++) {
        widget_image3d_t* img = windmill->item_img[i];
        if (tbl_windmill_img_rect[i].top == false) {
            widget_set_visible(img, widget_image3d_is_front(img));
        }
        widget_set_top(img, widget_image3d_is_front(img));
//        printf("img[%d,%d]\n", i, widget_image3d_get_front(img));
    }
}

s32 compo_windmill_set_rotation(compo_windmill_t *windmill, s32 angle)
{
    if (angle < 0) {
        angle = 3600 - (-angle % 3600);
    }
    if (angle >= 3600) {
        angle = angle % 3600;
    }

    if (windmill->sph.rotation != angle) {
        windmill->sph.rotation = angle;
        windmill->flag_need_update = true;
    }
    return angle;
}

void compo_windmill_roll(compo_windmill_t* windmill, s16 roll_polar)
{
    compo_windmill_move_cb_t *mcb = &windmill->move_cb;
    windmill->sph = widget_axis3d_roll(windmill->axis, roll_polar, mcb->roll_azimuth);
}

void compo_windmill_roll_from(compo_windmill_t *windmill, s16 roll_polar, s16 roll_azimuth)
{
    compo_windmill_move_cb_t *mcb = &windmill->move_cb;
    windmill->sph = widget_axis3d_roll_from(windmill->axis, mcb->focus_sph, roll_polar, roll_azimuth);
    windmill->flag_need_update = true;
}

s32 compo_windmill_set_polar(compo_windmill_t *windmill, s32 angle)
{
    if (angle < WINDMILL_MIN_POLAR) {
        angle = WINDMILL_MIN_POLAR;
    } else if (angle > WINDMILL_MAX_POLAR) {
        angle = WINDMILL_MAX_POLAR;
    }


    if (windmill->sph.polar != angle) {
        if (windmill->mode != COMPO_WINDMILL_MOVE_CMD_AUTO) {
            windmill->wind_grade = windmill->sph.polar - angle;
        } else {
            windmill->wind_grade = windmill->sph.polar = 0;
        }
        windmill->sph.polar = angle;
        windmill->flag_need_update = true;
    }
    return angle;
}

u8 compo_windmill_get_sta(compo_windmill_t *windmill)
{
    compo_windmill_move_cb_t *mcb = &windmill->move_cb;
    if (mcb == NULL) {
        return COMPO_WINDMILL_STA_IDLE;
    }
    if (mcb->flag_drag) {
        return COMPO_WINDMILL_STA_DARG;
    } else if (mcb->flag_move_auto) {
        return COMPO_WINDMILL_STA_MOVE;
    } else {
        return COMPO_WINDMILL_STA_IDLE;
    }
}

int compo_windmill_get_idx(compo_windmill_t *windmill, s16 x, s16 y)
{
    int i;
    for (i=0; i<WINDMILL_ITEM_CNT; i++) {
        widget_image3d_t *img = windmill->item_img[i];
        if (widget_image3d_is_front(img) && widget_image3d_contains(img, x, y)) {
            return i;
        }
    }
    return -1;
}

void compo_windmill_move(compo_windmill_t *windmill)
{
    compo_windmill_move_cb_t *mcb = &windmill->move_cb;
    if (mcb == NULL) {
        printf("%s->mcb null\n", __func__);
        return;
    }
    if (mcb->flag_drag) {
        s32 dx, dy, ax, ay;
        mcb->flag_drag = ctp_get_dxy(&dx, &dy);
        if (mcb->flag_drag) {
            //拖动菜单图标
            ax = abs(dx) * 1800 / WINDMILL_HALF_CIRCUM(60);
//            ay = dy * 1800 / WINDMILL_HALF_CIRCUM(60);
            ay = -dx * 1800 / WINDMILL_HALF_CIRCUM(1000);
#if WINDMILL_ROLL360_MODE
            int rp = sqrt64(ax * ax + ay * ay);
            int ra = ARCTAN2(-ay, ax);
            compo_windmill_roll_from(windmill, rp, ra);
#else
//            printf("polar [%d,%d]\n", ay, mcb->focus_sph.polar - ay);
            compo_windmill_set_polar(windmill, mcb->focus_sph.polar - ay);
            compo_windmill_set_rotation(windmill, mcb->focus_sph.rotation - ax);
#endif
            compo_windmill_update(windmill);
        } else {
            //抬手后开始自动移动
            point_t last_dxy = ctp_get_last_dxy();
            int da;
#if WINDMILL_ROLL360_MODE
            mcb->roll_azimuth = ARCTAN2(-last_dxy.y, last_dxy.x);
            da = sqrt64(last_dxy.x * last_dxy.x + last_dxy.y * last_dxy.y);
#else
            da = abs(last_dxy.x);
#endif
            mcb->moveto_a = da * DRAG_AUTO_SPEED / WINDMILL_HALF_CIRCUM(60);
            mcb->flag_move_auto = true;
            mcb->start_a = 0;
            mcb->tick = tick_get();
        }
    }
    if (mcb->flag_move_auto) {
        //自动移动
        if (mcb->start_a == mcb->moveto_a) {
            mcb->flag_move_auto = false;              //移动完成
            compo_windmill_update(windmill);
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
#if WINDMILL_ROLL360_MODE
            compo_windmill_roll(windmill, da);
#else
            compo_windmill_set_rotation(windmill, windmill->sph.rotation - da);
#endif
            compo_windmill_update(windmill);
        }
    }
}

void compo_windmill_move_control(compo_windmill_t *windmill, int cmd)
{
    compo_windmill_move_cb_t *mcb = &windmill->move_cb;
    windmill->mode = cmd;
    static u32 ticks = 0;
    bool flag_auto = false;
    //3s后开始自动转动
    if (tick_check_expire(ticks, 3000)) {
        if (cmd >= COMPO_WINDMILL_MOVE_CMD_DRAG && cmd <= COMPO_WINDMILL_MOVE_CMD_BACKWARD) {
            ticks = tick_get();
        } else {
            flag_auto = true;
        }

    }

    if (mcb == NULL) {
        printf("mcb NULL\n");
        return;
    }
    switch (cmd) {
    case COMPO_WINDMILL_MOVE_CMD_DRAG:
        //开始拖动
        mcb->flag_drag = true;
        mcb->flag_move_auto = false;
        mcb->focus_sph = windmill->sph;
        break;

    case COMPO_WINDMILL_MOVE_CMD_FORWARD:
        //向前滚动
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->start_a = 0;
            mcb->moveto_a = 0;
            mcb->roll_azimuth = 0;
        }
        mcb->moveto_a += WINDMILL_ITEM_ANGLE;
        break;

    case COMPO_WINDMILL_MOVE_CMD_BACKWARD:
        //向后滚动
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->start_a = 0;
            mcb->moveto_a = 0;
            mcb->roll_azimuth = 0;
        }
        mcb->moveto_a -= WINDMILL_ITEM_ANGLE;
        break;

    case COMPO_WINDMILL_MOVE_CMD_AUTO:
        if (flag_auto) {
        //向后滚动
            if (!mcb->flag_move_auto) {
                mcb->flag_move_auto = true;
                mcb->start_a = 0;
                mcb->moveto_a = 0;
                mcb->roll_azimuth = 0;
            }
            mcb->moveto_a -= WINDMILL_ITEM_ANGLE/10;
        }
        break;

    default:
//        halt(HALT_GUI_COMPO_WINDMILL_MOVE_CMD);
        break;
    }
}

void compo_windmill_set_pos(compo_windmill_t *windmill, s16 x, s16 y)
{
    widget_set_pos(windmill->page, x, y);
}



