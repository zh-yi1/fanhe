#ifndef _COMPO_ANIMATION_H
#define _COMPO_ANIMATION_H

#define COMPO_ANIMATION_CLICK_NUM            6 //区域点击动画最大个数

typedef struct compo_animation_t_ {
    COMPO_STRUCT_COMMON;
    widget_image_t *img;
    widget_page_t *page;
    rect_t click_area;      //点击区域
    s16 wid;                //单图宽
    s16 hei;                //单图高
    u32 interval;           //间隔
    const u32 *res_addr_set;      //资源地址集合
    u32 ticks;              //计时
    u8 index;               //当前图片编号
    u8 radix;               //拼图个数
} compo_animation_t;

/**
 * @brief 创建一个动画框组件
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 动画资源地址
 * @return 返回动画框指针
 **/
compo_animation_t *compo_animation_create(compo_form_t *frm, u32 res_addr);

/**
 * @brief 创建一个动画框组件
 * @param[in] frm : 窗体指针
 * @param[in] res_addr_set : 动画资源地址集合
 * @return 返回动画框指针
 **/
compo_animation_t *compo_animation_create_by_addr_set(compo_form_t *frm, const u32 *res_addr_set);

/**
 * @brief 设置动画
          用于改变图像组件中的图片
 * @param[in] animation : 动画指针
 * @param[in] res_addr : 动画资源地址
 **/
void compo_animation_set(compo_animation_t *animation, u32 res_addr);

/**
 * @brief 设置下一个动画
          用于切换动画组件中的图片
 * @param[in] animation : 动画指针
 **/
void compo_animation_manual_next(compo_animation_t *animation);

/**
 * @brief 动画进程
          用于切换动画
 * @param[in] animation : 动画指针
 **/
void compo_animation_process(compo_animation_t *animation);

/**
 * @brief 设置动画的编号
 * @param[in] animation : 动画指针
 **/
void compo_animation_set_index(compo_animation_t *animation, u8 index);

/**
 * @brief 设置动画的拼图总数
 * @param[in] animation : 动画指针
 * @param[in] radix : 拼图总数
 **/
void compo_animation_set_radix(compo_animation_t *animation, u8 radix);

/**
 * @brief 设置动画的播放间隔
 * @param[in] animation : 动画指针
 * @param[in] interval : 动画播放间隔
 **/
void compo_animation_set_interval(compo_animation_t *animation, u32 interval);

/**
 * @brief 获取animation ID
 **/
int compo_get_animation_id(void);

/**
 * @brief 设置图像框组件的坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] animation : 图像指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_animation_set_pos(compo_animation_t *animation, s16 x, s16 y);

/**
 * @brief 设置图像框组件的大小
 * @param[in] animation : 图像指针
 * @param[in] width : 图像框宽度
 * @param[in] height : 图像框高度
 **/
void compo_animation_set_size(compo_animation_t *animation, s16 width, s16 height);

/**
 * @brief 设置图像框的旋转角度
          图片旋转时，需要经过转换工具勾上 旋转压缩 的选项
 * @param[in] animation : 图像指针
 * @param[in] angle : 旋转的角度
 **/
void compo_animation_set_rotation(compo_animation_t *animation, s16 angle);
/**
 * @brief 设置图像框的旋转中心点
 * @param[in] animation : 图像指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_animation_set_rotation_center(compo_animation_t *animation, s16 x, s16 y);

/**
 * @brief 设置图像框是否可见
 * @param[in] animation : 图像指针
 * @param[in] visible : true  可见
                        false 不可见
 **/
void compo_animation_set_visible(compo_animation_t *animation, bool visible);

/**
 * @brief 获取图像框的坐标和大小
 * @param[in] animation : 图像指针
 * @return 返回图像的坐标和大小
 **/
 #if BOX_GUI_ROTATE_DISP
rect_t compo_animation_get_size(compo_animation_t *animation);
#endif
rect_t compo_animation_get_location(compo_animation_t *animation);

/**
 * @brief 创建一个长图像框滚动组件
          用于首尾可衔接的图片，进行循环滚动
          切图时需要将长图按radix裁剪，然后上下排布
 * @param[in] animation : 动画指针
 * @param[in] res_addr : 动画资源
 **/
void compo_animation_set_roll(compo_animation_t *animation, u32 res_addr);

/**
 * @brief 设置动画的点击区域
 * @param[in] animation : 动画指针
 **/
void compo_animation_set_click_area(compo_animation_t *animation, s16 x, s16 y, s16 width, s16 height);

/**
 * @brief 获取动画是否在点击区域内
 * @param[in] animation : 动画指针
 * @return 返回是或否
 **/
bool compo_animation_get_click(compo_animation_t *animation);

/**
 * @brief 设置区域点击动画可视
 **/
void compo_animation_click_set_vis(compo_animation_t *animation);

/**
 * @brief 设置区域点击动画不可视
 * @param[in] animation : 动画指针
 **/
void compo_animation_click_set_notvis(compo_animation_t *animation);

/**
 * @brief 设置动画播放次数
 * @param[in] animation : 动画指针
 * @param[in] times : 播放次数
 * @param[in] interval : 播放间隔
 * @param[in] is_change : true  播放完成后跳转主页面
 *                         false 播放完成后停在最后一帧
 * @param[in] sta : 切换的页面ID
 **/
void compo_animation_play_times(compo_animation_t *animation, u8 times, u16 interval, bool is_change, u8 sta);

/**
 * @brief 判断当前动画是否播放完成
 * @param[in] animation : 动画指针
 * @return 返回是, 完成一次动画播放; 返回否, 当前播放未完成;
 **/
#if BOX_GUI_ROTATE_DISP
bool compo_animation_complete(compo_animation_t *animation);
#endif // BOX_GUI_ROTATE_DISP
#endif
