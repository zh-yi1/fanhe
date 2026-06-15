#include "include.h"
#include "api.h"

#if BT_FCC_TEST_EN
    #undef WORK_MODE
    #if LE_BQB_RF_EN
        #define WORK_MODE           MODE_BQB_RF_BLE
    #else
        #define WORK_MODE           MODE_FCC_TEST
    #endif
#elif BT_BQB_RF_EN
    #define WORK_MODE           MODE_BQB_RF_BREDR
#elif LE_BQB_RF_EN
    #define WORK_MODE           MODE_BQB_RF_BLE
#elif BT_DUT_MODE_EN
    #define WORK_MODE           MODE_NORMAL_DUT
#else
    #define WORK_MODE           MODE_NORMAL
#endif

#define BT_PROFILE          (PROF_A2DP*BT_A2DP_EN) | \
                            (PROF_HFP*BT_HFP_EN) | \
                            (PROF_SPP*BT_SPP_EN) | \
                            (PROF_HID*BT_HID_EN) | \
                            (PROF_HSP*BT_HSP_EN) | \
                            (PROF_PANU * BT_PANU_EN)

#define BT_CODEC            (CODEC_SBC) | \
                            (CODEC_AAC*BT_A2DP_AAC_AUDIO_EN) | \
                            (CODEC_MSBC * BT_HFP_MSBC_EN) | \
                            (CODEC_PLC * BT_PLC_EN)

#define HFP_FEATURE         (HFP_BAT_REPORT*BT_HFP_BAT_REPORT_EN) | \
                            (HFP_INBAND_RING_TONE*BT_HFP_INBAND_RING_EN) | \
                            (HFP_CALL_PRIVATE*BT_HFP_CALL_PRIVATE_FORCE_EN) | \
                            (HFP_RING_NUMBER_EN*BT_HFP_RING_NUMBER_EN)

#define HFP_FEATURE1        (HFP_RECORD_DEVICE_VOL * BT_HFP_RECORD_DEVICE_VOL_EN)

#define A2DP_FEATURE        (A2DP_AVRCP_VOL_CTRL*BT_A2DP_VOL_CTRL_EN) | \
                            (A2DP_RESTORE_PLAYING*BT_A2DP_SUPTO_RESTORE_PLAY_EN) | \
                            (A2DP_AVDTP_DELAY_REPORT) | \
                            (A2DP_AVDTP_DYN_LATENCY * BT_AVDTP_DYN_LATENCY_EN) | \
                            (A2DP_AVDTP_EXCEPT_REST_PLAY * BT_A2DP_EXCEPT_RESTORE_PLAY_EN)

#define A2DP_FEATURE1       (A2DP_AVRCP_RECORD_DEVICE_VOL*BT_A2DP_RECORD_DEVICE_VOL_EN) | \
                            (A2DP_RESET_DEVICE_VOL*BT_A2DP_VOL_REST_EN)


///baseband
uint8_t cfg_bt_rf_def_txpwr         = 0;        //降低预置参数RF发射功率，单位0.5dbm
uint8_t cfg_bt_page_txpwr           = 0;        //降低回连RF发射功率，单位3dbm
uint8_t cfg_bt_inq_txpwr            = 0;        //降低搜索RF发射功率，单位3dbm
uint8_t cfg_ble_page_txpwr          = 0;        //降低组队RF发射功率，单位3dbm
uint8_t cfg_ble_page_rssi_thr       = 0;        //设置组队范围rssi

///stack
uint8_t cfg_bt_work_mode            = WORK_MODE;
#if BT_EMIT_EN
uint8_t cfg_bt_max_acl_link         = 2;
#else
uint8_t cfg_bt_max_acl_link         = 1;
#endif
uint8_t cfg_bt_sniff_clk_sel        = 3;        //3:31.25KHz RC, 4:31.25KHz XOSC, 5:31.25KHz LP_XOSC

bool cfg_bt_dual_mode               = BT_DUAL_MODE_EN;
bool cfg_bt_tws_mode                = 0;
uint8_t cfg_bt_scan_ctrl_mode       = BT_DISCOVER_CTRL_EN;
bool cfg_bt_simple_pair_mode        = BT_SIMPLE_PAIR_EN;

uint16_t cfg_bt_support_profile     = BT_PROFILE;
uint16_t cfg_bt_support_codec       = BT_CODEC;

uint8_t cfg_bt_a2dp_feature         = A2DP_FEATURE;
uint8_t cfg_bt_a2dp_feature1        = A2DP_FEATURE1;

uint8_t cfg_bt_hfp_feature          = HFP_FEATURE;
uint8_t cfg_bt_hfp_feature1         = HFP_FEATURE1;

uint8_t cfg_bt_hid_type             = BT_HID_TYPE;

uint8_t cfg_bt_connect_times        = 2;                                //按键回连重试次数, 5.12s * n
uint8_t cfg_bt_pwrup_connect_times  = BT_POWER_UP_RECONNECT_TIMES;      //上电回连重试次数, 5.12s * n
uint16_t cfg_bt_sup_to_connect_times = BT_TIME_OUT_RECONNECT_TIMES;     //超时断线回连重试次数, 5.12s * n, 设置(-1)为一直回连

bool cfg_bt_voip_reject_en          = BT_VOIP_REJECT_EN;
bool cfg_bt_hfp_switch_en           = BT_HFP_SWITCH_EN;

bool cfg_bt_emit_mode               = true;
bool cfg_bt_connect_name_en         = false;
bool cfg_bt_sleep_timer_en          = false;
uint16_t cfg_sleep_normal_wakeup_delay = 2000;      //单位us


#if BT_LINK_INFO_PAGE1_EN
uint8_t const cfg_bt_link_info_items = 8;   //保存回连信息的个数（最小1，最大8）
#endif

// uint8_t cfg_bb_rf_gfsk_pwr = 127;     //调节GFSK TXPWR，范围：90~127
/* cfg_bb_rf_path_gain 由 libplatform.a(rf_rapid.o) 提供，勿在 bt.c 重复定义 */

#if 0//BT_RF_EXT_CTL_EN
const uint8_t cfg_bb_rf_ctl = BIT(0);   //bit0=RF_EXT_CTL_EN

AT(.com_text.isr.txrx)
void bb_rf_ext_ctl_cb(u32 rf_sta)
{

//    printf(w_str);
	if(rf_sta & BIT(8)) {	//tx on
        PWRCON2 = (PWRCON2 & (~0xf)) | 0xa; //1.35V
	}
	if(rf_sta & BIT(9)){	//tx down
        PWRCON2 = (PWRCON2 & (~0xf)) | 0x7; //1.25V
	}
	if(rf_sta & BIT(10)){	//rx on
	    PWRCON2 = (PWRCON2 & (~0xf)) | 0xa; //1.35V
	}
	if(rf_sta & BIT(11)){	//rx down
        PWRCON2 = (PWRCON2 & (~0xf)) | 0x7; //1.25V
	}
}
#endif

//自定义蓝牙类别图标，根据需要选择
u32 bt_get_class_of_device(void)
{
//    if(cfg_bt_work_mode == MODE_BQB_RF_BREDR) {
//        return 0x0000;
//    }
#if BT_HID_TYPE == 2
    return 0x0025c0;    //GamePad           - 游戏手柄
#else
//    return 0x002540;    //Keyboard          - 键盘图标，Android带显示电量，IOS不带电量显示。全部IOS均可连接HID拍照。
//    return 0x240418;    //HeadPhone         - 耳机图标，Android和IOS均带电量显示。
//    return 0x240404;    //WearableHeadset   - 耳机图标，Android和IOS均带电量显示。（默认使用）
    return 0x240404;    //WearablePager     - 手表图标
#endif
}

//PIN配对码（最多16个字符），默认"0000"
//#if !BT_SIMPLE_PAIR_EN
//const char *bt_get_pin_code(void)
//{
//    return "0000";
//}
//#endif

//回连间隔（N+3000）mS，间隔越大，下一次回连越慢，更容易被其他手机搜索连接，关闭TWS 功能时 N应大于等于2000
//u32 bt_get_conn_fail_delay(void)
//{
//    return 200;
//}

//是否需要一直回连手机
//bool bt_is_always_reconn(void)
//{
//    return false;
//}

//自定义回连方式，order为回连信息序号
#if BT_EMIT_EN
static uint8_t connect_addr[6];
static char connect_name[32];

static char *bt_get_allow_connect_name(void)
{
    return xcfg_cb.bt_connect_name;
}

void bt_set_connect_addr(uint8_t *bd_addr)
{
    memcpy(connect_addr, bd_addr, 6);
}

void bt_set_connect_name(char *name, u8 len)
{
    if (len > 32) {
        len = 32;
    }

    memcpy(connect_name, name, len);
}

uint8_t bt_get_connect_addr(uint8_t *bd_addr, uint16_t *times)
{
    memcpy(bd_addr, connect_addr, 6);
    return 1;
}

char *bt_get_connect_name(void)
{
    char *name = NULL;

    if (xcfg_cb.bt_connect_name_en) {
        name = bt_get_allow_connect_name();
    } else {
        name = connect_name;
    }

    return name;
}
#endif // BT_EMIT_EN


//是否支持根据AVRCP快速上报播放暂停状态，避免支持播放暂停快速切换功能导致ios播放微信小视频无声
//bool bt_emit_music_status_according_to_avrcp(void)
//{
//    return true;
//}

const char *bt_get_local_name(void)
{
    return xcfg_cb.bt_name;
}

void bt_get_local_bd_addr(u8 *addr)
{
#if LE_SM_SC_EN
    memcpy(addr, xcfg_cb.bt_addr, 6);
    if (!app_phone_type_get()) {
        addr[5] ^= 0x55;
    }
#elif BT_LOCAL_ADDR
    param_random_key_read(&addr[2]);
    addr[0] = 0x41;
    addr[1] = 0x42;
#else
    memcpy(addr, xcfg_cb.bt_addr, 6);
#endif
}

u32 bt_get_rand_seed(void)
{
    return sys_cb.rand_seed;
}

void bt_get_link_info(void *buf, u16 addr, u16 size)
{
    //printf("bt_read: %04x,%04x, %08lx\n", addr, size, BT_CM_PAGE(addr));
    if ((addr + size) <= PAGE_DATA_SIZE) {
    	cm_read(buf, BT_CM_PAGE(addr), size);
#if BT_LINK_INFO_PAGE1_EN
	} else {
        cm_read(buf, BT_CM_PAGE1(addr - link_info_page_size), size);
#endif
	}
    //print_r(buf, size);
}

void bt_put_link_info(void *buf, u16 addr, u16 size)
{
    //printf("bt_write: %04x,%04x, %08lx\n", addr, size, BT_CM_PAGE(addr));
    //print_r(buf, size);
    if ((addr + size) <= PAGE_DATA_SIZE) {
    	cm_write(buf, BT_CM_PAGE(addr), size);
#if BT_LINK_INFO_PAGE1_EN
	} else {
    	cm_write(buf, BT_CM_PAGE1(addr - link_info_page_size), size);
#endif
	}
}

void bt_get_ext_link_info(void *buf, u16 addr, u16 size)
{
    //printf("bt_read: %04x,%04x, %08lx\n", addr, size, BT_CM_PAGE(addr));
    cm_read(buf, EXT_CM_PAGE(addr), size);
    //print_r(buf, size);
}

void bt_put_ext_link_info(void *buf, u16 addr, u16 size)
{
    //printf("bt_write: %04x,%04x, %08lx\n", addr, size, BT_CM_PAGE(addr));
    //print_r(buf, size);
    cm_write(buf, EXT_CM_PAGE(addr), size);
}

void bt_sync_link_info(void)
{
    cm_sync();
}

void bt_call_volume_change(u8 up_flag)
{
    if ((up_flag) && (sys_cb.hfp_vol < 15)) {
        bt_ctrl_msg(BT_CTL_VOL_UP);
    } else if ((!up_flag) && (sys_cb.hfp_vol > 0)) {
        bt_ctrl_msg(BT_CTL_VOL_DOWN);
    } else {
        return;
    }
}

void bt_volume_up(void)
{
    if (sys_cb.incall_flag) {
        bt_call_volume_change(1);
    } else {
        bsp_set_volume(bsp_volume_inc(sys_cb.vol));
        bsp_bt_vol_change();
        printf("volume: %d\n", sys_cb.vol);
    }
}

void bt_volume_down(void)
{
    if (sys_cb.incall_flag) {
        bt_call_volume_change(0);
    } else {
        bsp_set_volume(bsp_volume_dec(sys_cb.vol));
        bsp_bt_vol_change();
        printf("volume: %d\n", sys_cb.vol);
    }
}

void bt_set_music_sta(u8 status)
{
    if (status == 1) {
        bt_cb.music_playing = true;
    } else {
        bt_cb.music_playing = false;
    }
}

#if BT_FCC_TEST_EN || BT_DUT_MODE_EN
uint8_t ble_set_delta_gain(void) {
    return 0;           //设置BLE与BT的功率差值
}
#endif
