#ifndef _BSP_PWM_H
#define _BSP_PWM_H

#define TM5_PWM_GROUP_ID                0
#define TM4_PWM_GROUP_ID                50
#define TM3_PWM_GROUP_ID                100


enum
{
    GROUP_TM5,
    GROUP_TM4,
    GROUP_TM3,
};

typedef enum
{
//time5 pwm 属于该组的所有PWM频率一致，占空比各自可调
	GPIO_PB3 = TM5_PWM_GROUP_ID, //TM5 PWM0
	GPIO_PA2,
	GPIO_PA11,
    GPIO_PA3,
    GPIO_PB8,

	GPIO_PB4,     //TM5 PWM1
	GPIO_PA6,
	GPIO_PA12,
    GPIO_PB1,
    GPIO_PB9,

	GPIO_PA0,     //TM5 PWM2
	GPIO_PA7,
	GPIO_PA13,
    GPIO_PB2,
    GPIO_PB10,

	GPIO_PA1,     //TM5 PWM3
	GPIO_PA8,
	GPIO_PA14,
    GPIO_PB5,
    GPIO_PB11,

	GPIO_PA4,     //TM5 PWM4
	GPIO_PA9,
	GPIO_PA15,
    GPIO_PB6,
    GPIO_PB12,

	GPIO_PA5,     //TM5 PWM5
	GPIO_PA10,
	GPIO_PB0,
    GPIO_PB7,
    GPIO_PE0,

//time4 pwm 属于该组的所有PWM频率一致，占空比各自可调
	GPIO_T4_PE0 = TM4_PWM_GROUP_ID,    //TM4 PWM0
	GPIO_PE2,
	GPIO_PE4,
    GPIO_PE6,
    GPIO_PE8,
    PG_MOTOR_TMR4,

	GPIO_PE1,    //TM4 PWM1
	GPIO_PE3,
	GPIO_PE5,
    GPIO_PE7,
    GPIO_PE9,
    PG_BL_TMR4,

//time3 pwm 属于该组的所有PWM频率一致，占空比各自可调
	GPIO_PE10 = TM3_PWM_GROUP_ID,    //TM3 PWM0
	GPIO_PE12,
	GPIO_PE14,
    PG_MOTOR_TMR3,

	GPIO_PE11,    //TM3 PWM1
	GPIO_PE13,
	GPIO_PG6,
    PG_BL_TMR3,

    GPIO_PWM0_MAX,
    GPIO_NONE,
}pwm_gpio;


/**
 * @brief 设置PWM0的占空比
 * @param[in] gpio   PWM0对应的GPIO选项
 * @param[in] duty   占空比，0-100%
 * @param[in] invert 输出是否反向
 *
 * @return  返回是否成功
 **/
bool bsp_pwm_duty_set(pwm_gpio gpio, u32 duty, bool invert);



/**
 * @brief 关闭pwmx
 * @param[in] gpio   PWMx对应的GPIO选项
 *
 * @return  返回是否成功
 **/
bool bsp_pwm_disable(pwm_gpio gpio);



/**
 * 休眠唤醒需要重新初始化定时器
 */
void bsp_pwm_wakeup(void);

#endif

