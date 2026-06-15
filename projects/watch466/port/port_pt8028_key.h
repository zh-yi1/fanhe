#ifndef _PORT_PT8028_KEY_H

#define _PORT_PT8028_KEY_H



#include "include.h"



/*

 * PT8028S U3 硬件连接（主板 MCU 原理图）

 *

 * 芯片 TCH 引脚 -> 面板功能（BCD = TCH 编号）：

 *   Pin1  TCH5 BCD5 开关   | Pin2  TCH4 BCD4 确认

 *   Pin3  TCH3 BCD3 模式   | Pin4  TCH2 BCD2 减号

 *   Pin5  TCH1 BCD1 加热   | Pin6  TCH0 BCD0 锁键

 *   Pin15 TCH7 BCD7 预约   | Pin16 TCH6 BCD6 加号

 *

 * PT8028 输出 -> MCU（经 22Ω，网标 TCH_*）：

 *   Pin9  OUT_FLAG -> PE1  低=有键

 *   Pin10 D0       -> PE2

 *   Pin13 D1       -> PE3  3 线 BCD(D2:D1:D0)，OUT_FLAG=0 时有效

 *   Pin14 D2       -> PE4

 *

 * 表2 输出键值（MCU 引脚见下图4）：
 *   TCHn 按下：OUT_FLAG=0，D2:D1:D0 = n 的三位 BCD
 *   例 TCH3 模式：OUT_FLAG,D2,D1,D0 = 0,0,1,1  (PE1=0 PE4=0 PE3=1 PE2=1)
 *   松开：OUT_FLAG=1，D 线 Hold 上次值
 *   上电/空闲：OUT_FLAG=1，D=111 — 无按键，不可当 TCH7
 */
#define PT8028_GPIO_OUT_FLAG          IO_PE1      /* TCH_OUT_FLAG -> PE1 */
#define PT8028_GPIO_D0                IO_PE2      /* TCH_D0       -> PE2 */
#define PT8028_GPIO_D1                IO_PE3      /* TCH_D1       -> PE3 */
#define PT8028_GPIO_D2                IO_PE4      /* TCH_D2       -> PE4 */

/* OUT_FLAG 空闲为高用上拉；D0~D2 高阻，由 PT8028 推挽驱动 */
#ifndef PT8028_GPIO_FLAG_PULL
#define PT8028_GPIO_FLAG_PULL           GPIOxPU
#endif
#ifndef PT8028_GPIO_BCD_PULL
#define PT8028_GPIO_BCD_PULL            GPIOxPU     /* 空闲 D=111；PT8028 按下时仍可驱动 */
#endif
#ifndef PT8028_BCD_STABLE_CNT
#define PT8028_BCD_STABLE_CNT           2           /* 释放/消息路径：连续 N 次 5ms 相同 BCD */
#endif
#ifndef PT8028_PRESS_SETTLE_SCANS
#define PT8028_PRESS_SETTLE_SCANS       2           /* OUT_FLAG 变 0 后约 10ms 再采 BCD */
#endif



extern const u8 tbl_pt8028_bcd_to_key[8];



void pt8028_port_gpio_init(void);

void pt8028_port_gpio_dump(void);

void pt8028_key_name_print(u8 tch, u8 key, u16 ku, u8 bcd, u8 out_flag);

void pt8028_key_reject_print(u8 hold_bcd, u8 latch_bcd, u8 hist_bcd, u8 out_flag);

void pt8028_key_edge_print(u8 edge_up, u8 bcd, u8 out_flag);



#endif // _PORT_PT8028_KEY_H

