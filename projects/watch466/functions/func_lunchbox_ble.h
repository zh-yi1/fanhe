/**
 * @file    func_lunchbox_ble.h
 * @brief   饭盒 BLE 通道 (蓝牙接收/发送)
 *
 * BLE 数据入口: APP 通过 BLE GATT 写入饭盒协议帧，
 * 由本模块解析、分发（本地模式）或翻译转发到 UART（桥模式）。
 */
#ifndef __FUNC_LUNCHBOX_BLE_H
#define __FUNC_LUNCHBOX_BLE_H

#include "include.h"

#if FUNC_LUNCHBOX_UART_EN

/** @brief 注册 BLE 发送函数 (lb_ble_tx_fn_t 定义在 func_lunchbox_uart.h) */
void lunchbox_ble_set_tx_fn(lb_ble_tx_fn_t fn);

/** @brief 处理 BLE 接收到的饭盒协议帧，自动校验+分发给命令处理器/翻译转发 */
void lunchbox_ble_rx_handle(u8 *data, u16 len);

/** @brief 累积缓冲区是否有待处理数据 */
bool lunchbox_ble_rx_pending(void);

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __FUNC_LUNCHBOX_BLE_H
