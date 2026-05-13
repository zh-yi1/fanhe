#include "alipay_net_kal.h"
#include "sfr.h"
#include "iotsec.h"
//#include <stdlib.h>    // for malloc(), free()
//#include <string.h>    // for memset()
//#include <unistd.h>    // for usleep()

#if 1//SECURITY_PAY_EN

#define printf(...)                     a_printf(__VA_ARGS__)

//Memory
/*
 * @brief: 申请一块连续的指定大小的内存块
 * @param: size 内存块的大小，单位字节
 * @return: 非NULL 被分配内存的指针(此存储区中的初始值不确定)
 *            NULL 分配失败
 */
void *alipay_iot_ble_malloc(size_t size)
{
    void* buffer = csi_malloc(size);
    return buffer;
}


void *alipay_iot_ble_realloc(void *ptr, size_t new_size)
{

    void* buffer;
    if (ptr == NULL) {
        buffer = csi_malloc(new_size);
    } else {
        buffer = csi_realloc(ptr, new_size);
    }
//    printf("### %s, ptr:%x, new_size:%d, new_ptr:%x\n", __func__, ptr, new_size, buffer);
    return buffer;
}


/*
 * @brief: 释放动态分配的内存
 * @param: s 内存块的首地址
 */
void alipay_iot_std_free(void *s)
{
    csi_free(s);
}



/*
 * @brief: 将s所指向的某一块内存中的后size个字节的内容全部设置为ch
 * @param: s 内存块的首地址
 * @param: ch 要设置的值
 * @param: size 要设置的字节个数
 * @return: 内存块的首地址
 */
void *alipay_iot_memset(void *s, int ch, size_t size)
{
    return memset(s, ch, size);
}

/*
 * @brief: 从源src所指的内存地址的起始位置开始拷贝size个字节到目标dest所指的内存地址的起始位置中
 * @param: dest 目标内存块的首地址
 * @param: src 源内存块的首地址
 * @param: size 要拷贝的字节个数
 * @return: 指向dest的指针
 */
void* alipay_iot_memcpy(void *dest, const void *src, size_t size)
{
//    return kal_mem_cpy(dest, src, size);
    return memcpy(dest, src, size);
}

/*
* @brief: 使任务休眠milliseconds毫秒
* @input: milliseconds 休眠的时间，单位毫秒
*/
void alipay_iot_sleep(unsigned long milliseconds) {
    //os_sleep(milliseconds);
}

void alipay_task_sleep(uint32_t milliseconds) {
    //os_sleep(milliseconds);
}

/*
* @brief: 获取UTC时间
* note:  1.精确级别,微妙级别
*        2.受系统时间修改影响
*        3.返回的秒数是从1970年1月1日0时0分0秒开始
*/
void alipay_iot_gettimeofday(alipay_iot_timeval *now){
//    #warning alipay_iot_gettimeofday demo
    uint32_t day_offset = (2020 - 1970)*365 + (2020 - 1970)/4;       //RTCCNT时间从2020年开始, 标准的是从1970年开始算,需计算偏移
    uint32_t offset = day_offset*86400;

    now->tv_sec = RTCCNT + offset - 8 * 3600;
    now->tv_usec = 0;
//    printf("%s:%d\n", __func__, now->tv_sec );
}
#endif
