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
 * Language 页 UI（320×240 / 466×466）：返回 + 标题 + 状态栏；四语列表。
 * 图标 bin 保持原始尺寸，不缩放。
 *
 * 320×240 设计图布局：
 *   顶栏 0~44：Y=22 — 返回(11×17) + "Language" 左对齐 + 右上 BT/锁/电量
 *   分隔线 Y=44
 *   四行各 45px：English / Deutsch / Italiano / Français，文字 X=25，箭头右距 16
 *   选中行 #1A1A1A 底；行间分隔线
 * PT8028：TCH3 模式键循环选中；TCH4 OK 确认语言；TCH5 开关返回 Setup。
 */
#define UI_LANG_PLACEHOLDER               UI_BUF_ICON_ACTIVITY_BIN

#ifndef UI_BUF_HOME_LEFT_BIN
#error "Missing left.bin: run tools/convert_ui_home.bat + Output/bin/prebuild.bat"
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

#define LANG_COLOR_ROW_BG                 make_color(29, 29, 29)
#define LANG_COLOR_DIVIDER                make_color(60, 60, 60)

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define LANG_COLOR_ROW_BG_SEL             make_color(26, 26, 26)
#define LANG_COLOR_DIVIDER_LINE           make_color(51, 51, 51)
#else
#define LANG_COLOR_ROW_BG_SEL             LANG_COLOR_ROW_BG
#define LANG_COLOR_DIVIDER_LINE           LANG_COLOR_DIVIDER
#endif

#define LANG_LEFT_W                       11
#define LANG_LEFT_H                       17
#define LANG_LEFT_RAM_SIZE                (8 + LANG_LEFT_W * LANG_LEFT_H * 2)

#define LANG_RIGHT_W                      9
#define LANG_RIGHT_H                      13
#define LANG_RIGHT_RAM_SIZE               (8 + LANG_RIGHT_W * LANG_RIGHT_H * 2)

#define LANG_REF_W                        466
#define LANG_REF_H                        466
#define LANG_SX(v)                        ((s16)((s32)(v) * GUI_SCREEN_WIDTH / LANG_REF_W))
#define LANG_SY(v)                        ((s16)((s32)(v) * GUI_SCREEN_HEIGHT / LANG_REF_H))

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define LANG_HEADER_Y                     22
#define LANG_STATUS_Y                     22
#define LANG_STATUS_RIGHT_MARGIN          10
#define LANG_STATUS_GAP                   6
#define LANG_BACK_X                       16
#define LANG_BACK_BTN_X                   6
#define LANG_BACK_BTN_Y                   4
#define LANG_BACK_BTN_W                   48
#define LANG_BACK_BTN_H                   38
#define LANG_TITLE_LEFT                   28
#define LANG_TITLE_Y                      22
#define LANG_TITLE_H                      36
#define LANG_TITLE_TOP                    (LANG_TITLE_Y - LANG_TITLE_H / 2)
#define LANG_DIVIDER_Y                    44
#define LANG_DIVIDER_H                    1
#define LANG_LIST_TOP                     46
#define LANG_LIST_BOTTOM                  226
#define LANG_ROW_H                        45
#define LANG_ROW_TEXT_LEFT                25
#define LANG_ROW_ARROW_RIGHT              16
#else
#define LANG_HEADER_Y                     LANG_SY(48)
#define LANG_STATUS_Y                     LANG_SY(48)
#define LANG_STATUS_RIGHT_MARGIN          LANG_SX(24)
#define LANG_STATUS_GAP                   LANG_SX(10)
#define LANG_BACK_X                       LANG_SX(36)
#define LANG_BACK_BTN_X                   LANG_SX(16)
#define LANG_BACK_BTN_Y                   LANG_SY(28)
#define LANG_BACK_BTN_W                   LANG_SX(56)
#define LANG_BACK_BTN_H                   LANG_SY(40)
#define LANG_TITLE_LEFT                   LANG_SX(130)
#define LANG_TITLE_Y                      LANG_SY(48)
#define LANG_TITLE_H                      LANG_SY(36)
#define LANG_TITLE_TOP                    (LANG_TITLE_Y - LANG_TITLE_H / 2)
#define LANG_DIVIDER_Y                    LANG_SY(90)
#define LANG_DIVIDER_H                    1
#define LANG_LIST_TOP                     (LANG_DIVIDER_Y + LANG_DIVIDER_H / 2 + LANG_SY(4))
#define LANG_LIST_BOTTOM                  (GUI_SCREEN_HEIGHT - LANG_SY(8))
#define LANG_ROW_TEXT_LEFT                LANG_SX(36)
#define LANG_ROW_ARROW_RIGHT              LANG_SX(24)
#endif

#define LANG_STATUS_BAT_X                 (GUI_SCREEN_WIDTH - LANG_STATUS_RIGHT_MARGIN - HOME_STATUS_BAT_W / 2)
#define LANG_STATUS_LOCK_X                (LANG_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - LANG_STATUS_GAP - HOME_STATUS_LOCK_W / 2)
#define LANG_STATUS_BT_X                  (LANG_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - LANG_STATUS_GAP - HOME_STATUS_BT_W / 2)

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define LANG_TITLE_W                      (LANG_STATUS_BT_X - HOME_STATUS_BT_W / 2 - LANG_STATUS_GAP - LANG_TITLE_LEFT)
#else
#define LANG_TITLE_W                      LANG_SX(260)
#endif

enum {
    LANG_ROW_ENGLISH = 0,
    LANG_ROW_DEUTSCH,
    LANG_ROW_ITALIANO,
    LANG_ROW_FRANCAIS,
    LANG_ROW_CNT,
};

#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define LANG_ROW_STEP                     LANG_ROW_H
#else
#define LANG_ROW_H                        ((LANG_LIST_BOTTOM - LANG_LIST_TOP) / LANG_ROW_CNT)
#define LANG_ROW_STEP                     LANG_ROW_H
#endif

#define LANG_ROW_ARROW_X                  (GUI_SCREEN_WIDTH - LANG_ROW_ARROW_RIGHT - LANG_RIGHT_W / 2)
#define LANG_ROW_TEXT_RIGHT               (LANG_ROW_ARROW_X - LANG_RIGHT_W / 2 - 12)
#define LANG_ROW_LABEL_W                  (LANG_ROW_TEXT_RIGHT - LANG_ROW_TEXT_LEFT)

#define LANG_MSG_OK                       KU_BACK
#define LANG_MSG_POWER                    (KEY_RIGHT | KEY_SHORT_UP)

enum {
    COMPO_ID_PIC_BACK = 1,
    COMPO_ID_BTN_BACK,
    COMPO_ID_TITLE,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_LOCK,
    COMPO_ID_PIC_BAT,
    COMPO_ID_HEADER_DIVIDER,
    COMPO_ID_ROW0_BG,
    COMPO_ID_ROW0_LABEL,
    COMPO_ID_ROW0_ARROW,
    COMPO_ID_ROW0_BTN,
    COMPO_ID_ROW0_DIVIDER,
    COMPO_ID_ROW1_BG,
    COMPO_ID_ROW1_LABEL,
    COMPO_ID_ROW1_ARROW,
    COMPO_ID_ROW1_BTN,
    COMPO_ID_ROW1_DIVIDER,
    COMPO_ID_ROW2_BG,
    COMPO_ID_ROW2_LABEL,
    COMPO_ID_ROW2_ARROW,
    COMPO_ID_ROW2_BTN,
    COMPO_ID_ROW2_DIVIDER,
    COMPO_ID_ROW3_BG,
    COMPO_ID_ROW3_LABEL,
    COMPO_ID_ROW3_ARROW,
    COMPO_ID_ROW3_BTN,
};

typedef struct lang_row_ui_t_ {
    compo_shape_t *bg;
    compo_shape_t *divider;
    compo_textbox_t *label;
    compo_picturebox_t *arrow;
    compo_button_t *btn;
} lang_row_ui_t;

typedef struct f_languageing_t_ {
    u8 focus;
    bool screen_locked;
    compo_picturebox_t *pic_back;
    compo_button_t *btn_back;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
    lang_row_ui_t rows[LANG_ROW_CNT];
} f_languageing_t;

static u8 lang_left_ram[LANG_LEFT_RAM_SIZE];
static u8 lang_right_ram[LANG_ROW_CNT][LANG_RIGHT_RAM_SIZE];

static const u16 tbl_lang_row_id_bg[LANG_ROW_CNT] = {
    COMPO_ID_ROW0_BG,
    COMPO_ID_ROW1_BG,
    COMPO_ID_ROW2_BG,
    COMPO_ID_ROW3_BG,
};

static const u16 tbl_lang_row_id_label[LANG_ROW_CNT] = {
    COMPO_ID_ROW0_LABEL,
    COMPO_ID_ROW1_LABEL,
    COMPO_ID_ROW2_LABEL,
    COMPO_ID_ROW3_LABEL,
};

static const u16 tbl_lang_row_id_arrow[LANG_ROW_CNT] = {
    COMPO_ID_ROW0_ARROW,
    COMPO_ID_ROW1_ARROW,
    COMPO_ID_ROW2_ARROW,
    COMPO_ID_ROW3_ARROW,
};

static const u16 tbl_lang_row_id_btn[LANG_ROW_CNT] = {
    COMPO_ID_ROW0_BTN,
    COMPO_ID_ROW1_BTN,
    COMPO_ID_ROW2_BTN,
    COMPO_ID_ROW3_BTN,
};

static const u16 tbl_lang_row_id_divider[LANG_ROW_CNT] = {
    COMPO_ID_ROW0_DIVIDER,
    COMPO_ID_ROW1_DIVIDER,
    COMPO_ID_ROW2_DIVIDER,
    0,
};

static const char * const tbl_lang_row_label[LANG_ROW_CNT] = {
    "English",
    "Deutsch",
    "Italiano",
    "Français",
};

static s16 func_languageing_row_center_y(u8 idx)
{
    return (s16)(LANG_LIST_TOP + LANG_ROW_H / 2 + idx * LANG_ROW_STEP);
}

static s16 func_languageing_row_divider_y(u8 idx)
{
    return (s16)(LANG_LIST_TOP + (idx + 1) * LANG_ROW_H);
}

static bool func_languageing_gpu_ram_set(u8 *ram, u16 buf_size, u16 data_len, compo_picturebox_t *pic)
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

static bool func_languageing_gpu_flash_to_ram(u8 *ram, u16 buf_size, u32 flash_addr, u16 flash_len,
                                               compo_picturebox_t *pic)
{
    if (ram == NULL || flash_len == 0 || flash_len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    os_spiflash_read(ram, flash_addr, flash_len);
    return func_languageing_gpu_ram_set(ram, buf_size, flash_len, pic);
}

static compo_shape_t *func_languageing_shape_create(compo_form_t *frm, u16 id, s16 x, s16 y,
                                                    s16 w, s16 h, u16 color)
{
    compo_shape_t *shape = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(shape, id);
    compo_shape_set_location(shape, x, y, w, h);
    compo_shape_set_color(shape, color);
    compo_shape_set_radius(shape, 0);
    return shape;
}

static void func_languageing_config_title(compo_textbox_t *txt)
{
    widget_text_t *widget = txt->txt;
    rect_t rect;
    area_t text_area;

    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
    compo_textbox_set_align_center(txt, false);
    widget_set_align_center(widget, false);
    compo_textbox_set_location(txt, LANG_TITLE_LEFT, LANG_TITLE_TOP,
                               LANG_TITLE_W, LANG_TITLE_H);
#else
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_location(txt, LANG_TITLE_LEFT, LANG_TITLE_TOP,
                               LANG_TITLE_W, LANG_TITLE_H);
#endif
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, "Language");

    rect = widget_get_location(widget);
    text_area = widget_text_get_area(widget);
    if (rect.hei > text_area.hei) {
        widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
    } else {
        widget_text_set_client(widget, 0, 0);
    }
}

static void func_languageing_config_row_label(compo_textbox_t *txt, const char *text, s16 row_y)
{
    widget_text_t *widget = txt->txt;
    s16 text_x = LANG_ROW_TEXT_LEFT;
    s16 text_y = row_y - LANG_ROW_H / 2;
    rect_t rect;
    area_t text_area;

    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_align_center(txt, false);
    widget_set_align_center(widget, false);
    compo_textbox_set_wholewrap(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(widget, false);
    compo_textbox_set_location(txt, text_x, text_y, LANG_ROW_LABEL_W, LANG_ROW_H);
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

static void func_languageing_lock_icon_apply(f_languageing_t *f_lang);

static void func_languageing_status_icons_apply(f_languageing_t *f_lang)
{
    home_ui_shared_status_init();

    if (f_lang->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f_lang->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f_lang->pic_bt, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
        compo_picturebox_set_visible(f_lang->pic_bt, true);
    }
    if (f_lang->pic_lock != NULL && gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
        compo_picturebox_set_ram(f_lang->pic_lock, home_ui_shared_status_lock_ram);
        compo_picturebox_set_size(f_lang->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    }
    if (f_lang->pic_bat != NULL && gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        compo_picturebox_set_ram(f_lang->pic_bat, home_ui_shared_status_bat_ram);
        compo_picturebox_set_size(f_lang->pic_bat, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
        compo_picturebox_set_visible(f_lang->pic_bat, true);
    }

    func_languageing_lock_icon_apply(f_lang);
}

static void func_languageing_lock_icon_apply(f_languageing_t *f_lang)
{
    if (f_lang == NULL || f_lang->pic_lock == NULL) {
        return;
    }
    if (f_lang->screen_locked) {
        home_ui_shared_status_init();
        compo_picturebox_set_pos(f_lang->pic_lock, LANG_STATUS_LOCK_X, LANG_STATUS_Y);
        if (gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
            compo_picturebox_set_ram(f_lang->pic_lock, home_ui_shared_status_lock_ram);
            compo_picturebox_set_size(f_lang->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
            compo_picturebox_set_visible(f_lang->pic_lock, true);
        }
    } else {
        compo_picturebox_set_visible(f_lang->pic_lock, false);
    }
}

static void func_languageing_row_focus_refresh(f_languageing_t *f_lang)
{
    u8 i;

    for (i = 0; i < LANG_ROW_CNT; i++) {
        lang_row_ui_t *row = &f_lang->rows[i];
        bool selected = (i == f_lang->focus);

        if (row->bg != NULL) {
            compo_shape_set_visible(row->bg, selected);
        }
    }
}

static void func_languageing_row_select_next(f_languageing_t *f_lang)
{
    if (f_lang == NULL) {
        return;
    }
    f_lang->focus = (u8)((f_lang->focus + 1) % LANG_ROW_CNT);
    func_languageing_row_focus_refresh(f_lang);
}

static void func_languageing_confirm(f_languageing_t *f_lang)
{
    if (f_lang == NULL || f_lang->focus >= LANG_ROW_CNT) {
        return;
    }
    sys_cb.lang_id = f_lang->focus;
    param_lang_id_write();
    lang_select(sys_cb.lang_id);
    func_switch_to(FUNC_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

static void func_languageing_power_key(f_languageing_t *f_lang)
{
    (void)f_lang;
    func_switch_to(FUNC_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

static void func_languageing_row_create(compo_form_t *frm, u8 idx)
{
    compo_picturebox_t *pic;
    compo_textbox_t *txt;
    compo_button_t *btn;
    compo_shape_t *bg;
    s16 y = func_languageing_row_center_y(idx);

    bg = func_languageing_shape_create(frm, tbl_lang_row_id_bg[idx], GUI_SCREEN_CENTER_X, y,
                                       GUI_SCREEN_WIDTH, LANG_ROW_H, LANG_COLOR_ROW_BG_SEL);
    compo_shape_set_visible(bg, false);

    txt = compo_textbox_create(frm, 20);
    compo_setid(txt, tbl_lang_row_id_label[idx]);
    func_languageing_config_row_label(txt, tbl_lang_row_label[idx], y);

    pic = compo_picturebox_create(frm, UI_LANG_PLACEHOLDER);
    compo_setid(pic, tbl_lang_row_id_arrow[idx]);
    compo_picturebox_set_pos(pic, LANG_ROW_ARROW_X, y);
    compo_picturebox_set_size(pic, LANG_RIGHT_W, LANG_RIGHT_H);
    compo_picturebox_set_visible(pic, false);

    if (tbl_lang_row_id_divider[idx] != 0) {
        func_languageing_shape_create(frm, tbl_lang_row_id_divider[idx], GUI_SCREEN_CENTER_X,
                                      func_languageing_row_divider_y(idx), GUI_SCREEN_WIDTH, 1,
                                      LANG_COLOR_DIVIDER_LINE);
    }

    btn = compo_button_create(frm);
    compo_setid(btn, tbl_lang_row_id_btn[idx]);
    compo_button_set_location(btn, GUI_SCREEN_CENTER_X, y, GUI_SCREEN_WIDTH, LANG_ROW_H);
}

static void func_languageing_row_bind(f_languageing_t *f_lang, u8 idx)
{
    lang_row_ui_t *row = &f_lang->rows[idx];

    row->bg = compo_getobj_byid(tbl_lang_row_id_bg[idx]);
    row->label = compo_getobj_byid(tbl_lang_row_id_label[idx]);
    row->arrow = compo_getobj_byid(tbl_lang_row_id_arrow[idx]);
    row->btn = compo_getobj_byid(tbl_lang_row_id_btn[idx]);
    if (tbl_lang_row_id_divider[idx] != 0) {
        row->divider = compo_getobj_byid(tbl_lang_row_id_divider[idx]);
    }

    func_languageing_gpu_flash_to_ram(lang_right_ram[idx], LANG_RIGHT_RAM_SIZE,
                                      UI_BUF_HOME_RIGHT_BIN, UI_LEN_HOME_RIGHT_BIN, row->arrow);
}

static u8 func_languageing_row_index_from_btn(int id)
{
    switch (id) {
    case COMPO_ID_ROW0_BTN:
        return LANG_ROW_ENGLISH;
    case COMPO_ID_ROW1_BTN:
        return LANG_ROW_DEUTSCH;
    case COMPO_ID_ROW2_BTN:
        return LANG_ROW_ITALIANO;
    case COMPO_ID_ROW3_BTN:
        return LANG_ROW_FRANCAIS;
    default:
        return 0xff;
    }
}

static void func_languageing_button_click(f_languageing_t *f_lang)
{
    int id = compo_get_button_id();
    u8 row;

    switch (id) {
    case COMPO_ID_BTN_BACK:
        func_switch_to(FUNC_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    default:
        row = func_languageing_row_index_from_btn(id);
        if (row < LANG_ROW_CNT) {
            f_lang->focus = row;
            func_languageing_row_focus_refresh(f_lang);
        }
        break;
    }
}

compo_form_t *func_languageing_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_picturebox_t *pic;
    compo_textbox_t *txt;
    compo_button_t *btn;
    u8 i;

    pic = compo_picturebox_create(frm, UI_LANG_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BACK);
    compo_picturebox_set_pos(pic, LANG_BACK_X, LANG_HEADER_Y);
    compo_picturebox_set_size(pic, LANG_LEFT_W, LANG_LEFT_H);
    compo_picturebox_set_visible(pic, false);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_BACK);
    compo_button_set_location(btn, LANG_BACK_BTN_X, LANG_BACK_BTN_Y,
                              LANG_BACK_BTN_W, LANG_BACK_BTN_H);

    txt = compo_textbox_create(frm, 16);
    compo_setid(txt, COMPO_ID_TITLE);
    func_languageing_config_title(txt);

    pic = compo_picturebox_create(frm, UI_LANG_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, LANG_STATUS_BT_X, LANG_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_LANG_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_LOCK);
    compo_picturebox_set_pos(pic, LANG_STATUS_LOCK_X, LANG_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_LANG_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, LANG_STATUS_BAT_X, LANG_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
    compo_picturebox_set_visible(pic, false);

    func_languageing_shape_create(frm, COMPO_ID_HEADER_DIVIDER, GUI_SCREEN_CENTER_X, LANG_DIVIDER_Y,
                                GUI_SCREEN_WIDTH, LANG_DIVIDER_H, LANG_COLOR_DIVIDER_LINE);

    for (i = 0; i < LANG_ROW_CNT; i++) {
        func_languageing_row_create(frm, i);
    }

    return frm;
}

static void func_languageing_process(void)
{
    func_process();
}

static void func_languageing_message(size_msg_t msg)
{
    f_languageing_t *f_lang = (f_languageing_t *)func_cb.f_cb;

    if (f_lang != NULL && f_lang->screen_locked) {
        if (msg != LANG_MSG_POWER && msg != LANG_MSG_OK && msg != KU_LEFT) {
            return;
        }
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        if (f_lang != NULL) {
            func_languageing_button_click(f_lang);
        }
        break;

    case KU_LEFT:
        if (f_lang != NULL) {
            f_lang->screen_locked = !f_lang->screen_locked;
            func_languageing_lock_icon_apply(f_lang);
        }
        break;

    case KU_MODE:
        func_languageing_row_select_next(f_lang);
        break;

    case LANG_MSG_OK:
        func_languageing_confirm(f_lang);
        break;

    case LANG_MSG_POWER:
        func_languageing_power_key(f_lang);
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_languageing_enter(void)
{
    f_languageing_t *f_lang;
    u8 i;

    func_cb.f_cb = func_zalloc(sizeof(f_languageing_t));
    func_cb.frm_main = func_languageing_form_create();

    f_lang = (f_languageing_t *)func_cb.f_cb;
    if (sys_cb.lang_id < LANG_ROW_CNT) {
        f_lang->focus = sys_cb.lang_id;
    } else {
        f_lang->focus = LANG_ROW_ENGLISH;
    }
    f_lang->screen_locked = false;

    f_lang->pic_back = compo_getobj_byid(COMPO_ID_PIC_BACK);
    f_lang->btn_back = compo_getobj_byid(COMPO_ID_BTN_BACK);
    f_lang->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f_lang->pic_lock = compo_getobj_byid(COMPO_ID_PIC_LOCK);
    f_lang->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);

    func_languageing_gpu_flash_to_ram(lang_left_ram, LANG_LEFT_RAM_SIZE,
                                      UI_BUF_HOME_LEFT_BIN, UI_LEN_HOME_LEFT_BIN,
                                      f_lang->pic_back);
    func_languageing_status_icons_apply(f_lang);

    for (i = 0; i < LANG_ROW_CNT; i++) {
        func_languageing_row_bind(f_lang, i);
    }

    func_languageing_row_focus_refresh(f_lang);
}

void func_languageing_exit(void)
{
    func_cb.last = FUNC_LANGUAGEING;
}

void func_languageing(void)
{
    printf("%s\n", __func__);
    func_languageing_enter();
    while (func_cb.sta == FUNC_LANGUAGEING) {
        func_languageing_process();
        func_languageing_message(msg_dequeue());
    }
    func_languageing_exit();
}
