/**
 * @file    bsp_uart.h
 * @brief   BSP 层 UART1 收发管理接口
 * @note    提供环形缓冲接收、非阻塞取字节、字符串/单字节发送等封装，
 *          底层由 uart_init / huart 驱动完成硬件配置。
 */

#ifndef _BSP_UART_H
#define _BSP_UART_H

#define UART1_EXAMPLE_EN            0       // 串口1测试用例使能

#define BSP_UART1_EN                1       // 是否打开串口1收发管理器
// #define BSP_UART1_EN              UART1_EXAMPLE_EN
#define BSP_UART_EN                 (BSP_UART1_EN)

/**
 * @brief UART1 运行时状态（环形缓冲 + 可选用户回调）
 */
typedef struct {
    volatile u8 w_cnt;          // 写指针（ISR 写入位置）
    volatile u8 r_cnt;          // 读指针（主循环读取位置）
    u32 ticks;                  // 最近一次收到数据的时间戳，用于超时清缓冲
    u8 *rxbuf;                  // 外部提供的环形接收缓冲区
    u16 rxbuf_len;              // 环形缓冲区长度(字节)
    uart_rx_isr_t rx_isr;       // 用户注册的接收回调（可选，在 ISR 末尾调用）
} bsp_uart_t;

extern bsp_uart_t bsp_uart1;

uart_t *bsp_uart1_cfg_get(void);

/**
 * @brief UART1 发送字符串
 * @param[in] str  以 '\0' 结尾的字符串
 */
void bsp_uart1_send_str(char *str);

/**
 * @brief UART1 发送单字节
 * @param[in] ch  待发送字节
 */
void bsp_uart1_putchar(char ch);

/**
 * @brief 从 UART1 环形缓冲区非阻塞读取一字节
 *
 * 有数据时读出并推进读指针，无数据立即返回 0。
 * 若初始化时未注册 rx_isr，则退化为轮询 huart 硬件 FIFO。
 *
 * @param[out] ch  读出的字节
 * @return 1 成功读到数据，0 缓冲区为空
 */
u8 bsp_uart1_get_char(u8 *ch);

/** @brief 清空 UART1 环形缓冲读写指针 */
void bsp_uart1_rxclr(void);

/**
 * @brief 设置 UART1 波特率
 * @param[in] baud  波特率，如 9600、115200
 */
void bsp_uart1_set_baud(u32 baud);

/**
 * @brief UART1 发送字符串（同 bsp_uart1_send_str）
 * @param[in] str  需发送的字符串
 */
void bsp_uart1_str_tx(char *str);

/**
 * @brief 初始化 UART1
 *
 * 若 uart->rx_isr 非 NULL，会先保存用户回调，再将其替换为内部 bsp_uart1_isr，
 * 由内部 ISR 完成环形缓冲写入后再转发给用户回调。
 *
 * @param[in] uart       硬件 UART 配置（引脚映射、波特率等）
 * @param[in] rxbuf      接收环形缓冲区，可为 NULL（不使用缓冲管理）
 * @param[in] rxbuf_len  接收缓冲区长度(字节)
 * @return ERR_UART_SUCCESS 成功，其他见 Err_Uart
 */
Err_Uart bsp_uart1_init(uart_t *uart, u8 *rxbuf, u16 rxbuf_len);

/**
 * @brief BSP UART 模块总入口（当前主要用于示例代码）
 */
void bsp_uart_init(void);

#endif
