#include "include.h"


/*        G1    G2      G3      G4      G5

  PWM0:   PB3   PA2     PA11    PA3     PB8
  PWM1:   PB4   PA6     PA12    PB1     PB9
  PWM2:   PA0   PA7     PA13    PB2     PB10
  PWM3:   PA1   PA8     PA14    PB5     PB11
  PWM4:   PA4   PA9     PA15    PB6     PB12
  PWM5:   PA5   PA10    PB0     PB7     PE0
*/
static const int t5_pwm_gpio_reg[] = {
    IO_PB3, IO_PA2,  IO_PA11, IO_PA3, IO_PB8,
    IO_PB4, IO_PA6,  IO_PA12, IO_PB1, IO_PB9,
	IO_PA0,	IO_PA7,	 IO_PA13, IO_PB2, IO_PB10,
	IO_PA1,	IO_PA8,	 IO_PA14, IO_PB5, IO_PB11,
	IO_PA4,	IO_PA9,	 IO_PA15, IO_PB6, IO_PB12,
	IO_PA5,	IO_PA10, IO_PB0,  IO_PB7, IO_PE0,
};

static bool t5_pwm_freq_is_init = 0, t4_pwm_freq_is_init = 0, t3_pwm_freq_is_init = 0;


/*        G1    G2      G3      G4      G5

  PWM0:   PE0   PE2     PE4     PE6     PE8
  PWM1:   PE1   PE3     PE5     PE7     PE9
*/
static const int t4_pwm_gpio_reg[] = {
    IO_PE0, IO_PE2,  IO_PE4,  IO_PE6, IO_PE8, IO_NONE,
    IO_PE1, IO_PE3,  IO_PE5,  IO_PE7, IO_PE9, IO_NONE,
};


/*        G1      G2      G3

  PWM0:   PE10   PE12     PE14
  PWM1:   PE11   PE13     PG6
*/
static const int t3_pwm_gpio_reg[] = {
    IO_PE10, IO_PE12,  IO_PE14, IO_NONE,
    IO_PE11, IO_PE13,  IO_PG6, IO_NONE,
};



static u8 time_type_section(pwm_gpio gpio)
{
    if(gpio < TM4_PWM_GROUP_ID){
        return GROUP_TM5;
    }else if(gpio < TM3_PWM_GROUP_ID){
        return GROUP_TM4;
    }else{
        return GROUP_TM3;
    }
}


/**
 * @brief 设置PWM的频率
 * @param[in] freq 设置频率 Hz
 *
 * @return  返回是否成功
 **/
bool bsp_pwm_freq_set(pwm_gpio gpio, u32 freq)
{
    if(time_type_section(gpio) == GROUP_TM5){
        if(t5_pwm_freq_is_init == 0){
            CLKGAT0 |= BIT(5);
            t5_pwm_freq_is_init = 1;
            TMR5CON  = (2<<1)  | BIT(0);  // XOSC_CLK
        }
        TMR5PR   = 24000000/freq - 1;
        TMR5CPND = BIT(9);
        TMR5CNT  = 0;
    }else if(time_type_section(gpio) == GROUP_TM4){
        if(t4_pwm_freq_is_init == 0){
            CLKGAT0 |= BIT(4);
            t4_pwm_freq_is_init = 1;
            TMR4CON  = (2<<1)  | BIT(0);  // XOSC_CLK
        }
        TMR4PR   = 24000000/freq - 1;
        TMR4CPND = BIT(9);
        TMR4CNT  = 0;
    }else if(time_type_section(gpio) == GROUP_TM3){
        if(t3_pwm_freq_is_init == 0){
            CLKGAT0 |= BIT(3);
            t3_pwm_freq_is_init = 1;
            TMR3CON  = (2<<1)  | BIT(0);  // XOSC_CLK
        }
        TMR3PR   = 24000000/freq - 1;
        TMR3CPND = BIT(9);
        TMR3CNT  = 0;
    }

    return true;
}


/**
 * @brief 设置PWM0的占空比
 * @param[in] gpio   PWM0对应的GPIO选项
 * @param[in] duty   占空比，0-100%
 * @param[in] invert 输出是否反向
 *
 * @return  返回是否成功
 **/
bool bsp_pwm_duty_set(pwm_gpio gpio, u32 duty, bool invert)
{
    gpio_t gpio_reg;
    int group_num, group_id;
    volatile uint32_t *duty_reg;

    if(gpio >= GPIO_PWM0_MAX){
        return false;
    }

	if(duty > 100){
		duty = 100;
	}

    if(time_type_section(gpio) == GROUP_TM5){
        CLKGAT0 |= BIT(5);
        if(t5_pwm_freq_is_init == 0){
            bsp_pwm_freq_set(gpio, 20000);            //如果没初始化频率，默认设置为20K
        }

        //IO Init
        bsp_gpio_cfg_init(&gpio_reg, t5_pwm_gpio_reg[gpio]);
        gpio_reg.sfr[GPIOxDE] |= BIT(gpio_reg.num);
        gpio_reg.sfr[GPIOxPU] &= ~BIT(gpio_reg.num);
        gpio_reg.sfr[GPIOxDIR] &= ~ BIT(gpio_reg.num);

        group_num = gpio/5;
        group_id  = gpio - group_num*5;

        TMR5CON |= BIT(16 + group_num*2);
        TMR5CON |= invert << (17 + group_num*2);
        duty_reg  = (void *)&TMR5DUTY0 + group_num*4;
        *duty_reg = TMR5PR*duty/100;
        FUNCMCON1 = (FUNCMCON1 & (~(0xf << (8 + group_num*4)))) | ((group_id+1) << (8 + group_num*4));
    }else if(time_type_section(gpio) == GROUP_TM4){
        CLKGAT0 |= BIT(4);
        if(t4_pwm_freq_is_init == 0){
            bsp_pwm_freq_set(gpio, 20000);            //如果没初始化频率，默认设置为20K
        }

        //IO Init
        if(gpio == PG_MOTOR_TMR4) {
            //PWRCON4 |= BIT(9);
            //PWRCON4 = (PWRCON4 & ~(3 << 10)) | (0 << 10);         //DI_MOTOPG_PREDRV(0)
        } else if(gpio == PG_BL_TMR4) {
            //PWRCON4 |= BIT(5);
            //PWRCON4 = (PWRCON4 & ~(3 << 6)) | (0 << 6);         //DI_LEDPG_PREDRV(0)
        } else {
            bsp_gpio_cfg_init(&gpio_reg, t4_pwm_gpio_reg[gpio - TM4_PWM_GROUP_ID]);
            if(gpio_reg.sfr != NULL){
                gpio_reg.sfr[GPIOxDE] |= BIT(gpio_reg.num);
                gpio_reg.sfr[GPIOxPU] &= ~BIT(gpio_reg.num);
                gpio_reg.sfr[GPIOxDIR] &= ~ BIT(gpio_reg.num);
            }
        }

        gpio -= TM4_PWM_GROUP_ID;
        group_num = gpio/6;
        group_id  = gpio - group_num*6;

        TMR4CON |= BIT(16 + group_num*2);
        TMR4CON |= invert << (17 + group_num*2);
        duty_reg  = (void *)&TMR4DUTY0 + group_num*4;
        *duty_reg = TMR4PR*duty/100;

        FUNCMCON4 = (FUNCMCON4 & (~(0xf << (8 + group_num*4)))) | ((group_id+1) << (8 + group_num*4));
    } else if(time_type_section(gpio) == GROUP_TM3){
        CLKGAT0 |= BIT(3);
        if(t3_pwm_freq_is_init == 0){
            bsp_pwm_freq_set(gpio, 20000);            //如果没初始化频率，默认设置为20K
        }

        //IO Init
        if(gpio == PG_MOTOR_TMR3){
            //PWRCON4 |= BIT(9);
            //PWRCON4 = (PWRCON4 & ~(3 << 10)) | (0 << 10);         //DI_MOTOPG_PREDRV(0)
        }else if(gpio == PG_BL_TMR3){
            //PWRCON4 |= BIT(5);
            //PWRCON4 = (PWRCON4 & ~(3 << 6)) | (0 << 6);         //DI_LEDPG_PREDRV(0)
        }else{
            bsp_gpio_cfg_init(&gpio_reg, t3_pwm_gpio_reg[gpio - TM3_PWM_GROUP_ID]);
            if(gpio_reg.sfr != NULL){
                gpio_reg.sfr[GPIOxDE] |= BIT(gpio_reg.num);
                gpio_reg.sfr[GPIOxPU] &= ~BIT(gpio_reg.num);
                gpio_reg.sfr[GPIOxDIR] &= ~ BIT(gpio_reg.num);
            }
        }

        gpio -= TM3_PWM_GROUP_ID;
        group_num = gpio/4;
        group_id  = gpio - group_num*4;

        TMR3CON |= BIT(16 + group_num*2);
        TMR3CON |= invert << (17 + group_num*2);
        duty_reg  = (void *)&TMR3DUTY0 + group_num*4;
        *duty_reg = TMR3PR*duty/100;

        FUNCMCON4 = (FUNCMCON4 & (~(0xf << (0 + group_num*4)))) | ((group_id+1) << (0 + group_num*4));
    }

    return true;
}

/**
 * @brief 关闭pwmx
 * @param[in] gpio   PWMx对应的GPIO选项
 *
 * @return  返回是否成功
 **/
bool bsp_pwm_disable(pwm_gpio gpio)
{
    gpio_t gpio_reg;
    int pwm_num;

    if(gpio >= GPIO_PWM0_MAX) {
        return false;
    }

    if(time_type_section(gpio) == GROUP_TM5){
        //IO deinit
        bsp_gpio_cfg_init(&gpio_reg, t5_pwm_gpio_reg[gpio]);
        gpio_reg.sfr[GPIOxDE] &= ~BIT(gpio_reg.num);
        gpio_reg.sfr[GPIOxDIR] |= BIT(gpio_reg.num);

        //TMR5CON deinit
        pwm_num = gpio / 5;
        TMR5CON &= ~BIT(16 + pwm_num * 2);
        FUNCMCON1 = (FUNCMCON1 & (~(0xf << (8 + pwm_num * 4))));
    }else if(time_type_section(gpio) == GROUP_TM4){
    //IO deinit
		gpio -= TM4_PWM_GROUP_ID;
        bsp_gpio_cfg_init(&gpio_reg, t4_pwm_gpio_reg[gpio]);
        if(gpio_reg.sfr != NULL){
            gpio_reg.sfr[GPIOxDE] &= ~BIT(gpio_reg.num);
            gpio_reg.sfr[GPIOxDIR] |= BIT(gpio_reg.num);
        }

        //TMR5CON deinit
        pwm_num = gpio / 6;
        TMR4CON &= ~BIT(16 + pwm_num * 2);
        FUNCMCON4 = (FUNCMCON4 & (~(0xf << (8 + pwm_num * 4))));
    }else if(time_type_section(gpio) == GROUP_TM3){
    //IO deinit
		gpio -= TM3_PWM_GROUP_ID;
        bsp_gpio_cfg_init(&gpio_reg, t3_pwm_gpio_reg[gpio]);
        if(gpio_reg.sfr != NULL){
            gpio_reg.sfr[GPIOxDE] &= ~BIT(gpio_reg.num);
            gpio_reg.sfr[GPIOxDIR] |= BIT(gpio_reg.num);
        }

        //TMR5CON deinit
        pwm_num = gpio / 4;
        TMR3CON &= ~BIT(16 + pwm_num * 2);
        FUNCMCON4 = (FUNCMCON4 & (~(0x0 << (8 + pwm_num * 4))));
    }

    return true;
}


/**
 * 休眠唤醒需要重新初始化定时器
 */
void bsp_pwm_wakeup(void)
{
    t5_pwm_freq_is_init = 0;
    t4_pwm_freq_is_init = 0;
    t3_pwm_freq_is_init = 0;
}

