#ifndef _GENERAL_UI_H
#define _GENERAL_UI_H

#include "ui.h"
#include "app_ui.h"     /* ui_sys_t / g_ui_sys */

/* 通用顶部状态栏：时间、页面名、蓝牙、电量（含充电动画）
 * 使用方式：
 *   1. 在 xxx_form_create() 中调用 general_status_bar_create()
 *   2. 在 xxx_enter() 中调用 general_status_bar_attach()
 *   3. 在 xxx_process() 中调用 general_status_bar_tick()
 *   4. 在 xxx_exit() 中调用 general_status_bar_detach()
 *
 * func_process() 内部已调用 home_ui_shared_ble_status_poll() 和
 * home_ui_shared_battery_chg_poll()，页面无需重复调用。
 */

#define GENERAL_SB_Y                    20
#define GENERAL_SB_LEFT_MARGIN          12
#define GENERAL_SB_RIGHT_MARGIN         10
#define GENERAL_SB_ICON_GAP             6
#define GENERAL_SB_TIME_FONT_H          16
#define GENERAL_SB_NAME_FONT_H          16

/* home bin 图标尺寸（原 home_ui_shared.h） */
#define NEW_HOME_BT_W                   10
#define NEW_HOME_BT_H                   16
#define NEW_HOME_BAT_W                  31
#define NEW_HOME_BAT_H                  18

/* composet ID，跨页面复用（不同 form 互不干扰） */
enum {
    GENERAL_SB_ID_TXT_TIME = 0x200,
    GENERAL_SB_ID_TXT_PAGENAME,
    GENERAL_SB_ID_PIC_BT,
    GENERAL_SB_ID_PIC_BAT,
};

typedef struct
{
    compo_textbox_t *txt_time;
    compo_textbox_t *txt_pagename;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_bat;
    ui_sys_t *sys_data;     /* 串口解析后的系统状态，由 general_status_bar_create 传入 */
    u8 last_min;
    u8 last_hour;           /* 串口时间追踪 */
    u8 last_bat_level;      /* 电量档位追踪 0~4，0xff 强制首次刷新 */
    bool last_charging;     /* 充电状态追踪 */
    bool last_full_charge;  /* 充满状态追踪 */
    bool last_bt_linked;    /* 蓝牙连接状态追踪 */
    u32 charge_tick_ms;     /* 充电动画上次切帧时间戳 (tick_get)，500ms 切一帧 */
    u8  charge_frame;       /* 当前充电动画帧索引 0~3 */
} general_status_bar_t;

/**
 * 在 form 上创建状态栏所有控件。
 * @param frm        所属 form
 * @param bar        状态栏实例（调用方分配）
 * @param page_name  页面名称字符串（i18n 或字面量）
 * @param sys_data   串口解析后的系统状态指针，tick 时从此读取数据
 */
void general_status_bar_create(compo_form_t *frm, general_status_bar_t *bar,
                               const char *page_name, ui_sys_t *sys_data);

/**
 * 绑定控件指针（form 重建后调用，如 resume/wakeup 场景）。
 * 内部调用 home_ui_shared_status_init() 和 home_ui_shared_status_refresh_bt()。
 */
void general_status_bar_bind(general_status_bar_t *bar);

/**
 * 页面 enter 时调用：绑定电池 pic 到共享模块（仅 ELUNCHBOX_PANEL_EN 有效）。
 */
void general_status_bar_attach(general_status_bar_t *bar);

/**
 * 页面 exit 时调用：解绑电池 pic（仅 ELUNCHBOX_PANEL_EN 有效）。
 */
void general_status_bar_detach(void);

/**
 * 每帧 tick：从 g_ui_sys（串口数据）实时更新时间、电量图标、充电动画、蓝牙图标。
 * 仅在数据变化时更新 UI，避免无谓的 GPU 操作。
 */
void general_status_bar_tick(general_status_bar_t *bar);

/*===========================================================================
 * 以下是通信移植层给新 UI 的接口。
 * 放这里而不放 app_ui.h —— 后者由 UI 端整份重发覆盖, 加进去会被冲掉。
 *=========================================================================*/

/**
 * @brief 串口状态镜像 → g_ui_sys (func_process 每轮调用, 页面不用管)
 *
 * 单向: lb_ui_state_get() 是唯一数据源, 页面只读 g_ui_sys。
 * 实现见 general_ui.c 末尾。
 */
void lb_ui_sync_pull(void);

/*---------------------------------------------------------------------------
 * 预约暂存 —— 预约时间页选好触发时刻后交给加热设置页, 由后者确认时一并下发
 *
 * 流程: 预约键 → 预约时间页(选时分秒) → set() → 加热设置页(选温度/时长)
 *       → 确认时 take() 命中 → 下发 0x03 新增预约, 回首页
 *       → 未命中(直接进的加热设置页) → 下发 0x01 立即加热, 进加热页
 *
 * 实现在 func_appointment_time.c。
 *-------------------------------------------------------------------------*/

/** @brief 暂存预约触发时刻 (unix 秒) */
void new_ui_appointment_set(u32 unix_time);

/** @brief 取出暂存的触发时刻; 无待下发预约返回 false (取出即清) */
bool new_ui_appointment_take(u32 *out_unix_time);

/** @brief 丢弃暂存 (用户中途退出) */
void new_ui_appointment_clear(void);

#endif
