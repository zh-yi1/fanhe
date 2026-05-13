#ifndef _AB_COMMON_H_
#define _AB_COMMON_H_

#include "app_data.h"

//大小端转换
#define SWAP_UINT16(x) ((x >> 8) | (x << 8))
#define SWAP_UINT32(x) ((x >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | (x << 24))

u16 check_sum_get(u8 *buf, u16 len);
/**
 * \brief               保存数据时间戳
 * \param buf           存储时间戳结构体
 * \param value         存储的数据
 */
void timestamp_data_save(timestamp_data_t *buf, u16 value);

/**
 * \brief               判断当前时间是否在设定时间段内
 * \param nowtime       当前时间
 * \param begin_hour    起始时间(小时)
 * \param begin_minute  起始时间(分钟)
 * \param end_hour      结束时间(小时)
 * \param end_minute    结束时间(分钟)
 * \return              判断结果
 */
bool clock_in_time(tm_t *nowtime, uint8_t begin_hour, uint8_t begin_minute, uint8_t end_hour, uint8_t end_minute);

/**
 * \brief               时间段结构体赋值
 * \param period        时间段结构体
 * \return              判断结果
 */
void period_set(period_t *period, uint8_t begin_hour, uint8_t begin_minute, uint8_t end_hour, uint8_t end_minute);

/**
 * \brief               判断当前时间是否在设定时间段内(使用系统时间)
 * \param period        时间段结构体
 * \return              判断结果
 */
bool clock_in_time_range(period_t *period);

/**
 * \brief               生成累加校验和
 * \param buf           缓冲区地址
 * \param period        缓冲区长度
 * \return              累加校验和(高位舍去)
 */
u16 check_sum_get(u8 *buf, u16 len);

//platform manager
void param_pm_write(u8 sync_swi);                                     //sector同步flash中(延时)
void param_pm_write_real(u32 sync_swi);                               //sector同步flash中(立即)
#endif  //_AB_COMMON_H_
