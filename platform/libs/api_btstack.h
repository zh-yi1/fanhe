/*
 *  api_btstack.h
 *
 *  Created by zoro on 2017-8-24.
 *
 *  Note:the file is copied from the btstack\btconfig path.
 */

#ifndef _API_BTSTACK_H
#define _API_BTSTACK_H

#ifdef __cplusplus
extern "C" {
#endif

//工作模式
#define MODE_NORMAL                     0       //正常连接模式，关闭DUT测试
#define MODE_NORMAL_DUT                 1       //正常连接模式，使能DUT测试
#define MODE_FCC_TEST                   2       //FCC测试模式，通过串口通信
#define MODE_BQB_RF_BREDR               3       //BR/EDR DUT测试模式，通过蓝牙通信
#define MODE_BQB_RF_BLE                 4       //BLE DUT测试模式，通过串口通信
#define MODE_IODM_TEST                  5       //IODM测试模式，通过串口通信

//蓝牙功能
#define PROF_A2DP                       0x0007  //蓝牙音乐功能
#define PROF_HID                        0x0018  //蓝牙键盘功能
#define PROF_HFP                        0x0060  //蓝牙通话功能
#define PROF_SPP                        0x0080  //蓝牙串口功能
#define PROF_PBAP                       0x0100  //蓝牙电话本功能
#define PROF_HSP                        0x0200
#define PROF_MAP                        0x0400  //蓝牙短信功能
#define PROF_GATT                       0x0800  //GATT over BREDR功能
#define PROF_PANU                       0x1000  //蓝牙PANU功能
#define PROF_TWS                        0x8000  //TWS功能

//蓝牙状态
enum {
    BT_STA_OFF,                                 //蓝牙模块已关闭
    BT_STA_INITING,                             //初始化中
    BT_STA_IDLE,                                //蓝牙模块打开，未连接

    BT_STA_SCANNING,                            //扫描中
    BT_STA_DISCONNECTING,                       //断开中
    BT_STA_CONNECTING,                          //连接中

    BT_STA_CONNECTED,                           //已连接
    BT_STA_PLAYING,                             //播放
    BT_STA_INCOMING,                            //来电响铃
    BT_STA_OUTGOING,                            //正在呼出
    BT_STA_INCALL,                              //通话中
    BT_STA_OTA,                                 //OTA升级中
};

//蓝牙消息
enum bt_msg_t {
    BT_MSG_CTRL                     = 0,        //蓝牙控制消息
    BT_MSG_COMM,
    BT_MSG_RES1,
    BT_MSG_TWS,                                 //TWS消息
    BT_MSG_ADV0,
    BT_MSG_A2DP,                                //A2DP消息
    BT_MSG_HFP,                                 //HFP消息
    BT_MSG_HSP,                                 //HSP消息
    BT_MSG_HID,                                 //HID消息，KEYPAD按键/CONSUMER按键/触摸屏
    BT_MSG_PBAP,                                //PBAP消息
    BT_MSG_LECLT,                               //BLE CLIENT消息
    BT_MSG_MAP,                                 //MAP消息
    BT_MSG_MAX,
    BT_MSG_RES                      = 0xf0,     //0xf0~0xff保留给传参较多的api
};

//控制消息
enum {
    BT_CTL_VOL_CHANGE               = 0,        //音量调整，之后通过回调函数设置音量
    BT_CTL_PLAY_PAUSE,                          //切换播放、暂停
    BT_CTL_VOL_UP,                              //音乐加音量，之后通过回调函数调节音量
    BT_CTL_VOL_DOWN,                            //音乐减音量，之后通过回调函数调节音量
    BT_CTL_2ACL_PALY_SWITCH,                    //一拖二播放切换
    BT_CTL_GET_ID3_TAG,
    BT_CTL_GET_PLAY_STATUS_INFO,
    BT_CTL_MSC_RES3,

    BT_CTL_CALL_REDIAL,                         //回拨电话（最后一次通话）
    BT_CTL_CALL_REDIAL_NUMBER,                  //回拨电话（从hfp_get_outgoing_number获取号码）
    BT_CTL_CALL_ANSWER_INCOM,                   //接听来电（三通时挂起当前通话）
    BT_CTL_CALL_ANSWER_INCOM_REJ_OTHER,         //接听来电（三通时挂断当前通话）
    BT_CTL_CALL_TERMINATE,                      //挂断通话或来电
    BT_CTL_CALL_SWAP,                           //切换三通电话
    BT_CTL_CALL_PRIVATE_SWITCH,                 //切换私密通话
    BT_CTL_HFP_REPORT_BAT,                      //报告电池电量
    BT_CTL_HFP_MIC_GAIN,                        //设置通话麦克风音量
    BT_CTL_HFP_AT_CMD,                          //发送AT命令（从hfp_get_at_cmd获取命令）
    BT_CTL_HFP_SIRI_SW,                         //唤出/关闭siri
    BT_CTL_HFP_REMOTE_PHONE_NUM,
    BT_CTL_HFP_SWITCH_TO_PHONE,
    BT_CTL_HFP_SWITCH_TO_WATCH,
    BT_CTL_HFP_CUSTOM_AT_CMD,
    BT_CTL_CALL_ANSWER_INCOM_HOLD_OTHER,
    BT_CTL_HFP_RES3,

    BT_CTL_TWS_SWITCH,                          //主从切换
    BT_CTL_NOR_CONNECT,
    BT_CTL_NOR_DISCONNECT,
    BT_CTL_HID_CONNECT,
    BT_CTL_HID_DISCONNECT,
    BT_CTL_CONN_RES1,
    BT_CTL_CONN_RES2,

    BT_CTL_BLE_ADV_DISABLE,                     //关闭BLE 广播
    BT_CTL_BLE_ADV_ENABLE,                      //打开BLE 广播
    BT_CTL_BLE_RES1,
    BT_CTL_BLE_RES2,

    BT_CTL_FOT_RESP,
    BT_CTL_FCC_TEST,
    BT_CTL_LOW_LATENCY_EN,
    BT_CTL_LOW_LATENCY_DIS,
    BT_CTL_EAR_STA_CHANGE,
    BT_CTL_NR_STA_CHANGE,
    BT_CTL_AAP_USER_DATA,
    BT_CTL_A2DP_CONNECT_AND_DISCONNECT,
    BT_CTL_A2DP_PROFILE_EN,
    BT_CTL_A2DP_PROFILE_DIS,
    BT_CTL_HFP_CONNECT,
    BT_CTL_HFP_DISCONNECT,
    BT_CTL_UPD_BT_SCAN_PARAM,
    BT_CTL_FORCE_CLOSE_SIRI,
    BT_CTL_FORCE_OPEN_SIRI,
    BT_CTL_SNIFF_DROP_OUT,
    BT_CTL_HFP_SPK_GAIN,                        //设置通话扬声器音量
    BT_CTL_PANU_CONNECT,
    BT_CTL_PANU_DISCONNECT,
    BT_CTL_HFP_AT_CONFIRM_CODEC,
    BT_CTL_MAX,

    BT_CTL_A2DP_VOLUME_UP           = 0xff0041, //音量加
    BT_CTL_A2DP_VOLUME_DOWN         = 0xff0042, //音量减
    BT_CTL_A2DP_MUTE                = 0xff0043, //MUTE
    BT_CTL_A2DP_PLAY                = 0xff0044, //播放
    BT_CTL_A2DP_STOP                = 0xff0045, //停止
    BT_CTL_A2DP_PAUSE               = 0xff0046, //暂停
    BT_CTL_A2DP_RECORD              = 0xff0047,
    BT_CTL_A2DP_REWIND              = 0xff0048, //快退
    BT_CTL_A2DP_FAST_FORWARD        = 0xff0049, //快进
    BT_CTL_A2DP_EJECT               = 0xff004a,
    BT_CTL_A2DP_FORWARD             = 0xff004b, //下一曲
    BT_CTL_A2DP_BACKWARD            = 0xff004c, //上一曲
    BT_CTL_A2DP_REWIND_END          = 0xff00c8, //结束快退
    BT_CTL_A2DP_FAST_FORWARD_END    = 0xff00c9, //结束快进

    BT_CTL_NO                       = 0xffffff,
};

//需要封装api的控制消息
enum {
    BT_CTL0_OFF                     = 0,        //关闭蓝牙
    BT_CTL0_ON,                                 //打开蓝牙
    BT_CTL0_AUDIO_BYPASS,                       //忽略蓝牙SBC/SCO AUDIO
    BT_CTL0_AUDIO_ENABLE,                       //使能蓝牙SBC/SCO AUDIO
    BT_CTL0_CONNECT_ADDRESS,
    BT_CTL0_RESET_BT_NAME,
    BT_CTL0_RESET_BT_ADDR,
    BT_CTL0_SLEEP_ENTER,
    BT_CTL0_SLEEP_EXIT,
    BT_CTL0_DISCONNECT_ADDRESS,
    BT_CTL0_LATT_SEND,

    BT_CTL0_HID_SEND,
    BT_CTL0_SPP_SEND,
    BT_CTL0_BLE_SEND,
    BT_CTL0_BLE_UPDATE_CONN_PARAM,
    BT_CTL0_BLE_SET_ADV_INTV,
    BT_CTL0_BLE_SET_ADV_DATA,
    BT_CTL0_BLE_DISCONNECT,
    BT_CTL0_BLE_SET_SCAN_RSP_DATA,
    BT_CTL0_BLE_SM_REQ,
    BT_CTL0_BLE_EXCHANGE_MTU_REQ,
    BT_CTL0_OTA_READ,
    BT_CTL0_OTA_STATUS,
    BT_CTL0_SPP_DISCONNECT,
    BT_CTL0_BLE_ANCS_CONNECT,
    BT_CTL0_BLE_ANCS_DISCONNECT,
    BT_CTL0_BLE_SM_REQ_FOR_ANDROID,
    BT_CTL0_BLE_SET_BLE_ADDR,
    BT_CTL0_MAX,
};

enum bt_msg_comm_t {
    COMM_BT_CTL0,                               //无传参的消息
    COMM_BT_START_WORK,                         //蓝牙开始工作
    COMM_BT_SET_SCAN,                           //设置可被发现/可被连接
    COMM_BT_CONNECT,                            //连接蓝牙
    COMM_BT_DISCONNECT,                         //断开蓝牙
    COMM_BT_ABORT_RECONNECT,                    //中止回连
    COMM_BT_ATT_MSG,
    COMM_BT_SET_SCAN_INTV,                      //设置PAGE SCAN PARAM
    COMM_BT_SET_A2DP,
    COMM_BT_ABORT_PAGE_PSCAN,                   //中止PAGE和PAGESCAN
    COMM_LE_AMS_REMOTE_CTRL,                    //LE AMS remote ctl
    COMM_BT_AUTO_SNIFF,
    COMM_BT_ABORT_CONNECT,
};

//pkt
struct txbuf_tag {
    uint8_t *ptr;
    uint16_t len;
    uint16_t handle;
} __attribute__ ((packed)) ;

typedef void (*kick_func_t)(void);

void bt_send_msg_do(uint msg);
#define bt_send_msg(ogf, ocf)                   bt_send_msg_do((ogf<<24) | (ocf))
#define bt_ctrl_msg(msg)                        bt_send_msg(BT_MSG_CTRL, msg)
#define bt_ctrl0_msg(param)                     bt_send_msg(BT_MSG_COMM, (COMM_BT_CTL0<<16) | param)
#define bt_comm_msg(msg, param)                 bt_send_msg(BT_MSG_COMM, (msg<<16) | (u16)param)
#define bt_tws_msg(msg, param)                  bt_send_msg(BT_MSG_TWS, (msg<<16) | (u16)param)
#define bt_hid_msg(msg, param)                  bt_send_msg(BT_MSG_HID, (msg<<16) | (u16)param)
#define bt_pbap_msg(msg, param)                 bt_send_msg(BT_MSG_PBAP, (msg<<16) | (u16)param)

extern uint8_t cfg_bt_work_mode;
extern uint8_t cfg_bt_max_acl_link;
extern bool cfg_bt_dual_mode;
extern bool cfg_bt_tws_mode;
extern uint8_t cfg_bt_scan_ctrl_mode;
extern bool cfg_bt_simple_pair_mode;
extern bool cfg_bt_voip_reject_en;
extern bool cfg_bt_hfp_switch_en;
extern uint16_t cfg_bt_support_profile;
extern uint16_t cfg_bt_support_codec;
extern uint8_t cfg_bt_hid_type;
extern uint8_t cfg_bt_connect_times;
extern uint8_t cfg_bt_pwrup_connect_times;
extern uint16_t cfg_bt_sup_to_connect_times;

extern uint8_t cfg_bt_rf_def_txpwr;
extern uint8_t cfg_bt_page_txpwr;
extern uint8_t cfg_bt_inq_txpwr;
extern uint8_t cfg_ble_page_txpwr;
extern uint8_t cfg_ble_page_rssi_thr;

extern uint8_t cfg_bt_a2dp_feature;
extern uint8_t cfg_bt_a2dp_feature1;
extern uint8_t cfg_bt_hfp_feature;
extern uint8_t cfg_bt_hfp_feature1;
extern uint8_t cfg_bt_tws_pair_mode;
extern uint16_t cfg_bt_tws_feat;
extern uint8_t cfg_bt_tws_not_auto_connect;
extern uint8_t cfg_bt_hci_disc_only_spp;

extern uint8_t cfg_bt_lhdc_codec_feature;

extern const uint16_t link_info_page_size;
extern bool cfg_bt_emit_mode;
extern bool cfg_bt_connect_name_en;

extern volatile bool bt_auto_sniff_en;
extern const uint8_t cfg_bb_bt_opt;

#ifdef __cplusplus
}
#endif

#endif /* _API_BTSTACK_H */

