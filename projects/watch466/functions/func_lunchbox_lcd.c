/**
 * @file    func_lunchbox_lcd.c
 * @brief   饭盒 LCD/屏幕 控制接口实现
 *
 * 提供给屏幕端(LCD UI)调用的加热控制、预约管理、模式查询等 API。
 * 所有函数直接构造 UART 帧 (通过 lb_uart_send_raw) 发往加热模块，
 * 不依赖 BLE↔UART 翻译路径。
 */
#include "include.h"
#include "func_lunchbox_uart.h"
#include "func_lunchbox_uart_internal.h"
#include "func_lunchbox_lcd.h"
#if ELUNCHBOX_PANEL_EN
#include "home_ui_shared.h"
#include "func.h"
#include "func_key_lock.h"
#endif
#include "bsp_vbat.h"
#include "heat_display_reg.h"

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// 保温常量
//-----------------------------------------------------------------------------
#define LB_KEEP_WARM_MODE       5
#define LB_KEEP_WARM_TEMP_F     194

//-----------------------------------------------------------------------------
// LCD 加热任务状态 (lb_heating_sync_from_dp 在 core 中通过 extern 访问)
//-----------------------------------------------------------------------------
bool lb_keep_warm_active = false;
bool lb_heat_lcd_active;     /* LCD 已下发加热/保温，至 stop 或 MCU 确认结束 */
static u8 lb_warm_temp_idx = 0xff;  /* 0xff=默认 194°F；BLE 跳转保温页前可指定 */

//-----------------------------------------------------------------------------
// 模式界面 → 加热界面 预设参数传递
//-----------------------------------------------------------------------------
static lb_mode_to_heat_preset_t lb_mode_heat_preset;

void lb_mode_to_heat_set(u8 proto_mode, u16 temp_f, u8 hour, u8 min)
{
    lb_mode_heat_preset.active     = true;
    lb_mode_heat_preset.proto_mode = proto_mode;
    lb_mode_heat_preset.temp_f     = temp_f;
    lb_mode_heat_preset.hour       = hour;
    lb_mode_heat_preset.min        = min;
}

bool lb_mode_to_heat_get(lb_mode_to_heat_preset_t *out)
{
    if (!lb_mode_heat_preset.active) return false;
    if (out) memcpy(out, &lb_mode_heat_preset, sizeof(lb_mode_to_heat_preset_t));
    lb_mode_heat_preset.active = false;
    return true;
}

//-----------------------------------------------------------------------------
// 新加热页 → 加热页 自动启动标志 (一次性消费)
//-----------------------------------------------------------------------------
static bool lb_heat_autostart_flag;

void lb_heat_autostart_set(bool en)
{
    lb_heat_autostart_flag = en;
}

bool lb_heat_autostart_consume(void)
{
    bool val = lb_heat_autostart_flag;
    lb_heat_autostart_flag = false;
    return val;
}

/** @brief BLE 0x04 已转发 UART 时，func_heat 侧跳过重复 lunchbox_heat_start */
static bool lb_heat_uart_remote_flag;

void lb_heat_uart_remote_set(bool en)
{
    lb_heat_uart_remote_flag = en;
}

bool lb_heat_uart_remote_consume(void)
{
    bool val = lb_heat_uart_remote_flag;
    lb_heat_uart_remote_flag = false;
    return val;
}

/** @brief 协议温度档位 → 华氏度 (0=40°C ~ 6=100°C) */
u16 lunchbox_temp_idx_to_f(u8 idx)
{
    u16 temp_c;

    if (idx > 6) {
        idx = 6;
    }
    temp_c = 40 + (u16)idx * 10;
    return (u16)(temp_c * 9 / 5 + 32);
}

//-----------------------------------------------------------------------------
// 状态查询
//-----------------------------------------------------------------------------

/** @brief 是否有进行中的加热/保温任务（用于禁止自动息屏） */
bool lunchbox_heating_task_active(void)
{
    if (lb_heat_lcd_active || lb_keep_warm_active || lb_heat_task_active) {
        return true;
    }
#if !LB_BRIDGE_MODE
    if (lb_attr_heat_enable) {
        return true;
    }
    if (lb_attr_heat_mode != 0 && lb_attr_remain_time > 0) {
        return true;
    }
#endif
    return false;
}

void lunchbox_set_device_info(lb_device_info_t *info)
{
    if (info) memcpy(&lb_dev_info, info, sizeof(lb_device_info_t));
}

//-----------------------------------------------------------------------------
// 温度/模式工具
//-----------------------------------------------------------------------------

/** @brief 华氏度 → 温度档位 (协议: 0=40°C ~ 6=100°C, 取最近档位) */
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

u8 lunchbox_mode_get_temp(u8 mode) {
    return (mode <= 5) ? lb_mode_temp[mode] : 0;
}

u8 lunchbox_mode_get_duration(u8 mode) {
    return (mode <= 5) ? lb_mode_duration[mode] : 0;
}

#if LB_BRIDGE_MODE
u8 lunchbox_get_heat_mode(void)   { return 0; }
u8 lunchbox_get_heat_enable(void) { return 0; }
#else
u8 lunchbox_get_heat_mode(void)   { return lb_attr_heat_mode; }
u8 lunchbox_get_heat_enable(void) { return lb_attr_heat_enable; }
#endif

//-----------------------------------------------------------------------------
// 加热控制
//-----------------------------------------------------------------------------

/**
 * @brief LCD 启动加热 — 构造 UART 0x01 DataPoint 帧发往加热模块
 */
void lunchbox_heat_start(u8 mode, u8 temp, u32 duration)
{
    lb_keep_warm_active = (mode == LB_KEEP_WARM_MODE);
    lb_heat_lcd_active = true;
    lb_heat_task_active = true;
    u32 ts = lb_get_unix_time();
    u8 data[64];
    u8 *p = data;

    p += lb_dp_encode_enum(p, LB_DPID_HEAT_MODE, mode);
    p += lb_dp_encode_value(p, LB_DPID_HEAT_DURATION, duration);
    p += lb_dp_encode_enum(p, LB_DPID_HEAT_TEMP, temp);
    p += lb_dp_encode_bool(p, LB_DPID_HEAT_ENABLE, 1);
    p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, ts);
    p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);

    u16 data_len = (u16)(p - data);
    lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, data_len);

#if ELUNCHBOX_PANEL_EN
    if (!elunchbox_pwr_is_manual_off()
        && (elunchbox_pwr_gui_off_is_on() || sys_cb.gui_sleep_sta)) {
        elunchbox_pwr_gui_wake_reason("heat_start");
    }
    if (!elunchbox_pwr_is_manual_off()) {
        elunchbox_user_activity_reset();
    }
    func_key_lock_on_heating_start();
#endif

#if !LB_BRIDGE_MODE
    lb_attr_heat_mode     = mode;
    lb_attr_heat_temp     = temp;
    lb_attr_heat_duration = duration;
    lb_attr_heat_enable   = 1;
    lunchbox_report_all_attrs();
#endif
}

/**
 * @brief LCD 停止加热 — 构造 UART 0x01 DataPoint 帧发往加热模块
 */
void lunchbox_heat_stop(void)
{
#if ELUNCHBOX_PANEL_EN
    func_key_lock_on_heating_stop();
#endif
    lb_keep_warm_active = false;
    lb_heat_lcd_active = false;
    lb_heat_task_active = false;
    u32 ts = lb_get_unix_time();
    u8 data[32];
    u8 *p = data;

    p += lb_dp_encode_bool(p, LB_DPID_HEAT_ENABLE, 0);
    p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, ts);
    p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);

    u16 data_len = (u16)(p - data);
    lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, data_len);

#if !LB_BRIDGE_MODE
    lb_attr_heat_enable = 0;
    lb_attr_heat_mode   = 0;
    lunchbox_report_all_attrs();
#endif
}

//-----------------------------------------------------------------------------
// 保温控制
//-----------------------------------------------------------------------------

void lunchbox_keep_warm_set_temp_idx(u8 temp_idx)
{
    lb_warm_temp_idx = (temp_idx <= 6) ? temp_idx : 0xff;
}

#if ELUNCHBOX_PANEL_EN
static void lunchbox_warm_mark_active(void)
{
    lb_keep_warm_active = true;
    lb_heat_lcd_active = true;
    lb_heat_task_active = true;
    if (!elunchbox_pwr_is_manual_off()) {
        elunchbox_user_activity_reset();
    }
    func_key_lock_on_heating_start();
}
#endif

void lunchbox_keep_warm_apply(void)
{
    u8 temp_idx;

    if (lb_warm_temp_idx <= 6) {
        temp_idx = lb_warm_temp_idx;
        lb_warm_temp_idx = 0xff;
    } else {
        temp_idx = lunchbox_temp_f_to_idx(LB_KEEP_WARM_TEMP_F);
    }

#if LB_BRIDGE_MODE
    if (lb_heat_uart_remote_consume()) {
#if ELUNCHBOX_PANEL_EN
        lunchbox_warm_mark_active();
#endif
        printf("keep_warm_apply: bridge skip UART temp_idx=%u\n", temp_idx);
        return;
    }
#endif
    lunchbox_heat_start(LB_KEEP_WARM_MODE, temp_idx, 0);
}

void lunchbox_keep_warm_start(void)
{
    if (lb_keep_warm_active) {
        return;
    }
    lunchbox_keep_warm_apply();
}

void lunchbox_keep_warm_stop(void)
{
    if (!lb_keep_warm_active) return;
    lunchbox_heat_stop();
}

bool lunchbox_keep_warm_is_active(void)
{
    return lb_keep_warm_active;
}

void lunchbox_keep_warm_poll(void)
{
    if (!lb_keep_warm_active) return;
    if (bsp_vbat_get_lpwr_status()) {
        lunchbox_keep_warm_stop();
    }
}

//-----------------------------------------------------------------------------
// 按键/开关机/时间同步
//-----------------------------------------------------------------------------

void lunchbox_key_notify(u8 key_val)
{
    u8 data[8];
    u16 len = lb_dp_encode_enum(data, LB_DPID_KEY_NOTIFY, key_val);
    lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, len);
}

void lunchbox_power_off(void)
{
    u8 data[8];
    u16 len = lb_dp_encode_bool(data, LB_DPID_POWER_SWITCH, 0);
    lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, len);
    led_pg_off();   // 关背光 (先于关屏，避免花屏)
    lcd_pg_off();   // 关VDDLCD
    hr_vdd_ldo_off(); // 关VDDHR 3.3V
}

void lunchbox_power_on(void)
{
    hr_vdd_ldo_on(); // 开VDDHR 3.3V
    lcd_pg_on();    // 开VDDLCD (必须先于背光，否则花屏)

    u8 data[16];
    u8 *p = data;

    p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);
    if (ble_is_connected()) {
        u32 ts = lb_get_unix_time();
        p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, ts);
    }

    u16 len = (u16)(p - data);
    lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, len);
    led_pg_on();    // 开背光 (UART发包约1~2ms，给LCD供电留出稳定时间)
}

void lunchbox_time_sync(u32 unix_time)
{
    u8 data[16];
    u8 *p = data;

    p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, unix_time);
    p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);

    u16 len = (u16)(p - data);
    lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, len);
}

//-----------------------------------------------------------------------------
// 预约管理
//-----------------------------------------------------------------------------

void lunchbox_reservation_send(u8 action, u8 id, const char *name, u32 unix_time,
                               u8 temp, u8 duration, u8 enabled, u8 repeat)
{
    u8 data[42];
    memset(data, 0, 42);

    data[0] = action;
    data[1] = id;
    if (name) {
        u8 i;
        for (i = 0; i < 32 && name[i]; i++) {
            data[2 + i] = (u8)name[i];
        }
    }
    data[2 + 32 + 0] = (u8)(unix_time >> 24);
    data[2 + 32 + 1] = (u8)(unix_time >> 16);
    data[2 + 32 + 2] = (u8)(unix_time >> 8);
    data[2 + 32 + 3] = (u8)(unix_time & 0xFF);
    data[2 + 32 + 4] = temp;
    data[2 + 32 + 5] = duration;
    data[2 + 32 + 6] = enabled;
    data[2 + 32 + 7] = repeat;

    lb_uart_send_raw(LB_UART_CMD_SCHEDULE_OP, data, 42);
}

void lunchbox_reservation_delete(u8 id)
{
    u8 data[2];
    data[0] = 0x00;
    data[1] = id;
    lb_uart_send_raw(LB_UART_CMD_SCHEDULE_OP, data, 2);
}

//-----------------------------------------------------------------------------
// BLE 连接回调
//-----------------------------------------------------------------------------

#if 0
/**
 * @brief 计算下一个指定时分(北京时间)的Unix时间戳
 */
static u32 lb_next_time_of_day(u8 hour, u8 min)
{
    u32 now_unix = lb_get_unix_time();
    u32 today_midnight = now_unix - (now_unix % 86400);
    u32 target_unix = today_midnight + (u32)hour * 3600 + (u32)min * 60;
    if (target_unix <= now_unix) {
        target_unix += 86400;
    }
    return target_unix;
}
#endif

/**
 * @brief BLE 连接后发送5个固定预约预设到加热模块 (UART 0x03)
 *
 * 桥模式: 通过 UART 0x03 发往加热模块 (ID 1~5 固定不可改)。
 * 本地模式: 预设已在 lb_local_init_presets() 中初始化到 lb_schedules[]，
 *           此处只需向 APP 上报当前预约列表即可。
 */
void lunchbox_ble_send_presets(void)
{
#if LB_BRIDGE_MODE
    // 守护: 未收到 APP 权威时间戳前不发预设, 避免时间偏差
    if (!lb_has_ble_ts) {
        //printf("BLE: presets skipped, no synced timestamp yet\n");
        lb_ble_presets_pending = true;
        return;
    }

    // u8 temp_idx = lunchbox_temp_f_to_idx(149);

    // lunchbox_reservation_send(1, 1, "\xe6\x97\xa9\xe9\xa4\x90",
    //                           lb_next_time_of_day(8, 0), temp_idx, 60, 0, 0xff);
    // lunchbox_reservation_send(1, 2, "\xe5\x8d\x88\xe9\xa4\x90",
    //                           lb_next_time_of_day(10, 50), temp_idx, 70, 0, 0xff);
    // lunchbox_reservation_send(1, 3, "\xe6\x99\x9a\xe9\xa4\x90",
    //                           lb_next_time_of_day(16, 30), temp_idx, 90, 0, 0xff);
    // lunchbox_reservation_send(2, 4, "\xe9\xb8\xa1\xe8\x85\xbf\xe6\xa8\xa1\xe5\xbc\x8f",
    //                           lb_get_unix_time(), temp_idx, 60, 0, 0xff);
    // lunchbox_reservation_send(3, 5, "\xe6\x84\x8f\xe9\x9d\xa2\xe6\xa8\xa1\xe5\xbc\x8f",
    //                           lb_get_unix_time(), temp_idx, 60, 0, 0xff);

    //printf("BLE connected: 5 presets sent to heat module via UART 0x03\n");
#else
    // 本地模式: 预设已在 lunchbox_uart_init() → lb_local_init_presets() 中初始化
    // 后续用户新增预约 ID 从 6 开始 (lb_next_schedule_id = 6)
    printf("BLE connected: local mode, presets already initialized (ID 1~5)\n");
#endif
}

/**
 * @brief BLE 连接成功回调 — 主动上报时间戳(0x03, dpid=11)给 APP
 */
void lunchbox_ble_on_connected(void)
{
    u32 ts = lb_get_unix_time();
    u8 data[8];
    u16 len = lb_dp_encode_value(data, LB_DPID_TIME_SYNC, ts);
    lunchbox_uart_send_async(LB_CMD_STATUS_REPORT, data, len);
    printf("BLE connected: report timestamp=%lu via 0x03\n", (unsigned long)ts);

    // 标记等待 APP 应答时间戳后再发送预设，不立即发送
    lb_ble_presets_pending = true;
}

#if ELUNCHBOX_PANEL_EN
typedef struct {
    bool got_mode;
    bool got_duration;
    bool got_temp;
    bool got_enable;
    u8   mode;
    u8   temp_idx;
    u32  duration_min;
    u8   enable;
} lb_heat_control_dp_t;

/** @brief 最近一次 0x04 下发的加热模式 (分包时 enable 帧可能不带 mode) */
static u8 lb_ble_cached_heat_mode = 1;

void lunchbox_mode_preset_local_set(u8 mode, u8 temp_idx, u8 duration_min)
{
    if (mode < 1 || mode > 5) {
        return;
    }
    lb_mode_temp[mode]     = temp_idx;
    lb_mode_duration[mode] = duration_min;
    lb_ble_cached_heat_mode = mode;
    printf("mode preset local: id=%u temp=%u dur=%u min\n", mode, temp_idx, duration_min);
}

static void lb_dp_parse_heat_control(const u8 *data, u16 len, lb_heat_control_dp_t *out)
{
    u16 off = 0;

    memset(out, 0, sizeof(*out));
    while (off + 4 <= len) {
        u8  dpid    = data[off];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];

        if (off + 4 + val_len > len) {
            break;
        }
        u8 *val = data + off + 4;

        switch (dpid) {
        case LB_DPID_HEAT_MODE:
            if (val_len >= 1) {
                out->mode = val[0];
                out->got_mode = true;
            }
            break;
        case LB_DPID_HEAT_DURATION:
            if (val_len >= 4) {
                out->duration_min = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                                  | ((u32)val[2] << 8) | val[3];
                out->got_duration = true;
            } else if (val_len >= 1) {
                out->duration_min = val[0];
                out->got_duration = true;
            }
            break;
        case LB_DPID_HEAT_TEMP:
            if (val_len >= 1) {
                out->temp_idx = val[0];
                out->got_temp = true;
            }
            break;
        case LB_DPID_HEAT_ENABLE:
            if (val_len >= 1) {
                out->enable = val[0];
                out->got_enable = true;
            }
            break;
        default:
            break;
        }
        off += 4 + val_len;
    }
}

/** @brief 判断是否应跳转加热/保温页（兼容 APP 未带 DP10 仅下发模式/温度/时长） */
static bool lb_heat_control_wants_panel(const lb_heat_control_dp_t *dp)
{
    /* 保温(5)：下发模式即进入保温页 */
    if (dp->got_mode && dp->mode == 5) {
        return true;
    }
    /* 鸡腿(2)/意面(3)：协议 §3.4 下发模式即开始对应预设加热 */
    if (dp->got_mode && (dp->mode == 2 || dp->mode == 3)) {
        return true;
    }
    if (dp->got_enable) {
        return dp->enable != 0;
    }
    if (dp->got_mode && dp->mode >= 1 && dp->mode <= 3) {
        return true;
    }
    if (dp->got_duration || dp->got_temp) {
        return true;
    }
    return false;
}

/** @brief 跳转 func_heat_panel 并带入预设（鸡腿/意面与 func_mode 一致：靠 proto_mode 自动开加热） */
static void lunchbox_ble_goto_heat_panel(u8 proto_mode, u16 temp_f, u8 hour, u8 min)
{
    lb_mode_to_heat_set(proto_mode, temp_f, hour, min);
    if (proto_mode == 2 || proto_mode == 3) {
        lb_heat_autostart_set(false);
    } else {
        lb_heat_autostart_set(true);
    }
#if LB_BRIDGE_MODE
    lb_heat_uart_remote_set(true);
#endif
    func_elunchbox_switch_to_heat_panel();
}

/** @brief 跳转 func_new_warm 保温页（温度档位由 lunchbox_keep_warm_set_temp_idx 预设） */
static void lunchbox_ble_goto_warm_panel(u8 temp_idx)
{
    lunchbox_keep_warm_set_temp_idx(temp_idx);
#if LB_BRIDGE_MODE
    lb_heat_uart_remote_set(true);
#endif
    func_elunchbox_switch_to_warm_panel();
}

/** @brief 0x04 控制帧：保温模式 → func_new_warm */
void lunchbox_control_apply_warm(const u8 *data, u16 len)
{
    lb_heat_control_dp_t dp;
    u8 proto_mode;
    u8 temp_idx;

    lb_dp_parse_heat_control(data, len, &dp);

    if (dp.got_enable && dp.enable == 0) {
        return;
    }

    if (dp.got_mode && dp.mode >= 1 && dp.mode <= 5) {
        lb_ble_cached_heat_mode = dp.mode;
    }
    proto_mode = dp.got_mode ? dp.mode : lb_ble_cached_heat_mode;
    if (proto_mode != 5) {
        return;
    }

    if (!lb_heat_control_wants_panel(&dp)) {
        return;
    }

    if (dp.got_temp) {
        temp_idx = dp.temp_idx;
    } else {
        temp_idx = lb_mode_temp[5];
    }
    if (temp_idx > 6) {
        temp_idx = lunchbox_temp_f_to_idx(LB_KEEP_WARM_TEMP_F);
    }

    printf("BLE warm: temp_idx=%u (%uF) -> warm panel\n",
           temp_idx, lunchbox_temp_idx_to_f(temp_idx));

    lunchbox_ble_goto_warm_panel(temp_idx);
}

/** @brief 0x04 控制帧含总开关时，驱动面板关机/开机（桥模式与本地模式共用） */
void lunchbox_control_apply_power_switch(const u8 *data, u16 len)
{
    u8 sw;

    if (!lb_dp_scan_bool(data, len, LB_DPID_POWER_SWITCH, &sw)) {
        return;
    }
    elunchbox_pwr_ble_switch(sw != 0);
}

/** @brief 0x04 控制帧含「是否加热=1」时，跳转加热页并同步 UI 参数 */
void lunchbox_control_apply_heat(const u8 *data, u16 len)
{
    lb_heat_control_dp_t dp;
    u8   proto_mode;
    u16  temp_f;
    u32  duration_min;
    u8   hour;
    u8   min;

    lb_dp_parse_heat_control(data, len, &dp);

    printf("BLE heat ctrl: en=%u got_en=%u mode=%u got_mode=%u dur=%lu got_dur=%u temp=%u got_temp=%u\n",
           dp.enable, dp.got_enable ? 1u : 0u,
           dp.mode, dp.got_mode ? 1u : 0u,
           (unsigned long)dp.duration_min, dp.got_duration ? 1u : 0u,
           dp.temp_idx, dp.got_temp ? 1u : 0u);

    if (!lb_heat_control_wants_panel(&dp)) {
        return;
    }

    if (dp.got_enable && dp.enable == 0) {
        return;
    }

    if (dp.got_mode && dp.mode >= 1 && dp.mode <= 5) {
        lb_ble_cached_heat_mode = dp.mode;
    }
    proto_mode = dp.got_mode ? dp.mode : lb_ble_cached_heat_mode;
    if (proto_mode == 0 || proto_mode == 4) {
        return;
    }
    if (proto_mode == 5) {
        lunchbox_control_apply_warm(data, len);
        return;
    }

    if (dp.got_temp) {
        temp_f = lunchbox_temp_idx_to_f(dp.temp_idx);
    } else if (proto_mode <= 5) {
        temp_f = lunchbox_temp_idx_to_f(lb_mode_temp[proto_mode]);
    } else {
        temp_f = lunchbox_temp_idx_to_f(4);
    }

    if (dp.got_duration) {
        duration_min = dp.duration_min;
    } else if (proto_mode <= 5 && lb_mode_duration[proto_mode] > 0) {
        duration_min = lb_mode_duration[proto_mode];
    } else {
        duration_min = 30;
    }
    if (duration_min < 1) {
        duration_min = 1;
    } else if (duration_min > 5999) {
        duration_min = 5999;
    }

    hour = (u8)(duration_min / 60);
    min  = (u8)(duration_min % 60);

    if (proto_mode == 2) {
        printf("BLE heat: chicken mode temp_f=%u dur=%lu -> heat panel\n",
               temp_f, (unsigned long)duration_min);
    } else if (proto_mode == 3) {
        printf("BLE heat: pasta mode temp_f=%u dur=%lu -> heat panel\n",
               temp_f, (unsigned long)duration_min);
    } else {
        printf("BLE heat: mode=%u temp_f=%u dur=%lu -> heat panel\n",
               proto_mode, temp_f, (unsigned long)duration_min);
    }

    lunchbox_ble_goto_heat_panel(proto_mode, temp_f, hour, min);
}

/** @brief 0x04 控制帧：面板侧总开关 + 加热页跳转 */
void lunchbox_control_apply_panel(const u8 *data, u16 len)
{
    lunchbox_control_apply_power_switch(data, len);
    lunchbox_control_apply_heat(data, len);
}
#endif

#endif // FUNC_LUNCHBOX_UART_EN
