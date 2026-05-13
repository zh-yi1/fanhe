#ifndef _COMPO_DATETIME_H
#define _COMPO_DATETIME_H

typedef struct compo_datetime_t_ {
    COMPO_STRUCT_COMMON;
    widget_image_t *img;
    s16 start_angle;
    bool anticlockwise;
} compo_datetime_t;

/**
 * @brief 创建一个时钟组件
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图像资源地址
 * @return 返回时钟指针
 **/
compo_datetime_t *compo_datetime_create(compo_form_t *frm, u32 res_addr);

/**
 * @brief 设置时钟组件的坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] dtime : 时钟指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_datetime_set_pos(compo_datetime_t *dtime, s16 x, s16 y);

/**
 * @brief 设置时钟组件的旋转中心点
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] dtime : 时钟指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_datetime_set_center(compo_datetime_t *dtime, s16 x, s16 y);

/**
 * @brief 设置时钟组件的起始角度
 * @param[in] dtime : 时钟指针
 * @param[in] angle : 原图的起始角度
 **/
void compo_datetime_set_start_angle(compo_datetime_t *dtime, s16 angle);

/**
 * @brief 设置时钟组件的旋转方向
 * @param[in] dtime : 时钟指针
 * @param[in] anticlockwise : 0：顺时针转动, 1:逆时针转动
 **/
void compo_datetime_set_anticlockwise(compo_datetime_t *dtime, bool anticlockwise);
#endif
