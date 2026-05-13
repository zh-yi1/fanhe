#include "include.h"

#if FLASH_DISK_EN
extern void spi_flash_disk_flush(void);
extern void spi_flash_disk_initial(u32 spi_flash_start, u32 spi_flash_len);
extern void spi_flash_disk_read_sector(u32 blkno, u32 blkcnt, u32 buf);
extern void spi_flash_disk_write_sector(u32 blkno, u32 blkcnt, u32 buf);

#define C_BUFFERSIZE       0x400  //4k

typedef struct
{
    u32 spiflash_offset;
    u32 spiflash_len;
    u32 Flag;
    u32 BlkNo;   //the block number
    u32 Buffer[C_BUFFERSIZE];
}SPI_FLASH_BUFFER_STRUCT;

static SPI_FLASH_BUFFER_STRUCT  spi_flash_buffer_struct AT(.spiflash_disk);

WEAK SPI_FLASH_BUFFER_STRUCT* disk_spiflash_get_handle(void)
{
    return &spi_flash_buffer_struct;
}

//usb device
u32 get_spiflash_capacity(void)
{
    return FLASH_DISK_LEN / 512;;
}


//A:
static u8 drv_spiflash_is_online(void)
{
    return RES_OK;
}


static u8 drv_spiflash_init(void)
{
    spi_flash_disk_initial(FLASH_DISK_START, FLASH_DISK_LEN);
    return RES_OK;
}


static u8 drv_spiflash_ioctl (u8 cmd, void *buff)
{

	u8 res = RES_OK;

    if(cmd == CTRL_SYNC){
        spi_flash_disk_flush();
    }else if(cmd == GET_SECTOR_COUNT){
        *(LBA_t *)buff = FLASH_DISK_LEN / 512;
    }else if(cmd == GET_SECTOR_SIZE){
        *(WORD *)buff = 512;
    }else if(cmd == GET_BLOCK_SIZE){
        *(DWORD *)buff = 512;
    }else if(cmd == CTRL_TRIM){

    }
	return res;
}


static u8 drv_spiflash_read(
	u8 *buff,		/* Data buffer to store read data */
	u32 sector,	    /* Start sector in LBA */
	u32 count		/* Number of sectors to read */
){
    u32 addr = sector * 512;
    u32 len  = count  * 512;

    if(((addr + len) > (FLASH_DISK_LEN ))){
        return RES_PARERR;
    }

    os_gui_draw_w4_done();
    spi_flash_disk_read_sector(sector, count, (u32)buff);
    return RES_OK;
}


static u8 drv_spiflash_write(
	u8 *buff,		/* Data buffer to store read data */
	u32 sector,	/* Start sector in LBA */
	u32 count		/* Number of sectors to read */
){
    u32 addr = sector * 512;
    u32 len  = count  * 512;

    if(((addr + len) > (FLASH_DISK_LEN ))){
        return RES_PARERR;
    }

    os_gui_draw_w4_done();
    spi_flash_disk_write_sector(sector, count, (u32)buff);
    return RES_OK;
}


fs_disk_cb_t drv_spiflash_cb = {
    .disk_is_online = drv_spiflash_is_online,
    .disk_init      = drv_spiflash_init,
    .disk_read      = drv_spiflash_read,
    .disk_write     = drv_spiflash_write,
    .disk_ioctl     = drv_spiflash_ioctl
};
#else
void spi_flash_disk_flush(void){}
void spi_flash_disk_initial(u32 spi_flash_start, u32 spi_flash_len){}
void spi_flash_disk_read_sector(u32 blkno, u32 blkcnt, u32 buf){}
void spi_flash_disk_write_sector(u32 blkno, u32 blkcnt, u32 buf){}
#endif
