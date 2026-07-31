/**
 * @file    lb_uart_link.c
 * @brief   饭盒 UART1 收发层实现 — 直接对接 api_uart (绕过 bsp_uart)
 * @note    不经 bsp_uart1_*: 其 u8 计数器撑不起 512B 缓冲, 且其 ISR 的
 *          "满则覆盖未读数据 + 300ms 超时清缓冲" 正是本层要修复的缺陷。
 */
#include "include.h"
#include "lb_uart_link.h"

#if FUNC_LUNCHBOX_UART_EN

#if (LB_LINK_RXBUF_SIZE & (LB_LINK_RXBUF_SIZE - 1)) != 0
#error "LB_LINK_RXBUF_SIZE must be a power of 2"
#endif
#define LB_LINK_RXBUF_MASK      (LB_LINK_RXBUF_SIZE - 1)

typedef struct {
    u8           buf[LB_LINK_RXBUF_SIZE];
    volatile u16 w;                     // 只有 ISR 写
    volatile u16 r;                     // 只有主循环写
    volatile u16 overflow;              // 满时丢弃的字节数 (ISR 递增, 主循环读清)
} lb_link_rx_t;

static lb_link_rx_t lb_link_rx;
static bool lb_link_suspended;
static u32  lb_link_baud = 115200;      // init 记录, resume 沿用

//-----------------------------------------------------------------------------
// RX 中断 (中断上下文, 只搬字节)
//-----------------------------------------------------------------------------
AT(.com_text.uart)
static void lb_link_rx_isr(uint8_t *buf, uint32_t len)
{
    u16 w = lb_link_rx.w;
    while (len--) {
        u16 nw = (w + 1) & LB_LINK_RXBUF_MASK;
        if (nw == lb_link_rx.r) {       // 满: 丢新字节保未读旧数据
            lb_link_rx.overflow += (u16)(len + 1);
            break;
        }
        lb_link_rx.buf[w] = *buf++;
        w = nw;
    }
    lb_link_rx.w = w;                   // 写指针最后一次性发布
}

//-----------------------------------------------------------------------------
// 主循环侧读取
//-----------------------------------------------------------------------------
AT(.com_text.uart)
u8 lb_link_getc(u8 *ch)
{
    u16 r = lb_link_rx.r;
    if (r == lb_link_rx.w) {
        return 0;
    }
    *ch = lb_link_rx.buf[r];
    lb_link_rx.r = (r + 1) & LB_LINK_RXBUF_MASK;
    return 1;
}

AT(.com_text.uart)
u16 lb_link_rx_pending(void)
{
    return (u16)((lb_link_rx.w - lb_link_rx.r) & LB_LINK_RXBUF_MASK);
}

u16 lb_link_rx_overflow(void)
{
    u16 n = lb_link_rx.overflow;
    lb_link_rx.overflow = 0;
    return n;
}

void lb_link_rx_clear(void)
{
    lb_link_rx.r = lb_link_rx.w;        // 只动读指针, 避免与 ISR 写指针竞态
}

//-----------------------------------------------------------------------------
// 发送 (阻塞)
//-----------------------------------------------------------------------------
void lb_link_tx(const u8 *buf, u16 len)
{
    if (lb_link_suspended || !buf || !len) {
        return;
    }
    uart_bufs_tx(UART_TYPE_1, (u8 *)buf, len);
}

//-----------------------------------------------------------------------------
// 初始化 / 电源管理
//-----------------------------------------------------------------------------
bool lb_link_init(u32 baud)
{
    uart_t uart1;
    memset(&uart1, 0, sizeof(uart1));
    uart1.type   = UART_TYPE_1;
    uart1.tx_map = UT1TXMAP_G2_PB8;
    uart1.rx_map = UT1RXMAP_G2_PB9;
    uart1.baud   = baud;
    uart1.rx_isr = lb_link_rx_isr;

    lb_link_rx.w = 0;
    lb_link_rx.r = 0;
    lb_link_rx.overflow = 0;
    lb_link_baud = baud;

    Err_Uart ret = uart_init(&uart1);
    if (ret != ERR_UART_SUCCESS) {
        printf("lb_link: init fail, err=%d\n", ret);
        return false;
    }
    lb_link_suspended = false;
    printf("lb_link: init ok TX=PB8 RX=PB9 baud=%d\n", baud);
    return true;
}

void lb_link_suspend(void)
{
    if (lb_link_suspended) {
        return;
    }
    UART1CON = 0;                       
    /* 关 UART1 外设与中断; resume 时由 uart_init 重建 */
    /* 释放 PB8(TX)/PB9(RX) 从 UART1 功能回到 GPIO 模式。
     * FUNCMCON0: UT1TXMAP=bit24(27:24), UT1RXMAP=bit28(31:28) → CLEAR(0xf)
     * 否则 PB9 仍被 UART RX 占用, port_wakeup_init 的下降沿检测不生效,
     * 对方发数据的起始位无法唤醒芯片。 */
    FUNCMCON0 = (FUNCMCON0 & ~((0xf << 28) | (0xf << 24))) | (0xf << 28) | (0xf << 24);
    lb_link_suspended = true;           /* 先置位停用 TX, 再清缓冲 (中断已关) */
    lb_link_rx.w = 0;
    lb_link_rx.r = 0;
    lb_link_rx.overflow = 0;
}

void lb_link_resume(void)
{
    if (!lb_link_suspended) {
        return;
    }
    lb_link_init(lb_link_baud);
}

bool lb_link_is_suspended(void)
{
    return lb_link_suspended;
}

#endif // FUNC_LUNCHBOX_UART_EN
