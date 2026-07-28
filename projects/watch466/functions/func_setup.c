#include "include.h"
#include "func.h"

/* home bin 已移除：本页仅保留菜单骨架，不再加载 UI_BUF_HOME_* / home_ui_* */

enum {
    SETUP_ROW_TIME = 0,
    SETUP_ROW_LANGUAGE,
    SETUP_ROW_INFO,
    SETUP_ROW_CNT,
};

enum {
    COMPO_ID_TXT_TITLE = 1,
    COMPO_ID_TXT_ROW,
};

typedef struct f_setup_t_ {
    u8 focus;
    compo_textbox_t *txt_row;
} f_setup_t;

static const char * const tbl_setup_row_name[SETUP_ROW_CNT] = {
    "Time",
    "Language",
    "Info",
};

static const u8 tbl_setup_row_func[SETUP_ROW_CNT] = {
    FUNC_TIMEING,
    FUNC_LANGUAGEING,
    FUNC_VERINFO,
};

compo_form_t *func_setup_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    txt = compo_textbox_create(frm, 32);
    compo_setid(txt, COMPO_ID_TXT_TITLE);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 40);
    compo_textbox_set(txt, "SETUP");

    txt = compo_textbox_create(frm, 48);
    compo_setid(txt, COMPO_ID_TXT_ROW);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_textbox_set(txt, "Time");

    return frm;
}

static void func_setup_row_refresh(f_setup_t *f_setup)
{
    if (f_setup == NULL || f_setup->txt_row == NULL) {
        return;
    }
    if (f_setup->focus >= SETUP_ROW_CNT) {
        f_setup->focus = 0;
    }
    compo_textbox_set(f_setup->txt_row, tbl_setup_row_name[f_setup->focus]);
}

static void func_setup_process(void)
{
    func_process();
}

static void func_setup_message(size_msg_t msg)
{
    f_setup_t *f_setup = (f_setup_t *)func_cb.f_cb;

    switch (msg) {
    case KU_MODE:
    case K_MODE:
        if (f_setup != NULL) {
            f_setup->focus = (u8)((f_setup->focus + 1) % SETUP_ROW_CNT);
            func_setup_row_refresh(f_setup);
        }
        break;

    case KU_BACK:
        if (f_setup != NULL && f_setup->focus < SETUP_ROW_CNT) {
            func_switch_to(tbl_setup_row_func[f_setup->focus],
                           FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
        break;

    case KU_RIGHT:
        func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_setup_enter(void)
{
    f_setup_t *f_setup;

    func_cb.f_cb = func_zalloc(sizeof(f_setup_t));
    func_cb.frm_main = func_setup_form_create();
    f_setup = (f_setup_t *)func_cb.f_cb;
    f_setup->focus = SETUP_ROW_TIME;
    f_setup->txt_row = compo_getobj_byid(COMPO_ID_TXT_ROW);
    func_setup_row_refresh(f_setup);
}

void func_setup_exit(void)
{
    func_cb.last = FUNC_SETUP;
}

void func_setup(void)
{
    printf("%s\n", __func__);
    func_setup_enter();
    while (func_cb.sta == FUNC_SETUP) {
        func_setup_process();
        func_setup_message(msg_dequeue());
    }
    func_setup_exit();
}
