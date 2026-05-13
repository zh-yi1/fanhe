
#ifndef _PORT_HED_H_
#define _PORT_HED_H_
#include "include.h"
#include "hed_private.h"

u8 i2c_write(unsigned char *pData, unsigned short wLen);
u8 i2c_read(unsigned char *pData, unsigned short wLen);
se_error_t port_i2c_periph_transmit(uint8_t *inbuf, uint32_t  inbuf_len);
se_error_t port_i2c_periph_receive(uint8_t *outbuf, uint16_t outbuf_len);

#endif









