/**
 * @file    func_lunchbox_partition.h
 * @brief   饭盒项目 — 可调参数宏定义 (方便统一调整时序/重试策略)
 * @note    此文件集中管理 MCU ↔ 加热模块 UART 通信的时序参数，
 *          修改宏值即可调整行为，无需改动业务逻辑代码。
 */
#ifndef __FUNC_LUNCHBOX_PARTITION_H
#define __FUNC_LUNCHBOX_PARTITION_H

#include "include.h"

//-----------------------------------------------------------------------------
// UART 指令发送时序
//-----------------------------------------------------------------------------

/**
 * @brief 发送指令后等待加热模块回应的超时时间 (毫秒)
 *
 * 加热模块在 115200bps 下通常 10~50ms 内应答。
 * 设为 100ms 以兼顾响应速度与容错。
 */
#define LB_UART_CMD_INTERVAL_MS     70

/**
 * @brief UART 指令无回应时的最大重试次数 (不含首次发送)
 *
 * 首次发送 + 最多重试 2 次 = 总共最多发送 3 次。
 * 3 次均无回应则放弃当前指令，继续发送下一条。
 */
#define LB_UART_CMD_MAX_RETRIES     2

//-----------------------------------------------------------------------------
// UART 发送队列 (MCU → 加热模块)
// 每次只发一条指令，等待回应；超时无回应则重试，三次均无回应则放弃继续下一条。
//-----------------------------------------------------------------------------

/** @brief 发送队列深度 (应对 burst 场景，如 BLE 连接后一次发 5 个预设) */
#define LB_SEND_QUEUE_SIZE  8

/** @brief 发送队列单项 */
typedef struct {
    u8   cmd;                           // UART 命令字
    u8   data[128];                     // 数据载荷
    u16  data_len;                      // 数据长度
    bool no_wait;                       // true=发送后不等回应、不重试
    u8   ble_cmd;                       // 0=LCD来源, 非0=BLE命令字(响应路由用)
    bool no_ble_report;                 // true=跳过BLE上报(按键通知/心跳等)
} lb_send_q_item_t;

// 发送队列 (环形缓冲)
extern lb_send_q_item_t lb_send_queue[LB_SEND_QUEUE_SIZE];
extern u8  lb_send_q_head;             // 队头 (下一个出队位置)
extern u8  lb_send_q_tail;             // 队尾 (下一个入队位置)
extern u8  lb_send_q_count;            // 队列中元素数量

// 当前正在等待加热模块回应的指令 (重试用)
extern bool lb_send_waiting;           // true = 等待回应中
extern u8   lb_send_wait_msg;          // 等待回应的 msg_flag
extern u8   lb_send_retry;             // 已重试次数 (0 = 首次发送)
extern u32  lb_send_tick;              // 最近一次发送的时刻 (超时判断用)
extern u8   lb_send_cur_cmd;           // 当前指令的命令字 (重试重建帧用)
extern u8   lb_send_cur_data[128];     // 当前指令的数据载荷 (重试重建帧用)
extern u16  lb_send_cur_dlen;          // 当前指令的数据长度

// lb_uart_send_raw() 使用的 msg_flag 自增计数器
extern u8   lb_uart_raw_msg_flag;

#endif // __FUNC_LUNCHBOX_PARTITION_H
