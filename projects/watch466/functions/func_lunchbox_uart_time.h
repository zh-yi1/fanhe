/**
 * @file    func_lunchbox_uart_time.h
 * @brief   饭盒时间工具 — Unix时间戳 → 可读字符串
 */

#ifndef __FUNC_LUNCHBOX_UART_TIME_H
#define __FUNC_LUNCHBOX_UART_TIME_H

#include "include.h"

/**
 * @brief Unix时间戳 → "MM-DD-HH:MM" (北京时间)
 * @param unix_ts  Unix时间戳(秒)
 * @return 静态缓冲区指针, 下次调用会覆盖
 */
const char *lb_unix_time_str(u32 unix_ts);

#endif
