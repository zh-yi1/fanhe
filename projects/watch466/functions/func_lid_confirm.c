#include "include.h"
#include "func.h"
#include "func_lid_confirm.h"
#include "new_time_res.h"
#include "new_home_icon_res.h"
#include "home_top_time_txt.h"
#include "home_ui_shared.h"
#include "home_ui_gpu_detach.h"
#include "func_key_lock.h"
#include "func_lunchbox_uart.h"

#if ELUNCHBOX_PANEL_EN
extern volatile u8 elunchbox_te_block_flag;
#endif

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

#ifndef UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN
#error "Run tools/gen_new_time_icons.py then Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_0FONT_FONT_TEST_BIN
#error "UI_BUF_0FONT_FONT_TEST_BIN missing: add font_test.bin to ui.bin then Output/bin/prebuild.bat"
#endif
#ifndef UI_BUF_0FONT_FONT_TEST_10_BIN
#error "UI_BUF_0FONT_FONT_TEST_10_BIN missing: add font_test_10.bin to ui.bin then Output/bin/prebuild.bat"
#endif
#define LID_CONFIRM_FONT                    UI_BUF_0FONT_FONT_TEST_BIN
#define LID_CONFIRM_MSG_FONT                UI_BUF_0FONT_FONT_TEST_10_BIN

#if ((NEW_TIME_NEW_GRAY_BJ2_RAM_SIZE + NEW_TIME_NEW_BLUE_BJ2_RAM_SIZE) > NEW_HOME_TAB_RAM_SIZE)
#error "lid confirm btn icons exceed home_ui_shared_icon_runtime slot"
#endif

/*
 * 上盖开启确认弹窗（上电开机弹出）
 *   白半透明遮罩 + 白色圆角对话框
 *   NO/YES：new_gray_bj2 / new_blue_bj2
 */
#define LID_CONFIRM_STATUS_Y                20
#define LID_CONFIRM_STATUS_RIGHT_MARGIN     10
#define LID_CONFIRM_STATUS_GAP              6

#define LID_CONFIRM_DIM_ALPHA               160

#define LID_CONFIRM_PANEL_W                 280
#define LID_CONFIRM_PANEL_H                 160
#define LID_CONFIRM_PANEL_Y                 GUI_SCREEN_CENTER_Y
#define LID_CONFIRM_PANEL_RADIUS            16

#define LID_CONFIRM_MSG_W                   240
#define LID_CONFIRM_MSG_H                   48
#define LID_CONFIRM_MSG_Y                   ((s16)(LID_CONFIRM_PANEL_Y - 38))

#define LID_CONFIRM_SUB_W                   240
#define LID_CONFIRM_SUB_H                   28
#define LID_CONFIRM_SUB_Y                   ((s16)(LID_CONFIRM_PANEL_Y + 10))

#define LID_CONFIRM_BTN_CENTER_DIST         130
#define LID_CONFIRM_BTN_Y                   ((s16)(LID_CONFIRM_PANEL_Y + LID_CONFIRM_PANEL_H / 2 - NEW_TIME_BTN_H / 2 - 14))
#define LID_CONFIRM_BTN_NO_X                ((s16)(GUI_SCREEN_CENTER_X - LID_CONFIRM_BTN_CENTER_DIST / 2))
#define LID_CONFIRM_BTN_YES_X               ((s16)(GUI_SCREEN_CENTER_X + LID_CONFIRM_BTN_CENTER_DIST / 2))

#define LID_CONFIRM_STATUS_BAT_X            (GUI_SCREEN_WIDTH - LID_CONFIRM_STATUS_RIGHT_MARGIN - NEW_HOME_BAT_W / 2)
#define LID_CONFIRM_STATUS_BT_X             (LID_CONFIRM_STATUS_BAT_X - NEW_HOME_BAT_W / 2 - LID_CONFIRM_STATUS_GAP - NEW_HOME_BT_W / 2)

#define LID_CONFIRM_COLOR_MSG               0x0AD8
#define LID_CONFIRM_COLOR_SUB               0x9CD3
#define LID_CONFIRM_COLOR_ON                COLOR_WHITE
#define LID_CONFIRM_COLOR_OFF               COLOR_BLACK

#define LID_CONFIRM_MSG_OK                  KU_BACK
#define LID_CONFIRM_MSG_PLUS                KU_VOL_UP
#define LID_CONFIRM_MSG_MINUS               KU_VOL_DOWN
#define LID_CONFIRM_MSG_POWER               (KEY_RIGHT | KEY_SHORT_UP)

enum {
    COMPO_ID_SHAPE_BG = 1,
    COMPO_ID_SHAPE_DIM,
    COMPO_ID_SHAPE_PANEL,
    COMPO_ID_TXT_TOP_TIME,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_BAT,
    COMPO_ID_TXT_MSG,
    COMPO_ID_TXT_SUB,
    COMPO_ID_PIC_NO_BG,
    COMPO_ID_PIC_YES_BG,
    COMPO_ID_TXT_NO,
    COMPO_ID_TXT_YES,
};

enum {
    LID_CONFIRM_SEL_NO = 0,
    LID_CONFIRM_SEL_YES,
};

enum {
    LID_CONFIRM_LOAD_STATUS = 0,
    LID_CONFIRM_LOAD_BTNS,
    LID_CONFIRM_LOAD_TEXT,
    LID_CONFIRM_LOAD_DONE,
};

typedef struct {
    u8 sel;
    u8 last_top_min;
    u8 last_top_sec;
    bool display_pending;
#if ELUNCHBOX_PANEL_EN
    u8 load_stage;
    bool key_ready;
#endif
    home_top_time_txt_t top_time;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_no_bg;
    compo_picturebox_t *pic_yes_bg;
    compo_textbox_t *txt_msg;
    compo_textbox_t *txt_sub;
    compo_textbox_t *txt_no;
    compo_textbox_t *txt_yes;
} f_lid_confirm_t;

#if ELUNCHBOX_PANEL_EN
#define LID_CONFIRM_NO_BTN_RAM              (home_ui_shared_icon_runtime[2])
#define LID_CONFIRM_YES_BTN_RAM             (home_ui_shared_icon_runtime[2] + NEW_TIME_NEW_GRAY_BJ2_RAM_SIZE)

static bool lid_confirm_font_ready;

static void lid_confirm_font_bind_txt(compo_textbox_t *txt)
{
    if (txt != NULL) {
        compo_textbox_set_font(txt, LID_CONFIRM_FONT);
    }
}

static void lid_confirm_font_apply_once(f_lid_confirm_t *f)
{
    if (lid_confirm_font_ready || f == NULL) {
        return;
    }
    WDT_CLR();
    if (f->txt_msg != NULL) {
        compo_textbox_set_font(f->txt_msg, LID_CONFIRM_MSG_FONT);
    }
    if (f->txt_sub != NULL) {
        compo_textbox_set_font(f->txt_sub, LID_CONFIRM_MSG_FONT);
    }
    lid_confirm_font_bind_txt(f->txt_no);
    lid_confirm_font_bind_txt(f->txt_yes);
    lid_confirm_font_ready = true;
}

static bool lid_confirm_gpu_ram_bind(u8 *ram, u16 buf_size, u32 addr, u16 len,
                                     compo_picturebox_t *pic, u16 w, u16 h, s16 x, s16 y)
{
    u16 need;

    if (pic == NULL || addr == 0 || len == 0 || ram == NULL || len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }
    WDT_CLR();
    os_spiflash_read(ram, addr, len);
    if (!gui_set_ram_check(ram, __func__)) {
        compo_picturebox_set_visible(pic, false);
        return false;
    }
    need = (u16)(8 + (u32)GET_LE16(&ram[4]) * GET_LE16(&ram[6]) * 2);
    if (need > len || need > buf_size) {
        compo_picturebox_set_visible(pic, false);
        return false;
    }
    compo_picturebox_set_ram(pic, ram);
    compo_picturebox_set_size(pic, w, h);
    compo_picturebox_set_pos(pic, x, y);
    compo_picturebox_set_visible(pic, true);
    return true;
}
#endif

static compo_picturebox_t *lid_confirm_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);

    if (pic == NULL) {
        return NULL;
    }
    pic->img = (void *)widget_image_create(frm->page_body, 0);
    pic->radix = 1;
    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

static compo_textbox_t *lid_confirm_txt_create(compo_form_t *frm, u16 id, s16 x, s16 y,
                                               u16 color, bool center)
{
    compo_textbox_t *txt = compo_textbox_create(frm, 48);

    compo_setid(txt, id);
    compo_textbox_set_wholewrap(txt, false);
#if ELUNCHBOX_PANEL_EN
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_align_center(txt, center);
    compo_textbox_set_visible(txt, false);
#else
    compo_textbox_set_font(txt, LID_CONFIRM_FONT);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, center);
#endif
    compo_textbox_set_pos(txt, x, y);
    compo_textbox_set_forecolor(txt, color);
    return txt;
}

static void lid_confirm_bg_create(compo_form_t *frm)
{
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, COMPO_ID_SHAPE_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(bg, 0xEF5D);
    compo_shape_set_radius(bg, 0);
}

static void lid_confirm_dim_create(compo_form_t *frm)
{
    compo_shape_t *dim = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(dim, COMPO_ID_SHAPE_DIM);
    compo_shape_set_location(dim, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(dim, COLOR_WHITE);
    compo_shape_set_radius(dim, 0);
    compo_shape_set_alpha(dim, LID_CONFIRM_DIM_ALPHA);
}

static void lid_confirm_panel_create(compo_form_t *frm)
{
    compo_shape_t *panel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(panel, COMPO_ID_SHAPE_PANEL);
    compo_shape_set_location(panel, GUI_SCREEN_CENTER_X, LID_CONFIRM_PANEL_Y,
                             LID_CONFIRM_PANEL_W, LID_CONFIRM_PANEL_H);
    compo_shape_set_color(panel, COLOR_WHITE);
    compo_shape_set_radius(panel, LID_CONFIRM_PANEL_RADIUS);
}

static void lid_confirm_status_refresh(f_lid_confirm_t *f)
{
    if (f == NULL) {
        return;
    }
    home_top_time_txt_force(&f->top_time, &f->last_top_min, &f->last_top_sec);
    home_ui_shared_status_init();
    if (f->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_pos(f->pic_bt, LID_CONFIRM_STATUS_BT_X, LID_CONFIRM_STATUS_Y);
        home_ui_shared_status_refresh_bt(f->pic_bt);
    }
    if (f->pic_bat != NULL) {
        compo_picturebox_set_pos(f->pic_bat, LID_CONFIRM_STATUS_BAT_X, LID_CONFIRM_STATUS_Y);
        home_ui_shared_battery_attach_pic(f->pic_bat);
    }
}

static void lid_confirm_btns_apply(f_lid_confirm_t *f)
{
    u32 no_addr;
    u32 yes_addr;
    u16 no_len;
    u16 yes_len;

    if (f == NULL) {
        return;
    }
    if (f->sel == LID_CONFIRM_SEL_NO) {
        no_addr = UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN;
        no_len = UI_LEN_NEW_UI_NEW_BLUE_BJ2_BIN;
        yes_addr = UI_BUF_NEW_UI_NEW_GRAY_BJ2_BIN;
        yes_len = UI_LEN_NEW_UI_NEW_GRAY_BJ2_BIN;
    } else {
        no_addr = UI_BUF_NEW_UI_NEW_GRAY_BJ2_BIN;
        no_len = UI_LEN_NEW_UI_NEW_GRAY_BJ2_BIN;
        yes_addr = UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN;
        yes_len = UI_LEN_NEW_UI_NEW_BLUE_BJ2_BIN;
    }
#if ELUNCHBOX_PANEL_EN
    (void)lid_confirm_gpu_ram_bind(LID_CONFIRM_NO_BTN_RAM, NEW_TIME_NEW_GRAY_BJ2_RAM_SIZE,
                                   no_addr, no_len, f->pic_no_bg,
                                   NEW_TIME_BTN_W, NEW_TIME_BTN_H,
                                   LID_CONFIRM_BTN_NO_X, LID_CONFIRM_BTN_Y);
    (void)lid_confirm_gpu_ram_bind(LID_CONFIRM_YES_BTN_RAM,
                                   (u16)(NEW_HOME_TAB_RAM_SIZE - NEW_TIME_NEW_GRAY_BJ2_RAM_SIZE),
                                   yes_addr, yes_len, f->pic_yes_bg,
                                   NEW_TIME_BTN_W, NEW_TIME_BTN_H,
                                   LID_CONFIRM_BTN_YES_X, LID_CONFIRM_BTN_Y);
#else
    (void)no_addr;
    (void)yes_addr;
    (void)no_len;
    (void)yes_len;
#endif
}

static void lid_confirm_btn_label_show(compo_textbox_t *txt, s16 cx, s16 cy,
                                       const char *label, u16 color)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL) {
        return;
    }
    widget = txt->txt;
    compo_textbox_set_location(txt, cx, cy, NEW_TIME_BTN_W, NEW_TIME_BTN_H);
    compo_textbox_set_align_center(txt, true);
    if (widget != NULL) {
        widget_set_align_center(widget, true);
        widget_text_set_ellipsis(widget, false);
    }
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set(txt, label);
    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
    compo_textbox_set_visible(txt, true);
    if (widget != NULL) {
        widget_set_top(widget, true);
    }
}

static void lid_confirm_msg_show(compo_textbox_t *txt)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL) {
        return;
    }
    widget = txt->txt;
    if (widget == NULL) {
        return;
    }
    compo_textbox_set_multiline(txt, true);
    compo_textbox_set_align_center(txt, true);
    if (widget != NULL) {
        widget_set_align_center(widget, true);
        widget_text_set_ellipsis(widget, false);
    }
    compo_textbox_set_wholewrap(txt, true);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, LID_CONFIRM_MSG_Y,
                               LID_CONFIRM_MSG_W, LID_CONFIRM_MSG_H);
    compo_textbox_set_font(txt, LID_CONFIRM_MSG_FONT);
    compo_textbox_set_forecolor(txt, LID_CONFIRM_COLOR_MSG);
    compo_textbox_set(txt, "The lid has been detected to be open.");
    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
    compo_textbox_set_visible(txt, true);
}

static void lid_confirm_sub_show(compo_textbox_t *txt)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL) {
        return;
    }
    widget = txt->txt;
    if (widget == NULL) {
        return;
    }
    compo_textbox_set_align_center(txt, true);
    widget_set_align_center(widget, true);
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, LID_CONFIRM_SUB_Y,
                               LID_CONFIRM_SUB_W, LID_CONFIRM_SUB_H);
    compo_textbox_set_font(txt, LID_CONFIRM_MSG_FONT);
    compo_textbox_set_forecolor(txt, LID_CONFIRM_COLOR_SUB);
    compo_textbox_set(txt, "Should we continue heating?");
    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
    compo_textbox_set_visible(txt, true);
}

static void lid_confirm_text_apply(f_lid_confirm_t *f)
{
    u16 no_color;
    u16 yes_color;

    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    lid_confirm_font_apply_once(f);
#endif
    lid_confirm_msg_show(f->txt_msg);
    lid_confirm_sub_show(f->txt_sub);
    if (f->sel == LID_CONFIRM_SEL_NO) {
        no_color = LID_CONFIRM_COLOR_ON;
        yes_color = LID_CONFIRM_COLOR_OFF;
    } else {
        no_color = LID_CONFIRM_COLOR_OFF;
        yes_color = LID_CONFIRM_COLOR_ON;
    }
    lid_confirm_btn_label_show(f->txt_no, LID_CONFIRM_BTN_NO_X, LID_CONFIRM_BTN_Y,
                               "NO", no_color);
    lid_confirm_btn_label_show(f->txt_yes, LID_CONFIRM_BTN_YES_X, LID_CONFIRM_BTN_Y,
                               "YES", yes_color);
}

static void lid_confirm_ui_apply(f_lid_confirm_t *f)
{
    if (f == NULL) {
        return;
    }
    lid_confirm_status_refresh(f);
    lid_confirm_btns_apply(f);
    lid_confirm_text_apply(f);
}

static void lid_confirm_bind_objects(f_lid_confirm_t *f)
{
    if (f == NULL) {
        return;
    }
    home_top_time_txt_bind(&f->top_time, COMPO_ID_TXT_TOP_TIME);
    f->pic_bt = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_BT);
    f->pic_bat = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_BAT);
    f->pic_no_bg = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_NO_BG);
    f->pic_yes_bg = (compo_picturebox_t *)compo_getobj_byid(COMPO_ID_PIC_YES_BG);
    f->txt_msg = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_MSG);
    f->txt_sub = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_SUB);
    f->txt_no = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_NO);
    f->txt_yes = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_TXT_YES);
}

static void lid_confirm_finish(f_lid_confirm_t *f)
{
    u8 mcu_mode;

    if (f == NULL || sys_cb.flag_swithing) {
        return;
    }
    elunchbox_lid_confirm_disarm();
    if (f->sel == LID_CONFIRM_SEL_YES) {
        mcu_mode = lunchbox_get_heat_mode();
        lb_heat_mcu_nav_set(true);
        lb_heat_uart_remote_set(true);
        if (mcu_mode == 5) {
            printf("lid_confirm: YES MCU mode=5 -> warm panel\n");
#if ELUNCHBOX_PANEL_EN
            lunchbox_warm_mark_active();
#endif
            func_elunchbox_switch_to_warm_panel();
        } else {
            printf("lid_confirm: YES MCU mode=%u -> heat panel\n", mcu_mode);
            lb_heat_autostart_set(true);
            func_elunchbox_switch_to_heat_panel();
        }
    } else {
        printf("lid_confirm: NO send heat_stop -> home\n");
        /* 用户主动停止：强制下发 DP10=0，避免 mcu_nav/remote 跳过 UART */
        lb_heat_mcu_nav_set(false);
        lb_heat_user_uart_tx_force_set(true);
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_heat_stop();
#endif
        func_elunchbox_uart_stop_and_home();
        if (func_cb.sta != FUNC_HOME && !sys_cb.flag_swithing) {
            func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
    }
}

static void lid_confirm_toggle_sel(f_lid_confirm_t *f)
{
    if (f == NULL) {
        return;
    }
    f->sel = (u8)((f->sel == LID_CONFIRM_SEL_NO)
                  ? LID_CONFIRM_SEL_YES : LID_CONFIRM_SEL_NO);
    f->display_pending = true;
}

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static void lid_confirm_pt8028_keys_process(f_lid_confirm_t *f)
{
    u8 press_tch;

    if (f == NULL || !f->key_ready) {
        return;
    }
    if (func_key_lock_press_take_guarded(&press_tch)) {
        return;
    }
    if (press_tch == 0xff) {
        return;
    }
    if (press_tch == PT8028_KEY_TCH2 || press_tch == PT8028_KEY_TCH6) {
        lid_confirm_toggle_sel(f);
    } else if (press_tch == PT8028_KEY_TCH4) {
        lid_confirm_finish(f);
    }
    /* 开关键(TCH5)：盖确认页不响应返回/切页 */
}

static void lid_confirm_keys_poll(f_lid_confirm_t *f)
{
    if (f == NULL || !f->key_ready) {
        return;
    }
    pt8028_key_scan_page();
    lid_confirm_pt8028_keys_process(f);
}
#endif

static void func_lid_confirm_message(size_msg_t msg)
{
    f_lid_confirm_t *f = (f_lid_confirm_t *)func_cb.f_cb;

    if (sys_cb.flag_swithing) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (f != NULL && !f->key_ready) {
        return;
    }
#endif
    if (func_key_lock_ku_blocked(msg)) {
        return;
    }
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    switch (msg) {
    case KU_MODE:
    case KU_BACK:
    case KU_VOL_UP:
    case KU_VOL_DOWN:
    case KEY_RIGHT | KEY_SHORT_UP:
        return;
    default:
        break;
    }
#endif
    switch (msg) {
    case LID_CONFIRM_MSG_OK:
        lid_confirm_finish(f);
        break;
    case LID_CONFIRM_MSG_PLUS:
    case LID_CONFIRM_MSG_MINUS:
        lid_confirm_toggle_sel(f);
        break;
    case LID_CONFIRM_MSG_POWER:
        /* 开关键不可退出本页，须选 NO/YES 确认 */
        break;
    default:
        func_message(msg);
        break;
    }
}

static void func_lid_confirm_process(void)
{
    f_lid_confirm_t *f = (f_lid_confirm_t *)func_cb.f_cb;

    if (f == NULL) {
        func_process();
        return;
    }

#if ELUNCHBOX_PANEL_EN
    if (!f->key_ready) {
        WDT_CLR();
        switch (f->load_stage) {
        case LID_CONFIRM_LOAD_STATUS:
            lid_confirm_status_refresh(f);
            f->load_stage = LID_CONFIRM_LOAD_BTNS;
            break;
        case LID_CONFIRM_LOAD_BTNS:
            lid_confirm_btns_apply(f);
            f->load_stage = LID_CONFIRM_LOAD_TEXT;
            break;
        case LID_CONFIRM_LOAD_TEXT:
            lid_confirm_text_apply(f);
            f->load_stage = LID_CONFIRM_LOAD_DONE;
            f->key_ready = true;
            break;
        default:
            f->key_ready = true;
            break;
        }
        func_process();
#if USER_PT8028_KEY
        func_home_drain_stale_key_msgs();
        pt8028_release_clear();
        (void)pt8028_take_press_tch();
#endif
        return;
    }
#endif

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    if (elunchbox_ui_is_live()) {
        lid_confirm_keys_poll(f);
    }
#endif
    if (f->display_pending) {
        lid_confirm_ui_apply(f);
        f->display_pending = false;
    }

#if ELUNCHBOX_PANEL_EN
    if (elunchbox_ui_is_live()) {
        lid_confirm_status_refresh(f);
    }
#endif
    func_process();
}

compo_form_t *func_lid_confirm_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    compo_picturebox_t *pic;

    lid_confirm_bg_create(frm);
    home_top_time_txt_create(frm, COMPO_ID_TXT_TOP_TIME);

    pic = lid_confirm_pic_create_hidden(frm, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, LID_CONFIRM_STATUS_BT_X, LID_CONFIRM_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);

    pic = lid_confirm_pic_create_hidden(frm, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, LID_CONFIRM_STATUS_BAT_X, LID_CONFIRM_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);

    lid_confirm_dim_create(frm);
    lid_confirm_panel_create(frm);

    txt = lid_confirm_txt_create(frm, COMPO_ID_TXT_MSG,
                                 GUI_SCREEN_CENTER_X, LID_CONFIRM_MSG_Y,
                                 LID_CONFIRM_COLOR_MSG, true);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, LID_CONFIRM_MSG_Y,
                               LID_CONFIRM_MSG_W, LID_CONFIRM_MSG_H);

    txt = lid_confirm_txt_create(frm, COMPO_ID_TXT_SUB,
                                 GUI_SCREEN_CENTER_X, LID_CONFIRM_SUB_Y,
                                 LID_CONFIRM_COLOR_SUB, true);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, LID_CONFIRM_SUB_Y,
                               LID_CONFIRM_SUB_W, LID_CONFIRM_SUB_H);

    (void)lid_confirm_pic_create_hidden(frm, COMPO_ID_PIC_NO_BG);
    (void)lid_confirm_pic_create_hidden(frm, COMPO_ID_PIC_YES_BG);

    txt = lid_confirm_txt_create(frm, COMPO_ID_TXT_NO,
                                 LID_CONFIRM_BTN_NO_X, LID_CONFIRM_BTN_Y,
                                 LID_CONFIRM_COLOR_OFF, true);
    compo_textbox_set_location(txt, LID_CONFIRM_BTN_NO_X, LID_CONFIRM_BTN_Y,
                               NEW_TIME_BTN_W, NEW_TIME_BTN_H);

    txt = lid_confirm_txt_create(frm, COMPO_ID_TXT_YES,
                                 LID_CONFIRM_BTN_YES_X, LID_CONFIRM_BTN_Y,
                                 LID_CONFIRM_COLOR_ON, true);
    compo_textbox_set_location(txt, LID_CONFIRM_BTN_YES_X, LID_CONFIRM_BTN_Y,
                               NEW_TIME_BTN_W, NEW_TIME_BTN_H);

    return frm;
}

void func_lid_confirm_enter(void)
{
    f_lid_confirm_t *f;

    printf("func_lid_confirm_enter\n");

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(1);
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif

#if ELUNCHBOX_PANEL_EN
    WDT_CLR();
#endif

    msg_queue_detach(KU_BACK, 0);
    msg_queue_detach(KU_MODE, 0);
    msg_queue_detach(KU_VOL_UP, 0);
    msg_queue_detach(KU_VOL_DOWN, 0);
    msg_queue_detach(KU_RIGHT, 0);

    func_cb.f_cb = func_zalloc(sizeof(f_lid_confirm_t));
    f = (f_lid_confirm_t *)func_cb.f_cb;
    f->sel = LID_CONFIRM_SEL_YES;
#if ELUNCHBOX_PANEL_EN
    f->load_stage = LID_CONFIRM_LOAD_STATUS;
    f->key_ready = false;
    lid_confirm_font_ready = false;
#endif
    f->display_pending = false;
    f->last_top_min = 0xff;
    f->last_top_sec = 0xff;

    func_cb.frm_main = func_lid_confirm_form_create();
    lid_confirm_bind_objects(f);

#if ELUNCHBOX_PANEL_EN
    home_ui_shared_status_init();
    WDT_CLR();
    home_gpu_wait_idle();
#endif
}

void func_lid_confirm_exit(void)
{
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(0);
#endif
#if ELUNCHBOX_PANEL_EN
    lid_confirm_font_ready = false;
#endif
    func_cb.last = FUNC_LID_CONFIRM;
    printf("func_lid_confirm_exit\n");
}

void func_lid_confirm(void)
{
    func_lid_confirm_enter();
    while (func_cb.sta == FUNC_LID_CONFIRM) {
        func_lid_confirm_process();
        func_lid_confirm_message(msg_dequeue());
    }
    func_lid_confirm_exit();
}

#if ELUNCHBOX_PANEL_EN
static bool s_lid_confirm_armed;

void elunchbox_lid_confirm_arm_boot(void)
{
    s_lid_confirm_armed = true;
    printf("lid_confirm: armed for power-on MCU heat check\n");
}

void elunchbox_lid_confirm_disarm(void)
{
    if (s_lid_confirm_armed) {
        printf("lid_confirm: disarmed\n");
    }
    s_lid_confirm_armed = false;
}

bool elunchbox_lid_confirm_is_armed(void)
{
    return s_lid_confirm_armed;
}

void func_elunchbox_switch_to_lid_confirm(void)
{
    if (func_cb.sta == FUNC_LID_CONFIRM) {
        return;
    }
    if (sys_cb.flag_swithing) {
        return;
    }
#if USER_PT8028_KEY
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif
    s_lid_confirm_armed = false;
    home_gpu_wait_idle();
    WDT_CLR();
    printf("lid_confirm: show dialog (MCU heating on power-on)\n");
    func_switch_to(FUNC_LID_CONFIRM, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

bool elunchbox_lid_confirm_try_redirect(void)
{
    if (!s_lid_confirm_armed) {
        return false;
    }
    if (func_cb.sta == FUNC_LID_CONFIRM) {
        return true;
    }
    /* 已在加热页：无需弹窗 */
    if (func_cb.sta == FUNC_HEAT) {
        elunchbox_lid_confirm_disarm();
        return false;
    }
    func_elunchbox_switch_to_lid_confirm();
    return true;
}
#endif
