#include "include.h"
#include "api.h"
#include "func.h"

bsp_bt_t bt_cb;

void dev_vol_set_cb(uint8_t dev_vol, uint8_t media_index, uint8_t setting_type);

void bsp_bt_init(void)
{
    if (BT_HFP_EN && xcfg_cb.bt_sco_en && !support_hfp()) {
        printf("NOT SUPPORT HFP\n");
        WDT_RST();
    }

    //更新配置工具的设置
    cfg_bt_rf_def_txpwr = xcfg_cb.bt_rf_pwrdec;
    cfg_bt_page_txpwr = xcfg_cb.bt_rf_txpwr_recon;

    cfg_bt_support_profile = (PROF_HFP*BT_HFP_EN*xcfg_cb.bt_sco_en)\
        | (PROF_SPP*BT_SPP_EN*(xcfg_cb.bt_spp_en||xcfg_cb.eq_dgb_spp_en)) | (PROF_HID*BT_HID_EN) | (PROF_HSP*BT_HSP_EN*xcfg_cb.bt_sco_en)\
        | (PROF_PBAP*BT_PBAP_EN) | (PROF_MAP*BT_MAP_EN)
        | (PROF_PANU * BT_PANU_EN);

#if BT_A2DP_PROFILE_DEFAULT_EN
    cfg_bt_support_profile |= (PROF_A2DP*BT_A2DP_EN*xcfg_cb.bt_a2dp_en);
#endif

#if BT_A2DP_VOL_CTRL_EN
    if(!xcfg_cb.bt_a2dp_vol_ctrl_en) {
        cfg_bt_a2dp_feature &= ~A2DP_AVRCP_VOL_CTRL;
        cfg_bt_a2dp_feature1 &= ~A2DP_AVRCP_RECORD_DEVICE_VOL;
    } else {
        cfg_bt_a2dp_feature |= A2DP_AVRCP_VOL_CTRL;
    }
#endif

#if BT_A2DP_AVRCP_PLAY_STATUS_EN
    cfg_bt_a2dp_feature |= A2DP_AVRCP_PLAY_STATUS;
#endif
    cfg_bt_dual_mode = BT_DUAL_MODE_EN * xcfg_cb.ble_en;
    cfg_bt_max_acl_link = BT_2ACL_EN + 1;

#if BT_EMIT_EN
		cfg_bt_connect_name_en = xcfg_cb.bt_connect_name_en;
#endif

#if BT_HFP_EN
    if(!xcfg_cb.bt_hfp_ring_number_en) {
        cfg_bt_hfp_feature &= ~HFP_RING_NUMBER_EN;
    }
#endif

#if BT_FCC_TEST_EN
    bt_fcc_init();
#endif

#if BT_PBAP_EN
    bt_pbap_param_reset();
#endif

    memset(&bt_cb, 0, sizeof(bsp_bt_t));
    bt_cb.disp_status = 0xfe;
    bt_cb.need_pairing = 1;          //开机若回连不成功，需要播报pairing
    bt_cb.hid_menu_flag = 1;

    bt_setup();
}

void bsp_bt_close(void)
{
}

void bsp_bt_vol_change(void)
{
#if BT_A2DP_VOL_CTRL_EN
    if((xcfg_cb.bt_a2dp_vol_ctrl_en && (bt_get_status() >= BT_STA_CONNECTED))) {
        bt_music_vol_change();     //通知手机音量已调整
    }
#endif
}

uint bsp_bt_get_hfp_vol(uint hfp_vol)
{
    uint vol;
    vol = (hfp_vol + 1) * sys_cb.hfp2sys_mul;
    if (vol > VOL_MAX) {
        vol = VOL_MAX;
    }
    return vol;
}

bool phone_conecting_flag = false;
void bt_emit_notice(uint evt, void *params)
{
    u8 *packet = params;
    u8 opcode = 0;
    u8 scan_status = 0x03;

    printf("bt notice:%x\n", evt);

    switch(evt) {
    case BT_NOTICE_INIT_FINISH:
        if(cfg_bt_work_mode == MODE_BQB_RF_BREDR) {
            opcode = 1;                     //测试模式，不回连，打开可被发现可被连接
        }
        bt_start_work(opcode, scan_status);
        break;

    case BT_NOTICE_DISCONNECT:
        bt_cb.warning_status |= BT_WARN_DISCON;
        bt_redial_reset(((u8 *)params)[0] & 0x01);
#if BT_HID_ONLY_FOR_IOS_EN
        bt_init_lib_hid();
#endif
        break;

    case BT_NOTICE_CONNECTED:
        bt_cb.warning_status |= BT_WARN_CON;
        bt_redial_reset(((u8 *)params)[0] & 0x01);
#if BT_PANU_EN
        bt_panu_network_connect();
#endif
        break;

//    case BT_NOTICE_LOSTCONNECT:
//        break;
//    case BT_NOTICE_INCOMING:
//    case BT_NOTICE_RING:
//    case BT_NOTICE_OUTGOING:
//    case BT_NOTICE_CALL:
//        break;

    case BT_NOTICE_SET_SPK_GAIN:
        dev_vol_set_cb(packet[0], packet[1], 1 | BIT(3));
        break;

    case BT_NOTICE_MUSIC_PLAY:
        msg_enqueue(EVT_A2DP_MUSIC_PLAY);
        break;

    case BT_NOTICE_MUSIC_STOP:
        if (bt_get_disp_status() > BT_STA_PLAYING) {
            break;
        }
        msg_enqueue(EVT_A2DP_MUSIC_STOP);
        break;

    case BT_NOTICE_MUSIC_CHANGE_VOL:
        if(params == 0) {
            msg_enqueue(KU_VOL_DOWN);
        } else {
            msg_enqueue(KU_VOL_UP);
        }
        break;

    case BT_NOTICE_MUSIC_SET_VOL:
        if((sys_cb.incall_flag & INCALL_FLAG_SCO) == 0) {
            dev_vol_set_cb(packet[0], packet[1], 1);
        }
        break;

    case BT_NOTICE_MUSIC_CHANGE_DEV:
        dev_vol_set_cb(packet[0], packet[1], 0);
        break;

    case BT_NOTICE_CALL_CHANGE_DEV:
        dev_vol_set_cb(packet[0], packet[1], 0 | BIT(3));
        break;

    case BT_NOTICE_HID_CONN_EVT:
#if BT_HID_MENU_EN
        if (xcfg_cb.bt_hid_menu_en) {
            if (bt_cb.hid_menu_flag == 2) {
                //按键连接/断开HID Profile完成
                if (param) {
                    bt_cb.warning_status |= BT_WARN_HID_CON;
                } else {
                    bt_cb.warning_status |= BT_WARN_HID_DISCON;
                }
                bt_cb.hid_menu_flag = 1;
            }
    #if BT_HID_DISCON_DEFAULT_EN
            else if (bt_cb.hid_menu_flag == 1) {
                if ((param) & (xcfg_cb.bt_hid_discon_default_en)) {
                    bt_cb.hid_discon_flag = 1;
                }
            }
    #endif // BT_HID_DISCON_DEFAULT_EN
        }
#endif // BT_HID_MENU_EN
        break;

    case BT_NOTICE_A2DP_CONN_EVT:
        {
#if BT_EMIT_EN
            printf("BT_NOTICE_A2DP_CONN_EVT:%d\n", packet[0]);
            if (packet[0]) {
                msg_enqueue(EVT_EMIT_START);
            } else {
                msg_enqueue(EVT_EMIT_STOP);
            }
#endif

        }
        break;

    case BT_NOTICE_HFP_CONN_EVT:
        break;

    case BT_NOTICE_CONNECT_FAIL:
    {
#if LE_SM_SC_EN
        if (packet[0] != 0x0B) {   //BB_ERROR_ACL_CON_EXISTS
            if (sys_cb.bt_reconn_flag) {
                sys_cb.bt_reconn_flag = false;
                ble_bt_connect();       //双连接
            }
        }
#endif // LE_SM_SC_EN
#if BT_EMIT_EN
        printf("BT_NOTICE_CONNECT_FAIL:%x\n", packet[0]);
        msg_enqueue(EVT_EMIT_RECONNECT);
#endif
        break;
    }
    case BT_NOTICE_SCO_SETUP:
        printf("BT_NOTICE_SCO_SETUP\n");
        sys_cb.sco_state = true;
        break;

    case BT_NOTICE_SCO_KILL:
        printf("BT_NOTICE_SCO_KILL\n");
        sys_cb.sco_state = false;
        bt_cb.call_type = CALL_TYPE_NONE;
        break;

    case BT_NOTICE_NETWORK_CALL:
        printf("BT_NOTICE_NETWORK_CALL\n");
        bt_cb.call_type = CALL_TYPE_NETWORK;
        break;
    case BT_NOTICE_PHONE_CALL:
        printf("BT_NOTICE_PHONE_CALL\n");
        bt_cb.call_type = CALL_TYPE_PHONE;
        break;

#if BT_PBAP_EN
    case BT_NOTICE_PBAP_CONNECTED:
    case BT_NOTICE_PBAP_GET_PHONEBOOK_SIZE_COMPLETE:
    case BT_NOTICE_PBAP_PULL_PHONEBOOK_COMPLETE:
    case BT_NOTICE_PBAP_DISCONNECT:
        bt_pbap_event_handle(evt, params);
        break;
#endif
// 1，表示play，2表示pause
    case BT_NOTICE_FAST_MUSIC_STATUS:
//        my_printf("fast music status(%d)\n",((u8 *)param)[0]);
        break;


//#if BT_EMIT_EN
//    case BT_NOTICE_BT_INFO:
//        bsp_emit_info_handle(packet);
//        break;
//
//    case BT_NOTICE_INQUIRY_FINISH:
//        printf("BT_NOTICE_INQUIRY_FINISH\n");
//        break;
//#endif

    case BT_NOTICE_MAP_CONNECT_SUCCESS:
        break;
    case BT_NOTICE_MAP_CONNECT_FAIL:
        break;
    case BT_NOTICE_MAP_GET_FINISH:
        break;
    case BT_NOTICE_MAP_GET_FAIL:
        break;

    default:
        break;
    }
}

ALIGNED(64)
u16 bsp_bt_chkclr_warning(u16 bits)
{
    u16 value;
    GLOBAL_INT_DISABLE();
    value = bt_cb.warning_status & bits;
    if(value != 0) {
        bt_cb.warning_status &= ~value;
        GLOBAL_INT_RESTORE();
        return value;
    }
    GLOBAL_INT_RESTORE();
    return value;
}

void bsp_bt_warning(void)
{

#if BT_HID_MENU_EN
    //按键手动断开HID Profile的提示音
    if (xcfg_cb.bt_hid_menu_en) {

    #if BT_HID_DISCON_DEFAULT_EN
        if (bt_cb.hid_discon_flag) {
            if (bt_hid_is_ready_to_discon()) {
                bt_cb.hid_discon_flag = 0;
                bt_hid_disconnect();
            }
        }
    #endif // BT_HID_DISCON_DEFAULT_EN
    }
#endif // BT_HID_MENU_EN
}

AT(.text.func.disp_status)
uint bsp_bt_disp_status(void)
{
    uint status = bt_get_disp_status();

    if(bt_cb.disp_status != status) {
        bt_cb.disp_status = status;
        //if(!bt_is_connected()) {
        if (0) {
            en_auto_pwroff();
            sys_cb.sleep_en = BT_PAIR_SLEEP_EN;
        } else {
            dis_auto_pwroff();
#if !ELUNCHBOX_KEEP_AWAKE
            sys_cb.sleep_en = 1;
#endif
        }

        if(bt_cb.disp_status >= BT_STA_CONNECTED) {
            bt_cb.need_pairing = 1;
        }
    }
    return bt_cb.disp_status;
}

AT(.text.func.bt)
void bsp_bt_status(void)
{
    bsp_bt_disp_status();
    bsp_bt_warning();
}

