#ifndef _BSP_VBAT_H
#define _BSP_VBAT_H

#define VBAT_PERCENT_STEP                               1  // 1mv步进

typedef struct __attribute__((packed)) _vbat_cb_t {
    u16 vbat_adc_curr;          //用于中间过渡的电压值
    u16 vbat_adc_last;          //上一次拿去实际显示的电压值
} vbat_cb_t;


u16 bsp_vbat_get_voltage(u32 rst_flag);
void bsp_vbat_voltage_init(void);
int bsp_vbat_get_lpwr_status(void);         //0: 正常电压, 1:低电提醒状态, 2:低电关机状态
void bsp_vbat_lpwr_process(void);



/**
 * 电池百分比标定初始化
 */
void bsp_vbat_percent_init(void);


/**
 * 获取电池电量百分比 
 * 返回: 0~100
 */
u16 bsp_vbat_percent_get(void);

#endif // _BSP_VBAT_H
