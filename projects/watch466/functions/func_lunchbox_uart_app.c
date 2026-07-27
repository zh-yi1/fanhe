/**
 * @file    func_lunchbox_uart_app.c
 * @brief   饭盒串口应用层 — 收发调度核心
 * @note    三层结构:
 *            底层收发  func_lunchbox_uart_link   UART1 硬件 + RX 环形缓冲
 *            协议层    func_lunchbox_proto       0x55AA 帧编解码 (纯函数)
 *            应用层    本文件                    数据泵调度 + 帧处理入口
 *
 *          数据流:
 *            收: 主循环 lunchbox_uart_process() 从 link 取字节 → 喂协议解析器
 *                → 每解出一帧调 lb_uart_on_frame()
 *            发: lunchbox_uart_send_frame() 协议层组帧 → link 发出
 *
 *          帧的业务处理 (lb_uart_on_frame 内) 按需求逐项添加, 当前仅打印。
 */
#include "include.h"
#include "func_lunchbox_uart.h"
#include "func_lunchbox_uart_link.h"

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

/** @brief 串口帧到达时的应答配对 (cmd + msg_flag 与在飞请求一致才算应答) */
static void lb_bridge_on_response(lb_rx_frame_t *rx)
{
    if (!lb_uart.br_waiting) {
        return;
    }
    lb_bridge_req_t *req = &lb_uart.brq[lb_uart.br_head];
    if (rx->cmd != req->uart_cmd || rx->msg_flag != req->msg_flag) {
        return;
    }

    lb_bridge_ble_reply(req, rx);
    if (lb_bridge_rsp_is_last(req, rx)) {
        lb_bridge_pop_next();
    } else {
        lb_uart.br_tick = tick_get();    // 多帧应答: 刷新超时, 继续等后续帧
    }
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
// 接收: 帧处理入口
//-----------------------------------------------------------------------------

/**
 * @brief 收到一条完整帧 (校验已通过, 主循环上下文)
 *
 * 业务处理按需求在此逐项添加 (心跳应答/状态同步/BLE转发/OTA 等)。
 * rx->data 指向解析器缓冲, 本函数返回后失效, 需保留须拷贝。
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
    lb_bridge_on_response(rx);

    switch (rx->cmd) {
    case LB_UART_CMD_DYNAMIC:       // 0x01 动态属性上报/查询应答
        // TODO: DP 解析 → 状态同步 / 喂 UI
        break;

    case LB_UART_CMD_SCHEDULE:      // 0x02 预约列表应答
        // TODO: 写入本地预约缓存
        break;

    case LB_UART_CMD_SCHEDULE_OP:   // 0x03 预约增/改/删应答
        // TODO: 操作结果确认
        break;

    case LB_UART_CMD_OTA:           // 0x04 OTA 应答
        // TODO: 加热模块 OTA 状态机
        break;

    case LB_UART_CMD_HEARTBEAT:     // 0x05 心跳请求
        // TODO: 回心跳应答
        break;

    default:                        // 未知命令
        // TODO: BLE 桥转发 / 丢弃
        break;
    }
}

//-----------------------------------------------------------------------------
// 主循环调度
//-----------------------------------------------------------------------------

/**
 * @brief 主循环处理 (func_process 每轮调用)
 *
 * 边读边解: 每个字节喂进解析器状态机, 帧凑齐立即处理。
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

    // 收发层溢出诊断 (读清零, 正常应恒为 0)
    u16 ovf = lb_link_rx_overflow();
    if (ovf) {
        printf("lb_link: rx overflow, %u bytes dropped\n", ovf);
    }
}

//-----------------------------------------------------------------------------
// 生命周期
//-----------------------------------------------------------------------------

void lunchbox_uart_init(u32 baud)
{
    memset(&lb_uart, 0, sizeof(lb_uart));
    lb_proto_parser_reset(&lb_uart.parser);
    if (!lb_link_init(baud)) {
        return;
    }
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
    // 而手动关机唤醒后 TX 须保持 blocked, 由主循环确认真开机后再解锁
    lb_uart.suspended = false;
    lb_proto_parser_reset(&lb_uart.parser);
    lb_link_init(LB_BAUD);
}

#endif // FUNC_LUNCHBOX_UART_EN
