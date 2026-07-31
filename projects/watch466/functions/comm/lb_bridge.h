/**
 * @file    lb_bridge.h
 * @brief   饭盒协议翻译层 (BLE ↔ UART)
 *
 * MCU 作为 BLE↔UART 翻译桥，
 * 将 APP 的 BLE 帧翻译为 UART 帧发往加热模块，反之亦然。
 * 转发队列(异步一发一收+超时重试)在 lb_uart_app.c (lb_bridge_forward)。
 */
#ifndef __LB_BRIDGE_H
#define __LB_BRIDGE_H

#include "include.h"
#include "lb_proto.h"   // lb_rx_frame_t / 帧常量

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// 命令字/数据翻译
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------
// 时间服务 — 翻译层组 UART 帧时前拼时间戳 DataPoint (dpid=11) 用
// (APP 权威时间优先, 未同步回退本地 RTC 推算)
//-----------------------------------------------------------------------------

// RTCCNT 从 2020-01-01 起算(秒), 且存的是北京时间(UTC+8)
// 北京时间 2020-01-01 00:00:00 的 Unix 时间戳 = 1577836800 - 8*3600
#define LB_RTC_UNIX_OFFSET      (1577836800u - 8*3600)   // = 1577808000

/** @brief 记录权威时间戳 (加热模块确认/推送后由通信层调用) */
void lb_time_set_synced(u32 unix_ts);

/** @brief 是否已收到过 APP 权威时间 */
bool lb_time_is_synced(void);

/** @brief 当前 Unix 时间戳: 已同步用权威时间推算, 否则本地 RTC + 偏移 */
u32 lb_get_unix_time(void);

/** @brief 屏幕显示用的年月日时分 (未同步时退回本机 RTC) */
tm_t lb_get_display_tm(void);

//-----------------------------------------------------------------------------
// CRC32 (加热模块 OTA 透传时累积校验用; 主MCU OTA 模块移植时可复用)
//-----------------------------------------------------------------------------

/**
 * @brief CRC32 校验码计算 (兼容 zlib/uzlib 算法, 支持增量计算)
 *
 *   crc = lb_crc32(buf1, len1, 0xffffffff);        // 第一段
 *   crc = lb_crc32(buf2, len2, crc);               // 续算
 *   final = crc ^ 0xffffffff;                      // 取反得最终值
 */
u32 lb_crc32(const void *data, u32 len, u32 crc);

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __LB_BRIDGE_H
