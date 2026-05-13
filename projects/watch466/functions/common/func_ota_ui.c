#include "include.h"
#include "func.h"
#include "func_idle.h"
#include "bsp_led.h"

#if FOTA_UI_EN

AT(.text.func.idle)
void func_ota_ui_process(void)
{
    func_process();
}

void func_ota_ui_enter(void)
{

}

void func_ota_ui_message(u16 msg)
{
    evt_message(msg);
}

void func_ota_ui_exit(void)
{
    func_cb.last = FUNC_IDLE;
}

AT(.text.func.idle)
void func_ota_ui(void)
{
    printf("%s\n", __func__);

    func_ota_ui_enter();

    while (func_cb.sta == FUNC_OTA_UI_MODE) {
        func_ota_ui_process();
        func_ota_ui_message(msg_dequeue());
    }

    func_ota_ui_exit();
}

#endif
