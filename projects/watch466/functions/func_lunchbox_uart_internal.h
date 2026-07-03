/**
 * @file    func_lunchbox_uart_internal.h
 * @brief   饭盒协议内部共享头文件 — 各子系统(.c)之间的 extern 声明
 * @note    **仅** func_lunchbox_uart*.c 内部引用, 不得被外部模块 include
 *
 * 所有跨文件的共享变量/函数在此集中声明 extern。
 * 变量定义在 func_lunchbox_uart.c (核心), 其他子系统文件通过此头引用。
 */
#ifndef __FUNC_LUNCHBOX_UART_INTERNAL_H
#define __FUNC_LUNCHBOX_UART_INTERNAL_H

#include "func_lunchbox_uart.h"

//-----------------------------------------------------------------------------
// 共享状态变量 (定义在 func_lunchbox_uart.c)
//-----------------------------------------------------------------------------

// BLE 发送回调 (非 NULL 时帧走 BLE 通道)
extern lb_ble_tx_fn_t lb_ble_tx_fn;

// 命令处理器表 (BLE 本地模式分发用)
extern lb_cmd_handler_t cmd_handler[16];

// 帧发送缓冲区
extern u8 lb_tx_buf[LB_TXBUF_SIZE];

// UART 帧解析缓冲区
extern u8  lb_rx_buf[LB_RXBUF_SIZE];
extern u16 lb_rx_idx;
extern u32 lb_rx_ticks;

// 同步/异步状态
extern bool lb_uart_sync_pending;

// 异步消息自增标志
extern u8 lb_async_msg_flag;

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

// 待处理 BLE 命令路由表 (桥模式)
extern u8 lb_pending_ble_cmd[256];

// 设备信息 (产品信息查询应答)
extern lb_device_info_t lb_dev_info;

// 加热任务状态
extern bool lb_heat_task_active;      // 加热模块正在加热
extern bool lb_heat_lcd_active;       // LCD 已下发加热/保温
extern bool lb_keep_warm_active;      // 保温模式激活

// 模式预设 (温度档位 + 时长)
extern u8 lb_mode_temp[6];
extern u8 lb_mode_duration[6];

// 属性缓存 (本地模式 !LB_BRIDGE_MODE)
#if !LB_BRIDGE_MODE
extern u8  lb_attr_power_switch;
extern u8  lb_attr_heat_mode;
extern u8  lb_attr_battery;
extern u8  lb_attr_charge_status;
extern u32 lb_attr_heat_duration;
extern u32 lb_attr_remain_time;
extern u8  lb_attr_heat_temp;
extern u8  lb_attr_language;
extern u8  lb_attr_fault;
extern u8  lb_attr_heat_enable;
extern u32 lb_attr_mcu_version;
#endif

//-----------------------------------------------------------------------------
// 共享函数 (定义在 func_lunchbox_uart.c)
//-----------------------------------------------------------------------------

// 校验和 (帧校验用)
u8 lb_checksum(u8 *data, u16 len);

// CRC32 (OTA/桥模式 CRC 校验用)
u32 lb_crc32(const void *data, u32 len, u32 crc);

// UART 原始发送 (LCD/加热控制/预约 等模块用)
void lb_uart_send_raw(u8 uart_cmd, u8 *data, u16 data_len);

// DataPoint 编码器
u16 lb_dp_encode_bool(u8 *buf, u8 dpid, u8 val);
u16 lb_dp_encode_enum(u8 *buf, u8 dpid, u8 val);
u16 lb_dp_encode_value(u8 *buf, u8 dpid, u32 val);

// DataPoint 调试打印
void lb_dp_dump_hex(const u8 *data, u16 data_len);

// BLE 帧调试打印 (BLE/桥 模块用)
void lb_ble_dump_frame(u8 cmd, const u8 *data, u16 len, bool is_rx);

// 按键通知检测 (帧解析用)
bool lb_data_is_key_notify(u8 *data, u16 len);

// 产品信息查询处理 (BLE 桥模式回调用)
u8 lb_handler_product_info(lb_rx_frame_t *rx);

// OTA 升级命令处理器 (定义在 func_lunchbox_ota.c)
u8 lb_handler_ota_start(lb_rx_frame_t *rx);
u8 lb_handler_ota_data(lb_rx_frame_t *rx);
u8 lb_handler_ota_end(lb_rx_frame_t *rx);

// OTA 目标设备识别 (桥模式 BLE RX 路由用)
#if LB_BRIDGE_MODE
u8 lb_ota_get_target(lb_rx_frame_t *rx);
#endif

// 桥模式: 转发加热模块 OTA 时累积 CRC32
extern u32  lb_ota_uart_crc32;
extern bool lb_ota_uart_crc_active;

#endif // __FUNC_LUNCHBOX_UART_INTERNAL_H
