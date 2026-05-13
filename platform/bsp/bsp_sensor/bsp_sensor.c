#include "include.h"

sensor_sta_t bsensor;
//获得外设初始化状态
AT(.com_text.sensor)
u8 bsp_sensor_init_sta_get(sensor_init_type type)
{
    return (bsensor.init & type);
}

//设置外设初始化状态
void bsp_sensor_init_sta_set(sensor_init_type type)
{
    bsensor.init |= type;
}

//清除外设初始化状态
void bsp_sensor_init_sta_clr(sensor_init_type type)
{
    bsensor.init &= ~type;
}

//开 外设共用电源控制脚 PE2
void bsp_sensor_pe2_pwr_pg_on(void)
{
    GPIOFDE |= BIT(2);
    GPIOFDIR &= ~BIT(2);
    GPIOFCLR |= BIT(2);
}

//关 外设共用电源控制脚 PE2
void bsp_sensor_pe2_pwr_pg_off(void)
{
    GPIOFDE &= ~BIT(2);
    GPIOFDIR |= BIT(2);
    GPIOFSET |= BIT(2);
}

//心率开启
u8 bsp_sensor_hr_init(u8 mode)
{
    if (bsp_sensor_init_sta_get(SENSOR_INIT_HR)) return true;
    bool init = false;
    hr_vdd_ldo_on();
#if (SENSOR_HR_SEL == SENSOR_HR_TYHX_HX3605)
    init = sensor_hx3605_init(mode);
#elif(SENSOR_HR_SEL == SENSOR_HR_TYHX_HRS3300)
    init = sensor_hrs3300_init(mode);
#endif

    if (init) bsp_sensor_init_sta_set(SENSOR_INIT_HR);

    return init;
}

//心率关闭
u8 bsp_sensor_hr_stop(void)
{
    if (!bsp_sensor_init_sta_get(SENSOR_INIT_HR)) return false;
    bool stop = true;
    hr_vdd_ldo_off();
#if (SENSOR_HR_SEL == SENSOR_HR_TYHX_HX3605)
    stop = sensor_hx3605_stop();
#elif(SENSOR_HR_SEL == SENSOR_HR_TYHX_HRS3300)
    stop = sensor_hrs3300_stop();
#endif

    if (!stop) bsp_sensor_init_sta_clr(SENSOR_INIT_HR);

    return stop;
}

//保存心率值
void bsp_sensor_hrs_data_save(u8 bpm)
{
    if (bsensor.hr.back_bpm != bpm) {
        bsensor.hr.back_bpm = bpm;
        printf("%s: %d\n", __func__, bpm);
    }
}

//保存血氧值
void bsp_sensor_spo2_data_save(u8 spo2)
{
    if (bsensor.hr.spo2_value != spo2) {
        bsensor.hr.spo2_value = spo2;
        printf("%s: %d\n", __func__, spo2);
    }
}

//保存血压值
void bsp_sensor_bp_data_save(u8 sbp, u8 dbp)
{
    if (bsensor.hr.sbp != sbp || bsensor.hr.dbp != dbp) {
        bsensor.hr.sbp = sbp;
        bsensor.hr.dbp = dbp;
        printf("%s: sbp->%d, dbp->%d\n", __func__, sbp, dbp);
    }
}

//获取心率值
u8 bsp_sensor_hrs_data_get(void)
{
	return bsensor.hr.back_bpm;
}

//获取血氧值
u8 bsp_sensor_spo2_data_get(void)
{
    return bsensor.hr.spo2_value;
}

//获取血压值
void bsp_sensor_bp_data_get(u8 *sbp, u8 *dbp)
{
    if (NULL == sbp || NULL == dbp) {
        return ;
    }

    *sbp = bsensor.hr.sbp;
    *dbp = bsensor.hr.dbp;
}

//获取佩戴状态
bool bsp_sensor_hr_wear_sta_get(void)
{
    bool sta = false;

#if (SENSOR_HR_SEL == SENSOR_HR_TYHX_HX3605)
    sta = sensor_hx3605_wear_sta_get();
#elif(SENSOR_HR_SEL == SENSOR_HR_TYHX_HRS3300)
    sta = sensor_hx3300_wear_sta_get();
#endif

    return sta;
}

//空函数
void sensor_step_init(void) {}
void sensor_step_stop(void) {}

AT(.sleep_text.sleep.sensor.pro)
void sensor_step_lowpwr_pro(void) {}

AT(.com_text.sleep.sensor.isr)
void sensor_step_pro_isr(void) {}

void sensor_step_wrist_set(bool x) {}
u8   sensor_step_wrist_get(void) {return 0;}
s8   sensor_step_wkup_get(void) {return 0;}
void sensor_step_sway_set(bool x) {}
bool sensor_step_sway_get(void) {return 0;}
bool sensor_step_sway_status_get(void) {return 0;}
int  sensor_step_cur_steps_get(void) {return 0;}
int  sensor_step_cur_kcal_get(void) {return 0;}
int  sensor_step_cur_distance_get(void) {return 0;}
