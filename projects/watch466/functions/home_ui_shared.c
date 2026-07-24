#include "include.h"
#include "home_ui_shared.h"
#include "func.h"
#if FUNC_LUNCHBOX_UART_EN
#include "func_lunchbox_uart.h"
#endif

/* 保留在 sram BSS；新主页 Tab 79×88×3，Home/Heat/Mode 互斥共用 */
u8 home_ui_shared_icon_runtime[HOME_UI_SHARED_TAB_CNT][NEW_HOME_TAB_RAM_SIZE];
u8 home_ui_shared_status_bt_ram[HOME_STATUS_BT_RAM_SIZE];
u8 home_ui_shared_status_lock_ram[HOME_STATUS_LOCK_RAM_SIZE];
u8 home_ui_shared_status_bat_ram[HOME_STATUS_BAT_RAM_SIZE];
u8 home_ui_shared_dash_runtime_sel[HOME_DASH_RAM_SIZE];
u8 home_ui_shared_dash_runtime_nor[HOME_DASH_RAM_SIZE];
u8 home_ui_shared_top_time_digit_ram[HOME_TOP_TIME_DIGIT_SLOTS][HOME_TOP_TIME_DIGIT_RAM_MAX_SIZE];
u8 home_ui_shared_top_time_colon_ram[HOME_TOP_TIME_COLONM_RAM_SIZE];
u8 home_ui_shared_top_time_ampm_ram[HOME_TOP_TIME_AMPM_RAM_MAX_SIZE];
bool home_ui_shared_status_inited;
bool home_ui_shared_status_lock_preloaded;
bool home_ui_shared_dash_inited;

/* Heat/Mode 中部倒计时 wbx + w0x 共享缓冲（BSS，不增加总量） */
u8 home_ui_shared_timer_colon_ram[HEAT_WBX_RAM_SIZE];
u8 home_ui_shared_timer_digit_ram[4][HEAT_B_DIGIT_RAM_MAX_SIZE];

#if ELUNCHBOX_PANEL_EN
static u8 home_bat_level = 4;
static u8 home_bat_charge = 0;
static u8 home_bat_icon_idx = 0xFF;
static u8 home_bat_chg_frame = 0;
static u32 home_bat_chg_tick = 0;
static compo_picturebox_t *home_bat_pic;

#define HOME_BAT_CHG_ANIM_MS            500

static void home_ui_shared_battery_refresh_attached(void);

enum {
    HOME_BAT_ICON_DL1 = 1,
    HOME_BAT_ICON_DL2 = 2,
    HOME_BAT_ICON_DL3 = 3,
    HOME_BAT_ICON_DL4 = 4,
    HOME_BAT_ICON_CHG1 = 5,
    HOME_BAT_ICON_CHG2 = 6,
    HOME_BAT_ICON_CHG3 = 7,
    HOME_BAT_ICON_CHG4 = 8,
};

static u8 home_bat_pick_icon(void)
{
    if (home_bat_charge == 2) {
        return HOME_BAT_ICON_CHG4;
    }
    if (home_bat_charge == 1) {
        return (u8)(HOME_BAT_ICON_CHG1 + home_bat_chg_frame);
    }
    if (home_bat_level == 0) {
        return HOME_BAT_ICON_DL1;
    }
    if (home_bat_level >= 1 && home_bat_level <= 4) {
        return home_bat_level;
    }
    return HOME_BAT_ICON_DL4;
}

static void home_bat_chg_anim_reset(void)
{
    home_bat_chg_frame = 0;
    home_bat_chg_tick = tick_get();
}

static void home_bat_charge_state_apply(u8 charge_sta)
{
    u8 prev = home_bat_charge;

    if (charge_sta > 2) {
        charge_sta = 0;
    }
    home_bat_charge = charge_sta;
    if (charge_sta == 1 && prev != 1) {
        home_bat_chg_anim_reset();
    }
}

static bool home_bat_icon_flash(u8 icon, u32 *addr, u32 *len)
{
#ifdef UI_BUF_NEW_UI_NEW_DL4_BIN
    switch (icon) {
    case HOME_BAT_ICON_CHG1:
        *addr = UI_BUF_NEW_UI_NEW_CHARGING_1_BIN;
        *len = UI_LEN_NEW_UI_NEW_CHARGING_1_BIN;
        return true;
    case HOME_BAT_ICON_CHG2:
        *addr = UI_BUF_NEW_UI_NEW_CHARGING_2_BIN;
        *len = UI_LEN_NEW_UI_NEW_CHARGING_2_BIN;
        return true;
    case HOME_BAT_ICON_CHG3:
        *addr = UI_BUF_NEW_UI_NEW_CHARGING_3_BIN;
        *len = UI_LEN_NEW_UI_NEW_CHARGING_3_BIN;
        return true;
    case HOME_BAT_ICON_CHG4:
        *addr = UI_BUF_NEW_UI_NEW_CHARGING_4_BIN;
        *len = UI_LEN_NEW_UI_NEW_CHARGING_4_BIN;
        return true;
    case HOME_BAT_ICON_DL1:
        *addr = UI_BUF_NEW_UI_NEW_DL1_BIN;
        *len = UI_LEN_NEW_UI_NEW_DL1_BIN;
        return true;
    case HOME_BAT_ICON_DL2:
        *addr = UI_BUF_NEW_UI_NEW_DL2_BIN;
        *len = UI_LEN_NEW_UI_NEW_DL2_BIN;
        return true;
    case HOME_BAT_ICON_DL3:
        *addr = UI_BUF_NEW_UI_NEW_DL3_BIN;
        *len = UI_LEN_NEW_UI_NEW_DL3_BIN;
        return true;
    case HOME_BAT_ICON_DL4:
    default:
        *addr = UI_BUF_NEW_UI_NEW_DL4_BIN;
        *len = UI_LEN_NEW_UI_NEW_DL4_BIN;
        return true;
    }
#elif defined(UI_BUF_HOME_DL4_BIN)
    switch (icon) {
    case HOME_BAT_ICON_CHG1:
        *addr = UI_BUF_HOME_DL_BIN;
        *len = UI_LEN_HOME_DL_BIN;
        return true;
    case HOME_BAT_ICON_CHG2:
        *addr = UI_BUF_HOME_DL1_BIN;
        *len = UI_LEN_HOME_DL1_BIN;
        return true;
    case HOME_BAT_ICON_CHG3:
        *addr = UI_BUF_HOME_DL2_BIN;
        *len = UI_LEN_HOME_DL2_BIN;
        return true;
    case HOME_BAT_ICON_CHG4:
        *addr = UI_BUF_HOME_DL3_BIN;
        *len = UI_LEN_HOME_DL3_BIN;
        return true;
    case HOME_BAT_ICON_DL1:
        *addr = UI_BUF_HOME_DL1_BIN;
        *len = UI_LEN_HOME_DL1_BIN;
        return true;
    case HOME_BAT_ICON_DL2:
        *addr = UI_BUF_HOME_DL2_BIN;
        *len = UI_LEN_HOME_DL2_BIN;
        return true;
    case HOME_BAT_ICON_DL3:
        *addr = UI_BUF_HOME_DL3_BIN;
        *len = UI_LEN_HOME_DL3_BIN;
        return true;
    case HOME_BAT_ICON_DL4:
    default:
        *addr = UI_BUF_HOME_DL4_BIN;
        *len = UI_LEN_HOME_DL4_BIN;
        return true;
    }
#else
    (void)icon;
    (void)addr;
    (void)len;
    return false;
#endif
}

static void home_ui_shared_battery_refresh_attached(void)
{
    if (home_bat_pic == NULL) {
        return;
    }
    if (gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        compo_picturebox_set_ram(home_bat_pic, home_ui_shared_status_bat_ram);
#if defined(NEW_HOME_BAT_W) && defined(NEW_HOME_BAT_H)
        compo_picturebox_set_size(home_bat_pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);
#else
        compo_picturebox_set_size(home_bat_pic, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
#endif
        compo_picturebox_set_visible(home_bat_pic, true);
    }
}

static void home_ui_shared_battery_mark_dirty(void)
{
#if ELUNCHBOX_PANEL_EN
    func_home_gui_mark_dirty();
#endif
}

static void home_ui_shared_battery_reload(void)
{
    u8 icon;
    u32 addr;
    u32 len;
    bool icon_changed;

    icon = home_bat_pick_icon();
    if (!home_bat_icon_flash(icon, &addr, &len)) {
        return;
    }
    icon_changed = (icon != home_bat_icon_idx || !home_ui_shared_status_inited);
    if (icon_changed) {
        /* 勿 wait_idle：Home 切子页后 gui thread miss 时会死等导致 WDT */
        WDT_CLR();
        os_spiflash_read(home_ui_shared_status_bat_ram, addr, len);
        home_bat_icon_idx = icon;
    }

    if (home_bat_pic != NULL && icon_changed) {
        home_ui_shared_battery_refresh_attached();
        home_ui_shared_battery_mark_dirty();
    }
}

void home_ui_shared_battery_boot_init(void)
{
    home_bat_level = 4;
    home_bat_charge = 0;
    home_bat_icon_idx = 0xFF;
    home_bat_chg_anim_reset();
    home_ui_shared_battery_reload();
}

void home_ui_shared_battery_attach_pic(compo_picturebox_t *pic)
{
    home_bat_pic = pic;
    home_ui_shared_status_bind_bat(pic);
}

void home_ui_shared_battery_detach_pic(void)
{
    home_bat_pic = NULL;
}

void home_ui_shared_battery_feed_dp(u8 *data, u16 len)
{
    u16 off = 0;
    bool got_bat = false;
    bool got_chg = false;
    u8 bat = home_bat_level;
    u8 chg = home_bat_charge;

    if (data == NULL || len < 4) {
        return;
    }

    while (off + 4 <= len) {
        u8  dpid    = data[off];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];

        if (off + 4 + val_len > len) {
            break;
        }
        u8 *val = data + off + 4;

#if FUNC_LUNCHBOX_UART_EN
        switch (dpid) {
        case LB_DPID_BATTERY:
            if (val_len >= 1) {
                bat = val[0];
                got_bat = true;
            }
            break;
        case LB_DPID_CHARGE_STATUS:
            if (val_len >= 1) {
                chg = val[0];
                got_chg = true;
            }
            break;
        default:
            break;
        }
#endif
        off += 4 + val_len;
    }

    if (!got_bat && !got_chg) {
        return;
    }
    if (bat > 4) {
        bat = 4;
    }
    if (chg > 2) {
        chg = 0;
    }
    if (bat == home_bat_level && chg == home_bat_charge) {
        return;
    }

    home_bat_level = bat;
    home_bat_charge_state_apply(chg);
    home_bat_icon_idx = 0xFF;

#if ELUNCHBOX_PANEL_EN
    if (!elunchbox_ui_is_live() || !is_gpu_init()) {
        return;
    }
#endif
    home_ui_shared_battery_reload();
}

u8 home_ui_shared_battery_level(void)
{
    return home_bat_level;
}

bool home_ui_shared_battery_is_low(void)
{
    return home_bat_level <= 1;
}

bool home_ui_shared_battery_is_charging(void)
{
    return home_bat_charge != 0;
}

void home_ui_shared_battery_charge_apply(u8 charge_sta)
{
    if (charge_sta > 2) {
        charge_sta = 0;
    }
    if (home_bat_charge == charge_sta) {
        return;
    }
    home_bat_charge_state_apply(charge_sta);
    home_bat_icon_idx = 0xFF;
    home_ui_shared_battery_reload();
}

void home_ui_shared_battery_chg_poll(void)
{
    u8 icon;

    if (home_bat_charge != 1) {
        return;
    }
    if (!elunchbox_ui_is_live() || sys_cb.flag_swithing) {
        return;
    }
    if (!tick_check_expire(home_bat_chg_tick, HOME_BAT_CHG_ANIM_MS)) {
        return;
    }
    home_bat_chg_tick = tick_get();
    home_bat_chg_frame = (u8)((home_bat_chg_frame + 1u) & 3u);
    icon = home_bat_pick_icon();
    if (icon == home_bat_icon_idx) {
        return;
    }
    home_bat_icon_idx = 0xFF;
    home_ui_shared_battery_reload();
}

void home_ui_shared_battery_icon_refresh(void)
{
    home_bat_icon_idx = 0xFF;
    home_ui_shared_battery_reload();
}

u32 home_ui_shared_battery_flash_addr(void)
{
    u32 addr = 0;
    u32 len = 0;

    if (!home_bat_icon_flash(home_bat_pick_icon(), &addr, &len)) {
#ifdef UI_BUF_NEW_UI_NEW_DL4_BIN
        return UI_BUF_NEW_UI_NEW_DL4_BIN;
#elif defined(UI_BUF_HOME_DL4_BIN)
        return UI_BUF_HOME_DL4_BIN;
#else
        return 0;
#endif
    }
    return addr;
}

void home_ui_shared_status_bind_bat(compo_picturebox_t *pic)
{
    if (pic == NULL) {
        return;
    }
    home_ui_shared_status_init();
    home_ui_shared_battery_reload();
    if (gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        compo_picturebox_set_ram(pic, home_ui_shared_status_bat_ram);
#if defined(NEW_HOME_BAT_W) && defined(NEW_HOME_BAT_H)
        compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);
#else
        compo_picturebox_set_size(pic, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
#endif
        compo_picturebox_set_visible(pic, true);
    }
}
#else
void home_ui_shared_battery_boot_init(void)
{
}

void home_ui_shared_battery_attach_pic(compo_picturebox_t *pic)
{
    (void)pic;
}

void home_ui_shared_battery_detach_pic(void)
{
}

void home_ui_shared_battery_feed_dp(u8 *data, u16 len)
{
    (void)data;
    (void)len;
}

u32 home_ui_shared_battery_flash_addr(void)
{
    return 0;
}

u8 home_ui_shared_battery_level(void)
{
    return 4;
}

bool home_ui_shared_battery_is_low(void)
{
    return false;
}

bool home_ui_shared_battery_is_charging(void)
{
    return false;
}

void home_ui_shared_battery_charge_apply(u8 charge_sta)
{
    (void)charge_sta;
}

void home_ui_shared_battery_icon_refresh(void)
{
}

void home_ui_shared_battery_chg_poll(void)
{
}

void home_ui_shared_status_bind_bat(compo_picturebox_t *pic)
{
    (void)pic;
}
#endif

void home_ui_shared_status_lock_preload(void)
{
    if (home_ui_shared_status_lock_preloaded) {
        return;
    }
    os_spiflash_read(home_ui_shared_status_lock_ram, UI_BUF_HOME_LOCK_BIN, UI_LEN_HOME_LOCK_BIN);
    home_ui_shared_status_lock_preloaded = true;
}

/* 蓝牙图标须为白底黑图（RGB565: 0xFFFF 底 + 0x0000 描边） */
static bool home_ui_shared_bt_ram_has_shape(const u8 *ram, u32 len)
{
    u16 w;
    u16 h;
    u32 i;
    u32 n;

    if (ram == NULL || len < 8 || GET_LE32(&ram[0]) != 0x24150) {
        return false;
    }
    w = GET_LE16(&ram[4]);
    h = GET_LE16(&ram[6]);
    n = (u32)w * h;
    if (n == 0 || 8 + n * 2 > len) {
        return false;
    }
    for (i = 0; i < n; i++) {
        if (GET_LE16(&ram[8 + i * 2]) != 0xFFFF) {
            return true;
        }
    }
    return false;
}

static bool home_ui_shared_bt_ram_border_mostly_black(const u8 *ram, u32 len)
{
    u16 w;
    u16 h;
    u16 black;
    u16 total;
    u16 x;
    u16 y;

    if (ram == NULL || len < 8 || GET_LE32(&ram[0]) != 0x24150) {
        return false;
    }
    w = GET_LE16(&ram[4]);
    h = GET_LE16(&ram[6]);
    if (w == 0 || h == 0 || 8 + (u32)w * h * 2 > len) {
        return false;
    }
    black = 0;
    total = 0;
    for (x = 0; x < w; x++) {
        for (y = 0; y < h; y++) {
            if (x != 0 && x + 1 != w && y != 0 && y + 1 != h) {
                continue;
            }
            total++;
            if (GET_LE16(&ram[8 + ((u32)y * w + x) * 2]) == 0) {
                black++;
            }
        }
    }
    return total > 0 && black * 2 > total;
}

static void home_ui_shared_bt_ram_invert(u8 *ram, u32 len)
{
    u16 w;
    u16 h;
    u32 i;
    u32 n;

    if (ram == NULL || len < 8 || GET_LE32(&ram[0]) != 0x24150) {
        return;
    }
    w = GET_LE16(&ram[4]);
    h = GET_LE16(&ram[6]);
    n = (u32)w * h;
    if (n == 0 || 8 + n * 2 > len) {
        return;
    }
    for (i = 0; i < n; i++) {
        u16 *pc = (u16 *)(void *)&ram[8 + i * 2];

        if (*pc == 0) {
            *pc = 0xFFFF;
        } else if (*pc == 0xFFFF) {
            *pc = 0;
        }
    }
}

static void home_ui_shared_bt_ram_ensure_white_bg(u8 *ram, u32 len)
{
    if (home_ui_shared_bt_ram_border_mostly_black(ram, len)) {
        home_ui_shared_bt_ram_invert(ram, len);
    }
}

void home_ui_shared_status_init(void)
{
    if (home_ui_shared_status_inited) {
        return;
    }
#ifdef UI_BUF_NEW_UI_NEW_BLUETOOTH_BIN
    os_spiflash_read(home_ui_shared_status_bt_ram, UI_BUF_NEW_UI_NEW_BLUETOOTH_BIN,
                     UI_LEN_NEW_UI_NEW_BLUETOOTH_BIN);
#if defined(UI_BUF_HOME_BLUETOOTH_BIN) && defined(UI_LEN_HOME_BLUETOOTH_BIN)
    if (!home_ui_shared_bt_ram_has_shape(home_ui_shared_status_bt_ram,
                                         UI_LEN_NEW_UI_NEW_BLUETOOTH_BIN)) {
        os_spiflash_read(home_ui_shared_status_bt_ram, UI_BUF_HOME_BLUETOOTH_BIN,
                         UI_LEN_HOME_BLUETOOTH_BIN);
    }
#endif
#else
    os_spiflash_read(home_ui_shared_status_bt_ram, UI_BUF_HOME_BLUETOOTH_BIN, UI_LEN_HOME_BLUETOOTH_BIN);
#endif
    home_ui_shared_bt_ram_ensure_white_bg(home_ui_shared_status_bt_ram, HOME_STATUS_BT_RAM_SIZE);
    if (!home_ui_shared_status_lock_preloaded) {
        os_spiflash_read(home_ui_shared_status_lock_ram, UI_BUF_HOME_LOCK_BIN, UI_LEN_HOME_LOCK_BIN);
    }
    home_ui_shared_status_lock_preloaded = true;
#if ELUNCHBOX_PANEL_EN
    home_ui_shared_battery_reload();
#else
#ifndef UI_BUF_HOME_BATTERY_LEVEL_BIN
    os_spiflash_read(home_ui_shared_status_bat_ram, UI_BUF_HOME_DL4_BIN, UI_LEN_HOME_DL4_BIN);
#else
    os_spiflash_read(home_ui_shared_status_bat_ram, UI_BUF_HOME_BATTERY_LEVEL_BIN, UI_LEN_HOME_BATTERY_LEVEL_BIN);
#endif
#endif
    home_ui_shared_status_inited = true;
}

#if ELUNCHBOX_PANEL_EN
static compo_picturebox_t *home_ui_shared_attached_bt_pic;
static bool home_ui_shared_bt_last_linked;
static bool home_ui_shared_bt_link_dirty;
static bool home_ui_shared_bt_icon_inited;

void home_ui_shared_bt_icon_wake_reset(void)
{
    home_ui_shared_bt_icon_inited = false;
}
#endif

bool home_ui_shared_ble_linked(void)
{
#if LE_EN
    if (ble_is_connected()) {
        return true;
    }
    return ble_is_connect();
#else
    return false;
#endif
}

void home_ui_shared_status_refresh_bt(compo_picturebox_t *pic)
{
    bool vis;

    if (pic == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    home_ui_shared_attached_bt_pic = pic;
#endif
    home_ui_shared_status_init();
    if (!gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        return;
    }
    vis = home_ui_shared_ble_linked();
#if ELUNCHBOX_PANEL_EN
    /* 仅首次或连接状态变化时才做 GPU 绑定，避免每帧 set_ram/set_size/widget_set_top
     * 挤占 TE 间隔 → gui thread miss。唤醒后调 home_ui_shared_bt_icon_wake_reset 可强制重绑。 */
    if (!home_ui_shared_bt_icon_inited || vis != home_ui_shared_bt_last_linked) {
        printf("elunchbox: bt icon %s (linked=%u)\n", vis ? "show" : "hide", vis ? 1u : 0u);
        home_ui_shared_bt_last_linked = vis;
        home_ui_shared_bt_icon_inited = true;

        compo_picturebox_set_ram(pic, home_ui_shared_status_bt_ram);
#if defined(NEW_HOME_BT_W) && defined(NEW_HOME_BT_H)
        compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);
#else
        compo_picturebox_set_size(pic, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
#endif
        compo_picturebox_set_visible(pic, vis);
        if (vis && pic->img != NULL) {
            widget_set_top(pic->img, true);
        }
    }
#else
    compo_picturebox_set_ram(pic, home_ui_shared_status_bt_ram);
    compo_picturebox_set_size(pic, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
    compo_picturebox_set_visible(pic, vis);
    if (vis && pic->img != NULL) {
        widget_set_top(pic->img, true);
    }
#endif
}

void home_ui_shared_ble_status_poll(void)
{
#if ELUNCHBOX_PANEL_EN && LE_EN
    compo_picturebox_t *pic;
    bool linked;

    if (!elunchbox_ui_is_live() || sys_cb.flag_swithing) {
        return;
    }
    pic = home_ui_shared_attached_bt_pic;
    if (pic == NULL) {
        return;
    }
    linked = home_ui_shared_ble_linked();
    if (!home_ui_shared_bt_link_dirty
        && home_ui_shared_bt_icon_inited
        && linked == home_ui_shared_bt_last_linked) {
        return;
    }
    home_ui_shared_bt_link_dirty = false;
    home_ui_shared_status_refresh_bt(pic);
#endif
}

void home_ui_shared_ble_link_notify(void)
{
#if ELUNCHBOX_PANEL_EN
    home_ui_shared_bt_link_dirty = true;
    func_home_gui_mark_dirty();
#endif
}

void home_ui_shared_bt_detach_pic(void)
{
#if ELUNCHBOX_PANEL_EN
    home_ui_shared_attached_bt_pic = NULL;
#endif
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

void home_gpu_wait_idle(void)
{
    os_gui_draw_w4_done();
    os_gui_draw_w4_done();
}
