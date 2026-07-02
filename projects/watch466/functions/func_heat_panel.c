#include "include.h"
#include "func.h"
#include "func_heat_panel.h"
#include "new_heat_res.h"
#include "home_ui_shared.h"
#include "home_icon_res.h"
#include "new_home_icon_res.h"
#include "home_ui_ram.h"
#include "home_ui_gpu_detach.h"

#if ELUNCHBOX_PANEL_EN
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

#define HEAT_PANEL_STATUS_Y             20
#define HEAT_PANEL_STATUS_RIGHT_MARGIN  10
#define HEAT_PANEL_STATUS_GAP           6

#define HEAT_PANEL_COLOR_VALUE          0x2BF4
#define HEAT_PANEL_COLOR_LABEL          0x0AD8

#define HEAT_PANEL_REMAIN_Y             88
#define HEAT_PANEL_REMAIN_LBL_Y         112
#define HEAT_PANEL_SHOW_Y               210
#define HEAT_PANEL_TEMP_X               72
#define HEAT_PANEL_DUR_X                248
#define HEAT_PANEL_VAL_Y                192
#define HEAT_PANEL_LBL_Y                212

#define HEAT_PANEL_ARC_CX               (GUI_SCREEN_WIDTH / 2)
#define HEAT_PANEL_ARC_CY               95
#define HEAT_PANEL_ARC_R                72

enum {
    HEAT_PANEL_ID_BG = 1,
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

static heat_panel_ui_t g_hp;

/* bg→union heat_bg(.disp)；show→tab 图标池；蓝弧→digit_ram（≤31KB，互斥 new_heat） */
#define HEAT_PANEL_SHOW_RAM         ((u8 *)home_ui_shared_icon_runtime)
#define HEAT_PANEL_SHOW_RAM_CAP     ((u32)(HOME_UI_SHARED_TAB_CNT * NEW_HOME_TAB_RAM_SIZE))
#define HEAT_PANEL_PROGRESS_RAM     ((u8 *)home_ui_digit_ram)
#define HEAT_PANEL_PROGRESS_RAM_CAP ((u32)sizeof(home_ui_digit_ram))

#if NEW_HEAT_SHOW_RAM_SIZE > (HOME_UI_SHARED_TAB_CNT * NEW_HOME_TAB_RAM_SIZE)
#error "new_show.bin too large for home_ui_shared_icon_runtime pool"
#endif

static void heat_panel_pic_set_ram(compo_picturebox_t *pic, u32 flash_addr,
                                   u16 w, u16 h, u8 *buf, u32 buf_size)
{
    u32 len;

    if (pic == NULL || flash_addr == 0 || w == 0 || h == 0 || buf == NULL) {
        return;
    }
    len = (u32)w * h * 2 + 8;
    if (len > buf_size) {
        printf("pic_ram: buf too small! need=%u have=%u\n", len, buf_size);
        return;
    }
    printf("pic_ram: spiflash_read addr=0x%x len=%u\n", flash_addr, len);
    os_spiflash_read(buf, flash_addr, len);
    WDT_CLR();
    printf("pic_ram: read done\n");
    home_gpu_wait_idle();
    compo_picturebox_set_ram(pic, buf);
    printf("pic_ram: set_ram done\n");
}

static const u16 tbl_heat_temp_preset[7] = {
    104, 122, 140, 158, 176, 194, 212,
};

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

static void heat_panel_point_pos(u8 progress_idx, s16 *x, s16 *y)
{
    u8 step;

    if (progress_idx < 1) {
        progress_idx = 1;
    }
    if (progress_idx > NEW_HEAT_PROGRESS_CNT) {
        progress_idx = NEW_HEAT_PROGRESS_CNT;
    }
    step = progress_idx - 1;
    {
        static const s16 tbl_x[NEW_HEAT_PROGRESS_CNT] = {
            88, 96, 108, 122, 138, 154, 160, 166, 182, 198, 212, 224, 232,
        };
        static const s16 tbl_y[NEW_HEAT_PROGRESS_CNT] = {
            118, 102, 90, 82, 76, 72, 70, 72, 76, 82, 90, 102, 118,
        };
        *x = tbl_x[step];
        *y = tbl_y[step];
    }
}

static void heat_panel_point_bind(u8 progress_idx)
{
    s16 px;
    s16 py;

    if (g_hp.pic_point == NULL) {
        printf("point_bind: skip pic=NULL\n");
        return;
    }
    heat_panel_point_pos(progress_idx, &px, &py);
    printf("point_bind: idx=%u pos=(%d,%d) spiflash_read addr=0x%x len=%u\n",
           progress_idx, px, py, UI_BUF_NEW_UI_NEW_POINT_BIN, UI_LEN_NEW_UI_NEW_POINT_BIN);
    home_gpu_wait_idle();
    os_spiflash_read(home_ui_colon_ram, UI_BUF_NEW_UI_NEW_POINT_BIN, UI_LEN_NEW_UI_NEW_POINT_BIN);
    printf("point_bind: spiflash done\n");
    if (!gui_set_ram_check(home_ui_colon_ram, __func__)) {
        printf("point_bind: ram check fail\n");
        compo_picturebox_set_visible(g_hp.pic_point, false);
        return;
    }
    home_gpu_wait_idle();
    compo_picturebox_set_ram(g_hp.pic_point, home_ui_colon_ram);
    printf("point_bind: set_ram done\n");
    compo_picturebox_set_size(g_hp.pic_point, NEW_HEAT_POINT_W, NEW_HEAT_POINT_H);
    compo_picturebox_set_pos(g_hp.pic_point, px, py);
    compo_picturebox_set_visible(g_hp.pic_point, true);
    home_gpu_wait_idle();
    printf("point_bind: done\n");
}

static void heat_panel_track_apply(void)
{
    if (g_hp.track_ready || g_hp.pic_progress_bg == NULL) {
        printf("track_apply: skip (ready=%d bg=%p)\n", g_hp.track_ready, g_hp.pic_progress_bg);
        return;
    }
    printf("track_apply: set_ram bg addr=0x%x w=%u h=%u\n",
           UI_BUF_NEW_UI_NEW_PROGRESS_BG_BIN, NEW_HEAT_NEW_PROGRESS_BG_W, NEW_HEAT_NEW_PROGRESS_BG_H);
    heat_panel_pic_set_ram(g_hp.pic_progress_bg, UI_BUF_NEW_UI_NEW_PROGRESS_BG_BIN,
                            NEW_HEAT_NEW_PROGRESS_BG_W, NEW_HEAT_NEW_PROGRESS_BG_H,
                            home_ui_heat_bg_ram, sizeof(home_ui_heat_bg_ram));
    compo_picturebox_set_size(g_hp.pic_progress_bg, NEW_HEAT_NEW_PROGRESS_BG_W, NEW_HEAT_NEW_PROGRESS_BG_H);
    compo_picturebox_set_visible(g_hp.pic_progress_bg, true);
    compo_picturebox_set_pos(g_hp.pic_progress_bg,
                             NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_X, NEW_HEAT_NEW_PROGRESS_BG_ANCHOR_Y);
    printf("track_apply: done\n");
    g_hp.track_ready = true;
}

static void heat_panel_progress_overlay_apply(u8 idx)
{
    u8 overlay_idx;
    u8 step;
    u16 pw;
    u16 ph;

    overlay_idx = idx;
    step = overlay_idx - 1;
    pw = tbl_progress_w[step];
    ph = tbl_progress_h[step];
    /* new_progress_1 裁剪后为空；用第 2 帧作为起始蓝弧 */
    if (pw == 0 || ph == 0) {
        overlay_idx = 2;
        step = overlay_idx - 1;
        pw = tbl_progress_w[step];
        ph = tbl_progress_h[step];
    }
    if (pw == 0 || ph == 0) {
        home_gpu_wait_idle();
        compo_picturebox_set_visible(g_hp.pic_progress, false);
        return;
    }
    printf("progress_overlay: idx=%u overlay=%u pw=%u ph=%u\n", idx, overlay_idx, pw, ph);
    {
        u32 need = (u32)pw * ph * 2 + 8;

        if (need <= HEAT_PANEL_PROGRESS_RAM_CAP) {
            heat_panel_pic_set_ram(g_hp.pic_progress, heat_panel_progress_addr(overlay_idx),
                                    pw, ph, HEAT_PANEL_PROGRESS_RAM, HEAT_PANEL_PROGRESS_RAM_CAP);
        } else {
            home_ui_pic_set_flash(g_hp.pic_progress, heat_panel_progress_addr(overlay_idx), pw, ph);
        }
    }
    compo_picturebox_set_size(g_hp.pic_progress, pw, ph);
    compo_picturebox_set_pos(g_hp.pic_progress, tbl_progress_ax[step], tbl_progress_ay[step]);
    compo_picturebox_set_visible(g_hp.pic_progress, true);
}

static void heat_panel_progress_apply(u8 idx)
{
    if (g_hp.pic_progress == NULL || idx == 0) {
        printf("progress_apply: skip pic=%p idx=%u\n", g_hp.pic_progress, idx);
        return;
    }
    if (idx == g_hp.last_progress_idx) {
        printf("progress_apply: skip same idx=%u\n", idx);
        return;
    }
    printf("progress_apply: track_apply (idx=%u)\n", idx);
    heat_panel_track_apply();
    printf("progress_apply: idx=%u\n", idx);
    heat_panel_progress_overlay_apply(idx);
    g_hp.last_progress_idx = idx;
    printf("progress_apply: point_bind idx=%u\n", idx);
    heat_panel_point_bind(idx);
    printf("progress_apply: done idx=%u\n", idx);
}

static void heat_panel_show_apply(void)
{
    if (g_hp.pic_show == NULL || g_hp.show_ready) {
        printf("show_apply: skip pic=%p ready=%d\n", g_hp.pic_show, g_hp.show_ready);
        return;
    }
    printf("show_apply: set_ram addr=0x%x w=%u h=%u\n",
           UI_BUF_NEW_UI_NEW_SHOW_BIN, NEW_HEAT_NEW_SHOW_W, NEW_HEAT_NEW_SHOW_H);
    heat_panel_pic_set_ram(g_hp.pic_show, UI_BUF_NEW_UI_NEW_SHOW_BIN,
                            NEW_HEAT_NEW_SHOW_W, NEW_HEAT_NEW_SHOW_H,
                            HEAT_PANEL_SHOW_RAM, HEAT_PANEL_SHOW_RAM_CAP);
    compo_picturebox_set_size(g_hp.pic_show, NEW_HEAT_NEW_SHOW_W, NEW_HEAT_NEW_SHOW_H);
    compo_picturebox_set_visible(g_hp.pic_show, true);
    compo_picturebox_set_pos(g_hp.pic_show, GUI_SCREEN_CENTER_X, HEAT_PANEL_SHOW_Y);
    printf("show_apply: set_ram done\n");
    g_hp.show_ready = true;
}

static u16 heat_panel_target_temp_f(u8 temp_idx)
{
    if (temp_idx >= 7) {
        return tbl_heat_temp_preset[0];
    }
    return tbl_heat_temp_preset[temp_idx];
}

static u16 heat_panel_total_min(u8 set_hour, u8 set_min)
{
    return (u16)set_hour * 60 + set_min;
}

static u8 heat_panel_calc_progress_idx(u16 total_min, u32 remain_min)
{
    u32 elapsed;

    if (total_min == 0) {
        total_min = 1;
    }
    if (remain_min > total_min) {
        remain_min = total_min;
    }
    elapsed = total_min - remain_min;
    if (elapsed >= total_min) {
        return NEW_HEAT_PROGRESS_CNT;
    }
    return (u8)(1 + (elapsed * (NEW_HEAT_PROGRESS_CNT - 1)) / total_min);
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
extern u8 func_heat_panel_get_ui_state(const void *f);
extern u8 func_heat_panel_get_set_hour(const void *f);
extern u8 func_heat_panel_get_set_min(const void *f);
extern u8 func_heat_panel_get_temp_idx(const void *f);
extern u32 func_heat_panel_get_remain_min(const void *f);
extern bool func_heat_panel_get_live_ready(const void *f);

static void heat_panel_text_apply(const void *f_heat)
{
    char buf[32];
    u16 total_min;
    u32 remain_min;
    u16 temp_f;

    if (f_heat == NULL) {
        return;
    }
    total_min = heat_panel_total_min(func_heat_panel_get_set_hour(f_heat),
                                     func_heat_panel_get_set_min(f_heat));
    if (func_heat_panel_get_live_ready(f_heat)) {
        remain_min = func_heat_panel_get_remain_min(f_heat);
    } else {
        remain_min = total_min;
    }
    temp_f = heat_panel_target_temp_f(func_heat_panel_get_temp_idx(f_heat));

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
        compo_textbox_set_visible(g_hp.txt_temp_lbl, true);
    }

    heat_panel_format_duration(buf, total_min);
    if (g_hp.txt_dur != NULL) {
        compo_textbox_set(g_hp.txt_dur, buf);
        compo_textbox_set_visible(g_hp.txt_dur, true);
    }
    if (g_hp.txt_dur_lbl != NULL) {
        compo_textbox_set(g_hp.txt_dur_lbl, "Heating Duration");
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
    compo_textbox_set_location(g_hp.txt_remain_lbl, GUI_SCREEN_CENTER_X, HEAT_PANEL_REMAIN_LBL_Y, 260, 24);

    g_hp.txt_temp = heat_panel_txt(frm, HEAT_PANEL_ID_TXT_TEMP,
                                   HEAT_PANEL_TEMP_X, HEAT_PANEL_VAL_Y,
                                   HEAT_PANEL_COLOR_VALUE, true);
    compo_textbox_set_location(g_hp.txt_temp, HEAT_PANEL_TEMP_X, HEAT_PANEL_VAL_Y, 120, 28);

    g_hp.txt_temp_lbl = heat_panel_txt(frm, HEAT_PANEL_ID_TXT_TEMP_LBL,
                                       HEAT_PANEL_TEMP_X, HEAT_PANEL_LBL_Y,
                                       HEAT_PANEL_COLOR_LABEL, true);
    compo_textbox_set_location(g_hp.txt_temp_lbl, HEAT_PANEL_TEMP_X, HEAT_PANEL_LBL_Y, 120, 22);

    g_hp.txt_dur = heat_panel_txt(frm, HEAT_PANEL_ID_TXT_DUR,
                                  HEAT_PANEL_DUR_X, HEAT_PANEL_VAL_Y,
                                  HEAT_PANEL_COLOR_VALUE, true);
    compo_textbox_set_location(g_hp.txt_dur, HEAT_PANEL_DUR_X, HEAT_PANEL_VAL_Y, 120, 28);

    g_hp.txt_dur_lbl = heat_panel_txt(frm, HEAT_PANEL_ID_TXT_DUR_LBL,
                                      HEAT_PANEL_DUR_X, HEAT_PANEL_LBL_Y,
                                      HEAT_PANEL_COLOR_LABEL, true);
    compo_textbox_set_location(g_hp.txt_dur_lbl, HEAT_PANEL_DUR_X, HEAT_PANEL_LBL_Y, 120, 22);

    g_hp.ui_ready = true;

    printf("heat_panel_form_create: done (all icons, hidden, no flash)\n");
    return frm;
}

void func_heat_panel_bind(struct f_heat_t_ *f_heat)
{
    (void)f_heat;
    home_ui_shared_battery_attach_pic(g_hp.pic_bat);
}

void func_heat_panel_mark_dirty(struct f_heat_t_ *f_heat)
{
    (void)f_heat;
    g_hp.text_pending = true;
    g_hp.last_remain_min = 0xffffffff;
    g_hp.last_progress_idx = 0xff;
}

void func_heat_panel_status_refresh(struct f_heat_t_ *f_heat)
{
    (void)f_heat;
    home_ui_shared_status_init();
    if (g_hp.pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(g_hp.pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(g_hp.pic_bt, NEW_HOME_BT_W, NEW_HOME_BT_H);
        compo_picturebox_set_visible(g_hp.pic_bt, true);
    }
    home_ui_shared_status_bind_bat(g_hp.pic_bat);
}

void func_heat_panel_process(struct f_heat_t_ *f_heat)
{
    u16 total_min;
    u32 remain_min;
    u8 progress_idx;

    if (!g_hp.ui_ready || f_heat == NULL) {
        return;
    }

    total_min = heat_panel_total_min(func_heat_panel_get_set_hour(f_heat),
                                   func_heat_panel_get_set_min(f_heat));
    if (func_heat_panel_get_live_ready(f_heat)) {
        remain_min = func_heat_panel_get_remain_min(f_heat);
    } else {
        remain_min = total_min;
    }
    progress_idx = heat_panel_calc_progress_idx(total_min, remain_min);

    heat_panel_progress_apply(progress_idx);

    if (g_hp.text_pending || remain_min != g_hp.last_remain_min) {
        home_gpu_wait_idle();
        heat_panel_text_apply(f_heat);
        g_hp.text_pending = false;
        g_hp.last_remain_min = remain_min;
    }

    func_heat_panel_status_refresh(f_heat);
}

void func_heat_panel_enter(struct f_heat_t_ *f_heat)
{
    if (!g_hp.ui_ready || f_heat == NULL) {
        printf("heat_panel_enter: skip (ui_ready=%d f_heat=%p)\n", g_hp.ui_ready, f_heat);
        return;
    }

    printf("heat_panel_enter: start\n");
    home_gpu_wait_idle();
    printf("heat_panel_enter: wait1 done\n");
    home_ui_shared_status_init();
    func_heat_panel_status_refresh(f_heat);
    printf("heat_panel_enter: status_refresh done\n");
    heat_panel_track_apply();
    printf("heat_panel_enter: track_apply done\n");

    heat_panel_show_apply();
    printf("heat_panel_enter: show_apply done\n");

    u32 total_min, remain_min;
    u8 progress_idx;
    total_min = heat_panel_total_min(func_heat_panel_get_set_hour(f_heat),
                                     func_heat_panel_get_set_min(f_heat));
    if (func_heat_panel_get_live_ready(f_heat)) {
        remain_min = func_heat_panel_get_remain_min(f_heat);
    } else {
        remain_min = total_min;
    }
    progress_idx = heat_panel_calc_progress_idx(total_min, remain_min);
    printf("heat_panel_enter: total=%u remain=%u idx=%u\n", total_min, remain_min, progress_idx);
    g_hp.last_progress_idx = 0xff;
    heat_panel_progress_apply(progress_idx);
    printf("heat_panel_enter: progress_apply done (idx=%u, no DMA for zero size)\n", progress_idx);

    heat_panel_text_apply(f_heat);
}

void func_heat_panel_exit(void)
{
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
    g_hp.track_ready = false;
    g_hp.show_ready = false;
    g_hp.last_progress_idx = 0xff;
    g_hp.ui_ready = false;
    for (i = 0; i < n; i++) {
        pics[i] = NULL;
    }
    memset(&g_hp, 0, sizeof(g_hp));
}
