#include "include.h"

#if BSP_SPI_EN

#define TRACE_EN                0       // 是否打开调试信息
#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

//--------------------------------------------------------------------------
#if BSP_SPI1_EN
bsp_spi_t bsp_spi1;

AT(.com_text.bsp_spi1)
void bsp_spi1_isr(void)
{
    if (bsp_spi1.isr) {
        bsp_spi1.isr();
    }
    bsp_spi1.busy = 0;
}

AT(.com_text.bsp_spi)
void bsp_spi1_wait(void)
{
    u32 ticks = tick_get();

    if (bsp_spi1.isr) { //等中断
        while (bsp_spi1.busy) {
            if (tick_check_expire(ticks, 500)) {
                printf("%s: timeout\n", __func__);
                break;
            }
        };
    } else {            //轮询pending
        while (1) {
            if (REG_GET_POS(SPI1CON, SPIxCON_SPIPND)) {
                REG_SET_BIT(SPI1CPND, SPIxCPND_SPICPND);
                break;
            }
            if (tick_check_expire(ticks, 500)) {
                printf("%s: timeout\n", __func__);
                break;
            }
        };
    }
}

///TX
Err_Spi bsp_spi1_buf_tx(u8 buf)
{
    TRACE("%s\n", __func__);
    if (bsp_spi1.isr) {
        return ERR_SPI_DMA_MODE_NO_USE_BUF;
    }
    return spi_buf_tx(SPI_TYPE_1, buf);
}

Err_Spi bsp_spi1_dma_tx(u8 *buf, u32 len)
{
    TRACE("%s\n", __func__);
    bsp_spi1.busy = 1;
    return spi_dma_tx(SPI_TYPE_1, buf, len);
}

///RX
Err_Spi bsp_spi1_buf_rx(u8 *buf)
{
    return spi_buf_rx(SPI_TYPE_1, buf);
}

Err_Spi bsp_spi1_dma_rx(u8 *buf, u32 len)
{
    if (buf == NULL) {
        return ERR_SPI_BUF;
    }

    if (len == 0) {
        return ERR_SPI_LEN;
    }

    TRACE("%s: buf[%p] len[%d]\n", __func__, buf, len);
    bsp_spi1.rxbuf = buf;
    bsp_spi1.rxbuf_len = len;
    bsp_spi1.busy = 1;
    return spi_dma_rx(SPI_TYPE_1, buf, len);
}

AT(.com_text.uart)
u16 bsp_spi1_rxcnt_get(u8 **rxbuf)
{
    u16 rxbuf_len = 0;
    if (bsp_spi1.rxbuf_len && (bsp_spi1.busy == 0)) {
        *rxbuf = bsp_spi1.rxbuf;
        rxbuf_len = bsp_spi1.rxbuf_len;
        bsp_spi1.rxbuf_len = 0;
    }
    return rxbuf_len;
}

Err_Spi bsp_spi1_init(spi_t *spi)
{
    memset(&bsp_spi1, 0, sizeof(bsp_spi1));
    if (spi->isr) {
        bsp_spi1.isr = spi->isr;
        spi->isr = bsp_spi1_isr;
    }
    return spi_init(spi);
}
#endif // BSP_SPI1_EN

//--------------------------------------------------------------------------
#if EXAMPLE_SPI1_EN
#define EXAMPLE_SPI_CS_IO           IO_PA0
#define EXAMPLE_SPI_ISR_EN          1   // 是否使能中断, 否则使用buf loop模式
static u8 example_spi1_dma_txbuf[32];   // DMA发送数据缓冲区
static u8 example_spi1_dma_rxbuf[32];   // DMA接收数据缓冲区
#if TRACE_EN
AT(.com_text.example)
char spi_tx_str[] = "example_spi_isr\n";
#endif

AT(.com_text.bsp_spi)
void example_spi_isr(void)
{
//    TRACE(spi_tx_str);  //开了会复位
}

void bsp_spi_example_process(void)
{
    static u32 ticks = 0;
    if (tick_check_expire(ticks, 1000)) {
        ticks = tick_get();

        spi_cs_en(EXAMPLE_SPI_CS_IO);
        memset(example_spi1_dma_txbuf, 0x11, 10);
        memset(example_spi1_dma_rxbuf, 0x88, 10);
#if EXAMPLE_SPI_ISR_EN
        printf("spi tx: 11 11 11\n");
        bsp_spi1_wait();
        bsp_spi1_dma_tx(example_spi1_dma_txbuf, 3);
        printf("spi tx done\n");
        bsp_spi1_wait();
        bsp_spi1_dma_rx(example_spi1_dma_rxbuf, 3);
#else
       spi_cs_en(EXAMPLE_SPI_CS_IO);
//       //buf mode
//        u8 data = 0x11;
//        printf("spi tx: 11\n");
//        bsp_spi1_buf_tx(data);
//        data = 0x88;
//        bsp_spi1_buf_rx(&data);
//        printf("spi rx: %x\n", data);

        //dma mode
        printf("spi tx: 11 11 11\n");
        bsp_spi1_dma_tx(example_spi1_dma_txbuf, 3);
        bsp_spi1_wait();
        bsp_spi1_dma_rx(example_spi1_dma_rxbuf, 3);
        bsp_spi1_wait();
        printf("spi rx:");
        print_r(example_spi1_dma_rxbuf, 3);
        spi_cs_dis(EXAMPLE_SPI_CS_IO);
#endif
    }

#if EXAMPLE_SPI_ISR_EN
    u8 *rxbuf = NULL;
    u8 rxbuf_len = bsp_spi1_rxcnt_get(&rxbuf);
    if (rxbuf && rxbuf_len) {
        printf("spi rx[%p %d]:", rxbuf, rxbuf_len);
        print_r(rxbuf, rxbuf_len);
        spi_cs_dis(EXAMPLE_SPI_CS_IO);
    }
#endif
}

void bsp_spi_example_init(void)
{
    TRACE("%s\n", __func__);

    spi_t spi1;
    memset(&spi1, 0, sizeof(spi_t));

    spi1.type = SPI_TYPE_1;
    //Example: G1: CLK(PA4), DO/D0(PA3), DI/D1(PA1), D2(PA0), D3/HOLD(PA5)
    spi1.map = SPI1MAP_G1;

//    spi1.type = SPI_TYPE_2;
//    //Example: G3: CLK(PB9),  DO(PB10), DI(PB8)
//    spi1.map = SPI2MAP_G3;

    spi1.cs = EXAMPLE_SPI_CS_IO;
    spi1.clk_hz = 4000000;
    spi1.bus = BUSMODE_1BIT_IN_1BIT_OUT;
#if EXAMPLE_SPI_ISR_EN
    spi1.isr = example_spi_isr;
#endif

    u8 ret = bsp_spi1_init(&spi1);
    if (ret) {
        printf("%s: spi_init err[%d]\n", __func__, ret);
    }
}
#endif // EXAMPLE_SPI1_EN
//--------------------------------------------------------------------------

void bsp_spi_init(void)
{
#if EXAMPLE_SPI1_EN
    bsp_spi_example_init();
    while(1) {
        WDT_CLR();
        bsp_spi_example_process();
    }
#endif
}
#endif // BSP_SPI_EN

