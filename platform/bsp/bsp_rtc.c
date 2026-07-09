#include "include.h"

uint calc_crc(void *buf, uint len, uint seed);

typedef struct {
    uint crc;
    uint32_t rtccnt;
}rtc_recode_t;

static rtc_recode_t s_rtc_recode AT(.rtc_cache);

void bsp_rtc_recode_set(bool clear)
{
    if (clear) {
        memset(&s_rtc_recode, 0, sizeof(s_rtc_recode));
        s_rtc_recode.crc = 0xffff;
    } else {
        s_rtc_recode.rtccnt = RTCCNT;
        s_rtc_recode.crc = calc_crc(&s_rtc_recode.rtccnt, sizeof(s_rtc_recode.rtccnt), 0xffff);
    }
}

uint32_t bsp_rtc_recode_get(void)
{
    uint crc = calc_crc(&s_rtc_recode.rtccnt, sizeof(s_rtc_recode.rtccnt), 0xffff);
    if (crc == s_rtc_recode.crc && s_rtc_recode.crc != 0xffff) {
        return s_rtc_recode.rtccnt;
    }

    return 0;
}


void rtc_clock_init(void)
{
    tm_t tm;
    tm.year = 2026;
    tm.mon  = 7;
    tm.day = 9;
    tm.hour = 12;
    tm.min  = 0;
    tm.sec  = 0;
    //tm.weekday = get_weekday(tm.year, tm.mon, tm.day);
    RTCCNT = tm_to_time(tm);
}

//多少秒后闹钟响
AT(.text.rtc)
void rtc_set_alarm_relative_time(u32 nsec)
{
    tm_t rtc_tm;
    rtc_tm = time_to_tm(RTCCNT);                //更新时间结构体
    RTCALM = tm_to_time(rtc_tm) + nsec;         //设置闹钟相对于当前时间n秒后
}

//设置多少秒后闹钟唤醒
AT(.text.rtc)
void rtc_set_alarm_wakeup(u32 nsec)
{
    uint rtccon3 = RTCCON3;

    RTCCPND = BIT(17) | BIT(18);                //clear RTC alarm pending
    RTCCON9 = BIT(0);                           //clear alarm pending
    rtc_set_alarm_relative_time(nsec);

    rtccon3 |= BIT(8);                          //RTC alarm wakeup enable
    RTCCON3 = rtccon3;
}

//关闭闹钟
AT(.text.rtc)
void rtc_alarm_disable(void)
{
//    printf("%s\n", __func__);
    RTCCPND = BIT(17) | BIT(18);                //clear RTC alarm pending
    RTCCON9 = BIT(0);                           //clear alarm pending
	RTCCON &= ~BIT(8);                          //clear sniff wakeup
    RTCCON3 &= ~BIT(8);                         //clear power down wakeup
}

/*
 * 设置RTC时间
 * 输入：tm结构体指针
*/
void rtc_clock_set(tm_t rtc_tm)
{
    if (sleep_cb.sys_is_sleep) {
        rtc_sleep_exit();
    }
    RTCCNT = tm_to_time(rtc_tm);
    if (sleep_cb.sys_is_sleep) {
        rtc_sleep_enter();
    }
}

/*
 * 设置RTC时间
 * 输入：timestamp格林威治时间(UTC/GMT)
 *       time_zone时区(范围0~24, 表示西十二区(UTC_W12) ~ 东十二区(UTC_E12))
*/
void rtc_clock_timestamp_set(u32 timestamp, u8 time_zone)
{
    int zone_offset = (time_zone - UTC_0);
    u32 day_offset = (2020 - 1970)*365 + (2020 - 1970)/4;       //RTCCNT时间从2020年开始, 标准的是从1970年开始算,需计算偏移
    u32 offset = day_offset*86400;

    RTCCNT = timestamp + zone_offset*3600 - offset;

}

/*
 * 获取RTC时间
 * 输入: 空
   输出: rtc_tm结构体
*/
tm_t rtc_clock_get(void)
{
    return time_to_tm(RTCCNT);
}

//休眠后后RTC校准时回调函数
AT(.com_text.rtc.notify)
void rtc_clock_calc_notify(void)
{
}
