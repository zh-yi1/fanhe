#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/**
 * @brief 创建一个动画框组件
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 动画资源地址
 * @return 返回动画框指针
 **/
compo_animation_t *compo_animation_create(compo_form_t *frm, u32 res_addr)
{
    compo_animation_t *animation = compo_create(frm, COMPO_TYPE_ANIMATION);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_size(page, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    widget_image_t *img = widget_image_create(page, res_addr);
    widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    animation->img = img;
    animation->page = page;
    animation->res_addr_set = NULL;
    return animation;
}

/**
 * @brief 创建一个动画框组件
 * @param[in] frm : 窗体指针
 * @param[in] res_addr_set : 动画资源地址集合
 * @return 返回动画框指针
 **/
compo_animation_t *compo_animation_create_by_addr_set(compo_form_t *frm, const u32 *res_addr_set)
{
    if (!res_addr_set) {
        return NULL;
    }
    compo_animation_t *animation = compo_create(frm, COMPO_TYPE_ANIMATION);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_size(page, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    widget_image_t *img = widget_image_create(page, res_addr_set[0]);
    widget_set_pos(img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    animation->img = img;
    animation->page = page;
    animation->res_addr_set = res_addr_set;
    return animation;
}

/**
 * @brief 设置动画
          用于改变图像组件中的图片
 * @param[in] animation : 动画指针
 * @param[in] res_addr : 动画资源地址
 **/
void compo_animation_set(compo_animation_t *animation, u32 res_addr)
{
    widget_image_set(animation->img, res_addr);
}

/**
 * @brief 设置下一个动画
          用于切换动画组件中的图片
 * @param[in] animation : 动画指针
 **/
void compo_animation_manual_next(compo_animation_t *animation)
{
    animation->index++;
    if (animation->index >= animation->radix) {
        animation->index = 0;
        compo_animation_click_set_notvis(animation);
    }

    if (animation->res_addr_set) {
        widget_image_set(animation->img, animation->res_addr_set[animation->index]);
    } else {
        widget_image_cut(animation->img, 0, animation->hei * animation->index, animation->wid, animation->hei);
    }
}

/**
 * @brief 动画进程
          用于切换动画
 * @param[in] animation : 动画指针
 **/
void compo_animation_process(compo_animation_t *animation)
{
    if (animation->interval == 0) {
        return;
    }

    if (tick_check_expire(animation->ticks, animation->interval)) {
        animation->ticks = tick_get();
        compo_animation_manual_next(animation);
    }
}

/**
 * @brief 设置动画的编号
 * @param[in] animation : 动画指针
 **/
void compo_animation_set_index(compo_animation_t *animation, u8 index)
{
    animation->index = index;
}

/**
 * @brief 设置动画的拼图总数
 * @param[in] animation : 动画指针
 * @param[in] radix : 拼图总数
 **/
void compo_animation_set_radix(compo_animation_t *animation, u8 radix)
{
    area_t area = widget_image_get_size(animation->img);
    animation->radix = radix;
    animation->wid = area.wid;
    if (animation->res_addr_set) {
        animation->hei = area.hei;
        widget_image_set(animation->img, animation->res_addr_set[animation->index]);
    } else {
        animation->hei = area.hei / radix;
        widget_image_cut(animation->img, 0, animation->hei * animation->index, animation->wid, animation->hei);
    }
}

/**
 * @brief 设置动画的播放间隔
 * @param[in] animation : 动画指针
 * @param[in] interval : 动画播放间隔
 **/
void compo_animation_set_interval(compo_animation_t *animation, u32 interval)
{
    animation->interval = interval * 10;
}

/**
 * @brief 获取animation ID
 **/
int compo_get_animation_id(void)
{
    component_t *compo = (component_t *)compo_pool_get_top();
    while (compo != NULL) {
        if (compo->type == COMPO_TYPE_ANIMATION) {
            compo_animation_t *animation = (compo_animation_t *)compo;
            if (compo_animation_get_click(animation)) {
                if (animation->bond_data == COMPO_BOND_IMAGE_CLICK) {
                    return animation->id;
                } else if (animation->bond_data == COMPO_BOND_ANIMATION_AREA_CLICK) {
                    if (!widget_get_visble(animation->img)) {
                        return animation->id;
                    }
                }
            }
        }
        compo = compo_get_next(compo);          //遍历组件
    }
    return ID_NULL;
}

/**
 * @brief 设置动画组件的坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] animation : 动画指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_animation_set_pos(compo_animation_t *animation, s16 x, s16 y)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_pos(animation->page, x, y);
#else
    widget_set_pos(animation->page, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 设置动画组件的大小
 * @param[in] animation : 动画指针
 * @param[in] width : 动画宽度
 * @param[in] height : 动画高度
 **/
void compo_animation_set_size(compo_animation_t *animation, s16 width, s16 height)
{
#if BOX_GUI_ROTATE_DISP
    if (animation->wid && animation->hei) {
        s16 wid = COMPO_GET_WID(width, height);
        s16 hei = COMPO_GET_HEI(width, height);
        widget_page_scale_to(animation->page, wid * GUI_SCREEN_WIDTH / animation->wid, hei * GUI_SCREEN_HEIGHT / animation->hei);
    }
#else
    widget_set_size(animation->img, width, height);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 设置动画的旋转角度
          图片旋转时，需要经过转换工具勾上 旋转压缩 的选项
 * @param[in] animation : 动画指针
 * @param[in] angle : 旋转的角度
 **/
void compo_animation_set_rotation(compo_animation_t *animation, s16 angle)
{
    widget_image_set_rotation(animation->img, angle);
}

/**
 * @brief 设置动画的旋转中心点
 * @param[in] animation : 动画指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_animation_set_rotation_center(compo_animation_t *animation, s16 x, s16 y)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_image_set_rotation_center(animation->img, x, y);
#else
    widget_image_set_rotation_center(animation->img, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 设置动画是否可见
 * @param[in] animation : 动画指针
 * @param[in] visible : true  可见
                        false 不可见
 **/
void compo_animation_set_visible(compo_animation_t *animation, bool visible)
{
    widget_set_visible(animation->img, visible);
}

/**
 * @brief 获取动画的坐标和大小
 * @param[in] animation : 动画指针
 * @return 返回动画的坐标和大小
 **/

 //
#if BOX_GUI_ROTATE_DISP
rect_t compo_animation_get_size(compo_animation_t *animation)
{
    rect_t rect = rotate_widget_get_location(animation->img);
    return rect;
}
#endif // BOX_GUI_ROTATE_DISP

rect_t compo_animation_get_location(compo_animation_t *animation)
{
    rect_t rect = widget_get_location(animation->img);
    return rect;
}

/**
 * @brief 创建一个长图像框滚动组件
          用于首尾可衔接的图片，进行循环滚动
          切图时需要将长图按radix裁剪，然后上下排布
 * @param[in] animation : 动画指针
 * @param[in] res_addr : 动画资源
 **/
void compo_animation_set_roll(compo_animation_t *animation, u32 res_addr)
{
    widget_image_t *img[animation->radix+1];
    widget_set_size(animation->page, animation->wid, animation->hei);
    for (int i=0; i<=animation->radix; i++) {
        img[i] = widget_image_create(animation->page, res_addr);
        if (i != animation->radix) {
            widget_image_cut(img[i], 0, animation->hei * i, animation->wid, animation->hei);
        } else {
            widget_image_cut(img[i], 0, animation->hei * 0, animation->wid, animation->hei);    //首尾图片一致
        }
        widget_set_align_center(img[i], false);
        widget_set_pos(img[i], animation->wid * i, 0);      //图片裁剪排布 0-1-2...-0
    }

}

/**
 * @brief 设置动画的点击区域
 * @param[in] animation : 动画指针
 **/
void compo_animation_set_click_area(compo_animation_t *animation, s16 x, s16 y, s16 width, s16 height)
{
    animation->click_area.x   = x;
    animation->click_area.y   = y;
    animation->click_area.wid = width;
    animation->click_area.hei = height;
}

/**
 * @brief 获取动画是否在点击区域内
 * @param[in] animation : 动画指针
 * @return 返回是或否
 **/
bool compo_animation_get_click(compo_animation_t *animation)
{
    point_t pt = ctp_get_sxy();
    rect_t *rect = &animation->click_area;
    if (abs_s(pt.x - rect->x) * 2 <= rect->wid && abs_s(pt.y - rect->y) * 2 <= rect->hei) {
        return true;
    }
    return false;
}

/**
 * @brief 设置区域点击动画可视
 **/
void compo_animation_click_set_vis(compo_animation_t *animation)
{
    if (animation->bond_data == COMPO_BOND_ANIMATION_AREA_CLICK) {
        point_t pt = ctp_get_rxy(func_cb.frm_main->page_body);
        compo_animation_set_pos(animation, pt.x, pt.y);
        compo_animation_set_index(animation, 0);
        compo_animation_set_visible(animation, true);
        widget_image_cut(animation->img, 0, 0, animation->wid, animation->hei);
    }
}

/**
 * @brief 设置区域点击动画不可视
 * @param[in] animation : 动画指针
 **/
void compo_animation_click_set_notvis(compo_animation_t *animation)
{
    if (animation->bond_data == COMPO_BOND_ANIMATION_AREA_CLICK) {
        compo_animation_set_visible(animation, false);
    }
}

/**
 * @brief 设置动画播放次数
 * @param[in] animation : 动画指针
 * @param[in] times : 播放次数
 * @param[in] interval : 播放间隔
 * @param[in] is_change : true  播放完成后跳转主页面
 *                         false 播放完成后停在最后一帧
 * @param[in] sta : 切换的页面ID
 **/
void compo_animation_play_times(compo_animation_t *animation, u8 times, u16 interval, bool is_change, u8 sta)
{
    static u8 check_times;
    if (tick_check_expire(animation->ticks, interval)) {
        animation->ticks = tick_get();
        animation->index++;
        if (animation->index >= animation->radix) {
            check_times++;
            if (check_times >= times) {  // 完成n次循环
                if (is_change) {
                    func_cb.sta = sta;
                    check_times = 0;  // 重置计数器
                } else {
                    animation->index= animation->radix-1; // 设置停留在最后一帧
                }
            } else {
                animation->index = 0;  // 重置帧索引继续播放
            }
        } else {
            widget_image_cut(animation->img, 0, animation->hei * animation->index, animation->wid, animation->hei);
        }
    }
}

/**
 * @brief 判断当前动画是否播放完成
 * @param[in] animation : 动画指针
 * @return 返回是, 完成一次动画播放; 返回否, 当前播放未完成;
 **/
#if BOX_GUI_ROTATE_DISP
bool compo_animation_complete(compo_animation_t *animation)
{

    return (animation->index == animation->radix - 1 && animation->ticks);
}
#endif // BOX_GUI_ROTATE_DISP
