#include "include.h"
#include "list.h"
#if SECURITY_PAY_EN

#if SECURITY_PAY_EN

AT(.text.bsp.i2c)
void bsp_alipay_i2c_init(void)
{
    bsp_i2c_init(SECURITY_PAY_IICX, SECURITY_PAY_SCL, SECURITY_PAY_SDA, SECURITY_PAY_MAP_GX);
}


AT(.text.bsp.i2c)
void bsp_alipay_i2c_tx_buf(u8 i2cx_idx, u16 dev_addr, u8 *buf, u16 len)
{
    bsp_hw_i2c_dma_write_buf(i2cx_idx, dev_addr, buf, len, true);

//    u32 ticks;
//    HS_IIC->sfr->IICxCMDA =  (buf[0]<<8) | dev_addr;
//
//    HS_IIC->sfr->IICxCON1 = BIT(12) | BIT(11)  | BIT(10) | BIT(5) | BIT(4) | BIT(3);
//    HS_IIC->sfr->IICxDMAADR = DMA_ADR(&buf[1]);
//    HS_IIC->sfr->IICxDMACNT = ((len - 2) << 16) | BIT(1) |BIT(0);
//    HS_IIC->sfr->IICxCON0 |= BIT(28);                //KICK
////    printf("IICxCMDA: 0x%x, IICxDMAADR: 0x%x, IICxDMACNT: 0x%x, IICxCON0: 0x%x, IICxCON1: 0x%x\n",
////           HS_IIC->sfr->IICxCMDA, HS_IIC->sfr->IICxDMAADR, HS_IIC->sfr->IICxDMACNT, HS_IIC->sfr->IICxCON0, HS_IIC->sfr->IICxCON1);
//    ticks = tick_get();
//    while (!(HS_IIC->sfr->IICxCON0 & BIT(31))) {
//        if (tick_check_expire(ticks, 1000)) {
//            printf("i2c_write time out ERROR\n");
//            return;
//        }
//    }
//    HS_IIC->sfr->IICxCON0 |= BIT(29);                //Clear Pending
}

AT(.text.bsp.i2c)
void bsp_alipay_i2c_rx_buf(u8 i2cx_idx, u16 dev_addr, u8 *buf, u16 len)
{

    bsp_hw_i2c_dma_read_buf(i2cx_idx, dev_addr, NULL, 0, buf, len);

//    u32 ticks;
//    HS_IIC->sfr->IICxCMDA =  dev_addr<<24;
//    HS_IIC->sfr->IICxCON1 = BIT(12) | BIT(11) | BIT(9) | BIT(8) | BIT(7);
//    HS_IIC->sfr->IICxDMAADR = DMA_ADR(buf);
//    HS_IIC->sfr->IICxDMACNT = ((len - 1) << 16) | BIT(0);
//    HS_IIC->sfr->IICxCON0 |= BIT(28);                //KICK
//
//    ticks = tick_get();
//    while (!(HS_IIC->sfr->IICxCON0 & BIT(31))) {
//        if (tick_check_expire(ticks, 2000)) {
//            printf("alipay_i2c_read time out ERROR\n");
//            return;
//        }
//    }
//    HS_IIC->sfr->IICxCON0 |= BIT(29);                //Clear Pending
}

//AT(.text.bsp.i2c)
//void bsp_hs_i2c_rx_byte(u8 i2cx_idx, u16 dev_addr, u8 *buf)
//{
//    u32 ticks;
//    HS_IIC->sfr->IICxCMDA =  dev_addr<<24;
//    HS_IIC->sfr->IICxCON1 = BIT(12) | BIT(11) | BIT(9) | BIT(8) | BIT(7);
//    HS_IIC->sfr->IICxDMAADR = DMA_ADR(buf);
//    HS_IIC->sfr->IICxDMACNT = BIT(0);
//    HS_IIC->sfr->IICxCON0 |= BIT(28);                //KICK
//
//    ticks = tick_get();
//    while (!(HS_IIC->sfr->IICxCON0 & BIT(31))) {
//        if (tick_check_expire(ticks, 50)) {
//            printf("alipay_i2c_read time out ERROR\n");
//            return;
//        }
//    }
//    HS_IIC->sfr->IICxCON0 |= BIT(29);                //Clear Pending
//}
#endif // SECURITY_PAY_EN



struct Node* csi_head = NULL;

uint32_t csi_get_size(void *ptr)
{
    uint32_t size = 0;
    memcpy(&size, ptr - 4, 4);
    return size;
}


void *csi_malloc(uint32_t size)
{

    //void *pt = func_zalloc(size);
    //void *pt = ab_malloc(size);
    void *pt = SECURITY_MALLOC(size);
    if (pt == NULL) {
        printf("pt == NULL\n");
        return NULL;
    }
//    printf("%s, pt: 0x%x, size: 0x%x, %x\n", __func__, pt, size, __builtin_return_address(0));
    insertNode(&csi_head, pt);
    return pt;
}

void csi_free(void *pt)
{
//    printf("%s, pt: 0x%x, %x\n", __func__, pt, __builtin_return_address(0));
    deleteNode(&csi_head, pt);
    //func_free(pt);
}

void *ab_realloc(void * ptr, size_t size);
void *ab_calloc(size_t nitems, size_t size);

void *csi_calloc(uint32_t nblock, uint32_t size)
{
    void *pt = ab_calloc(nblock, size);
    insertNode(&csi_head, pt);
    return pt;
}

void *csi_realloc(void *pt, uint32_t size)
{

    void *ptr_new = ab_realloc(pt, size);
//    printf("%s, pt: 0x%x, size: 0x%x, %x\n", __func__, ptr_new, size,  __builtin_return_address(0));
    return ptr_new;
}

#if 0

#define	_U	01
#define	_L	02
#define	_N	04
#define	_S	010
#define _P	020
#define _C	040
#define _X	0100
#define	_B	0200

#define _CTYPE_DATA_0_127 \
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C, \
	_C,	_C|_S, _C|_S, _C|_S,	_C|_S,	_C|_S,	_C,	_C, \
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C, \
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C, \
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_N,	_N,	_N,	_N,	_N,	_N,	_N,	_N, \
	_N,	_N,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_P,	_P,	_P,	_P,	_P, \
	_P,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_C

#define _CTYPE_DATA_128_255 \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0

const char __ctype__[1 + 256] = { 0, _CTYPE_DATA_0_127, _CTYPE_DATA_128_255 };
#define _CONST const

#ifndef _MB_CAPABLE
_CONST
#endif
__IMPORT char *__ctype_ptr__ = __ctype__;
#endif

int rand(void)
{
    // printf("%s\n", __func__);
    return get_random(0x7fff);
}

void srand(unsigned int seed)
{
    printf("%s:%x\n", __func__, seed);
}

//int sscanf(char *s, const char *format, ...)
//{
//    va_list param;
//    va_start(param, format);
//    printf("s:%s, param:%s, format:%s\n", s, param, format);
//    if ((format[0] == '%') && (format[1] == '[') && (format[2] == '^') &&
//        (format[3] == '#') && (format[4] == ']')) {
//
//
//        u16   n      = 0;
//        char *ptr    = s;
//        char  buf[2] = { 0 };
//        memset(buf, 0, 2);
//        uint32_t *des;
//
//
//        print_r(param, 10);
//
//        u32 **param1 = param;
//
//        while (*ptr != '#') {
//            n++;
//            ptr++;
//        }
//
//        ptr -= n;
//        des = *param1;
//        //    printf("1---> n = %d, ptr:%x\n", n, ptr);
//        memcpy(des, ptr, n);
//        ptr += n;
//        n = 0;
//        //    printf("1---->buf1:%s\n", des);
//
//        while (*ptr == '#') {
//            ptr++;
//        }
//
//        va_arg(param, u32);
//        u32 **param2 = param;
//        while (*ptr != '#') {
//            n++;
//            ptr++;
//        }
//
//        ptr -= n;
//        //    printf("2---> n = %d, ptr:%x\n", n, ptr);
//        des = *param2;
//        memcpy(des, ptr, n);
//        ptr += n;
//        n = 0;
//
//        while (*ptr == '#') {
//            ptr++;
//        }
//        //
//        va_arg(param, u32);
//        u32 **param3 = param;
//        while (*ptr != '#') {
//            n++;
//            ptr++;
//        }
//        memcpy(buf, ptr - n, n);
//        des  = *param3;
//        *des = atoi(buf);
//        n    = 0;
//        va_end(param);
//        return 3;
//    } else {
//        printf("sscanf format not correct:%s\n", format);
//        va_arg(param, u32);
//        int res =  my_sscanf(s, format, param);
//        return res;
//    }
//}

void write32(uint8_t* buffer, uint32_t value)
{
    buffer[0] = value & 0xff;
    buffer[1] = (value>>8) & 0xff;
    buffer[2] = (value>>16) & 0xff;
    buffer[3] = (value>>24) & 0xff;
}

uint32_t read32(const uint8_t* buffer)
{
    uint32_t result = 0;
    result |= buffer[0];       // 将第一个字节放在最低位
    result |= buffer[1] << 8;  // 将第二个字节放在次低位
    result |= buffer[2] << 16;  // 将第三个字节放在次高位
    result |= buffer[3] << 24;  // 将第四个字节放在最高位
    return result;
}


// sscanf implementation

static int
is_space(char c)
{
    return (c == ' ' || c == '\t' || c == '\v' ||
            c == '\f' || c == '\r' || c == '\n');
}

static char*
skip_spaces(const char *str)
{
    while (is_space(*str)) {
        ++str;
    }
    return (char*)str;
}

/* Returns a pointer after the last read char, or 'str' on error. */
static char*
dec_to_signed(const char *str, long *out)
{
    const char * cur = skip_spaces(str);
    long value = 0;
    int isneg = 0, isempty = 1;

    if (cur[0] == '+') {
        cur += 1;
    } else if(cur[0] == '-') {
        cur += 1;
        isneg = 1;
    }

    while (*cur != '\0' && *cur >= '0' && *cur <= '9') {
        value = (value * 10) + (*cur - '0');
        isempty = 0;
        ++cur;
    }

    if (isempty) {
        return (char*)str;
    }
    if (isneg) {
        *out = -value;
    } else {
        *out = value;
    }
    return (char*)cur;
}

/* Returns a pointer after the last read char, or 'str' on error. */
static char*
dec_to_unsigned(const char *str, unsigned long *out)
{
    const char * cur = skip_spaces(str);
    unsigned long value = 0;
    int isempty = 1;

    while (*cur != '\0' && *cur >= '0' && *cur <= '9') {
        value = (value * 10) + (*cur - '0');
        isempty = 0;
        ++cur;
    }

    if (isempty) {
        return (char*)str;
    }
    *out = value;
    return (char*)cur;
}

/* Returns a pointer after the last read char, or 'str' on error. */
//static char*
//hex_to_signed(const char *str, long *out)
//{
//    const char * cur = skip_spaces(str);
//    long value = 0;
//    int isneg = 0, isempty = 1;
//
//    if (cur[0] == '+') {
//        cur += 1;
//    } else if(cur[0] == '-') {
//        cur += 1;
//        isneg = 1;
//    }
//    if (cur[0] == '0' && cur[1] == 'x') {
//        cur += 2;
//    }
//
//    while (*cur != '\0') {
//        if(*cur >= '0' && *cur <= '9') {
//            value = (value * 16) + (*cur - '0');
//        } else if (*cur >= 'a' && *cur <= 'f') {
//            value = (value * 16) + 10 + (*cur - 'a');
//        } else if (*cur >= 'A' && *cur <= 'F') {
//            value = (value * 16) + 10 + (*cur - 'A');
//        } else {
//            break;
//        }
//        isempty = 0;
//        ++cur;
//    }
//
//    if (isempty) {
//        return (char*)str;
//    }
//    if (isneg) {
//        *out = -value;
//    } else {
//        *out = value;
//    }
//    return (char*)cur;
//}

/* Returns a pointer after the last read char, or 'str' on error. */
static char*
hex_to_unsigned(const char *str, unsigned long *out)
{
    const char * cur = skip_spaces(str);
    unsigned long value = 0;
    int isempty = 1;

    if (cur[0] == '0' && cur[1] == 'x') {
        cur += 2;
    }

    while (*cur != '\0') {
        if(*cur >= '0' && *cur <= '9') {
            value = (value * 16) + (*cur - '0');
        } else if (*cur >= 'a' && *cur <= 'f') {
            value = (value * 16) + 10 + (*cur - 'a');
        } else if (*cur >= 'A' && *cur <= 'F') {
            value = (value * 16) + 10 + (*cur - 'A');
        } else {
            break;
        }
        isempty = 0;
        ++cur;
    }

    if (isempty) {
        return (char*)str;
    }
    *out = value;
    return (char*)cur;
}

#define MFMT_DEC_TO_SIGNED(TYPE, NAME)                          \
static char*                                                    \
dec_to_##NAME(const char *str, TYPE *out)                       \
{                                                               \
        long v;                                                 \
        char *cur = dec_to_signed(str, &v);                     \
        if (cur != str){                                        \
                *out = (TYPE)v;                                 \
        }                                                       \
        return cur;                                             \
}

#define MFMT_DEC_TO_UNSIGNED(TYPE, NAME)                        \
static char*                                                    \
dec_to_##NAME(const char *str, TYPE *out)                       \
{                                                               \
        unsigned long v;                                        \
        char *cur = dec_to_unsigned(str, &v);                   \
        if (cur != str){                                        \
                *out = (TYPE)v;                                 \
        }                                                       \
        return cur;                                             \
}

#define MFMT_HEX_TO_SIGNED(TYPE, NAME)                                  \
static char*                                                            \
hex_to_##NAME(const char *str, TYPE *out)                               \
{                                                                       \
        long v;                                                         \
        char *cur = hex_to_signed(str, &v);                             \
        if (cur != str){                                                \
                *out = (TYPE)v;                                         \
        }                                                               \
        return cur;                                                     \
}

#define MFMT_HEX_TO_UNSIGNED(TYPE, NAME)                        \
static char*                                                    \
hex_to_##NAME(const char *str, TYPE *out)                       \
{                                                               \
        unsigned long v;                                        \
        char *cur = hex_to_unsigned(str, &v);                   \
        if (cur != str){                                        \
                *out = (TYPE)v;                                 \
        }                                                       \
        return cur;                                             \
}

/* Returns a pointer after the last written char, or 'str' on error. */
#define MFMT_SIGNED_TO_HEX(TYPE, NAME)                                  \
static char*                                                            \
NAME##_to_hex(TYPE val, int uppercase, char padchar, size_t padlen,     \
              size_t len, char *str)                                    \
{                                                                       \
        char buf[24];                                                   \
        size_t isneg = 0, cnt = 0;                                      \
        if (uppercase){                                                 \
                uppercase = 'A';                                        \
        }else{                                                          \
                uppercase = 'a';                                        \
        }                                                               \
        if (val < 0){                                                   \
                isneg = 1;                                              \
                val = -val;                                             \
        }                                                               \
        do{                                                             \
                buf[cnt++] = val % 16;                                  \
                val = val / 16;                                         \
        }while (val != 0);                                              \
        if (padlen > isneg + cnt){                                      \
                padlen -= isneg + cnt;                                  \
                padlen = (padlen < len ? padlen : len);                 \
                memset(str, padchar, padlen);                           \
                str += padlen;                                          \
                len -= padlen;                                          \
        }                                                               \
        if (isneg && len > 0){                                          \
                str[0] = '-';                                           \
                str += 1;                                               \
                len -= 1;                                               \
        }                                                               \
        while (cnt-- > 0 && len-- > 0){                                 \
                if (buf[cnt] < 10){                                     \
                        *str = buf[cnt] + '0';                          \
                }else{                                                  \
                        *str = (buf[cnt] - 10) + uppercase;             \
                }                                                       \
                ++str;                                                  \
        }                                                               \
        return str;                                                     \
}

/* Returns a pointer after the last written char, or 'str' on error. */
#define MFMT_SIGNED_TO_DEC(TYPE, NAME)                                  \
static char*                                                            \
NAME##_to_dec(TYPE val, char padchar, size_t padlen,                    \
              size_t len, char *str)                                    \
{                                                                       \
        char buf[24];                                                   \
        size_t isneg = 0, cnt = 0;                                      \
        if (val < 0){                                                   \
                isneg = 1;                                              \
                val = -val;                                             \
        }                                                               \
        do{                                                             \
                buf[cnt++] = val % 10;                                  \
                val = val / 10;                                         \
        }while (val != 0);                                              \
        if (padlen > isneg + cnt){                                      \
                padlen -= isneg + cnt;                                  \
                padlen = (padlen < len ? padlen : len);                 \
                memset(str, padchar, padlen);                           \
                str += padlen;                                          \
                len -= padlen;                                          \
        }                                                               \
        if (isneg && len > 0){                                          \
                str[0] = '-';                                           \
                str += 1;                                               \
                len -= 1;                                               \
        }                                                               \
        while (cnt-- > 0 && len-- > 0){                                 \
                *str = buf[cnt] + '0';                                  \
                ++str;                                                  \
        }                                                               \
        return str;                                                     \
}

/* Returns a pointer after the last written char, or 'str' on error. */
#define MFMT_UNSIGNED_TO_HEX(TYPE, NAME)                                \
static char*                                                            \
NAME##_to_hex(TYPE val, int uppercase, char padchar, size_t padlen,     \
              size_t len, char *str)                                    \
{                                                                       \
        char buf[24];                                                   \
        size_t cnt = 0;                                                 \
        if (uppercase){                                                 \
                uppercase = 'A';                                        \
        }else{                                                          \
                uppercase = 'a';                                        \
        }                                                               \
        do{                                                             \
                buf[cnt++] = val % 16;                                  \
                val = val / 16;                                         \
        }while (val != 0);                                              \
        if (padlen > cnt){                                              \
                padlen -= cnt;                                          \
                padlen = (padlen < len ? padlen : len);                 \
                memset(str, padchar, padlen);                           \
                str += padlen;                                          \
                len -= padlen;                                          \
        }                                                               \
        while (cnt-- > 0 && len-- > 0){                                 \
                if (buf[cnt] < 10){                                     \
                        *str = buf[cnt] + '0';                          \
                }else{                                                  \
                        *str = (buf[cnt] - 10) + uppercase;             \
                }                                                       \
                ++str;                                                  \
        }                                                               \
        return str;                                                     \
}

/* Returns a pointer after the last written char, or 'str' on error. */
#define MFMT_UNSIGNED_TO_DEC(TYPE, NAME)                                \
static char*                                                            \
NAME##_to_dec(TYPE val, char padchar, size_t padlen,                    \
              size_t len, char *str)                                    \
{                                                                       \
        char buf[24];                                                   \
        size_t cnt = 0;                                                 \
        do{                                                             \
                buf[cnt++] = val % 10;                                  \
                val = val / 10;                                         \
        }while (val != 0);                                              \
        if (padlen > cnt){                                              \
                padlen -= cnt;                                          \
                padlen = (padlen < len ? padlen : len);                 \
                memset(str, padchar, padlen);                           \
                str += padlen;                                          \
                len -= padlen;                                          \
        }                                                               \
        while (cnt-- > 0 && len-- > 0){                                 \
                *str = buf[cnt] + '0';                                  \
                ++str;                                                  \
        }                                                               \
        return str;                                                     \
}

MFMT_DEC_TO_SIGNED(int, int)
//MFMT_HEX_TO_SIGNED(int, int)
//MFMT_SIGNED_TO_DEC(int, int)
//MFMT_SIGNED_TO_HEX(int, int)

MFMT_DEC_TO_UNSIGNED(unsigned int, uint)
MFMT_HEX_TO_UNSIGNED(unsigned int, uint)
//MFMT_UNSIGNED_TO_DEC(unsigned int, uint)
//MFMT_UNSIGNED_TO_HEX(unsigned int, uint)
//MFMT_UNSIGNED_TO_HEX(size_t, siz)

static const char*
parse_arg(const char *fmt, const char *str, va_list args)
{
//    printf("%s\n", __func__);
//    printf("%s, %s\n", str, fmt);

    int *intp, intv = 0;
    unsigned int *uintp, uintv = 0, width = 0;
    char *charp;
    char *cur = (char *)str;

    char length_method = '\0';
    float *floatp;
    double *doublep;

    if (*fmt == 'l' || *fmt == 'L') {
        length_method = 'l';
        ++fmt;
    }

    fmt = dec_to_uint(fmt, &width);

    if (*fmt == 'd') {
        cur = dec_to_int(str, &intv);
        if (cur != str) {
            intp = va_arg(args, int*);
            *intp = intv;
        }
    } else if (*fmt == 'u') {
        cur = dec_to_uint(str, &uintv);
        if (cur != str) {
            uintp = va_arg(args, unsigned int*);
            *uintp = uintv;
        }
    } else if (*fmt == 'x' || *fmt == 'X') {
        cur = hex_to_uint(str, &uintv);
        if (cur != str) {
            uintp = va_arg(args, unsigned int*);
            *uintp = uintv;
        }
    } else if (*fmt == 'f' || *fmt == 'F' || *fmt == 'g' || *fmt == 'G' || *fmt == 'e' || *fmt == 'e') {
        double doublev = strtod(str, &cur);
        if (cur != str) {
            if (length_method == 'l') {
                doublep = va_arg(args, double *);
                *doublep = doublev;
            } else {
                floatp = va_arg(args, float *);
                *floatp = (float) doublev;
            }
        }
    } else if (*fmt == 'c') {
        charp = va_arg(args, char*);
        if (width == 0) {
            width = 1;
        }
        while (cur[0] != '\0' && uintv < width) {
            charp[uintv] = cur[0];
            ++cur;
            ++uintv;
        }
    } else if (*fmt == 's') {
        charp = va_arg(args, char*);
        while (cur[0] != '\0' && ! is_space(cur[0]) &&
                (width == 0 || uintv < width)) {
            charp[uintv] = cur[0];
            ++cur;
            ++uintv;
        }
        charp[uintv] = '\0';
    } else if (*fmt == '%' && str[0] == '%') {
        ++cur;
    }
    return cur;
}

int sscanf(const char *str, const char *fmt, ...)
{
//    printf("%s\n", __func__);
//    printf("%s, %s\n", str, fmt);

    if ((fmt[0] == '%') && (fmt[1] == '[') && (fmt[2] == '^') && (fmt[3] == '#') && (fmt[4] == ']')) {
        va_list param;
        va_start(param, fmt);

        u16 n = 0;
        char *ptr = (char *)str;
        char buf[2] = {0};
        memset(buf, 0, 2);
        uint32_t *des;


//        printf("s:%s, param:%s, fmt:%s\n", str, param, fmt);
//        print_r(param, 10);

        u32 **param1 = param;

        while (*ptr != '#') {
            n++;
            ptr++;
        }

        ptr -= n;
        des = *param1;
    //    printf("1---> n = %d, ptr:%x\n", n, ptr);
        memcpy(des, ptr, n);
        ptr += n;
        n = 0;
    //    printf("1---->buf1:%s\n", des);

        while(*ptr == '#') {
            ptr++;
        }

        va_arg(param, u32);
        u32 **param2 = param;
        while (*ptr != '#') {
            n++;
            ptr++;
        }


        ptr -= n;
    //    printf("2---> n = %d, ptr:%x\n", n, ptr);
        des = *param2;
        memcpy(des, ptr, n);
        ptr += n;
        n = 0;

        while(*ptr == '#') {
            ptr++;
        }
    //
        va_arg(param, u32);
        u32 **param3 = param;
        while (*ptr != '#') {
            n++;
            ptr++;
        }
        memcpy(buf, ptr-n, n);
        des = *param3;
        *des = atoi(buf);
        n = 0;
        va_end(param);
        return 3;
    } else {

        int ret = 0;
        va_list args;
        va_start(args, fmt);
        while (fmt[0] != '\0' && str[0] != '\0') {
            if (fmt[0] == '%') {
                const char * tmp = parse_arg(&fmt[1], str, args);
                if (tmp == str) {
                    break;
                }
                if (fmt[1] != '%') {
                    ++ret;
                }
                ++fmt;
                while (fmt[0] >= '0' && fmt[0] <= '9') {
                    ++fmt;
                }
                ++fmt;
                str = tmp;
            } else if (is_space(fmt[0])) {
                ++fmt;
                str = skip_spaces(str);
            } else if (fmt[0] == str[0]) {
                ++fmt;
                ++str;
            } else {
                break;
            }
        }
        va_end(args);
        return ret;
    }
}


#endif // SECURITY_PAY_EN



