#include "include.h"
#include "func.h"
#include "func_key_lock.h"
#include "general_ui.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/* ---- 内部 helpers ---- */

static void general_sb_time_fmt(char *buf, u8 buf_size, tm_t *tm)
{
    snprintf(buf, buf_size, "%02d:%02d", tm->hour, tm->min);
}

/* ---- 公开 API ---- */
void general_status_bar_create(compo_form_t *frm, general_status_bar_t *bar, const char *page_name)
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
    /* textbox Y 是左上角，图标 Y 是中心；减去半高让文本与图标垂直居中 */
    compo_textbox_set_location(txt, GENERAL_SB_LEFT_MARGIN,
                               (s16)(GENERAL_SB_Y - 9), 0, 0);
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
                                   (s16)(GENERAL_SB_Y - 7), 0, 0);
        compo_textbox_set_autosize(txt, true);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_forecolor(txt, COLOR_BLACK);
        compo_textbox_set(txt, page_name);
        bar->txt_pagename = txt;
    } else {
        bar->txt_pagename = NULL;
    }

    /* 蓝牙图标 */
    pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_setid(pic, GENERAL_SB_ID_PIC_BT);
    compo_picturebox_set_pos(pic, bt_x, GENERAL_SB_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);
    bar->pic_bt = pic;

    /* 电量图标 */
    pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_setid(pic, GENERAL_SB_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, bat_x, GENERAL_SB_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);
    bar->pic_bat = pic;

    bar->last_min = 0xff;
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
    tm_t tm;
    char buf[16];

    if (bar == NULL || bar->txt_time == NULL) {
        return;
    }

    tm = rtc_clock_get();

    /* 仅分钟变化时更新 textbox */
    if (tm.min == bar->last_min) {
        return;
    }
    bar->last_min = tm.min;

    general_sb_time_fmt(buf, sizeof(buf), &tm);
    compo_textbox_set(bar->txt_time, buf);
}
