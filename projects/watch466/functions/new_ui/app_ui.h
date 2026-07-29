#ifndef __NEW_UI_H__
#define __NEW_UI_H__

#include "include.h"

/* ---- 全局系统状态（各页面共用，由串口轮询/中断更新） ---- */
typedef struct {
    /* 加热参数（func_heat_set_page → func_heat_page） */
    u16 temp;           // 选中温度 (°F)
    u8  time_min;       // 选中总时间 (分钟)
    u8  keep_warm_min;  // 保温时长 (分钟)
    u32 remain_min;     // 剩余倒计时 (分钟，串口下发)
    /* 系统状态 */
    u8  bat_level;      // 电量档位 0~4
    u8  bat_pct;        // 电量百分比 0~100
    bool charging;       // 充电中
    bool lowbat;         // 低电状态
    bool full_charge;    // 充满电
    bool bt_linked;      // 蓝牙已连接
    bool lid_open;       // 上盖打开
    u8  hour;            // 系统时钟 时
    u8  min;             // 系统时钟 分
} ui_sys_t;
extern ui_sys_t g_ui_sys;

#endif /* __NEW_UI_H__ */
