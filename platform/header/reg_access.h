#ifndef REG_ACCESS_H_
#define REG_ACCESS_H_

/********************  Macro Control for Bit definition  **********************/
// 安全生成 n 位全 1 的掩码 (0 ≤ n ≤ 32)
#define BITS_MASK(n)        ((uint32_t)((0xFFFFFFFFULL >> (32 - (n))) & ((n) ? 0xFFFFFFFF : 0)))

#define SET_BIT(bits)                   (BIT(bits))             //构造位对应的值(单位)
#define SET_BITS(bits, value)           (value << bits)         //构造位对应的值(多位)

#define REG_EQUAL(reg, value)           (reg = value)           //寄存器等于
#define REG_SET(reg, value)             (reg |= value)          //寄存器或
#define REG_SET_ALL(reg)                (reg = 0xffffffff)      //寄存器全置位
#define REG_CLR(reg, value)             (reg &= ~value)         //寄存器非
#define REG_CLR_ALL(reg)           		(reg = 0)               //寄存器清空

///寄存器操作(不自动拼接前缀)
#define REG_EQUAL_BIT(reg, pos)                 (reg = BIT(pos))            //寄存器等于位值(单bit)
#define REG_SET_BIT(reg, pos)                   (reg |= BIT(pos))           //寄存器置位位值(单bit)
#define REG_SET_BIT_VALUE(reg, pos, value)      (reg = ((reg & ~(0x1 << pos)) | ((value) << pos)))  //寄存器置位位值(单bit)
#define REG_CLR_BIT(reg, pos)                   (reg &= ~BIT(pos))          //寄存器清除位值(单bit)
#define REG_GET_BIT(reg, pos)                   ((reg & BIT(pos)) >> pos)   //获取寄存器位值(单bit)
#define REG_GET_POS(reg, pos)                   (reg & BIT(pos))            //获取寄存器对应位是否置位(单bit)

//置位寄存器域值(多个连续bit)
#define REG_SET_BITS(reg, pos, value)               (reg = ((reg & ~(BITS_MASK(pos##_LEN) << pos)) | ((value) << pos)))
#define REG_SET_BITS_LEN(reg, pos, len, value)      (reg = ((reg & ~(BITS_MASK(len) << pos)) | ((value) << pos)))
#define REG_SET_BITS_MASK(reg, pos, mask, value)    (reg = ((reg & ~(mask << pos)) | ((value) << pos)))
//清除寄存器域值(多个连续bit)
#define REG_CLR_BITS(reg, pos)                      (reg = (reg & ~((BITS_MASK(pos##_LEN)) << pos)))
//获取寄存器域值(多个连续bit)
#define REG_GET_BITS(reg, pos)                      ((reg >> pos) & BITS_MASK(pos##_LEN))
#define REG_GET_BITS_LEN(reg, pos, len)             ((reg >> pos) & BITS_MASK(len))
#define REG_GET_BITS_MASK(reg, pos, mask)           ((reg >> pos) & mask)

///寄存器操作(自动拼接前缀)
#define SFR_EQUAL_BIT(sfr, pos)                 (sfr = BIT(sfr##_##pos))                //寄存器等于位值(单bit)
#define SFR_SET_BIT(sfr, pos)                   (sfr |= BIT(sfr##_##pos))               //寄存器置位位值(单bit)
#define SFR_SET_BIT_VALUE(sfr, pos, value)      (sfr = ((sfr & ~(0x1 << sfr##_##pos)) | ((value) << sfr##_##pos)))  //寄存器置位位值(单bit)
#define SFR_CLR_BIT(sfr, pos)                   (sfr &= ~BIT(sfr##_##pos))              //寄存器清除位值(单bit)
#define SFR_GET_BIT(sfr, pos)                   (sfr & BIT(sfr##_##pos) >> sfr##_##pos) //获取寄存器位值(单bit)
#define SFR_GET_POS(sfr, pos)                   (sfr & BIT(sfr##_##pos))                //获取寄存器对应位是否置位(单bit)

//置位寄存器域值(多个连续bit)
#define SFR_SET_BITS(sfr, pos, value)  (sfr = ((sfr & ~(BITS_MASK(sfr##_##pos##_LEN) << sfr##_##pos)) | ((value) << sfr##_##pos)))
//清除寄存器域值(多个连续bit)
#define SFR_CLR_BITS(sfr, pos)         (sfr = (sfr & ~((BITS_MASK(sfr##_##pos##_LEN)) << sfr##_##pos)))
//获取寄存器域值(多个连续bit)
#define SFR_GET_BITS(sfr, pos)         ((sfr >> sfr##_##pos) & BITS_MASK(sfr##_##pos##_LEN))

//寄存器置两位
#define SFR_SET_BIT_2(sfr, bit1, bit2)  (sfr |= BIT(sfr##_##bit1) | BIT(sfr##_##bit2))
//寄存器清两位
#define SFR_CLR_BIT_2(sfr, bit1, bit2)  (sfr &= ~(BIT(sfr##_##bit1) | BIT(sfr##_##bit2)))
//寄存器获取对应两位是否置位
#define SFR_GET_POS_2(sfr, bit1, bit2)  (sfr & (BIT(sfr##_##bit1) | BIT(sfr##_##bit2)))
//-------------------------------- SFR Group0 -----------------------------------//
/*******************  Bit definition for UARTxCON register  ********************/
#define UARTxCON_UTEN           0       //UART EN
#define UARTxCON_BIT9EN         1       //bit select; 0:8-bit; 1:9-bit;
#define UARTxCON_RXIE           2       //RX interrupt enable
#define UARTxCON_TXIE           3       //TX interrupt enable
#define UARTxCON_SB2EN          4       //stop bit sel; 0:1-bit; 1:2-bit;
#define UARTxCON_FIXBAUD        5       //指定波特率; 0:自适应; 1:指定;
#define UARTxCON_ONELINE        6       //One-line mode
#define UARTxCON_RXEN           7       //RX EN
#define UARTxCON_TXPND          8       //TX one byte finish
#define UARTxCON_RXPND          9       //RX one byte finish
#define UARTxCON_KEYMAT         10      //Key match pending

/*******************  Bit definition for UARTxBAUD register  ********************/
#define UARTxBAUD_BAUD          0       //UART Badu Rate, Baud = clock/(UARTxBAUD_BAUD + 1)
#define UARTxBAUD_DETBAUD       16      //UART detect baud by hardware, Read only

#define UARTxBAUD_BAUD_LEN      16
#define UARTxBAUD_DETBAUD_LEN   16
//-------------------------------- SFR Group1 -----------------------------------//
/*******************  Bit definition for RTCCON register  *********************/
#define RTCCON_VUSBRSTEN                6   //VUSB insert reset system enable
#define RTCCON_VUSBOFF                  20  //VUSB off stae

/*******************  Bit definition for PWRCON0 register  ********************/
#define PWRCON0_DI_VCORES               0   //VDDCORE voltage
#define PWRCON0_DI_GPIO_OE              29  //VUSB as GPIO output enable
#define PWRCON0_DI_GPIO_IE              30  //VUSB as GPIO input enable

#define PWRCON0_DI_VCORES_LEN           5


/*******************  Bit definition for PWRCON1 register  ********************/
#define PWRCON1_DI_GPIO_DIN             1   //VUSB as GPIO, data used to send to VUSB

/*******************  Bit definition for CLKCON1 register  ********************/
#define CLKCON1_HSUT0_CLK_SEL           16
#define CLKCON1_UART0_CLK_SEL           19
#define CLKCON1_UART1_CLK_SEL           21

#define CLKCON1_HSUT0_CLK_SEL_LEN       3
#define CLKCON1_UART0_CLK_SEL_LEN       2
#define CLKCON1_UART1_CLK_SEL_LEN       2

typedef enum {
    HSUT0_CLK_RC24M = 0,
    HSUT0_CLK_TMR_INC,
    HSUT0_CLK_XOSC,
    HSUT0_CLK_XOSC_X2,
    HSUT0_CLK_PLL0DIV2,
    HSUT0_CLK_PLL1DIV2,
    HSUT0_CLK_PLL0DIV4,
    HSUT0_CLK_PLL0DIV8,
}Hsut0_Clk;

typedef enum {
    UART0_CLK_RC24M = 0,
    UART0_CLK_TMR_INC,
    UART0_CLK_XOSC,
    UART0_CLK_XOSC_X2,
}Uart0_Clk;

typedef enum {
    UART1_CLK_RC24M = 0,
    UART1_CLK_TMR_INC,
    UART1_CLK_XOSC,
    UART1_CLK_XOSC_X2,
}Uart1_Clk;
/*******************  Bit definition for CLKGAT0 register  ********************/
#define CLKGAT0_TMR0_CLK_EN             0   //TMR0模块时钟使能
#define CLKGAT0_TMR1_CLK_EN             1
#define CLKGAT0_TMR2_CLK_EN             2
#define CLKGAT0_UART0_CLK_EN            6
#define CLKGAT0_UART1_CLK_EN            7
#define CLKGAT0_HSUT0_CLK_EN            9
#define CLKGAT0_IIS_CLK_EN              10
#define CLKGAT0_SPI0_CLK_EN             12
#define CLKGAT0_SPI1_CLK_EN             13
#define CLKGAT0_SPI2_CLK_EN             23

//-------------------------------- SFR Group2 -----------------------------------//
/*******************  Bit definition for FUNCMCON0 register  ********************/
#define FUNCMCON0_SD0MAP                0
#define FUNCMCON0_SPI0MAP               4
#define FUNCMCON0_UT0TXMAP              8       //UART0 TX mapping
#define FUNCMCON0_UT0RXMAP              12      //UART0 RX mapping
#define FUNCMCON0_HSUTTXMAP             16      //High speed UART TX mapping
#define FUNCMCON0_HSUTRXMAP             20      //High speed UART RX mapping
#define FUNCMCON0_UT1TXMAP              24      //UART1 TX mapping
#define FUNCMCON0_UT1RXMAP              28      //UART1 RX mapping

#define FUNCMCON0_SD0MAP_LEN            4
#define FUNCMCON0_SPI0MAP_LEN           4
#define FUNCMCON0_UT0TXMAP_LEN          4
#define FUNCMCON0_UT0RXMAP_LEN          4
#define FUNCMCON0_HSUTTXMAP_LEN         4
#define FUNCMCON0_HSUTRXMAP_LEN         4
#define FUNCMCON0_UT1TXMAP_LEN          4
#define FUNCMCON0_UT1RXMAP_LEN          4

typedef enum {
    SPI0MAP_NO_AFFECT = 0,
    SPI0MAP_G1,         //G1  CLK(PG4), DO/D0(PG5), DI/D1(PG1), D2(PG0), D3/HOLD(PG3), CS(PG2)
    SPI0MAP_G2,         //G2  CLK(PB4), DIO(PB3)
    SPI0MAP_MAX,
    SPI0MAP_CLEAR = 0xf,
}SPI0Map;

typedef enum {
    UT0TXMAP_NO_AFFECT = 0,
    UT0TXMAP_G1_PB3,
    UT0TXMAP_G2_PA0,
    UT0TXMAP_G3_PB0,
    UT0TXMAP_G4_PB12,
    UT0TXMAP_G5_PE0,
    UT0TXMAP_G6_PE7,
    UT0TXMAP_OUT_CH,
    UT0TXMAP_G8_VUSB,
    UT0TXMAP_MAX,
    UT0TXMAP_CLEAR = 0xf,
}UT0TxMap;

typedef enum {
    UT0RXMAP_NO_AFFECT = 0,
    UT0RXMAP_G1_PB4,
    UT0RXMAP_G2_PA1,
    UT0RXMAP_G3_PB1,
    UT0RXMAP_G4_PB11,
    UT0RXMAP_G5_PE1,
    UT0RXMAP_G6_PE8,
    UT0RXMAP_G7_MAP_TO_TX,
    UT0RXMAP_G8_IN_CH2,
    UT0RXMAP_G9_IN_CH3,
    UT0RXMAP_MAX,
    UT0RXMAP_CLEAR = 0xf,
}UT0RxMap;

typedef enum {
    HSUTTXMAP_NO_AFFECT = 0,
    HSUTTXMAP_G1_PB3,
    HSUTTXMAP_G2_VUSB,
    HSUTTXMAP_OUT_CH,
    HSUTTXMAP_G4_PA10,
    HSUTTXMAP_G5_PB0,
    HSUTTXMAP_G6_PB1,
    HSUTTXMAP_G7_PB8,
    HSUTTXMAP_G8_PB9,
    HSUTTXMAP_G9_PE0,
    HSUTTXMAP_G10_PE1,
    HSUTTXMAP_G11_PE4,
    HSUTTXMAP_G12_PE5,
    HSUTTXMAP_G13_PE13,
    HSUTTXMAP_MAX,
    HSUTTXMAP_CLEAR = 0xf,
}HsutTxMap;

typedef enum {
    HSUTRXMAP_NO_AFFECT = 0,
    HSUTRXMAP_G1_PB3,
    HSUTRXMAP_G2_VUSB,
    HSUTRXMAP_G3_IN_CH3,
    HSUTRXMAP_G4_PA10,
    HSUTRXMAP_G5_PB0,
    HSUTRXMAP_G6_PB1,
    HSUTRXMAP_G7_PB8,
    HSUTRXMAP_G8_PB9,
    HSUTRXMAP_G9_PE0,
    HSUTRXMAP_G10_PE1,
    HSUTRXMAP_G11_PE4,
    HSUTRXMAP_G12_PE5,
    HSUTRXMAP_G13_PE13,
    HSUTRXMAP_G14_IN_CH2,
    HSUTRXMAP_CLEAR = 0xf,
    HSUTRXMAP_MAX = HSUTRXMAP_CLEAR,
}HsutRxMap;

typedef enum {
    UT1TXMAP_NO_AFFECT = 0,
    UT1TXMAP_G1_VUSB,
    UT1TXMAP_G2_PB8,
    UT1TXMAP_G3_PA10,
    UT1TXMAP_G4_PE1,
    UT1TXMAP_G5_PE4,
    UT1TXMAP_G6_PE13,
    UT1TXMAP_OUT_CH,
    UT1TXMAP_MAX,
    UT1TXMAP_CLEAR = 0xf,
}UT1TxMap;

typedef enum {
    UT1RXMAP_NO_AFFECT = 0,
    UT1RXMAP_G1_MAP_TO_TX,
    UT1RXMAP_G2_PB9,
    UT1RXMAP_G3_PA11,
    UT1RXMAP_G4_PE2,
    UT1RXMAP_G5_PE5,
    UT1RXMAP_G6_PE14,
    UT1RXMAP_G7_IN_CH2,
    UT1RXMAP_G8_IN_CH3,
    UT1RXMAP_MAX,
    UT1RXMAP_CLEAR = 0xf,
}UT1RxMap;
/*******************  Bit definition for FUNCMCON1 register  ********************/
#define FUNCMCON1_CLKOMAP               0
#define FUNCMCON1_SPI1MAP               4
#define FUNCMCON1_TMR5PWM0MAP           8       //Timer5 PWM0 mapping

#define FUNCMCON1_SPI1MAP_LEN           4

typedef enum {
    SPI1MAP_NO_AFFECT = 0,
    SPI1MAP_G1,         //G1: CLK(PA4), DO/D0(PA3), DI/D1(PA1), D2(PA0), D3/HOLD(PA5)
    SPI1MAP_G2,         //G2: CLK(PB6), DO/D0(PB5), DI/D1(PB1), D2(PB0), D3/HOLD(PB7)
    SPI1MAP_G3,         //G3: CLK(IN Channel2), DO(IN Channel1), DI(IN Channel0)
    SPI1MAP_MAX,
    SPI1MAP_CLEAR = 0xf,
}SPI1Map;

/*******************  Bit definition for FUNCMCON2 register  ********************/
#define FUNCMCON2_IISMAP                0
#define FUNCMCON2_TMR3CPTMAP            4
#define FUNCMCON2_SPI2MAP               8
#define FUNCMCON2_IIC1MAP               12
#define FUNCMCON2_IIC2MAP               16
#define FUNCMCON2_QDECMAP               20
#define FUNCMCON2_IRMAP                 24
#define FUNCMCON2_DESPIMAP              28

#define FUNCMCON2_SPI2MAP_LEN           4
#define FUNCMCON2_DESPIMAP_LEN          4

typedef enum {
    SPI2MAP_NO_AFFECT = 0,
    SPI2MAP_G1,         //G1: CLK(PG4), DO/D0(PG5), DI/D1(PG1), D2(PG0), D3/HOLD(PG3), CS(PG2)
    SPI2MAP_G2,         //G2: CLK(PA10), DO(PA11), DI(PA9)
    SPI2MAP_G3,         //G3: CLK(PB9),  DO(PB10), DI(PB8)
    SPI2MAP_G4,         //G4: CLK(PE9),  DO(PE10), DI(PE8)
    SPI2MAP_G5,         //G5: CLK(PA6),  DO(PA7),  DI(PA5)
    SPI2MAP_G6,         //G6: CLK(IN Channel2), DO(IN Channel1), DI(IN Channel0)
    SPI2MAP_MAX,
    SPI2MAP_CLEAR = 0xf,
}SPI2Map;

typedef enum {
    DESPIMAP_NO_AFFECT = 0,
    DESPIMAP_G1,        //G1  SCL(PA4), SDA/D0(PA2), DC/D1(PA3), D2(PA1), D3(PA0)
    DESPIMAP_G2,
    DESPIMAP_G3,
    DESPIMAP_G4,
    DESPIMAP_G5,
    DESPIMAP_MAX,
    DESPIMAP_CLEAR = 0xf,
}DESPIMap;
/******************  Bit definition for FUNCINCON register  ********************/
#define FUNCINCON_CH0INSEL              0   //Chanel 0 function unput select. Please lookup "Function input select table"
#define FUNCINCON_CH1INSEL              8   //Chanel 1 function unput select.
#define FUNCINCON_CH2INSEL              16  //Chanel 2 function unput select.
#define FUNCINCON_CH3INSEL              24  //Chanel 3 function unput select.

#define FUNCINCON_CH0INSEL_LEN          6
#define FUNCINCON_CH1INSEL_LEN          6
#define FUNCINCON_CH2INSEL_LEN          6
#define FUNCINCON_CH3INSEL_LEN          6

#define IO_FUNI_SEL(io_num)             (io_num - 1)    //io_num映射值, FUNCINCON值为[PA0:PG0], 减一跳过NONE

typedef enum {
    CHxINSEL_PA0 = 0,
    CHxINSEL_PA1,
    CHxINSEL_PA2,
    CHxINSEL_PA3,
    CHxINSEL_PA4,
    CHxINSEL_PA5,
    CHxINSEL_PA6,
    CHxINSEL_PA7,
    CHxINSEL_PA8,
    CHxINSEL_PA9,
    CHxINSEL_PA10,
    CHxINSEL_PA11,
    CHxINSEL_PA12,
    CHxINSEL_PA13,
    CHxINSEL_PA14,
    CHxINSEL_PA15,
    CHxINSEL_PB0,   //16
    CHxINSEL_PB1,
    CHxINSEL_PB2,
    CHxINSEL_PB3,
    CHxINSEL_PB4,
    CHxINSEL_PB5,
    CHxINSEL_PB6,
    CHxINSEL_PB7,
    CHxINSEL_PB8,
    CHxINSEL_PB9,
    CHxINSEL_PB10,
    CHxINSEL_PB11,
    CHxINSEL_PB12,
    CHxINSEL_PE0,   //29
    CHxINSEL_PE1,
    CHxINSEL_PE2,
    CHxINSEL_PE3,
    CHxINSEL_PE4,
    CHxINSEL_PE5,
    CHxINSEL_PE6,
    CHxINSEL_PE7,
    CHxINSEL_PE8,
    CHxINSEL_PE9,
    CHxINSEL_PE10,
    CHxINSEL_PE11,
    CHxINSEL_PE12,
    CHxINSEL_PE13,
    CHxINSEL_PE14,
    CHxINSEL_PG0,   //44
    CHxINSEL_PG1,
    CHxINSEL_PG2,
    CHxINSEL_PG3,
    CHxINSEL_PG4,
    CHxINSEL_PG5,
    CHxINSEL_PG6,
    CHxINSEL_MAX,
} CHxINSEL;

/*******************  Bit definition for FUNCOUTCON register  ********************/
#define FUNCOUTCON_CH0OUTSEL            0
#define FUNCOUTCON_CH1OUTSEL            8
#define FUNCOUTCON_CH2OUTSEL            16
#define FUNCOUTCON_CH3OUTSEL            24

#define FUNCOUTCON_CH0OUTSEL_LEN        4
#define FUNCOUTCON_CH1OUTSEL_LEN        4
#define FUNCOUTCON_CH2OUTSEL_LEN        4
#define FUNCOUTCON_CH3OUTSEL_LEN        4

typedef enum {
    CHxOUTSEL_TMR5PWM0 = 0,
    CHxOUTSEL_TMR5PWM1,
    CHxOUTSEL_TMR3PWM0,
    CHxOUTSEL_TMR4PWM0,
    CHxOUTSEL_UART0_TX,
    CHxOUTSEL_HSUT_TX,
    CHxOUTSEL_UART1_TX,
    CHxOUTSEL_IIC2_SCL,
    CHxOUTSEL_IIC2_SDA,
    CHxOUTSEL_CLKOUT,
    CHxOUTSEL_SPI1_D0,
    CHxOUTSEL_SPI2_D0G5 = CHxOUTSEL_SPI1_D0,
    CHxOUTSEL_SPI1_D1,
    CHxOUTSEL_SPI2_D1_G5 = CHxOUTSEL_SPI1_D1,
    CHxOUTSEL_SPI1_CLK,
    CHxOUTSEL_SPI2_CLK_G5 = CHxOUTSEL_SPI1_CLK,
    CHxOUTSEL_IIC1_SCL,
    CHxOUTSEL_IIC1_SDA,
    CHxOUTSEL_IR_TX_OUT,
} CHxOUTSEL;
/*******************  Bit definition for FUNCOUTMCON register  ********************/
#define FUNCOUTMCON_CH0OUTMAP           0
#define FUNCOUTMCON_CH1OUTMAP           8
#define FUNCOUTMCON_CH2OUTMAP           16
#define FUNCOUTMCON_CH3OUTMAP           24

#define FUNCOUTMCON_CH0OUTMAP_LEN       5
#define FUNCOUTMCON_CH1OUTMAP_LEN       5
#define FUNCOUTMCON_CH2OUTMAP_LEN       5
#define FUNCOUTMCON_CH3OUTMAP_LEN       5

typedef enum {
    CHxOUTMAP_NONE = 0,
    CHxOUTMAP_PA0,
    CHxOUTMAP_PA1,
    CHxOUTMAP_PA2,
    CHxOUTMAP_PA3,
    CHxOUTMAP_PA4,
    CHxOUTMAP_PA5,
    CHxOUTMAP_PA6,
    CHxOUTMAP_PA7,
    CHxOUTMAP_PA8,
    CHxOUTMAP_PA9,
    CHxOUTMAP_PA10,
    CHxOUTMAP_PA11,
    CHxOUTMAP_PA12,
    CHxOUTMAP_PA13,
    CHxOUTMAP_PA14,
    CHxOUTMAP_PA15,
    CHxOUTMAP_PB0,  //17
    CHxOUTMAP_PB1,
    CHxOUTMAP_PB2,
    CHxOUTMAP_PB3,
    CHxOUTMAP_PB4,
    CHxOUTMAP_PB5,
    CHxOUTMAP_PB6,
    CHxOUTMAP_PB7,
    CHxOUTMAP_PB8,
    CHxOUTMAP_PB9,
    CHxOUTMAP_PB10,
    CHxOUTMAP_PB11,
    CHxOUTMAP_PB12,
    CHxOUTMAP_PE0,  //30
    CHxOUTMAP_PE1,
    CHxOUTMAP_PE2,
    CHxOUTMAP_PE3,
    CHxOUTMAP_PE4,
    CHxOUTMAP_PE5,
    CHxOUTMAP_PE6,
    CHxOUTMAP_PE7,
    CHxOUTMAP_PE8,
    CHxOUTMAP_PE9,
    CHxOUTMAP_PE10,
    CHxOUTMAP_PE11,
    CHxOUTMAP_PE12,
    CHxOUTMAP_PE13,
    CHxOUTMAP_PE14,
    CHxOUTMAP_MAX,
} CHxOUTMAP;
//-------------------------------- SFR Group3 -----------------------------------//
/***************  Bit definition for SPIxCON/LCDSPICON register  ****************/
#define SPIxCON_SPIEN           0       //spi模块总使能位
#define SPIxCON_SPISM           1       //从机模式使能
#define SPIxCON_BUSMODE         2       //总线位宽选择
#define SPIxCON_RXSEL           4       //dma模式或2线模式下, 传输选择位, 0:发送; 1:接收;
#define SPIxCON_CLKIDS          5       //时钟空闲电平, 0:低电平; 1:高电平;
#define SPIxCON_SMPS            6       //输出数据有效边沿选择, 0:下降沿; 1:上升沿;
#define SPIxCON_SPIIE           7       //中断使能位
#define SPIxCON_SPILF_EN        8       //SPI LFSR enable bit(no support dtr mode)
#define SPIxCON_SPIMBEN         9       //multiple bit bus使能位, 启用多条发送数据线(2条及以上)
#define SPIxCON_SPIOSS          10      //数据采集和发送是否在相同的时钟边沿; 0: 不同; 1:相同;
#define SPIxCON_HOLDENRX        11      //蓝牙rx时是否拉住hold脚
#define SPIxCON_HOLDENTX        12      //蓝牙tx时是否拉住hold脚
#define SPIxCON_HOLDENSW        13      //hold脚使用软件控制
#define SPIxCON_DTR_EN          14      //DTR_EN MODE(only spi0)
#define SPIxCON_DTR_CHANGE_SEL  15      //Dtr sample chose(only spi0)
#define SPIxCON_SPIPND          16      //rx/tx完成挂起位, 写1清pending;
#define LCDSPICON_SP_8W_EN      17
#define LCDSPICON_3W_CTR        18
#define LCDSPICON_3W_BIT9       19
#define LCDSPICON_DCX_REUSE     20
#define LCDSPICON_CS            21
#define LCDSPICON_CS_EN         22
#define LCDSPICON_1P1T_EN       26
#define LCDSPICON_DDR_EN        27

#define SPIxCON_BUSMODE_LEN          2
#define LCDSPICON_SP_8W_EN_LEN       1
#define LCDSPICON_3W_CTR_LEN         1
#define LCDSPICON_3W_BIT9_LEN        1
#define LCDSPICON_DCX_REUSE_LEN      1
#define LCDSPICON_CS_EN_LEN          1
#define LCDSPICON_1P1T_EN_LEN        1
#define LCDSPICON_DDR_EN_LEN         1

typedef enum {
    BUSMODE_1BIT_IN_1BIT_OUT = 0,   //发送数据总线为1条, 每个clk发1BIT, 1条信号线发(SDA_D0), 另1条信号线收(DCX_D1)
    BUSMODE_1BIT_IN_OUT,            //发送数据总线为1条, 每个clk发1BIT, 1条信号线上收发(SDA_D0)
    BUSMODE_2BIT_DATA_BUS,          //发送数据总线为2条, 每个clk发2BIT, 发送数据使用(SDA_D0 DCX_D1), 无法接收数据
    BUSMODE_4BIT_DATA_BUS,          //发送数据总线为4条, 每个clk发4BIT, 发送数据使用(SDA_D0 DCX_D1 D2 D3), 无法接收数据
} Spi_BusMode;
/***************  Bit definition for SPIxCPND register  ****************/
#define SPIxCPND_SPICPND                16  //write 1 will clear SPI pending

//-------------------------------- SFR Group4 -----------------------------------//
//-------------------------------- SFR Group5 -----------------------------------//
//-------------------------------- SFR Group6 -----------------------------------//
//-------------------------------- SFR Group7 -----------------------------------//
//-------------------------------- SFR Group8 -----------------------------------//
//-------------------------------- SFR Group9 -----------------------------------//

/*******************  Bit definition for HSUT0CON register  ********************/
#define HSUT0CON_URXEN          0       //UART RX Enable Bit
#define HSUT0CON_UTXEN          1       //UART TX Enable Bit
#define HSUT0CON_RXIE           2       //Rx interrupt enable
#define HSUT0CON_TXIE           3       //Tx interrupt enable
#define HSUT0CON_RXTRSMODE      4       //Rx传输模式选择; 0: buf mode; 1:DMA mode;
#define HSUT0CON_RXBITSEL       5       //RX data bit select; 0:8-bit; 1:9-bit;
#define HSUT0CON_RXLPBUFEN      6       //RX dma loop buffer mode enable
#define HSUT0CON_TXTRSMODE      7       //Tx传输模式选择; 0: buf mode; 1:DMA mode;
#define HSUT0CON_TXBITSEL       8       //TX data bit select; 0:8-bit; 1:9-bit;
#define HSUT0CON_SPBITSEL       9       //TX stop bit select; 0: 1BIT, 1:2BIT;
#define HSUT0CON_TMREN          10      //DMA RX timer cnt enable
#define HSUT0CON_RXHF_PND       11      //RX DMA half_full pending(需打开循环buf)
#define HSUT0CON_RXPND          12      //RX one byte/MDA n byte finish pending
#define HSUT0CON_TXPND          13      //TX one byte/MDA n byte finish pending
#define HSUT0CON_RXFAIL         14      //RX DMA fail flag
#define HSUT0CON_TMROV          15      //RX timer overfilow flag
#define HSUT0CON_RXOVEPND       16      //RX DMA buffer overflow err pending
#define HSUT0CON_RXHFPND_IE     17      //RX DMA half_full pending interrupt enable
#define HSUT0CON_ONELINE        18      //One-line mode

/*******************  Bit definition for HSUT0CPND register  ********************/
//write 0 no affect
#define HSUT0CPND_CUTRX         0       //HS RX interface clear, 1: clear to idle status at hsuartclk domain
#define HSUT0CPND_CUTTX         1       //HS TX interface clear
#define HSUT0CPND_CCTSPND       10      //CTS status changing pending clear
#define HSUT0CPND_CRXHFPND      11      //RX DMA half pending clear
#define HSUT0CPND_CRXPND        12      //RX pending clear
#define HSUT0CPND_CTXPND        13      //TX pending clear
#define HSUT0CPND_CRXFAIL       14      //RX fail clear
#define HSUT0CPND_CTMROV        15      //RX timer overflow flag clear
#define HSUT0CPND_CRXOVEPND     16      //RX overfile error pending clear
#define HSUT0CPND_CRXLBBBUF     17      //RX loopback buffer clear

/*******************  Bit definition for HSUT0BAUD register  ********************/
#define HSUT0BAUD_HSUTTXBAUD    0       //HSUART TX Baud Rate, Baud Rate = source clock/(HSUTRXBAUD + 1)
#define HSUT0BAUD_HSUTRXBAUD    16      //HSUART RX Baud Rate

/*******************  Bit definition for HSUT0FIFOCNT register  ********************/
#define HSUT0FIFOCNT_RXFIFOCNT      0   //HSUART RX data counter left
#define HSUT0FIFOCNT_RXFIFOCNT_LEN  16

//-------------------------------- SFR Group10 ----------------------------------//
#endif  //REG_ACCESS_H_
