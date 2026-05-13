#ifndef _BSP_SPI1FLASH_H
#define _BSP_SPI1FLASH_H

//use QSPI1-G2
#define SPI1_CLK_PORT           IO_PB6
#define SPI1_CS_PORT            IO_PB2
#define SPI1_DO_IO0_PORT        IO_PB5
#define SPI1_DI_IO1_PORT        IO_PB1
#define SPI1_WP_IO2_PORT        IO_PB0
#define SPI1_HOLD_IO3_PORT      IO_PB7
#define SPI1_MAP_PORT           SPI1MAP_G2

#define SPI1_CS_EN()            GPIOBCLR = BIT(2)
#define SPI1_CS_DIS()           GPIOBSET = BIT(2)
#define SPI1_CS_IS_INIT()       (GPIOBDE & BIT(2)) && !(GPIOBDIR & BIT(2))
#define SPI1_CS_IS_DILE()       (GPIOB & BIT(2))   

//适配不同厂家NOR FLASH时需要根据手册校对以下指令是否正确(开发板默认型号为XT25Q128B)
#define SF_WRITESSR             0x01                    //SPIFlash写状态寄存器
#define SF_PROGRAM              0x02                    //SPIFlash编程
#define SF_READ                 0x03                    //SPIFlash读取
#define SF_FAST_READ            0x0B                    //SPIFlash快速读取
#define SF_QSPI_FAST_READ       0xEB                    //SPIFlash快速读取(QSPI)
#define SF_READSSR_L            0x05                    //SPIFlash读状态寄存器S7~S0
#define SF_READSSR_H            0x35                    //SPIFlash读状态寄存器S15~S8
#define SF_WRITE_EN             0x06                    //SPIFlash写使能
#define SF_ERASE                0x20                    //SPIFlash4K擦除
#define SF_32KERASE             0x52                    //SPIFlash32K擦除
#define SF_64KERASE             0xD8                    //SPIFlash64K擦除
#define SF_STA_QE_EN            BIT(9)                  //ENABLE QE STA
#define SF_READID               0x9F                    //SPIFlash读设备ID


/**
 * @brief 扩展SPIFLASH初始化
 * @return  无
 **/
void bsp_spi1flash_init(void);

/**
 * @brief FLASH设备ID读取
 *
 * @return  设备ID
 **/
u32 spi1flash_id_read(void);

/**
 * @brief 扩展SPIFLASH写入
 * @param[in] buf   写入数据buf
 * @param[in] addr  FLASH绝对地址，内部已做256页对齐
 * @param[in] len   写入长度
 *
 * @return  无
 **/
void spi1flash_program(void *buf, u32 addr, uint len);

/**
 * @brief 扩展SPIFLASH读取
 * @param[in] buf   读取数据buf
 * @param[in] addr  FLASH绝对地址
 * @param[in] len   读取长度
 *
 * @return  true or false
 **/
bool spi1flash_read(void *buf, u32 addr, uint len);

/**
 * @brief 4K擦除
 * @param[in] addr  FLASH绝对地址，需4K对齐
 *
 * @return  true or false
 **/
void spi1flash_erase(u32 addr);

/**
 * @brief 32K擦除
 * @param[in] addr  FLASH绝对地址，需32K对齐
 *
 * @return  true or false
 **/
void spi1flash_erase32k(u32 addr);

/**
 * @brief 64K擦除
 * @param[in] addr  FLASH绝对地址，需64K对齐
 *
 * @return  true or false
 **/
void spi1flash_erase64k(u32 addr);

#endif
