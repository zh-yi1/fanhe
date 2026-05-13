#include "include.h"

#if BSP_UART_EN

#define TRACE_EN                0       // 是否打开调试信息
#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

#if TRACE_EN
static char uart1_isr_str[] = "bsp_uart1_isr: buf[%x] tick[%d] w_cnt[%d]\n";
#endif // TRACE_EN

#if BSP_UART1_EN
bsp_uart_t bsp_uart1;
AT(.com_text.uart)
void bsp_uart1_isr(uint8_t *buf, uint32_t len)
{
//    TRACE(uart1_isr_str, buf[1] << 8 | buf[0], tick_get(), bsp_uart1.w_cnt);

    if (tick_check_expire(bsp_uart1.ticks, 300)) {  //超时处理数据, 直接清空buf
        bsp_uart1.w_cnt = bsp_uart1.r_cnt = 0;
    }
    bsp_uart1.ticks = tick_get();

    for (u8 i = 0; i < len; i++) {
        bsp_uart1.rxbuf[bsp_uart1.w_cnt] = *buf;
        bsp_uart1.w_cnt = (bsp_uart1.w_cnt + 1) % bsp_uart1.rxbuf_len;
        buf++;
    }

    if (bsp_uart1.rx_isr) {
        bsp_uart1.rx_isr(buf, len);
    }
}

///TX
void bsp_uart1_str_tx(char *str)
{
    uart_str_tx(UART_TYPE_1, str);
}

void bsp_uart1_putchar(char ch)
{
//    TRACE("%s: ch[%x]\n", __func__, ch);
    uart_buf_tx(UART_TYPE_1, ch);
}

///RX
AT(.com_text.uart)
u8 bsp_uart1_get_char(u8 *ch)
{
    if (bsp_uart1.rx_isr) {
        if (bsp_uart1.r_cnt != bsp_uart1.w_cnt) {
            *ch = bsp_uart1.rxbuf[bsp_uart1.r_cnt];
            bsp_uart1.r_cnt = (bsp_uart1.r_cnt + 1) % bsp_uart1.rxbuf_len;
            return 1;
        }
    } else {                    //loop mode
        if (huart_get_rxcnt()) {
            *ch = huart_getchar();
            return 1;
        }
    }
    return 0;
}

AT(.com_text.uart)
u16 bsp_uart1_rxcnt_get(void)
{
    u16 rx_len = 0;
    if (bsp_uart1.w_cnt > bsp_uart1.r_cnt) {
        rx_len = bsp_uart1.w_cnt - bsp_uart1.r_cnt;
    } else {
        rx_len = bsp_uart1.w_cnt + (bsp_uart1.rxbuf_len - bsp_uart1.r_cnt);
    }
    return rx_len;
}

void bsp_uart1_rxclr(void)
{
    bsp_uart1.w_cnt = bsp_uart1.r_cnt = 0;
}

///config
void bsp_uart1_set_baud(u32 baud)
{
    uart_baud_set(UART_TYPE_1, baud);
}

Err_Uart bsp_uart1_init(uart_t *uart, u8 *rxbuf, u16 rxbuf_len)
{
    memset(&bsp_uart1, 0, sizeof(bsp_uart1));
    if (uart->rx_isr) {
        bsp_uart1.rx_isr = uart->rx_isr;
        uart->rx_isr = bsp_uart1_isr;
    }
    if (rxbuf && rxbuf_len) {
        bsp_uart1.rxbuf = rxbuf;
        bsp_uart1.rxbuf_len = rxbuf_len;
    }
    return uart_init(uart);
}

Err_Uart bsp_uart1_cfg_init(uart_cfg_t *uart_cfg, u8 *rxbuf, u16 rxbuf_len)
{
    memset(&bsp_uart1, 0, sizeof(bsp_uart1));
    if (uart_cfg->rx_isr) {
        bsp_uart1.rx_isr = uart_cfg->rx_isr;
        uart_cfg->rx_isr = bsp_uart1_isr;
    }
    if (rxbuf && rxbuf_len) {
        bsp_uart1.rxbuf = rxbuf;
        bsp_uart1.rxbuf_len = rxbuf_len;
    }
    return uart_cfg_init(uart_cfg);
}
#endif  // BSP_UART1_EN

#if UART1_EXAMPLE_EN
static u8 uart1_example_rxbuf[64];
AT(.com_text.uart)
void bsp_uart1_example_isr(uint8_t *buf, uint32_t len)
{
//    TRACE(uart_isr_str, buf, tick_get());
//    TRACE_R(buf, len);
}

void bsp_uart1_example_process(void)
{
    u8 ch;
    while (bsp_uart1_get_char(&ch)) {
        WDT_CLR();
        printf("rx ch[%x]\n", ch);
    }

    static u32 ticks = 0;
    if (tick_check_expire(ticks, 1000)) {
        ticks = tick_get();
        printf("send 123\n");
        bsp_uart1_str_tx("123");
    }
}

void bsp_uart1_example_init(void)
{
#if 1   //手动指定MAP示例
    uart_t uart1;
    memset(&uart1, 0x00, sizeof(uart_t));

    uart1.type = UART_TYPE_1;

//    //Example: TX: PB8, RX: PB9
//    uart1.tx_map = UT1TXMAP_G2_PB8;
//    uart1.rx_map = UT1RXMAP_G2_PB9;

//    //Example: crossbar TX: PB8, RX: PB9
//    uart1.tx_map = UT1TXMAP_OUT_CH;
//    uart1.rx_map = UT1RXMAP_G7_IN_CH2;
//    uart1.tx_crossbar_ch = 2;
//    uart1.tx_crossbar = CHxOUTMAP_PB8;
//    uart1.rx_crossbar = CHxINSEL_PB9;

//    //Example: TRX: PB8
//    uart1.tx_map = UT1TXMAP_G2_PB8;
//    uart1.rx_map = UT1RXMAP_G1_MAP_TO_TX;

    //Example: crossbar, IN_CH = OUT_CH = 2
    uart1.tx_map = UT1TXMAP_OUT_CH;
    uart1.rx_map = UT1RXMAP_G7_IN_CH2;
    uart1.tx_crossbar_ch = 2;
    //TX: PB8, RX: PB9
    uart1.tx_crossbar = CHxOUTMAP_PB8;
    uart1.rx_crossbar = CHxINSEL_PB9;
//    //TRX: PB8
//    uart1.tx_crossbar = CHxOUTMAP_PB8;
//    uart1.rx_crossbar = CHxINSEL_PB8;

//    //Example: TRX: VUSB
//    uart1.tx_map = UT1TXMAP_G1_VUSB;
//    uart1.rx_map = UT1RXMAP_G1_MAP_TO_TX;

    uart1.baud = 9600;
    uart1.rx_isr = bsp_uart1_example_isr;
    bsp_uart1_init(&uart1, uart1_example_rxbuf, sizeof(uart1_example_rxbuf));

#else   //自动MAP查询示例
    uart_cfg_t uart1_cfg;
    memset(&uart1_cfg, 0x00, sizeof(uart_t));

    uart1_cfg.type = UART_TYPE_1;
//    //Example: TX: PB8, RX: PB9
//    uart1_cfg.tx_io = IO_PB8;
//    uart1_cfg.rx_io = IO_PB9;

//    //Example: TRX: PB8
//    uart1_cfg.tx_io = IO_PB8;
//    uart1_cfg.rx_io = IO_PB8;

//    //Example: TRX: VUSB
//    uart1_cfg.tx_io = IO_VUSB;
//    uart1_cfg.rx_io = IO_VUSB;

//    //Example: crossbar, IN_CH = OUT_CH = 2
//    //TX: PB8, RX: PB9
//    uart1_cfg.tx_io = IO_PB8;
//    uart1_cfg.rx_io = IO_PB9;
////    //TRX: PB8
////    uart1_cfg.tx_io = IO_PB8;
////    uart1_cfg.rx_io = IO_PB8;
//    uart1_cfg.crossbar_en = 1;
//    uart1_cfg.tx_crossbar_ch = 2;
//    uart1_cfg.rx_crossbar_ch = 2;

    uart1_cfg.baud = 9600;
    uart1_cfg.rx_isr = bsp_uart1_example_isr;
    bsp_uart1_cfg_init(&uart1_cfg, uart1_example_rxbuf, sizeof(uart1_example_rxbuf));
#endif
}
#endif // UART1_EXAMPLE_EN

void bsp_uart_init(void)
{
#if UART1_EXAMPLE_EN
    bsp_uart1_example_init();
    while(1) {
        WDT_CLR();
        bsp_uart1_example_process();
    }
#endif
}
#endif  //BSP_UART_EN
