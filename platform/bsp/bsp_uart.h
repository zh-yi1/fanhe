#ifndef _BSP_UART_H
#define _BSP_UART_H

#define UART1_EXAMPLE_EN  		    0       // 串口1测试用例使能

// 是否打开串口1收发管理器
#define BSP_UART1_EN    (UART1_EXAMPLE_EN)
#define BSP_UART_EN     (BSP_UART1_EN)

typedef struct {
    volatile u8 w_cnt;
    volatile u8 r_cnt;
    u32 ticks;
    u8 *rxbuf;
    u16 rxbuf_len;
    uart_rx_isr_t rx_isr;           //接收中断回调函数
} bsp_uart_t;
extern bsp_uart_t bsp_uart1;

uart_t *bsp_uart1_cfg_get(void);
void bsp_uart1_send_str(char *str);
void bsp_uart1_putchar(char ch);

/**
 * @brief 从UART1接收数据缓冲区中获取字节数据
 * @param[in] ch: 字节指针
 * @return 成功与否
 **/
u8 bsp_uart1_get_char(u8 *ch);

/**
 * @brief UART1设置波特率
 **/
void bsp_uart1_set_baud(u32 baud);

/**
 * @brief UART1发送单字节
 * @param[in] type 对应串口类型
 * @param[in] buf 数据
 **/
void bsp_uart1_putchar(char ch);

/**
 * @brief UART1发送字符串
 * @param[in] str 需发送的字符串
 **/
void bsp_uart1_str_tx(char *str);

/**
 * @brief 初始化UART1
 * @param[in] uart: 初始化结构体
 * @param[in] rxbuf: 接收数据缓冲区, 若有则使用默认的接收管理器
 * @param[in] rxbuf_len: 接收数据缓冲区长度
 **/
Err_Uart bsp_uart1_init(uart_t *uart, u8 *rxbuf, u16 rxbuf_len);

void bsp_uart_init(void);
#endif
