#include "config.h"
depend(0x01010200);
setflash(1, FLASH_SIZE, FLASH_ERASE_4K, FLASH_DUAL_READ, FLASH_QUAD_READ);
setspace(FLASH_CM_SIZE);
#if FOTA_DOG_EN
setuserbin(FLASH_UI_BASE, FLASH_UI_SIZE, ui.bin, 0);   #起始地址要与ui.xm同步 升级狗导出不带UI资源文件
#else
setuserbin(FLASH_UI_BASE, FLASH_UI_SIZE, ui.bin, 0);   #起始地址要与ui.xm同步
#endif
#if FLASHDB_EN
setkeepseg(0x4000); //数据库使用
#endif
//压缩升级只看 AB_FOT_TYPE_PACK (饭盒 OTA 走自研 lb_ota 通道, LE_AB_FOT_EN 原厂BLE服务保持0)
#if AB_FOT_TYPE_PACK
setunpack(unpack.bin);
setpkgarea(FLASH_PKG_START, FLASH_PKG_SIZE);
#endif
#if DONGLE_AUTH_EN
setauth(0xDDE05A0D, soft_key);
#endif
#if ASR_USBKEY_PSD
setauth(ASR_USBKEY_PSD, asr_soft_key);
#endif

#if BT_HFP_KEY
addkeyn(BT_HFP_KEY)
#endif

make(dcf_buf, header.bin, app.bin, res.bin, xcfg.bin, updater.bin);
save(dcf_buf, app.dcf);


//------------------------app.xm_cmd------------------------
//**setflash(param0, param1, param2, param3, param4);
//说明：     设置用户资源区
//param0:    type:默认1，代表flash
//param1:    FLASH_SIZE:flash大小
//param2:    FLASH_ERASE_4K:是否支持4K擦除
//param3:    FLASH_DUAL_READ:是否支持两线读取
//param4:    FLASH_QUAD_READ:是否支持4线读取
//示例：     setflash(1, FLASH_SIZE, FLASH_ERASE_4K, FLASH_DUAL_READ, FLASH_QUAD_READ)

//**setspace(param0)
//说明：    设置参数区大小
//param0:   size:参数区大小
//示例：    setspace(0x5000)

//**setuserbin(param0,param1,param2,param3)
//说明：     设置用户资源区
//param0:    start_addr:用户资源区起始地址
//param1:    bin_size:用户资源区大小
//param2:    bin_file:用户资源文件
//param3:    type:0:导出fot文件不带资源 1：导出fot文件携带压缩资源
//                2:导出fot携带资源，资源不压缩
//示例：     setuserbin(0x87000,0x304000, ui.bin,1);

//**setunpack(param0)
//说明：          设置FOTA压缩升级引导loader
//param0:file:    同一目录下bin文件
//示例：          setunpack(unpack.bin)

//**setpkgarea(param0,param1)
//说明：     设置FOTA压缩升级压缩包存放信息
//param0:    start_addr:压缩包存放起始地址
//param1:    end_addr:压缩包大小
//示例：     setunpack(0x400000,0x300000)
//压缩包存放起始地址为0x400000,最大为0x300000

//**setkeepseg(size)
//说明：     设置升级保持区
//param0:    末尾开始，扣除20k参数区后的大小，必须0x1000倍数
//------------------------app.xm_cmd------------------------


