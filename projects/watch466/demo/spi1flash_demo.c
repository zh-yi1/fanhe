#include "include.h"
#include "spi1flash_demo.h"

#if FLASH_EXTERNAL_EN

//测试地址选择Flash高地址4K扇区，避免覆盖低地址UI资源数据
//注意：执行demo会擦除该扇区原有数据，请确认该地址未被使用
#define SPI1FLASH_DEMO_ADDR     0x00000000
#define SPI1FLASH_DEMO_LEN      256         //测试长度(一页大小)

AT(.text.spi1flash)
void spi1flash_demo(void)
{
    u32 id;
    u8 wbuf[SPI1FLASH_DEMO_LEN];
    u8 rbuf[SPI1FLASH_DEMO_LEN];

    printf("========== spi1flash demo start ==========\n");

    //1.读取Flash设备ID
    id = spi1flash_id_read();
    printf("spi1flash demo: id = 0x%06x\n", id);

    //2.擦除测试扇区
    printf("spi1flash demo: erase addr = 0x%06x\n", SPI1FLASH_DEMO_ADDR);
    spi1flash_erase(SPI1FLASH_DEMO_ADDR);
    printf("spi1flash demo: erase done\n");

    //3.填充测试数据并写入(递增序列0x00~0xFF)
    for (uint i = 0; i < SPI1FLASH_DEMO_LEN; i++) {
        wbuf[i] = (u8)i;
    }
    spi1flash_program(wbuf, SPI1FLASH_DEMO_ADDR, SPI1FLASH_DEMO_LEN);
    printf("spi1flash demo: write %d bytes done\n", SPI1FLASH_DEMO_LEN);

    //4.回读并比对数据
    spi1flash_read(rbuf, SPI1FLASH_DEMO_ADDR, SPI1FLASH_DEMO_LEN);
    if (memcmp(rbuf, wbuf, SPI1FLASH_DEMO_LEN) == 0) {
        printf("spi1flash demo: pass\n");
    } else {
        for (uint i = 0; i < SPI1FLASH_DEMO_LEN; i++) {
            if (rbuf[i] != wbuf[i]) {
                printf("spi1flash demo: fail at offset %d, expect 0x%02x, got 0x%02x\n", i, wbuf[i], rbuf[i]);
                break;
            }
        }
    }

    printf("========== spi1flash demo end ==========\n");
}

#endif // FLASH_EXTERNAL_EN
