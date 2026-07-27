/**
 * @file    func_lunchbox_uart_internal.h
 * @brief   饭盒协议内部共享头文件 — 各子系统(.c)之间的 extern 声明
 * @note    **仅** func_lunchbox_uart*.c 内部引用, 不得被外部模块 include
 *
 * !! 串口栈重写中: 本头声明的共享变量原定义在旧 func_lunchbox_uart.c,
 * !! 当前多数定义已随重写移除, 随业务功能重建时逐项恢复或改为访问接口。
 */
#ifndef __FUNC_LUNCHBOX_UART_INTERNAL_H
#define __FUNC_LUNCHBOX_UART_INTERNAL_H

#include "func_lunchbox_uart.h"

//-----------------------------------------------------------------------------
// 共享状态变量 (定义在 func_lunchbox_uart.c)
// 注: 帧缓冲/BLE通道/配对表已私有化到 func_lunchbox_proto.c, 经访问器使用
//-----------------------------------------------------------------------------

// 桥模式: 0x01 产品信息查询挂起状态
extern bool lb_product_info_pending;
extern u8   lb_product_info_msg_flag;
extern u32  lb_product_info_pend_tick;

// BLE 时间同步 (APP 同步用)
// 当 APP 通过 0x01(产品信息查询) 或 0x03(状态上报回传) 下发权威时间戳时，
// 同时记录当时的 RTCCNT，
// 后续通过 lb_get_unix_time() 推算当前时间 = synced_unix_ts + (RTCCNT - synced_rtccnt)
extern u32  lb_synced_unix_ts;       // APP 同步的权威 Unix 时间戳
extern u32  lb_synced_rtccnt;        // 同步时的 RTCCNT 值
extern bool lb_has_ble_ts;           // 是否已收到过 APP 时间同步

// BLE 连接后等待 APP 时间戳应答，收到后再发送预设到加热模块
extern bool lb_ble_presets_pending;

// 获取当前 Unix 时间戳: 已同步则用权威时间推算, 否则用本地 RTC 偏移
u32 lb_get_unix_time(void);

// 设备信息 (产品信息查询应答)
extern lb_device_info_t lb_dev_info;

// 加热任务状态
extern bool lb_heat_task_active;      // 加热模块正在加热
extern bool lb_heat_lcd_active;       // LCD 已下发加热/保温
extern bool lb_keep_warm_active;      // 保温模式激活

// 模式预设 (温度档位 + 时长)
extern u8 lb_mode_temp[6];
extern u8 lb_mode_duration[6];

//-----------------------------------------------------------------------------
// 共享函数 (定义在 func_lunchbox_uart.c)
//-----------------------------------------------------------------------------

// lb_checksum / lb_dp_encode_* / lb_dp_dump_hex / lb_ble_dump_frame /
// lb_data_is_key_notify → 声明见 func_lunchbox_proto.h
// lb_uart_send_raw* → 声明见 func_lunchbox_uart.h (待重建)

// CRC32 (OTA/桥模式 CRC 校验用, 定义在 func_lunchbox_ota.c)
u32 lb_crc32(const void *data, u32 len, u32 crc);

// UART 原始消息标志 (LCD发送时自增, 待重建)
extern u8   lb_uart_raw_msg_flag;

// 产品信息查询处理 (BLE 桥模式回调用)
u8 lb_handler_product_info(lb_rx_frame_t *rx);

// OTA 升级命令处理器 (定义在 func_lunchbox_ota.c)
u8 lb_handler_ota_start(lb_rx_frame_t *rx);
u8 lb_handler_ota_data(lb_rx_frame_t *rx);
u8 lb_handler_ota_end(lb_rx_frame_t *rx);

// OTA 目标设备识别 (桥模式 BLE RX 路由用)
u8 lb_ota_get_target(lb_rx_frame_t *rx);

// 桥模式: 转发加热模块 OTA 时累积 CRC32
extern u32  lb_ota_uart_crc32;
extern bool lb_ota_uart_crc_active;

// 保温指令的 msg_flag (用于等待应答, 过滤过时数据)
extern u8 lb_keep_warm_msg_flag;

#endif // __FUNC_LUNCHBOX_UART_INTERNAL_H
