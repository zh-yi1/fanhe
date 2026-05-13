#include "include.h"
// 640 * 480

//#define Delay(ms) bsp_spide_cs(1);delay_ms(ms)

u8 share_lcd_indata  AT(.share.tft_vga);// 是否将数据转换成黑白图

#if ((GUI_SELECT == GUI_VGS_640))

// 读寄存器数据 0x79 cmd 0xxx  完了之后需要把cs 拉高
/*
static u8 tft_vga_read_data(u8 cmd)
{

    u32 lcdcon_bak = LCDCON;
    u32 lcdspicon_bak = LCDSPICON;
    u8 data;
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON = (LCDSPICON & ~(0x03<<2)) | (0x00<<2);       //1data in 1data out
    u8 lcdcon = LCDCON;
    LCDCON = lcdcon | BIT(21);

    tft_spi_sendbyte(0x79);
    tft_spi_sendbyte(cmd);
    data = tft_spi_getbyte();
    TFT_SPI_CS_DIS();
    LCDCON = lcdcon_bak;
    LCDSPICON = lcdspicon_bak;
    return data;

}
*/
// 读寄存器数据 0x78 cmd data  完了之后需要把cs 拉高
static void tft_vga_write_data(u8 cmd, u8 data)
{
    u32 lcdcon_bak = LCDCON;
    u32 lcdspicon_bak = LCDSPICON;

    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON = (LCDSPICON & ~(0x03<<2)) | (0x00<<2);       //1data in 1data out
    u8 lcdcon = LCDCON;
    LCDCON = lcdcon | BIT(21);

    tft_spi_sendbyte(0x78);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(data); // data
    TFT_SPI_CS_DIS();
    LCDCON = lcdcon_bak;
    LCDSPICON = lcdspicon_bak;

}

static void tft_vga012a_init(void)
{
    printf("tft_vga012a_init\n");
    share_lcd_indata = 1;

    delay_5ms(10);


    // 使用RGB565数据 默认值是0x02 GRAY256
    tft_vga_write_data(0x00, 0x81);
    // 设置亮度为03  默认值是0xff
    tft_vga_write_data(0x1d, 0x03);
    // 设置镜像 BIT(4) 1 垂直镜像开启 0 垂直镜像关闭  BIT(3) 1 水平镜像开启 0 水平镜像关闭  默认值是00
    tft_vga_write_data(0x1c, 0x10);

    tft_vga_write_data(0x1b, 0x04);


}

AT(.com_text.tft_spi)
static void tft_vga012a_set_window(u16 x0, u16 y0, u16 x1, u16 y1)
{
    printf("%s line: %d %x %x \n", __func__, __LINE__, x0, x1);
    x0 += GUI_SCREEN_OFS_X;
    x1 += GUI_SCREEN_OFS_X;
    y0 += GUI_SCREEN_OFS_Y;
    y1 += GUI_SCREEN_OFS_Y;
    printf("%s line: %d %x %x \n", __func__, __LINE__, x0, x1);


    u32 lcdcon_bak = LCDCON;
    u32 lcdspicon_bak = LCDSPICON;

    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON = (LCDSPICON & ~(0x03<<2)) | (0x00<<2);       //1data in 1data out
    u8 lcdcon = LCDCON;
    LCDCON = lcdcon | BIT(21);

    tft_spi_sendbyte(0x02);

    tft_spi_sendbyte(00);
    tft_spi_sendbyte(0x2a); // data
    tft_spi_sendbyte(0x00);

    tft_spi_sendbyte(BYTE1(y0));
    tft_spi_sendbyte(BYTE0(y0)); // data
    tft_spi_sendbyte(BYTE1(y1));
    tft_spi_sendbyte(BYTE0(y1));

    TFT_SPI_CS_DIS();
    LCDCON = lcdcon_bak;
    LCDSPICON = lcdspicon_bak;


}

static uint32_t tft_read_id(void)
{

    uint32_t id = 0;

    return id;
}

static void tft_vga012a_set_brightness(uint8_t brightness)
{
    tft_cb_t* tft_get_tft_cb(void);
    tft_cb_t *tft_cb = tft_get_tft_cb();

    int8_t  base_duty = 0;  //根据限流电阻调整占空比增益
    int8_t  duty = 0;

    if(100 < brightness)
    {
        brightness = 100;
    }
    duty = base_duty + brightness;
    tft_cb->tft_bglight_duty = duty;

    if (tft_cb->tft_bglight_last_duty != tft_cb->tft_bglight_duty)
    {
        bsp_pwm_duty_set(PORT_TFT_BL, tft_cb->tft_bglight_duty, false);
        tft_cb->tft_bglight_last_duty = tft_cb->tft_bglight_duty;
    }
}

lcd_drv_t lcd_vga012a_drv = {
    .io = {
        .spi_drv_io = {
            .data = {
                .dio = {
                    .d0 = PORT_TFT_LCD_D0,
                    .d1 = PORT_TFT_LCD_D1,
                    .d2 = PORT_TFT_LCD_D2,
                    .d3 = PORT_TFT_LCD_D3,
                    .d4 = PORT_TFT_LCD_D4,
                    .d5 = PORT_TFT_LCD_D5,
                    .d6 = PORT_TFT_LCD_D6,
                    .d7 = PORT_TFT_LCD_D7,
                }
            },

            .control = {
                .cio = {
                    .dc = PORT_TFT_DC,
                    .cs = PORT_TFT_CS,
                    .rst = PORT_TFT_RST,
                    .te = PORT_TFT_INT,
                    .scl = PORT_TFT_LCD_SCL,
                }
            },
        },
    },

    .io_type        = LCD_IO_QSPI,
    .param          = {
        .spi_drv_param = {
            .clk_select = LCD_SELECT_PLL0_DIV2,
            .clk_div = 0,
            .speed1_mhz = 48,
            .speed2_mhz = 48,
        },
    },
    .tft_reg_init   = tft_vga012a_init,
    .tft_set_window = tft_vga012a_set_window,
    .tft_read_id    = tft_read_id,
    .tft_set_brightness = tft_vga012a_set_brightness,
    .tft_te_isr     = tft_te_isr,
};
#endif

