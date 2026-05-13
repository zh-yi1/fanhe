#include "include.h"

/**
 * @brief 创建一个图表框组件
 * @param[in] frm : 窗体指针
 * @param[in] type : 柱子类型（0：直角，1：圆角上下都有 2:上圆角下直角  3.下圆角上直角）
 * @param[in] max_num : 图表中需要最大的柱形数量
              max_num设置一个最大值，实际使用数量用后面widget_chart_set_real_num函数来设置
 * @return 返回图表框指针
 **/
compo_chartbox_t *compo_chartbox_create(compo_form_t *frm, WGT_CHART_TYPE type, u8 max_num)
{
    compo_chartbox_t *chartbox = compo_create(frm, COMPO_TYPE_CHARTBOX);

    widget_chart_t *chart = widget_chart_create(frm->page_body, type, max_num + 1);     //创建时需要比实际大
    widget_chart_set_real_num(chart, max_num);                                          //实际柱子数量

    chartbox->chart = chart;
    chartbox->max_num = max_num;
    chartbox->real_num = max_num;
    return chartbox;
}

/**
 * @brief 设置图表框位置和大小
 * @param[in] chartbox : 图表框指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 图表框组件宽度
 * @param[in] height : 图表框组件高度
 **/
void compo_chartbox_set_location(compo_chartbox_t *chartbox, s16 x, s16 y, s16 width, s16 height)
{
    compo_chartbox_set_pos(chartbox, x, y);
    compo_chartbox_set_size(chartbox, width, height);
}

/**
 * @brief 设置图表框位置
 * @param[in] chartbox : 图表框指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_chartbox_set_pos(compo_chartbox_t *chartbox, s16 x, s16 y)
{
    s16 width = chartbox->rect.wid;
    s16 height = chartbox->rect.hei;
    chartbox->rect.x = x;
    chartbox->rect.y = y;

    if (width >= height) {
        y -= (width - height) / 2;
    } else {
        x += (height - width) / 2;
    }
    widget_set_pos(chartbox->chart, x, y);
}

/**
 * @brief 设置图表框大小
 * @param[in] chartbox : 图表框指针
 * @param[in] width : 图表框组件宽度
 * @param[in] height : 图表框组件高度
 **/
void compo_chartbox_set_size(compo_chartbox_t *chartbox, s16 width, s16 height)
{
    s16 x = chartbox->rect.x;
    s16 y = chartbox->rect.y;
    chartbox->rect.wid = width;
    chartbox->rect.hei = height;

    if (width >= height) {
        y -= (width - height) / 2;
        height = width;
    } else {
        x += (height - width) / 2;
        width = height;
    }
    widget_set_location(chartbox->chart, x, y, width, height);  //宽高相等，否则旋转坐标轴会显示异常

    if (chartbox->pixel == 0) { //默认pixel为1
        chartbox->pixel = 1;
    } else {
        width /= chartbox->pixel;
    }
    widget_chart_set_range(chartbox->chart, width, width);
}

/**
 * @brief 设置图表框中柱子的宽、高一份占多少像素
          例:  pixel为1时，如果设置柱子宽/高为100时，那么屏幕显示的宽/高为100像素点
               pixel为2时，如果设置柱子宽/高为100时，那么屏幕显示的宽/高为200像素点
 * @param[in] chartbox : 图表框指针
 * @param[in] pixel : 一份多少像素
 **/
void compo_chartbox_set_pixel(compo_chartbox_t *chartbox, u8 pixel)
{
    if (pixel == 0) {
        halt(HALT_GUI_COMPO_CHARTBOX_PIXEL);
    }

    rect_t rect_chart = widget_get_location(chartbox->chart);
    if (rect_chart.wid == 0 || rect_chart.hei == 0) {
        halt(HALT_GUI_COMPO_CHARTBOX_NOSIZE);
    }

//   图表的高和宽是widget_set_size 或 widget_set_location设置；
//   range_x和range_y是设置多少等份；举例widget_set_size设置的宽度是200像素，range_x设置100，表示将200分成100等份；
//   这样做的原因是参考LVGL，方便客户代码通用性；
    chartbox->pixel = pixel;
    u16 range_x = rect_chart.wid / pixel;
    u16 range_y = rect_chart.hei / pixel;
    widget_chart_set_range(chartbox->chart, range_x, range_y);
}

/**
 * @brief 设置图表框中柱子实际数量
 * @param[in] chartbox : 图表框指针
 * @param[in] real_num : 图表柱状个数
              real_num : 设置实际使用的图表柱状个数，出现这个函数的原因是方便客户设置需要显示的数量；
              需要小于create时候设置的max_num；
 **/
void compo_chartbox_set_real_num(compo_chartbox_t *chartbox, u8 real_num)
{
    if (real_num > chartbox->max_num + 1) {
        halt(HALT_GUI_COMPO_CHARTBOX_REALNUM);
    }
    chartbox->real_num = real_num;
    widget_chart_set_real_num(chartbox->chart, real_num);
}

/**
 * @brief 获取图表框中柱子最大数量
 * @param[in] chartbox : 图表框指针
 * @return 返回图表框中柱子最大数量
 **/
u8 compo_chartbox_get_max_num(compo_chartbox_t *chartbox)
{
    return chartbox->max_num;
}

/**
 * @brief 获取图表框中柱子实际使用数量
 * @param[in] chartbox : 图表框指针
 * @return 返回图表框中柱子实际使用数量
 **/
u8 compo_chartbox_get_real_num(compo_chartbox_t *chartbox)
{
    return chartbox->real_num;
}

/**
 * @brief 设置图表框数据
 * @param[in] chartbox : 图表框指针
 * @param[in] id : 柱形图序列号，0开始，0 到 real_num-1(包含real_num-1)
 * @param[in] chart_info : 图表中柱形的信息
 * @param[in] color : 图表中柱形的颜色
 **/
void compo_chartbox_set_value(compo_chartbox_t *chartbox, u8 id, chart_t chart_info, u16 color)
{
    if (chartbox->pixel == 0) {
        halt(HALT_GUI_COMPO_CHARTBOX_PIXEL);
    }

    u16 x = chart_info.x;            //柱子左下角在x轴的位置
    u16 y = chart_info.y;            //柱子左下角在y轴的位置
    u16 width = chart_info.width;    //柱子宽度
    u16 height = chart_info.height;  //柱子高度
    s16 wid_limit = chartbox->dir ? chartbox->rect.hei : chartbox->rect.wid;
    s16 hei_limit = chartbox->dir ? chartbox->rect.wid : chartbox->rect.hei;

    if(((x + width) * chartbox->pixel > wid_limit) || (y * chartbox->pixel > hei_limit)){
        printf("chartbox area[%d] exceed! [%d %d] [%d %d]\n", id, (x + width) * chartbox->pixel, y * chartbox->pixel, wid_limit, hei_limit);
        x = y = width = height = 0;
    } else if((y + height) * chartbox->pixel > hei_limit){
        printf("chartbox y[%d] limit: [%d-->%d]\n", id, (y + height) * chartbox->pixel, hei_limit);
        height = hei_limit / chartbox->pixel - y;
    }

    //详细介绍在api_gui.h中
    widget_chart_set_value(chartbox->chart, id, x, x + width, y, y + height, color);
}

/**
 * @brief 设置chart显示方向
 * @param[in] chartbox : 图表框指针
 * @param[in] id : 0垂直方向，1水平方向
 **/
void compo_chartbox_set_dir(compo_chartbox_t *chartbox, u8 dir)
{
    //详细介绍在api_gui.h中
    widget_chart_set_dir(chartbox->chart, dir);
    chartbox->dir = dir;
}



