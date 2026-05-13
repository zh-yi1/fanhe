/*
 *  api_bt.h
 *
 *  Created by zoro on 2017-8-24.
 *
 *  Note:the file is copied from the btstack\btconfig path.
 */

#ifndef _API_BT_H
#define _API_BT_H

#include "api_btstack.h"

#ifdef __cplusplus
extern "C" {
#endif

//蓝牙特性
#define HFP_BAT_REPORT                  0x01    //是否支持手机电量显示
#define HFP_3WAY_CALL                   0x02    //是否支持三通电话
#define HFP_INBAND_RING_TONE            0x04    //是否支持手机来电铃声
#define HFP_CALL_PRIVATE                0x08    //是否打开强制私密接听
#define HFP_SIRI_CMD                    0x10    //是否打开siri控制命令
#define HFP_EC_AND_NR                   0x20    //是否打开手机端回音和降噪
#define HFP_RING_NUMBER_EN              0x40    //是否支持来电报号
#define HFP_IOS_FLAG                    0x04    //是否iso系统
#define A2DP_AVRCP_VOL_CTRL             0x01    //是否支持手机音量控制同步
#define A2DP_AVRCP_PLAY_STATUS_IOS      0x02    //是否支持IOS手机播放状态同步，可加快播放暂停响应速度，蓝牙后台建议打开；注意：打开后微信小视频会无声
#define A2DP_AVRCP_PLAY_STATUS          0x04    //是否支持手机播放状态同步，可加快播放暂停响应速度，蓝牙后台建议打开
#define A2DP_RESTORE_PLAYING            0x08    //是否支持掉线回连后恢复播放
#define A2DP_AVDTP_DELAY_REPORT         0x10    //是否支持AVDTP delay report功能
#define A2DP_AVDTP_DYN_LATENCY          0x20    //是否支持动态延迟控制功能
#define A2DP_AVDTP_EXCEPT_REST_PLAY     0x40    //是否支持异常复位后恢复连接和播放状态功能
#define A2DP_IOS_FLAG                   0x80    //是否iso系统

//a2dp特性扩展1
#define A2DP_AVRCP_RECORD_DEVICE_VOL    0x01    //分别记录不同连接设备的音量
#define A2DP_RESET_DEVICE_VOL           0x02    //不支持音量同步手机，连接恢复音量
//hfp特性扩展
#define HFP_RECORD_DEVICE_VOL           0x01    //分别记录不同连接设备的音量

//蓝牙编解码
#define CODEC_SBC                       0x01
#define CODEC_AAC                       0x02
#define CODEC_MSBC                      0x04
#define CODEC_PLC                       0x08

//通话状态
enum {
    BT_CALL_IDLE,                               //
    BT_CALL_INCOMING,                           //来电响铃
    BT_CALL_OUTGOING,                           //正在呼出
    BT_CALL_ACTIVE,                             //通话中
    BT_CALL_3WAY_CALL,                          //三通电话或两部手机通话
};

//蓝牙通知
enum {
    BT_NOTICE_INIT_FINISH,                      //蓝牙初始化完成
    BT_NOTICE_CONNECT_START,                    //开始回连手机, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_CONNECT_FAIL,                     //回连手机失败, param[0]=reason, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_DISCONNECT,                       //蓝牙断开,     param[0]=feat,index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_LOSTCONNECT,                      //蓝牙连接丢失, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_CONNECTED,                        //蓝牙连接成功, param[0]=feat,index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_SCO_SETUP,
    BT_NOTICE_SCO_FAIL,
    BT_NOTICE_SCO_KILL,
    BT_NOTICE_INCOMING,                         //来电
    BT_NOTICE_RING,                             //来电响铃
    BT_NOTICE_OUTGOING,                         //去电
    BT_NOTICE_CALL_NUMBER,                      //来电/去电号码
    BT_NOTICE_INCALL,                           //建立通话
    BT_NOTICE_ENDCALL,                          //结束通话
    BT_NOTICE_NETWORK_CALL,                     //网络通话
    BT_NOTICE_PHONE_CALL,                       //手机通话

    BT_NOTICE_SET_SPK_GAIN,                     //设置通话音量
    BT_NOTICE_CALL_CHANGE_DEV,                  //1拖2时改变了通话设备
    BT_NOTICE_MUSIC_PLAY,                       //蓝牙音乐开始播放
    BT_NOTICE_MUSIC_STOP,                       //蓝牙音乐停止播放
    BT_NOTICE_MUSIC_CHANGE_VOL,                 //手机端改变蓝牙音乐音量, param[0]=down/up, param[1]=index, param[7:2]=bd_addr
    BT_NOTICE_MUSIC_SET_VOL,                    //手机端设置蓝牙音乐音量, param[0]=a2dp_vol, param[1]=index, param[7:2]=bd_addr
    BT_NOTICE_MUSIC_CHANGE_DEV,                 //1拖2时改变了播放设备, 例如从A手机切换到B手机, param[0]=a2dp_vol, param[1]=index, param[7:2]=bd_addr
    BT_NOTICE_HID_CONN_EVT,                     //HID服务连接事件
    BT_NOTICE_A2DP_CONN_EVT,                    //A2DP服务连接事件
    BT_NOTICE_HFP_CONN_EVT,                     //HFP服务连接事件
    BT_NOTICE_RECON_FINISH,                     //回连完成, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_ABORT_STATUS,                     //中止状态, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_NORCONNECT_FAIL,                  //手机发起连接到一半失败, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_LOW_LATENCY_STA,                  //低延时状态切换

    ///PBAP相关消息
    BT_NOTICE_PBAP_CONNECTED,
    BT_NOTICE_PBAP_GET_PHONEBOOK_SIZE_COMPLETE, //获取电话薄数量完成
    BT_NOTICE_PBAP_PULL_PHONEBOOK_COMPLETE,     //获取电话薄完成
    BT_NOTICE_PBAP_DISCONNECT,

    BT_NOTICE_HFP_HF_ERROR,                     //通话错误，比如拨打错误电话号码
    BT_NOTICE_FAST_MUSIC_STATUS,                //快速上报音乐播放暂停状态
    BT_NOTICE_AUTH_FAIL,                        //上报AUTH失败消息

    BT_NOTICE_MAP_CONNECT_SUCCESS,
    BT_NOTICE_MAP_CONNECT_FAIL,
    BT_NOTICE_MAP_DISCONNECTED,
    BT_NOTICE_MAP_GET_FINISH,
    BT_NOTICE_MAP_GET_FAIL,

    BT_NOTICE_TWS_SEARCH_FAIL = 0x40,           //搜索TWS失败, param[0]=reason(0=timeout, 0x0B=conn_exists, 0x0C=cmd_disallow)
    BT_NOTICE_TWS_CONNECT_START,                //开始回连TWS, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_TWS_CONNECT_FAIL,                 //TWS回连失败, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_TWS_DISCONNECT,                   //TWS牙断开,   param[0]=feat,index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_TWS_LOSTCONNECT,                  //TWS连接丢失, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_TWS_CONNECTED,                    //TWS连接成功, param[0]=feat,index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_TWS_INIT_VOL,                     //TWS设置副耳初始音乐音量
    BT_NOTICE_TWS_HID_SHUTTER,                  //远端TWS拍照键
    BT_NOTICE_TWS_USER_KEY,                     //TWS自定义按键
    BT_NOTICE_TWS_SET_OPERATION,
    BT_NOTICE_TWS_STATUS_CHANGE,
    BT_NOTICE_TWS_ROLE_CHANGE,                  //主从角色变换
    BT_NOTICE_TWS_RES5,
    BT_NOTICE_TWS_RES6,
    BT_NOTICE_TWS_RES7,
    BT_NOTICE_TWS_WARNING,
};

//param[0]=feat,index
enum
{
    FEAT_TWS_FLAG       = 0x80,
    FEAT_TWS_ROLE       = 0x40,
    FEAT_TWS_MUTE_FLAG  = 0x20,
    FEAT_TWS_FIRST_ROLE = 0x10,
    FEAT_INCOME_CON     = 0x08,
    FEAT_FIRST_CON      = 0x04,
    FEAT_INDEX_MASK     = 0x03,
};

enum bt_msg_hid_t {
    HID_KEYBOARD,
    HID_CONSUMER,
    HID_TOUCH_SCREEN,
    HID_MOUSE,
    HID_TIKTOK,                                 //抖音神器
};

enum bt_msg_pbap_t {
    BT_PBAP_CTRL,
    BT_PBAP_SELECT_PHONEBOOK,
    BT_PBAP_GET_PHONEBOOK_SIZE,
    BT_PBAP_PULL_PHONEBOOK,
    BT_PBAP_SET_PATH,
};

//hid
enum {
    HID_MOUSE_BUTTON_1,
    HID_MOUSE_BUTTON_MAX,

    HID_MOUSE_UP_SLIDE    = 0x10,
    HID_MOUSE_DOWN_SLIDE,

    HID_MOUSE_WHEEL_UP    = 0xF0,
    HID_MOUSE_WHEEL_DOWN,
    HID_MOUSE_ACPAN_UP,
    HID_MOUSE_ACPAN_DOWN,

    HID_MOUSE_MAX,
};

//control
void bb_run_loop(void);
void bt_fcc_init(void);
void bt_init(void);                             //初始化蓝牙变量
int bt_setup(void);                             //打开蓝牙模块
void bt_off(void);                              //关闭蓝牙模块
void bt_wakeup(void);                           //唤醒蓝牙模块
void bt_start_work(uint8_t opcode, uint8_t scan_en); //蓝牙开始工作，opcode: 0=回连, 1=不回连
void bt_thread_check_trigger(void);
void bt_audio_bypass(void);                     //蓝牙SBC/SCO通路关闭
void bt_audio_enable(void);                     //蓝牙SBC/SCO通路使能
void bt_get_stack_local_name(char* name);
void bt_set_stack_local_name(const char* name);
void bt_set_sco_far_delay(void *buf, uint size, uint delay);

void bt_set_scan(uint8_t scan_en);              //打开/关闭可被发现和可被连接, bit0=可被发现, bit1=可被连接
uint8_t bt_get_scan(void);                      //获取设置的可被发现可被连接状态（已连接时设置完不会立即生效，需要等断开连接）
uint8_t bt_get_curr_scan(void);                 //获取当前可被发现可被连接状态
void bt_connect(void);                          //蓝牙设备回连, 回连次数在cfg_bt_connect_times配置
void bt_connect_address(void);                  //蓝牙设备回连地址, 回连地址在bt_get_connect_addr函数设置
void bt_disconnect_address(void);               //蓝牙设备断开地址, 断开地址在bt_get_disconnect_addr函数设置
void bt_disconnect(uint reason);                //蓝牙设备断开, reason: 0=单独断开（入仓）; 1=断开并同步关机（按键/自动关机）;用户单独调用断开，并不关机reason=0xff
void bt_hid_connect(void);                      //蓝牙HID服务回连
void bt_hid_disconnect(void);                   //蓝牙HID服务断开
int bt_hid_is_connected(void);
bool bt_hid_is_ready_to_discon(void);

//status
uint bt_get_disp_status(void);                  //获取蓝牙的当前显示状态, V060
uint bt_get_status(void);                       //获取蓝牙的当前状态
uint8_t bt_get_scan(void);                      //判断当前可被连接可被扫描状态
uint8_t bt_get_curr_scan(void);                 //获取实时可被连接可被扫描状态
uint bt_get_call_indicate(void);                //获取通话的当前状态
uint bt_get_siri_status(void);                  //获取SIRI当前状态, 0=SIRI已退出, 1=SIRI已唤出
uint bt_get_force_siri_status(void);            //获取SIRI当前状态，0=SIRI已退出, 1=SIRI已唤出，配合bt_hfp_siri_close和bt_hfp_siri_open使用
bool bt_is_calling(void);                       //判断是否正在通话
bool bt_is_playing(void);                       //判断是否正在播放
bool bt_is_ring(void);                          //判断是否正在响铃
bool bt_is_testmode(void);                      //判断当前蓝牙是否处于测试模式
bool bt_is_sleep(void);                         //判断蓝牙是否进入休眠状态
bool bt_is_allow_sleep(void);                   //判断蓝牙是否允许进入休眠状态
bool bt_is_connected(void);                     //判断蓝牙是否已连接（TWS副耳被连接，或主耳与手机已连接）
bool bt_is_ios_device(void);                    //判断当前播放的是否ios设备
bool bt_is_support_vol_ctrl(void);              //判断当前播放的是否支持音量同步
uint32_t bt_sleep_proc(void);
void bt_enter_sleep(void);
void bt_exit_sleep(void);
void bt_updata_local_name(const char *bt_name);
bool sco_is_connected(void);
bool bt_sco_is_msbc(void);                      //判断当前通话是否是宽带通话
bool sbc_is_bypass(void);
bool bt_is_low_latency(void);                   //判断蓝牙是否在低延时状态
bool bt_is_silence(void);
bool bt_decode_is_aac(void);                    //判断蓝牙解码是否是aac
uint8_t bt_get_connected_num(void);             //一拖二获取当前连接了几台设备
uint8_t bt_get_cur_a2dp_media_index(void);      //一拖二获取当前播放设备的index
bool bt_a2dp_profile_completely_connected(void);//根据profile来判断A2DP是否已连接完全
void bt_reset_addr(void);                       //重置bt地址, 值通过bt_get_local_bd_addr设置;

//info
bool bt_get_link_btname(uint8_t index, char *name, uint8_t max_size); //index: 0=link0, 1=link1, 0xff=auto(default link0)
void bt_get_local_bd_addr(u8 *addr);
void bt_read_conn_rssi(void);                   //获取当前连接RSSI


void bt_nor_connect(void);                      //回连手机
void bt_nor_disconnect(void);                   //断开手机
bool bt_nor_get_link_info(uint8_t *bd_addr);    //获取手机配对信息，bd_addr=NULL时仅查询是否存在回连信息
bool bt_nor_get_link_info_addr(uint8_t *bd_addr, uint8_t order);    //获取第n个手机配对信息，bd_addr=NULL时仅查询是否存在回连信息
void bt_nor_delete_link_info(void);             //删除手机配对信息
void bt_nor_unpair_device(void);                //删除手机配对信息并断开

///四个参数分别为iscan interval，iscan窗大小，pscan interval，pscan窗大小，默认值为4096,18,2048,18
///iscan对应搜索窗，pascn对应连接窗，调整interval跟窗大小会影响对应搜索及连接速度
void bt_update_bt_scan_param(uint16_t inq_scan_int, uint16_t inq_scan_win, uint16_t page_scan_int, uint16_t page_scan_win);
void bt_update_bt_scan_param_default(void);
void ble_send_sm_req_for_android(void);

//蓝牙连接
#define bt_scan_enable()                        bt_set_scan(0x03)                       //打开扫描
#define bt_scan_disable()                       bt_set_scan(0x00)                       //关闭扫描
#define bt_set_scan_param(ps_intv,is_intv)      bt_comm_msg(COMM_BT_SET_SCAN_INTV, (ps_intv << 8) | is_intv)  //设置scan 参数，0x100*(intv+1)
#define bt_abort_reconnect()                    bt_comm_msg(COMM_BT_ABORT_RECONNECT, 0xffff)        //终止回接
#define bt_abort_reconnect_silence(feat)        bt_comm_msg(COMM_BT_ABORT_RECONNECT, (u16)feat)     //终止回接，没有消息回调。feat:0=手机, BT_FEAT_TWS=TWS

//蓝牙音乐
#define bt_music_play()                         bt_ctrl_msg(BT_CTL_A2DP_PLAY)               //播放
#define bt_music_pause()                        bt_ctrl_msg(BT_CTL_A2DP_PAUSE)              //暂停
#define bt_music_play_pause()                   bt_ctrl_msg(BT_CTL_PLAY_PAUSE)              //切换播放/暂停
#define bt_music_stop()                         bt_ctrl_msg(BT_CTL_A2DP_STOP)               //停止
#define bt_music_prev()                         bt_ctrl_msg(BT_CTL_A2DP_BACKWARD)           //上一曲
#define bt_music_next()                         bt_ctrl_msg(BT_CTL_A2DP_FORWARD)            //下一曲
#define bt_music_rewind()                       bt_ctrl_msg(BT_CTL_A2DP_REWIND)             //开始快退
#define bt_music_rewind_end()                   bt_ctrl_msg(BT_CTL_A2DP_REWIND_END)         //结束快退
#define bt_music_fast_forward()                 bt_ctrl_msg(BT_CTL_A2DP_FAST_FORWARD)       //开始快进
#define bt_music_fast_forward_end()             bt_ctrl_msg(BT_CTL_A2DP_FAST_FORWARD_END)   //结束快进
#define bt_music_vol_change()                   bt_ctrl_msg(BT_CTL_VOL_CHANGE)              //调节音乐音量，之后通过回调函数a2dp_vol_set_cb设置音量
#define bt_music_vol_up()                       bt_ctrl_msg(BT_CTL_VOL_UP)                  //音乐加音量，之后通过回调函数a2dp_vol_adj_cb调节音量
#define bt_music_vol_down()                     bt_ctrl_msg(BT_CTL_VOL_DOWN)                //音乐减音量，之后通过回调函数a2dp_vol_adj_cb调节音量
#define bt_music_play_switch()                  bt_ctrl_msg(BT_CTL_2ACL_PALY_SWITCH)        //一拖二切换播放手机
#define bt_music_get_id3_tag()                  bt_ctrl_msg(BT_CTL_GET_ID3_TAG);            //主动查询id3 tag信息
#define bt_music_paly_status_info()             bt_ctrl_msg(BT_CTL_GET_PLAY_STATUS_INFO);   //获取歌曲时长单位ms,当前播放位置,播放状态信息
#define bt_low_latency_enable()                 bt_ctrl_msg(BT_CTL_LOW_LATENCY_EN)          //蓝牙使能低延时
#define bt_low_latency_disable()                bt_ctrl_msg(BT_CTL_LOW_LATENCY_DIS)         //蓝牙关闭低延时
#define bt_fcc_test_start()                     bt_ctrl_msg(BT_CTL_FCC_TEST)                //FCC test模式

//蓝牙通话
#define bt_call_redial_last_number()            bt_ctrl_msg(BT_CTL_CALL_REDIAL)         //电话回拨（最后一次通话）
#define bt_call_answer_incoming()               bt_ctrl_msg(BT_CTL_CALL_ANSWER_INCOM)   //接听电话，三通时挂起当前通话
#define bt_call_answer_incom_rej_other()        bt_ctrl_msg(BT_CTL_CALL_ANSWER_INCOM_REJ_OTHER)     //接听电话，三通时挂断当前通话，1拖2时挂断当前的手机通话
#define bt_call_answer_incom_hold_other()       bt_ctrl_msg(BT_CTL_CALL_ANSWER_INCOM_HOLD_OTHER)    //接听电话，三通时挂起当前通话，1拖2时挂起当前的手机通话
#define bt_call_terminate()                     bt_ctrl_msg(BT_CTL_CALL_TERMINATE)      //挂断电话
#define bt_call_swap()                          bt_ctrl_msg(BT_CTL_CALL_SWAP)           //切换三通电话
#define bt_call_private_switch()                bt_ctrl_msg(BT_CTL_CALL_PRIVATE_SWITCH) //切换私密通话
#define bt_call_redial_number()                 bt_ctrl_msg(BT_CTL_CALL_REDIAL_NUMBER)
#define bt_call_get_remote_phone_number()       bt_ctrl_msg(BT_CTL_HFP_REMOTE_PHONE_NUM) //获取远端号码
#define bt_hfp_siri_switch()                    bt_ctrl_msg(BT_CTL_HFP_SIRI_SW)         //开关SIRI, android需要在语音助手中打开“蓝牙耳机按键启动”, ios需要打开siri功能
#define bt_hfp_report_bat()                     bt_ctrl_msg(BT_CTL_HFP_REPORT_BAT)
#define bt_hfp_set_spk_gain()                   bt_ctrl_msg(BT_CTL_HFP_SPK_GAIN)
#define bt_hfp_send_at_cmd()                    bt_ctrl_msg(BT_CTL_HFP_AT_CMD)
#define bt_hfp_send_custom_at_cmd()             bt_ctrl_msg(BT_CTL_HFP_CUSTOM_AT_CMD)
#define bt_hfp_switch_to_phone()                bt_ctrl_msg(BT_CTL_HFP_SWITCH_TO_PHONE) //通话切到手机
#define bt_hfp_switch_to_watch()                bt_ctrl_msg(BT_CTL_HFP_SWITCH_TO_WATCH) //通话切在手表
#define bt_hfp_siri_close()                     bt_ctrl_msg(BT_CTL_FORCE_CLOSE_SIRI)    //把siri关闭，暴力测试专属，siri状态改用bt_get_force_siri_status
#define bt_hfp_siri_open()                      bt_ctrl_msg(BT_CTL_FORCE_OPEN_SIRI)     //把siri打开，暴力测试专属，siri状态改用bt_get_force_siri_status
#define bt_hfp_send_at_confirm_codec()          bt_ctrl_msg(BT_CTL_HFP_AT_CONFIRM_CODEC)

//服务控制
#define bt_a2dp_profile_en()                    bt_ctrl_msg(BT_CTL_A2DP_PROFILE_EN)     //打开A2DP服务
#define bt_a2dp_profile_dis()                   bt_ctrl_msg(BT_CTL_A2DP_PROFILE_DIS)    //关闭A2DP服务
#define bt_hfp_profile_en()                     bt_ctrl_msg(BT_CTL_HFP_CONNECT)         //打开HFP服务
#define bt_hfp_profile_dis()                    bt_ctrl_msg(BT_CTL_HFP_DISCONNECT)      //关闭HFP服务
#define bt_panu_network_connect()               bt_ctrl_msg(BT_CTL_PANU_CONNECT)        //打开PANU服务
#define bt_panu_network_disconnect()            bt_ctrl_msg(BT_CTL_PANU_DISCONNECT)     //关闭PANU服务

//PBAP
#define bt_pbap_connect()                       bt_pbap_msg(BT_PBAP_CTRL, 1)
#define bt_pbap_disconnect()                    bt_pbap_msg(BT_PBAP_CTRL, 0)
#define bt_pbap_select_phonebook(book, sim)     bt_pbap_msg(BT_PBAP_SELECT_PHONEBOOK, (sim<<8) | (u8)book)
                                                    // sim - 1:选择SIM卡，0:本机
                                                    // book- 0:pb, 1:fav, 2-ich, 3:och, 4-mch, 5-cch, 6-spd
                                                    // 若不配置，则选择默认值为本机pb
#define bt_pbap_get_phonebook_size()            bt_pbap_msg(BT_PBAP_GET_PHONEBOOK_SIZE, 0)
#define bt_pbap_pull_phonebook_whole()          bt_pbap_msg(BT_PBAP_PULL_PHONEBOOK, 0)
#define bt_pbap_pull_phonebook_single(idx)      bt_pbap_msg(BT_PBAP_PULL_PHONEBOOK, idx)
                                                    // 按编号获取联系人信息
                                                    // idx不为零，如果为零则直接获取整个电话本信息
#define bt_pbap_set_path()                      bt_pbap_msg(BT_PBAP_SET_PATH, 0)

typedef uint8_t bd_addr_t[6];


//init
void hfp_hf_init(void);
void a2dp_init(void);
void aap_init(void);
uint8_t sdp_add_service(void *item);
void sdp_rmv_service(uint32_t service_record_handle);
uint bt_get_hfp_feature(void);

//a2dp
bool a2dp_is_playing_fast(void);
uint8_t a2dp_vol_reverse(uint vol);                 //将系统音量转换为a2dp_vol
uint8_t a2dp_vol_conver(uint8_t a2dp_vol);          //将a2dp_vol转换为系统音量级数

void spp_init(void);
void spp_txpkt_init_with_pool(kick_func_t send_func, void *mem_pool, uint8_t total, uint16_t buf_size);
void spp_send_kick(void);
int bt_spp_tx(uint8_t ch, uint8_t *packet, uint16_t len);
void spp_support_mul_server(uint8_t support);
bool spp_is_connect(void);
bool spp_is_connected_with_channel(uint8_t ch);  //判断某一个SPP通路是否连接，目前只有 0,1,2 共3个通路
void spp_disconnect(void);

extern u8* douyin_hid_code;
void hid_device_init(void);
bool bt_hid_send(void *buf, uint len, bool auto_release);                                           //自定义HID数组
bool bt_hid_send_key(uint type, uint keycode);                                                      //标准HID按键
#define bt_hid_key(keycode)                     bt_hid_send_key(HID_KEYBOARD, keycode)              //标准HID键, 如Enter
#define bt_hid_consumer(keycode)                bt_hid_send_key(HID_CONSUMER, keycode)              //自定义HID键, 如VOL+ VOL-
#define bt_hid_touch_screen(keycode)            bt_hid_send_key(HID_TOUCH_SCREEN, keycode)          //触屏
bool bt_hid_touch_screen_set_key(void *ts);

#define bt_hid_finger_select_ios()              bt_hid_msg(HID_TOUCH_SCREEN, 1)                //抖音视频选择IOS系统
#define bt_hid_finger_select_andriod()          bt_hid_msg(HID_TOUCH_SCREEN, 2)                //抖音视频选择andriod系统
/**
 * @brief 模拟触点函数
   注意:IOS 范围是-2047-2048 ，安卓是0-4096;
        IOS设备，x,y是相对位置，比如10,10是相对当前位置移动10,10;
        安卓设备，x，y是绝对位置，10,10是在手机10,10的位置上;
 * @param is_press  1按下，0抬起
 * @param x 模拟触点横坐标
 * @param y 模拟触点纵坐标
 **/
void bt_hid_point_pos(bool is_press, s16 x, s16 y);
void bsp_bt_hid_finger(bool is_press, s16 x, s16 y);
bool hid_mouse_handler(u8 opcode);
//goep
void goep_client_init(void);
//pbap
void pbap_client_init(void);
void bt_pbap_start(u8 type);                    //type：获取本地号码:BIT(0)、获取来电号码:BIT(1)、获取去电号码:BIT(2)、获取未接号码:BIT(3)
void bt_pbap_abort(void);                       //终止电话本的获取
u32 bt_pbap_get_sta(void);                      //获取电话本状态，0：IDLE； 1：获取中
void bt_pbap_lookup_number(char *phone_number); //根据电话号码获取联系人名字
//map
void map_client_init(void);
void bt_map_start(void);                        //MAP的获取
void bt_map_abort(void);                        //终止MAP的获取
//hsp
void hsp_hs_init(void);
void hsp_hs_init_var(void);
void bt_hsp_call_switch(void);                  //挂断/接听
void bt_hsp_sco_conn(void);                     //建立HSP SCO连接
void bt_hsp_sco_disconn(void);                  //断开HSP SCO连接

#ifdef __cplusplus
}
#endif

#endif /* _API_BT_H */

