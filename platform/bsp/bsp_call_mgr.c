#include "include.h"

void bsp_bt_call_process(bsp_call_mgr_msg_t msg);
void bsp_modem_call_process(bsp_call_mgr_msg_t msg);
void bsp_modem_call_get_current_call(void);
bool cat1_is_bypass(void);
void func_call_mgr_endcall(void);

bsp_call_mgr_t call_mgr;

void bsp_call_mgr_init(void)
{
    memset(&call_mgr, 0, sizeof(call_mgr));
}

#if CALL_MGR_DEFAULT
// clang-format off
AT(.com_text.bsp_call_mgr) NO_INLINE
// clang-format on
static void bsp_call_mgr_send_do(bsp_call_mgr_msg_t msg)
{
    GLOBAL_INT_DISABLE();
    switch (msg) {
    case CALL_MGR_BT_INCOM:
    case CALL_MGR_BT_OUTGO:
    case CALL_MGR_BT_INCALL:
        call_mgr.bt_call_num = 1;
        break;
    case CALL_MGR_BT_ENDCALL:
        call_mgr.bt_call_num = 0;
        break;
    case CALL_MGR_MODEM_INCOM:
    case CALL_MGR_MODEM_OUTGO:
    case CALL_MGR_MODEM_INCALL:
        call_mgr.modem_call_num = 1;
        break;
    case CALL_MGR_MODEM_ENDCALL:
        call_mgr.modem_call_num = 0;
        break;
    default:
        break;
    }
    GLOBAL_INT_RESTORE();
}
#endif

// 重点在于解决挂断通话的时候，如何决定通话状态
void bsp_call_mgr_send(bsp_call_mgr_msg_t msg)
{
#if CALL_MGR_DEFAULT
    bsp_call_mgr_send_do(msg);

    if (msg == CALL_MGR_BT_ENDCALL) {
        call_mgr.update = CALL_UPDATE_IDLE;
        if (call_mgr.modem_call_num != 0) {
            bsp_modem_call_get_current_call();
            return;
        } else {
            func_call_mgr_endcall();
        }
    } else if (msg == CALL_MGR_MODEM_ENDCALL) {
        if (call_mgr.bt_call_num != 0) {
            bt_call_get_remote_phone_number();
        } else {
            func_call_mgr_endcall();
        }
    }

    if (msg == CALL_MGR_BT_INCOM || msg == CALL_MGR_BT_OUTGO ||
        msg == CALL_MGR_BT_INCALL) {
        if (!((call_mgr.sta != CALL_MGR_BT_INCALL) &&
              (call_mgr.modem_call_num == 0))) {
            return;
        }
    }
#endif

    call_mgr.msg = msg;
#if BT_VOIP_REJECT_EN
    // 网络电话会在短时间内自动断开
    if (msg == CALL_MGR_BT_INCOM || msg == CALL_MGR_BT_OUTGO) {
        call_mgr.update = CALL_UPDATE_DELAY;
        call_mgr.ticks  = tick_get();
        return;
    }
#endif

    call_mgr.update = CALL_UPDATE_NOW;
#if CALL_MGR_DEFAULT
    call_mgr.sta = msg;
#endif
}

bool bsp_call_mgr_need_update(void)
{
    return call_mgr.update;
}

void bsp_call_mgr_set_audio_path(bsp_call_audio_path_t path)
{
    switch (path) {
    case CALL_AUDIO_PATH_NONE:
        bt_hfp_switch_to_phone();
        bsp_modem_audio_stop();
        break;
    case CALL_AUDIO_PATH_BTHF2DAC:
        bsp_modem_audio_stop();
        bt_hfp_switch_to_watch();
        break;
    case CALL_AUDIO_PATH_MODEM2DAC:
        bt_hfp_switch_to_phone();
        bsp_modem_audio_start();
        break;

    default:
        break;
    }
}
