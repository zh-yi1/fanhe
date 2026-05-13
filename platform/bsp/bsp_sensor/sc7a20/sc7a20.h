#ifndef _SC7A20_H
#define _SC7A20_H

#include "sl_watch_application.h"

#define SC7A20_SLEEP_CHECK_EN           1                               //是否开启睡眠功能
#define SLEEP_WEAR_CHECK_CNT            120                             //gsensor两小时检测是否佩戴
#define SLEEP_RECORD_MAX                20                              //睡眠变化最大记录条数
#define SLEEP_LAST_MAX                  (SLEEP_WEAR_CHECK_CNT + 10)     //记录最近睡眠状态的最大条数
#define SCHANGE_LAST_MAX                15                              //记录最近睡眠变化的最大条数

enum{
    SLEEP_CHECK_NULL,                   //睡眠监测未开始
    SLEEP_CHECK_ING,                    //睡眠监测中
};

enum{
    SLEEP_NULL,                         //无睡眠
    SLEEP_AWAKE,                        //清醒
    SLEEP_LIGHT,                        //浅睡
    SLEEP_DEEP,                         //深睡
    SLEEP_CHANGE_MAX,

    SLEEP_WKUP,                         //醒来
};

typedef struct _user_t{
    bool                                    custom;                                     //是否自定义
    uint8_t                                 sex;                                        //性别：0x00：男 0x01：女
    uint8_t                                 age;                                        //年龄（单位：岁）
    uint8_t                                 height;                                     //身高（单位：厘米）
    uint8_t                                 weight;                                     //体重（单位：kg）
}user_t;

typedef struct _gsensor_data_t{
    uint                                    step;                                       //步数
    uint16_t                                cal;                                        //卡路里(0.1大卡)
    uint                                    distance;                                   //距离(米)
    u8                                      yest_day;                                   //昨天
}gsensor_data_t;

//时间戳数据
typedef struct timestamp_data_t_{

    uint32_t                                 timestamp;                                 //UTC时间戳：s
    uint8_t                                  value;
}b_timestamp_data_t;

//睡眠监测
typedef struct sleep_check_t_{
    uint8_t                                 start_hour;                                 //开始时间：小时
    uint8_t                                 start_minute;                               //开始时间：分钟
    uint8_t                                 end_hour;                                   //结束时间：小时
    uint8_t                                 end_minute;                                 //结束时间：分钟
    uint8_t                                 all_day;                                    //是否全天监测
}b_sleep_check_t;

//睡眠
typedef struct sleep_record_t_{
    uint16_t                                 awake_min;                                 //清醒时长
    uint16_t                                 light_min;                                 //浅睡时长
    uint16_t                                 deep_min;                                  //深睡时长
    uint8_t                                  wkup_time;                                 //醒来次数

    uint8_t                                  index;
    b_timestamp_data_t                       change[SLEEP_RECORD_MAX];
    bool                                     remind_flag;                               //今日是否已提醒
}b_sleep_record_t;

typedef struct b_sleep_sta_t_{
    u8 sleep_status_last;                                                               //实时
    u8 sleep_last_w;                                                                    //睡眠状态写指针
    u8 sleep_last_sta[SLEEP_LAST_MAX];                                                  //记录最近的睡眠状态缓存, [7:4]为时间戳变化索引; [4:0]睡眠状态缓存
//    uint8_t change_last_mark;                                                         //睡眠变化记录空闲标志位
    b_timestamp_data_t change_last[SCHANGE_LAST_MAX + 10];                              //记录最近的[睡眠变化+醒来次数]缓存
    bool data_valid;
}b_sleep_sta_t;

typedef struct _gsensor_axis_data_t{
    short                                    x;                                          //x
    short                                    y;                                          //y
    short                                    z;                                          //z
}gsensor_axis_data_t;

typedef struct {
#if SC7A20_SLEEP_CHECK_EN
    b_sleep_check_t                    	    sleep_check;
    b_sleep_record_t                        sleep;
    b_sleep_sta_t                           sleep_sta;
    uint8_t                                 sleep_status;                                //睡眠状态
#endif
    gsensor_data_t                          gsensor_data;                                //gsensor
    user_t                                  user;                                        //user
    gsensor_axis_data_t                     gsensor_axis;                                //gsensor3轴坐标
}sc7a20_info_t;

extern sc7a20_info_t sc7a20_info;

unsigned char SL_SC7A20_I2c_Spi_Write(bool sl_spi_iic,unsigned char reg, unsigned char dat);
unsigned char SL_SC7A20_I2c_Spi_Read(bool sl_spi_iic,unsigned char reg, unsigned char len, unsigned char *buf);
void sc7a20_int_enable(void);
void sc7a20_int_disable(void);
void sc7a20_isr(void);
void sc7a20_isr_process(void);
void sc7a20_init(void);

void sc7a20_sleep_check(tm_t *now_time);
void sc7a20_lowpwr_process(void);

#endif
