/**
 * @file    lb_uart_app.h
 * @brief   饭盒串口应用层 — 与加热模块通信 (MCU通信协议.md v1.0.7)
 *
 * 引脚: TX=PB8=UT1TXMAP_G2_PB8
 *       RX=PB9=UT1RXMAP_G2_PB9
 * 帧格式与 BLE 相同 (0x55AA, 见 lb_proto.h), 命令字为 LB_UART_CMD_*。
 * 通信机制:
 *   同步: 一问一答, 应答的 cmd/msg_flag 与请求帧一致
 *   异步: 加热模块主动上报(温控/故障), msg_flag 自增
 *
 * 三层结构:
 *   底层收发  lb_uart_link   UART1 硬件 + RX 环形缓冲
 *   协议层    lb_proto       0x55AA 帧编解码 (纯函数)
 *   应用层    本文件         数据泵调度 + 帧处理入口 (业务 ZH TODO 待填充)
 */
#ifndef __LB_UART_APP_H
#define __LB_UART_APP_H

#include "include.h"
#include "lb_proto.h"   // lb_rx_frame_t / LB_UART_CMD_* / LB_FRAME_TIMEOUT_MS

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// 应用配置
//-----------------------------------------------------------------------------
#define LB_BAUD                 115200      // 波特率（按需修改）

//-----------------------------------------------------------------------------
// 生命周期
//-----------------------------------------------------------------------------

/**
 * @brief 初始化串口模块 (解析器 + UART1 硬件), 系统初始化阶段调用一次
 * @param[in] baud  波特率, 如 LB_BAUD
 */
void lunchbox_uart_init(u32 baud);

/** @brief 关闭 UART1 (低功耗/手动关机用); 解析器复位, 释放 PB8/PB9 回 GPIO */
void lunchbox_uart_suspend(void);

/** @brief 从 suspend 恢复 (重建 UART1, 沿用 LB_BAUD) */
void lunchbox_uart_resume(void);

/**
 * @brief 主循环处理（func_process 每轮调用）
 *
 * 边读边解: 底层取字节 → 协议层拼帧/残帧超时 → 逐帧调 lb_uart_on_frame()
 */
void lunchbox_uart_process(void);

//-----------------------------------------------------------------------------
// 发送接口 (MCU → 加热模块)
//-----------------------------------------------------------------------------

/**
 * @brief 组帧并从串口发出 (应用层唯一发送原语)
 * @return true=已发出, false=被阻断或组帧失败
 */
bool lunchbox_uart_send_frame(u8 cmd, u8 msg_flag, u8 err,
                              const u8 *data, u16 len);

/** @brief 阻止/恢复所有 UART TX (手动关机期间用); true=阻塞 false=恢复 */
void lb_uart_tx_block(bool block);
bool lb_uart_tx_is_blocked(void);

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __LB_UART_APP_H
