#ifndef __API_HUART_H__
#define __API_HUART_H__

typedef enum {
    ERR_HUART_SUCCESS = 0,
    ERR_HUART_NULL_PTR,
    ERR_HUART_TX_MAP,
    ERR_HUART_RX_MAP,
    ERR_HUART_TX_CH,
    ERR_HUART_TX_CROSSBAR,
    ERR_HUART_RX_CROSSBAR,
    ERR_HUART_TX_ISR,
    ERR_HUART_RX_ISR,
    ERR_HUART_NO_INIT,
    ERR_HUART_BUF,
    ERR_HUART_LEN,
}Err_Huart;

typedef void (*huart_rx_isr_t)(uint8_t *buf, uint32_t len);
typedef void (*huart_tx_isr_t)(void);

typedef struct {
    uint8_t tx_map      : 4,    //发送脚, 传入 HsutTxMap
            rx_map      : 4;    //接收脚, 传入 HsutRxMap
    uint8_t rxbitsel    : 1,    //RX data bit select; 0:8-bit; 1:9-bit;
            rxlpbufen   : 1,    //RX dma loop buffer mode enable
            txbitsel    : 1,    //TX data bit select; 0:8-bit; 1:9-bit;
            spbitsel    : 1,    //TX stop bit select; 0: 1BIT, 2:2BIT;
            resv        : 2,
            oneline     : 1,    //是否为单线
            tx_1st      : 1;
    uint32_t baud;              //波特率
    uint8_t *rxbuf;             //接收数据缓冲区
    uint16_t rxbuf_len;         //接收数据缓冲长度
    uint16_t tx_crossbar_ch :2, //发送脚映射通道
             tx_crossbar    :6, //发送脚映射号, 传入 CHxOUTMAP
             rx_crossbar    :6; //接收脚映射号, 传入 CHxINSEL
    huart_tx_isr_t tx_isr;      //发送中断回调函数
    huart_rx_isr_t rx_isr;      //接收中断回调函数
} huart_t;

///设置高速串口波特率
void huart_set_baudrate(uint baudrate);

/**
 * @brief 高速串口发送数据
 * @param[in] buf 发送数据缓冲区指针
 * @param[in] len 发送数据缓冲区长度
 * @return Err_Huart
 * 当buf地址为flash地址时, 使用buf mode传输;
 * 当buf地址为ram地址时, 使用dma mode传输; 非阻塞的, 注意dma地址在发送完成前不要释放, 比如局部变量;
 **/
Err_Huart huart_tx(const void *buf, uint len);

///获取高速串口HSUT0FIFOCNT数据长度
uint huart_get_rxcnt(void);

///高速串口按字节获取HSUT0FIFO数据
char huart_getchar(void);

/**
 * @brief 高速串口发送字节数据
 * @param[in] ch 发送数据缓冲区指针
 * 使用buf mode传输;
 **/
void huart_putchar(const char ch);

///关闭高速串口
void huart_exit(void);

/**
 * @brief 高速串口初始化
 * @param[in] huart 初始化控制结构图
 * @return Err_Huart
 * 特别注意: 当使用crossbar双线映射时, trx不能同时使用, 属于半双工通讯;
     一般crossbar作为debug或备用mapping;
 **/
Err_Huart huart_init(huart_t *huart);
#endif // __API_HUART_H__
