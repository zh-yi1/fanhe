/**
 * @file    func_lunchbox_ble.c
 * @brief   饭盒 BLE 通道实现 (蓝牙接收/发送)
 *
 * BLE 数据入口: APP 通过 BLE GATT 写入饭盒协议帧。
 *
 * 时间同步流程 (BLE 连接后):
 *   1. MCU → APP: 0x03 异步上报本地时间戳 (dpid=11)
 *   2. APP → MCU: 0x03 回传权威时间戳 (dpid=11) → lb_ble_handle_app_time_sync()
 *      或 APP → MCU: 0x01 产品信息查询 (数据区带 4B 时间戳) → lb_handler_product_info()
 *   3. MCU 收到 APP 时间后 → 下发 5 个固定预设 (ID 1~5) 到加热模块
 *
 * 桥模式(LB_BRIDGE_MODE=1):
 *   - 翻译 BLE 帧为 UART 帧 → 转发加热模块
 *   - 0x01 产品信息 → MCU 本地回复 + 透传加热模块
 *   - 0x03 时间同步 → MCU 本地处理 (不转发 UART)
 *   - OTA 命令按 target 字段分流
 *
 * 本地模式(LB_BRIDGE_MODE=0):
 *   - 原帧透传到 UART 串口 + 解析分发给 cmd_handler
 *   - 0x03 时间同步 → MCU 本地处理 (不转发 UART)
 */
#include "include.h"
#include "func_lunchbox_uart.h"
#include "func_lunchbox_uart_internal.h"
#include "func_lunchbox_ble.h"
#include "func_lunchbox_bridge.h"
#include "func_lunchbox_uart_heat.h"

#include "heat_display_reg.h"
#if ELUNCHBOX_PANEL_EN
#include "home_ui_shared.h"
#include "func_lunchbox_lcd.h"
#endif

#if FUNC_LUNCHBOX_UART_EN

// BLE 调试打印 (与 func_lunchbox_uart.c 中 LB_DEBUG 独立)
#if 0
#define LB_TRACE(...)       printf(__VA_ARGS__)
#else
#define LB_TRACE(...)
#endif

// BLE 接收累积缓冲区 (支持跨 notification 帧拼接)
static u8  ble_rx_buf[512];
static u16 ble_rx_idx;
static u32 ble_rx_ticks;

/**
 * @brief 注册 BLE 发送函数 — 启用蓝牙通道
 */
void lunchbox_ble_set_tx_fn(lb_ble_tx_fn_t fn)
{
    lb_ble_tx_fn = fn;
}

/**
 * @brief BLE 帧解析 — 校验并拆解收到的饭盒协议帧
 */
static bool lb_ble_frame_parse(u8 *raw, u16 raw_len, lb_rx_frame_t *frame)
{
    if (raw_len < 9) return false;
    if (raw[0] != 0x55 || raw[1] != 0xAA) return false;

    u16 data_len = ((u16)raw[6] << 8) | raw[7];
    if (raw_len < 9 + data_len) {
        printf("BLE: len err raw=%d data=%d expected=%d\n", raw_len, data_len, 9 + data_len);
        return false;
    }

    u16 frame_len = 9 + data_len;
    u8 checksum = lb_checksum(raw, frame_len - 1);
    if (checksum != raw[frame_len - 1]) return false;

    frame->version  = raw[2];
    frame->msg_flag = raw[3];
    frame->cmd      = raw[4];
    frame->err_flag = raw[5];
    frame->data_len = data_len;
    frame->data     = data_len > 0 ? &raw[8] : NULL;
    frame->valid    = true;
    return true;
}

/**
 * @brief 处理 APP 通过 0x03 回传的时间戳同步 (DataPoint dpid=11)
 *
 * 解析 0x03 帧中的 DataPoint 数据，提取 TIME_SYNC(11) 的 4B Unix 时间戳，
 * 更新本地同步基准，并在等待标志置位时触发预设下发。
 *
 * @param rx  已解析的接收帧
 * @return true=找到时间戳并已处理, false=未找到
 */
static bool lb_ble_handle_app_time_sync(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 8) return false;

    u16 off = 0;
    while (off + 4 <= rx->data_len) {
        u8  dpid    = rx->data[off];
        u8  type    = rx->data[off + 1];
        u16 val_len = ((u16)rx->data[off + 2] << 8) | rx->data[off + 3];
        if (off + 4 + val_len > rx->data_len) break;

        if (dpid == LB_DPID_TIME_SYNC && type == LB_DP_TYPE_VALUE && val_len >= 4) {
            u8 *val = rx->data + off + 4;
            lb_synced_unix_ts = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                              | ((u32)val[2] << 8)  | val[3];
            lb_synced_rtccnt  = RTCCNT;
            lb_has_ble_ts     = true;
            printf("BLE: APP time sync via 0x03, ts=%lu\n", (unsigned long)lb_synced_unix_ts);

            // BLE 连接后首次收到 APP 时间戳应答 → 发送5个预设到加热模块
            if (lb_ble_presets_pending) {
                lb_ble_presets_pending = false;
                lunchbox_ble_send_presets();
            }
            return true;
        }
        off += 4 + val_len;
    }
    return false;
}

/**
 * @brief BLE 饭盒帧入口
 */
void lunchbox_ble_rx_handle(u8 *data, u16 len)
{
    // ──── BLE 收包日志 ────
    printf("BLE==>RX [%d]: ", len);
    for (u16 i = 0; i < len; i++) printf("%02X ", data[i]);
    printf("\n");
    if (len >= 9) {
        u8  cmd = data[4];
        u16 dl  = ((u16)data[6] << 8) | data[7];
        if (dl && len >= 9 + dl) lb_ble_dump_frame(cmd, data + 8, dl, true);
        else if (dl == 0)        lb_ble_dump_frame(cmd, NULL, 0, true);
    }

    // ──── 追加到 BLE 累积缓冲区 ────
    if (ble_rx_idx > 0 && tick_check_expire(ble_rx_ticks, 3000)) {
        printf("BLE: rx buf timeout, reset\n");
        ble_rx_idx = 0;
    }

    if (ble_rx_idx + len > sizeof(ble_rx_buf)) {
        printf("BLE: rx buf overflow, reset\n");
        ble_rx_idx = 0;
        return;
    }
    memcpy(ble_rx_buf + ble_rx_idx, data, len);
    ble_rx_idx += len;
    ble_rx_ticks = tick_get();

    // ──── 循环解析所有完整帧 ────
    while (ble_rx_idx >= 9) {
        if (ble_rx_buf[0] != 0x55 || ble_rx_buf[1] != 0xAA) {
            if (ble_rx_idx > 1) {
                memmove(ble_rx_buf, ble_rx_buf + 1, ble_rx_idx - 1);
                ble_rx_idx--;
            } else {
                ble_rx_idx = 0;
            }
            continue;
        }

        u16 data_len = ((u16)ble_rx_buf[6] << 8) | ble_rx_buf[7];
        u16 frame_total = 9 + data_len;

        if (frame_total > sizeof(ble_rx_buf)) {
            printf("BLE: frame too large dlen=%d, skip\n", data_len);
            memmove(ble_rx_buf, ble_rx_buf + 1, ble_rx_idx - 1);
            ble_rx_idx--;
            continue;
        }

        if (ble_rx_idx < frame_total) {
            break;
        }

        lb_rx_frame_t frame;
        memset(&frame, 0, sizeof(frame));
        if (!lb_ble_frame_parse(ble_rx_buf, ble_rx_idx, &frame)) {
            printf("BLE: frame parse fail\n");
            memmove(ble_rx_buf, ble_rx_buf + 1, ble_rx_idx - 1);
            ble_rx_idx--;
            continue;
        }

#if LB_BRIDGE_MODE
        // 0x01 产品信息 → 先透传加热模块, 等UART应答后再回复APP
        if (frame.cmd == LB_CMD_PRODUCT_INFO) {
            if (frame.data && frame.data_len >= 4) {
                lb_synced_unix_ts = ((u32)frame.data[0] << 24) | ((u32)frame.data[1] << 16)
                                  | ((u32)frame.data[2] << 8)  | frame.data[3];
                lb_synced_rtccnt  = RTCCNT;
                lb_has_ble_ts     = true;
            }

            // BLE 连接后首次收到 APP 时间戳应答 → 发送5个预设到加热模块
            if (lb_ble_presets_pending) {
                lb_ble_presets_pending = false;
                lunchbox_ble_send_presets();
            }

            lb_product_info_pending   = true;
            lb_product_info_msg_flag  = frame.msg_flag;
            lb_product_info_pend_tick = tick_get();
            u8 uart_buf[LB_TXBUF_SIZE];
            u16 uart_len = 0;
            if (lb_translate_ble_to_uart(&frame, uart_buf, &uart_len)) {
                printf("BLE->UART==>TX[%d]: ", uart_len);
                for (u16 i = 0; i < uart_len; i++) printf("%02X ", uart_buf[i]);
                printf("\n");
                {
                    u16 dl = ((u16)uart_buf[6] << 8) | uart_buf[7];
                    if (dl) lb_ble_dump_frame(frame.cmd, uart_buf + 8, dl, true);
                }
                uart_bufs_tx(UART_TYPE_1, uart_buf, uart_len);
            } else {
                lb_product_info_pending = false;
                lb_handler_product_info(&frame);
            }
            goto next_frame;
        }

        if (frame.cmd == LB_CMD_STATUS_REPORT) {
            // APP 通过 0x03 回传时间戳 → 解析并同步, 触发预设下发
            lb_ble_handle_app_time_sync(&frame);
            goto next_frame;
        }

        // OTA 命令: 根据 target 字段决定路由
        if (frame.cmd >= LB_CMD_OTA_START && frame.cmd <= LB_CMD_OTA_END) {
            u8 target = lb_ota_get_target(&frame);
            bool to_main = (target == 0x00 || target == LB_OTA_TARGET_MAIN_MCU);
            bool to_heat = (target == LB_OTA_TARGET_HEAT_MODULE);

            if (to_main) {
                printf("OTA: target=0x%02X -> local handler\n", target ? target : LB_OTA_TARGET_MAIN_MCU);
                if (frame.cmd < 16 && cmd_handler[frame.cmd]) {
                    cmd_handler[frame.cmd](&frame);
                }
            }

            if (to_heat) {
                // 加热模块 OTA: 存储到 SPI Flash → 校验CRC → UART发送
                printf("OTA: target=0x%02X -> heat module OTA handler\n", target);
                switch (frame.cmd) {
                case LB_CMD_OTA_START:
                    heat_ota_handler_start(&frame, frame.msg_flag);
                    break;
                case LB_CMD_OTA_DATA:
                    heat_ota_handler_data(&frame, frame.msg_flag);
                    break;
                case LB_CMD_OTA_END:
                    heat_ota_handler_end(&frame, frame.msg_flag);
                    break;
                default:
                    break;
                }
            }
            goto next_frame;
        }

        // 所有其他命令 → 翻译为 UART 协议
        {
            u8 uart_buf[LB_TXBUF_SIZE];
            u16 uart_len = 0;

#if ELUNCHBOX_PANEL_EN
            /* 0x0a 修改模式预设：桥模式也更新本地 lb_mode_*，供后续 0x04 跳转加热页使用 */
            if (frame.cmd == LB_CMD_MODE_MODIFY && frame.data && frame.data_len >= 3) {
                lunchbox_mode_preset_local_set(frame.data[0], frame.data[1], frame.data[2]);
            }
#endif

            if (lb_translate_ble_to_uart(&frame, uart_buf, &uart_len)) {
                printf("BLE->UART==>TX[%d]: ", uart_len);
                for (u16 i = 0; i < uart_len; i++) printf("%02X ", uart_buf[i]);
                printf("\n");
                {
                    u16 dl = ((u16)uart_buf[6] << 8) | uart_buf[7];
                    if (dl) lb_ble_dump_frame(frame.cmd, uart_buf + 8, dl, true);
                }
                uart_bufs_tx(UART_TYPE_1, uart_buf, uart_len);
            }

            if (frame.cmd == LB_CMD_CONTROL && frame.data && frame.data_len > 0) {
                heat_display_feed_dp(frame.data, frame.data_len);
#if ELUNCHBOX_PANEL_EN
                home_ui_shared_battery_feed_dp(frame.data, frame.data_len);
                lunchbox_control_apply_panel(frame.data, frame.data_len);
#endif
            }
        }
#else
        // ──── 本地模式：0x03 时间同步 → 本地处理, 不转发到 UART ────
        if (frame.cmd == LB_CMD_STATUS_REPORT) {
            lb_ble_handle_app_time_sync(&frame);
            goto next_frame;
        }

        // ──── 本地模式：逐帧转发到串口 + 解析分发给 cmd_handler ────
        printf("BLE->UART==>TX[%d]: ", frame_total);
        for (u16 i = 0; i < frame_total; i++) printf("%02X ", ble_rx_buf[i]);
        printf("\n");
        {
            if (frame.data_len) lb_dp_dump_hex(ble_rx_buf + 8, frame.data_len);
        }
        uart_bufs_tx(UART_TYPE_1, ble_rx_buf, frame_total);

        LB_TRACE("lb_ble: rx cmd=0x%02x msg=%d len=%d\n", frame.cmd, frame.msg_flag, frame.data_len);

        if (frame.cmd < 16 && cmd_handler[frame.cmd]) {
            cmd_handler[frame.cmd](&frame);
        }
        goto next_frame;
#endif

next_frame:
        {
            u16 remaining = ble_rx_idx - frame_total;
            if (remaining > 0) {
                memmove(ble_rx_buf, ble_rx_buf + frame_total, remaining);
                ble_rx_idx = remaining;
            } else {
                ble_rx_idx = 0;
            }
        }
    }
}

/**
 * @brief 检查 BLE 累积缓冲区是否有待处理数据
 */
bool lunchbox_ble_rx_pending(void)
{
    return ble_rx_idx > 0;
}

#endif // FUNC_LUNCHBOX_UART_EN
