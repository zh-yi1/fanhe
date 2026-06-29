#include "include.h"
#include "func.h"

void func_new_mode_enter(void)
{
    printf("func_new_mode_enter\n");
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

compo_form_t *func_new_mode_form_create(void)
{
    return compo_form_create(true);
}
