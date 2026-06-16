#include "include.h"
// 240 * 296

//#define Delay(ms) bsp_spide_cs(1);delay_ms(ms)
#if (GUI_SELECT == GUI_TFT_240_ST789_i80)
static void tft_240_st7789_i80_init(void)
{
    port_gpio_set_out(IO_PA6, 1);
#if !ELUNCHBOX_PANEL_EN
//     port_gpio_set_out(IO_PA6, 1);   /* 参考板 TE/使能；饭盒 RST=PE8 TE=PE9，勿动 PA6 */
// #endif
// #if !ELUNCHBOX_PANEL_EN
    printf("tft_240_st7789_i80_init\n");
#endif
    delay_ms(120);

    WriteComm(0x11);
    delay_ms(120);                //ms

    WriteComm(0xdf);  //??RGB??);????????);?????
    WriteData(0x5a);   //ee
    WriteData(0x69); //04
    WriteData(0x02); //14
    WriteData(0x01); //14

    WriteComm(0x36);
#if GUI_SPU_ROTATE_90 == 0
    WriteData(0x60);
#else
    WriteData(0x00);
#endif
    WriteComm(0x3A);
    WriteData(0x55);

    WriteComm(0xB2);
    WriteData(0x0C);
    WriteData(0x0C);
    WriteData(0x00);
    WriteData(0x33);
    WriteData(0x33);

    WriteComm(0xB7);
    WriteData(0x35);   //75 VGH=14.97V); VGL=-10.43V

    WriteComm(0xBB);     //VCOM
    WriteData(0x21);   //21

    WriteComm(0xC0);
    WriteData(0x2C);

    WriteComm(0xC2);
    WriteData(0x01);

    WriteComm(0xC3); // VRH
    WriteData(0x10); // 18 0x1A);

    WriteComm(0xC4);   //VRL
    WriteData(0x10); // 10 0x23);

    WriteComm(0xC6);
    WriteData(0x0F);

    WriteComm(0xD0);
    WriteData(0xA4);
    WriteData(0xA1);

    WriteComm(0xd6);
    WriteData(0xa1);

    WriteComm(0xE0);
    WriteData(0x70);
    WriteData(0x04);
    WriteData(0x0A);
    WriteData(0x08);
    WriteData(0x07);
    WriteData(0x05);
    WriteData(0x32);
    WriteData(0x32);
    WriteData(0x48);
    WriteData(0x38);
    WriteData(0x15);
    WriteData(0x15);
    WriteData(0x2A);
    WriteData(0x2E);

    WriteComm(0xE1);
    WriteData(0x70);
    WriteData(0x07);
    WriteData(0x0D);
    WriteData(0x09);
    WriteData(0x09);
    WriteData(0x16);
    WriteData(0x30);
    WriteData(0x44);
    WriteData(0x49);
    WriteData(0x39);
    WriteData(0x16);
    WriteData(0x16);
    WriteData(0x2B);
    WriteData(0x2F);

    WriteComm(0x21);
    /*
    WriteComm(0x2a);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x01);
    WriteData(0x3f);

    WriteComm(0x2b);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0xef);*/

    WriteComm(0x35);
    WriteData(0x00);

    WriteComm(0x29);	  //Display on
    CommEnd();
}


AT(.com_text.tft_spi)
static void tft_240_st7789_i80_set_window(u16 x0, u16 y0, u16 x1, u16 y1)
{
    x0 += GUI_SCREEN_OFS_X;
    x1 += GUI_SCREEN_OFS_X;
    y0 += GUI_SCREEN_OFS_Y;
    y1 += GUI_SCREEN_OFS_Y;
#if GUI_SPU_ROTATE_90 == 0
    tft_write_cmd(0x2A);        //TFT_PASET
    tft_write_data(BYTE1(x0));
    tft_write_data(BYTE0(x0));
    tft_write_data(BYTE1(x1));
    tft_write_data(BYTE0(x1));

    tft_write_cmd(0x2B);        //TFT_CASET
    tft_write_data(BYTE1(y0));
    tft_write_data(BYTE0(y0));
    tft_write_data(BYTE1(y1));
    tft_write_data(BYTE0(y1));
#else
    tft_write_cmd(0x2A);        //TFT_PASET
    tft_write_data(BYTE1(y0));
    tft_write_data(BYTE0(y0));
    tft_write_data(BYTE1(y1));
    tft_write_data(BYTE0(y1));

    tft_write_cmd(0x2B);        //TFT_CASET
    tft_write_data(BYTE1(x0));
    tft_write_data(BYTE0(x0));
    tft_write_data(BYTE1(x1));
    tft_write_data(BYTE0(x1));
#endif
    tft_write_end();
}


/////屏幕如果旋转需要设置下
AT(.com_text.tft_spi)
void tft_halt_240_st7789_i80_set_window(u16 x0, u16 y0, u16 x1, u16 y1)
{
#if GUI_SPU_ROTATE_90 == 1

    WriteComm(0x36);
    WriteData(0xA0);

    x0 += GUI_SCREEN_OFS_X;
    x1 += GUI_SCREEN_OFS_X;
    y0 += GUI_SCREEN_OFS_Y;
    y1 += GUI_SCREEN_OFS_Y;

    tft_write_cmd(0x2A);        //TFT_PASET
    tft_write_data(BYTE1(x0));
    tft_write_data(BYTE0(x0));
    tft_write_data(BYTE1(x1));
    tft_write_data(BYTE0(x1));

    tft_write_cmd(0x2B);        //TFT_CASET
    tft_write_data(BYTE1(y0));
    tft_write_data(BYTE0(y0));
    tft_write_data(BYTE1(y1));
    tft_write_data(BYTE0(y1));

    tft_write_end();
#endif

}

static void tft_240_st7789_i80_set_brightness(uint8_t brightness)
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

lcd_drv_t lcd_240_st7789V3_i80_drv = {
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

    .io_type        = LCD_IO_I8080,
    .param          = {
        .spi_drv_param = {
            .clk_select = LCD_SELECT_PLL0_DIV2,
            .clk_div = 0,
            .speed1_mhz = 14,
            .speed2_mhz = 21,
        },
    },
    .tft_reg_init   = tft_240_st7789_i80_init,
    .tft_set_window = tft_240_st7789_i80_set_window,
    .tft_set_brightness = tft_240_st7789_i80_set_brightness,
    .tft_te_isr     = tft_te_isr,
};

lcd_drv_t *lcd_drv_display = &lcd_240_st7789V3_i80_drv;


#endif

