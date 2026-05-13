#include "include.h"
// 240 * 296

//#define Delay(ms) bsp_spide_cs(1);delay_ms(ms)
#if (GUI_SELECT == GUI_TFT_240_ST7789)

#define WriteData(v) tft_write_data(v)
#define CommEnd()    tft_write_end()

//0x02: 1CMD 1ADDR 1DATA
AT(.com_text.tft_spi)
void tft_write_cmd(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    TFT_SPI_CMD_EN();
    DESPICON &= ~BIT(3);                        //1BIT
    tft_spi_sendbyte(cmd);
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
    DESPICON |= BIT(3);                        //1BIT
    tft_spi_sendbyte(0x03);
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

AT(.com_text.tft_spi)
void tft_write_data(u8 data)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    TFT_SPI_DATA_EN();
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

    // tft_write_cmd(0x2C);
    tft_write_end();
}

AT(.com_text.tft_spi)
void tft_write_data_start(void)
{
    tft_write_cmd(0x2C);      //TFT_RAMWR
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


void st7789_display_point(uint16_t x1, uint16_t y1, uint16_t color)
{
    TFT_SPI_DATA_EN();
    delay_5ms(200);
    printf("st7789_display_point_test_nomal\n");
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

void HW_Reset(void)
{

    GPIOAFEN &= ~BIT(3);                            //RS
    GPIOADE  |= BIT(3);
    GPIOASET = BIT(3);
    GPIOADIR &= ~BIT(3);

    /*GPIOEFEN &= ~BIT(7);                            //RESET
    GPIOEDE  |= BIT(7);
    GPIOESET = BIT(7);
    GPIOEDIR &= ~BIT(7);
    delay_ms(10);
    GPIOECLR = BIT(7);
    delay_ms(20);
    GPIOESET = BIT(7);
    delay_ms(50);*/
}

void tft_240_st7789_init(void)
{
    printf("tft_st7789_init\n");

    HW_Reset();

    delay_ms(120);

    WriteComm(0x11);

    delay_ms(120);

    WriteComm(0x36);
    WriteData(0x00);

    WriteComm(0x3A);
    WriteData(0x05);

    WriteComm(0x35);
    WriteData(0x00);

    WriteComm(0xB2);
    WriteData(0x0C);
    WriteData(0x0C);
    WriteData(0x00);
    WriteData(0x33);
    WriteData(0x33);

    WriteComm(0xB7);
    WriteData(0x65);

    WriteComm(0xBB);
    WriteData(0x29);

    WriteComm(0xC0);
    WriteData(0x2C);

    WriteComm(0xC2);
    WriteData(0x01);

    WriteComm(0xC3);
    WriteData(0x18);

    WriteComm(0xC6);
    WriteData(0x0F);

    WriteComm(0xD0);
    WriteData(0xA7);

    WriteComm(0xD0);
    WriteData(0xA4);
    WriteData(0xA1);

    WriteComm(0xD6);
    WriteData(0xA1);   //sleep in后，gate输出为GND

    WriteComm(0xE0);
    WriteData(0xF0);
    WriteData(0x03);
    WriteData(0x08);
    WriteData(0x06);
    WriteData(0x06);
    WriteData(0x04);
    WriteData(0x2E);
    WriteData(0x44);
    WriteData(0x45);
    WriteData(0x39);
    WriteData(0x16);
    WriteData(0x16);
    WriteData(0x2C);
    WriteData(0x31);

    WriteComm(0xE1);
    WriteData(0xF0);
    WriteData(0x06);
    WriteData(0x0B);
    WriteData(0x0C);
    WriteData(0x0B);
    WriteData(0x17);
    WriteData(0x2E);
    WriteData(0x43);
    WriteData(0x44);
    WriteData(0x3A);
    WriteData(0x14);
    WriteData(0x14);
    WriteData(0x2A);
    WriteData(0x32);

    WriteComm(0x21);

    WriteComm(0x29);

    WriteComm(0x2C);

}
#endif

