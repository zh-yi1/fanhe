#ifndef _BSP_APP_H
#define _BSP_APP_H

#include "app_platform.h"
#include "app_ancs.h"
#include "ab_command/ab_common.h"

#define little_endian_read_16(buf, ofs)        *(uint16_t *)((uint8_t *)buf + (ofs))
#define little_endian_read_32(buf, ofs)        *(uint32_t *)((uint8_t *)buf + (ofs))
uint32_t big_endian_read_16(const uint8_t *buffer, int pos);
uint32_t big_endian_read_32(const uint8_t *buffer, int pos);

void little_endian_store_16(uint8_t *buffer, uint16_t pos, uint16_t value);
void little_endian_store_32(uint8_t *buffer, uint16_t pos, uint32_t value);
void big_endian_store_16(uint8_t *buffer, uint16_t pos, uint16_t value);
void big_endian_store_32(uint8_t *buffer, uint16_t pos, uint32_t value);

//0x59:查找手机
void ab_app_search_phone(bool en);

//0x81:设备绑定
void bind_sta_set(u8 sta);
bool ab_app_is_connect(void);					//设备是否已与app连接
void app_phone_type_set(u8 type);
u8 app_phone_type_get(void);

uint8_t app_distance_unit_get(void);


#endif
