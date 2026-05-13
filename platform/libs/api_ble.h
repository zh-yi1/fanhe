/*
 *  api_ble.h
 *
 *  Created by zoro on 2017-8-24.
 *
 *  Note:the file is copied from the btstack\btconfig path.
 */

#ifndef _API_BLE_H
#define _API_BLE_H

#include "api_btstack.h"

#ifdef __cplusplus
extern "C" {
#endif

//BLE GATTS ERR return
#define BLE_GATTS_SUCCESS                          0x00
#define BLE_GATTS_SRVC_TYPE_ERR                    0x01
#define BLE_GATTS_SRVC_RULES_ERR                   0x02
#define BLE_GATTS_SRVC_PROPS_ERR                   0x03
#define BLE_GATTS_SRVC_ATT_FULL_ERR                0x04
#define BLE_GATTS_SRVC_PROFILE_FULL_ERR            0x05

//att property flag
#define ATT_BROADCAST                              0x01
#define ATT_READ                                   0x02
#define ATT_WRITE_WITHOUT_RESPONSE                 0x04
#define ATT_WRITE                                  0x08
#define ATT_NOTIFY                                 0x10
#define ATT_INDICATE                               0x20
#define ATT_AUTHENTICATED_SIGNED_WRITE             0x40
#define ATT_EXTENDED_PROPERTIES                    0x80
#define ATT_DYNAMIC                                0x100
#define ATT_UUID128                                0x200
#define ATT_AUTHENTICATION_REQUIRED                0x400
#define ATT_AUTHORIZATION_REQUIRED                 0x800

#define GATT_CLIENT_CONFIG_NOTIFY                  1
#define GATT_CLIENT_CONFIG_INDICATE                2

//LE状态
enum {
    LE_STA_STANDBY,
    LE_STA_ADVERTISING,                         //正在广播
    LE_STA_CONNECTION,                          //已连接
};

//LE通知
enum {
    LE_NOTICE_CONNECTED,                        //连接成功
    LE_NOTICE_DISCONNECT,                       //断开成功
    LE_NOTICE_CONN_PARAM_UPDATE,                //连接参数更新
    LE_NOTICE_DATA_LEN_CHANGE,                  //收发长度改变
    LE_NOTICE_CLINET_CFG,                       //客户端配置特性

    LE_NOTICE_ANCS_CONN_EVT,                    //ancs client连接事件
    LE_NOTICE_AMS_CONN_EVT,                     //ams  client连接事件

    LE_NOTICE_INDICATION_COMPLETE,              //indicate完成
    LE_NOTICE_INDICATION_TIMEOUT,               //indicate超时
};

typedef enum {
    GAP_RANDOM_ADDRESS_TYPE_OFF = 0,
    GAP_RANDOM_ADDRESS_TYPE_STATIC,
    GAP_RANDOM_ADDRESS_NON_RESOLVABLE,
    GAP_RANDOM_ADDRESS_RESOLVABLE,
} gap_random_address_type_t;

//define group GATT Server Service Types
typedef enum {
    BLE_GATTS_SRVC_TYPE_PRIMARY   = 0x00,     //Primary Service
    BLE_GATTS_SRVC_TYPE_SEVONDARY = 0x01,     //Secondary Service
    BLE_GATTS_SRVC_TYPE_INCLUDE   = 0x02,     //include Type
} ble_gatts_service_type;

//define group GATT Server UUID Types
typedef enum {
    BLE_GATTS_UUID_TYPE_16BIT     = 0x00,     //UUID 16BIT
    BLE_GATTS_UUID_TYPE_128BIT    = 0x01,     //UUID 128BIT
} ble_gatts_uuid_type;

typedef int (*ble_gatt_callback_func)(uint16_t con_handle, uint16_t handle, uint32_t offset, uint8_t *ptr, uint16_t len);

typedef struct {
    uint16_t handle;
} gatts_service_base_st;

//define GATT service uuid
typedef struct {
    uint16_t props;
    uint8_t type;
    const uint8_t *uuid;
} gatts_uuid_base_st;

typedef struct {
    uint16_t client_config;
    uint16_t value_len;
    ble_gatt_callback_func att_write_callback_func;
    ble_gatt_callback_func att_read_callback_func;
    uint8_t *value;
} ble_gatt_characteristic_cb_info_t;

void ble_txpkt_init_with_pool(kick_func_t send_func, void *mem_pool, uint8_t total, uint16_t buf_size);

void ble_set_adv(u8 chanel, u8 type);
void ble_set_adv_interval(u16 interval);
bool ble_set_adv_data(const u8 *adv_buf, u32 size);
bool ble_set_scan_rsp_data(const u8 *scan_rsp_buf, u32 size);
void ble_update_conn_param(u16 interval, u16 latency, u16 timeout);
u8 ble_get_status(void);
void ble_disconnect(void);
bool ble_is_connect(void);
u16 ble_get_gatt_mtu(void);
void ble_set_adv_interval(u16 interval);
void ble_send_kick(void);
int ble_tx_notify(u16 att_handle, u8* buf, uint16_t len);
uint8_t ble_set_delta_gain(void);
void ble_update_conn_param(u16 interval, u16 latency, u16 timeout);
bool ble_set_adv_data(const u8 *adv_buf, u32 size);
void ble_send_sm_req(void);
void ble_exchange_mtu_request(void);
void ble_set_gap_name(char *gap_name, u8 len);
u16 ble_get_conn_interval(void);                //N*1.25ms
u16 ble_get_conn_latency(void);
u16 ble_get_conn_timeout(void);                 //N*10ms
u16 ble_get_adv_interval(void);                 //N*625us

//init gatt
//profile_table: profile cache buf
//cb_info_table_p: call back info cache
void ble_gatts_init(uint8_t *profile_table, uint16_t profile_table_size,
                    ble_gatt_characteristic_cb_info_t **cb_info_table_p,
                    uint16_t gatt_max_att);
int ble_gatts_service_add(ble_gatts_service_type service_type, const uint8_t *service_uuid, ble_gatts_uuid_type uuid_type, uint8_t *service_handle);
int ble_gatts_characteristic_add(const uint8_t *att_uuid, ble_gatts_uuid_type uuid_type, uint16_t props,
                                 uint16_t *att_handle,
                                 ble_gatt_characteristic_cb_info_t *cb_info);
int ble_gatts_descriptor_add(const uint8_t *att_uuid, uint16_t props,
                             uint16_t *att_handle,
                             ble_gatt_characteristic_cb_info_t *cb_info);
bool ble_gatt_init_att_info(uint16_t att_handle, ble_gatt_characteristic_cb_info_t *att_cb_info);
int ble_gatts_inlcude_service_add(ble_gatts_uuid_type uuid_type, const uint8_t *service_uuid, uint16_t start_handle, uint16_t end_handle, uint16_t *att_handle);
int ble_gatts_attribute_add(const uint8_t *att_data, uint16_t *att_handle);
bool ble_gatts_profile_mg_alloc_att_num_check(uint8_t att_num);

#define ble_adv_dis()                           bt_ctrl_msg(BT_CTL_BLE_ADV_DISABLE)
#define ble_adv_en()                            bt_ctrl_msg(BT_CTL_BLE_ADV_ENABLE)


/**
 * @brief 检测BLE Notify的缓存是否满了
 * @param[in] rsvd_num 这个参数的意思是这样的，比如您设置1，如果蓝牙底层可以用来缓存的数量小于或等于1，就返回1表示满了。
 * 这个作用的目的是方便客户预留固定数量的force数量；举个栗子：有两个GATT，一个是Notify状态的，一个是Notfiy 音频数据的，由于音频数据大
 * 会有机会把协议栈填满导致没有buff给Notify状态使用了，所以就引出这个函数用来给音频发送前使用，设置为1则可以保证每次至少有1个buff给Notify那边
 * 而不至于阻塞；
 * return : 1表示设置数量满了，0没满
 **/
bool is_le_buff_full(uint rsvd_num);

/**
 * @brief ams复位，设备端控制手机端播放暂停等
 * opcode: 0播放，1暂停，2播放暂停，3下一曲，4上一去，5音量+，6音量-，详细参考AMS服务
*/
void ble_ams_remote_ctrl(uint8_t opcode);

/**
 * @brief 发起ancs连接(不可与ble_bt_connect交叉使用)
*/
void ble_ancs_start(void);

/**
 * @brief 断开ancs连接(不可与ble_bt_connect交叉使用)
*/
void ble_ancs_stop(void);

/**
 * @brief ANCS Pefrom Notifction Action
 * 控制手机端的操作，比如来电可以控制是否接听，拒接等
 * uid: 通信连接ID，可以通过ANCS回调获取，用于保证是在同一个事件上
 * opcode: 0 确认(接听)  1否定(拒接)
*/
void ble_ancs_remote_action(uint32_t uid, bool opcode);

/*****************************************************************************
 * BLE无连接广播相关（通道0，与BLE连接相互独立）
 *****************************************************************************/
#define BLE_ADV0_EN_BIT                  0x01
#define BLE_ADV0_MS_VAR_BIT              0x02
#define BLE_ADV0_ADDR_PUBIC_BIT          0x04

extern const uint8_t cfg_ble_adv0_en;

void ble_adv0_set_intv(u16 intv);                       //设置广播间隔，单位625us
void ble_adv0_set_ctrl(uint opcode);                    //0=关闭广播, 1=打开广播, 2=更新广播数据（打开时直接广播，关闭时仅更新buffer）
uint8_t ble_adv0_get_adv_en(void);

void ble_adv0_update_adv(void);
void ble_adv0_idx_update(void);

#ifdef __cplusplus
}
#endif

#endif /* _API_BLE_H */

