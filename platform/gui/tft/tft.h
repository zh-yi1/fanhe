#ifndef _TFT_H
#define _TFT_H

#include "tft_drv.h"

typedef struct {
    //TE控制相关
    bool tft_bglight_kick;      //背光控制
    u8   tft_bglight_duty;      //背光pwm占空比
    u8   tft_bglight_last_duty; //背光pwm上一次占空比
    u8 te_mode;
    u8 te_mode_next;
    bool tft_bglight_first_set;

    u8 te_bglight_cnt;          //在收到需要打开背光控制时，推完第一帧数据后延时打开背光
    u8 despi_baud;
    u8 despi_baud1;
    u8 despi_baud2;
    bool flag_in_frame;
    bool tft_set_baud_kick;     //需要切换时钟，等TFT_END后再切
} tft_cb_t;


typedef enum {
    LCD_IO_3WIRE_9BIT,
    LCD_IO_3WIRE_9BIT_2LINE,
    LCD_IO_4WIRE_8BIT,
    LCD_IO_QSPI,
    LCD_IO_I8080,
    LCD_IO_SRGB8,
    LCD_IO_PRGB16,
}lcd_io_type;

#define GUI_COLOR_DEPTH                 2                     //gui 底层颜色支持深度, 中科这的GUI颜色就是565色, 不可以改喔

enum {
    LCD_SELECT_XOSC_CLK     = 0,
    LCD_SELECT_XOSC_X2CLK,
    LCD_SELECT_PLL0_DIV2,
    LCD_SELECT_PLL0_DIV3,
    LCD_SELECT_PLL1_DIV1,
    LCD_SELECT_PLL1_DIV2,
};

enum {
    LCD_SELECT_RGB_8BIT = 0,            //SRGB时序: Vsync为帧同步信号, 表示帧开始. Hsync 为行同步信号, 表示行开始. DE 为数据有效使能, 表示数据有效, 每一个CLK周期, 采样一个像素数据D[7:0], 每3个CLK为一个像素RGB, 以此完成一帧图像的输出显示.
    LCD_SELECT_RGB_16BIT,               //PRGB时序: Vsync为帧同步信号, 表示帧开始. Hsync 为行同步信号, 表示行开始. DE 为数据有效使能, 表示数据有效, 每一个CLK周期, 采样一个像素数据D[23:0], 以此完成一帧图像的输出显示
};

enum {
    LCD_IN_RGB565,
    LCD_IN_RGB888,
    LCD_IN_ARGB8565,
    LCD_IN_ARGB8888,
};

enum {
    LCD_OUT_RGB565,
    LCD_OUT_BGR565,
    LCD_OUT_RGB888,
    LCD_OUT_BGR888,
};

#if (GUI_COLOR_DEPTH == 2)
typedef union {
    struct {
        u16 b : 5;
        u16 g : 6;
        u16 r : 5;
    } PACKED ch;
    u16 full;
} PACKED lcd_color_t;
#elif (GUI_COLOR_DEPTH == 3)
typedef union rgb888_t_ {
    struct {
        u8 b;
        u8 g;
        u8 r;
    } PACKED ch;
    u32 full : 24;
} PACKED lcd_color_t;
#else
#error GUI_COLOR_DEPTH undefine !!!
#endif

typedef union {
    struct {
        union {
            struct {
                u32 dc;
                u32 cs;
                u32 rst;
                u32 te;
                u32 scl;
            } cio;
        } control;


        union {
            struct {
                u32 d0;
                u32 d1;
                u32 d2;
                u32 d3;
                u32 d4;
                u32 d5;
                u32 d6;
                u32 d7;
            } dio;
            u32 group[8];
        } data;

//        u32 rev[7];
    } spi_drv_io;

    struct {
        union {
            struct {
                u32 d0;
                u32 d1;
                u32 d2;
                u32 d3;
                u32 d4;
                u32 d5;
                u32 d6;
                u32 d7;
                u32 d8;
                u32 d9;
                u32 d10;
                u32 d11;
                u32 d12;
                u32 d13;
                u32 d14;
                u32 d15;
            } dio;
            u32 group[16];
        } data;

        union {
            struct {
                u32 display;
                u32 de;
                u32 clk;
                u32 vsync;
                u32 hsync;
            } cio;
        } control;
    } rgb_drv_io;
} lcd_drv_io_t;

typedef struct _lcd_drv_t {
    lcd_io_type     io_type;
    lcd_drv_io_t    io;
    union {
        struct {
            u32 clk_select;
            u32 clk_div;
            u32 speed1_mhz;
            u32 speed2_mhz;
        } spi_drv_param;

        struct {
            u32 clk_select;
            u32 clk_div;
            u32 in_rgb;
            u32 out_rgb;
            u32 vsync_width;
            u32 vfront_proch;
            u32 vback_porch;
            u32 hsync_width;
            u32 hfront_porch;
            u32 hback_porch;
            lcd_color_t *refersh_dma_buf;       //整屏颜色显示内存
            u32 refersh_dma_buf_size;           //整屏颜色显示内存大小
        } rgb_drv_param;
    } param;
    void (*tft_te_isr)(void);
    void (*tft_reg_init)(void);
    void (*tft_set_window)(u16 x0, u16 y0, u16 x1, u16 y1);
    u32  (*tft_read_id)(void);
    void (*tft_set_brightness)(u8 brightness);
}lcd_drv_t;

/*系统使用*/
void tft_spi_send(void *buf, uint wid, uint hei);
void tft_frame_start(void);
void tft_frame_end(void);
void tft_write_data_start(void);
void tft_write_end();
void tft_write_cmd(u8 cmd);
void tft_write_data(u8 data);
void tft_write_end(void);

void tft_write_cmd32(u8 cmd);
void tft_write_cmd42(u8 cmd);
void tft_write_cmd52(u8 cmd);

void tft_spi_sendbyte(u8 val);
u32 tft_spi_getbyte(void);

#define WriteComm(v)        tft_write_cmd(v)
#define WriteData(v)        tft_write_data(v)
#define CommEnd(v)          tft_write_end()

#define TFT_SPI_CS_DIS()      {lcd_drv_cs_out(1);}
#define TFT_SPI_CS_EN()       {lcd_drv_cs_out(0);}
#define DC_CMD_EN()           {lcd_drv_dc_out(0);}      // DC 拉低
#define DC_DATA_EN()          {lcd_drv_dc_out(1);}      // DC 拉高

/*
 * lcd drv extern
 */
//extern lcd_drv_t lcd_240_st7789V3_i80_drv;
extern lcd_drv_t lcd_320_st77916_drv;
extern lcd_drv_t lcd_oled_466_icna3310b_drv;
//extern lcd_drv_t lcd_360_gc9c01_drv;
//extern lcd_drv_t lcd_240_jd9853_3w9bit_drv;
//extern lcd_drv_t lcd_240_st7789_4w8bit_drv;
//extern lcd_drv_t lcd_128_160_jd9853_i80_drv;
extern lcd_drv_t lcd_oled_368_st7801n_drv;
extern lcd_drv_t lcd_240_st7789V3_i80_drv;
extern lcd_drv_t lcd_vga012a_drv;
extern lcd_drv_t lcd_480_st7283_drv;
extern lcd_drv_t lcd_800_st7265_drv;

/**
 * 初始化LCD
 */
void tft_init(void);

/**
 * 关闭LCD
 */
void tft_exit(void);

/**
 * 注册LCD驱动，且初始化屏幕
 */
void lcd_drv_register(lcd_drv_t *drv);

/**
 * 注销LCD驱动，且把IO口设置为模拟态
 */
void lcd_drv_deregister(void);

/**
 * 重置LCD相关CLK
 */
void lcd_drv_clk_deregister(void);


/**
 * 初始化已注册的LCD驱动
 */
void lcd_drv_init(void);

/**
 * 设置已注册LCD窗
 */
void lcd_drv_set_window(u16 x0, u16 y0, u16 x1, u16 y1);

/**
 * 读取已注册LCD ID
 */
uint32_t lcd_drv_read_id(void);

/**
 * OLED 屏调背光
 */
void lcd_drv_set_brightness(u8 brightness);

void lcd_drv_cs_out(bool is_high);

void lcd_drv_dc_out(bool is_high);

/**
 * 设置TE 模式
 */
void tft_set_temode(u8 mode);

/**
 * 直接打开背光
 */
void tft_bglight_en(void);

/**
 * 设置TE1和TE2模式下的LCD CLK速度
   TE1模式->lcd_clk = 496Mhz/4/(baud1 + 1);
   TE2模式->lcd_clk = 496Mhz/4/(baud2 + 1);
 */
void tft_set_baud(u8 baud1, u8 baud2);

/**
 * @brief 设置oled亮度
 * @param[in] level       亮度等级，使用无极调节时，范围0 ~ 100，否则为1~5
 * @param[in] stepless_en 是否使用无极调节
 *
 * @return  无
 **/
void oled_brightness_set_level(uint8_t level, bool stepless_en);

/**
 * @brief 设置背光亮度
 * @param[in] level       亮度等级，使用无极调节时，范围0 ~ 100，否则为1~5
 * @param[in] stepless_en 是否使用无极调节
 *
 * @return  无
 **/
void tft_bglight_set_level(uint8_t level, bool stepless_en);

/**
 * @brief 首次设置亮度检测
 * @param 无
 *
 * @return  无
 **/
void tft_bglight_frist_set_check(void);

void tft_te_isr(void);

#endif
