#include "include.h"
#include "func.h"
#include "new_mode_res.h"
#include "new_home_icon_res.h"
#include "new_home_top_time.h"
#include "home_ui_shared.h"
#include "home_ui_gpu_detach.h"
#include "home_ui_ram.h"
#include "home_top_time.h"
#include "func_key_lock.h"
#include "func_lunchbox_uart.h"

#if ELUNCHBOX_PANEL_EN
extern volatile u8 elunchbox_te_block_flag;
#endif

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

#ifndef UI_BUF_NEW_UI_NEW_CHAGE_BIN
#error "Run tools/gen_new_mode_icons.py then Output/bin/prebuild.bat"
#endif

/*
 * 模式页 — 列表：Delay / Chicken / Pasta / Warm
 *   模式键：循环选中；确认键：进入对应功能
 *   开关键：回 Home
 */
#define NEW_MODE_STATUS_Y                 20
#define NEW_MODE_STATUS_RIGHT_MARGIN      10
#define NEW_MODE_STATUS_GAP               6
#define NEW_MODE_TITLE_Y                  38
#define NEW_MODE_TITLE_W                  120
#define NEW_MODE_TITLE_H                  36

#define NEW_MODE_PANEL_Y                  132
#define NEW_MODE_PANEL_W                  280
#define NEW_MODE_PANEL_H                  188

#define NEW_MODE_ROW_FIRST_Y              68
#define NEW_MODE_ROW_GAP                  42
#define NEW_MODE_ICON_X                   36
#define NEW_MODE_LABEL_GAP                12
#define NEW_MODE_ARROW_X                  298
#define NEW_MODE_LABEL_X                  (NEW_MODE_ICON_X + NEW_MODE_ICON_W / 2 + NEW_MODE_LABEL_GAP)
#define NEW_MODE_LABEL_H                  36
#define NEW_MODE_LABEL_W                  ((s16)(NEW_MODE_ARROW_X - NEW_MODE_NEW_LEFT_W / 2 - 10 - NEW_MODE_LABEL_X))

#define NEW_MODE_COLOR_TITLE              COLOR_BLACK
#define NEW_MODE_COLOR_SEL                0x0AD8
#define NEW_MODE_COLOR_NOR                COLOR_BLACK
#define NEW_MODE_COLOR_SEL_BG             0xCFDF   /* 选中行浅蓝底（替代 chage.bin，省 RAM/Flash） */
#define NEW_MODE_PANEL_BG                 0xEF5D

#define NEW_MODE_MSG_OK                   KU_BACK
#define NEW_MODE_MSG_MODE                 KU_MODE
#define NEW_MODE_MSG_POWER                (KEY_RIGHT | KEY_SHORT_UP)

enum {
    NEW_MODE_ITEM_DELAY = 0,
    NEW_MODE_ITEM_CHICKEN,
    NEW_MODE_ITEM_PASTA,
    NEW_MODE_ITEM_WARM,
    NEW_MODE_ITEM_CNT,
};

enum {
    COMPO_ID_SHAPE_BG = 1,
    COMPO_ID_SHAPE_PANEL,
    COMPO_ID_PIC_TOP_TIME_H10,
    COMPO_ID_PIC_TOP_TIME_H1,
    COMPO_ID_PIC_TOP_TIME_COLON,
    COMPO_ID_PIC_TOP_TIME_M10,
    COMPO_ID_PIC_TOP_TIME_M1,
    COMPO_ID_PIC_TOP_TIME_AMPM,
    COMPO_ID_TXT_TITLE,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_BAT,
    COMPO_ID_SHAPE_SEL = 20,
    COMPO_ID_PIC_ICON_BASE = 30,
    COMPO_ID_PIC_ARROW_BASE = 40,
    COMPO_ID_TXT_LABEL_BASE = 50,
};

typedef struct {
    u8 sel;
    bool display_pending;
#if ELUNCHBOX_PANEL_EN
    bool key_ready;
#else
    u8 last_top_min;
    u8 last_top_sec;
    home_top_time_ui_t top_time;
#endif
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_bat;
#if ELUNCHBOX_PANEL_EN
    compo_shape_t *shape_sel;
#else
    compo_picturebox_t *pic_sel;
#endif
    compo_picturebox_t *pic_icon[NEW_MODE_ITEM_CNT];
    compo_picturebox_t *pic_arrow[NEW_MODE_ITEM_CNT];
    compo_textbox_t *txt_title;
    compo_textbox_t *txt_label[NEW_MODE_ITEM_CNT];
} f_new_mode_t;

static const char * const tbl_new_mode_label[NEW_MODE_ITEM_CNT] = {
    "Delay",
    "Chicken",
    "Pasta",
    "Warm",
};

static s16 new_mode_row_y(u8 row)
{
    return (s16)(NEW_MODE_ROW_FIRST_Y + (s16)row * NEW_MODE_ROW_GAP);
}

static void new_mode_label_txt_prepare(compo_textbox_t *txt, s16 row_y);
static void new_mode_label_txt_show(compo_textbox_t *txt, s16 row_y, const char *label, u16 color);
static void new_mode_label_txt_color(compo_textbox_t *txt, u16 color);
static void new_mode_title_txt_prepare(compo_textbox_t *txt);
static void new_mode_title_txt_show(compo_textbox_t *txt);

static u32 new_mode_icon_addr(u8 item, bool selected)
{
    static const u32 tbl[NEW_MODE_ITEM_CNT][2] = {
        { UI_BUF_NEW_UI_NEW_ORDER_0_BIN,   UI_BUF_NEW_UI_NEW_ORDER_1_BIN },
        { UI_BUF_NEW_UI_NEW_CHICKEN_0_BIN, UI_BUF_NEW_UI_NEW_CHICKEN_1_BIN },
        { UI_BUF_NEW_UI_NEW_PASTA_0_BIN,   UI_BUF_NEW_UI_NEW_PASTA_1_BIN },
        { UI_BUF_NEW_UI_NEW_WARM_0_BIN,    UI_BUF_NEW_UI_NEW_WARM_1_BIN },
    };

    if (item >= NEW_MODE_ITEM_CNT) {
        return tbl[0][0];
    }
    return tbl[item][selected ? 1 : 0];
}

static u16 new_mode_icon_len(u8 item, bool selected)
{
    static const u16 tbl[NEW_MODE_ITEM_CNT][2] = {
        { UI_LEN_NEW_UI_NEW_ORDER_0_BIN,   UI_LEN_NEW_UI_NEW_ORDER_1_BIN },
        { UI_LEN_NEW_UI_NEW_CHICKEN_0_BIN, UI_LEN_NEW_UI_NEW_CHICKEN_1_BIN },
        { UI_LEN_NEW_UI_NEW_PASTA_0_BIN,   UI_LEN_NEW_UI_NEW_PASTA_1_BIN },
        { UI_LEN_NEW_UI_NEW_WARM_0_BIN,    UI_LEN_NEW_UI_NEW_WARM_1_BIN },
    };

    if (item >= NEW_MODE_ITEM_CNT) {
        return tbl[0][0];
    }
    return tbl[item][selected ? 1 : 0];
}

#if ELUNCHBOX_PANEL_EN
static bool new_mode_arrow_ram_ready;

static bool new_mode_gpu_ram_bind(const char *tag, u8 *ram, u16 buf_size, u32 addr, u16 len,
                                  compo_picturebox_t *pic, u16 w, u16 h, s16 x, s16 y)
{
    u16 need;

    if (pic == NULL || addr == 0 || len == 0) {
        printf("nm_bind %s skip null pic=%p addr=%x\n", tag, pic, (unsigned)addr);
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }
    if (ram == NULL || len > buf_size) {
        printf("nm_bind %s ram err buf=%u len=%u\n", tag, buf_size, len);
        compo_picturebox_set_visible(pic, false);
        return false;
    }

    WDT_CLR();
    os_spiflash_read(ram, addr, len);
    if (!gui_set_ram_check(ram, __func__)) {
        printf("nm_bind %s ram_check fail\n", tag);
        compo_picturebox_set_visible(pic, false);
        return false;
    }

    need = (u16)(8 + (u32)GET_LE16(&ram[4]) * GET_LE16(&ram[6]) * 2);
    if (need > len || need > buf_size) {
        printf("nm_bind %s size need=%u len=%u buf=%u\n", tag, need, len, buf_size);
        compo_picturebox_set_visible(pic, false);
        return false;
    }

    compo_picturebox_set_ram(pic, ram);
    compo_picturebox_set_size(pic, w, h);
    compo_picturebox_set_pos(pic, x, y);
    compo_picturebox_set_visible(pic, true);
    return true;
}

static void new_mode_arrow_ram_ensure(void)
{
    if (new_mode_arrow_ram_ready) {
        return;
    }
    if (UI_LEN_NEW_UI_NEW_LEFT_BIN > HOME_COLON_RAM_SIZE) {
        printf("nm_arrow_ram too large\n");
        return;
    }
    WDT_CLR();
    os_spiflash_read(home_ui_colon_ram, UI_BUF_NEW_UI_NEW_LEFT_BIN,
                     UI_LEN_NEW_UI_NEW_LEFT_BIN);
    if (gui_set_ram_check(home_ui_colon_ram, __func__)) {
        new_mode_arrow_ram_ready = true;
        printf("nm_arrow_ram ok\n");
    }
}

static void new_mode_row_icon_apply(f_new_mode_t *f, u8 row, bool selected, s16 row_y)
{
    u32 addr;
    u16 len;
    char tag[8];

    if (f == NULL || row >= NEW_MODE_ITEM_CNT || f->pic_icon[row] == NULL) {
        return;
    }
    addr = new_mode_icon_addr(row, selected);
    len = new_mode_icon_len(row, selected);
    tag[0] = 'i';
    tag[1] = '0' + row;
    tag[2] = selected ? 's' : 'n';
    tag[3] = '\0';
    (void)new_mode_gpu_ram_bind(tag, home_ui_digit_ram[row], HOME_DIGIT_RAM_MAX_SIZE,
                                addr, len, f->pic_icon[row],
                                NEW_MODE_ICON_W, NEW_MODE_ICON_H,
                                NEW_MODE_ICON_X, row_y);
}

static void new_mode_row_arrow_apply(f_new_mode_t *f, u8 row, s16 row_y)
{
    if (f == NULL || row >= NEW_MODE_ITEM_CNT || f->pic_arrow[row] == NULL) {
        return;
    }
    new_mode_arrow_ram_ensure();
    if (!new_mode_arrow_ram_ready) {
        compo_picturebox_set_visible(f->pic_arrow[row], false);
        return;
    }
    compo_picturebox_set_ram(f->pic_arrow[row], home_ui_colon_ram);
    compo_picturebox_set_size(f->pic_arrow[row], NEW_MODE_NEW_LEFT_W, NEW_MODE_NEW_LEFT_H);
    compo_picturebox_set_pos(f->pic_arrow[row], NEW_MODE_ARROW_X, row_y);
    compo_picturebox_set_visible(f->pic_arrow[row], true);
}
#endif

#if ELUNCHBOX_PANEL_EN
/* forward: defined below in same file with #if ELUNCHBOX_PANEL_EN */
static void new_mode_sel_bg_apply(f_new_mode_t *f);

static void new_mode_row_sel_update(f_new_mode_t *f, u8 row, bool selected)
{
    s16 row_y;

    if (f == NULL || row >= NEW_MODE_ITEM_CNT) {
        return;
    }
    row_y = new_mode_row_y(row);
    if (f->pic_icon[row] != NULL) {
        new_mode_row_icon_apply(f, row, selected, row_y);
    }
    new_mode_label_txt_color(f->txt_label[row],
                             selected ? NEW_MODE_COLOR_SEL : NEW_MODE_COLOR_NOR);
}

static void new_mode_row_apply_state(f_new_mode_t *f, u8 row, bool selected, bool with_arrow, bool with_text)
{
    s16 row_y;

    if (f == NULL || row >= NEW_MODE_ITEM_CNT) {
        return;
    }
    row_y = new_mode_row_y(row);

    if (f->pic_icon[row] != NULL) {
        new_mode_row_icon_apply(f, row, selected, row_y);
    }
    if (with_text && f->txt_label[row] != NULL) {
        new_mode_label_txt_show(f->txt_label[row], row_y, tbl_new_mode_label[row],
                                selected ? NEW_MODE_COLOR_SEL : NEW_MODE_COLOR_NOR);
    } else if (!with_text) {
        new_mode_label_txt_color(f->txt_label[row],
                                 selected ? NEW_MODE_COLOR_SEL : NEW_MODE_COLOR_NOR);
    }
    if (with_arrow && f->pic_arrow[row] != NULL) {
        new_mode_row_arrow_apply(f, row, row_y);
    }
}

static void new_mode_sel_apply_delta(f_new_mode_t *f, u8 prev, u8 new_sel)
{
    if (f == NULL) {
        return;
    }
    new_mode_sel_bg_apply(f);
    if (prev < NEW_MODE_ITEM_CNT && prev != new_sel) {
        new_mode_row_sel_update(f, prev, false);
    }
    if (new_sel < NEW_MODE_ITEM_CNT) {
        new_mode_row_sel_update(f, new_sel, true);
    }
}
#endif

#if ELUNCHBOX_PANEL_EN
static void new_mode_sel_shape_create(compo_form_t *frm)
{
    compo_shape_t *sel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(sel, COMPO_ID_SHAPE_SEL);
    compo_shape_set_radius(sel, 12);
    compo_shape_set_color(sel, NEW_MODE_COLOR_SEL_BG);
    compo_shape_set_visible(sel, false);
}

static void func_new_mode_gpu_detach_before_leave(f_new_mode_t *f)
{
    u8 i;

    if (f == NULL) {
        return;
    }
    WDT_CLR();
    home_ui_gpu_pic_detach_light(f->pic_bt);
    home_ui_gpu_pic_detach_light(f->pic_bat);
    if (f->shape_sel != NULL) {
        compo_shape_set_visible(f->shape_sel, false);
    }
    for (i = 0; i < NEW_MODE_ITEM_CNT; i++) {
        home_ui_gpu_pic_detach_light(f->pic_icon[i]);
        home_ui_gpu_pic_detach_light(f->pic_arrow[i]);
    }
    home_ui_shared_battery_detach_pic();
    WDT_CLR();
}
#endif

#if ELUNCHBOX_PANEL_EN
static compo_picturebox_t *new_mode_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);

    pic->img = (void *)widget_image_create(frm->page_body, 0);
    pic->radix = 1;
    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

#else
static compo_picturebox_t *new_mode_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);

    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

static void new_mode_pic_apply(compo_picturebox_t *pic, u32 addr, u16 w, u16 h, s16 x, s16 y)
{
    if (pic == NULL) {
        return;
    }
    compo_picturebox_set(pic, addr);
    compo_picturebox_set_size(pic, w, h);
    compo_picturebox_set_pos(pic, x, y);
    compo_picturebox_set_visible(pic, true);
}
#endif

static compo_textbox_t *new_mode_txt_create(compo_form_t *frm, u16 id, u16 buf_size,
                                            s16 x, s16 y, u16 w, u16 h, u16 color)
{
    compo_textbox_t *txt = compo_textbox_create(frm, buf_size);

    compo_setid(txt, id);
    compo_textbox_set_wholewrap(txt, false);
#if ELUNCHBOX_PANEL_EN
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_visible(txt, false);
#endif
    compo_textbox_set_pos(txt, x, y);
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set_location(txt, x, y, w, h);
    return txt;
}

static void new_mode_label_txt_prepare(compo_textbox_t *txt, s16 row_y)
{
    widget_text_t *widget;
    s16 label_y;

    if (txt == NULL) {
        return;
    }
    widget = txt->txt;
    label_y = (s16)(row_y - NEW_MODE_LABEL_H / 2);
    compo_textbox_set_align_center(txt, false);
    widget_set_align_center(widget, false);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_location(txt, NEW_MODE_LABEL_X, label_y, NEW_MODE_LABEL_W, NEW_MODE_LABEL_H);
    compo_textbox_set_visible(txt, false);
}

static void new_mode_label_txt_show(compo_textbox_t *txt, s16 row_y, const char *label, u16 color)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL) {
        return;
    }
    new_mode_label_txt_prepare(txt, row_y);
    widget = txt->txt;
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set(txt, label);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);

    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
    compo_textbox_set_visible(txt, true);
}

static void new_mode_label_txt_color(compo_textbox_t *txt, u16 color)
{
    if (txt != NULL) {
        compo_textbox_set_forecolor(txt, color);
    }
}

static void new_mode_title_txt_prepare(compo_textbox_t *txt)
{
    widget_text_t *widget;

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
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, NEW_MODE_TITLE_Y,
                               NEW_MODE_TITLE_W, NEW_MODE_TITLE_H);
    compo_textbox_set_forecolor(txt, NEW_MODE_COLOR_TITLE);
    compo_textbox_set_visible(txt, false);
}

static void new_mode_title_txt_show(compo_textbox_t *txt)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL) {
        return;
    }
    new_mode_title_txt_prepare(txt);
    widget = txt->txt;
    compo_textbox_set(txt, "MODE");
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);

    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
    compo_textbox_set_visible(txt, true);
}

static void new_mode_bg_create(compo_form_t *frm)
{
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, COMPO_ID_SHAPE_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(bg, NEW_MODE_PANEL_BG);
    compo_shape_set_radius(bg, 0);
}

static void new_mode_panel_create(compo_form_t *frm)
{
    compo_shape_t *panel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(panel, COMPO_ID_SHAPE_PANEL);
    compo_shape_set_location(panel, GUI_SCREEN_CENTER_X, NEW_MODE_PANEL_Y,
                             NEW_MODE_PANEL_W, NEW_MODE_PANEL_H);
    compo_shape_set_color(panel, COLOR_WHITE);
    compo_shape_set_radius(panel, 16);
}

static void new_mode_bind_objects(f_new_mode_t *f)
{
    u8 i;

    if (f == NULL) {
        return;
    }
#if !ELUNCHBOX_PANEL_EN
    new_home_top_time_bind(&f->top_time,
                           COMPO_ID_PIC_TOP_TIME_H10, COMPO_ID_PIC_TOP_TIME_H1,
                           COMPO_ID_PIC_TOP_TIME_COLON, COMPO_ID_PIC_TOP_TIME_M10,
                           COMPO_ID_PIC_TOP_TIME_M1, COMPO_ID_PIC_TOP_TIME_AMPM);
#endif
    f->txt_title = compo_getobj_byid(COMPO_ID_TXT_TITLE);
    f->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);
#if ELUNCHBOX_PANEL_EN
    f->shape_sel = compo_getobj_byid(COMPO_ID_SHAPE_SEL);
#else
    f->pic_sel = compo_getobj_byid(COMPO_ID_SHAPE_SEL);
#endif
    for (i = 0; i < NEW_MODE_ITEM_CNT; i++) {
        f->pic_icon[i] = compo_getobj_byid(COMPO_ID_PIC_ICON_BASE + i);
        f->pic_arrow[i] = compo_getobj_byid(COMPO_ID_PIC_ARROW_BASE + i);
        f->txt_label[i] = compo_getobj_byid(COMPO_ID_TXT_LABEL_BASE + i);
    }
}

static void new_mode_status_refresh(f_new_mode_t *f)
{
    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (f->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f->pic_bt, NEW_HOME_BT_W, NEW_HOME_BT_H);
        compo_picturebox_set_visible(f->pic_bt, true);
    }
    if (f->pic_bat != NULL && gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
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

static void new_mode_sel_bg_apply(f_new_mode_t *f)
{
    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (f->shape_sel == NULL) {
        return;
    }
    compo_shape_set_location(f->shape_sel, GUI_SCREEN_CENTER_X, new_mode_row_y(f->sel),
                             NEW_MODE_NEW_CHAGE_W, NEW_MODE_NEW_CHAGE_H);
    compo_shape_set_visible(f->shape_sel, true);
#else
    if (f->pic_sel == NULL) {
        return;
    }
    new_mode_pic_apply(f->pic_sel, UI_BUF_NEW_UI_NEW_CHAGE_BIN,
                       NEW_MODE_NEW_CHAGE_W, NEW_MODE_NEW_CHAGE_H,
                       GUI_SCREEN_CENTER_X, new_mode_row_y(f->sel));
#endif
}

static void new_mode_list_apply_rows(f_new_mode_t *f, u8 from, u8 to, bool with_title, bool with_text)
{
    u8 i;
#if !ELUNCHBOX_PANEL_EN
    s16 row_y;
#endif

    if (f == NULL || from > to || to >= NEW_MODE_ITEM_CNT) {
        return;
    }

    if (with_text && with_title && f->txt_title != NULL) {
        new_mode_title_txt_show(f->txt_title);
    }

#if ELUNCHBOX_PANEL_EN
    new_mode_sel_bg_apply(f);
#endif

    for (i = from; i <= to; i++) {
#if ELUNCHBOX_PANEL_EN
        new_mode_row_apply_state(f, i, (i == f->sel), true, with_text);
#else
        bool selected = (i == f->sel);

        row_y = new_mode_row_y(i);

        if (f->pic_icon[i] != NULL) {
            new_mode_pic_apply(f->pic_icon[i], new_mode_icon_addr(i, selected),
                               NEW_MODE_ICON_W, NEW_MODE_ICON_H,
                               NEW_MODE_ICON_X, row_y);
        }

        if (with_text && f->txt_label[i] != NULL) {
            new_mode_label_txt_show(f->txt_label[i], row_y, tbl_new_mode_label[i],
                                    selected ? NEW_MODE_COLOR_SEL : NEW_MODE_COLOR_NOR);
        }

        if (f->pic_arrow[i] != NULL) {
            new_mode_pic_apply(f->pic_arrow[i], UI_BUF_NEW_UI_NEW_LEFT_BIN,
                               NEW_MODE_NEW_LEFT_W, NEW_MODE_NEW_LEFT_H,
                               NEW_MODE_ARROW_X, row_y);
        }
#endif
#if ELUNCHBOX_PANEL_EN
        WDT_CLR();
#endif
    }

#if !ELUNCHBOX_PANEL_EN
    if (f->sel >= from && f->sel <= to) {
        new_mode_sel_bg_apply(f);
    }
#endif
}

static void new_mode_list_apply(f_new_mode_t *f)
{
    if (f == NULL) {
        return;
    }
    printf("nm_list_apply sel=%u\n", f->sel);
    new_mode_list_apply_rows(f, 0, (u8)(NEW_MODE_ITEM_CNT - 1), true, true);
}

static void new_mode_ui_refresh(f_new_mode_t *f)
{
    if (f == NULL) {
        return;
    }
    new_mode_list_apply(f);
    f->display_pending = false;
}

static void new_mode_sel_next(f_new_mode_t *f)
{
    u8 prev;

    if (f == NULL) {
        return;
    }
    prev = f->sel;
    f->sel = (u8)((f->sel + 1) % NEW_MODE_ITEM_CNT);
#if ELUNCHBOX_PANEL_EN
    printf("nm_sel %u->%u\n", prev, f->sel);
    new_mode_sel_apply_delta(f, prev, f->sel);
#else
    f->display_pending = true;
#endif
}

static void new_mode_confirm(f_new_mode_t *f)
{
    if (f == NULL || sys_cb.flag_swithing) {
        return;
    }

    printf("nm_confirm sel=%u\n", f->sel);

    switch (f->sel) {
    case NEW_MODE_ITEM_DELAY:
        func_res_allow_switch = 1;
        func_switch_to(FUNC_RESERVATION, FUNC_SWITCH_DIRECT | FUNC_SWITCH_AUTO);
        func_res_allow_switch = 0;
        break;

    case NEW_MODE_ITEM_CHICKEN:
        lb_mode_to_heat_set(2, 212, 1, 0);
        lb_heat_autostart_set(true);
        func_cb.sta = FUNC_HEAT;
        break;

    case NEW_MODE_ITEM_PASTA:
        lb_mode_to_heat_set(3, 194, 1, 0);
        lb_heat_autostart_set(true);
        func_cb.sta = FUNC_HEAT;
        break;

    case NEW_MODE_ITEM_WARM:
        lb_mode_to_heat_set(5, 140, 1, 0);
        lb_heat_autostart_set(true);
        func_cb.sta = FUNC_HEAT;
        break;

    default:
        break;
    }
}

static void new_mode_power_key(void)
{
    if (sys_cb.flag_swithing) {
        return;
    }
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

compo_form_t *func_new_mode_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    compo_picturebox_t *pic;
    s16 bat_x;
    s16 bt_x;
    u8 i;

    new_mode_bg_create(frm);
    new_mode_panel_create(frm);

#if !ELUNCHBOX_PANEL_EN
    new_home_top_time_create(frm, UI_BUF_ICON_ACTIVITY_BIN,
                             COMPO_ID_PIC_TOP_TIME_H10, COMPO_ID_PIC_TOP_TIME_H1,
                             COMPO_ID_PIC_TOP_TIME_COLON, COMPO_ID_PIC_TOP_TIME_M10,
                             COMPO_ID_PIC_TOP_TIME_M1, COMPO_ID_PIC_TOP_TIME_AMPM);
#endif
    /* ELUNCHBOX：暂不创建顶栏时钟 picturebox（与 Home 共用 RAM，易 C245）；仅保留 MODE 标题+状态栏+列表 */

    txt = new_mode_txt_create(frm, COMPO_ID_TXT_TITLE, 8,
                              GUI_SCREEN_CENTER_X, NEW_MODE_TITLE_Y,
                              NEW_MODE_TITLE_W, NEW_MODE_TITLE_H,
                              NEW_MODE_COLOR_TITLE);
    new_mode_title_txt_prepare(txt);

    bat_x = (s16)(GUI_SCREEN_WIDTH - NEW_MODE_STATUS_RIGHT_MARGIN - NEW_HOME_BAT_W / 2);
    bt_x = (s16)(bat_x - NEW_HOME_BAT_W / 2 - NEW_MODE_STATUS_GAP - NEW_HOME_BT_W / 2);

    pic = new_mode_pic_create_hidden(frm, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, bt_x, NEW_MODE_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);

    pic = new_mode_pic_create_hidden(frm, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, bat_x, NEW_MODE_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);

#if ELUNCHBOX_PANEL_EN
    new_mode_sel_shape_create(frm);
#else
    (void)new_mode_pic_create_hidden(frm, COMPO_ID_SHAPE_SEL);
#endif

    for (i = 0; i < NEW_MODE_ITEM_CNT; i++) {
        s16 row_y = new_mode_row_y(i);

        (void)new_mode_pic_create_hidden(frm, (u16)(COMPO_ID_PIC_ICON_BASE + i));
        (void)new_mode_pic_create_hidden(frm, (u16)(COMPO_ID_PIC_ARROW_BASE + i));
        new_mode_label_txt_prepare(
            new_mode_txt_create(frm, (u16)(COMPO_ID_TXT_LABEL_BASE + i), 12,
                                NEW_MODE_LABEL_X, row_y, NEW_MODE_LABEL_W, NEW_MODE_LABEL_H,
                                NEW_MODE_COLOR_NOR),
            row_y);
    }

    return frm;
}

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static void new_mode_pt8028_keys_process(f_new_mode_t *f)
{
    u8 press_tch;

    if (f == NULL || !f->key_ready) {
        return;
    }
    press_tch = pt8028_take_press_tch();
    if (press_tch == 0xff) {
        return;
    }
    printf("nm_key press tch=%u\n", press_tch);
    if (press_tch <= PT8028_KEY_TCH6 && press_tch != PT8028_KEY_TCH4) {
        elunchbox_user_activity_reset();
    }
    if (func_key_lock_filter_tch(press_tch)) {
        return;
    }
    if (press_tch == PT8028_KEY_TCH3) {
        printf("nm_key TCH3 mode\n");
        new_mode_sel_next(f);
    } else if (press_tch == PT8028_KEY_TCH4) {
        printf("nm_key TCH4 confirm\n");
        new_mode_confirm(f);
    } else if (press_tch == PT8028_KEY_TCH5) {
        printf("nm_key TCH5 power\n");
        new_mode_power_key();
    }
}

/* 须在 func_process() 之前调用：func.c 内也会 take_press_tch，顺序反了会丢键 */
static void new_mode_keys_poll(f_new_mode_t *f)
{
    if (f == NULL || !f->key_ready) {
        return;
    }
    pt8028_gpio_ensure_periodic();
    pt8028_key_scan();
    new_mode_pt8028_keys_process(f);
}
#endif

static void func_new_mode_message(size_msg_t msg)
{
    f_new_mode_t *f = (f_new_mode_t *)func_cb.f_cb;

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
    /* PT8028 走 new_mode_pt8028_keys_process，勿重复处理消息队列 */
    switch (msg) {
    case KU_MODE:
    case KU_BACK:
    case KEY_RIGHT | KEY_SHORT_UP:
        return;
    default:
        break;
    }
#endif
    switch (msg) {
    case KU_MODE:
        new_mode_sel_next(f);
        break;
    case KU_BACK:
        new_mode_confirm(f);
        break;
    case KEY_RIGHT | KEY_SHORT_UP:
        new_mode_power_key();
        break;
    default:
        func_message(msg);
        break;
    }
}

static void func_new_mode_process(void)
{
    f_new_mode_t *f = (f_new_mode_t *)func_cb.f_cb;
#if !ELUNCHBOX_PANEL_EN
    tm_t tm;
#endif

    if (f == NULL) {
        func_process();
        return;
    }

#if ELUNCHBOX_PANEL_EN
    if (!f->key_ready) {
        if (f->display_pending) {
            printf("nm_p0 ui_apply\n");
            home_gpu_wait_idle();
            WDT_CLR();
            new_mode_list_apply(f);
            f->display_pending = false;
        }
        printf("nm_p1 first_frame te_block=%u\n", elunchbox_te_block_flag);
        func_process();
        func_home_drain_stale_key_msgs();
        pt8028_release_clear();
        /* release_clear 清 press_emitted，若 TCH4 仍按住，下一帧 key_scan 会重新 emit_press。
         * 此处主动扫一次 + take 消费这个残留按下，避免循环等待阻塞 GUI 线程。 */
        pt8028_gpio_ensure_periodic();
        pt8028_key_scan();
        {
            u8 stale = pt8028_take_press_tch();

            if (stale != 0xff) {
                printf("nm_p1 stale tch=%u consumed\n", stale);
            }
        }
        f->key_ready = true;
        printf("nm_p3 key_ready=1\n");
        return;
    }
#endif

    if (f->display_pending) {
        new_mode_ui_refresh(f);
    }
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    new_mode_keys_poll(f);
#endif
#if !ELUNCHBOX_PANEL_EN
    tm = rtc_clock_get();
    if (f->last_top_min != tm.min || f->last_top_sec != tm.sec) {
        f->last_top_min = tm.min;
        f->last_top_sec = tm.sec;
        new_home_top_time_refresh(&f->top_time, &tm);
    }
#endif
    func_process();
}

void func_new_mode_enter(void)
{
    f_new_mode_t *f;

    printf("func_new_mode_enter te_block=%u\n", elunchbox_te_block_flag);

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(1);
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
    printf("nm_e0 keys drained\n");
#endif

#if ELUNCHBOX_PANEL_EN
    WDT_CLR();
#endif

    msg_queue_detach(KU_BACK, 0);
    msg_queue_detach(KU_MODE, 0);
    msg_queue_detach(KU_RIGHT, 0);

    func_cb.f_cb = func_zalloc(sizeof(f_new_mode_t));
    f = (f_new_mode_t *)func_cb.f_cb;
    f->sel = NEW_MODE_ITEM_DELAY;
#if ELUNCHBOX_PANEL_EN
    f->key_ready = false;
    new_mode_arrow_ram_ready = false;
#endif

    printf("nm_e1 form_create\n");
    func_cb.frm_main = func_new_mode_form_create();
    new_mode_bind_objects(f);

#if ELUNCHBOX_PANEL_EN
    home_ui_digit_pool_reset();
    printf("nm_e2 status_init\n");
    home_ui_shared_status_init();
    new_mode_status_refresh(f);
    WDT_CLR();
    f->display_pending = true;

    printf("nm_e4 gpu_wait\n");
    home_gpu_wait_idle();
    WDT_CLR();
    elunchbox_te_block_flag = 0;
    tft_bglight_force_on();
    printf("func_new_mode_enter: ok te_block=0 pending=1\n");
#else
    f->top_time.last_key = 0xffff;
    f->last_top_min = 0xff;
    f->last_top_sec = 0xff;
    home_ui_shared_status_init();
    new_mode_status_refresh(f);
    {
        tm_t tm = rtc_clock_get();
        f->last_top_min = tm.min;
        f->last_top_sec = tm.sec;
        new_home_top_time_refresh(&f->top_time, &tm);
    }
    new_mode_list_apply(f);
    f->display_pending = false;
#endif
}

void func_new_mode_exit(void)
{
#if ELUNCHBOX_PANEL_EN
    f_new_mode_t *f = (f_new_mode_t *)func_cb.f_cb;

    if (f != NULL) {
        func_new_mode_gpu_detach_before_leave(f);
    } else {
        home_ui_shared_battery_detach_pic();
    }
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(0);
    pt8028_release_clear();
#endif
    func_cb.last = FUNC_NEW_MODE;
    printf("func_new_mode_exit\n");
}

void func_new_mode(void)
{
    printf("func_new_mode run\n");
    func_new_mode_enter();
    while (func_cb.sta == FUNC_NEW_MODE) {
        func_new_mode_process();
        func_new_mode_message(msg_dequeue());
    }
    func_new_mode_exit();
}
