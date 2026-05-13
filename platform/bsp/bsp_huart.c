#include "include.h"

#define HUART_EXAMPLE_EN  		    0       // 高速串口测试用例使能
#define HUART_EXAMPLE_ISR_EN  	    1       // 高速串口测试用例中断使能, 否则使用buf loop模式

#define TRACE_EN                    0       // 是否打开调试信息

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

#if TRACE_EN
char huart_rx_done_str[] = "rx len[%d]\n";
char huart_tx_done_str[] = "tx done\n";
#endif // TRACE_EN

#if HUART_EXAMPLE_EN
static u8 huart_dma_tx_buf[64];
static u8 huart_dma_rx_buf[64];
#if HUART_EXAMPLE_ISR_EN
AT(.com_text.bsp_huart)
void bsp_huart_example_tx_isr(void)
{
    TRACE(huart_tx_done_str);
}

AT(.com_text.bsp_huart)
void bsp_huart_example_rx_isr(uint8_t *buf, uint32_t len)
{
    TRACE(huart_rx_done_str);
    TRACE_R(buf, len);
}
#endif

void bsp_huart_example_process(void)
{
#if TRACE_EN
    static u32 ticks = 0;
    if (tick_check_expire(ticks, 1000)) {
        ticks = tick_get();
        printf("send 123\n");

        //HUART tx test
        strcpy((char *)huart_dma_tx_buf, "123");
        huart_tx((char *)huart_dma_tx_buf, 3);
    }
#endif

#if !HUART_EXAMPLE_ISR_EN
    while (huart_get_rxcnt()) {
        printf("rx ch[0x%x]\n", huart_getchar());
    }
#endif
}

void bsp_huart_example_init(void)
{
    huart_t huart;

    memset(huart_dma_rx_buf, 0, 64);
    memset(&huart, 0x00, sizeof(huart));

    //example: TX: PB8, RX: PB9
    huart.tx_map = HSUTTXMAP_G7_PB8;
    huart.rx_map = HSUTTXMAP_G8_PB9;

//    //example: TRX: VUSB  //todo: test
//    huart.tx_map = HSUTTXMAP_G11_PE4;
//    huart.rx_map = HSUTRXMAP_G12_PE5;

//    //example: crossbar, , IN_CH = OUT_CH = 3
//    huart.tx_map = HSUTTXMAP_OUT_CH;
//    huart.rx_map = HSUTRXMAP_G3_IN_CH3;
//    huart.tx_crossbar_ch = 3;
//    //TRX: PB8
//    huart.tx_crossbar = CHxOUTMAP_PB8;
//    huart.rx_crossbar = CHxINSEL_PB8;
//    //TX: PB8, RX: PB9
//    huart.tx_crossbar = CHxOUTMAP_PB8;
//    huart.rx_crossbar = CHxINSEL_PB9;

    huart.baud     = 1500000;
    huart.rxbuf    = huart_dma_rx_buf;
    huart.rxbuf_len = 64;
#if HUART_EXAMPLE_ISR_EN
    huart.tx_isr   = bsp_huart_example_tx_isr;
    huart.rx_isr   = bsp_huart_example_rx_isr;
#else
    huart.rxbuf_loop = 1;
#endif
    huart_init(&huart);

    while(1) {
        WDT_CLR();
        bsp_huart_example_process();
    }
}
#endif // HUART_EXAMPLE_EN

#if HUART_DUMP_EN
void bt_sco_huart_tx_done(void);
void huart_loop_rx_once_cb(void);

AT(.com_text.bsp_huart)
void huart_tx_done_cb(void)
{
    TRACE(huart_tx_done_str);
#if BT_SCO_DUMP_EN || BT_SCO_EQ_DUMP_EN || BT_SCO_FAR_DUMP_EN
    bt_sco_huart_tx_done();
#endif

}

AT(.com_text.bsp_huart)
void huart_rx_done_cb(uint8_t *buf, uint32_t len)
{
    TRACE(huart_rx_done_str);
    TRACE_R(buf, len);
//    if(eq_rx_buf[0] == 0xA5 && eq_rx_buf[1] == 0x96 && eq_rx_buf[2] == 0x87 && eq_rx_buf[3] == 0x5A){
//		WDT_RST();
//		while(1);
//	}

#if EQ_DBG_IN_UART
    if (bsp_eq_rx_done(eq_rx_buf)){
        return;
    }
#endif
}

void bsp_huart_dump_init(void)
{
    huart_t huart0;

    TRACE("%s: huart_dump_sel[%d]\n", __func__, xcfg_cb.huart_dump_sel);
    if (xcfg_cb.huart_dump_sel == HSUTRXMAP_G5_PB0) {
        if (UART0_PRINTF_SEL == PRINTF_PB0) {
            FUNCMCON0 = 0x0f << 8;
        }
    } else if (xcfg_cb.huart_dump_sel == HSUTRXMAP_G9_PE0) {
        if (UART0_PRINTF_SEL == PRINTF_PE0) {
            FUNCMCON0 = 0x0f << 8;
        }
    } else if (xcfg_cb.huart_dump_sel == HSUTRXMAP_G1_PB3) {
        if (UART0_PRINTF_SEL == PRINTF_PB3) {
            FUNCMCON0 = 0x0f << 8;
        }
    } else if ((xcfg_cb.huart_dump_sel == HSUTRXMAP_G2_VUSB)) {
        if (UART0_PRINTF_SEL == PRINTF_VUSB) {
            FUNCMCON0 = 0x0f << 8;
        }
        PWRCON0 |= BIT(30);                //Enable VUSB GPIO
    }

    memset(eq_rx_buf, 0, EQ_BUFFER_LEN);
    memset(&huart0, 0x00, sizeof(huart0));
    huart0.tx_map = xcfg_cb.huart_dump_sel;
    huart0.rx_map = xcfg_cb.huart_dump_sel;
    huart0.baud     = 1500000;
    huart0.rxbuf    = eq_rx_buf;
    huart0.rxbuf_len = EQ_BUFFER_LEN;
    huart0.tx_isr   = huart_tx_done_cb;
    huart0.rx_isr   = huart_rx_done_cb;

    huart0.tx_map = HSUTTXMAP_G11_PE4;
    huart0.rx_map = HSUTRXMAP_G12_PE5;

    huart_init(&huart0);
}
#else
void bsp_huart_dump_init(void) {}
#endif  // HUART_DUMP_EN

void bsp_huart_init(void)
{
#if HUART_EXAMPLE_EN
    bsp_huart_example_init();
#elif HUART_DUMP_EN
    if(xcfg_cb.huart_dump_en) {
        bsp_huart_dump_init();
    }
#endif // HUART_DUMP_EN
}

u8* huart_get_rxbuf(u16 *len)
{
    *len  = EQ_BUFFER_LEN;
    return eq_rx_buf;
}

#if BT_FCC_TEST_EN || LE_BQB_RF_EN
ALIGNED(4)
u8 huart_buffer[128];

void bt_uart_init(void)
{
    huart_t huart0;

    memset(&huart0, 0x00, sizeof(huart0));
#if LE_BQB_RF_EN
    huart0.rx_port = HSUTRXMAP_G5_PB0;
    huart0.tx_port = HSUTRXMAP_G6_PB1;
    huart0.rxbuf_loop = 1;
    huart0.baud       = 9600;
    huart0.rxbuf      = huart_buffer;
    huart0.rxbuf_len  = 128;

    huart_init(&huart0);
#else
    huart0.rx_port = xcfg_cb.huart_sel;
    huart0.tx_port = xcfg_cb.huart_sel;
    huart0.rxbuf_loop = 1;
    huart0.baud       = 1500000;
    huart0.rxbuf      = huart_buffer;
    huart0.rxbuf_len  = 128;

    huart_init(&huart0);
#endif
}

void bt_uart_exit(void)
{
}
#endif
