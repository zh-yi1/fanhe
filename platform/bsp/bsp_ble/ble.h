#ifndef __BLE_H
#define __BLE_H

/**
 * @brief 重置bt地址为bt_get_local_bd_addr的返回值
 **/
void bsp_change_bt_mac(void);

/**
 * @brief 获取本机ble地址
 **/
void ble_get_local_bd_addr(u8 *addr);

bool user_common_ble_send(u8* buf, u32 len, void* send_func);

#endif  //__BLE_H
