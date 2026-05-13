#include "include.h"
#include "lwip/api.h"
#include "nopoll/nopoll.h"
#include "webclient/webclient.h"

void sntp_set_system_time(unsigned long s, unsigned long f)
{
    //printf("%s %u %u\n", __func__, s, f);

    // 70year
    u32 ts = s - 2208988800UL;
    rtc_clock_timestamp_set(ts, UTC_E8);

    tm_t tm = rtc_clock_get();
    printf("-- %d %d %d %d %d %d %d\n", tm.year, tm.mon, tm.day, tm.weekday, tm.hour, tm.min, tm.sec);
    //printf("RTCCNT: %u %u\n", rtc_clock_get_utime(), RTCCNT);
}

const char *sys_arch_format_time(uint32_t sec)
{
    return "no impl\n";
}

void lwip_wdt_rst(void)
{
    WDT_RST();
}

void *lwip_mem_malloc(size_t size)
{
    void *ptr;
    ptr = ab_malloc(size);
    // printf("lwip_malloc=%x 1 %d\n", ptr, size);
    return ptr;
}

void lwip_mem_free(void *ptr)
{
    ab_free(ptr);
    // printf("lwip_free=%x\n", ptr);
}

void *lwip_mem_calloc(size_t nitems, size_t size)
{
    void *ptr = ab_calloc(nitems, size);
    // printf("lwip_malloc=%x 2 %d\n", ptr, size);
    return ptr;
}
