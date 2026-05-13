#include "include.h"
#include "ab_common.h"

#if (USE_APP_TYPE == APP_BLUE_FIT)

u32 timestamp_get(void)
{
    u32 timestamp = RTCCNT - (app_data.time_zone - 12)*3600;
    return timestamp;
}

void timestamp_data_save(timestamp_data_t *buf, u16 value)
{
    u32 rtccnt = timestamp_get();

    buf->timestamp = rtccnt;
    buf->value = value;
}

bool clock_in_time(tm_t *nowtime, uint8_t begin_hour, uint8_t begin_minute, uint8_t end_hour, uint8_t end_minute)
{
    bool res = false;
    uint16_t now = 0, begin = 0, end = 0;
    now   = (nowtime->hour) * 60 + nowtime->min;
    begin = begin_hour * 60 + begin_minute;
    end   = end_hour *60 + end_minute;

    if (begin != end) {
        if (begin > end) {
            res = (now >= begin && now < 1440) || (now < end);
        } else {
            res = (now >= begin && now < end);
        }
    } else if (begin == 0 && end == 0) {	//00:00~00:00
        res = true;
    } else {
        res = false;
    }
    return res;
}

bool clock_in_time_range(period_t *period)
{
//    printf("clock check: %d:%d - %d:%d\n", period->start_hour, period->start_minute, period->end_hour, period->end_minute);
    return clock_in_time(&compo_cb.tm, period->start_hour, period->start_minute, period->end_hour, period->end_minute);
}

void period_set(period_t *period, uint8_t begin_hour, uint8_t begin_minute, uint8_t end_hour, uint8_t end_minute)
{
    period->start_hour = begin_hour;
    period->start_minute = begin_minute;
    period->end_hour = end_hour;
    period->end_minute = end_minute;
}


u16 check_sum_get(u8 *buf, u16 len)
{
    u16 data_sum = 0;
    for (u16 i = 0; i < len; i++) {
        data_sum += buf[i];
    }
//    printf("cal data_sum[%x] len[%d]\n", data_sum, len);
    return data_sum;
}
#endif
