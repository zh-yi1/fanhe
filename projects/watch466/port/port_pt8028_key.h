#ifndef _PORT_PT8028_KEY_H
#define _PORT_PT8028_KEY_H

#include "include.h"

/*
 * PT8028S BCD 接口 GPIO（占位，按原理图 TCH_D0/D1/D2/OUT_FLAG 修改）
 * 原理图：TCH_D0 Pin10, TCH_D1 Pin13, TCH_D2 Pin14, TCH_OUT_FLAG Pin9
 */
#define PT8028_GPIO_D0                IO_PB0
#define PT8028_GPIO_D1                IO_PB1
#define PT8028_GPIO_D2                IO_PB2
#define PT8028_GPIO_OUT_FLAG          IO_PB3

/* BCD 0~7 -> bsp KEY_*，与面板 TCH0~TCH7 对应 */
extern const u8 tbl_pt8028_bcd_to_key[8];

void pt8028_port_gpio_init(void);

#endif // _PORT_PT8028_KEY_H
