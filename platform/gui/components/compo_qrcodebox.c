#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define QRCODEBOX_PADDING       (GUI_SCREEN_WIDTH / 10)         //二维码和背景的间隙
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
#define USR_MODE8_ONLY          1
#else 
#define USR_MODE8_ONLY          0
#endif

#if !USR_MODE8_ONLY
bool qr_encode_create_ex(const char *str, u8 *qr_buff, int str_len, u16 *symble_size, u8 level)
{
    return false;
}
#endif

/**
 * @brief 创建一个码
 * @param[in] frm : 窗体指针
 * @param[in] qr_type : 二维码或条形码
 * @param[in] max_ch_cnt : 最大字符长度
 * @return 返回码
 **/
compo_qrcodebox_t *compo_qrcodebox_create(compo_form_t *frm, u8 qr_type, u16 max_ch_cnt)
{
    compo_qrcodebox_t *qrcodebox = compo_create(frm, COMPO_TYPE_QRCODEBOX);

    //背景
    widget_rect_t *bg = widget_rect_create(frm->page);

    //二维码/条形码
    widget_qrcode_t *qrcode = widget_qrcode_create(frm->page, qr_type, max_ch_cnt);
    widget_set_pos(qrcode, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);

    //白色背景区域
    rect_t rect = widget_get_location(qrcode);
    widget_set_location(bg, rect.x, rect.y, rect.wid + QRCODEBOX_PADDING, rect.hei + QRCODEBOX_PADDING);

    qrcodebox->bg = bg;
    qrcodebox->qrcode = qrcode;
    return qrcodebox;
}

/**
 * @brief 设置码的内容
 * @param[in] qrcodebox : 码指针
 * @param[in] str : 字符内容
 **/
void compo_qrcodebox_set(compo_qrcodebox_t *qrcodebox, const char *str)
{
    widget_qrcode_set(qrcodebox->qrcode, str);
}


/**
 * @brief 设置二维码的内容
 * @param[in] qrcodebox : 码指针
 * @param[in] code : 字符内容
 * @param[in] len : 长度
 *  使用的使用要把 USR_MODE8_ONLY 这个宏设置为1；为了让绝大部分不用的客户省flash和ram
 **/
void compo_qrcodebox_2d_set(compo_qrcodebox_t *qrcodebox, const char *code, int len)
{
    widget_qrcode_2d_set(qrcodebox->qrcode, code, len);
}

/**
 * @brief 设置码的内容
 * @param[in] qrcodebox : 码指针
 * @param[in] level: 纠错等级
 **/
void compo_qrcodebox_set_level(compo_qrcodebox_t *qrcodebox, u8 level)
{
    widget_qrcode_set_level(qrcodebox->qrcode, level);
}

/**
 * @brief 设置二维码的编码格式
 * @param[in] qrcodebox : 码指针
 * @param[in] type: 0 code128A, 1 code 128C(不支持字符串，优点比较小)
 **/
void compo_barcode_set_type(compo_qrcodebox_t *qrcodebox, u8 type)
{
    widget_barcode_set_type(qrcodebox->qrcode, type);
}


/**
 * @brief 设置最小单位宽度
 * @param[in] qrcodebox : 码指针
 * @param[in] bit_wid : 设置像素宽度
 **/
void compo_qrcodebox_set_bitwid(compo_qrcodebox_t *qrcodebox, u16 bit_wid)
{
    widget_qrcode_t *qrcode = qrcodebox->qrcode;
    widget_qrcode_set_bitwid(qrcode, bit_wid);

    //更新白色背景区域
    rect_t rect = widget_get_location(qrcode);
    widget_set_location(qrcodebox->bg, rect.x, rect.y, rect.wid + QRCODEBOX_PADDING, rect.hei + QRCODEBOX_PADDING);
}

/**
 * @brief 设置最小单位宽度
 * @param[in] qrcodebox : 码指针
 * @param[in] qr_wid : 设置要显示宽度(高度=宽度)
 **/
void compo_qrcodebox_set_bitwid_by_qrwid(compo_qrcodebox_t *qrcodebox, u16 qr_wid)
{
    widget_qrcode_t *qrcode = qrcodebox->qrcode;
    widget_qrcode_set_bitwid_by_qrwid(qrcode, qr_wid);

    //更新白色背景区域
    rect_t rect = widget_get_location(qrcode);
    widget_set_location(qrcodebox->bg, rect.x, rect.y, rect.wid + QRCODEBOX_PADDING, rect.hei + QRCODEBOX_PADDING);
}
