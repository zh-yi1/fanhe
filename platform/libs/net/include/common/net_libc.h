#ifndef NET_LIBC_H__
#define NET_LIBC_H__

int net_vsnprintf(char * buffer, size_t count, const char * format, va_list va);

uint16_t get_be16(void *ptr);
uint32_t get_be32(void *ptr);
void put_be16(void *ptr, uint16_t val);
void put_be32(void *ptr, uint32_t val);

#endif
