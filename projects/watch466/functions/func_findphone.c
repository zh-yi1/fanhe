#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define FINDPHONE_POS_Y_MAX         240
#define FINDPHONE_POS_Y_MIN         140

typedef struct f_findphone_t_ {
    u16 pos_xy;
    u8 flag_find;
} f_findphone_t;

enum {
    COMPO_ID_TEXT_FIND = 1,
    COMPO_ID_PIC_FIND,
};

//创建查找手机窗体
compo_form_t *func_findphone_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_FIND_PHONE]);

    //新建图像
    compo_picturebox_t *pic;
    pic = compo_picturebox_create(frm, UI_BUF_COMMON_FIND_PHONE_BIN);
    compo_setid(pic, COMPO_ID_PIC_FIND);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, 200);

    //新建文字
    compo_label_t *txt = compo_label_create(frm, 20);
    compo_label_set(txt, i18n[STR_FIND_PHONEING]);
    compo_label_set_pos(txt, GUI_SCREEN_CENTER_X, 340);
    compo_label_set_align_center(txt, true);

    return frm;
}

//查找手机功能事件处理
static void func_findphone_process(void)
{
    compo_picturebox_t *pic = compo_getobj_byid(COMPO_ID_PIC_FIND);
    f_findphone_t *f_act = (f_findphone_t *)func_cb.f_cb;
    static u32 ticks = 0;
    if (tick_check_expire(ticks, 5)) {
        ticks = tick_get();
        if (f_act->pos_xy <= FINDPHONE_POS_Y_MAX && f_act->flag_find) {
            f_act->pos_xy++;
        } else {
            f_act->flag_find = 0;
            f_act->pos_xy--;
            if (f_act->pos_xy <= FINDPHONE_POS_Y_MIN) {
                f_act->flag_find = 1;
            }
        }
    }
    compo_picturebox_set_size(pic, f_act->pos_xy, f_act->pos_xy);
    func_process();
}

//查找手机功能消息处理
static void func_findphone_message(size_msg_t msg)
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

//进入查找手机功能
void func_findphone_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_findphone_t));
    func_cb.frm_main = func_findphone_form_create();
    f_findphone_t *f_act = (f_findphone_t *)func_cb.f_cb;
    f_act->pos_xy = 140;
    f_act->flag_find = 1;

//    ab_app_search_phone(true);
}

//退出查找手机功能
void func_findphone_exit(void)
{
    func_cb.last = FUNC_FINDPHONE;

//    ab_app_search_phone(false);
}

//查找手机功能
void func_findphone(void)
{
    printf("%s\n", __func__);
    func_findphone_enter();
    while (func_cb.sta == FUNC_FINDPHONE) {
        func_findphone_process();
        func_findphone_message(msg_dequeue());
    }
    func_findphone_exit();
}
