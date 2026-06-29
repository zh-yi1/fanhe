#include "include.h"
#include "func.h"

void func_new_heat_enter(void)
{
    printf("func_new_heat_enter\n");
}

void func_new_heat_exit(void)
{
    printf("func_new_heat_exit\n");
}

void func_new_heat(void)
{
    func_new_heat_enter();
    while (func_cb.sta == FUNC_NEW_HEAT) {
        func_process();
    }
    func_new_heat_exit();
}

compo_form_t *func_new_heat_form_create(void)
{
    return compo_form_create(true);
}
