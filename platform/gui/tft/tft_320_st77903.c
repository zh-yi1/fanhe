#include "include.h"
// 320 * 386

//#define Delay(ms) bsp_spide_cs(1);delay_ms(ms)
#if (GUI_SELECT == GUI_TFT_320_ST77903)


#define WriteData(v) tft_write_data(v)
#define CommEnd()    tft_write_end()
// cmd 12h 1:cmd      4:addr/data
// cmd 32h 1:cmd/addr 4:data
// cmd 02h 1:cmd      1:addr/data
#define WriteComm12(v) \
({                   \
bsp_spide_cs(1);         \
bsp_spide_cs(0);         \
bsp_spide_bus_mode(SPIDE_1IO);   \
bsp_spide_snd_byte(0x12);\
bsp_spide_bus_mode(SPIDE_4IO);   \
bsp_spide_snd_byte(0x00);\
bsp_spide_snd_byte(v);   \
bsp_spide_snd_byte(0x00);\
})
#define WriteComm32(v) \
({                   \
bsp_spide_cs(1);         \
bsp_spide_cs(0);         \
bsp_spide_bus_mode(SPIDE_1IO);   \
bsp_spide_snd_byte(0x32);\
bsp_spide_snd_byte(0x00);\
bsp_spide_snd_byte(v);   \
bsp_spide_snd_byte(0x00);\
bsp_spide_bus_mode(SPIDE_4IO);   \
})
#define WriteComm02(v) \
({                   \
bsp_spide_cs(1);         \
bsp_spide_cs(0);         \
bsp_spide_bus_mode(SPIDE_1IO);   \
bsp_spide_snd_byte(0x02);\
bsp_spide_snd_byte(0x00);\
bsp_spide_snd_byte(v);   \
bsp_spide_snd_byte(0x00);\
})
#define ReadComm03(v) \
({                   \
bsp_spide_cs(1);         \
bsp_spide_cs(0);         \
bsp_spide_bus_mode(SPIDE_1IO);   \
bsp_spide_snd_byte(0x03);\
bsp_spide_snd_byte(0x00);\
bsp_spide_snd_byte(v);   \
bsp_spide_snd_byte(0x00);\
})

#define LCD_FRAME_CMD   0xDE
#define LCD_FRAME_ADR   0x61
#define LCD_LINE_CMD    0xDE
#define LCD_LINE_ADR    0x60

static u8 frame_buf[GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*2] AT(.ramless.buf);

//0x02: 1CMD 1ADDR 1DATA
AT(.com_text.tft_spi)
void tft_write_cmd(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON &= ~BIT(3);                       //1BIT
    tft_spi_sendbyte(0xDE);
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

//0xd8: 1CMD 1ADDR 1DATA
AT(.com_text.tft_spi)
void tft_write_cmdd8(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON &= ~BIT(3);                       //1BIT
    tft_spi_sendbyte(0xd8);
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

//0xd8: 1CMD 1ADDR 1DATA
AT(.com_text.tft_spi)
void tft_write_cmdde(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON &= ~BIT(3);                       //1BIT
    tft_spi_sendbyte(0xde);
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

//0x12: 1CMD 4ADDR 4DATA
AT(.com_text.tft_spi)
void tft_write_cmd12(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON &= ~BIT(3);                       //1BIT
    tft_spi_sendbyte(0x12);
    LCDSPICON |= BIT(3);                        //4BIT
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

//0x03: 1CMD 1ADDR 1DATA
AT(.com_text.tft_spi)
void tft_read_cmd(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON &= ~BIT(3);                       //1BIT
    tft_spi_sendbyte(0x03);
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

AT(.com_text.tft_spi)
void tft_write_data(u8 data)
{
    tft_spi_sendbyte(data);
}

AT(.com_text.tft_spi)
void tft_write_end()
{
    TFT_SPI_CS_DIS();
}

AT(.com_text.tft_spi)
void tft_set_window(u16 x0, u16 y0, u16 x1, u16 y1)
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

AT(.com_text.tft_spi)
void tft_write_data_start(void)
{
//    tft_write_cmd12(0x2C);      //TFT_RAMWR
}

//u8 tbuf[320*2];
void my_test(void)
{
//    memset(tbuf, 0, GUI_SCREEN_WIDTH*2);
//    for (int y = 0;y < 385; y++){                   //行间隔40us
//        LCDSPICON |= BIT(3);                         //4bit                       //4bit
//        tft_spi_send(tbuf, GUI_SCREEN_WIDTH, 1);
////        tft_write_end();
////        delay_us(2);                               //加点延时否则屏幕处理不过来
//    }
//    LCDSPICON &= ~BIT(3);
}

void st77903_frame_begin (void)
{
    int i;

    tft_write_cmdde(0x61);
    my_test();


    delay_us(40);
    tft_write_end();

    for (i = 0; i < 3; ++i) {
        tft_write_cmdde(0x60);
        my_test();

        delay_us(40);
        tft_write_end();
    }

}

void st77903_frame_end (void)
{
    int i;

    for (i = 0; i < 3; ++i) {
        tft_write_cmdde(0x60);
        my_test();

        delay_us(40);
        tft_write_end();
    }

}

//AT(.com_text.tft_spi)
//bool tft_ramless_is_support(void)
//{
//    return true;
//}

//AT(.com_text.tft_spi)
//void tft_spi_kick(void *buf, u16 line_cur, u16 len)
//{
////    GPIOESET = BIT(0);
//    memcpy(frame_buf + line_cur * (GUI_SCREEN_WIDTH << 1), buf, len << 1);
////    GPIOECLR = BIT(0);
//}

void st77903_display_test_nomal(uint16_t color)
{
    for (int y = 0;y < 385; y++){                   //行间隔40us
        tft_write_cmdde(0x60);
        LCDSPICON |= BIT(3);                         //4bit                       //4bit
//        memcpy(frame_buf, pic_buf + GUI_SCREEN_WIDTH*2*y, GUI_SCREEN_WIDTH*2);
//        memset(frame_buf, 0xF8, GUI_SCREEN_WIDTH*2);
        tft_spi_send(frame_buf + GUI_SCREEN_WIDTH*2*y, GUI_SCREEN_WIDTH, 1);
        tft_write_end();
//        delay_us(2);                               //加点延时否则屏幕处理不过来
    }

}

void st77903_auto_refresh(void)
{
    u32 tick = tick_get();
    for(u16 i=0;i<GUI_SCREEN_HEIGHT;i++) {
        memset(frame_buf + GUI_SCREEN_WIDTH * 2 * i, 0, GUI_SCREEN_WIDTH * 2);
    }
    printf("tick:%d\n", tick_get() - tick);
#if 1
    LCDCMD = (LCD_FRAME_CMD << 24) | (LCD_FRAME_ADR << 16) | (LCD_LINE_CMD << 8) | LCD_LINE_ADR;
    LCDVPP = (2 << 16) | (3 << 8) | 1;
    LCDHPP = BIT(27);
    LCDCON |= BIT(24) | BIT(22) | BIT(20) | BIT(16) /*| BIT(19)*/;
    LCDSPICON |= BIT(3);                        //4BIT

    GPIOAFEN |= BIT(5);                        // CS
    GPIOADE  |=  BIT(5);
    GPIOADIR &= ~BIT(5);

    printf("LCDSPICON:%x, LCDCON:%x\n", LCDSPICON, LCDCON);
    tft_spi_send(frame_buf, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    printf("auto test\n");

    printf("LCDCON:%08X\n", LCDCON);
    printf("LCDADR:%08X\n", LCDADR);
    printf("LCDSIZE:%08X\n", LCDSIZE);
    printf("LCDSTRD:%08X\n", LCDSTRD);
    printf("LCDVPP:%08X\n", LCDVPP);
    printf("LCDHPP:%08X\n", LCDHPP);
    printf("LCDCMD:%08X\n", LCDCMD);
    printf("LCDPEND:%08X\n", LCDPEND);
    printf("LCDSPICON:%x\n", LCDSPICON);

    extern void gpu_share_tft_init(bool rgbw_support, bool ramless_support,
                        void* ramless_buf, u16 ramless_line_wid);
    gpu_share_tft_init(false, true, frame_buf, GUI_SCREEN_WIDTH);

#else
    memset(frame_buf, 0xff, GUI_SCREEN_WIDTH * 2);
    while(1) {

        u32 tick = tick_get();
        st77903_frame_begin();
        st77903_display_test_nomal(0xff);
        st77903_frame_end();
        WDT_CLR();
        printf("%d ", tick_get() - tick);
    }
#endif
}


void tft_write_cmd(u8 cmd);
void tft_write_data(u8 data);
void tft_write_end(void);

#define WriteComm(v) tft_write_cmdde(v)
#define ReadComm(v)  ReadComm03(v)

//0x03: 2bit
void tft_read_id_cmd(u8 cmd)
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

uint32_t tft_read_id(void)
{
printf("tft_read_id\n");
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
printf("tft_read_id: %x\n", id);
    return id;
}

void tft_320_st77903_init(void)
{
    printf("tft_st77903_init\n");

    WriteComm(0xF0);
    WriteData(0xC3);

    WriteComm(0xF0);
    WriteData(0x96);

    WriteComm(0xF0);
    WriteData(0xA5);

    WriteComm(0xC1);
    WriteData(0x55);   //VGH=14.5V
    WriteData(0x08);   //VGL=-11.6V
    WriteData(0xBC);   //AVDD=6.7V ,AVEE=-4.1V
    WriteData(0x11);   //VOP=4.5V 2022/6/20

    WriteComm(0xC2);
    WriteData(0x55);   //VGH=14.5V
    WriteData(0x08);   //VGL=-11.6V
    WriteData(0xBC);   //AVDD=6.7V ,AVEE=-4.1V
    WriteData(0x11);   //VOP=4.5V  2022/6/20

    WriteComm(0xC3);
    WriteData(0x44);
    WriteData(0x04);
    WriteData(0x44);
    WriteData(0x04);

    WriteComm(0xC4);
    WriteData(0x44);
    WriteData(0x04);
    WriteData(0x44);
    WriteData(0x04);

    WriteComm(0xC5);
    WriteData(0x5B);   //VCOM=1.3V
    WriteData(0x80);   //Uses VMF1

    WriteComm(0xD6);
    WriteData(0x00);

    WriteComm(0xD7);
    WriteData(0x00);

    WriteComm(0xE0);     //2022 6 20 Gamma 2.2
    WriteData(0xF0);
    WriteData(0x04);
    WriteData(0x08);
    WriteData(0x09);
    WriteData(0x08);
    WriteData(0x25);
    WriteData(0x2A);
    WriteData(0x33);
    WriteData(0x42);
    WriteData(0x28);
    WriteData(0x14);
    WriteData(0x15);
    WriteData(0x2B);
    WriteData(0x31);

    WriteComm(0xE1);
    WriteData(0xF0);
    WriteData(0x04);
    WriteData(0x07);
    WriteData(0x08);
    WriteData(0x07);
    WriteData(0x25);
    WriteData(0x29);
    WriteData(0x33);
    WriteData(0x41);
    WriteData(0x27);
    WriteData(0x14);
    WriteData(0x14);
    WriteData(0x2A);
    WriteData(0x31);

    WriteComm(0xE5);
    WriteData(0xCB);   //GVDD=6.7V，GVEE=-4.1V
    WriteData(0xF5);
    WriteData(0xD3);   //SVDD=6.7V，SVEE=-4.1V
    WriteData(0x55);
    WriteData(0x22);
    WriteData(0x25);
    WriteData(0x10);
    WriteData(0x22);
    WriteData(0x22);
    WriteData(0x22);
    WriteData(0x22);
    WriteData(0x22);
    WriteData(0x22);
    WriteData(0x22);

    WriteComm(0xE6);
    WriteData(0xCB);   //GVDD=6.7V，GVEE=-4.1V
    WriteData(0xF5);
    WriteData(0xD3);   //SVDD=6.7V，SVEE=-4.1V
    WriteData(0x55);
    WriteData(0x22);
    WriteData(0x25);
    WriteData(0x10);
    WriteData(0x22);
    WriteData(0x22);
    WriteData(0x22);
    WriteData(0x22);
    WriteData(0x22);
    WriteData(0x22);
    WriteData(0x22);

    WriteComm(0x36);
    WriteData(0x0C);

    WriteComm(0x3A);
    WriteData(0x05);

    WriteComm(0xB2);
    WriteData(0x09);

    WriteComm(0xB3);
    WriteData(0x01);

    WriteComm(0xB4);
    WriteData(0x01);

    WriteComm(0xB5);
    WriteData(0x00);
    WriteData(0x08);
    WriteData(0x00);
    WriteData(0x08);

    WriteComm(0xB6);
    WriteData(0xC0);
    WriteData(0x27);

    WriteComm(0xA4);
    WriteData(0xC0);
    WriteData(0x63);

    WriteComm(0xA5);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x15);
    WriteData(0x2A);
    WriteData(0xBA);
    WriteData(0x02);

    WriteComm(0xA6);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x15);
    WriteData(0x2A);
    WriteData(0xBA);
    WriteData(0x02);

    WriteComm(0xBA);
    WriteData(0x2A);
    WriteData(0x06);
    WriteData(0x32);
    WriteData(0x00);
    WriteData(0x21);
    WriteData(0x01);
    WriteData(0x04);

    WriteComm(0xBB);
    WriteData(0x00);
    WriteData(0x29);
    WriteData(0x00);
    WriteData(0x31);
    WriteData(0x83);
    WriteData(0x07);
    WriteData(0x18);
    WriteData(0x00);

    WriteComm(0xBC);
    WriteData(0x00);
    WriteData(0x29);
    WriteData(0x00);
    WriteData(0x31);
    WriteData(0x83);
    WriteData(0x07);
    WriteData(0x18);
    WriteData(0x00);

    WriteComm(0xBD);
    WriteData(0x93);
    WriteData(0x39);
    WriteData(0xFF);
    WriteData(0xFF);
    WriteData(0x85);
    WriteData(0x76);
    WriteData(0x67);
    WriteData(0x58);
    WriteData(0xFF);
    WriteData(0x14);
    WriteData(0x0F);

    WriteComm(0xA0);
    WriteData(0x00);   //RGB setting DE+SYNC mode,
    WriteData(0x06);
    WriteData(0x06);

    WriteComm(0x35);
    WriteData(0x00);

    WriteComm(0x21);

    WriteComm(0x11);
CommEnd();
	delay_ms(120);
	WriteComm(0x29);

CommEnd();
}
#endif

