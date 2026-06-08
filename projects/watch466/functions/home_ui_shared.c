#include "include.h"
#include "home_ui_shared.h"

/* 保留在 sram BSS（勿放 .disp.home_ram，disp 仅 96KB 已满）；Home/Mode 互斥共用 icon 槽 */
u8 home_ui_shared_icon_runtime[HOME_UI_SHARED_TAB_CNT][HOME_ICON_RAM_SIZE];
u8 home_ui_shared_status_bt_ram[HOME_STATUS_BT_RAM_SIZE];
u8 home_ui_shared_status_lock_ram[HOME_STATUS_LOCK_RAM_SIZE];
u8 home_ui_shared_status_bat_ram[HOME_STATUS_BAT_RAM_SIZE];
u8 home_ui_shared_dash_runtime_sel[HOME_DASH_RAM_SIZE];
u8 home_ui_shared_dash_runtime_nor[HOME_DASH_RAM_SIZE];
bool home_ui_shared_status_inited;
bool home_ui_shared_dash_inited;

void home_ui_shared_status_init(void)
{
    if (home_ui_shared_status_inited) {
        return;
    }
    os_spiflash_read(home_ui_shared_status_bt_ram, UI_BUF_HOME_BLUETOOTH_BIN, UI_LEN_HOME_BLUETOOTH_BIN);
    os_spiflash_read(home_ui_shared_status_lock_ram, UI_BUF_HOME_LOCK_BIN, UI_LEN_HOME_LOCK_BIN);
    os_spiflash_read(home_ui_shared_status_bat_ram, UI_BUF_HOME_BATTERY_LEVEL_BIN, UI_LEN_HOME_BATTERY_LEVEL_BIN);
    home_ui_shared_status_inited = true;
}

void home_ui_shared_dash_init(void)
{
    if (home_ui_shared_dash_inited) {
        return;
    }
    os_spiflash_read(home_ui_shared_dash_runtime_sel, UI_BUF_HOME_WHILE_LINE_BIN, UI_LEN_HOME_WHILE_LINE_BIN);
    os_spiflash_read(home_ui_shared_dash_runtime_nor, UI_BUF_HOME_BLUE_LINE_BIN, UI_LEN_HOME_BLUE_LINE_BIN);
    home_ui_shared_dash_inited = true;
}
