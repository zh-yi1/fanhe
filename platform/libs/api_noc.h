#ifndef _API_NOC_H
#define _API_NOC_H

void noc_init(u8 mode);									//BIT0:打开noc flash, BIT(1):打开noc psram
void noc_spiflash_read(void *buf, u32 addr, uint len);  //默认是32字节一页快速读并cache
void bsp_cache_to_psram_refresh(u8 *ptr, int len);

#endif // _API_NOC_H
