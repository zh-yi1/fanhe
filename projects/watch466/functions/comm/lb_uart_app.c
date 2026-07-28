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
 *          帧的业务处理 (lb_uart_on_frame 内) 为骨架, 各命令 ZH TODO 待填充。
 *          仅心跳应答 (0x05) 保留实现 — 属链路保活, 不答会被模块记
 *          "蓝牙模组心跳超时" 故障 (MCU协议 §4.1.6 fault=0x09)。
 */
#include "include.h"
#include "lb_proto.h"
#include "lb_uart_link.h"
#include "lb_uart_app.h"

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// 应用层状态
//-----------------------------------------------------------------------------
static struct {
    lb_proto_parser_t parser;      // UART 帧解析器 (协议层实例)
    bool suspended;                // 手动关机时 UART1 已关闭
    bool tx_blocked;               // 阻止所有 UART TX (手动关机期间)
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
// 接收: 帧处理入口 (骨架, 待填充)
//-----------------------------------------------------------------------------

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

    switch (rx->cmd) {
    case LB_UART_CMD_DYNAMIC:       // 0x01 动态属性上报/查询应答 (DataPoints)
        // ZH TODO
        break;

    case LB_UART_CMD_SCHEDULE:      // 0x02 预约列表应答 (44B/条, 多帧)
        // ZH TODO
        break;

    case LB_UART_CMD_SCHEDULE_OP:   // 0x03 预约增/改/删应答
        // ZH TODO
        break;

    case LB_UART_CMD_OTA:           // 0x04 加热模块 OTA 应答
        // ZH TODO
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
