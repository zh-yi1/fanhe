#include "include.h"
// 466 * 466

//#define Delay(ms) bsp_spide_cs(1);delay_ms(ms)
#if (GUI_SELECT == GUI_OLED_466_ICNA3310B)
static void oled_466_icna3310b_init(void)
{
 	WriteComm(0xfe);
	WriteData(0x20);
	WriteComm(0xf4);
	WriteData(0x5a);
	WriteComm(0xf5);
	WriteData(0x59);
	WriteComm(0xfe);
	WriteData(0x40);
	WriteComm(0x08);
	WriteData(0x0a);
	WriteComm(0xfe);
	WriteData(0x00);
	WriteComm(0xc4);
	WriteData(0x80);
	WriteComm(0x3a);
	WriteData(0x55);
	WriteComm(0x35);
	WriteData(0x00);
	WriteComm(0x53);
	WriteData(0x20);
	WriteComm(0x51);
	WriteData(0x00);        //brightness off
	WriteComm(0x63);
	WriteData(0xff);

    WriteComm(0x35);        //TE On
    WriteData(0x00);

	WriteComm(0x11);

    CommEnd();
	delay_ms(120);
	WriteComm(0x29);
//  	WriteComm(0x2C);

    CommEnd();
}

AT(.com_text.tft_spi)
static void tft_oled_466_icna3310b_set_window(u16 x0, u16 y0, u16 x1, u16 y1)
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

    tft_write_cmd(0x2B);        //TFT_PASET
    tft_write_data(BYTE1(y0));
    tft_write_data(BYTE0(y0));
    tft_write_data(BYTE1(y1));
    tft_write_data(BYTE0(y1));
    tft_write_end();
}

static void tft_oled_466_icna3310b_brightness(u8 brightness)
{
    uint16_t level_tmp = 0xff * brightness / 100;
    brightness = (uint8_t)level_tmp;
    os_mutex_gui_take();
	tft_write_cmd(0x51);
	tft_write_data(BYTE0(brightness));
	tft_write_end();
	os_mutex_gui_release();
}

//0x03: 2bit
static void tft_read_id_cmd(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON = (LCDSPICON & ~(0x03<<2)) | (0x01<<2);       //1data in 1data out
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
    tft_read_id_cmd(0x04);
    id = tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    tft_write_end();
    LCDCON = lcdcon_bak;
    LCDSPICON = lcdspicon_bak;
	printf("oled_466_icna3310b id:%08x\n", id);

    return id;
}

lcd_drv_t lcd_oled_466_icna3310b_drv = {
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
            .speed1_mhz = 58,
            .speed2_mhz = 45,
        },
    },
    .tft_reg_init   = oled_466_icna3310b_init,
    .tft_set_window = tft_oled_466_icna3310b_set_window,
    .tft_set_brightness = tft_oled_466_icna3310b_brightness,
	.tft_read_id = tft_read_id,
    .tft_te_isr     = tft_te_isr,
};
#endif
