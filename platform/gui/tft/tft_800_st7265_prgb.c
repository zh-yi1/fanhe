#include "include.h"

#if (GUI_SELECT == GUI_LCD_800_ST7265)

static u8 refersh_dma_buf[GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*GUI_COLOR_DEPTH] AT(.psram_buf.lcd);

static void tft_800_st7265_prgb_set_brightness(uint8_t brightness)
{
    printf("%s\n", __func__);
}

lcd_drv_t lcd_800_st7265_drv = {
    .io = {
        .rgb_drv_io = {
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
                    .d8 = PORT_TFT_LCD_D8,
                    .d9 = PORT_TFT_LCD_D9,
                    .d10 = PORT_TFT_LCD_D10,
                    .d11 = PORT_TFT_LCD_D11,
                    .d12 = PORT_TFT_LCD_D12,
                    .d13 = PORT_TFT_LCD_D13,
                    .d14 = PORT_TFT_LCD_D14,
                    .d15 = PORT_TFT_LCD_D15,
                }
            },

            .control = {
                .cio = {
                    .display = PORT_LCD_DISPLAY,
                    .de = PORT_LCD_DE,
                    .clk = PORT_TFT_LCD_SCL,
                    .vsync = PORT_LCD_VSYNC,
                    .hsync = PORT_LCD_HSYNC,
                }
            },
        },
    },

    .io_type = LCD_IO_PRGB16,
    .param = {
        .rgb_drv_param = {
            .clk_select     = LCD_SELECT_XOSC_X2CLK,
            .clk_div        = 1,
            .in_rgb         = LCD_IN_RGB565,
            .out_rgb        = LCD_OUT_RGB888,
            .vsync_width    = 4,
            .vfront_proch   = 16,
            .vback_porch    = 16,
            .hsync_width    = 4,
            .hfront_porch   = 8,
            .hback_porch    = 8,
            .refersh_dma_buf = (lcd_color_t*)refersh_dma_buf,
            .refersh_dma_buf_size = GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*GUI_COLOR_DEPTH,
        },
    },

    .tft_reg_init = NULL,
    .tft_set_window = NULL,
    .tft_read_id = NULL,
    .tft_set_brightness = tft_800_st7265_prgb_set_brightness,
    .tft_te_isr = NULL,
};

#endif

