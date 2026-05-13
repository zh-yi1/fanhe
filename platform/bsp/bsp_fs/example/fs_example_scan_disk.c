#include "include.h"
#include "api_fs.h"

#if 0

#define FILE_BUFF_SIZE              DISKCAN_FILENOTE_SIZE * 20
#define FOLD_BUFF_SIZE              DISKCAN_FOLDNOTE_SIZE * 100

static FATFS fat_fs;
static FIL fp;
static FILINFO fno;    /* File information */
static u8 read_buff[17] = {0};
static STDSCAN_LE stdscan;
static u8 scanbuff[FILE_BUFF_SIZE];
static u8 scanfoldbuff[FOLD_BUFF_SIZE];
static TCHAR scan_path[] = "B:\\";

void fs_scandisk_demo(void)
{
    FRESULT ret;
    UINT bw;
    DWORD file_num;
    DWORD fold_num;

    ret = fs_mount (&fat_fs, "B:", 1);
    printf("f_mount:%d\n", ret);

    u32 ticks = tick_get();
    stdscan.fbuff     = scanbuff;
    stdscan.fbuff_len = FILE_BUFF_SIZE;
    stdscan.foldbuff  = scanfoldbuff;
    stdscan.foldbuff_len  = DISKCAN_FOLDNOTE_SIZE;
    stdscan.path    =  scan_path;
    strcpy(stdscan.pattern, "*.txt");
    stdscan.match_func = NULL;

    ret = fs_scan_disk(&stdscan, &file_num, &fold_num);

    printf("scan_disk:%d %d %d\n", ret, file_num, fold_num);
    printf("tick:%d\n", tick_get()-ticks);

    if(ret == FR_OK){
        for(DWORD i=0; i<file_num; i++){
            ret = fs_node2file_info (&stdscan, i, &fno);
            if(ret == FR_OK){
                printf("info:%d : %s\n", i, fno.fname);
                if(fs_openfno (&fp, &fno, FA_READ|FA_WRITE) == FR_OK){
                    if(fs_read (&fp, read_buff, 16, &bw) == FR_OK){
                        read_buff[16] = 0;
                        printf("read:%s\n", read_buff);
                    }
                    fs_close(&fp);
                }
            }
        }

        for(DWORD i=0; i<fold_num; i++){
            ret = fs_node2folder_info (&stdscan, i, &fno);
            if(ret == FR_OK){
                printf("info:%d : %s\n", i, fno.fname);
            }
        }

        for(DWORD i=0; i<fold_num; i++){
            DWORD fileinfolder_num;
            ret = fs_numoffolder (&stdscan, i, &fileinfolder_num);
            if(ret == FR_OK){
                printf("foldid:%d : %d\n", i, fileinfolder_num);
            }
        }

        for(DWORD i=0; i<file_num; i++){
            DWORD folder_id;
            ret = fs_file2folder_index (&stdscan, i, &folder_id);
            if(ret == FR_OK){
                printf("fileid:%d : %d\n", i, folder_id);
            }
        }

        for(DWORD i=0; i<fold_num; i++){
            DWORD file_id;
            ret = fs_folder2file_index (&stdscan, i, &file_id);
            if(ret == FR_OK){
                printf("foldid:%d : %d\n", i, file_id);
            }
        }
    }
}

#endif
