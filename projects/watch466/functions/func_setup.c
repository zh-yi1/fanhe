#include "include.h"
#include "func.h"
#include "home_icon_res.h"
#include "home_ui_shared.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * Setup 页 UI（320×240 / 466×466）：返回 + SETUP 标题 + 状态栏；Time / Language / Ver. Info. 列表。
 * 图标 bin 保持原始尺寸，不缩放。
 *
 * 320×240 设计图布局（严格按效果图）：
 *   顶栏 0~44：Y=22 垂直居中 — 返回(11×17) + SETUP 左对齐 + 右上 BT/锁/电量
 *   分隔线 Y=44
 *   列表三行各 60px：行1 Y=46~105，行2 Y=106~165，行3 Y=166~225（选中行 #1A1A1A 底）
 *   行内：图标左距 18，文字 X=60，箭头右距 16；行间分隔线 Y=106/166
 */
#define UI_SETUP_PLACEHOLDER              UI_BUF_ICON_ACTIVITY_BIN

#ifndef UI_BUF_HOME_LEFT_BIN
#error "Missing left.bin: run tools/convert_ui_home.bat + Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_HOME_TIME_BIN
#error "Missing time.bin: run tools/convert_ui_home.bat + Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_HOME_LANGUAGE_BIN
#error "Missing language.bin: run tools/convert_ui_home.bat + Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_HOME_INFO_BIN
#error "Missing info.bin: run tools/convert_ui_home.bat + Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_HOME_RIGHT_BIN
#error "Missing right.bin: run tools/convert_ui_home.bat + Output/bin/prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BLUETOOTH_BIN
#error "Missing bluetooth.bin: run tools/gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_LOCK_BIN
#error "Missing lock.bin: run tools/gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_BATTERY_LEVEL_BIN
#error "Missing battery_level.bin: run tools/gen_home_icons.py + prebuild.bat"
#endif

#define SETUP_COLOR_ROW_BG                make_color(29, 29, 29)
#define SETUP_COLOR_DIVIDER               make_color(60, 60, 60)

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define SETUP_COLOR_ROW_BG_SEL            make_color(26, 26, 26)
#define SETUP_COLOR_DIVIDER_LINE          make_color(51, 51, 51)
#else
#define SETUP_COLOR_ROW_BG_SEL            SETUP_COLOR_ROW_BG
#define SETUP_COLOR_DIVIDER_LINE          SETUP_COLOR_DIVIDER
#endif

#define SETUP_LEFT_W                      11
#define SETUP_LEFT_H                      17
#define SETUP_LEFT_RAM_SIZE               (8 + SETUP_LEFT_W * SETUP_LEFT_H * 2)

#define SETUP_MENU_ICON_W                 28
#define SETUP_MENU_ICON_H                 28
#define SETUP_MENU_ICON_RAM_SIZE          (8 + SETUP_MENU_ICON_W * SETUP_MENU_ICON_H * 2)

#define SETUP_RIGHT_W                     9
#define SETUP_RIGHT_H                     13
#define SETUP_RIGHT_RAM_SIZE              (8 + SETUP_RIGHT_W * SETUP_RIGHT_H * 2)

/* 466×466 参考布局；320×240 横屏按设计图固定坐标 */
#define SETUP_REF_W                       466
#define SETUP_REF_H                       466
#define SETUP_SX(v)                       ((s16)((s32)(v) * GUI_SCREEN_WIDTH / SETUP_REF_W))
#define SETUP_SY(v)                       ((s16)((s32)(v) * GUI_SCREEN_HEIGHT / SETUP_REF_H))

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
/* 顶栏高约 45px，元素 Y=22 垂直居中 */
#define SETUP_HEADER_Y                    22
#define SETUP_STATUS_Y                    22
#define SETUP_STATUS_RIGHT_MARGIN         10
#define SETUP_STATUS_GAP                  6
#define SETUP_BACK_X                      16
#define SETUP_BACK_BTN_X                  6
#define SETUP_BACK_BTN_Y                  4
#define SETUP_BACK_BTN_W                  48
#define SETUP_BACK_BTN_H                  38
#define SETUP_TITLE_LEFT                  28
#define SETUP_TITLE_Y                     22
#define SETUP_TITLE_H                     36
#define SETUP_TITLE_TOP                   (SETUP_TITLE_Y - SETUP_TITLE_H / 2)
#define SETUP_DIVIDER_Y                   44
#define SETUP_LIST_TOP                    46
#define SETUP_LIST_BOTTOM                 226
#define SETUP_ROW_H                       60
#define SETUP_ROW_ICON_LEFT               18
#define SETUP_ROW_ARROW_RIGHT             16
#define SETUP_ROW_TEXT_GAP                14
#else
#define SETUP_HEADER_Y                    SETUP_SY(48)
#define SETUP_STATUS_Y                    SETUP_SY(48)
#define SETUP_STATUS_RIGHT_MARGIN         SETUP_SX(24)
#define SETUP_STATUS_GAP                  SETUP_SX(10)
#define SETUP_BACK_X                      SETUP_SX(36)
#define SETUP_BACK_BTN_X                  SETUP_SX(16)
#define SETUP_BACK_BTN_Y                  SETUP_SY(28)
#define SETUP_BACK_BTN_W                  SETUP_SX(56)
#define SETUP_BACK_BTN_H                  SETUP_SY(40)
#define SETUP_TITLE_X                     SETUP_SX(130)
#define SETUP_TITLE_Y                     SETUP_SY(48)
#define SETUP_TITLE_W                     SETUP_SX(260)
#define SETUP_TITLE_H                     SETUP_SY(36)
#define SETUP_TITLE_TOP                   (SETUP_TITLE_Y - SETUP_TITLE_H / 2)
#define SETUP_DIVIDER_Y                   SETUP_SY(90)
#define SETUP_LIST_TOP                    (SETUP_DIVIDER_Y + SETUP_DIVIDER_H / 2 + SETUP_SY(4))
#define SETUP_LIST_BOTTOM                 (GUI_SCREEN_HEIGHT - SETUP_SY(8))
#define SETUP_ROW_ICON_LEFT               SETUP_SX(36)
#define SETUP_ROW_ARROW_RIGHT             SETUP_SX(24)
#define SETUP_ROW_TEXT_GAP                SETUP_SX(6)
#endif

#define SETUP_DIVIDER_H                   1

#define SETUP_STATUS_BAT_X                (GUI_SCREEN_WIDTH - SETUP_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define SETUP_STATUS_LOCK_X               (SETUP_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - SETUP_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define SETUP_STATUS_BT_X                 (SETUP_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - SETUP_STATUS_GAP - HOME_STATUS_BT_W / 2)

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
/* 标题区延伸至状态栏左侧，避免 "SETUP" 被裁切 */
#define SETUP_TITLE_W                     (SETUP_STATUS_BT_X - HOME_STATUS_BT_W / 2 - SETUP_STATUS_GAP - SETUP_TITLE_LEFT)
#endif

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define SETUP_ROW_STEP                    SETUP_ROW_H
#else
#define SETUP_ROW_H                       ((SETUP_LIST_BOTTOM - SETUP_LIST_TOP) / 3)
#define SETUP_ROW_STEP                    SETUP_ROW_H
#endif
#define SETUP_ROW_ICON_X                  (SETUP_ROW_ICON_LEFT + SETUP_MENU_ICON_W / 2)
#define SETUP_ROW_ARROW_X                 (GUI_SCREEN_WIDTH - SETUP_ROW_ARROW_RIGHT - SETUP_RIGHT_W / 2)
#define SETUP_ROW_TEXT_LEFT               (SETUP_ROW_ICON_LEFT + SETUP_MENU_ICON_W + SETUP_ROW_TEXT_GAP)
#define SETUP_ROW_TEXT_RIGHT              (SETUP_ROW_ARROW_X - SETUP_RIGHT_W / 2 - SETUP_ROW_TEXT_GAP)
#define SETUP_ROW_LABEL_W                 (SETUP_ROW_TEXT_RIGHT - SETUP_ROW_TEXT_LEFT)
#define SETUP_MSG_OK                      KU_BACK
#define SETUP_MSG_POWER                   (KEY_RIGHT | KEY_SHORT_UP)

static void func_setup_config_title(compo_textbox_t *txt)
{
    widget_text_t *widget = txt->txt;
    rect_t rect;
    area_t text_area;

    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
    compo_textbox_set_align_center(txt, false);
    widget_set_align_center(widget, false);
    compo_textbox_set_location(txt, SETUP_TITLE_LEFT, SETUP_TITLE_TOP,
                               SETUP_TITLE_W, SETUP_TITLE_H);
#else
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_location(txt, SETUP_TITLE_X, SETUP_TITLE_TOP,
                               SETUP_TITLE_W, SETUP_TITLE_H);
#endif
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, "SETUP");

    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
}

static void func_setup_config_row_label(compo_textbox_t *txt, const char *text, s16 row_y)
{
    widget_text_t *widget = txt->txt;
    s16 text_x = SETUP_ROW_TEXT_LEFT;
    s16 text_y = row_y - SETUP_ROW_H / 2;
    rect_t rect;
    area_t text_area;

    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_align_center(txt, false);
    widget_set_align_center(widget, false);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_location(txt, text_x, text_y, SETUP_ROW_LABEL_W, SETUP_ROW_H);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, text);

    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
}

enum {
    SETUP_ROW_TIME = 0,
    SETUP_ROW_LANGUAGE,
    SETUP_ROW_VERINFO,
    SETUP_ROW_CNT,
};

enum {
    COMPO_ID_PIC_BACK = 1,
    COMPO_ID_BTN_BACK,
    COMPO_ID_TITLE,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_LOCK,
    COMPO_ID_PIC_BAT,
    COMPO_ID_HEADER_DIVIDER,
    COMPO_ID_ROW0_BG,
    COMPO_ID_ROW0_ICON,
    COMPO_ID_ROW0_LABEL,
    COMPO_ID_ROW0_ARROW,
    COMPO_ID_ROW0_BTN,
    COMPO_ID_ROW0_DIVIDER,
    COMPO_ID_ROW1_BG,
    COMPO_ID_ROW1_ICON,
    COMPO_ID_ROW1_LABEL,
    COMPO_ID_ROW1_ARROW,
    COMPO_ID_ROW1_BTN,
    COMPO_ID_ROW1_DIVIDER,
    COMPO_ID_ROW2_BG,
    COMPO_ID_ROW2_ICON,
    COMPO_ID_ROW2_LABEL,
    COMPO_ID_ROW2_ARROW,
    COMPO_ID_ROW2_BTN,
};

typedef struct setup_row_ui_t_ {
    compo_shape_t *bg;
    compo_shape_t *divider;
    compo_picturebox_t *icon;
    compo_textbox_t *label;
    compo_picturebox_t *arrow;
    compo_button_t *btn;
} setup_row_ui_t;

typedef struct f_setup_t_ {
    u8 focus;
    bool screen_locked;
    compo_picturebox_t *pic_back;
    compo_button_t *btn_back;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
    setup_row_ui_t rows[SETUP_ROW_CNT];
} f_setup_t;

static u8 setup_left_ram[SETUP_LEFT_RAM_SIZE];
static u8 setup_menu_icon_ram[SETUP_ROW_CNT][SETUP_MENU_ICON_RAM_SIZE];
static u8 setup_right_ram[SETUP_RIGHT_RAM_SIZE];

static const u16 tbl_setup_row_id_bg[SETUP_ROW_CNT] = {
    COMPO_ID_ROW0_BG,
    COMPO_ID_ROW1_BG,
    COMPO_ID_ROW2_BG,
};

static const u16 tbl_setup_row_id_icon[SETUP_ROW_CNT] = {
    COMPO_ID_ROW0_ICON,
    COMPO_ID_ROW1_ICON,
    COMPO_ID_ROW2_ICON,
};

static const u16 tbl_setup_row_id_label[SETUP_ROW_CNT] = {
    COMPO_ID_ROW0_LABEL,
    COMPO_ID_ROW1_LABEL,
    COMPO_ID_ROW2_LABEL,
};

static const u16 tbl_setup_row_id_arrow[SETUP_ROW_CNT] = {
    COMPO_ID_ROW0_ARROW,
    COMPO_ID_ROW1_ARROW,
    COMPO_ID_ROW2_ARROW,
};

static const u16 tbl_setup_row_id_btn[SETUP_ROW_CNT] = {
    COMPO_ID_ROW0_BTN,
    COMPO_ID_ROW1_BTN,
    COMPO_ID_ROW2_BTN,
};

static const u16 tbl_setup_row_id_divider[SETUP_ROW_CNT] = {
    COMPO_ID_ROW0_DIVIDER,
    COMPO_ID_ROW1_DIVIDER,
    0,
};

static const char * const tbl_setup_row_label[SETUP_ROW_CNT] = {
    "Time",
    "Language",
    "Ver. Info.",
};

static const u32 tbl_setup_row_icon_addr[SETUP_ROW_CNT] = {
    UI_BUF_HOME_TIME_BIN,
    UI_BUF_HOME_LANGUAGE_BIN,
    UI_BUF_HOME_INFO_BIN,
};

static const u16 tbl_setup_row_icon_len[SETUP_ROW_CNT] = {
    UI_LEN_HOME_TIME_BIN,
    UI_LEN_HOME_LANGUAGE_BIN,
    UI_LEN_HOME_INFO_BIN,
};

static const u8 tbl_setup_row_func[SETUP_ROW_CNT] = {
    FUNC_TIMEING,
    FUNC_LANGUAGEING,
    FUNC_VERINFO,
};

static s16 func_setup_row_center_y(u8 idx)
{
    return (s16)(SETUP_LIST_TOP + SETUP_ROW_H / 2 + idx * SETUP_ROW_STEP);
}

static s16 func_setup_row_divider_y(u8 idx)
{
    return (s16)(SETUP_LIST_TOP + (idx + 1) * SETUP_ROW_H);
}

static bool func_setup_gpu_ram_set(u8 *ram, u16 buf_size, u16 data_len, compo_picturebox_t *pic)
{
    u32 magic;
    u16 w;
    u16 h;
    u16 need;

    if (ram == NULL || data_len < 8) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    magic = GET_LE32(&ram[0]);
    w = GET_LE16(&ram[4]);
    h = GET_LE16(&ram[6]);
    need = (u16)(8 + (u32)w * h * 2);
    if (magic != 0x24150 || w == 0 || h == 0 || need != data_len || data_len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    if (pic != NULL) {
        compo_picturebox_set_ram(pic, ram);
        compo_picturebox_set_size(pic, w, h);
        compo_picturebox_set_visible(pic, true);
    }
    return true;
}

static bool func_setup_gpu_flash_to_ram(u8 *ram, u16 buf_size, u32 flash_addr, u16 flash_len,
                                        compo_picturebox_t *pic)
{
    if (ram == NULL || flash_len == 0 || flash_len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    os_spiflash_read(ram, flash_addr, flash_len);
    return func_setup_gpu_ram_set(ram, buf_size, flash_len, pic);
}

static compo_shape_t *func_setup_shape_create(compo_form_t *frm, u16 id, s16 x, s16 y,
                                               s16 w, s16 h, u16 color)
{
    compo_shape_t *shape = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(shape, id);
    compo_shape_set_location(shape, x, y, w, h);
    compo_shape_set_color(shape, color);
    compo_shape_set_radius(shape, 0);
    return shape;
}

static void func_setup_lock_icon_apply(f_setup_t *f_setup);

static void func_setup_status_icons_apply(f_setup_t *f_setup)
{
    home_ui_shared_status_init();

    if (f_setup->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f_setup->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f_setup->pic_bt, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
        compo_picturebox_set_visible(f_setup->pic_bt, true);
    }
    if (f_setup->pic_lock != NULL && gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
        compo_picturebox_set_ram(f_setup->pic_lock, home_ui_shared_status_lock_ram);
        compo_picturebox_set_size(f_setup->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    }
    if (f_setup->pic_bat != NULL && gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        compo_picturebox_set_ram(f_setup->pic_bat, home_ui_shared_status_bat_ram);
        compo_picturebox_set_size(f_setup->pic_bat, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
        compo_picturebox_set_visible(f_setup->pic_bat, true);
    }

    func_setup_lock_icon_apply(f_setup);
}

static void func_setup_lock_icon_apply(f_setup_t *f_setup)
{
    if (f_setup == NULL || f_setup->pic_lock == NULL) {
        return;
    }
    if (f_setup->screen_locked && gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
        compo_picturebox_set_ram(f_setup->pic_lock, home_ui_shared_status_lock_ram);
        compo_picturebox_set_size(f_setup->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
        compo_picturebox_set_visible(f_setup->pic_lock, true);
    } else {
        compo_picturebox_set_visible(f_setup->pic_lock, false);
    }
}

static void func_setup_row_icon_ram_apply(u8 *ram, u16 len, bool selected)
{
    u16 i;
    u16 pix_cnt;

    if (len <= 8) {
        return;
    }

    pix_cnt = (len - 8) / 2;
    for (i = 0; i < pix_cnt; i++) {
        u8 *px = &ram[8 + i * 2];

        if (GET_LE16(px) == 0x0000) {
            PUT_LE16(px, selected ? SETUP_COLOR_ROW_BG_SEL : 0x0000);
        }
    }
}

static void func_setup_row_icon_update(f_setup_t *f_setup, u8 idx)
{
    setup_row_ui_t *row = &f_setup->rows[idx];
    bool selected = (idx == f_setup->focus);

    if (row->icon == NULL) {
        return;
    }

    os_spiflash_read(setup_menu_icon_ram[idx], tbl_setup_row_icon_addr[idx], tbl_setup_row_icon_len[idx]);
    func_setup_row_icon_ram_apply(setup_menu_icon_ram[idx], tbl_setup_row_icon_len[idx], selected);
    func_setup_gpu_ram_set(setup_menu_icon_ram[idx], SETUP_MENU_ICON_RAM_SIZE,
                           tbl_setup_row_icon_len[idx], row->icon);
}

static void func_setup_row_focus_refresh(f_setup_t *f_setup)
{
    u8 i;

    for (i = 0; i < SETUP_ROW_CNT; i++) {
        setup_row_ui_t *row = &f_setup->rows[i];
        bool selected = (i == f_setup->focus);

        if (row->bg != NULL) {
            compo_shape_set_visible(row->bg, selected);
        }
        func_setup_row_icon_update(f_setup, i);
    }
}

static void func_setup_row_select_next(f_setup_t *f_setup)
{
    if (f_setup == NULL) {
        return;
    }
    f_setup->focus = (u8)((f_setup->focus + 1) % SETUP_ROW_CNT);
    func_setup_row_focus_refresh(f_setup);
}

static void func_setup_row_enter(f_setup_t *f_setup)
{
    if (f_setup == NULL || f_setup->focus >= SETUP_ROW_CNT) {
        return;
    }
    func_switch_to(tbl_setup_row_func[f_setup->focus], FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

static void func_setup_power_key(f_setup_t *f_setup)
{
    (void)f_setup;
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

static void func_setup_row_create(compo_form_t *frm, u8 idx)
{
    compo_picturebox_t *pic;
    compo_textbox_t *txt;
    compo_button_t *btn;
    compo_shape_t *bg;
    s16 y = func_setup_row_center_y(idx);

    bg = func_setup_shape_create(frm, tbl_setup_row_id_bg[idx], GUI_SCREEN_CENTER_X, y,
                                 GUI_SCREEN_WIDTH, SETUP_ROW_H, SETUP_COLOR_ROW_BG_SEL);
    compo_shape_set_visible(bg, false);

    pic = compo_picturebox_create(frm, UI_SETUP_PLACEHOLDER);
    compo_setid(pic, tbl_setup_row_id_icon[idx]);
    compo_picturebox_set_pos(pic, SETUP_ROW_ICON_X, y);
    compo_picturebox_set_size(pic, SETUP_MENU_ICON_W, SETUP_MENU_ICON_H);
    compo_picturebox_set_visible(pic, false);

    txt = compo_textbox_create(frm, 20);
    compo_setid(txt, tbl_setup_row_id_label[idx]);
    func_setup_config_row_label(txt, tbl_setup_row_label[idx], y);

    pic = compo_picturebox_create(frm, UI_SETUP_PLACEHOLDER);
    compo_setid(pic, tbl_setup_row_id_arrow[idx]);
    compo_picturebox_set_pos(pic, SETUP_ROW_ARROW_X, y);
    compo_picturebox_set_size(pic, SETUP_RIGHT_W, SETUP_RIGHT_H);
    compo_picturebox_set_visible(pic, false);

    if (tbl_setup_row_id_divider[idx] != 0) {
        func_setup_shape_create(frm, tbl_setup_row_id_divider[idx], GUI_SCREEN_CENTER_X,
                                func_setup_row_divider_y(idx), GUI_SCREEN_WIDTH, 1,
                                SETUP_COLOR_DIVIDER_LINE);
    }

    btn = compo_button_create(frm);
    compo_setid(btn, tbl_setup_row_id_btn[idx]);
    compo_button_set_location(btn, GUI_SCREEN_CENTER_X, y, GUI_SCREEN_WIDTH, SETUP_ROW_H);
}

static void func_setup_row_bind(f_setup_t *f_setup, u8 idx)
{
    setup_row_ui_t *row = &f_setup->rows[idx];

    row->bg = compo_getobj_byid(tbl_setup_row_id_bg[idx]);
    row->icon = compo_getobj_byid(tbl_setup_row_id_icon[idx]);
    row->label = compo_getobj_byid(tbl_setup_row_id_label[idx]);
    row->arrow = compo_getobj_byid(tbl_setup_row_id_arrow[idx]);
    row->btn = compo_getobj_byid(tbl_setup_row_id_btn[idx]);
    if (tbl_setup_row_id_divider[idx] != 0) {
        row->divider = compo_getobj_byid(tbl_setup_row_id_divider[idx]);
    }

    func_setup_gpu_flash_to_ram(setup_right_ram, SETUP_RIGHT_RAM_SIZE,
                                UI_BUF_HOME_RIGHT_BIN, UI_LEN_HOME_RIGHT_BIN, row->arrow);
}

static u8 func_setup_row_index_from_btn(int id)
{
    switch (id) {
    case COMPO_ID_ROW0_BTN:
        return SETUP_ROW_TIME;
    case COMPO_ID_ROW1_BTN:
        return SETUP_ROW_LANGUAGE;
    case COMPO_ID_ROW2_BTN:
        return SETUP_ROW_VERINFO;
    default:
        return 0xff;
    }
}

static void func_setup_button_click(f_setup_t *f_setup)
{
    int id = compo_get_button_id();
    u8 row;

    switch (id) {
    case COMPO_ID_BTN_BACK:
        func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    default:
        row = func_setup_row_index_from_btn(id);
        if (row < SETUP_ROW_CNT) {
            f_setup->focus = row;
            func_setup_row_focus_refresh(f_setup);
            func_setup_row_enter(f_setup);
        }
        break;
    }
}

compo_form_t *func_setup_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_picturebox_t *pic;
    compo_textbox_t *txt;
    compo_button_t *btn;
    u8 i;

    pic = compo_picturebox_create(frm, UI_SETUP_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BACK);
    compo_picturebox_set_pos(pic, SETUP_BACK_X, SETUP_HEADER_Y);
    compo_picturebox_set_size(pic, SETUP_LEFT_W, SETUP_LEFT_H);
    compo_picturebox_set_visible(pic, false);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_BACK);
    compo_button_set_location(btn, SETUP_BACK_BTN_X, SETUP_BACK_BTN_Y,
                              SETUP_BACK_BTN_W, SETUP_BACK_BTN_H);

    txt = compo_textbox_create(frm, 16);
    compo_setid(txt, COMPO_ID_TITLE);
    func_setup_config_title(txt);

    pic = compo_picturebox_create(frm, UI_SETUP_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, SETUP_STATUS_BT_X, SETUP_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_SETUP_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_LOCK);
    compo_picturebox_set_pos(pic, SETUP_STATUS_LOCK_X, SETUP_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_SETUP_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, SETUP_STATUS_BAT_X, SETUP_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
    compo_picturebox_set_visible(pic, false);

    func_setup_shape_create(frm, COMPO_ID_HEADER_DIVIDER, GUI_SCREEN_CENTER_X, SETUP_DIVIDER_Y,
                            GUI_SCREEN_WIDTH, SETUP_DIVIDER_H, SETUP_COLOR_DIVIDER_LINE);

    for (i = 0; i < SETUP_ROW_CNT; i++) {
        func_setup_row_create(frm, i);
    }

    return frm;
}

static void func_setup_process(void)
{
    func_process();
}

static void func_setup_message(size_msg_t msg)
{
    f_setup_t *f_setup = (f_setup_t *)func_cb.f_cb;

    if (f_setup != NULL && f_setup->screen_locked) {
        if (msg != SETUP_MSG_POWER && msg != SETUP_MSG_OK && msg != KU_LEFT) {
            return;
        }
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        if (f_setup != NULL) {
            func_setup_button_click(f_setup);
        }
        break;

    case KU_LEFT:
        if (f_setup != NULL) {
            f_setup->screen_locked = !f_setup->screen_locked;
            func_setup_lock_icon_apply(f_setup);
        }
        break;

    case KU_MODE:
        func_setup_row_select_next(f_setup);
        break;

    case SETUP_MSG_OK:
        func_setup_row_enter(f_setup);
        break;

    case SETUP_MSG_POWER:
        func_setup_power_key(f_setup);
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_setup_enter(void)
{
    f_setup_t *f_setup;
    u8 i;

    func_cb.f_cb = func_zalloc(sizeof(f_setup_t));
    func_cb.frm_main = func_setup_form_create();

    f_setup = (f_setup_t *)func_cb.f_cb;
    f_setup->focus = SETUP_ROW_TIME;
    f_setup->screen_locked = false;

    f_setup->pic_back = compo_getobj_byid(COMPO_ID_PIC_BACK);
    f_setup->btn_back = compo_getobj_byid(COMPO_ID_BTN_BACK);
    f_setup->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f_setup->pic_lock = compo_getobj_byid(COMPO_ID_PIC_LOCK);
    f_setup->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);

    func_setup_gpu_flash_to_ram(setup_left_ram, SETUP_LEFT_RAM_SIZE,
                                UI_BUF_HOME_LEFT_BIN, UI_LEN_HOME_LEFT_BIN,
                                f_setup->pic_back);

    func_setup_status_icons_apply(f_setup);

    for (i = 0; i < SETUP_ROW_CNT; i++) {
        func_setup_row_bind(f_setup, i);
    }

    func_setup_row_focus_refresh(f_setup);
}

void func_setup_exit(void)
{
    func_cb.last = FUNC_SETUP;
}

void func_setup(void)
{
    printf("%s\n", __func__);
    func_setup_enter();
    while (func_cb.sta == FUNC_SETUP) {
        func_setup_process();
        func_setup_message(msg_dequeue());
    }
    func_setup_exit();
}
