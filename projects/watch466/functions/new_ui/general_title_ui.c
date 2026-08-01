#include "include.h"
#include "func.h"
#include "func_key_lock.h"
#include "general_ui.h"
#include "general_title_ui.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/* 电量档位 0~4 映射到 DL1~DL4 图标（与 general_status_bar 一致） */
static const u32 tb_bat_icons[5] = {
    UI_BUF_NEW_UI_DL1_BIN,
    UI_BUF_NEW_UI_DL1_BIN,
    UI_BUF_NEW_UI_DL2_BIN,
    UI_BUF_NEW_UI_DL3_BIN,
    UI_BUF_NEW_UI_DL4_BIN,
};

static const u32 tb_charge_icons[4] = {
    UI_BUF_NEW_UI_CHARGING_1_BIN,
    UI_BUF_NEW_UI_CHARGING_2_BIN,
    UI_BUF_NEW_UI_CHARGING_3_BIN,
    UI_BUF_NEW_UI_CHARGING_4_BIN,
};

/* ---- 公开 API ---- */

void general_title_bar_create(compo_form_t *frm, general_title_bar_t *bar,
                              const char *title, ui_sys_t *sys_data)
{
    compo_picturebox_t *pic;
    compo_textbox_t *txt;
    s16 bat_x, bt_x;

    if (frm == NULL || bar == NULL) {
        return;
    }

    /* 右侧图标 x 坐标：电池 → 蓝牙（从右往左排） */
    bat_x = (s16)(GUI_SCREEN_WIDTH - GENERAL_TB_RIGHT_MARGIN - NEW_HOME_BAT_W / 2);
    bt_x  = (s16)(bat_x - NEW_HOME_BAT_W / 2 - GENERAL_TB_ICON_GAP - NEW_HOME_BT_W / 2);

    /* 时间 textbox — 左侧（FONT_18，与 status_bar 一致） */
    txt = compo_textbox_create(frm, 32);
    compo_setid(txt, GENERAL_TB_ID_TXT_TIME);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_TEST_18_BIN);
    compo_textbox_set_location(txt, GENERAL_TB_LEFT_MARGIN,
                               (s16)(GENERAL_TB_Y - 14), 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
    bar->txt_time = txt;

    /* 页面标题 — 时间右侧
     * 必须用 FONT_TEST_14：FONT_TEST_18 对中文标题渲染会卡住。 */
    if (title != NULL) {
        s16 name_x = (s16)(GENERAL_TB_LEFT_MARGIN + 125);
        txt = compo_textbox_create(frm, 32);
        compo_setid(txt, GENERAL_TB_ID_TXT_TITLE);
        compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_TEST_14_BIN);
        compo_textbox_set_location(txt, name_x,
                                   (s16)(GENERAL_TB_Y - 12), 0, 0);
        compo_textbox_set_autosize(txt, true);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_forecolor(txt, COLOR_BLACK);
        compo_textbox_set(txt, title);
        bar->txt_title = txt;
    } else {
        bar->txt_title = NULL;
    }

    /* 蓝牙图标 — 默认隐藏，tick 中根据 bt_linked 显隐 */
    pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_BLUETOOTH_BIN);
    compo_setid(pic, GENERAL_TB_ID_PIC_BT);
    compo_picturebox_set_pos(pic, bt_x, GENERAL_TB_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);
    compo_picturebox_set_visible(pic, false);
    bar->pic_bt = pic;

    /* 电量图标 */
    pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_DL1_BIN);
    compo_setid(pic, GENERAL_TB_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, bat_x, GENERAL_TB_Y);
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

void general_title_bar_set_title(general_title_bar_t *bar, const char *title)
{
    if (bar == NULL || bar->txt_title == NULL || title == NULL) {
        return;
    }
    compo_textbox_set(bar->txt_title, title);
}

void general_title_bar_bind(general_title_bar_t *bar)
{
    if (bar == NULL) {
        return;
    }
    bar->txt_time  = (compo_textbox_t *)compo_getobj_byid(GENERAL_TB_ID_TXT_TIME);
    bar->txt_title = (compo_textbox_t *)compo_getobj_byid(GENERAL_TB_ID_TXT_TITLE);
    bar->pic_bt    = (compo_picturebox_t *)compo_getobj_byid(GENERAL_TB_ID_PIC_BT);
    bar->pic_bat   = (compo_picturebox_t *)compo_getobj_byid(GENERAL_TB_ID_PIC_BAT);

    home_ui_shared_status_init();
    if (bar->pic_bt != NULL && home_ui_shared_status_bt_ram != NULL
        && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        home_ui_shared_status_refresh_bt(bar->pic_bt);
    }
    home_ui_shared_status_bind_bat(bar->pic_bat);
}

void general_title_bar_attach(general_title_bar_t *bar)
{
    if (bar == NULL) {
        return;
    }

    /*
     * enter 阶段只做 Flash→RAM 预载 + 图标绑定，不要 te_block + home_gpu_wait_idle。
     * form_create 刚画完标题文字时 GUI 可能仍有未完成绘制；若此处再置 te_block=1
     * 并 wait_idle，会与 gui 线程互相等待 → 卡在 "bt icon hide" 之后，屏幕不切页。
     * 首帧由随后的 func_process/gui_process 正常刷新。
     */
    home_ui_shared_status_init();
    home_ui_shared_bt_icon_wake_reset();

    if (bar->pic_bt != NULL && home_ui_shared_status_bt_ram != NULL
        && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        home_ui_shared_status_refresh_bt(bar->pic_bt);
    }
    home_ui_shared_status_bind_bat(bar->pic_bat);
    printf("title_bar: attach done\n");
}

void general_title_bar_detach(void)
{
#if ELUNCHBOX_PANEL_EN
    home_ui_shared_battery_detach_pic();
#endif
}

void general_title_bar_tick(general_title_bar_t *bar)
{
    char buf[16];

    if (bar == NULL || bar->txt_time == NULL || bar->sys_data == NULL) {
        return;
    }

    /* ---- 时间 ---- */
    if (bar->sys_data->hour != bar->last_hour || bar->sys_data->min != bar->last_min) {
        bar->last_hour = bar->sys_data->hour;
        bar->last_min  = bar->sys_data->min;
        snprintf(buf, sizeof(buf), "%02d:%02d", bar->sys_data->hour, bar->sys_data->min);
        compo_textbox_set(bar->txt_time, buf);
    }

    /* ---- 电量图标 ---- */
    if (bar->pic_bat != NULL) {
        if (bar->sys_data->charging && !bar->sys_data->full_charge) {
            if (tick_check_expire(bar->charge_tick_ms, 500)) {
                bar->charge_tick_ms = tick_get();
                compo_picturebox_set(bar->pic_bat, tb_charge_icons[bar->charge_frame]);
                bar->charge_frame = (bar->charge_frame + 1) & 0x03;
            }
            bar->last_bat_level   = 0xff;
            bar->last_full_charge = false;
        } else if (bar->sys_data->charging && bar->sys_data->full_charge) {
            if (bar->last_full_charge != true || bar->last_charging != true) {
                bar->last_charging    = true;
                bar->last_full_charge = true;
                bar->last_bat_level   = 0xff;
                bar->charge_tick_ms   = 0;
                bar->charge_frame     = 0;
                compo_picturebox_set(bar->pic_bat, tb_charge_icons[3]);
            }
        } else if (bar->sys_data->bat_level != bar->last_bat_level
                   || bar->sys_data->full_charge != bar->last_full_charge
                   || bar->sys_data->charging != bar->last_charging) {
            bar->last_bat_level   = bar->sys_data->bat_level;
            bar->last_full_charge = bar->sys_data->full_charge;
            bar->last_charging    = bar->sys_data->charging;
            bar->charge_tick_ms   = 0;
            bar->charge_frame     = 0;

            if (bar->sys_data->bat_level < 5) {
                compo_picturebox_set(bar->pic_bat,
                                     tb_bat_icons[bar->sys_data->bat_level]);
            }
        }
    }

    /* ---- 蓝牙图标 ---- */
    if (bar->pic_bt != NULL && bar->sys_data->bt_linked != bar->last_bt_linked) {
        bar->last_bt_linked = bar->sys_data->bt_linked;
        compo_picturebox_set_visible(bar->pic_bt, bar->sys_data->bt_linked);
    }
}
