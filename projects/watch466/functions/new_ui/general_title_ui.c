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

/* ---- 公开 API ---- */

void general_title_bar_create(compo_form_t *frm, general_title_bar_t *bar, const char *title)
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

    /* 页面标题 — 左上角（替代时间）
     * 必须用 FONT_TEST_14：与 general_status_bar 的 page_name 一致。
     * FONT_TEST_18 主要为 HH:MM 数字；对中文标题（设置/版本信息）渲染会卡住，
     * 表现为日志已切页但屏幕不刷新。 */
    if (title != NULL) {
        txt = compo_textbox_create(frm, 32);
        compo_setid(txt, GENERAL_TB_ID_TXT_TITLE);
        compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_TEST_14_BIN);
        /* textbox Y 是左上角，图标 Y 是中心；减去半高让文本与图标垂直居中 */
        compo_textbox_set_location(txt, GENERAL_TB_LEFT_MARGIN,
                                   (s16)(GENERAL_TB_Y - 7), 0, 0);
        compo_textbox_set_autosize(txt, true);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_forecolor(txt, COLOR_BLACK);
        compo_textbox_set(txt, title);
        bar->txt_title = txt;
    } else {
        bar->txt_title = NULL;
    }

    /* 蓝牙图标 */
    pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_setid(pic, GENERAL_TB_ID_PIC_BT);
    compo_picturebox_set_pos(pic, bt_x, GENERAL_TB_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);
    bar->pic_bt = pic;

    /* 电量图标 */
    pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_setid(pic, GENERAL_TB_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, bat_x, GENERAL_TB_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);
    bar->pic_bat = pic;
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
    bar->txt_title = (compo_textbox_t *)compo_getobj_byid(GENERAL_TB_ID_TXT_TITLE);
    bar->pic_bt    = (compo_picturebox_t *)compo_getobj_byid(GENERAL_TB_ID_PIC_BT);
    bar->pic_bat   = (compo_picturebox_t *)compo_getobj_byid(GENERAL_TB_ID_PIC_BAT);

    home_ui_shared_status_init();
    if (bar->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
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

    if (bar->pic_bt != NULL
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
