#include "include.h"


/**
 * @brief 创建标签文本框
 * @param[in] frm : 窗体句柄
 * @param[in] max_word_cnt : 最大字数
 * @return 返回标签句柄
 **/
compo_label_t* compo_label_create(compo_form_t *frm, u16 max_word_cnt)
{
    compo_label_t *label = compo_create(frm, COMPO_TYPE_LABEL);

    //前景文本
    void *txt = widget_text_create(frm->page_body, max_word_cnt);
    widget_text_set_autosize(txt, true);  //默认为自适应大小
    label->txt = txt;

    return label;
}

/**
 * @brief 设置标签的文本框内容
 * @param[in] label : 标签句柄
 * @param[in] text : 文本内容 例："遥遥领先"
 **/
void compo_label_set(compo_label_t *label, const char *text)
{
    widget_text_set(label->txt, text);
}

/**
 * @brief 设置标签字库资源地址
 * @param[in] label : 标签句柄
 * @param[in] font_res_addr : 设置字库资源地址, 0则表示用回系统字体
 **/
void compo_label_set_font(compo_label_t *label, u32 font_res_addr)
{
    widget_text_set_font(label->txt, font_res_addr);
}

/**
 * @brief 获取标签文本框内容行数
 * @param[in] label : 标签句柄
 **/
u8 compo_label_get_line_cnt(compo_label_t *label)
{
    return widget_text_get_line_cnt(label->txt);
}

/**
 * @brief 设置标签文本框的坐标及大小
          注意：文本框设置的坐标是以默认的中心点作为参考
 * @param[in] label : 文本指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 文本框宽度
 * @param[in] height : 文本框高度
 **/
void compo_label_set_location(compo_label_t *label, s16 x, s16 y, s16 width, s16 height)
{
    widget_set_location(label->txt, x, y, width, height);
    if (0 != width && 0 != height) {
         widget_text_set_autosize(label->txt, false);
    }
}

/**
 * @brief 设置标签文本框的坐标
          注意：文本框设置的坐标是以默认的中心点作为参考
 * @param[in] label : 文本指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_label_set_pos(compo_label_t *label, s16 x, s16 y)
{
    widget_set_pos(label->txt, x, y);
}

/**
 * @brief 设置标签文本框是否自适应尺寸
 * @param[in] label : 文本指针
 * @param[in] autosize : 是否自适应尺寸
 **/
void compo_label_set_autosize(compo_label_t *label, bool autosize)
{
    widget_text_set_autosize(label->txt, autosize);
}

/**
 * @brief 设置标签文本框是否居中
 * @param[in] label : 文本指针
 * @param[in] align_center : 是否居中
 **/
void compo_label_set_align_center(compo_label_t *label, bool align_center)
{
    widget_set_align_center(label->txt, align_center);
}

/**
 * @brief 设置标签文本框文字的颜色
 * @param[in] label : 文本指针
 * @param[in] color : 颜色
 **/
void compo_label_set_forecolor(compo_label_t *label, u16 color)
{
    widget_text_set_color(label->txt, color);
}

/**
 * @brief 设置标签文本框的透明度
 * @param[in] label : 文本指针
 * @param[in] alpha : 透明度
 **/
void compo_label_set_alpha(compo_label_t *label, u8 alpha)
{
    widget_set_alpha(label->txt, alpha);
}

/**
 * @brief 设置标签文本框是否可见
 * @param[in] label : 文本指针
 * @param[in] visible : 是否可见
 **/
void compo_label_set_visible(compo_label_t *label, bool visible)
{
    widget_set_visible(label->txt, visible);
}
