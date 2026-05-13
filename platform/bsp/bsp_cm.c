#include "include.h"
#include "func.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

///CM Init时，判断该Page参数是否有效，有效则加载
bool cm_loadparam(void *buff, uint page)
{
    return true;
}

///CM Init时，如果找不到有效的参数，则进行出厂值设置
void cm_factory(void *buff, uint page)
{
    u8 *buf = buff;
    memset(buf, 0, 250);

    TRACE("cm_factory: %d\n", page);
    if (page == SYS_CM_PAGE_NUM) {
        //系统参数初始化
        sys_cb.vol = SYS_INIT_VOLUME;
        sys_cb.hfp_vol = SYS_INIT_VOLUME / sys_cb.hfp2sys_mul;
        sys_cb.lang_id = LANG_SELECT;
        buf[PARAM_SYS_VOL] = sys_cb.vol;
        //buf[PARAM_HSF_VOL] = sys_cb.hfp_vol;
        buf[PARAM_LANG_ID] = sys_cb.lang_id;
        PUT_LE32(buf + PARAM_RANDOM_KEY, sys_get_rand_key());
    }
}
