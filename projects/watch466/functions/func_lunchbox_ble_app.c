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
 * 模式: 仅桥模式 (LB_BRIDGE_MODE=1) — MCU 作 BLE↔UART 翻译桥
 */
#include "include.h"
#include "app_blue_fit.h"
#include "func_lunchbox_uart.h"
#include "func_lunchbox_uart_internal.h"
#include "func_lunchbox_ble_app.h"
#include "func_lunchbox_bridge.h"
#include "func_lunchbox_uart_heat.h"
#include "func_lunchbox_lcd.h"

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

/**
 * @brief 记录 APP 下发的权威时间戳
 *
 * 收到后若有挂起的预设下发请求, 立即补发 (预设的触发时间依赖正确的时基)。
 */
static void lb_ble_accept_app_time(u32 unix_ts)
{
    lb_synced_unix_ts = unix_ts;
    lb_synced_rtccnt  = RTCCNT;
    lb_has_ble_ts     = true;

    if (lb_ble_presets_pending) {
        lb_ble_presets_pending = false;
        lunchbox_ble_send_presets();
    }
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

    u8  uart_buf[LB_TXBUF_SIZE];
    u16 uart_len = 0;

    if (!lb_translate_ble_to_uart(rx, uart_buf, &uart_len)) {
        lb_product_info_pending = false;
        lb_handler_product_info(rx);            // 加热模块拿不到, 用本地缓存回复
        return;
    }

    printf("BLE->UART==>TX[%u]: ", uart_len);
    for (u16 i = 0; i < uart_len; i++) {
        printf("%02X ", uart_buf[i]);
    }
    printf("\n");
    {
        u16 dl = ((u16)uart_buf[6] << 8) | uart_buf[7];
        if (dl) {
            lb_ble_dump_frame(rx->cmd, uart_buf + 8, dl, true);
        }
    }
    // 注: 0x01 直发 UART, 不进重试队列 —— 与加热模块的应答配对靠
    //     lb_product_info_pending, 排队会打乱 1 秒超时窗口
    uart_bufs_tx(UART_TYPE_1, uart_buf, uart_len);
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
        if (rx->cmd < 16 && cmd_handler[rx->cmd]) {
            cmd_handler[rx->cmd](rx);
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
 * @brief 其余命令 — 翻译成 UART 帧转发加热模块
 *
 * 0x02 查询动态属性 / 0x04 控制 / 0x05~0x08 预约 / 0x09~0x0a 模式
 * 应答由 UART 侧收到回帧后按 msg_flag 配对回传 APP。
 */
static void lb_ble_on_forward(lb_rx_frame_t *rx)
{
#if ELUNCHBOX_PANEL_EN
    // 0x0a 修改模式预设: 本地也存一份, 供后续 0x04 跳加热页时取用
    if (rx->cmd == LB_CMD_MODE_MODIFY && rx->data && rx->data_len >= 3) {
        lunchbox_mode_preset_local_set(rx->data[0], rx->data[1], rx->data[2]);
    }
#endif

    u8  uart_buf[LB_TXBUF_SIZE];
    u16 uart_len = 0;

    if (!lb_translate_ble_to_uart(rx, uart_buf, &uart_len)) {
        printf("BLE: cmd=0x%02X not forwardable\n", rx->cmd);
        return;
    }

    u16 uart_dlen = ((u16)uart_buf[6] << 8) | uart_buf[7];
    lb_uart_send_from_ble(uart_buf[4],
                          uart_dlen ? uart_buf + 8 : NULL, uart_dlen,
                          rx->cmd, rx->msg_flag);
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

    case LB_CMD_OTA_START:                      // 0x0c
    case LB_CMD_OTA_DATA:                       // 0x0d
    case LB_CMD_OTA_END:                        // 0x0e
        lb_ble_on_ota(rx);
        break;

    default:                                    // 0x02/0x04~0x0a
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

void lunchbox_ble_rx_reset(void)
{
    ble_rx_len  = 0;
    ble_rx_tick = 0;
}

void lunchbox_ble_set_tx_fn(lb_ble_tx_fn_t fn)
{
    lb_ble_tx_fn = fn;
}

