#ifndef _FIT_HX3602_H_
#define _FIT_HX3602_H_

typedef enum {
	HX3602_HRS_MODE,
	HX3602_SPO2_MODE,
	HX3602_WEAR_MODE,
	HX3602_HRV_MODE,
	HX3602_LIVING_MODE,
	HX3602_LAB_TEST_MODE,
	HX3602_FT_LEAK_LIGHT_MODE,
	HX3602_FT_GRAY_CARD_MODE,
	HX3602_FT_INT_TEST_MODE
} HX3602_MODE_T;

void hx3602_isr(void);
void hx3602_gpioint_enable(void);
void hx3602_gpioint_disable(void);
void hx3602_isr_process(void);

void hx3602_kick(u8 select);         //0:40ms, 1:320ms
void hx3602_40ms_timer_set(bool en);
void hx3602_320ms_timer_set(bool en);
void hx3602_40ms_process(void);
void hx3602_320ms_process(void);

bool sensor_hx3602_init(void);
bool sensor_hx3602_stop(void);
bool sensor_hx3602_wear_sta_get(void);
//u16 rand(u16 num);    //stdlib.h中已有声明int	_EXFUN(rand,(_VOID));

#endif // _FIT_HX3602_H_
