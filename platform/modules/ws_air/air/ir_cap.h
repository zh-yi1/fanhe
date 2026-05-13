#ifndef _IR_CAP_H_
#define _IR_CAP_H_

#if (ASR_SELECT == ASR_WS_AIR && IR_AIR_FUNC)

/*****************************************************************************
 * Module    : IRRX配置
 *****************************************************************************/
#define IR_CAPTURE_PORT()               {GPIOFDE |= BIT(2); GPIOFPU  |= BIT(2); GPIOFDIR |= BIT(2);}
#define IRRX_MAPPING                    IO_PF2


void ir_cap_timer_start(u8 type);
void ir_timeout();
int isd_get_air_match_status();

#endif // ASR_WS_AIR

#endif // _BSP_IR_H
