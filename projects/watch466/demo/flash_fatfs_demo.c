#include "include.h"
#include "flash_fatfs_demo.h"

#if FLASH_DISK_EN

#define FFTEST_PATH         "A:\\fftest.txt"
#define FFTEST_STR          "Hello FatFS on SPI Flash!"
#define FFTEST_STR_LEN      (sizeof(FFTEST_STR) - 1)

void flash_fatfs_demo(void)
{
    FRESULT res;
    FIL fp;
    UINT bw, br;
    FILINFO fno;
    u32 file_cnt;
    u64 free_size;
    u8 rbuf[FFTEST_STR_LEN];

    printf("========== flash fatfs demo start ==========\n");

    //1.挂载SPI0 Flash磁盘
    if (!bsp_flash_disk_mount()) {
        printf("flash fatfs demo: mount failed\n");
        goto __end;
    }
    printf("flash fatfs demo: mount ok\n");

    //2.创建文件并写入测试数据
    res = fs_open(&fp, FFTEST_PATH, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        printf("flash fatfs demo: open failed, res=%d\n", res);
        goto __scan;
    }
    res = fs_write(&fp, FFTEST_STR, FFTEST_STR_LEN, &bw);
    fs_close(&fp);
    if (res != FR_OK) {
        printf("flash fatfs demo: write failed, res=%d\n", res);
        goto __scan;
    }
    printf("flash fatfs demo: write ok, %d bytes\n", bw);

    //3.读取文件并校验数据
    res = fs_open(&fp, FFTEST_PATH, FA_READ);
    if (res != FR_OK) {
        printf("flash fatfs demo: read open failed, res=%d\n", res);
        goto __scan;
    }
    res = fs_read(&fp, rbuf, FFTEST_STR_LEN, &br);
    fs_close(&fp);
    if (res != FR_OK) {
        printf("flash fatfs demo: read failed, res=%d\n", res);
        goto __scan;
    }
    if (memcmp(rbuf, FFTEST_STR, FFTEST_STR_LEN) == 0) {
        printf("flash fatfs demo: read verify pass\n");
    } else {
        for (u32 i = 0; i < FFTEST_STR_LEN; i++) {
            if (rbuf[i] != ((const u8 *)FFTEST_STR)[i]) {
                printf("flash fatfs demo: read verify fail at offset %d, expect 0x%02x, got 0x%02x\n",
                       i, ((const u8 *)FFTEST_STR)[i], rbuf[i]);
                break;
            }
        }
    }

    //4.目录扫描 *.txt
__scan:
    file_cnt = 0;
    res = fs_findfirst(&fno, "A:\\", "*.txt", D_FILE, NULL);
    while (res == FR_OK && fno.fname[0]) {
        printf("flash fatfs demo: found file: %s\n", fno.fname);
        file_cnt++;
        res = fs_findnext(&fno);
    }
    printf("flash fatfs demo: scan done, %d txt files\n", file_cnt);

    //5.删除测试文件
    res = fs_unlink(FFTEST_PATH);
    if (res == FR_OK) {
        printf("flash fatfs demo: delete ok\n");
    } else {
        printf("flash fatfs demo: delete failed, res=%d\n", res);
    }

    //6.查询剩余空间
    free_size = fs_getfree("A:");
    printf("flash fatfs demo: free space = %d KB\n", (u32)(free_size / 1024));

    //7.卸载磁盘
    bsp_flash_disk_unmount();
    printf("flash fatfs demo: unmount done\n");

__end:
    printf("========== flash fatfs demo end ==========\n");
}

#endif // FLASH_DISK_EN
