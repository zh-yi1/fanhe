#ifndef _API_SARADC_H
#define _API_SARADC_H

#define ADCCH15_ANA_VUSB    0x01        //VUSB DIV3
#define ADCCH15_ANA_BG      0x02
#define ADCCH15_ANA_VRTC    0x04
#define ADCCH15_ANA_MIC     0x08        //AUDIO MIC TRIM
#define ADCCH15_ANA_CLKGEN  0x10
#define ADCCH15_ANA_SYSPLL  0x20
#define ADCCH15_ANA_TS      0x40
#define ADCCH15_ANA_USBPLL  0x80

#define ADCCH15_ANA_MAX     8

typedef struct {
    psfr_t sfr;             //SADCDAT SFR
    u16 channel;
    u8 adc15_ana_en;
    u8 adc15_ana_cur;
} saradc_cb_t;
extern saradc_cb_t saradc_cb;
extern volatile bool adc_chstops_flag;
extern volatile bool adc_vbat2_flag;

#define saradc_get_value10(n)   saradc_cb.sfr[n]                //获取某个channel的ADC值
#define saradc_get_value8(n)    ((u8)(saradc_cb.sfr[n]>>2))     //获取某个channel的ADC值
#define saradc_adc15_is_vrtc()  (bool)(saradc_cb.adc15_ana_cur == ADCCH15_ANA_VRTC)
#define saradc_adc15_is_ts()    (bool)(saradc_cb.adc15_ana_cur == ADCCH15_ANA_TS)
#define saradc_adc15_is_bg()    (bool)(saradc_cb.adc15_ana_cur == ADCCH15_ANA_BG)
#define saradc_adc15_is_vusb()  (bool)(saradc_cb.adc15_ana_cur == ADCCH15_ANA_VUSB)
#define saradc_adc15_is_mic()   (bool)(saradc_cb.adc15_ana_cur == ADCCH15_ANA_MIC)
#define saradc_kick_start(auto_anl_en, isr_en)  saradc_kick_start_do(saradc_cb.channel, auto_anl_en, isr_en)

void saradc_var_init(void);
void saradc_init(void);                                         //初始化ADC
uint16_t saradc_exit(void);                                     //关闭ADC
void saradc_set_baud(u16 div);                                  //设置ADC时钟分频
uint16_t saradc_set_channel(uint16_t channel);                  //设置ADC channel，返回值：设置前的channel
void saradc_clr_channel(uint16_t channel);                      //清除ADC channel
uint16_t saradc_get_channel(void);                              //获取ADC channel
void saradc_kick_start_do(u16 adc_ch, u8 auto_anl_en, u8 isr_en);//启动ADC转换
bool saradc_is_finish(void);                                    //ADC转换是否完成
void saradc_set_setup_time(u32 adc_ch,  u32 delay_clk);         //设置通道的采样延时
void saradc_adc15_analog_select(u8 ana_ch);                     //ADC15的模拟信号选择
void saradc_adc15_analog_next(void);                            //ADC15自动选择下一个模拟信号
u8 saradc_adc15_ana_set_channel(u8 ana_ch);                     //ADC15设置模拟通路
void saradc_adc15_ana_clr_channel(u8 ana_ch);                   //ADC15清除模拟通路
u32 saradc_vbat_get_calc_value(u32 vbat2, u32 bg, u32 vrtc_val, u32 vrtc_first); //根据ADC值计算VBAT电压
u32 saradc_vusb_get_calc_value(u32 vusb, u32 bg);               //根据ADC值计算VUSB电压
void saradc_adc15_analog_channel_sleep(void);
void mic_bias_trim_w4_done(void);
#endif
