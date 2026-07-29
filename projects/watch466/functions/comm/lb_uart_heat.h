/**
 * @file    lb_uart_heat.h
 * @brief   加热模块 OTA 升级 (BLE接收 → 存储 → CRC校验 → UART发送)
 *
 * 流程:
 *   1. APP 通过 BLE (target=0x02) 发送加热模块 OTA 数据 → MCU 存储到本地缓冲
 *   2. APP 发送完毕后 (OTA_END) → MCU 校验 CRC32
 *   3. CRC 错误 → 回复 APP 升级失败 (result=0x00)
 *   4. CRC 正确 → 通过 UART (cmd=0x04) 逐包发送给加热模块
 *   5. 发送 0xFFFFFFFF 复位指令 → 不等应答, 直接等 1s → 发第一包数据
 *   6. UART 发送每包后等待加热模块应答, 超时无应答则重试 (最多3次)
 *   7. 3次重试均失败 → 发送 0xFFFFFFFF+CRC32 让加热模块重新升级, 然后从第一包重新开始
 *
 * 对接协议:
 *   BLE侧: 蓝牙通讯协议1.0.7.md (cmd 0x0c/0x0d/0x0e, target=0x02)
 *   UART侧: MCU通信协议.md §5.1 (cmd 0x04, offset+data)
 *   2026-7-17-15：49
 */
#ifndef __LB_UART_HEAT_H
#define __LB_UART_HEAT_H

#include "include.h"
#include "lb_proto.h"   // lb_rx_frame_t / 帧常量

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// 加热模块 OTA 配置
//-----------------------------------------------------------------------------

/** @brief OTA 数据存储缓冲区大小 (字节), 需能容纳完整的加热模块固件 */
#define HEAT_OTA_BUF_SIZE           (20 * 1024)   

/** @brief 每包 UART 发送的数据大小 (MCU协议默认128字节, 必须可被16整除) */
#define HEAT_OTA_PACKET_SIZE        128

/** @brief UART 发送后等待加热模块应答的超时时间 (毫秒) */
#define HEAT_OTA_UART_TIMEOUT_MS    3000

/** @brief UART 单包最大重试次数 (超过则发送 0xFFFFFFFF+CRC 重新开始) */
#define HEAT_OTA_MAX_RETRIES        3

/** @brief UART 传输最大重启次数 (数据/END 超时后重新从 BOOT 开始的次数)
    0=不重启直接失败, 1=允许重启1次 */
#define HEAT_OTA_MAX_RESTARTS       1

/** @brief 发送 BOOT 复位指令后等待加热模块就绪的延迟时间 (毫秒)
    发送 0xFFFFFFFF 后不验证模块应答, 直接等待此延迟后发第一条数据 (offset=0) */
#define HEAT_OTA_BOOT_DELAY_MS      1000

//-----------------------------------------------------------------------------
// OTA 状态机
//-----------------------------------------------------------------------------
typedef enum {
    HEAT_OTA_IDLE = 0,          // 空闲
    HEAT_OTA_RECEIVING,         // 正在接收 APP 数据 (BLE → 本地缓冲)
    HEAT_OTA_VERIFY,            // 校验 CRC32 中 (非阻塞, 由 heat_ota_process 驱动)
    HEAT_OTA_CRC_CALC,          // 后台分块计算 CRC (防主循环阻塞→GPU 线程复位)
    HEAT_OTA_SENDING,           // 正在通过 UART 发送给加热模块
    HEAT_OTA_WAIT_ACK,          // 等待加热模块 UART 应答
} heat_ota_state_t;

//-----------------------------------------------------------------------------
// API
//-----------------------------------------------------------------------------

/**
 * @brief 处理 BLE OTA_START (0x0c) — target=0x02 加热模块
 *
 * APP 发送: [target=0x02][fw_size:4B BE]
 * 初始化存储缓冲区和状态机, 准备接收 OTA 数据。
 *
 * @param rx        BLE 接收帧
 * @param msg_flag  BLE 帧的 msg_flag (用于应答)
 * @return LB_ERR_SUCCESS / LB_ERR_EXEC_FAIL
 */
u8 heat_ota_handler_start(lb_rx_frame_t *rx, u8 msg_flag);

/**
 * @brief 处理 BLE OTA_DATA (0x0d) — target=0x02 加热模块
 *
 * APP 发送: [target=0x02][offset:4B BE][data:N]
 * 将数据存入本地缓冲区 (含256字节包头), 增量计算 CRC32。
 *
 * @param rx        BLE 接收帧
 * @param msg_flag  BLE 帧的 msg_flag
 * @return LB_ERR_SUCCESS / LB_ERR_EXEC_FAIL
 */
u8 heat_ota_handler_data(lb_rx_frame_t *rx, u8 msg_flag);

/**
 * @brief 处理 BLE OTA_END (0x0e) — target=0x02 加热模块
 *
 * APP 发送: [target=0x02]
 * 验证 CRC32:
 *   - 失败 → 回复 APP result=0x00 (升级失败), 回到 IDLE
 *   - 成功 → 回复 APP result=0x01 (升级成功), 启动 UART 发送流程
 *
 * @param rx        BLE 接收帧
 * @param msg_flag  BLE 帧的 msg_flag
 * @return LB_ERR_SUCCESS / LB_ERR_EXEC_FAIL
 */
u8 heat_ota_handler_end(lb_rx_frame_t *rx, u8 msg_flag);

/**
 * @brief 主循环轮询: 驱动加热模块 OTA 状态机
 *
 * 负责:
 *   - UART 发送超时检测与重试
 *   - 逐包发送加热模块固件
 *   - 重试次数超限后的复位重传
 *
 * 调用位置: func.c 主循环, 与 lunchbox_uart_process() 并列
 */
void heat_ota_process(void);

/**
 * @brief UART 应答帧注入: 加热模块对 OTA 包的回复
 *
 * 当 UART 帧解析器收到 cmd=0x04 的帧时,
 * 若当前处于 HEAT_OTA_WAIT_ACK 状态, 则调用此函数通知状态机。
 *
 * @param rx  UART 接收帧 (cmd=0x04 的应答帧)
 * @return true=应答已被 OTA 状态机消费 (UART 帧不转发 BLE)
 */
bool heat_ota_uart_response(lb_rx_frame_t *rx);

/**
 * @brief 查询是否正在进行加热模块 OTA
 * @return true=OTA 进行中 (非 IDLE)
 */
bool heat_ota_is_active(void);

/**
 * @brief 获取当前 OTA 状态 (调试用)
 */
heat_ota_state_t heat_ota_get_state(void);

/**
 * @brief BLE 重连后检查并发送延迟的 OTA 升级结果
 *
 * 断电恢复场景: 加热模块进入 BOOT 导致 MCU 掉电,
 * BLE 已断开, OTA 结果保存到 Flash。
 * BLE 重新连接后调用此函数发送延迟的 0x0e 应答给 APP。
 *
 * 调用时机: lunchbox_ble_on_connected() 中。
 *
 * @return true=有待上报结果且已发送, false=无待上报结果
 */
bool heat_ota_send_deferred_result(void);

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __LB_UART_HEAT_H
