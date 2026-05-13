#include "include.h"

#if GUI_USE_SCREENSHOOT

//转场速度控制
#define SWITCHING_TICK_AUTO                 180                                 //快速松手视为自动模式(ms)
#define SWITCHING_TICK_EXPIRE               18                                  //松手后自动切换单位时间(ms)

#define SWITCHING3D_LR_ANGLE                900                                 //3D左右转场90度
#define SWITCHING3D_LR_STEP                 (SWITCHING3D_LR_ANGLE / 12)         //3D左右转场松手后自动切换角度单位步进
#define SWITCHING3D_LR_DRAG_THRESHOLD       (SWITCHING3D_LR_ANGLE / 3)          //3D左右转场松手自动的判断角度阈值(需要拉多少距离才能自动完成)

#define SWITCHING3D_ROTA_LR_ANGLE           600                                     //3D左右旋转转场60度
#define SWITCHING3D_ROTA_LR_STEP            (SWITCHING3D_ROTA_LR_ANGLE / 12)         //3D左右转场松手后自动切换角度单位步进
#define SWITCHING3D_ROTA_LR_DRAG_THRESHOLD  (SWITCHING3D_ROTA_LR_ANGLE / 3)          //3D左右转场松手自动的判断角度阈值(需要拉多少距离才能自动完成)
#define SWITCHING3D_ROTA_LR_AXIS_Y_OFFSET   (GUI_SCREEN_WIDTH)                      //旋转轴y方向屏幕底部开始偏移量，GUI_SCREEN_WIDTH/2/sin(SWITCHING3D_ROTA_LR_ANGLE/2)

#define SWITCHING3D_LR_FLIP_ANGLE           1800                                //3D翻转180度
#define SWITCHING3D_LR_FLIP_STEP            (SWITCHING3D_LR_FLIP_ANGLE / 12)    //3D左右转场松手后自动切换角度单位步进
#define SWITCHING3D_LR_FLIP_DRAG_THRESHOLD  (SWITCHING3D_LR_FLIP_ANGLE / 3)     //3D左右转场松手自动的判断角度阈值(需要拉多少距离才能自动完成)
#define SWITCHING3D_LR_FLIP_Z               (-SWITCHING3D_LR_R)                 //3D翻转时调整Z

#define SWITCHING3D_LR_FOLD_ANGLE           1800                                //3D翻转180度
#define SWITCHING3D_LR_FOLD_STEP            (SWITCHING3D_LR_FOLD_ANGLE / 12)    //3D左右转场松手后自动切换角度单位步进
#define SWITCHING3D_LR_FOLD_DRAG_THRESHOLD  (SWITCHING3D_LR_FOLD_ANGLE / 3)     //3D左右转场松手自动的判断角度阈值(需要拉多少距离才能自动完成)

#define SWITCHING3D_LR_PLATE_ANGLE           1800                                 //3D翻转180度
#define SWITCHING3D_LR_PLATE_STEP            (SWITCHING3D_LR_PLATE_ANGLE / 12)    //3D左右转场松手后自动切换角度单位步进
#define SWITCHING3D_LR_PLATE_DRAG_THRESHOLD  (SWITCHING3D_LR_PLATE_ANGLE / 3)     //3D左右转场松手自动的判断角度阈值(需要拉多少距离才能自动完成)
#define SWITCHING3D_LR_PLATE_THICKNESS       15                                   //翻板厚度

#define SWITCHING3D_LR_DRIFT_ANGLE           900                                 //3D翻转90度
#define SWITCHING3D_LR_DRIFT_STEP            (SWITCHING3D_LR_DRIFT_ANGLE / 12)    //3D左右转场松手后自动切换角度单位步进
#define SWITCHING3D_LR_DRIFT_DRAG_THRESHOLD  (SWITCHING3D_LR_DRIFT_ANGLE / 3)     //3D左右转场松手自动的判断角度阈值(需要拉多少距离才能自动完成)
#define SWITCHING3D_LR_FRONT_DISTANCE        1500
#define SWITCHING3D_LR_REAR_DISTANCE         300

#define SWITCHING3D_LR_DISTANCE             600                                     //3D视距(影响透视效果)
#define SWITCHING3D_LR_R                    (GUI_SCREEN_WIDTH / 2)                  //3D旋转球半径


#define SWITCHING3D_LR_REFLECT_ALPHA        (0xff / 3)                              //倒影的透明度

void func_switching_message(size_msg_t msg);
extern u8 heap_func[HEAP_FUNC_SIZE];


#if GUI_USE_SCREENSHOOT
//截图
void func_switch_screenshot(void *cur_scbuf, void *next_scbuf, u8 next_sta)
{
    os_gui_draw_w4_done();
    sys_cb.flag_swithing = true;

    if (cur_scbuf) {
        gui_set_screenshot(cur_scbuf);
        compo_update();                                     //更新组件
        gui_refresh();
        os_gui_draw_force();
        os_gui_draw_w4_done();

        while(gui_get_screenshot());
    }

    if (next_scbuf) {
        compo_form_t *frm_cur = compo_pool_get_top();
        compo_form_destroy(frm_cur);
        gui_set_screenshot(next_scbuf);
        compo_form_t *frm_next = func_create_form(next_sta);                        //创建下一个任务的窗体
        compo_update();                                     //更新组件
        gui_refresh();

        os_gui_draw_force();
        os_gui_draw_w4_done();
        while(gui_get_screenshot());
        compo_form_destroy(frm_next);
        func_create_form(func_cb.sta);
    }
}
#endif


//创建截图窗体, flag_next 是否创建下一个任务截图窗体
compo_form_t *func_form_create_by_screenshoot(bool flag_next)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    void *sc_ptr = flag_next ? next_scbuf : cur_scbuf;

    //新建图标
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_ram(pic, sc_ptr);
    if (flag_next && sys_cb.nav_index == NAV_FLIP) {
        widget_set_visible(frm->page, false);
    }

    return frm;
}

//创建截图窗体-用于折叠翻页  lr_dir: 0:左翻  1:右翻
compo_form_t *func_form_create_screenshoot_for_fold(void *cur_draw_buf, void *new_draw_buf, bool lr_dir)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //bg gape
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_axis3d_t *axis = widget_axis3d_create(page);
    widget_page3d_set_axis(page, axis);
    widget_axis3d_set_distance(axis, lr_dir ? -SWITCHING3D_LR_DISTANCE : SWITCHING3D_LR_DISTANCE);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    for (u8 i = 0; i < 2; i++) {
        void *img = widget_image_create(page, 0);
        widget_set_pos(img, GUI_SCREEN_CENTER_X / 2 + GUI_SCREEN_CENTER_X * i, GUI_SCREEN_CENTER_Y);
        widget_image_set_ram(img, i == lr_dir ? cur_draw_buf : new_draw_buf);
        widget_image_cut(img, GUI_SCREEN_CENTER_X * i, 0, GUI_SCREEN_WIDTH / 2, GUI_SCREEN_HEIGHT);
    }

    //fold page
    page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    void *img = widget_image_create(page, 0);
    widget_set_pos(img, GUI_SCREEN_CENTER_X / 2 + GUI_SCREEN_CENTER_X * (!lr_dir), GUI_SCREEN_CENTER_Y);
    widget_image_set_ram(img, cur_draw_buf);
    widget_image_cut(img, GUI_SCREEN_CENTER_X * (!lr_dir), 0, GUI_SCREEN_WIDTH / 2, GUI_SCREEN_HEIGHT);
    frm->axis = widget_axis3d_create(page);
    widget_page3d_set_axis(page, frm->axis);
    widget_axis3d_set_distance(frm->axis, lr_dir ? -SWITCHING3D_LR_DISTANCE : SWITCHING3D_LR_DISTANCE);

    return frm;
}

//创建截图窗体-翻板效果     lr_dir: 0:左翻  1:右翻
compo_form_t *func_form_create_for_plate(void *cur_draw_buf, void *new_draw_buf, bool lr_dir)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);
    frm->axis = widget_axis3d_create(frm->page);
    widget_axis3d_set_pos(frm->axis, 0, 0, -GUI_SCREEN_WIDTH);
    for (u8 i = 0; i < 3; i++) {
        widget_page_t *page = widget_page_create(frm->page_body);
        widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
        if (i < 2) {
            void *img = widget_image_create(page, 0);
            widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
            widget_page3d_set_axis(page, frm->axis);
            widget_page3d_set_r(page, SWITCHING3D_LR_PLATE_THICKNESS);
            if (i == 0) {
                widget_image_set_ram(img, new_draw_buf);
                widget_page3d_set_azimuth(page, lr_dir ? 0 : SWITCHING3D_LR_PLATE_ANGLE);
            } else {
                widget_image_set_ram(img, cur_draw_buf);
                widget_page3d_set_azimuth(page, lr_dir ? SWITCHING3D_LR_PLATE_ANGLE : 0);
            }
        } else { //厚度
            void *shape = widget_rect_create(page);
            widget_set_location(shape, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, SWITCHING3D_LR_PLATE_THICKNESS * 2, GUI_SCREEN_HEIGHT);
            widget_rect_set_color(shape, COLOR_GRAY);
            widget_page3d_set_azimuth(page, -900);
            widget_page3d_set_axis(page, frm->axis);
            widget_page3d_set_r(page, -GUI_SCREEN_WIDTH / 2);
        }
    }

    return frm;
}

//创建截图窗体-漂移折叠效果 lr_dir: 0:左翻  1:右翻
compo_form_t *func_form_create_for_drift(void *cur_draw_buf, void *new_draw_buf, bool lr_dir)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);
    widget_page_t *page = NULL;
    widget_axis3d_t *axis3d = NULL;
    void *img = NULL;

    for (u8 i = 0; i < 2; i++) {
        //创建page
        page = widget_page_create(frm->page_body);
        widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

        //创建轴
        axis3d = widget_axis3d_create(page);
        widget_page3d_set_axis(page, axis3d);
        if (i != lr_dir) {  //左轴
            widget_axis3d_set_distance(axis3d, lr_dir ? SWITCHING3D_LR_FRONT_DISTANCE : SWITCHING3D_LR_REAR_DISTANCE);
            widget_axis3d_set_pos(axis3d, -GUI_SCREEN_CENTER_X, 0, 0);
            widget_page3d_set_rotation_center(page, GUI_SCREEN_WIDTH, GUI_SCREEN_CENTER_Y);
            widget_set_flip(page, true);
        } else {            //右轴
            widget_axis3d_set_distance(axis3d, lr_dir ? SWITCHING3D_LR_REAR_DISTANCE : SWITCHING3D_LR_FRONT_DISTANCE);
            widget_axis3d_set_pos(axis3d, GUI_SCREEN_CENTER_X, 0, 0);
            widget_page3d_set_rotation_center(page, GUI_SCREEN_WIDTH, GUI_SCREEN_CENTER_Y);
        }

        //截图
        img = widget_image_create(page, 0);
        widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
        void *p_buf = NULL;
        p_buf = i ? cur_draw_buf : new_draw_buf;
        widget_image_set_ram(img, p_buf);

        img = widget_image_create(page, 0);
        widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + GUI_SCREEN_HEIGHT);
        widget_image_set_ram(img, p_buf);
        widget_image_set_rotation(img, 1800);
        widget_set_flip(img, true);
        widget_set_alpha(img, SWITCHING3D_LR_REFLECT_ALPHA);
    }

    return frm;
}


void func_switching3d_form_create(u16 switch_mode, u16 sta, compo_form_t **cur_frm, compo_form_t **new_frm)
{
    *cur_frm = NULL;
    *new_frm = NULL;
    u8 mode = switch_mode & 0xff;
#if GUI_USE_BLUR
    u16 type_blur = switch_mode & 0xff00;
#endif

    func_switch_screenshot(cur_scbuf, next_scbuf, sta);
    compo_form_t *frm_cur = compo_pool_get_top();
    func_cur_sta_exit();
    compo_form_destroy(frm_cur);                                     //切换完成或取消，销毁窗体
    if (func_cb.f_cb != NULL) {
        func_free(func_cb.f_cb);
    }

    switch (mode) {
        case FUNC_SWITCH3D_LR_FOLD_LEFT:
        case FUNC_SWITCH3D_LR_FOLD_RIGHT:
            *cur_frm = func_form_create_screenshoot_for_fold(cur_scbuf, next_scbuf, mode == FUNC_SWITCH3D_LR_FOLD_RIGHT);
            break;

        case FUNC_SWITCH3D_LR_PLATE_FLIP_LEFT:
        case FUNC_SWITCH3D_LR_PLATE_FLIP_RIGHT:
            *cur_frm = func_form_create_for_plate(cur_scbuf, next_scbuf, mode == FUNC_SWITCH3D_LR_PLATE_FLIP_RIGHT);
            break;

        case FUNC_SWITCH3D_LR_DRIFT_LEFT:
        case FUNC_SWITCH3D_LR_DRIFT_RIGHT:
            *cur_frm = func_form_create_for_drift(cur_scbuf, next_scbuf, mode == FUNC_SWITCH3D_LR_DRIFT_RIGHT);
            break;

        default:
#if GUI_USE_BLUR
            compo_form_t *func_clock_form_create_by_screenshoot(void);
            if (type_blur) {
                *cur_frm = func_clock_form_create_by_screenshoot();
            } else
#endif
            {
                *cur_frm = func_form_create_by_screenshoot(false);
            }
            if (mode != FUNC_SWITCH_FADE_OUT) {
                *new_frm = func_form_create_by_screenshoot(true);                  //创建上一个任务的窗体
            }
            break;
    }
}



//3D左右切换 内切换 翻转
static bool func_switching3d_lr(u16 switch_mode, bool flag_auto)
{
    compo_form_t *frm_last = compo_pool_get_bottom();
    compo_form_t *frm_cur = compo_pool_get_top();
    bool flag_press = !flag_auto;
    u8 flag_pos;                            //当前坐标状态
    bool flag_change;
    s32 da = 0;
    int da_auto;
    u32 tick = tick_get();
    if (frm_last == NULL || frm_cur == NULL) {
        halt(HALT_FUNC_SWITCH_LR_PTR);
    }

    if (switch_mode == FUNC_SWITCH3D_LR_ROTA_RIGHT ||
        switch_mode == FUNC_SWITCH3D_LR_ROTA_LEFT) {
        s16 rota_y = -GUI_SCREEN_HEIGHT / 2 - SWITCHING3D_ROTA_LR_AXIS_Y_OFFSET;
        compo_form3d_set_axis_pos(frm_cur, 0, rota_y, 0);
        compo_form3d_set_axis_polar(frm_cur, 0);
        compo_form3d_set_polar(frm_last, 0);
        compo_form3d_set_polar(frm_cur, 0);
        compo_form3d_set_rotation_center(frm_last, GUI_SCREEN_WIDTH / 2, GUI_SCREEN_HEIGHT / 2 + abs(rota_y));
        compo_form3d_set_rotation_center(frm_cur, GUI_SCREEN_WIDTH / 2, GUI_SCREEN_HEIGHT / 2 + abs(rota_y));
    } else {
        compo_form3d_set_r(frm_cur, SWITCHING3D_LR_R);
        compo_form3d_set_r(frm_last, SWITCHING3D_LR_R);
        compo_form3d_set_azimuth(frm_last, 0);
        compo_form3d_set_axis_distance(frm_cur, SWITCHING3D_LR_DISTANCE);
    }

    switch (switch_mode) {
    case FUNC_SWITCH3D_LR_LEFT:
        //左划，顺时针从右转入
        compo_form3d_set_axis_pos(frm_cur, 0, 0, -SWITCHING3D_LR_R);
        compo_form3d_set_azimuth(frm_cur, SWITCHING3D_LR_ANGLE);
        compo_form3d_auto_visible(frm_cur);
        da_auto = SWITCHING3D_LR_STEP;
        break;

    case FUNC_SWITCH3D_LR_RIGHT:
        //右划，逆时针从左转入
        compo_form3d_set_axis_pos(frm_cur, 0, 0, -SWITCHING3D_LR_R);
        compo_form3d_set_azimuth(frm_cur, -SWITCHING3D_LR_ANGLE);
        compo_form3d_auto_visible(frm_cur);
        da_auto = -SWITCHING3D_LR_STEP;
        break;

    case FUNC_SWITCH3D_LR_IN_LEFT:
        //左划，逆时针从右转入
        compo_form3d_set_axis_pos(frm_cur, 0, 0, SWITCHING3D_LR_R);
        compo_form3d_set_azimuth(frm_cur, -SWITCHING3D_LR_ANGLE);
        compo_form3d_set_axis_rotation(frm_cur, -SWITCHING3D_LR_ANGLE);
        compo_form_set_flip(frm_cur, true);
        compo_form_set_flip(frm_last, true);
        da_auto = -SWITCHING3D_LR_STEP;
        break;

    case FUNC_SWITCH3D_LR_IN_RIGHT:
        //右划，顺时针从左转入
        compo_form3d_set_axis_pos(frm_cur, 0, 0, SWITCHING3D_LR_R);
        compo_form3d_set_azimuth(frm_cur, SWITCHING3D_LR_ANGLE);
        compo_form3d_set_axis_rotation(frm_cur, -SWITCHING3D_LR_ANGLE);
        compo_form_set_flip(frm_cur, true);
        compo_form_set_flip(frm_last, true);
        da_auto = SWITCHING3D_LR_STEP;
        break;

    case FUNC_SWITCH3D_LR_FLIP_LEFT:
        //左划，顺时针翻转
        compo_form3d_set_r(frm_cur, 0);
        compo_form3d_set_r(frm_last, 0);
        compo_form3d_set_azimuth(frm_cur, SWITCHING3D_LR_FLIP_ANGLE);
        da_auto = SWITCHING3D_LR_STEP;
        break;

    case FUNC_SWITCH3D_LR_FLIP_RIGHT:
        //右划，逆时针翻转
        compo_form3d_set_r(frm_cur, 0);
        compo_form3d_set_r(frm_last, 0);
        compo_form3d_set_azimuth(frm_cur, -SWITCHING3D_LR_FLIP_ANGLE);
        da_auto = -SWITCHING3D_LR_STEP;
        break;

    case FUNC_SWITCH3D_LR_ROTA_LEFT:
        //左划，逆时针从右旋转
        compo_form3d_set_rotation(frm_cur, SWITCHING3D_ROTA_LR_ANGLE);
        da_auto = -SWITCHING3D_ROTA_LR_STEP;
        break;

    case FUNC_SWITCH3D_LR_ROTA_RIGHT:
        //右划，顺时针从左转入
        compo_form3d_set_rotation(frm_cur, -SWITCHING3D_ROTA_LR_ANGLE);
        da_auto = SWITCHING3D_ROTA_LR_STEP;
        break;

    default:
        halt(HALT_FUNC_SWITCH_LR_MODE);
        return false;
    }
    compo_form3d_set_axis(frm_last, frm_cur);

    for (;;) {
        flag_pos = FLAG_POS_NORM;
        flag_change = false;
        if (flag_press) {
            s32 dx, dy;
            flag_press = ctp_get_dxy(&dx, &dy);

            switch (switch_mode) {
                case FUNC_SWITCH3D_LR_LEFT:
                case FUNC_SWITCH3D_LR_RIGHT:
                    da = -dx * SWITCHING3D_LR_ANGLE / GUI_SCREEN_WIDTH;
                    break;

                case FUNC_SWITCH3D_LR_IN_LEFT:
                case FUNC_SWITCH3D_LR_IN_RIGHT:
                    da = dx * SWITCHING3D_LR_ANGLE / GUI_SCREEN_WIDTH;
                    break;

                case FUNC_SWITCH3D_LR_FLIP_LEFT:
                case FUNC_SWITCH3D_LR_FLIP_RIGHT:
                    da = -dx * SWITCHING3D_LR_FLIP_ANGLE / GUI_SCREEN_WIDTH;
                    break;

                case FUNC_SWITCH3D_LR_ROTA_LEFT:
                case FUNC_SWITCH3D_LR_ROTA_RIGHT:
                    da = dx * SWITCHING3D_ROTA_LR_ANGLE / GUI_SCREEN_WIDTH;
                    break;

                default:
                    halt(HALT_FUNC_SWITCH_MENU_MODE);
                    return false;
                }

            if (flag_press) {
                if (tick_check_expire(tick, SWITCHING_TICK_AUTO)) {
                    switch (switch_mode) {
                    case FUNC_SWITCH3D_LR_LEFT:
                        if (da >= SWITCHING3D_LR_DRAG_THRESHOLD) {
                            da_auto = SWITCHING3D_LR_STEP;
                        } else {
                            da_auto = -SWITCHING3D_LR_STEP;
                        }
                        break;

                    case FUNC_SWITCH3D_LR_RIGHT:
                        if (da <= -SWITCHING3D_LR_DRAG_THRESHOLD) {
                            da_auto = -SWITCHING3D_LR_STEP;
                        } else {
                            da_auto = SWITCHING3D_LR_STEP;
                        }
                        break;

                    case FUNC_SWITCH3D_LR_IN_LEFT:
                        if (da <= -SWITCHING3D_LR_DRAG_THRESHOLD) {
                            da_auto = -SWITCHING3D_LR_STEP;
                        } else {
                            da_auto = SWITCHING3D_LR_STEP;
                        }
                        break;

                    case FUNC_SWITCH3D_LR_IN_RIGHT:
                        if (da >= SWITCHING3D_LR_DRAG_THRESHOLD) {
                            da_auto = SWITCHING3D_LR_STEP;
                        } else {
                            da_auto = -SWITCHING3D_LR_STEP;
                        }
                        break;

                    case FUNC_SWITCH3D_LR_FLIP_LEFT:
                        if (da >= SWITCHING3D_LR_FLIP_DRAG_THRESHOLD) {
                            da_auto = SWITCHING3D_LR_FLIP_STEP;
                        } else {
                            da_auto = -SWITCHING3D_LR_FLIP_STEP;
                        }
                        break;

                    case FUNC_SWITCH3D_LR_FLIP_RIGHT:
                        if (da <= -SWITCHING3D_LR_FLIP_DRAG_THRESHOLD) {
                            da_auto = -SWITCHING3D_LR_FLIP_STEP;
                        } else {
                            da_auto = SWITCHING3D_LR_FLIP_STEP;
                        }
                        break;

                    case FUNC_SWITCH3D_LR_ROTA_LEFT:
                        da = dx * SWITCHING3D_ROTA_LR_ANGLE / GUI_SCREEN_WIDTH;
                        if (da <= -SWITCHING3D_ROTA_LR_DRAG_THRESHOLD) {
                            da_auto = -SWITCHING3D_ROTA_LR_STEP;
                        } else {
                            da_auto = SWITCHING3D_ROTA_LR_STEP;
                        }
                        break;

                    case FUNC_SWITCH3D_LR_ROTA_RIGHT:
                        da = dx * SWITCHING3D_ROTA_LR_ANGLE / GUI_SCREEN_WIDTH;
                        if (da >= SWITCHING3D_ROTA_LR_DRAG_THRESHOLD) {
                            da_auto = SWITCHING3D_ROTA_LR_STEP;
                        } else {
                            da_auto = -SWITCHING3D_ROTA_LR_STEP;
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
            da += da_auto;
            flag_change = true;
        }

        if (flag_change) {
            switch (switch_mode) {
            case FUNC_SWITCH3D_LR_LEFT:
                //左划，顺时针从右转入
                if (da <= 0) {
                    da = 0;
                    flag_pos = FLAG_POS_START;
                } else if (da >= SWITCHING3D_LR_ANGLE) {
                    da = SWITCHING3D_LR_ANGLE;
                    flag_pos = FLAG_POS_END;
                }
                compo_form3d_set_axis_rotation(frm_cur, 900 + da);
                compo_form3d_set_axis_pos(frm_cur, 0, 0, muls_shift16(-SWITCHING3D_LR_R * 1.414, COS(450 - da)));
                break;

            case FUNC_SWITCH3D_LR_RIGHT:
                //右划，逆时针从左转入
                if (da >= 0) {
                    da = 0;
                    flag_pos = FLAG_POS_START;
                } else if (da <= -SWITCHING3D_LR_ANGLE) {
                    da = -SWITCHING3D_LR_ANGLE;
                    flag_pos = FLAG_POS_END;
                }
                compo_form3d_set_axis_rotation(frm_cur, 900 + da);
                compo_form3d_set_axis_pos(frm_cur, 0, 0, muls_shift16(-SWITCHING3D_LR_R * 1.414, COS(450 + da)));
                break;

            case FUNC_SWITCH3D_LR_IN_LEFT:
                //左划，逆时针从右转入
                if (da >= 0) {
                    da = 0;
                    flag_pos = FLAG_POS_START;
                } else if (da <= -SWITCHING3D_LR_ANGLE) {
                    da = -SWITCHING3D_LR_ANGLE;
                    flag_pos = FLAG_POS_END;
                }
                compo_form3d_set_axis_rotation(frm_cur, -900 + da);
                compo_form3d_set_axis_pos(frm_cur, 0, 0, SWITCHING3D_LR_R * 2 - muls_shift16(SWITCHING3D_LR_R * 1.414, COS(450 + da)));
                break;

            case FUNC_SWITCH3D_LR_IN_RIGHT:
                //右划，顺时针从左转入
                if (da <= 0) {
                    da = 0;
                    flag_pos = FLAG_POS_START;
                } else if (da >= SWITCHING3D_LR_ANGLE) {
                    da = SWITCHING3D_LR_ANGLE;
                    flag_pos = FLAG_POS_END;
                }
                compo_form3d_set_axis_rotation(frm_cur, -900 + da);
                compo_form3d_set_axis_pos(frm_cur, 0, 0, SWITCHING3D_LR_R * 2 - muls_shift16(SWITCHING3D_LR_R * 1.414, COS(450 - da)));
                break;

            case FUNC_SWITCH3D_LR_FLIP_LEFT:
                //左划，顺时针翻转
                if (da <= 0) {
                    da = 0;
                    flag_pos = FLAG_POS_START;
                } else if (da >= SWITCHING3D_LR_FLIP_ANGLE) {
                    da = SWITCHING3D_LR_FLIP_ANGLE;
                    flag_pos = FLAG_POS_END;
                }
                compo_form3d_set_axis_rotation(frm_cur, 900 + da);
                compo_form3d_set_axis_pos(frm_cur, 0, 0, muls_shift16(SWITCHING3D_LR_FLIP_Z, SIN(da)));
                break;

            case FUNC_SWITCH3D_LR_FLIP_RIGHT:
                //右划，逆时针翻转
                if (da >= 0) {
                    da = 0;
                    flag_pos = FLAG_POS_START;
                } else if (da <= -SWITCHING3D_LR_FLIP_ANGLE) {
                    da = -SWITCHING3D_LR_FLIP_ANGLE;
                    flag_pos = FLAG_POS_END;
                }
                compo_form3d_set_axis_rotation(frm_cur, 900 + da);
                compo_form3d_set_axis_pos(frm_cur, 0, 0, muls_shift16(SWITCHING3D_LR_FLIP_Z, SIN(-da)));
                break;

            case FUNC_SWITCH3D_LR_ROTA_LEFT:
                 //左划，顺时针从右转入
                if (da >= 0) {
                    da = 0;
                    flag_pos = FLAG_POS_START;
                } else if (da <= -SWITCHING3D_ROTA_LR_ANGLE) {
                    da = -SWITCHING3D_ROTA_LR_ANGLE;
                    flag_pos = FLAG_POS_END;
                }
                compo_form3d_set_axis_rotation(frm_cur, 900 + da);
                break;

            case FUNC_SWITCH3D_LR_ROTA_RIGHT:
                 //右划，逆时针从左转入
                if (da <= 0) {
                    da = 0;
                    flag_pos = FLAG_POS_START;
                } else if (da >= SWITCHING3D_ROTA_LR_ANGLE) {
                    da = SWITCHING3D_ROTA_LR_ANGLE;
                    flag_pos = FLAG_POS_END;
                }
                compo_form3d_set_axis_rotation(frm_cur, 900 + da);
                break;

            default:
                halt(HALT_FUNC_SWITCH_LR_MODE);
                return false;
            }
            compo_form3d_auto_visible(frm_cur);
            compo_form3d_auto_visible(frm_last);
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

//3D左右折叠翻转
static bool func_switching3d_lr_fold(u16 switch_mode, bool flag_auto)
{
    compo_form_t *frm_cur = compo_pool_get_top();

    bool flag_press = !flag_auto;
    u8 flag_pos;                            //当前坐标状态
    bool flag_change;
    s32 da = 0;
    int da_auto = SWITCHING3D_LR_FOLD_STEP;
    u32 tick = tick_get();
    if (frm_cur == NULL) {
        halt(HALT_FUNC_SWITCH_LR_PTR);
    }
    bool lr_dir = (bool)(FUNC_SWITCH3D_LR_FOLD_LEFT != switch_mode);

    widget_t *widget = NULL;
    widget_page_t *bg_page = NULL, *fold_page = NULL;

    //旋转轴
    widget = widget_get_next(frm_cur->page_body);
    while (widget != NULL)
    {
        if (widget_is_page(widget)) {
            if (NULL == bg_page) {
                bg_page = widget;
            } else if (NULL == fold_page) {
                fold_page = widget;
                break;
            }
        }
        widget = widget_get_next(widget);
    }

    for (;;) {
        flag_pos = FLAG_POS_NORM;
        flag_change = false;
        if (flag_press) {
            s32 dx, dy;
            flag_press = ctp_get_dxy(&dx, &dy);
            if (!lr_dir) {
                da = -dx * SWITCHING3D_LR_FOLD_ANGLE / GUI_SCREEN_WIDTH;
            } else {
                da = dx * SWITCHING3D_LR_FOLD_ANGLE / GUI_SCREEN_WIDTH;
            }

            if (flag_press) {
                if (tick_check_expire(tick, SWITCHING_TICK_AUTO)) {
                    if (!lr_dir) {
                        da = -dx * SWITCHING3D_LR_FOLD_ANGLE / GUI_SCREEN_WIDTH;
                    } else {
                        da = dx * SWITCHING3D_LR_FOLD_ANGLE / GUI_SCREEN_WIDTH;
                    }
                    if (da >= SWITCHING3D_LR_FOLD_DRAG_THRESHOLD) {
                        da_auto = SWITCHING3D_LR_FOLD_STEP;
                    } else {
                        da_auto = -SWITCHING3D_LR_FOLD_STEP;
                    }
                }
            }
            flag_change = true;
        } else if (tick_check_expire(tick, SWITCHING_TICK_EXPIRE)) {
            tick = tick_get();
            da += da_auto;
            flag_change = true;
        }

        if (flag_change) {
            if (da <= 0) {
                da = 0;
                flag_pos = FLAG_POS_START;
            } else if (da >= SWITCHING3D_LR_FOLD_ANGLE) {
                da = SWITCHING3D_LR_FOLD_ANGLE;
                flag_pos = FLAG_POS_END;
            }
            void *img = widget_get_next(fold_page);
            if (da > 900) {
                widget_set_flip(fold_page, true);
                widget_image_set_ram(img, next_scbuf);
                widget_set_pos(img, GUI_SCREEN_CENTER_X / 2 + GUI_SCREEN_CENTER_X * (lr_dir), GUI_SCREEN_CENTER_Y);
                widget_image_cut(img, GUI_SCREEN_CENTER_X * (lr_dir), 0, GUI_SCREEN_WIDTH / 2, GUI_SCREEN_HEIGHT);
            } else {
                widget_set_flip(fold_page, false);
                widget_image_set_ram(img, cur_scbuf);
                widget_set_pos(img, GUI_SCREEN_CENTER_X / 2 + GUI_SCREEN_CENTER_X * (!lr_dir), GUI_SCREEN_CENTER_Y);
                widget_image_cut(img, GUI_SCREEN_CENTER_X * (!lr_dir), 0, GUI_SCREEN_WIDTH / 2, GUI_SCREEN_HEIGHT);
            }
            compo_form3d_set_axis_rotation(frm_cur, da + 900);
            s16 z = lr_dir ? muls_shift16(SWITCHING3D_LR_FLIP_Z, SIN(-da)) : muls_shift16(SWITCHING3D_LR_FLIP_Z, SIN(da));
            compo_form3d_set_axis_pos(frm_cur, 0, 0, z);
            widget_axis3d_set_pos(widget_get_next(bg_page), 0, 0, z);
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

//3D左右翻板
static bool func_switching3d_lr_plate_flip(u16 switch_mode, bool flag_auto)
{
    compo_form_t *frm_cur = compo_pool_get_top();

    bool flag_press = !flag_auto;
    u8 flag_pos;                            //当前坐标状态
    bool flag_change;
    s32 da = 0;
    int da_auto = SWITCHING3D_LR_PLATE_STEP;
    u32 tick = tick_get();
    if (frm_cur == NULL) {
        halt(HALT_FUNC_SWITCH_LR_PTR);
    }

    widget_page_t *cur_page = NULL, *new_page = NULL, *gap_page = NULL;
    widget_t *widget = widget_get_next(frm_cur->page_body);
    while (widget != NULL)
    {
        if (widget_is_page(widget)) {
            if (NULL == new_page) {
                new_page = widget;
            } else if (NULL == cur_page) {
                cur_page = widget;
            } else if (NULL == gap_page) {
                gap_page = widget;
                break;
            }
        }
        widget = widget_get_next(widget);
    }

    for (;;) {
        flag_pos = FLAG_POS_NORM;
        flag_change = false;
        if (flag_press) {
            s32 dx, dy;
            flag_press = ctp_get_dxy(&dx, &dy);
            if (FUNC_SWITCH3D_LR_PLATE_FLIP_LEFT == switch_mode) {
                da = -dx * SWITCHING3D_LR_PLATE_ANGLE / GUI_SCREEN_WIDTH;
            } else {
                da = dx * SWITCHING3D_LR_PLATE_ANGLE / GUI_SCREEN_WIDTH;
            }

            if (flag_press) {
                if (tick_check_expire(tick, SWITCHING_TICK_AUTO)) {
                    if (FUNC_SWITCH3D_LR_PLATE_FLIP_LEFT == switch_mode) {
                        da = -dx * SWITCHING3D_LR_PLATE_ANGLE / GUI_SCREEN_WIDTH;
                    } else {
                        da = dx * SWITCHING3D_LR_PLATE_ANGLE / GUI_SCREEN_WIDTH;
                    }
                    if (da >= SWITCHING3D_LR_PLATE_DRAG_THRESHOLD) {
                        da_auto = SWITCHING3D_LR_PLATE_STEP;
                    } else {
                        da_auto = -SWITCHING3D_LR_PLATE_STEP;
                    }
                }
            }
            flag_change = true;
        } else if (tick_check_expire(tick, SWITCHING_TICK_EXPIRE)) {
            tick = tick_get();
            da += da_auto;
            flag_change = true;
        }

        if (flag_change) {
            if (da <= 0) {
                da = 0;
                flag_pos = FLAG_POS_START;
            } else if (da >= SWITCHING3D_LR_PLATE_ANGLE) {
                da = SWITCHING3D_LR_PLATE_ANGLE;
                flag_pos = FLAG_POS_END;
            }
            if (da > 900) {
                widget_set_visible(cur_page, false);
                widget_set_visible(new_page, true);
            } else {
                widget_set_visible(cur_page, true);
                widget_set_visible(new_page, false);
            }
            if (FUNC_SWITCH3D_LR_PLATE_FLIP_LEFT == switch_mode) {
                compo_form3d_set_axis_rotation(frm_cur, 900 + da);
            } else {
                compo_form3d_set_axis_rotation(frm_cur, -( 900 + da));
            }
            if (flag_press) {
                if (da >= 200) {
                    widget_set_visible(gap_page, true);
                } else {
                    widget_set_visible(gap_page, false);
                }
                compo_form3d_set_axis_distance(frm_cur, SWITCHING3D_LR_DISTANCE);
                compo_form3d_set_axis_pos(frm_cur, 0, 0, muls_shift16(SWITCHING3D_LR_FLIP_Z, SIN(da)));
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

//3D左右漂移翻页
static bool func_switching3d_lr_drift(u16 switch_mode, bool flag_auto)
{
    compo_form_t *frm_cur = compo_pool_get_top();

    bool flag_press = !flag_auto;
    u8 flag_pos;                            //当前坐标状态
    bool flag_change;
    s32 da = 0;
    int da_auto = SWITCHING3D_LR_DRIFT_STEP;
    u32 tick = tick_get();
    if (frm_cur == NULL) {
        halt(HALT_FUNC_SWITCH_LR_PTR);
    }
    bool lr_dir = FUNC_SWITCH3D_LR_DRIFT_RIGHT == switch_mode ? true : false;
    widget_page_t *cur_page = NULL, *new_page = NULL;
    widget_t *widget = widget_get_next(func_cb.frm_main->page_body);
    while (widget != NULL)
    {
        if (widget_is_page(widget)) {
            if (NULL == new_page) {
                new_page = widget;
            } else if (NULL == cur_page) {
                cur_page = widget;
            } else {
                break;
            }
        }
        widget = widget_get_next(widget);
    }

    for (;;) {
        flag_pos = FLAG_POS_NORM;
        flag_change = false;
        if (flag_press) {
            s32 dx, dy;
            flag_press = ctp_get_dxy(&dx, &dy);
            if (lr_dir) {
                da = dx * SWITCHING3D_LR_DRIFT_ANGLE / GUI_SCREEN_WIDTH;
            } else {
                da = -dx * SWITCHING3D_LR_DRIFT_ANGLE / GUI_SCREEN_WIDTH;
            }

            if (flag_press) {
                if (tick_check_expire(tick, SWITCHING_TICK_AUTO)) {
                    if (lr_dir) {
                        da = dx * SWITCHING3D_LR_DRIFT_ANGLE / GUI_SCREEN_WIDTH;
                    } else {
                        da = -dx * SWITCHING3D_LR_DRIFT_ANGLE / GUI_SCREEN_WIDTH;
                    }
                    if (da >= SWITCHING3D_LR_DRIFT_DRAG_THRESHOLD) {
                        da_auto = SWITCHING3D_LR_DRIFT_STEP;
                    } else {
                        da_auto = -SWITCHING3D_LR_DRIFT_STEP;
                    }
                }
            }
            flag_change = true;
        } else if (tick_check_expire(tick, SWITCHING_TICK_EXPIRE)) {
            tick = tick_get();
            da += da_auto;
            flag_change = true;
        }

        if (flag_change) {
            if (da <= 0) {
                da = 0;
                flag_pos = FLAG_POS_START;
            } else if (da >= SWITCHING3D_LR_DRIFT_ANGLE) {
                da = SWITCHING3D_LR_DRIFT_ANGLE;
                flag_pos = FLAG_POS_END;
            }
            if (da < SWITCHING3D_LR_DRIFT_ANGLE) {
                widget_set_visible(cur_page, true);
            } else {
                widget_set_visible(cur_page, false);
            }
            widget_axis3d_t *axis3d = (widget_axis3d_t *)widget_get_next(new_page);
            if (!lr_dir) {
                widget_axis3d_set_rotation(axis3d, 1800 - da);
            } else {
                widget_axis3d_set_rotation(axis3d, 1800 + da);
            }
            void *img = widget_get_next(axis3d);
            rect_t img_location;
            img_location.wid = GUI_SCREEN_WIDTH / 2 + GUI_SCREEN_WIDTH / 2 * da / 900;
            img_location.hei = GUI_SCREEN_HEIGHT / 2 + GUI_SCREEN_HEIGHT / 2 * da / 900;
            if (!lr_dir) {
                img_location.x = GUI_SCREEN_WIDTH - img_location.wid / 2;
            } else {
                img_location.x = img_location.wid / 2;
            }
            img_location.y = GUI_SCREEN_HEIGHT / 2;
            widget_set_location(img, img_location.x, img_location.y, img_location.wid, img_location.hei);
            img = widget_get_next(img);
            img_location.y = GUI_SCREEN_HEIGHT / 2 + img_location.hei;
            widget_set_location(img, img_location.x, img_location.y, img_location.wid, img_location.hei);


            axis3d = (widget_axis3d_t *)widget_get_next(cur_page);
            if (!lr_dir) {
                widget_axis3d_set_rotation(axis3d, 2700 - da);
            } else {
                widget_axis3d_set_rotation(axis3d, 900 + da);
            }
            img = widget_get_next(axis3d);
            img_location.wid = GUI_SCREEN_WIDTH - GUI_SCREEN_WIDTH * da / 900 / 2;
            img_location.hei = GUI_SCREEN_HEIGHT - GUI_SCREEN_HEIGHT * da / 900 / 2;
            if (!lr_dir) {
                img_location.x = img_location.wid / 2;
            } else {
                img_location.x = GUI_SCREEN_WIDTH - img_location.wid / 2;
            }
            img_location.y = GUI_SCREEN_HEIGHT / 2;
            widget_set_location(img, img_location.x, img_location.y, img_location.wid, img_location.hei);

            img = widget_get_next(img);
            img_location.y = GUI_SCREEN_HEIGHT / 2 + img_location.hei;
            widget_set_location(img, img_location.x, img_location.y, img_location.wid, img_location.hei);
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


//切换场景
bool func_switching3d(u16 switch_mode, bool flag_auto, void *param)
{
    bool res = false;
    switch_mode = switch_mode & 0x7FFF;
    if (switch_mode <= FUNC_SWITCH3D_LR_ROTA_RIGHT) {
        res = func_switching3d_lr(switch_mode, flag_auto);                      //3D左右切换 内切换 翻转
    } else if (switch_mode <= FUNC_SWITCH3D_LR_FOLD_RIGHT) {
        res = func_switching3d_lr_fold(switch_mode, flag_auto);                 //3D左右切换 折叠 翻转
    } else if (switch_mode <= FUNC_SWITCH3D_LR_PLATE_FLIP_RIGHT) {
        res = func_switching3d_lr_plate_flip(switch_mode, flag_auto);
    } else if (switch_mode <= FUNC_SWITCH3D_LR_DRIFT_RIGHT) {
        res = func_switching3d_lr_drift(switch_mode, flag_auto);
    }
    else {
        halt(HALT_FUNC_SWITCH_MENU_MODE);                                       //无效的模式
    }
    return res;
}
#endif
