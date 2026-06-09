/**
 * @file    func_lunchbox_uart.c
 * @brief   智能盒饭 - UART串口协议实现 (大端, 同步/异步双模式)
 * @note    引脚: TX=PB8(UT1TXMAP_G2_PB8), RX=PB9(UT1RXMAP_G2_PB9)
 *          接收用 bsp_uart1_get_char() 轮询，不走 ISR 回调链
 *          >1 字节字段(data_len)采用大端传输
 */

#include "include.h"
#include "func_lunchbox_uart.h"

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
static lb_cmd_handler_t   cmd_handler[256];  // 命令字 → 回调，下标即 cmd 值

//-----------------------------------------------------------------------------
// 工具
//-----------------------------------------------------------------------------

/**
 * @brief 计算协议校验和
 *
 * 从 data[0] 到 data[len-1] 逐字节累加，结果对 256 取余（取低 8 位）。
 */
static u8 lb_checksum(u8 *data, u16 len)
{
    u32 sum = 0;                                    // u32 防累加上溢，255 字节全 0xFF 也不溢出
    for (u16 i = 0; i < len; i++) sum += data[i];   
    return (u8)(sum % 256);                         // 取低 8 位，等效 sum & 0xFF，% 256 更直观
}

/** @brief 重置帧接收状态，丢弃当前未完成的帧 */
static void lb_rx_reset(void) { lb_rx_idx = 0; lb_rx_ticks = 0; }

//-----------------------------------------------------------------------------
// 帧解析
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
 * @return true  解析成功，已回调 cmd_handler，lb_rx_idx 已重置
 * @return false 数据不够/校验失败/帧太大，调用者需继续收或丢弃
 *
 * 例：lb_rx_buf 里的原始字节（16 进制，大端）:
 *      55 aa 01 00 10 00  00 02 0a 0b 27
 *      │      帧头结构体 8B      │ │ d │chk│
 *      data_len = 0x0002（大端 00 02 即 2），所以 data 有 2 个字节
 *      校验：55+aa+01+00+10+00+00+02+0a+0b = 127，取低8位 = 0x27 ✓
 */
static bool lb_frame_parse(void)
{
    // ──── 检查 1：帧缓冲区数据至少够一个完整帧头 + 1 字节 checksum ────
    if (lb_rx_idx < sizeof(lb_frame_head_t) + 1) return false;
    // sizeof(lb_frame_head_t) = 8，至少收够 9 字节才有校验和可算

    // ──── 检查 2：帧头校验，用单字节比较避免大小端问题 ────
    if (lb_rx_buf[0] != 0x55 || lb_rx_buf[1] != 0xaa) {
        // 虽然前面状态机已经做了 0x55 0xaa 的检查，但这里再防一次
        // 如果数据异常（比如 buf 被意外篡改），直接丢弃整帧
        lb_rx_reset();              // idx = 0，丢弃当前 buf 内容
        return false;
    }

    // ──── 把字节数组当作 lb_frame_head_t 结构体解释 ────
    // 注意：#pragma pack(1) 保证了结构体和字节数组布局一致
    // 注意：data_len 在线路上是大端，CPU 是小端，读后需要字节交换
    lb_frame_head_t *h = (lb_frame_head_t *)lb_rx_buf;

    // ──── 检查 3：计算完整帧长度，看是否超出缓冲或数据未收完 ────
    u16 data_len = ((u16)h->data_len << 8) | (h->data_len >> 8);  // 大端→小端
    u16 total = sizeof(lb_frame_head_t) + data_len + 1;
    //              帧头 8 字节         +  数据长度 +  校验和 1 字节
    if (total > LB_RXBUF_SIZE) {
        // 帧大得离谱（比如 data_len 字段损坏），直接丢弃，防数组越界
        lb_rx_reset();
        return false;
    }
    if (lb_rx_idx < total) {
        // 帧头表示应该收 total 字节，但当前只收了 lb_rx_idx 字节
        // 数据还没到齐，返回 false，主循环下次继续尝试
        return false;
    }

    // ──── 检查 4：校验和验证 ────
    // 把 total-1 个字节累加，和最后一字节（checksum）比较
    if (lb_checksum(lb_rx_buf, total - 1) != lb_rx_buf[total - 1]) {
        LB_TRACE("lb: chk fail\n"); // 调试模式下打印，正常模式不输出
        lb_rx_reset();              // 校验失败，丢弃
        return false;
    }

    // ──── 校验通过，组装接收帧结构体 ────
    lb_rx_frame_t rx = {
        .version  = h->version,                             // 协议版本
        .msg_flag = h->msg_flag,                            // 消息标志（请求/应答匹配用）
        .cmd      = h->cmd,                                 // 命令字
        .err_flag = h->err_flag,                            // 错误标志
        .data_len = data_len,                               // 数据长度 (已转为小端)
        .data     = data_len                               // 数据指针
                    ? lb_rx_buf + sizeof(lb_frame_head_t)   //   有数据 → 指向 data 区
                    : NULL,                                 //   无数据 → NULL
        .valid    = true,                                   // 标记为有效帧
    };

    // ──── 派发到已注册的命令处理器 ────
    LB_TRACE("lb: rx cmd=0x%02X dlen=%d\n", rx.cmd, rx.data_len);
    if (cmd_handler[rx.cmd])                // 检查该 cmd 是否注册过处理器
        cmd_handler[rx.cmd](&rx);           // 调回调，传 &rx（业务只读，不修改原始 buf）

    lb_rx_reset();                          // 帧处理完，重置 idx，准备收下一帧
    return true;
}

/**
 * @brief 帧接收超时检测
 *
 * 若已开始收帧（lb_rx_idx > 0）且超过 LB_FRAME_TIMEOUT_MS 未收到新字节，
 * 认为帧不完整，清空缓冲区。
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
 * @brief 组帧并通过 UART1 发送
 *
 * @param[in] cmd       命令字
 * @param[in] msg_flag  消息标志
 * @param[in] err       错误码（请求帧填 LB_ERR_SUCCESS）
 * @param[in] data      数据区，可为 NULL
 * @param[in] len       数据长度(字节)
 */
static void lb_send_frame(u8 cmd, u8 msg_flag, u8 err, u8 *data, u16 len)
{
    u16 off = 0;
    // 逐字节写，线上永远是 0x55 再 0xaa
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
    uart_bufs_tx(UART_TYPE_1, lb_tx_buf, off);
}

/** @brief 发送请求/命令帧，err_flag 固定为成功 */
void lunchbox_uart_send(u8 cmd, u8 msg_flag, u8 *data, u16 len)
    { lb_send_frame(cmd, msg_flag, LB_ERR_SUCCESS, data, len); }

/** @brief 发送应答帧，可指定 err_flag */
void lunchbox_uart_send_response(u8 cmd, u8 msg_flag, u8 err, u8 *data, u16 len)
    { lb_send_frame(cmd, msg_flag, err, data, len); }

static u8 lb_async_msg_flag = 0;    // 异步消息自动递增的 msg_flag

/**
 * @brief 异步发送帧（MCU 主动上报，无需主机先请求）
 *
 * 与同步通信（一问一答）不同，异步帧由 MCU 自发推送（如温度上报、故障通知）。
 * msg_flag 自动递增，主机可用它检测是否丢帧。
 *
 * @param[in] cmd   命令字，如 LB_CMD_TEMP_REPORT、LB_CMD_FAULT_REPORT
 * @param[in] data  待发送数据
 * @param[in] len   数据长度
 */
void lunchbox_uart_send_async(u8 cmd, u8 *data, u16 len)
    { lb_send_frame(cmd, lb_async_msg_flag++, LB_ERR_SUCCESS, data, len); }

//-----------------------------------------------------------------------------
// 业务状态
//-----------------------------------------------------------------------------

static lb_device_info_t lb_dev_info;                // 设备信息（应用层通过 API 填入）
static lb_schedule_t     lb_schedules[LB_SCHEDULE_MAX]; // 预约列表
static u8                lb_schedule_count;         // 当前预约条数
static u8                lb_next_schedule_id = 1;   // 自增预约 ID

// 当前属性值
static u8 lb_attr_power_switch  = 1;    // 总开关: 默认开
static u8 lb_attr_heat_mode     = 0;    // 加热模式: 默认关闭
static u8 lb_attr_battery       = 4;    // 电量: 默认满
static u8 lb_attr_charge_status = 0;    // 充电状态: 默认未充电
static u32 lb_attr_heat_duration = 0;   // 加热时长(分钟)
static u32 lb_attr_remain_time   = 0;   // 剩余时间(分钟)
static u8 lb_attr_heat_temp     = 0;    // 加热温度: 默认40°C
static u8 lb_attr_language      = 0;    // 语言: 默认中文
static u8 lb_attr_fault         = 0;    // 故障: 默认正常

//-----------------------------------------------------------------------------
// DataPoint 编码工具
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
    default: return 0;
    }
    return 1;
}

/** @brief 解析 APP 下发的 DataPoint 并写入属性 */
static u8 lb_attr_write(u8 *data, u16 len)
{
    u16 off = 0;
    while (off + 4 <= len) {
        u8  dpid    = data[off];
        u8  type    = data[off + 1];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];
        if (off + 4 + val_len > len) break;
        u8 *val = data + off + 4;

        switch (dpid) {
        case LB_DPID_POWER_SWITCH:  if (val_len >= 1) lb_attr_power_switch  = val[0]; break;
        case LB_DPID_HEAT_MODE:     if (val_len >= 1) lb_attr_heat_mode     = val[0]; break;
        case LB_DPID_HEAT_DURATION: if (val_len >= 4) lb_attr_heat_duration = ((u32)val[0]<<24)|((u32)val[1]<<16)|((u32)val[2]<<8)|val[3]; break;
        case LB_DPID_HEAT_TEMP:     if (val_len >= 1) lb_attr_heat_temp     = val[0]; break;
        case LB_DPID_LANGUAGE:      if (val_len >= 1) lb_attr_language      = val[0]; break;
        default: break; // 只读属性 (3,4,6,9) 不允许 APP 写入
        }
        off += 4 + val_len;
    }
    return LB_ERR_SUCCESS;
}

void lunchbox_set_device_info(lb_device_info_t *info) { if (info) memcpy(&lb_dev_info, info, sizeof(lb_device_info_t)); }

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
    default: break;
    }
}

//-----------------------------------------------------------------------------
// 预约管理
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

/** @brief 从帧数据解析预约记录 */
static void lb_schedule_parse(lb_schedule_t *s, u8 *data)
{
    s->id       = data[0];
    memcpy(s->name, data + 1, 32);
    s->time     = ((u32)data[33] << 24) | ((u32)data[34] << 16) | ((u32)data[35] << 8) | data[36];
    s->temp     = data[37];
    s->duration = data[38];
    s->enabled  = data[39];
    s->repeat   = data[40];
}

/** @brief 将预约记录编码到 buf（41 字节），返回写入长度 */
static u16 lb_schedule_encode(u8 *buf, lb_schedule_t *s)
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
    for (u8 dpid = 1; dpid <= 9; dpid++) {
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
// 命令处理器
//-----------------------------------------------------------------------------

/**
 * @brief 0x01 — 查询产品信息
 * APP 发送: [timestamp:4B] → MCU 返回设备信息
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

/**
 * @brief 0x02 — 查询设备动态属性
 * APP 发送: 无数据 → MCU 返回所有 DataPoint
 */
static u8 lb_handler_dynamic_attr(lb_rx_frame_t *rx)
{
    u8 buf[256];
    u16 len = lb_encode_all_attrs(buf);
    lunchbox_uart_send_response(LB_CMD_DYNAMIC_ATTR, rx->msg_flag, LB_ERR_SUCCESS, buf, len);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x04 — 查询预约列表
 * APP 发送: 无数据 → MCU 逐条返回预约记录
 */
static u8 lb_handler_schedule_list(lb_rx_frame_t *rx)
{
    if (lb_schedule_count == 0) {
        // 无预约，返回空数据
        u8 empty[2] = { 0, 0 };  // 总条数=0, 序号=0
        lunchbox_uart_send_response(LB_CMD_SCHEDULE_LIST, rx->msg_flag, LB_ERR_SUCCESS, empty, 2);
        return LB_ERR_SUCCESS;
    }

    for (u8 i = 0; i < lb_schedule_count; i++) {
        u8 buf[43]; // 2(总条数+序号) + 41(预约数据)
        buf[0] = lb_schedule_count;         // 总条数
        buf[1] = i + 1;                     // 当前序号(从1开始)
        lb_schedule_encode(buf + 2, &lb_schedules[i]);
        lunchbox_uart_send_response(LB_CMD_SCHEDULE_LIST, rx->msg_flag, LB_ERR_SUCCESS, buf, 43);
    }
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x05 — 新增预约
 * APP 发送: 预约数据(40B) → MCU 返回分配的 ID
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

    lb_schedule_t *s = &lb_schedules[lb_schedule_count];
    memset(s, 0, sizeof(lb_schedule_t));
    lb_schedule_parse(s, rx->data);
    s->id = lb_schedule_new_id();   // MCU 自动分配 ID（忽略 APP 传入值）
    lb_schedule_count++;

    u8 assigned_id = s->id;
    lunchbox_uart_send_response(LB_CMD_SCHEDULE_ADD, rx->msg_flag, LB_ERR_SUCCESS, &assigned_id, 1);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x06 — 修改预约
 * APP 发送: 预约数据(40B) → MCU 返回成功/失败
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

    lb_schedule_parse(&lb_schedules[idx], rx->data);
    lb_schedules[idx].id = id;  // 保持原 ID
    lunchbox_uart_send_response(LB_CMD_SCHEDULE_MODIFY, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x07 — 删除预约
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
        memcpy(&lb_schedules[idx], &lb_schedules[lb_schedule_count], sizeof(lb_schedule_t));
    lunchbox_uart_send_response(LB_CMD_SCHEDULE_DELETE, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x08 — 状态查询
 * APP 发送: 无数据 → 触发 0x03 全量上报
 */
static u8 lb_handler_status_query(lb_rx_frame_t *rx)
{
    lunchbox_report_all_attrs(); // ← 先发 0x03 异步帧
    lunchbox_uart_send_response(LB_CMD_STATUS_QUERY, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);
    return LB_ERR_SUCCESS;       // ← 再发 0x08 同步应答
}

/**
 * @brief 0x09 — 升级查询 (桩)
 */
static u8 lb_handler_ota_query(lb_rx_frame_t *rx)
{
    u8 mode = 0x00;  // 0=不支持 MCU 升级
    lunchbox_uart_send_response(LB_CMD_OTA_QUERY, rx->msg_flag, LB_ERR_SUCCESS, &mode, 1);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x0a — 升级启动 (桩)
 */
static u8 lb_handler_ota_start(lb_rx_frame_t *rx)
{
    u8 status = 0x00;  // 收到升级指令
    lunchbox_uart_send_response(LB_CMD_OTA_START, rx->msg_flag, LB_ERR_SUCCESS, &status, 1);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x0b — 升级包传输 (桩)
 */
static u8 lb_handler_ota_data(lb_rx_frame_t *rx)
{
    lunchbox_uart_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x0c — 升级结束 (桩)
 */
static u8 lb_handler_ota_end(lb_rx_frame_t *rx)
{
    u8 result = 0x01;  // 升级成功
    lunchbox_uart_send_response(LB_CMD_OTA_END, rx->msg_flag, LB_ERR_SUCCESS, &result, 1);
    return LB_ERR_SUCCESS;
}

//-----------------------------------------------------------------------------
// 注册 / 初始化 / 主循环
//-----------------------------------------------------------------------------

/** @brief 注册所有命令处理器 */
void lunchbox_uart_init_handlers(void)
{
    lunchbox_uart_reg_handler(LB_CMD_PRODUCT_INFO,    lb_handler_product_info);
    lunchbox_uart_reg_handler(LB_CMD_DYNAMIC_ATTR,    lb_handler_dynamic_attr);
    lunchbox_uart_reg_handler(LB_CMD_SCHEDULE_LIST,   lb_handler_schedule_list);
    lunchbox_uart_reg_handler(LB_CMD_SCHEDULE_ADD,    lb_handler_schedule_add);
    lunchbox_uart_reg_handler(LB_CMD_SCHEDULE_MODIFY, lb_handler_schedule_modify);
    lunchbox_uart_reg_handler(LB_CMD_SCHEDULE_DELETE, lb_handler_schedule_delete);
    lunchbox_uart_reg_handler(LB_CMD_STATUS_QUERY,    lb_handler_status_query);
    lunchbox_uart_reg_handler(LB_CMD_OTA_QUERY,       lb_handler_ota_query);
    lunchbox_uart_reg_handler(LB_CMD_OTA_START,       lb_handler_ota_start);
    lunchbox_uart_reg_handler(LB_CMD_OTA_DATA,        lb_handler_ota_data);
    lunchbox_uart_reg_handler(LB_CMD_OTA_END,         lb_handler_ota_end);

    // 初始化默认设备信息
    memset(&lb_dev_info, 0, sizeof(lb_dev_info));
    memcpy(lb_dev_info.bt_name, "AR0MA-NY_0000\0\0\0", 16);
    memcpy(lb_dev_info.version, "01.00.00", 8);
    memcpy(lb_dev_info.model,  "SF101\0\0\0\0\0", 10);
    // MAC/SN/Color 由应用层通过 lunchbox_set_device_info() 写入
}

/** @brief 为指定命令字注册处理回调，后注册覆盖先注册 */
void  lunchbox_uart_reg_handler(u8 cmd, lb_cmd_handler_t h) { cmd_handler[cmd] = h; }

/** @brief 初始化 UART1 及协议模块内部状态，见 func_lunchbox_uart.h */
void lunchbox_uart_init(u32 baud)
{
    memset(lb_rx_buf, 0, sizeof(lb_rx_buf));
    lb_rx_idx = 0;
    memset(cmd_handler, 0, sizeof(cmd_handler));
    memset(lb_schedules, 0, sizeof(lb_schedules));
    lb_schedule_count = 0;

    uart_t uart1;
    memset(&uart1, 0, sizeof(uart1));
    uart1.type   = UART_TYPE_1;
    uart1.tx_map = UT1TXMAP_G2_PB8;
    uart1.rx_map = UT1RXMAP_G2_PB9;
    uart1.baud   = baud;
    uart1.rx_isr = lb_dummy_isr;    // uart_init 要求 rx_isr 非 NULL，给个空函数占位

    Err_Uart ret = bsp_uart1_init(&uart1, lb_ring_buf, sizeof(lb_ring_buf));
    if (ret != ERR_UART_SUCCESS) {
        printf("lb_uart: init fail, err=%d\n", ret); return;
    }
    printf("lb_uart: init ok TX=PB8 RX=PB9 baud=%d\n", baud);
}

/**
 * @brief 主循环中周期调用的接收处理函数
 *
 * 设计思路：不在 ISR 里做帧解析（ISR 必须短），而是在主循环里主动取数据。
 * bsp_uart1_get_char() 是非阻塞的——有数据就返回 true，没数据立刻返回 false。
 *
 * 接收状态机（lb_rx_idx 既作写入位置，也作状态指示）：
 *   lb_rx_idx == 0   等 0x55   （帧头第一个字节）
 *   lb_rx_idx == 1   等 0xaa   （帧头第二个字节）
 *   lb_rx_idx >= 2   接收帧体   （帧头 + 数据 + 校验，组装完整帧）
 *
 * 例：硬件收到 5 个字节 → 内部调用 lb_dummy_isr(不会影响我们的处理)
 *     假设数据流为 [0x77, 0x55, 0xaa, 0x01, 0x03, 0x06, 0xFF, 0x00, 0x0A]
 */
void lunchbox_uart_process(void)
{
    u8 ch;

    // ──── 第 1 步：从 BSP 环形缓冲区取字节，存入帧缓冲区 ────
    while (bsp_uart1_get_char(&ch)) {           // 每次取 1 字节，取完自动退出

        // --- 状态 0：死等帧头第一个字节 0x55 ---
        if (lb_rx_idx == 0) {
            // LB_FRAME_HEADER = 0x55aa，右移 8 位取高字节 → 0x55
            if (ch != (u8)(LB_FRAME_HEADER >> 8))
                continue;                       // 不是 0x55？跳过，不存入 buf，idx 保持 0
            // 是 0x55 → 不执行 continue，穿过检查，落入后面的"存入 buf"
        }

        // --- 状态 1：帧头第二个字节必须是 0xaa ---
        if (lb_rx_idx == 1) {
            if (ch != (u8)LB_FRAME_HEADER) {    // LB_FRAME_HEADER 低字节 = 0xaa
                lb_rx_idx = 0;                  // 不是 0xaa？前功尽弃，复位重新等 0x55
                continue;                       // 跳过本轮
            }
            // 是 0xaa → 继续往下走
        }

        // --- 通过帧头检查 → 存入帧缓冲区 ---
        if (lb_rx_idx < LB_RXBUF_SIZE) {
            lb_rx_buf[lb_rx_idx] = ch;          // 把收到的字节放进帧缓冲区
            lb_rx_idx++;                        // 写入位置后移
            lb_rx_ticks = tick_get();           // 刷新时间戳（超时用）
        } else {
            // 缓冲区满了（帧太大或垃圾数据），全部丢弃
            lb_rx_reset();                      // lb_rx_idx = 0
        }
    }

    // ──── 第 2 步：超时检查 ────
    // 如果收到一半卡住了（比如发了 5 个字节后断电），300ms 后自动丢弃半帧
    lb_timeout_check();

    // ──── 第 3 步：尝试帧解析 ────
    // 帧头至少 8 字节 + 1 字节校验 = 至少 9 字节才够一帧
    while (lb_rx_idx >= sizeof(lb_frame_head_t) + 1) {
        if (!lb_frame_parse())                  // 校验失败返回 false → break 退出
            break;                              // 等更多数据到齐再试
        // 校验成功 → lb_frame_parse 内部调了 lb_rx_reset()
        //          → lb_rx_idx 回到 0，准备收下一帧
        //          → while 条件不满足，自然退出
    }
}

//-----------------------------------------------------------------------------
// 自测
//-----------------------------------------------------------------------------

#if LB_SELFTEST_EN

/**
 * @brief 自测 echo 处理器：打印收到的帧并原样应答
 * @param[in] rx  解析后的接收帧
 * @return LB_ERR_SUCCESS
 */
static u8 lb_test_echo_handler(lb_rx_frame_t *rx)
{
    // 打印收到的帧信息（从 PB3 输出）
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

/** @brief 自测入口：发送就绪字符串并为所有命令字注册 echo 回调 */
void func_lunchbox_uart_test(void)
{
    bsp_uart1_str_tx("LUNCHBOX_UART_OK\r\n");

    for (u8 c = 0; c < 255; c++) {
        lunchbox_uart_reg_handler(c, lb_test_echo_handler);
    }
}

#endif // LB_SELFTEST_EN

#endif // FUNC_LUNCHBOX_UART_EN
