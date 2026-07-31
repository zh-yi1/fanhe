/**
 * @file    lb_bridge.c
 * @brief   饭盒协议翻译层实现 (BLE ↔ UART)
 *
 * BLE ↔ UART 双向翻译。
 *
 * BLE→UART 翻译:
 *   - 命令字映射: 0x01→0x01, 0x02→0x01, 0x04→0x01, 0x05→0x02, 0x06~0x08→0x03, ...
 *   - 数据载荷格式转换 (DataPoints / 预约帧结构转换)
 *   - OTA 透传 (含 CRC32 累积)
 *
 * UART→BLE 翻译:
 *   - 命令字映射: 0x01→0x02/0x03(同步/异步), 0x02→0x05, 0x03→0x06/0x07/0x08, ...
 *   - 数据载荷格式转换 (预约条目 / DataPoints)
 */
#include "include.h"
#include "lb_proto.h"
#include "lb_bridge.h"
#include "lb_uart_app.h"    // lb_dev_info (dpid=13 版本号回填)

#if FUNC_LUNCHBOX_UART_EN

// 桥模式: 转发加热模块 OTA 时累积 CRC32 (MCU通信协议.md §5.1)
static u32  lb_ota_uart_crc32 = 0;
static bool lb_ota_uart_crc_active = false;

/**
 * @brief APP 预约时间违例补丁 (2026-07-31, APP 暂无法改版)
 *
 * 协议 §3.6 预约时间应为 unix 时间戳, 现 APP 发的是"本地天内秒"
 * (如 16:03 → 57780); 模块用 UTC 天内秒比较, 差 8 小时永远不触发。
 * 透传前把 <86400 的值换算成"下一次到点"的 unix 时间戳 (今天该时刻,
 * 已过则明天); >=86400 视为正常 unix 原样放行, APP 改好后补丁自动失效。
 *
 * @param entry42  UART 0x03 的 42B 预约结构体 (time 在偏移 34..37, BE)
 */
static void lb_schedule_time_fixup(u8 *entry42)
{
    u8 *t = entry42 + 34;
    u32 val = ((u32)t[0] << 24) | ((u32)t[1] << 16) | ((u32)t[2] << 8) | t[3];
    if (val >= 86400) {
        return;
    }

    u32 local  = lb_get_unix_time() + 8 * 3600;
    u32 target = local - (local % 86400) + val;   // 今天该时刻(本地)
    if (target <= local) {
        target += 86400;                          // 已过 → 明天
    }
    target -= 8 * 3600;                           // 回到 unix(UTC)

    t[0] = (u8)(target >> 24);
    t[1] = (u8)(target >> 16);
    t[2] = (u8)(target >> 8);
    t[3] = (u8)(target);
    printf("lb_bridge: sched time fixup %u day-sec -> unix %u\n",
           (unsigned)val, (unsigned)target);
}

/**
 * @brief BLE 命令字 → UART 命令字映射
 */
u8 lb_ble_cmd_to_uart_cmd(u8 ble_cmd)
{
    switch (ble_cmd) {
    case LB_CMD_PRODUCT_INFO:    return LB_UART_CMD_DYNAMIC;     // 0x01 → 0x01
    case LB_CMD_DYNAMIC_ATTR:    return LB_UART_CMD_DYNAMIC;     // 0x02 → 0x01
    case LB_CMD_CONTROL:         return LB_UART_CMD_DYNAMIC;     // 0x04 → 0x01
    case LB_CMD_SCHEDULE_LIST:   return LB_UART_CMD_SCHEDULE;    // 0x05 → 0x02
    case LB_CMD_SCHEDULE_ADD:    return LB_UART_CMD_SCHEDULE_OP; // 0x06 → 0x03
    case LB_CMD_SCHEDULE_MODIFY: return LB_UART_CMD_SCHEDULE_OP; // 0x07 → 0x03
    case LB_CMD_SCHEDULE_DELETE: return LB_UART_CMD_SCHEDULE_OP; // 0x08 → 0x03
    case LB_CMD_OTA_START:       return LB_UART_CMD_OTA;         // 0x0c → 0x04
    case LB_CMD_OTA_DATA:        return LB_UART_CMD_OTA;         // 0x0d → 0x04
    case LB_CMD_OTA_END:         return LB_UART_CMD_OTA;         // 0x0e → 0x04
    // 0x09/0x0a 模式查询/修改: 预设是本地数据, 本地应答不转发 (lb_ble_app.c)
    default:                     return 0x00;                    // 不转发
    }
}

/**
 * @brief UART 命令字 → BLE 命令字映射
 */
u8 lb_uart_cmd_to_ble_cmd(u8 uart_cmd, bool is_async)
{
    switch (uart_cmd) {
    case LB_UART_CMD_DYNAMIC:
        return is_async ? LB_CMD_STATUS_REPORT : LB_CMD_DYNAMIC_ATTR;
    case LB_UART_CMD_SCHEDULE:
        return LB_CMD_SCHEDULE_LIST;
    case LB_UART_CMD_SCHEDULE_OP:
        return LB_CMD_SCHEDULE_ADD;
    case LB_UART_CMD_OTA:
        return LB_CMD_OTA_DATA;
    default:
        return 0x00;
    }
}

/**
 * @brief BLE帧数据 → UART帧数据翻译
 */
bool lb_translate_ble_data_to_uart(lb_rx_frame_t *rx, u8 *out_data, u16 *out_len)
{
    *out_len = 0;

    switch (rx->cmd) {
    // ─── 0x01 查询产品信息 → UART 0x01: 透传时间戳+使能信号+MCU版本号查询 ───
    case LB_CMD_PRODUCT_INFO: {
        // 优先用 APP 请求自带的 4B 时间戳 (本机时间在模块确认前不保存, 不可用)
        u32 ts;
        if (rx->data && rx->data_len >= 4) {
            ts = ((u32)rx->data[0] << 24) | ((u32)rx->data[1] << 16)
               | ((u32)rx->data[2] << 8)  | rx->data[3];
        } else {
            ts = lb_get_unix_time();
        }
        u8 *p = out_data;
        p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, ts);
        p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);
        p += lb_dp_encode_value(p, LB_DPID_MCU_VERSION, 0);
        *out_len = p - out_data;
        return true;
    }

    // ─── 0x02 查询动态属性 → UART 0x01 ───
    case LB_CMD_DYNAMIC_ATTR: {
        u32 ts = lb_get_unix_time();
        u8 *p = out_data;
        p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, ts);
        p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);
        *out_len = p - out_data;
        return true;
    }

    // ─── 0x04 控制指令 → UART 0x01: 前拼时间戳 DataPoint ───
    case LB_CMD_CONTROL: {
        if (rx->data && rx->data_len > 0) {
            u32 ts = lb_get_unix_time();
            u8 *p = out_data;
            p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, ts);
            memcpy(p, rx->data, rx->data_len);
            p += rx->data_len;

            // 扫描 BLE 数据: 若 APP 仅下发 DP02=0(模式=关闭) 而未带 DP10(是否加热),
            // 则自动补充 DP10=0(停止加热), 使模块收到明确的停止加热信号
            // 对应蓝牙通讯协议 §3.4 / §4.1.7 — DP10 是控制加热启停的专用属性
            {
                bool has_dp02_off = false;
                bool has_dp10     = false;
                u16 off = 0;
                while (off + 4 <= rx->data_len) {
                    u8  dpid    = rx->data[off];
                    u16 val_len = ((u16)rx->data[off + 2] << 8) | rx->data[off + 3];
                    if (off + 4 + val_len > rx->data_len) break;
                    if (dpid == LB_DPID_HEAT_MODE && val_len >= 1 && rx->data[off + 4] == 0) {
                        has_dp02_off = true;
                    }
                    if (dpid == LB_DPID_HEAT_ENABLE) {
                        has_dp10 = true;
                    }
                    off += 4 + val_len;
                }
                if (has_dp02_off && !has_dp10) {
                    p += lb_dp_encode_bool(p, LB_DPID_HEAT_ENABLE, 0);
                }
            }

            *out_len = p - out_data;
            return true;
        }
        return false;
    }

    // ─── 0x05 查询预约列表 → UART 0x02: 无数据 ───
    case LB_CMD_SCHEDULE_LIST:
        *out_len = 0;
        return true;

    // ─── 0x06 新增预约 → UART 0x03: 插入 action 字节 ───
    case LB_CMD_SCHEDULE_ADD: {
        if (!rx->data || rx->data_len < 41) return false;
        out_data[0] = 0x01;  // 默认: 自定义加热
        memcpy(out_data + 1, rx->data, 41);
        lb_schedule_time_fixup(out_data);
        *out_len = 42;
        return true;
    }

    // ─── 0x07 修改预约 → UART 0x03 ───
    case LB_CMD_SCHEDULE_MODIFY: {
        if (!rx->data || rx->data_len < 41) return false;
        out_data[0] = 0x01;
        memcpy(out_data + 1, rx->data, 41);
        lb_schedule_time_fixup(out_data);
        *out_len = 42;
        return true;
    }

    // ─── 0x08 删除预约 → UART 0x03: 预约mode=0 表示删除 ───
    case LB_CMD_SCHEDULE_DELETE: {
        if (!rx->data || rx->data_len < 1) return false;
        // MCU 协议 §3.6 要求完整 42 字节结构体, 只发 [mode,id] 模块回 err=01。
        // 除 mode/ID 外全部填 0, 模块按 mode=0 删除对应 ID。
        memset(out_data, 0, 42);
        out_data[0] = 0x00;         // 预约mode: 0=删除该预约
        out_data[1] = rx->data[0];  // 要删除的预约 ID
        *out_len = 42;
        return true;
    }

    // 0x09/0x0a 模式查询/修改: 本地应答不转发, 无需翻译 (lb_ble_app.c)

    // ─── OTA 命令 → UART 0x04 (加热模块 OTA 透传) ───
    case LB_CMD_OTA_START: {
        if (!rx->data || rx->data_len < 5) return false;
        memset(out_data, 0xFF, 4);
        *out_len = 4;
        lb_ota_uart_crc32 = 0xffffffff;
        lb_ota_uart_crc_active = true;
        return true;
    }
    case LB_CMD_OTA_DATA: {
        if (!rx->data || rx->data_len < 5) return false;
        u16 copy_len = rx->data_len - 1;
        memcpy(out_data, rx->data + 1, copy_len);
        *out_len = copy_len;
        if (lb_ota_uart_crc_active) {
            lb_ota_uart_crc32 = lb_crc32(out_data + 4, copy_len - 4, lb_ota_uart_crc32);
        }
        return true;
    }
    case LB_CMD_OTA_END: {
        if (!rx->data || rx->data_len < 1) return false;
        memset(out_data, 0xFF, 4);
        u32 final_crc = lb_ota_uart_crc_active ? (lb_ota_uart_crc32 ^ 0xFFFFFFFF) : 0;
        out_data[4] = (u8)(final_crc >> 24);
        out_data[5] = (u8)(final_crc >> 16);
        out_data[6] = (u8)(final_crc >> 8);
        out_data[7] = (u8)(final_crc);
        *out_len = 8;
        lb_ota_uart_crc_active = false;
        return true;
    }

    default:
        return false;
    }
}

/**
 * @brief UART帧数据 → BLE帧数据翻译
 */
bool lb_translate_uart_data_to_ble(lb_rx_frame_t *rx, u8 ble_cmd, u8 *out_data, u16 *out_len)
{
    (void)ble_cmd;
    *out_len = 0;

    switch (rx->cmd) {
    // ─── UART 0x01 → BLE 0x02/0x03: DataPoints 透传 ───
    case LB_UART_CMD_DYNAMIC: {
        if (rx->data && rx->data_len > 0) {
            // 从 DataPoints 中提取加热模块版本号 (dpid=13) 回填设备信息
            u16 off = 0;
            while (off + 4 <= rx->data_len) {
                u8  dpid    = rx->data[off];
                u16 val_len = ((u16)rx->data[off + 2] << 8) | rx->data[off + 3];
                if (off + 4 + val_len > rx->data_len) break;
                if (dpid == LB_DPID_MCU_VERSION && val_len >= 4) {
                    u8 *v = rx->data + off + 4;
                    lb_dev_info.heat_module_version = ((u32)v[0] << 24) | ((u32)v[1] << 16)
                                                    | ((u32)v[2] << 8)  | v[3];
                }
                off += 4 + val_len;
            }
            memcpy(out_data, rx->data, rx->data_len);
            *out_len = rx->data_len;
            return true;
        }
        return false;
    }

    // ─── UART 0x02 → BLE 0x05: 44B 条目 → 43B 条目 ───
    case LB_UART_CMD_SCHEDULE: {
        if (!rx->data || rx->data_len < 44) return false;

        out_data[0] = rx->data[0];  // total_count
        out_data[1] = rx->data[1];  // seq
        memcpy(out_data + 2, rx->data + 3, 41);
        out_data[42] |= 0x80;  // repeat bit7=1
        *out_len = 43;
        return true;
    }

    // ─── UART 0x03 → BLE 0x06/0x07/0x08 ───
    case LB_UART_CMD_SCHEDULE_OP: {
        if (rx->data && rx->data_len >= 1) {
            out_data[0] = rx->data[0];
            *out_len = 1;
            return true;
        }
        *out_len = 0;
        return true;
    }

    // ─── UART 0x04 → BLE OTA 应答 ───
    case LB_UART_CMD_OTA: {
        if (rx->data && rx->data_len > 0) {
            memcpy(out_data, rx->data, rx->data_len);
            *out_len = rx->data_len;
        }
        return true;
    }

    default:
        return false;
    }
}

//-----------------------------------------------------------------------------
// 时间服务 (原 func_lunchbox_ui_state.c 的 lb_time_*, 随翻译层迁入)
//-----------------------------------------------------------------------------

typedef struct {
    bool synced;           // 是否收到过 APP 权威时间
    u32  unix_ts;          // 权威 Unix 时间戳
    u32  rtccnt;           // 同步时刻的 RTCCNT (推算当前时间用)
} lb_time_t;

static lb_time_t lb_time;

void lb_time_set_synced(u32 unix_ts)
{
    lb_time.synced  = true;
    lb_time.unix_ts = unix_ts;
    lb_time.rtccnt  = RTCCNT;
    printf("lb_time: synced ts=%lu\n", (unsigned long)unix_ts);
}

bool lb_time_is_synced(void)
{
    return lb_time.synced;
}

u32 lb_get_unix_time(void)
{
    if (lb_time.synced) {
        return lb_time.unix_ts + (RTCCNT - lb_time.rtccnt);
    }
    return RTCCNT + LB_RTC_UNIX_OFFSET;
}

tm_t lb_get_display_tm(void)
{
    // 已同步: 按权威时间推算; 未同步: 退回本机 RTC (开机到首次同步之间)
    if (lb_time.synced) {
        return time_to_tm(lb_get_unix_time() - LB_RTC_UNIX_OFFSET);
    }
    return rtc_clock_get();
}

//-----------------------------------------------------------------------------
// CRC32 (原 func_lunchbox_ota.c, 随 OTA 透传迁入)
//-----------------------------------------------------------------------------

u32 lb_crc32(const void *data, u32 len, u32 crc)
{
    static const u32 crc32_table[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
    };
    const u8 *buf = (const u8 *)data;
    u32 i;
    for (i = 0; i < len; ++i) {
        crc ^= buf[i];
        crc = crc32_table[crc & 0x0f] ^ (crc >> 4);
        crc = crc32_table[crc & 0x0f] ^ (crc >> 4);
    }
    return crc;
}

#endif // FUNC_LUNCHBOX_UART_EN
