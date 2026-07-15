#ifndef _HEAT_DISPLAY_REG_H
#define _HEAT_DISPLAY_REG_H

#include "include.h"

/*
 * 加热/预约信息显示注册层（不做串口解析）
 *
 * 加热页 / 模式页 / 预约页 enter 时 register，exit 时 unregister。
 * 串口或其它模块解析完成后调用 show 接口推送数据，已注册 UI 回调即刷新。
 *
 * 示例:
 *   heat_display_register(my_cb);
 *   heat_display_show_schedule(120);   // 预约剩余 120 分钟
 *   heat_display_show(45, 176);        // 加热剩余 45 分钟，176°F
 *   heat_display_show_all(120, 45, 176);
 *   heat_display_unregister();
 */

typedef struct {
    u32 schedule_min;   /* 预约剩余时长（分钟） */
    u32 remain_min;     /* 加热剩余时长（分钟） */
    u16 temp_f;         /* 实时温度（华氏度） */
} heat_display_info_t;

typedef void (*heat_display_cb_t)(const heat_display_info_t *info);

/** 注册显示回调（同时只保留一个） */
void heat_display_register(heat_display_cb_t cb);

/** 注销显示回调 */
void heat_display_unregister(void);

/** 推送预约剩余时长 */
void heat_display_show_schedule(u32 schedule_min);

/** 推送加热剩余时长与实时温度 */
void heat_display_show(u32 remain_min, u16 temp_f);

/** 推送预约剩余、加热剩余、实时温度（三字段同时更新） */
void heat_display_show_all(u32 schedule_min, u32 remain_min, u16 temp_f);

/** 读取最近一次推送的数据 */
bool heat_display_get_last(heat_display_info_t *out);

/** 最近一次推送显示加热剩余时间 > 0（UART 已确认在加热） */
bool heat_display_heating_active(void);

/**
 * @brief 从 DataPoint 数组提取剩余时间/温度/加热使能，推送给已注册的显示回调
 *
 * 解析协议 DataPoint 格式（dpid:1B + type:1B + len:2B-BE + val:lenB），
 * 提取 LB_DPID_REMAIN_TIME / LB_DPID_HEAT_TEMP / LB_DPID_HEAT_ENABLE，
 * 调用 heat_display_show() 通知 LCD 刷新。
 *
 * 适用场景：
 *   - UART 收到 0x01 动态属性上报
 *   - BLE 收到 0x02 动态属性查询应答 / 0x03 状态上报 / 0x04 控制指令
 *
 * @param data     DataPoint 数组首字节指针
 * @param len      数组总长度(字节)
 * @param msg_flag UART/BLE 帧的消息标志位 (保温状态用于过滤过时数据)
 */
void heat_display_feed_dp(u8 *data, u16 len, u8 msg_flag);

/** 查询是否收到充电中状态(charge_status==1)，调用后自动清除 */
bool heat_display_charge_wake_pending(void);

/** 查询是否收到加热使能(heat_enable=1)，调用后自动清除 */
bool heat_display_heat_wake_pending(void);

#if ELUNCHBOX_PANEL_EN
/** 熄屏时充电+保温 pending，调用后自动清除 */
bool heat_display_warm_charge_wake_pending(void);
/** 是否仍有 deferred 充电进保温（不清除） */
bool heat_display_warm_charge_pending_active(void);
/** 亮屏后执行 deferred 充电进保温（预约进加热页后与直接加热相同） */
void heat_display_warm_charge_route_poll(void);
/** 预约到点是否可跳加热页：已在保温/加热页则否 */
bool heat_display_reservation_can_switch_heat(void);
#endif

#if ELUNCHBOX_PANEL_EN
/** 进入保温页时调用，重置拔电退出去重状态 */
void heat_display_warm_exit_reset(void);

/** 获取当前 MCU 加热模式 (优先缓存, 桥模式下也不返回 0) */
u8 heat_display_get_mcu_mode(void);
#endif

#endif
