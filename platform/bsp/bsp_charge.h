#ifndef _BSP_CHARGE_H
#define _BSP_CHARGE_H


typedef enum
{
    CHARGE_NO_FULL = 0,			//电池没满
    CHARGE_FULL,				//电池满了
    CHARGE_REFILL				//电池续充
}charge_sta_m;


void bsp_charge_init(void);
void bsp_charge_off(void);
u8 charge_dc_detect(void);
void charge_detect(u8 mode);
void bsp_charge_sta(u8 sta);
void bsp_charge_set_stop_time(u16 stop_time);
void bsp_charge_bled_ctrl(void);
void charge_set_detect_cnt(u8 value);
bool bsp_charge_check_vio0v_bit(void);

/**
 *   充电状态函数
 *   返回充电状态
 *   0 : 5V没接上
     1 : 充电中
     2 ：充满
 */
u8 bsp_charge_sta_get(void);

#endif
