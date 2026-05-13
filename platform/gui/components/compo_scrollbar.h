#ifndef __COMPO_SCROLLBAR_H__
#define __COMPO_SCROLLBAR_H__

/**********************************************************
 * 滚动条组件
 * 1. 圆弧滚动条基于圆弧组件实现，修改0°位置时是相对于12点钟方向顺时针方向
 * 2. 圆弧滚动条接口是基于0°位置顺时针方向的角度
 * 3. 角度范围：0° ~ 3600°
**********************************************************/

///滚动条的类型定义
enum {
    SCROLL_TYPE_VERTICAL,       //竖向滚动条
    SCROLL_TYPE_HORIZONTAL,     //横向滚动条
    SCROLL_TYPE_ARC,            //圆弧型滚动条
};

///圆弧滚动条参数结构体
typedef struct scroll_arc_t_ {
    u32 rotation_offset;        //角度偏移
    u32 bg_start_angle;         //圆弧背景滚动条可以显示开始角度
    u32 bg_end_angle;           //圆弧背景滚动条可以显示结束角度

    u32 fg_start_angle;
    u32 fg_end_angle;
} scroll_arc_t;

///矩形滚动条参数结构体
typedef struct scroll_rect_t_ {
    point_t bg_xy;
    point_t fg_xy;

    u32 scope;                      //矩形滚动条显示总体高/宽, 横向则是宽度
    u32 width;
} scroll_rect_t;

///滚动条组件结构体
typedef struct compo_scroll_t_ {
    COMPO_STRUCT_COMMON;
    void* scroll_bg1; // 上边或者左边
    void* scroll_bg2;// 下边或者右边
    void* scroll_fg;

    u32 fg_range;
    u8 stype;
    u8 content_ratio;           //滚动条内按钮占滚动条比例 1 / content_ratio
    u16 value;                  //滚动条滚动的进度值
    union {
        scroll_arc_t    scroll_arc;     //圆弧滚动条参数
        scroll_rect_t   scroll_rect;    //矩形滚动条参数
    };
} compo_scroll_t;


/**
 * @brief 创建一个滚动条
 * @param[in] frm : 窗体指针
 * @param[in] type : 滚动条类型
 * @return  返回滚动条指针
 */
compo_scroll_t* compo_scroll_create(compo_form_t* frm, u8 type);

/**
 * @brief 设置滚动条的宽度, 当滚动条为圆弧滚动条时，后面的可变参数则为设置圆弧半径
 * @param[in] scroll : 滚动条指针
 * @param[in] width : 滚动条的宽度
 * @param[in] ... : 可选参数 -> arg1 : 当滚动条为圆弧滚动条时，可变参数则为设置圆弧半径
 */
void compo_scroll_set_w_r(compo_scroll_t *scroll, u16 width, ...);

/**
 * @brief 设置滚动条的内部滚动按钮占滚动条整体的比例大小
 * @param[in] scroll : 滚动条指针
 * @param[in] ratio : 滚动条内按钮占滚动条比例 1 / ratio
 */
void compo_scroll_set_content_ratio(compo_scroll_t* scroll, u8 ratio);

/**
 * @brief 设置滚动条的可滚动范围
 * @param[in] scroll : 滚动条指针
 * @param[in] ... : 可选参数
 *                  当滚动条类型为 矩形横向/竖向滚动条时 arg1 -> 设置矩形滚动条的范围
 *                  当滚动条类型为 圆弧滚动条时  arg1 -> 设置圆弧的显示开始角度
                                                   arg2 -> 设置圆弧的显示结束角度
 */
void compo_scroll_set_range(compo_scroll_t* scroll, ...);

/**
 * @brief 设置滚动条的颜色
 * @param[in] scroll : 滚动条指针
 * @param[in] content_color : 滚动条内滚动按钮颜色
 * @param[in] bg1_color : 滚动条上面背景颜色
 * @param[in] bg2_color : 滚动条下面背景颜色
 */
void compo_scroll_set_color(compo_scroll_t* scroll, u16 content_color, u16 bg1_color, u16 bg2_color);

/**
 * @brief 设置滚动条位置
 * @param[in] scroll : 滚动条指针
 * @param[in] x : 当滚动条为矩形滚动条时，设置滚动条 x 坐标（以滚动条的左上角为原点坐标）
 *                当滚动条为圆弧滚动条时，设置滚动条 x 坐标（以圆弧的中心为原点坐标）
 * @param[in] y : 当滚动条为矩形滚动条时，设置滚动条 y 坐标（以滚动条的左上角为原点坐标）
 *                当滚动条为圆弧滚动条时，设置滚动条 y 坐标（以圆弧的中心为原点坐标）
 */
void compo_scroll_set_pos(compo_scroll_t* scroll, s16 x, s16 y);

/**
 * @brief 设置滚动条滚动按钮滚动的位置
 * @param[in] scroll : 滚动条指针
 * @param[in] value : 滚动值，最大为 SCROLLBAR_VALUE_MAX + 回弹值（回弹值通过设置回弹比例自动计算）
 *                            最小为 0 - 回弹值（回弹值通过设置回弹比例自动计算）
 */
void compo_scroll_set_value(compo_scroll_t* scroll, int value);

/**
 * @brief 设置滚动条的边角是否圆弧化
 * @param[in] scroll : 滚动条指针
 * @param[in] circle : 是否圆弧化
 */
void compo_scroll_set_edge_circle(compo_scroll_t* scroll, bool circle);


#endif // __COMPO_SCROLLBAR_H__
