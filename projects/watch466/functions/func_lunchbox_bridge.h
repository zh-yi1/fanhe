/**
 * @file    func_lunchbox_bridge.h
 * @brief   饭盒协议翻译层 (BLE ↔ UART)
 *
 * 桥模式(LB_BRIDGE_MODE=1)下，MCU 作为 BLE↔UART 翻译桥，
 * 将 APP 的 BLE 帧翻译为 UART 帧发往加热模块，反之亦然。
 */
#ifndef __FUNC_LUNCHBOX_BRIDGE_H
#define __FUNC_LUNCHBOX_BRIDGE_H

#include "include.h"

#if FUNC_LUNCHBOX_UART_EN

/**
 * @brief BLE 命令字 → UART 命令字映射
 * @param ble_cmd  BLE 命令字
 * @return UART 命令字, 0x00 表示不转发
 */
u8 lb_ble_cmd_to_uart_cmd(u8 ble_cmd);

/**
 * @brief UART 命令字 → BLE 命令字映射
 * @param uart_cmd  UART 命令字
 * @param is_async  true=异步状态上报
 * @return BLE 命令字
 */
u8 lb_uart_cmd_to_ble_cmd(u8 uart_cmd, bool is_async);

/**
 * @brief BLE帧 → UART帧翻译 (完整帧, 含帧头+校验)
 * @param rx        BLE 接收帧(已解析)
 * @param out_buf   输出缓冲区
 * @param out_len   输出数据长度
 * @return true=翻译成功, false=不转发
 */
bool lb_translate_ble_to_uart(lb_rx_frame_t *rx, u8 *out_buf, u16 *out_len);

/**
 * @brief UART帧 → BLE帧翻译 (完整帧, 含帧头+校验)
 * @param rx        UART 接收帧(已解析)
 * @param out_buf   输出缓冲区
 * @param out_len   输出数据长度
 * @return true=翻译成功, false=不转发
 */
bool lb_translate_uart_to_ble(lb_rx_frame_t *rx, u8 *out_buf, u16 *out_len);

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __FUNC_LUNCHBOX_BRIDGE_H
