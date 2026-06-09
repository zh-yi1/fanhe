/**
 * @file    bsp_uart.c
 * @brief   BSP 层 UART1 收发管理实现
 * @note    ISR 中将硬件收到的字节写入环形缓冲，主循环通过 bsp_uart1_get_char()
 *          非阻塞取数；也可选配用户 rx_isr 做额外通知。
 */

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

/**
 * @brief UART1 内部接收 ISR
 *
 * 由底层 uart 驱动在中断上下文调用，职责：
 * 1. 超时（300ms 无新数据）则清空环形缓冲
 * 2. 将本次收到的 len 字节逐字节写入 rxbuf
 * 3. 若用户注册了 rx_isr，在末尾转发通知
 *
 * @param[in] buf  本次中断收到的数据首地址
 * @param[in] len  本次中断收到的字节数
 */
AT(.com_text.uart)
void bsp_uart1_isr(uint8_t *buf, uint32_t len)
{
    if (tick_check_expire(bsp_uart1.ticks, 300)) {  // 超时未续帧，丢弃旧数据
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

//-----------------------------------------------------------------------------
// 发送
//-----------------------------------------------------------------------------

/** @brief 发送以 '\0' 结尾的字符串 */
void bsp_uart1_str_tx(char *str)
{
    uart_str_tx(UART_TYPE_1, str);
}

/** @brief 发送单字节 */
void bsp_uart1_putchar(char ch)
{
//    TRACE("%s: ch[%x]\n", __func__, ch);
    uart_buf_tx(UART_TYPE_1, ch);
}

//-----------------------------------------------------------------------------
// 接收
//-----------------------------------------------------------------------------

/**
 * @brief 从环形缓冲非阻塞读取一字节
 * @see bsp_uart.h::bsp_uart1_get_char
 */
AT(.com_text.uart)
u8 bsp_uart1_get_char(u8 *ch)
{
    if (bsp_uart1.rx_isr) {
        if (bsp_uart1.r_cnt != bsp_uart1.w_cnt) {
            *ch = bsp_uart1.rxbuf[bsp_uart1.r_cnt];
            bsp_uart1.r_cnt = (bsp_uart1.r_cnt + 1) % bsp_uart1.rxbuf_len;
            return 1;
        }
    } else {                    // 未启用缓冲管理，直接轮询硬件 FIFO
        if (huart_get_rxcnt()) {
            *ch = huart_getchar();
            return 1;
        }
    }
    return 0;
}

/**
 * @brief 获取环形缓冲中尚未读取的字节数
 * @return 待读字节数，0 表示空
 */
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

/** @brief 清空环形缓冲读写指针 */
void bsp_uart1_rxclr(void)
{
    bsp_uart1.w_cnt = bsp_uart1.r_cnt = 0;
}

//-----------------------------------------------------------------------------
// 配置 / 初始化
//-----------------------------------------------------------------------------

/** @brief 运行时修改 UART1 波特率 */
void bsp_uart1_set_baud(u32 baud)
{
    uart_baud_set(UART_TYPE_1, baud);
}

/**
 * @brief 初始化 UART1 并挂载环形缓冲
 *
 * rx_isr 链式替换：用户回调存于 bsp_uart1.rx_isr，底层实际注册 bsp_uart1_isr。
 */
Err_Uart bsp_uart1_init(uart_t *uart, u8 *rxbuf, u16 rxbuf_len)
{
    memset(&bsp_uart1, 0, sizeof(bsp_uart1));
    if (uart->rx_isr) {
        bsp_uart1.rx_isr = uart->rx_isr;   // 保存用户回调
        uart->rx_isr = bsp_uart1_isr;      // 底层改为调用 BSP 内部 ISR
    }
    if (rxbuf && rxbuf_len) {
        bsp_uart1.rxbuf = rxbuf;
        bsp_uart1.rxbuf_len = rxbuf_len;
    }
    return uart_init(uart);
}

/**
 * @brief 通过 uart_cfg_t 结构初始化 UART1（自动引脚映射方式）
 *
 * 与 bsp_uart1_init 类似，适用于 IO 自动查询 crossbar 的场景。
 */
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

/** @brief 示例用空 ISR，可按需在此打印或处理原始数据 */
AT(.com_text.uart)
void bsp_uart1_example_isr(uint8_t *buf, uint32_t len)
{
//    TRACE(uart_isr_str, buf, tick_get());
//    TRACE_R(buf, len);
}

/**
 * @brief 示例主循环：回显收到的字符，并对 A/B 等键做简单分支演示
 */
void bsp_uart1_example_process(void)
{
    u8 ch;
    while (bsp_uart1_get_char(&ch)) {
        WDT_CLR();
        printf("rx ch[%x]\n", ch);

        bsp_uart1_putchar(ch);

        // if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
        //     switch (ch) {
        //         printf("%s:%d\n", __func__, __LINE__);
        //     case 'A':
        //         printf("1\n");
        //         printf("%s:%d\n", __func__, __LINE__);
        //         break;

        //     case 'B':
        //         printf("2\n");
        //         printf("%s %d\n", __func__, __LINE__);
        //         break;

        //     default:
        //         printf("3\n");
        //         printf("%s %d\n", __func__, __LINE__);
        //         break;
        //     }
        // }
    }

    static u32 ticks = 0;
    if (tick_check_expire(ticks, 1000)) {
        ticks = tick_get();
        printf("send 123\n");
        bsp_uart1_str_tx("123");
    }
}

/**
 * @brief 示例初始化：PB8(TX) / PB9(RX)，9600 波特率
 *
 * 文件中保留了 crossbar、单线 TRX、VUSB 等多种引脚配置示例（注释块），
 * 可按硬件连接取消对应注释切换。
 */
void bsp_uart1_example_init(void)
{
#if 1   // 手动指定 MAP 示例
    uart_t uart1;
    memset(&uart1, 0x00, sizeof(uart_t));

    uart1.type = UART_TYPE_1;

//    //Example: TX: PB8, RX: PB9
    uart1.tx_map = UT1TXMAP_G2_PB8;
    uart1.rx_map = UT1RXMAP_G2_PB9;

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
    // uart1.tx_map = UT1TXMAP_OUT_CH;
    // uart1.rx_map = UT1RXMAP_G7_IN_CH2;
    // uart1.tx_crossbar_ch = 2;
    // //TX: PB8, RX: PB9
    // uart1.tx_crossbar = CHxOUTMAP_PB8;
    // uart1.rx_crossbar = CHxINSEL_PB9;
//    //TRX: PB8
//    uart1.tx_crossbar = CHxOUTMAP_PB8;
//    uart1.rx_crossbar = CHxINSEL_PB8;

//    //Example: TRX: VUSB
//    uart1.tx_map = UT1TXMAP_G1_VUSB;
//    uart1.rx_map = UT1RXMAP_G1_MAP_TO_TX;

    uart1.baud = 9600;
    uart1.rx_isr = bsp_uart1_example_isr;
    bsp_uart1_init(&uart1, uart1_example_rxbuf, sizeof(uart1_example_rxbuf));

#else   // 自动 MAP 查询示例
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

/** @brief BSP UART 模块入口，开启示例时进入阻塞演示循环 */
void bsp_uart_init(void)
{
#if UART1_EXAMPLE_EN
    bsp_uart1_example_init();
    while (1) {
        WDT_CLR();
        bsp_uart1_example_process();
    }
#endif
}

#endif  // BSP_UART_EN
