#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_scan_t_ {

} f_scan_t;

void *barcode_creat(void *parent, char *str, int x, int y, int h, u8 length, bool dir);

//创建扫一扫窗体
compo_form_t *func_scan_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_SCAN]);

	//创建按键
    //compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_ICON_SCAN_BIN);
    //compo_button_set_pos(btn, 160, 180);

    compo_qrcodebox_t *qrbox = compo_qrcodebox_create(frm, QRCODE_TYPE_2D, 64);
    compo_qrcodebox_set(qrbox, "http://www.bluetrum.com");
    compo_qrcodebox_set_bitwid_by_qrwid(qrbox, 180);
    //barcode_creat(frm->page_body, "123896\0", GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 80, 6, false);

    return frm;
}

//扫一扫功能事件处理
static void func_scan_process(void)
{
    func_process();
}

//扫一扫功能消息处理
static void func_scan_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        break;

    case MSG_CTP_SHORT_UP:
        break;

    case MSG_CTP_SHORT_DOWN:
        break;

    case MSG_CTP_LONG:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入扫一扫功能
void func_scan_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_scan_t));
    func_cb.frm_main = func_scan_form_create();
}

//退出扫一扫功能
void func_scan_exit(void)
{
    func_cb.last = FUNC_SCAN;
}

//扫一扫功能
void func_scan(void)
{
    printf("%s\n", __func__);
    func_scan_enter();
    while (func_cb.sta == FUNC_SCAN) {
        func_scan_process();
        func_scan_message(msg_dequeue());
    }
    func_scan_exit();
}
