#include "include.h"

void func_bt_call_mgr(bsp_call_mgr_msg_t msg);
void func_modem_call_mgr(bsp_call_mgr_msg_t msg);

void func_call_mgr_endcall(void)
{
    while (func_cb.sta == FUNC_BT_RING || func_cb.sta == FUNC_BT_CALL
#if MODEM_CAT1_EN
           || func_cb.sta == FUNC_MODEM_RING || func_cb.sta == FUNC_MODEM_CALL
#endif
    ) {
        func_cb.sta = func_directly_back_to();
    }
}

void func_call_mgr_process(void)
{
    if ((call_mgr.update != CALL_UPDATE_IDLE) && (call_mgr.msg == CALL_MGR_NONE)) {
        printf("call_mgr.msg error\n");
        call_mgr.update = CALL_UPDATE_IDLE;
        return;
    }
#if BT_VOIP_REJECT_EN
    if ((call_mgr.update == CALL_UPDATE_DELAY) && tick_check_expire(call_mgr.ticks, 500)) {
        call_mgr.update = CALL_UPDATE_IDLE;

        func_bt_call_mgr(call_mgr.msg);
#if CALL_MGR_DEFAULT
        call_mgr.sta = call_mgr.msg;
#endif
    } else
#endif
    if (call_mgr.update == CALL_UPDATE_NOW) {
        call_mgr.update = CALL_UPDATE_IDLE;
        func_bt_call_mgr(call_mgr.msg);
        func_modem_call_mgr(call_mgr.msg);
    }
}
