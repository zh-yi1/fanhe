#include "include.h"
// 320 * 385

//#define Delay(ms) bsp_spide_cs(1);delay_ms(ms)
#if (GUI_SELECT == GUI_TFT_RGBW_320_ST77916)

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

AT(.com_text.tft_spi)
bool tft_rgbw_is_support(void)
{
    return true;
}

//0x02: 1CMD 1ADDR 1DATA
AT(.com_text.tft_spi)
void tft_write_cmd(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    DESPICON &= ~BIT(3);                        //1BIT
    tft_spi_sendbyte(0x02);
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
    DESPICON &= ~BIT(3);                        //1BIT
    tft_spi_sendbyte(0x12);
    DESPICON |= BIT(3);                         //4BIT
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
    DESPICON &= ~BIT(3);                        //1BIT
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


    x0 = (x0+1)*4/3 - 1;
    x1 = (x1+1)*4/3 - 1;
    printf("x0 = %d, x1 = %d\n", x0, x1);

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
    tft_write_cmd12(0x2C);      //TFT_RAMWR
}



void tft_write_cmd(u8 cmd);
void tft_write_data(u8 data);
void tft_write_end(void);

#define WriteComm(v) tft_write_cmd(v)
#define ReadComm(v)  ReadComm03(v)

uint32_t tft_read_id(void)
{
    uint32_t id = 0;
    tft_read_cmd(0x04);
    id = tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    tft_write_end();
    return id;
}


void tft_rgbw_320_st77916_init(void)
{
    printf("tft_st77916_init rgbw\n");

    WriteComm(0xF0);
    WriteData(0x08);

    WriteComm(0xF2);
    WriteData(0x08);

    WriteComm(0x9B);
    WriteData(0x51);

    WriteComm(0x86);
    WriteData(0x53);

    WriteComm(0xF2);
    WriteData(0x80);

    WriteComm(0xF0);
    WriteData(0x00);

    WriteComm(0xF0);
    WriteData(0x28);

    WriteComm(0xF2);
    WriteData(0x28);

    WriteComm(0x83);
    WriteData(0xE0);

    WriteComm(0x84);
    WriteData(0x61);

    WriteComm(0xF2);
    WriteData(0x82);

    WriteComm(0xF0);
    WriteData(0x00);

    WriteComm(0xF0);
    WriteData(0x01);

    WriteComm(0xF1);
    WriteData(0x01);

    WriteComm(0xB0);
    WriteData(0x52);

    WriteComm(0xB1);
    WriteData(0x49);

    WriteComm(0xB2);
    WriteData(0x24);

    WriteComm(0xB4);
    WriteData(0x69);

    WriteComm(0xB5);
    WriteData(0x45);

    WriteComm(0xB6);
    WriteData(0x8B);

    WriteComm(0xB7);
    WriteData(0x41);

    WriteComm(0xB8);
    WriteData(0x05);

    WriteComm(0xBA);
    WriteData(0x00);

    WriteComm(0xBB);
    WriteData(0x08);

    WriteComm(0xBC);
    WriteData(0x08);

    WriteComm(0xBD);
    WriteData(0x00);

    WriteComm(0xC0);
    WriteData(0x80);

    WriteComm(0xC1);
    WriteData(0x08);

    WriteComm(0xC2);
    WriteData(0x35);

    WriteComm(0xC3);
    WriteData(0x80);

    WriteComm(0xC4);
    WriteData(0x08);

    WriteComm(0xC5);
    WriteData(0x35);

    WriteComm(0xC6);
    WriteData(0xA9);

    WriteComm(0xC7);
    WriteData(0x41);

    WriteComm(0xC8);
    WriteData(0x51);

    WriteComm(0xC9);
    WriteData(0xA9);

    WriteComm(0xCA);
    WriteData(0x41);

    WriteComm(0xCB);
    WriteData(0x51);

    WriteComm(0xD0);
    WriteData(0xD1);

    WriteComm(0xD1);
    WriteData(0x40);

    WriteComm(0xD2);
    WriteData(0x82);

    WriteComm(0xF5);
    WriteData(0x00);
    WriteData(0xA5);

    WriteComm(0xDD);
    WriteData(0x49);

    WriteComm(0xDE);
    WriteData(0x49);

    WriteComm(0xF1);
    WriteData(0x10);

    WriteComm(0xF0);
    WriteData(0x00);

    WriteComm(0xF0);
    WriteData(0x02);

    WriteComm(0xe0);
    WriteData(0xf0);
    WriteData(0x0c);
    WriteData(0x13);
    WriteData(0x0b);
    WriteData(0x0b);
    WriteData(0x07);
    WriteData(0x3d);
    WriteData(0x33);
    WriteData(0x54);
    WriteData(0x0a);
    WriteData(0x16);
    WriteData(0x16);
    WriteData(0x34);
    WriteData(0x37);

    WriteComm(0xe1);
    WriteData(0xf0);
    WriteData(0x0c);
    WriteData(0x14);
    WriteData(0x0c);
    WriteData(0x0c);
    WriteData(0x08);
    WriteData(0x3e);
    WriteData(0x44);
    WriteData(0x54);
    WriteData(0x0a);
    WriteData(0x17);
    WriteData(0x17);
    WriteData(0x35);
    WriteData(0x37);

    WriteComm(0xF0);
    WriteData(0x10);

    WriteComm(0xF3);
    WriteData(0x10);

    WriteComm(0xE0);
    WriteData(0x08);

    WriteComm(0xE1);
    WriteData(0x00);

    WriteComm(0xE2);
    WriteData(0x0B);

    WriteComm(0xE3);
    WriteData(0x00);

    WriteComm(0xE4);
    WriteData(0xE0);

    WriteComm(0xE5);
    WriteData(0x06);

    WriteComm(0xE6);
    WriteData(0x21);

    WriteComm(0xE7);
    WriteData(0x10);

    WriteComm(0xE8);
    WriteData(0x8A);

    WriteComm(0xE9);
    WriteData(0x82);

    WriteComm(0xEA);
    WriteData(0xE4);

    WriteComm(0xEB);
    WriteData(0x00);

    WriteComm(0xEC);
    WriteData(0x00);

    WriteComm(0xED);
    WriteData(0x14);

    WriteComm(0xEE);
    WriteData(0xFF);

    WriteComm(0xEF);
    WriteData(0x00);

    WriteComm(0xF8);
    WriteData(0xFF);

    WriteComm(0xF9);
    WriteData(0x00);

    WriteComm(0xFA);
    WriteData(0x00);

    WriteComm(0xFB);
    WriteData(0x30);

    WriteComm(0xFC);
    WriteData(0x00);

    WriteComm(0xFD);
    WriteData(0x00);

    WriteComm(0xFE);
    WriteData(0x00);

    WriteComm(0xFF);
    WriteData(0x00);

    WriteComm(0x60);
    WriteData(0x50);

    WriteComm(0x61);
    WriteData(0x02);

    WriteComm(0x62);
    WriteData(0x0B);

    WriteComm(0x63);
    WriteData(0x50);

    WriteComm(0x64);
    WriteData(0x04);

    WriteComm(0x65);
    WriteData(0x0B);

    WriteComm(0x66);
    WriteData(0x53);

    WriteComm(0x67);
    WriteData(0x08);

    WriteComm(0x68);
    WriteData(0x0B);

    WriteComm(0x69);
    WriteData(0x53);

    WriteComm(0x6A);
    WriteData(0x0A);

    WriteComm(0x6B);
    WriteData(0x0B);

    WriteComm(0x70);
    WriteData(0x50);

    WriteComm(0x71);
    WriteData(0x01);

    WriteComm(0x72);
    WriteData(0x0B);

    WriteComm(0x73);
    WriteData(0x50);

    WriteComm(0x74);
    WriteData(0x03);

    WriteComm(0x75);
    WriteData(0x0B);

    WriteComm(0x76);
    WriteData(0x53);

    WriteComm(0x77);
    WriteData(0x07);

    WriteComm(0x78);
    WriteData(0x0B);

    WriteComm(0x79);
    WriteData(0x53);

    WriteComm(0x7A);
    WriteData(0x09);

    WriteComm(0x7B);
    WriteData(0x0B);

    WriteComm(0x80);
    WriteData(0x58);

    WriteComm(0x81);
    WriteData(0x00);

    WriteComm(0x82);
    WriteData(0x04);

    WriteComm(0x83);
    WriteData(0x03);

    WriteComm(0x84);
    WriteData(0x0C);

    WriteComm(0x85);
    WriteData(0x00);

    WriteComm(0x86);
    WriteData(0x00);

    WriteComm(0x87);
    WriteData(0x00);

    WriteComm(0x88);
    WriteData(0x58);

    WriteComm(0x89);
    WriteData(0x00);

    WriteComm(0x8A);
    WriteData(0x06);

    WriteComm(0x8B);
    WriteData(0x03);

    WriteComm(0x8C);
    WriteData(0x0E);

    WriteComm(0x8D);
    WriteData(0x00);

    WriteComm(0x8E);
    WriteData(0x00);

    WriteComm(0x8F);
    WriteData(0x00);

    WriteComm(0x90);
    WriteData(0x58);

    WriteComm(0x91);
    WriteData(0x00);

    WriteComm(0x92);
    WriteData(0x08);

    WriteComm(0x93);
    WriteData(0x03);

    WriteComm(0x94);
    WriteData(0x10);

    WriteComm(0x95);
    WriteData(0x00);

    WriteComm(0x96);
    WriteData(0x00);

    WriteComm(0x97);
    WriteData(0x00);

    WriteComm(0x98);
    WriteData(0x58);

    WriteComm(0x99);
    WriteData(0x00);

    WriteComm(0x9A);
    WriteData(0x0A);

    WriteComm(0x9B);
    WriteData(0x03);

    WriteComm(0x9C);
    WriteData(0x12);

    WriteComm(0x9D);
    WriteData(0x00);

    WriteComm(0x9E);
    WriteData(0x00);

    WriteComm(0x9F);
    WriteData(0x00);

    WriteComm(0xA0);
    WriteData(0x58);

    WriteComm(0xA1);
    WriteData(0x00);

    WriteComm(0xA2);
    WriteData(0x03);

    WriteComm(0xA3);
    WriteData(0x03);

    WriteComm(0xA4);
    WriteData(0x0B);

    WriteComm(0xA5);
    WriteData(0x00);

    WriteComm(0xA6);
    WriteData(0x00);

    WriteComm(0xA7);
    WriteData(0x00);

    WriteComm(0xA8);
    WriteData(0x58);

    WriteComm(0xA9);
    WriteData(0x00);

    WriteComm(0xAA);
    WriteData(0x05);

    WriteComm(0xAB);
    WriteData(0x03);

    WriteComm(0xAC);
    WriteData(0x0D);

    WriteComm(0xAD);
    WriteData(0x00);

    WriteComm(0xAE);
    WriteData(0x00);

    WriteComm(0xAF);
    WriteData(0x00);

    WriteComm(0xB0);
    WriteData(0x58);

    WriteComm(0xB1);
    WriteData(0x00);

    WriteComm(0xB2);
    WriteData(0x07);

    WriteComm(0xB3);
    WriteData(0x03);

    WriteComm(0xB4);
    WriteData(0x0F);

    WriteComm(0xB5);
    WriteData(0x00);

    WriteComm(0xB6);
    WriteData(0x00);

    WriteComm(0xB7);
    WriteData(0x00);

    WriteComm(0xB8);
    WriteData(0x58);

    WriteComm(0xB9);
    WriteData(0x00);

    WriteComm(0xBA);
    WriteData(0x09);

    WriteComm(0xBB);
    WriteData(0x03);

    WriteComm(0xBC);
    WriteData(0x11);

    WriteComm(0xBD);
    WriteData(0x00);

    WriteComm(0xBE);
    WriteData(0x00);

    WriteComm(0xBF);
    WriteData(0x00);

    WriteComm(0xC0);
    WriteData(0x77);

    WriteComm(0xC1);
    WriteData(0x46);

    WriteComm(0xC2);
    WriteData(0x55);

    WriteComm(0xC3);
    WriteData(0x64);

    WriteComm(0xC4);
    WriteData(0xBB);

    WriteComm(0xC5);
    WriteData(0x21);

    WriteComm(0xC6);
    WriteData(0x30);

    WriteComm(0xC7);
    WriteData(0xAA);

    WriteComm(0xC8);
    WriteData(0x12);

    WriteComm(0xC9);
    WriteData(0x03);

    WriteComm(0xD0);
    WriteData(0x77);

    WriteComm(0xD1);
    WriteData(0x46);

    WriteComm(0xD2);
    WriteData(0x55);

    WriteComm(0xD3);
    WriteData(0x64);

    WriteComm(0xD4);
    WriteData(0xBB);

    WriteComm(0xD5);
    WriteData(0x21);

    WriteComm(0xD6);
    WriteData(0x30);

    WriteComm(0xD7);
    WriteData(0xAA);

    WriteComm(0xD8);
    WriteData(0x12);

    WriteComm(0xD9);
    WriteData(0x03);

    WriteComm(0xF3);
    WriteData(0x01);

    WriteComm(0xF0);
    WriteData(0x00);

    WriteComm(0xF0);
    WriteData(0x01);

    WriteComm(0xF1);
    WriteData(0x01);

    WriteComm(0xA0);
    WriteData(0x0B);

    WriteComm(0xA3);
    WriteData(0x2A);

    WriteComm(0xA5);
    WriteData(0xC3);
    delay_ms(10);

    WriteComm(0xA3);
    WriteData(0x2B);

    WriteComm(0xA5);
    WriteData(0xC3);
    delay_ms(10);

    WriteComm(0xA3);
    WriteData(0x2C);

    WriteComm(0xA5);
    WriteData(0xC3);
    delay_ms(10);

    WriteComm(0xA3);
    WriteData(0x2D);

    WriteComm(0xA5);
    WriteData(0xC3);
    delay_ms(10);

    WriteComm(0xA3);
    WriteData(0x2E);

    WriteComm(0xA5);
    WriteData(0xC3);
    delay_ms(10);

    WriteComm(0xA3);
    WriteData(0x2F);

    WriteComm(0xA5);
    WriteData(0xC3);
    delay_ms(10);

    WriteComm(0xA3);
    WriteData(0x30);

    WriteComm(0xA5);
    WriteData(0xC3);
    delay_ms(10);

    WriteComm(0xA3);
    WriteData(0x31);

    WriteComm(0xA5);
    WriteData(0xC3);
    delay_ms(10);

    WriteComm(0xA3);
    WriteData(0x32);

    WriteComm(0xA5);
    WriteData(0xC3);
    delay_ms(10);

    WriteComm(0xA3);
    WriteData(0x33);

    WriteComm(0xA5);
    WriteData(0xC3);
    delay_ms(10);
    WriteComm(0xA0);
    WriteData(0x09);

    WriteComm(0xF1);
    WriteData(0x10);

    WriteComm(0xF0);
    WriteData(0x00);

    WriteComm(0x2A);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x01);
    WriteData(0x3F);

    WriteComm(0x2B);
    WriteData(0x01);
    WriteData(0x81);
    WriteData(0x01);
    WriteData(0x81);

    WriteComm(0x4D);
    WriteData(0x00);

    WriteComm(0x4E);
    WriteData(0x00);

    WriteComm(0x4F);
    WriteData(0x00);

    WriteComm(0x4C);
    WriteData(0x01);
    delay_ms(10);

    WriteComm(0x4C);
    WriteData(0x00);

    WriteComm(0x2A);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x01);
    WriteData(0x3F);

    WriteComm(0x2B);
    WriteData(0x00);
    WriteData(0x00);
    WriteData(0x01);
    WriteData(0x80);

    WriteComm(0x21);

    WriteComm(0x35);
    WriteData(0x00);

    WriteComm(0x3A);

    WriteData(0x55);

    WriteComm(0x11);

	delay_ms(120);
	WriteComm(0x29);

    CommEnd();
}
#endif

