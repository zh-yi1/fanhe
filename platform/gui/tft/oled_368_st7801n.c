#include "include.h"
// 368 * 448

//#define delay_ms(ms) bsp_spide_cs(1);delay_ms(ms)
#if (GUI_SELECT == GUI_OLED_368_ST7801N)

static void tft_320_st7801n_init(void)
{
    printf("tft_st77801n_init\n");
    // // bits
    // WriteComm(0x40);
    // WriteData(0x02);
    // tft_write_end();

    // WriteComm(0x61);
    // WriteData(0x0b);
    // WriteData(0xA0);
    // tft_write_end();

    // WriteComm(0xF9);
    // WriteData(0x86);  //0x80-0x86
    // tft_write_end();

    // WriteComm(0x61);
    // WriteData(0x00);
    // WriteData(0x00);
    // tft_write_end();

    // WriteComm(0x40);
    // WriteData(0x80);
    // tft_write_end();
    // //bits end

    WriteComm(0x11);
    tft_write_end();
    delay_ms(100);

    WriteComm(0x53);
    WriteData(0x20);
    tft_write_end();

    WriteComm(0x51);  //亮度
    WriteData(0xff);
    tft_write_end();

    WriteComm(0x35);  //te
    WriteData(0x00);
    tft_write_end();

    WriteComm(0x3A);  //rgb565
    WriteData(0x55);
    tft_write_end();

    WriteComm(0x44);
    WriteData(0x00);
    WriteData(0x0A);
    tft_write_end();

    WriteComm(0x29);
    tft_write_end();
    delay_ms(80);
}

static void oled_368_st7801n_set_brightness(u8 brightness)
{
#if MSPI_DDR_MODE_EN
    printf("ddr mode\n");
    tft_write_cmd42(0x51);
#else
    printf("sdr mode\n");
    tft_write_cmd(0x51);
#endif
    tft_write_data(BYTE0(brightness));
    tft_write_end();
}

AT(.com_text.tft_spi)
static void tft_set_window(u16 x0, u16 y0, u16 x1, u16 y1)
{
    x0 += GUI_SCREEN_OFS_X;
    x1 += GUI_SCREEN_OFS_X;
    y0 += GUI_SCREEN_OFS_Y;
    y1 += GUI_SCREEN_OFS_Y;

    tft_write_cmd(0x2A);        //TFT_CASET
    tft_write_data(BYTE1(x0));
    tft_write_data(BYTE0(x0));
    tft_write_data(BYTE1(x1));
    tft_write_data(BYTE0(x1));
    tft_write_end();

    tft_write_cmd(0x2B);        //TFT_PASET
    tft_write_data(BYTE1(y0));
    tft_write_data(BYTE0(y0));
    tft_write_data(BYTE1(y1));
    tft_write_data(BYTE0(y1));
    tft_write_end();
}

//0x03: 2bit
static void tft_read_id_cmd(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON = (LCDSPICON & ~(0x03<<2)) | (0x00<<2);       //1data in 1data out
    u8 lcdcon = LCDCON;
    LCDCON = lcdcon | BIT(21);
    tft_spi_sendbyte(0x03);
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

static uint32_t tft_read_id(void)
{
    u32 lcdcon_bak = LCDCON;
    u32 lcdspicon_bak = LCDSPICON;
    uint32_t id = 0;
    //tft_read_cmd(0x04);
    tft_read_id_cmd(0x04);
    id = tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    tft_write_end();
    LCDCON = lcdcon_bak;
    LCDSPICON = lcdspicon_bak;
    return id;
}

#define DMA_FILL_TEST 0

#if DMA_FILL_TEST
static u8 debug_tft_dma_buf[368*2];
#endif

void spi_display_point(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    delay_5ms(200);
    tft_set_window(x0, y0, x1, y1);
    tft_frame_start();

#if DMA_FILL_TEST //DMA buf
    for (int i = 0; i <= x1*2; i += 2) {
        debug_tft_dma_buf[i + 1] = color >> 8;      //dataH => SDA
        debug_tft_dma_buf[i] = color & 0xff;        //dataL => DC
    }

    for (int i = 0; i <= (y1-y0)+1; i++) {
        WDT_CLR();
        tft_spi_send(debug_tft_dma_buf, (x1-x0)+1, 1);
    }
#else
    for (int x = 0; x <= x1; x++) {
        for (int y = 0; y <= y1; y++) {
            WDT_CLR();
            WriteData(color >> 8);
            WriteData(color);
        }
    }
#endif
    tft_write_end();
    printf("%s done\n", __func__);
}

lcd_drv_t lcd_oled_368_st7801n_drv = {
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
            .clk_div = 3,
            .speed1_mhz = 28,
            .speed2_mhz = 28,
        },
    },
    .tft_reg_init   = tft_320_st7801n_init,
    .tft_set_window = tft_set_window,
    .tft_read_id     = tft_read_id,
    .tft_set_brightness = oled_368_st7801n_set_brightness,
    .tft_te_isr     = tft_te_isr,
};

#endif
