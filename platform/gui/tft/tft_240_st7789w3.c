#include "include.h"

//240*296
#if (GUI_SELECT == GUI_TFT_240_ST7789W3)


#define WriteData(v) tft_write_data(v)
#define CommEnd()    tft_write_end()

void tft_write_cmd(u8 cmd);
void tft_write_data(u8 data);
void tft_write_end();

AT(.com_text.tft_spi)
void tft_write_end()
{
    TFT_SPI_CS_DIS();
}

AT(.com_text.tft_spi)
void tft_write_cmd(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();

    // BIT 3:2   0: 3wire mode 1 wire in 1 wrie out  1: 2wire mode 1wire in/out  2: 2bit bidirectional 3:4 bit bidirecional
#if MODE_3WIRE_9BIT
    DESPICON &= ~(3<<2);
    DESPICON |= BIT(18);                        //1BIT 3wire control singnal
    DESPICON &= ~BIT(26);
    DESPICON &= ~BIT(19);                       // DC为 0
#elif MODE_4WIRE_8BIT
    DC_CMD_EN();
    DESPICON &= ~(3<<2);
#endif
    tft_spi_sendbyte(cmd);
}

AT(.com_text.tft_spi)
void tft_write_data(u8 data)
{
    TFT_SPI_CS_EN();
#if MODE_3WIRE_9BIT
    DESPICON |= BIT(19);
#elif MODE_4WIRE_8BIT
    DC_DATA_EN();
#endif
    tft_spi_sendbyte(data);

    TFT_SPI_CS_DIS();
}

//0x03: 1CMD 1ADDR 1DATA
AT(.com_text.tft_spi)
void tft_read_cmd(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
#if MODE_3WIRE_9BIT
    DESPICON &= ~BIT(3);                        //1BIT
#elif MODE_4WIRE_8BIT
    DESPICON &= ~BIT(3);                        //1BIT
#endif
    tft_spi_sendbyte(0x03);
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
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
    tft_write_data(BYTE0(x0)&0XFF);
    tft_write_data(BYTE1(x1));
    tft_write_data(BYTE0(x1)&0XFF);

    tft_write_cmd(0x2B);        //TFT_PASET
    tft_write_data(BYTE1(y0));
    tft_write_data(BYTE0(y0)&0XFF);
    tft_write_data(BYTE1(y1));
    tft_write_data(BYTE0(y1)&0XFF);
    tft_write_end();
}

AT(.com_text.tft_spi)
void tft_write_data_start(void)
{
    tft_write_cmd(0x2C);
#if MODE_3WIRE_9BIT
    DESPICON |= (2<<2);
    //DESPICON &= ~(3<<2);
    DESPICON |= BIT(18);
    DESPICON |= BIT(26);
    DESPICON |= BIT(19);
#elif MODE_4WIRE_8BIT
    DC_DATA_EN();
    DESPICON &= ~(3<<2);
#endif
}

void st7789w3_display_point(uint16_t x1, uint16_t y1, uint16_t color)
{
//    TFT_SPI_DATA_EN();
    delay_5ms(200);
    printf("st7789w3_display_point_test_nomal\n");
    tft_set_window(0, 0, x1, y1);
    tft_frame_start();
    for (int x = 0;x <= x1; x++){
        for (int y = 0; y <= y1; y++){
            WDT_CLR();
            WriteData(color >> 8);
            WriteData(color);
        }
    }
    tft_write_end();

}

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


#define WriteComm(v) tft_write_cmd(v)

void tft_240_st7789w3_init(void)
{
    printf("tft_240_st7789w3_init\n");
   //HW_Reset();
   delay_ms(120);
   WriteComm(0x11);
   delay_ms(120);

   WriteComm(0x36);
   WriteData(0x00);

   WriteComm(0x3A);
   WriteData(0x05);

   WriteComm(0xB2);
   WriteData(0x0C);
   WriteData(0x0C);
   WriteData(0x00);
   WriteData(0x33);
   WriteData(0x33);

//  WriteComm(0xB0);
//  WriteData(0x00);
//  WriteData(0xE0);

   WriteComm(0xB7);
   WriteData(0x14);

   WriteComm(0xBB);
   WriteData(0x17);

   WriteComm(0xC0);
   WriteData(0x2C);

   WriteComm(0xC2);
   WriteData(0x01);

   WriteComm(0xC3);
   WriteData(0x05);

   WriteComm(0xC6);
   WriteData(0x0F);

   WriteComm(0xD0);
   WriteData(0xA7);

   WriteComm(0xD0);
   WriteData(0xA7);
   WriteData(0xA1);

   WriteComm(0xD6);
   WriteData(0xA1);   //sleep in后，gate输出为GND

   WriteComm(0xE0);
   WriteData(0x0F);
   WriteData(0x0E);
   WriteData(0x10);
   WriteData(0x01);
   WriteData(0x01);
   WriteData(0x00);
   WriteData(0x25);
   WriteData(0x33);
   WriteData(0x3C);
   WriteData(0x3B);
   WriteData(0x19);
   WriteData(0x17);
   WriteData(0x26);
   WriteData(0x27);

   WriteComm(0xE1);
   WriteData(0x0F);
   WriteData(0x10);
   WriteData(0x12);
   WriteData(0x07);
   WriteData(0x04);
   WriteData(0x00);
   WriteData(0x25);
   WriteData(0x32);
   WriteData(0x3B);
   WriteData(0x3C);
   WriteData(0x16);
   WriteData(0x18);
   WriteData(0x24);
   WriteData(0x28);

//   WriteComm(0xE4);
//   WriteData(0x26);
//   WriteData(0x00);   //设定gate起点位置
//  WriteData(0x00);   //当gate没有用完时，bit4(TMG)设为0

   WriteComm(0xE7);
   WriteData(0x10);  //2 data

   WriteComm(0x35);
   WriteData(0x00);

   WriteComm(0x44);
   WriteData(0x00);
   WriteData(0x33);

   WriteComm(0x21);

   WriteComm(0x29);

   WriteComm(0x2C);
}

#endif

