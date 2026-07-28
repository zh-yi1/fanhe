/**
 * @file    func_lunchbox_ble_app.c
 * @brief   饭盒 BLE 应用层 — 接收链路唯一实现
 *
 * 取代原 func_lunchbox_ble.c，并接管原本由 ble_app_watch_process() 起头的
 * 数据分析流程。主循环只调用 lunchbox_ble_process() 一个入口。
 *
 * 三段职责严格分开，互不引用对方的内部状态:
 *
 *   §1 取包   lunchbox_ble_process()
 *             从平台环形缓冲区 (gatt_callback_app 填充) 把包一次取空，
 *             直接落到重组缓冲区尾部，不经过中间拷贝、不占栈。
 *
 *   §2 组帧   lb_ble_stream_*()
 *             字节流 → 完整帧。只认帧头/长度/校验和，不看命令字，
 *             不做任何业务判断。半帧等待、错帧重同步、残帧超时都在这层。
 *
 *   §3 分发   lb_ble_dispatch() 及各 lb_ble_on_*()
 *             帧 → 业务。一个命令一个函数，不再有 goto 和层层嵌套的 #if。
 *
 * 协议: 蓝牙通讯协议1.0.6.md (v1.0.10) §2 帧格式 / §3 命令字
 * 模式: MCU 作 BLE↔UART 翻译桥
 */
#include "include.h"
#include "app_blue_fit.h"
#include "func_lunchbox_uart.h"
#include "func_lunchbox_uart_link.h"
#include "func_lunchbox_uart_internal.h"
#include "func_lunchbox_ble_app.h"
#include "func_lunchbox_bridge.h"
#include "func_lunchbox_uart_heat.h"
#include "func_lunchbox_heat_cmd.h"
#include "func_lunchbox_ui_state.h"

//=============================================================================
// 配置
//=============================================================================

/** 重组缓冲区: 要能装下「一个残帧 + 一个整包」*/
#define LB_BLE_RX_BUF_SIZE      512

/** 平台单包最大字节数 (与 app_blue_fit.c 的 BLE_RX_BUF_LEN 一致) */
#define LB_BLE_RX_PKT_MAX       256

/** 残帧超时: 超过此时长没等到后续分包就丢弃, 防止半帧长期占着缓冲区头部 */
#define LB_BLE_FRAME_TIMEOUT_MS 3000

/** 最短帧长: 帧头(2)+版本(1)+消息标志(1)+命令字(1)+错误标志(1)+长度(2)+校验和(1) */
#define LB_BLE_FRAME_MIN_LEN    9

/** 逐包收发日志 (排查分包问题时打开; 帧级日志始终保留) */
#define LB_BLE_CHUNK_LOG_EN     0

//=============================================================================
// §2 组帧层 — 字节流 → 完整帧
//=============================================================================

static u8  ble_rx_buf[LB_BLE_RX_BUF_SIZE];  // 重组缓冲区
static u16 ble_rx_len;                      // 缓冲区中有效字节数
static u32 ble_rx_tick;                     // 最近一次写入时刻, 残帧超时用

static void lb_ble_dispatch(lb_rx_frame_t *rx);

/** @brief 丢弃缓冲区头部 n 字节 */
static void lb_ble_stream_drop(u16 n)
{
    if (n >= ble_rx_len) {
        ble_rx_len = 0;
        return;
    }
    memmove(ble_rx_buf, ble_rx_buf + n, ble_rx_len - n);
    ble_rx_len -= n;
}

/**
 * @brief 腾出至少 need 字节的写入空间
 *
 * 腾不出就丢缓冲区头部最旧的字节 —— 那些字节属于一个已经不可能补齐的残帧
 * (合法帧不会跨越 缓冲区容量-单包上限)。旧实现是把**新来的包**一起丢掉，
 * 结果是好数据陪着坏数据一起没了。
 */
static void lb_ble_stream_reserve(u16 need)
{
    u16 free_len = LB_BLE_RX_BUF_SIZE - ble_rx_len;

    if (free_len >= need) {
        return;
    }
    u16 drop = need - free_len;
    printf("BLE: rx buf full, drop %u stale bytes\n", drop);
    lb_ble_stream_drop(drop);
}

/** @brief 追加一段原始字节到重组缓冲区 */
static void lb_ble_stream_push(const u8 *data, u16 len)
{
    if (!data || len == 0) {
        return;
    }
    if (len > LB_BLE_RX_BUF_SIZE) {
        printf("BLE: rx chunk %u > buf %u, drop\n", len, LB_BLE_RX_BUF_SIZE);
        return;
    }
    lb_ble_stream_reserve(len);
    memcpy(ble_rx_buf + ble_rx_len, data, len);
    ble_rx_len += len;
    ble_rx_tick = tick_get();
}

/**
 * @brief 从缓冲区头部切出所有完整帧并分发
 *
 * 处理规则:
 *   帧头不符 / 校验失败 / 长度字段异常 → 丢 1 字节后重新找帧头
 *   数据未收齐                        → 原地等待下一个分包
 */
static void lb_ble_stream_pump(void)
{
    while (ble_rx_len >= LB_BLE_FRAME_MIN_LEN) {
        if (ble_rx_buf[0] != 0x55 || ble_rx_buf[1] != 0xAA) {
            lb_ble_stream_drop(1);
            continue;
        }

        u16 data_len = ((u16)ble_rx_buf[6] << 8) | ble_rx_buf[7];
        u32 total    = (u32)LB_BLE_FRAME_MIN_LEN + data_len;   // u32: 防 9+65535 溢出

        if (total > LB_BLE_RX_BUF_SIZE) {
            printf("BLE: bad data_len=%u, resync\n", data_len);
            lb_ble_stream_drop(1);
            continue;
        }
        if (ble_rx_len < total) {
            break;                                  // 等后续分包
        }
        if (lb_checksum(ble_rx_buf, (u16)(total - 1)) != ble_rx_buf[total - 1]) {
            printf("BLE: checksum err, resync\n");
            lb_ble_stream_drop(1);
            continue;
        }

        lb_rx_frame_t frame;
        frame.version  = ble_rx_buf[2];
        frame.msg_flag = ble_rx_buf[3];
        frame.cmd      = ble_rx_buf[4];
        frame.err_flag = ble_rx_buf[5];
        frame.data_len = data_len;
        frame.data     = data_len ? (ble_rx_buf + 8) : NULL;
        frame.valid    = true;

        // 帧级日志: 重组完成后才打印, 内容一定是一个完整帧
        // (旧实现在重组前按 data[4] 当命令字解析, 分包时打印的是假信息)
        printf("BLE==>RX[%lu]: ", (unsigned long)total);
        for (u16 i = 0; i < total; i++) {
            printf("%02X ", ble_rx_buf[i]);
        }
        printf("\n");
        lb_ble_dump_frame(frame.cmd, frame.data, frame.data_len, true);

        lb_ble_dispatch(&frame);
        lb_ble_stream_drop((u16)total);             // frame.data 到此失效
    }
}

/** @brief 残帧超时清理 — 由主循环轮询, 不再依赖"有新数据到达" */
static void lb_ble_stream_timeout(void)
{
    if (ble_rx_len > 0 && tick_check_expire(ble_rx_tick, LB_BLE_FRAME_TIMEOUT_MS)) {
        printf("BLE: rx timeout, drop %u stale bytes\n", ble_rx_len);
        ble_rx_len = 0;
    }
}

//=============================================================================
// §3 分发层 — 帧 → 业务
//=============================================================================

//-----------------------------------------------------------------------------
// 0x01 产品信息查询流程 (§3.1, v1.0.7 起需带加热模块版本号)
//
//   APP 0x01 → ①透传加热模块 (直发, 不进转发队列)
//            → ②模块 0x01 应答: dpid=13 回填加热模块版本 → 回 APP 81B 设备信息
//            → ②' 1 秒无应答: 用缓存版本号直接回 APP
//-----------------------------------------------------------------------------

#define LB_PRODUCT_INFO_TIMEOUT_MS  1000    // 等加热模块应答的窗口

static bool lb_product_info_pending;        // 已透传加热模块, 等其应答
static u8   lb_product_info_msg_flag;       // 待回复 APP 的 msg_flag
static u32  lb_product_info_pend_tick;      // 开始等待时刻

/** @brief 组 81B 设备信息回复 APP (§3.1 布局, 各字段大端) */
static void lb_ble_reply_product_info(u8 msg_flag)
{
    u8  info[81];   // bt_name(16)+version(8)+model(10)+mac(6)+sn(32)+color(1)+主版本(4)+模块版本(4)
    u16 off = 0;

    memcpy(info + off, lb_dev_info.bt_name, 16); off += 16;
    memcpy(info + off, lb_dev_info.version, 8);  off += 8;
    memcpy(info + off, lb_dev_info.model, 10);   off += 10;
    memcpy(info + off, lb_dev_info.mac, 6);      off += 6;
    memcpy(info + off, lb_dev_info.sn, 32);      off += 32;
    info[off++] = lb_dev_info.color;
    info[off++] = (u8)(lb_dev_info.main_mcu_version >> 24);
    info[off++] = (u8)(lb_dev_info.main_mcu_version >> 16);
    info[off++] = (u8)(lb_dev_info.main_mcu_version >> 8);
    info[off++] = (u8)(lb_dev_info.main_mcu_version);
    info[off++] = (u8)(lb_dev_info.heat_module_version >> 24);
    info[off++] = (u8)(lb_dev_info.heat_module_version >> 16);
    info[off++] = (u8)(lb_dev_info.heat_module_version >> 8);
    info[off++] = (u8)(lb_dev_info.heat_module_version);

    u8  frame[128];
    u16 total = lb_proto_build_frame(frame, LB_CMD_PRODUCT_INFO, msg_flag,
                                     LB_ERR_SUCCESS, info, off);
    if (total) {
        lunchbox_ble_tx(frame, total);
    }
}

/**
 * @brief 串口 0x01 帧到达时的产品信息应答配对 (uart_app 的 on_frame 调用)
 * @return true=该帧是产品信息查询的应答, 已消化 (勿再当模块主动上报转发 APP)
 */
bool lb_ble_product_info_on_heat_frame(lb_rx_frame_t *rx)
{
    if (!lb_product_info_pending) {
        return false;
    }
    if (rx->msg_flag != lb_product_info_msg_flag) {
        return false;                       // 模块自己的上报, 不是本次查询的应答
    }

    // 应答 DataPoints 里取加热模块固件版本 (dpid=13)
    if (rx->data) {
        u16 off = 0;
        while (off + 4 <= rx->data_len) {
            u8  dpid    = rx->data[off];
            u16 val_len = ((u16)rx->data[off + 2] << 8) | rx->data[off + 3];
            if (off + 4 + val_len > rx->data_len) {
                break;
            }
            if (dpid == LB_DPID_MCU_VERSION && val_len >= 4) {
                const u8 *v = rx->data + off + 4;
                lb_dev_info.heat_module_version = ((u32)v[0] << 24) | ((u32)v[1] << 16)
                                                | ((u32)v[2] << 8)  |  (u32)v[3];
            }
            off += 4 + val_len;
        }
    }

    lb_product_info_pending = false;
    lb_ble_reply_product_info(lb_product_info_msg_flag);
    return true;
}

/** @brief 超时保护: 模块无应答时用缓存版本号回复 APP (lunchbox_ble_process 轮询) */
static void lb_ble_product_info_poll(void)
{
    if (lb_product_info_pending
        && tick_check_expire(lb_product_info_pend_tick, LB_PRODUCT_INFO_TIMEOUT_MS)) {
        lb_product_info_pending = false;
        printf("product info: heat module no reply, use cached ver\n");
        lb_ble_reply_product_info(lb_product_info_msg_flag);
    }
}

//-----------------------------------------------------------------------------
// BLE 连接后的时间同步 + 三餐预设下发流程
//
//   连接 → ①向 APP 上报当前时间(0x03 dpid=11) = 获取权威时间请求
//        → ②APP 回时间戳(经 0x03 或 0x01) → 存时间 → time_sync 发加热模块
//        → ③模块应答该 msg_flag → 下发早/午/晚三餐预约预设 (每次连接一次)
//-----------------------------------------------------------------------------

static struct {
    bool wait_heat_ack;    // 已向模块发 time_sync, 等其应答
    u8   sync_flag;        // time_sync 的 msg_flag (配对应答用)
    bool presets_sent;     // 本次连接已发过三餐预设
    u8   report_flag;      // 上报 APP 的异步流水号
} lb_timesync;

/** @brief 计算下一个指定时分(北京时间)的 Unix 时间戳 */
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

/** @brief ③ 时间同步成功后: 下发早/午/晚三餐预约预设 (ID 1~3 固定) */
static void lb_ble_send_meal_presets(void)
{
    u8 temp_idx = lunchbox_temp_f_to_idx(149);

    lb_heat_cmd_schedule_set(1, 1, "\xe6\x97\xa9\xe9\xa4\x90",          // 早餐 8:00
                             lb_next_time_of_day(8, 0), temp_idx, 60, 0, 0xff);
    lb_heat_cmd_schedule_set(1, 2, "\xe5\x8d\x88\xe9\xa4\x90",          // 午餐 10:50
                             lb_next_time_of_day(10, 50), temp_idx, 70, 0, 0xff);
    lb_heat_cmd_schedule_set(1, 3, "\xe6\x99\x9a\xe9\xa4\x90",          // 晚餐 16:30
                             lb_next_time_of_day(16, 30), temp_idx, 90, 0, 0xff);
    printf("timesync: 3 meal presets sent to heat module\n");
}

/** @brief ② APP 权威时间到达 (0x03 dpid=11 或 0x01 数据区) */
static void lb_ble_accept_app_time(u32 unix_ts)
{
    lb_time_set_synced(unix_ts);

    // 同步给加热模块, 记 msg_flag 等它应答
    if (lb_heat_cmd_time_sync(unix_ts)) {
        lb_timesync.sync_flag = lb_heat_cmd_last_flag();
        lb_timesync.wait_heat_ack = true;
    }
}

/**
 * @brief 串口 0x01 帧到达时的时间同步应答配对 (uart_app 的 on_frame 调用)
 * @return true=该帧是时间同步的应答, 已消化 (勿再当模块主动上报转发 APP)
 */
bool lb_ble_timesync_on_heat_frame(lb_rx_frame_t *rx)
{
    if (!lb_timesync.wait_heat_ack) {
        return false;
    }
    if (rx->cmd != LB_UART_CMD_DYNAMIC || rx->msg_flag != lb_timesync.sync_flag) {
        return false;
    }
    lb_timesync.wait_heat_ack = false;
    printf("timesync: heat module acked\n");

    if (!lb_timesync.presets_sent) {
        lb_timesync.presets_sent = true;
        lb_ble_send_meal_presets();
    }
    return true;
}

/** @brief ① BLE 连接成功 (平台 app_blue_fit.c 调用) — 向 APP 请求权威时间 */
void lunchbox_ble_on_connected(void)
{
    lb_timesync.wait_heat_ack = false;
    lb_timesync.presets_sent = false;
    lb_product_info_pending = false;       // 上次连接的挂起查询作废

    // 上报当前时间戳(0x03 dpid=11), APP 收到后回权威时间戳
    u8 dp[8];
    u16 dlen = lb_dp_encode_value(dp, LB_DPID_TIME_SYNC, lb_get_unix_time());
    u8  frame[32];
    u16 total = lb_proto_build_frame(frame, LB_CMD_STATUS_REPORT,
                                     lb_timesync.report_flag, LB_ERR_SUCCESS, dp, dlen);
    lb_timesync.report_flag++;
    if (total && lunchbox_ble_tx(frame, total)) {
        printf("BLE connected: time request sent via 0x03\n");
    }

    // 断电恢复后延迟的 OTA 升级结果补报
    heat_ota_send_deferred_result();
}

/**
 * @brief 0x01 查询产品信息 (§3.1)
 *
 * 数据区带 4B 时间戳。先把 0x01 透传给加热模块 (v1.0.7 起要求同步给它),
 * 等 UART 应答里的加热模块版本号回来后再回复 APP; 转发不成立就直接本地回复。
 */
static void lb_ble_on_product_info(lb_rx_frame_t *rx)
{
    if (rx->data && rx->data_len >= 4) {
        u32 ts = ((u32)rx->data[0] << 24) | ((u32)rx->data[1] << 16)
               | ((u32)rx->data[2] << 8)  | rx->data[3];
        lb_ble_accept_app_time(ts);
    }

    lb_product_info_pending   = true;
    lb_product_info_msg_flag  = rx->msg_flag;
    lb_product_info_pend_tick = tick_get();

    u8  uart_data[LB_TXBUF_SIZE];
    u16 uart_dlen = 0;
    u8  uart_cmd = lb_ble_cmd_to_uart_cmd(rx->cmd);

    if (!uart_cmd || !lb_translate_ble_data_to_uart(rx, uart_data, &uart_dlen)) {
        lb_product_info_pending = false;
        lb_ble_reply_product_info(rx->msg_flag);    // 加热模块拿不到, 用本地缓存回复
        return;
    }

    u8  uart_buf[LB_TXBUF_SIZE];
    u16 uart_len = lb_proto_build_frame(uart_buf, uart_cmd, rx->msg_flag,
                                        LB_ERR_SUCCESS, uart_data, uart_dlen);
    if (!uart_len) {
        lb_product_info_pending = false;
        lb_ble_reply_product_info(rx->msg_flag);
        return;
    }

    printf("BLE->UART==>TX[%u]: ", uart_len);
    for (u16 i = 0; i < uart_len; i++) {
        printf("%02X ", uart_buf[i]);
    }
    printf("\n");
    if (uart_dlen) {
        lb_ble_dump_frame(rx->cmd, uart_data, uart_dlen, true);
    }
    // 注: 0x01 直发 UART, 不进转发队列 —— 与加热模块的应答配对靠
    //     lb_product_info_pending, 排队会打乱 1 秒超时窗口
    lb_link_tx(uart_buf, uart_len);
}

/**
 * @brief 0x03 状态上报 (§3.3) — APP 方向只用于回传权威时间戳 (dpid=11)
 *
 * 本地消化, 不转发给加热模块。
 */
static void lb_ble_on_status_report(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 8) {
        return;
    }

    u16 off = 0;
    while ((u32)off + 4 <= rx->data_len) {
        u8  dpid    = rx->data[off];
        u8  type    = rx->data[off + 1];
        u16 val_len = ((u16)rx->data[off + 2] << 8) | rx->data[off + 3];

        if ((u32)off + 4 + val_len > rx->data_len) {
            break;
        }
        if (dpid == LB_DPID_TIME_SYNC && type == LB_DP_TYPE_VALUE && val_len >= 4) {
            const u8 *v = rx->data + off + 4;
            u32 ts = ((u32)v[0] << 24) | ((u32)v[1] << 16) | ((u32)v[2] << 8) | v[3];
            printf("BLE: APP time sync via 0x03, ts=%lu\n", (unsigned long)ts);
            lb_ble_accept_app_time(ts);
            return;
        }
        off += 4 + val_len;
    }
}

/**
 * @brief 0x0c/0x0d/0x0e OTA (§5) — 按 target 字段分流
 *
 * target=0x01 主单片机 → 本地 OTA 处理器 (写自身 Flash)
 * target=0x02 加热模块 → 加热模块 OTA 处理器 (存 SPI Flash 后经 UART 转发)
 * target=0x00 (缺省)   → 按主单片机处理, 兼容不带 target 的老 APP
 */
static void lb_ble_on_ota(lb_rx_frame_t *rx)
{
    u8 target = lb_ota_get_target(rx);

    if (target == 0x00 || target == LB_OTA_TARGET_MAIN_MCU) {
        printf("OTA: target=0x%02X -> main mcu\n",
               target ? target : LB_OTA_TARGET_MAIN_MCU);
        switch (rx->cmd) {
        case LB_CMD_OTA_START:
            lb_handler_ota_start(rx);
            break;
        case LB_CMD_OTA_DATA:
            lb_handler_ota_data(rx);
            break;
        case LB_CMD_OTA_END:
            lb_handler_ota_end(rx);
            break;
        default:
            break;
        }
        return;
    }

    if (target == LB_OTA_TARGET_HEAT_MODULE) {
        printf("OTA: target=0x%02X -> heat module\n", target);
        switch (rx->cmd) {
        case LB_CMD_OTA_START:
            heat_ota_handler_start(rx, rx->msg_flag);
            break;
        case LB_CMD_OTA_DATA:
            heat_ota_handler_data(rx, rx->msg_flag);
            break;
        case LB_CMD_OTA_END:
            heat_ota_handler_end(rx, rx->msg_flag);
            break;
        default:
            break;
        }
        return;
    }

    printf("OTA: unknown target=0x%02X, ignore\n", target);
}

/**
 * @brief 0x09 获取指定模式信息 — 模式预设是手表本地数据, 不转发加热模块
 *
 * 协议 §3.9: APP 可查的模式仅 1~3 (自定义/鸡腿/意面);
 * 预约的参数跟每条预约走, 保温行为固定, 都不是可编辑模板。
 * APP 发送: 无数据=查询全部 3 种模式; 1 字节 mode(1~3)=查询指定模式
 * MCU 返回: 每条 3 字节 (模式标志 + 温度档位 + 加热时长)
 */
static void lb_ble_on_mode_query(lb_rx_frame_t *rx)
{
    u8  buf[9];     // 最多 3 条 × 3 字节
    u16 off = 0;

    if (rx->data && rx->data_len == 1
        && rx->data[0] >= LB_MODE_CUSTOM && rx->data[0] <= LB_MODE_PASTA) {
        u8 mode = rx->data[0];
        buf[off++] = mode;
        buf[off++] = lunchbox_mode_get_temp(mode);
        buf[off++] = lunchbox_mode_get_duration(mode);
    } else {
        for (u8 m = LB_MODE_CUSTOM; m <= LB_MODE_PASTA; m++) {
            buf[off++] = m;
            buf[off++] = lunchbox_mode_get_temp(m);
            buf[off++] = lunchbox_mode_get_duration(m);
        }
    }
    lb_ble_send_response(LB_CMD_MODE_QUERY, rx->msg_flag, LB_ERR_SUCCESS, buf, off);
}

/**
 * @brief 0x0a 修改指定模式信息 — 更新本地预设表后直接应答
 *
 * 协议 §3.10: APP 可改的模式仅 1~3, 越界回执行失败
 * APP 发送: 3 字节 (模式标志 + 温度档位 + 加热时长)
 */
static void lb_ble_on_mode_modify(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 3) {
        lb_ble_send_response(LB_CMD_MODE_MODIFY, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return;
    }
    u8 mode = rx->data[0];
    if (mode < LB_MODE_CUSTOM || mode > LB_MODE_PASTA) {
        lb_ble_send_response(LB_CMD_MODE_MODIFY, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return;
    }
    lunchbox_mode_preset_local_set(mode, rx->data[1], rx->data[2]);
    lb_ble_send_response(LB_CMD_MODE_MODIFY, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);
}

/**
 * @brief 其余命令 — 翻译后经转发队列发往加热模块
 *
 * 0x02 查询动态属性 / 0x04 控制 / 0x05~0x08 预约
 * 异步: 入队即返回。应答到达/重试耗尽后由 lb_bridge 自动回传 APP。
 */
static void lb_ble_on_forward(lb_rx_frame_t *rx)
{

    u8  uart_data[LB_TXBUF_SIZE];
    u16 uart_dlen = 0;
    u8  uart_cmd = lb_ble_cmd_to_uart_cmd(rx->cmd);

    if (!uart_cmd || !lb_translate_ble_data_to_uart(rx, uart_data, &uart_dlen)) {
        printf("BLE: cmd=0x%02X not forwardable\n", rx->cmd);
        return;
    }

    lb_bridge_forward(rx->cmd, rx->msg_flag, uart_cmd, uart_data, uart_dlen);
}

/** @brief 命令分发 */
static void lb_ble_dispatch(lb_rx_frame_t *rx)
{
    switch (rx->cmd) {
    case LB_CMD_PRODUCT_INFO:                   // 0x01
        lb_ble_on_product_info(rx);
        break;

    case LB_CMD_STATUS_REPORT:                  // 0x03
        lb_ble_on_status_report(rx);
        break;

    case LB_CMD_MODE_QUERY:                     // 0x09 模式预设是本地数据
        lb_ble_on_mode_query(rx);
        break;

    case LB_CMD_MODE_MODIFY:                    // 0x0a 同上
        lb_ble_on_mode_modify(rx);
        break;

    case LB_CMD_OTA_START:                      // 0x0c
    case LB_CMD_OTA_DATA:                       // 0x0d
    case LB_CMD_OTA_END:                        // 0x0e
        lb_ble_on_ota(rx);
        break;

    default:                                    // 0x02/0x04~0x08
        lb_ble_on_forward(rx);
        break;
    }
}

//=============================================================================
// §1 取包层 + 对外接口
//=============================================================================

void lunchbox_ble_process(void)
{
    for (;;) {
        // 先保证尾部有一个整包的空间, 否则 rx_pop 会把超出部分截断丢掉
        lb_ble_stream_reserve(LB_BLE_RX_PKT_MAX);

        u16 n = ble_app_lunchbox_rx_pop(ble_rx_buf + ble_rx_len, LB_BLE_RX_PKT_MAX);
        if (n == 0) {
            break;                              // 队列已空
        }
#if LB_BLE_CHUNK_LOG_EN
        printf("BLE: chunk %u bytes (buf %u)\n", n, ble_rx_len);
#endif
        ble_rx_len += n;
        ble_rx_tick = tick_get();

        lb_ble_stream_pump();
    }

    lb_ble_stream_timeout();
    lb_ble_product_info_poll();
}

void lunchbox_ble_rx_handle(u8 *data, u16 len)
{
    lb_ble_stream_push(data, len);
    lb_ble_stream_pump();
}

bool lunchbox_ble_rx_pending(void)
{
    return ble_rx_len > 0;
}

// BLE 发送通道 (平台侧连接后注册)
static lb_ble_tx_fn_t lb_ble_tx_fn;

void lunchbox_ble_set_tx_fn(lb_ble_tx_fn_t fn)
{
    lb_ble_tx_fn = fn;
}

/** @brief 发送一条完整 BLE 帧给 APP; false=通道未注册(未连接) */
bool lunchbox_ble_tx(u8 *frame, u16 len)
{
    if (!lb_ble_tx_fn) {
        return false;
    }
    lb_ble_tx_fn(frame, len);
    return true;
}

/** @brief 组帧并应答 APP (echo 请求的 msg_flag; OTA 等本地处理的业务用) */
bool lb_ble_send_response(u8 ble_cmd, u8 msg_flag, u8 err, const u8 *data, u16 len)
{
    u8  frame[LB_TXBUF_SIZE];
    u16 total = lb_proto_build_frame(frame, ble_cmd, msg_flag, err, data, len);
    if (!total) {
        return false;
    }
    return lunchbox_ble_tx(frame, total);
}

static u8 lb_ble_async_flag;    // MCU 主动推送的自增流水号

/** @brief 组帧并主动推送 APP (MCU 发起, msg_flag 自增) */
bool lb_ble_send_async(u8 ble_cmd, const u8 *data, u16 len)
{
    bool ok = lb_ble_send_response(ble_cmd, lb_ble_async_flag, LB_ERR_SUCCESS, data, len);
    lb_ble_async_flag++;
    return ok;
}

