#include "include.h"
#include "lwip/sys.h"

int extern_vsnprintf_builtin(char * buffer, size_t count, const char * format, va_list va);

#if BT_PANU_EN
extern u32 __net_data_start, __net_data_size;
void net_bss_init(void)
{
    memset(&__net_data_start, 0, (u32)&__net_data_size);
}

void netlib_keep_symbols(void)
{
    asm(".global strcasecmp");
    asm(".global strerror");
    asm(".global strtod");
    asm(".global strdup");
    asm(".global time");
    asm(".global srand");
    asm(".global strtol");
    // asm(".global vsnprintf");

    // awk
    asm(".global powf");
    asm(".global ceilf");
    asm(".global strtok");
    asm(".global atol");
    asm(".global strrchr");
    asm(".global sqrt");
    asm(".global sqrtf");
    asm(".global qsort");
    asm(".global sin");
    asm(".global cos");
    asm(".global tan");
    asm(".global log");
    asm(".global hypotf");
    asm(".global floorf");
    asm(".global __fixsfsi");
    asm(".global __fixsfdi");
    asm(".global __fixdfdi");
    asm(".global __floatdidf");
    asm(".global __floatunsisf");
    asm(".global __floatdisf");
}

void *netlib_malloc(size_t size, const char *tag)
{
    return ab_malloc(size);
}

void *netlib_calloc(size_t nitems, size_t size, const char *tag)
{
    void *ret = ab_calloc(nitems, size);
    return ret;
}

void *netlib_realloc(void *p, size_t new_size, const char *tag)
{
    return ab_realloc(p, new_size);
}

void netlib_free(void *p)
{
    return ab_free(p);
}

int gettimeofday(struct timeval *__tp, void *__tzp)
{
    if (__tp)
    {
        u32 tick = tick_get();
        __tp->tv_sec = tick / 1000;
        __tp->tv_usec = (tick % 1000) * 1000;
    }

    return 0;
}

// 需要完整的vsnprintf实现
int net_vsnprintf(char * buffer, size_t count, const char * format, va_list va)
{
    // int ret = extern_vsnprintf_builtin(buffer, count, format, va);
    int ret = vsnprintf(buffer, count, format, va);
    // printf("%s %d %s\n", __func__, ret, buffer);
    return ret;
}

#endif
