#ifndef _BSP_GPIO__H
#define _BSP_GPIO__H

bool bsp_gpio_de_en(u8 io_num);

//功能：配置上拉
// level_flag 设置开内部上拉：
// GPIOxPU,
// GPIOxPU200K,
// GPIOxPU300,
bool bsp_gpio_pu_en(u8 io_num,u8 level_flag);

//功能：配置下拉
// level_flag 设置开内部下拉：
// GPIOxPD,
// GPIOxPD200K,
// GPIOxPD300,
bool bsp_gpio_pd_en(u8 io_num,u8 level_flag);

//功能：读取io电平
bool bsp_gpio_get_sta(u8 io_num);

//功能：配置上拉,关闭上下拉
// 内部上拉或下拉：
// GPIOxPU,
// GPIOxPD,
// GPIOxPU200K,
// GPIOxPD200K,
// GPIOxPU300,
// GPIOxPD300,
bool bsp_gpio_no_pu_en(u8 io_num);

// 功能：gpio 输出功能
// io_num :gpio pin
// level_flag 0 : 低电平，1 ： 高电平
bool port_gpio_set_out(u8 io_num,bool level_flag);

// 功能：gpio 设置高组态
bool port_gpio_disable(u8 io_num);

// 功能：gpio 输入功能
// io_num :gpio pin 
// level_flag 设置输入上下：
// GPIOxPU,
// GPIOxPD,
// GPIOxPU200K,
// GPIOxPD200K,
// GPIOxPU300,
// GPIOxPD300,
bool port_gpio_set_in(u8 io_num,u8 level_flag);
// 功能：gpio 设置模拟功能
// io_num :gpio pin
// level_flag 设置开内部上拉或下拉：(0为不配置上下拉)
// GPIOxPU,
// GPIOxPD,
// GPIOxPU200K,
// GPIOxPD200K,
// GPIOxPU300,
// GPIOxPD300,
bool port_gpio_set_func(u8 io_num,u8 level_flag);
//功能：读取io电平
u8 port_gpio_readlevel(u8 io_num);

//设置输出电平
//level_flag 0 : 低电平，1 ： 高电平
u8 port_gpio_out_level(u8 io_num,bool level_flag);
#endif // _BSP_GPIO_H