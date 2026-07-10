#ifndef _HOME_UI_SHARED_H
#define _HOME_UI_SHARED_H

#include "home_icon_res.h"
#include "new_home_tab_res.h"
#include "new_home_icon_res.h"

/* Home / Heat / Mode 互斥；新主页底部 Tab 为 79×88，占 3 槽静态 RAM */
#define HOME_UI_SHARED_TAB_CNT          3

extern u8 home_ui_shared_icon_runtime[HOME_UI_SHARED_TAB_CNT][NEW_HOME_TAB_RAM_SIZE];
extern u8 home_ui_shared_status_bt_ram[HOME_STATUS_BT_RAM_SIZE];
extern u8 home_ui_shared_status_lock_ram[HOME_STATUS_LOCK_RAM_SIZE];
extern u8 home_ui_shared_status_bat_ram[HOME_STATUS_BAT_RAM_SIZE];
extern u8 home_ui_shared_dash_runtime_sel[HOME_DASH_RAM_SIZE];
extern u8 home_ui_shared_dash_runtime_nor[HOME_DASH_RAM_SIZE];
extern u8 home_ui_shared_top_time_digit_ram[HOME_TOP_TIME_DIGIT_SLOTS][HOME_TOP_TIME_DIGIT_RAM_MAX_SIZE];
extern u8 home_ui_shared_top_time_colon_ram[HOME_TOP_TIME_COLONM_RAM_SIZE];
extern u8 home_ui_shared_top_time_ampm_ram[HOME_TOP_TIME_AMPM_RAM_MAX_SIZE];
extern bool home_ui_shared_status_inited;
extern bool home_ui_shared_status_lock_preloaded;
extern bool home_ui_shared_dash_inited;

void home_ui_shared_status_lock_preload(void);
void home_ui_shared_status_init(void);
void home_ui_shared_dash_init(void);

/** BLE 是否已连接（GATT 链路） */
bool home_ui_shared_ble_linked(void);
/** 刷新右上角蓝牙图标显隐（连接/断开后须调用） */
void home_ui_shared_status_refresh_bt(compo_picturebox_t *pic);
/** BLE 连接/断开时由协议栈回调；主循环 poll 刷新右上角蓝牙图标 */
void home_ui_shared_ble_status_poll(void);
void home_ui_shared_ble_link_notify(void);
void home_ui_shared_bt_detach_pic(void);

/** 上电预载 dl4 到 bat_ram（不依赖 UART DP） */
void home_ui_shared_battery_boot_init(void);

/** 解析 DP3/DP4 并刷新电量图标 RAM；当前页已通过 attach 绑定时自动重绘 */
void home_ui_shared_battery_feed_dp(u8 *data, u16 len);

/** 页面 enter 时绑定 pic_bat，exit 时 detach */
void home_ui_shared_battery_attach_pic(compo_picturebox_t *pic);
void home_ui_shared_battery_detach_pic(void);

/** 将共享 bat_ram 绑定到右上角电量 picturebox（按当前电量/充电状态选 dl1~dl4/dl） */
void home_ui_shared_status_bind_bat(compo_picturebox_t *pic);

/** 当前电量图标 Flash 地址（供 reservation 等 flash 直绘路径） */
u32 home_ui_shared_battery_flash_addr(void);

/** 电量 DP：1=低(<25%) 2=中 3=高 4=满 */
u8 home_ui_shared_battery_level(void);
bool home_ui_shared_battery_is_low(void);
bool home_ui_shared_battery_is_charging(void);
/** 提前应用充电 DP（feed_dp 判保温前同步，避免同包 charge+enable 顺序误判） */
void home_ui_shared_battery_charge_apply(u8 charge_sta);
/** 按当前电量/充电状态重载右上角电池图标 RAM */
void home_ui_shared_battery_icon_refresh(void);

/* Heat / Mode 中部倒计时共享 RAM（互斥使用，不增加 BSS 总量） */
extern u8 home_ui_shared_timer_colon_ram[HEAT_WBX_RAM_SIZE];
extern u8 home_ui_shared_timer_digit_ram[4][HEAT_B_DIGIT_RAM_MAX_SIZE];

/* 修改 set_ram / Flash→RAM 贴图前调用，避免 GPU 读缓冲时被改写触发 C241 */
void home_gpu_wait_idle(void);

#endif
