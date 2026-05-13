#ifndef _API_UPDATE_H
#define _API_UPDATE_H

typedef enum {
    FOT_ERR_OK,
    FOT_ERR_NO_UPDATA,
    FOT_ERR_KEY,
    FOT_ERR_FILE_FORMAT,
    FOT_ERR_FILE_TAG,
    FOT_ERR_FILE_DATA,
    FOT_ERR_MAC_SIZE,
    FOT_ERR_START,
    FOT_ERR_DOWN_START,
    FOT_ERR_HEAD2_START,
    FOT_ERR_DOWN_LAST,
    FOT_ERR_CRC_VERIFY,
    FOT_ERR_AREA_SIZE,
    /*以上err类型和库里面对应,通过fot_get_err()或ota_pack_get_err()获取到*/

    FOT_ERR_SEQ = 0x40,
    FOT_ERR_DATA_LEN,

    FOT_ERR_TWS_DISCONNECT = 0x80,

    FOT_UPDATE_PAUSE = 0xfd,
    FOT_UPDATE_CONTINUE = 0xfe,
    FOT_UPDATE_DONE = 0xff,
} FOT_ERR_ENUM;

//OTA PACK
/**
 * @name    void ota_pack_init(void)
 * @brief   初始化函数，在开始进行升级之前需调用该函数进行初始化操作
 * @return  NULL
 **/
void ota_pack_init(void);


/**
 * @name      bool ota_pack_write(u8 *buf);
 * @brief     写数据函数，收到fot文件的数据后通过该接口将数据写到flash，长度需固定为512字节
 * @param[in] buf 数据
 * @return    NULL
 **/
bool ota_pack_write(u8 *buf);


/**
 * @name    u8 ota_pack_get_err(void)
 * @brief   获取升级状态函数，升级过程有无错误发生可通过该函数进行获取
 * @return  error code
 **/
u8 ota_pack_get_err(void);


/**
 * @name    bool ota_pack_is_write_done(void)
 * @brief   判断当前fot数据是否全部接收完
 * @return  成功返回 true
            失败返回 fasle
 **/
bool ota_pack_is_write_done(void);


/**
 * @name    bool ota_pack_verify(void)
 * @brief   升级校验函数，数据接收完后需通过该接口进行数据的校验
 * @return  成功返回 true
            失败返回 fasle
 **/
bool ota_pack_verify(void);


/**
 * @name    bool ota_pack_done(void)
 * @brief   升级完成函数，在升级校验成功后调用，该函数会擦除之前旧的引导程序，
            下次复位后将跑压缩的引导程序进行解压缩操作
 * @return  成功返回 true
            失败返回 fasle
 **/
void ota_pack_done(void);


/**
 * @name    bool ota_pack_breakpoint_info_read(void)
 * @brief   断点获取函数，需要断点续传功能的，可通过该函数触发断点信息的获取，
            断点位置通过u32 ota_pack_get_curaddr(void)接口返回
 * @return  成功返回 true
            失败返回 fasle
 **/
bool ota_pack_breakpoint_info_read(void);


/**
 * @name    u32 ota_pack_get_curaddr(void)
 * @brief   获取断点位置
 * @return  断点位置
 **/
u32 ota_pack_get_curaddr(void);

/**
 * @name    u32 ota_pack_get_total_size(void)
 * @brief   获取代码压缩升级文件大小
 * @return  代码压缩升级文件大小
 **/
u32 ota_pack_get_total_size(void);

//OTA Common
void load_code_fota(void);
void unlock_code_fota(void);

int updatefile_init(const char *file);      //初始化UPDATE模块
void updateproc(void);                      //升级流程
void ota_enter(void);                       //进入OTA流程
void ota_exit(void);                        //退出OTA流程

#endif
