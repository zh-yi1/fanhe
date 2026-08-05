#include "include.h"
#include "tft.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

extern u32 __psram_vma;
static lcd_drv_t *lcd_drv_reg = NULL;
extern void tft_spi_sendbyte(u8 val);

void lcd_drv_dc_out(bool is_high);
void lcd_drv_cs_out(bool is_high);
#define TFT_SPI_CS_DIS()      {lcd_drv_cs_out(1);}
#define TFT_SPI_CS_EN()       {lcd_drv_cs_out(0);}
#define DC_CMD_EN()           {lcd_drv_dc_out(0);}      // DC 拉低
#define DC_DATA_EN()          {lcd_drv_dc_out(1);}      // DC 拉高

// cmd 12h 1:cmd      4:addr/data
// cmd 32h 1:cmd/addr 4:data
// cmd 02h 1:cmd      1:addr/data
#define WriteComm12(v) \
({                   \
TFT_SPI_CS_DIS();         \
delay_us(1);            \
TFT_SPI_CS_EN();         \
LCDSPICON &= ~BIT(3);   \
tft_spi_sendbyte(0x12);\
LCDSPICON |= BIT(3);   \
tft_spi_sendbyte(0x00);\
tft_spi_sendbyte(v);   \
tft_spi_sendbyte(0x00);\
})
#define WriteComm32(v) \
({                   \
TFT_SPI_CS_DIS();         \
delay_us(1);            \
TFT_SPI_CS_EN();         \
LCDSPICON &= ~BIT(3);   \
tft_spi_sendbyte(0x32);\
tft_spi_sendbyte(0x00);\
tft_spi_sendbyte(v);   \
tft_spi_sendbyte(0x00);\
LCDSPICON |= BIT(3);   \
})
#define WriteComm02(v) \
({                   \
TFT_SPI_CS_DIS();         \
delay_us(1);            \
TFT_SPI_CS_EN();         \
LCDSPICON &= ~BIT(3);   \
tft_spi_sendbyte(0x02);\
tft_spi_sendbyte(0x00);\
tft_spi_sendbyte(v);   \
tft_spi_sendbyte(0x00);\
})
#define ReadComm03(v) \
({                   \
TFT_SPI_CS_DIS();         \
delay_us(1);            \
LCDSPICON &= ~BIT(3);   \
tft_spi_sendbyte(0x03);\
tft_spi_sendbyte(0x00);\
tft_spi_sendbyte(v);   \
tft_spi_sendbyte(0x00);\
})




AT(.com_text.tft_spi)
void tft_write_end()
{
    if (lcd_drv_reg->io_type == LCD_IO_PRGB16
        || lcd_drv_reg->io_type == LCD_IO_SRGB8) {
        return;
    }

    TFT_SPI_CS_DIS();
#if MSPI_DDR_MODE_EN
    LCDSPICON &= ~BIT(27);
#endif
}

//0x02: 1CMD 1ADDR 1DATA
AT(.com_text.tft_spi)
void tft_write_cmd(u8 cmd)
{
    if (lcd_drv_reg->io_type == LCD_IO_PRGB16
        || lcd_drv_reg->io_type == LCD_IO_SRGB8) {
        return;
    }

    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    if (lcd_drv_reg->io_type == LCD_IO_3WIRE_9BIT) {
        LCDSPICON = BIT(18) | BIT(0);
        DC_CMD_EN();
        tft_spi_sendbyte(cmd);
    } else if(lcd_drv_reg->io_type == LCD_IO_3WIRE_9BIT_2LINE) {
        LCDSPICON = BIT(18) | BIT(0);
        DC_CMD_EN();
        tft_spi_sendbyte(cmd);
    } else if(lcd_drv_reg->io_type == LCD_IO_4WIRE_8BIT) {
        LCDSPICON = BIT(0);
        DC_CMD_EN();
        tft_spi_sendbyte(cmd);
    } else if(lcd_drv_reg->io_type == LCD_IO_QSPI) {
#if MSPI_DDR_MODE_EN
        LCDSPICON = (LCDSPICON & ~(0x3<<2)) | (0x0<<2);  //发送命令用一线
        LCDSPICON &= ~BIT(9);
#else
        LCDSPICON &= ~BIT(3);                        //1BIT
#endif // MSPI_DDR_MODE_EN
        tft_spi_sendbyte(0x02);
        tft_spi_sendbyte(0x00);
        tft_spi_sendbyte(cmd);
        tft_spi_sendbyte(0x00);
    } else if(lcd_drv_reg->io_type == LCD_IO_I8080) {
        DC_CMD_EN();
        tft_spi_sendbyte(cmd);
    }
}

//0x12: 1CMD 4ADDR 4DATA
AT(.com_text.tft_spi)
void tft_write_cmd12(u8 cmd)
{
    if (lcd_drv_reg->io_type == LCD_IO_PRGB16
        || lcd_drv_reg->io_type == LCD_IO_SRGB8) {
        return;
    }

    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON &= ~BIT(3);                        //1BIT
    tft_spi_sendbyte(0x12);
    LCDSPICON |= BIT(3);                         //4BIT
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

//0x32: CMD:8bit 1line sdr, ADDR: 24bit 4line sdr, DATA：nbit 4line sdr
AT(.com_text.tft_spi)
void tft_write_cmd32(u8 cmd)
{
    if (lcd_drv_reg->io_type == LCD_IO_PRGB16
        || lcd_drv_reg->io_type == LCD_IO_SRGB8) {
        return;
    }

    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON &= ~BIT(3);                       //1BIT
    tft_spi_sendbyte(0x32);                     //sdr模式命令
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
    LCDSPICON = (LCDSPICON & ~(0x3<<2)) | (0x3<<2) | BIT(9);  //4 line
}

//0x42: CMD:8bit 1line sdr, ADDR: 24bit 1line ddr, DATA：nbit 1line ddr
AT(.com_text.tft_spi)
void tft_write_cmd42(u8 cmd)
{
    if (lcd_drv_reg->io_type == LCD_IO_PRGB16
        || lcd_drv_reg->io_type == LCD_IO_SRGB8) {
        return;
    }

    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    //st7801n的1CMD为sdr模式，3addr支持mspi ddr
    LCDSPICON = (LCDSPICON & ~(0x3<<2)) | (0x0<<2);  //发送命令用一线
    LCDSPICON &= ~BIT(9);
    tft_spi_sendbyte(0x42);

    // st7801n addr需要ddr模式
    LCDSPICON |= BIT(27);       //ddr mode，开启ddr后，baud寄存器内部会自动配置为0，因此需要通进来前的时钟分频控制spi时钟
    //cpu手动发送
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

// 0x52: CMD:8bit 1line sdr, ADDR: 24bit 4line ddr, DATA：nbit 4line ddr
void tft_write_cmd52(u8 cmd)
{
    if (lcd_drv_reg->io_type == LCD_IO_PRGB16
        || lcd_drv_reg->io_type == LCD_IO_SRGB8) {
        return;
    }

    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    //st7801n的1CMD为sdr模式，3addr支持mspi ddr
    LCDSPICON = (LCDSPICON & ~(0x3<<2)) | (0x0<<2);  //发送命令用一线
    LCDSPICON &= ~BIT(9);
    tft_spi_sendbyte(0x52);

    // st7801n addr需要ddr模式
    LCDSPICON |= BIT(27);       //ddr mode，开启ddr后，baud寄存器内部会自动配置为0，因此需要通进来前的时钟分频控制spi时钟
    LCDSPICON = (LCDSPICON & ~(0x3<<2)) | (0x3<<2) | BIT(9);
    //cpu手动发送
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

//0x72: CMD:8bit 1line sdr, ADDR: 24bit 1line ddr, DATA：nbit 4line ddr
AT(.com_text.tft_spi)
void tft_write_cmd72(u8 cmd)
{
    if (lcd_drv_reg->io_type == LCD_IO_PRGB16
        || lcd_drv_reg->io_type == LCD_IO_SRGB8) {
        return;
    }

    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    //st7801n的1CMD为sdr模式，3addr支持mspi ddr
    LCDSPICON = (LCDSPICON & ~(0x3<<2)) | (0x0<<2);  //发送命令用一线
    LCDSPICON &= ~BIT(9);
    tft_spi_sendbyte(0x72);

    // st7801n addr需要ddr模式
    LCDSPICON |= BIT(27);       //ddr mode，开启ddr后，baud寄存器内部会自动配置为0，因此需要通进来前的时钟分频控制spi时钟
    //cpu手动发送
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
    //通过cmd寄存器自动发送，适用于cmd也是ddr的模式
    // LCDCON = (LCDCON | BIT(22));                    //cmd mode： 1 cmd + 3 addr
    // LCDCMD = (0x72 << 8) | (((u32)cmd) << 0);       //cmd模式设置命令和addr
    //dma发送命令
    // u32 addr_ = ((u32)cmd) << 8;
    // LCDCON = (LCDCON & ~(0x3<<6)) | (0x2<<6); //output format rgb888，为了发送24bit地址
    // LCDCON = (LCDCON & ~(0x3<<2)) | (0x1<<2); //input format rgb888，为了发送24bit地址
    // tft_spi_send(&addr_, 1, 1);              //dma发送
    // LCDCON = (LCDCON & ~(0x3<<6)) | (0<<6);  //output format rgb565
    // LCDCON = (LCDCON & ~(0x3<<2)) | (0<<2);  //input format rgb565

    //数据4线
    LCDSPICON = (LCDSPICON & ~(0x3<<2)) | (0x3<<2) | BIT(9);
}


AT(.com_text.tft_spi)
void tft_write_data(u8 data)
{
    if(lcd_drv_reg->io_type == LCD_IO_3WIRE_9BIT) {
        TFT_SPI_CS_EN();
        DC_DATA_EN();
        tft_spi_sendbyte(data);
        TFT_SPI_CS_DIS();
    } else if(lcd_drv_reg->io_type == LCD_IO_3WIRE_9BIT_2LINE) {
        TFT_SPI_CS_EN();
        DC_DATA_EN();
        tft_spi_sendbyte(data);
        TFT_SPI_CS_DIS();
    } else if(lcd_drv_reg->io_type == LCD_IO_4WIRE_8BIT) {
        TFT_SPI_CS_EN();
        DC_DATA_EN();
        tft_spi_sendbyte(data);
        TFT_SPI_CS_DIS();
    } else if(lcd_drv_reg->io_type == LCD_IO_QSPI){
        tft_spi_sendbyte(data);
    } else if(lcd_drv_reg->io_type == LCD_IO_I8080){
        DC_DATA_EN();
        tft_spi_sendbyte(data);
    }
}

//0x03: 1CMD 1ADDR 1DATA
AT(.com_text.tft_spi)
void tft_read_cmd(u8 cmd)
{
    if (lcd_drv_reg->io_type == LCD_IO_PRGB16
        || lcd_drv_reg->io_type == LCD_IO_SRGB8) {
        return;
    }

    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();

    if(lcd_drv_reg->io_type == LCD_IO_3WIRE_9BIT) {
        tft_spi_sendbyte(cmd);
    } else if(lcd_drv_reg->io_type == LCD_IO_3WIRE_9BIT_2LINE) {
        DC_CMD_EN();
        tft_spi_sendbyte(cmd);
    } else if(lcd_drv_reg->io_type == LCD_IO_4WIRE_8BIT) {
        DC_CMD_EN();
        tft_spi_sendbyte(cmd);
    } else if(lcd_drv_reg->io_type == LCD_IO_QSPI){
        LCDSPICON = (LCDSPICON & ~(3<<2)) | ((0x1)<<2); //[3:2]1BIT in/out
        tft_spi_sendbyte(0x03);
        tft_spi_sendbyte(0x00);
        tft_spi_sendbyte(cmd);
        tft_spi_sendbyte(0x00);
    } else if(lcd_drv_reg->io_type == LCD_IO_I8080){
        DC_CMD_EN();
        tft_spi_sendbyte(cmd);
    }
}


AT(.com_text.tft_spi)
void tft_write_data_start(void)
{
    if(lcd_drv_reg->io_type == LCD_IO_3WIRE_9BIT) {
        tft_write_cmd(0x2C);
        DC_DATA_EN();
        LCDSPICON |= BIT(26);
    } else if(lcd_drv_reg->io_type == LCD_IO_3WIRE_9BIT_2LINE) {
        tft_write_cmd(0x2C);
        DC_DATA_EN();
        //3w-9b 2line mode
        LCDSPICON |=  BIT(26) | BIT(9) | BIT(3); //[26]dual en, [9]MultiBit, [3:2]2BIT data bus
    } else if(lcd_drv_reg->io_type == LCD_IO_4WIRE_8BIT) {
        tft_write_cmd(0x2C);
        DC_DATA_EN();
//        LCDSPICON |=  BIT(26);
    } else if(lcd_drv_reg->io_type == LCD_IO_QSPI){
        #if MSPI_DDR_MODE_EN
            tft_write_cmd72(0x2C);
        #else
            tft_write_cmd32(0x2C);      //TFT_RAMWR
        #endif
    } else if(lcd_drv_reg->io_type == LCD_IO_I8080){
        tft_write_cmd(0x2C);        //TFT_RAMWR
        DC_DATA_EN();
    }
}

/**
 * lcd CS
 */
AT(.com_text.tft_spi)
void lcd_drv_cs_out(bool is_high)
{
    if (lcd_drv_reg->io_type == LCD_IO_PRGB16
        || lcd_drv_reg->io_type == LCD_IO_SRGB8) {
        return;
    }
    gpio_t gpio;
    gpio_cfg_init(&gpio, lcd_drv_reg->io.spi_drv_io.control.cio.cs);
    if (gpio.sfr == NULL) {
        return;
    }

    if(is_high){
        gpio.sfr[GPIOxSET] = BIT(gpio.num);
    }else{
        gpio.sfr[GPIOxCLR] = BIT(gpio.num);
    }
}


/**
 * lcd DC
 */
AT(.com_text.tft_spi)
void lcd_drv_dc_out(bool is_high)
{
    if (lcd_drv_reg->io_type == LCD_IO_PRGB16
        || lcd_drv_reg->io_type == LCD_IO_SRGB8) {
        return;
    }

    gpio_t gpio;
    gpio_cfg_init(&gpio, lcd_drv_reg->io.spi_drv_io.control.cio.dc);
    if (gpio.sfr == NULL) {
        return;
    }

    if ((lcd_drv_reg->io_type == LCD_IO_4WIRE_8BIT) || (lcd_drv_reg->io_type == LCD_IO_I8080)) {
        if(is_high){
            gpio.sfr[GPIOxSET] = BIT(gpio.num);
        }else{
            gpio.sfr[GPIOxCLR] = BIT(gpio.num);
        }
    } else {
        if(is_high){
            LCDSPICON |= BIT(19);
        }else{
            LCDSPICON &= ~BIT(19);
        }
    }
}

void lcd_drv_set_speed(lcd_drv_t *drv)
{
    switch (drv->io_type) {
    case LCD_IO_3WIRE_9BIT:
    case LCD_IO_3WIRE_9BIT_2LINE:
    case LCD_IO_4WIRE_8BIT:
    case LCD_IO_QSPI:
    case LCD_IO_I8080: {
        u16 speed1, speed2;
        if(drv->param.spi_drv_param.speed1_mhz == 0){
            drv->param.spi_drv_param.speed1_mhz = 41;
        }
        if(drv->param.spi_drv_param.speed2_mhz == 0){
            drv->param.spi_drv_param.speed2_mhz = 31;
        }

//        u8 sys_clk = sys_clk_get();
        const u32 pll_clk_tbl[] = {24, 48, 589/2, 589/3, 576, 576/2};
//        speed1 = sys_clk_tbl[sys_clk] /(drv->param.spi_drv_param.speed1_mhz) - 1; //248
//        speed2 = sys_clk_tbl[sys_clk] /(drv->param.spi_drv_param.speed2_mhz) - 1; //248
        u32 lcd_clk = pll_clk_tbl[drv->param.spi_drv_param.clk_select] / (drv->param.spi_drv_param.clk_div+1);
        speed1 = lcd_clk / (drv->param.spi_drv_param.speed1_mhz) - 1; //248
        speed2 = lcd_clk / (drv->param.spi_drv_param.speed2_mhz) - 1; //248
        tft_set_baud(speed1, speed2);                       //设置LCD时钟
        printf("%s:clk[%d]=>%d,%d\n",__func__, lcd_clk, speed1, speed2);
    }   break;

    case LCD_IO_SRGB8:
    case LCD_IO_PRGB16:
        tft_set_baud(0, 0);
        break;

    default:
        break;
    }
}

void lcd_drv_rst(lcd_drv_t *drv)
{
    switch (drv->io_type) {
    case LCD_IO_3WIRE_9BIT:
    case LCD_IO_3WIRE_9BIT_2LINE:
    case LCD_IO_4WIRE_8BIT:
    case LCD_IO_QSPI:
    case LCD_IO_I8080: {
        gpio_t gpio;
        gpio_cfg_init(&gpio, drv->io.spi_drv_io.control.cio.rst);
        gpio.sfr[GPIOxSET] = BIT(gpio.num);
        delay_ms(10);
        gpio.sfr[GPIOxCLR] = BIT(gpio.num);
        delay_ms(20);
        gpio.sfr[GPIOxSET] = BIT(gpio.num);
        delay_ms(50);
    }   break;

    case LCD_IO_SRGB8:
    case LCD_IO_PRGB16:
        break;

    default:
        break;
    }
}

void lcd_drv_io_init(lcd_drv_t *drv)
{
    gpio_t gpio;

    switch (drv->io_type) {
    case LCD_IO_3WIRE_9BIT:
    case LCD_IO_3WIRE_9BIT_2LINE:
    case LCD_IO_4WIRE_8BIT:
    case LCD_IO_QSPI:
    case LCD_IO_I8080:
        //data io init
        for (int i=0; i<sizeof(drv->io.spi_drv_io.data.group)/sizeof(drv->io.spi_drv_io.data.group[0]); i++) {
            if (drv->io.spi_drv_io.data.group[i] != IO_NONE) {
                gpio_cfg_init(&gpio, drv->io.spi_drv_io.data.group[i]);
                gpio.sfr[GPIOxDE]   |= BIT(gpio.num);
                gpio.sfr[GPIOxFEN]  |= BIT(gpio.num);
                gpio.sfr[GPIOxDIR]  |= BIT(gpio.num);
                TRACE("lcd port D%d init\n", i);
            }
        }
        FUNCMCON2 = PORT_TFT_LCD_GROUP_NUM;
        TRACE("lcd port data init end\n");

        //control io init
        //dc
        gpio_cfg_init(&gpio, drv->io.spi_drv_io.control.cio.dc);
        if (drv->io_type == LCD_IO_3WIRE_9BIT
            || drv->io_type == LCD_IO_3WIRE_9BIT_2LINE
            || drv->io_type == LCD_IO_QSPI) {       //1
            gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
            gpio.sfr[GPIOxDE]  |= BIT(gpio.num);
            gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
        } else {           //0
            gpio.sfr[GPIOxFEN] &= ~BIT(gpio.num);
            gpio.sfr[GPIOxDE]  |= BIT(gpio.num);
            gpio.sfr[GPIOxSET]  = BIT(gpio.num);
            gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
        }
        TRACE("lcd port dc init end\n");

        //cs
        gpio_cfg_init(&gpio, drv->io.spi_drv_io.control.cio.cs);
        gpio.sfr[GPIOxFEN] &= ~BIT(gpio.num);
        gpio.sfr[GPIOxDE] |= BIT(gpio.num);
        gpio.sfr[GPIOxSET] = BIT(gpio.num);
        gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
        TRACE("lcd port cs init end\n");

        //rst
        gpio_cfg_init(&gpio, drv->io.spi_drv_io.control.cio.rst);
        gpio.sfr[GPIOxFEN] &= ~BIT(gpio.num);
        gpio.sfr[GPIOxDE] |= BIT(gpio.num);
        gpio.sfr[GPIOxSET] = BIT(gpio.num);
        gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
        TRACE("lcd port rst init end\n");

        //te
        //TFT_TE_EDGE: 0=上升沿(TE拉高=垂直消隐开始, 面板停止读GRAM), 1=下降沿(消隐结束, 面板开始扫描第1行)
        //ST7789P3 手册 8.14.3: MPU 推屏比面板扫描快时, 应在消隐期(上升沿)起写, 写指针全程领先扫描线
        //        手册 8.14.4: MPU 推屏比面板扫描慢时, 才在扫描开始后(下降沿)起写, 写指针跟在读指针后面跨帧写完
        //未定义时保持 SDK 原行为(下降沿)
#ifndef TFT_TE_EDGE
#define TFT_TE_EDGE                     1
#endif
        if (drv->io.spi_drv_io.control.cio.te != IO_NONE) {
            port_irq_register(PORT_TFT_INT_VECTOR, drv->tft_te_isr);
            port_wakeup_init(drv->io.spi_drv_io.control.cio.te, TFT_TE_EDGE, 1);     //开内部上拉
        }
        TRACE("lcd port te init end\n");

        //scl
        gpio_cfg_init(&gpio, drv->io.spi_drv_io.control.cio.scl);
        gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
        gpio.sfr[GPIOxDE]  |= BIT(gpio.num);
        gpio.sfr[GPIOxCLR]  = BIT(gpio.num);
        gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
        TRACE("lcd port scl init end\n");
        break;

    case LCD_IO_SRGB8:
    case LCD_IO_PRGB16:
        //data io init
        for (int i=0; i<sizeof(drv->io.rgb_drv_io.data.group)/sizeof(drv->io.rgb_drv_io.data.group[0]); i++) {
            if (drv->io.rgb_drv_io.data.group[i] != IO_NONE) {
                gpio_cfg_init(&gpio, drv->io.rgb_drv_io.data.group[i]);
                gpio.sfr[GPIOxDE]   |= BIT(gpio.num);
                gpio.sfr[GPIOxFEN]  |= BIT(gpio.num);
                gpio.sfr[GPIOxDIR]  &= ~BIT(gpio.num);
                gpio.sfr[GPIOxCLR]   = BIT(gpio.num);
                TRACE("lcd port D%d init\n", i);
            }
        }
        FUNCMCON2 = PORT_TFT_LCD_GROUP_NUM;
        TRACE("lcd port data init end:%x\n", PORT_TFT_LCD_GROUP_NUM);

        //display
        gpio_cfg_init(&gpio, drv->io.rgb_drv_io.control.cio.display);
        gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
        gpio.sfr[GPIOxDE]  |= BIT(gpio.num);
        gpio.sfr[GPIOxSET]  = BIT(gpio.num);
        gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
        TRACE("lcd port display init end\n");

        //de
        gpio_cfg_init(&gpio, drv->io.rgb_drv_io.control.cio.de);
        gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
        gpio.sfr[GPIOxDE]  |= BIT(gpio.num);
        gpio.sfr[GPIOxCLR]  = BIT(gpio.num);
        gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
        TRACE("lcd port de init end\n");

        //clk
        gpio_cfg_init(&gpio, drv->io.rgb_drv_io.control.cio.clk);
        gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
        gpio.sfr[GPIOxDE]  |= BIT(gpio.num);
        gpio.sfr[GPIOxCLR]  = BIT(gpio.num);
        gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
        TRACE("lcd port clk init end\n");

        //vsync
        gpio_cfg_init(&gpio, drv->io.rgb_drv_io.control.cio.vsync);
        gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
        gpio.sfr[GPIOxDE]  |= BIT(gpio.num);
        gpio.sfr[GPIOxCLR]  = BIT(gpio.num);
        gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
        TRACE("lcd port vsync init end\n");

        //hsync
        gpio_cfg_init(&gpio, drv->io.rgb_drv_io.control.cio.hsync);
        gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
        gpio.sfr[GPIOxDE]  |= BIT(gpio.num);
        gpio.sfr[GPIOxCLR]  = BIT(gpio.num);
        gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
        TRACE("lcd port hsync init end\n");
        break;

    default:
        printf("Err LCD IO Type\n");
        break;
    }
}

void lcd_drv_io_deinit(lcd_drv_t *drv)
{
    gpio_t gpio;

    switch (drv->io_type) {
    case LCD_IO_3WIRE_9BIT:
    case LCD_IO_3WIRE_9BIT_2LINE:
    case LCD_IO_4WIRE_8BIT:
    case LCD_IO_QSPI:
    case LCD_IO_I8080:
        //data io init
        for (int i=0; i<sizeof(drv->io.spi_drv_io.data.group)/sizeof(drv->io.spi_drv_io.data.group[0]); i++) {
            if (drv->io.spi_drv_io.data.group[i] != IO_NONE) {
                gpio_cfg_init(&gpio, drv->io.spi_drv_io.data.group[i]);
                gpio.sfr[GPIOxDE]   &= ~BIT(gpio.num);
//                gpio.sfr[GPIOxFEN]  |= BIT(gpio.num);
                gpio.sfr[GPIOxDIR]  |= BIT(gpio.num);
                TRACE("lcd port D%d deinit\n", i);
            }
        }
        TRACE("lcd port data deinit end\n");

        //control io init
        //dc
        gpio_cfg_init(&gpio, drv->io.spi_drv_io.control.cio.dc);
//      gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
        gpio.sfr[GPIOxDE]  &= ~BIT(gpio.num);
        gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
        TRACE("lcd port dc deinit end\n");

        //cs
        gpio_cfg_init(&gpio, drv->io.spi_drv_io.control.cio.cs);
        gpio.sfr[GPIOxDE] &= ~BIT(gpio.num);
        gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
        TRACE("lcd port cs deinit end\n");

        //rst
        if(!vddio_sleep_level) {
            gpio_cfg_init(&gpio, drv->io.spi_drv_io.control.cio.rst);
            gpio.sfr[GPIOxDE] &= ~BIT(gpio.num);
            gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
        }
        TRACE("lcd port rst deinit end\n");

        //te
        if (drv->io.spi_drv_io.control.cio.te != IO_NONE) {
        #if (GUI_SELECT == GUI_VGS_640)
            //  TE 脚发送命令之前要提前拉起来
            gpio_t gpio;
            gpio_cfg_init(&gpio, drv->io.spi_drv_io.control.cio.te);
            gpio.sfr[GPIOxDE] |= BIT(gpio.num);
            gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
            gpio.sfr[GPIOxFEN] &= ~BIT(gpio.num);
            gpio.sfr[GPIOxPU] |= BIT(gpio.num);
        #else
            port_irq_free(PORT_TFT_INT_VECTOR);
            port_wakeup_exit(drv->io.spi_drv_io.control.cio.te);
        #endif
        }
        TRACE("lcd port te deinit end\n");

        //scl
        gpio_cfg_init(&gpio, drv->io.spi_drv_io.control.cio.scl);
        gpio.sfr[GPIOxDE]  &= ~BIT(gpio.num);
        gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
        TRACE("lcd port scl deinit end\n");
        break;

    case LCD_IO_SRGB8:
    case LCD_IO_PRGB16:
        //data io init
        for (int i=0; i<sizeof(drv->io.rgb_drv_io.data.group)/sizeof(drv->io.rgb_drv_io.data.group[0]); i++) {
            if (drv->io.rgb_drv_io.data.group[i] != IO_NONE) {
                gpio_cfg_init(&gpio, drv->io.rgb_drv_io.data.group[i]);
                gpio.sfr[GPIOxDE]   &= ~BIT(gpio.num);
                gpio.sfr[GPIOxDIR]  |= BIT(gpio.num);
                TRACE("lcd port D%d init\n", i);
            }
        }
        FUNCMCON2 = PORT_TFT_LCD_GROUP_NUM;
        TRACE("lcd port data init end\n");

        //de
        gpio_cfg_init(&gpio, drv->io.rgb_drv_io.control.cio.de);
        gpio.sfr[GPIOxDE]   &= ~BIT(gpio.num);
        gpio.sfr[GPIOxDIR]  |= BIT(gpio.num);
        TRACE("lcd port de init end\n");

        //clk
        gpio_cfg_init(&gpio, drv->io.rgb_drv_io.control.cio.clk);
        gpio.sfr[GPIOxDE]   &= ~BIT(gpio.num);
        gpio.sfr[GPIOxDIR]  |= BIT(gpio.num);
        TRACE("lcd port clk init end\n");

        //vsync
        gpio_cfg_init(&gpio, drv->io.rgb_drv_io.control.cio.vsync);
        gpio.sfr[GPIOxDE]   &= ~BIT(gpio.num);
        gpio.sfr[GPIOxDIR]  |= BIT(gpio.num);
        TRACE("lcd port vsync init end\n");

        //hsync
        gpio_cfg_init(&gpio, drv->io.rgb_drv_io.control.cio.hsync);
        gpio.sfr[GPIOxDE]   &= ~BIT(gpio.num);
        gpio.sfr[GPIOxDIR]  |= BIT(gpio.num);
        TRACE("lcd port hsync init end\n");
        break;


    default:
        printf("Err LCD IO Type\n");
        break;
    }
}

void lcd_drv_reg_init(lcd_drv_t *drv)
{
    switch (drv->io_type) {
    case LCD_IO_3WIRE_9BIT:
        LCDSTRD = GUI_SCREEN_WIDTH * 2;
        LCDSPICON = BIT(18) | BIT(0);
        LCDCON    = BIT(31) | BIT(0);
        break;
    case LCD_IO_3WIRE_9BIT_2LINE:
        LCDSTRD = GUI_SCREEN_WIDTH * 2;
        LCDSPICON = BIT(18) | BIT(0);
        LCDCON    = BIT(31) | BIT(0);
        break;
    case LCD_IO_4WIRE_8BIT:
        LCDSTRD = GUI_SCREEN_WIDTH * 2;
        LCDSPICON = BIT(0);
        LCDCON    = BIT(31) | BIT(0);
        break;
    case LCD_IO_QSPI:
        LCDSTRD = GUI_SCREEN_WIDTH * 2;
        LCDSPICON = BIT(9)  | BIT(3) | BIT(2) | BIT(0);                          //[9]MultiBit, [3:2]4BIT, [0]EN
        LCDCON    = BIT(31) | BIT(0);
        break;
    case LCD_IO_I8080:
        LCDSTRD = GUI_SCREEN_WIDTH * 2;
        LCDSPICON = BIT(17) | BIT(9) | BIT(0);                                  //[9]MultiBit, [0]EN
        LCDCON    = BIT(25) | BIT(0);
        break;

    case LCD_IO_PRGB16:
    case LCD_IO_SRGB8:
        if (drv->param.rgb_drv_param.refersh_dma_buf_size != GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*sizeof(lcd_color_t)) {
            printf("refersh_dma_buf_size error\n");
        }

        LCDSPIBAUD = 0;
        LCDSPICON = BIT(0) | BIT(17);
        LCDCON    = BIT(19) | BIT(16) | BIT(0); //| BIT(31);
        LCDCON |= (drv->param.rgb_drv_param.in_rgb << 2);
        LCDCON |= (drv->param.rgb_drv_param.out_rgb << 6);
        if (drv->io_type == LCD_IO_PRGB16) {
            LCDCON |= BIT(18);
        }
        LCDVPP = (drv->param.rgb_drv_param.vsync_width << 0)                                            |
                 ((drv->param.rgb_drv_param.vback_porch - drv->param.rgb_drv_param.vsync_width) << 8)   |
                 (drv->param.rgb_drv_param.vfront_proch << 16);
        LCDHPP = (drv->param.rgb_drv_param.hsync_width << 0)                                            |
                 ((drv->param.rgb_drv_param.hback_porch - drv->param.rgb_drv_param.hsync_width) << 8)   |
                 (drv->param.rgb_drv_param.hfront_porch << 16);
        LCDSIZE = (GUI_SCREEN_WIDTH - 1) | ((GUI_SCREEN_HEIGHT - 1) << 16);
        u8 depth = 2;
        if (drv->param.rgb_drv_param.in_rgb == LCD_IN_RGB565) {
            depth = 2;
        } else if (drv->param.rgb_drv_param.in_rgb == LCD_IN_RGB888
                   || drv->param.rgb_drv_param.in_rgb == LCD_IN_ARGB8565) {
            depth = 3;
        } else if (drv->param.rgb_drv_param.in_rgb == LCD_IN_ARGB8888) {
            depth = 4;
        }
        LCDSTRD = (GUI_SCREEN_WIDTH)*depth;
        memset(drv->param.rgb_drv_param.refersh_dma_buf, 0, drv->param.rgb_drv_param.refersh_dma_buf_size);
        TRACE("refersh_dma_buf:0x%x\n", drv->param.rgb_drv_param.refersh_dma_buf);
        u32 refersh_dma_buf = 0;
        if ((u32)drv->param.rgb_drv_param.refersh_dma_buf > (u32)&__psram_vma) {
            refersh_dma_buf = DMA_ADR(drv->param.rgb_drv_param.refersh_dma_buf) | BIT(28);
        } else {
            refersh_dma_buf = DMA_ADR(drv->param.rgb_drv_param.refersh_dma_buf);
        }
        LCDADR = refersh_dma_buf;
        LCDCON |= BIT(1);

        extern void gpu_share_tft_init(bool rgbw_support, bool ramless_support,
                            void* ramless_buf, u16 ramless_line_wid);
        gpu_share_tft_init(false, true, (void*)refersh_dma_buf, GUI_SCREEN_WIDTH);
        TRACE("refersh_dma_buf end:0x%x\n", refersh_dma_buf);

        break;

    default:
        printf("Err LCD IO Type\n");
        break;
    }
}

void lcd_drv_reg_deinit(lcd_drv_t *drv)
{
    switch (drv->io_type) {
    case LCD_IO_3WIRE_9BIT:
    case LCD_IO_3WIRE_9BIT_2LINE:
    case LCD_IO_4WIRE_8BIT:
    case LCD_IO_QSPI:
    case LCD_IO_I8080:
        LCDSPICON = 0;
        LCDCON = 0;
        break;

    case LCD_IO_SRGB8:
    case LCD_IO_PRGB16:
        LCDCON &= ~BIT(16);
        while (!(LCDPEND & BIT(0)));
        LCDPEND = BIT(0);
        LCDCON = 0;
        LCDSPICON = 0;
        extern void gpu_share_tft_init(bool rgbw_support, bool ramless_support,
                            void* ramless_buf, u16 ramless_line_wid);
        gpu_share_tft_init(false, false, NULL, 0);
        break;

    default:
        printf("Err LCD IO Type\n");
        break;
    }
}

void lcd_drv_clk_init(lcd_drv_t *drv)
{
    switch (drv->io_type) {
    case LCD_IO_3WIRE_9BIT:
    case LCD_IO_3WIRE_9BIT_2LINE:
    case LCD_IO_4WIRE_8BIT:
    case LCD_IO_QSPI:
    case LCD_IO_I8080:
        //clk init
//    #if MSPI_DDR_MODE_EN
//        CLKDIVCON2 = (CLKDIVCON2 & ~(0xf << 28)) | (3 << 28);           //LCD_CLK DIV2，ddr模式下，spibaud==0，只能通过分频系数设置时钟频率
//    #else
        CLKDIVCON2 = (CLKDIVCON2 & ~(0xf << 28)) | (drv->param.spi_drv_param.clk_div << 28);           //LCD_CLK DIV2
//    #endif
        CLKCON3 = (CLKCON3 & ~(7 << 12)) | (drv->param.spi_drv_param.clk_select << 12);                   //LCD_CLK select pll0_div2  LCD_CLK = 589/2
        CLKGAT3 |= BIT(8) | BIT(3) | BIT(2) |  BIT(0);                  //PAR EN, LCD SPI EN, DE EN, GPU EN
        RSTCON0 |= BIT(21) | BIT(16) | BIT(10);                         //LCD Release, DE Release, GPU Release
        break;

    case LCD_IO_SRGB8:
    case LCD_IO_PRGB16:
        TRACE("select:%d, div:%d\n", drv->param.rgb_drv_param.clk_select, drv->param.rgb_drv_param.clk_div);
        CLKDIVCON2 = (CLKDIVCON2 & ~(0xf << 28)) | (drv->param.rgb_drv_param.clk_div << 28);
        CLKCON3 = (CLKCON3 & ~(7 << 12)) | (drv->param.rgb_drv_param.clk_select << 12);
        CLKGAT3 |= BIT(8) | BIT(3) | BIT(2) |  BIT(0);                  //PAR EN, LCD SPI EN, DE EN, GPU EN
        RSTCON0 |= BIT(21) | BIT(16) | BIT(10);                         //LCD Release, DE Release, GPU Release
        break;

    default:
        break;
    }
}

void lcd_drv_clk_deinit(lcd_drv_t *drv)
{
    switch (drv->io_type) {
    case LCD_IO_3WIRE_9BIT:
    case LCD_IO_3WIRE_9BIT_2LINE:
    case LCD_IO_4WIRE_8BIT:
    case LCD_IO_QSPI:
    case LCD_IO_I8080:
    case LCD_IO_SRGB8:
    case LCD_IO_PRGB16:
        RSTCON0 &= ~(BIT(16) | BIT(21));                    //DE Reset
        CLKGAT3 &= ~(BIT(8) | BIT(3) | BIT(2));             //PAR DIS, LCD SPI DIS, DE DIS
        break;

    default:
        break;
    }
}

/**
 * lcd register
 */
void lcd_drv_register(lcd_drv_t *drv)
{
    lcd_drv_reg = drv;
    if(lcd_drv_reg == NULL){
        return;
    }
    lcd_drv_clk_init(drv);
    lcd_drv_set_speed(drv);
    lcd_drv_io_init(drv);
    lcd_drv_reg_init(drv);
    lcd_drv_rst(drv);
}

void lcd_drv_deregister(void)
{
//    gpio_t gpio;

    if(lcd_drv_reg == NULL){
        return;
    }

    lcd_drv_io_deinit(lcd_drv_reg);
    lcd_drv_reg_deinit(lcd_drv_reg);
}

void lcd_drv_clk_deregister(void)
{
    lcd_drv_clk_deinit(lcd_drv_reg);
}


void lcd_drv_init(void)
{
    if(lcd_drv_reg == NULL){
        return;
    }

    if (lcd_drv_reg->tft_reg_init) {
        lcd_drv_reg->tft_reg_init();
    }
}

AT(.com_text.tft_spi)
void lcd_drv_set_window(u16 x0, u16 y0, u16 x1, u16 y1)
{
    if(lcd_drv_reg == NULL){
        return;
    }

    if (lcd_drv_reg->tft_set_window) {
        lcd_drv_reg->tft_set_window(x0, y0, x1, y1);
    }
}

uint32_t lcd_drv_read_id(void)
{
    if((lcd_drv_reg == NULL) || (lcd_drv_reg->tft_read_id == NULL)){
        return 0;
    }

    if (lcd_drv_reg->tft_read_id) {
        return lcd_drv_reg->tft_read_id();
    }

    return 0;
}

void lcd_drv_set_brightness(u8 brightness)
{
    if((lcd_drv_reg == NULL) || (lcd_drv_reg->tft_set_brightness == NULL)){
        return;
    }

    if (lcd_drv_reg->tft_set_brightness) {
        lcd_drv_reg->tft_set_brightness(brightness);
    }
}
