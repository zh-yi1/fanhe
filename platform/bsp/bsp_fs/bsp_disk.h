/*-----------------------------------------------------------------------/
/  Low level disk interface modlue include file   (C)ChaN, 2019          /
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED


typedef struct _disk_cb_t {
    u8 (*disk_is_online)(void);
    u8 (*disk_init)(void);
    u8 (*disk_read)(u8 *buff,u32 sector,u32 count);
    u8 (*disk_write)(u8 *buff, u32 sector, u32 count);
    u8 (*disk_ioctl) (u8 cmd, void *buff);
}fs_disk_cb_t;


/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;


/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

/* Generic command (Used by FatFs) */
#define CTRL_SYNC			0	/* Complete pending write process (needed at FF_FS_READONLY == 0) */
#define GET_SECTOR_COUNT	1	/* Get media size (needed at FF_USE_MKFS == 1) */
#define GET_SECTOR_SIZE		2	/* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (needed at FF_USE_MKFS == 1) */
#define CTRL_TRIM			4	/* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */

/* Generic command (Not used by FatFs) */
#define CTRL_POWER			5	/* Get/Set power status */
#define CTRL_LOCK			6	/* Lock/Unlock media removal */
#define CTRL_EJECT			7	/* Eject media */
#define CTRL_FORMAT			8	/* Create physical format on the media */

/* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE		10	/* Get card type */
#define MMC_GET_CSD			11	/* Get CSD */
#define MMC_GET_CID			12	/* Get CID */
#define MMC_GET_OCR			13	/* Get OCR */
#define MMC_GET_SDSTAT		14	/* Get SD status */
#define ISDIO_READ			55	/* Read data form SD iSDIO register */
#define ISDIO_WRITE			56	/* Write data to SD iSDIO register */
#define ISDIO_MRITE			57	/* Masked write data to SD iSDIO register */

/* ATA/CF specific ioctl command */
#define ATA_GET_REV			20	/* Get F/W revision */
#define ATA_GET_MODEL		21	/* Get model name */
#define ATA_GET_SN			22	/* Get serial number */

typedef struct {
    FILINFO         fno;
    FIL             fp;
    STDSCAN_LE      stdscan;
    u8              scanbuff[DISKCAN_FILENOTE_SIZE * 16];
    u8              scanfoldbuff[DISKCAN_FOLDNOTE_SIZE * 20];
} fatfs_t;

bool bsp_sd_mount_state(void);
void bsp_sd_disk_mount(u16 sd_Mhz);
void bsp_sd_disk_unmount(void);
void bsp_sd_disk_format(void);
u32 bsp_sd_disk_scan(const char* scan_path, const char* file_type,  bool (*match_func)(BYTE attr, char *name));
bool bsp_sd_disk_open_file_idx(u32 idx);
bool bsp_sd_disk_close_file(void);
fatfs_t* bsp_sd_disk_get_fatfs(void);
u64 bsp_sd_disk_get_free(void);

bool bsp_flash_mount_state(void);
bool bsp_flash_disk_mount(void);
void bsp_flash_disk_format(void);
u64 bsp_flash_disk_get_free(void);
void bsp_flash_disk_unmount(void);
fatfs_t* bsp_flash_disk_get_fatfs(void);
u32 bsp_flash_disk_scan(const char* scan_path, const char* file_type,  bool (*match_func)(BYTE attr, char *name));
bool bsp_flash_disk_open_file_idx(u32 idx);
bool bsp_flash_disk_close_file(void);

#endif
