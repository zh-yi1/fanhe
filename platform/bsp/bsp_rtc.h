#ifndef _BSP_RTC_H
#define _BSP_RTC_H

enum{
	UTC_W12 = 0,    //西十二区
	UTC_W11,
	UTC_W10,
	UTC_W9,
	UTC_W8,
	UTC_W7,
	UTC_W6,
	UTC_W5,
	UTC_W4,
	UTC_W3,
	UTC_W2,
	UTC_W1,
	UTC_0,          //零时区
    UTC_E1,
	UTC_E2,
	UTC_E3,
	UTC_E4,
	UTC_E5,
	UTC_E6,
	UTC_E7,
	UTC_E8,
	UTC_E9,
	UTC_E10,
	UTC_E11,
	UTC_E12,        //东十二区
	UTC_NULL,
};

void rtc_clock_init(void);
void rtc_set_alarm_relative_time(u32 nsec);
void rtc_set_alarm_wakeup(u32 nsec);

/*
 * 设置RTC时间
 * 输入：tm结构体指针
*/
void rtc_clock_set(tm_t rtc_tm);

/*
 * 设置RTC时间
 * 输入：timestamp格林威治时间(UTC/GMT)
 *       time_zone时区(范围0~24, 表示西十二区(UTC_W12) ~ 东十二区(UTC_E12))
*/
void rtc_clock_timestamp_set(u32 timestamp, u8 time_zone);

/*
 * 获取RTC时间
 * 输入: 空
   输出: rtc_tm结构体
*/
tm_t rtc_clock_get(void);

// 休眠后后RTC校准时回调函数
void rtc_clock_calc_notify(void);

/*
 * 更新rtc记录（该记录异常复位不丢失，关机或掉电丢失）
 * 用户使用时软件定时调用该接口更新rtccnt记录
 * clear: 1:清除记录 0：更新记录
*/
void bsp_rtc_recode_set(bool clear);

/*
 * 获取rtc记录值，为0时无效
*/
uint32_t bsp_rtc_recode_get(void);

#endif // _FUNC_CLOCK_H
