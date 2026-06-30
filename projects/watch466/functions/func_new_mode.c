#include "include.h"
#include "func.h"

compo_form_t *func_new_mode_form_create(void)
{
    return compo_form_create(true);
}

void func_new_mode_enter(void)
{
    printf("func_new_mode_enter\n");

    func_cb.frm_main = func_new_mode_form_create();
}

void func_new_mode_exit(void)
{
    printf("func_new_mode_exit\n");
}

void func_new_mode(void)
{
    func_new_mode_enter();
    while (func_cb.sta == FUNC_NEW_MODE) {
        func_process();
    }
    func_new_mode_exit();
}
