#ifndef _MACRO_H
#define _MACRO_H

#define UINT_MAX                0xffffffff
#define INT_MAX                 0x7fffffff
#define INT_MIN                 -0x80000000

#define M_PI                    3.1415926535897932384626433f

#define BIT(n)                  (1ul << (n))

#define AT(x)                   __attribute__((section(#x)))
#define ALIGNED(n)              __attribute__((aligned(n)))
#define DMA_ADR(x)              ((u32)x)
#define ALWAYS_INLINE           __attribute__((always_inline)) inline
#define NO_INLINE               __attribute__((noinline))
#define WEAK                    __attribute__((weak))
#define PACKED                  __attribute__((packed))
#define FIQ                     __attribute__((fiq("machine")))

#define WDT_CLR()               {WDTCON = 0xa; RTCCON10 = BIT(14);}//两条狗一起清
#define WDT_EN()                WDTCON = 0x110
#define WDT_DIS()               WDTCON = 0xaa0
#define WDT_RST()               WDTCON = 0xa000110; while (1)
#define WDT_RST_DELAY()         WDTCON = 0xa100110; while (1)

#define RTC_WDT_CLR()           RTCCON10 = BIT(14)
#define RTC_WDT_DIS()           RTCCON12 |= (0x3<<6)
#define RTC_WDT_RST()           RTCCON12 = (RTCCON12 & ~((0x03 << 6) | (0x07 << 16))) | (1 << 19)

#define BYTE0(n)                ((unsigned char)(n))
#define BYTE1(n)                ((unsigned char)((n)>>8))
#define BYTE2(n)                ((unsigned char)((n)>>16))
#define BYTE3(n)                ((unsigned char)((n)>>24))

#define GET_LE16(ptr)           (u16)(*(u16*)(u8*)(ptr))
#define GET_LE32(ptr)           (u32)(*(u32*)(u8*)(ptr))
#define PUT_LE16(ptr, val)      *(u16*)(u8*)(ptr) = (u16)(val)
#define PUT_LE32(ptr, val)      *(u32*)(u8*)(ptr) = (u32)(val)

#define GET_BE16(ptr)           get_be16(ptr)
#define GET_BE32(ptr)           get_be32(ptr)
#define PUT_BE16(ptr, val)      put_be16(ptr, val)
#define PUT_BE32(ptr, val)      put_be32(ptr, val)

#define ABS(x)                  ((s32)(x) < 0 ? (-(s32)(x)) : (s32)(x))
#define MIN(a,b)                ((a) < (b) ? (a) : (b))
#define MAX(a,b)                ((a) > (b) ? (a) : (b))

#define CONST_CAT(x, y)         x ## y
#define SET_MACRO(x, y)         CONST_CAT(x, y)

void wdt_clr(void);

#endif // _MACRO_H
