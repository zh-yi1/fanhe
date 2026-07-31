/**
 * @file    lb_proto.h
 * @brief   饭盒 0x55AA 帧协议层 — 纯编解码器 (无 I/O、无传输策略)
 * @note    帧格式 (大端): [0x55][0xAA][ver][flag][cmd][err][dlen_H][dlen_L][data...][chk]
 *
 *          本层只做两件事:
 *            1. 解包: 字节流 → 完整帧 (lb_proto_parser_t, 串口/蓝牙可各建实例)
 *            2. 组包: 字段 → 帧字节 (lb_proto_build_frame, 串口帧和蓝牙帧同格式)
 *          附带 DataPoint 编解码与协议调试打印。
 *
 *          不含: 发送队列/重试 (见 串口发送队列(未移植))、通道选择、
 *                业务分发 (见 串口应用层(未移植))。本层不依赖任何其他模块。
 */
#ifndef __LB_PROTO_H
#define __LB_PROTO_H

#include "include.h"

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// 协议常量
//-----------------------------------------------------------------------------
#define LB_FRAME_HEADER         0x55aa      // 帧头固定值
#define LB_FRAME_VERSION        0x00        // 协议版本号

#define LB_RXBUF_SIZE           256         // 单帧解析缓冲区大小(字节)
#define LB_TXBUF_SIZE           256         // 组帧缓冲区大小(字节)
#define LB_FRAME_TIMEOUT_MS     300         // 帧超时(毫秒), 超时未收完一帧则丢弃

//-----------------------------------------------------------------------------
// 属性 ID（DataPoint dpid）— 蓝牙通讯协议1.0.5.md §4 + MCU通信协议.md §4
//-----------------------------------------------------------------------------
enum {
    LB_DPID_POWER_SWITCH    = 1,        // 总开关: bool, 1=开 0=关
    LB_DPID_HEAT_MODE       = 2,        // 加热模式: enum, 0=关 1=自定义 2=鸡腿 3=意面 4=预约 5=保温
    LB_DPID_BATTERY         = 3,        // 电量: enum, 0=没电 1=低 2=中 3=高 4=满
    LB_DPID_CHARGE_STATUS   = 4,        // 充电状态: enum, 0=未充电 1=充电中 2=已充满
    LB_DPID_HEAT_DURATION   = 5,        // 加热时长: value(4B), 60~210 分钟
    LB_DPID_REMAIN_TIME     = 6,        // 剩余加热时间: value(4B), 分钟
    LB_DPID_HEAT_TEMP       = 7,        // 加热温度: enum, 0=40°C ~ 6=100°C
    LB_DPID_LANGUAGE        = 8,        // 语言: enum, 0=中文 1=英文...
    LB_DPID_FAULT           = 9,        // 故障: enum, 0=正常 1=高温告警
    LB_DPID_HEAT_ENABLE     = 10,       // 是否加热: bool, 0=停止 1=加热 (v1.0.5 新增)
    LB_DPID_MCU_VERSION     = 13,       // MCU版本号: value(4B), 固件版本号 (v1.0.7 新增)
    LB_DPID_TIME_SYNC       = 11,       // app同步时间戳: value(4B) unix时间 (APP→加热模块设置时间)
    LB_DPID_KEY_NOTIFY      = 12,       // 模组按键通知: enum, 0-9, MCU→加热模块通知按键按下
    LB_DPID_RTC_TIME        = 14,       // rtc的unix时间: value(4B) unix时间 (加热模块→MCU上报设备时间, v1.0.7新增)
};

// 故障码 (DP9 取值, 见《MCU通信协议》§4.1.6)
// 注: 文档把 DP9 描述为 0/1 两值, fault_code 标注为"MCU 内部, 通过日志区分",
//     实际模块是否把 fault_code 原值放进 DP9 需实测确认。
enum {
    LB_FAULT_NONE           = 0x00,     // 无故障
    LB_FAULT_DRY_BURN       = 0x01,     // 干烧超温 (NTC > 105°C)
    LB_FAULT_TEMP_RISE_FAST = 0x02,     // 温升过快 (> 11°C/秒)
    LB_FAULT_NTC_BROKEN     = 0x03,     // NTC 传感器故障 (短路/开路)
    LB_FAULT_OVER_CURRENT   = 0x04,     // 加热丝过流 (> 10A)
    LB_FAULT_COVER_5V_SHORT = 0x05,     // 上盖 5V 短路
    LB_FAULT_NTC_NO_RESP    = 0x06,     // NTC 无响应 (5 分钟未达目标温度)
    LB_FAULT_NTC_UNPLUG     = 0x07,     // NTC 未连接 (加热中温度恒 0 达 20 秒)
    LB_FAULT_NTC_ABNORMAL   = 0x08,     // NTC 异常 (非加热态温度 >100°C 达 20 秒)
    LB_FAULT_BT_HEARTBEAT   = 0x09,     // 蓝牙模组心跳超时
    LB_FAULT_LOW_BATTERY    = 0x0a,     // 低电上报
};

// 加热模式 (DP2 取值)
enum {
    LB_MODE_OFF     = 0,    // 关
    LB_MODE_CUSTOM  = 1,    // 自定义加热
    LB_MODE_CHICKEN = 2,    // 鸡腿
    LB_MODE_PASTA   = 3,    // 意面
    LB_MODE_RESERVE = 4,    // 预约
    LB_MODE_WARM    = 5,    // 保温
    LB_MODE_MAX     = LB_MODE_WARM,
};

// DataPoint 数据类型
#define LB_DP_TYPE_BOOL     0x01
#define LB_DP_TYPE_VALUE    0x02
#define LB_DP_TYPE_ENUM     0x04

//-----------------------------------------------------------------------------
// BLE 命令字 (蓝牙通讯协议1.0.5.md — APP ↔ MCU)
//-----------------------------------------------------------------------------
enum {
    // --- 同步命令：APP 请求 → MCU 应答 ---
    LB_CMD_PRODUCT_INFO     = 0x01,     // [同步] 查询产品信息 (v1.0.7: 同时透传加热模块)
    LB_CMD_DYNAMIC_ATTR     = 0x02,     // [同步] 查询设备动态属性
    LB_CMD_CONTROL          = 0x04,     // [同步] 控制指令 (DataPoints修改属性, v1.0.5新增)
    LB_CMD_SCHEDULE_LIST    = 0x05,     // [同步] 查询预约列表
    LB_CMD_SCHEDULE_ADD     = 0x06,     // [同步] 新增预约
    LB_CMD_SCHEDULE_MODIFY  = 0x07,     // [同步] 修改预约
    LB_CMD_SCHEDULE_DELETE  = 0x08,     // [同步] 删除预约
    LB_CMD_MODE_QUERY       = 0x09,     // [同步] 获取指定模式信息
    LB_CMD_MODE_MODIFY      = 0x0a,     // [同步] 修改指定模式信息
    // 0x0b 升级查询已删除 (v1.0.7)
    LB_CMD_OTA_START        = 0x0c,     // [同步] 升级启动
    LB_CMD_OTA_DATA         = 0x0d,     // [同步] 升级包传输
    LB_CMD_OTA_END          = 0x0e,     // [同步] 升级结束

    // --- 异步命令：MCU 主动推送 ---
    LB_CMD_STATUS_REPORT    = 0x03,     // [异步] 状态上报（属性变化/故障通知）
};

//-----------------------------------------------------------------------------
// MCU UART 命令字 (MCU通信协议.md v1.0.7 — MCU ↔ 加热模块)
//-----------------------------------------------------------------------------
enum {
    LB_UART_CMD_DYNAMIC     = 0x01,     // 查询设备动态属性+状态上报(合并)
    LB_UART_CMD_SCHEDULE    = 0x02,     // 查询预约列表
    LB_UART_CMD_SCHEDULE_OP = 0x03,     // 新增/修改/删除预约(合并)
    LB_UART_CMD_OTA         = 0x04,     // OTA(合并start/transfer/end)
    LB_UART_CMD_HEARTBEAT   = 0x05,     // 心跳包: MCU↔加热模块, 验证双方在线 (MCU协议 v1.0.7 §6.1)
};

// 错误标志
enum {
    LB_ERR_SUCCESS = 0x00,
    LB_ERR_EXEC_FAIL = 0x01,
};

//-----------------------------------------------------------------------------
// 帧结构（#pragma pack(1) 保证与线上字节序一致，禁止编译器插入填充）
//-----------------------------------------------------------------------------
#pragma pack(1)

/**
 * @brief 协议帧头（发送/接收共用）
 *
 * 一条完整帧 = lb_frame_head_t + data[data_len] + checksum(1B)
 * checksum 为从 head.header 开始逐字节累加后对256取余
 */
typedef struct {
    u16 header;             // 帧头，固定 0x55aa
    u8  version;            // 协议版本号
    u8  msg_flag;           // 消息标志，请求与应答保持一致，用于匹配
    u8  cmd;                // 命令字，见 LB_CMD_* / LB_UART_CMD_* 枚举
    u8  err_flag;           // 错误标志，0=成功，其他见 LB_ERR_* 枚举
    u16 data_len;           // 数据区长度(字节)，大端序
} lb_frame_head_t;

#pragma pack()

/**
 * @brief 解析后的接收帧
 *
 * data 指针直接指向解析器内部缓冲区，非独立拷贝。
 * 有效期: 到下一次对同一解析器调用 feed 为止, 需要保留须自行拷贝。
 */
typedef struct {
    u8  version;            // 协议版本号
    u8  msg_flag;           // 消息标志
    u8  cmd;                // 命令字
    u8  err_flag;           // 错误标志
    u16 data_len;           // 数据区长度(字节)
    u8  *data;              // 指向数据区首字节，data_len==0 时为 NULL
    bool valid;             // 帧校验是否通过（true=通过）
} lb_rx_frame_t;

/** @brief BLE 发送函数类型 (应用层 BLE 通道用) */
typedef void (*lb_ble_tx_fn_t)(u8 *data, u16 len);

//-----------------------------------------------------------------------------
// 解包: 帧解析器 (每条字节流一个实例, 串口/蓝牙可各建一个)
//-----------------------------------------------------------------------------

typedef struct {
    u8  buf[LB_RXBUF_SIZE];   // 当前帧积累缓冲 (帧总是从 buf[0] 开始)
    u16 len;                  // 已积累字节数
    u32 tick;                 // 最近一次进字节的时刻 (残帧超时用)
} lb_proto_parser_t;

/** @brief 复位解析器, 丢弃所有已积累字节 */
void lb_proto_parser_reset(lb_proto_parser_t *p);

/**
 * @brief 喂入一个字节, 边收边解
 *
 * 内部做帧头搜索 (空态只认 0x55, 第二字节须为 0xAA), 帧刚好收齐且
 * 校验通过时填入 out 并返回 true; 坏帧 (长度非法/校验失败) 静默丢弃。
 * @return true=out 填入一帧; false=帧未完成或字节被丢弃
 */
bool lb_proto_parser_feed(lb_proto_parser_t *p, u8 ch, lb_rx_frame_t *out);

/**
 * @brief 残帧超时检查 (主循环周期调用, 不依赖新字节到达)
 * @return true=丢弃了超时残帧
 */
bool lb_proto_parser_timeout(lb_proto_parser_t *p, u32 timeout_ms);

/** @brief 当前帧在缓冲区中的原始字节总长 (刚 next 出的帧, 打日志用) */
u16 lb_proto_frame_total(const lb_rx_frame_t *rx);

//-----------------------------------------------------------------------------
// 组包
//-----------------------------------------------------------------------------

/** @brief 计算协议校验和: 逐字节累加对 256 取余 */
u8 lb_checksum(u8 *data, u16 len);

/**
 * @brief 组一条完整 0x55AA 帧到 out (须容纳 LB_TXBUF_SIZE)
 * @return 帧总长, 0=数据过长失败
 */
u16 lb_proto_build_frame(u8 *out, u8 cmd, u8 msg_flag, u8 err,
                         const u8 *data, u16 len);

//-----------------------------------------------------------------------------
// DataPoint 编解码 / 调试打印
//-----------------------------------------------------------------------------

u16 lb_dp_encode(u8 *buf, u8 dpid, u8 type, u8 *val, u16 val_len);
u16 lb_dp_encode_bool(u8 *buf, u8 dpid, u8 val);
u16 lb_dp_encode_enum(u8 *buf, u8 dpid, u8 val);
u16 lb_dp_encode_value(u8 *buf, u8 dpid, u32 val);

/** @brief 扫描 DataPoint 缓冲区，查找指定 dpid 的 bool/enum 首字节 */
bool lb_dp_scan_bool(const u8 *data, u16 len, u8 dpid, u8 *val);

/** @brief 检查 DataPoints 数据中是否包含按键通知 (dpid=12) */
bool lb_data_is_key_notify(u8 *data, u16 len);

/** @brief 遍历 DataPoints 打印 hex + 可读描述 (调试) */
void lb_dp_dump_hex(const u8 *data, u16 data_len);

/** @brief BLE 帧协议级解析打印; is_rx: true=APP→MCU, false=MCU→APP */
void lb_ble_dump_frame(u8 cmd, const u8 *data, u16 len, bool is_rx);

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __LB_PROTO_H
