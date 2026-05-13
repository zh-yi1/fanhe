#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define MAX_WORD_CNT                            32                          //每条列表项最多16个字符

/**
 * @brief 创建一个卡片控件
 * @param[in] frm : 窗体指针
 * @param[in] rect_num : 卡片包含的矩形背景数量
 * @param[in] icon_num : 卡片包含的图片数量
 * @param[in] text_num : 卡片包含的文本数量
 * @param[in] org_wid, org_hei : 卡片原始宽高
 * @return 返回卡片指针
 **/
compo_cardbox_t *compo_cardbox_create(compo_form_t *frm, u8 rect_num, u8 icon_num, u8 text_num, u16 org_wid, u16 org_hei)
{
    int i;
    compo_cardbox_t *cardbox = compo_create(frm, COMPO_TYPE_CARDBOX);
    widget_page_t *page = widget_page_create(frm->page_body);
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_size(page, org_wid, org_hei);
    rotate_widget_page_set_client(page, org_wid / 2, org_hei / 2); //卡片内控件参考坐标：卡片中心
    cardbox->page = page;
    cardbox->org_wid = org_wid;
    cardbox->org_hei = org_hei;
#else
    widget_set_size(page, org_wid, org_hei);
    widget_page_set_client(page, org_wid / 2, org_hei / 2); //卡片内控件参考坐标：卡片中心
    cardbox->page = page;
#endif // BOX_GUI_ROTATE_DISP
    rect_num = MIN(rect_num, CARD_RECT_MAX);
    for (i=0; i<rect_num; i++) {
        cardbox->rect[i] = widget_rect_create(page);
    }
    icon_num = MIN(icon_num, CARD_ICON_MAX);
    for (i=0; i<icon_num; i++) {
        cardbox->icon[i] = widget_image_create(page, 0);
    }
    text_num = MIN(text_num, CARD_TEXT_MAX);
    for (i=0; i<text_num; i++) {
        cardbox->text[i] = widget_text_create(page, MAX_WORD_CNT);
        widget_text_set_autosize(cardbox->text[i], false);
    }

    return cardbox;
}

/**
 * @brief 设置卡片（圆角）矩形背景颜色
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 矩形指针存储序号
 * @param[in] color : 色值
 **/
void compo_cardbox_rect_set_color(compo_cardbox_t *cardbox, u8 idx, u16 color)
{
    if (cardbox->rect[idx] && idx < CARD_RECT_MAX) {
        widget_rect_set_color(cardbox->rect[idx], color);
    }
}

/**
 * @brief 设置卡片（圆角）矩形背景尺寸
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 矩形指针存储序号
 * @param[in] x,y,wid,hei,r : 位置、宽、高、圆角半径
 **/
void compo_cardbox_rect_set_location(compo_cardbox_t *cardbox, u8 idx, s16 x, s16 y, u16 wid, u16 hei, u16 r)
{
    if (cardbox->rect[idx] && idx < CARD_RECT_MAX) {
#if BOX_GUI_ROTATE_DISP
        ro_widget_set_location(cardbox->rect[idx], x, y, wid, hei, cardbox->org_wid, cardbox->org_hei);
#else
        widget_set_location(cardbox->rect[idx], x, y, wid, hei);
#endif // BOX_GUI_ROTATE_DISP
        widget_rect_set_radius(cardbox->rect[idx], r);
    }
}


/**
 * @brief 设置卡片控件的图片裁剪
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 图片指针存储序号
 * @param[in] index : 裁剪序号
 * @param[in] total_cnt : 分割总数
 **/
void compo_cardbox_icon_cut(compo_cardbox_t *cardbox, u8 idx, u8 index, u8 total_cnt)
{
    widget_image_t *img = cardbox->icon[idx] ;
	if (img && idx < CARD_ICON_MAX && total_cnt && index < total_cnt) {
	    rect_t rect = widget_get_location(img);
        area_t area = widget_image_get_size(img);
        widget_image_cut(img, 0, index * (area.hei / total_cnt), area.wid, area.hei / total_cnt);
        widget_set_size(img, rect.wid, rect.hei);
	}
}

/**
 * @brief 设置卡片控件的图片
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 图片指针存储序号
 * @param[in] x,y,wid,hei : 位置和大小
 **/
void compo_cardbox_icon_set_location(compo_cardbox_t *cardbox, u8 idx, s16 x, s16 y, u16 wid, u16 hei)
{
    if (cardbox->icon[idx] && idx < CARD_ICON_MAX) {
#if BOX_GUI_ROTATE_DISP
        ro_widget_set_location(cardbox->icon[idx], x, y, wid, hei, cardbox->org_wid, cardbox->org_hei);
#else
        widget_set_location(cardbox->icon[idx], x, y, wid, hei);
#endif // BOX_GUI_ROTATE_DISP
    }
}

/**
 * @brief 设置卡片控件的图片
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 图片指针存储序号
 * @param[in] x,y,wid,hei : 位置和大小
 **/
void compo_cardbox_icon_set_pos(compo_cardbox_t *cardbox, u8 idx, s16 x, s16 y)
{
    if (cardbox->icon[idx] && idx < CARD_ICON_MAX) {
#if BOX_GUI_ROTATE_DISP
        ro_widget_set_pos(cardbox->icon[idx], x, y, cardbox->org_wid, cardbox->org_hei);
#else
        widget_set_pos(cardbox->icon[idx], x, y);
#endif // BOX_GUI_ROTATE_DISP
    }
}

/**
 * @brief 更改卡片控件的图片
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 图片指针存储序号
 * @param[in] res_addr : 资源地址
 **/
void compo_cardbox_icon_set(compo_cardbox_t *cardbox, u8 idx, u32 res_addr)
{
    if (cardbox->icon[idx] && idx < CARD_ICON_MAX) {
        widget_image_set(cardbox->icon[idx], res_addr);
    }
}

/**
 * @brief 设置卡片控件的文本
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @param[in] x,y,wid,hei : 位置和大小（默认非autosize）
 **/
void compo_cardbox_text_set_location(compo_cardbox_t *cardbox, u8 idx, s16 x, s16 y, u16 wid, u16 hei)
{
    if (cardbox->text[idx] && idx < CARD_TEXT_MAX) {
#if BOX_GUI_ROTATE_DISP
        ro_widget_set_location(cardbox->text[idx], x, y, wid, hei, cardbox->org_wid, cardbox->org_hei);
#else
        widget_set_location(cardbox->text[idx], x, y, wid, hei);
#endif // BOX_GUI_ROTATE_DISP
    }
}

/**
 * @brief 设置卡片控件的文本前景色
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @param[in] color : 色值
 **/
void compo_cardbox_text_set_forecolor(compo_cardbox_t *cardbox, u8 idx, u16 color)
{
    if (cardbox->text[idx] && idx < CARD_TEXT_MAX) {
        widget_text_set_color(cardbox->text[idx], color);
    }
}

/**
 * @brief 更新卡片控件的文本内容
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @param[in] str : 文本字符串指针
 **/
void compo_cardbox_text_set(compo_cardbox_t *cardbox, u8 idx, const char *str)
{
#if BOX_GUI_ROTATE_DISP
    if (cardbox->text[idx] && idx < CARD_TEXT_MAX) {
        widget_text_t *txt = cardbox->text[idx];
        widget_text_set(txt, str);
        if (idx == 0) {
            memset(&cardbox->roll_cb, 0, sizeof(compo_roll_cb_t));
            widget_text_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
            widget_text_set_client(txt, 0, 0);
            if (widget_get_visble(txt)) {
                area_t text_area = widget_text_get_area(txt);
                area_t textbox_area = widget_text_get_box_area_rel(txt);
                if (textbox_area.wid && text_area.wid > textbox_area.wid) {
                    cardbox->roll_cb.tick = tick_get();
                    cardbox->roll_cb.mode = TEXT_AUTOROLL_MODE_SROLL_CIRC;
                    cardbox->roll_cb.direction = -1;
                    widget_text_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_SROLL_CIRC);
                }
            }
        }
    }
#else
    if (cardbox->text[idx] && idx < CARD_TEXT_MAX) {
        widget_text_set(cardbox->text[idx], str);
    }
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 更新卡片控件文本0的内容（带滚动控制器）
 * @param[in] cardbox : 卡片指针
 * @param[in] str : 文本字符串指针
 * @param[in] roll_cb_p : 滚动控制器指针
 **/
void compo_cardbox_text0_set(compo_cardbox_t *cardbox, const char *str, const compo_roll_cb_t *roll_cb_p)
{
    if (cardbox->text[0]) {
        widget_text_t *txt = cardbox->text[0];
        widget_text_set(txt, str);
        memset(&cardbox->roll_cb, 0, sizeof(compo_roll_cb_t));
        area_t text_area = widget_text_get_area(txt);
        area_t textbox_area = widget_text_get_box_area_rel(txt);
        if (textbox_area.wid && text_area.wid > textbox_area.wid && roll_cb_p->mode == TEXT_AUTOROLL_MODE_SROLL_CIRC) {
            if (abs_s(roll_cb_p->offset) < text_area.wid + widget_text_get_autoroll_circ_pixel(txt)) {  //防止offset过大导致蓝屏
                cardbox->roll_cb = *roll_cb_p;
            } else {
                cardbox->roll_cb.tick = tick_get();
                cardbox->roll_cb.mode = TEXT_AUTOROLL_MODE_SROLL_CIRC;
                cardbox->roll_cb.direction = -1;
            }
            widget_text_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_SROLL_CIRC);
            widget_text_set_client(txt, cardbox->roll_cb.offset, 0);
        }
        if (cardbox->roll_cb.mode == TEXT_AUTOROLL_MODE_NULL) {
            widget_text_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
            widget_text_set_client(txt, 0, 0);
        }
    }
}
/**
 * @brief 设置卡片控件的文本是否中心点对齐
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @param[in] str : 文本字符串指针
 **/
void compo_cardbox_text_set_align_center(compo_cardbox_t *cardbox, u8 idx, bool align_center)
{
    if (cardbox->text[idx] && idx < CARD_TEXT_MAX) {
        widget_set_align_center(cardbox->text[idx], align_center);
    }
}

/**
 * @brief 更换卡片控件的文本字库
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @param[in] font_res_addr : 设置字库资源地址
 **/
void compo_cardbox_text_set_font(compo_cardbox_t *cardbox, u8 idx, u32 font_res_addr)
{
    if (cardbox->text[idx] && idx < CARD_TEXT_MAX) {
        widget_text_set_font(cardbox->text[idx], font_res_addr);
    }
}

/**
 * @brief 设置卡片内文本框是否自适应尺寸
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @param[in] autosize : 是否自适应尺寸
 **/
void compo_cardbox_text_set_autosize(compo_cardbox_t *cardbox, u8 idx, bool autosize)
{
    if (cardbox->text[idx] && idx < CARD_TEXT_MAX) {
        widget_text_set_autosize(cardbox->text[idx], autosize);
    }
}

/**
 * @brief 获取卡片内文本字体区域
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @return 文本内容的宽度和高度
 **/
area_t compo_cardbox_text_get_area(compo_cardbox_t *cardbox, u8 idx)
{
    area_t area = {0};
    if (cardbox->text[idx] && idx < CARD_TEXT_MAX) {
        area = widget_text_get_area(cardbox->text[idx]);
    }
    return area;
}

/**
 * @brief 设置卡片控件的位置和缩放大小
 * @param[in] cardbox : 卡片指针
 * @param[in] x,y,wid,hei : 位置和缩放大小
 **/
void compo_cardbox_set_location(compo_cardbox_t *cardbox, s16 x, s16 y, u16 wid, u16 hei)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_page_scale_to(cardbox->page, wid, hei);
    rotate_widget_set_pos(cardbox->page, x, y);
#else
    widget_page_scale_to(cardbox->page, wid, hei);
    widget_set_pos(cardbox->page, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 设置卡片控件的位置
 * @param[in] cardbox : 卡片指针
 * @param[in] x,y : 位置
 **/
void compo_cardbox_set_pos(compo_cardbox_t *cardbox, s16 x, s16 y)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_pos(cardbox->page, x, y);
#else
    widget_set_pos(cardbox->page, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 获取卡片控件的位置和缩放大小
 * @param[in] cardbox : 卡片指针
 * @param[in] x,y,wid,hei : 位置和缩放大小
 **/
rect_t compo_cardbox_get_location(compo_cardbox_t *cardbox)
{
    rect_t rect = {0};
#if BOX_GUI_ROTATE_DISP
    rect = rotate_widget_get_location(cardbox->page);

#else
    rect = widget_get_location(cardbox->page);
#endif // BOX_GUI_ROTATE_DISP
    return rect;
}

/**
 * @brief 获取卡片绝对位置和大小
 * @param[in] cardbox : 卡片指针
 * @return 返回卡片page绝对位置和大小
 **/
rect_t compo_cardbox_get_absolute(compo_cardbox_t *cardbox)
{
#if BOX_GUI_ROTATE_DISP
    return rotate_widget_get_absolute(cardbox->page);
#else
    return widget_get_absolute(cardbox->page);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 获取卡片内矩形的绝对位置和大小
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 矩形指针存储序号
 * @return 返回相应序号图片的绝对位置和大小
 **/
rect_t compo_cardbox_get_rect_absolute(compo_cardbox_t *cardbox, u8 idx)
{
    rect_t rect = {0};
    if (cardbox->rect[idx] && idx < CARD_RECT_MAX) {
#if BOX_GUI_ROTATE_DISP
        rect = rotate_widget_get_absolute(cardbox->rect[idx]);
#else
        rect = widget_get_absolute(cardbox->rect[idx]);
#endif // BOX_GUI_ROTATE_DISP
    }
    return rect;
}

/**
 * @brief 获取卡片内图片的绝对位置和大小
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 图片指针存储序号
 * @return 返回相应序号图片的绝对位置和大小
 **/
rect_t compo_cardbox_get_icon_absolute(compo_cardbox_t *cardbox, u8 idx)
{
    rect_t rect = {0};
    if (cardbox->icon[idx] && idx < CARD_ICON_MAX) {
#if BOX_GUI_ROTATE_DISP
        rect = rotate_widget_get_absolute(cardbox->icon[idx]);
#else
        rect = widget_get_absolute(cardbox->icon[idx]);
#endif // BOX_GUI_ROTATE_DISP
    }
    return rect;
}

/**
 * @brief 设置卡片中图片可见性
 * @param[in] cardbox : 卡片指针
 **/
void compo_cardbox_icon_set_visible(compo_cardbox_t *cardbox, u8 idx, bool visible)
{
    if (cardbox->icon[idx] && idx < CARD_ICON_MAX) {
        widget_set_visible(cardbox->icon[idx], visible);
    }
}

/**
 * @brief 设置卡片中文字可见性
 * @param[in] cardbox : 卡片指针
 **/
void compo_cardbox_text_set_visible(compo_cardbox_t *cardbox, u8 idx, bool visible)
{
    if (cardbox->text[idx] && idx < CARD_TEXT_MAX) {
        widget_set_visible(cardbox->text[idx], visible);
    }
}

/**
 * @brief 设置卡片可见性
 * @param[in] cardbox : 卡片指针
 **/
void compo_cardbox_set_visible(compo_cardbox_t *cardbox, bool visible)
{
    widget_set_visible(cardbox->page, visible);
}

/**
 * @brief 获取卡片可见性
 * @param[in] cardbox : 卡片指针
 **/
bool compo_cardbox_get_visible(compo_cardbox_t *cardbox)
{
    return widget_get_visble(cardbox->page);
}


/**
 * @brief 设置相对坐标
 * @param[in] cardbox : 卡片指针
 **/
void compo_cardbox_set_client(compo_cardbox_t *cardbox, s16 x, s16 y)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_page_set_client(cardbox->page, x, y);
#else
    widget_page_set_client(cardbox->page, x, y);
#endif // BOX_GUI_ROTATE_DISP
}


/**
 * @brief 判断点击区域是否在卡片内
 * @param[in] cardbox : 卡片指针
 * @param[in] pt : 点击区域
 * @return true or false
 **/
bool compo_cardbox_btn_is(compo_cardbox_t *cardbox, point_t pt)
{
    rect_t rect = compo_cardbox_get_absolute(cardbox);

    if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
        return true;
    }

    return false;
}


/**
 * @brief 设置透明度
 * @param[in] cardbox : 卡片指针
* @param[in] alpha : 透明度(0 ~ 0xff)
 **/
void compo_cardbox_set_alpha(compo_cardbox_t *cardbox, u8 alpha)
{
    widget_set_alpha(cardbox->page, alpha);
}
