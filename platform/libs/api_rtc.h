#ifndef _API_RTC_H
#define _API_RTC_H

//RTC tm time互转。有效年份：2020~2120
typedef struct tm_t_ {
    u16 year;
    u8 mon;
    u8 day;
    u8 weekday;
    u8 hour;
    u8 min;
    u8 sec;
} tm_t;

tm_t time_to_tm(u32 time);
u32 tm_to_time(tm_t tm);
u8 get_month_days(u16 year, u8 mon);
u8 get_weekday(u16 year, u8 mon, u8 day);

//RTC 寄存器相关
void irtc_sfr_write(u32 cmd, u8 dat);
u8 irtc_sfr_read(u32 cmd);

u32 irtc_time_read(u32 cmd);
void irtc_time_write(u32 cmd, u32 dat);

void rtcram_write(u8 *buf, u8 addr, u8 cnt);
void rtcram_read(u8 *buf, u8 addr, u8 cnt);

void rtc_sleep_process(void);
void rtc_sleep_enter(void);
void rtc_rc_sleep_enter(void);
void rtc_sleep_exit(void);
u32 rtc_get_freq(void);

#if !LP_XOSC_CLOCK_EN
void rtc_calibration_write(u8 addr);
void rtc_calibration_read(u8 addr);
bool rtc_clock_calc_by_rc(u32 exit_flag);
void sniff_rc_init(void);
#endif


#endif // _API_RTC_H
