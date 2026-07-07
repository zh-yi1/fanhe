#include "include.h"

//转场速度控制
#define SWITCHING_TICK_AUTO             180                         //快速松手视为自动模式(ms)
#define SWITCHING_TICK_EXPIRE           18                          //松手后自动切换单位时间(ms)

#define SWITCHING_FADE_STEP             20                          //淡入淡出速度

#define SWITCHING_MENU_STEP             (GUI_SCREEN_WIDTH / 8)      //上下菜单松手后自动切换单位时间步进
#define SWITCHING_MENU_DRAG_THRESHOLD   (GUI_SCREEN_WIDTH / 7)      //上下拖动松手自动的判断阈值(需要拉多少距离才能自动完成)

#define SWITCHING_LR_STEP               (GUI_SCREEN_WIDTH / 12)     //左右转场松手后自动切换单位步进
#define SWITCHING_LR_DRAG_THRESHOLD     (GUI_SCREEN_WIDTH / 3)      //左右转场松手自动的判断阈值(需要拉多少距离才能自动完成)

#define SWITCHING_UD_STEP               (GUI_SCREEN_HEIGHT / 12)    //上下转场松手后自动切换单位步进
#define SWITCHING_UD_DRAG_THRESHOLD     (GUI_SCREEN_HEIGHT / 3)     //上下转场松手自动的判断阈值(需要拉多少距离才能自动完成)

#define SWITCHING_ZOOM_ROLL_STEP        (GUI_SCREEN_WIDTH / 10)     //滚动步进
#define SWITCHING_ZOOM_ROLL_KICK_CNT    6                           //滚动步进分几次动画
#define SWITCHING_ZOOM_ROLL_STEP_PER    (SWITCHING_ZOOM_ROLL_STEP / SWITCHING_ZOOM_ROLL_KICK_CNT)
#define SWITCHING_ZOOM_ROLL_TICK_EXPIRE 300                         //滚动超时
#define SWITCHING_ZOOM_STEP             (GUI_SCREEN_WIDTH / 10)     //进出应用自动切换单位
#define SWITCHING_ZOOM_EXIT_THRESHOLD   (GUI_SCREEN_WIDTH / 2)      //退出应用自动切换判断阈值(需要拉多少距离才能自动退出)
#define SWITCHING_ZOOM_RATE             320                         //放大倍数速度，越大越快

#define SWITCHING_ZOOM_FADE_RATE        280                         //放大倍数速度，越大越快
#define SWITCHING_ZOOM_FADE_MAX         0x1A000                     //最大放大到多少
#define SWITCHING_ZOOM_FADE_1           (0x10000 + (SWITCHING_ZOOM_FADE_MAX - 0x10000) / 4)
#define SWITCHING_ZOOM_FADE_2           (0x10000 + (SWITCHING_ZOOM_FADE_MAX - 0x10000) * 3 / 4)

//左右转场的起始宽度
#define SWITCHING_LR_WIDTH              (GUI_SCREEN_WIDTH / 2)
#define SWITCHING_LR_HEIGHT             (GUI_SCREEN_HEIGHT / 2)

//上下转场的起始宽度
#define SWITCHING_UD_WIDTH              (GUI_SCREEN_WIDTH / 2)
#define SWITCHING_UD_HEIGHT             (GUI_SCREEN_HEIGHT / 2)

//转场的起始Alpha
#define SWITCHING_LR_ALPHA              64
#define SWITCHING_UD_ALPHA              64
#define SWITCHING_ZOOM_ALPHA_BMAX       192
#define SWITCHING_ZOOM_ALPHA            24

//左右两个场景的中心距离
#define SWITCHING_LR_DISTANCE           ((GUI_SCREEN_WIDTH + SWITCHING_LR_WIDTH) / 2 + (GUI_SCREEN_WIDTH / 8))

//上下两个场景的中心距离
#define SWITCHING_UD_DISTANCE           ((GUI_SCREEN_HEIGHT + SWITCHING_UD_HEIGHT) / 2 + (GUI_SCREEN_HEIGHT / 8))

//高斯模糊最大等级, 等级越高越模糊0-127
#define SWITCHING_UD_BLUR_MAX_LEVEL		32

static const func_switching_mode_t tbl_switching_mode[] = {
#if GUI_USE_SCREENSHOOT
    {FUNC_SWITCH3D_LR_RIGHT,     		FUNC_SWITCH3D_LR_LEFT    		},           //ZOOM
#else
    {FUNC_SWITCH_LR_ZOOM_RIGHT,     		FUNC_SWITCH_LR_ZOOM_LEFT    		},           //ZOOM
#endif // GUI_USE_SCREENSHOOT

    {FUNC_SWITCH_FADE_OUT,              FUNC_SWITCH_FADE_OUT            },           //FADE
    {FUNC_SWITCH_LR_RIGHT,              FUNC_SWITCH_LR_LEFT             },           //SHIFT
#if GUI_USE_SCREENSHOOT
    {FUNC_SWITCH3D_LR_FLIP_RIGHT,       FUNC_SWITCH3D_LR_FLIP_LEFT      },           //FLIP
    {FUNC_SWITCH3D_LR_PLATE_FLIP_RIGHT, FUNC_SWITCH3D_LR_PLATE_FLIP_LEFT},           //PLATE FLIP
    {FUNC_SWITCH3D_LR_RIGHT,            FUNC_SWITCH3D_LR_LEFT           },           //CUBE
    {FUNC_SWITCH3D_LR_IN_RIGHT,         FUNC_SWITCH3D_LR_IN_LEFT        },           //INSIDE CUBE
    {FUNC_SWITCH3D_LR_ROTA_RIGHT,       FUNC_SWITCH3D_LR_ROTA_LEFT      },           //ROTA
    {FUNC_SWITCH3D_LR_FOLD_RIGHT,       FUNC_SWITCH3D_LR_FOLD_LEFT      },           //FOLD
    {FUNC_SWITCH3D_LR_DRIFT_RIGHT,      FUNC_SWITCH3D_LR_DRIFT_LEFT     },
#endif // GUI_USE_SCREENSHOOT
    {FUNC_SWITCH_DIRECT,                FUNC_SWITCH_DIRECT              },           //NONE(DIRECT)
};



u8 func_get_switching_mode_byidx(u8 idx, bool flag_next)
{
    if (flag_next) {
        return tbl_switching_mode[idx].next;
    }

    return tbl_switching_mode[idx].prev;
}

void func_switching_blur_process(u16 switch_mode, u16 type_blur, s32 ypos, u8 *blur_level)
{

#if GUI_USE_BLUR
    s32 y = ypos;
    if (type_blur == FUNC_SWITCH_DOWN_BLUR || type_blur == FUNC_SWITCH_DOWN_BG_BLUR) {
        f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
        if (f_clk->blur_pic) {
            u8 val = 0;
            if (switch_mode == FUNC_SWITCH_MENU_DROPDOWN_UP || switch_mode == FUNC_SWITCH_MENU_DROPDOWN_DOWN) {
                val = min(SWITCHING_UD_BLUR_MAX_LEVEL*(y+GUI_SCREEN_CENTER_Y)/GUI_SCREEN_CENTER_Y, SWITCHING_UD_BLUR_MAX_LEVEL);      //2分之1
            } else {
                y = GUI_SCREEN_HEIGHT + GUI_SCREEN_CENTER_Y - y;
                val = min(SWITCHING_UD_BLUR_MAX_LEVEL*y/GUI_SCREEN_CENTER_Y, SWITCHING_UD_BLUR_MAX_LEVEL);      //2分之1
            }

            if (*blur_level != val) {
                *blur_level = val;
                compo_picturebox_blur_set_ram(f_clk->blur_pic, cur_scbuf, blur_obuf, *blur_level);
//                printf("blur_level:%d\n", *blur_level);
            }

            if (type_blur == FUNC_SWITCH_DOWN_BLUR) {
                compo_picturebox_set_pos(f_clk->blur_pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - y);
            } else {
//                        compo_picturebox_set_size(f_clk->blur_pic, GUI_SCREEN_WIDTH*0.9, GUI_SCREEN_HEIGHT*0.9);
            }
        }
    }
#endif
}

//手动场景切换中消息处理
void func_switching_message(size_msg_t msg)
{
    evt_message(msg);
}

//单独淡入淡出
static bool func_switching_fade(u16 switch_mode, bool flag_auto)
{
    compo_form_t *frm_cur = compo_pool_get_top();
    u8 flag_pos;                                                //当前状态
    s32 cur_alpha;
    u32 tick = tick_get();
    switch_mode = switch_mode & 0xFF;

    if (frm_cur == NULL || frm_cur->page == NULL) {
        return true;
    }

    switch (switch_mode) {
    case FUNC_SWITCH_FADE_OUT:
        cur_alpha = 255;
        break;

    default:
        halt(HALT_FUNC_SWITCH_ZOOM_MODE);
        return false;
    }
    compo_form_set_alpha(frm_cur, cur_alpha);
    for (;;) {
        flag_pos = FLAG_POS_NORM;
        if (tick_check_expire(tick, SWITCHING_TICK_EXPIRE)) {
            tick = tick_get();
            switch (switch_mode) {
            case FUNC_SWITCH_FADE_OUT:
                //淡出
                cur_alpha -= SWITCHING_FADE_STEP;
                if (cur_alpha <= 0) {
                    cur_alpha = 0;
                    flag_pos = FLAG_POS_END;
                }
                break;

            default:
                halt(HALT_FUNC_SWITCH_ZOOM_MODE);
                break;
            }
            compo_form_set_alpha(frm_cur, cur_alpha);
        }
        func_process();
        if (!flag_auto) {
            func_switching_message(msg_dequeue());
        }
        if (flag_pos != FLAG_POS_NORM) {
            break;
        }
    }
    return true;
}

//左右切换任务
static bool func_switching_lr(u16 switch_mode, bool flag_auto)
{
    compo_form_t *frm_last = compo_pool_get_bottom();
    compo_form_t *frm_cur = compo_pool_get_top();
    bool flag_press = !flag_auto;
    u8 flag_pos;                            //当前坐标状态
    bool flag_change;
    s32 dx = 0;
    int dx_auto;
    u32 tick = tick_get();
    s32 xpos1 = GUI_SCREEN_CENTER_X;
    s32 xpos2, xpos2_start;
    s32 x_distance;
    if (frm_last == NULL || frm_cur == NULL) {
        halt(HALT_FUNC_SWITCH_LR_PTR);
    }

    switch_mode = switch_mode & 0xFF;
    if (switch_mode == FUNC_SWITCH_LR_ZOOM_LEFT || switch_mode == FUNC_SWITCH_LR_ZOOM_RIGHT) {
        x_distance = SWITCHING_LR_DISTANCE;     //带缩放的切换，两个场景中心距离要稍微近一点
        compo_form_scale_to(frm_cur, SWITCHING_LR_WIDTH, SWITCHING_LR_HEIGHT);
        compo_form_set_alpha(frm_cur, SWITCHING_LR_ALPHA);
    } else {
        x_distance = GUI_SCREEN_WIDTH;          //平移切换，按屏幕宽度做中心距离
    }
    switch (switch_mode) {
    case FUNC_SWITCH_LR_LEFT:
    case FUNC_SWITCH_LR_ZOOM_LEFT:
        //左划，右侧任务向左移入
        xpos2_start = GUI_SCREEN_CENTER_X + x_distance;
        dx_auto = -SWITCHING_LR_STEP;
        break;

    case FUNC_SWITCH_LR_RIGHT:
    case FUNC_SWITCH_LR_ZOOM_RIGHT:
        //右划，左侧任务向右移入
        xpos2_start = GUI_SCREEN_CENTER_X - x_distance;
        dx_auto = SWITCHING_LR_STEP;
        break;

    default:
        halt(HALT_FUNC_SWITCH_LR_MODE);
        return false;
    }
    compo_form_set_pos(frm_cur, xpos2_start, GUI_SCREEN_CENTER_Y);

    for (;;) {
        flag_pos = FLAG_POS_NORM;
        flag_change = false;
        if (flag_press) {
            s32 dy;
            flag_press = ctp_get_dxy(&dx, &dy);
            if (flag_press) {
                if (tick_check_expire(tick, SWITCHING_TICK_AUTO)) {
                    switch (switch_mode) {
                    case FUNC_SWITCH_LR_LEFT:
                    case FUNC_SWITCH_LR_ZOOM_LEFT:
                        //下拉往上, 上拉往上
                        if (dx <= -SWITCHING_LR_DRAG_THRESHOLD) {
                            dx_auto = -SWITCHING_LR_STEP;
                        } else {
                            dx_auto = SWITCHING_LR_STEP;
                        }
                        break;

                    case FUNC_SWITCH_LR_RIGHT:
                    case FUNC_SWITCH_LR_ZOOM_RIGHT:
                        //下拉往下, 上拉往下
                        if (dx >= SWITCHING_LR_DRAG_THRESHOLD) {
                            dx_auto = SWITCHING_LR_STEP;
                        } else {
                            dx_auto = -SWITCHING_LR_STEP;
                        }
                        break;

                    default:
                        halt(HALT_FUNC_SWITCH_MENU_MODE);
                        return false;
                    }
                }
            }
            flag_change = true;
        } else if (tick_check_expire(tick, SWITCHING_TICK_EXPIRE)) {
            tick = tick_get();
            dx += dx_auto;
            flag_change = true;
        }

        if (flag_change) {
            switch (switch_mode) {
            case FUNC_SWITCH_LR_LEFT:
            case FUNC_SWITCH_LR_ZOOM_LEFT:
                //左划，右侧任务向左移入
                if (dx >= 0) {
                    dx = 0;
                    flag_pos = FLAG_POS_START;
                } else if (dx <= -x_distance) {
                    dx = -x_distance;
                    flag_pos = FLAG_POS_END;
                }
                break;

            case FUNC_SWITCH_LR_RIGHT:
            case FUNC_SWITCH_LR_ZOOM_RIGHT:
                //右划，左侧任务向右移入
                if (dx <= 0) {
                    dx = 0;
                    flag_pos = FLAG_POS_START;
                } else if (dx >= x_distance) {
                    dx = x_distance;
                    flag_pos = FLAG_POS_END;
                }
                break;

            default:
                halt(HALT_FUNC_SWITCH_LR_MODE);
                return false;
            }
            xpos1 = GUI_SCREEN_CENTER_X + dx;
            xpos2 = xpos2_start + dx;
            compo_form_set_pos(frm_last, xpos1, GUI_SCREEN_CENTER_Y);
            compo_form_set_pos(frm_cur, xpos2, GUI_SCREEN_CENTER_Y);
            if (switch_mode == FUNC_SWITCH_LR_ZOOM_LEFT || switch_mode == FUNC_SWITCH_LR_ZOOM_RIGHT) {
                s32 abs_dx = abs_s(dx);
                s32 dw = (GUI_SCREEN_WIDTH - SWITCHING_LR_WIDTH) * abs_dx / x_distance;
                s32 xw1 = GUI_SCREEN_WIDTH - dw;
                s32 xw2 = SWITCHING_LR_WIDTH + dw;
                s32 dh = (GUI_SCREEN_HEIGHT - SWITCHING_LR_HEIGHT) * abs_dx / x_distance;
                s32 xh1 = GUI_SCREEN_HEIGHT - dh;
                s32 xh2 = SWITCHING_LR_HEIGHT + dh;
                u8 da = (255 - SWITCHING_LR_ALPHA) * abs_dx / x_distance;
                u8 xa1 = 255 - da;
                u8 xa2 = SWITCHING_LR_ALPHA + da;
                compo_form_scale_to(frm_last, xw1, xh1);
                compo_form_scale_to(frm_cur, xw2, xh2);
                compo_form_set_alpha(frm_last, xa1);
                compo_form_set_alpha(frm_cur, xa2);
            }
        }
        func_process();
        if (!flag_auto) {
            func_switching_message(msg_dequeue());
        }
        if (!flag_press && flag_pos != FLAG_POS_NORM) {
            break;
        }
    }
    return (flag_pos == FLAG_POS_END);
}

//上下切换任务
static bool func_switching_ud(u16 switch_mode, bool flag_auto)
{
    compo_form_t *frm_last = compo_pool_get_bottom();
    compo_form_t *frm_cur = compo_pool_get_top();
    bool flag_press = !flag_auto;
    u8 flag_pos;                            //当前坐标状态
    bool flag_change;
    s32 dy = 0;
    int dy_auto;
    u32 tick = tick_get();
    s32 ypos1 = GUI_SCREEN_CENTER_Y;
    s32 ypos2, ypos2_start;
    s32 y_distance;
    switch_mode = switch_mode & 0xFF;

    if (frm_last == NULL || frm_cur == NULL) {
        halt(HALT_FUNC_SWITCH_UD_PTR);
    }

    if (switch_mode == FUNC_SWITCH_UD_ZOOM_UP || switch_mode == FUNC_SWITCH_UD_ZOOM_DOWN) {
        y_distance = SWITCHING_UD_DISTANCE;         //带缩放的切换，两个场景中心距离要稍微近一点
    } else {
        y_distance = GUI_SCREEN_HEIGHT;             //平移切换，按屏幕高度做中心距离
    }
    switch (switch_mode) {
    case FUNC_SWITCH_UD_UP:
    case FUNC_SWITCH_UD_ZOOM_UP:
        //上划，下面任务向上移动
        ypos2_start = GUI_SCREEN_CENTER_Y + y_distance;
        dy_auto = -SWITCHING_UD_STEP;
        break;

    case FUNC_SWITCH_UD_DOWN:
    case FUNC_SWITCH_UD_ZOOM_DOWN:
        //下划，上面任务向下移动
        ypos2_start = GUI_SCREEN_CENTER_Y - y_distance;
        dy_auto = SWITCHING_UD_STEP;
        break;

    default:
        halt(HALT_FUNC_SWITCH_UD_MODE);
        return false;
    }
    compo_form_set_pos(frm_cur, GUI_SCREEN_CENTER_X, ypos2_start);
    if (switch_mode == FUNC_SWITCH_UD_ZOOM_UP || switch_mode == FUNC_SWITCH_UD_ZOOM_DOWN) {
        compo_form_scale_to(frm_cur, SWITCHING_UD_WIDTH, SWITCHING_UD_HEIGHT);
    } else {
        compo_form_scale_to(frm_cur, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    }
    compo_form_set_alpha(frm_cur, SWITCHING_UD_ALPHA);
    for (;;) {
        flag_pos = FLAG_POS_NORM;
        flag_change = false;
        if (flag_press) {
            s32 dx;
            flag_press = ctp_get_dxy(&dx, &dy);
            if (flag_press) {
                if (tick_check_expire(tick, SWITCHING_TICK_AUTO)) {
                    switch (switch_mode) {
                    case FUNC_SWITCH_UD_UP:
                    case FUNC_SWITCH_UD_ZOOM_UP:
                        if (dy <= -SWITCHING_UD_DRAG_THRESHOLD) {
                            dy_auto = -SWITCHING_UD_STEP;
                        } else {
                            dy_auto = SWITCHING_UD_STEP;
                        }
                        break;

                    case FUNC_SWITCH_UD_DOWN:
                    case FUNC_SWITCH_UD_ZOOM_DOWN:
                        if (dy >= SWITCHING_UD_DRAG_THRESHOLD) {
                            dy_auto = SWITCHING_UD_STEP;
                        } else {
                            dy_auto = -SWITCHING_UD_STEP;
                        }
                        break;

                    default:
                        halt(HALT_FUNC_SWITCH_MENU_MODE);
                        return false;
                    }
                }
            }
            flag_change = true;
        } else if (tick_check_expire(tick, SWITCHING_TICK_EXPIRE)) {
            tick = tick_get();
            dy += dy_auto;
            flag_change = true;
        }

        if (flag_change) {
            switch (switch_mode) {
            case FUNC_SWITCH_UD_UP:
            case FUNC_SWITCH_UD_ZOOM_UP:
                if (dy >= 0) {
                    dy = 0;
                    flag_pos = FLAG_POS_START;
                } else if (dy <= -y_distance) {
                    dy = -y_distance;
                    flag_pos = FLAG_POS_END;
                }
                break;

            case FUNC_SWITCH_UD_DOWN:
            case FUNC_SWITCH_UD_ZOOM_DOWN:
                if (dy <= 0) {
                    dy = 0;
                    flag_pos = FLAG_POS_START;
                } else if (dy >= y_distance) {
                    dy = y_distance;
                    flag_pos = FLAG_POS_END;
                }
                break;

            default:
                halt(HALT_FUNC_SWITCH_UD_MODE);
                return false;
            }
            ypos1 = GUI_SCREEN_CENTER_Y + dy;
            ypos2 = ypos2_start + dy;
            compo_form_set_pos(frm_last, GUI_SCREEN_CENTER_X, ypos1);
            compo_form_set_pos(frm_cur, GUI_SCREEN_CENTER_X, ypos2);
            if (switch_mode == FUNC_SWITCH_UD_ZOOM_UP || switch_mode == FUNC_SWITCH_UD_ZOOM_DOWN) {

                s32 abs_dy = abs_s(dy);
                s32 dw = (GUI_SCREEN_WIDTH - SWITCHING_UD_WIDTH) * abs_dy / y_distance;
                s32 xw1 = GUI_SCREEN_WIDTH - dw;
                s32 xw2 = SWITCHING_UD_WIDTH + dw;
                s32 dh = (GUI_SCREEN_HEIGHT - SWITCHING_UD_HEIGHT) * abs_dy / y_distance;
                s32 xh1 = GUI_SCREEN_HEIGHT - dh;
                s32 xh2 = SWITCHING_UD_HEIGHT + dh;
                u8 da = (255 - SWITCHING_UD_ALPHA) * abs_dy / y_distance;
                u8 xa1 = 255 - da;
                u8 xa2 = SWITCHING_UD_ALPHA + da;
                compo_form_scale_to(frm_last, xw1, xh1);
                compo_form_scale_to(frm_cur, xw2, xh2);
                compo_form_set_alpha(frm_last, xa1);
                compo_form_set_alpha(frm_cur, xa2);


            }
        }
        func_process();
        if (!flag_auto) {
            func_switching_message(msg_dequeue());
        }
        if (!flag_press && flag_pos != FLAG_POS_NORM) {
            break;
        }
    }
    return (flag_pos == FLAG_POS_END);
}

//上下帘菜单等切换
static bool func_switching_menu(u16 switch_mode, bool flag_auto)
{
    compo_form_t *sub_frm = compo_pool_get_top();
    bool flag_press = !flag_auto;
    u8 flag_pos;                            //当前坐标状态
    bool flag_change;
    s32 dx = 0;
    s32 dy = 0;
    int dx_auto = 0;
    int dy_auto = 0;
    u32 tick = tick_get();
    int xpos = GUI_SCREEN_CENTER_X;
    int ypos = GUI_SCREEN_CENTER_Y;
    if (sub_frm == NULL) {
        /* 勿 halt：会进 halt 忙等并最终 WDT_RST（PC 约 0x1002afxx）。池顶偶发为空时跳过帘式动画。 */
        return false;
    }

#if GUI_USE_BLUR
    u16 type_blur = switch_mode & 0xff00;
    u8 blur_level = 0;
#endif

    switch_mode = switch_mode & 0xFF;
    switch (switch_mode) {
    case FUNC_SWITCH_MENU_DROPDOWN_UP:
    case FUNC_SWITCH_MENU_PULLUP_UP:
        //下拉往上, 上拉往上
        dy_auto = -SWITCHING_MENU_STEP;
        break;

    case FUNC_SWITCH_MENU_DROPDOWN_DOWN:
    case FUNC_SWITCH_MENU_PULLUP_DOWN:
        //下拉往下, 上拉往下
        dy_auto = SWITCHING_MENU_STEP;
        break;

    case FUNC_SWITCH_MENU_SIDE_POP:
        //边菜单往右拉出
        dx_auto = SWITCHING_MENU_STEP;
        break;

    case FUNC_SWITCH_MENU_SIDE_BACK:
        //边菜单往左退回
        dx_auto = -SWITCHING_MENU_STEP;
        break;

    default:
        halt(HALT_FUNC_SWITCH_MENU_MODE);
        break;
    }
    for (;;) {
        flag_pos = FLAG_POS_NORM;
        flag_change = false;
        if (flag_press) {
            flag_press = ctp_get_dxy(&dx, &dy);
            if (flag_press) {
                if (tick_check_expire(tick, SWITCHING_TICK_AUTO)) {
                    switch (switch_mode) {
                    case FUNC_SWITCH_MENU_DROPDOWN_UP:
                    case FUNC_SWITCH_MENU_PULLUP_UP:
                        //下拉往上, 上拉往上
                        if (dy <= -SWITCHING_MENU_DRAG_THRESHOLD) {
                            dy_auto = -SWITCHING_MENU_STEP;
                        } else {
                            dy_auto = SWITCHING_MENU_STEP;
                        }
                        break;

                    case FUNC_SWITCH_MENU_DROPDOWN_DOWN:
                    case FUNC_SWITCH_MENU_PULLUP_DOWN:
                        //下拉往下, 上拉往下
                        if (dy >= SWITCHING_MENU_DRAG_THRESHOLD) {
                            dy_auto = SWITCHING_MENU_STEP;
                        } else {
                            dy_auto = -SWITCHING_MENU_STEP;
                        }
                        break;

                    case FUNC_SWITCH_MENU_SIDE_POP:
                        //边菜单往右拉出
                        if (dx >= SWITCHING_MENU_DRAG_THRESHOLD) {
                            dx_auto = SWITCHING_MENU_STEP;
                        } else {
                            dx_auto = -SWITCHING_MENU_STEP;
                        }
                        break;

                    case FUNC_SWITCH_MENU_SIDE_BACK:
                        //边菜单往左退回
                        if (dx <= -SWITCHING_MENU_DRAG_THRESHOLD) {
                            dx_auto = -SWITCHING_MENU_STEP;
                        } else {
                            dx_auto = SWITCHING_MENU_STEP;
                        }
                        break;

                    default:
                        halt(HALT_FUNC_SWITCH_MENU_MODE);
                        break;
                    }
                }
            }
            flag_change = true;
        } else if (tick_check_expire(tick, SWITCHING_TICK_EXPIRE)) {
            tick = tick_get();
            dx += dx_auto;
            dy += dy_auto;
            flag_change = true;
        }
        if (flag_change) {
            switch (switch_mode) {
            case FUNC_SWITCH_MENU_DROPDOWN_UP:
            case FUNC_SWITCH_MENU_PULLUP_UP:
                //下拉往上, 上拉往上
                if (dy >= 0) {
                    dy = 0;
                    flag_pos = FLAG_POS_START;
                    ypos = GUI_SCREEN_CENTER_Y;
                } else if (dy <= -GUI_SCREEN_HEIGHT) {
                    dy = -GUI_SCREEN_HEIGHT;
                    flag_pos = FLAG_POS_END;
                    ypos = GUI_SCREEN_CENTER_Y - GUI_SCREEN_HEIGHT;
                }
                ypos = flag_pos == FLAG_POS_NORM ? GUI_SCREEN_CENTER_Y + dy : ypos;
                if (switch_mode == FUNC_SWITCH_MENU_PULLUP_UP) {
                    ypos += GUI_SCREEN_HEIGHT;
                }
                break;

            case FUNC_SWITCH_MENU_DROPDOWN_DOWN:
            case FUNC_SWITCH_MENU_PULLUP_DOWN:
                //下拉往下, 上拉往下
                if (dy <= 0) {
                    dy = 0;
                    flag_pos = FLAG_POS_START;
                    ypos = GUI_SCREEN_CENTER_Y - GUI_SCREEN_HEIGHT;
                } else if (dy >= GUI_SCREEN_HEIGHT) {
                    dy = GUI_SCREEN_HEIGHT;
                    flag_pos = FLAG_POS_END;
                    ypos = GUI_SCREEN_CENTER_Y;
                }
                ypos = flag_pos == FLAG_POS_NORM ? -GUI_SCREEN_CENTER_Y + dy : ypos;
                if (switch_mode == FUNC_SWITCH_MENU_PULLUP_DOWN) {
                    ypos += GUI_SCREEN_HEIGHT;
                }
                break;

            case FUNC_SWITCH_MENU_SIDE_POP:
                //边菜单往右拉出
                if (dx <= 0) {
                    dx = 0;
                    flag_pos = FLAG_POS_START;
                } else if (dx >= GUI_SIDE_MENU_WIDTH) {
                    dx = GUI_SIDE_MENU_WIDTH;
                    flag_pos = FLAG_POS_END;
                }
                xpos = -(GUI_SIDE_MENU_WIDTH / 2) + dx;
                break;

            case FUNC_SWITCH_MENU_SIDE_BACK:
                //边菜单往左退回
                if (dx >= 0) {
                    dx = 0;
                    flag_pos = FLAG_POS_START;
                } else if (dx <= -GUI_SIDE_MENU_WIDTH) {
                    dx = -GUI_SIDE_MENU_WIDTH;
                    flag_pos = FLAG_POS_END;
                }
                xpos = (GUI_SIDE_MENU_WIDTH / 2) + dx;
                break;

            default:
                halt(HALT_FUNC_SWITCH_MENU_MODE);
                break;
            }

#if GUI_USE_BLUR
            func_switching_blur_process(switch_mode, type_blur, ypos, &blur_level);
#endif

            compo_form_set_pos(sub_frm, xpos, ypos);
        }
        func_process();
        if (!flag_auto) {
            func_switching_message(msg_dequeue());
        }
        if (!flag_press && flag_pos != FLAG_POS_NORM) {
            break;
        }
    }
    return (flag_pos == FLAG_POS_END);
}

//缩放进出
static bool func_switching_zoom(u16 switch_mode, bool flag_auto, widget_icon_t *icon)
{
    compo_form_t *frm_back = compo_pool_get_bottom();
    compo_form_t *frm_cur = compo_pool_get_top();
    bool flag_press, flag_rolling, flag_roll;
    u8 flag_pos;                                                //当前状态
    bool flag_change;
    s32 dx = 0;
    s32 dx_target = 0;
    int dx_auto;
    rect_t area;
    s32 cur_rate_start = 0, cur_rate, cur_x, cur_y, cur_wid, cur_hei, cur_alpha, cur_show_hei;
    s32 cur_show_hei_delta_h, cur_show_hei_delta_s;
    s32 back_rate, back_x, back_y, back_wid, back_hei, back_alpha, back_hide_hei;
    s32 d_rate;
    u32 tick = tick_get();
    switch_mode = switch_mode & 0xFF;

    if (icon == NULL) {
        halt(HALT_FUNC_SWITCH_ZOOM_PTR);
    }
    widget_page_update();
    area = widget_get_absolute(icon);
    area.x -= GUI_SCREEN_CENTER_X;
    area.y -= GUI_SCREEN_CENTER_Y;
    if (!widget_is_icon(icon)) {
        halt(HALT_FUNC_SWITCH_ZOOM_PARAM);
    }
    if (area.wid <= 0 || area.hei <= 0) {
        return false;
    }
    compo_pool_set_top(frm_back);

    cur_show_hei = (area.hei * 3) >> 1;                             //图标与应用显示分界
    back_hide_hei = area.hei * 2;

    cur_show_hei_delta_h = back_hide_hei - area.hei;
    cur_show_hei_delta_s = GUI_SCREEN_HEIGHT - cur_show_hei;
    flag_press = false;
    flag_roll = false;
    flag_rolling = false;
    switch (switch_mode) {
    case FUNC_SWITCH_ZOOM_ENTER:
        flag_rolling = flag_roll = !flag_auto;   //手动滚动进入
        dx_target = dx + SWITCHING_ZOOM_ROLL_STEP;
        dx_auto = flag_auto ? SWITCHING_ZOOM_STEP : -SWITCHING_ZOOM_STEP;
        cur_rate = cur_rate_start = (area.hei << 16) / GUI_SCREEN_HEIGHT;
        back_rate = 0x10000;
        d_rate = (GUI_SCREEN_HEIGHT << 16) / (GUI_SCREEN_HEIGHT - area.hei);
        cur_alpha = 0;
        back_alpha = 255;
        break;

    case FUNC_SWITCH_ZOOM_EXIT:
        flag_press = !flag_auto;                                //手动滑动退出
        dx_auto = flag_auto ? SWITCHING_ZOOM_STEP : -SWITCHING_ZOOM_STEP;
        cur_rate = 0x10000;
        back_rate = (GUI_SCREEN_HEIGHT << 16) / area.hei;
        d_rate = (GUI_SCREEN_HEIGHT << 16) / (GUI_SCREEN_HEIGHT - area.hei);
        cur_alpha = 255;
        back_alpha = 0;
        break;

    default:
        halt(HALT_FUNC_SWITCH_ZOOM_MODE);
        return false;
    }
    cur_x = muls_shift16(muls_shift16(area.x, 0x10000 - cur_rate), d_rate);
    cur_y = muls_shift16(muls_shift16(area.y, 0x10000 - cur_rate), d_rate);
    back_x = muls_shift16(muls_shift16(area.x, 0x10000 - back_rate), d_rate);
    back_y = muls_shift16(muls_shift16(area.y, 0x10000 - back_rate), d_rate);
    cur_wid = muls_shift16(GUI_SCREEN_WIDTH, cur_rate);
    cur_hei = muls_shift16(GUI_SCREEN_HEIGHT, cur_rate);
    back_wid = muls_shift16(GUI_SCREEN_WIDTH, back_rate);
    back_hei = muls_shift16(GUI_SCREEN_HEIGHT, back_rate);
    compo_form_set_pos(frm_cur, GUI_SCREEN_CENTER_X + cur_x, GUI_SCREEN_CENTER_Y + cur_y);
    compo_form_set_pos(frm_back, GUI_SCREEN_CENTER_X + back_x, GUI_SCREEN_CENTER_Y + back_y);
    compo_form_scale_to(frm_cur, cur_wid, cur_hei);
    compo_form_scale_to(frm_back, back_wid, back_hei);
    compo_form_set_alpha(frm_cur, cur_alpha);
    widget_set_alpha(icon, back_alpha);

    for (;;) {
        flag_pos = FLAG_POS_NORM;
        flag_change = false;
        if (flag_press) {
            s32 dy;
            if (switch_mode == FUNC_SWITCH_ZOOM_EXIT) {
                flag_press = ctp_get_dxy(&dx, &dy);
                if (flag_press) {
                    if (dx <= SWITCHING_ZOOM_EXIT_THRESHOLD) {
                        dx_auto = -SWITCHING_ZOOM_STEP;
                    } else {
                        dx_auto = SWITCHING_ZOOM_STEP;
                    }
                }
            }
            flag_change = true;
        } else if (flag_rolling) {
            if (dx != dx_target) {
                if (tick_check_expire(tick, SWITCHING_TICK_EXPIRE)) {
                    tick = tick_get();
                    dx_auto = (dx_target - dx);
                    if (dx_auto > SWITCHING_ZOOM_ROLL_STEP_PER) {
                        dx_auto = (dx_target - dx) / SWITCHING_ZOOM_ROLL_KICK_CNT;
                        if (dx_auto < SWITCHING_ZOOM_ROLL_STEP_PER) {
                            dx_auto = SWITCHING_ZOOM_ROLL_STEP_PER;
                        }
                    } else if (dx_auto < -SWITCHING_ZOOM_ROLL_STEP_PER) {
                        dx_auto = (dx_target - dx) / SWITCHING_ZOOM_ROLL_KICK_CNT;
                        if (dx_auto > -SWITCHING_ZOOM_ROLL_STEP_PER) {
                            dx_auto = -SWITCHING_ZOOM_ROLL_STEP_PER;
                        }
                    }
                    dx += dx_auto;
                    flag_change = true;
                    if (dx >= GUI_SCREEN_WIDTH / 2) {
                        flag_roll = false;
                        flag_rolling = false;
                        dx_auto = SWITCHING_ZOOM_ROLL_STEP_PER * 2;
                    }
                }
            }
            if (tick_check_expire(tick, SWITCHING_ZOOM_ROLL_TICK_EXPIRE)) {
                dx_auto = -SWITCHING_ZOOM_ROLL_STEP * 2 / SWITCHING_ZOOM_ROLL_KICK_CNT;
                flag_rolling = false;
            }
        } else if (tick_check_expire(tick, SWITCHING_TICK_EXPIRE)) {
            tick = tick_get();
            dx += dx_auto;
            flag_change = true;
        }
        if (flag_change) {
            switch (switch_mode) {
            case FUNC_SWITCH_ZOOM_ENTER:
                //图标放大
                if (dx <= 0) {
                    dx = 0;
                    flag_pos = FLAG_POS_START;
                    flag_roll = false;
                } else if (dx >= GUI_SCREEN_WIDTH) {
                    dx = GUI_SCREEN_WIDTH;
                    flag_pos = FLAG_POS_END;
                    flag_roll = false;
                }
                if (dx == 0) {
                    cur_rate = cur_rate_start;
                    back_rate = 0x10000;
                } else {
                    cur_rate = cur_rate_start + (dx << 16) / GUI_SCREEN_WIDTH;
                    back_rate = (GUI_SCREEN_HEIGHT * cur_rate) / area.hei;
                    if (cur_rate >= 0x10000) {
                        cur_rate = 0x10000;
                        back_rate = (GUI_SCREEN_HEIGHT << 16) / area.hei;
                        flag_pos = FLAG_POS_END;
                    }
                }
                break;

            case FUNC_SWITCH_ZOOM_EXIT:
                //图标缩小
                if (dx <= 0) {
                    dx = 0;
                    flag_pos = FLAG_POS_START;
                } else if (dx >= GUI_SCREEN_WIDTH) {
                    dx = GUI_SCREEN_WIDTH;
                    flag_pos = FLAG_POS_END;
                }
                cur_rate = 0x10000 - (dx << 16) / GUI_SCREEN_WIDTH;
                back_rate = (GUI_SCREEN_HEIGHT * cur_rate) / area.hei;
                if (back_rate <= 0x10000) {
                    back_rate = 0x10000;
                    cur_rate = (area.hei << 16) / GUI_SCREEN_HEIGHT;
                    flag_pos = FLAG_POS_END;
                }
                break;

            default:
                halt(HALT_FUNC_SWITCH_ZOOM_MODE);
                break;
            }
            cur_x = muls_shift16(muls_shift16(area.x, 0x10000 - cur_rate), d_rate);
            cur_y = muls_shift16(muls_shift16(area.y, 0x10000 - cur_rate), d_rate);
            back_x = muls_shift16(muls_shift16(area.x, 0x10000 - back_rate), d_rate);
            back_y = muls_shift16(muls_shift16(area.y, 0x10000 - back_rate), d_rate);
            cur_wid = muls_shift16(GUI_SCREEN_WIDTH, cur_rate);
            cur_hei = muls_shift16(GUI_SCREEN_HEIGHT, cur_rate);
            back_wid = muls_shift16(GUI_SCREEN_WIDTH, back_rate);
            back_hei = muls_shift16(GUI_SCREEN_HEIGHT, back_rate);
            if (cur_hei >= cur_show_hei) {
                cur_alpha = SWITCHING_ZOOM_ALPHA + (255 - SWITCHING_ZOOM_ALPHA) * (cur_hei - cur_show_hei) / cur_show_hei_delta_s;
            } else {
                cur_alpha = 0;
            }
            if (cur_hei >= back_hide_hei) {
                back_alpha = 0;
            } else if (cur_hei > area.hei) {
                back_alpha = SWITCHING_ZOOM_ALPHA_BMAX - (SWITCHING_ZOOM_ALPHA_BMAX - SWITCHING_ZOOM_ALPHA) * (cur_hei - area.hei) / cur_show_hei_delta_h;
            } else {
                back_alpha = 255;
            }
            compo_form_set_pos(frm_cur, GUI_SCREEN_CENTER_X + cur_x, GUI_SCREEN_CENTER_Y + cur_y);
            compo_form_set_pos(frm_back, GUI_SCREEN_CENTER_X + back_x, GUI_SCREEN_CENTER_Y + back_y);
            compo_form_scale_to(frm_cur, cur_wid, cur_hei);
            compo_form_scale_to(frm_back, back_wid, back_hei);
            compo_form_set_alpha(frm_cur, cur_alpha);
            widget_set_alpha(icon, back_alpha);
        }
        func_process();
        if (!flag_auto) {
            if (flag_roll) {
                //获取滚动消息
                size_msg_t msg = msg_dequeue();
                switch (msg) {
                case MSG_QDEC_FORWARD:
                    tick = tick_get();
                    flag_rolling = true;
                    dx_target += SWITCHING_ZOOM_ROLL_STEP;
                    break;

                case MSG_QDEC_BACKWARD:
                    tick = tick_get();
                    flag_rolling = true;
                    dx_target -= SWITCHING_ZOOM_ROLL_STEP;
                    break;

                default:
                    func_switching_message(msg);
                    break;
                }
            } else {
                func_switching_message(msg_dequeue());
            }
        }
        if (flag_pos != FLAG_POS_NORM) {
            break;
        }
    }
    compo_pool_set_top(frm_cur);
    return (flag_pos == FLAG_POS_END);
}

//缩放淡入淡出
static bool func_switching_zoom_fade(u16 switch_mode, bool flag_auto)
{
    compo_form_t *frm_back = compo_pool_get_bottom();
    compo_form_t *frm_cur = compo_pool_get_top();
    u8 flag_pos;                                                //当前状态
    s32 back_rate, back_wid, back_hei;
    s32 cur_alpha, back_alpha;
    s32 zoom_rate;
    u32 tick = tick_get();
    switch_mode = switch_mode & 0xFF;

    //cur_show_hei = area.hei * 2;                                //图标与应用显示分界
    switch (switch_mode) {
    case FUNC_SWITCH_ZOOM_FADE_ENTER:
        back_rate = 0x10000;
        zoom_rate = SWITCHING_ZOOM_FADE_RATE;
        cur_alpha = 0;
        back_alpha = 255;
        break;

    case FUNC_SWITCH_ZOOM_FADE_EXIT:
        back_rate = SWITCHING_ZOOM_FADE_MAX;
        zoom_rate = (256 * 256 / SWITCHING_ZOOM_FADE_RATE);
        cur_alpha = 255;
        back_alpha = 0;
        break;

    default:
        halt(HALT_FUNC_SWITCH_ZOOM_MODE);
        return false;
    }

    back_wid = muls_shift16(GUI_SCREEN_WIDTH, back_rate);
    back_hei = muls_shift16(GUI_SCREEN_HEIGHT, back_rate);
    compo_form_scale_to(frm_back, back_wid, back_hei);
    compo_form_set_alpha(frm_cur, cur_alpha);
    compo_form_set_alpha(frm_back, back_alpha);
    for (;;) {
        flag_pos = FLAG_POS_NORM;
        if (tick_check_expire(tick, SWITCHING_TICK_EXPIRE)) {
            tick = tick_get();
            switch (switch_mode) {
            case FUNC_SWITCH_ZOOM_FADE_ENTER:
                //图标放大
                back_rate = (back_rate * zoom_rate) >> 8;
                if (back_rate >= SWITCHING_ZOOM_FADE_MAX) {
                    back_rate = SWITCHING_ZOOM_FADE_MAX;
                    flag_pos = FLAG_POS_END;
                }
                break;

            case FUNC_SWITCH_ZOOM_FADE_EXIT:
                //图标缩小
                back_rate = (back_rate * zoom_rate) >> 8;
                if (back_rate <= 0x10000) {
                    back_rate = 0x10000;
                    flag_pos = FLAG_POS_END;
                }
                break;

            default:
                halt(HALT_FUNC_SWITCH_ZOOM_MODE);
                break;
            }
            back_wid = muls_shift16(GUI_SCREEN_WIDTH, back_rate);
            back_hei = muls_shift16(GUI_SCREEN_HEIGHT, back_rate);
            if (back_rate > SWITCHING_ZOOM_FADE_1) {
                cur_alpha = 255 * (back_rate - SWITCHING_ZOOM_FADE_1) / (SWITCHING_ZOOM_FADE_MAX - SWITCHING_ZOOM_FADE_1);
            } else {
                cur_alpha = 0;
            }
            if (back_rate < SWITCHING_ZOOM_FADE_2) {
                back_alpha = 255 - 255 * (back_rate - 0x10000) / (SWITCHING_ZOOM_FADE_2 - 0x10000);
            } else {
                back_alpha = 0;
            }
            compo_form_scale_to(frm_back, back_wid, back_hei);
            compo_form_set_alpha(frm_cur, cur_alpha);
            compo_form_set_alpha(frm_back, back_alpha);
        }
        func_process();
        if (!flag_auto) {
            func_switching_message(msg_dequeue());
        }
        if (flag_pos != FLAG_POS_NORM) {
            break;
        }
    }
    return true;
}

//切换场景
bool func_switching(u16 switch_mode, void *param)
{
    if ((switch_mode & 0x7FFF) == FUNC_SWITCH_DIRECT) {
        return true;
    }
    if ((switch_mode & 0x7FFF) == FUNC_SWITCH_CANCEL) {
        halt(HALT_FUNC_SWITCH_MODE);
    }

#if ASR_SELECT
  bsp_asr_pause();
#endif

    sys_cb.flag_swithing = true;
    bool flag_auto = ((switch_mode & FUNC_SWITCH_AUTO) != 0);
    u8 mode = switch_mode & 0xFF;
    bool res = false;
    if (mode < FUNC_SWITCH_LR) {
        res = func_switching_fade(switch_mode, flag_auto);                     //淡入淡出
    } else if (mode < FUNC_SWITCH_UD) {
        res = func_switching_lr(switch_mode, flag_auto);                       //左右切换任务
    } else if (mode < FUNC_SWITCH_MENU) {
        res = func_switching_ud(switch_mode, flag_auto);                       //上下切换任务
    } else if (mode < FUNC_SWITCH_ZOOM) {
        res = func_switching_menu(switch_mode, flag_auto);                     //切换上下帘及边栏
    } else if (mode < FUNC_SWITCH_ZOOM_FADE) {
        res = func_switching_zoom(switch_mode, flag_auto, param);              //缩放进出
    } else if (mode < FUNC_SWITCH3D) {
        res = func_switching_zoom_fade(switch_mode, flag_auto);                //缩放加淡入淡出
#if GUI_USE_SCREENSHOOT
    } else {
        res = func_switching3d(switch_mode, flag_auto, param);                  //3D转场效果
    }
#else
    }
#endif
    sys_cb.flag_swithing = false;

#if ASR_SELECT
    bsp_asr_start();
#endif

    return res;
}
