#include "include.h"
#include "func.h"

/* home bin removed: skeleton page only, no UI_BUF_HOME_* / home_ui_* */

enum {
    COMPO_ID_TXT_TITLE = 1,
    COMPO_ID_TXT_INFO,
};

typedef struct f_heat_t_ {
    compo_textbox_t *txt_info;
    u8 set_hour;
    u8 set_min;
} f_heat_t;

static u32 heat_countdown_remain_sec;
static bool heat_countdown_running;

void func_heat_countdown_set(u8 hour, u8 min)
{
    heat_countdown_remain_sec = (u32)hour * 3600 + (u32)min * 60;
}

void func_heat_countdown_start(void)
{
    heat_countdown_running = true;
}

void func_heat_countdown_stop(void)
{
    heat_countdown_running = false;
}

u32 func_heat_countdown_remain_sec(void)
{
    return heat_countdown_remain_sec;
}

void func_heat_temp_set_f(u16 temp_f)
{
    (void)temp_f;
}

compo_form_t *func_heat_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    txt = compo_textbox_create(frm, 32);
    compo_setid(txt, COMPO_ID_TXT_TITLE);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 40);
    compo_textbox_set(txt, "HEAT");

    txt = compo_textbox_create(frm, 48);
    compo_setid(txt, COMPO_ID_TXT_INFO);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_textbox_set(txt, "no home bin");

    return frm;
}

static void func_heat_process(void)
{
    func_process();
}

static void func_heat_message(size_msg_t msg)
{
    switch (msg) {
    case KU_RIGHT:
        func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_heat_enter(void)
{
    f_heat_t *f_heat;

    func_cb.f_cb = func_zalloc(sizeof(f_heat_t));
    func_cb.frm_main = func_heat_form_create();

    f_heat = (f_heat_t *)func_cb.f_cb;
    f_heat->set_hour = 1;
    f_heat->set_min = 0;
    f_heat->txt_info = compo_getobj_byid(COMPO_ID_TXT_INFO);

    func_heat_countdown_set(f_heat->set_hour, f_heat->set_min);
    func_heat_countdown_stop();
}

void func_heat_exit(void)
{
    func_heat_countdown_stop();
    func_cb.last = FUNC_HEAT;
}

void func_heat(void)
{
    printf("%s\n", __func__);
    func_heat_enter();
    while (func_cb.sta == FUNC_HEAT) {
        func_heat_process();
        func_heat_message(msg_dequeue());
    }
    func_heat_exit();
}
