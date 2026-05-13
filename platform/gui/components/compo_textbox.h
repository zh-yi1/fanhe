#ifndef _COMPO_TEXTBOX_H
#define _COMPO_TEXTBOX_H
//
enum {
    TEXT_AUTOROLL_MODE_NULL,
    TEXT_AUTOROLL_MODE_SROLL_CIRC,      //文字环形循环滚动
    TEXT_AUTOROLL_MODE_SROLL,           //文字前后往返滚动
};

/**
 * @brief 创建一个文本框
 * @param[in] frm : 窗体指针
 * @param[in] max_word_cnt : 最大字数
 * @return 返回文本指针
 **/
compo_textbox_t *compo_textbox_create(compo_form_t *frm, u16 max_word_cnt);

/**
 * @brief 创建一个文本框
 * @param[in] frm : 窗体指针
 * @param[in] parent : 父页面
 * @param[in] max_word_cnt : 最大字数
 * @return 返回文本指针
 **/
compo_textbox_t *compo_textbox_create_for_page(compo_form_t *frm, void* page, u16 max_word_cnt);

/**
 * @brief 设置字库资源地址
 * @param[in] textbox : 文本指针
 * @param[in] font_res_addr : 设置字库资源地址, 0则表示用回系统字体
 **/
void compo_textbox_set_font(compo_textbox_t *textbox, u32 font_res_addr);

/**
 * @brief 设置文本框内容
 * @param[in] textbox : 文本指针
 * @param[in] text : 文本内容 例："遥遥领先"
 **/
void compo_textbox_set(compo_textbox_t *textbox, const char *text);

/**
 * @brief 获取文本框内容行数
 * @param[in] textbox : 文本指针
 **/
u8 compo_textbox_get_line_cnt(compo_textbox_t *textbox);

/**
 * @brief 设置文本框的坐标及大小
          注意：文本框设置的坐标是以默认的中心点作为参考
          注意：当文本框为高度自适应(wid != 0,hei == 0)的情况，文本坐标相对左上角对齐
 * @param[in] textbox : 文本指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 文本框宽度
 * @param[in] height : 文本框高度
 **/
void compo_textbox_set_location(compo_textbox_t *textbox, s16 x, s16 y, s16 width, s16 height);

/**
 * @brief 设置文本框的坐标
          注意：文本框设置的坐标是以默认的中心点作为参考
 * @param[in] textbox : 文本指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_textbox_set_pos(compo_textbox_t *textbox, s16 x, s16 y);

/**
 * @brief 设置文本框是否自适应尺寸
 * @param[in] textbox : 文本指针
 * @param[in] autosize : 是否自适应尺寸
 **/
void compo_textbox_set_autosize(compo_textbox_t *textbox, bool autosize);

/**
 * @brief 设置文本框是否为多行
 * @param[in] textbox : 文本指针
 * @param[in] multiline : 是否换行
 **/
void compo_textbox_set_multiline(compo_textbox_t *textbox, bool multiline);

/**
 * @brief 设置文本框是否居中
 * @param[in] textbox : 文本指针
 * @param[in] align_center : 是否居中
 **/
void compo_textbox_set_align_center(compo_textbox_t *textbox, bool align_center);

/**
 * @brief 设置文本框文字的颜色
 * @param[in] textbox : 文本指针
 * @param[in] color : 颜色
 **/
void compo_textbox_set_forecolor(compo_textbox_t *textbox, u16 color);

/**
 * @brief 设置文本框文字间的行间距
 * @param[in] textbox : 文本指针
 * @param[in] hspace : 行间距
 **/
void compo_textbox_set_hspace(compo_textbox_t *textbox, s8 hspace);

/**
 * @brief 设置文本框文字是否单词整体换行, 需要在widget_text_set()之前调用
 * @param[in] textbox : 文本指针
 * @param[in] wholewrap : 是否换行
 **/
void compo_textbox_set_wholewrap(compo_textbox_t *textbox, bool wholewrap);

/**
 * @brief 设置文本框文字是否等宽行间距
 * @param[in] textbox : 文本指针
 * @param[in] equal_hspace : 等宽行间距
 **/
void compo_textbox_set_equal_hspace(compo_textbox_t *textbox, bool equal_hspace);

/**
 * @brief 设置文本框的透明度
 * @param[in] textbox : 文本指针
 * @param[in] alpha : 透明度
 **/
void compo_textbox_set_alpha(compo_textbox_t *textbox, u8 alpha);

/**
 * @brief 设置文本框是否可见
 * @param[in] textbox : 文本指针
 * @param[in] visible : 是否可见
 **/
void compo_textbox_set_visible(compo_textbox_t *textbox, bool visible);

/**
 * @brief 设置文本框自动滚动
 * @param[in] textbox : 文本指针
 * @param[in] autoroll : 是否自动滚动
 **/
void compo_textbox_set_autoroll(compo_textbox_t *textbox, bool autoroll);

/**
 * @brief 设置文本框滚动模式
 * @param[in] textbox : 文本指针
 * @param[in] mode : 滚动模式，1:环形滚动 0、左右、普通滚动
 **/
void compo_textbox_set_autoroll_mode(compo_textbox_t *textbox, u8 mode);

/**
 * @brief 设置文本框滚动前后间隔
 * @param[in] textbox : 文本指针
 * @param[in] space_pixel : 环形滚动间隔，以像素为单位，默认是50
 **/
void compo_textbox_set_autoroll_circ_space(compo_textbox_t *textbox, u16 space_pixel);

/**
 * @brief 设置多行文本框可以拖动文本
 * @param[in] textbox : 文本指针
 * @param[in] autoroll : 是否可以拖动文本
 **/
void compo_textbox_set_multiline_drag(compo_textbox_t *textbox, bool is_drag);

/**
 * @brief 设置文本排版方向
 * @param[in] textbox : 文本指针
 * @param[in] right_align : 是否右对齐
 **/
void compo_textbox_set_right_align(compo_textbox_t *textbox, bool right_align);

/**
 * @brief 设置文本排版方向
 * @param[in] txt:文本控件
 * @param[in] direction:true 倒序(阿拉伯/波斯/希伯来文从右到左)排版, false 正序(CJK等从左到右)排版
 * @return 无
 **/
void compo_textbox_set_direction(compo_textbox_t *textbox, bool direction);

/**
 * @brief 获取文本排版方向
 * @param[in] txt:文本控件
 * @param[in] direction:true 倒序(阿拉伯/波斯/希伯来文从右到左)排版, false 正序(CJK等从左到右)排版
 * @return 无
 **/
bool compo_textbox_get_direction(compo_textbox_t *textbox);


/**
 * @brief 获取文本内容宽度
 * @return 返回文本内容宽度
 **/
s16 compo_textbox_get_wid(compo_textbox_t *textbox);


#endif
