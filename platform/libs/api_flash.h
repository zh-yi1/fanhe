#ifndef _API_FLASH_H
#define _API_FLASH_H

uint os_spiflash_read(void *buf, u32 addr, uint len);
void os_spiflash_program(void *buf, u32 addr, uint len);
void os_spiflash_erase(u32 addr);           //addr需要4k对齐
void os_spiflash_erase_32k(u32 addr);       //addr需要32k对齐
void os_spiflash_erase_64k(u32 addr);       //addr需要64k对齐


//读取4byte flash唯一id
void spiflash_readuid(uint8_t *buf);
//读取4byte flash唯一id 无缓存从硬件读
void spiflash_readuid_do(uint8_t *buf);

#endif // _API_FLASH_H
