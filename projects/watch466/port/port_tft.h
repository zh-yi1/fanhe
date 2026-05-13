#ifndef _PORT_TFT_H
#define _PORT_TFT_H

#define PORT_TFT_INT            IO_PA6
#define PORT_TFT_INT_VECTOR     PORT_INT2_VECTOR

//RST
#define PORT_TFT_RST            IO_PA7
#define PORT_TFT_RST_H()        GPIOASET = BIT(7);//GPIOBSET = BIT(7);
#define PORT_TFT_RST_L()        GPIOACLR = BIT(7);//GPIOBCLR = BIT(7);

//CS
#define TFT_SPI_CS_EN()         GPIOACLR = BIT(5);
#define TFT_SPI_CS_DIS()        GPIOASET = BIT(5);

#define PORT_TFT_BL             GPIO_NONE
#define LCD_BL_EN()             //{GPIOADIR &= ~BIT(14); GPIOADE  |= BIT(14); GPIOASET  = BIT(14);}
#define LCD_BL_DIS()            //{GPIOADIR |= BIT(14); GPIOADE  &= ~BIT(14);}

//RS
#define TFT_SPI_DATA_EN()       GPIOASET = BIT(3);
#define TFT_SPI_CMD_EN()        GPIOACLR = BIT(3);

#if (GUI_MODE_SELECT == MODE_4WIRE_8BIT)
#define   DC_ENABLE()           {GPIOAFEN &= ~BIT(3); GPIOADE |=  BIT(3);GPIOASET = BIT(3); GPIOADIR &= ~BIT(3);} // DC脚 设置输出
#define   DC_CMD_EN()           TFT_SPI_CMD_EN();           // DC 拉低
#define   DC_DATA_EN()          TFT_SPI_DATA_EN();          // DC 拉高
#elif (GUI_MODE_SELECT == MODE_3WIRE_9BIT_2LINE)
#define   DC_ENABLE()           {GPIOAFEN |= BIT(3); GPIOADE |=  BIT(3); GPIOADIR |= BIT(3);} //DC脚 设置输入
#define   DC_CMD_EN()           {DESPICON &= ~BIT(19);}     // DC 拉低
#define   DC_DATA_EN()          {DESPICON |= BIT(19);}      // DC 拉高
#endif

void port_tft_init(void);
void port_tft_exit(void);
void port_tft_reset(void);

#endif
