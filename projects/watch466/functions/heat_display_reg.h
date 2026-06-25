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

#endif
