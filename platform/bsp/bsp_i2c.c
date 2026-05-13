#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_K(...)            printk(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_K(...)
#endif

#if I2C_SW_EN
//AT(.text.bsp.i2c)
//static void bsp_i2c_delay(void)
//{
//    u8 delay = 60;
//    while (delay--) {
//        asm("nop");
//    }
//}
#define bsp_i2c_delay() delay_us(5)

//ACK: The transmitter releases the SDA line (HIGH->LOW) during the acknowledge clock pulse
AT(.text.bsp.i2c)
void bsp_i2c_tx_ack(void)
{
    I2C_SDA_OUT();
    I2C_SDA_L();
    bsp_i2c_delay();
    I2C_SCL_H();
    bsp_i2c_delay();
    I2C_SCL_L();
}

AT(.text.bsp.i2c)
bool bsp_i2c_rx_ack(void)
{
    bool ret = false;
    I2C_SDA_IN();
    bsp_i2c_delay();
    I2C_SCL_H();
    bsp_i2c_delay();
    if (!I2C_SDA_IS_H()) {
        ret = true;
    }
    I2C_SCL_L();
    return ret;
}

//NACK: The transmitter holds the SDA line (keep HIGH) during the acknowledge clock pulse
AT(.text.bsp.i2c)
void bsp_i2c_tx_nack(void)
{
    I2C_SDA_OUT();
    I2C_SDA_H();
    bsp_i2c_delay();
    I2C_SCL_H();
    bsp_i2c_delay();
    I2C_SCL_L();
}

//START: A HIGH to LOW transition on the SDA line while SCL is HIGH is one such unique case.
AT(.text.bsp.i2c)
void bsp_i2c_start(void)
{
    I2C_SDA_SCL_OUT();
    I2C_SDA_SCL_H();
    bsp_i2c_delay();
    I2C_SDA_L();
    bsp_i2c_delay();
    I2C_SCL_L();
}

//STOP: A LOW to HIGH transition on the SDA line while SCL is HIGH
AT(.text.bsp.i2c)
void bsp_i2c_stop(void)
{
    I2C_SDA_OUT();
    I2C_SDA_L();
    bsp_i2c_delay();
    I2C_SCL_H();
    bsp_i2c_delay();
    I2C_SDA_H();
}

//tx 1byte
AT(.text.bsp.i2c)
void bsp_i2c_tx_byte(uint8_t dat)
{
    u8 i;
    I2C_SDA_OUT();
    for (i=0; i<8; i++) {
        if (dat & BIT(7)) {
            I2C_SDA_H();
        } else {
            I2C_SDA_L();
        }
        bsp_i2c_delay();
        I2C_SCL_H();
        bsp_i2c_delay();
        I2C_SCL_L();
        dat <<= 1;
    }
}

//rx 1byte
AT(.text.bsp.i2c)
uint8_t bsp_i2c_rx_byte(void)
{
    u8 i, dat = 0;
    I2C_SDA_IN();
    for (i=0; i<8; i++) {
        bsp_i2c_delay();
        I2C_SCL_H();
        bsp_i2c_delay();
        dat <<= 1;
        if (I2C_SDA_IS_H()) {
            dat |= BIT(0);
        }
        I2C_SCL_L();
    }
    return dat;
}
#endif //I2C_SW_EN


#if I2C_HW_EN


static bool is_i2c0_init = false;
static bool is_i2c1_init = false;
static bool is_i2c2_init = false;

i2c_t HW_IIC0 = {
    .sfr         = (i2c_sfr_t *) &IIC0CON0,
    .map         = (i2c_map_t *) &FUNCMCON2,
};

i2c_t HW_IIC1 = {
    .sfr         = (i2c_sfr_t *) &IIC1CON0,
    .map         = (i2c_map_t *) &FUNCMCON2,
};

i2c_t HW_IIC2 = {
    .sfr         = (i2c_sfr_t *) &IIC2CON0,
    .map         = (i2c_map_t *) &FUNCMCON2,
};

AT(.com_text.i2c)
i2c_t* bsp_i2c_get_register(u8 i2cx_idx)
{
    i2c_t *HW_IIC = NULL;
    if (i2cx_idx == 0) {  //I2C0
        HW_IIC = &HW_IIC0;
    } else if (i2cx_idx == 1) { //I2C1
        HW_IIC = &HW_IIC1;
    } else if (i2cx_idx == 2) {
        HW_IIC = &HW_IIC2;
    }

    return HW_IIC;
}

AT(.text.bsp.i2c)
void bsp_i2c_sensor_hub_set_hold(bool en)
{
    if (en) {
        SENSHBCON &= ~BIT(SENSHB_IE_SHF);
//        bsp_sensor_hub_tmr_hold(true);
        SENSHBTCON |= BIT(SENSHB_TMR_HOLD_SHF);

//        bsp_sensor_hub_hold();
        SENSHBTCON |= BIT(SENSHB_HOLD_SET_SHF);
        if (SENSHBCON & BIT(SENSHB_IE_SHF)){
            ;
        }else{
            while(!(SENSHBCPND & BIT(SENSHB_HOLD_PND_SHF)));
            SENSHBCPND = BIT(SENSHB_HOLD_PND_SHF);
        }

//        bsp_sensor_hub_iic_en(0, false);
        SENSHBCON &= ~BIT(SENSHB_IIC_EN_SHF+0);
        IIC0DMACNT = 0;                         // Disable DMA
    } else {
//        bsp_sensor_hub_resume();
        SENSHBCLR |= BIT(SENSHB_HOLD_CLR_SHF);

//        bsp_sensor_hub_iic_en(0, true);
        SENSHBCON |= BIT(SENSHB_IIC_EN_SHF+0);
        SENSHBCON |= BIT(SENSHB_IE_SHF);
//        bsp_sensor_hub_tmr_hold(false);
        SENSHBTCON &= ~BIT(SENSHB_TMR_HOLD_SHF);
    }
}

AT(.text.bsp.i2c)
void bsp_i2c0_lock(void)
{
    os_i2c0_lock(100);
}

AT(.text.bsp.i2c)
void bsp_i2c0_unlock(void)
{
    os_i2c0_unlock();
}

AT(.com_text.isr.i2c)
void bsp_i2c0_isr(void)
{
    i2c_t *HW_IIC = bsp_i2c_get_register(0);
    if (HW_IIC->sfr->IICxCON0 & BIT(31)) {
        HW_IIC->sfr->IICxCON0 |=  BIT(29);                   // clear pending
        HW_IIC->sfr->IICxCON0 &= ~BIT(1);                    // disable irq

        HW_IIC->sfr->IICxDMACNT = 0;                         // Disable DMA
    }
}

AT(.com_text.isr.i2c)
void bsp_i2c1_isr(void)
{
    i2c_t *HW_IIC = bsp_i2c_get_register(1);
    if (HW_IIC->sfr->IICxCON0 & BIT(31)) {
        HW_IIC->sfr->IICxCON0 |=  BIT(29);                   // clear pending
        HW_IIC->sfr->IICxCON0 &= ~BIT(1);                    // disable irq

        HW_IIC->sfr->IICxDMACNT = 0;                         // Disable DMA
    }
}

AT(.com_text.isr.i2c)
void bsp_i2c2_isr(void)
{
    i2c_t *HW_IIC = bsp_i2c_get_register(2);
    if (HW_IIC->sfr->IICxCON0 & BIT(31)) {
        HW_IIC->sfr->IICxCON0 |=  BIT(29);                   // clear pending
        HW_IIC->sfr->IICxCON0 &= ~BIT(1);                    // disable irq

        HW_IIC->sfr->IICxDMACNT = 0;                         // Disable DMA
    }
}

AT(.text.bsp.i2c)
void bsp_i2c_irq_init(u8 i2cx_idx)
{
    if (i2cx_idx == 0) {
        sys_irq_init(IRQ_I2C_VECTOR, 0, bsp_i2c0_isr);
    } else if (i2cx_idx == 1) {
        sys_irq_init(IRQ_I2C_VECTOR, 0, bsp_i2c1_isr);
    } else if (i2cx_idx == 2) {
        sys_irq_init(IRQ_I2C_VECTOR, 0, bsp_i2c2_isr);
    }
}


AT(.text.bsp.i2c)
static u32 bsp_hw_i2c_config(u8 i2cx_idx, u32 i2c_cfg, u16 dev_addr, u16 reg_addr, u32 dat)
{
    TRACE("%s: cfg=%X dev=%X reg=%X\n", __FUNCTION__, i2c_cfg, dev_addr, reg_addr);

    i2c_t *HW_IIC = bsp_i2c_get_register(i2cx_idx);
    HW_IIC->sfr->IICxCMDA = (u8)dev_addr | ((u32)(dev_addr >> 8)) << 24 |
                              (u32)((u8)reg_addr) << 8 | (u32)((u8)(reg_addr >> 8)) << 16;

    HW_IIC->sfr->IICxCON1 = i2c_cfg;
    HW_IIC->sfr->IICxDATA = dat;
    HW_IIC->sfr->IICxCON0 |= BIT(28);                     // kick

    u32 ticks = tick_get();
    while ( (!(HW_IIC->sfr->IICxCON0 & BIT(31)))) {
        if (tick_check_expire(ticks, 20)) {
            printf("IIC ERROR\n");
            return false;
        }
    }

    HW_IIC->sfr->IICxCON0 |= BIT(29);

    if (i2c_cfg & RDATA) {
        return HW_IIC->sfr->IICxDATA;
    } else {
        return true;
    }
}

AT(.text.bsp.i2c)
void bsp_hw_i2c_tx_byte(u8 i2cx_idx, u32 i2c_cfg, u16 dev_addr, u16 reg_addr, u32 data)
{
    bsp_i2c_sensor_hub_set_hold(true);
    bsp_i2c0_lock();
    bsp_hw_i2c_config(i2cx_idx, i2c_cfg, dev_addr, reg_addr, data);
    bsp_i2c0_unlock();
    bsp_i2c_sensor_hub_set_hold(false);
}

AT(.text.bsp.i2c)
void bsp_hw_i2c_tx_buf(u8 i2cx_idx, u32 i2c_cfg, u16 dev_addr, u16 reg_addr, u8 *buf, u16 len)
{
    int i;
    u32 cfg;
    if (buf == NULL || len == 0) {
        return;
    }

    bsp_i2c0_lock();

    for (i = 0; i < len; i++) {
        cfg = WDATA;
        if (i == 0) {               //收第1byte
            cfg |= i2c_cfg;
        }
        if (i == (len - 1)) {       //收最后1byte
            cfg |= STOP_FLAG;
        }
        bsp_hw_i2c_config(i2cx_idx, cfg | DATA_CNT_1B, dev_addr, reg_addr, buf[i]);
    }

    bsp_i2c0_unlock();
}

AT(.text.bsp.i2c)
void bsp_hw_i2c_rx_buf(u8 i2cx_idx, u32 i2c_cfg, u16 dev_addr, u16 reg_addr, u8 *buf, u16 len)
{
    int i;
    u32 cfg;
    if (buf == NULL || len == 0) {
        return;
    }

    bsp_i2c0_lock();

    for (i = 0; i < len; i++) {
        cfg = RDATA;
        if (i == 0) {               //收第1byte
            cfg |= i2c_cfg;
        }
        if (i == (len - 1)) {       //收最后1byte
            cfg |= STOP_FLAG | NACK;
        }
        buf[i] = bsp_hw_i2c_config(i2cx_idx, cfg | DATA_CNT_1B, dev_addr, reg_addr, 0);
    }

    bsp_i2c0_unlock();
}

/**
 * @brief  IIC0~IIC2通用写接口
 * @param[in] dev_addr  从设备7bit设备地址
 * @param[in] wbuf  写buf，当使用IIC0时，传入的buf需AT到sensorhub
 * @param[in] wlen  写buf长度
 * @param[in] stop  当次写入是否终止，stop: true:当次写入结束  false:后续跟读操作
 *
 * @return  返回是否成功
 **/
AT(.text.bsp.i2c)
bool bsp_hw_i2c_dma_write_buf(u8 i2cx_idx, u16 dev_addr, u8 *wbuf, u16 wlen, bool stop)
{
    i2c_t *HW_IIC = bsp_i2c_get_register(i2cx_idx);

    u32 cfg = 0;
    u32 reg_addr = 0;
    u32 w_dev_addr = dev_addr << 1;

    if (wlen == 0 || wbuf == NULL) {
        return false;
    }

    bsp_i2c0_lock();
    if (HW_IIC == (&HW_IIC0)) {
        bsp_i2c_sensor_hub_set_hold(true);
    }

    HW_IIC->sfr->IICxDMACNT = 0;
    cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0;
    reg_addr = wbuf[0];
    wlen --;
    if (0 == wlen) {
        if (stop) {
            cfg |= STOP_FLAG;
        }
        bsp_hw_i2c_config(i2cx_idx, cfg, w_dev_addr, reg_addr, 0);
    } else {
        HW_IIC->sfr->IICxCMDA = (u8)w_dev_addr | ((u32)(w_dev_addr >> 8)) << 24 | (u32)((u8)reg_addr) << 8;
        if (stop) {
            HW_IIC->sfr->IICxCON1 = cfg | STOP_FLAG | WDATA;
        } else {
            HW_IIC->sfr->IICxCON1 = cfg | WDATA;
        }
        HW_IIC->sfr->IICxDMAADR = DMA_ADR(&wbuf[1]);
        HW_IIC->sfr->IICxDMACNT = ((wlen - 1) << 16) | BIT(1) | BIT(0);
        HW_IIC->sfr->IICxCON0 |= BIT(28);   //KICK

        u32 ticks = tick_get();
        while (!(HW_IIC->sfr->IICxCON0 & BIT(31))) {
            if (tick_check_expire(ticks, 20)) {
                    printf("iic write timeout\n");
                if (HW_IIC == (&HW_IIC0)) {
                    bsp_i2c_sensor_hub_set_hold(false);
                }
                bsp_i2c0_unlock();
                return false;
            }
        }
        HW_IIC->sfr->IICxCON0 |= BIT(29);                  //Clear Pending
    }
    if (HW_IIC == (&HW_IIC0)) {
        bsp_i2c_sensor_hub_set_hold(false);
    }
    bsp_i2c0_unlock();

    return true;
}

/**
 * @brief  IIC0~IIC2通用读接口,需要先写后读
 * @param[in] dev_addr  从设备7bit设备地址
 * @param[in] wbuf  写buf，当使用IIC0时，传入的buf需使用lpram，若使用bsp_hw_i2c_dma_write_buf已写入寄存器，可置NULL
 * @param[in] wlen  写buf长度, 若使用bsp_hw_i2c_dma_write_buf已写入寄存器，可置0
 * @param[in] rbuf  读buf，当使用IIC0时，传入的buf需AT到sensorhub
 * @param[in] rlen  读buf长度
 *
 * @return  返回是否成功
 **/
AT(.text.bsp.i2c)
bool bsp_hw_i2c_dma_read_buf(u8 i2cx_idx, u16 dev_addr, u8 *wbuf, u16 wlen, u8 *rbuf, u16 rlen)
{
    i2c_t *HW_IIC = bsp_i2c_get_register(i2cx_idx);

    int i;
    u32 cfg = 0;
    u32 reg_addr = 0;
    u32 w_dev_addr = dev_addr << 1;
    u32 r_dev_addr = ((dev_addr) << 1 | 1) << 8 | ((dev_addr) << 1);

    if (rlen == 0 || rbuf == NULL) {
        return false;
    }

    bsp_i2c0_lock();
    if (HW_IIC == (&HW_IIC0)) {
        bsp_i2c_sensor_hub_set_hold(true);
    }
    HW_IIC->sfr->IICxDMACNT = 0;

    //写入地址和读取寄存器
    if (wlen) {
        cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0;
        reg_addr = wbuf[0];
        bsp_hw_i2c_config(i2cx_idx, cfg, w_dev_addr, reg_addr, 0);
        wlen --;
        cfg = WDATA;
        for (i = 0; i < wlen; i++) {
            bsp_hw_i2c_config(i2cx_idx, cfg | DATA_CNT_1B, w_dev_addr, reg_addr, wbuf[i + 1]);
        }
    }

    //DMA接收读取数据
    cfg = START_FLAG1 | DEV_ADDR1 | RDATA | STOP_FLAG | NACK;
    HW_IIC->sfr->IICxCON1 = cfg;
    HW_IIC->sfr->IICxCMDA = ((u32)(r_dev_addr >> 8)) << 24;
    HW_IIC->sfr->IICxDMAADR = DMA_ADR(rbuf);
    HW_IIC->sfr->IICxDMACNT = ((rlen - 1) << 16) | BIT(0);
    HW_IIC->sfr->IICxCON0 |= BIT(28);                  //KICK

    u32 ticks = tick_get();
    while ( (!(HW_IIC->sfr->IICxDMACNT & BIT(2)))) {
        if (tick_check_expire(ticks, 20)) {
            if (HW_IIC == (&HW_IIC0)) {
                bsp_i2c_sensor_hub_set_hold(false);
            }
            bsp_i2c0_unlock();
            printf("IIC ERROR\n");
            return false;
        }
    }
    if (HW_IIC == (&HW_IIC0)) {
        bsp_i2c_sensor_hub_set_hold(false);
    }
    bsp_i2c0_unlock();

    return true;
}

AT(.text.bsp.i2c)
static void i2cx_init(u8 i2cx_idx, u8 port_scl, u8 port_sda, u8 port_map)
{
    i2c_t *HW_IIC = bsp_i2c_get_register(i2cx_idx);

    printf("%s\n", __func__);
  //IIC0由SENSOR HUB接管, 若要使用必须使能SENSOR HUB时钟
    if (HW_IIC == &HW_IIC0) {
        CLKGAT1 |= BIT(3);
        RTCCON0 |= BIT(0);      //RTC en
        RTCCON0 |= BIT(2);      //RTC clk to module
    //    RTCCON0 &= ~BIT(22);     //SNF_RC_EN
        RTCCON0 |= BIT(20);     //SNF_RC_EN
        RTCCON0 = (RTCCON0 & ~(BIT(14) | BIT(15))) | BIT(15);	//sniff rc
        RTCCON0 = (RTCCON0 & ~(BIT(8) | BIT(9))) | BIT(9);		//sniff rc
        SENSHBCON |= BIT(5+0*4) * 1 | BIT(3+0) | BIT(SENSHB_LPCLK_EN_SHF);
//        bsp_sensor_hub_clk_init();
//        bsp_sensor_hub_io_init(SENSHBMAP_Gx);
        SENSHBCON = BIT(SENSHB_IIC_MAP_SHF) * port_map | BIT(SENSHB_IIC_EN_SHF) | BIT(SENSHB_LPCLK_EN_SHF);
    } else {
        if (HW_IIC == &HW_IIC1) {
//            CLKCON2 |= BIT(2);         //x24m_clkdiv6
            CLKGAT0 |= BIT(28);        //en iic1 clk
            RSTCON0 |= BIT(7);         //Release IIC1
            HW_IIC->map->FUNCMCONx = (0xf << 12);
            HW_IIC->map->FUNCMCONx = (port_map << 12);
        } else if (HW_IIC == &HW_IIC2) {
//            CLKCON2 |= BIT(3);          //x24m_clkdiv6
            CLKGAT0 |= BIT(29);         //en iic2 clk
            RSTCON0 |= BIT(11);         //Release IIC2
            HW_IIC->map->FUNCMCONx = (0xf << 16);
            HW_IIC->map->FUNCMCONx = (port_map << 16);
        }
        delay_us(10);
    }

    u8 iic_io[] = {port_scl, port_sda};
    for (int i = 0; i < 2; i++) {
        gpio_t gpio;
        bsp_gpio_cfg_init(&gpio, iic_io[i]);
        if (gpio.sfr == NULL) {
            return;
        }
        gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
        gpio.sfr[GPIOxPU] |= BIT(gpio.num);
        gpio.sfr[GPIOxDE] |= BIT(gpio.num);
        if (i2cx_idx == 0) {
            gpio.sfr[GPIOxFEN] &= ~BIT(gpio.num);
//            gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
        } else {
            gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
        }
    }

    /*
        IIC速率计算方式: 时钟频率/分频数
        时钟频率：CLK = CLKCON1 |= BIT(7);        x26m_clkdiv8 24M时钟频率 分频数为8 则时钟频率为3M
        分频数：  DIV = IICxCON0[9:4];            在BIT(4)和BIT(9)之间填参 最小值=0;最大值=63
        IIC速率： S = CLK / (DIV + 1);            3M / (20 + 1) ~= 142KHz
     */

    HW_IIC->sfr->IICxCON0 =     1 << 0 |     //IIC EN
                                0 << 1 |     //IIC INT
                                0 << 2 |     //IIC HOLD CNT [3:2]
                                15 << 4 |    //IIC POSDIV [9:4]
                                1 << 10;     //IIC WSCL_OPT

    delay_5ms(8);
}

AT(.text.bsp.i2c)
static void i2cx_deinit(u8 i2cx_idx, u8 port_scl, u8 port_sda, u8 port_map)
{
    i2c_t *HW_IIC = bsp_i2c_get_register(i2cx_idx);
    if (HW_IIC == &HW_IIC0) {
        CLKGAT1 &= ~BIT(3);
        RTCCON0 &= ~BIT(0);      //RTC en
        RTCCON0 &= ~BIT(2);      //RTC clk to module
    //    RTCCON0 &= ~BIT(22);     //SNF_RC_EN
        RTCCON0 &= ~BIT(20);     //SNF_RC_EN
        RTCCON0 = (RTCCON0 & ~(BIT(14) | BIT(15)));	//sniff rc
        RTCCON0 = (RTCCON0 & ~(BIT(8) | BIT(9))) | BIT(8);		//sniff rc
        SENSHBCON &= ~(BIT(5+0*4) * 1 | BIT(3+0) | BIT(SENSHB_LPCLK_EN_SHF));
//        bsp_sensor_hub_clk_init();
//        bsp_sensor_hub_io_init(SENSHBMAP_Gx);
        SENSHBCON &= ~(BIT(SENSHB_IIC_MAP_SHF) * port_map | BIT(SENSHB_IIC_EN_SHF) | BIT(SENSHB_LPCLK_EN_SHF));
    } else {
        if (HW_IIC == &HW_IIC1) {
//            CLKCON2 &= ~BIT(2);         //x24m_clkdiv6
            CLKGAT0 &= ~BIT(28);        //en iic1 clk
            RSTCON0 &= ~BIT(7);         //Release IIC1
            HW_IIC->map->FUNCMCONx = (0xf << 12);
//            HW_IIC->map->FUNCMCONx = (port_map << 12);
        } else if (HW_IIC == &HW_IIC2) {
            CLKCON2 &= ~BIT(3);          //x24m_clkdiv6
            CLKGAT0 &= ~BIT(29);         //en iic2 clk
            RSTCON0 &= ~BIT(11);         //Release IIC2
            HW_IIC->map->FUNCMCONx = (0xf << 16);
//            HW_IIC->map->FUNCMCONx = (port_map << 16);
        }
        delay_us(10);
    }
    u8 iic_io[] = {port_scl, port_sda};
    for (int i = 0; i < 2; i++) {
        gpio_t gpio;
        bsp_gpio_cfg_init(&gpio, iic_io[i]);
        if (gpio.sfr == NULL) {
            return;
        }
        gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
        gpio.sfr[GPIOxPU] &= ~BIT(gpio.num);
        gpio.sfr[GPIOxDE] |= BIT(gpio.num);
        gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
    }

    HW_IIC->sfr->IICxCON0 = 0;
    delay_5ms(8);
}

AT(.text.bsp.i2c)
static bool bsp_i2c_check(u8 i2cx_idx)
{
    switch (i2cx_idx) {
    case 0:
        return !is_i2c0_init;
    case 1:
        return !is_i2c1_init;
    case 2:
        return !is_i2c2_init;
    }
    return false;
}

AT(.text.bsp.i2c)
static void bsp_i2cx_is_init_set(u8 i2cx_idx, bool flags)
{
    switch (i2cx_idx) {
    case 0:
        is_i2c0_init = flags;
        break;
    case 1:
        is_i2c1_init = flags;
        break;
    case 2:
        is_i2c2_init = flags;
        break;
    default:
        break;
    }
}

#endif



AT(.text.bsp.i2c)
void bsp_i2c_init(u8 i2cx_idx, u8 port_scl, u8 port_sda, u8 port_map)
{
#if I2C_SW_EN
    I2C_SDA_SCL_OUT();
    I2C_SDA_H();
    delay_5ms(2);
#endif

#if I2C_HW_EN
    if (bsp_i2c_check(i2cx_idx)) {
        i2cx_init(i2cx_idx, port_scl, port_sda, port_map);
#if !SENSOR_HUB_EN
//        bsp_i2c_irq_init(i2cx_idx);
#endif
        bsp_i2cx_is_init_set(i2cx_idx, true);
    }
#endif
}

AT(.text.bsp.i2c)
void bsp_i2c_deinit(u8 i2cx_idx, u8 port_scl, u8 port_sda, u8 port_map)
{
#if I2C_SW_EN
#endif

#if I2C_HW_EN
    if (bsp_i2c_check(i2cx_idx) == false) {
        i2cx_deinit(i2cx_idx, port_scl, port_sda, port_map);
        bsp_i2cx_is_init_set(i2cx_idx, false);
    }
#endif
}



