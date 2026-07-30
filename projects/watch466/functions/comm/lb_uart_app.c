/**
 * @file    lb_uart_app.c
 * @brief   饭盒串口应用层 — 收发调度核心
 * @note    三层结构:
 *            底层收发  lb_uart_link   UART1 硬件 + RX 环形缓冲
 *            协议层    lb_proto       0x55AA 帧编解码 (纯函数)
 *            应用层    本文件         数据泵调度 + 帧处理入口
 *
 *          数据流:
 *            收: 主循环 lunchbox_uart_process() 从 link 取字节 → 喂协议解析器
 *                → 每解出一帧调 lb_uart_on_frame()
 *            发: lunchbox_uart_send_frame() 协议层组帧 → link 发出
 *
 *          帧处理 (lb_uart_on_frame): 桥应答配对回传 APP / 时间与产品信息配对 /
 *          dpid=14 分钟时间同步 / UI 状态与预约镜像 / 心跳应答;
 *          0x04 应答驱动加热模块 OTA 状态机。
 */
#include "include.h"
#include "lb_proto.h"
#include "lb_uart_link.h"
#include "lb_uart_app.h"
#include "lb_bridge.h"
#include "lb_ble_app.h"
#include "lb_ui_state.h"
#include "lb_heat_cmd.h"    // 开机/关机时序: power/stop/last_flag
#include "lb_uart_heat.h"   // 加热模块 OTA 状态机

/* 开机/关机时序 (实现在本文件靠后, lb_uart_on_frame 先用到) */
static bool lb_seq_on_frame(lb_rx_frame_t *rx);

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// BLE→UART 转发 (异步一发一收 + 超时重试)
//-----------------------------------------------------------------------------
#define LB_BRIDGE_RSP_TIMEOUT_MS    100   // 单次等应答超时
#define LB_BRIDGE_RETRY_MAX         2     // 超时重发次数 (最多共发 3 次)
#define LB_BRIDGE_QUEUE_DEPTH       4     // 排队条数 (队首在飞, 其余等待)
#define LB_BRIDGE_DATA_MAX          64    // 单条转发数据区上限 (最大业务帧 42B)

typedef struct {
    u8  ble_cmd;                   // 来源 BLE 命令字 (应答翻译回 APP 用)
    u8  msg_flag;                  // BLE 请求 msg_flag, 串口转发沿用, 应答按它配对
    u8  uart_cmd;                  // 转发的 UART 命令字
    u16 data_len;
    u8  data[LB_BRIDGE_DATA_MAX];  // 转发数据区 (重发需要)
} lb_bridge_req_t;

//-----------------------------------------------------------------------------
// 应用层状态
//-----------------------------------------------------------------------------
static struct {
    lb_proto_parser_t parser;      // UART 帧解析器 (协议层实例)
    bool suspended;                // 手动关机时 UART1 已关闭
    bool tx_blocked;               // 阻止所有 UART TX (手动关机期间)

    // BLE→UART 转发队列 (环形, 队首为在飞请求)
    lb_bridge_req_t brq[LB_BRIDGE_QUEUE_DEPTH];
    u8   br_head;                  // 队首下标
    u8   br_count;                 // 队列条数
    bool br_waiting;               // 队首已发出, 等加热模块应答
    u8   br_retries;               // 队首已重发次数
    u32  br_tick;                  // 队首发出时刻
} lb_uart;

//-----------------------------------------------------------------------------
// 发送
//-----------------------------------------------------------------------------

/**
 * @brief 组帧并从串口发出 (应用层唯一发送原语)
 * @return true=已发出, false=被阻断或组帧失败
 */
bool lunchbox_uart_send_frame(u8 cmd, u8 msg_flag, u8 err,
                              const u8 *data, u16 len)
{
    if (lb_uart.tx_blocked || lb_uart.suspended) {
        return false;
    }
    u8  buf[LB_TXBUF_SIZE];
    u16 total = lb_proto_build_frame(buf, cmd, msg_flag, err, data, len);
    if (!total) {
        return false;
    }
    // ── 发包日志 (与 RX 对称; 按键通知帧太频繁, 不打印) ──
    if (!lb_data_is_key_notify((u8 *)data, len)) {
        printf("UART<==TX[%d]: ", total);
        for (u16 i = 0; i < total; i++) printf("%02X ", buf[i]);
        printf("\n");
        if (cmd == LB_UART_CMD_DYNAMIC) {
            lb_dp_dump_hex(data, len);
        }
    }
    lb_link_tx(buf, total);
    return true;
}

/** @brief 手动关机期间阻止所有 UART TX; true=阻塞 false=恢复 */
void lb_uart_tx_block(bool block)
{
    lb_uart.tx_blocked = block;
    printf("lb_uart: TX %s\n", block ? "blocked (manual off)" : "unblocked");
}

bool lb_uart_tx_is_blocked(void)
{
    return lb_uart.tx_blocked;
}

//-----------------------------------------------------------------------------
// BLE→UART 转发: 队列 + 超时重试 + 应答回传
//-----------------------------------------------------------------------------

/** @brief 发出队首请求, 开始等应答 */
static void lb_bridge_send_head(void)
{
    lb_bridge_req_t *req = &lb_uart.brq[lb_uart.br_head];
    lunchbox_uart_send_frame(req->uart_cmd, req->msg_flag, LB_ERR_SUCCESS,
                             req->data_len ? req->data : NULL, req->data_len);
    lb_uart.br_waiting = true;
    lb_uart.br_tick = tick_get();
}

/** @brief 弹出队首, 有排队请求则接着发 */
static void lb_bridge_pop_next(void)
{
    lb_uart.br_head = (lb_uart.br_head + 1) % LB_BRIDGE_QUEUE_DEPTH;
    lb_uart.br_count--;
    lb_uart.br_waiting = false;
    lb_uart.br_retries = 0;
    if (lb_uart.br_count) {
        lb_bridge_send_head();
    }
}

/**
 * @brief 应答翻译成 BLE 帧回传 APP
 * @param rx  加热模块应答帧; NULL=重试耗尽, 回执行失败
 */
static void lb_bridge_ble_reply(lb_bridge_req_t *req, lb_rx_frame_t *rx)
{
    u8  data[LB_TXBUF_SIZE];
    u16 data_len = 0;
    u8  err = LB_ERR_EXEC_FAIL;

    if (rx) {
        err = rx->err_flag;
        if (!lb_translate_uart_data_to_ble(rx, req->ble_cmd, data, &data_len)) {
            data_len = 0;
        }
    }

    u8  frame[LB_TXBUF_SIZE];
    u16 total = lb_proto_build_frame(frame, req->ble_cmd, req->msg_flag, err,
                                     data_len ? data : NULL, data_len);
    if (!total) {
        return;
    }
    if (!lunchbox_ble_tx(frame, total)) {
        printf("lb_bridge: BLE tx unavailable\n");
        return;
    }
    printf("UART->BLE[%u]: ", total);
    for (u16 i = 0; i < total; i++) printf("%02X ", frame[i]);
    printf("\n");
}

/**
 * @brief 转发一条 BLE 请求到串口 (异步: 入队即返回, 应答到达后自动回传 APP)
 *
 * 一发一收: 队首在飞, 其余排队。应答超时 LB_BRIDGE_RSP_TIMEOUT_MS 重发,
 * 共尝试 1+LB_BRIDGE_RETRY_MAX 次, 仍无应答则回 APP 执行失败。
 * @return false=数据过长或队列满 (请求被丢弃)
 */
bool lb_bridge_forward(u8 ble_cmd, u8 ble_msg_flag, u8 uart_cmd,
                       const u8 *data, u16 len)
{
    if (len > LB_BRIDGE_DATA_MAX) {
        printf("lb_bridge: data %u > %u, drop\n", len, LB_BRIDGE_DATA_MAX);
        return false;
    }
    if (lb_uart.br_count >= LB_BRIDGE_QUEUE_DEPTH) {
        printf("lb_bridge: queue full, drop ble_cmd=0x%02X\n", ble_cmd);
        return false;
    }
    u8 slot = (lb_uart.br_head + lb_uart.br_count) % LB_BRIDGE_QUEUE_DEPTH;
    lb_bridge_req_t *req = &lb_uart.brq[slot];
    req->ble_cmd  = ble_cmd;
    req->msg_flag = ble_msg_flag;
    req->uart_cmd = uart_cmd;
    req->data_len = len;
    if (len) {
        memcpy(req->data, data, len);
    }
    lb_uart.br_count++;

    if (!lb_uart.br_waiting) {
        lb_bridge_send_head();
    }
    return true;
}

/** @brief 预约列表应答是多帧: 收到最后一条(seq>=total)才算完成 */
static bool lb_bridge_rsp_is_last(lb_bridge_req_t *req, lb_rx_frame_t *rx)
{
    if (req->ble_cmd == LB_CMD_SCHEDULE_LIST && rx->data && rx->data_len >= 44) {
        u8 total = rx->data[0];
        u8 seq   = rx->data[1];
        if (total > 0 && seq < total) {
            return false;
        }
    }
    return true;
}

/**
 * @brief 串口帧到达时的应答配对 (cmd + msg_flag 与在飞请求一致才算应答)
 * @return true=该帧是转发请求的应答, 已翻译回传 APP
 */
static bool lb_bridge_on_response(lb_rx_frame_t *rx)
{
    if (!lb_uart.br_waiting) {
        return false;
    }
    lb_bridge_req_t *req = &lb_uart.brq[lb_uart.br_head];
    if (rx->cmd != req->uart_cmd || rx->msg_flag != req->msg_flag) {
        return false;
    }

    lb_bridge_ble_reply(req, rx);
    if (lb_bridge_rsp_is_last(req, rx)) {
        lb_bridge_pop_next();
    } else {
        lb_uart.br_tick = tick_get();    // 多帧应答: 刷新超时, 继续等后续帧
    }
    return true;
}

/** @brief 主循环轮询: 队首应答超时 → 重发, 重试耗尽 → 回 APP 失败 */
static void lb_bridge_poll(void)
{
    if (!lb_uart.br_waiting) {
        return;
    }
    if (!tick_check_expire(lb_uart.br_tick, LB_BRIDGE_RSP_TIMEOUT_MS)) {
        return;
    }

    lb_bridge_req_t *req = &lb_uart.brq[lb_uart.br_head];
    if (lb_uart.br_retries < LB_BRIDGE_RETRY_MAX) {
        lb_uart.br_retries++;
        printf("lb_bridge: rsp timeout, retry %u/%u uart_cmd=0x%02X\n",
               lb_uart.br_retries, LB_BRIDGE_RETRY_MAX, req->uart_cmd);
        lb_bridge_send_head();
        return;
    }
    printf("lb_bridge: no rsp after %u tries, reply APP err (ble_cmd=0x%02X)\n",
           LB_BRIDGE_RETRY_MAX + 1, req->ble_cmd);
    lb_bridge_ble_reply(req, NULL);
    lb_bridge_pop_next();
}

//-----------------------------------------------------------------------------
// 接收: 帧处理入口 (骨架, 待填充)
//-----------------------------------------------------------------------------

/**
 * @brief 模块主动上报的 0x01 → 翻译成 BLE 0x03 状态上报推送 APP
 *
 * 仅处理不属于任何在等应答的帧 (桥接没认领的)。
 * 含本地指令(加热启停等)的应答 —— 屏幕侧操作后 APP 也能同步到最新状态。
 */
static void lb_report_forward_to_app(lb_rx_frame_t *rx)
{
    if (!ble_is_connected()) {
        return;                         // 未连接不翻译不发送, 省功耗
    }
    if (lb_data_is_key_notify(rx->data, rx->data_len)) {
        return;                         // 按键通知仅 MCU↔模块内部使用
    }

    u8  data[LB_TXBUF_SIZE];
    u16 data_len = 0;
    if (!lb_translate_uart_data_to_ble(rx, LB_CMD_STATUS_REPORT, data, &data_len)) {
        return;
    }
    lb_ble_send_async(LB_CMD_STATUS_REPORT, data, data_len);
}

/**
 * @brief 收到一条完整帧 (校验已通过, 主循环上下文)
 *
 * 每个命令一个 case, 业务处理逐个填充。
 * rx->data 指向解析器缓冲, 本函数返回后失效, 需保留须拷贝。
 * 应答用 lunchbox_uart_send_frame(rx->cmd, rx->msg_flag, ...) 回显 msg_flag。
 */
static void lb_uart_on_frame(lb_rx_frame_t *rx)
{
    // ── 收包日志 (按键通知帧不打印) ──
    if (!lb_data_is_key_notify(rx->data, rx->data_len)) {
        u16 total = lb_proto_frame_total(rx);
        printf("UART==>RX[%d]: ", total);
        for (u16 i = 0; i < total; i++) printf("%02X ", lb_uart.parser.buf[i]);
        printf("\n");
        if (rx->cmd == LB_UART_CMD_DYNAMIC) {
            lb_dp_dump_hex(rx->data, rx->data_len);
        }
    }

    // BLE 桥: 若是转发请求的应答, 翻译回传 APP (与本地业务处理不互斥)
    bool consumed = lb_bridge_on_response(rx);

    switch (rx->cmd) {
    case LB_UART_CMD_DYNAMIC:       // 0x01 动态属性上报/查询应答 (DataPoints)
        // 模块每分钟推送的权威时间 (dpid=14) → 本机跟随同步
        {
            u16 off = 0;
            while (rx->data && off + 4 <= rx->data_len) {
                u8  dpid    = rx->data[off];
                u16 val_len = ((u16)rx->data[off + 2] << 8) | rx->data[off + 3];
                if (off + 4 + val_len > rx->data_len) break;
                if (dpid == LB_DPID_RTC_TIME && val_len >= 4) {
                    const u8 *v = rx->data + off + 4;
                    lb_time_set_synced(((u32)v[0] << 24) | ((u32)v[1] << 16)
                                     | ((u32)v[2] << 8)  |  (u32)v[3]);
                }
                off += 4 + val_len;
            }
        }
        if (lb_seq_on_frame(rx)) {                     // 开机/关机时序应答 → 推进下一步
            consumed = true;
        }
        if (lb_ble_timesync_on_heat_frame(rx)) {       // 时间同步应答 → 保存本机时间
            consumed = true;
        }
        if (lb_ble_product_info_on_heat_frame(rx)) {   // 产品信息查询应答 → 回复 APP
            consumed = true;
        }
        lb_ui_state_feed_dp(rx->data, rx->data_len);   // 更新 UI 状态镜像
        // 谁的应答都不是 → 模块主动上报 (状态变化/故障), 翻译成 0x03 推送 APP
        if (!consumed) {
            lb_report_forward_to_app(rx);
        }
        break;

    case LB_UART_CMD_SCHEDULE:      // 0x02 预约列表应答: 逐帧填充列表镜像 (桥已回传 APP)
        lb_ui_schedules_feed_entry(rx->data, rx->data_len);
        break;

    case LB_UART_CMD_SCHEDULE_OP:   // 0x03 预约增/改/删应答: 生效则本地列表过期 (桥已回传 APP)
        if (rx->err_flag == LB_ERR_SUCCESS) {
            lb_ui_schedules_mark_dirty();
        }
        break;

    case LB_UART_CMD_OTA:           // 0x04 OTA 应答: 加热模块升级期间驱动状态机
        // 蓝牙协议 §7: 升级过程只回 APP 成功/失败, 0x04 应答不转发 APP
        if (heat_ota_is_active()) {
            heat_ota_uart_response(rx);
        }
        break;

    case LB_UART_CMD_HEARTBEAT:     // 0x05 心跳: 收到请求(0x00)回应答(0x01)
        // 链路保活, 保留实现: 不答会被模块记"蓝牙模组心跳超时"故障(fault=0x09)
        if (rx->data && rx->data_len >= 1 && rx->data[0] == 0x00) {
            u8 ack = 0x01;
            lunchbox_uart_send_frame(LB_UART_CMD_HEARTBEAT, rx->msg_flag,
                                     LB_ERR_SUCCESS, &ack, 1);
        }
        break;

    default:                        // 未知命令
        printf("UART: unknown cmd=0x%02X\n", rx->cmd);
        break;
    }
}

//-----------------------------------------------------------------------------
// 主循环调度
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 开机 / 关机时序 — 一发一等 (声明见 lb_uart_app.h)
//
// 两条时序共用一个状态机, 同一时刻只可能跑一条 (关机优先: 关机期间不再起开机)。
// 每一步发出后记下 msg_flag, 靠 lb_seq_on_frame() 配对模块应答推进;
// 超时也推进 —— 关机必须能走完, 不能因为模块不回话卡死。
//-----------------------------------------------------------------------------

#define LB_SEQ_ACK_TIMEOUT_MS   500     // 每步等应答上限
#define LB_BOOT_SEQ_DELAY_MS    500     // 上电后起开机时序的延时 (等模块自己就绪)

typedef enum {
    LB_SEQ_IDLE = 0,
    LB_SEQ_BOOT_WAIT,       // 上电延时中, 还没发第一条
    LB_SEQ_BOOT_POWER,      // 已发 power_on, 等应答
    LB_SEQ_OFF_STOP,        // 已发 stop, 等应答
    LB_SEQ_OFF_POWER,       // 已发 power_off, 等应答
    LB_SEQ_OFF_DONE,        // 关机时序结束 (终态, 不再变)
} lb_seq_state_t;

static struct {
    lb_seq_state_t state;
    u8   flag;              // 在飞命令的 msg_flag
    u32  tick;              // 该步发出时刻 (BOOT_WAIT 时为上电时刻)
    bool from_app;          // 关机是 APP 下发的
    u8   app_flag;          // APP 请求的 msg_flag
} lb_seq;

/** @brief APP 下发的关机: 每步模块应答后回一条 BLE 应答 (0x04 控制) */
static void lb_seq_reply_app(u8 err)
{
    if (!lb_seq.from_app) {
        return;
    }
    lb_ble_send_response(LB_CMD_CONTROL, lb_seq.app_flag, err, NULL, 0);
}

/** @brief 第二步: 发关机指令 */
static void lb_seq_send_power_off(void)
{
    lb_heat_cmd_power(false);
    lb_seq.flag  = lb_heat_cmd_last_flag();
    lb_seq.tick  = tick_get();
    lb_seq.state = LB_SEQ_OFF_POWER;
    printf("seq: power_off sent\n");
}

/** @brief 关机时序收场 */
static void lb_seq_shutdown_finish(void)
{
    lb_seq.state = LB_SEQ_OFF_DONE;
    printf("seq: shutdown sequence done\n");
}

bool lunchbox_shutdown_blocked(void)
{
    if (lb_ota_is_active()) {
        printf("seq: shutdown blocked (OTA in progress)\n");
        return true;
    }
#if CHARGE_EN
    if (CHARGE_DC_IN()) {               // 主 MCU 侧检测到 DC 插入
        printf("seq: shutdown blocked (charging, DC in)\n");
        return true;
    }
#endif
    // 饭盒的充电状态由加热模块经 DP4 上报: 1=充电中 2=已充满(线还插着)
    {
        lb_ui_state_t *st = lb_ui_state_get();
        if (st->valid && (st->charge == 1 || st->charge == 2)) {
            printf("seq: shutdown blocked (charging, DP4=%u)\n", st->charge);
            return true;
        }
    }
    return false;
}

void lunchbox_shutdown_abort(void)
{
    if (lb_seq.state == LB_SEQ_OFF_STOP || lb_seq.state == LB_SEQ_OFF_POWER
        || lb_seq.state == LB_SEQ_OFF_DONE) {
        lb_seq.state = LB_SEQ_IDLE;
        printf("seq: shutdown aborted\n");
    }
}

void lunchbox_shutdown_start(bool from_app, u8 app_flag)
{
    if (lb_seq.state == LB_SEQ_OFF_STOP || lb_seq.state == LB_SEQ_OFF_POWER
        || lb_seq.state == LB_SEQ_OFF_DONE) {
        return;                         // 已在关机流程里
    }
    if (lunchbox_shutdown_blocked()) {
        if (from_app) {
            lb_ble_send_response(LB_CMD_CONTROL, app_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        }
        return;
    }
    lb_seq.from_app = from_app;
    lb_seq.app_flag = app_flag;

    lb_heat_cmd_stop();                 // 第一步: 停止加热
    lb_seq.flag  = lb_heat_cmd_last_flag();
    lb_seq.tick  = tick_get();
    lb_seq.state = LB_SEQ_OFF_STOP;
    printf("seq: shutdown start (from_app=%u), stop sent\n", from_app);
}

bool lunchbox_shutdown_is_active(void)
{
    return lb_seq.state == LB_SEQ_OFF_STOP || lb_seq.state == LB_SEQ_OFF_POWER;
}

bool lunchbox_shutdown_is_done(void)
{
    return lb_seq.state == LB_SEQ_OFF_DONE;
}

/**
 * @brief 串口 0x01 帧到达 → 推进时序 (lb_uart_on_frame 调用)
 * @return true=该帧是时序在等的应答, 已消化
 */
static bool lb_seq_on_frame(lb_rx_frame_t *rx)
{
    if (rx->cmd != LB_UART_CMD_DYNAMIC || rx->msg_flag != lb_seq.flag) {
        return false;
    }

    switch (lb_seq.state) {
    case LB_SEQ_BOOT_POWER:
        printf("seq: power_on acked, query schedules\n");
        lb_ui_schedules_refresh();      // 0x02 由列表同步机负责发送/重试/收齐
        lb_seq.state = LB_SEQ_IDLE;
        return true;

    case LB_SEQ_OFF_STOP:
        printf("seq: stop acked\n");
        lb_seq_reply_app(rx->err_flag);
        lb_seq_send_power_off();
        return true;

    case LB_SEQ_OFF_POWER:
        printf("seq: power_off acked\n");
        lb_seq_reply_app(rx->err_flag);
        lb_seq_shutdown_finish();
        return true;

    default:
        return false;
    }
}

/** @brief 时序轮询: 上电起开机时序 + 各步超时推进 */
static void lb_seq_poll(void)
{
    switch (lb_seq.state) {
    case LB_SEQ_BOOT_WAIT:
        if (tick_check_expire(lb_seq.tick, LB_BOOT_SEQ_DELAY_MS)) {
            lb_heat_cmd_power(true);    // 第一步: 开机
            lb_seq.flag  = lb_heat_cmd_last_flag();
            lb_seq.tick  = tick_get();
            lb_seq.state = LB_SEQ_BOOT_POWER;
            printf("seq: boot power_on sent\n");
        }
        break;

    case LB_SEQ_BOOT_POWER:
        if (tick_check_expire(lb_seq.tick, LB_SEQ_ACK_TIMEOUT_MS)) {
            printf("seq: power_on no ack, query schedules anyway\n");
            lb_ui_schedules_refresh();
            lb_seq.state = LB_SEQ_IDLE;
        }
        break;

    case LB_SEQ_OFF_STOP:
        if (tick_check_expire(lb_seq.tick, LB_SEQ_ACK_TIMEOUT_MS)) {
            printf("seq: stop no ack, continue to power_off\n");
            lb_seq_send_power_off();    // 不回 APP: 模块没应答
        }
        break;

    case LB_SEQ_OFF_POWER:
        if (tick_check_expire(lb_seq.tick, LB_SEQ_ACK_TIMEOUT_MS)) {
            printf("seq: power_off no ack, shutdown anyway\n");
            lb_seq_shutdown_finish();
        }
        break;

    default:
        break;
    }
}

/**
 * @brief 主循环处理 (func_process 每轮调用)
 *
 * 边读边解: 每个字节凑齐一帧立即处理。
 */
void lunchbox_uart_process(void)
{
    if (lb_uart.suspended) {
        return;
    }

    u8 ch;
    lb_rx_frame_t rx;
    while (lb_link_getc(&ch)) {
        if (lb_proto_parser_feed(&lb_uart.parser, ch, &rx)) {
            lb_uart_on_frame(&rx);
        }
    }

    // 残帧超时: 半截帧之后 300ms 没有后续字节, 丢弃重新找帧头
    lb_proto_parser_timeout(&lb_uart.parser, LB_FRAME_TIMEOUT_MS);

    // BLE 转发请求的应答超时/重试
    lb_bridge_poll();

    // 预约列表同步: 标脏后自动重查 + 超时重发
    lb_ui_schedules_sync_process();

    // 开机/关机时序: 上电自动起开机流程 + 每步应答超时推进
    lb_seq_poll();

    // 加热模块 OTA 状态机轮询 (超时检测/重试/继续发送)
    heat_ota_process();

    // 收发层溢出诊断 (读清零, 正常应恒为 0)
    u16 ovf = lb_link_rx_overflow();
    if (ovf) {
        printf("lb_link: rx overflow, %u bytes dropped\n", ovf);
    }
}

//-----------------------------------------------------------------------------
// 生命周期
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 设备信息 — 0x01 产品信息查询的应答内容
// 加热模块版本号开机为 0, 由模块 0x01 应答里的 dpid=13 回填
//-----------------------------------------------------------------------------

lb_device_info_t lb_dev_info;

/** @brief 填默认设备信息 (SN/颜色出厂默认 0) */
static void lb_dev_info_init(void)
{
    u8 ble_addr[6];

    memset(&lb_dev_info, 0, sizeof(lb_dev_info));
    ble_get_local_bd_addr(ble_addr);
    sprintf(lb_dev_info.bt_name, "AR0MA-NY_%02X%02X", ble_addr[4], ble_addr[5]);
    memcpy(lb_dev_info.version, "01.00.00", 8);
    memcpy(lb_dev_info.model, "SF101\0\0\0\0\0", 10);
    memcpy(lb_dev_info.mac, ble_addr, 6);
    lb_dev_info.main_mcu_version    = 0x76303031;   // "v001" 主MCU固件版本
    lb_dev_info.heat_module_version = 0;
}

void lunchbox_uart_init(u32 baud)
{
    memset(&lb_uart, 0, sizeof(lb_uart));
    lb_proto_parser_reset(&lb_uart.parser);
    lb_dev_info_init();
    if (!lb_link_init(baud)) {
        return;
    }
    // 开机时序: 延时后自动发 power_on → 应答 → 查预约列表 (见 lb_seq_poll)
    lb_seq.state = LB_SEQ_BOOT_WAIT;
    lb_seq.tick  = tick_get();
}

void lunchbox_uart_suspend(void)
{
    if (lb_uart.suspended) {
        return;
    }
    lb_link_suspend();
    lb_proto_parser_reset(&lb_uart.parser);
    lb_uart.br_count = 0;              // 清转发队列, 未答请求随关机作废
    lb_uart.br_waiting = false;
    lb_uart.br_retries = 0;
    lb_uart.suspended = true;
}

void lunchbox_uart_resume(void)
{
    if (!lb_uart.suspended) {
        return;
    }
    // 注意: 不走 lunchbox_uart_init() — memset 会把 tx_blocked 一起清掉,
    // 而手动关机唤醒后 TX 须保持 blocked, 由调用方确认真开机后再解锁
    lb_uart.suspended = false;
    lb_proto_parser_reset(&lb_uart.parser);
    lb_link_init(LB_BAUD);
}

#endif // FUNC_LUNCHBOX_UART_EN
