/**
 * @file    func_lunchbox_heat_cmd.h
 * @brief   主机 → 加热模块 命令封装 (本机发送的唯一出口)
 *
 * 纯发送: 组数据区 → lunchbox_uart_send_frame() 发出, 不带任何业务状态。
 * msg_flag 由本模块自增维护, 号段 0x80~0xFF, 与 APP 转发(沿用 APP 原号)错开。
 * 加热模块的应答/状态回到 lb_uart_on_frame() → lb_ui_state, UI 从结构体拉取。
 *
 * 返回值统一: true=已发出, false=TX被阻断(关机)或组帧失败。
 */
#ifndef __FUNC_LUNCHBOX_HEAT_CMD_H
#define __FUNC_LUNCHBOX_HEAT_CMD_H

#include "include.h"

#if FUNC_LUNCHBOX_UART_EN

/**
 * @brief 开始加热/保温 (0x01: 模式+时长+温度+加热使能+开机)
 * @param mode         1=自定义 2=鸡腿 3=意面 4=预约 5=保温
 * @param temp_idx     温度档位 0~6 (0=40°C ~ 6=100°C)
 * @param duration_min 加热时长(分钟)
 */
bool lb_heat_cmd_start(u8 mode, u8 temp_idx, u32 duration_min);

/** @brief 停止加热/保温 (0x01: 加热使能=0) */
bool lb_heat_cmd_stop(void);

/** @brief 总开关 (0x01: PowerSwitch); 只发命令, 屏幕电源时序由调用方处理 */
bool lb_heat_cmd_power(bool on);

/** @brief 按键通知 (0x01: dpid=12), key 0~9 */
bool lb_heat_cmd_key_notify(u8 key);

/** @brief 时间同步 (0x01: dpid=11 unix时间戳 + 开机位) */
bool lb_heat_cmd_time_sync(u32 unix_ts);

/**
 * @brief 预约新增/修改/删除 (0x03, 42B)
 * @param action    0=删除 1=自定义加热 2=鸡腿模式 3=意面模式
 * @param id        预约ID (删除时=目标ID)
 * @param name      预约名称, 最长32字节, 可为 NULL
 * @param unix_time 触发时间(unix秒)
 * @param temp_idx  温度档位
 * @param duration  加热时长(分钟)
 * @param enabled   0=关闭 1=开启
 * @param repeat    重复周期位掩码 (0xff=每天)
 */
bool lb_heat_cmd_schedule_set(u8 action, u8 id, const char *name, u32 unix_time,
                              u8 temp_idx, u8 duration, u8 enabled, u8 repeat);

/** @brief 删除预约 (0x03, 2B: action=0 + id) */
bool lb_heat_cmd_schedule_delete(u8 id);

/** @brief 查询预约列表 (0x02, 无数据; 应答逐帧进 lb_ui_schedules) */
bool lb_heat_cmd_schedule_query(void);

/** @brief 最近一次成功发出的 msg_flag (需要与应答配对时用) */
u8 lb_heat_cmd_last_flag(void);

//-----------------------------------------------------------------------------
// 温度档位换算 (协议 §4.1.4: 0=40°C ~ 6=100°C)
//-----------------------------------------------------------------------------

/** @brief 温度档位 → 华氏度 */
u16 lunchbox_temp_idx_to_f(u8 idx);

/** @brief 华氏度 → 温度档位 (取最近档位) */
u8 lunchbox_temp_f_to_idx(u16 temp_f);

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __FUNC_LUNCHBOX_HEAT_CMD_H
