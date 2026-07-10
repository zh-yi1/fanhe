/**
 * @file    func_lunchbox_lcd.h
 * @brief   饭盒 LCD/屏幕 控制接口
 *
 * 提供给屏幕端(LCD UI)调用的加热控制、预约管理、模式查询等 API。
 * 所有函数直接构造 UART 帧发往加热模块。
 */
#ifndef __FUNC_LUNCHBOX_LCD_H
#define __FUNC_LUNCHBOX_LCD_H

#include "include.h"

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// 加热控制
//-----------------------------------------------------------------------------

/** @brief LCD 启动加热 — 构造 UART 0x01 帧发给加热模块
 *  @param mode     加热模式: 1=自定义, 2=鸡腿, 3=意面, 4=预约, 5=保温
 *  @param temp     温度档位: 0=40°C ~ 6=100°C
 *  @param duration 加热时长(分钟) */
void lunchbox_heat_start(u8 mode, u8 temp, u32 duration);

/** @brief LCD 停止加热 — 构造 UART 0x01 帧发给加热模块 */
void lunchbox_heat_stop(void);

/** @brief LCD 开机 — 发送 PowerSwitch=ON 给加热模块 */
void lunchbox_power_on(void);

/** @brief LCD 关机 — 发送 PowerSwitch=OFF 给加热模块 */
void lunchbox_power_off(void);

//-----------------------------------------------------------------------------
// 保温控制
//-----------------------------------------------------------------------------

/** @brief 进入保温页时下发保温指令 (模式5, 默认 194°F) */
void lunchbox_keep_warm_apply(void);

/** @brief 加热自然结束后自动开启保温 (模式5, 194°F, 至低电关机) */
void lunchbox_keep_warm_start(void);

/** @brief 停止保温 */
void lunchbox_keep_warm_stop(void);

/** @brief BLE/0x04 跳转保温页前设置温度档位 (0~6)，0xff 表示用默认 194°F */
void lunchbox_keep_warm_set_temp_idx(u8 temp_idx);

/** @brief 当前是否处于保温状态 */
bool lunchbox_keep_warm_is_active(void);

/** @brief 主循环轮询保温 (低电关机时停止) */
void lunchbox_keep_warm_poll(void);

//-----------------------------------------------------------------------------
// 预约管理
//-----------------------------------------------------------------------------

/** @brief LCD 发送预约 */
void lunchbox_reservation_send(u8 action, u8 id, const char *name, u32 unix_time,
                               u8 temp, u8 duration, u8 enabled, u8 repeat);

/** @brief LCD 删除预约 */
void lunchbox_reservation_delete(u8 id);

//-----------------------------------------------------------------------------
// 模式预设查询
//-----------------------------------------------------------------------------

/** @brief 获取指定模式的预设温度档位 */
u8 lunchbox_mode_get_temp(u8 mode);

/** @brief 获取指定模式的预设加热时长(分钟) */
u8 lunchbox_mode_get_duration(u8 mode);

/** @brief 华氏度转温度档位 */
u8 lunchbox_temp_f_to_idx(u16 temp_f);

/** @brief 获取当前加热模式 */
u8 lunchbox_get_heat_mode(void);

/** @brief 获取当前加热使能状态 */
u8 lunchbox_get_heat_enable(void);

/** @brief 是否有进行中的加热/保温任务 */
bool lunchbox_heating_task_active(void);

//-----------------------------------------------------------------------------
// 设备信息/时间同步
//-----------------------------------------------------------------------------

/** @brief 设置设备信息 (用于 0x01 产品信息查询应答) */
void lunchbox_set_device_info(lb_device_info_t *info);

/** @brief LCD 时间同步 — 发送 UART 0x01 帧同步时间到加热模块 */
void lunchbox_time_sync(u32 unix_time);

/** @brief LCD 按键通知 — 发送按键值到加热模块 */
void lunchbox_key_notify(u8 key_val);

//-----------------------------------------------------------------------------
// BLE 回调
//-----------------------------------------------------------------------------

/** @brief BLE 连接成功回调 — 主动上报时间戳+发送预设 */
void lunchbox_ble_on_connected(void);

/** @brief BLE 连接后发送5个固定预约预设到加热模块 */
void lunchbox_ble_send_presets(void);

//-----------------------------------------------------------------------------
// 模式界面 → 加热界面 预设传递 (lb_mode_to_heat_preset_t 定义在 func_lunchbox_uart.h)
//-----------------------------------------------------------------------------

void lb_mode_to_heat_set(u8 proto_mode, u16 temp_f, u8 hour, u8 min);
bool lb_mode_to_heat_get(lb_mode_to_heat_preset_t *out);

void lb_heat_autostart_set(bool en);
bool lb_heat_autostart_consume(void);
/** @brief BLE 桥模式已转发 UART 时，func_heat 跳过重复 lunchbox_heat_start */
void lb_heat_uart_remote_set(bool en);
bool lb_heat_uart_remote_consume(void);

/** @brief 协议温度档位 → 华氏度 (0=40°C ~ 6=100°C) */
u16 lunchbox_temp_idx_to_f(u8 idx);

#if ELUNCHBOX_PANEL_EN
/** @brief 更新本地模式预设 (0x0a / 桥模式 BLE 侧同步) */
void lunchbox_mode_preset_local_set(u8 mode, u8 temp_idx, u8 duration_min);
/** @brief 解析 0x04 控制帧中的总开关 DP，执行面板关/开机 */
void lunchbox_control_apply_power_switch(const u8 *data, u16 len);
/** @brief 解析 0x04 控制帧中的加热 DP，跳转 func_heat_panel 并同步参数 */
void lunchbox_control_apply_heat(const u8 *data, u16 len);
/** @brief 解析 0x04 控制帧中的保温模式，跳转 func_new_warm 并同步参数 */
void lunchbox_control_apply_warm(const u8 *data, u16 len);
/** @brief 0x04 控制帧面板侧统一入口（开关机 + 加热/保温页） */
void lunchbox_control_apply_panel(const u8 *data, u16 len);
#endif

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __FUNC_LUNCHBOX_LCD_H
