#include "include.h"
#include "func.h"
#include "new_setup_res.h"
#include "new_home_icon_res.h"
#include "home_ui_shared.h"
#include "home_ui_gpu_detach.h"
#include "home_ui_ram.h"
#include "func_key_lock.h"

#if ELUNCHBOX_PANEL_EN
extern volatile u8 elunchbox_te_block_flag;
#endif

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

#ifndef UI_BUF_NEW_UI_NEW_TIMEING_0_BIN
#error "Run tools/gen_new_setup_icons.py then Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_HOME_LEFT_BIN
#error "Missing left.bin: run tools/convert_ui_home.bat + Output/bin/prebuild.bat"
#endif

/*
 * 设置页 — 效果图 SETUP
 *   顶栏：SETUP 标题 + 蓝牙/电量
 *   白底圆角卡片：Time / Language / Ver. Info.
 *   模式键：循环选中；确认键：进入子页；电源键：回 Home
 */
#define NEW_SETUP_STATUS_Y                 20
#define NEW_SETUP_STATUS_RIGHT_MARGIN      10
#define NEW_SETUP_STATUS_GAP               6
#define NEW_SETUP_TITLE_Y                  38
#define NEW_SETUP_TITLE_W                  120
#define NEW_SETUP_TITLE_H                  36

#define NEW_SETUP_PANEL_Y                  132
#define NEW_SETUP_PANEL_W                  280
#define NEW_SETUP_PANEL_H                  150

#define NEW_SETUP_ROW_FIRST_Y              95
#define NEW_SETUP_ROW_GAP                  42
#define NEW_SETUP_ICON_X                   36
#define NEW_SETUP_LABEL_GAP                12
#define NEW_SETUP_ARROW_X                  298
#define NEW_SETUP_LABEL_X                  (NEW_SETUP_ICON_X + NEW_SETUP_ICON_W / 2 + NEW_SETUP_LABEL_GAP)
#define NEW_SETUP_LABEL_H                  36
#define NEW_SETUP_LABEL_W                  ((s16)(NEW_SETUP_ARROW_X - NEW_SETUP_ARROW_W / 2 - 10 - NEW_SETUP_LABEL_X))

#define NEW_SETUP_COLOR_TITLE              COLOR_BLACK
#define NEW_SETUP_COLOR_SEL                0x0AD8
#define NEW_SETUP_COLOR_NOR                COLOR_BLACK
#define NEW_SETUP_COLOR_SEL_BG             0xCFDF
#define NEW_SETUP_PANEL_BG                 0xEF5D
#define NEW_SETUP_COLOR_DIVIDER            0xCE79

#define NEW_SETUP_MSG_OK                   KU_BACK
#define NEW_SETUP_MSG_MODE                 KU_MODE
#define NEW_SETUP_MSG_POWER                (KEY_RIGHT | KEY_SHORT_UP)

enum {
    NEW_SETUP_ITEM_TIME = 0,
    NEW_SETUP_ITEM_LANGUAGE,
    NEW_SETUP_ITEM_VERINFO,
    NEW_SETUP_ITEM_CNT,
};

enum {
    COMPO_ID_SHAPE_BG = 1,
    COMPO_ID_SHAPE_PANEL,
    COMPO_ID_TXT_TITLE,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_BAT,
    COMPO_ID_SHAPE_SEL,
    COMPO_ID_SHAPE_DIVIDER,
    COMPO_ID_PIC_ICON_BASE = 30,
    COMPO_ID_PIC_ARROW_BASE = 40,
    COMPO_ID_TXT_LABEL_BASE = 50,
};

typedef struct {
    u8 sel;
    bool display_pending;
#if ELUNCHBOX_PANEL_EN
    bool key_ready;
    bool text_pending;
#endif
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_bat;
    compo_shape_t *shape_sel;
    compo_textbox_t *txt_title;
    compo_picturebox_t *pic_icon[NEW_SETUP_ITEM_CNT];
    compo_picturebox_t *pic_arrow[NEW_SETUP_ITEM_CNT];
    compo_textbox_t *txt_label[NEW_SETUP_ITEM_CNT];
} f_new_setup_t;

static const char * const tbl_new_setup_label[NEW_SETUP_ITEM_CNT] = {
    "Time",
    "Language",
    "Ver. Info.",
};

static const u8 tbl_new_setup_target[NEW_SETUP_ITEM_CNT] = {
    FUNC_TIMEING,
    FUNC_LANGUAGEING,
    FUNC_VERINFO,
};

#if ELUNCHBOX_PANEL_EN
static bool new_setup_arrow_ram_ready;
#endif

static s16 new_setup_row_y(u8 row)
{
    return (s16)(NEW_SETUP_ROW_FIRST_Y + (s16)row * NEW_SETUP_ROW_GAP);
}

static s16 new_setup_divider_y(void)
{
    return (s16)((new_setup_row_y(NEW_SETUP_ITEM_LANGUAGE) +
                  new_setup_row_y(NEW_SETUP_ITEM_VERINFO)) / 2);
}

static void new_setup_label_txt_prepare(compo_textbox_t *txt, s16 row_y);
static void new_setup_label_txt_show(compo_textbox_t *txt, s16 row_y, const char *label, u16 color);
static void new_setup_label_txt_color(compo_textbox_t *txt, u16 color);
static void new_setup_title_txt_prepare(compo_textbox_t *txt);
static void new_setup_title_txt_show(compo_textbox_t *txt);

static u32 new_setup_icon_addr(u8 item, bool selected)
{
    static const u32 tbl[NEW_SETUP_ITEM_CNT][2] = {
        { UI_BUF_NEW_UI_NEW_TIMEING_0_BIN,   UI_BUF_NEW_UI_NEW_TIMEING_1_BIN },
        { UI_BUF_NEW_UI_NEW_LANGUAGE_0_BIN,  UI_BUF_NEW_UI_NEW_LANGUAGE_1_BIN },
        { UI_BUF_NEW_UI_NEW_INFO_0_BIN,      UI_BUF_NEW_UI_NEW_INFO_1_BIN },
    };

    if (item >= NEW_SETUP_ITEM_CNT) {
        return tbl[0][0];
    }
    return tbl[item][selected ? 1 : 0];
}

static u16 new_setup_icon_len(u8 item, bool selected)
{
    static const u16 tbl[NEW_SETUP_ITEM_CNT][2] = {
        { UI_LEN_NEW_UI_NEW_TIMEING_0_BIN,   UI_LEN_NEW_UI_NEW_TIMEING_1_BIN },
        { UI_LEN_NEW_UI_NEW_LANGUAGE_0_BIN,  UI_LEN_NEW_UI_NEW_LANGUAGE_1_BIN },
        { UI_LEN_NEW_UI_NEW_INFO_0_BIN,      UI_LEN_NEW_UI_NEW_INFO_1_BIN },
    };

    if (item >= NEW_SETUP_ITEM_CNT) {
        return tbl[0][0];
    }
    return tbl[item][selected ? 1 : 0];
}

#if ELUNCHBOX_PANEL_EN
static bool new_setup_gpu_ram_bind(const char *tag, u8 *ram, u16 buf_size, u32 addr, u16 len,
                                   compo_picturebox_t *pic, u16 w, u16 h, s16 x, s16 y)
{
    u16 need;

    if (pic == NULL || addr == 0 || len == 0) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }
    if (ram == NULL || len > buf_size) {
        compo_picturebox_set_visible(pic, false);
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
    (void)tag;
    return true;
}

static void new_setup_arrow_ram_ensure(void)
{
    if (new_setup_arrow_ram_ready) {
        return;
    }
    if (UI_LEN_NEW_UI_NEW_LEFT_BIN > HOME_COLON_RAM_SIZE) {
        return;
    }
    WDT_CLR();
    os_spiflash_read(home_ui_colon_ram, UI_BUF_NEW_UI_NEW_LEFT_BIN,
                     UI_LEN_NEW_UI_NEW_LEFT_BIN);
    if (gui_set_ram_check(home_ui_colon_ram, __func__)) {
        new_setup_arrow_ram_ready = true;
    }
}

static void new_setup_row_icon_apply(f_new_setup_t *f, u8 row, bool selected, s16 row_y)
{
    u32 addr;
    u16 len;
    char tag[8];

    if (f == NULL || row >= NEW_SETUP_ITEM_CNT || f->pic_icon[row] == NULL) {
        return;
    }
    addr = new_setup_icon_addr(row, selected);
    len = new_setup_icon_len(row, selected);
    tag[0] = 'i';
    tag[1] = '0' + row;
    tag[2] = selected ? 's' : 'n';
    tag[3] = '\0';
    (void)new_setup_gpu_ram_bind(tag, home_ui_digit_ram[row], HOME_DIGIT_RAM_MAX_SIZE,
                                 addr, len, f->pic_icon[row],
                                 NEW_SETUP_ICON_W, NEW_SETUP_ICON_H,
                                 NEW_SETUP_ICON_X, row_y);
}

static void new_setup_row_arrow_apply(f_new_setup_t *f, u8 row, s16 row_y)
{
    if (f == NULL || row >= NEW_SETUP_ITEM_CNT || f->pic_arrow[row] == NULL) {
        return;
    }
    new_setup_arrow_ram_ensure();
    if (!new_setup_arrow_ram_ready) {
        compo_picturebox_set_visible(f->pic_arrow[row], false);
        return;
    }
    compo_picturebox_set_ram(f->pic_arrow[row], home_ui_colon_ram);
    compo_picturebox_set_size(f->pic_arrow[row], NEW_SETUP_ARROW_W, NEW_SETUP_ARROW_H);
    compo_picturebox_set_pos(f->pic_arrow[row], NEW_SETUP_ARROW_X, row_y);
    compo_picturebox_set_visible(f->pic_arrow[row], true);
}

static void new_setup_sel_bg_apply(f_new_setup_t *f);

static void new_setup_row_sel_update(f_new_setup_t *f, u8 row, bool selected)
{
    s16 row_y;

    if (f == NULL || row >= NEW_SETUP_ITEM_CNT) {
        return;
    }
    row_y = new_setup_row_y(row);
    if (f->pic_icon[row] != NULL) {
        new_setup_row_icon_apply(f, row, selected, row_y);
    }
    new_setup_label_txt_color(f->txt_label[row],
                              selected ? NEW_SETUP_COLOR_SEL : NEW_SETUP_COLOR_NOR);
}

static void new_setup_row_apply_state(f_new_setup_t *f, u8 row, bool selected, bool with_arrow, bool with_text)
{
    s16 row_y;

    if (f == NULL || row >= NEW_SETUP_ITEM_CNT) {
        return;
    }
    row_y = new_setup_row_y(row);

    if (f->pic_icon[row] != NULL) {
        new_setup_row_icon_apply(f, row, selected, row_y);
    }
    if (with_text && f->txt_label[row] != NULL) {
        new_setup_label_txt_show(f->txt_label[row], row_y, tbl_new_setup_label[row],
                                 selected ? NEW_SETUP_COLOR_SEL : NEW_SETUP_COLOR_NOR);
    } else if (!with_text) {
        new_setup_label_txt_color(f->txt_label[row],
                                  selected ? NEW_SETUP_COLOR_SEL : NEW_SETUP_COLOR_NOR);
    }
    if (with_arrow && f->pic_arrow[row] != NULL) {
        new_setup_row_arrow_apply(f, row, row_y);
    }
}

static void new_setup_sel_apply_delta(f_new_setup_t *f, u8 prev, u8 new_sel)
{
    if (f == NULL) {
        return;
    }
    new_setup_sel_bg_apply(f);
    if (prev < NEW_SETUP_ITEM_CNT && prev != new_sel) {
        new_setup_row_sel_update(f, prev, false);
    }
    if (new_sel < NEW_SETUP_ITEM_CNT) {
        new_setup_row_sel_update(f, new_sel, true);
    }
}

static void new_setup_sel_shape_create(compo_form_t *frm)
{
    compo_shape_t *sel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(sel, COMPO_ID_SHAPE_SEL);
    compo_shape_set_radius(sel, 12);
    compo_shape_set_color(sel, NEW_SETUP_COLOR_SEL_BG);
    compo_shape_set_visible(sel, false);
}

static void new_setup_gpu_detach_before_leave(f_new_setup_t *f)
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
    for (i = 0; i < NEW_SETUP_ITEM_CNT; i++) {
        home_ui_gpu_pic_detach_light(f->pic_icon[i]);
        home_ui_gpu_pic_detach_light(f->pic_arrow[i]);
    }
    home_ui_shared_battery_detach_pic();
    WDT_CLR();
}

static compo_picturebox_t *new_setup_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);

    pic->img = (void *)widget_image_create(frm->page_body, 0);
    pic->radix = 1;
    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

#else
static compo_picturebox_t *new_setup_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);

    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

static void new_setup_pic_apply(compo_picturebox_t *pic, u32 addr, u16 w, u16 h, s16 x, s16 y)
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

static compo_textbox_t *new_setup_txt_create(compo_form_t *frm, u16 id, u16 buf_size,
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

static void new_setup_label_txt_prepare(compo_textbox_t *txt, s16 row_y)
{
    widget_text_t *widget;
    s16 label_y;

    if (txt == NULL) {
        return;
    }
    widget = txt->txt;
    label_y = (s16)(row_y - NEW_SETUP_LABEL_H / 2);
    compo_textbox_set_align_center(txt, false);
    widget_set_align_center(widget, false);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_location(txt, NEW_SETUP_LABEL_X, label_y,
                               NEW_SETUP_LABEL_W, NEW_SETUP_LABEL_H);
    compo_textbox_set_visible(txt, false);
}

static void new_setup_label_txt_show(compo_textbox_t *txt, s16 row_y, const char *label, u16 color)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL) {
        return;
    }
    new_setup_label_txt_prepare(txt, row_y);
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

static void new_setup_label_txt_color(compo_textbox_t *txt, u16 color)
{
    if (txt != NULL) {
        compo_textbox_set_forecolor(txt, color);
    }
}

static void new_setup_title_txt_prepare(compo_textbox_t *txt)
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
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, NEW_SETUP_TITLE_Y,
                               NEW_SETUP_TITLE_W, NEW_SETUP_TITLE_H);
    compo_textbox_set_forecolor(txt, NEW_SETUP_COLOR_TITLE);
    compo_textbox_set_visible(txt, false);
}

static void new_setup_title_txt_show(compo_textbox_t *txt)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL) {
        return;
    }
    new_setup_title_txt_prepare(txt);
    widget = txt->txt;
    compo_textbox_set(txt, "SETUP");
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

static void new_setup_bg_create(compo_form_t *frm)
{
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, COMPO_ID_SHAPE_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(bg, NEW_SETUP_PANEL_BG);
    compo_shape_set_radius(bg, 0);
}

static void new_setup_panel_create(compo_form_t *frm)
{
    compo_shape_t *panel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(panel, COMPO_ID_SHAPE_PANEL);
    compo_shape_set_location(panel, GUI_SCREEN_CENTER_X, NEW_SETUP_PANEL_Y,
                             NEW_SETUP_PANEL_W, NEW_SETUP_PANEL_H);
    compo_shape_set_color(panel, COLOR_WHITE);
    compo_shape_set_radius(panel, 16);
}

static void new_setup_divider_create(compo_form_t *frm)
{
    compo_shape_t *line = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(line, COMPO_ID_SHAPE_DIVIDER);
    compo_shape_set_location(line, GUI_SCREEN_CENTER_X, new_setup_divider_y(),
                             (s16)(NEW_SETUP_PANEL_W - 24), 1);
    compo_shape_set_color(line, NEW_SETUP_COLOR_DIVIDER);
    compo_shape_set_radius(line, 0);
}

static void new_setup_bind_objects(f_new_setup_t *f)
{
    u8 i;

    if (f == NULL) {
        return;
    }
    f->txt_title = compo_getobj_byid(COMPO_ID_TXT_TITLE);
    f->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);
#if ELUNCHBOX_PANEL_EN
    f->shape_sel = compo_getobj_byid(COMPO_ID_SHAPE_SEL);
#endif
    for (i = 0; i < NEW_SETUP_ITEM_CNT; i++) {
        f->pic_icon[i] = compo_getobj_byid((u16)(COMPO_ID_PIC_ICON_BASE + i));
        f->pic_arrow[i] = compo_getobj_byid((u16)(COMPO_ID_PIC_ARROW_BASE + i));
        f->txt_label[i] = compo_getobj_byid((u16)(COMPO_ID_TXT_LABEL_BASE + i));
    }
}

static void new_setup_status_refresh(f_new_setup_t *f)
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

static void new_setup_sel_bg_apply(f_new_setup_t *f)
{
    if (f == NULL) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (f->shape_sel == NULL) {
        return;
    }
    compo_shape_set_location(f->shape_sel, GUI_SCREEN_CENTER_X, new_setup_row_y(f->sel),
                             NEW_SETUP_SEL_BG_W, NEW_SETUP_SEL_BG_H);
    compo_shape_set_visible(f->shape_sel, true);
#endif
}

static void new_setup_list_apply_rows(f_new_setup_t *f, u8 from, u8 to, bool with_title, bool with_text)
{
    u8 i;

    if (f == NULL || from > to || to >= NEW_SETUP_ITEM_CNT) {
        return;
    }

    if (with_title && f->txt_title != NULL) {
        new_setup_title_txt_show(f->txt_title);
    }

#if ELUNCHBOX_PANEL_EN
    new_setup_sel_bg_apply(f);
#endif

    for (i = from; i <= to; i++) {
#if ELUNCHBOX_PANEL_EN
        new_setup_row_apply_state(f, i, (i == f->sel), true, with_text);
#else
        bool selected = (i == f->sel);
        s16 row_y = new_setup_row_y(i);

        if (f->pic_icon[i] != NULL) {
            new_setup_pic_apply(f->pic_icon[i], new_setup_icon_addr(i, selected),
                                NEW_SETUP_ICON_W, NEW_SETUP_ICON_H,
                                NEW_SETUP_ICON_X, row_y);
        }
        if (with_text && f->txt_label[i] != NULL) {
            new_setup_label_txt_show(f->txt_label[i], row_y, tbl_new_setup_label[i],
                                     selected ? NEW_SETUP_COLOR_SEL : NEW_SETUP_COLOR_NOR);
        }
        if (f->pic_arrow[i] != NULL) {
            new_setup_pic_apply(f->pic_arrow[i], UI_BUF_NEW_UI_NEW_LEFT_BIN,
                                NEW_SETUP_ARROW_W, NEW_SETUP_ARROW_H,
                                NEW_SETUP_ARROW_X, row_y);
        }
#endif
#if ELUNCHBOX_PANEL_EN
        WDT_CLR();
#endif
    }

}

static void new_setup_list_apply(f_new_setup_t *f)
{
    if (f == NULL) {
        return;
    }
    new_setup_list_apply_rows(f, 0, (u8)(NEW_SETUP_ITEM_CNT - 1), true, true);
}

#if ELUNCHBOX_PANEL_EN
/* 首帧仅绑图标/箭头（Flash→RAM），文字延后到 te_block=0 的下一帧，避免 gui thread miss */
static void new_setup_icons_apply(f_new_setup_t *f)
{
    if (f == NULL) {
        return;
    }
    new_setup_status_refresh(f);
    new_setup_list_apply_rows(f, 0, (u8)(NEW_SETUP_ITEM_CNT - 1), false, false);
}

static void new_setup_text_apply(f_new_setup_t *f)
{
    u8 i;

    if (f == NULL) {
        return;
    }
    if (f->txt_title != NULL) {
        new_setup_title_txt_show(f->txt_title);
    }
    for (i = 0; i < NEW_SETUP_ITEM_CNT; i++) {
        if (f->txt_label[i] != NULL) {
            new_setup_label_txt_show(f->txt_label[i], new_setup_row_y(i),
                                     tbl_new_setup_label[i],
                                     (i == f->sel) ? NEW_SETUP_COLOR_SEL : NEW_SETUP_COLOR_NOR);
        }
    }
}
#endif

static void new_setup_ui_refresh(f_new_setup_t *f)
{
    if (f == NULL) {
        return;
    }
    new_setup_list_apply(f);
    f->display_pending = false;
}

static void new_setup_sel_next(f_new_setup_t *f)
{
    u8 prev;

    if (f == NULL) {
        return;
    }
    prev = f->sel;
    f->sel = (u8)((f->sel + 1) % NEW_SETUP_ITEM_CNT);
#if ELUNCHBOX_PANEL_EN
    new_setup_sel_apply_delta(f, prev, f->sel);
#else
    f->display_pending = true;
#endif
}

static void new_setup_confirm(f_new_setup_t *f)
{
    if (f == NULL || sys_cb.flag_swithing || f->sel >= NEW_SETUP_ITEM_CNT) {
        return;
    }
    func_switch_to(tbl_new_setup_target[f->sel], FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

static void new_setup_power_key(void)
{
    if (sys_cb.flag_swithing) {
        return;
    }
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

compo_form_t *func_new_setup_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    compo_picturebox_t *pic;
    s16 bat_x;
    s16 bt_x;
    u8 i;

    new_setup_bg_create(frm);
    new_setup_panel_create(frm);
    new_setup_divider_create(frm);

    txt = new_setup_txt_create(frm, COMPO_ID_TXT_TITLE, 8,
                               GUI_SCREEN_CENTER_X, NEW_SETUP_TITLE_Y,
                               NEW_SETUP_TITLE_W, NEW_SETUP_TITLE_H,
                               NEW_SETUP_COLOR_TITLE);
    new_setup_title_txt_prepare(txt);

    bat_x = (s16)(GUI_SCREEN_WIDTH - NEW_SETUP_STATUS_RIGHT_MARGIN - NEW_HOME_BAT_W / 2);
    bt_x = (s16)(bat_x - NEW_HOME_BAT_W / 2 - NEW_SETUP_STATUS_GAP - NEW_HOME_BT_W / 2);

    pic = new_setup_pic_create_hidden(frm, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, bt_x, NEW_SETUP_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BT_W, NEW_HOME_BT_H);

    pic = new_setup_pic_create_hidden(frm, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, bat_x, NEW_SETUP_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);

#if ELUNCHBOX_PANEL_EN
    new_setup_sel_shape_create(frm);
#endif

    for (i = 0; i < NEW_SETUP_ITEM_CNT; i++) {
        s16 row_y = new_setup_row_y(i);

        (void)new_setup_pic_create_hidden(frm, (u16)(COMPO_ID_PIC_ICON_BASE + i));
        (void)new_setup_pic_create_hidden(frm, (u16)(COMPO_ID_PIC_ARROW_BASE + i));
        new_setup_label_txt_prepare(
            new_setup_txt_create(frm, (u16)(COMPO_ID_TXT_LABEL_BASE + i), 16,
                                 NEW_SETUP_LABEL_X, row_y, NEW_SETUP_LABEL_W, NEW_SETUP_LABEL_H,
                                 NEW_SETUP_COLOR_NOR),
            row_y);
    }

    return frm;
}

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static void new_setup_pt8028_keys_process(f_new_setup_t *f)
{
    u8 press_tch;

    if (f == NULL || !f->key_ready) {
        return;
    }
    press_tch = pt8028_take_press_tch();
    if (press_tch == 0xff) {
        return;
    }
    if (press_tch <= PT8028_KEY_TCH6 && press_tch != PT8028_KEY_TCH4) {
        elunchbox_user_activity_reset();
    }
    if (func_key_lock_filter_tch(press_tch)) {
        return;
    }
    if (press_tch == PT8028_KEY_TCH3) {
        new_setup_sel_next(f);
    } else if (press_tch == PT8028_KEY_TCH4) {
        new_setup_confirm(f);
    } else if (press_tch == PT8028_KEY_TCH5) {
        new_setup_power_key();
    }
}

static void new_setup_keys_poll(f_new_setup_t *f)
{
    if (f == NULL || !f->key_ready) {
        return;
    }
    pt8028_gpio_ensure_periodic();
    pt8028_key_scan();
    new_setup_pt8028_keys_process(f);
}
#endif

static void func_new_setup_message(size_msg_t msg)
{
    f_new_setup_t *f = (f_new_setup_t *)func_cb.f_cb;

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
    case KEY_RIGHT | KEY_SHORT_UP:
        return;
    default:
        break;
    }
#endif
    switch (msg) {
    case KU_MODE:
        new_setup_sel_next(f);
        break;
    case KU_BACK:
        new_setup_confirm(f);
        break;
    case KEY_RIGHT | KEY_SHORT_UP:
        new_setup_power_key();
        break;
    default:
        func_message(msg);
        break;
    }
}

static void func_new_setup_process(void)
{
    f_new_setup_t *f = (f_new_setup_t *)func_cb.f_cb;

    if (f == NULL) {
        func_process();
        return;
    }

#if ELUNCHBOX_PANEL_EN
    if (!f->key_ready) {
        if (f->display_pending) {
            home_gpu_wait_idle();
            WDT_CLR();
            new_setup_icons_apply(f);
            f->display_pending = false;
            f->text_pending = true;
        } else if (f->text_pending) {
            WDT_CLR();
            new_setup_text_apply(f);
            f->text_pending = false;
        }
        func_process();
        func_home_drain_stale_key_msgs();
        pt8028_release_clear();
        pt8028_gpio_ensure_periodic();
        pt8028_key_scan();
        {
            u8 stale = pt8028_take_press_tch();

            if (stale != 0xff) {
                printf("ns_p stale tch=%u\n", stale);
            }
        }
        if (!f->display_pending && !f->text_pending) {
            f->key_ready = true;
        }
        return;
    }
#endif

    if (f->display_pending) {
        new_setup_ui_refresh(f);
    }

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    new_setup_keys_poll(f);
#endif
    func_process();
}

void func_new_setup_enter(void)
{
    f_new_setup_t *f;

    printf("func_new_setup_enter te_block=%u\n", elunchbox_te_block_flag);

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
    msg_queue_detach(KU_RIGHT, 0);

    func_cb.f_cb = func_zalloc(sizeof(f_new_setup_t));
    f = (f_new_setup_t *)func_cb.f_cb;
#if ELUNCHBOX_PANEL_EN
    f->key_ready = false;
    f->text_pending = false;
    new_setup_arrow_ram_ready = false;
#endif
    f->display_pending = true;

    func_cb.frm_main = func_new_setup_form_create();
    new_setup_bind_objects(f);

#if ELUNCHBOX_PANEL_EN
    home_ui_digit_pool_reset();
    home_ui_shared_status_init();
    WDT_CLR();
    /* 不在 enter 里读 Flash/设文字；首帧 process(te_block=0) 再分步绘制 */
    home_gpu_wait_idle();
    WDT_CLR();
    elunchbox_te_block_flag = 0;
    tft_bglight_force_on();
    printf("func_new_setup_enter: ok te_block=0 pending=1\n");
#else
    new_setup_status_refresh(f);
    new_setup_list_apply(f);
    f->display_pending = false;
#endif
}

void func_new_setup_exit(void)
{
#if ELUNCHBOX_PANEL_EN
    f_new_setup_t *f = (f_new_setup_t *)func_cb.f_cb;

    if (f != NULL) {
        new_setup_gpu_detach_before_leave(f);
    } else {
        home_ui_shared_battery_detach_pic();
    }
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(0);
    pt8028_release_clear();
#endif
    func_cb.last = FUNC_NEW_SETUP;
    printf("func_new_setup_exit\n");
}

void func_new_setup(void)
{
    func_new_setup_enter();
    while (func_cb.sta == FUNC_NEW_SETUP) {
        func_new_setup_process();
        func_new_setup_message(msg_dequeue());
    }
    func_new_setup_exit();
}
