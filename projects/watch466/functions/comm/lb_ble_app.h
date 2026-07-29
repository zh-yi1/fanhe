/**
 * @file    lb_ble_app.h
 * @brief   饭盒 BLE 应用层 — 接收链路唯一入口
 *
 * 数据流:
 *   gatt_callback_app()            蓝牙中断: 收包入环形缓冲 (平台侧 app_blue_fit.c)
 *        ↓ ble_app_lunchbox_rx_pop()
 *   lunchbox_ble_process()         主循环: 取包 → 字节流重组 → 逐帧分发
 *        ↓
 *   lb_ble_dispatch()              按命令字分发 (OTA 尚未实现, 为 ZH TODO)
 */
#ifndef __LB_BLE_APP_H
#define __LB_BLE_APP_H

#include "include.h"
#include "lb_proto.h"   // lb_rx_frame_t / lb_ble_tx_fn_t

/**
 * @brief 饭盒 BLE 主循环处理 — 取代 ble_app_process()
 *
 * 每轮主循环调用一次。职责:
 *   1. 把环形缓冲区里所有待处理的包一次取空 (不是一轮只取一包)
 *   2. 追加到字节流重组缓冲区, 按 0x55AA 帧边界切帧
 *   3. 校验通过的帧交给分发逻辑
 *   4. 顺带做残帧超时清理 (不依赖新数据到达)
 */
void lunchbox_ble_process(void);

//-----------------------------------------------------------------------------
// 发送接口 (MCU → APP, 经 BLE Notify)
//-----------------------------------------------------------------------------

/** @brief 注册 BLE 发送函数 (平台 app_blue_fit.c 初始化时调用) */
void lunchbox_ble_set_tx_fn(lb_ble_tx_fn_t fn);

/** @brief 发送一条完整 BLE 帧给 APP; false=通道未注册(未连接) */
bool lunchbox_ble_tx(u8 *frame, u16 len);

/** @brief 组帧并应答 APP (echo 请求的 msg_flag) */
bool lb_ble_send_response(u8 ble_cmd, u8 msg_flag, u8 err, const u8 *data, u16 len);

/** @brief 组帧并主动推送 APP (MCU 发起, msg_flag 自增) */
bool lb_ble_send_async(u8 ble_cmd, const u8 *data, u16 len);

//-----------------------------------------------------------------------------
// 平台回调 / 兼容入口
//-----------------------------------------------------------------------------

/** @brief BLE 连接成功回调 (平台 app_blue_fit.c 调用); 业务待填充 */
void lunchbox_ble_on_connected(void);

/**
 * @brief 直接投喂一段 BLE 原始字节 (兼容旧入口, 平台侧仍可能调用)
 *
 * 与 lunchbox_ble_process() 走同一套重组/分发逻辑。
 */
void lunchbox_ble_rx_handle(u8 *data, u16 len);

/** @brief 重组缓冲区是否有半帧待补齐 */
bool lunchbox_ble_rx_pending(void);

//-----------------------------------------------------------------------------
// 串口应答配对钩子 (lb_uart_on_frame 收到 0x01 帧时调用)
//-----------------------------------------------------------------------------

/**
 * @brief 时间同步应答配对 — 模块确认后才保存本机时间
 * @return true=已消化, 勿再当模块主动上报转发 APP
 */
bool lb_ble_timesync_on_heat_frame(lb_rx_frame_t *rx);

/**
 * @brief 产品信息查询应答配对 — 回填模块版本号并回复 APP 81B 设备信息
 * @return true=已消化, 勿再当模块主动上报转发 APP
 */
bool lb_ble_product_info_on_heat_frame(lb_rx_frame_t *rx);

//-----------------------------------------------------------------------------
// 模式预设 (温度档位 + 时长; APP 经 0x0a 修改并同步模块, UI/模式页读取)
//-----------------------------------------------------------------------------

/** @brief 获取指定模式的预设温度档位 */
u8 lunchbox_mode_get_temp(u8 mode);

/** @brief 获取指定模式的预设加热时长(分钟) */
u8 lunchbox_mode_get_duration(u8 mode);

/** @brief 更新本机模式预设 (0x0a / UI 侧同步) */
void lunchbox_mode_preset_local_set(u8 mode, u8 temp_idx, u8 duration_min);

#endif // __LB_BLE_APP_H
