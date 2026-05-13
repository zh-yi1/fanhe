#include "include.h"

#if FLASH_EXTERNAL_EN

#define SPI1_TX()               SPI1CON &= ~BIT(4)
#define SPI1_RX()               SPI1CON |= BIT(4);  delay_us(5)


struct _type_spi_cb {
    u8 read_cmd[12];
    u8 read_cmd_cnt;
    u8 sf_read;
    u8 dummy;
    bool mode4io;
} spi1_cb;


AT(.com_text.sys)
void bsp_spi1flash_lock(void)
{
    spin_lock(SPIN_SPI1FLASH);
}

AT(.com_text.sys)
void bsp_spi1flash_unlock(void)
{
    spin_unlock(SPIN_SPI1FLASH);
}

//SPIFlash Delay
AT(.com_text.spi1flash)
void spi1_delay(void)
{
    uint cnt = 20;
    while (cnt--) {
        asm("nop");
    }
}

//SPI接口获取1Byte数据
AT(.com_text.spi1flash)
u8 spi1_getbyte(void)
{
    SPI1CON |= BIT(4);                              //RX
    SPI1BUF = 0xff;
    while (!(SPI1CON & BIT(16)));                   //Wait pending
    return SPI1BUF;
}

//SPI接口发送1Byte数据
AT(.com_text.spi1flash)
void spi1_sendbyte(u8 data)
{
    SPI1CON &= ~BIT(4);                             //TX
    SPI1BUF = data;
    while (!(SPI1CON & BIT(16)));                   //Wait pending
}

///SPIFlash写使能
AT(.com_text.spiflash)
void spi1flash_write_enable(void)
{
    SPI1_CS_EN();
    spi1_sendbyte(SF_WRITE_EN);
    SPI1_CS_DIS();
    spi1_delay();
}

//SPIFlash读状态寄存器
AT(.com_text.spiflash)
uint spi1flash_readssr(void)
{
    uint ssr = 0;
    SPI1_CS_EN();
    spi1_delay();
    spi1_sendbyte(SF_READSSR_H);
    ssr = spi1_getbyte() << 8;
    SPI1_CS_DIS();
    spi1_delay();

    SPI1_CS_EN();
    spi1_delay();
    spi1_sendbyte(SF_READSSR_L);
    ssr |= spi1_getbyte();
    SPI1_CS_DIS();
    spi1_delay();

    return ssr;
}

//SPIFlash等待BUSY
AT(.com_text.spi1flash)
void spi1flash_waitbusy(void)
{
    do {
        spi1_delay();
    } while (spi1flash_readssr() & 0x01);
}

//SPIFlash写状态寄存器
AT(.com_text.spi1flash)
void spi1flash_writessr(u16 ssr)
{
    spi1flash_write_enable();
    SPI1_CS_EN();
    spi1_sendbyte(SF_WRITESSR);
    spi1_sendbyte(BYTE0(ssr));
    spi1_sendbyte(BYTE1(ssr));
    SPI1_CS_DIS();
    spi1flash_waitbusy();
}

//发送SPIFlash的地址
AT(.com_text.spi1flash)
void spi1flash_sendaddr(u32 addr)
{
    spi1_sendbyte(addr >> 16);
    spi1_sendbyte(addr >> 8);
    spi1_sendbyte(addr);
}

//初始化SPIFlash接口
AT(.text.spi1flash)
void spi1flash_init(u8 sf_read, u8 dummy, bool mode4io)
{
    memset(spi1_cb.read_cmd, 0xff, sizeof(spi1_cb.read_cmd));
    spi1_cb.sf_read = sf_read;
    spi1_cb.dummy = dummy;
    spi1_cb.mode4io = mode4io;
    if (mode4io) {
        spi1_cb.read_cmd[0] = ((sf_read & BIT(7)) >> 3) | ((sf_read & BIT(6)) >> 6);
        spi1_cb.read_cmd[1] = ((sf_read & BIT(5)) >> 1) | ((sf_read & BIT(4)) >> 4);
        spi1_cb.read_cmd[2] = ((sf_read & BIT(3)) << 1) | ((sf_read & BIT(2)) >> 2);
        spi1_cb.read_cmd[3] = ((sf_read & BIT(1)) << 3) | (sf_read & BIT(0));
        spi1_cb.read_cmd_cnt = 8 + dummy;
    } else {
        spi1_cb.read_cmd[0] = sf_read;
        spi1_cb.read_cmd_cnt = 4 + dummy;
    }
}

//SPIFlash编程
AT(.com_text.spi1flash)
void spi1flash_program_do(void *buf, u32 addr, uint len)
{
    bsp_spi1flash_lock();
    spi1flash_write_enable();

    SPI1_CS_EN();
    spi1_sendbyte(SF_PROGRAM);
    spi1flash_sendaddr(addr);

    SPI1CON &= ~BIT(4);                             //TX
    SPI1DMAADR = DMA_ADR(buf);
    SPI1DMACNT = len;
    while (!(SPI1CON & BIT(16)));                   //Wait pending
    SPI1_CS_DIS();
    spi1flash_waitbusy();
    bsp_spi1flash_unlock();
}

AT(.com_text.spi1flash)
void spi1flash_program(void *buf, u32 addr, uint len)
{
    if (NULL == buf || 0 == len) {
        return ;
    }
    uint32_t page_remain;

    page_remain = 256 - addr%256;

    if(len <= page_remain)
    {
        page_remain = len;
    }
    while(1)
    {
        WDT_CLR();
        if (0 == page_remain) {
            break;
        }
        spi1flash_program_do(buf,addr,page_remain);

        if(len == page_remain)
            break;
         else
        {
             buf += page_remain;
             addr += page_remain;

             len -= page_remain;
            if(len > 256)
                page_remain = 256;
            else
                page_remain = len;
        }
    }
}

//SPIFlash Read Kick
AT(.com_text.spiflash)
bool spi1flash_read_kick(void *buf, u32 addr, uint len)
{
    bsp_spi1flash_lock();

    SPI1_CS_EN();
    if (spi1_cb.mode4io) {
        SPI1CON |= BIT(9);                          //MultBits
        spi1_cb.read_cmd[4] = (u8)(addr >> 16);
        spi1_cb.read_cmd[5] = (u8)(addr >> 8);
        spi1_cb.read_cmd[6] = (u8)(addr);
    } else {
        spi1_cb.read_cmd[1] = (u8)(addr >> 16);
        spi1_cb.read_cmd[2] = (u8)(addr >> 8);
        spi1_cb.read_cmd[3] = (u8)(addr);
    }
    SPI1CON &= ~BIT(4);                             //TX
    SPI1DMAADR = DMA_ADR(spi1_cb.read_cmd);
    SPI1DMACNT = spi1_cb.read_cmd_cnt;
    while (!(SPI1CON & BIT(16)));                   //Wait pending

    SPI1CON |= BIT(4);                     //MultBits, RX
    SPI1DMAADR = DMA_ADR(buf);
    SPI1DMACNT = len;
    return true;
}

//SPIFlash Read Wait
AT(.com_text.spiflash)
bool spi1flash_read_wait(void)
{
    while (!(SPI1CON & BIT(16)));                   //Wait pending
    SPI1CON &= ~BIT(9);                             //OneBit
    SPI1_CS_DIS();

    bsp_spi1flash_unlock();

    return true;
}

//SPIFlash读取
AT(.com_text.spiflash)
bool spi1flash_read(void *buf, u32 addr, uint len)
{
    spi1flash_read_kick(buf, addr, len);
    spi1flash_read_wait();
    return true;
}

//SPIFlash擦除
AT(.com_text.spi1flash)
void spi1flash_erase(u32 addr)
{
    bsp_spi1flash_lock();

    spi1flash_write_enable();
    SPI1_CS_EN();
    spi1_sendbyte(SF_ERASE);
    spi1flash_sendaddr(addr);
    SPI1_CS_DIS();
    spi1flash_waitbusy();

    bsp_spi1flash_unlock();
}

//SPIFlash 32K擦除
AT(.com_text.spi1flash)
void spi1flash_erase32k(u32 addr)
{
    bsp_spi1flash_lock();
    spi1flash_write_enable();

    SPI1_CS_EN();
    spi1_sendbyte(SF_32KERASE);
    spi1flash_sendaddr(addr);
    SPI1_CS_DIS();

    spi1flash_waitbusy();
    bsp_spi1flash_unlock();
}

//SPIFlash 64K擦除
AT(.com_text.spi1flash)
void spi1flash_erase64k(u32 addr)
{
    bsp_spi1flash_lock();
    spi1flash_write_enable();

    SPI1_CS_EN();
    spi1_sendbyte(SF_64KERASE);
    spi1flash_sendaddr(addr);
    SPI1_CS_DIS();

    spi1flash_waitbusy();
    bsp_spi1flash_unlock();
}

AT(.text.spi1flash)
u32 spi1flash_id_read(void)
{
    u32 id = 0, i;

    bsp_spi1flash_lock();
    SPI1_CS_EN();
    spi1_sendbyte(SF_READID);
    for (i = 0; i < 3; i++) {
        id = id << 8;
        id |= spi1_getbyte();
    }
    SPI1_CS_DIS();
    bsp_spi1flash_unlock();

    return id;
}

void bsp_spi1flash_init(void)
{
    gpio_t gpio;

    //CS
    bsp_gpio_cfg_init(&gpio, SPI1_CS_PORT);
    gpio.sfr[GPIOxFEN] &= ~BIT(gpio.num);
    gpio.sfr[GPIOxDE] |= BIT(gpio.num);
    gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
    gpio.sfr[GPIOxCLR] = BIT(gpio.num);

    //CLK D0 ~ D3
    uint8_t io_map[] = {SPI1_DO_IO0_PORT, SPI1_DI_IO1_PORT, SPI1_WP_IO2_PORT, SPI1_HOLD_IO3_PORT, SPI1_CLK_PORT};
    for (uint8_t i = 0; i < sizeof(io_map); i++) {
        bsp_gpio_cfg_init(&gpio, io_map[i]);
        gpio.sfr[GPIOxDE] |= BIT(gpio.num);
        gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
        if (io_map[i] == SPI1_CLK_PORT || io_map[i] == SPI1_DO_IO0_PORT) {
            gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
        } else {
            gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
        }
    }

    CLKGAT0 |= BIT(13);                                 //spi1_clken
    delay_us(10);
    FUNCMCON1 = 0xf0;
    FUNCMCON1 = SPI1_MAP_PORT;

    SPI1BAUD = 10;
    SPI1CON = 0;
    SPI1CON = BIT(0);
    spi1flash_writessr(0x02 << 8);                      //QE EN
    SPI1CON |= BIT(10);                                 //output data and sample data is at the same clock edge
    SPI1CON |= BIT(3) | BIT(2);                         //enable 4bit mode
    spi1flash_init(0xEB, 2, true);
    SPI1BAUD = 3;
}



#endif


