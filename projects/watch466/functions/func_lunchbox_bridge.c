/**
 * @file    func_lunchbox_bridge.c
 * @brief   饭盒协议翻译层实现 (BLE ↔ UART)
 *
 * 桥模式(LB_BRIDGE_MODE=1): BLE ↔ UART 双向翻译。
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
#include "func_lunchbox_uart.h"
#include "func_lunchbox_uart_internal.h"
#include "func_lunchbox_bridge.h"
#include "heat_display_reg.h"
#if ELUNCHBOX_PANEL_EN
#include "home_ui_shared.h"
#endif

#if FUNC_LUNCHBOX_UART_EN

// 桥模式: 转发加热模块 OTA 时累积 CRC32 (MCU通信协议.md §5.1)
u32  lb_ota_uart_crc32 = 0;
bool lb_ota_uart_crc_active = false;

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
    case LB_CMD_MODE_QUERY:      return LB_UART_CMD_SCHEDULE;    // 0x09 → 0x02
    case LB_CMD_MODE_MODIFY:     return LB_UART_CMD_SCHEDULE_OP; // 0x0a → 0x03
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
static bool lb_translate_ble_data_to_uart(lb_rx_frame_t *rx, u8 *out_data, u16 *out_len)
{
    *out_len = 0;

    switch (rx->cmd) {
    // ─── 0x01 查询产品信息 → UART 0x01: 透传时间戳+使能信号+MCU版本号查询 ───
    case LB_CMD_PRODUCT_INFO: {
        u32 ts = lb_get_unix_time();
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
            // 则自动补充 DP10=0(停止加热), 使 MCU 收到明确的停止加热信号
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
        *out_len = 42;
        return true;
    }

    // ─── 0x07 修改预约 → UART 0x03 ───
    case LB_CMD_SCHEDULE_MODIFY: {
        if (!rx->data || rx->data_len < 41) return false;
        out_data[0] = 0x01;
        memcpy(out_data + 1, rx->data, 41);
        *out_len = 42;
        return true;
    }

    // ─── 0x08 删除预约 → UART 0x03: action=0 ───
    case LB_CMD_SCHEDULE_DELETE: {
        if (!rx->data || rx->data_len < 1) return false;
        out_data[0] = 0x00;  // action=0 → 删除
        out_data[1] = rx->data[0];
        *out_len = 2;
        return true;
    }

    // ─── 0x09 获取指定模式信息 → UART 0x02 ───
    case LB_CMD_MODE_QUERY: {
        if (rx->data && rx->data_len >= 1) {
            out_data[0] = rx->data[0];
            *out_len = 1;
        }
        return true;
    }

    // ─── 0x0a 修改指定模式信息 → UART 0x03: 构造 42B 预约帧 ───
    case LB_CMD_MODE_MODIFY: {
        if (!rx->data || rx->data_len < 3) return false;
        memset(out_data, 0, 42);
        out_data[0] = rx->data[0];       // action = mode
        out_data[1] = 0;                 // id = 0 (模式模板)
        out_data[38] = rx->data[1];      // temp
        out_data[39] = rx->data[2];      // duration
        out_data[40] = 0x01;             // enabled = 1
        out_data[41] = 0xff;             // repeat = 0xff
        *out_len = 42;
        return true;
    }

    // ─── OTA 命令 → UART 0x04 ───
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
 * @brief BLE帧 → UART帧 (完整帧, 含帧头+校验)
 */
bool lb_translate_ble_to_uart(lb_rx_frame_t *rx, u8 *out_buf, u16 *out_len)
{
    u8 uart_cmd = lb_ble_cmd_to_uart_cmd(rx->cmd);
    if (uart_cmd == 0x00) return false;

    u8 data_buf[LB_TXBUF_SIZE];
    u16 data_len = 0;
    if (!lb_translate_ble_data_to_uart(rx, data_buf, &data_len)) return false;

    // 组 UART 帧
    u16 off = 0;
    out_buf[off++] = (u8)(LB_FRAME_HEADER >> 8);   // 0x55
    out_buf[off++] = (u8)LB_FRAME_HEADER;           // 0xaa
    out_buf[off++] = LB_FRAME_VERSION;
    out_buf[off++] = rx->msg_flag;
    out_buf[off++] = uart_cmd;
    out_buf[off++] = LB_ERR_SUCCESS;
    out_buf[off++] = (u8)(data_len >> 8);
    out_buf[off++] = (u8)(data_len & 0xFF);
    if (data_len > 0) {
        memcpy(out_buf + off, data_buf, data_len);
        off += data_len;
    }
    out_buf[off] = lb_checksum(out_buf, off);
    *out_len = off + 1;

    lb_uart_sync_pending = true;
    lb_pending_ble_cmd[rx->msg_flag] = rx->cmd;

    return true;
}

/**
 * @brief UART帧数据 → BLE帧数据翻译
 */
static bool lb_translate_uart_data_to_ble(lb_rx_frame_t *rx, u8 ble_cmd, u8 *out_data, u16 *out_len)
{
    *out_len = 0;

    switch (rx->cmd) {
    // ─── UART 0x01 → BLE 0x02/0x03: DataPoints 透传 ───
    case LB_UART_CMD_DYNAMIC: {
        if (rx->data && rx->data_len > 0) {
            // 从 DataPoints 中提取加热模块版本号 (dpid=13)
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

    // ─── UART 0x02 → BLE 0x05/0x09 ───
    case LB_UART_CMD_SCHEDULE: {
        if (!rx->data || rx->data_len < 44) return false;

        if (ble_cmd == LB_CMD_MODE_QUERY) {
            out_data[0] = rx->data[2];   // set_mode → mode
            out_data[1] = rx->data[37];  // temp
            out_data[2] = rx->data[38];  // duration
            *out_len = 3;
            return true;
        }

        out_data[0] = rx->data[0];  // total_count
        out_data[1] = rx->data[1];  // seq
        memcpy(out_data + 2, rx->data + 3, 41);
        out_data[42] |= 0x80;  // repeat bit7=1
        *out_len = 43;
        return true;
    }

    // ─── UART 0x03 → BLE 0x06/0x07/0x08/0x0a ───
    case LB_UART_CMD_SCHEDULE_OP: {
        if (ble_cmd == LB_CMD_MODE_MODIFY) {
            *out_len = 0;
            return true;
        }
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

/**
 * @brief UART帧 → BLE帧 (完整帧, 含帧头+校验)
 */
bool lb_translate_uart_to_ble(lb_rx_frame_t *rx, u8 *out_buf, u16 *out_len)
{
    bool is_async = false;
    u8 ble_cmd = 0;

    // 直接按 msg_flag 查 BLE cmd 映射 (msg_flag 唯一索引, 无需门控)
    ble_cmd = lb_pending_ble_cmd[rx->msg_flag];
    if (ble_cmd != 0) {
        lb_pending_ble_cmd[rx->msg_flag] = 0;
    }

    if (ble_cmd == 0) {
        if (rx->cmd == LB_UART_CMD_DYNAMIC) {
            is_async = true;
            ble_cmd = LB_CMD_STATUS_REPORT;
        } else {
            ble_cmd = lb_uart_cmd_to_ble_cmd(rx->cmd, false);
        }
    }

    if (ble_cmd == 0x00) return false;

    u8 data_buf[LB_TXBUF_SIZE];
    u16 data_len = 0;
    if (!lb_translate_uart_data_to_ble(rx, ble_cmd, data_buf, &data_len)) return false;

    // 组 BLE 帧
    u16 off = 0;
    out_buf[off++] = (u8)(LB_FRAME_HEADER >> 8);
    out_buf[off++] = (u8)LB_FRAME_HEADER;
    out_buf[off++] = LB_FRAME_VERSION;
    out_buf[off++] = is_async ? lb_async_msg_flag++ : rx->msg_flag;
    out_buf[off++] = ble_cmd;
    out_buf[off++] = rx->err_flag;
    out_buf[off++] = (u8)(data_len >> 8);
    out_buf[off++] = (u8)(data_len & 0xFF);
    if (data_len > 0) {
        memcpy(out_buf + off, data_buf, data_len);
        off += data_len;
    }
    out_buf[off] = lb_checksum(out_buf, off);
    *out_len = off + 1;

    printf("UART->BLE[%d]: ", *out_len);
    for (u16 i = 0; i < *out_len; i++) printf("%02X ", out_buf[i]);
    printf("\n");

    return true;
}

#endif // FUNC_LUNCHBOX_UART_EN
