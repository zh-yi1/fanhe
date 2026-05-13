#include "include.h"
// 360 * 360

//#define Delay(ms) bsp_spide_cs(1);delay_ms(ms)
#if (GUI_SELECT == GUI_TFT_360_GC9C01)


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
void tft_write_end(void)
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


void tft_360_gc9c01_init(void)
{
    printf("tft_gc9c01_init\n");
 	 WriteComm(0xfe);// internal reg enable
	WriteComm(0xef);// internal reg enable
	WriteComm(0x80);
	WriteData(0x11);
	WriteComm(0x81);
	WriteData(0x70);
	WriteComm(0x82);
	WriteData(0x09);
	WriteComm(0x83);
	WriteData(0x03);
	WriteComm(0x84);
	WriteData(0x20);
	WriteComm(0x89);
	WriteData(0x10);
	WriteComm(0x8A);
	WriteData(0x4f);
	WriteComm(0x8C);
	WriteData(0x59);
	WriteComm(0x8D);
	WriteData(0x51);

	WriteComm(0xA9);            // Unknown register, Denoise
	WriteData(0x88);

	WriteComm(0x36);
	WriteData(0x00);

	WriteComm(0x3a);
	WriteData(0x55);            //COLMOD: RGB565

  	WriteComm(0xEC);
	WriteData(0x77);

  	WriteComm(0x74);
	WriteData(0x01);
	WriteData(0x80);
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x00);

	WriteComm(0x98);
	WriteData(0x3E);
	WriteComm(0x99);
	WriteData(0x3E);

	WriteComm(0xC3);
	WriteData(0x3A);
	WriteComm(0xC4);
	WriteData(0x16);

	WriteComm(0xA1);
	WriteData(0x01);
	WriteData(0x04);

	WriteComm(0xA2);
	WriteData(0x01);
	WriteData(0x04);

	WriteComm(0xCB);
	WriteData(0x02);
	WriteComm(0x7C);
	WriteData(0xB6);
	WriteData(0x26);
	WriteComm(0xAC);
	WriteData(0x24);

	WriteComm(0xF6);
	WriteData(0x80);

	WriteComm(0xB5);
	WriteData(0x09);
	WriteData(0x09);

  	WriteComm(0x60);
	WriteData(0x38);
	WriteData(0x0B);
	WriteData(0x5B);
	WriteData(0x56);

  	WriteComm(0x63);
	WriteData(0x3A);
	WriteData(0xE0);
	WriteData(0x5B);
	WriteData(0x56);

 	 WriteComm(0x64);
	WriteData(0x38);
	WriteData(0x0D);
	WriteData(0x72);
	WriteData(0xDD);
	WriteData(0x5B);
	WriteData(0x56);

  	WriteComm(0x66);
	WriteData(0x38);
	WriteData(0x11);
	WriteData(0x72);
	WriteData(0xE1);
	WriteData(0x5B);
	WriteData(0x56);

  	WriteComm(0x68);
	WriteData(0x3B);
	WriteData(0x08);
	WriteData(0x08);
	WriteData(0x00);
	WriteData(0x08);
	WriteData(0x29);
	WriteData(0x5B);

WriteComm(0x6E);
WriteData(0x00);
WriteData(0x00);
WriteData(0x00);
WriteData(0x07);
WriteData(0x01);
WriteData(0x13);
WriteData(0x11);
WriteData(0x0B);
WriteData(0x09);
WriteData(0x16);
WriteData(0x15);
WriteData(0x1D);
WriteData(0x1E);
WriteData(0x00);
WriteData(0x00);
WriteData(0x00);
WriteData(0x00);
WriteData(0x00);
WriteData(0x00);
WriteData(0x1E);
WriteData(0x1D);
WriteData(0x15);
WriteData(0x16);
WriteData(0x0A);
WriteData(0x0C);
WriteData(0x12);
WriteData(0x14);
WriteData(0x02);
WriteData(0x08);
WriteData(0x00);
WriteData(0x00);
WriteData(0x00);

WriteComm(0xBE);
WriteData(0x11);

WriteComm(0x6C);
WriteData(0xCC);
WriteData(0x0C);
WriteData(0xCC);
WriteData(0x84);
WriteData(0xCC);
WriteData(0x04);
WriteData(0x50);

WriteComm(0x7D);
WriteData(0x72);

WriteComm(0x7E);
WriteData(0x38);

WriteComm(0x70);
WriteData(0x02);
WriteData(0x03);
WriteData(0x09);
WriteData(0x05);
WriteData(0x0C);
WriteData(0x06);
WriteData(0x09);
WriteData(0x05);
WriteData(0x0C);
WriteData(0x06);

WriteComm(0x90);
WriteData(0x06);
WriteData(0x06);
WriteData(0x05);
WriteData(0x06);

WriteComm(0x93);
WriteData(0x45);
WriteData(0xFF);
WriteData(0x00);

WriteComm(0xF0);
WriteData(0x46);
WriteData(0x0B);
WriteData(0x0F);
WriteData(0x0A);
WriteData(0x10);
WriteData(0x3F);

WriteComm(0xF1);
WriteData(0x52);
WriteData(0x9A);
WriteData(0x4F);
WriteData(0x36);
WriteData(0x37);
WriteData(0xFF);

WriteComm(0xF2);
WriteData(0x46);
WriteData(0x0B);
WriteData(0x0F);
WriteData(0x0A);
WriteData(0x10);
WriteData(0x3F);

WriteComm(0xF3);
WriteData(0x52);
WriteData(0x9A);
WriteData(0x4F);
WriteData(0x36);
WriteData(0x37);
WriteData(0xFF);

WriteComm(0x35);        //TE On
WriteData(0x00);

	WriteComm(0x11);

CommEnd();
	delay_ms(120);
	WriteComm(0x29);
//  	WriteComm(0x2C);

CommEnd();
}

#endif
