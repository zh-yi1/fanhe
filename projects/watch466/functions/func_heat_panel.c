#include "include.h"
#include "func.h"
#include "func_heat_panel.h"
#include "heat_display_reg.h"
#include "new_heat_res.h"
#include "new_heat_point_util.h"
#include "home_ui_shared.h"
#include "home_icon_res.h"
#include "new_home_icon_res.h"
#include "home_top_time_txt.h"
#include "home_ui_ram.h"
#include "home_ui_gpu_detach.h"

#if ELUNCHBOX_PANEL_EN
#include "func_key_lock.h"
extern volatile u8 elunchbox_te_block_flag;
#endif

#ifndef UI_BUF_NEW_UI_NEW_PROGRESS_BG_BIN
#error "Run tools/gen_new_heat_icons.py then Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_NEW_UI_NEW_PROGRESS_1_BIN
#error "Run tools/gen_new_heat_icons.py then Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_NEW_UI_NEW_POINT_BIN
#error "Missing new_point.bin in ui/new_ui"
#endif

#ifndef UI_BUF_NEW_UI_NEW_SHOW_BIN
#error "Missing new_show.bin in ui/new_ui"
#endif

#ifndef UI_BUF_0FONT_FONT_TEST_BIN
#error "UI_BUF_0FONT_FONT_TEST_BIN missing: add font_test.bin to ui.bin then Output/bin/prebuild.bat"
#endif
#define NEW_HEAT_FONT                       UI_BUF_0FONT_FONT_TEST_BIN

#define HEAT_PANEL_STATUS_Y             20
#define HEAT_PANEL_STATUS_RIGHT_MARGIN  10
#define HEAT_PANEL_STATUS_GAP           6

#define HEAT_PANEL_COLOR_VALUE          0x2BF4
#define HEAT_PANEL_COLOR_LABEL          0x0AD8

#define HEAT_PANEL_REMAIN_Y             100
#define HEAT_PANEL_REMAIN_LBL_Y         130
#define HEAT_PANEL_SHOW_Y               212
#define HEAT_PANEL_TEMP_X               72
#define HEAT_PANEL_DUR_X                233
#define HEAT_PANEL_VAL_Y                200
#define HEAT_PANEL_LBL_Y                220

#define HEAT_PANEL_ARC_CX               (GUI_SCREEN_WIDTH / 2)
#define HEAT_PANEL_ARC_CY               95
#define HEAT_PANEL_ARC_R                72

enum {
    HEAT_PANEL_ID_BG = 1,
    HEAT_PANEL_ID_TXT_TOP_TIME,
    HEAT_PANEL_ID_PROGRESS_BG,
    HEAT_PANEL_ID_PROGRESS,
    HEAT_PANEL_ID_POINT,
    HEAT_PANEL_ID_SHOW,
    HEAT_PANEL_ID_BT,
    HEAT_PANEL_ID_BAT,
    HEAT_PANEL_ID_TXT_REMAIN,
    HEAT_PANEL_ID_TXT_REMAIN_LBL,
    HEAT_PANEL_ID_TXT_TEMP,
    HEAT_PANEL_ID_TXT_TEMP_LBL,
    HEAT_PANEL_ID_TXT_DUR,
    HEAT_PANEL_ID_TXT_DUR_LBL,
};

typedef struct {
    home_top_time_txt_t top_time;
    u8 last_top_min;
    u8 last_top_sec;
    compo_picturebox_t *pic_progress_bg;
    compo_picturebox_t *pic_progress;
    compo_picturebox_t *pic_point;
    compo_picturebox_t *pic_show;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_bat;
    compo_textbox_t *txt_remain;
    compo_textbox_t *txt_remain_lbl;
    compo_textbox_t *txt_temp;
    compo_textbox_t *txt_temp_lbl;
    compo_textbox_t *txt_dur;
    compo_textbox_t *txt_dur_lbl;
    u8 last_progress_idx;
    u32 last_remain_min;
    bool text_pending;
    bool show_ready;
    bool track_ready;
    bool ui_ready;
    bool font_ready;
} heat_panel_ui_t;

static const u16 tbl_progress_w[NEW_HEAT_PROGRESS_CNT] = {
    NEW_HEAT_NEW_PROGRESS_1_W, NEW_HEAT_NEW_PROGRESS_2_W, NEW_HEAT_NEW_PROGRESS_3_W,
    NEW_HEAT_NEW_PROGRESS_4_W, NEW_HEAT_NEW_PROGRESS_5_W, NEW_HEAT_NEW_PROGRESS_6_W,
    NEW_HEAT_NEW_PROGRESS_7_W, NEW_HEAT_NEW_PROGRESS_8_W, NEW_HEAT_NEW_PROGRESS_9_W,
    NEW_HEAT_NEW_PROGRESS_10_W, NEW_HEAT_NEW_PROGRESS_11_W, NEW_HEAT_NEW_PROGRESS_12_W,
    NEW_HEAT_NEW_PROGRESS_13_W,
};

static const u16 tbl_progress_h[NEW_HEAT_PROGRESS_CNT] = {
    NEW_HEAT_NEW_PROGRESS_1_H, NEW_HEAT_NEW_PROGRESS_2_H, NEW_HEAT_NEW_PROGRESS_3_H,
    NEW_HEAT_NEW_PROGRESS_4_H, NEW_HEAT_NEW_PROGRESS_5_H, NEW_HEAT_NEW_PROGRESS_6_H,
    NEW_HEAT_NEW_PROGRESS_7_H, NEW_HEAT_NEW_PROGRESS_8_H, NEW_HEAT_NEW_PROGRESS_9_H,
    NEW_HEAT_NEW_PROGRESS_10_H, NEW_HEAT_NEW_PROGRESS_11_H, NEW_HEAT_NEW_PROGRESS_12_H,
    NEW_HEAT_NEW_PROGRESS_13_H,
};

static const s16 tbl_progress_ax[NEW_HEAT_PROGRESS_CNT] = {
    NEW_HEAT_NEW_PROGRESS_1_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_2_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_3_ANCHOR_X,
    NEW_HEAT_NEW_PROGRESS_4_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_5_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_6_ANCHOR_X,
    NEW_HEAT_NEW_PROGRESS_7_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_8_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_9_ANCHOR_X,
    NEW_HEAT_NEW_PROGRESS_10_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_11_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_12_ANCHOR_X,
    NEW_HEAT_NEW_PROGRESS_13_ANCHOR_X,
};

static const s16 tbl_progress_ay[NEW_HEAT_PROGRESS_CNT] = {
    NEW_HEAT_NEW_PROGRESS_1_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_2_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_3_ANCHOR_Y,
    NEW_HEAT_NEW_PROGRESS_4_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_5_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_6_ANCHOR_Y,
    NEW_HEAT_NEW_PROGRESS_7_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_8_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_9_ANCHOR_Y,
    NEW_HEAT_NEW_PROGRESS_10_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_11_ANCHOR_Y, NEW_HEAT_NEW_PROGRESS_12_ANCHOR_Y,
    NEW_HEAT_NEW_PROGRESS_13_ANCHOR_Y,
};

static const s16 tbl_progress_tip_x[NEW_HEAT_PROGRESS_CNT] = {
    NEW_HEAT_NEW_PROGRESS_1_TIP_X, NEW_HEAT_NEW_PROGRESS_2_TIP_X, NEW_HEAT_NEW_PROGRESS_3_TIP_X,
    NEW_HEAT_NEW_PROGRESS_4_TIP_X, NEW_HEAT_NEW_PROGRESS_5_TIP_X, NEW_HEAT_NEW_PROGRESS_6_TIP_X,
    NEW_HEAT_NEW_PROGRESS_7_TIP_X, NEW_HEAT_NEW_PROGRESS_8_TIP_X, NEW_HEAT_NEW_PROGRESS_9_TIP_X,
    NEW_HEAT_NEW_PROGRESS_10_TIP_X, NEW_HEAT_NEW_PROGRESS_11_TIP_X, NEW_HEAT_NEW_PROGRESS_12_TIP_X,
    NEW_HEAT_NEW_PROGRESS_13_TIP_X,
};

static const s16 tbl_progress_tip_y[NEW_HEAT_PROGRESS_CNT] = {
    NEW_HEAT_NEW_PROGRESS_1_TIP_Y, NEW_HEAT_NEW_PROGRESS_2_TIP_Y, NEW_HEAT_NEW_PROGRESS_3_TIP_Y,
    NEW_HEAT_NEW_PROGRESS_4_TIP_Y, NEW_HEAT_NEW_PROGRESS_5_TIP_Y, NEW_HEAT_NEW_PROGRESS_6_TIP_Y,
    NEW_HEAT_NEW_PROGRESS_7_TIP_Y, NEW_HEAT_NEW_PROGRESS_8_TIP_Y, NEW_HEAT_NEW_PROGRESS_9_TIP_Y,
    NEW_HEAT_NEW_PROGRESS_10_TIP_Y, NEW_HEAT_NEW_PROGRESS_11_TIP_Y, NEW_HEAT_NEW_PROGRESS_12_TIP_Y,
    NEW_HEAT_NEW_PROGRESS_13_TIP_Y,
};

static heat_panel_ui_t g_hp;
static struct f_heat_t_ *g_hp_f_heat;
static bool g_hp_track_ram_valid;
static bool g_hp_live_seen_positive;

/* 内存布局（单缓冲合成，无 Flash、无动态分配）：
 *   heat_bg → 灰轨 + 蓝弧 CPU 合成后一次 set_ram
 *   colon_ram → 圆点
 *   home_ui_show_ram → show 条（与保温页共用，互斥） */

#define HEAT_PANEL_OVERLAY_SKIP565      0xFFFF
#define HEAT_PANEL_OVERLAY_ROW_MAX      192
#define HEAT_PANEL_NEAR_FULL_IDX        11
#define HEAT_PANEL_LEFT_TIP_MAX_X       (NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_X - 84)

static bool heat_panel_is_progress_blue(u16 c)
{
    u16 r;
    u16 g;
    u16 b;

    if (c >= 0xFFFE) {
        return false;
    }
    r = (c >> 11) & 0x1F;
    g = (c >> 5) & 0x3F;
    b = c & 0x1F;
    return (b >= 10 && b > r && g < 42);
}

static u16 heat_panel_composite_color(u16 tw, u16 th, s16 ox, s16 oy, s16 sx, s16 sy)
{
    s16 ix;
    s16 iy;
    u32 di;

    ix = sx - ox;
    iy = sy - oy;
    if (ix < 0 || iy < 0 || ix >= (s16)tw || iy >= (s16)th) {
        return HEAT_PANEL_OVERLAY_SKIP565;
    }
    di = 8 + ((u32)iy * tw + (u32)ix) * 2;
    if (di + 1 >= NEW_HEAT_NEW_PROGRESS_BG_RAM_SIZE) {
        return HEAT_PANEL_OVERLAY_SKIP565;
    }
    return GET_LE16(&home_ui_heat_bg_ram[di]);
}

static bool heat_panel_is_blue_boundary(u16 tw, u16 th, s16 ox, s16 oy, s16 sx, s16 sy)
{
    static const s16 dx[4] = {1, -1, 0, 0};
    static const s16 dy[4] = {0, 0, 1, -1};
    u8 i;

    if (!heat_panel_is_progress_blue(heat_panel_composite_color(tw, th, ox, oy, sx, sy))) {
        return false;
    }
    for (i = 0; i < 4; i++) {
        if (!heat_panel_is_progress_blue(heat_panel_composite_color(tw, th, ox, oy,
                sx + dx[i], sy + dy[i]))) {
            return true;
        }
    }
    return false;
}

/* idx>=11 近满弧：圆点落在左侧蓝灰交界最下缘（倒计时 CCW 缩回端） */
static void heat_panel_point_tip_near_full(u8 idx, s16 *x, s16 *y)
{
    u16 tw;
    u16 th;
    s16 ox;
    s16 oy;
    s16 best_x;
    s16 best_y;
    bool found;
    u16 iy;
    u16 ix;

    if (idx < HEAT_PANEL_NEAR_FULL_IDX || !g_hp.track_ready) {
        return;
    }
    tw = GET_LE16(&home_ui_heat_bg_ram[4]);
    th = GET_LE16(&home_ui_heat_bg_ram[6]);
    if (tw == 0 || th == 0) {
        return;
    }
    ox = NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_X - (s16)(tw / 2);
    oy = NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_Y - (s16)(th / 2);
    best_x = *x;
    best_y = *y;
    found = false;

    for (iy = 0; iy < th; iy++) {
        for (ix = 0; ix < tw; ix++) {
            s16 sx = ox + (s16)ix;
            s16 sy = oy + (s16)iy;

            if (sx > HEAT_PANEL_LEFT_TIP_MAX_X) {
                continue;
            }
            if (!heat_panel_is_blue_boundary(tw, th, ox, oy, sx, sy)) {
                continue;
            }
            if (!found || sy > best_y || (sy == best_y && sx < best_x)) {
                found = true;
                best_x = sx;
                best_y = sy;
            }
        }
    }
    if (found) {
        *x = best_x;
        *y = best_y;
    }
}

static void heat_panel_blit_overlay(u8 *dst, u16 tw, u16 th,
                                    u32 overlay_addr, u16 ow, u16 oh,
                                    s16 overlay_ax, s16 overlay_ay,
                                    s16 bg_ax, s16 bg_ay)
{
    s16 dx0;
    s16 dy0;
    u16 y;
    u16 row_bytes;
    u8 row_buf[HEAT_PANEL_OVERLAY_ROW_MAX * 2];

    if (ow == 0 || oh == 0 || ow > HEAT_PANEL_OVERLAY_ROW_MAX) {
        return;
    }

    dx0 = (overlay_ax - (s16)(ow / 2)) - (bg_ax - (s16)(tw / 2));
    dy0 = (overlay_ay - (s16)(oh / 2)) - (bg_ay - (s16)(th / 2));
    row_bytes = (u16)(ow * 2);

    for (y = 0; y < oh; y++) {
        s16 dy = dy0 + (s16)y;
        u16 x;

        if (dy < 0 || dy >= (s16)th) {
            continue;
        }
        os_spiflash_read(row_buf, overlay_addr + 8 + (u32)y * ow * 2, row_bytes);
        for (x = 0; x < ow; x++) {
            s16 dx = dx0 + (s16)x;
            u16 c;
            u32 di;

            if (dx < 0 || dx >= (s16)tw) {
                continue;
            }
            c = (u16)row_buf[x * 2] | ((u16)row_buf[x * 2 + 1] << 8);
            if (c == HEAT_PANEL_OVERLAY_SKIP565) {
                continue;
            }
            di = 8 + ((u32)dy * tw + (u32)dx) * 2;
            if (di + 1 >= NEW_HEAT_NEW_PROGRESS_BG_RAM_SIZE) {
                continue;
            }
            dst[di] = row_buf[x * 2];
            dst[di + 1] = row_buf[x * 2 + 1];
        }
        WDT_CLR();
    }
}

/* 前向声明 */
static u8 heat_panel_clamp_progress_idx(u8 idx);
static u32 heat_panel_progress_addr(u8 idx);

static bool heat_panel_progress_composite_apply(u8 idx)
{
    u8 step;
    u16 tw;
    u16 th;
    u16 ow;
    u16 oh;

    if (g_hp.pic_progress_bg == NULL) {
        return false;
    }

    idx = heat_panel_clamp_progress_idx(idx);
    step = idx - 1;
    ow = tbl_progress_w[step];
    oh = tbl_progress_h[step];

    home_gpu_wait_idle();
    os_spiflash_read(home_ui_heat_bg_ram, UI_BUF_NEW_UI_NEW_PROGRESS_BG_BIN,
                     UI_LEN_NEW_UI_NEW_PROGRESS_BG_BIN);
    WDT_CLR();
    if (!gui_set_ram_check(home_ui_heat_bg_ram, __func__)) {
        return false;
    }
    tw = GET_LE16(&home_ui_heat_bg_ram[4]);
    th = GET_LE16(&home_ui_heat_bg_ram[6]);

    if (ow > 0 && oh > 0) {
        heat_panel_blit_overlay(home_ui_heat_bg_ram, tw, th,
                                heat_panel_progress_addr(idx), ow, oh,
                                tbl_progress_ax[step], tbl_progress_ay[step],
                                NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_X,
                                NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_Y);
    }

    home_gpu_wait_idle();
    compo_picturebox_set_ram(g_hp.pic_progress_bg, home_ui_heat_bg_ram);
    compo_picturebox_set_size(g_hp.pic_progress_bg, NEW_HEAT_NEW_PROGRESS_BG_W,
                              NEW_HEAT_NEW_PROGRESS_BG_H);
    compo_picturebox_set_pos(g_hp.pic_progress_bg,
                              NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_X,
                              NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_Y);
    compo_picturebox_set_visible(g_hp.pic_progress_bg, true);
    if (g_hp.pic_progress != NULL) {
        compo_picturebox_set_visible(g_hp.pic_progress, false);
        compo_picturebox_set_ram(g_hp.pic_progress, NULL);
    }
    g_hp.track_ready = true;
    g_hp_track_ram_valid = false;
    return true;
}

static void heat_panel_arc_detach(void)
{
    if (g_hp.pic_progress != NULL) {
        compo_picturebox_set_visible(g_hp.pic_progress, false);
        compo_picturebox_set_ram(g_hp.pic_progress, NULL);
    }
}

static bool heat_panel_gpu_ram_bind(compo_picturebox_t *pic, u32 addr, u32 len,
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
        printf("hp_bind: buf too small need=%u cap=%u\n", len, cap);
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

static const u16 tbl_heat_temp_preset[7] = {
    104, 122, 140, 158, 176, 194, 212,
};

static u32 heat_panel_progress_len(u8 idx)
{
    static const u32 tbl[NEW_HEAT_PROGRESS_CNT] = {
        UI_LEN_NEW_UI_NEW_PROGRESS_1_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_2_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_3_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_4_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_5_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_6_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_7_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_8_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_9_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_10_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_11_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_12_BIN,
        UI_LEN_NEW_UI_NEW_PROGRESS_13_BIN,
    };

    if (idx == 0 || idx > NEW_HEAT_PROGRESS_CNT) {
        return tbl[0];
    }
    return tbl[idx - 1];
}

static u32 heat_panel_progress_addr(u8 idx)
{
    static const u32 tbl[NEW_HEAT_PROGRESS_CNT] = {
        UI_BUF_NEW_UI_NEW_PROGRESS_1_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_2_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_3_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_4_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_5_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_6_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_7_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_8_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_9_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_10_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_11_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_12_BIN,
        UI_BUF_NEW_UI_NEW_PROGRESS_13_BIN,
    };

    if (idx == 0 || idx > NEW_HEAT_PROGRESS_CNT) {
        return tbl[0];
    }
    return tbl[idx - 1];
}

static compo_picturebox_t *heat_panel_pic_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);

    pic->img = (void *)widget_image_create(frm->page_body, 0);
    pic->radix = 1;
    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

static compo_textbox_t *heat_panel_txt(compo_form_t *frm, u16 id, s16 x, s16 y, u16 color, bool center)
{
    compo_textbox_t *txt = compo_textbox_create(frm, 24);

    compo_setid(txt, id);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_align_center(txt, center);
    compo_textbox_set_pos(txt, x, y);
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set_visible(txt, false);
    return txt;
}

static void heat_panel_white_bg(compo_form_t *frm)
{
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, HEAT_PANEL_ID_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_radius(bg, 0);
}

static u8 heat_panel_clamp_progress_idx(u8 idx)
{
    if (idx < 1) {
        return 1;
    }
    if (idx > NEW_HEAT_PROGRESS_CNT) {
        return NEW_HEAT_PROGRESS_CNT;
    }
    return idx;
}

static void heat_panel_point_pos(u8 idx, s16 *x, s16 *y)
{
    u8 tip_step;

    /* idx 与 TIP 表一一对应：idx=13→左端 TIP_13，idx=1→右端 TIP_1 */
    tip_step = heat_panel_clamp_progress_idx(idx) - 1;
    *x = tbl_progress_tip_x[tip_step];
    *y = tbl_progress_tip_y[tip_step];
}

static void heat_panel_point_bind(u8 progress_idx)
{
    s16 px;
    s16 py;
    new_heat_point_bg_t bg;

    if (g_hp.pic_point == NULL) {
        printf("point_bind: skip pic=NULL\n");
        return;
    }
    heat_panel_point_pos(progress_idx, &px, &py);
    heat_panel_point_tip_near_full(progress_idx, &px, &py);

    memset(&bg, 0, sizeof(bg));
    if (g_hp.track_ready && gui_set_ram_check(home_ui_heat_bg_ram, __func__)) {
        bg.bg_ram = home_ui_heat_bg_ram;
        bg.bg_ram_len = NEW_HEAT_NEW_PROGRESS_BG_RAM_SIZE;
        bg.bg_w = GET_LE16(&home_ui_heat_bg_ram[4]);
        bg.bg_h = GET_LE16(&home_ui_heat_bg_ram[6]);
        bg.bg_anchor_x = NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_X;
        bg.bg_anchor_y = NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_Y;
    }

    if (!new_heat_point_gpu_ram_bind(g_hp.pic_point, home_ui_colon_ram, HOME_COLON_RAM_SIZE,
                                     NEW_HEAT_POINT_W, NEW_HEAT_POINT_H, px, py, &bg)) {
        printf("point_bind: fail idx=%u tip=(%d,%d)\n", progress_idx, px, py);
        return;
    }
    if (g_hp.pic_point->img != NULL && !func_key_lock_hint_is_on()) {
        widget_set_top(g_hp.pic_point->img, true);
    }
}

static void heat_panel_progress_apply(u8 idx)
{
    if (g_hp.pic_progress_bg == NULL || idx == 0) {
        printf("progress_apply: skip bg=%p idx=%u\n", g_hp.pic_progress_bg, idx);
        return;
    }
    if (idx == g_hp.last_progress_idx) {
        return;
    }
    printf("progress_apply: composite idx=%u\n", idx);
    if (!heat_panel_progress_composite_apply(idx)) {
        printf("progress_apply: composite fail idx=%u\n", idx);
        return;
    }
    g_hp.last_progress_idx = idx;
    heat_panel_point_bind(idx);
    printf("progress_apply: done idx=%u\n", idx);
}

static void heat_panel_show_apply(void)
{
    if (g_hp.pic_show == NULL || g_hp.show_ready) {
        printf("show_apply: skip pic=%p ready=%d\n", g_hp.pic_show, g_hp.show_ready);
        return;
    }
    if (!heat_panel_gpu_ram_bind(g_hp.pic_show, UI_BUF_NEW_UI_NEW_SHOW_BIN,
                                  UI_LEN_NEW_UI_NEW_SHOW_BIN,
                                  home_ui_show_ram, sizeof(home_ui_show_ram),
                                  NEW_HEAT_NEW_SHOW_W, NEW_HEAT_NEW_SHOW_H,
                                  GUI_SCREEN_CENTER_X, HEAT_PANEL_SHOW_Y)) {
        printf("show_apply: fail\n");
        return;
    }
    g_hp.show_ready = true;
}

static u16 heat_panel_target_temp_f(u8 temp_idx)
{
    if (temp_idx >= 7) {
        return tbl_heat_temp_preset[0];
    }
    return tbl_heat_temp_preset[temp_idx];
}

static u8 heat_panel_calc_progress_idx(u16 total_min, u32 remain_min)
{
    if (total_min == 0) {
        total_min = 1;
    }
    if (remain_min > total_min) {
        remain_min = total_min;
    }
    /* 倒计时：remain=total→idx=13 满蓝弧；remain→0→idx=1 无蓝弧；蓝弧从远 CCW 端缩回 */
    if (remain_min == 0) {
        return 1;
    }
    return (u8)(1 + (remain_min * (NEW_HEAT_PROGRESS_CNT - 1) + total_min - 1) / total_min);
}

/* 由 func_heat.c 提供的状态读取 */
extern u8 func_heat_panel_get_ui_state(const void *f);
extern u8 func_heat_panel_get_set_hour(const void *f);
extern u8 func_heat_panel_get_set_min(const void *f);
extern u8 func_heat_panel_get_live_ready(const void *f);
extern u32 func_heat_panel_get_remain_min(const void *f);
extern u32 func_heat_panel_get_total_sec(const void *f);
extern bool func_heat_panel_is_heating(const void *f);

static u16 heat_panel_total_min(const void *f_heat)
{
    u16 total_min;
    u32 total_sec;

    total_min = (u16)(func_heat_panel_get_set_hour(f_heat) * 60
                      + func_heat_panel_get_set_min(f_heat));
    if (total_min > 0) {
        return total_min;
    }
    total_sec = func_heat_panel_get_total_sec(f_heat);
    if (total_sec > 0) {
        total_min = (u16)(total_sec / 60);
        if (total_min == 0) {
            total_min = 1;
        }
        return total_min;
    }
    return 1;
}

/** 加热剩余分钟：优先 MCU DP06（live 或 heat_display_last），勿用本地 total 顶替 */
static u32 heat_panel_remain_min_resolve(const void *f_heat)
{
    if (func_heat_panel_get_live_ready(f_heat)) {
        return func_heat_panel_get_remain_min(f_heat);
    }
    if (func_heat_panel_is_heating(f_heat)) {
        heat_display_info_t snap;

        if (heat_display_get_last(&snap) && snap.remain_min > 0) {
            return snap.remain_min;
        }
    }
    return heat_panel_total_min(f_heat);
}

static u8 heat_panel_progress_idx_resolve(const void *f_heat)
{
    u16 total_min;
    u32 remain_min;

    total_min = heat_panel_total_min(f_heat);
    remain_min = heat_panel_remain_min_resolve(f_heat);
    return heat_panel_calc_progress_idx(total_min, remain_min);
}

static void heat_panel_format_remain(char *buf, u32 remain_min)
{
    sprintf(buf, "%lumin", (unsigned long)remain_min);
}

static void heat_panel_format_duration(char *buf, u16 total_min)
{
    u8 hour = (u8)(total_min / 60);
    u8 min = (u8)(total_min % 60);

    if (min == 0 && hour > 0) {
        if (hour == 1) {
            sprintf(buf, "1 Hour");
        } else {
            sprintf(buf, "%u Hours", hour);
        }
    } else if (min == 30) {
        sprintf(buf, "%uH30min", hour);
    } else {
        sprintf(buf, "%uH%umin", hour, min);
    }
}

static void heat_panel_format_temp(char *buf, u16 temp_f)
{
    sprintf(buf, "%uF", temp_f);
}

/* 由 func_heat.c 提供的状态读取 */
extern u8 func_heat_panel_get_temp_idx(const void *f);
extern u16 func_heat_panel_get_live_temp_f(const void *f);
extern void func_heat_panel_set_live(struct f_heat_t_ *f_heat, u32 remain_min, u16 temp_f);
extern void func_heat_panel_heating_finish(struct f_heat_t_ *f_heat);

void func_heat_panel_push_live(u32 heat_remain_min, u16 temp_f)
{
    /* 加热中：推送剩余时长和温度 */
    heat_display_show(heat_remain_min, temp_f);
}

void func_heat_panel_ack_mcu_live(u32 remain_min)
{
    if (remain_min > 0) {
        g_hp_live_seen_positive = true;
    }
}

static void heat_panel_display_on_info(const heat_display_info_t *info)
{
    struct f_heat_t_ *f_heat;

    if (info == NULL || g_hp_f_heat == NULL || func_cb.sta != FUNC_HEAT) {
        return;
    }
    f_heat = g_hp_f_heat;
    if (!func_heat_panel_is_heating(f_heat)) {
        return;
    }

    if (info->remain_min > 0) {
        g_hp_live_seen_positive = true;
    } else if (!g_hp_live_seen_positive) {
        /* 开局常见上一轮 remain=0 残留；未收到正数前一律忽略，防止误触发加热完成 */
        return;
    }

    func_heat_panel_set_live(f_heat, info->remain_min, info->temp_f);

    if (info->remain_min == 0) {   //加热结束
        func_heat_panel_heating_finish(f_heat);
        return;
    }

    g_hp.last_remain_min = 0xffffffff;
    g_hp.last_progress_idx = 0xff;
    func_heat_panel_process(f_heat);
}

static void heat_panel_font_bind_txt(compo_textbox_t *txt)
{
    if (txt != NULL) {
        compo_textbox_set_font(txt, NEW_HEAT_FONT);
    }
}

static void heat_panel_font_apply_once(void)
{
    if (g_hp.font_ready) {
        return;
    }

    WDT_CLR();
    heat_panel_font_bind_txt(g_hp.txt_remain);
    heat_panel_font_bind_txt(g_hp.txt_remain_lbl);
    heat_panel_font_bind_txt(g_hp.txt_temp);
    heat_panel_font_bind_txt(g_hp.txt_temp_lbl);
    WDT_CLR();
    heat_panel_font_bind_txt(g_hp.txt_dur);
    heat_panel_font_bind_txt(g_hp.txt_dur_lbl);
    g_hp.font_ready = true;

    /* 方式 3：覆盖标签为 14px 字体 */
    compo_textbox_set_font(g_hp.txt_remain_lbl, UI_BUF_0FONT_FONT_TEST_10_BIN);
    compo_textbox_set_font(g_hp.txt_temp_lbl, UI_BUF_0FONT_FONT_TEST_12_BIN);
    compo_textbox_set_font(g_hp.txt_dur_lbl, UI_BUF_0FONT_FONT_TEST_12_BIN);
}

static void heat_panel_text_apply(const void *f_heat)
{
    char buf[32];
    u16 total_min;
    u32 remain_min;
    u16 temp_f;

    if (f_heat == NULL) {
        return;
    }
    heat_panel_font_apply_once();
    total_min = heat_panel_total_min(f_heat);
    remain_min = heat_panel_remain_min_resolve(f_heat);
    if (func_heat_panel_get_live_ready(f_heat)) {
        temp_f = func_heat_panel_get_live_temp_f(f_heat);
    } else {
        temp_f = heat_panel_target_temp_f(func_heat_panel_get_temp_idx(f_heat));
    }

    heat_panel_format_remain(buf, remain_min);
    if (g_hp.txt_remain != NULL) {
        compo_textbox_set(g_hp.txt_remain, buf);
        compo_textbox_set_visible(g_hp.txt_remain, true);
    }
    if (g_hp.txt_remain_lbl != NULL) {
        compo_textbox_set(g_hp.txt_remain_lbl, "Heating Time Remaining");
        compo_textbox_set_visible(g_hp.txt_remain_lbl, true);
    }

    heat_panel_format_temp(buf, temp_f);
    if (g_hp.txt_temp != NULL) {
        compo_textbox_set(g_hp.txt_temp, buf);
        compo_textbox_set_visible(g_hp.txt_temp, true);
    }
    if (g_hp.txt_temp_lbl != NULL) {
        compo_textbox_set(g_hp.txt_temp_lbl, "Heating Temp");
        widget_text_set_client(g_hp.txt_temp_lbl->txt, 0, 7);
        compo_textbox_set_visible(g_hp.txt_temp_lbl, true);
    }

    heat_panel_format_duration(buf, total_min);
    if (g_hp.txt_dur != NULL) {
        compo_textbox_set(g_hp.txt_dur, buf);
        compo_textbox_set_visible(g_hp.txt_dur, true);
    }
    if (g_hp.txt_dur_lbl != NULL) {
        compo_textbox_set(g_hp.txt_dur_lbl, "Heating Duration");
        widget_text_set_client(g_hp.txt_dur_lbl->txt, 0, 7);
        compo_textbox_set_visible(g_hp.txt_dur_lbl, true);
    }
}

compo_form_t *func_heat_panel_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_picturebox_t *pic;
    s16 bat_x;
    s16 bt_x;

    printf("heat_panel_form_create: start\n");

    memset(&g_hp, 0, sizeof(g_hp));
    g_hp.last_progress_idx = 0xff;
    g_hp.last_remain_min = 0xffffffff;
    g_hp.text_pending = true;
    g_hp.show_ready = false;
    g_hp.track_ready = false;

    heat_panel_white_bg(frm);

    home_top_time_txt_create(frm, HEAT_PANEL_ID_TXT_TOP_TIME);
    home_top_time_txt_bind(&g_hp.top_time, HEAT_PANEL_ID_TXT_TOP_TIME);
    g_hp.last_top_min = 0xff;
    g_hp.last_top_sec = 0xff;

    /* 状态图标 */
    bat_x = (s16)(GUI_SCREEN_WIDTH - HEAT_PANEL_STATUS_RIGHT_MARGIN - NEW_HOME_BAT_W / 2);
    bt_x = (s16)(bat_x - NEW_HOME_BAT_W / 2 - HEAT_PANEL_STATUS_GAP - NEW_HOME_BT_W / 2);

    pic = heat_panel_pic_hidden(frm, HEAT_PANEL_ID_BT);
    compo_picturebox_set_pos(pic, bt_x, HEAT_PANEL_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);
    g_hp.pic_bt = pic;

    pic = heat_panel_pic_hidden(frm, HEAT_PANEL_ID_BAT);
    compo_picturebox_set_pos(pic, bat_x, HEAT_PANEL_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);
    g_hp.pic_bat = pic;

    /* 圆环图层（自下而上：灰轨 → 蓝弧 → 圆点 → show 条） */
    pic = heat_panel_pic_hidden(frm, HEAT_PANEL_ID_PROGRESS_BG);
    g_hp.pic_progress_bg = pic;

    pic = heat_panel_pic_hidden(frm, HEAT_PANEL_ID_PROGRESS);
    g_hp.pic_progress = pic;

    pic = heat_panel_pic_hidden(frm, HEAT_PANEL_ID_POINT);
    compo_picturebox_set_size(pic, NEW_HEAT_POINT_W, NEW_HEAT_POINT_H);
    g_hp.pic_point = pic;

    pic = heat_panel_pic_hidden(frm, HEAT_PANEL_ID_SHOW);
    g_hp.pic_show = pic;

    /* 文字最后创建，保证叠在图标之上 */
    g_hp.txt_remain = heat_panel_txt(frm, HEAT_PANEL_ID_TXT_REMAIN,
                                     GUI_SCREEN_CENTER_X, HEAT_PANEL_REMAIN_Y,
                                     HEAT_PANEL_COLOR_VALUE, true);
    compo_textbox_set_location(g_hp.txt_remain, GUI_SCREEN_CENTER_X, HEAT_PANEL_REMAIN_Y, 200, 36);

    g_hp.txt_remain_lbl = heat_panel_txt(frm, HEAT_PANEL_ID_TXT_REMAIN_LBL,
                                         GUI_SCREEN_CENTER_X, HEAT_PANEL_REMAIN_LBL_Y,
                                         HEAT_PANEL_COLOR_LABEL, true);
    compo_textbox_set_location(g_hp.txt_remain_lbl, GUI_SCREEN_CENTER_X, HEAT_PANEL_REMAIN_LBL_Y, 280, 42);

    g_hp.txt_temp = heat_panel_txt(frm, HEAT_PANEL_ID_TXT_TEMP,
                                   HEAT_PANEL_TEMP_X, HEAT_PANEL_VAL_Y,
                                   HEAT_PANEL_COLOR_VALUE, true);
    compo_textbox_set_location(g_hp.txt_temp, HEAT_PANEL_TEMP_X, HEAT_PANEL_VAL_Y, 120, 28);

    g_hp.txt_temp_lbl = heat_panel_txt(frm, HEAT_PANEL_ID_TXT_TEMP_LBL,
                                       HEAT_PANEL_TEMP_X, HEAT_PANEL_LBL_Y,
                                       HEAT_PANEL_COLOR_LABEL, true);
    compo_textbox_set_location(g_hp.txt_temp_lbl, HEAT_PANEL_TEMP_X, HEAT_PANEL_LBL_Y, 160, 50);

    g_hp.txt_dur = heat_panel_txt(frm, HEAT_PANEL_ID_TXT_DUR,
                                  HEAT_PANEL_DUR_X, HEAT_PANEL_VAL_Y,
                                  HEAT_PANEL_COLOR_VALUE, true);
    compo_textbox_set_location(g_hp.txt_dur, HEAT_PANEL_DUR_X, HEAT_PANEL_VAL_Y, 120, 28);

    g_hp.txt_dur_lbl = heat_panel_txt(frm, HEAT_PANEL_ID_TXT_DUR_LBL,
                                      HEAT_PANEL_DUR_X, HEAT_PANEL_LBL_Y,
                                      HEAT_PANEL_COLOR_LABEL, true);
    compo_textbox_set_location(g_hp.txt_dur_lbl, HEAT_PANEL_DUR_X, HEAT_PANEL_LBL_Y, 200, 50);

    g_hp.ui_ready = true;

    printf("heat_panel_form_create: done (all icons, hidden, no flash)\n");
    return frm;
}

void func_heat_panel_bind(struct f_heat_t_ *f_heat)
{
    g_hp_f_heat = f_heat;
    g_hp_live_seen_positive = false;
    home_ui_shared_battery_attach_pic(g_hp.pic_bat);
    heat_display_register(heat_panel_display_on_info);
    if (f_heat != NULL && func_heat_panel_is_heating(f_heat)) {
        heat_display_info_t last;

        if (heat_display_get_last(&last)) {
            heat_panel_display_on_info(&last);
        }
    }
}

void func_heat_panel_mark_dirty(struct f_heat_t_ *f_heat)
{
    (void)f_heat;
    g_hp.text_pending = true;
    g_hp.last_remain_min = 0xffffffff;
    g_hp.last_progress_idx = 0xff;
    g_hp_live_seen_positive = false;
}

void func_heat_panel_status_refresh(struct f_heat_t_ *f_heat)
{
    (void)f_heat;
    home_top_time_txt_tick(&g_hp.top_time, &g_hp.last_top_min, &g_hp.last_top_sec);
    home_ui_shared_status_init();
    if (g_hp.pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(g_hp.pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(g_hp.pic_bt, NEW_HOME_BT_W, NEW_HOME_BT_H);
        home_ui_shared_status_refresh_bt(g_hp.pic_bt);
    }
    home_ui_shared_status_bind_bat(g_hp.pic_bat);
}

void func_heat_panel_process(struct f_heat_t_ *f_heat)
{
    u32 remain_min;
    u8 progress_idx;

    if (!g_hp.ui_ready || f_heat == NULL) {
        return;
    }

    remain_min = heat_panel_remain_min_resolve(f_heat);
    progress_idx = heat_panel_progress_idx_resolve(f_heat);

    heat_panel_progress_apply(progress_idx);

    if (g_hp.text_pending || remain_min != g_hp.last_remain_min) {
        home_gpu_wait_idle();
        heat_panel_text_apply(f_heat);
        g_hp.text_pending = false;
        g_hp.last_remain_min = remain_min;
    }

    func_heat_panel_status_refresh(f_heat);

#if ELUNCHBOX_PANEL_EN
    if (!func_heat_panel_is_heating(f_heat)) {
        func_key_lock_on_heating_stop();
    }
#endif
}

void func_heat_panel_enter(struct f_heat_t_ *f_heat)
{
    if (!g_hp.ui_ready || f_heat == NULL) {
        printf("heat_panel_enter: skip (ui_ready=%d f_heat=%p)\n", g_hp.ui_ready, f_heat);
        return;
    }

    printf("heat_panel_enter: start\n");
    home_gpu_wait_idle();
    memset(home_ui_colon_ram, 0, HOME_COLON_RAM_SIZE);
    printf("heat_panel_enter: wait1 done\n");
    home_ui_shared_status_init();
    func_heat_panel_status_refresh(f_heat);
    printf("heat_panel_enter: status_refresh done\n");

    u32 total_min;
    u32 remain_min;
    u8 progress_idx;

    total_min = heat_panel_total_min(f_heat);
    remain_min = heat_panel_remain_min_resolve(f_heat);
    progress_idx = heat_panel_progress_idx_resolve(f_heat);
    printf("heat_panel_enter: total=%u uart_remain=%u idx=%u live=%d\n",
           total_min, remain_min, progress_idx,
           func_heat_panel_get_live_ready(f_heat));
    g_hp.last_progress_idx = 0xff;
    heat_panel_progress_apply(progress_idx);
    printf("heat_panel_enter: progress_apply done (idx=%u)\n", progress_idx);

    home_gpu_wait_idle();
    heat_panel_show_apply();
    printf("heat_panel_enter: show_apply done\n");

    heat_panel_text_apply(f_heat);

#if ELUNCHBOX_PANEL_EN
    if (func_heat_panel_is_heating(f_heat) && !elunchbox_pwr_is_manual_off()) {
        func_key_lock_on_heating_start();
    }
#endif
}

void func_heat_panel_exit_to_warm(void)
{
    g_hp_f_heat = NULL;
#if ELUNCHBOX_PANEL_EN
    func_key_lock_on_heating_stop();
#endif
    heat_panel_arc_detach();
    g_hp.track_ready = false;
    g_hp.show_ready = false;
    g_hp.last_progress_idx = 0xff;
    g_hp.ui_ready = false;
    /* 灰轨为 Flash；heat_bg 曾用于蓝弧，保温页 enter 会重载灰轨 RAM */
    memset(&g_hp, 0, sizeof(g_hp));
}

void func_heat_panel_exit(void)
{
    g_hp_track_ram_valid = false;
    g_hp_f_heat = NULL;
#if ELUNCHBOX_PANEL_EN
    func_key_lock_on_heating_stop();
#endif
    compo_picturebox_t *pics[5];
    u8 n = 0;
    u8 i;

    if (g_hp.pic_progress_bg != NULL) {
        pics[n++] = g_hp.pic_progress_bg;
    }
    if (g_hp.pic_progress != NULL) {
        pics[n++] = g_hp.pic_progress;
    }
    if (g_hp.pic_point != NULL) {
        pics[n++] = g_hp.pic_point;
    }
    if (g_hp.pic_show != NULL) {
        pics[n++] = g_hp.pic_show;
    }
    if (g_hp.pic_bt != NULL) {
        pics[n++] = g_hp.pic_bt;
    }
    home_ui_gpu_pics_detach(pics, n);
    heat_panel_arc_detach();
    g_hp.track_ready = false;
    g_hp.show_ready = false;
    g_hp.last_progress_idx = 0xff;
    g_hp.ui_ready = false;
    for (i = 0; i < n; i++) {
        pics[i] = NULL;
    }
    memset(&g_hp, 0, sizeof(g_hp));
}

bool func_heat_panel_track_ram_valid(void)
{
    return g_hp_track_ram_valid;
}

void func_heat_panel_track_ram_consume(void)
{
    g_hp_track_ram_valid = false;
}
