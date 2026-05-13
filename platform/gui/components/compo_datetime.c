#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/**
 * @brief 创建一个时钟组件
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图像资源地址
 * @return 返回时钟指针
 **/
compo_datetime_t *compo_datetime_create(compo_form_t *frm, u32 res_addr)
{
    compo_datetime_t *dtime = compo_create(frm, COMPO_TYPE_DATETIME);
    void *img = widget_image_create(frm->page_body, res_addr);
    dtime->img = img;
    compo_cb.rtc_update = true;                         //创建时钟组件，需要同步更新时间
    return dtime;
}

/**
 * @brief 设置时钟组件的坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] dtime : 时钟指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_datetime_set_pos(compo_datetime_t *dtime, s16 x, s16 y)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_pos(dtime->img, x, y);
#else
    widget_set_pos(dtime->img, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 设置时钟组件的旋转中心点
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] dtime : 时钟指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_datetime_set_center(compo_datetime_t *dtime, s16 x, s16 y)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_image_set_rotation_center(dtime->img, x, y);
#else
    widget_image_set_rotation_center(dtime->img, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 设置时钟组件的起始角度
 * @param[in] dtime : 时钟指针
 * @param[in] angle : 原图的起始角度
 **/
void compo_datetime_set_start_angle(compo_datetime_t *dtime, s16 angle)
{
    dtime->start_angle = angle;
}

/**
 * @brief 设置时钟组件的旋转方向
 * @param[in] dtime : 时钟指针
 * @param[in] anticlockwise : 0：顺时针转动, 1:逆时针转动
 **/
void compo_datetime_set_anticlockwise(compo_datetime_t *dtime, bool anticlockwise)
{
    dtime->anticlockwise = anticlockwise;
}

