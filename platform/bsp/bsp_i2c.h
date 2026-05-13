#ifndef _BSP_I2C_H
#define _BSP_I2C_H

#define DATA_CNT_1B     1
#define DATA_CNT_2B     2
#define DATA_CNT_3B     3
#define DATA_CNT_4B     4

#define START_FLAG0     BIT(3)
#define DEV_ADDR0       BIT(4)
#define REG_ADDR_0      BIT(5)
#define REG_ADDR_1      (BIT(5) | BIT(6))

#define START_FLAG1     BIT(7)
#define DEV_ADDR1       BIT(8)
#define RDATA           BIT(9)
#define WDATA           BIT(10)

#define STOP_FLAG       BIT(11)
#define NACK            BIT(12)

typedef struct {
    volatile uint32_t IICxCON0;
    volatile uint32_t IICxCON1;
    volatile uint32_t IICxCMDA;
    volatile uint32_t IICxDATA;
    volatile uint32_t IICxDMAADR;
    volatile uint32_t IICxDMACNT;
} i2c_sfr_t;

typedef struct {
    volatile uint32_t FUNCMCONx;
} i2c_map_t;

typedef struct {
    i2c_sfr_t *sfr;
    i2c_map_t *map;
} i2c_t;

//void bsp_i2c_init(void);
//void bsp_i2c_start(void);
//void bsp_i2c_stop(void);
//void bsp_i2c_tx_byte(uint8_t dat);
//uint8_t bsp_i2c_rx_byte(void);
//bool bsp_i2c_rx_ack(void);
//void bsp_i2c_tx_ack(void);
//void bsp_i2c_tx_nack(void);
//
//void bsp_hw_i2c_rx_buf(u32 i2c_cfg, u16 dev_addr, u16 reg_addr, u8 *buf, u16 len);
//void bsp_hw_i2c_tx_buf(u32 i2c_cfg, u16 dev_addr, u16 reg_addr, u8 *buf, u16 len);
//void bsp_hw_i2c_tx_byte(u32 i2c_cfg, u16 dev_addr, u16 reg_addr, u32 data);

///**
// * @brief  IIC0~IIC2通用写接口
// * @param[in] dev_addr  从设备7bit设备地址
// * @param[in] wbuf  写buf，当使用IIC0时，传入的buf需使用lpram1
// * @param[in] wlen  写buf长度
// * @param[in] stop  当次写入是否终止，stop: true:当次写入结束  false:后续跟读操作
// *
// * @return  返回是否成功
// **/
//bool bsp_hw_i2c_dma_write_buf(u16 dev_addr, u8 *wbuf, u16 wlen, bool stop);

///**
// * @brief  IIC0~IIC2通用读接口,需要先写后读
// * @param[in] dev_addr  从设备7bit设备地址
// * @param[in] wbuf  写buf，当使用IIC0时，传入的buf需使用lpram1，若使用bsp_hw_i2c_dma_write_buf已写入寄存器，可置NULL
// * @param[in] wlen  写buf长度, 若使用bsp_hw_i2c_dma_write_buf已写入寄存器，可置0
// * @param[in] rbuf  读buf，当使用IIC0时，传入的buf需使用lpram1
// * @param[in] rlen  读buf长度
// *
// * @return  返回是否成功
// **/
//bool bsp_hw_i2c_dma_read_buf(u16 dev_addr, u8 *wbuf, u16 wlen, u8 *rbuf, u16 rlen);
//
void os_i2c0_lock(uint32_t ms);
void os_i2c0_unlock(void);

i2c_t* bsp_i2c_get_register(u8 i2cx_idx);
void bsp_i2c_sensor_hub_set_hold(bool en);
void bsp_i2c0_lock(void);
void bsp_i2c0_unlock(void);
void bsp_i2c0_isr(void);
void bsp_i2c1_isr(void);
void bsp_i2c2_isr(void);
void bsp_i2c_irq_init(u8 i2cx_idx);
void bsp_hw_i2c_tx_byte(u8 i2cx_idx, u32 i2c_cfg, u16 dev_addr, u16 reg_addr, u32 data);
void bsp_hw_i2c_tx_buf(u8 i2cx_idx, u32 i2c_cfg, u16 dev_addr, u16 reg_addr, u8 *buf, u16 len);
void bsp_hw_i2c_rx_buf(u8 i2cx_idx, u32 i2c_cfg, u16 dev_addr, u16 reg_addr, u8 *buf, u16 len);
bool bsp_hw_i2c_dma_write_buf(u8 i2cx_idx, u16 dev_addr, u8 *wbuf, u16 wlen, bool stop);
bool bsp_hw_i2c_dma_read_buf(u8 i2cx_idx, u16 dev_addr, u8 *wbuf, u16 wlen, u8 *rbuf, u16 rlen);
void bsp_i2c_init(u8 i2cx_idx, u8 port_scl, u8 port_sda, u8 port_map);
void bsp_i2c_deinit(u8 i2cx_idx, u8 port_scl, u8 port_sda, u8 port_map);
#endif
