#ifndef _PLATFROM_CONFIG_H_
#define _PLATFROM_CONFIG_H_

//手表类型
#define     WATCH_TYPE                      SQUARE_WATCH

//支持的语言
#define     DEVICE_LANG                     (BIT(ENGLISH_EN) | BIT(CHINESE_ZH_CH) | BIT(ITALIAN_IT_IT) | BIT(SPANISH_ES_ES) | BIT(PORTUGUESE_PT_PT) |\
                                             BIT(RUSSIAN_RU_RU) | BIT(THAI_TH_TH) | BIT(ARABIC_AR_AE) | BIT(TURKISH_TR_TR) | BIT(VIETNAMESE_VI) |\
                                             BIT(DUTCH_NL_NL) | BIT(HEBREW_IW) | BIT(PERSIAN) | BIT(MALAYSIA_MS) | BIT(BURMESE) | BIT(HINDI))

//支持的功能
#define     FUNC_ONCE_CONNECT               (1 << SUPORT_ONCE_CONNECT)          //是否支持一键双连
#define     DEVICE_FUNC                     (BIT(SUPORT_WATCH_MARKET) | BIT(SUPORT_MESSAGE) | BIT(SUPORT_WEATHER) | BIT(SUPORT_CONTACTS) | \
                                             BIT(SUPORT_ECARD) | BIT(SUPORT_NODISTURB) | BIT(SUPORT_BRIGHTEN) | BIT(SUPORT_CAMERA) | \
                                             BIT(SUPORT_TIMEFORMAT) | BIT(SUPORT_DISTANCE_UNIT) | FUNC_ONCE_CONNECT)
//支持的检测功能
#define     DETECTION_FUNC                  (BIT(SUPORT_HEART_RATE) | BIT(SUPORT_BLOOD_OXYGEN) | BIT(SUPORT_BLOOD_PRESSURE) | \
                                             BIT(SUPORT_BLOOD_GLUCOSE) | BIT(SUPORT_SLEEP) | BIT(SUPORT_STEP_CALORIE_DISTANCE))
//支持的消息推送
#define     DEVICE_MSG                      (0x1fffff)

//支持的运动类型
#define     DEVICE_SPORT                    (0xfff)

//支持的开关
#define     DEVICE_SWI                      (BIT(SWI_WRITST) | BIT(SWI_SLEEP_MONITOR) | BIT(SWI_NOTIFICATION) | BIT(SWI_NOTIFY_SCREEN_LIGHT) | \
                                             BIT(SWI_SHAKE) | BIT(SWI_MESSAGE_VIBRATION))

//app data define
#define     RECENT_APP_NUM_MAX              4           //最近任务最大个数
#define     ALARM_NUM_MAX                   3           //闹钟最大个数
#define     REMIND_NUM_MAX                  10          //提醒最大个数
#define     INTERVAL_REMIND_NUM_MAX         2           //间隔提醒最大个数
#define     CONTACTS_MAX_NUM                8           //联系人最大个数
#define     CONTACTS_NAME_LEN               200         //联系人人名buf缓冲长度
#define     SLEEP_RECORD_MAX                20          //睡眠变化最大记录条数

#define     CALL_RECORD_LEN                 4           //通话记录最大个数
#define     MAX_MSG_NUM                     4           //消息最大条数
#define     MSG_SIZE                        256         //消息内容最大长度

#define     CALL_NUM_LEN                    16          //电话号码长度
#define     CONTACTS_NAME_SAVE_MAX          30          //联系人人名最大长度

#define     DATE_NUM                        7           //天数

#define     MEASURE_UPLOAD_T                10          //测量数据自动上传间隔(分钟)
#define     SPORT_RECORD_NUM_MAX            10          //运动最大记录条数
#define     SPORT_RECORD_VALID_TIME         60          //运动判断记录有效时间(s)
#define     HR_RECORD_NUM_MAX               10          //心率间隔检测最大记录条数
#define     HR_INTERVAL_MEASURE_TIMEOUT     30          //心率间隔检测30秒超时

//platform manager
#define     PM_WRITE_DELAY                  5           //延时时间(s), 延时同步sector数据, 0不打开;
#define     FLASH_SECTOR_SIZE               (4*1024)    //flash最小擦除单位: 扇区(4K)
#define     FLASH_AB_PARAM_ADDR             (FLASH_SIZE - FLASH_CM_SIZE - FLASH_SECTOR_SIZE*2)  //放末尾的FLASH_CM_SIZE前面
#define     FLASH_SECTOR0_ADDR              (FLASH_AB_PARAM_ADDR )                              //sector0, 4K
#define     FLASH_SECTOR1_ADDR              (FLASH_SECTOR0_ADDR + FLASH_SECTOR_SIZE)            //sector1, 4K


void sector0_var_init(void);                            //sector0数据初始化
void sector1_var_init(void);                            //sector1数据初始化

#endif  //_PLATFROM_CONFIG_H_
