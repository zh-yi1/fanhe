/*****************************************************************************
 * Module    : Configs
 * File      : config_define.h
 * Function  : 定义用户参数常量
 *****************************************************************************/
#ifndef CONFIG_DEFINE_H
#define CONFIG_DEFINE_H
#include "config_define.h"

/*****************************************************************************
 * Module    : 系统定义
 *****************************************************************************/
#define XOSC_CLK_HZ                     24000000

/*****************************************************************************
 * Module    : 芯片选型
 *****************************************************************************/
#define CHIP_5790T                      0           //QFN68

/*****************************************************************************
 * Module    : 显示相关配置选择列表
 *****************************************************************************/
//显示驱动屏选择
#define DISPLAY_NO                      0                                       //无显示模块
#define DISPLAY_LCD                     0x100                                   //选用LCD点阵屏做为显示驱动
#define DISPLAY_LCDSEG                  0x200                                   //选用断码屏做为显示驱动
#define DISPLAY_LEDSEG                  0x400                                   //选用数码管做为显示驱动
#define DISPLAY_TFT                     0x800                                   //选用彩屏做为显示驱动

#define GUI_NO                          DISPLAY_NO                              //无主题，无显示

//TFT 屏幕
#define GUI_TFT_320_ST77916             (DISPLAY_TFT | 0x01)                    //彩屏 320 * 385
#define GUI_TFT_RGBW_320_ST77916        (DISPLAY_TFT | 0x02)                    //彩屏 320 * 385
#define GUI_TFT_360_GC9C01              (DISPLAY_TFT | 0x03)                    //彩屏 360 * 360
#define GUI_OLED_466_ICNA3310B          (DISPLAY_TFT | 0x04)                    //OLED彩屏 466 * 466
#define GUI_TFT_JD9853                  (DISPLAY_TFT | 0x05)                    //JD9853  240*296 DSPI 屏
#define GUI_TFT_170_560_AXS15231B       (DISPLAY_TFT | 0x06)                    //彩屏 170_560
#define GUI_TFT_320_ST77903             (DISPLAY_TFT | 0x07)                    //彩屏 320 * 386
#define GUI_OLED_368_ST7801N            (DISPLAY_TFT | 0x08)                    //彩屏 320 * 386
#define GUI_TFT_240_ST789_i80           (DISPLAY_TFT | 0x09)                    //彩屏 240*320 I8080

#define GUI_VGS_640                     (DISPLAY_TFT | 0x10)                    //光机640*480

//LCD 屏幕
#define GUI_LCD_800_ST7265              (DISPLAY_LCD | 0x01)                    //lcd 800*480
#define GUI_LCD_480_ST7283              (DISPLAY_LCD | 0x02)                    //lcd 480*272

#define CTP_NO                          0
#define CTP_CST8X                       1
#define CTP_CHSC6X                      2
#define CTP_AXS5106                     3
#define CTP_AXS152x                     4

/*****************************************************************************
 * Module    : FLASH大小定义
 *****************************************************************************/
#define FSIZE_16M                       0x1000000
#define FSIZE_8M                        0x800000
#define FSIZE_4M                        0x400000
#define FSIZE_2M                        0x200000
#define FSIZE_1M                        0x100000
#define FSIZE_512K                      0x80000


/*****************************************************************************
 * Module    : 提示音语言配置选择列表
 *****************************************************************************/
#define LANG_EN         0               //英文提示音
#define LANG_ZH         1               //中文提示音
#define LANG_EN_ZH      2               //英文、中文双语提示音

/*****************************************************************************
 * Module    : ADC通路选择列表
 *****************************************************************************/
#define ADCCH_PA8       0               //SARADC channel 0
#define ADCCH_PA9       1               //SARADC channel 1
#define ADCCH_PA13      2               //SARADC channel 2
#define ADCCH_PB0       3               //SARADC channel 3
#define ADCCH_PB3       4               //SARADC channel 4
#define ADCCH_PB4       5               //SARADC channel 5
#define ADCCH_PB7       6               //SARADC channel 6
#define ADCCH_PE4       7               //SARADC channel 7
#define ADCCH_PE7       8               //SARADC channel 8
#define ADCCH_PE8       9               //SARADC channel 9
#define ADCCH_PE9       10              //SARADC channel 10
#define ADCCH_PE10      11              //SARADC channel 11
#define ADCCH_PG6       12              //SARADC channel 12
#define ADCCH_WKO       13              //SARADC channel 13
#define ADCCH_VBAT      14              //SARADC channel 14
#define ADCCH_VRTC      15              //SARADC channel 15
#define ADCCH_TSENSOR   15              //SARADC channel 15
#define ADCCH_BGOP      15              //SARADC channel 15
#define ADCCH_VUSB      15              //SARADC channel 15
#define ADCCH_VTS       15              //SARADC channel 15
#define ADCCH_MIC       15              //SARADC channel 15

/*****************************************************************************
 * Module    : Timer3 Capture Mapping选择列表
 *          FUNCMCON2[7:4], 例如FUNCMCON2 = TMR3MAP_PA5;
 *****************************************************************************/
#define TMR3MAP_PA5     (1 << 4)        //G1  capture mapping: PA5
#define TMR3MAP_PA6     (2 << 4)        //G2  capture mapping: PA6
#define TMR3MAP_PB0     (3 << 4)        //G3  capture mapping: PB0
#define TMR3MAP_PB1     (4 << 4)        //G4  capture mapping: PB1
#define TMR3MAP_PE0     (5 << 4)        //G5  capture mapping: PE0
#define TMR3MAP_PE3     (6 << 4)        //G6  capture mapping: PE3
#define TMR3MAP_PE4     (7 << 4)        //G7  capture mapping: PE4
#define TMR3MAP_PE5     (8 << 4)        //G8  capture mapping: PE5
#define TMR3MAP_PF2     (9 << 4)        //G9  capture mapping: PF2

/*****************************************************************************
 * Module    : IRRX Mapping选择列表
 *          FUNCMCON2[23:20], 例如FUNCMCON2 = IRMAP_PA5;
 *****************************************************************************/
#define IRMAP_PA5       (1 << 20)       //G1  capture mapping: PA5
#define IRMAP_PA6       (2 << 20)       //G2  capture mapping: PA6
#define IRMAP_PB0       (3 << 20)       //G3  capture mapping: PB0
#define IRMAP_PB1       (4 << 20)       //G4  capture mapping: PB1
#define IRMAP_PE0       (5 << 20)       //G5  capture mapping: PE0
#define IRMAP_PE3       (6 << 20)       //G6  capture mapping: PE3
#define IRMAP_PE4       (7 << 20)       //G7  capture mapping: PE4
#define IRMAP_PE5       (8 << 20)       //G8  capture mapping: PE5
#define IRMAP_PF2       (9 << 20)       //G9  capture mapping: PF2


/*****************************************************************************
 * Module    : Timer5 PWM Mapping选择列表
 * PWM0 = FUNCMCON1[11:8],  PWM1 = FUNCMCON1[15:12], PWM2 = FUNCMCON1[19:16]
 * PWM3 = FUNCMCON1[23:20], PWM4 = FUNCMCON1[27:24], PWM5 = FUNCMCON1[31:28]
 * 示例：FUNCMCON1 = PWM0MAP_PA0;
 *****************************************************************************/
#define PWM0MAP_PA0    (1 << 8)        //G1 PWM0 mapping: PA0
#define PWM1MAP_PA1    (1 << 12)       //G1 PWM1 mapping: PA1
#define PWM2MAP_PA2    (1 << 16)       //G1 PWM2 mapping: PA2
#define PWM3MAP_PA3    (1 << 20)       //G1 PWM3 mapping: PA3
#define PWM4MAP_PA4    (1 << 24)       //G1 PWM3 mapping: PA4
#define PWM5MAP_PA5    (1 << 28)       //G1 PWM3 mapping: PA5

#define PWM0MAP_PA6    (2 << 8)        //G2 PWM0 mapping: PA6
#define PWM1MAP_PA7    (2 << 12)       //G2 PWM1 mapping: PA7
#define PWM2MAP_PB0    (2 << 16)       //G2 PWM2 mapping: PB0
#define PWM3MAP_PB1    (2 << 20)       //G2 PWM3 mapping: PB1
#define PWM4MAP_PB2    (2 << 24)       //G2 PWM3 mapping: PB2
#define PWM5MAP_PB3    (2 << 28)       //G2 PWM3 mapping: PB3

#define PWM0MAP_PB4    (3 << 8)        //G3 PWM0 mapping: PB4
#define PWM1MAP_PB5    (3 << 12)       //G3 PWM1 mapping: PB5
#define PWM2MAP_PB6    (3 << 16)       //G3 PWM2 mapping: PB6
#define PWM3MAP_PB7    (3 << 20)       //G3 PWM3 mapping: PB7
#define PWM4MAP_PE6    (3 << 24)       //G3 PWM3 mapping: PE6
#define PWM5MAP_PE7    (3 << 28)       //G3 PWM3 mapping: PE7

#define PWM0MAP_PE0    (4 << 8)        //G4 PWM0 mapping: PE0
#define PWM1MAP_PE1    (4 << 12)       //G4 PWM1 mapping: PE1
#define PWM2MAP_PE2    (4 << 16)       //G4 PWM2 mapping: PE2
#define PWM3MAP_PE3    (4 << 20)       //G4 PWM3 mapping: PE3
#define PWM4MAP_PE4    (4 << 24)       //G4 PWM3 mapping: PE4
#define PWM5MAP_PE5    (4 << 28)       //G4 PWM3 mapping: PE5

#define PWM0MAP_PF0    (5 << 8)        //G5 PWM0 mapping: PF0
#define PWM1MAP_PF1    (5 << 12)       //G5 PWM1 mapping: PF1
#define PWM2MAP_PF2    (5 << 16)       //G5 PWM2 mapping: PF2
#define PWM3MAP_PF3    (5 << 20)       //G5 PWM3 mapping: PF3
#define PWM4MAP_PF4    (5 << 24)       //G5 PWM3 mapping: PF4
#define PWM5MAP_PF5    (5 << 28)       //G5 PWM3 mapping: PF5


/*****************************************************************************
 * Module    : Clock output Mapping选择列表
 *          FUNCMCON1[3:0], 例如FUNCMCON1 = CLKOMAP_PB4;
 *****************************************************************************/
#define CLKOMAP_PA4     (1 << 0)        //G1 Clock output mapping: PA4
#define CLKOMAP_PA13    (2 << 0)        //G2 Clock output mapping: PA13
#define CLKOMAP_PB4     (3 << 0)        //G3 Clock output mapping: PB4
#define CLKOMAP_PB9     (4 << 0)        //G4 Clock output mapping: PB9
#define CLKOMAP_PE1     (5 << 0)        //G5 Clock output mapping: PE1
#define CLKOMAP_PE5     (6 << 0)        //G6 Clock output mapping: PE5
#define CLKOMAP_PE6     (7 << 0)        //G7 Clock output mapping: PE6


/*****************************************************************************
 * Module    : sd0 Mapping选择列表
 *          FUNCMCON0[3:0], 例如FUNCMCON0 = SD0MAP_G2;
 *****************************************************************************/
#define SD0MAP_NONE     (0 << 0)       //NONE
#define SD0MAP_G1       (1 << 0)       //G1  SDCLK(PG0), SDCMD(PG5), SDDAT0(PG4), SDDAT1(PG3), SDDAT2(PG2), SDDAT3(PG1)
#define SD0MAP_G2       (2 << 0)       //G2  SDCLK(PB0), SDCMD(PB5), SDDAT0(PB6), SDDAT1(PB7), SDDAT2(PB2), SDDAT3(PB1)
#define SD0MAP_G3       (3 << 0)       //G3  SDCLK(PA9), SDCMD(PA8), SDDAT0(PA10), SDDAT1(PA11), SDDAT2(PA6), SDDAT3(PA7)

/*****************************************************************************
 * Module    : sd1 Mapping选择列表
 *          FUNCMCON3[3:0], 例如FUNCMCON3 = SD1MAP_G3;
 *****************************************************************************/
#define SD1MAP_NONE     (0 << 0)       //NONE
#define SD1MAP_G1       (1 << 0)       //G1  SDCLK(PA0), SDCMD(PA3), SDDAT0(PA4), SDDAT1(PA5), SDDAT2(PA2), SDDAT3(PA1)
#define SD1MAP_G2       (2 << 0)       //G2  SDCLK(PB0), SDCMD(PB5), SDDAT0(PB6), SDDAT1(PB7), SDDAT2(PB2), SDDAT3(PB1)
#define SD1MAP_G3       (3 << 0)       //G3  SDCLK(PE4), SDCMD(PE3), SDDAT0(PE5), SDDAT1(PE6), SDDAT2(PE1), SDDAT3(PE2)

/*****************************************************************************
 * Module    : iic Mapping选择列表
 *****************************************************************************/
#define IICMAP_G1     (1 << 0)         //G1 iic0(SCL-PE8/SDA-PE7),  iic1(SCL-PA6/SDA-PA7), iic2(SCL-PA1/SDA-PA0)
#define IICMAP_G2     (2 << 0)         //G2 iic0(SCL-PE10/SDA-PE9), iic1(SCL-PA8/SDA-PA9), iic2(SCL-PB6/SDA-PB5)
#define IICMAP_G3     (3 << 0)         //G3 iic0(SCL-PE12/SDA-PE11),iic1(SCL-PB9/SDA-PB8), iic2(SCL-PE3/SDA-PE2)
#define IICMAP_G4     (4 << 0)         //G4 iic0(SCL-PE4/SDA-PE3),  iic1(SCL-PE6/SDA-PE5), iic2(SCL-PE14/SDA-PE13)

/*****************************************************************************
 * Module    : uart0 Mapping选择列表
 *****************************************************************************/
#define UTX0MAP_PB3     (1 << 8)        //G1 uart0 tx: PB3
#define UTX0MAP_PA0     (2 << 8)        //G2 uart0 tx: PA0
#define UTX0MAP_PB0     (3 << 8)        //G3 uart0 tx: PB0
#define UTX0MAP_PB12    (4 << 8)        //G4 uart0 tx: PB12
#define UTX0MAP_PE0     (5 << 8)        //G5 uart0 tx: PE0
#define UTX0MAP_PE7     (6 << 8)        //G6 uart0 tx: PE7
#define UTX0MAP_VUSB    (8 << 8)        //G8 uart0 tx: VUSB

#define URX0MAP_PB4     (1 << 12)       //G1 uart0 rx: PB4
#define URX0MAP_PA1     (2 << 12)       //G2 uart0 rx: PA1
#define URX0MAP_PB1     (3 << 12)       //G3 uart0 rx: PB1
#define URX0MAP_PB11    (4 << 12)       //G4 uart0 rx: PB11
#define URX0MAP_PE1     (5 << 12)       //G5 uart0 rx: PE1
#define URX0MAP_PE8     (6 << 12)       //G6 uart0 rx: PE8
#define URX0MAP_VUSB    (8 << 12)       //G6 uart0 rx: VUSB
#define URX0MAP_TX      (7 << 12)       //uart0 map to TX pin by UT0TXMAP select(1线模式)


/*****************************************************************************
 * Module    : Quadrate Decode Mapping选择列表
 *          FUNCMCON2[23:20], 例如FUNCMCON2 = QDEC_MAP_G1;
 *****************************************************************************/
#define QDEC_MAP_G1     (1 << 20)       //G1 QDEC_A: PA0, QDEC_B: PA1
#define QDEC_MAP_G2     (2 << 20)       //G2 QDEC_A: PB3, QDEC_B: PB4
#define QDEC_MAP_G3     (3 << 20)       //G3 QDEC_A: PE9, QDEC_B: PE10
#define QDEC_MAP_G4     (4 << 20)       //G4 QDEC_A: PE13, QDEC_B: PE14
#define QDEC_MAP_G5     (5 << 20)       //G5 Crossbus
#define QDEC_MAP_G6     (6 << 20)       //G5 QDEC_A: PE2, QDEC_B: PE3
#define QDEC_MAP_G7     (7 << 20)       //G5 QDEC_A: PB0, QDEC_B: PB1

/*****************************************************************************
 * Module    : 录音文件类型列表
 *****************************************************************************/
#define REC_NO          0
#define REC_WAV         1              //PCM WAV
#define REC_ADPCM       2              //ADPCM WAV
#define REC_MP3         3
#define REC_SBC         4
#define REC_OPUS        5

/*****************************************************************************
* Module    : DAC SELECT
*****************************************************************************/
#define DAC_DUAL        0               //DAC双声道输出
#define DAC_MONO        1               //DAC单声道输出

/*****************************************************************************
* Module    : DAC OUT Sample Rate
*****************************************************************************/
#define DAC_OUT_44K1    0               //dac out sample rate 44.1K
#define DAC_OUT_48K     1               //dac out sample rate 48K
#define DAC_OUT_88K2    2               //dac out sample rate 88.2K
#define DAC_OUT_96K     3               //dac out sample rate 96K

/*****************************************************************************
* Module    : DAC LDOH Select
*****************************************************************************/
#define AU_LDOH_2V4     0               //VDDAUD LDO voltage 2.4V
#define AU_LDOH_2V5     1               //VDDAUD LDO voltage 2.5V
#define AU_LDOH_2V7     2               //VDDAUD LDO voltage 2.7V
#define AU_LDOH_2V9     3               //VDDAUD LDO voltage 2.9V
#define AU_LDOH_3V1     4               //VDDAUD LDO voltage 3.1V
#define AU_LDOH_3V2     5               //VDDAUD LDO voltage 3.2V

/*****************************************************************************
* Module    : AUX or MIC Left&Right channel list
* AUX: 可以任意组合左右声道
*****************************************************************************/
#define MIC0                0x01        //MIC0
#define MIC1                0x02        //MIC1

#define AUX0                0x06        //PA14
#define AUX1                0x07        //PA15
#define AUX2                0x08        //PB0
#define AUX3                0x09        //PB1

#define ADC0                0x00        //ADC0
#define ADC1                0x01        //ADC1

//ADC0 channel config
#define CH_MIC0             (ADC0 << 4 | MIC0)      //MIC0          -> ADC0

//ADC1 channel config
#define CH_MIC1             (ADC1 << 4 | MIC1)      //MIC1          -> ADC1

//AUX - ADC SEL
#define CH_AUXL0            (ADC0 << 4 | AUX0)      //AUX0          -> ADC0
#define CH_AUXL1            (ADC0 << 4 | AUX1)      //AUX1          -> ADC0
#define CH_AUXL2            (ADC0 << 4 | AUX2)      //AUX2          -> ADC0
#define CH_AUXL3            (ADC0 << 4 | AUX3)      //AUX3          -> ADC0
#define CH_AUXR0            (ADC1 << 4 | AUX0)      //AUX0          -> ADC1
#define CH_AUXR1            (ADC1 << 4 | AUX1)      //AUX1          -> ADC1
#define CH_AUXR2            (ADC1 << 4 | AUX2)      //AUX2          -> ADC1
#define CH_AUXR3            (ADC1 << 4 | AUX3)      //AUX3          -> ADC1


/*****************************************************************************
* Module    : 电池低电电压列表
*****************************************************************************/
#define VBAT_2V8            0       //2.8v
#define VBAT_2V9            1       //2.9v
#define VBAT_3V0            2       //3.0v
#define VBAT_3V1            3       //3.1v
#define VBAT_3V2            4       //3.2v
#define VBAT_3V3            5       //3.3v
#define VBAT_3V4            6       //3.4v
#define VBAT_3V5            7       //3.5v
#define VBAT_3V6            8       //3.6v
#define VBAT_3V7            9       //3.7v
#define VBAT_3V8            10      //3.8v

/*****************************************************************************
* Module    : uart0 printf IO列表
*****************************************************************************/
#define PRINTF_NONE         0           //关闭UART0打印信息
#define PRINTF_PB3          1
#define PRINTF_PA0          2
#define PRINTF_PB0          3
#define PRINTF_PB12         4
#define PRINTF_PE0          5
#define PRINTF_PE7          6
#define PRINTF_VUSB         8

/*****************************************************************************
* Module    : GPIO list
*****************************************************************************/
#define IO_NONE             0
#define IO_PA0              1
#define IO_PA1              2
#define IO_PA2              3
#define IO_PA3              4
#define IO_PA4              5
#define IO_PA5              6
#define IO_PA6              7
#define IO_PA7              8
#define IO_PA8              9
#define IO_PA9              10
#define IO_PA10             11
#define IO_PA11             12
#define IO_PA12             13
#define IO_PA13             14
#define IO_PA14             15
#define IO_PA15             16
#define IO_PB0              17
#define IO_PB1              18
#define IO_PB2              19
#define IO_PB3              20
#define IO_PB4              21
#define IO_PB5              22
#define IO_PB6              23
#define IO_PB7              24
#define IO_PB8              25
#define IO_PB9              26
#define IO_PB10             27
#define IO_PB11             28
#define IO_PB12             29
#define IO_PE0              30
#define IO_PE1              31
#define IO_PE2              32
#define IO_PE3              33
#define IO_PE4              34
#define IO_PE5              35
#define IO_PE6              36
#define IO_PE7              37
#define IO_PE8              38
#define IO_PE9              39
#define IO_PE10             40
#define IO_PE11             41
#define IO_PE12             42
#define IO_PE13             43
#define IO_PE14             44
#define IO_PF0              45
#define IO_PF1              46
#define IO_PF2              47
#define IO_PF3              48
#define IO_PF4              49
#define IO_PF5              50
#define IO_PF6              51
#define IO_PF7              52
#define IO_PF8              53
#define IO_PF9              54
#define IO_PF10             55
#define IO_PF11             56
#define IO_PF12             57
#define IO_PG0              58
#define IO_PG1              59
#define IO_PG2              60
#define IO_PG3              61
#define IO_PG4              62
#define IO_PG5              63
#define IO_PG6              64

#define IO_MAX_NUM          IO_PG6

#define IO_WKO              65
#define IO_MUX_SDCLK        66
#define IO_MUX_SDCMD        67
#define IO_VUSB             68

#define IO_EDGE_FALL        0xfe
#define IO_EDGE_RISE        0xff



/*****************************************************************************
* Module    : APP protocol
*****************************************************************************/
#define APP_NULL            0
#define APP_BLUE_FIT        1
#define APP_AB_LINK         2

#endif //CONFIG_DEFINE_H
