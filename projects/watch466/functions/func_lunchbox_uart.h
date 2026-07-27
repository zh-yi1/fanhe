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

// 协议常量/枚举/帧结构/收发 API → 协议层头 (重构后唯一定义处)
#include "func_lunchbox_proto.h"

//-----------------------------------------------------------------------------
// 应用配置
//-----------------------------------------------------------------------------
#define LB_BAUD                 115200      // 波特率（按需修改）

// RTCCNT 从 2020-01-01 起算(秒), 且 RTCCNT 存的是北京时间(UTC+8)
// tm_to_time 的基准也是 2020-01-01 00:00:00 (北京时间)
// 北京时间 2020-01-01 00:00:00 = UTC 2019-12-31 16:00:00
// 其 Unix 时间戳 = (50年×365天 + 12闰日) × 86400秒 - 8小时 = 1,577,808,000秒
#define LB_RTC_UNIX_OFFSET      (1577836800u - 8*3600)   // = 1577808000

#define LB_HEAT_DURATION_MIN_MIN          60      // 加热时长下限(分钟)，协议范围 30-210 (v1.0.7 §4)
#define LB_HEAT_DURATION_MAX_MIN          120     // 加热时长上限(分钟)，协议范围 30-210 (v1.0.7 §4)

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
// 加热温度档位 (MCU通信协议 §4.1.4 ID=7): 0=40°C .. 6=100°C
//-----------------------------------------------------------------------------
#define LB_HEAT_TEMP_CNT    7

static const u16 tbl_heat_temp_f[LB_HEAT_TEMP_CNT] = {
    104, 122, 140, 158, 176, 194, 212,
};

//-----------------------------------------------------------------------------
// 应用层核心 API (func_lunchbox_uart_app.c)
//-----------------------------------------------------------------------------

/**
 * @brief 初始化串口模块 (解析器 + UART1 硬件), 系统初始化阶段调用一次
 * @param[in] baud  波特率, 如 115200
 */
void lunchbox_uart_init(u32 baud);

void lunchbox_uart_suspend(void);
void lunchbox_uart_resume(void);

/**
 * @brief 主循环处理（func_process 每轮调用）
 *
 * 底层取字节 → 协议层拼帧/超时 → 逐帧调用帧处理入口
 */
void lunchbox_uart_process(void);

/**
 * @brief 组帧并从串口发出 (应用层唯一发送原语)
 * @return true=已发出, false=被阻断或组帧失败
 */
bool lunchbox_uart_send_frame(u8 cmd, u8 msg_flag, u8 err,
                              const u8 *data, u16 len);

void lb_uart_tx_block(bool block);
bool lb_uart_tx_is_blocked(void);

/**
 * @brief BLE→UART 异步转发 (入队即返回)
 *
 * 一发一收: 队首在飞, 其余排队。应答超时重发 (共 3 次),
 * 应答到达或重试耗尽后自动翻译回传 APP (lunchbox_ble_tx)。
 * @param ble_cmd       来源 BLE 命令字
 * @param ble_msg_flag  来源 BLE msg_flag (串口转发沿用, 应答按它配对)
 * @param uart_cmd      转发的 UART 命令字
 * @return false=数据过长或队列满 (请求被丢弃)
 */
bool lb_bridge_forward(u8 ble_cmd, u8 ble_msg_flag, u8 uart_cmd,
                       const u8 *data, u16 len);

//=============================================================================
// ↓↓↓ 待重建业务接口 — 仅保留声明供旧调用方编译, 实现随功能重构逐步恢复 ↓↓↓
//=============================================================================

// ── 发送队列 (stop-and-wait, 待重建) ──
void lb_uart_send_raw(u8 uart_cmd, u8 *data, u16 data_len, bool no_wait);
void lb_uart_send_raw_noreport(u8 uart_cmd, u8 *data, u16 data_len, bool no_wait);

// ── BLE 应答/上报 (待重建) ──
void lunchbox_uart_send_response(u8 cmd, u8 msg_flag, u8 err, u8 *data, u16 len);
void lunchbox_uart_send_async(u8 cmd, u8 *data, u16 len);

// ── BLE 命令分发 (待重建, 现方案为 switch-case 无 handler 表) ──
bool lunchbox_uart_call_handler(lb_rx_frame_t *rx);

//-----------------------------------------------------------------------------
// 业务 API（待重建）
//-----------------------------------------------------------------------------

/** @brief 设置设备信息（用于 0x01 产品信息查询应答） */
void lunchbox_set_device_info(lb_device_info_t *info);

//-----------------------------------------------------------------------------
// LCD 加热/预约控制接口
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

/** @brief LCD 关机 — 发送 PowerSwitch=OFF 给加热模块 (长按开关键3秒) */
void lunchbox_power_off(void);

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

static inline u8 lb_schedule_alloc_id(void) { return 6; }  // 预约 ID 由 APP 分配, 本地固定返回起始值

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

/* 数据级翻译 lb_translate_ble_data_to_uart / lb_translate_uart_data_to_ble
 * 声明见 func_lunchbox_bridge.h (下方 include) */

/* BLE 通道注册见 func_lunchbox_ble_app.h (下方 include) */

// 子系统 API (拆分后的独立模块)
#include "func_lunchbox_lcd.h"
#include "func_lunchbox_ble_app.h"
#include "func_lunchbox_bridge.h"
#include "func_lunchbox_ota.h"
#include "func_lunchbox_uart_heat.h"

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __FUNC_LUNCHBOX_UART_H
