#ifndef _APP_AB_DATA_H_
#define _APP_AB_DATA_H_

#include "app_data_config.h"
#include "app_data.h"

enum{
    SQUARE_WATCH,                   //方屏
    CIRCULAR_WATCH,                 //圆屏
    BRACELET,                       //手环
};

enum DEVICE_LANGUAGE{
    ENGLISH_EN,                     //英文(英国)
    CHINESE_ZH_CH,                  //中文(简体)
    ITALIAN_IT_IT,                  //意大利语(意大利)
    SPANISH_ES_ES,                  //西班牙语(西班牙)
    PORTUGUESE_PT_PT,               //葡萄牙语(葡萄牙)
    RUSSIAN_RU_RU,                  //俄语(俄罗斯)
    JAPANESE_JA_JA,                 //日语(日本)
    CHINESE_ZH_TW,                  //中文(繁体)
    GERMAN_DE_DE,                   //德语(德国)
    KOREAN_KO,                      //韩语
    THAI_TH_TH,                     //泰语(泰国)
    ARABIC_AR_AE,                   //阿拉伯语(阿拉伯联合酋长国)
    TURKISH_TR_TR,                  //土耳其语(土耳其)
    FRENCH_FR_FR,                   //法语(法国)
    VIETNAMESE_VI,                  //越南语
    POLISH_PL_PL,                   //波兰语(波兰)
    DUTCH_NL_NL,                    //荷兰语(荷兰)
    HEBREW_IW,                      //希伯来语
    PERSIAN,                        //波斯语
    GREEK_EL_GR,                    //希腊语(希腊)
    MALAYSIA_MS,                    //马来西亚语
    BURMESE,                        //缅甸语
    DANISH_DA_DK,                   //丹麦语(丹麦)
    UKRAINIAN_UK_UA,                //乌克兰语(乌克兰)
    INDONESIAN_ID,                  //印度尼西亚语
    CZECH_CS_CZ,                    //捷克语(捷克共和国)
    HINDI,                          //印地语
    DV_LANG_MAX,
};

//手表支持的功能
enum DEVICE_FUNCTION{
    SUPORT_WATCH_MARKET,            //表盘市场
    SUPORT_MESSAGE,                 //消息提醒
    SUPORT_WEATHER,                 //天气功能
    SUPORT_NFC,                     //NFC
    SUPORT_CONTACTS,                //通讯录
    SUPORT_WALLET,                  //钱包
    SUPORT_ECARD,                   //名片
    SUPORT_UPDATE,                  //固件升级
    SUPORT_NODISTURB,               //勿扰模式
    SUPORT_BRIGHTEN,                //亮屏设置
    SUPORT_CAMERA,                  //拍一拍
    SUPORT_TIMEFORMAT,              //12/24小时制切换
    SUPORT_DISTANCE_UNIT,           //公制/英制切换
    SUPORT_STATICCALORIE,           //静态卡路里
    SUPORT_ONCE_CONNECT,            //一键双连
};

//手表支持的检测功能
enum DETECTION_FUNCTION{
    SUPORT_HEART_RATE,              //心率
    SUPORT_BLOOD_OXYGEN,            //血氧
    SUPORT_BLOOD_PRESSURE,          //血压
    SUPORT_BLOOD_GLUCOSE,           //血糖
    SUPORT_SLEEP,                   //睡眠
    SUPORT_STEP_CALORIE_DISTANCE,   //步数/卡路里/距离
};

//手环支持的开关
enum SUPORT_SWI{
    SWI_ANTILOST,                   //防丢开关
    SWI_WRITST,                     //抬腕亮屏
    SWI_AUTO_SYNC,                  //自动同步
    SWI_SLEEP,                      //睡眠
    SWI_SLEEP_MONITOR,              //睡眠监测
    SWI_NOTIFICATION,               //消息提醒总开关
    SWI_HOUR_SPORT_UPLOAD,          //整点上传运动数据
    SWI_GOAL_ACHEVE,                //目标达成开关
    SWI_RESERVE,                    //预留
    SWI_NOTIFY_SCREEN_LIGHT,        //消息通知亮屏开关
    SWI_SOUND,                      //声音开关
    SWI_SHAKE,                      //振动总开关
    SWI_HOUR_SAMPLE_UPLOAD,         //整点采集健康数据开关
    SWI_MESSAGE_VIBRATION,          //消息通知振动开关
};

enum sport_type{
    RUNNING,
    CYCLING,
    WALK1,
    CLIMBING,
    SITUP,
    JUMPROPE,
    SWIMMING,
    BADMINTON,
    PINGPONG,
    BASKETBALL,
    GOLF,
    TENNIS,
    VOLLEYBALL,
    SPORT_NULL = 0xff,
};
typedef enum sport_type sport_type_t;

enum BIND_STA{
    BIND_NULL = 0,              //未绑定
    BIND_DONE,                  //绑定完成

    BIND_ING,                   //绑定中
    BIND_ING_DEV_DONE,          //绑定中(设备已完成)
    BIND_ING_APP_DONE,          //绑定中(app已完成)
};

enum{
    CMD_TYPE_REQUEST = 0x01,
    CMD_TYPE_RESPONSE,
    CMD_TYPE_NOTIFY,
};

enum CMD_RERULT{
    CMD_RESULT_OK = 0,           /* Cmd result is no error */
    CMD_RESULT_FAILE = 1,        /* Cmd result have a generic error */
    CMD_RESULT_LEN_INVALID = 2,  /* Cmd package length is invalid */
    CMD_RESULT_CMD_INVALID = 3,  /* Cmd is inexistence */
    CMD_RESULT_IDX_INVALID = 4,  /* Cmd index is invalid */
    CMD_RESULT_NOT_SUPPORT = 5,  /* Cmd not support */
    CMD_RESULT_SWI_ERROR = 6,    /* Cmd switch is error */
    CMD_RESULT_CHECK_ERROR = 7,  /* Data check error*/
    CMD_RESULT_PACKET_LOSS = 8,  /* the last packet lost*/
};


enum {
    GMUSIC_BLE_STATE_STOP = 0x00,
    GMUSIC_BLE_STATE_PLAY,
    GMUSIC_BLE_STATE_PREV,
    GMUSIC_BLE_STATE_NEXT,
//    GMUSIC_BLE_VOLUME_DOWN,
//    GMUSIC_BLE_VOLUME_UP,
    GMUSIC_BLE_VOLUME_SETTING,
    GMUSIC_BLE_STATE_DISCONNECT,
    GMUSIC_BLE_STATE_CONNECTING,
    GMUSIC_BLE_STATE_CONNECT_FAIL,

    GMUSIC_BLE_STATE_NULL = 0xff,
};

enum music_query{
    MUSIC_QUERY_START = 0,
    MUSIC_QUERY_STOP,
};

enum music_data{
    MUSIC_DATA_TITLE = 0,
    MUSIC_DATA_ARTIST,
    MUSIC_DATA_DURATION,
    MUSIC_DATA_CUR_TIME,
};

enum weather_type{
    WEATHER_TYPE_CLOUDY,
    WEATHER_TYPE_SUNSHINE,
    WEATHER_TYPE_SNOW,
    WEATHER_TYPE_RAIN,
    WEATHER_TYPE_OVERCAST,
    WEATHER_TYPE_SAND_AND_DUST,
    WEATHER_TYPE_WINDY,
    WEATHER_TYPE_HAZE,
};
typedef enum weather_type weather_type_t;

enum sys_time_format{
    SYS_TIME_FORMAT_AM         = 0x00,
    SYS_TIME_FORMAT_PM         = 0x01,

    SYS_TIME_FORMAT_NULL       = 0xff,
};
typedef enum sys_time_format sys_time_format_t;

enum health_remind_type{
    HEALTH_TYPE_SPORT       = 0x00,
    HEALTH_TYPE_MEDICINE,
    HEALTH_TYPE_BOOK,
    HEALTH_TYPE_TRIP,

    HEALTH_TYPE_NULL        = 0xff,
};
typedef enum health_remind_type health_remind_type_t;

struct user_info_t{
    uint8_t           user_sex;                                         //协议：性别：0x00：男 0x01：女
    uint8_t           user_age;                                         //协议：年龄（单位：岁）
    uint8_t           user_height;                                      //协议：身高（单位：厘米）
    uint8_t           user_weight;                                      //协议：体重（单位：kg）
};
typedef struct user_info_t user_info;

struct period{
    uint8_t           start_hour;                                       //开始时间：小时
    uint8_t           start_minute;                                     //开始时间：分钟
    uint8_t           end_hour;                                         //结束时间：小时
    uint8_t           end_minute;                                       //结束时间：分钟
};
typedef struct period period_t;

struct not_disturb_t{
        uint8_t           sw;                                               //定时勿扰开关
        period_t          period;                                           //时间段
        uint8_t           all_day;                                          //全天勿扰
};
typedef struct not_disturb_t not_disturb;

struct weather_oneday_info{
    //uint8_t         weather_unit;                                       //协议：摄氏度 华氏度
    bool             temp_limit;                                         //温度限制：0:0度以上  1:0度以下
    int8_t           current_temp;                                       //当前温度
    int8_t           min_temp;                                           //最小温度
    int8_t           max_temp;                                           //最大温度

    weather_type_t   oneday_weather_type;                                //天气类型

};
typedef struct weather_oneday_info weather_oneday_info_t;

#define WEATHER_LOCATION_MAX    108
struct weather_condition{
    weather_oneday_info_t           first_day;                           //第一天天气数据
    weather_oneday_info_t           second_day;                          //第二天天气数据
    weather_oneday_info_t           third_day;                           //第三天天气数据
    uint8_t                         location_len;                        //天气地点长度
    uint8_t                         location[WEATHER_LOCATION_MAX];      //天气地点
};
typedef struct weather_condition weather_condition_t;

struct drink_info_t{
    uint8_t                                 dr_switch;                                                  //闹钟开关
    uint8_t                                 dr_start_hour;                                              //时
    uint8_t                                 dr_start_minute;                                            //分
    uint8_t                                 dr_end_hour;                                                //时
    uint8_t                                 dr_end_minute;                                              //分
    uint8_t                                 dr_cycle;                                                   //周期
};
typedef struct drink_info_t drink_info;

struct sedentary_info_t{
    uint8_t                                 sit_switch;                                                  //闹钟开关
    uint8_t                                 sit_start_hour;                                              //时
    uint8_t                                 sit_start_minute;                                            //分
    uint8_t                                 sit_end_hour;                                                //时
    uint8_t                                 sit_end_minute;                                              //分
    uint8_t                                 sit_cycle;                                                   //周期
};
typedef struct sedentary_info_t sedentary_info;

struct health_info_t{
    uint8_t                                 hea_type;                                                    //健康类型：0：运动 1：吃药 2：看书 3：出行
    uint8_t                                 hea_switch;                                                  //闹钟开关
    uint8_t                                 hea_hour;                                                    //时
    uint8_t                                 hea_minute;                                                  //分
    uint8_t                                 hea_cycle;                                                   //周期
    uint8_t                                 hea_repeat;                                                  //0:重复提醒 1:只提醒一次，当天有效
};
typedef struct health_info_t health_info;

struct health2_info_t{
    uint8_t                                 hea2_type;                                                    //第二类健康类型：
    uint8_t                                 hea2_num;                                                     //参数设置

    //uint8_t                                 hea2_hr_type;                                                 //
    //uint8_t                                 hea2_hr_max;                                                  //
    //uint8_t                                 hea2_hr_min;                                                  //周期
};
typedef struct health2_info_t health2_info;

union sys_resource_switch{
    uint16_t u16_sw;
    struct {
        uint16_t sw_sedentary                :1;                                                         //久坐
        uint16_t sw_drink                    :1;                                                         //喝水
        uint16_t sw_hr                       :1;                                                         //心率
        uint16_t sw_look                     :1;                                                         //远眺
        uint16_t sw_sport                    :1;                                                         //运动
        uint16_t sw_medinice                 :1;                                                         //吃药
        uint16_t sw_book                     :1;                                                         //看书
        uint16_t sw_trip                     :1;                                                         //出行

        uint16_t sw_activity_day             :1;                                                         //每日活动提醒
        uint16_t sw_step_complete            :1;                                                         //步数完成提醒
        uint16_t sw_workout_time_complete    :1;                                                         //锻炼时长完成提醒
        uint16_t sw_activity_num             :1;                                                         //活动次数完成提醒
        uint16_t sw_motion_recognition       :1;                                                         //运动识别提醒
        uint16_t sw_ring                     :1;                                                         //来电开关
        uint16_t sw_wrist                    :1;                                                         //抬腕亮屏
        uint16_t sw_chargesave               :1;                                                         //省电模式
        uint16_t sw_antilost                 :1;                                                         //防丢开关
        uint16_t sw_top_hour_sample          :1;                                                         //整点采集健康数据开关
    }sys_sw;
};
typedef union sys_resource_switch sys_resource_switch_t;

struct reminder_info_t{
//    uint8_t                                 alarm_num;                                                  //闹钟数量
//    alarm_info                              alarm[8];                                                   //闹钟到钟提醒
    drink_info                              drink;                                                      //喝水提醒
    sedentary_info                          sedentary;                                                  //久坐提醒
    health_info                             health[4];                                                  //健康提醒(运动，吃药，出行，看书)
    health2_info                            health2[2];                                                 //第二类健康提醒
    sys_resource_switch_t                   health_switch;                                              //健康提醒开关设置
};
typedef struct reminder_info_t reminder_info;


union sys_switch{
    uint32_t u32_sw;
    struct {
        uint32_t antilost                    :1;                                                         //防丢开关
        uint32_t wrist                       :1;                                                         //抬腕亮屏
        uint32_t auto_syns                   :1;                                                         //自动同步
        uint32_t reserve3                    :1;                                                         //预留
        uint32_t sleep                       :1;                                                         //睡眠监测
        uint32_t notification                :1;                                                         //消息提醒总开关
        uint32_t hour_upload_sport           :1;                                                         //整点上传运动数据
        uint32_t goal_achievement            :1;                                                         //目标达成开关
        uint32_t reserve8                    :1;                                                         //预留
//        uint32_t real_hr_upload              :1;                                                         //实时心率上传开关
        uint32_t notify_screen_light         :1;                                                         //消息通知亮屏开关
        uint32_t sound                       :1;                                                         //声音开关
        uint32_t shake                       :1;                                                         //振动总开关
        uint32_t top_hour_sample             :1;                                                         //整点采集健康数据开关
        uint32_t message_vibration           :1;                                                         //消息通知振动开关

        uint32_t reserve14                   :1;                                                         //保留
        uint32_t reserve15                   :1;                                                         //保留
        uint32_t reserve16                   :1;                                                         //保留
        uint32_t reserve17                   :1;                                                         //保留
        uint32_t reserve18                   :1;                                                         //保留
        uint32_t reserve19                   :1;                                                         //保留
        uint32_t reserve20                   :1;                                                         //保留
        uint32_t reserve21                   :1;                                                         //保留
        uint32_t reserve22                   :1;                                                         //保留
        uint32_t reserve23                   :1;                                                         //保留
        uint32_t reserve24                   :1;                                                         //保留
        uint32_t reserve25                   :1;                                                         //保留
        uint32_t reserve26                   :1;                                                         //保留
        uint32_t reserve27                   :1;                                                         //保留
        uint32_t reserve28                   :1;                                                         //保留
        uint32_t reserve29                   :1;                                                         //保留
        uint32_t reserve30                   :1;                                                         //保留
        uint32_t reserve31                   :1;                                                         //保留
    }sw;
};
typedef union sys_switch sys_switch_t;

union com_switch{
    uint32_t u32_sw;
    struct {
        uint32_t ble_bpm_upload              :1;                                                         //上传心率数据开关
        uint32_t ble_spo2_upload             :1;														 //上传血氧数据开关
        uint32_t ble_bp_upload               :1;														 //上传血压数据开关
    }sw;
};
typedef union com_switch com_switch_t;

union social_switch_list{
    uint32_t u32_sw;
    struct {
        uint32_t ring                        :1;                                                         //来电
        uint32_t missed_call                 :1;                                                         //未接来电
        uint32_t message                     :1;                                                         //短信
        uint32_t social_news                 :1;                                                         //社交消息
        uint32_t email                       :1;                                                         //邮件
        uint32_t schedule                    :1;                                                         //日程
        uint32_t QQ                          :1;                                                         //QQ
        uint32_t wechat                      :1;                                                         //微信
        uint32_t whatsapp                    :1;                                                         //
        uint32_t skype                       :1;                                                         //

        uint32_t twitter                     :1;                                                         //推特
        uint32_t facebook                    :1;                                                         //
        uint32_t instagram                   :1;                                                         //
        uint32_t weibo                       :1;                                                         //微博
        uint32_t viber                       :1;                                                         //
        uint32_t dingtalk                    :1;                                                         //钉钉
        uint32_t alipay                      :1;                                                         //支付宝
        uint32_t tiktok                      :1;                                                         //抖音
        uint32_t linkedln                    :1;                                                         //领英
        uint32_t line                        :1;                                                         //
        uint32_t snapchat                    :1;                                                         //
        uint32_t reserve21                   :1;                                                         //保留
        uint32_t reserve22                   :1;                                                         //保留
        uint32_t reserve23                   :1;                                                         //保留
        uint32_t reserve24                   :1;                                                         //保留
        uint32_t reserve25                   :1;                                                         //保留
        uint32_t reserve26                   :1;                                                         //保留
        uint32_t reserve27                   :1;                                                         //保留
        uint32_t reserve28                   :1;                                                         //保留
        uint32_t reserve29                   :1;                                                         //保留
        uint32_t reserve30                   :1;                                                         //保留
        uint32_t reserve31                   :1;                                                         //保留
    }sw;
};
typedef union social_switch_list social_switch_list_t;


struct daily_goals{
    uint16_t                    swi;                                    //开关
    uint8_t                     step;                                   //步数目标
    uint8_t                     distance;                               //距离目标
    uint16_t                    kcal;                                   //卡路里目标(千卡)
    uint16_t                    sleep;                                  //睡眠目标
    uint16_t                    duration;                               //运动时长目标
    uint16_t                    resv;                                   //保留
};
typedef struct daily_goals daily_goals_t;

//运动记录
struct sport_recored{
    uint8_t                                  sport_type;                                                //运动类型
    uint32_t                                 timestamp;                                                 //开始时间戳：s
    uint32_t                                 step;                                                      //步数(步)
    uint32_t                                 kcal;                                                      //卡路里(0.1大卡)
    uint32_t                                 distance;                                                  //距离(米)
    uint32_t                                 duration;                                                  //运动时间(s)
    uint8_t                                  avg_hr;                                                    //平均心率：未开启自动检测时为0
    uint32_t                                 base_cal;                                                  //基础卡路里：卡
}PACKED;
typedef struct sport_recored sport_record_t;

//睡眠监测
struct sleep_check{
    uint8_t           start_hour;                                       //开始时间：小时
    uint8_t           start_minute;                                     //开始时间：分钟
    uint8_t           end_hour;                                         //结束时间：小时
    uint8_t           end_minute;                                       //结束时间：分钟
    uint8_t           all_day;                                          //是否全天监测
};
typedef struct sleep_check sleep_check_t;

struct hr_alarm_range{
    uint8_t           sw;                                               //协议：开关
    uint8_t           hr_upper;                                         //协议：上限值
    uint8_t           hr_lower;                                         //协议：下限值
};
typedef struct hr_alarm_range hr_alarm_range_t;

struct sedentary_remind_setting{
    uint8_t           sit_sw;                                           //静坐使能开关
    uint8_t           cycle;                                            //协议：周期(周一~周日)
    uint8_t           interval;                                         //协议：检测频率(分钟)
    uint8_t           start_hour;                                       //协议：开始时间：小时
    uint8_t           start_minute;                                     //协议：开始时间：分钟
    uint8_t           end_hour;                                         //协议：结束时间：小时
    uint8_t           end_minute;                                       //协议：结束时间：分钟
    uint8_t           sedentary_wait_time;
    // uint8_t           sedentary_flag ;
};
typedef struct sedentary_remind_setting sedentary_remind_setting_t;

struct ble_protocol_upload
{
    uint8_t    remind_count;
    uint8_t    alarm_count;
    uint8_t    step_count;
    uint8_t    sleep_count;
    uint8_t    hr_count;
    uint8_t    hrv_count;
    uint8_t    workout_count;
};
typedef struct ble_protocol_upload ble_protocol_upload_t;

struct alarm_clock{
    uint8_t                                 swi;                                                     //闹钟开关
    uint8_t                                 cycle;                                                   //周期：0bit~6bit表示周一到周日,bit7:0:重复提醒 1:只提醒一次，当天有效
    uint8_t                                 hour;                                                    //时
    uint8_t                                 minute;                                                  //分
    uint8_t                                 moto_mode;                                               //震动模式
    uint8_t                                 remind_later;                                            //稍后提醒（分钟）
};
typedef struct alarm_clock alarm_clock_t;

struct normal_remind{
    uint8_t                                 re_id;                                                      //提醒ID
    uint8_t                                 re_switch;                                                  //提醒开关
    uint8_t                                 re_cycle;                                                   //周期：0bit~6bit表示周一到周日,bit7:0:重复提醒 1:只提醒一次，当天有效
    uint8_t                                 re_hour;                                                    //时
    uint8_t                                 re_minute;                                                  //分
    uint8_t                                 re_mototype;                                                //震动类型
    uint8_t                                 re_remind_later;                                            //稍后提醒（分钟）
};
typedef struct normal_remind normal_remind_t;

//保存提醒事件
struct interval_remind{
    uint8_t                                 id;                                                         //提醒ID
    uint8_t                                 cycle;                                                      //周期：0bit~6bit表示周一到周日,bit7: 提醒总开关
    uint8_t                                 start_hour;                                                 //时
    uint8_t                                 start_minute;                                               //分
    uint8_t                                 end_hour;                                                   //时
    uint8_t                                 end_minute;                                                 //分
    uint8_t                                 interval;                                                   //间隔：分钟
    uint32_t                                cnt_cycle;                                                  //计数判断是否到达时间间隔
}PACKED;
typedef struct interval_remind interval_remind_t;

//电话簿
typedef struct{
    u8 idx;                                     //联系人编号
    u8 name[CONTACTS_NAME_SAVE_MAX];
    u8 num_len;                                 //buf中有效号码长度
    u8 num_buf[CALL_NUM_LEN/2 + 1];                 //2个号码占一个字节, 低位在前, 高位在后, '+'用A代替
    //u8 numchar_buf[CALL_NUM_LEN + 1];               //显示字符串号码
}contact_info_t;

typedef struct {
    contact_info_t                          info;                       //联系人信息
    bool                                    contact_type;               //通话类型: 0来电, 1去电
    bool                                    answer_hang;                //接听/挂断:0挂断, 1接听
    u8                                      month;                      //月
    u8                                      day;                        //日
    u8                                      hour;                       //时
    u8                                      min;                        //分
}call_records_info_t;

//时间戳数据
struct timestamp_data{

    uint32_t                                 timestamp;                                                //UTC时间戳：s
    uint8_t                                  value;
};
typedef struct timestamp_data timestamp_data_t;

//睡眠
struct sleep_record{
    uint16_t                                 awake_min;     //清醒时长
    uint16_t                                 light_min;     //浅睡时长
    uint16_t                                 deep_min;      //深睡时长
    uint8_t                                  wkup_time;     //醒来次数

    uint8_t                                  index;
    timestamp_data_t                         change[SLEEP_RECORD_MAX];
    bool                                     remind_flag;                           //今日是否已提醒
};
typedef struct sleep_record sleep_record_t;

///心率
struct heartrate{
    uint8_t                                 bpm_last;                               //心率最后一次测量值
};
typedef struct heartrate heartrate_t;

//血氧
struct blood_oxygen{
    uint8_t                                 bo_last;                                //血氧最后一次测量值
};
typedef struct blood_oxygen blood_oxygen_t;

//血压
struct blood_pressure{
    uint8_t                                 sbp_last;                               //收缩压最后一次测量值
    uint8_t                                 dbp_last;                               //舒张压最后一次测量值
};
typedef struct blood_pressure blood_pressure_t;

typedef struct flash_sector0{
    u8                                      bind_flag:1;                                                //是否已被绑定标志
    user_info                               userinfo;                                                   //用户信息
    uint8_t                                 alarm_num;                                                  //闹钟数量
    alarm_clock_t                           alarm[ALARM_NUM_MAX];                                       //闹钟信息
	reminder_info                           reminder_all;
    normal_remind_t                         remind[REMIND_NUM_MAX];                                     //单次提醒信息
    interval_remind_t                       remind_group[REMIND_NUM_MAX];                               //健康提醒
    weather_condition_t                     weather;                                                    //天气
    sys_switch_t                            sys_sw;                                                     //系统开关(做存储)
    call_records_info_t                     call_records[CALL_RECORD_LEN];                              //通话记录
    sport_record_t                          sport_record[SPORT_RECORD_NUM_MAX];                         //运动数据
    uint8_t                                 sport_record_idx;                                           //运动数据当前索引
}flash_sector0_t;

typedef struct flash_sector1{
    uint8_t                                 time_format;                                                //时间格式(12h/24h) 0->24; 1->12;
    uint8_t                                 lang;                                                       //设备语言
    u32 									sleep_time;													//熄屏时间
    u16 									vbat;													    //电压

    contact_info_t                          contact_info[CONTACTS_MAX_NUM];                             //联系人
    uint                                    step;                                                       //步数
    uint16_t                                cal;                                                        //卡路里(0.1大卡)
    uint                                    distance;                                                   //距离(米)
    heartrate_t                             hr;                                                         //心率
    blood_oxygen_t                          bo;                                                         //血氧
    blood_pressure_t                        bp;                                                         //血压
    u8                                      yest_day;                                                   //昨天
    bool                                    weather_unit;                                               //天气单位,false:摄氏度,true:华氏度
    bool                                    factory_mack;                                               //恢复出厂设置标志
    reminder_info                           reminder_all;                                               //所有提醒类的
    u16                                     temp_remind_all[19];                                        //省电模式开时保存的提醒状态
    uint16_t                                password;                                                   //锁屏密码

    social_switch_list_t                    social_sw;                                                  //社交开关扩展
    not_disturb                             not_disturb_info;                                           //勿扰模式
    sleep_record_t                          sleep;                                                      //睡眠数据
    sleep_check_t                           sleep_check;                                                //自动睡眠检测设置
}flash_sector1_t;

/// Platform Manager Status
enum {
    PM_NULL,
    PM_CHANGE,
};

enum {
    PM_SYNC_SECTOR0 = 0,
    PM_SYNC_SECTOR1,
    PM_SYNC_MAX,
};

union pm_switch{
    uint32_t u32_sync;
    struct {
        uint32_t sector0                      :1;
        uint32_t sector1                      :1;
        uint32_t sector2                      :1;
    }sync;
};
typedef union pm_switch pm_switch_t;

typedef struct {
    uint8_t                                 ble_app_ios     :1;                                         //APP区别安卓苹果链接, 0:Android 1:IOS

    bool                                    init_mark;                                                  //绑定完成标志
    u8                                      bind_sta;                                                   //绑定状态

    bool                                    dayclr_ban;                                                 //app时间更新禁止清空数据
    uint8_t                                 brightness_level;                                           //亮度等级
//    uint16_t                                shake_period;                                               //震动周期(ms)
//    uint8_t                                 shake_level;                                                //振动等级
//    uint8_t                                 shake_time;                                                 //震动次数
//
//    uint8_t                                 fix_hour;                                                   //时
//    uint8_t                                 fix_minute;                                                 //分
//    uint8_t                                 alarm_ring : 4;                                             //当前响起的闹钟
//
//    uint8_t                                 auto_hr_interval;                                           //自动心率监测间隔
//	uint8_t                                 base_cal_sw;                                                //静态卡路里开关
//    password_type                           gpwdselect_sta;                                             //密码选择状态
//    uint8_t                                 recent_task[RECENT_APP_NUM_MAX];                            //最近任务
//    bool                                    exist_recent_task;                                          //存在最近任务标志,开机默认不存在
//
//    //音乐
//    uint8_t                                 music_state;                                                //音乐状态
//    uint8_t                                 music_title[128];                                           //音乐名称
//    uint8_t                                 music_vol;                                                  //音量大小
//
//    //心率
//    uint8_t                                 bpm;                                                        //心率实时值
//    uint8_t                                 bpm_avg;                                                    //心率平均值
//    uint8_t                                 spo2;                                                       //血氧实时值
//    uint8_t                                 sbp;                                                        //收缩压实时值
//    uint8_t                                 dbp;                                                        //舒张压实时值
//    uint8_t                                 hr_sampling;                                                //心率传感器自动测量检测信号量
//
//
//    //睡眠
//    uint8_t                                 sleep_status;                                               //睡眠状态
//    uint8_t                                 sleep_start_hour;                                           //睡眠开始小时
//    uint8_t                                 sleep_start_minute;                                         //睡眠开始分钟
//    uint8_t                                 sleep_end_hour;                                             //睡眠结束小时
//    uint8_t                                 sleep_end_minute;                                           //睡眠结束分钟
//    uint16_t                                deep_sleep_time;                                            //深入睡眠时间

    uint8_t                                 time_zone;                                                  //当前时区
    uint8_t                                 distance_unit;                                              //单位格式（距离）

    alarm_clock_t                           alarm_later;                                                //贪睡闹钟
    hr_alarm_range_t                        hr_alarm_threshold;                                         //心率报警范围
    daily_goals_t                           daily_goal;                                                 //目标

    com_switch_t                            com_sw;                                                     //通用开关(不做存储)
    flash_sector0_t                         sector0;                                                    //参数记忆区0
    flash_sector1_t                         sector1;                                                    //参数记忆区1

    //platform manager
    uint32_t                                sec_cnt;                                                    //平台秒计时器
    uint8_t                                 pm_times;                                                   //平台数据存储延时计数
    uint8_t                                 status;                                                     //数据状态: 0: NO CHANGE: 1:DIRTY;
    pm_switch_t                             pm_swi;                                                     //存储开关
} app_data_t;
extern app_data_t app_data;
#endif  //_APP_AB_DATA_H_
