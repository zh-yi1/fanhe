/**
 * @file    func_lunchbox_uart.c
 * @brief   智能盒饭 - BLE+UART双协议实现 (大端, 同步/异步双模式)
 * @note    BLE侧: 蓝牙通讯协议1.0.7.md (APP ↔ MCU)
 *          UART侧: MCU通信协议.md v1.0.7 (MCU ↔ 加热模块)
 *          引脚: TX=PB8(UT1TXMAP_G2_PB8), RX=PB9(UT1RXMAP_G2_PB9)
 *          接收用 bsp_uart1_get_char() 轮询，不走 ISR 回调链
 *          >1 字节字段(data_len)采用大端传输
 */

#include "include.h"
#include "func_lunchbox_uart.h"
#include "func_lunchbox_uart_internal.h"
#include "func_lunchbox_ota.h"
#include "func_lunchbox_bridge.h"
#include "func_lunchbox_uart_heat.h"
#include "home_ui_shared.h"
#include "func_lunchbox_ble.h"
#include "func_lunchbox_partition.h"

#include "func_lunchbox_lcd.h"
#include "func_lid_confirm.h"
#include "heat_display_reg.h"
#if ELUNCHBOX_PANEL_EN
#include "home_ui_shared.h"
#include "func.h"
#include "func_lowbat.h"
#endif
#include "bsp_vbat.h"

#if FUNC_LUNCHBOX_UART_EN

#define LB_DEBUG    0                   // 1=打开协议调试打印，0=关闭
#if LB_DEBUG
#define LB_TRACE(...)       printf(__VA_ARGS__)
#else
#define LB_TRACE(...)
#endif

// 心跳包数据定义 (MCU通信协议.md v1.0.7 §6.1)
// 加热模块每1分钟发一次请求，无回应则3秒重试，连续3次失败报警5声
#define LB_HEARTBEAT_REQUEST    0x00    // 请求响应 (加热模块→MCU)
#define LB_HEARTBEAT_RESPONSE   0x01    // 回复应答 (MCU→加热模块)

//-----------------------------------------------------------------------------
// 空 ISR（uart_init 要求 rx_isr 非 NULL，实际数据走 bsp_uart1 环形缓冲）
//-----------------------------------------------------------------------------
AT(.com_text.uart)
static void lb_dummy_isr(uint8_t *buf, uint32_t len) { (void)buf; (void)len; }

//-----------------------------------------------------------------------------
// 内部状态
//-----------------------------------------------------------------------------
u8  lb_rx_buf[LB_RXBUF_SIZE];     // 帧解析缓冲区
u16 lb_rx_idx;                    // 当前已写入帧缓冲的字节数 / 状态机位置
u32 lb_rx_ticks;                  // 最近一次收到字节的时间戳（超时丢弃用）


static u8  lb_ring_buf[LB_RXBUF_SIZE];   // bsp_uart1 环形缓冲区
u8  lb_tx_buf[LB_TXBUF_SIZE];     // 组帧发送缓冲区
lb_cmd_handler_t   cmd_handler[16];   // 命令字 → 回调，仅 0x01~0x0E 有效
lb_ble_tx_fn_t     lb_ble_tx_fn;      // BLE 发送回调（非 NULL 时走 BLE）
bool lb_uart_sync_pending = false;    // 是否有同步UART请求待应答(用于区分同步/异步0x01)
u32  lb_synced_unix_ts = 0;           // APP 同步的权威 Unix 时间戳
u32  lb_synced_rtccnt = 0;            // 同步时的 RTCCNT 值
bool lb_has_ble_ts = false;           // 是否已收到过 BLE 时间同步
u32  lb_synced_heat_unix_ts = 0;      // 加热模块同步的 Unix 时间戳
u32  lb_synced_heat_rtccnt = 0;       // 加热模块同步时的 RTCCNT 值
bool lb_has_heat_ts = false;          // 是否已收到过加热模块时间同步
bool lb_ble_presets_pending = false;  // BLE 连接后等待 APP 时间戳应答再发预设
bool lb_product_info_pending = false; // v1.0.7: 0x01 查询等待加热模块UART应答
u8   lb_product_info_msg_flag = 0;    // 待完成 0x01 查询的 BLE msg_flag
u32  lb_product_info_pend_tick = 0;   // 0x01 查询开始等待的时刻(tick), 超时用

// 发送队列 + 重试机制 (定义见 func_lunchbox_partition.h)
lb_send_q_item_t lb_send_queue[LB_SEND_QUEUE_SIZE];
u8  lb_send_q_head;
u8  lb_send_q_tail;
u8  lb_send_q_count;

bool lb_send_waiting;
u8   lb_send_wait_msg;
u8   lb_send_retry;
u32  lb_send_tick;
u8   lb_send_cur_cmd;
u8   lb_send_cur_data[128];
u16  lb_send_cur_dlen;

u8   lb_uart_raw_msg_flag;
u8   lb_keep_warm_msg_flag;       // 保温指令 msg_flag, 用于过滤过时应答
bool lb_send_no_ble_report;          /* 当前等待命令是否跳过BLE上报(按键/心跳) */

// v1.0.7: 前向声明 — lb_frame_parse() 引用了这些定义在后面的符号
u8  lb_pending_ble_cmd[256];
u8  lb_handler_product_info(lb_rx_frame_t *rx);

bool lb_uart_suspended;              /* 手动关机时 UART1 已关闭 */
static u32 lb_uart_saved_con;        /* suspend 前保存的 UART1CON 值，resume 时恢复 */
bool lb_uart_tx_blocked;             /* 手动关机时阻止所有 UART TX */
lb_device_info_t lb_dev_info;
/* 预约列表本地缓存：桥模式/本地模式均需可用（UART 0x02 应答 → lb_frame_parse 填充） */
static lb_schedule_ble_t   lb_schedules[LB_SCHEDULE_MAX];
static u8                  lb_schedule_count;
// OTA 升级状态机 → 已移至 func_lunchbox_ota.c
// 桥模式 CRC32 变量 → 已移至 func_lunchbox_bridge.c

//-----------------------------------------------------------------------------
// 模式界面 → 加热界面 预设参数传递 → 已移至 func_lunchbox_lcd.c
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 工具
//-----------------------------------------------------------------------------

void lb_dp_dump_hex(const u8 *data, u16 data_len);
void lb_ble_dump_frame(u8 cmd, const u8 *data, u16 len, bool is_rx);

// lb_crc32 → 已移至 func_lunchbox_ota.c

/**
 * @brief 计算协议校验和
 *
 * 从 data[0] 到 data[len-1] 逐字节累加，结果对 256 取余（取低 8 位）。
 */
u8 lb_checksum(u8 *data, u16 len)
{
    u32 sum = 0;                                    // u32 防累加上溢
    for (u16 i = 0; i < len; i++) sum += data[i];
    return (u8)(sum % 256);                         // 取低 8 位
}

/**
 * @brief 获取当前 Unix 时间戳
 *
 * 若 APP 已通过 0x01 或 0x03 同步过权威时间，则用同步基准推算当前时间；
 * 否则回退到本地 RTC + 固定偏移。
 *
 * @return 当前 Unix 时间戳 (秒)
 */
u32 lb_get_unix_time(void)
{
    if (lb_has_ble_ts) {
        return lb_synced_unix_ts + (RTCCNT - lb_synced_rtccnt);
    }
    return RTCCNT + LB_RTC_UNIX_OFFSET;
}

/**
 * @brief 获取屏幕显示时间 (优先级: APP > 加热模块 > 本地RTC)
 *
 * 开机时:
 *   - 若 APP 已蓝牙连接并同步过时间 → 使用 APP 权威时间
 *   - 若 APP 未连接但加热模块已上报时间 → 使用加热模块时间
 *   - 若两者均未同步 → 使用本地 RTC 默认时间
 *
 * @return tm_t 结构体 (北京时间), 可直接用于 UI 显示
 */
tm_t lb_get_display_tm(void)
{
    u32 unix_time;
    u32 rtccnt;

    /* 优先级1: APP 通过 BLE 同步的时间 */
    if (lb_has_ble_ts) {
        unix_time = lb_synced_unix_ts + (RTCCNT - lb_synced_rtccnt);
        rtccnt = unix_time - LB_RTC_UNIX_OFFSET;
        return time_to_tm(rtccnt);
    }

    /* 优先级2: 加热模块通过 UART 上报的时间 */
    if (lb_has_heat_ts) {
        unix_time = lb_synced_heat_unix_ts + (RTCCNT - lb_synced_heat_rtccnt);
        rtccnt = unix_time - LB_RTC_UNIX_OFFSET;
        return time_to_tm(rtccnt);
    }

    /* 优先级3: 本地 RTC 默认时间 */
    return rtc_clock_get();
}

/** @brief 检查 DataPoints 数据中是否包含按键通知 (dpid=12) */
bool lb_data_is_key_notify(u8 *data, u16 len)
{
    u16 off = 0;
    while (off + 4 <= len) {
        u8  dpid    = data[off];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];

        // 先检查 dpid, 避免 val_len 异常(大小端/字段错位)导致漏判
        if (dpid == LB_DPID_KEY_NOTIFY) return true;

        // val_len 合法性兜底: 单 DataPoint 值不超过 256 字节
        if (val_len > 256 || off + 4 + val_len > len) break;
        off += 4 + val_len;
    }
    return false;
}

/** @brief 重置帧接收状态，丢弃当前未完成的帧 */
static void lb_rx_reset(void) { lb_rx_idx = 0; lb_rx_ticks = 0; }

//-----------------------------------------------------------------------------
// 帧解析 (UART)
//-----------------------------------------------------------------------------
/**
 * @brief 尝试从 lb_rx_buf 中解析一帧
 *
 * 帧格式：
 *   [header:2B] [ver:1B] [flag:1B] [cmd:1B] [err:1B] [dlen:2B] [data:dlen B] [chk:1B]
 *    └─────────────── lb_frame_head_t ───────────────┘  └─可变长─┘  └尾─┘
 *
 * 校验规则：从 header 开始到 data 最后，所有字节累加取模 256，等于 checksum
 *
 * @return true  解析成功，已回调 cmd_handler 或转发到 BLE，lb_rx_idx 已重置
 * @return false 数据不够/校验失败/帧太大，调用者需继续收或丢弃
 */
void lb_heating_sync_from_dp(u8 *data, u16 len);

static bool lb_frame_parse(void)
{
    // ──── 检查 1：帧缓冲区数据至少够一个完整帧头 + 1 字节 checksum ────
    if (lb_rx_idx < sizeof(lb_frame_head_t) + 1) return false;

    // ──── 检查 2：帧头校验 ────
    if (lb_rx_buf[0] != 0x55 || lb_rx_buf[1] != 0xaa) {
        lb_rx_reset();
        return false;
    }

    // ──── 把字节数组当作 lb_frame_head_t 结构体解释 ────
    lb_frame_head_t *h = (lb_frame_head_t *)lb_rx_buf;

    // ──── 检查 3：data_len 大端→小端，计算完整帧长度 ────
    u16 data_len = ((u16)h->data_len << 8) | (h->data_len >> 8);
    u16 total = sizeof(lb_frame_head_t) + data_len + 1;
    if (total > LB_RXBUF_SIZE) {
        // 数据长度异常: 跳过帧头首字节 0x55, 前移剩余数据继续搜索
        if (lb_rx_idx > 1) {
            memmove(lb_rx_buf, lb_rx_buf + 1, lb_rx_idx - 1);
            lb_rx_idx -= 1;
        } else {
            lb_rx_idx = 0;
        }
        return false;
    }
    if (lb_rx_idx < total) {
        return false;                               // 数据还没到齐
    }

    // ──── 检查 4：校验和验证 ────
    if (lb_checksum(lb_rx_buf, total - 1) != lb_rx_buf[total - 1]) {
        LB_TRACE("lb: chk fail\n");
        // 校验失败: 跳过帧头首字节 0x55, 前移剩余数据继续搜索
        if (lb_rx_idx > 1) {
            memmove(lb_rx_buf, lb_rx_buf + 1, lb_rx_idx - 1);
            lb_rx_idx -= 1;
        } else {
            lb_rx_idx = 0;
        }
        return false;
    }

    // ──── 校验通过，组装接收帧结构体 ────
    lb_rx_frame_t rx = {
        .version  = h->version,
        .msg_flag = h->msg_flag,
        .cmd      = h->cmd,
        .err_flag = h->err_flag,
        .data_len = data_len,
        .data     = data_len ? lb_rx_buf + sizeof(lb_frame_head_t) : NULL,
        .valid    = true,
    };

    // ──── UART 收包日志 (按键通知不应答, 跳过) ────
    if (!lb_data_is_key_notify(rx.data, rx.data_len)) {
        printf("UART==>RX[%d]: ", total);
        for (u16 i = 0; i < total; i++) printf("%02X ", lb_rx_buf[i]);
        printf("\n");
        // 仅 0x01 动态属性帧的数据为 DataPoint 格式
        if (rx.cmd == LB_UART_CMD_DYNAMIC) lb_dp_dump_hex(rx.data, rx.data_len);
    }

#if FUNC_LUNCHBOX_UART_EN
    // 仅当加热模块执行成功 (err_flag == LB_ERR_SUCCESS) 时才将数据回调给屏幕
    if (rx.cmd == LB_UART_CMD_DYNAMIC && rx.data && rx.data_len > 0
        && rx.err_flag == LB_ERR_SUCCESS) {
        lb_heating_sync_from_dp(rx.data, rx.data_len);
#if ELUNCHBOX_PANEL_EN
        home_ui_shared_battery_feed_dp(rx.data, rx.data_len);
        elunchbox_lowbat_feed_dp(rx.data, rx.data_len);
        elunchbox_lowbat_poll();
        if (!elunchbox_lowbat_should_block_ui_route()) {
#endif
        heat_display_feed_dp(rx.data, rx.data_len, rx.msg_flag);
        /* 保温页的 UART 数据路由已由 heat_display_feed_dp 完整处理；
         * lunchbox_control_apply_panel 是为 BLE 0x04 控制指令设计的，
         * 若在保温页被 UART 帧触发，会导致：
         * 1) heat_finish→warm 后同帧 DP10=0 误杀回主页
         * 2) Mode=5 帧触发 func_new_warm_ble_restart→lunchbox_heat_stop→HeatEn=OFF 死循环 */
        if (func_cb.sta != FUNC_NEW_WARM && func_cb.sta != FUNC_LID_CONFIRM
            && !elunchbox_lid_confirm_is_armed() && lb_heat_lcd_active) {
            lunchbox_control_apply_panel(rx.data, rx.data_len);
        }
#if ELUNCHBOX_PANEL_EN
        }
#endif
    }
#endif

    // ──── 预约列表查询应答 (0x02): 保存到本地 lb_schedules[] ────
    // 加热模块逐条返回预约记录 (44B/条)，桥模式和本地模式均需本地保存
    if (rx.cmd == LB_UART_CMD_SCHEDULE && rx.data && rx.data_len >= 44) {
        u8 total = rx.data[0];
        if (total > 0) {
            u8 seq = rx.data[1];
            if (seq == 1) lb_schedule_count = 0;  // 第一条，重置计数
            if (lb_schedule_count < LB_SCHEDULE_MAX) {
                lb_schedule_ble_t *s = &lb_schedules[lb_schedule_count];
                s->id       = rx.data[3];
                memcpy(s->name, rx.data + 4, 32);
                s->time     = ((u32)rx.data[36] << 24) | ((u32)rx.data[37] << 16)
                            | ((u32)rx.data[38] << 8)  | rx.data[39];
                s->temp     = rx.data[40];
                s->duration = rx.data[41];
                s->enabled  = rx.data[42];
                s->repeat   = rx.data[43];
                lb_schedule_count++;
            }
        }
        // 无论是否保存，都继续走桥模式 BLE 转发（不 goto cleanup）
    }

    // ──── 心跳包 (0x05): MCU↔加热模块内部通信，不转发BLE ────
    // 加热模块每1分钟发一次请求(0x00)，MCU须回应(0x01)
    // 加热模块: 无回应则3秒重试，连续3次失败报警5声 (MCU协议 §6.1)
    if (rx.cmd == LB_UART_CMD_HEARTBEAT && rx.data && rx.data_len >= 1
        && rx.data[0] == LB_HEARTBEAT_REQUEST) {
        u8 rsp = LB_HEARTBEAT_RESPONSE;
        lb_ble_tx_fn_t saved_ble = lb_ble_tx_fn;
        lb_ble_tx_fn = NULL;  // 强制走UART，不能走BLE
        lunchbox_uart_send_response(LB_UART_CMD_HEARTBEAT, rx.msg_flag,
                                    LB_ERR_SUCCESS, &rsp, 1);
        lb_ble_tx_fn = saved_ble;
        //printf("UART==>TX[heartbeat]: 55 AA 00 %02X 05 00 00 01 01 %02X\n",
               //rx.msg_flag, (u8)(0x55+0xAA+0x00+rx.msg_flag+0x05+0x00+0x00+0x01+0x01) % 256);
        goto lb_frame_cleanup;  // 心跳不进入BLE翻译/本地分发，直接清理缓冲区
    }

    // ──── 加热模块 OTA: 注入 UART 0x04 应答到状态机 ────
    // 加热模块OTA期间, UART 0x04应答帧仅用于内部状态机驱动,
    // 不转发给APP (蓝牙通讯协议1.0.6 §7: 只回应蓝牙升级成功/失败)
    if (rx.cmd == LB_UART_CMD_OTA && heat_ota_is_active()) {
        heat_ota_uart_response(&rx);
        goto lb_frame_cleanup;
    }

#if LB_BRIDGE_MODE
    // ──── 桥模式：翻译为 BLE 协议 → 通过 BLE 发给 APP ────
    // 蓝牙未连接时跳过转发，节省协议翻译+BLE TX 尝试的功耗
    // 按键通知 (dpid=12) 仅 MCU ↔ 加热模块内部使用，不转发给 APP
    if (ble_is_connected() && !lb_data_is_key_notify(rx.data, rx.data_len)
        && !lb_send_no_ble_report) {
        // v1.0.7: 检查是否是 0x01 产品信息查询的加热模块应答
        // 此时应先完成 BLE 0x01 回复 (含加热模块版本号)，再将 DataPoints 异步上报
        if (lb_product_info_pending && rx.cmd == LB_UART_CMD_DYNAMIC) {
            // 从 DataPoints 中提取加热模块固件版本号 (dpid=13)
            if (rx.data && rx.data_len > 0) {
                u16 off = 0;
                while (off + 4 <= rx.data_len) {
                    u8  dpid    = rx.data[off];
                    u16 val_len = ((u16)rx.data[off + 2] << 8) | rx.data[off + 3];
                    if (off + 4 + val_len > rx.data_len) break;
                    if (dpid == LB_DPID_MCU_VERSION && val_len >= 4) {
                        u8 *v = rx.data + off + 4;
                        lb_dev_info.heat_module_version = ((u32)v[0] << 24) | ((u32)v[1] << 16)
                                                        | ((u32)v[2] << 8)  | v[3];
                    }
                    off += 4 + val_len;
                }
            }
            // 清除同步等待标志，使 lb_translate_uart_to_ble 将此帧视为异步上报 (0x03)
            lb_uart_sync_pending = false;
            lb_pending_ble_cmd[rx.msg_flag] = 0;
            // 用保存的 BLE msg_flag 构造合成帧，完成 BLE 0x01 应答
            {
                lb_rx_frame_t synth;
                memset(&synth, 0, sizeof(synth));
                synth.msg_flag = lb_product_info_msg_flag;
                synth.cmd      = LB_CMD_PRODUCT_INFO;
                synth.valid    = true;
                lb_handler_product_info(&synth);
            }
            lb_product_info_pending = false;
            // 继续走下面的 lb_translate_uart_to_ble —
            // 此时 pending 已清除, DataPoints 会作为 BLE 0x03 异步上报
        }

        u8 ble_buf[LB_TXBUF_SIZE];
        u16 ble_len = 0;
        if (lb_translate_uart_to_ble(&rx, ble_buf, &ble_len)) {
            if (lb_ble_tx_fn) {
                lb_ble_tx_fn(ble_buf, ble_len);
                // 打印 BLE TX 协议分析 (与 lb_send_frame 保持一致)
                u8  tx_cmd = ble_buf[4];
                u16 tx_dl  = ((u16)ble_buf[6] << 8) | ble_buf[7];
                lb_ble_dump_frame(tx_cmd, tx_dl ? ble_buf + 8 : NULL, tx_dl, false);
            }
        }
    }
#else
    // ──── 本地模式：派发到已注册的命令处理器 ────
    LB_TRACE("lb: rx cmd=0x%02X dlen=%d\n", rx.cmd, rx.data_len);
    if (rx.cmd < 16 && cmd_handler[rx.cmd]) {
        // UART 收到的帧，应答走 UART 而非 BLE
        lb_ble_tx_fn_t saved_ble = lb_ble_tx_fn;
        lb_ble_tx_fn = NULL;
        cmd_handler[rx.cmd](&rx);
        lb_ble_tx_fn = saved_ble;
    }
#endif

    // ──── 重试机制: 收到加热模块回应 (任意 cmd, msg_flag 匹配) 则清除等待 ────
    if (lb_send_waiting && rx.msg_flag == lb_send_wait_msg) {
        lb_send_waiting = false;
    }

lb_frame_cleanup:
    // 解析成功: 将缓冲区中剩余字节前移 (支持单次 BLE 写入含多帧的场景)
    {
        u16 remaining = lb_rx_idx - total;
        if (remaining > 0) {
            memmove(lb_rx_buf, lb_rx_buf + total, remaining);
            lb_rx_idx = remaining;
        } else {
            lb_rx_idx = 0;
            lb_rx_ticks = 0;
        }
    }
    return true;
}

/**
 * @brief 帧接收超时检测
 */
static void lb_timeout_check(void)
{
    if (lb_rx_idx && tick_check_expire(lb_rx_ticks, LB_FRAME_TIMEOUT_MS))
        lb_rx_reset();
}

//-----------------------------------------------------------------------------
// 发送
//-----------------------------------------------------------------------------

/**
 * @brief 组帧并发送（UART/BLE 双通道自动切换）
 *
 * 帧格式（大端）：
 *   [0x55][0xAA][ver][flag][cmd][err][dlen_H][dlen_L][data...][checksum]
 *
 * 双通道切换逻辑：
 *   - lb_ble_tx_fn == NULL  → uart_bufs_tx() 走串口 PB8 发出
 *   - lb_ble_tx_fn != NULL  → lb_ble_tx_fn() 走 BLE Notify 发出
 */
#if ELUNCHBOX_PANEL_EN
/** 充电中 / MCU 驱动跳页期间仅接收加热模块 UART，主动下发由 MCU 侧完成；心跳应答除外 */
static bool lb_uart_tx_skip_rx_only(u8 cmd)
{
    if (cmd == LB_UART_CMD_HEARTBEAT) {
        return false;
    }
    if (home_ui_shared_battery_is_charging()) {
        return true;
    }
    if (lb_heat_mcu_nav_active()) {
        return true;
    }
    return false;
}
#endif

static void lb_send_frame(u8 cmd, u8 msg_flag, u8 err, u8 *data, u16 len)
{
    if (lb_uart_tx_blocked) {
        return;  /* 手动关机期间禁止 UART TX */
    }
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_LOWBAT_MODE_EN
    if (elunchbox_lowbat_active()) {
        return;  /* 低电页：仅显示 UI，不应答/不下发任何 UART */
    }
#endif
#if ELUNCHBOX_PANEL_EN
    if (lb_ble_tx_fn == NULL && lb_uart_tx_skip_rx_only(cmd)) {
        return;
    }
#endif
    u16 off = 0;
    lb_tx_buf[off++] = (u8)(LB_FRAME_HEADER >> 8);  // 0x55
    lb_tx_buf[off++] = (u8)LB_FRAME_HEADER;          // 0xaa
    lb_tx_buf[off++] = LB_FRAME_VERSION;
    lb_tx_buf[off++] = msg_flag;
    lb_tx_buf[off++] = cmd;
    lb_tx_buf[off++] = err;
    lb_tx_buf[off++] = (u8)(len >> 8);               // data_len 高字节 (大端)
    lb_tx_buf[off++] = (u8)(len & 0xFF);             // data_len 低字节 (大端)

    if (data && len) {
        if (off + len > LB_TXBUF_SIZE - 1) {
            printf("lb_send_frame: data too large, len=%d\n", len);
            return;
        }
        memcpy(lb_tx_buf + off, data, len); off += len;
    }
    lb_tx_buf[off] = lb_checksum(lb_tx_buf, off); off++;

    if (lb_ble_tx_fn) {
        lb_ble_tx_fn(lb_tx_buf, off);           // 走 BLE Notify (含 BLE==>TX hex 打印)
        // 打印 BLE TX 协议分析 (蓝牙通讯协议1.0.7.md §3)
        lb_ble_dump_frame(cmd, data, len, false);
    } else {
        uart_bufs_tx(UART_TYPE_1, lb_tx_buf, off); // 走 UART
    }
}

/** @brief 发送请求/命令帧，err_flag 固定为成功 */
void lunchbox_uart_send(u8 cmd, u8 msg_flag, u8 *data, u16 len)
    { lb_send_frame(cmd, msg_flag, LB_ERR_SUCCESS, data, len); }

/** @brief 发送应答帧，可指定 err_flag */
void lunchbox_uart_send_response(u8 cmd, u8 msg_flag, u8 err, u8 *data, u16 len)
    { lb_send_frame(cmd, msg_flag, err, data, len); }

u8 lb_async_msg_flag = 0;    // 异步消息自动递增的 msg_flag

/**
 * @brief 异步发送帧 — MCU 主动推送（无需主机先请求）
 */
void lunchbox_uart_send_async(u8 cmd, u8 *data, u16 len)
    { lb_send_frame(cmd, lb_async_msg_flag++, LB_ERR_SUCCESS, data, len); }

//-----------------------------------------------------------------------------
// 设备信息
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// DataPoint 编码工具 (桥模式和本地模式均可用)
//-----------------------------------------------------------------------------

/**
 * @brief 向发送缓冲区写入一个 DataPoint 单元
 * @return 写入的字节数
 */
u16 lb_dp_encode(u8 *buf, u8 dpid, u8 type, u8 *val, u16 val_len)
{
    buf[0] = dpid;
    buf[1] = type;
    buf[2] = (u8)(val_len >> 8);    // 值长度大端
    buf[3] = (u8)(val_len & 0xFF);
    if (val && val_len) memcpy(buf + 4, val, val_len);
    return 4 + val_len;
}

/** @brief 编码 bool 型 DataPoint */
u16 lb_dp_encode_bool(u8 *buf, u8 dpid, u8 val)
{
    return lb_dp_encode(buf, dpid, LB_DP_TYPE_BOOL, &val, 1);
}

/** @brief 扫描 DataPoint 缓冲区，查找 bool/enum 型 dpid 的首字节值 */
bool lb_dp_scan_bool(const u8 *data, u16 len, u8 dpid, u8 *val)
{
    u16 off = 0;

    while (off + 4 <= len) {
        u8  id      = data[off];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];

        if (off + 4 + val_len > len) {
            break;
        }
        if (id == dpid && val_len >= 1) {
            if (val) {
                *val = data[off + 4];
            }
            return true;
        }
        off += 4 + val_len;
    }
    return false;
}

/** @brief 编码 enum 型 DataPoint */
u16 lb_dp_encode_enum(u8 *buf, u8 dpid, u8 val)
{
    return lb_dp_encode(buf, dpid, LB_DP_TYPE_ENUM, &val, 1);
}

/** @brief 编码 value 型 DataPoint (4B 大端) */
u16 lb_dp_encode_value(u8 *buf, u8 dpid, u32 val)
{
    u8 v[4];
    v[0] = (u8)(val >> 24);
    v[1] = (u8)(val >> 16);
    v[2] = (u8)(val >> 8);
    v[3] = (u8)(val & 0xFF);
    return lb_dp_encode(buf, dpid, LB_DP_TYPE_VALUE, v, 4);
}

//-----------------------------------------------------------------------------
// DataPoint 解析打印（调试用 — 在 UART TX/RX 日志下方打印可读描述）
//-----------------------------------------------------------------------------

/**
 * @brief 遍历数据区的 DataPoints 并打印 hex + 简短英文描述
 *
 * DataPoint 格式: dpid(1B) + type(1B) + val_len(2B,BE) + value(val_len)
 *
 * @param data     数据区首指针
 * @param data_len 数据区总字节数
 */
void lb_dp_dump_hex(const u8 *data, u16 data_len)
{
    if (!data || data_len < 4) return;

    u16  off  = 0;
    bool head = true;

    while (off + 4 <= data_len) {
        u8  dpid    = data[off];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];

        if (off + 4 + val_len > data_len) break;

        const u8 *val = data + off + 4;
        u16       dp_total = 4 + val_len;

        // --- 首条前打印顶部分隔线 ---
        if (head) {
            printf("-------------------------------------------------------\n");
            head = false;
        }

        // --- 打印原始 hex ---
        for (u16 i = 0; i < dp_total; i++) printf("%02X ", data[off + i]);

        // 填充至 26 列对齐（8 hex 字节 = 24 字符 + 余量）
        u8 hex_chars = dp_total * 3;
        for (u8 p = hex_chars; p < 26; p++) printf(" ");

        printf("DP%02d:", dpid);

        // --- 按 dpid 输出描述 ---
        switch (dpid) {
        case LB_DPID_POWER_SWITCH:   // 1: bool
            printf(" PowerSwitch=%s", val[0] ? "ON" : "OFF");
            break;
        case LB_DPID_HEAT_MODE: {    // 2: enum
            static const char *modes[] = {"Off","Custom","Chicken","Pasta","Schedule","KeepWarm"};
            printf(" HeatMode=%s(%d)", val[0] < 6 ? modes[val[0]] : "?", val[0]);
            break;
        }
        case LB_DPID_BATTERY: {      // 3: enum
            static const char *bats[] = {"Dead","Low","Mid","High","Full"};
            printf(" Battery=%s(%d)", val[0] <= 4 ? bats[val[0]] : "?", val[0]);
            break;
        }
        case LB_DPID_CHARGE_STATUS: { // 4: enum
            static const char *chgs[] = {"NoCharge","Charging","Full"};
            printf(" Charge=%s(%d)", val[0] < 3 ? chgs[val[0]] : "?", val[0]);
            break;
        }
        case LB_DPID_HEAT_DURATION: { // 5: value(4B)
            u32 v = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                  | ((u32)val[2] << 8)  |  (u32)val[3];
            printf(" HeatDur=%lumin", (unsigned long)v);
            break;
        }
        case LB_DPID_REMAIN_TIME: {   // 6: value(4B)
            u32 v = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                  | ((u32)val[2] << 8)  |  (u32)val[3];
            printf(" Remain=%lumin", (unsigned long)v);
            break;
        }
        case LB_DPID_HEAT_TEMP: {     // 7: enum 0=40°C(104°F) ~ 6=100°C(212°F)
            static const u16 temp_f[] = {104, 122, 140, 158, 176, 194, 212};
            if (val[0] <= 6) {
                u16 c = 40 + (u16)val[0] * 10;
                printf(" HeatTemp=%uC/%uF(%d)", c, temp_f[val[0]], val[0]);
            } else {
                printf(" HeatTemp=?(%d)", val[0]);
            }
            break;
        }
        case LB_DPID_LANGUAGE:        // 8: enum
            printf(" Lang=%d", val[0]);
            break;
        case LB_DPID_FAULT: {         // 9: enum (0=正常, 1=故障)
            // fault_code 详情见 MCU通信协议.md §4.1.6:
            // 0x01=干烧超温 0x02=温升过快 0x03=NTC传感器故障 0x04=加热丝过流
            // 0x05=上盖5V短路 0x06=NTC无响应 0x07=NTC未连接 0x08=NTC异常
            // 0x09=蓝牙模组心跳超时 0x0a=低电上报 (v1.0.7新增)
            static const char *faults[] = {"OK","Fault"};
            printf("Fault=%s(%d)", val[0] < 2 ? faults[val[0]] : "?", val[0]);
            if (val[0] == 0x0A) {
                printf(" [LOWBAT]");
            }
            break;
        }
        case LB_DPID_HEAT_ENABLE:     // 10: bool
            printf(" HeatEn=%s", val[0] ? "ON" : "OFF");
            break;
        case LB_DPID_TIME_SYNC: {     // 11: value(4B)
            u32 v = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                  | ((u32)val[2] << 8)  |  (u32)val[3];
            printf(" TimeSync=%lu", (unsigned long)v);
            break;
        }
        case LB_DPID_KEY_NOTIFY:      // 12: enum
            printf(" Key=%d", val[0]);
            break;
        case LB_DPID_MCU_VERSION: {   // 13: value(4B)
            u32 v = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                  | ((u32)val[2] << 8)  |  (u32)val[3];
            printf(" MCUVersion=0x%08lX", (unsigned long)v);
            break;
        }
        case LB_DPID_RTC_TIME: {     // 14: value(4B) 加热模块RTC时间 (v1.0.7新增)
            u32 v = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                  | ((u32)val[2] << 8)  |  (u32)val[3];
            printf(" RtcTime=%lu", (unsigned long)v);
            break;
        }
        default:
            printf(" DPID%d=?", dpid);
            break;
        }
        printf("\n");

        off += dp_total;
    }

    // --- 尾部分隔线 ---
    if (!head) {
        printf("-------------------------------------------------------\n");
    }
}

//-----------------------------------------------------------------------------
// BLE 帧数据分析 (蓝牙通讯协议1.0.7.md §3)
// 按命令字解析数据字段，输出协议级可读描述
//   is_rx=true  → APP→MCU 方向
//   is_rx=false → MCU→APP 方向
//-----------------------------------------------------------------------------
void lb_ble_dump_frame(u8 cmd, const u8 *data, u16 len, bool is_rx)
{
    // 无数据不做任何打印
    if (!data || !len) return;

    printf("-------------------------------------------------------\n");
    switch (cmd) {

    //=== 0x01: ProductInfo =============================================
    case LB_CMD_PRODUCT_INFO:
        if (is_rx) {
            // APP→MCU: timestamp(4B, BE)
            if (len >= 4) {
                u32 ts = ((u32)data[0] << 24) | ((u32)data[1] << 16)
                       | ((u32)data[2] << 8)  |  (u32)data[3];
                printf("Timestamp=%lu\n", (unsigned long)ts);
            }
        } else {
            // MCU→APP: product info struct (81 bytes)
            // 字段顺序: BLEname(16)+version(8)+modeltype(10)+MAC(6)+SN(32)+color(1)+masterMCU(4)+heatMCU(4)
            if (len >= 16) printf("BLEname=%.16s\n", data);
            if (len >= 24) printf("version=%.8s\n",  data + 16);
            if (len >= 34) printf("modeltype=%.10s\n", data + 24);
            if (len >= 40) printf("MAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
                                  data[34], data[35], data[36], data[37], data[38], data[39]);
            if (len >= 72) printf("SN=%.32s\n", data + 40);
            if (len >= 73) printf("color=%u\n", data[72]);
            if (len >= 77) {
                u32 main_ver = ((u32)data[73] << 24) | ((u32)data[74] << 16)
                             | ((u32)data[75] << 8)  |  (u32)data[76];
                printf("main_ver=0x%08lX\n", (unsigned long)main_ver);
            }
            if (len >= 81) {
                u32 heat_ver = ((u32)data[77] << 24) | ((u32)data[78] << 16)
                             | ((u32)data[79] << 8)  |  (u32)data[80];
                printf("heat_ver=0x%08lX\n", (unsigned long)heat_ver);
            }
        }
        break;

    //=== 0x02/0x03: DataPoints =======================================
    case LB_CMD_DYNAMIC_ATTR:
    case LB_CMD_STATUS_REPORT:
        lb_dp_dump_hex(data, len);
        break;

    //=== 0x04: Control =================================================
    case LB_CMD_CONTROL:
        // APP→MCU 和 MCU→APP 两个方向都可能携带 DataPoints
        lb_dp_dump_hex(data, len);
        break;

    //=== 0x05: ScheduleList ============================================
    case LB_CMD_SCHEDULE_LIST:
        if (!is_rx && len >= 44 && data[0] > 0) {
            // MCU→APP: single schedule entry (44 bytes), ALL=0 时不打印
            u8  ALL    = data[0];
            u8  now_id = data[1];
            u8  mode   = data[2];
            u8  ID     = data[3];
            u32 TIME   = ((u32)data[36] << 24) | ((u32)data[37] << 16)
                       | ((u32)data[38] << 8)  |  (u32)data[39];
            u8  temp   = data[40];
            u8  time   = data[41];
            u8  status = data[42];
            u8  rep    = data[43];
            printf("Schedule[%u/%u] ALL=%u now_id=%u mode=%u ID=%u name=%.32s TIME=%lu temp=%u time=%umin status=%u rep=0x%02X\n",
                   now_id, ALL, ALL, now_id, mode, ID, data + 4, (unsigned long)TIME, temp, time, status, rep);
        }
        break;

    //=== 0x06: ScheduleAdd =============================================
    case LB_CMD_SCHEDULE_ADD:
        if (is_rx && len >= 42) {
            // APP→MCU: 42 bytes schedule data (MCU通信协议.md §3.6)
            u8  mode   = data[0];
            u8  ID     = data[1];
            u32 TIME   = ((u32)data[34] << 24) | ((u32)data[35] << 16)
                       | ((u32)data[36] << 8)  |  (u32)data[37];
            u8  temp   = data[38];
            u8  time   = data[39];
            u8  status = data[40];
            u8  rep    = data[41];
            printf("mode=%u ID=%u name=%.32s TIME=%lu temp=%u time=%umin status=%u rep=0x%02X\n",
                   mode, ID, data + 2, (unsigned long)TIME, temp, time, status, rep);
        } else if (!is_rx && len >= 1) {
            // MCU→APP: assigned ID(1B)
            printf("AssignedID=%u\n", data[0]);
        }
        break;

    //=== 0x07: ScheduleModify ==========================================
    case LB_CMD_SCHEDULE_MODIFY:
        if (is_rx && len >= 42) {
            // APP→MCU: 42 bytes schedule data (MCU通信协议.md §3.6)
            u8  mode   = data[0];
            u8  ID     = data[1];
            u32 TIME   = ((u32)data[34] << 24) | ((u32)data[35] << 16)
                       | ((u32)data[36] << 8)  |  (u32)data[37];
            u8  temp   = data[38];
            u8  time   = data[39];
            u8  status = data[40];
            u8  rep    = data[41];
            printf("mode=%u ID=%u name=%.32s TIME=%lu temp=%u time=%umin status=%u rep=0x%02X\n",
                   mode, ID, data + 2, (unsigned long)TIME, temp, time, status, rep);
        }
        break;

    //=== 0x08: ScheduleDelete ==========================================
    case LB_CMD_SCHEDULE_DELETE:
        if (is_rx && len >= 1) {
            printf("ID=%u\n", data[0]);
        }
        break;

    //=== 0x09: ModeInfo ================================================
    case LB_CMD_MODE_QUERY:
        if (is_rx && len >= 1) {
            static const char *mode_names[] = {"?","Custom","Chicken","Pasta"};
            printf("Mode=%s(%u)\n", data[0] <= 3 ? mode_names[data[0]] : "?", data[0]);
        } else if (!is_rx && len >= 3) {
            // MCU→APP: each 3 bytes (mode+temp+dur), may be multiple
            static const char *mn[] = {"?","Custom","Chicken","Pasta"};
            u16 off = 0;
            while (off + 3 <= len) {
                u8 m = data[off], t = data[off + 1], d = data[off + 2];
                // 跳过空条目 (全零 = 加热模块未返回该模式信息)
                if (m == 0 && t == 0 && d == 0) { off += 3; continue; }
                printf("%s: temp=%u dur=%umin\n", m <= 3 ? mn[m] : "?", t, d);
                off += 3;
            }
        }
        break;

    //=== 0x0a: ModeModify ==============================================
    case LB_CMD_MODE_MODIFY:
        if (is_rx && len >= 3) {
            static const char *mn[] = {"?","Custom","Chicken","Pasta"};
            printf("%s temp=%u dur=%umin\n",
                   data[0] <= 3 ? mn[data[0]] : "?", data[1], data[2]);
        }
        break;

    //=== 0x0c: OTA Start ===============================================
    case LB_CMD_OTA_START:
        if (is_rx) {
            // APP→MCU: target(1B) + firmware_size(4B, BE)
            if (len >= 5) {
                u32 fw_size = ((u32)data[1] << 24) | ((u32)data[2] << 16)
                            | ((u32)data[3] << 8)  |  (u32)data[4];
                printf("target=0x%02X fw_size=%lu\n", data[0], (unsigned long)fw_size);
            }
        } else {
            // MCU→APP: target(1B) + status(1B)
            if (len >= 2) {
                static const char *sts[] = {"RECV","ERASING","ERASE_OK"};
                printf("target=0x%02X status=%s(%u)\n", data[0],
                       data[1] <= 2 ? sts[data[1]] : "?", data[1]);
            }
        }
        break;

    //=== 0x0d: OTA Data ================================================
    case LB_CMD_OTA_DATA:
        if (is_rx && len >= 5) {
            // APP→MCU: target(1B) + offset(4B, BE) + upgrade_data
            u32 offset = ((u32)data[1] << 24) | ((u32)data[2] << 16)
                       | ((u32)data[3] << 8)  |  (u32)data[4];
            printf("target=0x%02X offset=%lu data_len=%u\n", data[0], (unsigned long)offset, len - 5);
        }
        // MCU→APP: ack (no data) — skip
        break;

    //=== 0x0e: OTA End =================================================
    case LB_CMD_OTA_END:
        if (is_rx) {
            // APP→MCU: target(1B)
            printf("target=0x%02X\n", data[0]);
        } else if (len >= 2) {
            // MCU→APP: target(1B) + result(1B)
            printf("target=0x%02X result=%s(%u)\n", data[0],
                   data[1] ? "SUCCESS" : "FAIL", data[1]);
        }
        break;

    default:
        break;
    }
    printf("-------------------------------------------------------\n");
}

//-----------------------------------------------------------------------------
// 业务状态（本地模式使用）
//-----------------------------------------------------------------------------
// lb_schedules[] / lb_schedule_count 已移至文件顶部（lb_frame_parse 前向引用）
#if !LB_BRIDGE_MODE
static u8                  lb_next_schedule_id = 6;   // 自增预约 ID (ID 1~5 为固定预设)

// 当前属性值 (BLE 属性列表 §4)
u8  lb_attr_power_switch  = 1;    // 总开关: 默认开
u8  lb_attr_heat_mode     = 0;    // 加热模式: 默认关闭
u8  lb_attr_battery       = 4;    // 电量: 默认满
u8  lb_attr_charge_status = 0;    // 充电状态: 默认未充电
u32 lb_attr_heat_duration = 0;    // 加热时长(分钟)
u32 lb_attr_remain_time   = 0;    // 剩余时间(分钟)
u8  lb_attr_heat_temp     = 0;    // 加热温度: 默认40°C
u8  lb_attr_language      = 0;    // 语言: 默认中文
u8  lb_attr_fault         = 0;    // 故障: 默认正常
u8  lb_attr_heat_enable   = 0;    // 是否加热: 默认停止 (v1.0.5 新增 ID=10)
u32 lb_attr_mcu_version   = 0x76303031; // MCU固件版本号: 默认v001 (v1.0.7 新增 ID=13)

//-----------------------------------------------------------------------------
// 属性管理
//-----------------------------------------------------------------------------

/** @brief 读取属性值，供 DataPoint 编码用 */
static u8 lb_attr_read(u8 dpid, u8 *type, u32 *val)
{
    switch (dpid) {
    case LB_DPID_POWER_SWITCH:  *type = LB_DP_TYPE_BOOL;  *val = lb_attr_power_switch;  break;
    case LB_DPID_HEAT_MODE:     *type = LB_DP_TYPE_ENUM;  *val = lb_attr_heat_mode;     break;
    case LB_DPID_BATTERY:       *type = LB_DP_TYPE_ENUM;  *val = lb_attr_battery;       break;
    case LB_DPID_CHARGE_STATUS: *type = LB_DP_TYPE_ENUM;  *val = lb_attr_charge_status; break;
    case LB_DPID_HEAT_DURATION: *type = LB_DP_TYPE_VALUE; *val = lb_attr_heat_duration; break;
    case LB_DPID_REMAIN_TIME:   *type = LB_DP_TYPE_VALUE; *val = lb_attr_remain_time;   break;
    case LB_DPID_HEAT_TEMP:     *type = LB_DP_TYPE_ENUM;  *val = lb_attr_heat_temp;     break;
    case LB_DPID_LANGUAGE:      *type = LB_DP_TYPE_ENUM;  *val = lb_attr_language;      break;
    case LB_DPID_FAULT:         *type = LB_DP_TYPE_ENUM;  *val = lb_attr_fault;         break;
    case LB_DPID_HEAT_ENABLE:   *type = LB_DP_TYPE_BOOL;  *val = lb_attr_heat_enable;   break;
    case LB_DPID_MCU_VERSION:   *type = LB_DP_TYPE_VALUE; *val = lb_attr_mcu_version;   break;  // v1.0.7 新增
    default: return 0;
    }
    return 1;
}

/** @brief 解析 DataPoint 并写入属性（APP下发控制用） */
static u8 lb_attr_write(u8 *data, u16 len)
{
    u16 off = 0;
    while (off + 4 <= len) {
        u8  dpid    = data[off];
        u8  type    = data[off + 1];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];
        (void)type;  // 数据类型标记，暂未做校验（协议约定 bool=0x01, value=0x02, enum=0x04）
        if (off + 4 + val_len > len) break;
        u8 *val = data + off + 4;

        switch (dpid) {
        case LB_DPID_POWER_SWITCH:  if (val_len >= 1) lb_attr_power_switch  = val[0]; break;
        case LB_DPID_HEAT_MODE:     if (val_len >= 1) lb_attr_heat_mode     = val[0]; break;
        case LB_DPID_HEAT_DURATION: if (val_len >= 4) lb_attr_heat_duration = ((u32)val[0]<<24)|((u32)val[1]<<16)|((u32)val[2]<<8)|val[3]; break;
        case LB_DPID_HEAT_TEMP:     if (val_len >= 1) lb_attr_heat_temp     = val[0]; break;
        case LB_DPID_LANGUAGE:      if (val_len >= 1) lb_attr_language      = val[0]; break;
        case LB_DPID_HEAT_ENABLE:   if (val_len >= 1) lb_attr_heat_enable   = val[0]; break;  // v1.0.5 新增
        case LB_DPID_MCU_VERSION:   if (val_len >= 4) lb_attr_mcu_version   = ((u32)val[0]<<24)|((u32)val[1]<<16)|((u32)val[2]<<8)|val[3]; break;  // v1.0.7 新增
        default: break; // 只读属性 (3,4,6,9) 不允许 APP 写入
        }
        off += 4 + val_len;
    }
    return LB_ERR_SUCCESS;
}

#endif // !LB_BRIDGE_MODE

// ─── 以下变量/函数在桥模式和本地模式都需要 ───

// BLE→UART 转发时，记录原始 BLE 命令字（按 msg_flag 索引），
// UART 应答返回时用于确定正确的 BLE 响应命令字

// 模式信息（0x09 查询 / 0x0a 修改）：索引 1=自定义, 2=鸡腿, 3=意面, 4=预约, 5=保温
u8 lb_mode_temp[6]     = { 0, 3, 4, 5, 3, 5 };
u8 lb_mode_duration[6] = { 0, 30, 45, 20, 30, 0 };

// LB_KEEP_WARM_MODE/LB_KEEP_WARM_TEMP_F → 已移至 func_lunchbox_lcd.c
// lb_keep_warm_active/lb_heat_lcd_active → 已移至 func_lunchbox_lcd.c (extern via internal.h)
bool lb_heat_task_active;    /* 桥/本地：加热模块正在加热（含 UART 异步上报） */

/** @brief 从 UART DataPoint 同步加热任务状态 */
void lb_heating_sync_from_dp(u8 *data, u16 len)
{
    u16 off = 0;
    bool got_enable = false;
    bool heating = false;
    u32 remain = 0;
    bool got_remain = false;

    if (data == NULL || len == 0) {
        return;
    }
    while (off + 4 <= len) {
        u8  dpid    = data[off];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];
        if (off + 4 + val_len > len) {
            break;
        }
        u8 *val = data + off + 4;

        switch (dpid) {
        case LB_DPID_HEAT_ENABLE:
            if (val_len >= 1) {
                heating = (val[0] != 0);
                got_enable = true;
            }
            break;
        case LB_DPID_REMAIN_TIME:
            if (val_len >= 4) {
                remain = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                       | ((u32)val[2] << 8) | val[3];
                got_remain = true;
            }
            break;
        case LB_DPID_HEAT_MODE:
#if !LB_BRIDGE_MODE
            if (val_len >= 1) {
                lb_attr_heat_mode = val[0];
            }
#endif
            break;
        case LB_DPID_CHARGE_STATUS:
            /* 充电状态由 heat_display_feed_dp() 统一处理唤醒逻辑 */
            break;
        case LB_DPID_TIME_SYNC:
            /* 加热模块回传的时间戳 (dpid=11): 仅蓝牙连接时接受(此时APP已同步权威时间, lb_get_display_tm()优先用BLE时间);
             * 蓝牙未连接时忽略, 改用 dpid=14(RTC时间)作为加热模块时间源 */
            if (val_len >= 4 && ble_is_connected()) {
                lb_synced_heat_unix_ts = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                                       | ((u32)val[2] << 8) | val[3];
                lb_synced_heat_rtccnt = RTCCNT;
                lb_has_heat_ts = true;
            }
            break;
        case LB_DPID_RTC_TIME:
            /* 加热模块上报的 RTC 时间戳 (dpid=14): 未连蓝牙时作为显示时间源 (lb_get_display_tm() 优先级: APP > 加热模块 > 本地RTC) */
            if (val_len >= 4) {
                lb_synced_heat_unix_ts = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                                       | ((u32)val[2] << 8) | val[3];
                lb_synced_heat_rtccnt = RTCCNT;
                lb_has_heat_ts = true;
            }
            break;
        default:
            break;
        }
        off += 4 + val_len;
    }
#if !LB_BRIDGE_MODE
    if (got_enable) {
        lb_attr_heat_enable = heating ? 1 : 0;
    }
#endif
    if (got_enable && heating) {
        lb_heat_task_active = true;
    } else if (got_remain) {
#if ELUNCHBOX_PANEL_EN
        bool warm_hold = lb_keep_warm_active || (func_cb.sta == FUNC_NEW_WARM);
#else
        bool warm_hold = lb_keep_warm_active;
#endif
        if (remain > 0) {
            lb_heat_task_active = true;
        } else if (warm_hold) {
            /* 保温模式 Remain 恒为 0，勿因 DP06=0 清除保温/禁止息屏保护 */
            lb_heat_task_active = true;
            lb_heat_lcd_active = true;
#if ELUNCHBOX_PANEL_EN
            if (func_cb.sta == FUNC_NEW_WARM) {
                lb_keep_warm_active = true;
            }
#endif
        } else {
            lb_heat_task_active = false;
            lb_heat_lcd_active = false;
            lb_keep_warm_active = false;
        }
    } else if (got_enable && !heating && got_remain && remain == 0) {
#if ELUNCHBOX_PANEL_EN
        if (func_cb.sta != FUNC_NEW_WARM && !lb_keep_warm_active)
#endif
        {
            lb_heat_task_active = false;
            lb_heat_lcd_active = false;
            lb_keep_warm_active = false;
        }
    }
    /* UART 上报的 HEAT_ENABLE 同步到本地属性，避免 lunchbox_heating_task_active()
     * 因 lb_attr_heat_enable 过期而导致关机后 elunchbox_heating_blocks_idle 误唤醒 */
}

// lunchbox_heating_task_active / lunchbox_set_device_info → 已移至 func_lunchbox_lcd.c
// lunchbox_temp_f_to_idx / lunchbox_mode_get_* / lunchbox_get_heat_* → 已移至 func_lunchbox_lcd.c

/**
 * @brief 构造完整 UART 帧并发送到加热模块 (内部实现)
 * @return true=已入队(未发送), false=已发送或no_wait
 */
static bool lb_uart_send_internal(u8 uart_cmd, u8 *data, u16 data_len,
                                   u8 msg_flag, bool no_wait)
{
    if (lb_uart_tx_blocked) {
        return false;  /* 手动关机期间禁止 UART TX */
    }
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_LOWBAT_MODE_EN
    if (elunchbox_lowbat_active()) {
        return false;  /* 低电页：不下发任何 UART */
    }
#endif
#if ELUNCHBOX_PANEL_EN
    if (lb_ble_tx_fn == NULL && lb_uart_tx_skip_rx_only(uart_cmd)) {
        return false;
    }
#endif

    // 若已有指令等待加热模块回应 → 入队, 由 lb_uart_send_process() 后续处理
    if (lb_send_waiting) {
        if (lb_send_q_count < LB_SEND_QUEUE_SIZE) {
            lb_send_q_item_t *q = &lb_send_queue[lb_send_q_tail];
            q->cmd = uart_cmd;
            q->data_len = (data_len <= sizeof(q->data)) ? data_len : sizeof(q->data);
            if (data && q->data_len) memcpy(q->data, data, q->data_len);
            q->no_wait = no_wait;
            q->ble_cmd = lb_pending_ble_cmd[msg_flag];
            q->no_ble_report = lb_send_no_ble_report;
            lb_send_q_tail = (lb_send_q_tail + 1) % LB_SEND_QUEUE_SIZE;
            lb_send_q_count++;
        } else {
            printf("lb_uart_send_raw: queue full, drop cmd=0x%02X\n", uart_cmd);
        }
        return true;  // 已入队
    }

    // 构建帧并立即发送
    u8 buf[LB_TXBUF_SIZE];
    u16 off = 0;

    buf[off++] = (u8)(LB_FRAME_HEADER >> 8);  // 0x55
    buf[off++] = (u8)LB_FRAME_HEADER;          // 0xAA
    buf[off++] = LB_FRAME_VERSION;
    buf[off++] = msg_flag;                      // msg_flag
    buf[off++] = uart_cmd;
    buf[off++] = LB_ERR_SUCCESS;
    buf[off++] = (u8)(data_len >> 8);           // data_len 大端
    buf[off++] = (u8)(data_len & 0xFF);
    if (data && data_len) {
        memcpy(buf + off, data, data_len);
        off += data_len;
    }
    buf[off] = lb_checksum(buf, off);
    off++;

    // 按键通知: 跳过 TX 日志
    {
        const char *tx_src = (lb_pending_ble_cmd[msg_flag] != 0) ? "BLE->UART==>TX" : "LCD->UART==>TX";
        if (!lb_data_is_key_notify(data, data_len)) {
            printf("%s[%d]: ", tx_src, off);
            for (u16 i = 0; i < off; i++) printf("%02X ", buf[i]);
            printf("\n");
            lb_dp_dump_hex(data, data_len);
        }

        uart_bufs_tx(UART_TYPE_1, buf, off);

        // no_wait: 发完即走，不等待回应、不进入重试流程 (关机等关键指令用)
        if (no_wait) {
            printf("%s[no_wait]: cmd=0x%02X sent, skip response\n", tx_src, uart_cmd);
            return false;
        }
    }

    // 标记等待加热模块回应
    lb_send_waiting   = true;
    lb_send_wait_msg  = msg_flag;
    lb_send_retry     = 0;
    lb_send_tick      = tick_get();
    lb_send_cur_cmd   = uart_cmd;
    lb_send_cur_dlen  = (data_len <= sizeof(lb_send_cur_data)) ? data_len : sizeof(lb_send_cur_data);
    if (data && lb_send_cur_dlen) memcpy(lb_send_cur_data, data, lb_send_cur_dlen);
    return false;
}

/**
 * @brief LCD/屏幕发起的UART命令 (正常上报APP)
 */
void lb_uart_send_raw(u8 uart_cmd, u8 *data, u16 data_len, bool no_wait)
{
    u8 msg = lb_uart_raw_msg_flag;
    lb_send_no_ble_report = false;       // LCD命令默认允许上报APP
    if (!lb_uart_send_internal(uart_cmd, data, data_len, msg, no_wait)) {
        lb_uart_raw_msg_flag++;          // 仅实际发送时自增
    }
}

/**
 * @brief LCD按键/心跳专用 — 永不上报APP
 */
void lb_uart_send_raw_noreport(u8 uart_cmd, u8 *data, u16 data_len, bool no_wait)
{
    u8 msg = lb_uart_raw_msg_flag;
    lb_send_no_ble_report = true;        // 标记不上报APP
    if (!lb_uart_send_internal(uart_cmd, data, data_len, msg, no_wait)) {
        lb_uart_raw_msg_flag++;
    }
}

/**
 * @brief BLE发起的UART命令 — 追踪BLE来源用于响应路由
 */
void lb_uart_send_from_ble(u8 uart_cmd, u8 *data, u16 data_len,
                            u8 ble_cmd, u8 ble_msg_flag)
{
    lb_send_no_ble_report = false;       // BLE命令必须回复APP
    // lb_pending_ble_cmd 已由 lb_translate_ble_to_uart() 设置
    lb_uart_send_internal(uart_cmd, data, data_len, ble_msg_flag, false);
    // 注意: BLE命令不增 lb_uart_raw_msg_flag (使用BLE自己的msg_flag空间)
}

// lunchbox_heat_start/stop, keep_warm, key_notify, power_on/off, BLE callbacks,
// time_sync, reservation_send/delete → 已移至 func_lunchbox_lcd.c

#if !LB_BRIDGE_MODE

/** @brief 单属性写入辅助（前置声明） */
static void lb_attr_write_single(u8 dpid, u8 type, u8 *val, u16 val_len);

void lunchbox_set_attr_bool(u8 dpid, u8 val)  { lb_attr_write_single(dpid, LB_DP_TYPE_BOOL,  &val, 1); lunchbox_report_attr(dpid); }
void lunchbox_set_attr_enum(u8 dpid, u8 val)  { lb_attr_write_single(dpid, LB_DP_TYPE_ENUM,  &val, 1); lunchbox_report_attr(dpid); }
void lunchbox_set_attr_value(u8 dpid, u32 val) { u8 v[4]; v[0]=val>>24; v[1]=val>>16; v[2]=val>>8; v[3]=val&0xFF;
                                                 lb_attr_write_single(dpid, LB_DP_TYPE_VALUE, v, 4); lunchbox_report_attr(dpid); }

/** @brief 单属性写入辅助 */
static void lb_attr_write_single(u8 dpid, u8 type, u8 *val, u16 val_len)
{
    switch (dpid) {
    case LB_DPID_POWER_SWITCH:  if (val_len>=1) lb_attr_power_switch  = val[0]; break;
    case LB_DPID_BATTERY:       if (val_len>=1) lb_attr_battery       = val[0]; break;
    case LB_DPID_CHARGE_STATUS: if (val_len>=1) lb_attr_charge_status = val[0]; break;
    case LB_DPID_HEAT_MODE:     if (val_len>=1) lb_attr_heat_mode     = val[0]; break;
    case LB_DPID_HEAT_DURATION: if (val_len>=4) lb_attr_heat_duration = ((u32)val[0]<<24)|((u32)val[1]<<16)|((u32)val[2]<<8)|val[3]; break;
    case LB_DPID_REMAIN_TIME:   if (val_len>=4) lb_attr_remain_time   = ((u32)val[0]<<24)|((u32)val[1]<<16)|((u32)val[2]<<8)|val[3]; break;
    case LB_DPID_HEAT_TEMP:     if (val_len>=1) lb_attr_heat_temp     = val[0]; break;
    case LB_DPID_LANGUAGE:      if (val_len>=1) lb_attr_language      = val[0]; break;
    case LB_DPID_FAULT:         if (val_len>=1) lb_attr_fault         = val[0]; break;
    case LB_DPID_HEAT_ENABLE:   if (val_len>=1) lb_attr_heat_enable   = val[0]; break;  // v1.0.5 新增
    case LB_DPID_MCU_VERSION:   if (val_len>=4) lb_attr_mcu_version   = ((u32)val[0]<<24)|((u32)val[1]<<16)|((u32)val[2]<<8)|val[3]; break;  // v1.0.7 新增
    default: break;
    }
}

//-----------------------------------------------------------------------------
// 预约管理 (BLE 格式)
//-----------------------------------------------------------------------------

/** @brief 查找预约下标，-1 表示未找到 */
static int lb_schedule_find(u8 id)
{
    for (int i = 0; i < lb_schedule_count; i++)
        if (lb_schedules[i].id == id) return i;
    return -1;
}

/** @brief 分配下一个可用预约 ID (协议: 默认从 6 开始递增)
 *
 * 遍历 lb_schedules[] 跳过已占用 ID 和 ID 0，返回第一个空闲 ID 并自增。
 * 桥模式(LB_BRIDGE_MODE)下不可用 — 预约 ID 由 APP 分配。
 */
u8 lb_schedule_alloc_id(void)
{
    while (lb_schedule_find(lb_next_schedule_id) >= 0 || lb_next_schedule_id == 0)
        lb_next_schedule_id++;
    return lb_next_schedule_id++;
}

/** @brief 从帧数据解析 BLE 预约记录 (41B payload) */
static void lb_schedule_parse_ble(lb_schedule_ble_t *s, u8 *data)
{
    s->id       = data[0];
    memcpy(s->name, data + 1, 32);
    s->time     = ((u32)data[33] << 24) | ((u32)data[34] << 16) | ((u32)data[35] << 8) | data[36];
    s->temp     = data[37];
    s->duration = data[38];
    s->enabled  = data[39];
    s->repeat   = data[40];
}

/** @brief 将 BLE 预约记录编码到 buf（41 字节），返回写入长度 */
static u16 lb_schedule_encode_ble(u8 *buf, lb_schedule_ble_t *s)
{
    buf[0] = s->id;
    memcpy(buf + 1, s->name, 32);
    buf[33] = (u8)(s->time >> 24);
    buf[34] = (u8)(s->time >> 16);
    buf[35] = (u8)(s->time >> 8);
    buf[36] = (u8)(s->time & 0xFF);
    buf[37] = s->temp;
    buf[38] = s->duration;
    buf[39] = s->enabled;
    buf[40] = s->repeat;
    return 41;
}

//-----------------------------------------------------------------------------
// 状态上报
//-----------------------------------------------------------------------------

/** @brief 编码全部属性到 buf，返回总长度 */
static u16 lb_encode_all_attrs(u8 *buf)
{
    u16 off = 0;
    u32 val;
    u8  type;
    // ID 1~10 (v1.0.5 新增 ID=10)
    for (u8 dpid = 1; dpid <= 10; dpid++) {
        if (!lb_attr_read(dpid, &type, &val)) continue;
        switch (type) {
        case LB_DP_TYPE_BOOL:
        case LB_DP_TYPE_ENUM:
            off += lb_dp_encode(buf + off, dpid, type, (u8 *)&val, 1);
            break;
        case LB_DP_TYPE_VALUE: {
            u8 v[4] = { (u8)(val>>24), (u8)(val>>16), (u8)(val>>8), (u8)val };
            off += lb_dp_encode(buf + off, dpid, type, v, 4);
            break;
        }
        }
    }
    // ID=13 MCU版本号 (v1.0.7 新增)
    if (lb_attr_read(LB_DPID_MCU_VERSION, &type, &val)) {
        u8 v[4] = { (u8)(val>>24), (u8)(val>>16), (u8)(val>>8), (u8)val };
        off += lb_dp_encode(buf + off, LB_DPID_MCU_VERSION, type, v, 4);
    }
    return off;
}

/** @brief 编码单个属性到 buf，返回写入长度 */
static u16 lb_encode_one_attr(u8 *buf, u8 dpid)
{
    u32 val;
    u8  type;
    if (!lb_attr_read(dpid, &type, &val)) return 0;
    switch (type) {
    case LB_DP_TYPE_BOOL:
    case LB_DP_TYPE_ENUM:
        return lb_dp_encode(buf, dpid, type, (u8 *)&val, 1);
    case LB_DP_TYPE_VALUE: {
        u8 v[4] = { (u8)(val>>24), (u8)(val>>16), (u8)(val>>8), (u8)val };
        return lb_dp_encode(buf, dpid, type, v, 4);
    }
    }
    return 0;
}

void lunchbox_report_all_attrs(void)
{
    u8 buf[256];
    u16 len = lb_encode_all_attrs(buf);
    lunchbox_uart_send_async(LB_CMD_STATUS_REPORT, buf, len);
}

void lunchbox_report_attr(u8 dpid)
{
    u8 buf[16];
    u16 len = lb_encode_one_attr(buf, dpid);
    if (len) lunchbox_uart_send_async(LB_CMD_STATUS_REPORT, buf, len);
}

//-----------------------------------------------------------------------------
// BLE 命令处理器 (蓝牙通讯协议1.0.5.md)
//-----------------------------------------------------------------------------

#endif // !LB_BRIDGE_MODE

// ─── 0x01 产品信息在桥模式下也由 MCU 本地处理 ───

/**
 * @brief 0x01 — 查询产品信息（桥模式/本地模式均可用）
 *
 * APP 发送: [timestamp:4B]（v1.0.5: data_len=4, 大端unix时间戳）
 * MCU 返回: 81 字节设备信息
 *   布局: bt_name(16B) + version(8B) + model(10B) + MAC(6B) + SN(32B) + color(1B)
 *         + main_mcu_version(4B) + heat_module_version(4B)  (v1.0.7 新增)
 */
u8 lb_handler_product_info(lb_rx_frame_t *rx)
{
    u8 buf[81]; // 16+8+10+6+32+1+4+4 = 81
    u16 off = 0;

    // 保存 APP 发来的权威 Unix 时间戳 + 当时的 RTCCNT (大端 4B)
    if (rx->data && rx->data_len >= 4) {
        lb_synced_unix_ts = ((u32)rx->data[0] << 24) | ((u32)rx->data[1] << 16)
                          | ((u32)rx->data[2] << 8)  | rx->data[3];
        lb_synced_rtccnt  = RTCCNT;
        lb_has_ble_ts     = true;
    }

    // BLE 连接后首次收到 APP 时间戳应答 → 发送5个预设到加热模块
    if (lb_ble_presets_pending) {
        lb_ble_presets_pending = false;
        lunchbox_ble_send_presets();
    }

    // 蓝牙名称 16B
    memcpy(buf + off, lb_dev_info.bt_name, 16); off += 16;
    // 版本号 8B
    memcpy(buf + off, lb_dev_info.version, 8); off += 8;
    // 型号 10B
    memcpy(buf + off, lb_dev_info.model, 10); off += 10;
    // MAC 6B
    memcpy(buf + off, lb_dev_info.mac, 6); off += 6;
    // SN 32B
    memcpy(buf + off, lb_dev_info.sn, 32); off += 32;
    // 颜色 1B
    buf[off++] = lb_dev_info.color;
    // 主MCU固件版本 4B (v1.0.7 新增, 大端)
    buf[off++] = (u8)(lb_dev_info.main_mcu_version >> 24);
    buf[off++] = (u8)(lb_dev_info.main_mcu_version >> 16);
    buf[off++] = (u8)(lb_dev_info.main_mcu_version >> 8);
    buf[off++] = (u8)(lb_dev_info.main_mcu_version & 0xFF);
    // 加热模块固件版本 4B (v1.0.7 新增, 大端)
    buf[off++] = (u8)(lb_dev_info.heat_module_version >> 24);
    buf[off++] = (u8)(lb_dev_info.heat_module_version >> 16);
    buf[off++] = (u8)(lb_dev_info.heat_module_version >> 8);
    buf[off++] = (u8)(lb_dev_info.heat_module_version & 0xFF);

    lunchbox_uart_send_response(LB_CMD_PRODUCT_INFO, rx->msg_flag, LB_ERR_SUCCESS, buf, off);
    return LB_ERR_SUCCESS;
}

// lb_ota_get_target → 已移至 func_lunchbox_ota.c

void lunchbox_uart_reg_handler(u8 cmd, lb_cmd_handler_t h) { if (cmd < 16) cmd_handler[cmd] = h; }

/**
 * @brief 0x09 — 获取指定模式信息 (桥模式/本地模式均可用)
 *
 * APP 发送:
 *   - 无数据：查询全部 5 种模式（自定义/鸡腿/意面/预约/保温）
 *   - 1 字节 mode(1~5)：查询指定模式
 * MCU 返回: 每条 3 字节（模式标志 + 温度档位 + 加热时长）
 */
static u8 lb_handler_mode_query(lb_rx_frame_t *rx)
{
    u8 buf[15];  // 最多5条 × 3字节 = 15
    u16 off = 0;

    if (rx->data_len == 1 && rx->data && rx->data[0] >= 1 && rx->data[0] <= 5) {
        // 查询指定模式
        u8 mode = rx->data[0];
        buf[off++] = mode;
        buf[off++] = lb_mode_temp[mode];
        buf[off++] = lb_mode_duration[mode];
    } else {
        // 查询所有5种模式
        for (u8 m = 1; m <= 5; m++) {
            buf[off++] = m;
            buf[off++] = lb_mode_temp[m];
            buf[off++] = lb_mode_duration[m];
        }
    }

    lunchbox_uart_send_response(LB_CMD_MODE_QUERY, rx->msg_flag, LB_ERR_SUCCESS, buf, off);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x0a — 修改指定模式信息 (桥模式/本地模式均可用)
 *
 * APP 发送: 3 字节（模式标志 + 温度档位 + 加热时长）
 * 有效模式: 1=自定义, 2=鸡腿, 3=意面, 4=预约, 5=保温
 */
static u8 lb_handler_mode_modify(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 3) {
        lunchbox_uart_send_response(LB_CMD_MODE_MODIFY, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    u8 mode = rx->data[0];
    if (mode < 1 || mode > 5) {
        lunchbox_uart_send_response(LB_CMD_MODE_MODIFY, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

#if ELUNCHBOX_PANEL_EN
    lunchbox_mode_preset_local_set(mode, rx->data[1], rx->data[2]);
#else
    lb_mode_temp[mode]     = rx->data[1];
    lb_mode_duration[mode] = rx->data[2];
#endif
    lunchbox_uart_send_response(LB_CMD_MODE_MODIFY, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 注册所有协议命令的业务处理器
 *
 * 蓝牙通讯协议1.0.7.md 命令字 → 处理函数映射:
 *   0x01 → lb_handler_product_info   (查询产品信息)    [桥/本地]
 *   0x02 → lb_handler_dynamic_attr   (查询动态属性)    [仅本地]
 *   0x04 → lb_handler_control        (控制指令)        [仅本地]
 *   0x05 → lb_handler_schedule_list  (查询预约列表)    [仅本地]
 *   0x06 → lb_handler_schedule_add   (新增预约)        [仅本地]
 *   0x07 → lb_handler_schedule_modify(修改预约)        [仅本地]
 *   0x08 → lb_handler_schedule_delete(删除预约)        [仅本地]
 *   0x09 → lb_handler_mode_query     (获取模式信息)    [桥/本地]
 *   0x0a → lb_handler_mode_modify    (修改模式信息)    [桥/本地]
 *   0x0c → lb_handler_ota_start      (升级启动)        [桥/本地]
 *   0x0d → lb_handler_ota_data       (升级包传输)      [桥/本地]
 *   0x0e → lb_handler_ota_end        (升级结束)        [桥/本地]
 */

// 前向声明 (函数定义在 lunchbox_uart_init_handlers 之后)
// OTA handler: 桥模式和本地模式均需 (target=0x01 主单片机升级)
#if !LB_BRIDGE_MODE
static u8 lb_handler_dynamic_attr(lb_rx_frame_t *rx);
static u8 lb_handler_control(lb_rx_frame_t *rx);
static u8 lb_handler_schedule_list(lb_rx_frame_t *rx);
static u8 lb_handler_schedule_add(lb_rx_frame_t *rx);
static u8 lb_handler_schedule_modify(lb_rx_frame_t *rx);
static u8 lb_handler_schedule_delete(lb_rx_frame_t *rx);
#endif

void lunchbox_uart_init_handlers(void)
{
    // 桥模式和本地模式都需要
    lunchbox_uart_reg_handler(LB_CMD_PRODUCT_INFO,    lb_handler_product_info);
    lunchbox_uart_reg_handler(LB_CMD_MODE_QUERY,      lb_handler_mode_query);
    lunchbox_uart_reg_handler(LB_CMD_MODE_MODIFY,     lb_handler_mode_modify);
    // OTA 命令 (v1.0.7 §5): 主单片机(target=0x01)升级在桥模式/本地模式均需处理
    lunchbox_uart_reg_handler(LB_CMD_OTA_START,       lb_handler_ota_start);
    lunchbox_uart_reg_handler(LB_CMD_OTA_DATA,        lb_handler_ota_data);
    lunchbox_uart_reg_handler(LB_CMD_OTA_END,         lb_handler_ota_end);

#if !LB_BRIDGE_MODE
    // 仅本地模式
    lunchbox_uart_reg_handler(LB_CMD_DYNAMIC_ATTR,    lb_handler_dynamic_attr);
    lunchbox_uart_reg_handler(LB_CMD_CONTROL,         lb_handler_control);
    lunchbox_uart_reg_handler(LB_CMD_SCHEDULE_LIST,   lb_handler_schedule_list);
    lunchbox_uart_reg_handler(LB_CMD_SCHEDULE_ADD,    lb_handler_schedule_add);
    lunchbox_uart_reg_handler(LB_CMD_SCHEDULE_MODIFY, lb_handler_schedule_modify);
    lunchbox_uart_reg_handler(LB_CMD_SCHEDULE_DELETE, lb_handler_schedule_delete);
#endif
}

#if !LB_BRIDGE_MODE

/**
 * @brief 0x02 — 查询设备动态属性
 * APP 发送: 无数据 → MCU 返回所有 DataPoint (ID 1~10)
 */
static u8 lb_handler_dynamic_attr(lb_rx_frame_t *rx)
{
    u8 buf[256];
    u16 len = lb_encode_all_attrs(buf);
    lunchbox_uart_send_response(LB_CMD_DYNAMIC_ATTR, rx->msg_flag, LB_ERR_SUCCESS, buf, len);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x04 — 控制指令 (v1.0.5 新增)
 *
 * APP 发送: 多个 DataPoint 数据单元
 * MCU 行为:
 *   ① 解析 DataPoint 数组，调用 lb_attr_write() 写入属性
 *   ② 返回 ack（data_len=0，err_flag=成功/失败）
 *   ③ 属性变化后通过 0x03 状态上报异步通知 APP
 *
 * 可控制属性: ID=1(总开关), 2(模式), 5(加热时长), 7(加热温度), 8(语言), 10(是否加热)
 */
static u8 lb_handler_control(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len == 0) {
        lunchbox_uart_send_response(LB_CMD_CONTROL, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    u8 result = lb_attr_write(rx->data, rx->data_len);
    if (result != LB_ERR_SUCCESS) {
        lunchbox_uart_send_response(LB_CMD_CONTROL, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    lunchbox_uart_send_response(LB_CMD_CONTROL, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);

#if ELUNCHBOX_PANEL_EN
    if (!elunchbox_ui_is_live()) {
        return;
    }
    lunchbox_control_apply_power_switch(rx->data, rx->data_len);
    lunchbox_control_apply_panel(rx->data, rx->data_len);
#else
    lunchbox_control_apply_power_switch(rx->data, rx->data_len);
    lunchbox_control_apply_panel(rx->data, rx->data_len);
#endif

    // 属性变化后主动上报 APP
    lunchbox_report_all_attrs();

    // 同步推送 LCD 显示 (加热页/预约页可实时看到模式/温度/时长变化)
    if (rx->data && rx->data_len > 0) {
#if ELUNCHBOX_PANEL_EN
        if (elunchbox_ui_is_live()) {
#endif
        heat_display_feed_dp(rx->data, rx->data_len, rx->msg_flag);
#if ELUNCHBOX_PANEL_EN
        home_ui_shared_battery_feed_dp(rx->data, rx->data_len);
        }
#endif
    }
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x05 — 查询预约列表
 * APP 发送: 无数据 → MCU 逐条返回预约记录 (43B 每条: 总条数+序号+41B预约数据)
 */
static u8 lb_handler_schedule_list(lb_rx_frame_t *rx)
{
    if (lb_schedule_count == 0) {
        u8 empty[2] = { 0, 0 };
        lunchbox_uart_send_response(LB_CMD_SCHEDULE_LIST, rx->msg_flag, LB_ERR_SUCCESS, empty, 2);
        return LB_ERR_SUCCESS;
    }

    for (u8 i = 0; i < lb_schedule_count; i++) {
        u8 buf[43]; // 2(总条数+序号) + 41(预约数据)
        buf[0] = lb_schedule_count;
        buf[1] = i + 1;
        lb_schedule_encode_ble(buf + 2, &lb_schedules[i]);
        lunchbox_uart_send_response(LB_CMD_SCHEDULE_LIST, rx->msg_flag, LB_ERR_SUCCESS, buf, 43);
    }
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x06 — 新增预约
 *
 * APP 发送: 41 字节预约数据 (id+name+time+temp+duration+enabled+repeat)
 * MCU 返回: 分配的 ID（1 字节）
 */
static u8 lb_handler_schedule_add(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 41) {
        u8 fail = 0;
        lunchbox_uart_send_response(LB_CMD_SCHEDULE_ADD, rx->msg_flag, LB_ERR_EXEC_FAIL, &fail, 1);
        return LB_ERR_EXEC_FAIL;
    }

    if (lb_schedule_count >= LB_SCHEDULE_MAX) {
        u8 fail = 0;
        lunchbox_uart_send_response(LB_CMD_SCHEDULE_ADD, rx->msg_flag, LB_ERR_EXEC_FAIL, &fail, 1);
        return LB_ERR_EXEC_FAIL;
    }

    lb_schedule_ble_t *s = &lb_schedules[lb_schedule_count];
    memset(s, 0, sizeof(lb_schedule_ble_t));
    lb_schedule_parse_ble(s, rx->data);
    s->id = lb_schedule_alloc_id();  // MCU 自动分配 ID (>=6)
    lb_schedule_count++;

    u8 assigned_id = s->id;
    lunchbox_uart_send_response(LB_CMD_SCHEDULE_ADD, rx->msg_flag, LB_ERR_SUCCESS, &assigned_id, 1);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x07 — 修改预约
 * APP 发送: 41 字节预约数据 → MCU 返回成功/失败
 */
static u8 lb_handler_schedule_modify(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 41) {
        lunchbox_uart_send_response(LB_CMD_SCHEDULE_MODIFY, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    u8 id = rx->data[0];
    int idx = lb_schedule_find(id);
    if (idx < 0) {
        lunchbox_uart_send_response(LB_CMD_SCHEDULE_MODIFY, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    lb_schedule_parse_ble(&lb_schedules[idx], rx->data);
    lb_schedules[idx].id = id;  // 保持原 ID
    lunchbox_uart_send_response(LB_CMD_SCHEDULE_MODIFY, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x08 — 删除预约
 * APP 发送: [ID:1B] → MCU 返回成功/失败
 */
static u8 lb_handler_schedule_delete(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 1) {
        lunchbox_uart_send_response(LB_CMD_SCHEDULE_DELETE, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    u8 id = rx->data[0];
    int idx = lb_schedule_find(id);
    if (idx < 0) {
        lunchbox_uart_send_response(LB_CMD_SCHEDULE_DELETE, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    // 删除：将最后一条移到当前位置
    lb_schedule_count--;
    if (idx < lb_schedule_count)
        memcpy(&lb_schedules[idx], &lb_schedules[lb_schedule_count], sizeof(lb_schedule_ble_t));
    lunchbox_uart_send_response(LB_CMD_SCHEDULE_DELETE, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);
    return LB_ERR_SUCCESS;
}

#endif // !LB_BRIDGE_MODE

/**
 * @brief 0x0c — 升级启动 (v1.0.7)
 *
 * APP 发送: 5 字节 [target(1B)][firmware_size(4B, 大端)]
 * MCU 返回: 2 字节 [target(1B)][status(1B)]
 *   status: 0x00=收到升级指令, 0x01=擦除flash中, 0x02=擦除完成
 */
/**
 * @brief 本地模式: 初始化 5 个固定预约预设到 lb_schedules[]
 *
 * IDs 1~5 为出厂固定预设，不可删除/修改。
 * 后续用户/APP 新增预约 ID 从 6 开始 (lb_next_schedule_id = 6)。
 */
#if !LB_BRIDGE_MODE
static void lb_local_init_presets(void)
{
    // 计算下次触发时间 (北京时间)
    u32 rtc = RTCCNT;
    u32 today_midnight = rtc - (rtc % 86400);

    u32 ts_breakfast = today_midnight + 8*3600;
    if (ts_breakfast <= rtc) ts_breakfast += 86400;
    u32 ts_lunch = today_midnight + 10*3600 + 50*60;
    if (ts_lunch <= rtc) ts_lunch += 86400;
    u32 ts_dinner = today_midnight + 16*3600 + 30*60;
    if (ts_dinner <= rtc) ts_dinner += 86400;

    u8 temp_idx = 2;  // 149°F → 60°C

    // 预设1: 早餐 — 每天8:00, 149°F, 60min
    lb_schedules[0].id = 1;
    memcpy(lb_schedules[0].name, "\xe6\x97\xa9\xe9\xa4\x90", 6);  // "早餐"
    lb_schedules[0].time     = ts_breakfast + LB_RTC_UNIX_OFFSET;
    lb_schedules[0].temp     = temp_idx;
    lb_schedules[0].duration = 60;
    lb_schedules[0].enabled  = 0;   // 停止加热
    lb_schedules[0].repeat   = 0xff;

    // 预设2: 午餐 — 每天10:50, 149°F, 70min
    lb_schedules[1].id = 2;
    memcpy(lb_schedules[1].name, "\xe5\x8d\x88\xe9\xa4\x90", 6);  // "午餐"
    lb_schedules[1].time     = ts_lunch + LB_RTC_UNIX_OFFSET;
    lb_schedules[1].temp     = temp_idx;
    lb_schedules[1].duration = 70;
    lb_schedules[1].enabled  = 0;
    lb_schedules[1].repeat   = 0xff;

    // 预设3: 晚餐 — 每天16:30, 149°F, 90min
    lb_schedules[2].id = 3;
    memcpy(lb_schedules[2].name, "\xe6\x99\x9a\xe9\xa4\x90", 6);  // "晚餐"
    lb_schedules[2].time     = ts_dinner + LB_RTC_UNIX_OFFSET;
    lb_schedules[2].temp     = temp_idx;
    lb_schedules[2].duration = 90;
    lb_schedules[2].enabled  = 0;
    lb_schedules[2].repeat   = 0xff;

    // 预设4: 鸡腿模式 — 立即, 149°F, 60min
    lb_schedules[3].id = 4;
    memcpy(lb_schedules[3].name, "\xe9\xb8\xa1\xe8\x85\xbf\xe6\xa8\xa1\xe5\xbc\x8f", 12);  // "鸡腿模式"
    lb_schedules[3].time     = rtc + LB_RTC_UNIX_OFFSET;
    lb_schedules[3].temp     = temp_idx;
    lb_schedules[3].duration = 60;
    lb_schedules[3].enabled  = 0;
    lb_schedules[3].repeat   = 0xff;

    // 预设5: 意面模式 — 立即, 149°F, 60min
    lb_schedules[4].id = 5;
    memcpy(lb_schedules[4].name, "\xe6\x84\x8f\xe9\x9d\xa2\xe6\xa8\xa1\xe5\xbc\x8f", 12);  // "意面模式"
    lb_schedules[4].time     = rtc + LB_RTC_UNIX_OFFSET;
    lb_schedules[4].temp     = temp_idx;
    lb_schedules[4].duration = 60;
    lb_schedules[4].enabled  = 0;
    lb_schedules[4].repeat   = 0xff;

    lb_schedule_count = 5;
    printf("Local mode: 5 preset schedules initialized, next ID starts at %d\n", lb_next_schedule_id);
}
#endif

void lunchbox_uart_init(u32 baud)
{
    lb_uart_suspended = false;
    memset(lb_rx_buf, 0, sizeof(lb_rx_buf));
    lb_rx_idx = 0;
    memset(cmd_handler, 0, sizeof(cmd_handler));
    memset(lb_schedules, 0, sizeof(lb_schedules));
    lb_schedule_count = 0;
#if !LB_BRIDGE_MODE
    lb_local_init_presets();  // 初始化 5 个固定预约预设 (ID 1~5)
#endif

    // 默认设备信息
    memset(&lb_dev_info, 0, sizeof(lb_dev_info));
    u8 ble_addr[6];
    ble_get_local_bd_addr(ble_addr);   //mac
    sprintf(lb_dev_info.bt_name, "AR0MA-NY_%02X%02X", ble_addr[4], ble_addr[5]);
    memcpy(lb_dev_info.version, "01.00.00", 8);
    memcpy(lb_dev_info.model,  "SF101\0\0\0\0\0", 10);
    memcpy(lb_dev_info.mac, ble_addr, 6);
    lb_dev_info.main_mcu_version   = 0x76303031;  // "v001" 默认主MCU固件版本 (v1.0.7)
    lb_dev_info.heat_module_version = 0x00000000;  // 加热模块版本待查询 (v1.0.7)

    uart_t uart1;
    memset(&uart1, 0, sizeof(uart1));
    uart1.type   = UART_TYPE_1;
    uart1.tx_map = UT1TXMAP_G2_PB8;
    uart1.rx_map = UT1RXMAP_G2_PB9;
    uart1.baud   = baud;
    uart1.rx_isr = lb_dummy_isr;

    Err_Uart ret = bsp_uart1_init(&uart1, lb_ring_buf, sizeof(lb_ring_buf));
    if (ret != ERR_UART_SUCCESS) {
        printf("lb_uart: init fail, err=%d\n", ret); return;
    }
    printf("lb_uart: init ok TX=PB8 RX=PB9 baud=%d\n", baud);
}

void lunchbox_uart_suspend(void)
{
    if (lb_uart_suspended) {
        return;
    }
    lb_uart_saved_con = UART1CON;   /* 保存配置，resume 时恢复 */
    UART1CON = 0;
    /* 释放 PB8(TX)/PB9(RX) 从 UART1 功能回到 GPIO 模式。
     * FUNCMCON0: UT1TXMAP=bit24(27:24), UT1RXMAP=bit28(31:28) → CLEAR(0xf)
     * 否则 PB9 仍被 UART RX 占用，port_wakeup_init 的下降沿检测不生效，
     * 对方发数据的起始位无法唤醒芯片。 */
    FUNCMCON0 = (FUNCMCON0 & ~((0xf << 28) | (0xf << 24))) | (0xf << 28) | (0xf << 24);
    bsp_uart1_rxclr();
    lb_rx_idx = 0;
    lb_uart_suspended = true;
}

void lunchbox_uart_resume(void)
{
    if (!lb_uart_suspended) {
        return;
    }
    lunchbox_uart_init(LB_BAUD);
    lb_uart_suspended = false;
}

/**
 * @brief 发送队列处理: 超时重试 + 出队发送下一条
 *
 * 由 lunchbox_uart_process() 在主循环中调用。
 *
 * 逻辑:
 *   1. 等待回应中 → 检查超时:
 *        - 未超时 → 继续等待
 *        - 超时 + 重试次数未达上限 → 重建帧重发, retry++
 *        - 超时 + 重试次数已满 → 放弃当前指令, 从队列取下一个
 *   2. 未在等待 + 队列非空 → 出队, 构建帧, 发送, 标记等待
 */
static void lb_uart_send_process(void)
{
    // ── 1. 等待回应中: 检查超时 ──
    if (lb_send_waiting) {
        if (!tick_check_expire(lb_send_tick, LB_UART_CMD_INTERVAL_MS)) {
            return;  // 未超时, 继续等
        }

        // 超时: 判断是否重试
        if (lb_send_retry < LB_UART_CMD_MAX_RETRIES) {
            // 重试: 重建帧重新发送 (使用相同的 msg_flag)
            lb_send_retry++;
            u8 buf[LB_TXBUF_SIZE];
            u16 off = 0;

            buf[off++] = (u8)(LB_FRAME_HEADER >> 8);
            buf[off++] = (u8)LB_FRAME_HEADER;
            buf[off++] = LB_FRAME_VERSION;
            buf[off++] = lb_send_wait_msg;          // 重试用同一个 msg_flag
            buf[off++] = lb_send_cur_cmd;
            buf[off++] = LB_ERR_SUCCESS;
            buf[off++] = (u8)(lb_send_cur_dlen >> 8);
            buf[off++] = (u8)(lb_send_cur_dlen & 0xFF);
            if (lb_send_cur_dlen) {
                memcpy(buf + off, lb_send_cur_data, lb_send_cur_dlen);
                off += lb_send_cur_dlen;
            }
            buf[off] = lb_checksum(buf, off);
            off++;

            printf("UART==>TX[retry %u/%u]: ", lb_send_retry, LB_UART_CMD_MAX_RETRIES);
            for (u16 i = 0; i < off; i++) printf("%02X ", buf[i]);
            printf("\n");

            uart_bufs_tx(UART_TYPE_1, buf, off);
            lb_send_tick = tick_get();
        } else {
            // 重试次数已满: 放弃当前指令
            printf("UART==>TX: cmd=0x%02X no response after %u retries, skip\n",
                   lb_send_cur_cmd, LB_UART_CMD_MAX_RETRIES);
            // 清除BLE上下文: BLE命令超时不上报APP
            lb_pending_ble_cmd[lb_send_wait_msg] = 0;
            lb_send_no_ble_report = false;
            lb_send_waiting = false;
        }
        return;
    }

    // ── 2. 队列非空: 出队发送下一条 ──
    if (lb_send_q_count > 0) {
        lb_send_q_item_t *q = &lb_send_queue[lb_send_q_head];
        lb_send_q_head = (lb_send_q_head + 1) % LB_SEND_QUEUE_SIZE;
        lb_send_q_count--;

        // 恢复队列项的上下文
        lb_send_no_ble_report = q->no_ble_report;
        u8 msg_flag = lb_uart_raw_msg_flag;
        if (q->ble_cmd != 0) {
            lb_pending_ble_cmd[msg_flag] = q->ble_cmd;  // 重建BLE cmd映射
        }

        // 构建帧并发送
        u8 buf[LB_TXBUF_SIZE];
        u16 off = 0;

        buf[off++] = (u8)(LB_FRAME_HEADER >> 8);
        buf[off++] = (u8)LB_FRAME_HEADER;
        buf[off++] = LB_FRAME_VERSION;
        buf[off++] = msg_flag;
        buf[off++] = q->cmd;
        buf[off++] = LB_ERR_SUCCESS;
        buf[off++] = (u8)(q->data_len >> 8);
        buf[off++] = (u8)(q->data_len & 0xFF);
        if (q->data_len) {
            memcpy(buf + off, q->data, q->data_len);
            off += q->data_len;
        }
        buf[off] = lb_checksum(buf, off);
        off++;

        {
            const char *tx_src = (q->ble_cmd != 0) ? "BLE->UART==>TX" : "LCD->UART==>TX";
            if (!lb_data_is_key_notify(q->data, q->data_len)) {
                printf("%s[%d]: ", tx_src, off);
                for (u16 i = 0; i < off; i++) printf("%02X ", buf[i]);
                printf("\n");
                lb_dp_dump_hex(q->data, q->data_len);
            }

            uart_bufs_tx(UART_TYPE_1, buf, off);

            // no_wait: 发完即走，不等待回应 (关机等关键指令用)
            if (q->no_wait) {
                printf("%s[no_wait]: cmd=0x%02X sent (dequeued), skip response\n", tx_src, q->cmd);
                lb_uart_raw_msg_flag++;
            } else {
                // 标记等待回应
                lb_send_waiting  = true;
                lb_send_wait_msg = msg_flag;
                lb_send_retry    = 0;
                lb_send_tick     = tick_get();
                lb_send_cur_cmd  = q->cmd;
                lb_send_cur_dlen = q->data_len;
                if (q->data_len) memcpy(lb_send_cur_data, q->data, q->data_len);

                lb_uart_raw_msg_flag++;
            }
        }
    }
}

/**
 * @brief 主循环中周期调用的接收处理函数
 *
 * 接收状态机（lb_rx_idx 既作写入位置，也作状态指示）：
 *   lb_rx_idx == 0   等 0x55
 *   lb_rx_idx == 1   等 0xaa
 *   lb_rx_idx >= 2   接收帧体
 */
void lunchbox_uart_process(void)
{
    if (lb_uart_suspended) {
        return;
    }

    // v1.0.7: 0x01 查询超时保护 — 加热模块无应答时用缓存值直接回复 APP
    if (lb_product_info_pending
        && tick_check_expire(lb_product_info_pend_tick, 1000)) {
        lb_uart_sync_pending = false;
        lb_pending_ble_cmd[lb_product_info_msg_flag] = 0;
        {
            lb_rx_frame_t synth;
            memset(&synth, 0, sizeof(synth));
            synth.msg_flag = lb_product_info_msg_flag;
            synth.cmd      = LB_CMD_PRODUCT_INFO;
            synth.valid    = true;
            lb_handler_product_info(&synth);
        }
        lb_product_info_pending = false;
    }

    u8 ch;

    while (bsp_uart1_get_char(&ch)) {
        if (lb_rx_idx == 0) {
            if (ch != (u8)(LB_FRAME_HEADER >> 8))
                continue;
        }

        if (lb_rx_idx == 1) {
            if (ch != (u8)LB_FRAME_HEADER) {
                lb_rx_idx = 0;
                continue;
            }
        }

        if (lb_rx_idx < LB_RXBUF_SIZE) {
            lb_rx_buf[lb_rx_idx] = ch;
            lb_rx_idx++;
            lb_rx_ticks = tick_get();
        } else {
            lb_rx_reset();
        }
    }

    lb_timeout_check();

    while (lb_rx_idx >= sizeof(lb_frame_head_t) + 1) {
        if (!lb_frame_parse())
            break;
    }

    // 发送队列处理: 超时重试 + 出队发送下一条
    lb_uart_send_process();

    // 加热模块 OTA 状态机轮询 (超时检测/重试/继续发送)
    heat_ota_process();
}

void lb_uart_tx_block(bool block)
{
    lb_uart_tx_blocked = block;
    printf("lb_uart: TX %s\n", block ? "blocked (manual off)" : "unblocked");
}

bool lb_uart_tx_is_blocked(void)
{
    return lb_uart_tx_blocked;
}

//-----------------------------------------------------------------------------
// 自测
//-----------------------------------------------------------------------------

#if LB_SELFTEST_EN

static u8 lb_test_echo_handler(lb_rx_frame_t *rx)
{
    printf("lb_rx: cmd=0x%02X flag=0x%02X err=0x%02X dlen=%d",
           rx->cmd, rx->msg_flag, rx->err_flag, rx->data_len);
    if (rx->data && rx->data_len) {
        printf(" data=");
        for (u16 i = 0; i < rx->data_len; i++) {
            printf("%02X ", rx->data[i]);
        }
    }
    printf("\n");

    lunchbox_uart_send_response(rx->cmd, rx->msg_flag, LB_ERR_SUCCESS,
                                rx->data, rx->data_len);
    return LB_ERR_SUCCESS;
}

void func_lunchbox_uart_test(void)
{
    bsp_uart1_str_tx("LUNCHBOX_UART_OK\r\n");

    for (u8 c = 0; c < 255; c++) {
        lunchbox_uart_reg_handler(c, lb_test_echo_handler);
    }
}

#endif // LB_SELFTEST_EN

#endif // FUNC_LUNCHBOX_UART_EN
