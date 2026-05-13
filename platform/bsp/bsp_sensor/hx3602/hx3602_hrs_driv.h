#ifndef _HRS3600_DRIV_H_
#define _HRS3600_DRIV_H_

#include <stdint.h>


///* portable 8-bit unsigned integer */
//typedef unsigned char           uint8_t;
///* portable 8-bit signed integer */
//typedef signed char             int8_t;
///* portable 16-bit unsigned integer */
//typedef unsigned short int      uint16_t;
///* portable 16-bit signed integer */
//typedef signed short int        int16_t;
///* portable 32-bit unsigned integer */
//typedef unsigned int            uint32_t;
///* portable 32-bit signed integer */
//typedef signed int              int32_t;

#ifndef bool
#define bool unsigned char
#endif

#ifndef  true
#define  true  1
#endif
#ifndef  false
#define  false  0
#endif

typedef enum {
	MSG_NO_TOUCH,
	MSG_TOUCH,
	MSG_CHECK_INIT
} hrs3602_wear_status_t;

typedef struct {
	uint32_t data_cnt;
	uint8_t highlight;
	uint8_t no_touch_check_flg;
	uint32_t no_touch_check_cnt;
	uint32_t timer_notouch_check_cnt;
	uint8_t no_touch_cnt;
} agc_data_t;

void Hrs3602_driv_init(void);
void Hrs3602_low_power(void);
void Hrs3602_normal_power(void);
hrs3602_wear_status_t Hrs3602_agc(int32_t als_raw_data , int32_t infrared_data);

#endif
