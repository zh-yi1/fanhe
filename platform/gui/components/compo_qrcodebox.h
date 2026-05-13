#ifndef _COMPO_QRCODEBOX_H
#define _COMPO_QRCODEBOX_H

#define QR_LEVEL_L				0								//7%的字码可被修正
#define QR_LEVEL_M				1								//15%的字码可被修正
#define QR_LEVEL_Q				2								//25%的字码可被修正
#define QR_LEVEL_H				3								//30%的字码可被修正

typedef struct compo_qrcodebox_t_ {
    COMPO_STRUCT_COMMON;
    widget_rect_t *bg;
    widget_qrcode_t *qrcode;
} compo_qrcodebox_t;

/**
 * @brief 创建一个码
 * @param[in] frm : 窗体指针
 * @param[in] qr_type : 二维码或条形码
 * @param[in] max_ch_cnt : 最大字符长度
 * @return 返回码
 **/
compo_qrcodebox_t *compo_qrcodebox_create(compo_form_t *frm, u8 qr_type, u16 max_ch_cnt);

/**
 * @brief 设置码的内容
 * @param[in] qrcodebox : 码指针
 * @param[in] str : 字符内容
 **/
void compo_qrcodebox_set(compo_qrcodebox_t *qrcodebox, const char *str);

/**
 * @brief 设置二维码的内容
 * @param[in] qrcodebox : 码指针
 * @param[in] code : 字符内容
 * @param[in] len : 长度
 **/
void compo_qrcodebox_2d_set(compo_qrcodebox_t *qrcodebox, const char *code, int len);

/**
 * @brief 设置最小单位宽度
 * @param[in] qrcodebox : 码指针
 * @param[in] bit_wid : 最小宽度
 **/
void compo_qrcodebox_set_bitwid(compo_qrcodebox_t *qrcodebox, u16 qr_wid);

/**
 * @brief 设置最小单位宽度
 * @param[in] qrcodebox : 码指针
 * @param[in] bit_wid : 设置要显示宽度(高度=宽度)
 **/
void compo_qrcodebox_set_bitwid_by_qrwid(compo_qrcodebox_t *qrcodebox, u16 qr_wid);

/**
 * @brief 设置码的内容
 * @param[in] qrcodebox : 码指针
 * @param[in] level: 纠错等级
 **/
void compo_qrcodebox_set_level(compo_qrcodebox_t *qrcodebox, u8 level);

/**
 * @brief 设置二维码的编码格式
 * @param[in] qrcodebox : 码指针
 * @param[in] type: 0 code128A, 1 code 128C(不支持字符串，优点比较小)
 **/
void compo_barcode_set_type(compo_qrcodebox_t *qrcodebox, u8 type);

#endif
