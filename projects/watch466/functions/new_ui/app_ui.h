#ifndef __NEW_UI_H__
#define __NEW_UI_H__

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
    u8  hour;            // 系统时钟 时
    u8  min;             // 系统时钟 分
} ui_sys_t;
extern ui_sys_t g_ui_sys;

/**
 * @brief 串口状态镜像 → g_ui_sys (func_process 每轮调用, 页面不用管)
 *
 * 单向: lb_ui_state_get() 是唯一数据源, 页面只读 g_ui_sys。
 * 实现见 general_ui.c。
 */
void lb_ui_sync_pull(void);

/*---------------------------------------------------------------------------
 * 预约暂存 —— 预约时间页选好触发时刻后交给加热设置页, 由后者确认时一并下发
 *
 * 流程: 预约键 → 预约时间页(选时分秒) → set() → 加热设置页(选温度/时长)
 *       → 确认时 take() 命中 → 下发 0x03 新增预约, 回首页
 *       → 未命中(直接进的加热设置页) → 下发 0x01 立即加热, 进加热页
 *
 * 实现在 func_appointment_time.c。
 *-------------------------------------------------------------------------*/

/** @brief 暂存预约触发时刻 (unix 秒) */
void new_ui_appointment_set(u32 unix_time);

/** @brief 取出暂存的触发时刻; 无待下发预约返回 false (取出即清) */
bool new_ui_appointment_take(u32 *out_unix_time);

/** @brief 丢弃暂存 (用户中途退出) */
void new_ui_appointment_clear(void);

#endif /* __NEW_UI_H__ */
