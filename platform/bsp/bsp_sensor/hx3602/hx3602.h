#ifndef _HRS3602_H_
#define _HRS3602_H_
#include <stdint.h>
#include <stdbool.h>

//#include "opr_oled.h"


#define TIMER_READ_MODE
#define HRS_ALG_LIB
//#define BP_CUSTDOWN_ALG_LIB
#define IR_CHECK_TOUCH
#define  NO_MATH_LIB
//#define CHECK_TOUCH_DIFF

//#define TESTVEC

//#define TYHX_DEMO

//#define HRS_BLE_APP

//#define RTT_DEBUG

//#define GSENSER_DATA

//#define NEW_GSEN_SCHME

#ifdef RTT_DEBUG
#define  DEBUG_PRINTF(...)     SEGGER_RTT_printf(0,__VA_ARGS__)
#else
#define	 DEBUG_PRINTF(...)
#endif

#define HX3602_ADDR_7BIT                    0x44
#define HX3602_WRITE_ADDR(ADDR)     	    ((ADDR) << 1)
#define HX3602_READ_ADDR_UPDATE(ADDR)       ((ADDR) << 1 | 1)
#define HX3602_READ_ADDR(ADDR)      	    ((ADDR) << 1 | 1) << 8 | ((ADDR) << 1)

typedef struct {
   int16_t AXIS_X;
   int16_t AXIS_Y;
   int16_t AXIS_Z;
} GsensorRaw_t;

extern uint8_t alg_ram[5*1024];

void Hrs3602_chip_disable(void);
void Hrs3602_chip_enable(void);

bool Hrs3602_chip_init(void);
void Hrs3602_chip_reset(void);
void heart_rate_meas_timeout_handler(void * p_context);
void Hrs3602_INT_init(void);
void Hrs3602_INT_enable(void);
void Hrs3602_INT_disable(void);

bool    Hrs3602_write_reg(uint8_t addr, uint8_t data);
uint8_t Hrs3602_read_reg(uint8_t addr);
bool Hrs3602_brust_read_reg(uint8_t addr , uint8_t *buf, uint8_t length);

void    Hrs3602_timeout_handler(void);
void    Hrs3602_init_touch_mode(void);
void    Hrs3602_Int_handle(void);
void    Hrs3602_blood_presure_Int_handle(void);
void    Hrs3602_chip_disable(void);
void    Hrs3602_alg_config(void);
extern void Hrs3602_set_first_hr_mode(uint8_t mode);
extern void Hrs3602_set_dc_thres_low(int32_t value);

void Hrs3602_bp_alg_config(void); // add ericy 20180625

bool Hrs3602_read_hrs(int32_t *hrm_data, int32_t *als_data);
bool Hrs3602_read_ps1(int32_t *infrared_data);
void Hrs3602_set_motion_status(uint16_t motion_status);

uint32_t hrs3602_timers_start(void);
uint32_t hrs3602_timers_stop(void);
void Hrs3300_button_handler(void);
void read_data_packet(int32_t *ps_data);

#endif // _HRS3600_H_


