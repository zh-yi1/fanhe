#include "include.h"
#include "port_tft.h"

void port_tft_init(void)
{
    GPIOAFEN &= ~BIT(5);                        // CS
    GPIOADE  |=  BIT(5);
    GPIOASET  =  BIT(5);
    GPIOADIR &= ~BIT(5);

    GPIOAFEN |=  BIT(4);                        // CLK
    GPIOADE  |=  BIT(4);
    GPIOACLR  =  BIT(4);
    GPIOADIR &= ~BIT(4);

#if (GUI_MODE_SELECT == MODE_4WIRE_8BIT || GUI_MODE_SELECT == MODE_3WIRE_9BIT_2LINE)
    GPIOAFEN |= BIT(2);                         // D0
    GPIOADE  |= BIT(2);
    GPIOADIR |= BIT(2);

    DC_ENABLE();
#else
    GPIOAFEN |= (BIT(2)|BIT(3)|BIT(1)|BIT(0));  // D0/D1/D2/D3
    GPIOADE  |= (BIT(2)|BIT(3)|BIT(1)|BIT(0));
    GPIOADIR |= (BIT(2)|BIT(3)|BIT(1)|BIT(0));
#endif

    FUNCMCON2 = BIT(28);
}

void port_tft_reset(void)
{
    gpio_t gpio;
    bsp_gpio_cfg_init(&gpio, PORT_TFT_RST);
    if (gpio.sfr == NULL) {
        return;
    }

    gpio.sfr[GPIOxFEN] &= ~BIT(gpio.num);
    gpio.sfr[GPIOxDE] |= BIT(gpio.num);
    gpio.sfr[GPIOxSET] = BIT(gpio.num);
    gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
    delay_ms(10);
    gpio.sfr[GPIOxCLR] = BIT(gpio.num);
    delay_ms(20);
    gpio.sfr[GPIOxSET] = BIT(gpio.num);
    delay_ms(50);
}

void port_tft_exit(void)
{
    GPIOADE  &= ~(BIT(2)|BIT(3)|BIT(1)|BIT(0)|BIT(4)|BIT(5)|BIT(7));
    GPIOADIR |= (BIT(2)|BIT(3)|BIT(1)|BIT(0)|BIT(4)|BIT(5)|BIT(7));
}
