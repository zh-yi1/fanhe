#include "include.h"
#include "func.h"

/* home bin 已移除：本页仅保留可进可退骨架 */

enum {
    COMPO_ID_TXT_TITLE = 1,
};

compo_form_t *func_languageing_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt = compo_textbox_create(frm, 32);

    compo_setid(txt, COMPO_ID_TXT_TITLE);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_textbox_set(txt, "LANGUAGE");
    return frm;
}

static void func_languageing_process(void)
{
    func_process();
}

static void func_languageing_message(size_msg_t msg)
{
    switch (msg) {
    case KU_RIGHT:
        func_switch_to(FUNC_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_languageing_enter(void)
{
    func_cb.f_cb = NULL;
    func_cb.frm_main = func_languageing_form_create();
}

void func_languageing_exit(void)
{
    func_cb.last = FUNC_LANGUAGEING;
}

void func_languageing(void)
{
    printf("%s\n", __func__);
    func_languageing_enter();
    while (func_cb.sta == FUNC_LANGUAGEING) {
        func_languageing_process();
        func_languageing_message(msg_dequeue());
    }
    func_languageing_exit();
}
