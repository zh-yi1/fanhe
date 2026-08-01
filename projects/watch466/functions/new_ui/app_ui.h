#ifndef __APP_UI_H__
#define __APP_UI_H__

/* 本文件由 UI 端提供(原名 ui.h, 与资源头文件同名且被 .gitignore 吞掉, 故改名),
 * UI 那边改版时会整个重发覆盖 —— 所以这里只放 UI 自己的数据结构,
 * 通信移植层的接口声明一律写在 general_ui.h, 别再往这个文件里加。 */

#include "include.h"

/* ---- 全局系统状态（各页面共用，由串口轮询/中断更新） ---- */
typedef struct {
    /* 加热参数（func_heat_set_page → func_heat_page） */
    u16 temp;           // 选中温度 (°F)
    u32 time_min;       // 选中总时间 (分钟) —— 保温 24H=1440, u8 放不下
    u32 keep_warm_min;  // 已保温时长 (分钟) —— 满环 1440, u8 放不下
    u32 remain_min;     // 剩余倒计时 (分钟，串口下发)
    /* 系统状态 */
    u8  bat_level;      // 电量档位 0~4
    u8  bat_pct;        // 电量百分比 0~100
    bool charging;       // 充电中
    bool lowbat;         // 低电状态
    bool full_charge;    // 充满电
    bool bt_linked;      // 蓝牙已连接
    bool lid_open;       // 上盖打开
    u8  heat_trigger;    // DP15 加热触发模式: 0=正常 1=预约触发 2=开关盖恢复
    u8  hour;            // 系统时钟 时
    u8  min;             // 系统时钟 分
} ui_sys_t;
extern ui_sys_t g_ui_sys;

/* lb_ui_sync_pull() / new_ui_appointment_*() 见 general_ui.h */

#endif /* __APP_UI_H__ */
