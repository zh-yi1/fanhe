#include "include.h"
#include "func.h"

/* home bin removed: skeleton page only, no UI_BUF_HOME_* / home_ui_* */

enum {
    COMPO_ID_TXT_TITLE = 1,
    COMPO_ID_TXT_INFO,
};

typedef struct f_mode_t_ {
    compo_textbox_t *txt_info;
} f_mode_t;

static u32 mode_countdown_remain_sec;
static bool mode_countdown_running;

void func_mode_countdown_set(u8 hour, u8 min)
{
    mode_countdown_remain_sec = (u32)hour * 3600 + (u32)min * 60;
}

void func_mode_countdown_start(void)
{
    mode_countdown_running = true;
}

void func_mode_countdown_stop(void)
{
    mode_countdown_running = false;
}

void func_mode_temp_set_f(u16 temp_f)
{
    (void)temp_f;
}

compo_form_t *func_mode_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    txt = compo_textbox_create(frm, 32);
    compo_setid(txt, COMPO_ID_TXT_TITLE);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 40);
    compo_textbox_set(txt, "MODE");

    txt = compo_textbox_create(frm, 48);
    compo_setid(txt, COMPO_ID_TXT_INFO);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_textbox_set(txt, "no home bin");

    return frm;
}

static void func_mode_process(void)
{
    func_process();
}

static void func_mode_message(size_msg_t msg)
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

void func_mode_enter(void)
{
    f_mode_t *f_mode;

    func_cb.f_cb = func_zalloc(sizeof(f_mode_t));
    func_cb.frm_main = func_mode_form_create();
    f_mode = (f_mode_t *)func_cb.f_cb;
    f_mode->txt_info = compo_getobj_byid(COMPO_ID_TXT_INFO);
    func_mode_countdown_stop();
}

void func_mode_exit(void)
{
    func_mode_countdown_stop();
    func_cb.last = FUNC_MODE;
}

void func_mode(void)
{
    printf("%s\n", __func__);
    func_mode_enter();
    while (func_cb.sta == FUNC_MODE) {
        func_mode_process();
        func_mode_message(msg_dequeue());
    }
    func_mode_exit();
}
