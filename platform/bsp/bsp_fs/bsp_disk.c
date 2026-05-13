#include "include.h"
#include "bsp_disk.h"

extern fs_disk_cb_t drv_sd_cb;
extern fs_disk_cb_t drv_spiflash_cb;

fs_disk_cb_t *disk_cb[3] = {
/* A:> */
#if FLASH_DISK_EN
    &drv_spiflash_cb,
#else
    NULL,
#endif

/* B:> */
#if SD_SUPPORT_EN
    &drv_sd_cb,
#else
    NULL,
#endif // SD_SUPPORT_EN

/* C:> */
    NULL,
};

/*FS底层会调用*/
u32 get_fattime(void)
{
    tm_t rtc_tm = time_to_tm(RTCCNT);           	//更新时间结构体
    return ((DWORD)(rtc_tm.year - 1980) << 25
            | (DWORD)rtc_tm.mon << 21
            | (DWORD)rtc_tm.day << 16
            | (DWORD)rtc_tm.hour << 11
            | (DWORD)rtc_tm.min << 5
            | (DWORD)(rtc_tm.sec / 2));
}

void *sd_buf_malloc(u32 size)
{
    return ab_malloc(size);
}

void sd_buf_free(void *ptr)
{
    ab_free(ptr);
}

#if SD_SUPPORT_EN
static fatfs_t sd_fatfs AT(.fatfs.sd);
static FATFS fat_fs AT(.fatfs.sd);
static bool is_sd_mount AT(.fatfs.sd);

bool bsp_sd_mount_state(void)
{
    return is_sd_mount;
}

u64 bsp_sd_disk_get_free(void)
{
    if (!is_sd_mount) {
        return 0;
    }
    u64 free_size = fs_getfree ("B:");
    return free_size;
}

/**
 * SD卡 挂载磁盘
 */
void bsp_sd_disk_mount(u16 sd_Mhz)
{
    if (is_sd_mount) {
        return;
    }
    sd0_set_speed_mhz(sd_Mhz);
    fs_fcache_init();
    FRESULT res = fs_mount (&fat_fs, "B:", 1);
    u64 free_size = fs_getfree ("B:");
    free_size = free_size / 1024;
    if (res == FR_OK) {
        is_sd_mount = true;
    }
    printf("f_mount:%d 0x%x\n", res, free_size);
}

/**
 * SD卡 卸载磁盘
 */
void bsp_sd_disk_unmount(void)
{
    if (!is_sd_mount) {
        return;
    }
    FRESULT res = fs_unmount ("B:");
    if (res == FR_OK) {
        is_sd_mount = false;
    }
    sd0_stop(1);
    printf("sd disk fs_unmount:%d\n", res);
}

/**
 * SD卡 格式化
 */
void bsp_sd_disk_format(void)
{
    if (!is_sd_mount) {
        return;
    }
    u8 *work_buff = ab_malloc(512);

	if(work_buff != NULL){

        static MKFS_PARM fopt = {0x07, 0, 0, 0, 64*1024};    /* Default parameter */
	    FRESULT res = fs_mkfs ("B:", &fopt, work_buff, 512);    /* Create a FAT volume */
		ab_free(work_buff);

		printf("fs format:%d\n", res);
        if(res == FR_OK){
            msg_enqueue(EVT_SD_INSERT);
        }
	}
}



//static u8 file_fast_buff[4096];

fatfs_t* bsp_sd_disk_get_fatfs(void)
{
    if (!is_sd_mount) {
        return NULL;
    }
    return &sd_fatfs;
}

/**
 * SD卡 扫描固定名字的文件
 * @param[in] scan_path: 要扫描的起始路径
 * @param[in] file_type: 只扫描文件后缀（会自动去扫起始路劲下所有合适的文件）
 * @param[in] match_func: 用户自定义扫描方式，这个定义之后file_type参数失效
 * @param[in] 返回扫描到合适文件的总数
 */
u32 bsp_sd_disk_scan(const char* scan_path, const char* file_type,  bool (*match_func)(BYTE attr, char *name))
{
    if (!is_sd_mount) {
        return 0;
    }
    FRESULT res;
    static DWORD filenum, foldnum;
    sd_fatfs.stdscan.fbuff     = sd_fatfs.scanbuff;
    sd_fatfs.stdscan.fbuff_len = sizeof(sd_fatfs.scanbuff);
    sd_fatfs.stdscan.foldbuff  = sd_fatfs.scanfoldbuff;
    sd_fatfs.stdscan.foldbuff_len  = sizeof(sd_fatfs.scanfoldbuff);
    sd_fatfs.stdscan.path    =  (char*)scan_path;

    if (match_func) {
        sd_fatfs.stdscan.match_func = match_func;
    } else {
        strcpy(sd_fatfs.stdscan.pattern, (char*)file_type);
    }
    fs_chdrive (sd_fatfs.stdscan.path);
    res = fs_scan_disk(&sd_fatfs.stdscan, &filenum, &foldnum);
    printf("%s:%d, filenum:%d, foldnum:%d\n", __func__, res, filenum, foldnum);

    if (res == FR_OK) {
        for (DWORD i=0; i<filenum; i++){
            res = fs_node2file_info (&sd_fatfs.stdscan, i, &sd_fatfs.fno);
            if(res == FR_OK){
//                printf("info:%d : %s\n", i, sd_fatfs.fno.fname);
            }
        }
    }

    if((filenum == 0) || res){
        return 0;
    }

    return filenum;
}

bool bsp_sd_disk_open_file_idx(u32 idx)
{
    if (!is_sd_mount) {
        return 0;
    }
    FRESULT res = fs_node2file_info (&sd_fatfs.stdscan, idx, &sd_fatfs.fno);
    if(res == FR_OK){
        res = fs_openfno (&sd_fatfs.fp, &sd_fatfs.fno, FA_READ);
        if(res == FR_OK){
//            fs_creat_fastmap(&sd_fatfs.fp, file_fast_buff, sizeof(file_fast_buff));
        }
    }

    return (res == FR_OK);
}

bool bsp_sd_disk_close_file(void)
{
    if (!is_sd_mount) {
        return 0;
    }
    FRESULT res = fs_close(&sd_fatfs.fp);
    return (res == FR_OK);
}

#else
bool bsp_sd_disk_close_file(void){return false;}
bool bsp_sd_disk_open_file_idx(u32 idx){return false;}
u32 bsp_sd_disk_scan(const char* scan_path, const char* file_type,  bool (*match_func)(BYTE attr, char *name)){return 0;}
fatfs_t* bsp_sd_disk_get_fatfs(void){return NULL;}
void bsp_sd_disk_format(void){}
void bsp_sd_disk_unmount(void){}
void bsp_sd_disk_mount(u16 sd_Mhz){}
u64 bsp_sd_disk_get_free(void){return 0;}
bool bsp_sd_mount_state(void){return 0;}
#endif // SD_SUPPORT_EN


#if FLASH_DISK_EN
static FATFS fat_flash_fs AT(.fatfs.spiflash);
static fatfs_t flash_fatfs AT(.fatfs.spiflash);
static bool is_flash_mount AT(.fatfs.spiflash);

bool bsp_flash_mount_state(void)
{
    return is_flash_mount;
}

u64 bsp_flash_disk_get_free(void)
{
    if (!is_flash_mount) {
        return 0;
    }
    u64 free_size = fs_getfree ("A:");
    return free_size;
}

/**
 * spiflash 挂载磁盘
 */
bool bsp_flash_disk_mount(void)
{
    if (is_flash_mount) {
        return true;
    }
    fs_fcache_init();
    FRESULT res = fs_mount (&fat_flash_fs, "A:", 1);
    printf("flash disk res:%d\n", res);
    if (res == FR_NO_FILESYSTEM) {
        bsp_flash_disk_format();
    }

	if(res == FR_OK){
		u64 free_size = fs_getfree ("A:");
		free_size = free_size / 1024;

		printf("flash disk:%d %d kbytes\n", res, free_size);
		is_flash_mount = true;
		return true;
	}
	return false;
}

/**
 * spiflash 格式化
 */
void bsp_flash_disk_format(void)
{
    u8 *work_buff = ab_malloc(512);

	if(work_buff != NULL){
        static MKFS_PARM fopt = {0x07, 0, 0, 0, 64*1024};    /* Default parameter */
        os_gui_draw_w4_done();
	    FRESULT res = fs_mkfs ("A:", &fopt, work_buff, 512);    /* Create a FAT volume */
		ab_free(work_buff);
		printf("flash disk format:%d\n", res);
        res = fs_mount (&fat_flash_fs, "A:", 1);
		printf("flash disk fs_mount:%d\n", res);
	}
}

void bsp_flash_disk_unmount(void)
{
    if (!is_flash_mount) {
        return;
    }
    FRESULT res = fs_unmount ("A:");
    if (res == FR_OK) {
        is_flash_mount = false;
    }
    printf("flash disk fs_unmount:%d\n", res);
}



//static u8 file_fast_buff[4096];

fatfs_t* bsp_flash_disk_get_fatfs(void)
{
    if (!is_flash_mount) {
        return NULL;
    }
    return &flash_fatfs;
}

u32 bsp_flash_disk_scan(const char* scan_path, const char* file_type,  bool (*match_func)(BYTE attr, char *name))
{
    if (!is_flash_mount) {
        return 0;
    }
    FRESULT res;
    static DWORD filenum, foldnum;
    flash_fatfs.stdscan.fbuff     = flash_fatfs.scanbuff;
    flash_fatfs.stdscan.fbuff_len = sizeof(flash_fatfs.scanbuff);
    flash_fatfs.stdscan.foldbuff  = flash_fatfs.scanfoldbuff;
    flash_fatfs.stdscan.foldbuff_len  = sizeof(flash_fatfs.scanfoldbuff);
    flash_fatfs.stdscan.path    =  (char*)scan_path;

    if (match_func) {
        flash_fatfs.stdscan.match_func = match_func;
    } else {
        strcpy(flash_fatfs.stdscan.pattern, (char*)file_type);
    }
    fs_chdrive (flash_fatfs.stdscan.path);
    res = fs_scan_disk(&flash_fatfs.stdscan, &filenum, &foldnum);
    printf("%s:%d, filenum:%d, foldnum:%d\n", __func__, res, filenum, foldnum);

    if (res == FR_OK) {
        for (DWORD i=0; i<filenum; i++){
            res = fs_node2file_info (&flash_fatfs.stdscan, i, &flash_fatfs.fno);
            if(res == FR_OK){
                printf("info:%d : %s\n", i, flash_fatfs.fno.fname);
            }
        }
    }

    if((filenum == 0) || res){
        return 0;
    }

    return filenum;
}

bool bsp_flash_disk_open_file_idx(u32 idx)
{
    if (!is_flash_mount) {
        return 0;
    }
    FRESULT res = fs_node2file_info (&flash_fatfs.stdscan, idx, &flash_fatfs.fno);
    if(res == FR_OK){
        res = fs_openfno (&flash_fatfs.fp, &flash_fatfs.fno, FA_READ);
        if(res == FR_OK){
//            fs_creat_fastmap(&flash_fatfs.fp, file_fast_buff, sizeof(file_fast_buff));
        }
    }

    return (res == FR_OK);
}

bool bsp_flash_disk_close_file(void)
{
    if (!is_flash_mount) {
        return 0;
    }
    FRESULT res = fs_close(&flash_fatfs.fp);
    return (res == FR_OK);
}

#else
bool bsp_flash_disk_mount(void){return 0;}
void bsp_flash_disk_format(void){}
bool bsp_flash_disk_open_file_idx(u32 idx){return false;}
u32 bsp_flash_disk_scan(const char* scan_path, const char* file_type,  bool (*match_func)(BYTE attr, char *name)){return 0;}
fatfs_t* bsp_flash_disk_get_fatfs(void){return NULL;}
void bsp_flash_disk_unmount(void){}
bool bsp_flash_disk_close_file(void){return 0;}
bool bsp_flash_mount_state(void){return false;}
u64 bsp_flash_disk_get_free(void){return 0;}
#endif


