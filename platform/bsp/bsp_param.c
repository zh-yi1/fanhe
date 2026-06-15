#include "include.h"

#define CM_SIZE         FLASH_CM_SIZE
#define CM_START        (FLASH_SIZE - CM_SIZE)

#define param_read8(a, b)       b = cm_read8(PAGE0(a))
#define param_write8(a, b)      cm_write8(PAGE0(a), b)
#define param_read16(a, b)      b = cm_read16(PAGE0(a))
#define param_write16(a, b)     cm_write16(PAGE0(a), b)
#define param_read32(a, b)      b = cm_read32(PAGE0(a))
#define param_write332(a, b)    cm_write32(PAGE0(a), b)
#define param_read(a, b, c)     cm_read(a, PAGE0(b), c)
#define param_write(a, b, c)    cm_write(a, PAGE0(b), c)
#define param_sync_do()         cm_sync()


AT(.text.bsp.param)
void param_init(bool reset)
{
    printf("%s(%d)\r\n", __func__, __LINE__);
    cm_init(MAX_CM_PAGE, CM_START, CM_SIZE);
    printf("CM: %x\n", cm_read8(PAGE0(0)));
    printf("CM: %x\n", cm_read8(PAGE1(0)));

    printf("%s(%d)\r\n", __func__, __LINE__);
    sys_cb.rand_seed = sys_get_rand_key();

    printf("%s(%d)\r\n", __func__, __LINE__);
    param_sys_vol_read();
    if (sys_cb.vol < SYS_LIMIT_VOLUME) {
        sys_cb.vol = SYS_INIT_VOLUME;
    }
    printf("%s(%d)\r\n", __func__, __LINE__);
    sys_cb.hfp_vol = sys_cb.vol / sys_cb.hfp2sys_mul;
    printf("%s(%d)\r\n", __func__, __LINE__);
}

AT(.text.bsp.param)
void bsp_param_write(u8 *buf, u32 addr, uint len)
{
    param_write(buf, addr, len);
}

AT(.text.bsp.param)
void bsp_param_read(u8 *buf, u32 addr, uint len)
{
    param_read(buf, addr, len);
}

AT(.text.bsp.param)
void bsp_param_sync(void)
{
    param_sync_do();
}

#if BT_LOCAL_ADDR
AT(.text.bsp.param)
void param_random_key_read(u8 *key)
{
    if (key == NULL) {
        return;
    }
    param_read(key, PARAM_RANDOM_KEY, 4);
}

AT(.text.bsp.param)
void param_random_key_write(void)
{
    u32 key;
    param_random_key_read((u8*)&key);
    if (key == 0 || key == UINT_MAX) {
        key = sys_cb.rand_seed;
        param_write((u8 *)&key, PARAM_RANDOM_KEY, 4);
    }
}
#endif // BT_LOCAL_ADDR

AT(.text.bsp.param)
void param_sys_vol_write(void)
{
    param_write((u8 *)&sys_cb.vol, PARAM_SYS_VOL, 1);
}

AT(.text.bsp.param)
void param_sys_vol_read(void)
{
    param_read((u8 *)&sys_cb.vol, PARAM_SYS_VOL, 1);
}

AT(.text.bsp.param)
void param_lang_id_write(void)
{
    param_write((u8 *)&sys_cb.lang_id, PARAM_LANG_ID, 1);
}

AT(.text.bsp.param)
void param_lang_id_read(void)
{
    param_read((u8 *)&sys_cb.lang_id, PARAM_LANG_ID, 1);
}

AT(.text.bsp.param)
void param_sync(void)
{
    param_sync_do();
}

AT(.text.bsp.param)
u32 param_get_xosc_addr(void)
{
    return PARAM_BT_XOSC_CAP;
}

#if FUNC_MUSIC_EN
AT(.text.bsp.param)
void param_msc_num_write(void)
{
    if (msc_cb.cur_dev == DEV_SDCARD) {
        param_write((u8 *)&msc_cb.file_num, PARAM_MSC_NUM_SD, 2);
    } else if (msc_cb.cur_dev == DEV_SDCARD1) {
        param_write((u8 *)&msc_cb.file_num, PARAM_MSC_NUM_SD1, 2);
    } else {
        param_write((u8 *)&msc_cb.file_num, PARAM_MSC_NUM_USB, 2);
    }
}

AT(.text.bsp.param)
void param_msc_num_read(void)
{
    if (msc_cb.cur_dev == DEV_SDCARD) {
        param_read((u8 *)&msc_cb.file_num, PARAM_MSC_NUM_SD, 2);
    } else if (msc_cb.cur_dev == DEV_SDCARD1) {
        param_read((u8 *)&msc_cb.file_num, PARAM_MSC_NUM_SD1, 2);
    } else {
        param_read((u8 *)&msc_cb.file_num, PARAM_MSC_NUM_USB, 2);
    }
    if (msc_cb.file_num >= msc_cb.file_total) {
        msc_cb.file_num = msc_cb.file_total - 1;
    }
    if (msc_cb.file_num < 0) {
        msc_cb.file_num = 0;
    }
}

#if MUSIC_BREAKPOINT_EN
AT(.text.bsp.param)
void param_msc_breakpoint_write(void)
{
    if (msc_cb.cur_dev == DEV_SDCARD) {
        param_write((u8 *)&msc_cb.brkpt, PARAM_MSC_BRKPT_SD, 10);
    } else if (msc_cb.cur_dev == DEV_SDCARD1) {
        param_write((u8 *)&msc_cb.brkpt, PARAM_MSC_BRKPT_SD1, 10);
    } else {
        param_write((u8 *)&msc_cb.brkpt, PARAM_MSC_BRKPT_USB, 10);
    }
}

AT(.text.bsp.param)
void param_msc_breakpoint_read(void)
{
    if (msc_cb.cur_dev == DEV_SDCARD) {
        param_read((u8 *)&msc_cb.brkpt, PARAM_MSC_BRKPT_SD, 10);
    } else if (msc_cb.cur_dev == DEV_SDCARD1) {
        param_read((u8 *)&msc_cb.brkpt, PARAM_MSC_BRKPT_SD1, 10);
    } else {
        param_read((u8 *)&msc_cb.brkpt, PARAM_MSC_BRKPT_USB, 10);
    }
}
#endif // MUSIC_BREAKPOINT_EN
#endif // FUNC_MUSIC_EN

#if FUNC_FMRX_EN
AT(.text.bsp.param)
void param_fmrx_chcur_write(void)
{
    param_write(&fmrx_cb.ch_cur, PARAM_FMRX_CHCUR, 1);
}

AT(.text.bsp.param)
void param_fmrx_chcur_read(void)
{
    param_read(&fmrx_cb.ch_cur, PARAM_FMRX_CHCUR, 1);
}

AT(.text.bsp.param)
void param_fmrx_chcnt_write(void)
{
    param_write(&fmrx_cb.ch_cnt, PARAM_FMRX_CHCNT, 1);
}

AT(.text.bsp.param)
void param_fmrx_chcnt_read(void)
{
    param_read(&fmrx_cb.ch_cnt, PARAM_FMRX_CHCNT, 1);
}

AT(.text.bsp.param)
void param_fmrx_chbuf_write(void)
{
    param_write((u8 *)fmrx_cb.buf, PARAM_FMRX_CHBUF, 26);
}

AT(.text.bsp.param)
void param_fmrx_chbuf_read(void)
{
    param_read((u8 *)fmrx_cb.buf, PARAM_FMRX_CHBUF, 26);
}
#endif // FUNC_FMRX_EN

#if LE_AB_FOT_EN

AT(.text.bsp.param)
void param_fot_addr_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_ADDR, 4);
    param_sync();
}

AT(.text.bsp.param)
void param_fot_addr_read(u8 *param)
{
    param_read(param, PARAM_FOT_ADDR, 4);
}

AT(.text.bsp.param)
void param_fot_remote_ver_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_REMOTE_VER, 2);
    param_sync();
}

AT(.text.bsp.param)
void param_fot_remote_ver_read(u8 *param)
{
    param_read(param, PARAM_FOT_REMOTE_VER, 2);
}

AT(.text.bsp.param)
void param_fot_head_info_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_HEAD_INFO, 8);
    param_sync();
}

AT(.text.bsp.param)
void param_fot_head_info_read(u8 *param)
{
    param_read(param, PARAM_FOT_HEAD_INFO, 8);
}

AT(.text.bsp.param)
void param_fot_hash_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_HASH, 4);
    param_sync();
}

AT(.text.bsp.param)
void param_fot_hash_read(u8 *param)
{
    param_read(param, PARAM_FOT_HASH, 4);
}

AT(.text.bsp.param.fota)
void param_fot_type_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_TYPE, 1);
    param_sync();
}

AT(.text.bsp.param.fota)
void param_fot_type_read(u8 *param)
{
    param_read(param, PARAM_FOT_TYPE, 1);
}

#endif


#if (ASR_SELECT == ASR_WS_AIR && IR_AIR_FUNC)
/*********************************************************************************
 * 空伴：flash 数据
 *********************************************************************************/
 #include "../ws_air/air/asr_config.h"
AT(.text.air.param)
void air_flash_write(void)
{
    _ISD_REMOTE_DATA isd_remote_data_tmp;
    isd_remote_data_tmp = isd_remote_data;
    param_write(&isd_remote_data_tmp, PARAM_AIR_REMOTE, sizeof(_ISD_REMOTE_DATA));
    param_sync();
}

AT(.text.air.param)
int air_flash_read(void)
{
    _ISD_REMOTE_DATA isd_remote_data_tmp;
    param_read(&isd_remote_data_tmp, PARAM_AIR_REMOTE, sizeof(_ISD_REMOTE_DATA));
    isd_remote_data = isd_remote_data_tmp;

    printf("read---> offset=%d, len=%d, brand=%d, index=%d \n",
        isd_remote_data.offset, isd_remote_data.len,
        isd_remote_data.brand_id, isd_remote_data.brand_index);

    if(isd_remote_data.len == 0) {
        return -1;
    }
    return 0;
}
#endif

