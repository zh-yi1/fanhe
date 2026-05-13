#include "include.h"
#include "func.h"

void func_bt_update_enter(void)
{

}

void func_bt_update_message(u16 msg)
{
    func_message(msg);
}

void func_bt_update_exit(void)
{
    func_cb.last = FUNC_BT_UPDATE;
}

void func_bt_update_process(void)
{
    func_process();
}

AT(.text.func.bt_update)
void func_bt_update(void)
{
    printf("%s\n", __func__);

    func_bt_update_enter();

    while (func_cb.sta == FUNC_BT_UPDATE) {
        func_bt_update_process();
        func_bt_update_message(msg_dequeue());
    }

    func_bt_update_exit();
}
