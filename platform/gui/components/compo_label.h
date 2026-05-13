#ifndef __COMPO_LABEL_H
#define __COMPO_LABEL_H

/****************************************************************************
 *   标签组件：简单的文本框，不支持滚动和换行
****************************************************************************/

typedef struct tagcompo_label_t
{
    COMPO_STRUCT_COMMON
    widget_text_t *txt;
}compo_label_t;


/**
 * @brief 创建标签文本框
 * @param[in] frm : 窗体句柄
 * @param[in] max_word_cnt : 最大字数
 * @return 返回标签句柄
 **/
compo_label_t* compo_label_create(compo_form_t *frm, u16 max_word_cnt);

/**
 * @brief 设置标签的文本框内容
 * @param[in] label : 标签句柄
 * @param[in] text : 文本内容 例："遥遥领先"
 **/
void compo_label_set(compo_label_t *label, const char *text);

/**
 * @brief 设置标签字库资源地址
 * @param[in] label : 标签句柄
 * @param[in] font_res_addr : 设置字库资源地址, 0则表示用回系统字体
 **/
void compo_label_set_font(compo_label_t *label, u32 font_res_addr);

/**
 * @brief 获取标签文本框内容行数
 * @param[in] label : 标签句柄
 **/
u8 compo_label_get_line_cnt(compo_label_t *label);

/**
 * @brief 设置标签文本框的坐标及大小
          注意：标签文本框设置的坐标是以默认的中心点作为参考
 * @param[in] label : 标签句柄
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 文本框宽度
 * @param[in] height : 文本框高度
 **/
void compo_label_set_location(compo_label_t *label, s16 x, s16 y, s16 width, s16 height);

/**
 * @brief 设置标签文本框的坐标
          注意：标签文本框设置的坐标是以默认的中心点作为参考
 * @param[in] label : 标签句柄
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_label_set_pos(compo_label_t *label, s16 x, s16 y);

/**
 * @brief 设置标签文本框是否自适应尺寸
 * @param[in] label : 标签句柄
 * @param[in] autosize : 是否自适应尺寸
 **/
void compo_label_set_autosize(compo_label_t *label, bool autosize);

/**
 * @brief 设置标签文本框是否居中
 * @param[in] label : 标签句柄
 * @param[in] align_center : 是否居中
 **/
void compo_label_set_align_center(compo_label_t *label, bool align_center);

/**
 * @brief 设置标签文本框文字的颜色
 * @param[in] label : 标签句柄
 * @param[in] color : 颜色
 **/
void compo_label_set_forecolor(compo_label_t *label, u16 color);

/**
 * @brief 设置标签文本框的透明度
 * @param[in] label : 标签句柄
 * @param[in] alpha : 透明度
 **/
void compo_label_set_alpha(compo_label_t *label, u8 alpha);

/**
 * @brief 设置标签文本框是否可见
 * @param[in] label : 标签句柄
 * @param[in] visible : 是否可见
 **/
void compo_label_set_visible(compo_label_t *label, bool visible);


#endif

