#include "include.h"
#include "func.h"
#include "func_alipay.h"
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
#include "alipay_ble_common.h"
#include "alipay_transit.h"
#endif

#if SECURITY_PAY_EN

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

enum {
    COMPO_ID_RATE = 1,
    //按钮
    COMPO_ID_BTN_PAYCODE,       //付款码
    COMPO_ID_BTN_SETTING,       //设置
    COMPO_ID_BTN_HELP,          //帮助
    COMPO_ID_BTN_UNBIND,        //解除绑定
    COMPO_ID_BTN_UNBIND_YES,    //解除绑定-确认
    COMPO_ID_BTN_UNBIND_NO,     //解除绑定-取消
    COMPO_ID_BTN_CONFIRM,       //确认
    COMPO_ID_BTN_TRANSITCODE,   //乘车码
    COMPO_ID_BTN_SWITCH_TRANS_CODE, //切换乘车码
    COMPO_ID_BTN_YES,           //确定
    COMPO_ID_BTN_DETAILS,       //详情
    COMPO_ID_BTN_RETRY,         //重试
};

enum {
    ALIPAY_PAGE_LOGO = 1,
    ALIPAY_PAGE_SCAN,
    ALIPAY_PAGE_BINDING,
    ALIPAY_PAGE_BIND_FAIL,
    ALIPAY_PAGE_OPTION_LIST,
    ALIPAY_PAGE_QRCODE,
    ALIPAY_PAGE_BARCODE,
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    ALIPAY_PAGE_LAST_TRANSITCODE,
    ALIPAY_PAGE_TRANSIT_LIST,
    ALIPAY_PAGE_TRANSITCODE_FAIL,
    ALIPAY_PAGE_TRANSITCODE,
#endif
    ALIPAY_PAGE_UNBIND,
    ALIPAY_PAGE_HELP,
    ALIPAY_PAGE_ACTIVITY_HELP,
    ALIPAY_PAGE_UNBIND_CONFIRM,
    ALIPAY_PAGE_UNBIND_FINISH,
};

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
enum {
    RV_NO_ERROR,
    RV_IO_ERROR = 5,
    RV_NETWORK_ERROR = 7,
    RV_BUF_TOO_SHORT = 15,
    RV_SERVER_FAIL_ERROR = 26,
    RV_CARD_DATA_LIMITED = 40,
    RV_UNSUPPORTED_CARD = 43,
    RV_BUSCARDDATA_INVALID = 54,
    RV_COMMON_ERROR = 56,

    //自定义
    RV_CONNECT_DETAILS = 100,
    RV_CONNECT_HELP,

};

enum {
    COMPO_ID_LISTBOX = 1,
};

AT(.alipay_data)
alipay_tansit_CardBaseVO_t card_list[15];


AT(.alipay_data)
compo_listbox_custom_item_t tbl_txt_list[15];

typedef struct {
    u8  yes_blue  		: 1,
        yes_white       : 1,
        details       	: 1,
        retry    		: 1;
} alipay_btn_t;
#endif

typedef struct f_alipay_t_ {
    uint8_t page;
    uint8_t last_page;
    uint8_t logo_display_cnt;
    uint8_t binding_time_cnt;
    uint8_t bind_fail_time_cnt;
    uint8_t bind_success_time_cnt;
    uint8_t bind_code_refresh_time_cnt;
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    compo_listbox_t *listbox;
    uint8_t transitcode_sel;
    uint8_t transitcode[512];
    uint32_t len_transitcode;
    char error_str[256];
    uint8_t type_error;
    compo_listbox_item_t *transit_list;
    char card_title[40];
    int card_num;
#endif
} f_alipay_t;

extern struct Node* csi_head;
extern u32 __alipay_start, __alipay_size;
extern u32 __dram_ali_start, __dram_ali_size;
int initializeFileSystem(void);

//创建支付宝logo窗体
compo_form_t *func_alipay_form_logo_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //新建蓝色背景
    compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, make_color(51, 121, 246));
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_radius(rect, 80);

    //新建图像
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_ALIPAY_LOGO2_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);

    return frm;
}

//创建支付宝窗体
compo_form_t *func_alipay_form_scan_create(void)
{
    int binding_str_len = 256;
    char binding_str[binding_str_len];
    memset(binding_str, 0, binding_str_len);
    u8 res = alipay_get_binding_code(binding_str, &binding_str_len);
    printf("res:%d, binding_str: %s\n", res, binding_str);

    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_ALIPAY]);

    compo_qrcodebox_t *qrcode = compo_qrcodebox_create(frm, 0, 100);
    compo_qrcodebox_set(qrcode, (char *)binding_str);

    func_free(binding_str);
    return frm;
}

//创建支付宝窗体
compo_form_t *func_alipay_form_binding_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_ALIPAY]);

    compo_picturebox_t *pic_bg1 = compo_picturebox_create(frm, UI_BUF_ALIPAY_BG1_BIN);
    compo_picturebox_set_pos(pic_bg1, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_visible(pic_bg1, true);

    compo_arc_t *arc = compo_arc_create(frm);
    compo_setid(arc, COMPO_ID_RATE);
    compo_arc_set_alpha(arc, 0xff, 0);
    compo_arc_set_location(arc, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 180, 180);
    compo_arc_set_width(arc, 7);
    compo_arc_set_rotation(arc, 0);
    compo_arc_set_angles(arc, 0, 3600);
    compo_arc_set_color(arc, COLOR_BLUE, COLOR_WHITE);
    compo_arc_set_value(arc, 250);

    compo_picturebox_t *pic_logo = compo_picturebox_create(frm, UI_BUF_ALIPAY_LOGO_BIN);
    compo_picturebox_set_pos(pic_logo, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_visible(pic_logo, true);

    return frm;
}

//创建支付宝绑定状态窗体
compo_form_t *func_alipay_form_bind_fail_create(void)
{
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;
    if (!f_alipay) {
        return NULL;
    }

    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //新建图像
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_ALIPAY_BIND_STA_BIN);
    compo_picturebox_cut(pic, 0, 2);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 30);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 20, 300, widget_text_get_height());
    compo_textbox_set(txt, i18n[STR_BIND_FAILED]);

    return frm;
}

//创建支付宝选项列表窗体
compo_form_t *func_alipay_form_option_list_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

//带乘车码
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    //新建按钮
    compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BG_BIN);
    compo_setid(btn, COMPO_ID_BTN_PAYCODE);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 100);
    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BG_BIN);
    compo_setid(btn, COMPO_ID_BTN_TRANSITCODE);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 190);
    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BG_BIN);
    compo_setid(btn, COMPO_ID_BTN_SETTING);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 280);
    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BG_BIN);
    compo_setid(btn, COMPO_ID_BTN_HELP);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 370);

    //新建图像
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_ALIPAY_ALI_OPTION_BIN);
    compo_picturebox_cut(pic, 0, 3);
    compo_picturebox_set_pos(pic, 40, 100);
    pic = compo_picturebox_create(frm, UI_BUF_ALIPAY_ALI_OPTION_BIN);
    compo_picturebox_cut(pic, 0, 3);
    compo_picturebox_set_pos(pic, 40, 190);
    pic = compo_picturebox_create(frm, UI_BUF_ALIPAY_ALI_OPTION_BIN);
    compo_picturebox_cut(pic, 1, 3);
    compo_picturebox_set_pos(pic, 40, 280);
    pic = compo_picturebox_create(frm, UI_BUF_ALIPAY_ALI_OPTION_BIN);
    compo_picturebox_cut(pic, 2, 3);
    compo_picturebox_set_pos(pic, 40, 370);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_location(txt, 70, 100 - widget_text_get_height() / 2, 220, 70);
    compo_textbox_set(txt, i18n[STR_PAYCODE]);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_location(txt, 70, 190 - widget_text_get_height() / 2, 220, 70);
    compo_textbox_set(txt, i18n[STR_TRANSITCODE]);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_location(txt, 70, 280- widget_text_get_height() / 2, 220, 70);
    compo_textbox_set(txt, i18n[STR_SETTING]);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_location(txt, 70, 370 - widget_text_get_height() / 2, 220, 70);
    compo_textbox_set(txt, i18n[STR_HELP]);
#else

    //新建按钮
    compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BG_BIN);
    compo_setid(btn, COMPO_ID_BTN_PAYCODE);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 100);
    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BG_BIN);
    compo_setid(btn, COMPO_ID_BTN_SETTING);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 190);
    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BG_BIN);
    compo_setid(btn, COMPO_ID_BTN_HELP);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 280);

    //新建图像
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_ALIPAY_ALI_OPTION_BIN);
    compo_picturebox_cut(pic, 0, 3);
    compo_picturebox_set_pos(pic, 40, 100);
    pic = compo_picturebox_create(frm, UI_BUF_ALIPAY_ALI_OPTION_BIN);
    compo_picturebox_cut(pic, 1, 3);
    compo_picturebox_set_pos(pic, 40, 190);
    pic = compo_picturebox_create(frm, UI_BUF_ALIPAY_ALI_OPTION_BIN);
    compo_picturebox_cut(pic, 2, 3);
    compo_picturebox_set_pos(pic, 40, 280);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_location(txt, 70, 100 - widget_text_get_height() / 2, 220, 70);
    compo_textbox_set(txt, i18n[STR_PAYCODE]);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_location(txt, 70, 190 - widget_text_get_height() / 2, 220, 70);
    compo_textbox_set(txt, i18n[STR_SETTING]);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_location(txt, 70, 280 - widget_text_get_height() / 2, 220, 70);
    compo_textbox_set(txt, i18n[STR_HELP]);
#endif

    return frm;
}

//创建支付宝二维码窗体
compo_form_t *func_alipay_form_qrcode_create(void)
{

    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //新建蓝色背景
    compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, make_color(51, 121, 246));
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_radius(rect, 80);

    //页码示意条
    rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, COLOR_WHITE);
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X - 8, 15, 10, 10);
    widget_rect_set_radius(rect->rect, 5);
    rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, COLOR_GRAY);
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X + 8, 15, 10, 10);
    widget_rect_set_radius(rect->rect, 5);

    u32 paycode_len = 20;
    u8 *paycode = func_zalloc(paycode_len);
    memset(paycode, 0, paycode_len);
    u8 res = alipay_get_paycode(paycode, &paycode_len);
    u8 retry_cnt = 0;
    while (res != ALIPAY_RV_OK){
        res = alipay_get_paycode(paycode, &paycode_len);
        retry_cnt++;
        if (retry_cnt > ALIPAY_API_RETRY_CNT){
            retry_cnt = 0;
            printf("alipay_get_paycode err\n");
            break;
        }
    }
    printf("res:%d, paycode: %s\n", res, paycode);

    //新建二维码
    compo_qrcodebox_t *qrbox = compo_qrcodebox_create(frm, QRCODE_TYPE_2D, 64);
    compo_qrcodebox_set(qrbox, (char *)paycode);
    compo_qrcodebox_set_bitwid_by_qrwid(qrbox, 200);

    //新建图标
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_ALIPAY_LOGO_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_size(pic, 45, 45);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, 50, 300, widget_text_get_height());
    compo_textbox_set(txt, i18n[STR_PAYCODE]);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 50, 300, widget_text_get_height());
    compo_textbox_set(txt, i18n[STR_SWITCH_BARCODE]);

    func_free(paycode);
    return frm;
}


#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
void func_alipay_transitcode_fail_click_handle(u8 last_page, u8 *type_err, u8 id)
{
    u8 next_page = 0;
    switch (id) {
    case COMPO_ID_BTN_YES:
        if (last_page <= ALIPAY_PAGE_TRANSIT_LIST) {
            next_page = ALIPAY_PAGE_OPTION_LIST;
        } else {
            next_page = ALIPAY_PAGE_TRANSIT_LIST;
        }
        printf("COMPO_ID_BTN_YES PAGE:%d\n", last_page);
        break;

    case COMPO_ID_BTN_DETAILS:
        printf("COMPO_ID_BTN_DETAILS:%d\n", *type_err);
        if (*type_err == RV_BUSCARDDATA_INVALID || *type_err == RV_BUSCARDDATA_INVALID || *type_err == RV_NETWORK_ERROR) {
            *type_err = RV_CONNECT_DETAILS;
            next_page = ALIPAY_PAGE_TRANSITCODE_FAIL;
        } else if (*type_err == RV_CONNECT_DETAILS) {
            *type_err = RV_CONNECT_HELP;
            next_page = ALIPAY_PAGE_HELP;
        }


        break;

    case COMPO_ID_BTN_RETRY:
        printf("COMPO_ID_BTN_RETRY\n");
        next_page = ALIPAY_PAGE_LAST_TRANSITCODE;
        break;

    default:

        break;
    }
    func_alipay_form_create_by_page(next_page);
}

void func_alipay_transicode_get_err_str(char *error_str_buf, u8 type_err, u8 page, alipay_btn_t *btn)
{
    u16 err_str_id = STR_TRANSITCODE_UNKNOW_ERROR;
    switch (type_err) {
    case RV_BUSCARDDATA_INVALID:
    case RV_NETWORK_ERROR:
        err_str_id = STR_TRANSITCODE_NET_ERROR;
        btn->yes_white = true;
        btn->details = true;
        break;

    case RV_SERVER_FAIL_ERROR:
        if (page <= ALIPAY_PAGE_TRANSIT_LIST) {
            err_str_id = STR_TRANSITCODE_LIMIT_ERROR;
        } else {
            err_str_id = STR_TRANSITCODE_UNSUP_ERROR;
        }
        btn->yes_blue = true;
        break;

    case RV_BUF_TOO_SHORT:
        if (page <= ALIPAY_PAGE_TRANSIT_LIST) {
            err_str_id = STR_TRANSITCODE_UNKNOW_ERROR;

        } else {
            err_str_id = STR_TRANSITCODE_UNSUP_ERROR;
        }
        btn->yes_blue = true;
        break;

    case RV_UNSUPPORTED_CARD:
        err_str_id = STR_TRANSITCODE_UNSUP_ERROR;
        btn->yes_blue = true;
        break;

    case RV_CARD_DATA_LIMITED:
        err_str_id = STR_TRANSITCODE_SECURITY_ERROR;
        btn->yes_blue = true;
        break;

    case RV_CONNECT_DETAILS:
        err_str_id = STR_TRANSITCODE_NET_DETAILS_ERROR;
        btn->retry = true;
        btn->details = true;
        break;

    default:
        if (page <= ALIPAY_PAGE_TRANSIT_LIST) {
            err_str_id = STR_TRANSITCODE_UNKNOW_ERROR;
        }
        btn->yes_white = true;
        break;
    }


    strcpy(error_str_buf, i18n[err_str_id]);

    //拼接错误码
    if (err_str_id == STR_TRANSITCODE_UNKNOW_ERROR) {
        char *pos = strstr(error_str_buf, "00");
        u8 ofs = pos - error_str_buf;
        char *res_buf = ab_malloc(256);
        strncpy(res_buf, error_str_buf, ofs);
        res_buf[ofs] = '\0';
        char type_err_str[4];
        sprintf(type_err_str, "%02d", type_err);
        strcat(res_buf, type_err_str);
        strcat(res_buf, pos + 2);
        strcpy(error_str_buf, res_buf);
        ab_free(res_buf);
    }

    printf("%s:\n%s\n", __func__, error_str_buf);

}

//创建支付宝绑定状态窗体
compo_form_t *func_alipay_form_transitcode_fail_create(void)
{
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;
    if (!f_alipay) {
        return NULL;
    }

    alipay_btn_t btn;
    bool flag_pic = (f_alipay->type_error != RV_CONNECT_DETAILS);
    memset(&btn, 0, sizeof(alipay_btn_t));
    compo_form_t *frm = compo_form_create(true);
    if (flag_pic) {
        //新建图像
        compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_ALIPAY_BIND_STA_BIN);
        compo_picturebox_cut(pic, 0, 2);
        compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 110);
        compo_picturebox_set_size(pic, 80, 80);
    }

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 128);
    compo_textbox_set_location(txt, 10, 160 -  !flag_pic * 80, 300, 240);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_multiline(txt, true);

    func_alipay_transicode_get_err_str(f_alipay->error_str, f_alipay->type_error, f_alipay->page, &btn);
    compo_textbox_set(txt, f_alipay->error_str);

    printf("btn [%d, %d, %d, %d]\n", btn.yes_blue, btn.yes_white, btn.details, btn.retry);

    u8 btn_num = btn.yes_blue + btn.yes_white + btn.details + btn.retry;
    compo_button_t *btn_yes = NULL;
    compo_button_t *btn_details = NULL;
    compo_button_t *btn_retry = NULL;

    //新建蓝色背景
    if (btn.yes_blue || btn.details) {
        compo_button_t *tmp_btn = compo_button_create(frm);
        if (btn.yes_blue) {
            btn_yes = tmp_btn;
        } else {
            btn_details = tmp_btn;
        }

        compo_button_set_location(tmp_btn, GUI_SCREEN_CENTER_X + btn.details * 80, GUI_SCREEN_CENTER_Y + 120, 120 + (60 * (btn_num == 1)), 60);
        compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
        compo_shape_set_color(rect, make_color(51, 121, 246));
        compo_shape_set_location(rect, GUI_SCREEN_CENTER_X + btn.details * 80, GUI_SCREEN_CENTER_Y + 120, 120 + (60 * (btn_num == 1)), 60);
        compo_shape_set_radius(rect, 80);
    }

    if (btn.yes_white || btn.retry) {
        compo_button_t *tmp_btn = compo_button_create(frm);
        if (btn.yes_white) {
            btn_yes = tmp_btn;
        } else {
            btn_retry = tmp_btn;
        }

        compo_button_set_location(tmp_btn, GUI_SCREEN_CENTER_X - 80, GUI_SCREEN_CENTER_Y + 120 * (btn_num > 1), 120, 60);
        compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
        compo_shape_set_color(rect, COLOR_WHITE);
        compo_shape_set_location(rect, GUI_SCREEN_CENTER_X - 80, GUI_SCREEN_CENTER_Y + 120 * (btn_num > 1), 120, 60);
        compo_shape_set_radius(rect, 80);
    }

    if (btn.yes_blue || btn.yes_white) {
        compo_textbox_t *txt = compo_textbox_create(frm, 16);
        compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X - 80 * (btn_num > 1), GUI_SCREEN_CENTER_Y + 120, 120, 60);
        if (btn.yes_white) {
            compo_textbox_set_forecolor(txt, COLOR_BLACK);
        }
        compo_textbox_set(txt, i18n[STR_TRANSITCODE_BTN_YES]);
        if (btn_yes) {
            compo_setid(btn_yes, COMPO_ID_BTN_YES);
        }
    }

    if (btn.retry) {
        compo_textbox_t *txt = compo_textbox_create(frm, 16);
        compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X - 80, GUI_SCREEN_CENTER_Y + 120, 120, 60);
        compo_textbox_set_forecolor(txt, COLOR_BLACK);
        compo_textbox_set(txt, i18n[STR_TRANSITCODE_BTN_RETRY]);
        if (btn_retry) {
            compo_setid(btn_retry, COMPO_ID_BTN_RETRY);
        }
    }

    if (btn.details) {
        compo_textbox_t *txt = compo_textbox_create(frm, 16);
        compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X + 80, GUI_SCREEN_CENTER_Y + 120, 120, 60);
        compo_textbox_set(txt, i18n[STR_TRANSITCODE_BTN_DETAILS]);
        if (btn_details) {
            compo_setid(btn_details, COMPO_ID_BTN_DETAILS);
        }
    }

    f_alipay->type_error = RV_NO_ERROR;

    return frm;
}

compo_form_t *transit_list_create(f_alipay_t *f_alipay, compo_listbox_custom_item_t *tbl_txt_list, int card_num)
{
    compo_form_t *frm = compo_form_create(false);
    if(f_alipay->transit_list == NULL) {
        f_alipay->transit_list = func_zalloc(sizeof(compo_listbox_item_t)*card_num);
    }

    f_alipay->listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_MENU_NORMAL);
    compo_listbox_set(f_alipay->listbox, f_alipay->transit_list, card_num);
    compo_setid(f_alipay->listbox, COMPO_ID_LISTBOX);
    compo_listbox_set_bgimg(f_alipay->listbox, UI_BUF_COMMON_BG_BIN);
    compo_listbox_set_text_modify(f_alipay->listbox, tbl_txt_list);
    compo_listbox_set_focus_byidx(f_alipay->listbox, 1);
    compo_listbox_update(f_alipay->listbox);


    compo_listbox_t *listbox = f_alipay->listbox;
    printf("listbox->mcb:%x\n", listbox->mcb);
    if (listbox->mcb == NULL) {
        listbox->mcb = func_zalloc(sizeof(compo_listbox_move_cb_t));        //建立移动控制块，退出时需要释放
    }
    compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, card_num - 2));

    return frm;
}

bool func_alipay_transit_list_init(u32 *len_card_list, u32 *card_num, bool flag_force_update)
{
    int32_t res = RV_IO_ERROR;
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;

    if (flag_force_update == false) {
        res = alipay_transit_get_card_list_offline(card_list, len_card_list, card_num);
        if (res == RV_OK) {
            for(u8 i=0;i<*card_num;i++) {
                printf("offline [%d]---> cardNo:%d, cardType:%d, title:%s\n", i, card_list[i].cardNo, card_list[i].cardType, card_list[i].title);
                strcpy(tbl_txt_list[i].str_txt, card_list[i].title);
            }
        }
        printf("res2:%d, card_num:%d\n", res, *card_num);
    }

    if (res != RV_OK) {
        //读取失败, 本地不存在，通过联网接口获取
        if (res == RV_IO_ERROR || *card_num > 100) {
            if (!ble_is_connect()) {
                f_alipay->transitcode_sel = 0;
                f_alipay->type_error = RV_NETWORK_ERROR;
                return false;
            }

            res = alipay_transit_get_card_list_online(card_list, len_card_list, card_num);
            printf("res3:%d, %s\n", res, f_alipay->error_str);
            if (res == RV_OK) {
                if (*card_num == 0) {
                    //无乘车卡, 跳转到开通乘车码界面
                    return true;
                } else {
                    for(u8 i=0;i<*card_num;i++) {
                        printf("online [%d]---> cardNo:%d, cardType:%d, title:%s\n", i, card_list[i].cardNo, card_list[i].cardType, card_list[i].title);
                        strcpy(tbl_txt_list[i].str_txt, card_list[i].title);
                    }
                }
            }
        } else {
            f_alipay->type_error = res;
            return false;
        }
    }


    strcpy(tbl_txt_list[*card_num].str_txt, i18n[STR_TRANSITCODE_UPDATE]);
    *card_num += 1;
    f_alipay->card_num = *card_num;
    return true;
}

//创建乘车码开通二维码窗体
compo_form_t *func_alipay_transicode_form_activity_help_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //新建蓝色背景
    compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, make_color(51, 121, 246));
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_radius(rect, 80);


    //新建二维码
    compo_qrcodebox_t *qrbox = compo_qrcodebox_create(frm, QRCODE_TYPE_2D, 200);
    compo_qrcodebox_set(qrbox, "alipays://platformapi/startapp?appId=20002047&scene=bus");
    compo_qrcodebox_set_bitwid_by_qrwid(qrbox, 200);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, 45, 300, widget_text_get_height());
    compo_textbox_set(txt, i18n[STR_TRANSITCODE_NOT_ACT]);

    //新建文本
    compo_textbox_t *txt_switch = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt_switch, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 40, 300, widget_text_get_height());
    compo_textbox_set(txt_switch, i18n[STR_TRANSITCODE_SCAN_TO_ACT]);

    return frm;
}

//创建乘车列表窗体
compo_form_t *func_alipay_form_transit_list_create(u8 *page_sta)
{
    u32 len_card_list = 2048;
    u32 card_num = 2048;
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;

    printf("transitcode_sel:%d, card_num:%d\n", f_alipay->transitcode_sel, f_alipay->card_num);
    bool res = func_alipay_transit_list_init(&len_card_list, &card_num, (f_alipay->card_num != 0) && f_alipay->transitcode_sel == (f_alipay->card_num - 1));
    if (!res) {
        *page_sta = ALIPAY_PAGE_TRANSITCODE_FAIL;
        return func_alipay_form_transitcode_fail_create();
    }

    if (card_num > 0) {
        return transit_list_create(f_alipay, tbl_txt_list, card_num);
    } else {
        return func_alipay_transicode_form_activity_help_create();
    }

}


//创建默认卡信息窗体
compo_form_t *func_alipay_form_last_transitcode_create(u8 *page_sta)
{
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;

    //获取最近一次使用的乘车码码值，断电后会清空
    int32_t res = alipay_transit_get_the_last_transitCode(f_alipay->card_title, f_alipay->transitcode, &
                                                          f_alipay->len_transitcode, f_alipay->error_str, 256);

    printf("%s res:%d\n", __func__, res);

    if (res == RV_OK) {     //成功，显示默认乘车码
        //新建窗体
        compo_form_t *frm = compo_form_create(false);       //菜单一般创建在底层
        printf("Get transitcode success, len:%d\n", f_alipay->len_transitcode);
        print_r(f_alipay->transitcode, f_alipay->len_transitcode);
        func_alipay_form_create_by_page(ALIPAY_PAGE_TRANSITCODE);
        return frm;
    } else {
        *page_sta = ALIPAY_PAGE_TRANSIT_LIST;
        return func_alipay_form_transit_list_create(page_sta);
    }

}

//创建乘车码二维码窗体
compo_form_t *func_alipay_form_transitcode_create(void)
{
//    //新建窗体
    compo_form_t *frm = compo_form_create(true);
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;

    //新建蓝色背景
    compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, make_color(51, 121, 246));
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_radius(rect, 80);

    //新建二维码
    compo_qrcodebox_t *qrbox = compo_qrcodebox_create(frm, QRCODE_TYPE_2D, f_alipay->len_transitcode+1);
    compo_qrcodebox_set_level(qrbox, QR_LEVEL_L);
    compo_qrcodebox_2d_set(qrbox, (const char *)f_alipay->transitcode, f_alipay->len_transitcode);
    compo_qrcodebox_set_bitwid_by_qrwid(qrbox, 180);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, 30, 300, widget_text_get_height());
    compo_textbox_set(txt, f_alipay->card_title); //card_list[i].title
    txt = compo_textbox_create(frm, 50);
    compo_textbox_set_multiline(txt, true);

    //新建文本
    compo_textbox_t *txt_switch = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt_switch, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 20, 200, 50);
    compo_textbox_set(txt_switch, i18n[STR_TRANSITCODE_SWITH]);

    compo_button_t *btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_SWITCH_TRANS_CODE);
    compo_button_set_location(btn, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 40, 200, 50);

    printf("Get transitcode len:%d\n", f_alipay->len_transitcode);
    return frm;
}

#endif

//创建支付宝条形码窗体
compo_form_t *func_alipay_form_barcode_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //新建蓝色背景
    compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, make_color(51, 121, 246));
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_radius(rect, 80);

    //页码示意条
    rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, COLOR_GRAY);
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X - 8, 15, 10, 10);
    widget_rect_set_radius(rect->rect, 5);
    rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, COLOR_WHITE);
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X + 8, 15, 10, 10);
    widget_rect_set_radius(rect->rect, 5);

    u32 paycode_len = 20;
    u8 *paycode = func_zalloc(paycode_len);
    memset(paycode, 0, paycode_len);
    u8 res = alipay_get_paycode(paycode, &paycode_len);
    u8 retry_cnt = 0;
    while (res != ALIPAY_RV_OK){
        res = alipay_get_paycode(paycode, &paycode_len);
        retry_cnt++;
        if (retry_cnt > ALIPAY_API_RETRY_CNT){
            retry_cnt = 0;
            printf("alipay_get_paycode err\n");
            break;
        }
    }
    printf("res:%d, paycode: %s\n", res, paycode);

    //新建二维码
    compo_qrcodebox_t *qrbox = compo_qrcodebox_create(frm, QRCODE_TYPE_HORIZONTAL, 64);
    compo_qrcodebox_set(qrbox, (char *)paycode);
    compo_qrcodebox_set_bitwid(qrbox, 10);
    compo_qrcodebox_set_bitwid_by_qrwid(qrbox, 200);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, 50, 300, widget_text_get_height());
    compo_textbox_set(txt, i18n[STR_PAYCODE]);
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 80, 300, widget_text_get_height());
    compo_textbox_set(txt, (char *)paycode);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 50, 300, widget_text_get_height());
    compo_textbox_set(txt, i18n[STR_SWITCH_QRCODE]);

    func_free(paycode);
    return frm;
}

//创建支付宝解绑（设置）窗体
compo_form_t *func_alipay_form_unbind_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //新建按钮蓝色背景
    compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, make_color(51, 121, 246));
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 60, 150, 60);
    widget_rect_set_radius(rect->rect, 30);

    //新建按钮
    compo_button_t *btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_UNBIND);
    compo_button_set_location(btn, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 60, 150, 60);

    u8 res;
    u32 id_len = 128;
    u32 name_len = 128;
    u8 *id = func_zalloc(id_len);
    u8 *name = func_zalloc(name_len);
    memset(id, 0, id_len);
    memset(name, 0, name_len);
    res = alipay_get_logon_ID(id, &id_len);
    u8 retry_cnt = 0;
    while (res != ALIPAY_RV_OK){
        res = alipay_get_logon_ID(id, &id_len);
        retry_cnt++;
        if (retry_cnt > ALIPAY_API_RETRY_CNT){
            retry_cnt = 0;
            printf("alipay_get_logon_ID err\n");
            break;
        }
    }
    printf("alipay_get_logon_ID: %d: %s\n", res, id);
    res = alipay_get_nick_name(name, &name_len);
    retry_cnt = 0;
    while (res != ALIPAY_RV_OK){
        res = alipay_get_nick_name(name, &name_len);
        retry_cnt++;
        if (retry_cnt > ALIPAY_API_RETRY_CNT){
            retry_cnt = 0;
            printf("alipay_get_nick_name err\n");
            break;
        }
    }
    printf("alipay_get_logon_name: %d: %s\n", res, name);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, 50, 300, widget_text_get_height());
    compo_textbox_set(txt, i18n[STR_SETTING]);
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 20, 300, widget_text_get_height());
    compo_textbox_set(txt, (char *)id);
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 20, 300, widget_text_get_height());
    compo_textbox_set_forecolor(txt, COLOR_GRAY);
    compo_textbox_set(txt, (char *)name);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 60, 130, widget_text_get_height());
    compo_textbox_set(txt, i18n[STR_UNBIND]);

    func_free(id);
    func_free(name);

    return frm;
}

//创建支付宝二维码窗体
compo_form_t *func_alipay_form_help_create(void)
{

    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //新建蓝色背景
    compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, make_color(51, 121, 246));
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_radius(rect, 80);

    u32 len_aid_code = 256;
    char *aid_code = func_zalloc(len_aid_code);
    memset(aid_code, 0, len_aid_code);
    u8 res = alipay_get_aid_code(aid_code, &len_aid_code, true);
    u8 retry_cnt = 0;
    while (res != ALIPAY_RV_OK){
        res = alipay_get_aid_code(aid_code, &len_aid_code, true);
        retry_cnt++;
        if (retry_cnt > ALIPAY_API_RETRY_CNT){
            retry_cnt = 0;
            printf("alipay_get_paycode err\n");
            break;
        }
    }
    printf("res:%d, aid_code: %s, len_aid_code: %d\n", res, aid_code, len_aid_code);

    //新建二维码
    compo_qrcodebox_t *qrbox = compo_qrcodebox_create(frm, QRCODE_TYPE_2D, 200);
    compo_qrcodebox_set(qrbox, (char *)aid_code);
    compo_qrcodebox_set_bitwid_by_qrwid(qrbox, 200);

    //新建图标
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_ALIPAY_LOGO_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_size(pic, 45, 45);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, 40, 300, widget_text_get_height());
    compo_textbox_set(txt, i18n[STR_HELP_TILE]);

    func_free(aid_code);
    return frm;
}


//创建支付宝确认解绑窗体
compo_form_t *func_alipay_form_unbind_confirm_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //新建按钮背景
    compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE); //白
    compo_shape_set_color(rect, COLOR_WHITE);
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X - 60, GUI_SCREEN_HEIGHT - 60, 100, 60);
    widget_rect_set_radius(rect->rect, 30);
    rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE); //蓝
    compo_shape_set_color(rect, make_color(51, 121, 246));
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X + 60, GUI_SCREEN_HEIGHT - 60, 100, 60);
    widget_rect_set_radius(rect->rect, 30);

    //新建按钮
    compo_button_t *btn = compo_button_create(frm);  //确认
    compo_setid(btn, COMPO_ID_BTN_UNBIND_YES);
    compo_button_set_location(btn, GUI_SCREEN_CENTER_X - 60, GUI_SCREEN_HEIGHT - 60, 100, 60);
    btn = compo_button_create(frm);  //取消
    compo_setid(btn, COMPO_ID_BTN_UNBIND_NO);
    compo_button_set_location(btn, GUI_SCREEN_CENTER_X + 60, GUI_SCREEN_HEIGHT - 60, 100, 60);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, 50, 300, widget_text_get_height());
    compo_textbox_set(txt, i18n[STR_UNBIND]);
    txt = compo_textbox_create(frm, 64);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 300, 150);
    compo_textbox_set_multiline(txt, true);
    compo_textbox_set(txt, i18n[STR_ALIPAY_UNBIND_TIP]);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X - 60, GUI_SCREEN_HEIGHT - 60, 80, widget_text_get_height());
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
    compo_textbox_set(txt, i18n[STR_CONFIMR]);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X + 60, GUI_SCREEN_HEIGHT - 60, 80, widget_text_get_height());
    compo_textbox_set(txt, i18n[STR_CANCEL]);

    return frm;
}

//创建支付宝解绑完成窗体
compo_form_t *func_alipay_form_unbind_finish_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //新建按钮蓝色背景
    compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, make_color(51, 121, 246));
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 60, 150, 60);
    widget_rect_set_radius(rect->rect, 30);

    //新建按钮
    compo_button_t *btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_CONFIRM);
    compo_button_set_location(btn, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 60, 150, 60);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 64);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 300, 150);
    compo_textbox_set_multiline(txt, true);
    compo_textbox_set(txt, i18n[STR_ALIPAY_UNBIND_TIP2]);
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 60, 130, widget_text_get_height());
    compo_textbox_set(txt, i18n[STR_CONFIMR]);

    return frm;
}

AT(.text.func.alipay)
void func_alipay_prepare(void)
{
    printf("__alipay_start: 0x%x, size: 0x%x\n", &__alipay_start, &__alipay_size);
    memset(&__alipay_start, 0, (u32)&__alipay_size);
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    memset(&__dram_ali_start, 0, (u32)&__dram_ali_size);
	printf("__dram_ali_start: 0x%x, size: 0x%x\n", &__dram_ali_start, &__dram_ali_size);
#endif

#if (SECURITY_PAY_VENDOR == SECURITY_VENDOR_HS)
    HS_IIC_Init();
#endif
#if (SECURITY_PAY_VENDOR == SECURITY_VENDOR_HED)
    uint8_t *alipay_com_outbuf = func_zalloc(200);
    uint32_t alipay_outlen =0;
    HED_IIC_Init();
    api_connect(alipay_com_outbuf, &alipay_outlen);
    printf("api_connect atr:\n");
    print_r(alipay_com_outbuf, alipay_outlen);
    func_free(alipay_com_outbuf);
#endif
    u8 sta = alipay_pre_init();
    printf("alipay_pre_init: %d\n", sta);
    if (sta != ALIPAY_RV_OK) {
        func_back_to();
    }

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    alipay_init_transit_module();
    initializeFileSystem();
#endif

    sta = alipay_get_binding_status();
    printf("alipay_get_binding_status: %d\n", sta);
    if (sta == true) {
        func_alipay_form_create_by_page(ALIPAY_PAGE_OPTION_LIST);
    }else {
        func_alipay_form_create_by_page(ALIPAY_PAGE_SCAN);
    }

    printf("func_alipay_prepare finish\n");
}

void func_alipay_init(void)
{
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;
    f_alipay->page = ALIPAY_PAGE_LOGO;
}

//支付宝功能事件处理
static void func_alipay_process(void)
{
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;
    if (f_alipay->page == ALIPAY_PAGE_TRANSIT_LIST) {
        compo_listbox_move(f_alipay->listbox);
    }
#endif

    func_process();
}

//创建支付宝窗体（默认为logo页面）
compo_form_t *func_alipay_form_create(void)
{
    return func_alipay_form_logo_create();
}


//创建指定页面的窗体（销毁&创建）
void func_alipay_form_create_by_page(u8 page_sta)
{
    printf("%s: %d\n", __func__, page_sta);
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;
    u8 next_page_sta = page_sta;
    compo_form_t *frm = func_cb.frm_main;

    if (frm != NULL) {
        compo_form_destroy(frm);
        frm = NULL;
    }

    switch (next_page_sta) {
    case ALIPAY_PAGE_LOGO:
        frm = func_alipay_form_logo_create();
        break;

    case ALIPAY_PAGE_SCAN:
        frm = func_alipay_form_scan_create();
        break;

    case ALIPAY_PAGE_BINDING:
        frm = func_alipay_form_binding_create();
        break;

    case ALIPAY_PAGE_BIND_FAIL:
        frm = func_alipay_form_bind_fail_create();
        break;

    case ALIPAY_PAGE_OPTION_LIST:
        frm = func_alipay_form_option_list_create();
        break;

    case ALIPAY_PAGE_QRCODE:
        frm = func_alipay_form_qrcode_create();
        break;

    case ALIPAY_PAGE_BARCODE:
        frm = func_alipay_form_barcode_create();
        break;

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    case ALIPAY_PAGE_LAST_TRANSITCODE:
        frm = func_alipay_form_last_transitcode_create(&next_page_sta);
        break;

    case ALIPAY_PAGE_TRANSIT_LIST:
        frm = func_alipay_form_transit_list_create(&next_page_sta);
        break;

    case ALIPAY_PAGE_TRANSITCODE_FAIL:
        frm = func_alipay_form_transitcode_fail_create();
        break;

    case ALIPAY_PAGE_TRANSITCODE:
        frm = func_alipay_form_transitcode_create();
        break;
#endif

    case ALIPAY_PAGE_UNBIND:
        frm = func_alipay_form_unbind_create();
        break;

    case ALIPAY_PAGE_HELP:
        frm = func_alipay_form_help_create();
        break;

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    case ALIPAY_PAGE_ACTIVITY_HELP:
        frm = func_alipay_transicode_form_activity_help_create();
        break;
#endif // SECURITY_PAY_EN

    case ALIPAY_PAGE_UNBIND_CONFIRM:
        frm = func_alipay_form_unbind_confirm_create();
        break;

    case ALIPAY_PAGE_UNBIND_FINISH:
        frm = func_alipay_form_unbind_finish_create();
        break;
    }

    if (frm) {
        f_alipay->last_page = f_alipay->page;
        f_alipay->page = next_page_sta;
        printf(">>>next_page_sta[%d]\n", next_page_sta);
    }
}


//检测绑定状态改变之后跳转界面
void alipay_binding_detect(void)
{
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;
    if (f_alipay->page == ALIPAY_PAGE_LOGO){
        f_alipay->logo_display_cnt++;
        printf("logo_display_cnt: %d\n", f_alipay->logo_display_cnt);
        if (f_alipay->logo_display_cnt == ALIPAY_LOGO_TIMEOUT){
            f_alipay->logo_display_cnt = 0;
            func_alipay_prepare();
        }

    }
    else if (f_alipay->page == ALIPAY_PAGE_SCAN || f_alipay->page == ALIPAY_PAGE_BINDING){
        u32 ret;
        binding_status_e sta = ALIPAY_STATUS_UNKNOWN;
        ret = alipay_query_binding_result(&sta);
        printf("ret: %d, sta: %d, page: %d\n", ret, sta, f_alipay->page);

        if (sta == ALIPAY_STATUS_START_BINDING && f_alipay->page == ALIPAY_PAGE_SCAN) {
            func_alipay_form_create_by_page(ALIPAY_PAGE_BINDING);
        }
        compo_arc_t *arc = compo_getobj_byid(COMPO_ID_RATE);
        if (f_alipay->page == ALIPAY_PAGE_BINDING) {

            switch (sta) {
                case ALIPAY_STATUS_BINDING_OK:
                    compo_arc_set_value(arc, 1000);
                    break;
                case ALIPAY_STATUS_BINDING_FAIL:
                    break;
                default :
                    break;
            }
        }
        //binding success go to option
        if (sta == ALIPAY_STATUS_BINDING_OK && f_alipay->page == ALIPAY_PAGE_BINDING) {
            f_alipay->bind_success_time_cnt++;
            printf("bind_success_time_cnt: %d\n", f_alipay->bind_success_time_cnt);
            if (f_alipay->bind_success_time_cnt == ALIPAY_SUCCESS_TIMEOUT){
                f_alipay->bind_success_time_cnt = 0;
                func_alipay_form_create_by_page(ALIPAY_PAGE_OPTION_LIST);
            }
        }

        //binding timeout go to logo
        if (sta != ALIPAY_STATUS_BINDING_OK && f_alipay->page == ALIPAY_PAGE_BINDING) {
            f_alipay->binding_time_cnt++;
            if (f_alipay->binding_time_cnt == ALIPAY_BIND_TIMEOUT){
                f_alipay->binding_time_cnt = 0;
                func_alipay_form_create_by_page(ALIPAY_PAGE_BIND_FAIL);
            }
        }

    }

    //bind failed go to logo
    else if (f_alipay->page == ALIPAY_PAGE_BIND_FAIL) {
            f_alipay->bind_fail_time_cnt++;
            printf("bind_fail_time_cnt: %d\n", f_alipay->bind_fail_time_cnt);
            if (f_alipay->bind_fail_time_cnt == ALIPAY_FAIL_TIMEOUT){
                f_alipay->bind_fail_time_cnt = 0;
//                alipay_bind_env_destroy();
                printf("alipay_bind_env_destroy\n");
                printf("%s\n", __func__);
                freeList(&csi_head);
                func_alipay_prepare();
            }
    }

        //刷新支付码 ALIPAY_CODE_REFRESH_TIMEOUT 秒 刷新一次
    else if (f_alipay->page == ALIPAY_PAGE_QRCODE || f_alipay->page == ALIPAY_PAGE_BARCODE) {
        f_alipay->bind_code_refresh_time_cnt++;
        //printf("bind_code_refresh_time_cnt: %d\n", f_alipay->bind_code_refresh_time_cnt);

        if (f_alipay->bind_code_refresh_time_cnt >= ALIPAY_CODE_REFRESH_TIMEOUT) {
            printf("===>> qr code refresh\n");
            f_alipay->bind_code_refresh_time_cnt = 0;
            func_alipay_form_create_by_page(f_alipay->page);
        }
    }

}

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
void func_alipay_transitcode_list_click(void)
{
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_alipay->listbox;

    f_alipay->transitcode_sel = compo_listbox_select(listbox, ctp_get_sxy());
    func_free(listbox->mcb);
    func_free(f_alipay->transit_list);
    f_alipay->transit_list = NULL;
    listbox->mcb = NULL;

//    if (f_alipay->transitcode_sel < 0 || f_alipay->transitcode_sel >= MENU_LIST_CNT) {
//        return;
//    }

//    char error_message[256];

    printf("select card %d, card_num:%d\n", f_alipay->transitcode_sel, f_alipay->card_num);

    if (f_alipay->transitcode_sel == (f_alipay->card_num - 1)) {
        func_alipay_form_create_by_page(ALIPAY_PAGE_TRANSIT_LIST);
        return;
    }

    alipay_transit_card_status_t card_status;
    alipay_transit_check_card_status(card_list[f_alipay->transitcode_sel].cardNo, card_list[f_alipay->transitcode_sel].cardType, &card_status);
    printf("card 3 status--> is_exists:%d, expire_timestamp:%d, remain_use_count:%d\n", card_status.is_exists, card_status.expire_timestamp, card_status.remain_use_count);
    int32_t res = RV_OK;
    //判断本地是否有缓存、是否还有次数
    if (!card_status.is_exists || !card_status.remain_use_count) {
        printf("alipay_transit_update_card_data\n");
        if (ble_is_connect()) {
            res = alipay_transit_update_card_data(card_list[f_alipay->transitcode_sel].cardNo, card_list[f_alipay->transitcode_sel].cardType, f_alipay->error_str, 512);
            printf("alipay_transit_update_card_data END\n");
        } else {
            f_alipay->type_error = RV_NETWORK_ERROR;
            res = RV_NETWORK_ERROR;
        }
    }
    if (res == RV_OK) {
        f_alipay->len_transitcode = 512;
        res = alipay_transit_get_TransitCode(card_list[f_alipay->transitcode_sel].cardNo, card_list[f_alipay->transitcode_sel].cardType, f_alipay->transitcode, &f_alipay->len_transitcode, f_alipay->error_str, 256);
        if (res == RV_OK) {
            printf("Get transitcode success, len:%d\n", f_alipay->len_transitcode);
            print_r(f_alipay->transitcode, f_alipay->len_transitcode);
            memcpy(f_alipay->card_title, card_list[f_alipay->transitcode_sel].title, 40);
            func_alipay_form_create_by_page(ALIPAY_PAGE_TRANSITCODE);
        } else {
            printf("res5:%d, %s\n", res, f_alipay->error_str);
            f_alipay->type_error = res;
            func_alipay_form_create_by_page(ALIPAY_PAGE_TRANSITCODE_FAIL);

        }
    } else {
        printf("res4:%d, %s\n", res, f_alipay->error_str);
        f_alipay->type_error = res;
        func_alipay_form_create_by_page(ALIPAY_PAGE_TRANSITCODE_FAIL);
    }
}
#endif

//支付宝按钮点击处理
static void func_alipay_click_handler(void)
{
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    if (f_alipay->page == ALIPAY_PAGE_TRANSIT_LIST) {
        func_alipay_transitcode_list_click();
    } else
#endif

	{

        u8 id = compo_get_button_id();
        if (id) {
            printf(">>>button_id:%d\n", id);
            switch (f_alipay->page) {
            case ALIPAY_PAGE_OPTION_LIST:
                if (id == COMPO_ID_BTN_PAYCODE) {
                    func_alipay_form_create_by_page(ALIPAY_PAGE_QRCODE);
                }
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
				else if(id == COMPO_ID_BTN_TRANSITCODE){
                    func_alipay_form_create_by_page(ALIPAY_PAGE_LAST_TRANSITCODE);
                }
#endif


                else if(id == COMPO_ID_BTN_SETTING){
                    func_alipay_form_create_by_page(ALIPAY_PAGE_UNBIND);
                }else if(id == COMPO_ID_BTN_HELP){
                    func_alipay_form_create_by_page(ALIPAY_PAGE_HELP);
                }
                break;

            case ALIPAY_PAGE_UNBIND:
                func_alipay_form_create_by_page(ALIPAY_PAGE_UNBIND_CONFIRM);
                break;

            case ALIPAY_PAGE_UNBIND_CONFIRM:
                if (id == COMPO_ID_BTN_UNBIND_YES) {
                    if (alipay_unbinding() == ALIPAY_RV_OK){
                        freeList(&csi_head);
                        printf("alipay_unbinding success!\n");
                    }else{
                        printf("alipay_unbinding fail!\n");
                    }
                    func_alipay_form_create_by_page(ALIPAY_PAGE_UNBIND_FINISH);
                } else {
                    func_alipay_form_create_by_page(ALIPAY_PAGE_UNBIND);
                }
                break;

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
            case ALIPAY_PAGE_TRANSITCODE:
            case ALIPAY_PAGE_LAST_TRANSITCODE:
                if(id == COMPO_ID_BTN_SWITCH_TRANS_CODE){
                    func_alipay_form_create_by_page(ALIPAY_PAGE_TRANSIT_LIST);
                }
                break;
#endif

            case ALIPAY_PAGE_UNBIND_FINISH:
                func_alipay_prepare();
                break;
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
            case ALIPAY_PAGE_TRANSITCODE_FAIL:
                func_alipay_transitcode_fail_click_handle(f_alipay->last_page, &f_alipay->type_error, id);
                break;
#endif // SECURITY_PAY_EN
            }
        }
    }
}

//支付宝功能消息处理
static void func_alipay_touch_move(size_msg_t msg)
{
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;
    if (msg == MSG_CTP_SHORT_RIGHT && f_alipay->page == ALIPAY_PAGE_BARCODE){
        func_alipay_form_create_by_page(ALIPAY_PAGE_QRCODE);
    }

    if (msg == MSG_CTP_SHORT_LEFT && f_alipay->page == ALIPAY_PAGE_QRCODE){
        func_alipay_form_create_by_page(ALIPAY_PAGE_BARCODE);
    }
    if (f_alipay->page == ALIPAY_PAGE_SCAN || f_alipay->page == ALIPAY_PAGE_OPTION_LIST){
        func_message(msg);
    }
}

//支付宝功能消息处理
static void func_alipay_touch_key(void)
{
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;
    if (f_alipay->page == ALIPAY_PAGE_SCAN || f_alipay->page == ALIPAY_PAGE_OPTION_LIST){
        func_directly_back_to();
        return;
    }


    switch (f_alipay->page){
        case ALIPAY_PAGE_QRCODE:
        case ALIPAY_PAGE_BARCODE:
        case ALIPAY_PAGE_UNBIND:
        case ALIPAY_PAGE_UNBIND_CONFIRM:
        case ALIPAY_PAGE_HELP:
        case ALIPAY_PAGE_ACTIVITY_HELP:
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
        case ALIPAY_PAGE_TRANSIT_LIST:
            if (f_alipay->page == ALIPAY_PAGE_TRANSIT_LIST) {
                compo_listbox_t *listbox = f_alipay->listbox;
                func_free(listbox->mcb);
                func_free(f_alipay->transit_list);
                f_alipay->transit_list = NULL;
                listbox->mcb = NULL;
                printf("free listbox\n");
            }
#endif
            func_alipay_form_create_by_page(ALIPAY_PAGE_OPTION_LIST);
            break;

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
        case ALIPAY_PAGE_TRANSITCODE:
        case ALIPAY_PAGE_LAST_TRANSITCODE:
        case ALIPAY_PAGE_TRANSITCODE_FAIL:
            printf("last_page:%d\n", f_alipay->last_page);
            if (f_alipay->last_page == ALIPAY_PAGE_OPTION_LIST || f_alipay->page == ALIPAY_PAGE_LAST_TRANSITCODE) {
                func_alipay_form_create_by_page(ALIPAY_PAGE_OPTION_LIST);
            } else {
                func_alipay_form_create_by_page(ALIPAY_PAGE_TRANSIT_LIST);
            }
#endif
            break;
        default:
            break;
    }
}


//支付宝功能消息处理
static void func_alipay_message(size_msg_t msg)
{
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    f_alipay_t *f_alipay = (f_alipay_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_alipay->listbox;
    if (f_alipay->page == ALIPAY_PAGE_TRANSIT_LIST && compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }
#endif

    switch (msg) {
    case MSG_CTP_CLICK:
        func_alipay_click_handler();
        break;

    case MSG_CTP_SHORT_UP:
        break;

    case MSG_CTP_SHORT_DOWN:
        break;

    case MSG_CTP_LONG:
        break;
    case KU_BACK:
        func_alipay_touch_key();
        break;

    //FOR TEST
    case KL_BACK:
        alipay_reset_all();
        printf("reset_all\n");
        break;

    case MSG_SYS_1S:
        alipay_binding_detect();
        break;


    case MSG_CTP_SHORT_RIGHT:
        func_alipay_touch_move(MSG_CTP_SHORT_RIGHT);
        break;
    case MSG_CTP_SHORT_LEFT:
        func_alipay_touch_move(MSG_CTP_SHORT_LEFT);
        break;
    default:
        func_message(msg);
        break;
    }
}

//进入支付宝功能
void func_alipay_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_alipay_t));
    func_cb.frm_main = func_alipay_form_create();
    func_alipay_init();
}


//退出支付宝功能
void func_alipay_exit(void)
{
    printf("%s\n", __func__);
    freeList(&csi_head);

    func_cb.last = FUNC_ALIPAY;
}

//支付宝功能
void func_alipay(void)
{
    printf("%s\n", __func__);
    func_alipay_enter();
    while (func_cb.sta == FUNC_ALIPAY) {
        func_alipay_process();
        func_alipay_message(msg_dequeue());
    }
    func_alipay_exit();
}
#endif // SECURITY_PAY_EN

