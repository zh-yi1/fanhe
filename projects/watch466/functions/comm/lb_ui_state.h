/**
 * @file    lb_ui_state.h
 * @brief   设备状态结构体 — 加热模块上报数据的 UI 侧镜像
 *
 * 数据流:
 *   串口收到 0x01 DataPoints → lb_ui_state_feed_dp() 更新结构体 (唯一写入口)
 *   UI 处理函数 → lb_ui_state_get() 读取 (唯一读接口)
 *
 * UI 用法 (每个页面自己记一个 last_seq):
 *   lb_ui_state_t *st = lb_ui_state_get();
 *   if (st->seq != last_seq) {
 *       last_seq = st->seq;
 *       ...按 st->xxx 刷新控件...
 *   }
 *
 * 注: 时间服务 (lb_time_*) 在 lb_bridge.h; 模式预设 (lunchbox_mode_*) 在 lb_ble_app.h。
 */
#ifndef __LB_UI_STATE_H
#define __LB_UI_STATE_H

#include "include.h"
#include "lb_proto.h"

#if FUNC_LUNCHBOX_UART_EN

typedef struct {
    // ── 加热模块上报的设备状态 (MCU通信协议.md §4.1 属性列表) ──
    u8  power_on;          // DP1  总开关: 0=关 1=开
    u8  heat_mode;         // DP2  加热模式: 0=关 1=自定义 2=鸡腿 3=意面 4=预约 5=保温
    u8  battery;           // DP3  电量: 0=没电 1=低 2=中 3=高 4=满
    u8  charge;            // DP4  充电状态: 0=未充电 1=充电中 2=已充满
    u32 heat_duration;     // DP5  加热时长(分钟)
    u32 remain_time;       // DP6  剩余加热时间(分钟)
    u8  heat_temp;         // DP7  温度档位: 0=40°C ~ 6=100°C
    u8  fault;             // DP9  故障: 0=正常, 其他见协议 §4.1.6 fault_code
    u8  heat_enable;       // DP10 是否加热: 0=停止 1=加热中

    // ── 链路状态 (非模块上报, 由 BLE 回调写入) ──
    bool ble_connected;    // 蓝牙是否已连上 APP (画连接图标用)

    // ── 元信息 ──
    bool valid;            // 是否收到过至少一次上报 (false=以上加热字段无意义)
    u8   seq;              // 更新序号: 任一字段变化时自增, UI 对比判断是否刷新
    u32  tick;             // 最近一次上报时刻 (tick_get)
} lb_ui_state_t;

/** @brief 获取设备状态 (只读使用; 写入只能经 feed_dp) */
lb_ui_state_t *lb_ui_state_get(void);

/**
 * @brief 串口 0x01 数据区 → 更新状态结构体 (串口应用层调用)
 * @return true=有字段发生变化 (seq 已自增)
 */
bool lb_ui_state_feed_dp(const u8 *data, u16 len);

/** @brief 复位为未同步状态 (关机/模块失联时可调); 不影响蓝牙连接标志 */
void lb_ui_state_reset(void);

/**
 * @brief BLE 连接/断开通知 (平台 app_blue_fit.c 的连接/断开回调调用)
 *
 * 状态变化时 seq 自增, 页面按 seq 刷新即可连带刷新蓝牙图标。
 */
void lb_ui_ble_link_set(bool connected);

/** @brief 蓝牙是否已连上 APP (等价于 lb_ui_state_get()->ble_connected) */
bool lb_ui_ble_is_connected(void);

//-----------------------------------------------------------------------------
// 预约列表镜像 (串口 0x02 应答逐帧填充)
//
// UI 用法 (预约页):
//   lb_ui_schedules_t *sch = lb_ui_schedules_get();
//   if (sch->complete && sch->seq != last_seq) { last_seq = sch->seq; 刷新列表; }
//   if (!sch->complete) { ...列表过期/未收齐, 可触发重新查询... }
//-----------------------------------------------------------------------------

#define LB_SCHEDULE_MAX         10      // 最大预约条数

typedef struct {
    u8   id;               // 预约ID
    u8   mode;             // 设置模式: 0=关闭 1=自定义加热 2=鸡腿模式
    char name[32];         // 预约名称
    u32  time;             // unix 触发时间(秒)
    u8   temp;             // 温度档位
    u8   duration;         // 加热时长(分钟)
    u8   enabled;          // 0=关闭 1=开启
    u8   repeat;           // 重复周期位掩码
} lb_ui_schedule_t;

typedef struct {
    lb_ui_schedule_t list[LB_SCHEDULE_MAX];
    u8   count;            // 已收到的条数
    u8   total;            // 加热模块声明的总条数
    bool complete;         // 列表已收齐 (false=传输中或已过期)
    bool valid;            // 收到过至少一次列表应答
    u8   seq;              // 更新序号 (每收一条自增)
    u32  tick;             // 最近一次更新时刻
} lb_ui_schedules_t;

/** @brief 获取预约列表 (只读使用) */
lb_ui_schedules_t *lb_ui_schedules_get(void);

/**
 * @brief 串口 0x02 应答(44B 条目) → 填充列表 (串口应用层调用)
 *
 * seq==1 时重开列表; seq>=total 时置 complete; total==0 表示空列表。
 * @return true=条目已接受
 */
bool lb_ui_schedules_feed_entry(const u8 *data, u16 len);

/** @brief 预约增/改/删生效后调用: 本地列表标记过期 (complete=false) */
void lb_ui_schedules_mark_dirty(void);

//-----------------------------------------------------------------------------
// 自动跳页路由 (模块状态变化 → 应该去哪个页面)
//
// UI 用法 (主循环统一一处调用, 例如 func_process):
//   switch (lb_ui_route_poll()) {
//   case LB_UI_ROUTE_HEAT: 若当前不在加热页 → 跳加热页; break;
//   case LB_UI_ROUTE_WARM: 若当前不在保温页 → 跳保温页; break;
//   case LB_UI_ROUTE_HOME: 若在加热/保温页 → 回首页;   break;
//   default: break;   // LB_UI_ROUTE_NONE: 无需动作
//   }
// 边沿触发: 仅在加热模式/加热使能发生变化的那一轮返回非 NONE,
// 是否真的切页由调用方结合当前页面判断 (已在目标页则忽略)。
//-----------------------------------------------------------------------------

typedef enum {
    LB_UI_ROUTE_NONE = 0,      // 状态没变, 或变化不涉及页面
    LB_UI_ROUTE_HEAT,          // 模块开始加热 (模式1~4) → 加热页
    LB_UI_ROUTE_WARM,          // 模块开始保温 (模式5)   → 保温页
    LB_UI_ROUTE_HOME,          // 加热/保温停止          → 首页
} lb_ui_route_t;

/** @brief 轮询跳页建议 (边沿触发, 一次变化只报一次) */
lb_ui_route_t lb_ui_route_poll(void);

//-----------------------------------------------------------------------------
// 状态查询快捷接口 (读 lb_ui_state, 供 UI 直接用)
//-----------------------------------------------------------------------------

/** @brief 是否有进行中的加热/保温任务 (禁止自动息屏用) */
bool lunchbox_heating_task_active(void);

/** @brief 当前加热模式 (未同步时返回 0) */
u8 lunchbox_get_heat_mode(void);

/** @brief 当前加热使能 (未同步时返回 0) */
u8 lunchbox_get_heat_enable(void);

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __LB_UI_STATE_H
