#include "include.h"

//B:
static u8 drv_sd_is_online(void)
{
//    printf("%s\n", __func__);

    bool result = dev_is_online(DEV_SDCARD);

    return !result;
}


static u8 drv_sd_init(void)
{
    printf("%s\n", __func__);
    bool result = sd0_init();

    return !result;
}


static u8 drv_sd_ioctl (u8 cmd, void *buff)
{

	u8 res = RES_OK;

    if(cmd == CTRL_SYNC){
        sd0_stop(1);
    }else if(cmd == GET_SECTOR_COUNT){
        *(LBA_t *)buff = get_sdcard_capacity();
    }else if(cmd == GET_SECTOR_SIZE){
        *(WORD *)buff = 512;
    }else if(cmd == GET_BLOCK_SIZE){
        *(DWORD *)buff = 512;
    }else if(cmd == CTRL_TRIM){

    }
	return res;
}


static u8 drv_sd_read(
	u8 *buff,		/* Data buffer to store read data */
	u32 sector,	/* Start sector in LBA */
	u32 count		/* Number of sectors to read */
){
    bool ret;

    if(buff == NULL){
        return RES_ERROR;
    }

    if(count == 0){
        memset(buff, 0, 512);
        return RES_OK;
    }

    for(UINT i=0; i<count; i++){
        ret = sd0_read(buff, sector);
        if(ret == false){
            return RES_ERROR;
        }
        sector++;
        buff += 512;
    }

    return RES_OK;
}


static u8 drv_sd_write(
	u8 *buff,		/* Data buffer to store read data */
	u32 sector,	/* Start sector in LBA */
	u32 count		/* Number of sectors to read */
){
    bool ret;

    if(buff == NULL){
        return 1;
    }

    if(count == 0){
        return RES_OK;
    }

    for(UINT i=0; i<count; i++){
        ret = sd0_write(buff, sector);
        if(ret == false){
            return false;
        }
        sector++;
        buff += 512;
    }

    return RES_OK;
}


fs_disk_cb_t drv_sd_cb = {
    .disk_is_online = drv_sd_is_online,
    .disk_init      = drv_sd_init,
    .disk_read      = drv_sd_read,
    .disk_write     = drv_sd_write,
    .disk_ioctl     = drv_sd_ioctl
};

