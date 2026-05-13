#ifndef _HX3605_SPO2_AGC_H_
#define _HX3605_SPO2_AGC_H_
#include <stdint.h>
#include <stdbool.h>
#include "hx3605.h"

#define CAL_DELAY_COUNT (3)

#define CAL_FLG_LED_DR     0x01
#define CAL_FLG_LED_DAC    0x02 
#define CAL_FLG_AMB_DAC    0x04
#define CAL_FLG_RF         0x08

// hx3690l 20200805 ericy
#define SPO2_CAL_INIT_LED_RED 16
#define SPO2_CAL_INIT_LED_IR 16

//touch check 
#define SPO2_NO_TOUCH_CHECK_NUM 2     //1 second one times

typedef enum {    
    sCalStart,
    sCalStart1,	
    sCalLed,
    sCalLed2,
    sCalLed3,
    sCalRf,
    sCalRfEnd,
    sCalFinish, 
} SPO2_STATE_T;

void Init_Spo2_PPG_Calibration_Routine(SPO2_CAL_SET_T *calR,uint8_t led);  
void Restart_spo2_PPG_Calibration_Routine(SPO2_CAL_SET_T *calR);
void PPG_Spo2_Calibration_Routine(SPO2_CAL_SET_T *calR, int32_t r_led, int32_t amb, int32_t ir_led);

void hx3605_spo2_cal_init(void);
void hx3605_spo2_cal_off(void);

void read_spo2_data_packet(int32_t *buf);
void read_spo2_ir_packet(int32_t *buf);
void hx3605_spo2_read_data(int32_t *buf);

void hx3605_spo2_low_power(void);
void hx3605_spo2_normal_power(void);

void hx3605_spo2_updata_reg(void);
void hx3605_spo2_set_mode(uint8_t mode_cmd);
SENSOR_ERROR_T hx3605_spo2_enable(void);
void hx3605_spo2_disable(void);
hx3605_spo2_wear_msg_code_t hx3605_spo2_get_wear_status(void);
uint8_t hx3605_spo2_read(spo2_sensor_data_t * s_dat);
uint8_t hx3605_spo2_read_data_packet(int32_t *s_buf);
bool hx3605_spo2_check_unwear(WORK_MODE_T mode,int32_t infrared_data);
bool hx3605_spo2_check_wear(WORK_MODE_T mode,int32_t infrared_data);
bool  hx3605_spo2_change_to_wear(WORK_MODE_T mode,int32_t infrared_data);
SPO2_CAL_SET_T get_spo2_agc_status(void);

#endif
