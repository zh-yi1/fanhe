#include "include.h"
#include "api_fs.h"

#if 0

static FATFS fat_fs;
static FILINFO fno;    /* File information */
static u8 read_buff[17] = {0};
static FIL fp;

void fs_findall_demo(void)
{
    FRESULT ret;
    UINT bw;

    ret = fs_mount (&fat_fs, "B:", 1);           /* Mount/Unmount a logical drive */
    printf("f_mount:%d\n", ret);

    ret = fs_findall_frist(&fno, "B:\\", "*.txt", D_FILE, NULL);
    if(ret == FR_OK){
        printf("%s\n", fno.fname);
    }

    while (ret == FR_OK && fno.fname[0]) {         /* Repeat while an item is found */
        ret = fs_findall_next(&fno);

        if(ret == FR_OK && fno.fname[0]){
            printf("%s\n", fno.fname);
            if(fs_openfno (&fp, &fno, FA_READ|FA_WRITE) == FR_OK){
                if(fs_read (&fp, read_buff, 16, &bw) == FR_OK){
                    read_buff[16] = 0;
                    printf("read:%s\n", read_buff);
                }
                fs_close(&fp);
            }
        }
        WDT_CLR();
    }
}

#endif
