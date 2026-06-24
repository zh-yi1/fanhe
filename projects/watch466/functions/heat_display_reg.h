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

#endif
