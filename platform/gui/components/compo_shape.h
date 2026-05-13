#ifndef _COMPO_SHAPE_H
#define _COMPO_SHAPE_H

enum COMPO_SHAPE_TYPE {
    COMPO_SHAPE_TYPE_RECTANGLE,
};

typedef struct compo_shape_t_ {
    COMPO_STRUCT_COMMON;
    u8 shape_type;
    widget_rect_t *rect;
} compo_shape_t;

/**
 * @brief 创建一个形状
 * @param[in] frm : 窗体指针
 * @param[in] shape_type : COMPO_SHAPE_TYPE_RECTANGLE 矩形
 * @return 返回矩形指针
 **/
compo_shape_t *compo_shape_create(compo_form_t *frm, u8 shape_type);

/**
 * @brief 创建一个形状
 * @param[in] frm : 窗体指针
 * @param[in] shape_type : COMPO_SHAPE_TYPE_RECTANGLE 矩形
 * @return 返回矩形指针
 **/
compo_shape_t *compo_shape_create_for_page(compo_form_t *frm, widget_page_t* page, u8 shape_type);

/**
 * @brief 设置形状的坐标及大小
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] shape : 矩形指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 矩形宽度
 * @param[in] height : 矩形高度
 **/
void compo_shape_set_location(compo_shape_t *shape, s16 x, s16 y, s16 width, s16 height);

/**
 * @brief 获取形状的坐标及大小
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] shape : 矩形指针
 * @return 位置和大小
 **/
rect_t compo_shape_get_location(compo_shape_t *shape);

/**
 * @brief 设置形状的大小
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] shape : 矩形指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 矩形宽度
 * @param[in] height : 矩形高度
 **/
void compo_shape_set_size(compo_shape_t *shape, s16 width, s16 height);

/**
 * @brief 设置形状的坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] shape : 矩形指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_shape_set_pos(compo_shape_t *shape, s16 x, s16 y);

/**
 * @brief 设置形状的颜色
 * @param[in] shape : 矩形指针
 * @param[in] color : 颜色
 **/
void compo_shape_set_color(compo_shape_t *shape, u16 color);

/**
 * @brief 设置形状的透明度
 * @param[in] shape : 矩形指针
 * @param[in] alpha : 透明度
 **/
void compo_shape_set_alpha(compo_shape_t *shape, u8 alpha);

/**
 * @brief 设置形状是否可见
 * @param[in] shape : 矩形指针
 * @param[in] visible : true  可见
                        false 不可见
 **/
void compo_shape_set_visible(compo_shape_t *shape, bool visible);

/**
 * @brief 获取矩形可见性
 * @param[in] shape : 矩形指针
 * @return 可见性
 **/
bool compo_shape_get_visible(compo_shape_t *shape);

/**
 * @brief 设置形状（矩形）圆角半径
 * @param[in] shape : 矩形指针
 * @param[in] r : 半径
 **/
void compo_shape_set_radius(compo_shape_t *shape, u16 r);

/**
 * @brief 设置形状是否居中对齐
 * @param[in] shape : 矩形指针
 * @param[in] en : 是否居中对齐
 **/
void compo_shape_set_align_center(compo_shape_t *shape, bool en);

#endif
