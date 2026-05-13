#include "include.h"
// 320 * 385

//#define Delay(ms) bsp_spide_cs(1);delay_ms(ms)

#if (GUI_SELECT == GUI_TFT_JD9853)


#define WriteData(v) tft_write_data(v)
#define CommEnd()    tft_write_end()

void tft_write_cmd(u8 cmd);
void tft_write_data(u8 data);
void tft_write_end(void);

AT(.com_text.tft_spi)
void tft_write_end(void)
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
#if !MODE_4WIRE_8BIT
    DESPICON &= ~(3<<2);
    DESPICON |= BIT(18);                        //1BIT 3wire control singnal
    DESPICON &= ~BIT(26);
    DESPICON &= ~BIT(19);                       // DC为 0
#elif MODE_4WIRE_8BIT
    DC_CMD_EN();
    DESPICON &= ~(3<<2);
#endif
    tft_spi_sendbyte(cmd);
    // delay_ms(1); //9853需要延时
    //printf("DESPICON: %x cmd: %x \n", DESPICON, cmd);
}

AT(.com_text.tft_spi)
void tft_write_data(u8 data)
{
    TFT_SPI_CS_EN();
#if !MODE_4WIRE_8BIT
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
    tft_write_cmd(cmd);
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
#if !MODE_4WIRE_8BIT
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


void tft_240_jd9853_init(void)
{
    printf("tft_240_jd9853_init\n");

   //SET_PASSWD
   WriteComm(0xDF);
   WriteData(0x98);
   WriteData(0x53);

   //VGPOW_SET
   WriteComm(0xB2);
   WriteData(0x23);

   //GAMMA_SET
   WriteComm(0xB7);
   WriteData(0x00);
   WriteData(0x21);
   WriteData(0x00);
   WriteData(0x49);

   //DCDC_SEL
   WriteComm(0xBB);
   WriteData(0x1C);
   WriteData(0x9A);
   WriteData(0x55);
   WriteData(0x73);
   WriteData(0x6F);
   WriteData(0xF0);

   //SETSTBA
   WriteComm(0xC0);
   WriteData(0x22);
   WriteData(0x22);

   //SETPANEL
   WriteComm(0xC1);
   WriteData(0x12);

   //SETRGBCYC
   WriteComm(0xC3);
   WriteData(0x7D);
   WriteData(0x07);
   WriteData(0x14);
   WriteData(0x06);
   WriteData(0xC8);
   WriteData(0x71);
   WriteData(0x6C);
   WriteData(0x77);

   //SET_TCON
   WriteComm(0xC4);
   WriteData(0x00); // 00 60hz  01 50hz 02 42hz  粗调
   WriteData(0x00);
   WriteData(0x94);
   WriteData(0x79);
   WriteData(0x25);   // 每加大一个，减少0.17hz
   WriteData(0x0A);
   WriteData(0x16);
   WriteData(0x79);
   WriteData(0x25);
   WriteData(0x0A);
   WriteData(0x16);
   WriteData(0x82);

   //SET_R_GAMMA
   WriteComm(0xC8);
   WriteData(0x3F);
   WriteData(0x35);
   WriteData(0x2E);
   WriteData(0x29);
   WriteData(0x2C);
   WriteData(0x2D);
   WriteData(0x27);
   WriteData(0x27);
   WriteData(0x24);
   WriteData(0x24);
   WriteData(0x21);
   WriteData(0x15);
   WriteData(0x11);
   WriteData(0x0B);
   WriteData(0x08);
   WriteData(0x00);
   WriteData(0x3F);
   WriteData(0x35);
   WriteData(0x2E);
   WriteData(0x29);
   WriteData(0x2C);
   WriteData(0x2D);
   WriteData(0x27);
   WriteData(0x27);
   WriteData(0x24);
   WriteData(0x24);
   WriteData(0x21);
   WriteData(0x15);
   WriteData(0x11);
   WriteData(0x0B);
   WriteData(0x08);
   WriteData(0x00);

   //SET_GD
   WriteComm(0xD0);
   WriteData(0x04);
   WriteData(0x06);
   WriteData(0x6B);
   WriteData(0x0F);
   WriteData(0x00);

   //RAMCTRL
   WriteComm(0xD7);
   WriteData(0x30);
   WriteData(0x30);

   //
   WriteComm(0xE6);
   WriteData(0x04);

   //page1
   WriteComm(0xDE);
   WriteData(0x01);

   //DCDC_OPT
   WriteComm(0xB7);
   WriteData(0x03);
   WriteData(0x13);
   WriteData(0xEF);
   WriteData(0x35);
   WriteData(0x35);

   //SETRGBCYC2
   WriteComm(0xC1);
   WriteData(0x10);
   WriteData(0x15);
   WriteData(0xC0);

   //
   WriteComm(0xC2);
   WriteData(0x06);
   WriteData(0x3A);
   WriteData(0xE7);

   //SET_GAMMAOP
   WriteComm(0xC4);
   WriteData(0x72);
   WriteData(0x12);

   //GAMMA_POWER_TEST
   WriteComm(0xBE);
   WriteData(0x00);

   //page0
   WriteComm(0xDE);
   WriteData(0x00);

   WriteComm(0x35);
   WriteData(0x00);

   WriteComm(0x3A);
   WriteData(0x05);

   WriteComm(0x2A);
   WriteData(0x00);
   WriteData(0x00);
   WriteData(0x00);
   WriteData(0xEF);

   WriteComm(0x2B);
   WriteData(0x00);
   WriteData(0x00);
   WriteData(0x01);
   WriteData(0x27);


   WriteComm(0x11);
   CommEnd();
   delay_ms(120);
   WriteComm(0x29);
   delay_ms(50);
   CommEnd();


}

#endif
