#ifndef _API_MATH_H
#define _API_MATH_H

ALWAYS_INLINE u64 addu (u64 a, u32 b)   { return __builtin_addu(a, b);}                                 //a + b
ALWAYS_INLINE s64 adds (s64 a, s32 b) {return __builtin_adds(a, b);}                                    //a + b
ALWAYS_INLINE u64 addlu (u64 a, u32 b) {return __builtin_addlu(a, b);}                                  //a << 1 + b
ALWAYS_INLINE s64 addls (s64 a, s32 b) {return __builtin_addls(a, b);}                                  //a << 1 + b
ALWAYS_INLINE u64 addru (u64 a, u32 b) {return __builtin_addru(a, b);}                                  //a >> 1 + b
ALWAYS_INLINE s64 addrs (s64 a, s32 b) {return __builtin_addrs(a, b);}                                  //a >> 1 + b

ALWAYS_INLINE u64 subu (u64 a, u32 b) {return __builtin_subu(a, b);}                                    //a - b
ALWAYS_INLINE s64 subs (s64 a, s32 b) {return __builtin_subs(a, b);}                                    //a - b
ALWAYS_INLINE u64 sublu (u64 a, u32 b) {return __builtin_sublu(a, b);}                                  //a << 1 - b
ALWAYS_INLINE s64 subls (s64 a, s32 b) {return __builtin_subls(a, b);}                                  //a << 1 - b
ALWAYS_INLINE u64 subru (u64 a, u32 b) {return __builtin_subru(a, b);}                                  //a >> 1 - b
ALWAYS_INLINE s64 subrs (s64 a, s32 b) {return __builtin_subrs(a, b);}                                  //a >> 1 - b

ALWAYS_INLINE u64 mulu (u32 a, u32 b) {return __builtin_mulu(a, b);}                                    //a * b
ALWAYS_INLINE s64 muls (s32 a, s32 b) {return __builtin_muls(a, b);}                                    //a * b
ALWAYS_INLINE s64 mulus (u32 a, s32 b) {return __builtin_mulus(a, b);}                                  //a * b
ALWAYS_INLINE u64 macu (u64 c, u32 a, u32 b) {return __builtin_macu (c, a, b);}                         //c + a * b
ALWAYS_INLINE s64 macs (s64 c, s32 a, s32 b) {return __builtin_macs (c, a, b);}                         //c + a * b
ALWAYS_INLINE s64 macus (s64 c, u32 a, s32 b) {return __builtin_macus (c, a, b);}                       //c + a * b
ALWAYS_INLINE u64 msbu (u64 c, u32 a, u32 b) {return __builtin_msbu (c, a, b);}                         //c - a * b
ALWAYS_INLINE s64 msbs (s64 c, s32 a, s32 b) {return __builtin_msbs (c, a, b);}                         //c - a * b
ALWAYS_INLINE s64 msbus (s64 c, u32 a, s32 b) {return __builtin_msbus (c, a, b);}                       //c - a * b

ALWAYS_INLINE u64 mulu_shift16 (u32 a, u32 b) {return __builtin_mulu_shift16(a, b);}                    //(a * b) >> 16
ALWAYS_INLINE s64 muls_shift16 (s32 a, s32 b) {return __builtin_muls_shift16(a, b);}                    //(a * b) >> 16
ALWAYS_INLINE s64 muls_shift15 (s32 a, s32 b) {return __builtin_muls_shift15(a, b);}                    //(a * b) >> 15
ALWAYS_INLINE s64 mulus_shift16 (u32 a, s32 b) {return __builtin_mulus_shift16(a, b);}                  //(a * b) >> 16
ALWAYS_INLINE u64 macu_shift16 (u64 c, u32 a, u32 b) {return __builtin_macu_shift16 (c, a, b);}         //c + ((a * b) >> 16)
ALWAYS_INLINE s64 macs_shift16 (s64 c, s32 a, s32 b) {return __builtin_macs_shift16 (c, a, b);}         //c + ((a * b) >> 16)
ALWAYS_INLINE s64 macs_shift15 (s64 c, s32 a, s32 b) {return __builtin_macs_shift15 (c, a, b);}         //c + ((a * b) >> 15)
ALWAYS_INLINE s64 macus_shift16 (s64 c, u32 a, s32 b) {return __builtin_macus_shift16 (c, a, b);}       //c + ((a * b) >> 16)
ALWAYS_INLINE u64 msbu_shift16 (u64 c, u32 a, u32 b) {return __builtin_msbu_shift16 (c, a, b);}         //c - ((a * b) >> 16)
ALWAYS_INLINE s64 msbs_shift16 (s64 c, s32 a, s32 b) {return __builtin_msbs_shift16 (c, a, b);}         //c - ((a * b) >> 16)
ALWAYS_INLINE s64 msbs_shift15 (s64 c, s32 a, s32 b) {return __builtin_msbs_shift15 (c, a, b);}         //c - ((a * b) >> 15)
ALWAYS_INLINE s64 msbus_shift16 (s64 c, u32 a, s32 b) {return __builtin_msbus_shift16 (c, a, b);}       //c - ((a * b) >> 16)

ALWAYS_INLINE u64 mulu_shift24 (u32 a, u32 b) {return __builtin_mulu_shift24(a, b);}                    //(a * b) >> 24
ALWAYS_INLINE s64 muls_shift24 (s32 a, s32 b) {return __builtin_muls_shift24(a, b);}                    //(a * b) >> 24
ALWAYS_INLINE s64 muls_shift23 (s32 a, s32 b) {return __builtin_muls_shift23(a, b);}                    //(a * b) >> 23
ALWAYS_INLINE s64 mulus_shift24 (u32 a, s32 b) {return __builtin_mulus_shift24(a, b);}                  //(a * b) >> 24
ALWAYS_INLINE u64 macu_shift24 (u64 c, u32 a, u32 b) {return __builtin_macu_shift24 (c, a, b);}         //c + ((a * b) >> 24)
ALWAYS_INLINE s64 macs_shift24 (s64 c, s32 a, s32 b) {return __builtin_macs_shift24 (c, a, b);}         //c + ((a * b) >> 24)
ALWAYS_INLINE s64 macs_shift23 (s64 c, s32 a, s32 b) {return __builtin_macs_shift23 (c, a, b);}         //c + ((a * b) >> 23)
ALWAYS_INLINE s64 macus_shift24 (s64 c, u32 a, s32 b) {return __builtin_macus_shift24 (c, a, b);}       //c + ((a * b) >> 24)
ALWAYS_INLINE u64 msbu_shift24 (u64 c, u32 a, u32 b) {return __builtin_msbu_shift24 (c, a, b);}         //c - ((a * b) >> 24)
ALWAYS_INLINE s64 msbs_shift24 (s64 c, s32 a, s32 b) {return __builtin_msbs_shift24 (c, a, b);}         //c - ((a * b) >> 24)
ALWAYS_INLINE s64 msbs_shift23 (s64 c, s32 a, s32 b) {return __builtin_msbs_shift23 (c, a, b);}         //c - ((a * b) >> 23)
ALWAYS_INLINE s64 msbus_shift24 (s64 c, u32 a, s32 b) {return __builtin_msbus_shift24 (c, a, b);}       //c - ((a * b) >> 24)

ALWAYS_INLINE u64 mulu_shift32 (u32 a, u32 b) {return __builtin_mulu_shift32(a, b);}                    //(a * b) >> 32
ALWAYS_INLINE s64 muls_shift32 (s32 a, s32 b) {return __builtin_muls_shift32(a, b);}                    //(a * b) >> 32
ALWAYS_INLINE s64 muls_shift31 (s32 a, s32 b) {return __builtin_muls_shift31(a, b);}                    //(a * b) >> 31
ALWAYS_INLINE s64 mulus_shift32 (u32 a, s32 b) {return __builtin_mulus_shift32(a, b);}                  //(a * b) >> 32
ALWAYS_INLINE u64 macu_shift32 (u64 c, u32 a, u32 b) {return __builtin_macu_shift32 (c, a, b);}         //c + ((a * b) >> 32)
ALWAYS_INLINE s64 macs_shift32 (s64 c, s32 a, s32 b) {return __builtin_macs_shift32 (c, a, b);}         //c + ((a * b) >> 32)
ALWAYS_INLINE s64 macs_shift31 (s64 c, s32 a, s32 b) {return __builtin_macs_shift31 (c, a, b);}         //c + ((a * b) >> 31)
ALWAYS_INLINE s64 macus_shift32 (s64 c, u32 a, s32 b) {return __builtin_macus_shift32 (c, a, b);}       //c + ((a * b) >> 32)
ALWAYS_INLINE u64 msbu_shift32 (u64 c, u32 a, u32 b) {return __builtin_msbu_shift32 (c, a, b);}         //c - ((a * b) >> 32)
ALWAYS_INLINE s64 msbs_shift32 (s64 c, s32 a, s32 b) {return __builtin_msbs_shift32 (c, a, b);}         //c - ((a * b) >> 32)
ALWAYS_INLINE s64 msbs_shift31 (s64 c, s32 a, s32 b) {return __builtin_msbs_shift31 (c, a, b);}         //c - ((a * b) >> 31)
ALWAYS_INLINE s64 msbus_shift32 (s64 c, u32 a, s32 b) {return __builtin_msbus_shift32 (c, a, b);}       //c - ((a * b) >> 32)

ALWAYS_INLINE u64 lshift64 (u64 a, u32 b) {return __builtin_lshift64 (a,b);}                            //a << b
ALWAYS_INLINE u64 rshift64 (u64 a, u32 b) {return __builtin_rshift64 (a,b);}                            //a >> b
ALWAYS_INLINE s64 ashift64 (s64 a, u32 b) {return __builtin_ashift64 (a,b);}                            //a >> b(算术)
ALWAYS_INLINE u64 clip64u (u64 a, u32 b) {return __builtin_clip64u (a, b);}                             //clip(a >> b)
ALWAYS_INLINE s64 clip64s (s64 a, u32 b) {return __builtin_clip64s (a, b);}                             //clip(a >> b)
ALWAYS_INLINE u64 round64 (u64 a, u32 b) {return __builtin_round64 (a,b);}                              //round(a >> b)

ALWAYS_INLINE s32 clb(s32 x0)
{
    s32 y0;
    __asm__ volatile("p.clb %0, %1" : "=r"(y0) : "r"(x0));
    return y0;
}

ALWAYS_INLINE s32 max(s32 x0, s32 x1)
{
    s32 y0;
    __asm__ volatile("p.max %0, %1, %2" : "=r"(y0) : "r"(x0), "r"(x1));
    return y0;
}

ALWAYS_INLINE u32 maxu(u32 x0, u32 x1)
{
    u32 y0;
    __asm__ volatile("p.maxu %0, %1, %2" : "=r"(y0) : "r"(x0), "r"(x1));
    return y0;
}

ALWAYS_INLINE s32 min(s32 x0, s32 x1)
{
    s32 y0;
    __asm__ volatile("p.min %0, %1, %2" : "=r"(y0) : "r"(x0), "r"(x1));
    return y0;
}

ALWAYS_INLINE u32 minu(u32 x0, u32 x1)
{
    u32 y0;
    __asm__ volatile("p.minu %0, %1, %2" : "=r"(y0) : "r"(x0), "r"(x1));
    return y0;
}

ALWAYS_INLINE s32 clip(s32 x0, s32 x1)
{
    s32 y0;
    __asm__ volatile("p.clip %0, %1, %2" : "=r"(y0) : "r"(x0), "i"(x1));
    return y0;
}

ALWAYS_INLINE u32 clipu(u32 x0, u32 x1)
{
    u32 y0;
    __asm__ volatile("p.clipu %0, %1, %2" : "=r"(y0) : "r"(x0), "i"(x1));
    return y0;
}

ALWAYS_INLINE s32 abs_s(s32 x0)
{
    s32 y0;
    __asm__ volatile("p.abs %0, %1" : "=r"(y0) : "r"(x0));
    return y0;
}

ALWAYS_INLINE int fl1(s32 x0)
{
    s32 y0;
    __asm__ volatile("p.fl1 %0, %1" : "=r"(y0) : "r"(x0));
    return y0;
}

ALWAYS_INLINE unsigned int xbs_ff1(unsigned int rs1)
{
    unsigned int y0;
    __asm__ volatile("p.ff1 %0, %1" : "=r"(y0) : "r"(rs1));
    return y0;
}

ALWAYS_INLINE unsigned int soft_pow(unsigned int x, unsigned int n)
{
    unsigned int res = x;
    for(u8 i=0;(i<n - 1);i++) {
        res *= x;
    }
    return res;
}

typedef enum {
    MATH_Q31    = 0,
    MATH_Q23,
    MATH_Q15,
    MATH_Q7,
    MATH_Q0,
} MATH_Q_TYPE;

enum HM_DTYPE {
    HM_DT_UNUSED,
    HM_DT32,
    HM_DT16,
    HM_DT8,
};

u32 log2_hw(u32 X, u8 Q_in, u8 Q_out);
u32 log10_hw(u32 X, u8 Q_in, u8 Q_out);
u32 ln_hw(u32 X, u8 Q_in, u8 Q_out);
u32 pow2_hw(u32 X, u8 Q_in, u8 Q_out);
u32 powe_hw(u32 X, u8 Q_in, u8 Q_out);
s32 exp_hw(s32 x);                      //input Q11, Output Q16
s32 sin_hw(s32 x);                      //input Q28 output Q15, 0<=x<=2, y=sin(x*pi)
s32 cos_hw(s32 x);

#define DEG_V(deg)          ((int)((deg) * 8192 / 900))

#define SIN(deg)            my_sin(deg)   //my_sin(DEG_V(deg))
#define COS(deg)            my_cos(deg)   //my_cos(DEG_V(deg))
#define ARCTAN2(y, x)       cordic_arctan((y), (x))

s32 my_sin(s16 phase);
s32 my_cos(s16 phase);
s32 my_arctan(int y, int x);
u32 sqrt64(u64 x);

u32 cordic_sqrt_sum(int x, int y);  //sqrt(x^2 + y^2), 输入x, y及输出均为Q24
u32 cordic_sqrt_sub(int x, int y);  //sqrt(x^2 - y^2), 输入x, y及输出均为Q24
s32 cordic_arctan(int y, int x);    //arctan(y/x), 输入x, y均为Q8, 输出精度为0.1度

void math_init(void);
void math_exit(void);

#endif
