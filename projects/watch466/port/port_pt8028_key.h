#ifndef _PORT_PT8028_KEY_H

#define _PORT_PT8028_KEY_H

#include "include.h"

/*
 * 原理图：PT8028 -> MCU
 *   TCH_OUT_FLAG -> PE1 (Pin6)
 *   TCH_D0       -> PE2 (Pin7)  BCD bit0
 *   TCH_D1       -> PE3 (Pin8)  BCD bit1
 *   TCH_D2       -> PE4 (Pin9)  BCD bit2
 *
 * 表2 输出键值（D2:D1:D0 = TCH 编号）：
 *   上电/空闲：OUT_FLAG=1, D=111 — 不是按键（D 线 alone 不能表示“正在按”）
 *   TCHn 按下：OUT_FLAG=0, D=n
 *     TCH0 000 | TCH1 001 | TCH2 010 | TCH3 011(模式)
 *     TCH4 100(确认) | TCH5 101 | TCH6 110 | TCH7 111(预约, 仅 OUT_FLAG=0)
 *   TCHn 释放：OUT_FLAG=1, D=Hold(上次键值)
 */
#define PT8028_GPIO_OUT_FLAG          IO_PE1      /* TCH_OUT_FLAG -> PE1 */
#define PT8028_GPIO_D0                IO_PE2      /* TCH_D0       -> PE2 */
#define PT8028_GPIO_D1                IO_PE3      /* TCH_D1       -> PE3 */
#define PT8028_GPIO_D2                IO_PE4      /* TCH_D2       -> PE4 */

#ifndef PT8028_GPIO_FLAG_PULL
#define PT8028_GPIO_FLAG_PULL           GPIOxPU     /* 空闲 OUT_FLAG=1 */
#endif
#ifndef PT8028_GPIO_BCD_PULL
#define PT8028_GPIO_BCD_PULL            0           /* 空闲高阻；按下采样时临时关上下拉 */
#endif
#ifndef PT8028_BCD_STABLE_CNT
#define PT8028_BCD_STABLE_CNT           2
#endif
#ifndef PT8028_PRESS_SETTLE_SCANS
#define PT8028_PRESS_SETTLE_SCANS       5           /* FLAG 变 0 后等 BCD 稳定 */
#endif

extern const u8 tbl_pt8028_bcd_to_key[8];

/* 饭盒：按下阶段 BCD 重映射（OUT_FLAG=0 时） */
u8 pt8028_elunchbox_bcd_remap(u8 bcd, u8 out_flag);

void pt8028_port_gpio_init(void);
/* 热路径：寄存器级 reclaim PE1~4 为 GPIO 输入（读 BCD 前调用） */
void pt8028_pe_reclaim_fast(void);
bool pt8028_pe_gpio_ok(void);
void pt8028_port_gpio_dump(void);
void pt8028_key_name_print(u8 tch, u8 key, u16 ku, u8 bcd, u8 out_flag);
void pt8028_key_reject_print(u8 hold_bcd, u8 latch_bcd, u8 hist_bcd, u8 out_flag);
void pt8028_key_edge_print(u8 edge_up, u8 bcd, u8 out_flag);

#endif // _PORT_PT8028_KEY_H
