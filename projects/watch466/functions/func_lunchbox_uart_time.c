/**
 * @file    func_lunchbox_uart_time.c
 * @brief   饭盒时间工具实现
 */

#include "include.h"
#include "func_lunchbox_uart.h"

const char *lb_unix_time_str(u32 unix_ts)
{
    // 12字节上限，多了会起不来(bss溢出)
    static char buf[12];
    tm_t t = time_to_tm(unix_ts - LB_RTC_UNIX_OFFSET);
    u8 *p = (u8 *)buf;
    *p++ = (u8)('0' + t.mon  / 10); *p++ = (u8)('0' + t.mon  % 10); *p++ = '-';
    *p++ = (u8)('0' + t.day  / 10); *p++ = (u8)('0' + t.day  % 10); *p++ = '-';
    *p++ = (u8)('0' + t.hour / 10); *p++ = (u8)('0' + t.hour % 10); *p++ = ':';
    *p++ = (u8)('0' + t.min  / 10); *p++ = (u8)('0' + t.min  % 10);
    *p = '\0';
    return buf;
}
