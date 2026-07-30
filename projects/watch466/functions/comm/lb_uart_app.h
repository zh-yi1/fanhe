/**
 * @file    lb_uart_app.h
 * @brief   饭盒串口应用层 — 与加热模块通信 (MCU通信协议.md v1.0.7)
 *
 * 引脚: TX=PB8=UT1TXMAP_G2_PB8
 *       RX=PB9=UT1RXMAP_G2_PB9
 * 帧格式与 BLE 相同 (0x55AA, 见 lb_proto.h), 命令字为 LB_UART_CMD_*。
 * 通信机制:
 *   同步: 一问一答, 应答的 cmd/msg_flag 与请求帧一致
 *   异步: 加热模块主动上报(温控/故障), msg_flag 自增
 *
 * 三层结构:
 *   底层收发  lb_uart_link   UART1 硬件 + RX 环形缓冲
 *   协议层    lb_proto       0x55AA 帧编解码 (纯函数)
 *   应用层    本文件         数据泵调度 + 帧处理入口 + BLE转发队列
 */
#ifndef __LB_UART_APP_H
#define __LB_UART_APP_H

#include "include.h"
#include "lb_proto.h"   // lb_rx_frame_t / LB_UART_CMD_* / LB_FRAME_TIMEOUT_MS

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// 应用配置
//-----------------------------------------------------------------------------
#define LB_BAUD                 115200      // 波特率（按需修改）

//-----------------------------------------------------------------------------
// 设备信息（0x01 产品信息查询的应答内容, 定义在 lb_uart_app.c）
//-----------------------------------------------------------------------------
typedef struct {
    char bt_name[16];                   // 蓝牙名称，不足补 0
    char version[8];                    // 软件版本 "x.x.x"
    char model[10];                     // 产品型号
    u8   mac[6];                        // MAC 地址
    char sn[32];                        // 序列号
    u8   color;                         // 颜色枚举: 0=白 1=黑 2=红 3=蓝 4=绿 5=金
    u32  main_mcu_version;              // 主MCU固件版本号 (v1.0.7 新增)
    u32  heat_module_version;           // 加热模块固件版本号 (开机为0, 模块应答 dpid=13 回填)
} lb_device_info_t;

extern lb_device_info_t lb_dev_info;

//-----------------------------------------------------------------------------
// 生命周期
//-----------------------------------------------------------------------------

/**
 * @brief 初始化串口模块 (解析器 + UART1 硬件), 系统初始化阶段调用一次
 * @param[in] baud  波特率, 如 LB_BAUD
 */
void lunchbox_uart_init(u32 baud);

/** @brief 关闭 UART1 (低功耗/手动关机用); 解析器复位, 释放 PB8/PB9 回 GPIO */
void lunchbox_uart_suspend(void);

/** @brief 从 suspend 恢复 (重建 UART1, 沿用 LB_BAUD) */
void lunchbox_uart_resume(void);

/**
 * @brief 主循环处理（func_process 每轮调用）
 *
 * 边读边解: 底层取字节 → 协议层拼帧/残帧超时 → 逐帧调 lb_uart_on_frame()
 */
void lunchbox_uart_process(void);

//-----------------------------------------------------------------------------
// 发送接口 (MCU → 加热模块)
//-----------------------------------------------------------------------------

/**
 * @brief 组帧并从串口发出 (应用层唯一发送原语)
 * @return true=已发出, false=被阻断或组帧失败
 */
bool lunchbox_uart_send_frame(u8 cmd, u8 msg_flag, u8 err,
                              const u8 *data, u16 len);

/** @brief 阻止/恢复所有 UART TX (手动关机期间用); true=阻塞 false=恢复 */
void lb_uart_tx_block(bool block);
bool lb_uart_tx_is_blocked(void);

//-----------------------------------------------------------------------------
// BLE→UART 异步转发 (队列实现在 lb_uart_app.c, 翻译在 lb_bridge.c)
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------
// 开机 / 关机时序 (一发一等: 发一条 → 等模块应答 → 发下一条; 超时也推进)
//
// 开机: power_on(0x01 DP1=1) → 应答 → 查预约列表(0x02) → 结束
//       上电后由 lunchbox_uart_process() 自动起, 无需外部调用。
//
// 关机: stop(0x01 DP10=0) → 应答 → power_off(0x01 DP1=0) → 应答 → 结束
//       来源两种: 本机长按关机 / APP 下发关机。APP 来的每步应答都回 APP。
//-----------------------------------------------------------------------------

/**
 * @brief 当前是否禁止关机 —— 充电中 或 OTA 进行中
 *
 * 充电中禁止是硬要求: func_pwroff() 在 CHARGE_DC_IN() 时会 return 不断电,
 * 若还让关机时序跑完并置 FUNC_PWROFF, 会陷入"反复进关机页又回来"的死循环。
 */
bool lunchbox_shutdown_blocked(void);

/**
 * @brief 启动关机时序 (重复调用无副作用)
 *
 * 被 lunchbox_shutdown_blocked() 拦下时不启动; from_app 会回一条执行失败应答。
 * @param from_app  true=APP 下发的关机, 每步模块应答后回一条 BLE 应答
 * @param app_flag  APP 请求的 msg_flag (from_app=false 时忽略)
 */
void lunchbox_shutdown_start(bool from_app, u8 app_flag);

/** @brief 放弃关机, 状态机回 IDLE (时序跑完后才发现不能关机时用) */
void lunchbox_shutdown_abort(void);

/** @brief 关机时序进行中 */
bool lunchbox_shutdown_is_active(void);

/** @brief 关机时序已走完 (含超时收场), 可以真正断电 */
bool lunchbox_shutdown_is_done(void);

#endif // FUNC_LUNCHBOX_UART_EN
#endif // __LB_UART_APP_H
