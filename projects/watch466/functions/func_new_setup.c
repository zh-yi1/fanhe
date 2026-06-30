#include "include.h"
#include "func.h"

#if ELUNCHBOX_PANEL_EN
extern volatile u8 elunchbox_te_block_flag;
#endif

compo_form_t *func_new_setup_form_create(void)
{
    return compo_form_create(true);
}

void func_new_setup_enter(void)
{
    printf("func_new_setup_enter\n");

    func_cb.frm_main = func_new_setup_form_create();
#if ELUNCHBOX_PANEL_EN
    elunchbox_te_block_flag = 0;
#endif
}

void func_new_setup_exit(void)
{
    printf("func_new_setup_exit\n");
}

void func_new_setup(void)
{
    func_new_setup_enter();
    while (func_cb.sta == FUNC_NEW_SETUP) {
        func_process();
    }
    func_new_setup_exit();
}
