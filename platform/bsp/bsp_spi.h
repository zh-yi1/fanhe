#ifndef _BSP_SPI_H
#define _BSP_SPI_H

#define EXAMPLE_SPI1_EN         0       // SPI1测试用例使能

// 是否打开SPI1收发管理器
#define BSP_SPI1_EN             (EXAMPLE_SPI1_EN)

// 是否打开SPI软件适配层
#define BSP_SPI_EN              BSP_SPI1_EN

typedef struct {
    volatile u8 busy;       //收发防止冲突
    spi_isr_t isr;       	//发送/接收完成中断回调函数
    u8 *rxbuf;
    u16 rxbuf_len;
} bsp_spi_t;
extern bsp_spi_t bsp_spi1;

void bsp_spi_init(void);
#endif
