#include "include.h"


//功能：配置IO数字使能
AT(.com_text.bsp.gpio)
bool bsp_gpio_de_en(u8 io_num)
{
    gpio_t gpio;
    gpio_cfg_init(&gpio, io_num);
    if (gpio.sfr) {
        gpio.sfr[GPIOxDE] |= BIT(gpio.num);
        return true;
    }
    return false;
}

//功能：配置上拉
// level_flag 设置开内部上拉：
// GPIOxPU,
// GPIOxPU200K,
// GPIOxPU300,
AT(.com_text.bsp.gpio)
bool bsp_gpio_pu_en(u8 io_num,u8 level_flag)
{
    gpio_t gpio;
    gpio_cfg_init(&gpio, io_num);
    if (gpio.sfr) {
        //开内部上拉或下拉,先取消了
        for(u8 i = GPIOxPU; i <= GPIOxPD300; i++ ){
            gpio.sfr[i] &= ~BIT(gpio.num);
        }
        gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
        gpio.sfr[level_flag] |= BIT(gpio.num);      //配置开内部上拉
        return true;
    }
    return false;
}
//功能：配置上拉
// level_flag 设置开内部下拉：
// GPIOxPD,
// GPIOxPD200K,
// GPIOxPD300,
AT(.com_text.bsp.gpio)
bool bsp_gpio_pd_en(u8 io_num,u8 level_flag)
{
    gpio_t gpio;
    gpio_cfg_init(&gpio, io_num);
    if (gpio.sfr) {
        //开内部上拉或下拉,先取消了
        for(u8 i = GPIOxPU; i <= GPIOxPD300; i++ ){
            gpio.sfr[i] &= ~BIT(gpio.num);
        }
        gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
        gpio.sfr[level_flag] |= BIT(gpio.num);      //配置开内部下拉
        return true;
    }
    return false;
}
//功能：配置上拉,关闭上下拉
// 内部上拉或下拉：
// GPIOxPU,
// GPIOxPD,
// GPIOxPU200K,
// GPIOxPD200K,
// GPIOxPU300,
// GPIOxPD300,
AT(.com_text.bsp.gpio)
bool bsp_gpio_no_pu_en(u8 io_num)
{
    gpio_t gpio;
    gpio_cfg_init(&gpio, io_num);
    if (gpio.sfr) {
        //开内部上拉或下拉,先取消了
        for(u8 i = GPIOxPU; i <= GPIOxPD300; i++ ){
            gpio.sfr[i] &= ~BIT(gpio.num);
        }
        return true;
    }
    return false;
}

//功能：读取io电平
// io_num :gpio pin
// level_flag 0 : 低电平，1 ： 高电平
AT(.com_text.bsp.gpio)
bool bsp_gpio_get_sta(u8 io_num)
{
    gpio_t gpio;
    gpio_cfg_init(&gpio, io_num);
    if (gpio.sfr) {
        return ((gpio.sfr[GPIOx] & BIT(gpio.num)) >> gpio.num);
    }
    return false;
}

// 功能：gpio 输出功能
// io_num :gpio pin
// level_flag 0 : 低电平，1 ： 高电平
AT(.com_text.bsp.gpio)
bool port_gpio_set_out(u8 io_num,bool level_flag)
{
    gpio_t gpio;
    if ((io_num == IO_NONE) || (io_num > IO_MAX_NUM)) {
        return false;
    }
    gpio_cfg_init(&gpio, io_num);
    gpio.sfr[GPIOxFEN] &= ~BIT(gpio.num);
    gpio.sfr[GPIOxDE] |= BIT(gpio.num);
    gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
    if(!level_flag) {
        gpio.sfr[GPIOxCLR] = BIT(gpio.num);
    } else {
        gpio.sfr[GPIOxSET] = BIT(gpio.num);
    }
    return true;
}

//设置输出电平
//level_flag 0 : 低电平，1 ： 高电平
u8 port_gpio_out_level(u8 io_num,bool level_flag)
{
    gpio_t gpio;
    if ((io_num == IO_NONE) || (io_num > IO_MAX_NUM)) {
        return false;
    }
    gpio_cfg_init(&gpio, io_num);
    if(!level_flag) {
        gpio.sfr[GPIOxCLR] = BIT(gpio.num);
    } else {
        gpio.sfr[GPIOxSET] = BIT(gpio.num);
    }
    return true;
}


// 功能：gpio 输入功能
// io_num :gpio pin
// level_flag 设置开内部上拉或下拉：(0为不配置上下拉)
// GPIOxPU,
// GPIOxPD,
// GPIOxPU200K,
// GPIOxPD200K,
// GPIOxPU300,
// GPIOxPD300,
bool port_gpio_set_in(u8 io_num,u8 level_flag)
{
    gpio_t gpio;
    if ((io_num == IO_NONE) || (io_num > IO_MAX_NUM)) {
        return false;
    }
    gpio_cfg_init(&gpio, io_num);
    gpio.sfr[GPIOxDE] |= BIT(gpio.num);
    gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
    gpio.sfr[GPIOxFEN] &= ~BIT(gpio.num);
    //开内部上拉或下拉,先取消了
    for(u8 i = GPIOxPU; i <= GPIOxPD300; i++ ) {
        gpio.sfr[i] &= ~BIT(gpio.num);
    }
    if(level_flag >= GPIOxPU && level_flag <= GPIOxPD300) {
        gpio.sfr[level_flag] |= BIT(gpio.num);      //配置开内部上拉或下拉
    }
    return true;
}

// 功能：gpio 设置模拟功能
// io_num :gpio pin
// level_flag 设置开内部上拉或下拉：(0为不配置上下拉)
// GPIOxPU,
// GPIOxPD,
// GPIOxPU200K,
// GPIOxPD200K,
// GPIOxPU300,
// GPIOxPD300,
bool port_gpio_set_func(u8 io_num,u8 level_flag)
{
    gpio_t gpio;
    if ((io_num == IO_NONE) || (io_num > IO_MAX_NUM)) {
        return false;
    }
    gpio_cfg_init(&gpio, io_num);
    gpio.sfr[GPIOxDE]  |= BIT(gpio.num);
    gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
    gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
    //开内部上拉或下拉,先取消了
    for(u8 i = GPIOxPU; i <= GPIOxPD300; i++ ) {
        gpio.sfr[i] &= ~BIT(gpio.num);
    }
    if(level_flag >= GPIOxPU && level_flag <= GPIOxPD300) {
        gpio.sfr[level_flag] |= BIT(gpio.num);      //配置开内部上拉或下拉
    }
    return true;
}


// 功能：gpio 设置高组态
bool port_gpio_disable(u8 io_num)
{
    gpio_t gpio;
    if ((io_num == IO_NONE) || (io_num > IO_PF5)) {
        return false;
    }
    gpio_cfg_init(&gpio, io_num);
    gpio.sfr[GPIOxDE] &= ~BIT(gpio.num);
    gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
    //开内部上拉或下拉,先取消了
    for(u8 i = GPIOxPU; i <= GPIOxPD300; i++ ){
        gpio.sfr[i] &= ~BIT(gpio.num);
    }
    return true;
}











