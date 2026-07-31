/**
 * @file    lb_heat_cmd.c
 * @brief   主机 → 加热模块 命令封装实现 — 见 lb_heat_cmd.h
 */
#include "include.h"
#include "lb_proto.h"
#include "lb_uart_app.h"
#include "lb_heat_cmd.h"
#include "lb_ui_state.h"    // lb_ui_heat_stop_expected(): 主动停止不进保温
#include "lb_bridge.h"      // lb_get_unix_time(): 状态查询帧带时间戳

#if FUNC_LUNCHBOX_UART_EN

// 本机流水号: 号段 0x80~0xFF, 与 APP 转发(0x00 起的小号)错开, 避免应答误配对
static u8 lb_heat_cmd_flag = 0x80;
static u8 lb_heat_cmd_flag_last = 0x80;

u8 lb_heat_cmd_last_flag(void)
{
    return lb_heat_cmd_flag_last;
}

/** @brief 统一发送: 成功才自增流水号 */
static bool lb_heat_cmd_send(u8 uart_cmd, const u8 *data, u16 len)
{
    if (!lunchbox_uart_send_frame(uart_cmd, lb_heat_cmd_flag,
                                  LB_ERR_SUCCESS, data, len)) {
        return false;
    }
    lb_heat_cmd_flag_last = lb_heat_cmd_flag;
    lb_heat_cmd_flag++;
    lb_heat_cmd_flag |= 0x80;      // 溢出回卷后仍留在本机号段
    return true;
}

bool lb_heat_cmd_start(u8 mode, u8 temp_idx, u32 duration_min)
{
    u8 data[40];
    u8 *p = data;

    p += lb_dp_encode_enum(p, LB_DPID_HEAT_MODE, mode);
    p += lb_dp_encode_value(p, LB_DPID_HEAT_DURATION, duration_min);
    p += lb_dp_encode_enum(p, LB_DPID_HEAT_TEMP, temp_idx);
    p += lb_dp_encode_bool(p, LB_DPID_HEAT_ENABLE, 1);
    p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);
    if (!lb_heat_cmd_send(LB_UART_CMD_DYNAMIC, data, (u16)(p - data))) {
        return false;
    }
    lb_ui_state_predict_start(mode, temp_idx, duration_min, duration_min);
    return true;
}

bool lb_heat_cmd_resume(u8 mode, u8 temp_idx, u32 duration_min, u32 remain_min)
{
    u8 data[48];
    u8 *p = data;

    p += lb_dp_encode_enum(p, LB_DPID_HEAT_MODE, mode);
    p += lb_dp_encode_value(p, LB_DPID_HEAT_DURATION, duration_min);
    p += lb_dp_encode_value(p, LB_DPID_REMAIN_TIME, remain_min);
    p += lb_dp_encode_enum(p, LB_DPID_HEAT_TEMP, temp_idx);
    p += lb_dp_encode_bool(p, LB_DPID_HEAT_ENABLE, 1);
    p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);
    if (!lb_heat_cmd_send(LB_UART_CMD_DYNAMIC, data, (u16)(p - data))) {
        return false;
    }
    lb_ui_state_predict_start(mode, temp_idx, duration_min, remain_min);
    return true;
}

bool lb_heat_cmd_stop(void)
{
    u8 data[16];
    u8 *p = data;

    lb_ui_heat_stop_expected();         // 主动停止: 路由回首页, 不进保温

    p += lb_dp_encode_bool(p, LB_DPID_HEAT_ENABLE, 0);
    p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);
    if (!lb_heat_cmd_send(LB_UART_CMD_DYNAMIC, data, (u16)(p - data))) {
        return false;
    }
    lb_ui_state_predict_stop(true);     // 实测模块停止后 DP2 也回 0
    return true;
}

bool lb_heat_cmd_heat_off(void)
{
    u8 data[8];
    u16 len;

    lb_ui_heat_stop_expected();         // 关机时序的停止同样是主动停止

    len = lb_dp_encode_bool(data, LB_DPID_HEAT_ENABLE, 0);
    if (!lb_heat_cmd_send(LB_UART_CMD_DYNAMIC, data, len)) {
        return false;
    }
    lb_ui_state_predict_stop(false);    // 只清使能, 不碰模式
    return true;
}

bool lb_heat_cmd_power(bool on)
{
    u8 data[8];
    u16 len = lb_dp_encode_bool(data, LB_DPID_POWER_SWITCH, on ? 1 : 0);
    if (!lb_heat_cmd_send(LB_UART_CMD_DYNAMIC, data, len)) {
        return false;
    }
    lb_ui_state_predict_power(on);
    return true;
}

bool lb_heat_cmd_key_notify(u8 key)
{
    u8 data[8];
    u16 len = lb_dp_encode_enum(data, LB_DPID_KEY_NOTIFY, key);
    return lb_heat_cmd_send(LB_UART_CMD_DYNAMIC, data, len);
}

bool lb_heat_cmd_time_sync(u32 unix_ts)
{
    u8 data[16];
    u8 *p = data;

    p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, unix_ts);
    p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);
    return lb_heat_cmd_send(LB_UART_CMD_DYNAMIC, data, (u16)(p - data));
}

bool lb_heat_cmd_status_query(void)
{
    u8 data[8];
    u16 len = lb_dp_encode_value(data, LB_DPID_TIME_SYNC, lb_get_unix_time());
    return lb_heat_cmd_send(LB_UART_CMD_DYNAMIC, data, len);
}

bool lb_heat_cmd_mode_preset(u8 mode, u8 temp_idx, u32 duration_min)
{
    // 只发属性, 不带 DP10(是否加热): 通知模块预设变更, 不触发加热动作
    u8 data[24];
    u8 *p = data;

    p += lb_dp_encode_enum(p, LB_DPID_HEAT_MODE, mode);
    p += lb_dp_encode_enum(p, LB_DPID_HEAT_TEMP, temp_idx);
    p += lb_dp_encode_value(p, LB_DPID_HEAT_DURATION, duration_min);
    return lb_heat_cmd_send(LB_UART_CMD_DYNAMIC, data, (u16)(p - data));
}

bool lb_heat_cmd_schedule_set(u8 action, u8 id, const char *name, u32 unix_time,
                              u8 temp_idx, u8 duration, u8 enabled, u8 repeat)
{
    // 42B: action(1)+id(1)+name(32)+time(4,BE)+temp(1)+duration(1)+enabled(1)+repeat(1)
    u8 data[42];
    memset(data, 0, sizeof(data));

    data[0] = action;
    data[1] = id;
    if (name) {
        for (u8 i = 0; i < 32 && name[i]; i++) {
            data[2 + i] = (u8)name[i];
        }
    }
    data[34] = (u8)(unix_time >> 24);
    data[35] = (u8)(unix_time >> 16);
    data[36] = (u8)(unix_time >> 8);
    data[37] = (u8)(unix_time & 0xFF);
    data[38] = temp_idx;
    data[39] = duration;
    data[40] = enabled;
    data[41] = repeat;
    return lb_heat_cmd_send(LB_UART_CMD_SCHEDULE_OP, data, 42);
}

bool lb_heat_cmd_schedule_delete(u8 id)
{
    // MCU 协议 §3.6 要求完整 42 字节结构体, 只发 [action,id] 模块回 err=01。
    // 除 action/ID 外全部填 0, 模块按 action=0 删除对应 ID。
    return lb_heat_cmd_schedule_set(0x00, id, NULL, 0, 0, 0, 0, 0);
}

bool lb_heat_cmd_schedule_query(void)
{
    return lb_heat_cmd_send(LB_UART_CMD_SCHEDULE, NULL, 0);
}

//-----------------------------------------------------------------------------
// 温度档位换算
//-----------------------------------------------------------------------------

u16 lunchbox_temp_idx_to_f(u8 idx)
{
    u16 temp_c;

    if (idx > 6) {
        idx = 6;
    }
    temp_c = 40 + (u16)idx * 10;
    return (u16)(temp_c * 9 / 5 + 32);
}

u8 lunchbox_temp_f_to_idx(u16 temp_f)
{
    if (temp_f <= 113) return 0;      // ~40°C
    if (temp_f <= 131) return 1;      // ~50°C
    if (temp_f <= 149) return 2;      // ~60°C
    if (temp_f <= 167) return 3;      // ~70°C
    if (temp_f <= 185) return 4;      // ~80°C
    if (temp_f <= 203) return 5;      // ~90°C
    return 6;                         // 100°C
}

#endif // FUNC_LUNCHBOX_UART_EN
