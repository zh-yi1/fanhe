#ifndef __COMPO_CHARTBOX_H
#define __COMPO_CHARTBOX_H

typedef struct chart_t_ {
    u16 x;         //柱子左下角在x轴的位置
    u16 y;         //柱子左下角在y轴的位置
    u16 width;     //柱子宽度
    u16 height;    //柱子高度
} chart_t;


typedef struct compo_chartbox_t_ {
    COMPO_STRUCT_COMMON;
    widget_chart_t *chart;
    u8 max_num;             //创建时的最大数量
    u8 real_num;            //用户设置的实际数量
    rect_t rect;            //柱子实际显示区域
    u8 pixel;
    u8 dir;
} compo_chartbox_t;

/**
 * @brief 创建一个图表框组件
 * @param[in] frm : 窗体指针
 * @param[in] type : 柱子类型（0：直角，1：圆角）
 * @param[in] max_num : 图表中需要最大的柱形数量
              max_num设置一个最大值，实际使用数量用后面widget_chart_set_real_num函数来设置
 * @return 返回图表框指针
 **/
compo_chartbox_t *compo_chartbox_create(compo_form_t *frm, WGT_CHART_TYPE type, u8 max_num);

/**
 * @brief 设置图表框位置和大小
 * @param[in] chartbox : 图表框指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 图表框组件宽度
 * @param[in] height : 图表框组件高度
 **/
void compo_chartbox_set_location(compo_chartbox_t *chartbox, s16 x, s16 y, s16 width, s16 height);

/**
 * @brief 设置图表框位置
 * @param[in] chartbox : 图表框指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_chartbox_set_pos(compo_chartbox_t *chartbox, s16 x, s16 y);

/**
 * @brief 设置图表框大小
 * @param[in] chartbox : 图表框指针
 * @param[in] width : 图表框组件宽度
 * @param[in] height : 图表框组件高度
 **/
void compo_chartbox_set_size(compo_chartbox_t *chartbox, s16 width, s16 height);

/**
 * @brief 设置图表框中柱子的宽、高一份占多少像素
          例:  pixel为1时，如果设置柱子宽/高为100时，那么屏幕显示的宽/高为100像素点
               pixel为2时，如果设置柱子宽/高为100时，那么屏幕显示的宽/高为200像素点
 * @param[in] chartbox : 图表框指针
 * @param[in] pixel : 一份多少像素
 **/
void compo_chartbox_set_pixel(compo_chartbox_t *chartbox, u8 pixel);

/**
 * @brief 设置图表框中柱子实际数量
 * @param[in] chartbox : 图表框指针
 * @param[in] real_num : 图表柱状个数
              real_num : 设置实际使用的图表柱状个数，出现这个函数的原因是方便客户设置需要显示的数量；
              需要小于create时候设置的max_num；
 **/
void compo_chartbox_set_real_num(compo_chartbox_t *chartbox, u8 real_num);

/**
 * @brief 获取图表框中柱子最大数量
 * @param[in] chartbox : 图表框指针
 * @return 返回图表框中柱子最大数量
 **/
u8 compo_chartbox_get_max_num(compo_chartbox_t *chartbox);

/**
 * @brief 获取图表框中柱子实际使用数量
 * @param[in] chartbox : 图表框指针
 * @return 返回图表框中柱子实际使用数量
 **/
u8 compo_chartbox_get_real_num(compo_chartbox_t *chartbox);

/**
 * @brief 设置图表框数据
 * @param[in] chartbox : 图表框指针
 * @param[in] id : 柱形图序列号，0开始，0 到 real_num-1(包含real_num-1)
 * @param[in] chart_info : 图表中柱形的信息
 * @param[in] color : 图表中柱形的颜色
 **/
void compo_chartbox_set_value(compo_chartbox_t *chartbox, u8 id, chart_t chart_info, u16 color);

/**
 * @brief 设置chart显示方向
 * @param[in] chartbox : 图表框指针
 * @param[in] id : 0垂直方向，1水平方向
 **/
void compo_chartbox_set_dir(compo_chartbox_t *chartbox, u8 dir);

#endif
