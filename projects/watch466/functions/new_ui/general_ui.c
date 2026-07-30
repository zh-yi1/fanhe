#include "include.h"
#include "func.h"
#include "func_key_lock.h"
#include "general_ui.h"
#include "ui.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/* ---- 内部 helpers ---- */

/* 电量档位 0~4 映射到 DL1~DL4 图标 */
static const u32 sb_bat_icons[5] = {
    UI_BUF_NEW_UI_DL1_BIN,  /* 0: 空 */
    UI_BUF_NEW_UI_DL1_BIN,  /* 1: 25% */
    UI_BUF_NEW_UI_DL2_BIN,  /* 2: 50% */
    UI_BUF_NEW_UI_DL3_BIN,  /* 3: 75% */
    UI_BUF_NEW_UI_DL4_BIN,  /* 4: 100% */
};

/* 充电动画帧 */
static const u32 sb_charge_icons[4] = {
    UI_BUF_NEW_UI_CHARGING_1_BIN,
    UI_BUF_NEW_UI_CHARGING_2_BIN,
    UI_BUF_NEW_UI_CHARGING_3_BIN,
    UI_BUF_NEW_UI_CHARGING_4_BIN,
};

/* ---- 公开 API ---- */
void general_status_bar_create(compo_form_t *frm, general_status_bar_t *bar,
                               const char *page_name, ui_sys_t *sys_data)
{
    compo_picturebox_t *pic;
    compo_textbox_t *txt;
    s16 bat_x, bt_x;

    if (frm == NULL || bar == NULL) {
        return;
    }

    /* 右侧图标 x 坐标：电池 → 蓝牙（从右往左排） */
    bat_x = (s16)(GUI_SCREEN_WIDTH - GENERAL_SB_RIGHT_MARGIN - NEW_HOME_BAT_W / 2);
    bt_x  = (s16)(bat_x - NEW_HOME_BAT_W / 2 - GENERAL_SB_ICON_GAP - NEW_HOME_BT_W / 2);

    /* 时间 textbox — 左侧 */
    txt = compo_textbox_create(frm, 32);
    compo_setid(txt, GENERAL_SB_ID_TXT_TIME);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_TEST_18_BIN);
    /* textbox Y 左上角，图标 Y 中心；18px 字偏高，-14 补偿 */
    compo_textbox_set_location(txt, GENERAL_SB_LEFT_MARGIN,
                               (s16)(GENERAL_SB_Y - 14), 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
    bar->txt_time = txt;

    /* 页面名 textbox — 时间右侧；page_name 为 NULL 时不显示 */
    if (page_name != NULL) {
        s16 name_x = (s16)(GENERAL_SB_LEFT_MARGIN + 125); /* 给 "HH:MM" 留 5 个字符宽度 */
        txt = compo_textbox_create(frm, 32);
        compo_setid(txt, GENERAL_SB_ID_TXT_PAGENAME);
        compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_TEST_14_BIN);
        compo_textbox_set_location(txt, name_x,
                                   (s16)(GENERAL_SB_Y - 12), 0, 0);
        compo_textbox_set_autosize(txt, true);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_forecolor(txt, COLOR_BLACK);
        compo_textbox_set(txt, page_name);
        bar->txt_pagename = txt;
    } else {
        bar->txt_pagename = NULL;
    }

    /* 蓝牙图标 — 默认隐藏，tick 中根据 bar->sys_data->bt_linked 显隐 */
    pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_BLUETOOTH_BIN);
    compo_setid(pic, GENERAL_SB_ID_PIC_BT);
    compo_picturebox_set_pos(pic, bt_x, GENERAL_SB_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);
    compo_picturebox_set_visible(pic, false);
    bar->pic_bt = pic;

    /* 电量图标 */
    pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_DL1_BIN);
    compo_setid(pic, GENERAL_SB_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, bat_x, GENERAL_SB_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);
    bar->pic_bat = pic;

    bar->sys_data         = sys_data;
    bar->last_min         = 0xff;
    bar->last_hour        = 0xff;
    bar->last_bat_level   = 0xff;
    bar->last_charging    = false;
    bar->last_full_charge = false;
    bar->last_bt_linked   = false;
    bar->charge_tick_ms   = 0;
    bar->charge_frame     = 0;
}

void general_status_bar_bind(general_status_bar_t *bar)
{
    if (bar == NULL) {
        return;
    }
    bar->txt_time     = (compo_textbox_t *)compo_getobj_byid(GENERAL_SB_ID_TXT_TIME);
    bar->txt_pagename = (compo_textbox_t *)compo_getobj_byid(GENERAL_SB_ID_TXT_PAGENAME);
    bar->pic_bt       = (compo_picturebox_t *)compo_getobj_byid(GENERAL_SB_ID_PIC_BT);
    bar->pic_bat      = (compo_picturebox_t *)compo_getobj_byid(GENERAL_SB_ID_PIC_BAT);

    /* 蓝牙图标：载入 Flash → RAM → 绑定到 pic */
    home_ui_shared_status_init();
    if (bar->pic_bt != NULL && home_ui_shared_status_bt_ram != NULL
        && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        home_ui_shared_status_refresh_bt(bar->pic_bt);
    }

    /* 电量图标 */
    home_ui_shared_status_bind_bat(bar->pic_bat);
}

void general_status_bar_attach(general_status_bar_t *bar)
{
    if (bar == NULL) {
        return;
    }

    /* Flash→RAM 预载 + 一次性 GPU 绑定（enter 阶段完成）。
     * 不调用 battery_attach_pic 注册自动刷新，避免 chg_poll 在
     * func_process 内频繁 SPI Flash 读取充电动画帧导致 gui thread miss。
     * 充电动画后续通过 TE 同步机制再启用。
     * Flash 读在 TE block 外，GPU 绑定在 TE block 内 + WDT_CLR，
     * 避免与 func_process 内 gui_process 竞争 TE 窗口 → gui thread miss */
    home_ui_shared_status_init();

    home_gpu_wait_idle();
    WDT_CLR();

    if (bar->pic_bt != NULL && home_ui_shared_status_bt_ram != NULL
        && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        home_ui_shared_status_refresh_bt(bar->pic_bt);
    }

    /* 静态绑定电量图标（当前电量/充电状态），不注册自动刷新 */
    home_ui_shared_status_bind_bat(bar->pic_bat);
}

void general_status_bar_detach(void)
{
#if ELUNCHBOX_PANEL_EN
    home_ui_shared_battery_detach_pic();
#endif
}

void general_status_bar_tick(general_status_bar_t *bar)
{
    char buf[16];

    if (bar == NULL || bar->txt_time == NULL || bar->sys_data == NULL) {
        return;
    }

    /* ---- 时间：从串口 g_ui_sys 读取 ---- */
    if (bar->sys_data->hour != bar->last_hour || bar->sys_data->min != bar->last_min) {
        bar->last_hour = bar->sys_data->hour;
        bar->last_min  = bar->sys_data->min;
        snprintf(buf, sizeof(buf), "%02d:%02d", bar->sys_data->hour, bar->sys_data->min);
        compo_textbox_set(bar->txt_time, buf);
    }

    /* ---- 电量图标 ---- */
    if (bar->pic_bat != NULL) {
        /* 充电中（未满）：500ms 切一帧 */
        if (bar->sys_data->charging && !bar->sys_data->full_charge) {
            if (tick_check_expire(bar->charge_tick_ms, 500)) {
                bar->charge_tick_ms = tick_get();
                compo_picturebox_set(bar->pic_bat, sb_charge_icons[bar->charge_frame]);
                bar->charge_frame = (bar->charge_frame + 1) & 0x03; /* 0→1→2→3→0 */
            }
            bar->last_bat_level    = 0xff; /* 强制退出充电后刷新 */
            bar->last_full_charge  = false;
        }
        /* 充满电（充电中充满）：静态显示 CHARGING_4 */
        else if (bar->sys_data->charging && bar->sys_data->full_charge) {
            if (bar->last_full_charge != true || bar->last_charging != true) {
                bar->last_charging    = true;
                bar->last_full_charge = true;
                bar->last_bat_level   = 0xff;
                bar->charge_tick_ms   = 0;
                bar->charge_frame     = 0;
                compo_picturebox_set(bar->pic_bat, sb_charge_icons[3]);
            }
        }
        /* 充满/非充电态：显示静态电量档位 */
        else if (bar->sys_data->bat_level != bar->last_bat_level
                 || bar->sys_data->full_charge != bar->last_full_charge
                 || bar->sys_data->charging != bar->last_charging) {
            bar->last_bat_level    = bar->sys_data->bat_level;
            bar->last_full_charge  = bar->sys_data->full_charge;
            bar->last_charging     = bar->sys_data->charging;
            bar->charge_tick_ms    = 0;
            bar->charge_frame      = 0;

            if (bar->sys_data->bat_level < 5) {
                compo_picturebox_set(bar->pic_bat,
                                     sb_bat_icons[bar->sys_data->bat_level]);
            }
        }
    }

    /* ---- 蓝牙图标：连接时显示，断开时隐藏 ---- */
    if (bar->pic_bt != NULL && bar->sys_data->bt_linked != bar->last_bt_linked) {
        bar->last_bt_linked = bar->sys_data->bt_linked;
        compo_picturebox_set_visible(bar->pic_bt, bar->sys_data->bt_linked);
    }
}
