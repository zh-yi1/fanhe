#include "include.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "hal_os.h"
#include "csi_os.h"
#include "alipay_common.h"
#include "vendor_os.h"
#if SECURITY_PAY_EN

uint32_t alipay_get_timestamp(void) __attribute__ ((weak));
void alipay_log_ext(const char *format, ...) __attribute__ ((weak));
uint32_t alipay_get_compile_timestamp() __attribute__ ((weak));
void alipay_free(void* pt) __attribute__ ((weak));
void* alipay_malloc(uint32_t size) __attribute__ ((weak));
void* alipay_realloc(void* pt, uint32_t size) __attribute__ ((weak));
void* alipay_calloc(uint32_t nblock,uint32_t size) __attribute__ ((weak));

/*
* 获取系统当前时间
* month 为1表示1月，day 为1表示1号，year为2018表示2018年。
* hour 取值0-23.  min取值0-59. second取值0-59.
*/
void alipay_iot_get_local_time(alipay_iot_local_time *time)
{
//    printf("### %s\n", __func__);
    tm_t tm = time_to_tm(RTCCNT);
    time->day = tm.day;
    time->hour = tm.hour;/* + 8*/;
//    if (time->hour > 23) {
//        time->hour -= 23;
//    }
    time->minute = tm.min;
    time->month = tm.mon;
    time->second = tm.sec;
    time->wday = tm.weekday;
    time->year = tm.year;
    time->isdst = 0;
//    printf("### %d-%d-%d %d:%d:%d\n", time->year, time->month, time->day, time->hour, time->minute, time->second);
}

/**
* 获取一个非0的随机数
*/
int alipay_rand(void)
{
    int res = get_random(1000);
    return res;
}

//int32_t get_Rng_number(uint8_t *data, unsigned size) {
//    hal_rng_get_data(data, size);
//    alipay_log_ext("random bytes[%d]: data[0]:0x%02x, data[1]:0x%02x - data[n-2]:0x%02x, data[n-1]0x%02x", size, data[0], data[1], data[size-2], data[size-1]);
//    return 1;
//}

uint32_t alipay_get_timestamp(void) {
    uint32_t ts_now = 0;
    hal_get_timestamp(&ts_now);
//    printf("### %s:%d\n", __func__, ts_now);
    return ts_now;
}

time_t my_time_(time_t *val)
{
//    printf("@@@### %s\n", __func__);
    // alipay_iot_timeval now;
    uint32_t now = alipay_get_timestamp();
    if(val) {
        *val = (time_t)now;
    }

    return((time_t)now);
}
/*int32_t get_Rng_number(uint8_t *data, unsigned size) {
    hal_rng_get_data(data, size);
    alipay_log_ext("random bytes[%d]: data[0]:0x%02x, data[1]:0x%02x - data[n-2]:0x%02x, data[n-1]0x%02x", size, data[0], data[1], data[size-2], data[size-1]);
    return 1;
}*/


uint32_t alipay_get_compile_timestamp() {
    uint32_t build_timestamp = 0;
    hal_get_compile_timestamp(&build_timestamp);
    alipay_log_ext("build_timestamp: %d", build_timestamp);
    return build_timestamp;
}

void* alipay_malloc(uint32_t size) {
    void *ptr = hal_malloc(size);
    if (!ptr) {
        alipay_log_ext("malloc failed");
    }
    memset(ptr, 0, size);
    return ptr;
}

void* alipay_calloc(uint32_t nblock,uint32_t size) {
    void *ptr = hal_calloc(nblock, size);
    if (!ptr) {
        alipay_log_ext("calloc failed");
    }
    return ptr;
}

void alipay_free(void* pt) {
    if (pt) hal_free(pt);
}

void* alipay_realloc(void* ptr, uint32_t size) {
    return hal_realloc(ptr, size);
}


void alipay_log_ext(const char *format, ...) {
    char *alipay_log_buf = (char *)ab_malloc(512);
    memset(alipay_log_buf, 0, sizeof(512));
    va_list arg;
    va_start(arg, format);
    vsnprintf(alipay_log_buf, 512-1, format, arg);
    hal_log_ext("%s\n", alipay_log_buf);
    va_end(arg);
    ab_free(alipay_log_buf);
}

/*-------------------------------    HAL     --------------------------------------*/


hal_error_t hal_get_timestamp(uint32_t *tm)
{
    csi_error_t ret;

    if(!tm) {
        return HAL_INVALID_PARAM;
    }

    ret = csi_get_timestamp(tm);
    if(ret) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

hal_error_t hal_get_compile_timestamp(uint32_t *timestamp)
{
    csi_error_t ret;

    if(timestamp == NULL) {
        return HAL_INVALID_PARAM;
    }

    ret = csi_get_compile_timestamp(timestamp);
    if(ret) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

void *hal_malloc(uint32_t size)
{
    return csi_malloc(size);
}

void *hal_calloc(uint32_t nblock, uint32_t size)
{
    return csi_calloc(nblock, size);
}

void hal_free(void *pt)
{
    csi_free(pt);
}

void *hal_realloc(void *pt, uint32_t size)
{
    return csi_realloc(pt, size);
}

void hal_log_ext(const char *format, ...)
{
    va_list valist;
    va_start(valist,format);
    csi_log_ext(format,&valist);
    va_end(valist);
}

hal_error_t hal_rng_get_data(void *random, uint32_t size)
{
    csi_error_t ret;

    if(!random || !size) {
        return HAL_INVALID_PARAM;
    }

    ret = csi_rng_get_data(random, size);
    if(ret) {
        return HAL_ERROR;
    }

    return HAL_OK;
}
#endif
