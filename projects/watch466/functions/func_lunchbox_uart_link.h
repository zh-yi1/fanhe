/**
 * @file    func_lunchbox_uart_link.h
 * @brief   饭盒 UART1 收发层 — 硬件初始化 / 阻塞发送 / RX 中断环形缓冲
 * @note    引脚: TX=PB8(UT1TXMAP_G2_PB8), RX=PB9(UT1RXMAP_G2_PB9)
 *          本层无任何协议知识: 只搬运字节, 不认识 0x55AA 帧。
 *          分层: 应用层 → 协议层(func_lunchbox_proto) → 收发层(本文件) → api_uart
 *
 *          环形缓冲并发模型 (无锁单读单写):
 *            - 中断只写 w / overflow, 主循环只写 r
 *            - 缓冲满时丢弃新字节并累加 overflow 计数, 绝不覆盖未读数据
 *            - 中断内不做超时清理 (帧超时由协议层在主循环上下文处理)
 */
#ifndef __FUNC_LUNCHBOX_UART_LINK_H
#define __FUNC_LUNCHBOX_UART_LINK_H

#include "include.h"

#if FUNC_LUNCHBOX_UART_EN

/** RX 环形缓冲容量, 必须为 2 的幂 (实际可存 SIZE-1 字节)。
 *  预约列表 10 帧连发 = 530B @115200 ≈ 46ms 线速, 511B 可吸收约 44ms 主循环停顿 */
#define LB_LINK_RXBUF_SIZE      512

/**
 * @brief 初始化 UART1 (PB8-TX / PB9-RX) 并注册 RX 中断
 * @param[in] baud 波特率
 * @return true=成功
 */
bool lb_link_init(u32 baud);

/** @brief 阻塞发送 (逐字节 FIFO, 无 DMA/TX 中断), 115200 下约 87us/字节 */
void lb_link_tx(const u8 *buf, u16 len);

/** @brief 非阻塞取 1 字节, 返回 1=取到 0=空 (主循环上下文调用) */
u8 lb_link_getc(u8 *ch);

/** @brief 未读字节数 */
u16 lb_link_rx_pending(void);

/** @brief 缓冲满丢弃的字节累计数, 读取后清零 (诊断用) */
u16 lb_link_rx_overflow(void);

/** @brief 清空环形缓冲。仅允许在 suspend 后(中断已停)或 init 前调用, 否则与 ISR 竞态 */
void lb_link_rx_clear(void);

/** @brief 关闭 UART1 降低功耗: 保存 UART1CON → 清零 → 释放 PB8/PB9 回 GPIO(供下降沿唤醒) */
void lb_link_suspend(void);

/** @brief 从 suspend 恢复: 重新初始化 UART1 (沿用 init 时的波特率) */
void lb_link_resume(void);

/** @brief 当前是否处于 suspend 状态 */
bool lb_link_is_suspended(void);

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __FUNC_LUNCHBOX_UART_LINK_H
