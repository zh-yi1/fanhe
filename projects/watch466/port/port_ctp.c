//#include "include.h"
//#include "port_ctp.h"
//#if CTP_SELECT != CTP_NO
//
//void port_ctp_init(void)
//{
//    gpio_t gpio;
//
//    bsp_gpio_cfg_init(&gpio, PORT_CTP_SCL);
//    if (gpio.sfr) {
//        gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
//        gpio.sfr[GPIOxPU] |= BIT(gpio.num);
//        gpio.sfr[GPIOxDE] |= BIT(gpio.num);
//        gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
//    }
//
//    bsp_gpio_cfg_init(&gpio, PORT_CTP_SDA);
//    if (gpio.sfr) {
//        gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
//        gpio.sfr[GPIOxPU] |= BIT(gpio.num);
//        gpio.sfr[GPIOxDE] |= BIT(gpio.num);
//        gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
//    }
//
//    bsp_gpio_cfg_init(&gpio, PORT_CTP_RST);
//    if (gpio.sfr) {
//        gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
//        gpio.sfr[GPIOxSET] = BIT(gpio.num);
//        gpio.sfr[GPIOxDE] |= BIT(gpio.num);
//        gpio.sfr[GPIOxFEN] &= ~BIT(gpio.num);
//    }
//
//#if PORT_CTP_IIC_HW == 0
//    SENSHBCON = (PORT_CTP_MAP_GPIO << 5);
//#elif PORT_CTP_IIC_HW == 1
//    FUNCMCON2 = (PORT_CTP_MAP_GPIO << 12);
//#elif PORT_CTP_IIC_HW == 2
//    FUNCMCON2 = (PORT_CTP_MAP_GPIO << 16);
//#endif
//
//}
//
//void port_ctp_exit(void)
//{
//    gpio_t gpio;
//    bsp_gpio_cfg_init(&gpio, PORT_CTP_SCL);
//    if (gpio.sfr) {
//        gpio.sfr[GPIOxPU] &= ~BIT(gpio.num);
//        gpio.sfr[GPIOxDE] &= ~BIT(gpio.num);
//    }
//
//    bsp_gpio_cfg_init(&gpio, PORT_CTP_SDA);
//    if (gpio.sfr) {
//        gpio.sfr[GPIOxPU] &= ~BIT(gpio.num);
//        gpio.sfr[GPIOxDE] &= ~BIT(gpio.num);
//    }
//
//    bsp_gpio_cfg_init(&gpio, PORT_CTP_RST);
//    if (gpio.sfr) {
//        gpio.sfr[GPIOxDE] &= ~BIT(gpio.num);
//        gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
//    }
//}
//#endif // CTP_SELECT
