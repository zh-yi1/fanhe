#ifndef __SENSOR_H
#define __SENSOR_H

#include "sc7a20/sc7a20.h"
#include "hrs3300/fit_hrs3300.h"
#include "hx3605/fit_hx3605.h"
#include "hx3602/fit_hx3602.h"
#include "msa310/msa310.h"
#include "internal_sensor/bsp_internal_sensor.h"
#if  (SENSOR_GEO_SEL == SENSOR_GEO_QMC6309)
#include "qmc6309/qmc6309.h"
#endif

typedef enum {
    SENSOR_INIT_ALL  = 0xFF,
    SENSOR_INIT_STEP = BIT(0),
    SENSOR_INIT_HR   = BIT(1),
    SENSOR_INIT_GEO  = BIT(2),
}sensor_init_type;

typedef struct {
    bool                                    int_en;
    bool                                    mutex;
    bool                                    isr_kick;
}step_sta_t;

typedef struct {
	u8 back_bpm;
    u8 spo2_value;
    u8 sbp;
    u8 dbp;
}hr_sta_t;

typedef struct {

}geo_sta_t;

typedef struct sensor_sta_t_ {
    step_sta_t                              step;
    hr_sta_t                                hr;
    geo_sta_t                               geo;
    u8                                      init;
}sensor_sta_t;

//外设基础函数
extern sensor_sta_t bsensor;
//获得外设初始化状态
u8   bsp_sensor_init_sta_get(sensor_init_type type);
//设置外设初始化状态
void bsp_sensor_init_sta_set(sensor_init_type type);
//清除外设初始化状态
void bsp_sensor_init_sta_clr(sensor_init_type type);
//开 外设共用电源控制脚 PE2
void bsp_sensor_pe2_pwr_pg_on(void);
//关 外设共用电源控制脚 PE2
void bsp_sensor_pe2_pwr_pg_off(void);

//心率开启
u8   bsp_sensor_hr_init(u8 mode);
//心率关闭
u8   bsp_sensor_hr_stop(void);
//保存心率值
void bsp_sensor_hrs_data_save(u8 bpm);
//保存血氧值
void bsp_sensor_spo2_data_save(u8 spo2);
//获取血压值
void bsp_sensor_bp_data_save(u8 sbp, u8 dbp);
//获取心率值
u8 bsp_sensor_hrs_data_get(void);
//获取血氧值
u8 bsp_sensor_spo2_data_get(void);
//获取血压值
void bsp_sensor_bp_data_get(u8 *sbp, u8 *dbp);
//获取佩戴状态
bool bsp_sensor_hr_wear_sta_get(void);

//空函数
void sensor_step_init(void);
void sensor_step_stop(void);
void sensor_step_lowpwr_pro(void);
void sensor_step_pro_isr(void);
void sensor_step_wrist_set(bool x);
u8   sensor_step_wrist_get(void);
s8   sensor_step_wkup_get(void);
void sensor_step_sway_set(bool x);
bool sensor_step_sway_get(void);
bool sensor_step_sway_status_get(void);
int sensor_step_cur_steps_get(void);
int sensor_step_cur_kcal_get(void);
int sensor_step_cur_distance_get(void);

//计步
#if (SENSOR_STEP_SEL == SENSOR_STEP_SC7A20)
    #define bsp_sensor_step_init()                      sc7a20_init()
    #define bsp_sensor_step_stop()                      sensor_step_stop()
    #define bsp_sensor_step_lowpwr_pro()                sc7a20_lowpwr_process()
    #define bsp_sensor_step_pro_isr()                   sc7a20_isr_process()
    #define bsp_sensor_step_wrist_set(x)                sc7a20_wrist_set(x)
    #define bsp_sensor_step_wrist_get()                 sc7a20_wrist_get()
    #define bsp_sensor_step_wkup_get()                  sc7a20_wkup_get()
    #define bsp_sensor_step_sway_set(x)                 sc7a20_sway_set(x)          //注意启用摇晃功能会无计步，使用完需关闭
    #define bsp_sensor_step_sway_get()                  sc7a20_sway_get()
    #define bsp_sensor_step_sway_status_get()           sc7a20_sway_status_get()
    #define bsp_sensor_step_cur_steps_get()             sc7a20_cur_steps_get()
    #define bsp_sensor_step_cur_kcal_get()              sc7a20_cur_kcal_get()
    #define bsp_sensor_step_cur_distance_get()          sc7a20_cur_distance_get()
#elif (SENSOR_STEP_SEL == SENSOR_STEP_MSA310)
    #define bsp_sensor_step_init()                      msa310_init()
    #define bsp_sensor_step_stop()                      msa310_stop()
    #define bsp_sensor_step_lowpwr_pro()                sensor_step_lowpwr_pro()
    #define bsp_sensor_step_pro_isr()               	sensor_step_pro_isr()
    #define bsp_sensor_step_wrist_set(x)                sensor_step_wrist_set(x)
    #define bsp_sensor_step_wrist_get()                 sensor_step_wrist_get()
    #define bsp_sensor_step_wkup_get()                  sensor_step_wkup_get()
    #define bsp_sensor_step_sway_set(x)                 sensor_step_sway_set(x)
    #define bsp_sensor_step_sway_get()                  sensor_step_sway_get()
    #define bsp_sensor_step_sway_status_get()           sensor_step_sway_status_get()
    #define bsp_sensor_step_cur_steps_get()             sensor_step_cur_steps_get()
    #define bsp_sensor_step_cur_kcal_get()              sensor_step_cur_kcal_get()
    #define bsp_sensor_step_cur_distance_get()          sensor_step_cur_distance_get()
#elif (SENSOR_STEP_SEL == SENSOR_STEP_INTER)
    #define bsp_sensor_step_init()                      bsp_internal_sensor_init()
    #define bsp_sensor_step_stop()                      bsp_internal_sensor_stop()
    #define bsp_sensor_step_lowpwr_pro()                sensor_step_lowpwr_pro()
    #define bsp_sensor_step_pro_isr()               	sensor_step_pro_isr()
    #define bsp_sensor_step_wrist_set(x)                sensor_step_wrist_set(x)
    #define bsp_sensor_step_wrist_get()                 sensor_step_wrist_get()
    #define bsp_sensor_step_wkup_get()                  sensor_step_wkup_get()
    #define bsp_sensor_step_sway_set(x)                 sensor_step_sway_set(x)
    #define bsp_sensor_step_sway_get()                  sensor_step_sway_get()
    #define bsp_sensor_step_sway_status_get()           sensor_step_sway_status_get()
    #define bsp_sensor_step_cur_steps_get()             sensor_step_cur_steps_get()
    #define bsp_sensor_step_cur_kcal_get()              sensor_step_cur_kcal_get()
    #define bsp_sensor_step_cur_distance_get()          sensor_step_cur_distance_get()
#else
    #define bsp_sensor_step_init()                      sensor_step_init()                              //初始化
    #define bsp_sensor_step_stop()                      sensor_step_stop()                              //关闭
    #define bsp_sensor_step_lowpwr_pro()                sensor_step_lowpwr_pro()                        //lowpwr处理
    #define bsp_sensor_step_pro_isr()               	sensor_step_pro_isr()                           //中断处理
    #define bsp_sensor_step_wrist_set(x)                sensor_step_wrist_set(x)                        //抬腕亮屏功能设置
    #define bsp_sensor_step_wrist_get()                 sensor_step_wrist_get()                         //抬腕亮屏功能状态
    #define bsp_sensor_step_wkup_get()                  sensor_step_wkup_get()                          //抬腕亮屏是否触发
    #define bsp_sensor_step_sway_set(x)                 sensor_step_sway_set(x)                         //摇晃功能状态设置
    #define bsp_sensor_step_sway_get()                  sensor_step_sway_get()                          //摇晃功能状态获取
    #define bsp_sensor_step_sway_status_get()           sensor_step_sway_status_get()                   //摇晃状态获取
    #define bsp_sensor_step_cur_steps_get()             sensor_step_cur_steps_get()                     //当前步数获取
    #define bsp_sensor_step_cur_kcal_get()              sensor_step_cur_kcal_get()                      //当前卡路里获取
    #define bsp_sensor_step_cur_distance_get()          sensor_step_cur_distance_get()                  //当前距离获取
#endif


#endif // __SENSOR_H
