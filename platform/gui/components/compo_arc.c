#include "include.h"

#define ARC_ANGLE_MAX           3600
#if !BOX_GUI_ROTATE_DISP
#define ARC_ROTATION_DEFAULT    2700   //库默认的 0°位置在3点钟方向，组件默认的0°位置在12点钟方向
#else
#define ARC_ROTATION_DEFAULT    0
#endif


/**
 * @brief 创建一个圆弧组件(默认0°位置在12点钟方向)
 * @param[in] frm : 窗体指针
 * @return 返回圆弧指针
 **/
compo_arc_t *compo_arc_create(compo_form_t *frm)
{
#if GUI_USE_ARC
    compo_arc_t *compo_arc = compo_create(frm, COMPO_TYPE_ARC);
    if (NULL == compo_arc) {
        halt(HALT_GUI_COMPO_ARC_FAIL);
    }
    compo_arc->arc = widget_arc_create(frm->page_body);
    compo_arc->prec = ARC_ANGLE_PREC_DEFAULT; //默认精度
    compo_arc->rotation_offset = ARC_ROTATION_DEFAULT;
    widget_arc_set_angles(compo_arc->arc, compo_arc->rotation_offset, compo_arc->rotation_offset);

    return compo_arc;
#else
    return NULL;
#endif
}

/**
 * @brief 设置圆弧的坐标及大小(指的是圆弧组件自身的大小，而非圆弧宽度)
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] arc : 圆弧指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 圆弧组件宽度
 * @param[in] height : 圆弧组件高度
 * @return 无
 **/
void compo_arc_set_location(compo_arc_t *arc, s16 x, s16 y, s16 width, s16 height)
{
#if GUI_USE_ARC
    if (NULL == arc) {
        halt(HALT_GUI_COMPO_ARC_NULL);
    }
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_location(arc->arc, x, y, width, height);
#else
    widget_set_location(arc->arc, x, y, width, height);
#endif // BOX_GUI_ROTATE_DISP
#endif
}

/**
 * @brief 设置圆弧的大小（指的是圆弧组件自身的大小，而非圆弧宽度)
 * @param[in] arc : 圆弧指针
 * @param[in] width : 圆弧组件宽度
 * @param[in] height : 圆弧组件高度
 * @return 无
 **/
void compo_arc_set_size(compo_arc_t *arc, s16 width, s16 height)
{
#if GUI_USE_ARC
    if (NULL == arc) {
        halt(HALT_GUI_COMPO_ARC_NULL);
    }
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_size(arc->arc, width, height);
#else
    widget_set_size(arc->arc, width, height);
#endif // BOX_GUI_ROTATE_DISP
#endif
}

/**
 * @brief 设置圆弧的范围(start_angle和end_angle可任意角度)
 * @param[in] arc : 圆弧指针
 * @param[in] start_angle : 相对于0°顺时针方向角度 0 ~ 3600°
 * @param[in] end_angle : 相对于0°顺时针方向角度 0 ~ 3600°
 * @return 无
 **/
void compo_arc_set_angles(compo_arc_t *arc, u16 start_angle, u16 end_angle)
{
    arc->start_angle = start_angle;
    arc->end_angle = end_angle;

    u16 init_angle = arc->start_angle + arc->rotation_offset;
    widget_arc_set_angles(arc->arc, init_angle, init_angle);
}

/**
 * @brief 修改圆弧的原点0°位置
 * @param[in] arc : 圆弧指针
 * @param[in] angle : 相对于12点钟顺时针方向的角度
 * @return 无
 **/
void compo_arc_set_rotation(compo_arc_t *arc, u16 angle)
{
#if GUI_USE_ARC
    if (NULL == arc) {
        halt(HALT_GUI_COMPO_ARC_NULL);
    }

    arc->rotation_offset = ARC_ROTATION_DEFAULT + angle;
    arc->rotation_offset %= ARC_ANGLE_MAX;
#endif
}

/**
 * @brief 通过设置圆弧的值
 * @param[in] arc : 圆弧指针
 * @param[in] value : 0 ~ ARC_VALUE_MAX
 * @return 无
 **/
void compo_arc_set_value(compo_arc_t *arc, u16 value)
{
#if GUI_USE_ARC
    u32 s_angle, e_angle;

    if (value > ARC_VALUE_MAX) {
        value = ARC_VALUE_MAX;
    }
    arc->value = value;

    if (arc->start_angle < arc->end_angle) {
        s_angle = arc->start_angle;
        e_angle = arc->end_angle;
        e_angle = s_angle + (e_angle - s_angle) * value / ARC_VALUE_MAX;
    } else {
        s_angle = arc->end_angle;
        e_angle = arc->start_angle;
        s_angle = e_angle - (e_angle - s_angle) * value / ARC_VALUE_MAX;
    }
    s_angle = s_angle + arc->rotation_offset;
    e_angle = e_angle + arc->rotation_offset;

    if (ANGLE_PREC_1_1 == arc->prec) {
        s_angle = (s_angle / 10) * 10;
        e_angle = (e_angle / 10) * 10;
    }

    widget_arc_set_angles(arc->arc, s_angle, e_angle);
#endif
}

/**
 * @brief 设置圆弧的宽度（指的圆弧宽度，而不是组件宽度)
 * @param[in] arc : 圆弧指针
 * @param[in] arc_width : 圆弧宽度
 * @return 无
 **/
void compo_arc_set_width(compo_arc_t *arc, u16 arc_width)
{
#if GUI_USE_ARC
    if (NULL == arc) {
        halt(HALT_GUI_COMPO_ARC_NULL);
    }
    widget_arc_set_width(arc->arc, arc_width);
#endif
}

/**
 * @brief 设置圆弧的颜色
 * @param[in] arc : 圆弧指针
 * @param[in] content_color : 设置的角度范围内的颜色
 * @param[in] bg_color : 设置的角度范围外的颜色
 * @return 无
 **/
void compo_arc_set_color(compo_arc_t *arc, u16 content_color, u16 bg_color)
{
#if GUI_USE_ARC
    if (NULL == arc) {
        halt(HALT_GUI_COMPO_ARC_NULL);
    }

    widget_arc_set_color(arc->arc, content_color, bg_color);
#endif
}

/**
 * @brief 设置圆弧的透明度
 * @param[in] arc : 圆弧指针
 * @param[in] content_alpha : 设置的角度范围内的透明度
 * @param[in] bg_alpha : 设置的角度范围外的透明度
 * @return 无
 **/
void compo_arc_set_alpha(compo_arc_t *arc, u16 content_alpha, u16 bg_alpha)
{
#if GUI_USE_ARC
    if (NULL == arc) {
        halt(HALT_GUI_COMPO_ARC_NULL);
    }

    widget_arc_set_alpha(arc->arc, content_alpha, bg_alpha);
#endif
}

/**
 * @brief 设置圆弧截面是否圆弧化
 * @param[in] arc : 圆弧指针
 * @param[in] start_onoff: 开始角度位置 1:圆弧化 0:直线化
 * @param[in] end_onoff  : 结束角度位置 1:圆弧化 0:直线化
 * @return 无
 * 圆弧化后目前缩放对齐有点偏差
 **/
void compo_arc_set_edge_circle(compo_arc_t *arc, bool start_onoff, bool end_onoff)
{
    #if GUI_USE_ARC
    if (NULL == arc) {
        halt(HALT_GUI_COMPO_ARC_NULL);
    }
    widget_arc_set_edge_circle(arc->arc, start_onoff, end_onoff);
    #endif
}




/**
 * @brief 设置圆弧的精度
 * @param[in] arc : 圆弧指针
 * @param[in] prec : 圆弧精度 e_ANGLE_PREC
 * @return 无
 **/
void compo_arc_set_prec(compo_arc_t *arc, u8 prec)
{
#if GUI_USE_ARC
    if (NULL == arc) {
        halt(HALT_GUI_COMPO_ARC_NULL);
    }

    arc->prec = prec;
#endif
}


/**
 * @brief 设置图像框是否可见
 * @param[in] picbox : 图像指针
 * @param[in] visible : true  可见
                        false 不可见
 **/
void compo_arc_set_visible(compo_arc_t *arc, bool visible)
{
    widget_set_visible(arc->arc, visible);
}

