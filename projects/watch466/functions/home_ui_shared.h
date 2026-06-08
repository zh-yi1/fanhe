#ifndef _HOME_UI_SHARED_H
#define _HOME_UI_SHARED_H

#include "home_icon_res.h"

/* Home / Heat / Mode 互斥，Tab 图标与状态栏/横线缓冲共用一份 */
#define HOME_UI_SHARED_TAB_CNT          3

extern u8 home_ui_shared_icon_runtime[HOME_UI_SHARED_TAB_CNT][HOME_ICON_RAM_SIZE];
extern u8 home_ui_shared_status_bt_ram[HOME_STATUS_BT_RAM_SIZE];
extern u8 home_ui_shared_status_lock_ram[HOME_STATUS_LOCK_RAM_SIZE];
extern u8 home_ui_shared_status_bat_ram[HOME_STATUS_BAT_RAM_SIZE];
extern u8 home_ui_shared_dash_runtime_sel[HOME_DASH_RAM_SIZE];
extern u8 home_ui_shared_dash_runtime_nor[HOME_DASH_RAM_SIZE];
extern u8 home_ui_shared_top_time_digit_ram[HOME_TOP_TIME_DIGIT_SLOTS][HOME_TOP_TIME_DIGIT_RAM_MAX_SIZE];
extern u8 home_ui_shared_top_time_colon_ram[HOME_TOP_TIME_COLONM_RAM_SIZE];
extern u8 home_ui_shared_top_time_ampm_ram[HOME_TOP_TIME_AMPM_RAM_MAX_SIZE];
extern bool home_ui_shared_status_inited;
extern bool home_ui_shared_dash_inited;

void home_ui_shared_status_init(void);
void home_ui_shared_dash_init(void);

#endif
