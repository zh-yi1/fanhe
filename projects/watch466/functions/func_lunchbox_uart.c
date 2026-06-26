/**
 * @file    func_lunchbox_uart.c
 * @brief   智能盒饭 - BLE+UART双协议实现 (大端, 同步/异步双模式)
 * @note    BLE侧: 蓝牙通讯协议1.0.6.md (APP ↔ MCU)
 *          UART侧: MCU通信协议.md v1.0.8 (MCU ↔ 加热模块)
 *          引脚: TX=PB8(UT1TXMAP_G2_PB8), RX=PB9(UT1RXMAP_G2_PB9)
 *          接收用 bsp_uart1_get_char() 轮询，不走 ISR 回调链
 *          >1 字节字段(data_len)采用大端传输
 */

#include "include.h"
#include "func_lunchbox_uart.h"
#include "heat_display_reg.h"
#include "bsp_vbat.h"

#if FUNC_LUNCHBOX_UART_EN

#define LB_DEBUG    0                   // 1=打开协议调试打印，0=关闭
#if LB_DEBUG
#define LB_TRACE(...)       printf(__VA_ARGS__)
#else
#define LB_TRACE(...)
#endif

//-----------------------------------------------------------------------------
// 空 ISR（uart_init 要求 rx_isr 非 NULL，实际数据走 bsp_uart1 环形缓冲）
//-----------------------------------------------------------------------------
AT(.com_text.uart)
static void lb_dummy_isr(uint8_t *buf, uint32_t len) { (void)buf; (void)len; }

//-----------------------------------------------------------------------------
// 内部状态
//-----------------------------------------------------------------------------
static u8  lb_rx_buf[LB_RXBUF_SIZE];     // 帧解析缓冲区
static u16 lb_rx_idx;                    // 当前已写入帧缓冲的字节数 / 状态机位置
static u32 lb_rx_ticks;                  // 最近一次收到字节的时间戳（超时丢弃用）

static u8  lb_ring_buf[LB_RXBUF_SIZE];   // bsp_uart1 环形缓冲区
static u8  lb_tx_buf[LB_TXBUF_SIZE];     // 组帧发送缓冲区
static lb_cmd_handler_t   cmd_handler[16];   // 命令字 → 回调，仅 0x01~0x0E 有效
static lb_ble_tx_fn_t     lb_ble_tx_fn;      // BLE 发送回调（非 NULL 时走 BLE）
static bool lb_uart_sync_pending = false;    // 是否有同步UART请求待应答(用于区分同步/异步0x01)

//-----------------------------------------------------------------------------
// OTA 升级状态机 (蓝牙通讯协议1.0.6.md §5)
// 管理主单片机 (target=0x01) 的固件升级流程，对接 ota_pack_* 底层 FOTA 引擎
//-----------------------------------------------------------------------------
typedef enum {
    LB_OTA_IDLE = 0,        // 空闲
    LB_OTA_READY,           // 已收到启动命令(0x0c)，等待数据
    LB_OTA_RECEIVING,       // 正在接收升级包数据(0x0d)
} lb_ota_state_t;

typedef struct {
    lb_ota_state_t state;   // 当前状态
    u32 fw_size;            // 固件总大小（字节，从 0x0c 获取）
    u32 next_offset;        // 期望的下一个数据偏移量（用于连续性校验）
    u32 recv_size;          // 已接收的数据总大小
    u8  buf[512];           // 512 字节写入缓冲（ota_pack_write 要求 512 对齐）
    u16 buf_pos;            // 缓冲区已使用字节数
    u8  need_reset;         // 升级完成标志，主循环检测后延时复位
} lb_ota_ctx_t;

static lb_ota_ctx_t lb_ota_ctx;
static u32 lb_ota_reset_tick = 0;           // 升级完成后延时复位的 tick

//-----------------------------------------------------------------------------
// 模式界面 → 加热界面 预设参数传递
//-----------------------------------------------------------------------------
static lb_mode_to_heat_preset_t lb_mode_heat_preset;

void lb_mode_to_heat_set(u8 proto_mode, u16 temp_f, u8 hour, u8 min)
{
    lb_mode_heat_preset.active     = true;
    lb_mode_heat_preset.proto_mode = proto_mode;
    lb_mode_heat_preset.temp_f     = temp_f;
    lb_mode_heat_preset.hour       = hour;
    lb_mode_heat_preset.min        = min;
}

bool lb_mode_to_heat_get(lb_mode_to_heat_preset_t *out)
{
    if (!lb_mode_heat_preset.active) return false;
    if (out) memcpy(out, &lb_mode_heat_preset, sizeof(lb_mode_to_heat_preset_t));
    lb_mode_heat_preset.active = false;  // 一次性消费，防止重复触发
    return true;
}

//-----------------------------------------------------------------------------
// 工具
//-----------------------------------------------------------------------------

static void lb_dp_dump_hex(const u8 *data, u16 data_len);

/**
 * @brief 计算协议校验和
 *
 * 从 data[0] 到 data[len-1] 逐字节累加，结果对 256 取余（取低 8 位）。
 */
static u8 lb_checksum(u8 *data, u16 len)
{
    u32 sum = 0;                                    // u32 防累加上溢
    for (u16 i = 0; i < len; i++) sum += data[i];
    return (u8)(sum % 256);                         // 取低 8 位
}

/** @brief 检查 DataPoints 数据中是否包含按键通知 (dpid=12) */
static bool lb_data_is_key_notify(u8 *data, u16 len)
{
    u16 off = 0;
    while (off + 4 <= len) {
        u8  dpid    = data[off];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];
        if (off + 4 + val_len > len) break;
        if (dpid == LB_DPID_KEY_NOTIFY) return true;
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
        lb_rx_reset();
        return false;
    }
    if (lb_rx_idx < total) {
        return false;                               // 数据还没到齐
    }

    // ──── 检查 4：校验和验证 ────
    if (lb_checksum(lb_rx_buf, total - 1) != lb_rx_buf[total - 1]) {
        LB_TRACE("lb: chk fail\n");
        lb_rx_reset();
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
        lb_dp_dump_hex(rx.data, rx.data_len);
    }

#if FUNC_LUNCHBOX_UART_EN
    if (rx.cmd == LB_UART_CMD_DYNAMIC && rx.data && rx.data_len > 0) {
        if (!lb_data_is_key_notify(rx.data, rx.data_len)) {
            //printf("Trigger==>heat_display_feed_dp:%d\n",__LINE__);
            heat_display_feed_dp(rx.data, rx.data_len);
        }
    }
#endif

#if LB_BRIDGE_MODE
    // ──── 桥模式：翻译为 BLE 协议 → 通过 BLE 发给 APP ────
    {
        u8 ble_buf[LB_TXBUF_SIZE];
        u16 ble_len = 0;
        if (lb_translate_uart_to_ble(&rx, ble_buf, &ble_len)) {
            if (lb_ble_tx_fn) {
                lb_ble_tx_fn(ble_buf, ble_len);
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

    lb_rx_reset();
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
static void lb_send_frame(u8 cmd, u8 msg_flag, u8 err, u8 *data, u16 len)
{
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

    // 打印 TX 日志
    printf("TX[%d]: ", off);
    for (u16 i = 0; i < off; i++) printf("%02X ", lb_tx_buf[i]);
    printf("\n");

    if (lb_ble_tx_fn) {
        lb_ble_tx_fn(lb_tx_buf, off);           // 走 BLE Notify
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

static u8 lb_async_msg_flag = 0;    // 异步消息自动递增的 msg_flag

/**
 * @brief 异步发送帧 — MCU 主动推送（无需主机先请求）
 */
void lunchbox_uart_send_async(u8 cmd, u8 *data, u16 len)
    { lb_send_frame(cmd, lb_async_msg_flag++, LB_ERR_SUCCESS, data, len); }

//-----------------------------------------------------------------------------
// 设备信息
//-----------------------------------------------------------------------------
static lb_device_info_t lb_dev_info;

//-----------------------------------------------------------------------------
// DataPoint 编码工具 (桥模式和本地模式均可用)
//-----------------------------------------------------------------------------

/**
 * @brief 向发送缓冲区写入一个 DataPoint 单元
 * @return 写入的字节数
 */
static u16 lb_dp_encode(u8 *buf, u8 dpid, u8 type, u8 *val, u16 val_len)
{
    buf[0] = dpid;
    buf[1] = type;
    buf[2] = (u8)(val_len >> 8);    // 值长度大端
    buf[3] = (u8)(val_len & 0xFF);
    if (val && val_len) memcpy(buf + 4, val, val_len);
    return 4 + val_len;
}

/** @brief 编码 bool 型 DataPoint */
static u16 lb_dp_encode_bool(u8 *buf, u8 dpid, u8 val)
{
    return lb_dp_encode(buf, dpid, LB_DP_TYPE_BOOL, &val, 1);
}

/** @brief 编码 enum 型 DataPoint */
static u16 lb_dp_encode_enum(u8 *buf, u8 dpid, u8 val)
{
    return lb_dp_encode(buf, dpid, LB_DP_TYPE_ENUM, &val, 1);
}

/** @brief 编码 value 型 DataPoint (4B 大端) */
static u16 lb_dp_encode_value(u8 *buf, u8 dpid, u32 val)
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
static void lb_dp_dump_hex(const u8 *data, u16 data_len)
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
            static const char *bats[] = {"?","Low","Mid","High","Full"};
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
        case LB_DPID_HEAT_TEMP: {     // 7: enum
            static const char *temps[] = {"40C","50C","60C","70C","80C","90C"};
            printf(" HeatTemp=%s(%d)", val[0] < 6 ? temps[val[0]] : "?", val[0]);
            break;
        }
        case LB_DPID_LANGUAGE:        // 8: enum
            printf(" Lang=%d", val[0]);
            break;
        case LB_DPID_FAULT: {         // 9: enum
            static const char *faults[] = {"OK","HighTemp"};
            printf(" Fault=%s(%d)", val[0] < 2 ? faults[val[0]] : "?", val[0]);
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
// 业务状态（本地模式使用）
//-----------------------------------------------------------------------------
#if !LB_BRIDGE_MODE
static lb_schedule_ble_t   lb_schedules[LB_SCHEDULE_MAX]; // 预约列表 (BLE格式)
static u8                  lb_schedule_count;         // 当前预约条数
static u8                  lb_next_schedule_id = 1;   // 自增预约 ID

// 当前属性值 (BLE 属性列表 §4)
static u8  lb_attr_power_switch  = 1;    // 总开关: 默认开
static u8  lb_attr_heat_mode     = 0;    // 加热模式: 默认关闭
static u8  lb_attr_battery       = 4;    // 电量: 默认满
static u8  lb_attr_charge_status = 0;    // 充电状态: 默认未充电
static u32 lb_attr_heat_duration = 0;    // 加热时长(分钟)
static u32 lb_attr_remain_time   = 0;    // 剩余时间(分钟)
static u8  lb_attr_heat_temp     = 0;    // 加热温度: 默认40°C
static u8  lb_attr_language      = 0;    // 语言: 默认中文
static u8  lb_attr_fault         = 0;    // 故障: 默认正常
static u8  lb_attr_heat_enable   = 0;    // 是否加热: 默认停止 (v1.0.5 新增 ID=10)

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
static u8 lb_pending_ble_cmd[256];

// 模式信息（0x09 查询 / 0x0a 修改）：索引 1=自定义, 2=鸡腿, 3=意面, 4=预约, 5=保温
static u8 lb_mode_temp[6]     = { 0, 3, 4, 5, 3, 2 };  // 默认: 自定义70°C, 鸡腿80°C, 意面90°C, 预约70°C, 保温60°C(140°F)
static u8 lb_mode_duration[6] = { 0, 30, 45, 20, 30, 0 }; // 默认: 自定义30min, 鸡腿45min, 意面20min, 预约30min, 保温0min

#define LB_KEEP_WARM_MODE       5
#define LB_KEEP_WARM_TEMP_F     140

static bool lb_keep_warm_active = false;

void lunchbox_set_device_info(lb_device_info_t *info) { if (info) memcpy(&lb_dev_info, info, sizeof(lb_device_info_t)); }

//-----------------------------------------------------------------------------
// LCD 加热/预约控制接口 (桥模式和本地模式均可用)
// 直接构造 UART 帧发往加热模块，不依赖 BLE→UART 翻译路径
//-----------------------------------------------------------------------------

/** @brief 华氏度 → 温度档位 (0=40°C ~ 5=90°C) */
u8 lunchbox_temp_f_to_idx(u16 temp_f)
{
    if (temp_f < 149) return 2;      // <65°C:  档位2 (60°C)
    if (temp_f < 167) return 3;      // <75°C:  档位3 (70°C)
    if (temp_f < 185) return 4;      // <85°C:  档位4 (80°C)
    return 5;                         // ≥85°C:  档位5 (90°C)
}

u8 lunchbox_mode_get_temp(u8 mode) {
    return (mode <= 5) ? lb_mode_temp[mode] : 0;
}

u8 lunchbox_mode_get_duration(u8 mode) {
    return (mode <= 5) ? lb_mode_duration[mode] : 0;
}

#if LB_BRIDGE_MODE
// 桥模式: 属性缓存在加热模块侧, MCU 不维护本地副本
u8 lunchbox_get_heat_mode(void)   { return 0; }
u8 lunchbox_get_heat_enable(void) { return 0; }
#else
// 本地模式: 读写本地属性缓存 (lb_attr_write_single 在下方定义)
u8 lunchbox_get_heat_mode(void)   { return lb_attr_heat_mode; }
u8 lunchbox_get_heat_enable(void) { return lb_attr_heat_enable; }
#endif

/**
 * @brief 构造完整 UART 帧并发送到加热模块
 * @param uart_cmd  UART 命令字 (LB_UART_CMD_*)
 * @param data      数据载荷
 * @param data_len  数据长度
 */
static void lb_uart_send_raw(u8 uart_cmd, u8 *data, u16 data_len)
{
    u8 buf[LB_TXBUF_SIZE];
    u16 off = 0;
    static u8 s_uart_msg_flag = 0;

    buf[off++] = (u8)(LB_FRAME_HEADER >> 8);  // 0x55
    buf[off++] = (u8)LB_FRAME_HEADER;          // 0xAA
    buf[off++] = LB_FRAME_VERSION;
    buf[off++] = s_uart_msg_flag++;             // msg_flag 自增
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
    if (!lb_data_is_key_notify(data, data_len)) {
        printf("LCD->UART==>TX[%d]: ", off);
        for (u16 i = 0; i < off; i++) printf("%02X ", buf[i]);
        printf("\n");
        lb_dp_dump_hex(data, data_len);
    }

    uart_bufs_tx(UART_TYPE_1,buf, off);
}

/**
 * @brief LCD 启动加热 — 构造 UART 0x01 DataPoint 帧发往加热模块
 *
 * MCU协议 §3.1 cmd=0x01: DataPoints 格式
 * 关键: dpid=10(是否加热) bool, 1=立即加热 (MCU协议 §4 属性列表)
 * 帧结构: DataPoints(mode+duration+temp+heat_enable=1) + time_sync + power_switch
 */
void lunchbox_heat_start(u8 mode, u8 temp, u32 duration)
{
    lb_keep_warm_active = (mode == LB_KEEP_WARM_MODE);
    u32 ts = RTCCNT + LB_RTC_UNIX_OFFSET;
    u8 data[64];
    u8 *p = data;

    // 加热参数 DataPoints
    p += lb_dp_encode_enum(p, LB_DPID_HEAT_MODE, mode);
    p += lb_dp_encode_value(p, LB_DPID_HEAT_DURATION, duration);
    p += lb_dp_encode_enum(p, LB_DPID_HEAT_TEMP, temp);
    // dpid=10: 是否加热=1 (立即加热)
    p += lb_dp_encode_bool(p, LB_DPID_HEAT_ENABLE, 1);
    // 时间戳 + MCU使能开机
    p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, ts);
    p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);

    u16 data_len = (u16)(p - data);
    lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, data_len);

    // 更新本地属性 (本地模式) 或仅通知 APP
#if !LB_BRIDGE_MODE
    lb_attr_heat_mode     = mode;
    lb_attr_heat_temp     = temp;
    lb_attr_heat_duration = duration;
    lb_attr_heat_enable   = 1;
    lunchbox_report_all_attrs();
#endif
}

/**
 * @brief LCD 停止加热 — 构造 UART 0x01 DataPoint 帧发往加热模块
 *
 * MCU协议 §4 属性列表 ID=10: 是否加热, bool, 0=停止加热
 */
void lunchbox_heat_stop(void)
{
    lb_keep_warm_active = false;
    u32 ts = RTCCNT + LB_RTC_UNIX_OFFSET;
    u8 data[32];
    u8 *p = data;

    // dpid=10: 是否加热=0 (停止加热)
    p += lb_dp_encode_bool(p, LB_DPID_HEAT_ENABLE, 0);
    // 时间戳 + MCU使能开机
    p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, ts);
    p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);

    u16 data_len = (u16)(p - data);
    lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, data_len);

#if !LB_BRIDGE_MODE
    lb_attr_heat_enable = 0;
    lb_attr_heat_mode   = 0;  // 关闭
    lunchbox_report_all_attrs();
#endif
}

/** @brief 加热自然结束后自动开启保温 (模式5, 140°F, 无时长限制) */
void lunchbox_keep_warm_start(void)
{
    if (lb_keep_warm_active) {
        return;
    }
    lunchbox_heat_start(LB_KEEP_WARM_MODE,
                        lunchbox_temp_f_to_idx(LB_KEEP_WARM_TEMP_F), 0);
}

/** @brief 停止保温 (低电关机或用户/新加热打断) */
void lunchbox_keep_warm_stop(void)
{
    if (!lb_keep_warm_active) {
        return;
    }
    lunchbox_heat_stop();
}

bool lunchbox_keep_warm_is_active(void)
{
    return lb_keep_warm_active;
}

/** @brief 主循环轮询：低电关机时停止保温 */
void lunchbox_keep_warm_poll(void)
{
    if (!lb_keep_warm_active) {
        return;
    }
    if (bsp_vbat_get_lpwr_status() == 2) {
        lunchbox_keep_warm_stop();
    }
}

/**
 * @brief LCD 按键通知 — UART 0x01 DataPoint(dpid=12) 发往加热模块
 */
void lunchbox_key_notify(u8 key_val)
{
    u8 data[8];
    u16 len;

    if (key_val == 0 || key_val > 9) {
        return;
    }
    len = lb_dp_encode_enum(data, LB_DPID_KEY_NOTIFY, key_val);
    lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, len);
}

/**
 * @brief LCD 发送预约 — 构造 UART 0x03 帧发往加热模块
 *
 * UART 0x03 帧格式(42B): action(1)+id(1)+name(32)+time(4)+temp(1)+duration(1)+enabled(1)+repeat(1)
 */
void lunchbox_reservation_send(u8 action, u8 id, const char *name, u32 unix_time,
                               u8 temp, u8 duration, u8 enabled, u8 repeat)
{
    u8 data[42];
    memset(data, 0, 42);

    data[0] = action;
    data[1] = id;
    if (name) {
        u8 i;
        for (i = 0; i < 32 && name[i]; i++) {
            data[2 + i] = (u8)name[i];
        }
    }
    // unix_time: 4 bytes BE
    data[2 + 32 + 0] = (u8)(unix_time >> 24);
    data[2 + 32 + 1] = (u8)(unix_time >> 16);
    data[2 + 32 + 2] = (u8)(unix_time >> 8);
    data[2 + 32 + 3] = (u8)(unix_time & 0xFF);
    data[2 + 32 + 4] = temp;
    data[2 + 32 + 5] = duration;
    data[2 + 32 + 6] = enabled;
    data[2 + 32 + 7] = repeat;

    lb_uart_send_raw(LB_UART_CMD_SCHEDULE_OP, data, 42);
}

/**
 * @brief LCD 删除预约
 */
void lunchbox_reservation_delete(u8 id)
{
    u8 data[2];
    data[0] = 0x00;  // action=删除
    data[1] = id;
    lb_uart_send_raw(LB_UART_CMD_SCHEDULE_OP, data, 2);
}

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

/** @brief 分配新 ID */
static u8 lb_schedule_new_id(void)
{
    while (lb_schedule_find(lb_next_schedule_id) >= 0 || lb_next_schedule_id == 0)
        lb_next_schedule_id++;
    return lb_next_schedule_id;
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
 * MCU 返回: 73 字节设备信息
 *   布局: bt_name(16B) + version(8B) + model(10B) + MAC(6B) + SN(32B) + color(1B)
 */
static u8 lb_handler_product_info(lb_rx_frame_t *rx)
{
    u8 buf[73]; // 16+8+10+6+32+1 = 73
    u16 off = 0;

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

    lunchbox_uart_send_response(LB_CMD_PRODUCT_INFO, rx->msg_flag, LB_ERR_SUCCESS, buf, off);
    return LB_ERR_SUCCESS;
}

// ─── 以下 handler/函数在桥模式和本地模式都需要 ───

#if LB_BRIDGE_MODE
/**
 * @brief 从 OTA 命令帧中提取目标设备标识 (v1.0.6)
 *
 * 仅桥模式使用: 根据 target 字段决定 OTA 命令是本地处理还是转发 UART。
 * 本地模式下 APP 直接与 MCU 通信, target 恒为 0x01, 无需提取。
 *
 * 各 OTA 命令 data 区首字节均为 target:
 *   0x0b 升级查询: data[0] 或 无数据(查询全部)
 *   0x0c 升级启动: data[0]=target, data[1..4]=fw_size
 *   0x0d 升级包传输: data[0]=target, data[1..4]=offset, data[5..]=upgrade_data
 *   0x0e 升级结束: data[0]=target
 *
 * @return LB_OTA_TARGET_MAIN_MCU(0x01) / LB_OTA_TARGET_HEAT_MODULE(0x02) / 0x00(查询全部/未知)
 */
static u8 lb_ota_get_target(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len == 0) return 0x00;  // 无数据 → 查询全部

    switch (rx->cmd) {
    case LB_CMD_OTA_QUERY:   // data_len=1: [target]
        return rx->data[0];
    case LB_CMD_OTA_START:   // data_len=5: [target][fw_size:4B]
        return (rx->data_len >= 5) ? rx->data[0] : 0x00;
    case LB_CMD_OTA_DATA:    // data_len≥5: [target][offset:4B][data]
        return (rx->data_len >= 5) ? rx->data[0] : 0x00;
    case LB_CMD_OTA_END:     // data_len=1: [target]
        return rx->data[0];
    default:
        return 0x00;
    }
}
#endif // LB_BRIDGE_MODE

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

    lb_mode_temp[mode]     = rx->data[1];
    lb_mode_duration[mode] = rx->data[2];
    lunchbox_uart_send_response(LB_CMD_MODE_MODIFY, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 注册所有协议命令的业务处理器
 *
 * 蓝牙通讯协议1.0.6.md 命令字 → 处理函数映射:
 *   0x01 → lb_handler_product_info   (查询产品信息)    [桥/本地]
 *   0x02 → lb_handler_dynamic_attr   (查询动态属性)    [仅本地]
 *   0x04 → lb_handler_control        (控制指令)        [仅本地]
 *   0x05 → lb_handler_schedule_list  (查询预约列表)    [仅本地]
 *   0x06 → lb_handler_schedule_add   (新增预约)        [仅本地]
 *   0x07 → lb_handler_schedule_modify(修改预约)        [仅本地]
 *   0x08 → lb_handler_schedule_delete(删除预约)        [仅本地]
 *   0x09 → lb_handler_mode_query     (获取模式信息)    [桥/本地]
 *   0x0a → lb_handler_mode_modify    (修改模式信息)    [桥/本地]
 *   0x0b → lb_handler_ota_query      (升级查询)        [桥/本地]
 *   0x0c → lb_handler_ota_start      (升级启动)        [桥/本地]
 *   0x0d → lb_handler_ota_data       (升级包传输)      [桥/本地]
 *   0x0e → lb_handler_ota_end        (升级结束)        [桥/本地]
 */

// 前向声明 (函数定义在 lunchbox_uart_init_handlers 之后)
// OTA handler: 桥模式和本地模式均需 (target=0x01 主单片机升级)
static u8 lb_handler_ota_query(lb_rx_frame_t *rx);
static u8 lb_handler_ota_start(lb_rx_frame_t *rx);
static u8 lb_handler_ota_data(lb_rx_frame_t *rx);
static u8 lb_handler_ota_end(lb_rx_frame_t *rx);
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
    // OTA 命令 (v1.0.6 §5): 主单片机(target=0x01)升级在桥模式/本地模式均需处理
    lunchbox_uart_reg_handler(LB_CMD_OTA_QUERY,       lb_handler_ota_query);
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

    // 属性变化后主动上报 APP
    lunchbox_report_all_attrs();

    // 同步推送 LCD 显示 (加热页/预约页可实时看到模式/温度/时长变化)
    if (rx->data && rx->data_len > 0) {
        heat_display_feed_dp(rx->data, rx->data_len);
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
    s->id = lb_schedule_new_id();   // MCU 自动分配 ID
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
 * @brief 0x0b — 升级查询 (v1.0.6: 支持按 target 查询指定设备)
 *
 * APP 发送:
 *   - data_len=0: 查询全部设备 → 此 handler 仅返回主单片机状态
 *   - data_len=1: [target:1B] 查询指定设备
 *
 * 路由规则 (桥模式):
 *   - target=0x01(主单片机) 或 无target → 本 handler 处理, BLE 直接应答
 *   - target=0x02(加热模块) → lb_translate_ble_to_uart() 转发 UART,
 *     加热模块通过 UART→BLE 翻译应答
 *
 * MCU 返回: 每设备 2 字节 [target(1B)][status(1B)]
 * status: 0x00=不支持MCU升级, 0x01=MCU未就绪, 0x02=支持升级
 */
static u8 lb_handler_ota_query(lb_rx_frame_t *rx)
{
    u8 buf[4];  // 最多 2 设备 × 2 字节 = 4
    u16 off = 0;

    // 主单片机已对接 FOTA 引擎 (ota_pack_*)，支持 MCU 升级
    if (rx->data_len == 1 && rx->data) {
        u8 target = rx->data[0];
        buf[off++] = target;
        buf[off++] = LB_OTA_STATUS_SUPPORTED;
    } else {
        // 查询全部 → 仅返回主单片机状态
        buf[off++] = LB_OTA_TARGET_MAIN_MCU;
        buf[off++] = LB_OTA_STATUS_SUPPORTED;
    }

    lunchbox_uart_send_response(LB_CMD_OTA_QUERY, rx->msg_flag, LB_ERR_SUCCESS, buf, off);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x0c — 升级启动 (v1.0.6: 新增 target 字段指定目标设备)
 *
 * APP 发送: 5 字节 [target(1B)][firmware_size(4B, 大端)]
 * MCU 返回: 2 字节 [target(1B)][status(1B)]
 *   status: 0x00=收到升级指令, 0x01=擦除flash中, 0x02=擦除完成
 */
static u8 lb_handler_ota_start(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 5) {
        lunchbox_uart_send_response(LB_CMD_OTA_START, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    u8  target = rx->data[0];
    u32 fw_size = ((u32)rx->data[1] << 24) | ((u32)rx->data[2] << 16)
                | ((u32)rx->data[3] << 8)  | rx->data[4];

    printf("OTA start: target=0x%02X fw_size=%lu\n", target, fw_size);

    // 初始化 FOTA 引擎 (压缩升级包写入准备)
    ota_pack_init();
    load_code_fota();

    // 重置 OTA 上下文
    memset(&lb_ota_ctx, 0, sizeof(lb_ota_ctx));
    lb_ota_ctx.state = LB_OTA_READY;
    lb_ota_ctx.fw_size = fw_size;

    u8 rsp[2];
    rsp[0] = target;
    rsp[1] = LB_OTA_START_ERASE_DONE;  // 初始化完成，可以传输升级包
    lunchbox_uart_send_response(LB_CMD_OTA_START, rx->msg_flag, LB_ERR_SUCCESS, rsp, 2);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x0d — 升级包传输 (v1.0.6: 新增 target 字段区分设备)
 *
 * APP 发送: 5+N 字节 [target(1B)][offset(4B, 大端)][upgrade_data(N bytes)]
 *   数据长度 = N + 5, 每包数据长度必须可被 16 整除，不足补 0
 * MCU 返回: 无数据 (ack 帧)
 */
static u8 lb_handler_ota_data(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 5) {
        lunchbox_uart_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    // 必须先收到启动命令
    if (lb_ota_ctx.state < LB_OTA_READY) {
        printf("OTA data err: not started\n");
        lunchbox_uart_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    u8  target = rx->data[0];
    u32 offset = ((u32)rx->data[1] << 24) | ((u32)rx->data[2] << 16)
               | ((u32)rx->data[3] << 8)  | rx->data[4];
    u8 *data = rx->data + 5;
    u16 data_size = rx->data_len - 5;
    (void)target;  // target 字段已在路由层校验（本地模式恒为 0x01）

    // 校验 offset 连续性 (ota_pack_write 顺序写入，不支持随机偏移)
    if (offset != lb_ota_ctx.next_offset) {
        printf("OTA seq err: expected=%lu got=%lu\n", lb_ota_ctx.next_offset, offset);
        lunchbox_uart_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    lb_ota_ctx.state = LB_OTA_RECEIVING;

    // 缓冲写入: 将数据填入 512 字节缓冲，满一块写一块
    u16 remaining = data_size;
    u8 *src = data;

    while (remaining > 0) {
        u16 space = 512 - lb_ota_ctx.buf_pos;
        u16 copy = (remaining < space) ? remaining : space;
        memcpy(lb_ota_ctx.buf + lb_ota_ctx.buf_pos, src, copy);
        lb_ota_ctx.buf_pos += copy;
        src += copy;
        remaining -= copy;

        if (lb_ota_ctx.buf_pos >= 512) {
            // 检查 FOTA 引擎是否有错误
            if (ota_pack_get_err() != FOT_ERR_OK) {
                printf("OTA write err: 0x%x\n", ota_pack_get_err());
                lunchbox_uart_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
                return LB_ERR_EXEC_FAIL;
            }
            ota_pack_write(lb_ota_ctx.buf);
            lb_ota_ctx.buf_pos = 0;
        }
    }

    lb_ota_ctx.next_offset = offset + data_size;
    lb_ota_ctx.recv_size += data_size;

    lunchbox_uart_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x0e — 升级结束 (v1.0.6: 新增 target 字段，返回 target+结果)
 *
 * APP 发送: 1 字节 [target(1B)] 指定结束哪个设备的升级
 * MCU 返回: 2 字节 [target(1B)][result(1B)]
 *   result: 0x00=升级失败, 0x01=升级成功
 */
static u8 lb_handler_ota_end(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 1) {
        lunchbox_uart_send_response(LB_CMD_OTA_END, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    u8 target = rx->data[0];
    u8 result = LB_OTA_RESULT_FAIL;

    printf("OTA end: target=0x%02X recv_size=%lu fw_size=%lu\n",
           target, lb_ota_ctx.recv_size, lb_ota_ctx.fw_size);

    if (lb_ota_ctx.state >= LB_OTA_READY) {
        // 刷出缓冲区中剩余数据 (不足 512 字节的部分补 0)
        if (lb_ota_ctx.buf_pos > 0) {
            memset(lb_ota_ctx.buf + lb_ota_ctx.buf_pos, 0, 512 - lb_ota_ctx.buf_pos);
            ota_pack_write(lb_ota_ctx.buf);
            lb_ota_ctx.buf_pos = 0;
        }

        // 校验写入完整性
        if (ota_pack_is_write_done()) {
            ota_pack_verify();
            u8 err = ota_pack_get_err();
            printf("OTA verify: err=%d\n", err);
            if (err == FOT_ERR_OK) {
                ota_pack_done();
                printf("OTA success, will reset in 3s...\n");
                result = LB_OTA_RESULT_SUCCESS;
                lb_ota_ctx.need_reset = 1;
                lb_ota_reset_tick = tick_get();
            } else {
                printf("OTA verify failed: 0x%x\n", err);
            }
        } else {
            printf("OTA write incomplete: recv=%lu expected=%lu\n",
                   lb_ota_ctx.recv_size, lb_ota_ctx.fw_size);
        }
    }

    // 清理状态 (need_reset 保持，由 lb_ota_process 处理复位)
    lb_ota_ctx.state = LB_OTA_IDLE;

    if (result != LB_OTA_RESULT_SUCCESS) {
        unlock_code_fota();  // 升级失败，解锁代码区
    }

    u8 rsp[2];
    rsp[0] = target;
    rsp[1] = result;
    lunchbox_uart_send_response(LB_CMD_OTA_END, rx->msg_flag, LB_ERR_SUCCESS, rsp, 2);
    return LB_ERR_SUCCESS;
}

//-----------------------------------------------------------------------------
// OTA 升级流程管理 (主单片机 target=0x01)
// 桥模式和本地模式均可用
//-----------------------------------------------------------------------------

/**
 * @brief OTA 升级流程处理 (需在主循环中轮询调用)
 *
 * 职责: 升级成功后的延时复位。ota_pack_done() 完成后需复位 MCU
 * 才能让 bootloader 解压新固件。延时 3 秒是为了确保 BLE 应答帧
 * (0x0e 返回) 已成功发送给 APP。
 *
 * 调用位置: func.c 主循环, 与 bsp_fot_process() 并列
 */
void lb_ota_process(void)
{
    if (lb_ota_ctx.need_reset && lb_ota_reset_tick) {
        if (tick_check_expire(lb_ota_reset_tick, 3000)) {
            printf("OTA reset now...\n");
            WDT_RST();
        }
    }
}

//-----------------------------------------------------------------------------
// 协议翻译层 (BLE ↔ UART)
//-----------------------------------------------------------------------------

/**
 * @brief BLE 命令字 → UART 命令字映射
 *
 * 蓝牙通讯协议1.0.6.md → MCU通信协议.md v1.0.8:
 *   0x01 → 0x00 (不转发, MCU本地处理)
 *   0x02 → 0x01 (查询动态属性)
 *   0x03 → 0x00 (不转发, MCU主动上报, APP不会发)
 *   0x04 → 0x01 (控制指令 → UART查询动态属性/状态上报通道)
 *   0x05 → 0x02 (查询预约列表)
 *   0x06 → 0x03 (新增预约)
 *   0x07 → 0x03 (修改预约)
 *   0x08 → 0x03 (删除预约)
 *   0x09 → 0x02 (获取指定模式信息 → 查询预约列表)
 *   0x0a → 0x03 (修改指定模式信息 → 新增/修改/删除预约)
 *   0x0b → 0x04 (OTA)
 *   0x0c → 0x04 (OTA)
 *   0x0d → 0x04 (OTA)
 *   0x0e → 0x04 (OTA)
 *
 * @return UART 命令字, 0x00 表示不转发
 */
u8 lb_ble_cmd_to_uart_cmd(u8 ble_cmd)
{
    switch (ble_cmd) {
    case LB_CMD_DYNAMIC_ATTR:    return LB_UART_CMD_DYNAMIC;     // 0x02 → 0x01
    case LB_CMD_CONTROL:         return LB_UART_CMD_DYNAMIC;     // 0x04 → 0x01 (控制指令走动态属性通道)
    case LB_CMD_SCHEDULE_LIST:   return LB_UART_CMD_SCHEDULE;    // 0x05 → 0x02
    case LB_CMD_SCHEDULE_ADD:    return LB_UART_CMD_SCHEDULE_OP; // 0x06 → 0x03
    case LB_CMD_SCHEDULE_MODIFY: return LB_UART_CMD_SCHEDULE_OP; // 0x07 → 0x03
    case LB_CMD_SCHEDULE_DELETE: return LB_UART_CMD_SCHEDULE_OP; // 0x08 → 0x03
    case LB_CMD_OTA_QUERY:       return LB_UART_CMD_OTA;         // 0x0b → 0x04
    case LB_CMD_OTA_START:       return LB_UART_CMD_OTA;         // 0x0c → 0x04
    case LB_CMD_OTA_DATA:        return LB_UART_CMD_OTA;         // 0x0d → 0x04
    case LB_CMD_OTA_END:         return LB_UART_CMD_OTA;         // 0x0e → 0x04
    case LB_CMD_MODE_QUERY:      return LB_UART_CMD_SCHEDULE;    // 0x09 → 0x02 (查询指定模式的预约)
    case LB_CMD_MODE_MODIFY:     return LB_UART_CMD_SCHEDULE_OP; // 0x0a → 0x03 (修改模式模板)
    default:                     return 0x00;                    // 不转发
    }
}

/**
 * @brief UART 命令字 → BLE 命令字映射
 *
 * MCU通信协议.md v1.0.8 → 蓝牙通讯协议1.0.6.md:
 *   0x01(同步应答) → 0x02 (动态属性应答)
 *   0x01(异步上报) → 0x03 (状态上报)
 *   0x02 → 0x05 (预约列表条目)
 *   0x03 → 0x06/0x07/0x08 (需要根据数据内容判断)
 *   0x04 → 0x0d (OTA应答, 暂映射到升级包传输)
 *
 * @param uart_cmd  UART 命令字
 * @param is_async  true=异步状态上报
 * @return BLE 命令字
 */
u8 lb_uart_cmd_to_ble_cmd(u8 uart_cmd, bool is_async)
{
    switch (uart_cmd) {
    case LB_UART_CMD_DYNAMIC:
        return is_async ? LB_CMD_STATUS_REPORT : LB_CMD_DYNAMIC_ATTR;
    case LB_UART_CMD_SCHEDULE:
        return LB_CMD_SCHEDULE_LIST;
    case LB_UART_CMD_SCHEDULE_OP:
        // 无法静态判断是新增/修改/删除的应答，由翻译函数根据数据决定
        return LB_CMD_SCHEDULE_ADD;
    case LB_UART_CMD_OTA:
        return LB_CMD_OTA_DATA;
    default:
        return 0x00;
    }
}

/**
 * @brief BLE帧数据 → UART帧数据翻译
 *
 * 在 lb_translate_ble_frame_to_uart() 中只计算数据载荷(data部分)。
 * 调用者负责组帧(加帧头+校验和)然后发送。
 *
 * @param rx        BLE 接收帧(已解析)
 * @param out_data  输出: 翻译后的数据载荷
 * @param out_len   输出: 数据载荷长度
 * @return true=翻译成功需要转发, false=不转发
 */
static bool lb_translate_ble_data_to_uart(lb_rx_frame_t *rx, u8 *out_data, u16 *out_len)
{
    *out_len = 0;

    switch (rx->cmd) {
    // ─── 0x02 查询动态属性 → UART 0x01: DataPoint格式 (v1.0.8) ───
    case LB_CMD_DYNAMIC_ATTR: {
        // v1.0.8: 数据域为 DataPoint 数组 (固定13字节)
        //   DataPoint1: dpid=11(时间戳), type=value, len=4, value=unix时间戳
        //   DataPoint2: dpid=1(电源开关), type=bool, len=1, value=1(MCU使能开机)
        u32 ts = RTCCNT + LB_RTC_UNIX_OFFSET;  // RTCCNT 从2020起算, +offset 转Unix时间戳
        u8 *p = out_data;
        p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, ts);
        p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);
        *out_len = p - out_data;
        return true;
    }

    // ─── 0x04 控制指令 → UART 0x01: DataPoint格式 (v1.0.8) ───
    case LB_CMD_CONTROL: {
        if (rx->data && rx->data_len > 0) {
            // v1.0.8: 前拼 dpid=11(时间戳) DataPoint, 后跟控制 DataPoints 透传
            u32 ts = RTCCNT + LB_RTC_UNIX_OFFSET;  // RTCCNT 从2020起算, +offset 转Unix时间戳
            u8 *p = out_data;
            p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, ts);
            memcpy(p, rx->data, rx->data_len);
            p += rx->data_len;
            *out_len = p - out_data;

            // BLE 控制指令携带 DataPoints 时，同步推送 LCD 显示
            heat_display_feed_dp(rx->data, rx->data_len);
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
        // BLE: [id(1B)] [name(32B)] [time(4B)] [temp(1B)] [duration(1B)] [enabled(1B)] [repeat(1B)]
        // UART: [action(1B)] [id(1B)] [name(32B)] [time(4B)] [temp(1B)] [duration(1B)] [enabled(1B)] [repeat(1B)]
        // action: 从预约数据推断 — 简化: 默认用 1(自定义加热)
        out_data[0] = 0x01;  // 默认: 自定义加热 (后续可从数据推断)
        memcpy(out_data + 1, rx->data, 41);
        *out_len = 42;
        return true;
    }

    // ─── 0x07 修改预约 → UART 0x03: 插入 action 字节 ───
    case LB_CMD_SCHEDULE_MODIFY: {
        if (!rx->data || rx->data_len < 41) return false;
        out_data[0] = 0x01;  // 默认: 自定义加热
        memcpy(out_data + 1, rx->data, 41);
        *out_len = 42;
        return true;
    }

    // ─── 0x08 删除预约 → UART 0x03: action=0, 数据=[id:1B] ───
    case LB_CMD_SCHEDULE_DELETE: {
        if (!rx->data || rx->data_len < 1) return false;
        out_data[0] = 0x00;  // action=0 → 删除
        out_data[1] = rx->data[0];  // 预约ID
        *out_len = 2;
        return true;
    }

    // ─── 0x09 获取指定模式信息 → UART 0x02: 透传 mode 字节 ───
    case LB_CMD_MODE_QUERY: {
        if (rx->data && rx->data_len >= 1) {
            out_data[0] = rx->data[0];  // mode (1~5)
            *out_len = 1;
        }
        // data_len==0 → 查询全部模式
        return true;
    }

    // ─── 0x0a 修改指定模式信息 → UART 0x03: 构造 42B 预约帧 ───
    case LB_CMD_MODE_MODIFY: {
        if (!rx->data || rx->data_len < 3) return false;
        // BLE: [mode(1B)] [temp(1B)] [duration(1B)]
        // UART: [action(1B)] [id(1B)] [name(32B)] [time(4B)] [temp(1B)] [duration(1B)] [enabled(1B)] [repeat(1B)]
        memset(out_data, 0, 42);
        out_data[0] = rx->data[0];       // action = mode (加热模块据此识别模式模板)
        out_data[1] = 0;                 // id = 0 (模式模板, 非常规预约)
        // name[32] = zeros
        // time[4] = 0
        out_data[37] = rx->data[1];      // temp
        out_data[38] = rx->data[2];      // duration
        out_data[39] = 0x01;             // enabled = 1
        out_data[40] = 0xff;             // repeat = 0xff (默认)
        *out_len = 42;
        return true;
    }

    // ─── OTA 命令 → UART 0x04 (v1.0.6: 透传含 target 字段的原始数据) ───
    case LB_CMD_OTA_QUERY:
    case LB_CMD_OTA_START:
    case LB_CMD_OTA_DATA:
    case LB_CMD_OTA_END: {
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
    out_buf[off++] = rx->msg_flag;                  // 保持原 msg_flag
    out_buf[off++] = uart_cmd;
    out_buf[off++] = LB_ERR_SUCCESS;                // err_flag
    out_buf[off++] = (u8)(data_len >> 8);           // data_len 大端
    out_buf[off++] = (u8)(data_len & 0xFF);
    if (data_len > 0) {
        memcpy(out_buf + off, data_buf, data_len);
        off += data_len;
    }
    out_buf[off] = lb_checksum(out_buf, off);
    *out_len = off + 1;

    // 标记为同步请求（用于区分 UART 应答是同步还是异步上报）
    lb_uart_sync_pending = true;

    // 记录原始 BLE 命令字，UART 应答时用于确定正确的 BLE 响应 cmd
    lb_pending_ble_cmd[rx->msg_flag] = rx->cmd;

    printf("BLE->UART[%d]: ", *out_len);
    for (u16 i = 0; i < *out_len; i++) printf("%02X ", out_buf[i]);
    printf("\n");

    return true;
}

/**
 * @brief UART帧数据 → BLE帧数据翻译
 */
static bool lb_translate_uart_data_to_ble(lb_rx_frame_t *rx, u8 ble_cmd, u8 *out_data, u16 *out_len)
{
    *out_len = 0;

    switch (rx->cmd) {
    // ─── UART 0x01 → BLE 0x02(同步应答) 或 BLE 0x03(异步上报): DataPoints 透传 ───
    // 调用者决定是同步还是异步
    case LB_UART_CMD_DYNAMIC: {
        if (rx->data && rx->data_len > 0) {
            memcpy(out_data, rx->data, rx->data_len);
            *out_len = rx->data_len;
            return true;
        }
        return false;
    }

    // ─── UART 0x02(预约列表条目 44B) → BLE 0x05(43B) 或 BLE 0x09(3B) ───
    case LB_UART_CMD_SCHEDULE: {
        if (!rx->data || rx->data_len < 44) return false;

        if (ble_cmd == LB_CMD_MODE_QUERY) {
            // BLE 0x09 格式: [mode(1B)] [temp(1B)] [duration(1B)]
            // 从 UART 条目提取: set_mode, temp, duration
            out_data[0] = rx->data[2];   // set_mode → mode
            out_data[1] = rx->data[37];  // temp
            out_data[2] = rx->data[38];  // duration
            *out_len = 3;
            return true;
        }

        // BLE 0x05 格式: [total_count(1)] [seq(1)] [id(1)] [name(32)] [time(4)] [temp(1)] [duration(1)] [enabled(1)] [repeat(1)]
        // UART:          [total_count(1)] [seq(1)] [set_mode(1)] [id(1)] [name(32)] [time(4)] [temp(1)] [duration(1)] [enabled(1)] [repeat(1)]
        // 跳过 set_mode 字节(rx->data[2])
        out_data[0] = rx->data[0];  // total_count
        out_data[1] = rx->data[1];  // seq
        memcpy(out_data + 2, rx->data + 3, 41);  // id+name+time+temp+duration+enabled+repeat (41B)
        // 修正 repeat bit7: MCU协议 bit7=保留0, BLE协议 bit7=保留1
        out_data[42] |= 0x80;  // 设置 bit7=1
        *out_len = 43;
        return true;
    }

    // ─── UART 0x03(应答: [id:1B]) → BLE 0x06/0x07/0x08/0x0a 应答 ───
    case LB_UART_CMD_SCHEDULE_OP: {
        if (ble_cmd == LB_CMD_MODE_MODIFY) {
            // BLE 0x0a 应答: 无数据, 仅 err_flag (成功/失败)
            *out_len = 0;
            return true;
        }
        if (rx->data && rx->data_len >= 1) {
            // BLE 0x06/0x07: 返回 ID
            out_data[0] = rx->data[0];  // 分配的 ID
            *out_len = 1;
            return true;
        }
        // BLE 0x08 删除成功: 无数据
        *out_len = 0;
        return true;
    }

    // ─── UART 0x04(OTA应答, v1.0.6: 含 target 字段) → 对应 BLE OTA 应答 ───
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
 *
 * 根据 UART cmd 和 err_flag 判断是同步应答还是异步上报，
 * 自动选择合适的 BLE 命令字。
 */
bool lb_translate_uart_to_ble(lb_rx_frame_t *rx, u8 *out_buf, u16 *out_len)
{
    bool is_async = false;
    u8 ble_cmd = 0;

    // 优先使用 pending BLE cmd（同步应答：根据原始请求确定响应命令字）
    if (rx->cmd != LB_UART_CMD_DYNAMIC || lb_uart_sync_pending) {
        ble_cmd = lb_pending_ble_cmd[rx->msg_flag];
        if (ble_cmd != 0) {
            lb_pending_ble_cmd[rx->msg_flag] = 0;  // 清除
            if (rx->cmd == LB_UART_CMD_DYNAMIC) {
                lb_uart_sync_pending = false;
            }
        }
    }

    // pending cmd 未命中 → 回退到静态映射
    if (ble_cmd == 0) {
        if (rx->cmd == LB_UART_CMD_DYNAMIC) {
            if (lb_uart_sync_pending) {
                lb_uart_sync_pending = false;
                ble_cmd = LB_CMD_DYNAMIC_ATTR;  // BLE 0x02 同步应答
            } else {
                is_async = true;
                ble_cmd = LB_CMD_STATUS_REPORT;  // BLE 0x03 异步上报
            }
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
    out_buf[off++] = (u8)(LB_FRAME_HEADER >> 8);   // 0x55
    out_buf[off++] = (u8)LB_FRAME_HEADER;           // 0xaa
    out_buf[off++] = LB_FRAME_VERSION;
    out_buf[off++] = is_async ? lb_async_msg_flag++ : rx->msg_flag;
    out_buf[off++] = ble_cmd;
    out_buf[off++] = rx->err_flag;
    out_buf[off++] = (u8)(data_len >> 8);           // data_len 大端
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

//-----------------------------------------------------------------------------
// BLE 通道实现
//-----------------------------------------------------------------------------

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
    // 最小帧：帧头(2) + 版本(1) + msg_flag(1) + cmd(1) + err_flag(1) + data_len(2) + checksum(1) = 9
    if (raw_len < 9) return false;

    // 帧头校验
    if (raw[0] != 0x55 || raw[1] != 0xAA) return false;

    u16 data_len = ((u16)raw[6] << 8) | raw[7];   // 大端
    // 允许尾部多余字节 (BLE MTU 可能引入额外数据)
    if (raw_len < 9 + data_len) {
        printf("BLE: len err raw=%d data=%d expected=%d\n", raw_len, data_len, 9 + data_len);
        return false;
    }

    // 仅校验声明帧长部分，忽略尾部多余字节
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
 * @brief BLE 饭盒帧入口 — BLE→UART 协议翻译转发（桥模式）/ 解析分发+透传（本地模式）
 *
 * 调用链:
 *   手机 BLE Write → gatt_callback_app() → ble_app_watch_process()
 *   → ble_app_blue_fit_rx_callback() → 本函数
 *
 * 桥模式(LB_BRIDGE_MODE=1):
 *   - 0x01 产品信息 → MCU 本地回复
 *   - 0x03 状态上报 → APP不会发, 忽略
 *   - 0x0b~0x0e OTA命令(v1.0.6) → 按 target 分流:
 *       target=0x01(主单片机) 或 查询全部(无target) → 本地处理, BLE直接应答
 *       target=0x02(加热模块)                      → 翻译为 UART 协议, 串口发往加热模块
 *   - 其他命令(含0x09/0x0a) → 翻译为 UART 协议 → 串口发往加热模块
 *
 * 本地模式(LB_BRIDGE_MODE=0):
 *   - 原帧透传到串口 + 解析分发给 cmd_handler
 */
void lunchbox_ble_rx_handle(u8 *data, u16 len)
{
    // ──── BLE 收包日志 ────
    printf("BLE==>RX [%d]: ", len);
    for (u16 i = 0; i < len; i++) printf("%02X ", data[i]);
    printf("\n");

    lb_rx_frame_t frame;
    memset(&frame, 0, sizeof(frame));

    if (!lb_ble_frame_parse(data, len, &frame)) {
        printf("BLE: frame parse fail\n");
        return;
    }

#if LB_BRIDGE_MODE
    // ──── 桥模式：翻译转发 ────

    // 0x01 产品信息 → MCU 本地回复，不转发加热模块
    if (frame.cmd == LB_CMD_PRODUCT_INFO) {
        lb_ble_tx_fn_t saved = lb_ble_tx_fn;
        // 强制应答走 BLE（不做 UART 屏蔽）
        lb_handler_product_info(&frame);
        return;
    }

    // 0x03 状态上报 → APP 不会向 MCU 发此命令, 忽略
    if (frame.cmd == LB_CMD_STATUS_REPORT) {
        printf("BLE: unexpected 0x03 from APP, ignored\n");
        return;
    }

    // OTA 命令 (0x0b-0x0e, v1.0.6): 根据 target 字段决定路由
    //   target=0x01(主单片机) 或 查询全部(无target) → 本地处理, 不转发 UART
    //   target=0x02(加热模块)                      → 转发 UART, 本地不处理
    if (frame.cmd >= LB_CMD_OTA_QUERY && frame.cmd <= LB_CMD_OTA_END) {
        u8 target = lb_ota_get_target(&frame);
        bool to_main = (target == 0x00 || target == LB_OTA_TARGET_MAIN_MCU);
        bool to_heat = (target == LB_OTA_TARGET_HEAT_MODULE);

        if (to_main) {
            // 主单片机: 本地处理，直接通过 BLE 应答 APP
            printf("OTA: target=0x%02X → local handler\n", target ? target : LB_OTA_TARGET_MAIN_MCU);
            if (frame.cmd < 16 && cmd_handler[frame.cmd]) {
                cmd_handler[frame.cmd](&frame);
            }
        }

        if (to_heat) {
            // 加热模块: 翻译为 UART 协议 → 串口发往加热模块
            printf("OTA: target=0x%02X → forward to UART\n", target);
            u8 uart_buf[LB_TXBUF_SIZE];
            u16 uart_len = 0;
            if (lb_translate_ble_to_uart(&frame, uart_buf, &uart_len)) {
                printf("UART==>TX[%d]: ", uart_len);
                for (u16 i = 0; i < uart_len; i++) printf("%02X ", uart_buf[i]);
                printf("\n");
                {
                    u16 dl = ((u16)uart_buf[6] << 8) | uart_buf[7];
                    if (dl) lb_dp_dump_hex(uart_buf + 8, dl);
                }
                uart_bufs_tx(UART_TYPE_1, uart_buf, uart_len);
            }
        }
        return;
    }

    // 所有其他命令(含 0x09/0x0a) → 翻译为 UART 协议 → 通过串口发给加热模块
    {
        u8 uart_buf[LB_TXBUF_SIZE];
        u16 uart_len = 0;
        if (lb_translate_ble_to_uart(&frame, uart_buf, &uart_len)) {
            printf("UART==>TX[%d]: ", uart_len);
            for (u16 i = 0; i < uart_len; i++) printf("%02X ", uart_buf[i]);
            printf("\n");
            {
                u16 dl = ((u16)uart_buf[6] << 8) | uart_buf[7];
                if (dl) lb_dp_dump_hex(uart_buf + 8, dl);
            }
            uart_bufs_tx(UART_TYPE_1, uart_buf, uart_len);
        }

        // BLE 控制/状态类命令携带 DataPoints 时，同步推送 LCD 显示
        if (frame.cmd == LB_CMD_CONTROL && frame.data && frame.data_len > 0) {
            heat_display_feed_dp(frame.data, frame.data_len);
        }
    }
#else
    // ──── 本地模式：原帧转发到串口 + 解析分发给 cmd_handler ────
    printf("UART==>TX[%d]: ", len);
    for (u16 i = 0; i < len; i++) printf("%02X ", data[i]);
    printf("\n");
    {
        u16 dl = ((u16)data[6] << 8) | data[7];
        if (dl) lb_dp_dump_hex(data + 8, dl);
    }
    uart_bufs_tx(UART_TYPE_1,data, len);

    LB_TRACE("lb_ble: rx cmd=0x%02x msg=%d len=%d\n", frame.cmd, frame.msg_flag, frame.data_len);

    if (frame.cmd < 16 && cmd_handler[frame.cmd]) {
        cmd_handler[frame.cmd](&frame);
    }
#endif
}

/**
 * @brief 初始化 UART1 硬件及协议模块内部状态
 */
void lunchbox_uart_init(u32 baud)
{
    memset(lb_rx_buf, 0, sizeof(lb_rx_buf));
    lb_rx_idx = 0;
    memset(cmd_handler, 0, sizeof(cmd_handler));
#if !LB_BRIDGE_MODE
    memset(lb_schedules, 0, sizeof(lb_schedules));
    lb_schedule_count = 0;
#endif

    // 默认设备信息
    memset(&lb_dev_info, 0, sizeof(lb_dev_info));
    u8 ble_addr[6];
    ble_get_local_bd_addr(ble_addr);
    sprintf(lb_dev_info.bt_name, "AR0MA-NY_%02X%02X", ble_addr[4], ble_addr[5]);
    memcpy(lb_dev_info.version, "01.00.00", 8);
    memcpy(lb_dev_info.model,  "SF101\0\0\0\0\0", 10);
    memcpy(lb_dev_info.mac, ble_addr, 6);

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
