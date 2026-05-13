#ifndef __COMPO_ARC_H
#define __COMPO_ARC_H

/*****************************************************************
 *  圆弧组件:
 *  1.修改0°位置时是相对于12点钟方向顺时针方向
 *  2.其余接口的角度均是基于0°位置顺时针方向的角度
 *  3.角度范围：0° ~ 3600°
******************************************************************/

enum e_ANGLE_PREC
{
    ANGLE_PREC_1_1 = 1,     // 1/1    精度1°
    ANGLE_PREC_1_10 = 10,   // 1/10   精度0.1°
};


#define ARC_VALUE_MAX               1000               //设置圆弧的最大值
#define ARC_ANGLE_PREC_DEFAULT      ANGLE_PREC_1_10    //默认精度0.1°


typedef struct compo_arc_t_ {
    COMPO_STRUCT_COMMON;
    widget_arc_t *arc;

    u16 rotation_offset;
    u16 start_angle;
    u16 end_angle;
    u16 value;
    u8  prec;
} compo_arc_t;


/**
 * @brief 创建一个圆弧组件(默认0°位置在12点钟方向)
 * @param[in] frm : 窗体指针
 * @return 返回圆弧指针
 **/
compo_arc_t *compo_arc_create(compo_form_t *frm);

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
void compo_arc_set_location(compo_arc_t *arc, s16 x, s16 y, s16 width, s16 height);

/**
 * @brief 设置圆弧的大小（指的是圆弧组件自身的大小，而非圆弧宽度)
 * @param[in] arc : 圆弧指针
 * @param[in] width : 圆弧组件宽度
 * @param[in] height : 圆弧组件高度
 * @return 无
 **/
void compo_arc_set_size(compo_arc_t *arc, s16 width, s16 height);

/**
 * @brief 设置圆弧的范围(start_angle和end_angle可任意角度)
 * @param[in] arc : 圆弧指针
 * @param[in] start_angle : 相对于0°顺时针方向角度 0 ~ 3600°
 * @param[in] end_angle : 相对于0°顺时针方向角度 0 ~ 3600°
 * @return 无
 **/
void compo_arc_set_angles(compo_arc_t *arc, u16 start_angle, u16 end_angle);

/**
 * @brief 修改圆弧的原点0°位置
 * @param[in] arc : 圆弧指针
 * @param[in] angle : 相对于12点钟顺时针方向的角度
 * @return 无
 **/
void compo_arc_set_rotation(compo_arc_t *arc, u16 angle);

/**
 * @brief 通过设置圆弧的值
 * @param[in] arc : 圆弧指针
 * @param[in] value : 0 ~ ARC_VALUE_MAX
 * @return 无
 **/
void compo_arc_set_value(compo_arc_t *arc, u16 value);

/**
 * @brief 设置圆弧的宽度
 * @param[in] arc : 圆弧指针
 * @param[in] arc_width : 圆弧宽度（指的圆弧宽度，而不是组件宽度)
 * @return 无
 **/
void compo_arc_set_width(compo_arc_t *arc, u16 arc_width);

/**
 * @brief 设置圆弧的颜色
 * @param[in] arc : 圆弧指针
 * @param[in] content_color : 设置的角度范围内的颜色
 * @param[in] bg_color : 设置的角度范围外的颜色
 * @return 无
 **/
void compo_arc_set_color(compo_arc_t *arc, u16 content_color, u16 bg_color);

/**
 * @brief 设置圆弧的透明度
 * @param[in] arc : 圆弧指针
 * @param[in] content_alpha : 设置的角度范围内的透明度
 * @param[in] bg_alpha : 设置的角度范围外的透明度
 * @return 无
 **/
void compo_arc_set_alpha(compo_arc_t *arc, u16 content_alpha, u16 bg_alpha);
/**
 * @brief 设置圆弧截面是否圆弧化
 * @param[in] arc : 圆弧指针
 * @param[in] start_onoff: 开始角度位置 1:圆弧化 0:直线化
 * @param[in] end_onoff  : 结束角度位置 1:圆弧化 0:直线化
 * @return 无
 * 圆弧化后目前缩放对齐有点偏差
 **/
void compo_arc_set_edge_circle(compo_arc_t *arc, bool start_onoff, bool end_onoff);
/**
 * @brief 设置圆弧的精度(此精度影响圆弧横截面的平滑感)
 * @param[in] arc : 圆弧指针
 * @param[in] prec : 圆弧精度 e_ANGLE_PREC
 * @return 无
 **/
void compo_arc_set_prec(compo_arc_t *arc, u8 prec);

/**
 * @brief 设置图像框是否可见
 * @param[in] picbox : 图像指针
 * @param[in] visible : true  可见
                        false 不可见
 **/
void compo_arc_set_visible(compo_arc_t *arc, bool visible);
#endif
