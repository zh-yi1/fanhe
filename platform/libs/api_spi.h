#ifndef _API_SPI_H
#define _API_SPI_H

#define spi_cs_en(io_num)       port_gpio_out_level(io_num, 0)
#define spi_cs_dis(io_num)      port_gpio_out_level(io_num, 1)
#define despi_9b_dc_set(x)      (REG_SET_BITS(DESPICON, DESPICON_3W_BIT9, x))   //3w-9bitDC设置
#define despi_3w_ctr(x)         (REG_SET_BITS(DESPICON, DESPICON_3W_CTR, x))    //3w-9bit DC控制使能位

typedef enum {
    ERR_SPI_SUCCESS = 0,
    ERR_SPI_NULL_PTR,
    ERR_SPI_TYPE,
    ERR_SPI_MAP,
    ERR_SPI_CS,
    ERR_SPI_CLK_HZ,
    ERR_SPI_RX_ISR,
    ERR_SPI_NO_INIT,
    ERR_SPI_BUF,
    ERR_SPI_LEN,
    ERR_SPI_NOT_SUPPORT_FOR_NOW,
    ERR_SPI_DMA_MODE_NO_USE_BUF,    //开了中断清pending用buf mode发数据会直接复位
}Err_Spi;

typedef enum {
    SPI_TYPE_NONE = 0,
    SPI_TYPE_0,
    SPI_TYPE_1,
    SPI_TYPE_2,
    SPI_TYPE_LCD,
    SPI_TYPE_MAX,
} Spi_Type;

typedef void (*spi_isr_t)();

typedef struct {
    volatile uint32_t SPIxCON;
    volatile uint32_t SPIxBUF;
    volatile uint32_t SPIxBAUD;
    volatile uint32_t SPIxPND;
    volatile uint32_t SPIxDMACNT;   //除了LCDSPICON外
    volatile uint32_t SPIxDMAADR;   //除了LCDSPICON外
} spi_sfr_t;

typedef struct spi_config_t_ {
    Spi_Type type;              //硬件资源类型
    u8 cs;                      //片选脚, 任意io num
    u8 map;                     //传入 SPI0Map/SPI1Map/DESPIMap
    u8 spism            :1,     //从机模式使能
        bus             :2,     //总线模式, 传入 Spi_BusMode
        clkids          :1,     //时钟空闲电平, 0:低电平; 1:高电平;
        smps            :1,     //输出数据有效边沿选择, 0:下降沿; 1:上升沿;
        spilf_en        :1,     //SPI LFSR enable bit
        spimben         :1,     //multiple bit bus使能位, 启用多条发送数据线(2条及以上)
        spioss          :1;     //数据采集和发送是否在相同的时钟边沿; 0: 不同; 1:相同;
    u8  sp_8w_en        :1,
        despi_3w_ctr    :1,     //3w-9bit DC控制使能位
        despi_dcx_reuse :1,     //DXC引脚复用, D1数据线作DC;
        spi_1p1t_en     :1,
        ddr_en          :1,
        resv            :3;
    u32 clk_hz;             //时钟频率(Hz)
    spi_isr_t isr;          //发送/接收中断回调函数(dma下可通过SPIxCON_RXSEL区分)
    CHxINSEL clk_crossbar;  //自由映射CLK脚
    u8 clk_crossbar_ch;     //自由映射CLK脚通道号(0~3)
    CHxINSEL d0_crossbar;   //自由映射MOSI/D0脚
    u8 d0_crossbar_ch;      //自由映射MOSI/D0脚通道号(0~3)
    CHxINSEL d1_crossbar;   //自由映射MISO/D1脚
} spi_t;

void spi_sendbyte(u8 data); //SPI接口发送1Byte数据
u8 spi_getbyte(void);       //SPI0接口获取1Byte数据
void spi_delay(void);       //nop延时20次

/**
 * @brief 阻塞等待收发完成
 * @param[in] type SPI类型
 * @return Err_Spi
 **/
u8 spi_wait(Spi_Type type);

///TX
/**
 * @brief SPI buf模式发送字节数据, 阻塞的
 * @param[in] type SPI类型
 * @param[in] buf 数据
 * @return Err_Spi
 **/
Err_Spi spi_buf_tx(Spi_Type type, u8 buf);

/**
 * @brief SPI dma模式发送数据
 * @param[in] type SPI类型
 * @param[in] buf 发送数据缓冲区
 * @param[in] len 发送数据缓冲区长度
 * @return Err_Spi
 **/
Err_Spi spi_dma_tx(Spi_Type type, u8 *buf, u32 len);

/**
 * @brief SPI dma模式发送数据, 阻塞等待发送完成
 * @param[in] type SPI类型
 * @param[in] buf 发送数据缓冲区
 * @param[in] len 发送数据缓冲区长度
 * @return Err_Spi
 **/
Err_Spi spi_dma_tx_wait(Spi_Type type, u8 *buf, u32 len);

///RX
/**
 * @brief spi buf模式接受字节数据, 阻塞的
 * @param[in] type SPI类型
 * @return  返回接收内容
 **/
u8 spi_buf_getbyte(Spi_Type type);

/**
 * @brief spi buf模式接受字节数据, 阻塞的
 * @param[in] type SPI类型
 * @param[in] buf 数据缓冲区
 * @return Err_Spi
 **/
Err_Spi spi_buf_rx(Spi_Type type, u8 *buf);

/**
 * @brief SPI dma模式接受数据
 * @param[in] type SPI类型
 * @param[in] buf 接受数据缓冲区
 * @param[in] len 接受数据缓冲区长度
 * @return Err_Spi
 **/
Err_Spi spi_dma_rx(Spi_Type type, void *buf, u32 len);

/**
 * @brief SPI dma模式接受数据, 阻塞等待发送完成
 * @param[in] type SPI类型
 * @param[in] buf 接受数据缓冲区
 * @param[in] len 接受数据缓冲区长度
 * @return Err_Spi
 **/
Err_Spi spi_dma_rx_wait(Spi_Type type, u8 *buf, u32 len);


///config
/**
 * @brief 总线位宽选择
 * @param[in] type SPI类型
 * @param[in] bus 总线位宽
 * @return Err_Spi
 **/
Err_Spi spi_bus_set(Spi_Type type, Spi_BusMode bus);

/**
 * @brief spi初始化
 * @param[in] cfg: spi配置
 * @return  返回是否成功
 * 注: 不支持同时TX RX数据, 属于半双工通讯, 注意做防冲突
 **/
Err_Spi spi_init(spi_t *spi);

#endif // _API_SPI_H
