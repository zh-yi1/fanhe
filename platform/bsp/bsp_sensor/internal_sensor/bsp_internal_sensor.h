#ifndef _BSP_INTERNAL_SENSOR_H
#define _BSP_INTERNAL_SENSOR_H

#if (SENSOR_STEP_SEL == SENSOR_STEP_INTER)
typedef enum{
    LOW_VLD = 1,
    HIGH_VLD,
    BOTH_VLD,
} sensor_vld_t;

typedef enum{
    HIGH_ISR = 1,
    LOW_ISR  = 2,
    FIFO_OVERFLOW_ISR = 4,
}sensor_st_t;

typedef enum{
    INTER_SENSOR_PE6  = 0,
    INTER_SENSOR_PE3  = 1,
    INTER_SENSOR_PB7  = 2,
}sensor_gpio_group_t;

void bsp_internal_sensor_init(void);
void bsp_internal_sensor_stop(void);

#endif // (SENSOR_STEP_SEL == SENSOR_STEP_INTER)
#endif // _BSP_INTERNAL_SENSOR_H
