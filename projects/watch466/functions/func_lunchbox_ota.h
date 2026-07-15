/**
 * @file    func_lunchbox_ota.h
 * @brief   饭盒 OTA 固件升级 (蓝牙通讯协议1.0.7.md §5)
 *
 * 管理主单片机 (target=0x01) 的固件升级流程，对接 ota_pack_* 底层 FOTA 引擎。
 * 桥模式和本地模式均可用。
 */
#ifndef __FUNC_LUNCHBOX_OTA_H
#define __FUNC_LUNCHBOX_OTA_H

#include "include.h"

#if FUNC_LUNCHBOX_UART_EN

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

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __FUNC_LUNCHBOX_OTA_H
