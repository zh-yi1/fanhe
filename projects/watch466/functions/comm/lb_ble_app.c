/**
 * @file    lb_ble_app.c
 * @brief   饭盒 BLE 应用层 — 接收链路唯一实现
 *
 * 主循环只调用 lunchbox_ble_process() 一个入口。
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
 *   §3 分发   lb_ble_dispatch()
 *             帧 → 业务。当前为骨架, 各命令处理待填充 (ZH TODO)。
 *
 * 协议: 蓝牙通讯协议1.0.6.md §2 帧格式 / §3 命令字
 */
#include "include.h"
#include "app_blue_fit.h"
#include "lb_proto.h"
#include "lb_ble_app.h"

#if FUNC_LUNCHBOX_UART_EN

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
 * (合法帧不会跨越 缓冲区容量-单包上限)。只丢头部旧字节, 保住新来的好包。
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

/** @brief 残帧超时清理 — 由主循环轮询, 不依赖"有新数据到达" */
static void lb_ble_stream_timeout(void)
{
    if (ble_rx_len > 0 && tick_check_expire(ble_rx_tick, LB_BLE_FRAME_TIMEOUT_MS)) {
        printf("BLE: rx timeout, drop %u stale bytes\n", ble_rx_len);
        ble_rx_len = 0;
    }
}

//=============================================================================
// §3 分发层 — 帧 → 业务 (骨架, 待填充)
//=============================================================================

/**
 * @brief 命令分发 — 每个命令一个 case, 业务处理逐个填充
 *
 * rx->data 指向重组缓冲区内部, 仅在本次分发期间有效, 需要保留须自行拷贝。
 * 应答用 lb_ble_send_response(rx->cmd, rx->msg_flag, ...) 回显 msg_flag。
 */
static void lb_ble_dispatch(lb_rx_frame_t *rx)
{
    switch (rx->cmd) {
    case LB_CMD_PRODUCT_INFO:       // 0x01 查询产品信息 (数据区带 4B 时间戳)
        // ZH TODO: 应答 81B 设备信息
        break;

    case LB_CMD_DYNAMIC_ATTR:       // 0x02 查询设备动态属性
        // ZH TODO
        break;

    case LB_CMD_STATUS_REPORT:      // 0x03 状态上报 (APP→MCU 方向: 回传权威时间戳 dpid=11)
        // ZH TODO
        break;

    case LB_CMD_CONTROL:            // 0x04 控制指令 (DataPoints 修改属性)
        // ZH TODO
        break;

    case LB_CMD_SCHEDULE_LIST:      // 0x05 查询预约列表
        // ZH TODO
        break;

    case LB_CMD_SCHEDULE_ADD:       // 0x06 新增预约
        // ZH TODO
        break;

    case LB_CMD_SCHEDULE_MODIFY:    // 0x07 修改预约
        // ZH TODO
        break;

    case LB_CMD_SCHEDULE_DELETE:    // 0x08 删除预约
        // ZH TODO
        break;

    case LB_CMD_MODE_QUERY:         // 0x09 获取指定模式信息
        // ZH TODO
        break;

    case LB_CMD_MODE_MODIFY:        // 0x0a 修改指定模式信息
        // ZH TODO
        break;

    case LB_CMD_OTA_START:          // 0x0c 升级启动 (data[0]=target: 0x01主MCU/0x02加热模块)
        // ZH TODO
        break;

    case LB_CMD_OTA_DATA:           // 0x0d 升级包传输
        // ZH TODO
        break;

    case LB_CMD_OTA_END:            // 0x0e 升级结束
        // ZH TODO
        break;

    default:
        printf("BLE: unknown cmd=0x%02X\n", rx->cmd);
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

/** @brief BLE 连接成功回调 (平台 app_blue_fit.c 调用) */
void lunchbox_ble_on_connected(void)
{
    // ZH TODO: 连接后的业务 (如向 APP 请求权威时间)
    printf("BLE connected\n");
}

//=============================================================================
// 发送接口 (MCU → APP)
//=============================================================================

// BLE 发送通道 (平台侧初始化时注册)
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

/** @brief 组帧并应答 APP (echo 请求的 msg_flag) */
bool lb_ble_send_response(u8 ble_cmd, u8 msg_flag, u8 err, const u8 *data, u16 len)
{
    u8  frame[LB_TXBUF_SIZE];
    u16 total = lb_proto_build_frame(frame, ble_cmd, msg_flag, err, data, len);
    if (!total) {
        return false;
    }
    // ── 发包日志: 原始字节由平台层 app_blue_fit "BLE==>TX" 打印, 这里只补字段解析 ──
    lb_ble_dump_frame(ble_cmd, data, len, false);
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

#endif // FUNC_LUNCHBOX_UART_EN
