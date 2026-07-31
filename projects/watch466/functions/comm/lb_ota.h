/**
 * @file    lb_ota.h
 * @brief   饭盒 OTA 固件升级 (蓝牙通讯协议1.0.7.md §5)
 *
 * 管理主单片机 (target=0x01) 的固件升级流程，对接 ota_pack_* 底层 FOTA 引擎。
 * 桥模式和本地模式均可用。
 */
#ifndef __LB_OTA_H
#define __LB_OTA_H

#include "include.h"
#include "lb_proto.h"   // lb_rx_frame_t

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// OTA 目标设备标识 (蓝牙通讯协议1.0.7.md §5)
// APP 通过 target 字段区分升级目标设备
//-----------------------------------------------------------------------------
#define LB_OTA_TARGET_MAIN_MCU      0x01    // 主单片机（模组）
#define LB_OTA_TARGET_HEAT_MODULE   0x02    // 加热模块

// OTA 升级启动状态 (蓝牙通讯协议1.0.7.md §5.1)
#define LB_OTA_START_RECEIVED       0x00    // 收到升级指令
#define LB_OTA_START_ERASING        0x01    // MCU 擦除 flash 中
#define LB_OTA_START_ERASE_DONE     0x02    // 擦除完成，可以传输升级包

// OTA 升级结果 (蓝牙通讯协议1.0.7.md §5.3)
#define LB_OTA_RESULT_FAIL          0x00    // 升级失败
#define LB_OTA_RESULT_SUCCESS       0x01    // 升级成功

/**
 * @brief OTA 升级流程处理 (需在主循环中轮询调用)
 *
 * 职责: 升级成功后的延时复位。ota_pack_done() 完成后需复位 MCU
 * 才能让 bootloader 解压新固件。延时 3 秒是为了确保 BLE 应答帧
 * (0x0e 返回) 已成功发送给 APP。
 *
 * 调用位置: func.c 主循环, 与 bsp_fot_process() 并列
 */
void lb_ota_process(void);

/**
 * @brief 检查是否正在进行 OTA 升级
 * @return true=正在升级中 (已收到 0x0c 启动命令且未结束)
 *
 * 用途: 关屏/休眠等模块在操作前调用此函数，OTA 进行中时跳过关屏，
 *       避免 Flash 擦写期间 LCD 断电导致 GPU 状态异常复位。
 */
bool lb_ota_is_active(void);

//-----------------------------------------------------------------------------
// OTA 升级命令处理器 (BLE 分发层按 target=主MCU 调用)
//-----------------------------------------------------------------------------

u8 lb_handler_ota_start(lb_rx_frame_t *rx);
u8 lb_handler_ota_data(lb_rx_frame_t *rx);
u8 lb_handler_ota_end(lb_rx_frame_t *rx);

/** @brief 获取 OTA 帧的 target 字段 (0x01=主MCU 0x02=加热模块 0x00=未知) */
u8 lb_ota_get_target(lb_rx_frame_t *rx);

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __LB_OTA_H
