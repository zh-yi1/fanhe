#ifndef _CTP_H
#define _CTP_H

#include "axs5106/axs5106.h"
#include "axs152x/axs152x.h"
void ctp_init(void);
void ctp_exit(void);
void ctp_int_isr(void);
bool ctp_get_cur_point(s32 *sx, s32 *sy, s32 *x, s32 *y);
bool ctp_get_dxy(s32 *dx, s32 *dy);
point_t ctp_get_sxy(void);
point_t ctp_get_last_dxy(void);
point_t ctp_get_rxy(widget_t *widget);

void ctp_iic_readkick(void *buf, u8 addr, int len);
bool ctp_iic_read(void *buf, u8 addr, int len);

bool ctp_cst8x_init(void);
void ctp_cst8x_readkick(void);
bool ctp_cst8x_get_point(s32 *x, s32 *y);

bool ctp_chsc6x_init(void);
void ctp_chsc6x_readkick(void);
bool ctp_chsc6x_get_point(s32 *x, s32 *y);

bool ctp_get_kick_status(void);

//tp中断在支付宝iic通信时先关中断
void sys_ctp_irq_enble(int vector);
void sys_ctp_irq_disable(int vector);

i2c_t *ctp_iic_reg_get(void);

bool ctp_is_touch(void);
#endif
