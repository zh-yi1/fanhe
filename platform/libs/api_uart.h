#ifndef __API_UART_H__
#define __API_UART_H__

typedef enum {
    ERR_UART_SUCCESS = 0,
    ERR_UART_NULL_PTR,
    ERR_UART_TYPE,
    ERR_UART_TX_MAP,
    ERR_UART_RX_MAP,
    ERR_UART_TX_CH,
    ERR_UART_TX_CROSSBAR,
    ERR_UART_RX_CROSSBAR,
    ERR_UART_RX_NOT_MAP_TO_OUT_CH,
    ERR_UART_RX_ISR,
}Err_Uart;

typedef enum {  // 普通串口类型
    UART_TYPE_NONE,
    UART_TYPE_0,
    UART_TYPE_1,
    UART_TYPE_MAX,
} Uart_Type;

typedef void (*uart_rx_isr_t)(uint8_t *buf, uint32_t len);

typedef struct {
    volatile uint32_t UARTxCON;  // 控制寄存器：使能/数据位/停止位/中断开关
    volatile uint32_t UARTxCPND; // 状态寄存器：中断标志/收发完成标志
    volatile uint32_t UARTxBAUD; // 波特率寄存器：分频系数
    volatile uint32_t UARTxDATA; 
} uart_sfr_t;

typedef struct _uart_t_ {
    Uart_Type type;
    uint8_t  tx_map   : 4,          //发送脚, 传入 UT0RxMap/UT1RxMap
             rx_map   : 4;          //接收脚, 传入 UT0RxMap/UT1RxMap
    uint16_t tx_crossbar_ch :2,     //发送脚映射通道(有效值: 0~3)
             tx_crossbar    :6,     //发送脚映射号, 传入 CHxOUTMAP
             rx_crossbar    :6;     //接收脚映射号, 传入 CHxINSEL
    uint8_t  bit9en   : 1,          //是否打开9位数据
             stop2bit : 1,          //是否打开2停止位
             oneline  : 1;          //是否为单线
    uint32_t baud;                  //指定波特率, 否则采用自适应
    uart_rx_isr_t rx_isr;           //接收中断回调函数
} uart_t;

typedef struct _uart_cfg_t_ {
    Uart_Type type;
    uint8_t tx_io;                  //发送脚, 传入IO号
    uint8_t rx_io;                  //接收脚, 传入IO号
    uint16_t crossbar_en    : 1,    //是否使能自由映射
             tx_crossbar_ch : 2,    //发送脚映射通道(有效值: 0~3)
             rx_crossbar_ch : 2;    //接收脚映射通道(有效值: 2~3)
    uint8_t  bit9en   : 1,          //是否打开9位数据
             stop2bit : 1,          //是否打开2停止位
             oneline  : 1;          //是否为单线
    uint32_t baud;                  //指定波特率, 否则采用自适应
    uart_rx_isr_t rx_isr;           //接收中断回调函数
} uart_cfg_t;

/**
 * @brief 发送单字节
 * @param[in] type 对应串口类型
 * @param[in] buf 数据
 **/
Err_Uart uart_buf_tx(Uart_Type type, u8 buf);

/**
 * @brief 发送多字节
 * @param[in] type 对应串口类型
 * @param[in] buf 数据
 * @param[in] len 数据长度
 **/
Err_Uart uart_bufs_tx(Uart_Type type, u8 *buf, u16 len);

/**
 * @brief 发送字符串
 * @param[in] type 对应串口类型
 * @param[in] str 需发送的字符串
 **/
Err_Uart uart_str_tx(Uart_Type type, char *str);

/**
 * @brief 设置波特率
 * @param[in] type 对应串口类型
 * @param[in] baudrate 波特率
 **/
Err_Uart uart_baud_set(Uart_Type type, uint32_t baud);

/**
 * @brief 初始化普通串口
 * @param[in] uart 普通串口初始化结构体
 * 特别注意: 当使用crossbar双线映射时, trx不能同时使用, 属于半双工通讯;
     一般crossbar作为debug或备用mapping;
 **/
Err_Uart uart_init(uart_t *uart);

/**
 * @brief 初始化普通串口
 * @param[in] uart 普通串口初始化结构体
 **/
Err_Uart uart_cfg_init(uart_cfg_t *uart_cfg);
#endif  // __API_UART_H__
