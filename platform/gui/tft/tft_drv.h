#ifndef __TFT_DRV_H__
#define __TFT_DRV_H__

//彩屏IO分配
/*
        QSPI        3WIRE_9BIT       3WIRE_9BIT_2LINE        4WIRE_8BIT          8080

LDO     HR_LDO       HR_LDO              HR_LDO                HR_LDO           HR_LDO       //供电控制脚

BL      GPIO_PA14    GPIO_PA14           GPIO_PA14             GPIO_PA14        GPIO_PA14    //背光(OLED屏没有)

TE      IO_PE9       IO_PE9              IO_PE9                IO_PE9           IO_PE9

REST    IO_PE8       IO_PE8              IO_PE8                IO_PE8           IO_PE8

CS      IO_PA5       IO_PA5              IO_PA5                IO_PA5           IO_PA5

CLK     IO_PA4       IO_PA4              IO_PA4                IO_PA4           IO_PA4

RS       X              X                   X                  IO_PA3           IO_PA3

D0      IO_PA2      IO_PA2                IO_PA2               IO_PA2           IO_PA2

D1      IO_PA1          X                 IO_PA3                  X             IO_PA1

D2      IO_PA0          X                   X                     X             IO_PA0

D3      IO_PA3          X                   X                     X             IO_PE10

D4       X              X                   X                     X             IO_PE11

D5       X              X                   X                     X             IO_PE12

D6       X              X                   X                     X             IO_PE13

D7       X              X                   X                     X             IO_PE14

*/
////TE
//#define PORT_TFT_INT                        IO_PA6
//#define PORT_TFT_INT_VECTOR                 PORT_INT2_VECTOR
//
////RST
//#define PORT_TFT_RST                        IO_PA7
//#define PORT_TFT_RST_H()                    GPIOASET = BIT(7);
//#define PORT_TFT_RST_L()                    GPIOACLR = BIT(7);
//
////CS
//#define PORT_TFT_SPI_CS                     IO_PA5
//#define TFT_SPI_CS_EN()                     GPIOACLR = BIT(5);
//#define TFT_SPI_CS_DIS()                    GPIOASET = BIT(5);
//
////RS
//#define PORT_TFT_RS                         IO_PA3

#endif // __TFT_DRV_H__
