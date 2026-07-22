/*
    智能盒饭 - UART串口协议 (MCU通信协议.md v1.0.7)
    智能盒饭 - BLE蓝牙协议 (蓝牙通讯协议1.0.7.md v1.0.7)

    引脚: TX=PB8=UT1TXMAP_G2_PB8
          RX=PB9=UT1RXMAP_G2_PB9
           帧格式: 帧头(2B) + 版本(1B) + 消息标志(1B) + 命令字(1B)
                + 错误标志(1B) + 数据长度(2B-BE,大端) + 数据(xB) + 校验和(1B)
         通信机制:
             同步: 常规通信一问一答, msg_flag 与请求帧一致, cmd 与请求帧一致
             异步: MCU 主动上报(温控/故障), msg_flag 自增, 无需主机先请求
 */
#ifndef __FUNC_LUNCHBOX_UART_H
#define __FUNC_LUNCHBOX_UART_H

#include "include.h"

//是否开启串口协议：
#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// 协议常量
//-----------------------------------------------------------------------------
#define LB_FRAME_HEADER         0x55aa      // 帧头固定值
#define LB_FRAME_VERSION        0x00        // 协议版本号

#define LB_BAUD                 115200      // 波特率（按需修改）
#define LB_SELFTEST_EN          0           // 自测开关：1=开启echo，0=关闭
#define LB_BRIDGE_MODE          1           // 1=翻译桥(加热模块已到), 0=本地处理(调试用)
#define LB_RXBUF_SIZE           256         // 接收缓冲区大小(字节)
#define LB_TXBUF_SIZE           256         // 发送缓冲区大小(字节)
#define LB_FRAME_TIMEOUT_MS     300         // 帧超时时间(毫秒)，超过此时间未收完一帧则丢弃

// RTCCNT 从 2020-01-01 起算(秒), 且 RTCCNT 存的是北京时间(UTC+8)
// tm_to_time 的基准也是 2020-01-01 00:00:00 (北京时间)
// 北京时间 2020-01-01 00:00:00 = UTC 2019-12-31 16:00:00
// 其 Unix 时间戳 = (50年×365天 + 12闰日) × 86400秒 - 8小时 = 1,577,808,000秒
#define LB_RTC_UNIX_OFFSET      (1577836800u - 8*3600)   // = 1577808000

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

// BLE 协议属性 (与 UART 协议共用 ID 空间，按协议层区分用途)
#define LB_DPID_RESERVATION_TIME    12  // 预约开始时间: value(4B) unix时间 (v1.0.10 新增, LCD→BLE上报)

#define LB_HEAT_DURATION_MIN_MIN          30      // 加热时长下限(分钟)，协议范围 30-210 (v1.0.7 §4)
#define LB_HEAT_DURATION_MAX_MIN          210     // 加热时长上限(分钟)，协议范围 30-210 (v1.0.7 §4)

// DataPoint 数据类型
#define LB_DP_TYPE_BOOL     0x01
#define LB_DP_TYPE_VALUE    0x02
#define LB_DP_TYPE_ENUM     0x04

//-----------------------------------------------------------------------------
// BLE 预约记录 (蓝牙通讯协议1.0.5.md — 41B payload)
//-----------------------------------------------------------------------------
#define LB_SCHEDULE_MAX         10      // 最大预约条数

typedef struct {
    u8  id;                             // 唯一标识 (1~255, 0=无效/自动分配)
    char name[32];                      // 预约名称, 不足补0
    u32 time;                           // 开始加热的Unix时间戳（秒, 大端）
    u8  temp;                           // 温度档位 (0~10)
    u8  duration;                       // 加热时长（分钟）
    u8  enabled;                        // 0=关闭, 1=开启
    u8  repeat;                         // 重复周期位掩码 (bit7=保留默认1)
} lb_schedule_ble_t;

//-----------------------------------------------------------------------------
// MCU UART 预约 — 新增/修改/删除 (MCU通信协议.md 0x03 — 42B payload)
//-----------------------------------------------------------------------------
typedef struct {
    u8  action;                         // 0=删除, 1=自定义加热, 2=鸡腿模式
    u8  id;                             // 预约ID (删除时=目标ID, 新增时>=6)
    char name[32];                      // 预约名称
    u32 time;                           // unix触发时间（秒, 大端）
    u8  temp;                           // 温度档位
    u8  duration;                       // 加热时长（分钟）
    u8  enabled;                        // 0=关闭, 1=开启
    u8  repeat;                         // 重复周期 (bit7=保留0)
} lb_schedule_uart_t;

//-----------------------------------------------------------------------------
// MCU UART 预约列表条目 (MCU通信协议.md 0x02 返回 — 44B payload)
//-----------------------------------------------------------------------------
typedef struct {
    u8  total_count;                    // 预约列表总数量
    u8  seq;                            // 当前序号(从1开始)
    u8  set_mode;                       // 设置模式: 0=关闭, 1=自定义加热, 2=鸡腿模式
    u8  id;                             // 预约ID
    char name[32];                      // 预约名称
    u32 time;                           // unix触发时间（秒, 大端）
    u8  temp;                           // 温度档位
    u8  duration;                       // 加热时长（分钟）
    u8  enabled;                        // 0=关闭, 1=开启
    u8  repeat;                         // 重复周期 (bit7=保留0)
} lb_schedule_entry_uart_t;

//-----------------------------------------------------------------------------
// 设备信息（由应用层填充）
//-----------------------------------------------------------------------------
typedef struct {
    char bt_name[16];                   // 蓝牙名称，不足补 0
    char version[8];                    // 软件版本 "x.x.x"
    char model[10];                     // 产品型号
    u8   mac[6];                        // MAC 地址
    char sn[32];                        // 序列号
    u8   color;                         // 颜色枚举: 0=白 1=黑 2=红 3=蓝 4=绿 5=金
    u32  main_mcu_version;              // 主MCU固件版本号 (v1.0.7 新增)
    u32  heat_module_version;           // 加热模块固件版本号 (v1.0.7 新增)
} lb_device_info_t;

//-----------------------------------------------------------------------------
// 属性值联合体
//-----------------------------------------------------------------------------
typedef struct {
    u8 dpid;                            // 属性 ID
    u8 type;                            // 数据类型 (LB_DP_TYPE_*)
    union {
        u8  b;                          // bool / enum
        u32 v;                          // 32-bit value
    };
} lb_attr_t;

//-----------------------------------------------------------------------------
// BLE 命令字 (蓝牙通讯协议1.0.5.md — APP ↔ MCU)
// 同步(SYNC): APP→MCU 一问一答
// 异步(ASYNC): MCU 主动推送
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
// OTA 目标设备标识 (蓝牙通讯协议1.0.7.md §5)
// APP 通过 target 字段区分升级目标设备
//-----------------------------------------------------------------------------
#define LB_OTA_TARGET_MAIN_MCU      0x01    // 主单片机（模组）
#define LB_OTA_TARGET_HEAT_MODULE   0x02    // 加热模块

// OTA 升级状态 (已删除升级查询 §5.1, v1.0.7)
#define LB_OTA_STATUS_UNSUPPORTED   0x00    // 不支持 MCU 升级
#define LB_OTA_STATUS_NOT_READY     0x01    // MCU 未就绪
#define LB_OTA_STATUS_SUPPORTED     0x02    // 支持升级

// OTA 升级启动状态 (蓝牙通讯协议1.0.7.md §5.1)
#define LB_OTA_START_RECEIVED       0x00    // 收到升级指令
#define LB_OTA_START_ERASING        0x01    // MCU 擦除 flash 中
#define LB_OTA_START_ERASE_DONE     0x02    // 擦除完成，可以传输升级包

// OTA 升级结果 (蓝牙通讯协议1.0.7.md §5.3)
#define LB_OTA_RESULT_FAIL          0x00    // 升级失败
#define LB_OTA_RESULT_SUCCESS       0x01    // 升级成功

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

/*
    执行结果：LB_ERR_SUCCESS    (成功)
              LB_ERR_EXEC_FAIL  (失败)
*/
enum {
    LB_ERR_SUCCESS = 0x00,
    LB_ERR_EXEC_FAIL = 0x01,
};

//-----------------------------------------------------------------------------
// 加热温度档位 (MCU通信协议 §4.1.4 ID=7): 0=40°C .. 6=100°C
//-----------------------------------------------------------------------------
#define LB_HEAT_TEMP_CNT    7

static const u16 tbl_heat_temp_f[LB_HEAT_TEMP_CNT] = {
    104, 122, 140, 158, 176, 194, 212,
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
 * @brief 解析后的接收帧（给业务回调用）
 *
 * 帧解析器校验通过后，把各字段拆好填入此结构体，
 * data 指针直接指向接收缓冲区内对应位置，非独立拷贝。
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

/**
 * @brief 命令处理回调函数类型
 *
 * @param[in] rx  解析好的接收帧指针
 * @return        错误码，LB_ERR_SUCCESS(0) 表示处理成功
 *
 * 注意：回调在 lunchbox_uart_process() 的调用上下文中执行，
 *       不应长时间阻塞，否则会影响帧接收。
 */
typedef u8 (*lb_cmd_handler_t)(lb_rx_frame_t *rx);

//-----------------------------------------------------------------------------
// API
//-----------------------------------------------------------------------------

/**
 * @brief 初始化串口协议模块
 *
 * 配置 UART1 引脚(PB8-TX, PB9-RX)、波特率，初始化内部缓冲区和回调表。
 * 应在系统初始化阶段调用一次。
 *
 * @param[in] baud  波特率，如 9600、115200
 */
void lunchbox_uart_init(u32 baud);

/** @brief 手动关机时关闭 UART1 硬件以降低功耗；唤醒后调用 lunchbox_uart_resume() */
void lunchbox_uart_suspend(void);

/** @brief 从手动关机唤醒后恢复 UART1 */
void lunchbox_uart_resume(void);

/**
 * @brief 主循环处理（需在 func_process 或主循环中轮询调用）
 *
 * 职责：从 UART1 缓冲区取字节 → 拼帧 → 超时检测 → 帧校验 → 分发给注册的回调。
 * 调用频率越高越好，建议每帧调用一次。
 */
void lunchbox_uart_process(void);

/** 手动关机期间阻止所有 UART TX；参数 true=阻塞 false=恢复 */
void lb_uart_tx_block(bool block);
bool lb_uart_tx_is_blocked(void);

/**
 * @brief 发送请求/命令帧（同步模式下主机侧使用，err_flag 固定为成功）
 *
 * 按协议格式组帧（大端），计算校验和，通过 UART1 发出。
 * 一般用于外部主机→MCU的方向；MCU 内部测试/转发也可调用。
 *
 * @param[in] cmd       命令字
 * @param[in] msg_flag  消息标志（同步模式下由调用者管理配对）
 * @param[in] data      待发送数据（可为 NULL）
 * @param[in] len       数据长度(字节)，0 表示无数据
 */
void lunchbox_uart_send(u8 cmd, u8 msg_flag, u8 *data, u16 len);

/**
 * @brief 发送应答帧（同步模式，MCU 回复主机请求）
 *
 * 与 lunchbox_uart_send 类似，但 err_flag 由调用者指定。
 * msg_flag 应与请求帧一致，用于请求-应答配对。
 *
 * @param[in] cmd       命令字（与请求帧一致）
 * @param[in] msg_flag  消息标志（与请求帧一致）
 * @param[in] err       错误码，见 LB_ERR_* 枚举
 * @param[in] data      待发送数据（可为 NULL）
 * @param[in] len       数据长度(字节)
 */
void lunchbox_uart_send_response(u8 cmd, u8 msg_flag, u8 err, u8 *data, u16 len);

/**
 * @brief 异步发送帧（MCU 主动上报，无需主机先请求）
 *
 * 用于 LB_CMD_STATUS_REPORT 等异步命令。
 * msg_flag 自动递增（0~255 循环），主机可据此检测是否丢帧。
 * err_flag 固定为 LB_ERR_SUCCESS。
 *
 * @param[in] cmd       命令字（异步类，如 LB_CMD_STATUS_REPORT）
 * @param[in] data      待发送数据（可为 NULL）
 * @param[in] len       数据长度(字节)，0 表示无数据
 */
void lunchbox_uart_send_async(u8 cmd, u8 *data, u16 len);

/**
 * @brief 注册命令处理回调
 *
 * 收到完整帧且校验通过后，根据 cmd 查表调用对应的 handler。
 * 一个 cmd 只能注册一个 handler，后注册的覆盖前面的。
 *
 * @param[in] cmd       命令字
 * @param[in] handler   处理回调函数指针
 */
void lunchbox_uart_reg_handler(u8 cmd, lb_cmd_handler_t handler);

//-----------------------------------------------------------------------------
// 业务 API（应用层使用）
//-----------------------------------------------------------------------------

/** @brief 注册所有协议命令的业务处理器，初始化后调用一次 */
void lunchbox_uart_init_handlers(void);

/** @brief 设置设备信息（用于 0x01 产品信息查询应答） */
void lunchbox_set_device_info(lb_device_info_t *info);

/** @brief 设置属性值并触发异步上报（用于 MCU 状态变化通知 APP） */
void lunchbox_set_attr_bool(u8 dpid, u8 val);
void lunchbox_set_attr_enum(u8 dpid, u8 val);
void lunchbox_set_attr_value(u8 dpid, u32 val);

/** @brief 主动上报所有属性（0x03 全量推送） */
void lunchbox_report_all_attrs(void);

/** @brief 主动上报单个属性变化（0x03 增量推送） */
void lunchbox_report_attr(u8 dpid);

//-----------------------------------------------------------------------------
// LCD 加热/预约控制接口 (桥模式和本地模式均可用)
// 按键: 加热键→切页, 确认键→lunchbox_heat_start, 开关键→lunchbox_heat_stop,
//       预约键→lunchbox_reservation_send, 模式键/加/减/锁键→仅UI本地
//-----------------------------------------------------------------------------

/** @brief LCD 启动加热 — 构造 UART 0x03 帧发给加热模块
 *  @param mode     加热模式: 1=自定义, 2=鸡腿, 3=意面, 4=预约, 5=保温
 *  @param temp     温度档位: 0=40°C ~ 6=100°C
 *  @param duration 加热时长(分钟) */
void lunchbox_heat_start(u8 mode, u8 temp, u32 duration);

/** @brief MCU 已控制加热时仅同步本地状态，不回发 UART */
void lunchbox_heat_start_local(u8 mode, u8 temp, u32 duration);

/** @brief LCD 停止加热 — 构造 UART 0x03 帧发给加热模块 (action=0 删除) */
void lunchbox_heat_stop(void);

/** @brief MCU 已下发停止时仅清本地状态，不回发 UART */
void lunchbox_heat_clear_local(void);

/** @brief LCD 关机 — 发送 PowerSwitch=OFF 给加热模块 (长按开关键3秒，含关屏供电) */
void lunchbox_power_off(void);

/** @brief 长按关机 UART 序列：先 HeatEnable=0 再 PowerSwitch=OFF（均等 ACK，不关屏） */
void lunchbox_mcu_shutdown_sequence(void);

/** @brief LCD 开机 — 发送 PowerSwitch=ON 给加热模块
 *  蓝牙已连接时附带时间戳(DPID=11)，未连接时仅发送开机字段 (长按开关键3秒) */
void lunchbox_power_on(void);

/** @brief LCD 时间同步 — 发送 UART 0x01 帧同步 Unix 时间戳到加热模块 */
void lunchbox_time_sync(u32 unix_time);

/** @brief LCD 查询预约列表 — 发送 UART 0x02 帧查询加热模块的预约列表 */
void lunchbox_query_reservation_list(void);

/** @brief 获取当前 Unix 时间戳
 *
 * 若已通过 APP 0x01 同步过权威时间，则用 synced_unix_ts + (RTCCNT - synced_rtccnt) 推算；
 * 否则回退到 RTCCNT + LB_RTC_UNIX_OFFSET (本地RTC)。
 */
u32 lb_get_unix_time(void);

/** @brief 获取屏幕显示时间 (优先级: APP > 加热模块 > 本地RTC)
 *
 * 开机时:
 *   - 若 APP 已蓝牙同步过时间 → 使用 APP 权威时间
 *   - 若 APP 未同步但加热模块已上报时间 → 使用加热模块时间
 *   - 若两者均未同步 → 使用本地 RTC 默认时间
 *
 * @return tm_t 结构体 (北京时间), 可直接替代 rtc_clock_get() 用于 UI 显示
 */
tm_t lb_get_display_tm(void);

/** @brief 扫描 DataPoint 缓冲区，查找指定 dpid 的 bool/enum 首字节 */
bool lb_dp_scan_bool(const u8 *data, u16 len, u8 dpid, u8 *val);

/** @brief 进入保温页时下发保温指令 (模式5, 默认 194°F) */
void lunchbox_keep_warm_apply(void);

/** @brief 加热自然结束后自动开启保温 (模式5, 194°F, 至低电关机) */
void lunchbox_keep_warm_start(void);

/** @brief 停止保温 */
void lunchbox_keep_warm_stop(void);

/** @brief 用户按开关键停止保温：强制下发 UART stop */
void lunchbox_keep_warm_stop_user(void);

/** @brief 当前是否处于保温状态 */
bool lunchbox_keep_warm_is_active(void);

/** @brief 主循环轮询保温 (低电关机时停止) */
void lunchbox_keep_warm_poll(void);

#if ELUNCHBOX_PANEL_EN
/** @brief 仅更新本地保温/加热任务标志（充电 RX-only 时不发 UART） */
void lunchbox_warm_mark_active(void);
#endif

/** @brief LCD 按键通知 — 构造 UART 0x01 DataPoint(dpid=12) 帧发往加热模块
 *  @param key_val  按键值: 0-9, 每个按键对应不同的值 */
void lunchbox_key_notify(u8 key_val);

/** @brief LCD 发送预约 — 构造 UART 0x03 帧发给加热模块
 *  @param action    0=删除, 1=自定义加热, 2=鸡腿模式
 *  @param id        预约ID (新增时填0, 加热模块自动分配)
 *  @param name      预约名称(最长32字节, 可为NULL)
 *  @param unix_time 触发时间(unix时间戳, 大端)
 *  @param temp      温度档位
 *  @param duration  加热时长(分钟)
 *  @param enabled   0=关闭, 1=开启
 *  @param repeat    重复周期位掩码 (0x7e=工作日, 0xff=每天) */
void lunchbox_reservation_send(u8 action, u8 id, const char *name, u32 unix_time,
                               u8 temp, u8 duration, u8 enabled, u8 repeat);

/** @brief LCD 删除预约 */
void lunchbox_reservation_delete(u8 id);

#if !LB_BRIDGE_MODE
u8 lb_schedule_alloc_id(void);          // 分配下一个可用预约ID (>=6)
#else
static inline u8 lb_schedule_alloc_id(void) { return 6; }  // 桥模式: APP分配ID
#endif

/** @brief 获取指定模式的预设温度档位 */
u8 lunchbox_mode_get_temp(u8 mode);

/** @brief 获取指定模式的预设加热时长(分钟) */
u8 lunchbox_mode_get_duration(u8 mode);

/** @brief 华氏度转温度档位 (0=40°C ~ 6=100°C, 取最近档位) */
u8 lunchbox_temp_f_to_idx(u16 temp_f);

/** @brief 获取当前加热模式 (无本地缓存时返回0) */
u8 lunchbox_get_heat_mode(void);

/** @brief 获取当前加热使能状态 (无本地缓存时返回0) */
u8 lunchbox_get_heat_enable(void);

/** @brief 是否有进行中的加热/保温任务（用于禁止自动息屏） */
bool lunchbox_heating_task_active(void);

/** @brief BLE 连接成功回调 — 主动上报时间戳(0x03, dpid=11)给 APP
 *
 * 触发时机: ble_app_watch_connect_callback() 中调用。
 * MCU 向 APP 发送一条 0x03 状态上报帧，仅含时间戳 DataPoint(dpid=11)。
 * 时间戳来源: lb_get_unix_time() (已同步则用APP权威时间推算, 否则用本地RTC)。
 *
 * APP 可通过两条路径回传权威时间戳:
 *   - 0x03 回传 dpid=11 → lb_ble_handle_app_time_sync() 处理 (桥模式/本地模式均支持)
 *   - 0x01 产品信息查询 (数据区带 4B 时间戳) → lb_handler_product_info() 处理 (原有路径)
 *
 * MCU 收到 APP 时间戳后 → 下发 5 个固定预设 (ID 1~5) 到加热模块。
 */
void lunchbox_ble_on_connected(void);

/** @brief BLE 连接后发送5个固定预约预设到加热模块 (UART 0x03)
 *
 * 触发时机: lunchbox_ble_on_connected() 中调用。
 * MCU 向加热模块发送5条不可修改的固定预约(ID=1~5)，
 * 命令字 0x03 (LB_UART_CMD_SCHEDULE_OP), 帧格式见 MCU通信协议.md §3.6。
 *
 * 预设列表:
 *   ID=1: 早餐(8:00),   ID=2: 午餐(10:50), ID=3: 晚餐(16:30),
 *   ID=4: 鸡腿模式,      ID=5: 意面模式
 * 温度统一 149°F(60°C), enabled=0(停止加热), repeat=每天。
 */
void lunchbox_ble_send_presets(void);

//-----------------------------------------------------------------------------
// OTA 升级流程 (蓝牙通讯协议1.0.7.md §5)
//-----------------------------------------------------------------------------

/**
 * @brief OTA 升级流程处理 (需在主循环中轮询调用)
 *
 * 升级成功 (ota_pack_done) 后延时 3 秒复位 MCU，
 * 确保 BLE 应答帧已成功发送给 APP 后才复位。
 */
void lb_ota_process(void);

//-----------------------------------------------------------------------------
// 模式界面 → 加热界面 预设参数传递
// 鸡腿/意面模式按确认键后跳转到加热界面并自动开始加热
//-----------------------------------------------------------------------------

typedef struct {
    bool active;            // 是否有待消费的预设
    u8   proto_mode;        // 协议加热模式 (2=鸡腿, 3=意面)
    u16  temp_f;            // 预设温度 (°F)
    u8   hour;              // 预设小时
    u8   min;               // 预设分钟
} lb_mode_to_heat_preset_t;

/** @brief 设置模式→加热预设参数 (由 func_mode 调用) */
void lb_mode_to_heat_set(u8 proto_mode, u16 temp_f, u8 hour, u8 min);

/** @brief 获取并消费模式→加热预设参数 (由 func_heat 调用, 一次性) */
bool lb_mode_to_heat_get(lb_mode_to_heat_preset_t *out);

/** @brief 新加热页设置完成后自动开始加热 (由 func_new_heat 设置, func_heat 消费) */
void lb_heat_autostart_set(bool en);
bool lb_heat_autostart_consume(void);
void lb_heat_user_uart_tx_force_set(bool en);
/** @brief BLE 桥模式已转发 UART 时，func_heat 跳过重复 lunchbox_heat_start */
void lb_heat_uart_remote_set(bool en);
bool lb_heat_uart_remote_consume(void);
bool lb_heat_uart_remote_peek(void);
/** @brief 充电转保温后 MCU 驱动跳页期间禁止回发加热 UART */
void lb_heat_mcu_nav_set(bool on);
bool lb_heat_mcu_nav_active(void);

//-----------------------------------------------------------------------------
// 协议翻译层 (BLE ↔ UART)
//-----------------------------------------------------------------------------

/**
 * @brief BLE 命令字 → UART 命令字映射
 * @param ble_cmd  BLE 命令字
 * @return UART 命令字, 0x00 表示不转发
 */
u8 lb_ble_cmd_to_uart_cmd(u8 ble_cmd);

/**
 * @brief UART 命令字 → BLE 命令字映射
 * @param uart_cmd  UART 命令字
 * @param is_async  true=异步状态上报
 * @return BLE 命令字
 */
u8 lb_uart_cmd_to_ble_cmd(u8 uart_cmd, bool is_async);

/**
 * @brief BLE帧数据 → UART帧数据翻译
 * @param rx        BLE 接收帧(已解析)
 * @param out_buf   输出缓冲区
 * @param out_len   输出数据长度
 * @return true=翻译成功, false=不转发
 */
bool lb_translate_ble_to_uart(lb_rx_frame_t *rx, u8 *out_buf, u16 *out_len);

/**
 * @brief UART帧数据 → BLE帧数据翻译
 * @param rx        UART 接收帧(已解析)
 * @param out_buf   输出缓冲区
 * @param out_len   输出数据长度
 * @return true=翻译成功, false=不转发
 */
bool lb_translate_uart_to_ble(lb_rx_frame_t *rx, u8 *out_buf, u16 *out_len);

//-----------------------------------------------------------------------------
// 自测
//-----------------------------------------------------------------------------

/**
 * @brief 自测函数：上电发测试字符串 + 注册 echo 处理器
 *
 * 在初始化后调用一次即可验证串口收发链路。
 * 同时在 UART1 发送 LUNCHBOX_UART_OK\r\n 和注册命令 echo。
 */
void func_lunchbox_uart_test(void);

//-----------------------------------------------------------------------------
// BLE 通道接口
//-----------------------------------------------------------------------------

/** @brief BLE 发送回调函数类型 */
typedef void (*lb_ble_tx_fn_t)(u8 *data, u16 len);

/** @brief 注册 BLE 发送函数 */
void lunchbox_ble_set_tx_fn(lb_ble_tx_fn_t fn);

/** @brief 处理 BLE 接收到的饭盒协议帧，自动校验+分发给命令处理器/翻译转发 */
void lunchbox_ble_rx_handle(u8 *data, u16 len);
bool lunchbox_ble_rx_pending(void);     // 累积缓冲区是否有待处理数据

// 子系统 API (拆分后的独立模块)
#include "func_lunchbox_lcd.h"
#include "func_lunchbox_ble.h"
#include "func_lunchbox_bridge.h"
#include "func_lunchbox_ota.h"
#include "func_lunchbox_uart_heat.h"

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __FUNC_LUNCHBOX_UART_H
