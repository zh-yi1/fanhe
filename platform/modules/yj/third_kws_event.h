#ifndef _THIRD_KWS_EVENT_H__
#define _THIRD_KWS_EVENT_H__

#include "include.h"

#if (ASR_SELECT == ASR_YJ)

#if THIRD_UI_LIGHT_EN

enum{
    UI_EVENT_START=0x000,
    UI_EVENT_XIAO_LAN_XIAO_LAN,	//小蓝小蓝  1
    UI_EVENT_DA_KAI_SHE_ZHI,	//打开设置  2
    UI_EVENT_DA_KAI_YIN_YUE,	//打开音乐  3
    UI_EVENT_DUAN_SHI_PIN_YAO_KONG,	//打开短视频遥控  4
    UI_EVENT_DA_KAI_TIAN_QI,	//打开天气  5
    UI_EVENT_DA_KAI_XUE_YANG,	//打开血氧  6
    UI_EVENT_DA_KAI_SHUI_MIAN,	//打开睡眠  7
    UI_EVENT_DA_KAI_NAO_ZHONG,	//打开闹钟  8
    UI_EVENT_DA_KAI_XIN_LV,	//打开心率  9
    UI_EVENT_DA_KAI_ZHI_FU_BAO,	//打开支付宝  10
    UI_EVENT_DA_KAI_WEI_XIN_ZHI_FU,	//打开微信支付  11
    UI_EVENT_DA_KAI_SHOU_KUAN_MA,	//打开收款码  12
    UI_EVENT_DA_KAI_CHENG_CHE_MA,	//打开乘车码  13
    UI_EVENT_DA_KAI_YU_YIN_ZHU_SHOU,	//打开语音助手  14
    UI_EVENT_DA_KAI_JI_PI_TI,	//打开记辟涕  15
    UI_EVENT_DA_KAI_YUN_DONG,	//打开运动  16
    UI_EVENT_TING_ZHI_YUN_DONG,	//停止运动  17
    UI_EVENT_JI_XU_YUN_DONG,	//继续运动  18
    UI_EVENT_ZAN_TING_YUN_DONG,	//暂停运动  19
    UI_EVENT_DA_KAI_CHANG_LIANG_MO_SHI,	//打开常亮模式  20
    UI_EVENT_GUAN_BI_CHANG_LIANG_MO_SHI,	//关闭常亮模式  21
    UI_EVENT_DA_KAI_SAO_MA_LIAN_JIE,	//打开扫码连接  22
    UI_EVENT_CHA_ZHAO_SHOU_JI,	//查找手机  23
    UI_EVENT_FAN_HUI_BIAO_PAN,	//返回表盘  24
    UI_EVENT_MAX
};

#endif // THIRD_UI_LIGHT_EN

#endif //ASR_SELECT

#endif
