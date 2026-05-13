#ifndef _COMPO_CARDBOX_H
#define _COMPO_CARDBOX_H

#define CARD_RECT_MAX   2
#define CARD_ICON_MAX   5
#define CARD_TEXT_MAX   5

typedef struct compo_cardbox_t_ {
    COMPO_STRUCT_COMMON;
    widget_page_t *page;
    u16 org_wid;
    u16 org_hei;
    widget_rect_t *rect[CARD_RECT_MAX];
    widget_image_t *icon[CARD_ICON_MAX];    //icon已改为image，兼容旧版本，函数名不变
    widget_text_t *text[CARD_TEXT_MAX];
    compo_roll_cb_t roll_cb;    //只对text[0]设一个滚动控制器
} compo_cardbox_t;

/**
 * @brief 创建一个卡片控件
 * @param[in] frm : 窗体指针
 * @param[in] rect_num : 卡片包含的矩形背景数量
 * @param[in] icon_num : 卡片包含的图片数量
 * @param[in] text_num : 卡片包含的文本数量
 * @param[in] org_wid, org_hei : 卡片原始宽高
 * @return 返回卡片指针
 **/
compo_cardbox_t *compo_cardbox_create(compo_form_t *frm, u8 rect_num, u8 icon_num, u8 text_num, u16 org_wid, u16 org_hei);

/**
 * @brief 设置卡片（圆角）矩形背景颜色
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 矩形指针存储序号
 * @param[in] color : 色值
 **/
void compo_cardbox_rect_set_color(compo_cardbox_t *cardbox, u8 idx, u16 color);

/**
 * @brief 设置卡片（圆角）矩形背景尺寸
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 矩形指针存储序号
 * @param[in] x,y,wid,hei,r : 位置、宽、高、圆角半径
 **/
void compo_cardbox_rect_set_location(compo_cardbox_t *cardbox, u8 idx, s16 x, s16 y, u16 wid, u16 hei, u16 r);

/**
 * @brief 更改卡片控件的图片资源
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 图片指针存储序号
 * @param[in] res_addr : 资源地址
 **/
void compo_cardbox_icon_set(compo_cardbox_t *cardbox, u8 idx, u32 res_addr);

/**
 * @brief 设置卡片控件的图片裁剪
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 图片指针存储序号
 * @param[in] cnt : 裁剪序号
 * @param[in] total_cnt : 分割总数
 **/
void compo_cardbox_icon_cut(compo_cardbox_t *cardbox, u8 idx, u8 index, u8 total_cnt);

/**
 * @brief 设置卡片控件的图片位置和大小
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 图片指针存储序号
 * @param[in] x,y,wid,hei : 位置和大小
 **/
void compo_cardbox_icon_set_location(compo_cardbox_t *cardbox, u8 idx, s16 x, s16 y, u16 wid, u16 hei);

/**
 * @brief 更新卡片控件的文本内容
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @param[in] str : 文本字符串指针
 **/
void compo_cardbox_text_set(compo_cardbox_t *cardbox, u8 idx, const char *str);

/**
 * @brief 更新卡片控件文本0的内容（带滚动控制器）
 * @param[in] cardbox : 卡片指针
 * @param[in] str : 文本字符串指针
 * @param[in] roll_cb_p : 滚动控制器指针
 **/
void compo_cardbox_text0_set(compo_cardbox_t *cardbox, const char *str, const compo_roll_cb_t *roll_cb_p);


/**
 * @brief 设置卡片控件的文本是否中心点对齐
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @param[in] str : 文本字符串指针
 **/
void compo_cardbox_text_set_align_center(compo_cardbox_t *cardbox, u8 idx, bool align_center);

/**
 * @brief 更换卡片控件的文本字库
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @param[in] font_res_addr : 设置字库资源地址
 **/
void compo_cardbox_text_set_font(compo_cardbox_t *cardbox, u8 idx, u32 font_res_addr);

/**
 * @brief 设置卡片内文本框是否自适应尺寸
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @param[in] autosize : 是否自适应尺寸
 **/
void compo_cardbox_text_set_autosize(compo_cardbox_t *cardbox, u8 idx, bool autosize);

/**
 * @brief 获取卡片内文本字体区域
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @return 文本内容的宽度和高度
 **/
area_t compo_cardbox_text_get_area(compo_cardbox_t *cardbox, u8 idx);

/**
 * @brief 设置卡片控件的文本
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @param[in] str : 文本字符串指针
 * @param[in] x,y,wid,hei : 位置和大小（默认非autosize）
 **/
void compo_cardbox_text_set_location(compo_cardbox_t *cardbox, u8 idx, s16 x, s16 y, u16 wid, u16 hei);

/**
 * @brief 设置卡片控件的文本前景色
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 文本指针存储序号
 * @param[in] color : 色值
 **/
void compo_cardbox_text_set_forecolor(compo_cardbox_t *cardbox, u8 idx, u16 color);

/**
 * @brief 设置卡片控件的位置和缩放大小
 * @param[in] cardbox : 卡片指针
 * @param[in] x,y,wid,hei : 位置和缩放大小
 **/
void compo_cardbox_set_location(compo_cardbox_t *cardbox, s16 x, s16 y, u16 wid, u16 hei);

/**
 * @brief 设置卡片控件的位置
 * @param[in] cardbox : 卡片指针
 * @param[in] x,y : 位置
 **/
void compo_cardbox_set_pos(compo_cardbox_t *cardbox, s16 x, s16 y);

/**
 * @brief 设置卡片控件的图片
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 图片指针存储序号
 * @param[in] x,y,wid,hei : 位置和大小
 **/
void compo_cardbox_icon_set_pos(compo_cardbox_t *cardbox, u8 idx, s16 x, s16 y);

/**
 * @brief 获取卡片控件的位置和缩放大小
 * @param[in] cardbox : 卡片指针
 * @param[in] x,y,wid,hei : 位置和缩放大小
 **/
rect_t compo_cardbox_get_location(compo_cardbox_t *cardbox);

/**
 * @brief 获取卡片绝对位置和大小
 * @param[in] cardbox : 卡片指针
 * @return 返回卡片page绝对位置和大小
 **/
rect_t compo_cardbox_get_absolute(compo_cardbox_t *cardbox);

/**
 * @brief 获取卡片内矩形的绝对位置和大小
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 矩形指针存储序号
 * @return 返回相应序号图片的绝对位置和大小
 **/
rect_t compo_cardbox_get_rect_absolute(compo_cardbox_t *cardbox, u8 idx);

/**
 * @brief 获取卡片内图片的绝对位置和大小
 * @param[in] cardbox : 卡片指针
 * @param[in] idx : 图片指针存储序号
 * @return 返回相应序号图片的绝对位置和大小
 **/
rect_t compo_cardbox_get_icon_absolute(compo_cardbox_t *cardbox, u8 idx);

/**
 * @brief 设置卡片中图片可见性
 * @param[in] cardbox : 卡片指针
 **/
void compo_cardbox_icon_set_visible(compo_cardbox_t *cardbox, u8 idx, bool visible);

/**
 * @brief 设置卡片中文字可见性
 * @param[in] cardbox : 卡片指针
 **/
void compo_cardbox_text_set_visible(compo_cardbox_t *cardbox, u8 idx, bool visible);

/**
 * @brief 设置卡片可见性
 * @param[in] cardbox : 卡片指针
 **/
void compo_cardbox_set_visible(compo_cardbox_t *cardbox, bool visible);

/**
 * @brief 获取卡片可见性
 * @param[in] cardbox : 卡片指针
 **/
bool compo_cardbox_get_visible(compo_cardbox_t *cardbox);

/**
 * @brief 设置相对坐标
 * @param[in] cardbox : 卡片指针
 **/
void compo_cardbox_set_client(compo_cardbox_t *cardbox, s16 x, s16 y);

/**
 * @brief 判断点击区域是否在卡片内
 * @param[in] cardbox : 卡片指针
 * @param[in] pt : 点击区域
 * @return true or false
 **/
bool compo_cardbox_btn_is(compo_cardbox_t *cardbox, point_t pt);

/**
 * @brief 设置透明度
 * @param[in] cardbox : 卡片指针
* @param[in] alpha : 透明度(0 ~ 0xff)
 **/
void compo_cardbox_set_alpha(compo_cardbox_t *cardbox, u8 alpha);

#endif
