#ifndef _HEAT_DISPLAY_REG_H
#define _HEAT_DISPLAY_REG_H

#include "include.h"

/*
 * 加热信息显示注册层（不做串口解析）
 *
 * 加热页 enter 时 register，exit 时 unregister。
 * 串口或其它模块拿到剩余时长、实时温度后，调用 heat_display_show() 即可刷新 UI。
 *
 * 示例:
 *   heat_display_register(my_cb);
 *   heat_display_show(45, 176);   // 剩余 45 分钟，176°F
 *   heat_display_unregister();
 */

typedef struct {
    u32 remain_min;     /* 剩余加热时间（分钟） */
    u16 temp_f;         /* 实时温度（华氏度） */
} heat_display_info_t;

typedef void (*heat_display_cb_t)(const heat_display_info_t *info);

/** 注册显示回调（同时只保留一个） */
void heat_display_register(heat_display_cb_t cb);

/** 注销显示回调 */
void heat_display_unregister(void);

/** 推送加热显示数据，通知已注册回调 */
void heat_display_show(u32 remain_min, u16 temp_f);

/** 读取最近一次推送的数据 */
bool heat_display_get_last(heat_display_info_t *out);

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
 * @param data  DataPoint 数组首字节指针
 * @param len   数组总长度(字节)
 */
void heat_display_feed_dp(u8 *data, u16 len);

#endif
