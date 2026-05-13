#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define CTP_READ_ADDR            0xe0               //P1_XH, P1_XL, P1_YH, P1_YL
#define CTP_READ_SIZE            3

static u8 ctp_chsc6x_buf[CTP_READ_SIZE];

bool ctp_chsc6x_init(void)
{
    return true;
}

AT(.com_text.ctp)
void ctp_chsc6x_readkick(void)
{
    ctp_iic_readkick(ctp_chsc6x_buf, CTP_READ_ADDR, CTP_READ_SIZE);
}

AT(.com_text.ctp)
bool ctp_chsc6x_get_point(s32 *x, s32 *y)
{
    *x = ((ctp_chsc6x_buf[0] & 0x40) << 2) + ctp_chsc6x_buf[1];
    *y = ((ctp_chsc6x_buf[0] & 0x80) << 1) + ctp_chsc6x_buf[2];
    return ((ctp_chsc6x_buf[0] & 0x10) == 0);
}

