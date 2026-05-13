#include "include.h"


#if ECTP_SELECT
#define TRACE_EN            1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//客户触摸的i2c初始化
#define SCTP_PORT_SCL            IO_PE8
#define SCTP_PORT_SDA            IO_PE7

#define SCTP_PORT_INT1            IO_PE9
#define SCTP_PORT_INT1_VECTOR     PORT_INT2_VECTOR
#define SCTP_PORT_RST1            SCTP_PORT_INT1

//#define SCTP_PORT_RST_H()        port_gpio_set_in(SCTP_PORT_RST, GPIOxPU)//GPIOBSET = BIT(7)
//#define SCTP_PORT_RST_L()        port_gpio_set_in(SCTP_PORT_RST, GPIOxPD)//GPIOBCLR = BIT(7);
#define SCTP_PORT_MAP_GPIO_GX    1
#define SCTP_USE_I2CX            0

#define CUM_I2C_WRITE_ADDR(ADDR)     		 ((ADDR) << 1)				//CTP IIC写地址
#define CUM_I2C_READ_ADDR(ADDR)      		 ((ADDR) << 1 | 1)			//CTP IIC读地址

///连续写寄存器
AT(.com_text.ctp)
bool ectp_iic_write_regs(u16 dev_addr, u32 reg_addr, u8 *wbuf, u16 wlen, bool stop)
{
    return bsp_hw_i2c_dma_write_buf(SCTP_USE_I2CX, dev_addr, reg_addr, wbuf, wlen, stop);
}

///连续读寄存器
AT(.com_text.ctp)
bool ectp_iic_read_regs(u16 dev_addr, u32 reg_addr, u8 *wbuf, u16 wlen, u8 *rbuf, u16 rlen)
{
    return bsp_hw_i2c_dma_read_buf(SCTP_USE_I2CX, dev_addr, reg_addr, wbuf, wlen, rbuf, rlen);
}

AT(.com_text.ctp)
static void sctp_int_isr(void)
{
    if (WKUPEDG & BIT(16+SCTP_PORT_INT1_VECTOR)) {
        WKUPCPND = BIT(16+SCTP_PORT_INT1_VECTOR);
#if (ECTP_SELECT == ECTP_RM1015B)
    void rm1015B_irq_handler(void);
    rm1015B_irq_handler();
#else
    #error ECTP_SELECT err!!
#endif

    }
}

//static co_timer_t sctp_timer;
void ectp_init(void)
{
    lcd_pg_on();
    bsp_i2c_init(SCTP_USE_I2CX, SCTP_PORT_SCL, SCTP_PORT_SDA, SCTP_PORT_MAP_GPIO_GX);

//    while(1) WDT_CLR();

#if (ECTP_SELECT == ECTP_RM1015B)
    port_gpio_set_in(SCTP_PORT_RST1, GPIOxPU);
    bool rm1015B_init(void);
    rm1015B_init();
#else
    #error ECTP_SELECT err!!
#endif

    //INT信号中断
    port_irq_register(SCTP_PORT_INT1_VECTOR, sctp_int_isr);
    port_wakeup_init(SCTP_PORT_INT1, 1, 1);           //开内部上拉, 下降沿唤醒

}

void ectp_exit(void)
{
    bsp_i2c_deinit(SCTP_USE_I2CX, SCTP_PORT_SCL, SCTP_PORT_SDA, SCTP_PORT_MAP_GPIO_GX);
    port_irq_free(SCTP_PORT_INT1_VECTOR);
    port_wakeup_exit(SCTP_PORT_INT1);
}
#endif
