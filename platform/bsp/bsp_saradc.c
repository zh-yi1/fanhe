#include "include.h"

adc_cb_t adc_cb AT(.buf.key.cb);

void bsp_qdec_adc_process(u8 adc_val);

AT(.com_text.saradc.process)
bool bsp_saradc_process(u32 auto_en)
{
    if (!saradc_is_finish()) {
        return false;
    }
    if (saradc_adc15_is_vrtc()) {
        adc_cb.vrtc_val = saradc_get_value10(ADCCH_VRTC);
    }
    if (saradc_adc15_is_bg()) {
        adc_cb.bg = saradc_get_value10(ADCCH_BGOP);
    }
    if (saradc_adc15_is_mic()) {
        adc_cb.mic_volt = saradc_get_value10(ADCCH_MIC);
    }
#if TS_MODE_EN
    if (saradc_adc15_is_ts()) {
        adc_cb.vts = saradc_get_value10(ADCCH_VTS);
    }
#endif
#if VUSB_DETECT_EN
    if (saradc_adc15_is_vusb()) {
        adc_cb.vusb = saradc_get_value10(ADCCH_VUSB);
    }
#endif
    if (auto_en) {
        saradc_adc15_analog_next();
        saradc_kick_start(0, 0);                                    //启动下一次ADC转换
    }
    return true;
}

#define DC_IN()                  ((RTCCON >> 20) & 0x01)

AT(.com_text.saradc.process)
static void bsp_saradc_kick_process(void)
{
#if CHARGE_VOL_DYNAMIC_DET
    if (!DC_IN()) {
        saradc_kick_start(0, 0);                                    //每5毫秒启动全部ADC通路转换一次
        adc_cb.tmr1ms_cnt = 0;
    } else {
        if ((adc_cb.tmr1ms_cnt % 5000) == 0) {                      //充电状态, 每5s采样一次VBAT, 停10ms
            adc_cb.tmr1ms_cnt = 0;
            if(sys_cb.chg_on){
                gradient_process(1);
                RTCCON8 |= BIT(1);                                  //charge stop
                adc_chstops_flag = true;                            //转换完成, 中断清变量
            }
            saradc_adc15_analog_select(ADCCH15_ANA_BG);             //参考信号需要与vbat一起ADC转换
            saradc_kick_start(0, 1);                                //全部ADC通路转换一次
        } else {
            if((adc_cb.tmr1ms_cnt % 5000) == 4990){
                if(sys_cb.chg_on){
                    gradient_process(1);
                    RTCCON8 |= BIT(1);                              //charge stop
                }
            }
            saradc_kick_start_do(saradc_cb.channel & ~BIT(ADCCH_VBAT), 0, 0);
        }
    }
#else
    saradc_kick_start(0, 0);                                       //每5毫秒启动全部ADC通路转换一次
#endif
}

AT(.com_text.saradc.process)
void bsp_saradc_tmr1ms_process(void)
{
    adc_cb.tmr1ms_cnt++;
#if USER_ADKEY_QDEC_EN
    if (bsp_saradc_process(0)) {
        if ((adc_cb.tmr1ms_cnt % 5) == 0) {
            saradc_adc15_analog_next();
            bsp_saradc_kick_process();
        } else {
            saradc_kick_start_do(BIT(USER_QDEC_ADCH), 0, 0);        //仅QDEC ADC每毫秒采样一次
        }
        bsp_qdec_adc_process(saradc_get_value8(USER_QDEC_ADCH));
    }
#else
    if ((adc_cb.tmr1ms_cnt % 5) == 0) {
        bsp_saradc_process(0);
        saradc_adc15_analog_next();
        bsp_saradc_kick_process();
    }
#endif // USER_ADKEY_QDEC_EN
}


void bsp_saradc_init(void)
{
    memset(&adc_cb, 0, sizeof(adc_cb));
    saradc_init();
    //始化时采一次BG电压
    saradc_adc15_ana_set_channel(ADCCH15_ANA_BG);
    saradc_adc15_analog_select(ADCCH15_ANA_BG);
    delay_us(600);
    saradc_kick_start(0, 0);
    while(!bsp_saradc_process(0));

    //采集BG对应的VRTC电压
    saradc_adc15_ana_set_channel(ADCCH15_ANA_VRTC);
    saradc_adc15_analog_select(ADCCH15_ANA_VRTC);
#if TS_MODE_EN
    saradc_adc15_ana_set_channel(ADCCH15_ANA_TS);
#endif
#if VUSB_DETECT_EN
    saradc_adc15_ana_set_channel(ADCCH15_ANA_VUSB);
#endif
    delay_us(600);
    saradc_kick_start(0, 0);
    while(!bsp_saradc_process(1));
    adc_cb.vrtc_first = adc_cb.vrtc_val;

#if VBAT_DETECT_EN
    bsp_vbat_voltage_init();
#endif
}

uint16_t bsp_saradc_exit(void)
{
    return saradc_exit();
}

u16 saradc_get_mic_volt_val(void)
{
    return adc_cb.mic_volt;
}

u16 saradc_get_bg_volt_val(void)
{
    return adc_cb.bg;
}

#if TS_MODE_EN

//----NTC dac table部分
//
#define NTC_ADC_TABLE_MAX           33

static const int ntc_adc_value_table[NTC_ADC_TABLE_MAX][2] ={
    {233,    -40},
    {237,    -35},
    {241,    -30},
    {245,    -25},
    {250,    -20},
    {255,    -15},
    {259,    -10},
    {264,    -5},
    {269,    0},
    {275,    5},
    {279,    10},
    {285,    15},
    {290,    20},
    {295,    25},
    {300,    30},
    {306,    35},
    {311,    40},
    {317,    45},
    {322,    50},
    {327,    55},
    {332,    60},
    {338,    65},
    {343,    70},
    {348,    75},
    {353,    80},
    {358,    85},
    {363,    90},
    {369,    95},
    {375,    100},
    {379,    105},
    {384,    110},
    {389,    115},
    {395,    120}
};

/**
 * NTC ADC 转 温度
 * 输入dac值，返回实际温度
*/
s16 ts_adc2ntc(int adc_value)
{
    int id, temp;

    if(adc_value <= ntc_adc_value_table[0][0]){
        return ntc_adc_value_table[0][1];
    }

    if(adc_value >= ntc_adc_value_table[NTC_ADC_TABLE_MAX-1][0]){
        return ntc_adc_value_table[NTC_ADC_TABLE_MAX-1][1];
    }

    for(id=0; id<NTC_ADC_TABLE_MAX; id++){
        if(ntc_adc_value_table[id][0] >= adc_value){
            break;
        }
    }

    temp = adc_value-ntc_adc_value_table[id-1][0];
    temp = temp*5/(ntc_adc_value_table[id][0] - ntc_adc_value_table[id-1][0]) + ntc_adc_value_table[id-1][1];

    return temp;
}
#endif


