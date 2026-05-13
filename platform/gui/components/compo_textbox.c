#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define COMPO_TEXTBOX_DIR_LR	-1		//从左往右
#define COMPO_TEXTBOX_DIR_RL	 1		//从右往左

/**
 * @brief 创建一个文本框
 * @param[in] frm : 窗体指针
 * @param[in] max_word_cnt : 最大字数
 * @return 返回文本指针
 **/
compo_textbox_t *compo_textbox_create(compo_form_t *frm, u16 max_word_cnt)
{
    compo_textbox_t *textbox = compo_create(frm, COMPO_TYPE_TEXTBOX);

    //前景文本
    void *txt = widget_text_create(frm->page_body, max_word_cnt);
    widget_text_set_autosize(txt, true);                    //默认为自适应大小
    widget_text_set_ellipsis(txt, true);
    textbox->txt = txt;
    textbox->roll_cb.tick = tick_get();
    return textbox;
}

 /**
 * @brief 创建一个文本框
 * @param[in] frm : 窗体指针
 * @param[in] parent : 父页面
 * @param[in] max_word_cnt : 最大字数
 * @return 返回文本指针
 **/
compo_textbox_t *compo_textbox_create_for_page(compo_form_t *frm, void* page, u16 max_word_cnt)
{
    compo_textbox_t *textbox = compo_create(frm, COMPO_TYPE_TEXTBOX);

    //前景文本
    //void *txt = widget_text_create(frm->page, max_word_cnt);
    void *txt = widget_text_create(page, max_word_cnt);
    widget_text_set_autosize(txt, true);                    //默认为自适应大小
    textbox->txt = txt;
    textbox->roll_cb.tick = tick_get();
    return textbox;
}

/**
 * @brief 设置字库资源地址
 * @param[in] textbox : 文本指针
 * @param[in] font_res_addr : 设置字库资源地址, 0则表示用回系统字体
 **/
void compo_textbox_set_font(compo_textbox_t *textbox, u32 font_res_addr)
{
    widget_text_set_font(textbox->txt, font_res_addr);
}

/**
 * @brief 设置文本框内容
 * @param[in] textbox : 文本指针
 * @param[in] text : 文本内容 例："遥遥领先"
 **/
void compo_textbox_set(compo_textbox_t *textbox, const char *text)
{
    widget_text_t *txt = textbox->txt;
    widget_text_set(txt, text);
    area_t rel_textbox_area = widget_text_get_box_area_rel(txt);
    area_t rel_text_area = widget_text_get_area(txt);
    bool align_center = widget_get_align_center(txt);
    s16 client_y = 0;

    rect_t textbox_rect = widget_get_location(txt);
    if (textbox_rect.wid && align_center) {
        if (0 == textbox_rect.hei) {   //当文本框为高度自适应且使能居中时，文本的坐标需按照左上角对齐
            textbox_rect.x += textbox_rect.wid / 2;
            textbox_rect.y += rel_text_area.hei / 2;
            widget_set_pos(txt, textbox_rect.x, textbox_rect.y);
        } else {                       //当文本框指定了高度且为多行居中时，内容上下也居中(视用户需要可屏蔽)
            u8 line_cnt = widget_text_get_line_cnt(txt);
            if (textbox_rect.hei > rel_text_area.hei) {
                if (line_cnt > 1 || (line_cnt <= 1 && textbox_rect.wid < rel_text_area.wid)) {
                    client_y = (textbox_rect.hei - rel_text_area.hei) >> 1;
                }
            }
        }
    }

    if (textbox->multiline && rel_textbox_area.hei < rel_text_area.hei && rel_textbox_area.hei != 0 && !widget_text_get_ellipsis(txt)) {
        textbox->roll_cb.direction = -1;
        compo_textbox_set_autoroll_mode(textbox, TEXT_AUTOROLL_MODE_SROLL);
        widget_text_set_client(txt, 0, client_y);

    } else {
        if (rel_text_area.wid > rel_textbox_area.wid) {
            textbox->roll_cb.direction = widget_text_get_right_align(txt) ? COMPO_TEXTBOX_DIR_RL : COMPO_TEXTBOX_DIR_LR;
        } else {
            textbox->roll_cb.direction = 0;
        }

        textbox->roll_cb.sta = COMPO_ROLL_STA_IDLE;

        //未设置文本框宽度默认不滚动
        if (rel_textbox_area.wid) {
            if ((rel_text_area.wid > rel_textbox_area.wid)) {
                compo_textbox_set_autoroll_mode(textbox, TEXT_AUTOROLL_MODE_SROLL_CIRC);
            } else {
                textbox->roll_cb.offset = 0;
                compo_textbox_set_autoroll_mode(textbox, TEXT_AUTOROLL_MODE_NULL);

            }
        }

#if BOX_GUI_ROTATE_DISP
        textbox->roll_cb.offset = 0;
#endif
        widget_text_set_client(txt, textbox->roll_cb.offset, client_y);
    }
}

/**
 * @brief 获取文本框内容行数
 * @param[in] textbox : 文本指针
 **/
u8 compo_textbox_get_line_cnt(compo_textbox_t *textbox)
{
    return widget_text_get_line_cnt(textbox->txt);
}

/**
 * @brief 设置文本框的坐标及大小
          注意：文本框设置的坐标是以默认的中心点作为参考
 * @param[in] textbox : 文本指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 文本框宽度
 * @param[in] height : 文本框高度
 **/
void compo_textbox_set_location(compo_textbox_t *textbox, s16 x, s16 y, s16 width, s16 height)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_location(textbox->txt, x, y, width, height);
#else
    widget_set_location(textbox->txt, x, y, width, height);
#endif // BOX_GUI_ROTATE_DISP
    if (0 != width && 0 != height) {
         widget_text_set_autosize(textbox->txt, false);
    } else {
        widget_text_set_ellipsis(textbox->txt, false);
    }
}

/**
 * @brief 设置文本框的坐标
          注意：文本框设置的坐标是以默认的中心点作为参考
 * @param[in] textbox : 文本指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_textbox_set_pos(compo_textbox_t *textbox, s16 x, s16 y)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_pos(textbox->txt, x, y);
#else
    widget_set_pos(textbox->txt, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 设置文本框是否自适应尺寸
 * @param[in] textbox : 文本指针
 * @param[in] autosize : 是否自适应尺寸
 **/
void compo_textbox_set_autosize(compo_textbox_t *textbox, bool autosize)
{
    widget_text_set_autosize(textbox->txt, autosize);
}

/**
 * @brief 设置文本框是否为多行
 * @param[in] textbox : 文本指针
 * @param[in] multiline : 是否换行
 **/
void compo_textbox_set_multiline(compo_textbox_t *textbox, bool multiline)
{
    textbox->multiline = multiline;
    widget_text_set_wordwrap(textbox->txt, multiline);
}

/**
 * @brief 设置文本框是否居中
 * @param[in] textbox : 文本指针
 * @param[in] align_center : 是否居中
 **/
void compo_textbox_set_align_center(compo_textbox_t *textbox, bool align_center)
{
    widget_set_align_center(textbox->txt, align_center);
}

/**
 * @brief 设置文本框文字的颜色
 * @param[in] textbox : 文本指针
 * @param[in] color : 颜色
 **/
void compo_textbox_set_forecolor(compo_textbox_t *textbox, u16 color)
{
    widget_text_set_color(textbox->txt, color);
}

/**
 * @brief 设置文本框文字间的行间距
 * @param[in] textbox : 文本指针
 * @param[in] hspace : 行间距
 **/
void compo_textbox_set_hspace(compo_textbox_t *textbox, s8 hspace)
{
    widget_text_set_hspace(textbox->txt, hspace);
}

/**
 * @brief 设置文本框文字是否单词整体换行, 需要在widget_text_set()之前调用
 * @param[in] textbox : 文本指针
 * @param[in] wholewrap : 是否换行
 **/
void compo_textbox_set_wholewrap(compo_textbox_t *textbox, bool wholewrap)
{
    widget_text_set_wholewrap(textbox->txt, wholewrap);
}

/**
 * @brief 设置文本框文字是否等宽行间距
 * @param[in] textbox : 文本指针
 * @param[in] equal_hspace : 等宽行间距
 **/
void compo_textbox_set_equal_hspace(compo_textbox_t *textbox, bool equal_hspace)
{
    widget_text_set_equal_hspace(textbox->txt, equal_hspace);
}

/**
 * @brief 设置文本框的透明度
 * @param[in] textbox : 文本指针
 * @param[in] alpha : 透明度
 **/
void compo_textbox_set_alpha(compo_textbox_t *textbox, u8 alpha)
{
    widget_set_alpha(textbox->txt, alpha);
}

/**
 * @brief 设置文本框是否可见
 * @param[in] textbox : 文本指针
 * @param[in] visible : 是否可见
 **/
void compo_textbox_set_visible(compo_textbox_t *textbox, bool visible)
{
    widget_set_visible(textbox->txt, visible);
}

/**
 * @brief 设置文本框自动滚动
 * @param[in] textbox : 文本指针
 * @param[in] autoroll : 是否自动滚动
 **/
void compo_textbox_set_autoroll(compo_textbox_t *textbox, bool autoroll)
{
    area_t text_area = widget_text_get_area(textbox->txt);
    rect_t textbox_rect = rotate_widget_get_location(textbox->txt);

    textbox->roll_cb.autoroll = autoroll;

    if (textbox_rect.wid <= text_area.wid || (textbox->multiline && textbox_rect.hei <= text_area.hei)) {
        textbox->roll_cb.tick = tick_get();
        textbox->roll_cb.direction = widget_text_get_right_align(textbox->txt) ? COMPO_TEXTBOX_DIR_RL : COMPO_TEXTBOX_DIR_LR;
    } else {
        textbox->roll_cb.direction = 0;
    }
}

/**
 * @brief 设置文本框滚动模式
 * @param[in] textbox : 文本指针
 * @param[in] mode : 滚动模式，0:无滚动 1:环形滚动 2、左右、普通滚动
 **/
void compo_textbox_set_autoroll_mode(compo_textbox_t *textbox, u8 mode)
{
    if ((textbox->roll_cb.mode == mode) || (textbox->multiline && mode == TEXT_AUTOROLL_MODE_SROLL_CIRC)) {
        return;
    }

    textbox->roll_cb.mode = mode;
    widget_text_set_autoroll_mode(textbox->txt, mode);
    widget_text_set_ellipsis(textbox->txt, false);      //避免既有滚动又有省略号的情况
}

/**
 * @brief 设置文本框滚动前后间隔
 * @param[in] textbox : 文本指针
 * @param[in] space_pixel : 环形滚动间隔，以像素单位，默认是50
 **/
void compo_textbox_set_autoroll_circ_space(compo_textbox_t *textbox, u16 space_pixel)
{
    widget_text_set_autoroll_circ_space(textbox->txt, space_pixel);
}

/**
 * @brief 设置文本排版方向
 * @param[in] textbox : 文本指针
 * @param[in] right_align : 是否右对齐
 **/
void compo_textbox_set_right_align(compo_textbox_t *textbox, bool right_align)
{
    widget_text_set_right_align(textbox->txt, right_align);
}

/**
 * @brief 获取文本排版方向
 * @param[in] textbox : 文本指针
 * @return 返回排版方向0:左对齐 1:右对齐
 **/
bool compo_textbox_get_right_align(compo_textbox_t *textbox)
{
    return widget_text_get_right_align(textbox->txt);
}

/**
 * @brief 设置文本排版方向
 * @param[in] txt:文本控件
 * @param[in] direction:true 倒序(阿拉伯/波斯/希伯来文从右到左)排版, false 正序(CJK等从左到右)排版
 * @return 无
 **/
void compo_textbox_set_direction(compo_textbox_t *textbox, bool direction)
{
    widget_text_set_direction(textbox->txt, direction);
}

/**
 * @brief 获取文本排版方向
 * @param[in] txt:文本控件
 * @param[in] direction:true 倒序(阿拉伯/波斯/希伯来文从右到左)排版, false 正序(CJK等从左到右)排版
 * @return 无
 **/
bool compo_textbox_get_direction(compo_textbox_t *textbox)
{
    return widget_text_get_direction(textbox->txt);
}

/**
 * @brief 设置多行文本框可以拖动文本
 * @param[in] textbox : 文本指针
 * @param[in] autoroll : 是否可以拖动文本
 **/
void compo_textbox_set_multiline_drag(compo_textbox_t *textbox, bool is_drag)
{
    textbox->roll_cb.is_drag = is_drag;
    textbox->roll_cb.tick = tick_get();
}

/**
 * @brief 获取文本内容宽度
 * @return 返回实际显示内容的长宽
 **/
s16 compo_textbox_get_wid(compo_textbox_t *textbox)
{
    area_t rect;
    rect = widget_text_get_area(textbox->txt);
    return rect.wid;
}
