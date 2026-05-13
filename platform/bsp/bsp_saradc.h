#ifndef _BSP_SARADC_H
#define _BSP_SARADC_H

typedef struct {
    u16 vrtc_first;
    u16 vrtc_val;
    u16 bg;
    u16 vbat2;
    u16 vbat_val;
    u16 vbat_bak;
#if TS_MODE_EN
    u16 vts;
    s16 ntc_temp;
#endif
#if VUSB_DETECT_EN
    u32 vusb;
#endif
    u16 mic_volt;
    u32 vbat_total;
    u32 tmr1ms_cnt;
} adc_cb_t;
extern adc_cb_t adc_cb;

void bsp_saradc_init(void);
bool bsp_saradc_process(u32 auto_en);
uint16_t bsp_saradc_exit(void);
void bsp_saradc_tmr1ms_process(void);
void gradient_process(bool dir);

#define bsp_saradc_set_channel(n)       saradc_set_channel(n)

#if TS_MODE_EN
/**
 * NTC ADC 转 温度
 * 输入dac值，返回实际温度
*/
s16 ts_adc2ntc(int adc_value);
#endif

#endif // _BSP_SARADC_H
