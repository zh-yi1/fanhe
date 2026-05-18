#ifndef __FUNC_BLE_GATTS_H
#define __FUNC_BLE_GATTS_H

#include "include.h"

#if FUNC_BLE_GATTS_EN

// BLE GATTS Demo Service 初始化（由 ble_init_att() 调用）
void ble_gatts_demo_service_init(void);

// Notify 发送接口
bool ble_gatts_demo_send_notify(u8 *buf, u16 len);

// 屏幕功能函数（由 func_tbl.h 调用）
compo_form_t *func_ble_gatts_form_create(void);
void func_ble_gatts(void);
void func_ble_gatts_message(size_msg_t msg);
void func_ble_gatts_process(void);
void func_ble_gatts_enter(void);
void func_ble_gatts_exit(void);

#endif // FUNC_BLE_GATTS_EN

#endif // __FUNC_BLE_GATTS_H
