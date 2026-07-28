#ifndef _GENERAL_UI_H
#define _GENERAL_UI_H

/* 加热页共享参数（定义在 func_heat_set_page.c） */
typedef struct {
    u16 temp;
    u16 time_min;
    u16 keep_warm_min;
} ui_heat_t;
extern ui_heat_t g_ui_heat;

/* 系统级 UI 状态（定义在 func_home_page.c） */
typedef struct {
    u8 reserved;
} ui_sys_t;
extern ui_sys_t g_ui_sys;

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
#define NEW_HOME_BT_W                   32
#define NEW_HOME_BT_H                   32
#define NEW_HOME_BAT_W                  32
#define NEW_HOME_BAT_H                  32

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
    u8 last_min;
} general_status_bar_t;

/**
 * 在 form 上创建状态栏所有控件。
 * @param frm        所属 form
 * @param bar        状态栏实例（调用方分配）
 * @param page_name  页面名称字符串（i18n 或字面量）
 */
void general_status_bar_create(compo_form_t *frm, general_status_bar_t *bar, const char *page_name);

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
 * 每帧 tick：更新时间文本。分钟变化时才更新 textbox。
 */
void general_status_bar_tick(general_status_bar_t *bar);

#endif
