/**
 * @file    lb_proto.c
 * @brief   饭盒 0x55AA 帧协议层实现 — 纯编解码器
 * @note    协议: MCU通信协议.md v1.0.7 / 蓝牙通讯协议1.0.7.md (同帧格式)
 *          本文件无 I/O、无全局状态 (解析器状态由调用方持有实例),
 *          收发调度与业务分发见 串口应用层(未移植)。
 */
#include "include.h"
#include "lb_proto.h"

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// 校验和 / 组包
//-----------------------------------------------------------------------------

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

u16 lb_proto_build_frame(u8 *out, u8 cmd, u8 msg_flag, u8 err,
                         const u8 *data, u16 len)
{
    if (sizeof(lb_frame_head_t) + len + 1 > LB_TXBUF_SIZE) {
        printf("lb_proto: frame too large, len=%d\n", len);
        return 0;
    }
    u16 off = 0;
    out[off++] = (u8)(LB_FRAME_HEADER >> 8);  // 0x55
    out[off++] = (u8)LB_FRAME_HEADER;          // 0xaa
    out[off++] = LB_FRAME_VERSION;
    out[off++] = msg_flag;
    out[off++] = cmd;
    out[off++] = err;
    out[off++] = (u8)(len >> 8);               // data_len 高字节 (大端)
    out[off++] = (u8)(len & 0xFF);             // data_len 低字节 (大端)
    if (data && len) {
        memcpy(out + off, data, len);
        off += len;
    }
    out[off] = lb_checksum(out, off);
    off++;
    return off;
}

//-----------------------------------------------------------------------------
// 解包: 帧解析器
//-----------------------------------------------------------------------------

void lb_proto_parser_reset(lb_proto_parser_t *p)
{
    p->len = 0;
    p->tick = 0;
}

bool lb_proto_parser_feed(lb_proto_parser_t *p, u8 ch, lb_rx_frame_t *out)
{
    // 帧头搜索: 空态只认 0x55, 第二字节必须是 0xAA
    if (p->len == 0) {
        if (ch != (u8)(LB_FRAME_HEADER >> 8)) {
            return false;
        }
    } else if (p->len == 1) {
        if (ch != (u8)LB_FRAME_HEADER) {
            // 55 55 AA...: 本字节是 0x55 时可继续充当帧头首字节
            if (ch == (u8)(LB_FRAME_HEADER >> 8)) {
                p->len = 1;
            } else {
                p->len = 0;
            }
            return false;
        }
    }

    if (p->len >= LB_RXBUF_SIZE) {   // 防御: 状态机不变量下不会发生
        p->len = 0;
        return false;
    }
    p->buf[p->len] = ch;
    p->len++;
    p->tick = tick_get();

    if (p->len < sizeof(lb_frame_head_t) + 1) {
        return false;                            // 连最小帧(空数据)都不够
    }

    lb_frame_head_t *h = (lb_frame_head_t *)p->buf;

    // data_len 大端→小端, 计算完整帧长度
    u16 data_len = ((u16)h->data_len << 8) | (h->data_len >> 8);
    u16 total = sizeof(lb_frame_head_t) + data_len + 1;

    if (total > LB_RXBUF_SIZE) {
        p->len = 0;                              // 长度非法(假帧头), 整帧丢弃
        return false;
    }
    if (p->len < total) {
        return false;                            // 数据还没到齐
    }

    // 帧收齐: 无论校验好坏本帧都结束, 下一字节重新找帧头
    p->len = 0;
    if (lb_checksum(p->buf, total - 1) != p->buf[total - 1]) {
        return false;                            // 校验失败, 丢弃
    }

    // 填充输出帧 (data 指向内部缓冲, 下次 feed 前有效)
    out->version  = h->version;
    out->msg_flag = h->msg_flag;
    out->cmd      = h->cmd;
    out->err_flag = h->err_flag;
    out->data_len = data_len;
    out->data     = data_len ? p->buf + sizeof(lb_frame_head_t) : NULL;
    out->valid    = true;
    return true;
}

bool lb_proto_parser_timeout(lb_proto_parser_t *p, u32 timeout_ms)
{
    if (p->len && tick_check_expire(p->tick, timeout_ms)) {
        lb_proto_parser_reset(p);
        return true;
    }
    return false;
}

u16 lb_proto_frame_total(const lb_rx_frame_t *rx)
{
    return (u16)(sizeof(lb_frame_head_t) + rx->data_len + 1);
}

//-----------------------------------------------------------------------------
// DataPoint 编码工具
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
            // 平台 printf 不支持 %s 的精度限定(%.8s), 字段又是定长不补 0,
            // 直接打印会越界读到下一个字段, 故拷进临时缓冲显式补结束符
            char str[33];
            if (len >= 16) { memcpy(str, data,      16); str[16] = 0; printf("BLEname=%s\n", str); }
            if (len >= 24) { memcpy(str, data + 16,  8); str[8]  = 0; printf("version=%s\n", str); }
            if (len >= 34) { memcpy(str, data + 24, 10); str[10] = 0; printf("modeltype=%s\n", str); }
            if (len >= 40) printf("MAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
                                  data[34], data[35], data[36], data[37], data[38], data[39]);
            if (len >= 72) { memcpy(str, data + 40, 32); str[32] = 0; printf("SN=%s\n", str); }
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
            char nm[33];
            memcpy(nm, data + 4, 32); nm[32] = 0;   // name 定长不补 0, 需显式截断
            printf("Schedule[%u/%u] ALL=%u now_id=%u mode=%u ID=%u name=%s TIME=%lu temp=%u time=%umin status=%u rep=0x%02X\n",
                   now_id, ALL, ALL, now_id, mode, ID, nm, (unsigned long)TIME, temp, time, status, rep);
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
            char nm[33];
            memcpy(nm, data + 2, 32); nm[32] = 0;
            printf("mode=%u ID=%u name=%s TIME=%lu temp=%u time=%umin status=%u rep=0x%02X\n",
                   mode, ID, nm, (unsigned long)TIME, temp, time, status, rep);
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
            char nm[33];
            memcpy(nm, data + 2, 32); nm[32] = 0;
            printf("mode=%u ID=%u name=%s TIME=%lu temp=%u time=%umin status=%u rep=0x%02X\n",
                   mode, ID, nm, (unsigned long)TIME, temp, time, status, rep);
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

#endif // FUNC_LUNCHBOX_UART_EN
