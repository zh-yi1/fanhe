/**
 * @file    lb_ble_app.h
 * @brief   饭盒 BLE 应用层 — 接收链路唯一入口
 *
 * 数据流:
 *   gatt_callback_app()            蓝牙中断: 收包入环形缓冲 (平台侧 app_blue_fit.c)
 *        ↓ ble_app_lunchbox_rx_pop()
 *   lunchbox_ble_process()         主循环: 取包 → 字节流重组 → 逐帧分发
 *        ↓
 *   lb_ble_dispatch()              按命令字分发 — 业务处理待填充 (ZH TODO)
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

#endif // __LB_BLE_APP_H
