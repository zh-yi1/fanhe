#ifndef _API_PWR_H_
#define _API_PWR_H_

void pmu_init(u8 buck_en);
void set_buck_mode(u8 buck_en);
void hr_vdd_ldo_on(void); //开VDDHR 0x1c 3.3v
void hr_vdd_ldo_on_2_8v(void);//VDDHR 0x14: 2.8V
void hr_vdd_ldo_off(void);                  //关VDDHR
void gpu_pg_on(void);                       //开GPU power gate
void gpu_pg_off(void);                      //关GPU power gate
void led_pg_on(void);                       //开背光power gate 使用PWM后这个函数失效(使用PWM的话，pwm_gpio里面可以选择)
void led_pg_off(void);                      //关背光power gate 使用PWM后这个函数失效(使用PWM的话，pwm_gpio里面可以选择)
void led_pg_drv_select(bool lv);            //背光驱动能力 0: 5R 1:2.5R
void led_pg_speed_select(u8 speed);         //范围 0->3，值越大反转速度越快(切换时间越短)
void lcd_pg_on(void);                       //开VDDLCD
void lcd_pg_off(void);                      //关VDDLCD
void nand_pg_on(void);                      //开VDDNAND
void nand_pg_off(void);                     //关VDDNAND
void motor_pg_on(void);                     //开motor power gate 使用PWM后这个函数失效(使用PWM的话，pwm_gpio里面可以选择)
void motor_pg_off(void);                    //开motor power gate 使用PWM后这个函数失效(使用PWM的话，pwm_gpio里面可以选择)
void motor_pg_drv_select(bool lv);          //motor驱动能力 0: 5R 1:2.5R
void motor_pg_speed_select(u8 speed);       //范围 0->3，值越大反转速度越快(切换时间越短)
void pmu_vio0v_mode_exit(void);             //退出0v充电启动的电源配置
#endif // _API_PWR_H_

