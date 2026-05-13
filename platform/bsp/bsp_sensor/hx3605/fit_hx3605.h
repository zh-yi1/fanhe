#ifndef _FIT_HX3605_H_
#define _FIT_HX3605_H_

typedef enum {
	HX3605_HRS_MODE,
	HX3605_SPO2_MODE,
	HX3605_WEAR_MODE,
	HX3605_HRV_MODE,
	HX3605_LIVING_MODE,
	HX3605_LAB_TEST_MODE,
	HX3605_FT_LEAK_LIGHT_MODE,
	HX3605_FT_GRAY_CARD_MODE,
	HX3605_FT_INT_TEST_MODE
} HX3605_MODE_T;

void hx3605_isr(void);
void hx3605_gpioint_enable(void);
void hx3605_gpioint_disable(void);
void hx3605_isr_process(void);

void hx3605_kick(u8 select);         //0:40ms, 1:320ms
void hx3605_40ms_timer_set(bool en);
void hx3605_320ms_timer_set(bool en);
void hx3605_40ms_process(void);
void hx3605_320ms_process(void);

bool sensor_hx3605_init(HX3605_MODE_T mode);
bool sensor_hx3605_stop(void);
bool sensor_hx3605_wear_sta_get(void);
//u16 rand(u16 num);    //stdlib.h中已有声明int	_EXFUN(rand,(_VOID));

#endif // _FIT_HX3605_H_
