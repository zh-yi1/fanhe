#include "include.h"
#include "api.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

WEAK bool func_is_video_play(void) {return false;}
WEAK bool func_is_music(void) {return false;}

//设置音量的回调函数, setting_type: 0=同步音量, 1=手机设置音量, 2=按键设置音量
bool dev_vol_set_cb(uint8_t dev_vol, uint8_t media_index, uint8_t setting_type)
{
    if ((func_is_video_play() || func_is_music())) {
        return false;
    }

    if(setting_type & BIT(3)) {
        sys_cb.hfp_vol = dev_vol;
    } else {
        sys_cb.vol = a2dp_vol_conver(dev_vol);
    }

    TRACE("dev_vol_set_cb: %d(%x, %d, %d)\n", sys_cb.vol, dev_vol, media_index, setting_type);

    if(setting_type & BIT(3)) {
        msg_enqueue(EVT_HFP_SET_VOL);
    } else {
        msg_enqueue(EVT_A2DP_SET_VOL);
    }

    return true;
}

