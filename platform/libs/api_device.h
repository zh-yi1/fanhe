#ifndef _API_DEVICE_H
#define _API_DEVICE_H

#define DEV_FEAT_SDCARD             0x01            //支持SD卡
#define DEV_FEAT_SDCARD1            0x02            //支持SD1卡
#define DEV_FEAT_USB                0x04            //支持USB

enum {
    DEV_SDCARD = 0,
    DEV_SDCARD1,
    DEV_UDISK,
    DEV_USBPC,
    DEV_EARPHONE,
    DEV_LINEIN,
    DEV_MIC,
    DEV_HDMI,
    DEV_DP_GND,
    DEV_DPDM_SC,
    DEV_TOTAL_NUM,

    DEV_SPIFLASH = 0x30,   //读ID检测,不参与该头文件中的device检测机制
};

extern uint32_t cfg_dev_feat;
#define dev_sdcard_is_support()         (bool)(cfg_dev_feat & DEV_FEAT_SDCARD)
#define dev_sdcard1_is_support()        (bool)(cfg_dev_feat & DEV_FEAT_SDCARD1)
#define dev_usb_is_support()            (bool)(cfg_dev_feat & DEV_FEAT_USB)

void dev_init(void);
bool dev_is_online(u16 dev_num);
bool dev_online_filter(u16 dev_num);
bool dev_offline_filter(u16 dev_num);
void dev_delay_times(u8 dev_num, u8 times);          //同时设置, online去抖延时 和 offline 去抖延时, 它们的延时一样的,兼容以前.
void dev_delay_online_times(u8 dev_num, u8 times);   //单独设置online去抖延时(新增).
void dev_delay_offline_times(u8 dev_num, u8 times);  //单独设置offline 去抖延时.(新增)
void dev_clr_sta(u8 dev_num);
bool dev_udisk_activation_try(u8 mode);
bool dev_is_invalid(u8 dev_num);
void sd_disk_switch(u8 index);                       //双SD卡底层切换
void udisk_resume(void);
void udisk_suspend(void);
void sd_disable(void);
#endif // _API_DEVICE_H
