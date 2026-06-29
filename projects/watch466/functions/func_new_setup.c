#include "include.h"
#include "func.h"

void func_new_setup_enter(void)
{
    printf("func_new_setup_enter\n");
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

compo_form_t *func_new_setup_form_create(void)
{
    return compo_form_create(true);
}
