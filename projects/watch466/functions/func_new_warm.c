#include "include.h"
#include "func.h"
#include "new_heat_res.h"
#include "new_heat_point_util.h"
#include "new_home_icon_res.h"
#include "home_top_time_txt.h"
#include "home_ui_shared.h"
#include "home_ui_gpu_detach.h"
#include "home_ui_ram.h"
#include "func_lunchbox_uart.h"
#include "func_lunchbox_uart_internal.h"
#include "heat_display_reg.h"
#include "func_key_lock.h"

#if ELUNCHBOX_PANEL_EN
#include "func_heat_panel.h"
extern volatile u8 elunchbox_te_block_flag;
#endif

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

#ifndef UI_BUF_NEW_UI_NEW_PROGRESS_BG_BIN
#error "Run tools/gen_new_heat_icons.py then Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_0FONT_FONT_TEST_BIN
#error "UI_BUF_0FONT_FONT_TEST_BIN missing: add font_test.bin to ui.bin then Output/bin/prebuild.bat"
#endif
#ifndef UI_BUF_NEW_UI_NEW_SHOW_BIN
#error "Missing new_show.bin in ui/new_ui"
#endif
#define NEW_WARM_FONT                       UI_BUF_0FONT_FONT_TEST_BIN

/*
 * 保温页 — 效果图 WARM
 *   顶栏：蓝牙 + 电量 + 标题 WARM
 *   弧形进度条 + 圆点（复用 new_progress_* / new_point）
 *   弧内：累计保温时长；弧下：Total Warm Time
 *   194°F 持续保温至低电（lunchbox_keep_warm_apply）
 *   电源键：停止保温并回 Home
 */
#define NEW_WARM_STATUS_Y                 20
#define NEW_WARM_STATUS_RIGHT_MARGIN      10
#define NEW_WARM_STATUS_GAP               6
#define NEW_WARM_TITLE_Y                  25
#define NEW_WARM_TITLE_W                  120
#define NEW_WARM_TITLE_H                  36

#define NEW_WARM_TIME_Y                   115
#define NEW_WARM_TIME_LBL_Y               140

#define NEW_WARM_SHOW_Y                   212
#define NEW_WARM_SHOW_VAL_Y               200
#define NEW_WARM_SHOW_LBL_Y               220
#define NEW_WARM_SHOW_VAL_W               160
#define NEW_WARM_SHOW_LBL_W               200
#define NEW_WARM_WARM_TEMP_F              194

#define NEW_WARM_COLOR_TITLE              COLOR_BLACK
#define NEW_WARM_COLOR_VALUE              0x2BF4
#define NEW_WARM_COLOR_LABEL              0x0AD8

#define NEW_WARM_ARC_CYCLE_MIN            480   /* 8h 满圈，之后循环 */

#define NEW_WARM_MSG_POWER                (KEY_RIGHT | KEY_SHORT_UP)

enum {
    COMPO_ID_SHAPE_BG = 1,
    COMPO_ID_TXT_TOP_TIME,
    COMPO_ID_TXT_TITLE,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_BAT,
    COMPO_ID_PIC_PROGRESS_BG,
    COMPO_ID_PIC_PROGRESS,
    COMPO_ID_PIC_POINT,
    COMPO_ID_PIC_SHOW,
    COMPO_ID_TXT_ELAPSED,
    COMPO_ID_TXT_ELAPSED_LBL,
    COMPO_ID_TXT_SHOW_TEMP,
    COMPO_ID_TXT_SHOW_TEMP_LBL,
};

typedef struct {
    u32 start_tick;
    bool heating;
    bool display_pending;
    u8 last_top_min;
    u8 last_top_sec;
#if ELUNCHBOX_PANEL_EN
    bool key_ready;
#endif
    home_top_time_txt_t top_time;
    u8 last_progress_idx;
    u32 last_elapsed_min;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_progress_bg;
    compo_picturebox_t *pic_progress;
    compo_picturebox_t *pic_point;
    compo_picturebox_t *pic_show;
    compo_textbox_t *txt_title;
    compo_textbox_t *txt_elapsed;
    compo_textbox_t *txt_elapsed_lbl;
    compo_textbox_t *txt_show_temp;
    compo_textbox_t *txt_show_temp_lbl;
} f_new_warm_t;

static bool new_warm_show_ready;

#if ELUNCHBOX_PANEL_EN
static void new_warm_font_apply_once(f_new_warm_t *f);
#endif

static const u16 tbl_warm_progress_w[NEW_HEAT_PROGRESS_CNT] = {
    NEW_HEAT_NEW_PROGRESS_1_W, NEW_HEAT_NEW_PROGRESS_2_W, NEW_HEAT_NEW_PROGRESS_3_W,
    NEW_HEAT_NEW_PROGRESS_4_W, NEW_HEAT_NEW_PROGRESS_5_W, NEW_HEAT_NEW_PROGRESS_6_W,
    NEW_HEAT_NEW_PROGRESS_7_W, NEW_HEAT_NEW_PROGRESS_8_W, NEW_HEAT_NEW_PROGRESS_9_W,
    NEW_HEAT_NEW_PROGRESS_10_W, NEW_HEAT_NEW_PROGRESS_11_W, NEW_HEAT_NEW_PROGRESS_12_W,
    NEW_HEAT_NEW_PROGRESS_13_W,
};

static const u16 tbl_warm_progress_h[NEW_HEAT_PROGRESS_CNT] = {
    NEW_HEAT_NEW_PROGRESS_1_H, NEW_HEAT_NEW_PROGRESS_2_H, NEW_HEAT_NEW_PROGRESS_3_H,
    NEW_HEAT_NEW_PROGRESS_4_H, NEW_HEAT_NEW_PROGRESS_5_H, NEW_HEAT_NEW_PROGRESS_6_H,
    NEW_HEAT_NEW_PROGRESS_7_H, NEW_HEAT_NEW_PROGRESS_8_H, NEW_HEAT_NEW_PROGRESS_9_H,
    NEW_HEAT_NEW_PROGRESS_10_H, NEW_HEAT_NEW_PROGRESS_11_H, NEW_HEAT_NEW_PROGRESS_12_H,
    NEW_HEAT_NEW_PROGRESS_13_H,
};

static const s16 tbl_warm_progress_ax[NEW_HEAT_PROGRESS_CNT] = {
    NEW_HEAT_NEW_PROGRESS_1_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_2_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_3_ANCHOR_X,
    NEW_HEAT_NEW_PROGRESS_4_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_5_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_6_ANCHOR_X,
    NEW_HEAT_NEW_PROGRESS_7_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_8_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_9_ANCHOR_X,
    NEW_HEAT_NEW_PROGRESS_10_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_11_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_12_ANCHOR_X,
    NEW_HEAT_NEW_PROGRESS_13_ANCHOR_X,
};

static const s16 tbl_warm_progress_ay[NEW_HEAT_PROGRESS_CNT] = {
    NEW_HEAT_NEW_PROGRESS_1_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_2_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_3_ANCHOR_Y,
    NEW_HEAT_NEW_PROGRESS_4_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_5_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_6_ANCHOR_Y,
    NEW_HEAT_NEW_PROGRESS_7_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_8_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_9_ANCHOR_Y,
    NEW_HEAT_NEW_PROGRESS_10_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_11_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_12_ANCHOR_Y,
    NEW_HEAT_NEW_PROGRESS_13_ANCHOR_Y,
};

static const s16 tbl_warm_progress_tip_x[NEW_HEAT_PROGRESS_CNT] = {
    NEW_HEAT_NEW_PROGRESS_1_TIP_X, NEW_HEAT_NEW_PROGRESS_2_TIP_X, NEW_HEAT_NEW_PROGRESS_3_TIP_X,
    NEW_HEAT_NEW_PROGRESS_4_TIP_X, NEW_HEAT_NEW_PROGRESS_5_TIP_X, NEW_HEAT_NEW_PROGRESS_6_TIP_X,
    NEW_HEAT_NEW_PROGRESS_7_TIP_X, NEW_HEAT_NEW_PROGRESS_8_TIP_X, NEW_HEAT_NEW_PROGRESS_9_TIP_X,
    NEW_HEAT_NEW_PROGRESS_10_TIP_X, NEW_HEAT_NEW_PROGRESS_11_TIP_X, NEW_HEAT_NEW_PROGRESS_12_TIP_X,
    NEW_HEAT_NEW_PROGRESS_13_TIP_X,
};

static const s16 tbl_warm_progress_tip_y[NEW_HEAT_PROGRESS_CNT] = {
    NEW_HEAT_NEW_PROGRESS_1_TIP_Y, NEW_HEAT_NEW_PROGRESS_2_TIP_Y, NEW_HEAT_NEW_PROGRESS_3_TIP_Y,
    NEW_HEAT_NEW_PROGRESS_4_TIP_Y, NEW_HEAT_NEW_PROGRESS_5_TIP_Y, NEW_HEAT_NEW_PROGRESS_6_TIP_Y,
    NEW_HEAT_NEW_PROGRESS_7_TIP_Y, NEW_HEAT_NEW_PROGRESS_8_TIP_Y, NEW_HEAT_NEW_PROGRESS_9_TIP_Y,
    NEW_HEAT_NEW_PROGRESS_10_TIP_Y, NEW_HEAT_NEW_PROGRESS_11_TIP_Y, NEW_HEAT_NEW_PROGRESS_12_TIP_Y,
    NEW_HEAT_NEW_PROGRESS_13_TIP_Y,
};

#define NEW_WARM_ARC_RAM          ((u8 *)home_ui_shared_icon_runtime)
#define NEW_WARM_ARC_RAM_CAP      ((u32)(HOME_UI_SHARED_TAB_CNT * NEW_HOME_TAB_RAM_SIZE))

static bool new_warm_gpu_ram_bind(compo_picturebox_t *pic, u32 addr, u32 len,
                                    u8 *ram, u32 cap, u16 w, u16 h, s16 x, s16 y)
{
    u16 need;

    if (pic == NULL || addr == 0 || len == 0 || w == 0 || h == 0 || ram == NULL) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }
    if (len > cap) {
        compo_picturebox_set_visible(pic, false);
        return false;
    }
    home_gpu_wait_idle();
    os_spiflash_read(ram, addr, len);
    WDT_CLR();
    if (!gui_set_ram_check(ram, __func__)) {
        compo_picturebox_set_visible(pic, false);
        return false;
    }
    need = (u16)(8 + (u32)GET_LE16(&ram[4]) * GET_LE16(&ram[6]) * 2);
    if (need > len || need > cap) {
        compo_picturebox_set_visible(pic, false);
        return false;
    }
    home_gpu_wait_idle();
    compo_picturebox_set_ram(pic, ram);
    compo_picturebox_set_size(pic, w, h);
    compo_picturebox_set_pos(pic, x, y);
    compo_picturebox_set_visible(pic, true);
    return true;
}

static bool new_warm_gpu_ram_bind_existing(compo_picturebox_t *pic, u8 *ram,
                                             u16 w, u16 h, s16 x, s16 y)
{
    if (pic == NULL || ram == NULL || w == 0 || h == 0) {
        return false;
    }
    if (!gui_set_ram_check(ram, __func__)) {
        return false;
    }
    compo_picturebox_set_ram(pic, ram);
    compo_picturebox_set_size(pic, w, h);
    compo_picturebox_set_pos(pic, x, y);
    compo_picturebox_set_visible(pic, true);
    return true;
}

static u32 new_warm_progress_len(u8 idx)
{
    static const u32 tbl[NEW_HEAT_PROGRESS_CNT] = {
        UI_LEN_NEW_UI_NEW_PROGRESS_1_BIN, UI_LEN_NEW_UI_NEW_PROGRESS_2_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_3_BIN, UI_LEN_NEW_UI_NEW_PROGRESS_4_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_5_BIN, UI_LEN_NEW_UI_NEW_PROGRESS_6_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_7_BIN, UI_LEN_NEW_UI_NEW_PROGRESS_8_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_9_BIN, UI_LEN_NEW_UI_NEW_PROGRESS_10_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_11_BIN, UI_LEN_NEW_UI_NEW_PROGRESS_12_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_13_BIN,
    };

    if (idx == 0 || idx > NEW_HEAT_PROGRESS_CNT) {
        return tbl[0];
    }
    return tbl[idx - 1];
}

static u32 new_warm_progress_addr(u8 idx)
{
    static const u32 tbl[NEW_HEAT_PROGRESS_CNT] = {
        UI_BUF_NEW_UI_NEW_PROGRESS_1_BIN, UI_BUF_NEW_UI_NEW_PROGRESS_2_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_3_BIN, UI_BUF_NEW_UI_NEW_PROGRESS_4_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_5_BIN, UI_BUF_NEW_UI_NEW_PROGRESS_6_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_7_BIN, UI_BUF_NEW_UI_NEW_PROGRESS_8_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_9_BIN, UI_BUF_NEW_UI_NEW_PROGRESS_10_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_11_BIN, UI_BUF_NEW_UI_NEW_PROGRESS_12_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_13_BIN,
    };

    if (idx == 0 || idx > NEW_HEAT_PROGRESS_CNT) {
        return tbl[0];
    }
    return tbl[idx - 1];
}

static u8 new_warm_clamp_progress_idx(u8 idx)
{
    if (idx < 1) {
        return 1;
    }
    if (idx > NEW_HEAT_PROGRESS_CNT) {
        return NEW_HEAT_PROGRESS_CNT;
    }
    return idx;
}

static u32 new_warm_elapsed_min(f_new_warm_t *f)
{
    u32 ms;

    if (f == NULL || !f->heating || f->start_tick == 0) {
        return 0;
    }
    ms = tick_get() - f->start_tick;
    return ms / 60000;
}

static u8 new_warm_calc_progress_idx(u32 elapsed_min)
{
    u32 phase;

    if (elapsed_min == 0) {
        return 1;
    }
    phase = elapsed_min % NEW_WARM_ARC_CYCLE_MIN;
    if (phase == 0) {
        return NEW_HEAT_PROGRESS_CNT;
    }
    return (u8)(1 + (phase * (NEW_HEAT_PROGRESS_CNT - 1)) / NEW_WARM_ARC_CYCLE_MIN);
}

static void new_warm_format_elapsed(char *buf, u32 total_min)
{
    u8 hour = (u8)(total_min / 60);
    u8 min = (u8)(total_min % 60);

    if (hour > 0 && min > 0) {
        sprintf(buf, "%uH %umin", hour, min);
    } else if (hour > 0) {
        sprintf(buf, "%uH", hour);
    } else {
        sprintf(buf, "%umin", min);
    }
}

static compo_picturebox_t *new_warm_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);

    pic->img = (void *)widget_image_create(frm->page_body, 0);
    pic->radix = 1;
    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

static compo_textbox_t *new_warm_txt_create(compo_form_t *frm, u16 id, u32 font_addr,
                                            s16 x, s16 y, u16 color, bool center)
{
    compo_textbox_t *txt = compo_textbox_create(frm, 24);

    compo_setid(txt, id);
    compo_textbox_set_wholewrap(txt, false);
#if ELUNCHBOX_PANEL_EN
    /* 禁用 autosize → 后续 compo_textbox_set() 不读字体 Flash，避免 C281 */
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_visible(txt, false);
#else
    compo_textbox_set_font(txt, font_addr ? font_addr : NEW_WARM_FONT);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, center);
#endif
    compo_textbox_set_pos(txt, x, y);
    compo_textbox_set_forecolor(txt, color);
    return txt;
}

static void new_warm_bg_create(compo_form_t *frm)
{
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, COMPO_ID_SHAPE_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_radius(bg, 0);
}

static void new_warm_title_txt_show(compo_textbox_t *txt)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL) {
        return;
    }
    widget = txt->txt;
    compo_textbox_set_align_center(txt, true);
    widget_set_align_center(widget, true);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, NEW_WARM_TITLE_Y,
                               NEW_WARM_TITLE_W, NEW_WARM_TITLE_H);
    compo_textbox_set_forecolor(txt, NEW_WARM_COLOR_TITLE);
    compo_textbox_set(txt, "WARM");
    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }

    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
    compo_textbox_set_visible(txt, true);
}

static void new_warm_point_bind(f_new_warm_t *f, u8 progress_idx)
{
    u8 step;
    s16 px;
    s16 py;
    new_heat_point_bg_t bg;

    if (f == NULL || f->pic_point == NULL) {
        return;
    }
    step = new_warm_clamp_progress_idx(progress_idx) - 1;
    px = tbl_warm_progress_tip_x[step];
    py = tbl_warm_progress_tip_y[step];

    memset(&bg, 0, sizeof(bg));
    if (gui_set_ram_check(home_ui_heat_bg_ram, __func__)) {
        bg.bg_ram = home_ui_heat_bg_ram;
        bg.bg_ram_len = NEW_HEAT_NEW_PROGRESS_BG_RAM_SIZE;
        bg.bg_w = GET_LE16(&home_ui_heat_bg_ram[4]);
        bg.bg_h = GET_LE16(&home_ui_heat_bg_ram[6]);
        bg.bg_anchor_x = NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_X;
        bg.bg_anchor_y = NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_Y;
    }

    (void)new_heat_point_gpu_ram_bind(f->pic_point, home_ui_colon_ram, HOME_COLON_RAM_SIZE,
                                      NEW_HEAT_POINT_W, NEW_HEAT_POINT_H, px, py, &bg);
}

static void new_warm_show_apply(f_new_warm_t *f)
{
    if (f == NULL || f->pic_show == NULL || new_warm_show_ready) {
        return;
    }
    if (!new_warm_gpu_ram_bind(f->pic_show, UI_BUF_NEW_UI_NEW_SHOW_BIN,
                               UI_LEN_NEW_UI_NEW_SHOW_BIN,
                               home_ui_show_ram, sizeof(home_ui_show_ram),
                               NEW_HEAT_NEW_SHOW_W, NEW_HEAT_NEW_SHOW_H,
                               GUI_SCREEN_CENTER_X, NEW_WARM_SHOW_Y)) {
        return;
    }
    new_warm_show_ready = true;
}

static void new_warm_show_text_apply(f_new_warm_t *f)
{
    char buf[16];

    if (f == NULL) {
        return;
    }

#if ELUNCHBOX_PANEL_EN
    new_warm_font_apply_once(f);
#endif

    sprintf(buf, "%uF", (unsigned)NEW_WARM_WARM_TEMP_F);
    if (f->txt_show_temp != NULL) {
        compo_textbox_set_align_center(f->txt_show_temp, true);
        compo_textbox_set(f->txt_show_temp, buf);
        compo_textbox_set_visible(f->txt_show_temp, true);
    }
    if (f->txt_show_temp_lbl != NULL) {
        compo_textbox_set_align_center(f->txt_show_temp_lbl, true);
        compo_textbox_set(f->txt_show_temp_lbl, "Heating Temp");
        widget_text_set_client(f->txt_show_temp_lbl->txt, 0, 7);
        compo_textbox_set_visible(f->txt_show_temp_lbl, true);
    }
}

static void new_warm_track_apply(f_new_warm_t *f)
{
    if (f == NULL || f->pic_progress_bg == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (func_heat_panel_track_ram_valid()) {
        if (new_warm_gpu_ram_bind_existing(f->pic_progress_bg, home_ui_heat_bg_ram,
                                           NEW_HEAT_NEW_PROGRESS_BG_W, NEW_HEAT_NEW_PROGRESS_BG_H,
                                           NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_X,
                                           NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_Y)) {
            func_heat_panel_track_ram_consume();
            return;
        }
    }
#endif
    (void)new_warm_gpu_ram_bind(f->pic_progress_bg, UI_BUF_NEW_UI_NEW_PROGRESS_BG_BIN,
                                UI_LEN_NEW_UI_NEW_PROGRESS_BG_BIN,
                                home_ui_heat_bg_ram, sizeof(home_ui_heat_bg_ram),
                                NEW_HEAT_NEW_PROGRESS_BG_W, NEW_HEAT_NEW_PROGRESS_BG_H,
                                NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_X,
                                NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_Y);
}

static void new_warm_progress_overlay_apply(f_new_warm_t *f, u8 idx)
{
    u8 step;
    u16 pw;
    u16 ph;

    if (f == NULL || f->pic_progress == NULL) {
        return;
    }
    idx = new_warm_clamp_progress_idx(idx);
    step = idx - 1;
    pw = tbl_warm_progress_w[step];
    ph = tbl_warm_progress_h[step];
    if (pw == 0 || ph == 0) {
        compo_picturebox_set_visible(f->pic_progress, false);
        return;
    }
    {
        u32 addr = new_warm_progress_addr(idx);
        u32 len = new_warm_progress_len(idx);

        if (len <= NEW_WARM_ARC_RAM_CAP &&
            new_warm_gpu_ram_bind(f->pic_progress, addr, len,
                                  NEW_WARM_ARC_RAM, NEW_WARM_ARC_RAM_CAP,
                                  pw, ph, tbl_warm_progress_ax[step], tbl_warm_progress_ay[step])) {
            return;
        }
        home_ui_pic_set_flash(f->pic_progress, addr, pw, ph);
        compo_picturebox_set_size(f->pic_progress, pw, ph);
        compo_picturebox_set_pos(f->pic_progress, tbl_warm_progress_ax[step], tbl_warm_progress_ay[step]);
        compo_picturebox_set_visible(f->pic_progress, true);
    }
}

static void new_warm_progress_apply(f_new_warm_t *f, u8 idx)
{
    if (f == NULL || idx == 0) {
        return;
    }
    if (idx == f->last_progress_idx) {
        return;
    }
    new_warm_track_apply(f);
    new_warm_progress_overlay_apply(f, idx);
    new_warm_point_bind(f, idx);
    f->last_progress_idx = idx;
}

#if ELUNCHBOX_PANEL_EN
static bool new_warm_font_ready;

static void new_warm_font_bind_txt(compo_textbox_t *txt)
{
    if (txt != NULL) {
        compo_textbox_set_font(txt, NEW_WARM_FONT);
    }
}

static void new_warm_font_apply_once(f_new_warm_t *f)
{
    if (new_warm_font_ready || f == NULL) {
        return;
    }

    WDT_CLR();
    new_warm_font_bind_txt(f->txt_title);
    new_warm_font_bind_txt(f->txt_elapsed);
    new_warm_font_bind_txt(f->txt_elapsed_lbl);
    new_warm_font_bind_txt(f->txt_show_temp);
    new_warm_font_bind_txt(f->txt_show_temp_lbl);
    new_warm_font_ready = true;

    /* 方式 3：覆盖标签为 14px / 12px 字体 */
    compo_textbox_set_font(f->txt_elapsed_lbl, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_font(f->txt_show_temp_lbl, UI_BUF_0FONT_FONT_TEST_12_BIN);
}
#endif

static void new_warm_text_apply(f_new_warm_t *f)
{
    char buf[32];
    u32 elapsed_min;

    if (f == NULL) {
        return;
    }

#if ELUNCHBOX_PANEL_EN
    new_warm_font_apply_once(f);
#endif

    elapsed_min = new_warm_elapsed_min(f);
    new_warm_format_elapsed(buf, elapsed_min);

    if (f->txt_elapsed != NULL) {
        compo_textbox_set(f->txt_elapsed, buf);
        compo_textbox_set_visible(f->txt_elapsed, true);
    }
    if (f->txt_elapsed_lbl != NULL) {
        compo_textbox_set(f->txt_elapsed_lbl, "Total Warm Time");
        widget_text_set_client(f->txt_elapsed_lbl->txt, 0, 8);
        compo_textbox_set_visible(f->txt_elapsed_lbl, true);
    }
    f->last_elapsed_min = elapsed_min;
}

static void new_warm_status_refresh(f_new_warm_t *f)
{
    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (f->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        home_ui_shared_status_refresh_bt(f->pic_bt);
    }
    if (f->pic_bat != NULL) {
        home_ui_shared_battery_attach_pic(f->pic_bat);
    }
#else
    home_ui_shared_status_init();
    if (f->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f->pic_bt, NEW_HOME_BT_W, NEW_HOME_BT_H);
        compo_picturebox_set_visible(f->pic_bt, true);
    }
    home_ui_shared_status_bind_bat(f->pic_bat);
#endif
}

static void new_warm_bind_objects(f_new_warm_t *f)
{
    if (f == NULL) {
        return;
    }
    home_top_time_txt_bind(&f->top_time, COMPO_ID_TXT_TOP_TIME);
    f->txt_title = compo_getobj_byid(COMPO_ID_TXT_TITLE);
    f->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);
    f->pic_progress_bg = compo_getobj_byid(COMPO_ID_PIC_PROGRESS_BG);
    f->pic_progress = compo_getobj_byid(COMPO_ID_PIC_PROGRESS);
    f->pic_point = compo_getobj_byid(COMPO_ID_PIC_POINT);
    f->pic_show = compo_getobj_byid(COMPO_ID_PIC_SHOW);
    f->txt_elapsed = compo_getobj_byid(COMPO_ID_TXT_ELAPSED);
    f->txt_elapsed_lbl = compo_getobj_byid(COMPO_ID_TXT_ELAPSED_LBL);
    f->txt_show_temp = compo_getobj_byid(COMPO_ID_TXT_SHOW_TEMP);
    f->txt_show_temp_lbl = compo_getobj_byid(COMPO_ID_TXT_SHOW_TEMP_LBL);
}

static void new_warm_heating_start(f_new_warm_t *f)
{
    if (f == NULL || f->heating) {
        return;
    }
#if FUNC_LUNCHBOX_UART_EN
#if ELUNCHBOX_PANEL_EN
    if (home_ui_shared_battery_is_charging()) {
        lunchbox_warm_mark_active();
    } else
#endif
    {
        /* func_elunchbox_enter_warm_common_prep 已下发保温指令时跳过重复发送 */
        if (lb_keep_warm_msg_flag == 0) {
            lunchbox_keep_warm_apply();
        }
    }
#endif
    f->heating = true;
    f->start_tick = tick_get();
    f->last_progress_idx = 0xff;
    f->last_elapsed_min = 0xffffffff;
    printf("new_warm: heating start 194F\n");
}

static void new_warm_heating_stop(void)
{
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_keep_warm_stop();
#endif
}

#if ELUNCHBOX_PANEL_EN
/** @brief 已在保温页时 BLE 再次下发保温参数：重启 UART 保温任务 */
void func_new_warm_ble_restart(void)
{
    f_new_warm_t *f;

    if (func_cb.sta != FUNC_NEW_WARM || func_cb.f_cb == NULL) {
        return;
    }
    f = (f_new_warm_t *)func_cb.f_cb;
    new_warm_heating_stop();
    f->heating = false;
    f->start_tick = tick_get();
    f->last_progress_idx = 0xff;
    f->last_elapsed_min = 0;
#if LB_BRIDGE_MODE
    lb_heat_uart_remote_set(true);
#endif
    new_warm_heating_start(f);
    printf("new_warm_ble_restart: ok\n");
}
#endif

static void new_warm_ui_apply_visual(f_new_warm_t *f)
{
    u32 elapsed_min;
    u8 progress_idx;

    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    new_warm_font_apply_once(f);
#endif
    home_top_time_txt_force(&f->top_time, &f->last_top_min, &f->last_top_sec);
    new_warm_title_txt_show(f->txt_title);
    new_warm_status_refresh(f);

    elapsed_min = new_warm_elapsed_min(f);
    progress_idx = new_warm_calc_progress_idx(elapsed_min);
    new_warm_progress_apply(f, progress_idx);
    new_warm_show_apply(f);
    new_warm_show_text_apply(f);

    if (elapsed_min != f->last_elapsed_min) {
        new_warm_text_apply(f);
    }
}

static void new_warm_ui_apply(f_new_warm_t *f)
{
    if (f == NULL) {
        return;
    }
    new_warm_ui_apply_visual(f);

    if (!f->heating) {
        new_warm_heating_start(f);
    }
}

static void new_warm_gpu_detach_before_leave(f_new_warm_t *f)
{
    if (f == NULL) {
        return;
    }
    WDT_CLR();
    home_ui_gpu_pic_detach_light(f->pic_bt);
    home_ui_gpu_pic_detach_light(f->pic_bat);
    home_ui_gpu_pic_detach_light(f->pic_progress_bg);
    home_ui_gpu_pic_detach_light(f->pic_progress);
    home_ui_gpu_pic_detach_light(f->pic_point);
    home_ui_gpu_pic_detach_light(f->pic_show);
    home_ui_shared_battery_detach_pic();
    WDT_CLR();
}

static void new_warm_power_key(void)
{
    f_new_warm_t *f = (f_new_warm_t *)func_cb.f_cb;

    if (sys_cb.flag_swithing) {
        return;
    }
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_keep_warm_stop_user();
#endif
    if (f != NULL) {
        f->heating = false;
    }
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

compo_form_t *func_new_warm_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    compo_picturebox_t *pic;
    s16 bat_x;
    s16 bt_x;

    new_warm_bg_create(frm);

    home_top_time_txt_create(frm, COMPO_ID_TXT_TOP_TIME);

    txt = new_warm_txt_create(frm, COMPO_ID_TXT_TITLE, NEW_WARM_FONT,
                              GUI_SCREEN_CENTER_X, NEW_WARM_TITLE_Y,
                              NEW_WARM_COLOR_TITLE, true);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, NEW_WARM_TITLE_Y,
                               NEW_WARM_TITLE_W, NEW_WARM_TITLE_H);

    bat_x = (s16)(GUI_SCREEN_WIDTH - NEW_WARM_STATUS_RIGHT_MARGIN - NEW_HOME_BAT_W / 2);
    bt_x = (s16)(bat_x - NEW_HOME_BAT_W / 2 - NEW_WARM_STATUS_GAP - NEW_HOME_BT_W / 2);

    pic = new_warm_pic_create_hidden(frm, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, bt_x, NEW_WARM_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);

    pic = new_warm_pic_create_hidden(frm, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, bat_x, NEW_WARM_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);

    (void)new_warm_pic_create_hidden(frm, COMPO_ID_PIC_PROGRESS_BG);
    (void)new_warm_pic_create_hidden(frm, COMPO_ID_PIC_PROGRESS);
    (void)new_warm_pic_create_hidden(frm, COMPO_ID_PIC_POINT);

    pic = new_warm_pic_create_hidden(frm, COMPO_ID_PIC_SHOW);
    compo_picturebox_set_size(pic, NEW_HEAT_NEW_SHOW_W, NEW_HEAT_NEW_SHOW_H);

    txt = new_warm_txt_create(frm, COMPO_ID_TXT_ELAPSED, NEW_WARM_FONT,
                               GUI_SCREEN_CENTER_X, NEW_WARM_TIME_Y,
                               NEW_WARM_COLOR_VALUE, true);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, NEW_WARM_TIME_Y, 220, 36);
    txt = new_warm_txt_create(frm, COMPO_ID_TXT_ELAPSED_LBL, NEW_WARM_FONT,
                               GUI_SCREEN_CENTER_X, NEW_WARM_TIME_LBL_Y,
                               NEW_WARM_COLOR_LABEL, true);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, NEW_WARM_TIME_LBL_Y, 280, 40);

    txt = new_warm_txt_create(frm, COMPO_ID_TXT_SHOW_TEMP, NEW_WARM_FONT,
                              GUI_SCREEN_CENTER_X, NEW_WARM_SHOW_VAL_Y,
                              NEW_WARM_COLOR_VALUE, true);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, NEW_WARM_SHOW_VAL_Y,
                               NEW_WARM_SHOW_VAL_W, 28);
    txt = new_warm_txt_create(frm, COMPO_ID_TXT_SHOW_TEMP_LBL, NEW_WARM_FONT,
                              GUI_SCREEN_CENTER_X, NEW_WARM_SHOW_LBL_Y,
                              NEW_WARM_COLOR_LABEL, true);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, NEW_WARM_SHOW_LBL_Y,
                               NEW_WARM_SHOW_LBL_W, 50);

    return frm;
}

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static void new_warm_pt8028_keys_process(f_new_warm_t *f)
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
    if (press_tch <= PT8028_KEY_TCH6 && press_tch != PT8028_KEY_TCH4) {
        elunchbox_user_activity_reset();
    }
    if (press_tch == PT8028_KEY_TCH5) {
        new_warm_power_key();
    } else if (press_tch == PT8028_KEY_TCH1) {
        /* 加热键：跳转到加热设置页 */
        if (!sys_cb.flag_swithing) {
            func_switch_to(FUNC_NEW_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
    } else if (press_tch == PT8028_KEY_TCH7) {
        /* 预约键：跳转到预约设置页 */
        if (!sys_cb.flag_swithing) {
            func_switch_to(FUNC_RESERVATION, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
    }
}
#endif

static void func_new_warm_message(size_msg_t msg)
{
    f_new_warm_t *f = (f_new_warm_t *)func_cb.f_cb;

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
    switch (msg) {
    case NEW_WARM_MSG_POWER:
        new_warm_power_key();
        break;
    default:
        func_message(msg);
        break;
    }
}

static void func_new_warm_process(void)
{
    f_new_warm_t *f = (f_new_warm_t *)func_cb.f_cb;
    u32 elapsed_min;
    u8 progress_idx;

    if (f == NULL) {
        func_process();
        return;
    }

#if ELUNCHBOX_PANEL_EN
    if (!f->key_ready) {
        if (f->display_pending) {
            /* 首帧: 先 TE block 阻止新帧，等 GPU（快速），再绑进度条/指针 */
            {
                u8 was_blocked = elunchbox_te_block_flag;
                if (!was_blocked) {
                    elunchbox_te_block_flag = 1;
                }
                home_gpu_wait_idle();
                WDT_CLR();
                new_warm_ui_apply(f);
                if (!was_blocked) {
                    elunchbox_te_block_flag = 0;
                }
            }
            f->display_pending = false;
        }
        func_process();
        func_home_drain_stale_key_msgs();
        pt8028_release_clear();
        (void)pt8028_take_press_tch();
        f->key_ready = true;
        return;
    }
#endif

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_key_scan_page();
    new_warm_pt8028_keys_process(f);
#endif

    if (f->display_pending) {
        new_warm_ui_apply(f);
        f->display_pending = false;
    }

    elapsed_min = new_warm_elapsed_min(f);
    progress_idx = new_warm_calc_progress_idx(elapsed_min);
    new_warm_progress_apply(f, progress_idx);
    if (elapsed_min != f->last_elapsed_min) {
        new_warm_text_apply(f);
    }

    home_top_time_txt_tick(&f->top_time, &f->last_top_min, &f->last_top_sec);

    func_process();
}

void func_new_warm_enter(void)
{
    f_new_warm_t *f;

    printf("func_new_warm_enter\n");
    heat_display_warm_exit_reset();
    new_warm_show_ready = false;

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(1);
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif

#if ELUNCHBOX_PANEL_EN
    WDT_CLR();
    if (!elunchbox_te_block_flag) {
        elunchbox_te_block_flag = 1;
    }
#endif

    msg_queue_detach(NEW_WARM_MSG_POWER, 0);

    func_cb.f_cb = func_zalloc(sizeof(f_new_warm_t));
    f = (f_new_warm_t *)func_cb.f_cb;
    f->last_progress_idx = 0xff;
    f->last_elapsed_min = 0xffffffff;
    f->last_top_min = 0xff;
    f->last_top_sec = 0xff;

#if ELUNCHBOX_PANEL_EN
    new_warm_font_ready = false;
#endif

    func_cb.frm_main = func_new_warm_form_create();
    new_warm_bind_objects(f);

#if ELUNCHBOX_PANEL_EN
    if (func_cb.last != FUNC_HEAT) {
        home_ui_digit_pool_reset();
    }
    home_ui_shared_status_init();
    WDT_CLR();

    /* enter 内一次性完成 UI，首帧即完整显示保温页 */
    new_warm_ui_apply_visual(f);
    f->display_pending = false;
    f->key_ready = true;

    printf("nw_e gpu_wait\n");
    home_gpu_wait_idle();
    WDT_CLR();
    elunchbox_te_block_flag = 0;
    tft_bglight_force_on();
    new_warm_heating_start(f);
    printf("func_new_warm_enter: ok\n");
#else
    new_warm_ui_apply(f);
    f->display_pending = false;
#endif
}

void func_new_warm_exit(void)
{
    f_new_warm_t *f = (f_new_warm_t *)func_cb.f_cb;

    new_warm_heating_stop();
    new_warm_show_ready = false;
#if ELUNCHBOX_PANEL_EN
    if (f != NULL) {
        new_warm_gpu_detach_before_leave(f);
    } else {
        home_ui_shared_battery_detach_pic();
    }
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(0);
    pt8028_release_clear();
#endif
    printf("func_new_warm_exit: ok1111111\n");
    func_cb.last = FUNC_NEW_WARM;
    printf("func_new_warm_exit\n");
}

void func_new_warm(void)
{
    printf("func_new_warm run\n");
    func_new_warm_enter();
    printf("FUNC_NEW_WARM%d\n", FUNC_NEW_WARM);
    printf("func_cb.sta%d\n", func_cb.sta);
    while (func_cb.sta == FUNC_NEW_WARM) {
        // printf("func_new_warm_process111\n");
        func_new_warm_process();
        // printf("func_new_warm_message2222\n");
        func_new_warm_message(msg_dequeue());
    }
    // func_new_warm_exit();
}
