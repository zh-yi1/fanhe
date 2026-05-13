#include "include.h"


#if SCTP_SELECT
#define TRACE_EN            1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//客户触摸的i2c初始化
#define SCTP_PORT_SCL            IO_PE8
#define SCTP_PORT_SDA            IO_PE7

#define SCTP_PORT_INT1            IO_PE10
#define SCTP_PORT_INT1_VECTOR     PORT_INT5_VECTOR
#define SCTP_PORT_RST1            SCTP_PORT_INT1

//#define SCTP_PORT_RST_H()        port_gpio_set_in(SCTP_PORT_RST, GPIOxPU)//GPIOBSET = BIT(7)
//#define SCTP_PORT_RST_L()        port_gpio_set_in(SCTP_PORT_RST, GPIOxPD)//GPIOBCLR = BIT(7);
#define SCTP_PORT_MAP_GPIO_GX    1
#define SCTP_USE_I2CX            0
#define SCTP_USE_HW_TIMER        HW_TIMER1

#define CUM_I2C_WRITE_ADDR(ADDR)     		 ((ADDR) << 1)				//CTP IIC写地址
#define CUM_I2C_READ_ADDR(ADDR)      		 ((ADDR) << 1 | 1)			//CTP IIC读地址

///连续写寄存器
AT(.com_text.ctp)
bool sctp_iic_write_regs(u16 dev_addr, u32 reg_addr, u8 *wbuf, u16 wlen, bool stop)
{
    return bsp_hw_i2c_dma_write_buf(SCTP_USE_I2CX, dev_addr, reg_addr, wbuf, wlen, stop);
}

///连续读寄存器
AT(.com_text.ctp)
bool sctp_iic_read_regs(u16 dev_addr, u32 reg_addr, u8 *wbuf, u16 wlen, u8 *rbuf, u16 rlen)
{
    return bsp_hw_i2c_dma_read_buf(SCTP_USE_I2CX, dev_addr, reg_addr, wbuf, wlen, rbuf, rlen);
}

AT(.com_text.ctp)
void sctp_int_isr(void)
{
    if (WKUPEDG & BIT(16+SCTP_PORT_INT1_VECTOR)) {
        WKUPCPND = BIT(16+SCTP_PORT_INT1_VECTOR);
#if (SCTP_SELECT == SCTP_RM1002B)
    void sctp_rm1002b_int_handler(void);
    sctp_rm1002b_int_handler();
#else
    #error SCTP_SELECT err!!
#endif

    }
}

AT(.com_text.ctp)
//void sctp_timer_isr(co_timer_t *timer, void *param)
void sctp_timer_isr(void)
{
#if (SCTP_SELECT == SCTP_RM1002B)
    void rm1002_timer_handler(void);
    rm1002_timer_handler();
#else
    #error SCTP_SELECT err!!
#endif


}

//static co_timer_t sctp_timer;
void sctp_init(void)
{
    lcd_pg_on();
    bsp_i2c_init(SCTP_USE_I2CX, SCTP_PORT_SCL, SCTP_PORT_SDA, SCTP_PORT_MAP_GPIO_GX);

#if (SCTP_SELECT == SCTP_RM1002B)
    port_gpio_set_in(SCTP_PORT_RST1, GPIOxPU);
    bool sctp_rm1002b_init(void);
    sctp_rm1002b_init();
#else
    #error SCTP_SELECT err!!
#endif

    //INT信号中断
    port_irq_register(SCTP_PORT_INT1_VECTOR, sctp_int_isr);
    port_wakeup_init(SCTP_PORT_INT1, 1, 1);           //开内部上拉, 下降沿唤醒

//    sys_irq_init(IRQ_I2C_VECTOR, 0, sctp_timer_isr);
//    i2c_t* CTP_IIC =  bsp_i2c_get_register(SCTP_USE_I2CX);
//    CTP_IIC->sfr->IICxCON0 |= BIT(1);                             //IIC INT EN

    //定时器
    bsp_hw_timer_set(SCTP_USE_HW_TIMER, 10000, sctp_timer_isr);
//    co_timer_set(&sctp_timer, 1, TIMER_REPEAT, LEVEL_HIGH_PRI, sctp_timer_isr, NULL);

}

void sctp_exit(void)
{
//    co_timer_del(&sctp_timer);
    bsp_hw_timer_del(SCTP_USE_HW_TIMER);
    bsp_i2c_deinit(SCTP_USE_I2CX, SCTP_PORT_SCL, SCTP_PORT_SDA, SCTP_PORT_MAP_GPIO_GX);
    port_irq_free(SCTP_PORT_INT1_VECTOR);
    port_wakeup_exit(SCTP_PORT_INT1);
}
#endif
