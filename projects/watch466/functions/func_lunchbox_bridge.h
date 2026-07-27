/**
 * @file    func_lunchbox_bridge.h
 * @brief   饭盒协议翻译层 (BLE ↔ UART)
 *
 * MCU 作为 BLE↔UART 翻译桥，
 * 将 APP 的 BLE 帧翻译为 UART 帧发往加热模块，反之亦然。
 */
#ifndef __FUNC_LUNCHBOX_BRIDGE_H
#define __FUNC_LUNCHBOX_BRIDGE_H

#include "include.h"
#include "func_lunchbox_proto.h"   // lb_rx_frame_t / 帧常量

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
 * @brief BLE帧数据 → UART帧数据翻译 (仅数据区, 组帧由调用方 lb_proto_build_frame)
 * @param rx        BLE 接收帧(已解析)
 * @param out_data  输出数据区缓冲 (须容纳 LB_TXBUF_SIZE)
 * @param out_len   输出数据区长度
 * @return true=翻译成功, false=不转发
 */
bool lb_translate_ble_data_to_uart(lb_rx_frame_t *rx, u8 *out_data, u16 *out_len);

/**
 * @brief UART帧数据 → BLE帧数据翻译 (仅数据区)
 * @param rx        UART 接收帧(已解析)
 * @param ble_cmd   目标 BLE 命令字 (决定数据区格式)
 * @param out_data  输出数据区缓冲 (须容纳 LB_TXBUF_SIZE)
 * @param out_len   输出数据区长度
 * @return true=翻译成功, false=不转发
 */
bool lb_translate_uart_data_to_ble(lb_rx_frame_t *rx, u8 ble_cmd, u8 *out_data, u16 *out_len);

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __FUNC_LUNCHBOX_BRIDGE_H
