#ifndef __BSP_PARAM_H
#define __BSP_PARAM_H

#define PARAM_MSC_NUM_SD            0
#define PARAM_MSC_NUM_USB           2
#define PARAM_SYS_VOL               4
#define PARAM_HSF_VOL               5
#define PARAM_LANG_ID               6
#define PARAM_RANDOM_KEY            7           //4Byte随机数
#define PARAM_BT_XOSC_CAP           0x0b        //1Byte xosc value
#define PARAM_SYS_MODE              0x0c        //系统模式记忆
#define PARAM_MSC_NUM_SD1           0x0d        //2BYTE

#define PARAM_MSC_BRKPT_SD          0x10        //10BYTE = frame_count(4byte) +  file_ptr(4byte) + fname_crc(2byte)
#define PARAM_MSC_BRKPT_USB         0x1a        //10BYTE = frame_count(4byte) +  file_ptr(4byte) + fname_crc(2byte)
#define PARAM_MSC_BRKPT_SD1         0x24        //10BYTE = frame_count(4byte) +  file_ptr(4byte) + fname_crc(2byte)

#define PARAM_FMRX_CHCUR            0x30
#define PARAM_FMRX_CHCNT            0x31
#define PARAM_FMRX_CHBUF            0x32        //26Byte   [0x32,0x4C)

#define PARAM_MASTER_ADDR_VALID     0x50        //RFU
#define PARAM_MASTER_ADDR           0x51        //6BYTE
#define PARAM_NEW_LOCAL_NAME        0x57        //64byte=len+sum+62BYTE

#define RTCRAM_PWROFF_FLAG          63         //软关机的标识放在RTCRAM的最后一BYTE

#define BT_NAME_UPDATA              0x98        //U盘升级蓝牙名字 //32+4 byte

#define PARAM_FOT_ADDR              0xa2        //4BYTE
#define PARAM_FOT_REMOTE_VER        0xa6        //2BYTE
#define PARAM_FOT_HEAD_INFO         0xa8        //8BYTE
#define PARAM_FOT_HASH              0xb0        //4BYTE

#if !LP_XOSC_CLOCK_EN
#define PARAM_RTC_CAL_VALID         0xb4		//1BYTE
#define PARAM_RTC_CAL_ADDR          0xb5		//16BYTE
#endif

#define PARAM_ALI_FILE_VALID        0xcb        //1byte

#define PARAM_FOT_TYPE              0xec        //1byte

#if (ASR_SELECT == ASR_WS_AIR && IR_AIR_FUNC)
#define PARAM_AIR_REMOTE            0xed        //12byte
#endif

void param_init(bool reset);
void param_sync(void);
void param_msc_num_write(void);
void param_msc_num_read(void);
void param_msc_breakpoint_write(void);
void param_msc_breakpoint_read(void);
void param_sys_vol_write(void);
void param_sys_vol_read(void);
void param_lang_id_write(void);
void param_lang_id_read(void);
void param_random_key_write(void);
void param_random_key_read(u8 *key);
u8 param_sys_mode_read(void);
void param_sys_mode_write(u8 mode);
void param_spiflash_breakpoint_write(void);
void param_spiflash_breakpoint_read(void);
void param_fot_type_read(u8 *param);
void param_fot_hash_read(u8 *param);
void param_fot_remote_ver_read(u8 *param);

//FMRX
void param_fmrx_chcur_write(void);
void param_fmrx_chcur_read(void);
void param_fmrx_chcnt_write(void);
void param_fmrx_chcnt_read(void);
void param_fmrx_chbuf_write(void);
void param_fmrx_chbuf_read(void);
#endif // __BSP_PARAM_H

