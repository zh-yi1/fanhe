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
 * Ver. Info 页：返回 + 标题 + 状态栏；中部显示版本号。
 * 图标：left + 状态栏 bluetooth/lock/battery_level
 */
#define UI_VERINFO_PLACEHOLDER            UI_BUF_ICON_ACTIVITY_BIN

#ifndef UI_BUF_HOME_LEFT_BIN
#error "Missing left.bin: run tools/convert_ui_home.bat + Output/bin/prebuild.bat"
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

#define VERINFO_COLOR_DIVIDER             make_color(60, 60, 60)

#define VERINFO_LEFT_W                    11
#define VERINFO_LEFT_H                    17
#define VERINFO_LEFT_RAM_SIZE             (8 + VERINFO_LEFT_W * VERINFO_LEFT_H * 2)

#define VERINFO_HEADER_Y                  48
#define VERINFO_BACK_X                    36
#define VERINFO_TITLE_LEFT                58
#define VERINFO_TITLE_W                   220
#define VERINFO_TITLE_H                   36
#define VERINFO_STATUS_Y                  48
#define VERINFO_STATUS_BAT_X              (GUI_SCREEN_WIDTH - 24 - HOME_STATUS_BAT_W / 2)
#define VERINFO_STATUS_LOCK_X             (VERINFO_STATUS_BAT_X - HOME_STATUS_BAT_W / 2 - 10 - HOME_STATUS_LOCK_W / 2)
#define VERINFO_STATUS_BT_X               (VERINFO_STATUS_LOCK_X - HOME_STATUS_LOCK_W / 2 - 10 - HOME_STATUS_BT_W / 2)

#define VERINFO_DIVIDER_Y                 90
#define VERINFO_DIVIDER_H                 1
#define VERINFO_CONTENT_TOP               (VERINFO_DIVIDER_Y + VERINFO_DIVIDER_H / 2 + 4)
#define VERINFO_CONTENT_Y                 ((VERINFO_CONTENT_TOP + GUI_SCREEN_HEIGHT) / 2)
#define VERINFO_VERSION_W                 420
#define VERINFO_VERSION_H                 40

#ifndef VERINFO_VERSION_STR
#define VERINFO_VERSION_STR               "3E 610317-V1.0"
#endif

#define VERINFO_MSG_POWER                 (KEY_RIGHT | KEY_SHORT_UP)

enum {
    COMPO_ID_PIC_BACK = 1,
    COMPO_ID_BTN_BACK,
    COMPO_ID_TITLE,
    COMPO_ID_PIC_BT,
    COMPO_ID_PIC_LOCK,
    COMPO_ID_PIC_BAT,
    COMPO_ID_HEADER_DIVIDER,
    COMPO_ID_VERSION,
};

typedef struct f_verinfo_t_ {
    compo_picturebox_t *pic_back;
    compo_button_t *btn_back;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
    compo_textbox_t *txt_version;
} f_verinfo_t;

static u8 verinfo_left_ram[VERINFO_LEFT_RAM_SIZE];

static bool func_verinfo_gpu_ram_set(u8 *ram, u16 buf_size, u16 data_len, compo_picturebox_t *pic)
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

static bool func_verinfo_gpu_flash_to_ram(u8 *ram, u16 buf_size, u32 flash_addr, u16 flash_len,
                                          compo_picturebox_t *pic)
{
    if (ram == NULL || flash_len == 0 || flash_len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    os_spiflash_read(ram, flash_addr, flash_len);
    return func_verinfo_gpu_ram_set(ram, buf_size, flash_len, pic);
}

static compo_shape_t *func_verinfo_shape_create(compo_form_t *frm, u16 id, s16 x, s16 y,
                                                 s16 w, s16 h, u16 color)
{
    compo_shape_t *shape = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(shape, id);
    compo_shape_set_location(shape, x, y, w, h);
    compo_shape_set_color(shape, color);
    compo_shape_set_radius(shape, 0);
    return shape;
}

static void func_verinfo_status_icons_apply(f_verinfo_t *f_verinfo)
{
    home_ui_shared_status_init();

    if (f_verinfo->pic_bt != NULL && gui_set_ram_check(home_ui_shared_status_bt_ram, __func__)) {
        compo_picturebox_set_ram(f_verinfo->pic_bt, home_ui_shared_status_bt_ram);
        compo_picturebox_set_size(f_verinfo->pic_bt, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
        compo_picturebox_set_visible(f_verinfo->pic_bt, true);
    }
    if (f_verinfo->pic_lock != NULL && gui_set_ram_check(home_ui_shared_status_lock_ram, __func__)) {
        compo_picturebox_set_ram(f_verinfo->pic_lock, home_ui_shared_status_lock_ram);
        compo_picturebox_set_size(f_verinfo->pic_lock, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
        compo_picturebox_set_visible(f_verinfo->pic_lock, true);
    }
    if (f_verinfo->pic_bat != NULL && gui_set_ram_check(home_ui_shared_status_bat_ram, __func__)) {
        compo_picturebox_set_ram(f_verinfo->pic_bat, home_ui_shared_status_bat_ram);
        compo_picturebox_set_size(f_verinfo->pic_bat, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
        compo_picturebox_set_visible(f_verinfo->pic_bat, true);
    }
}

static void func_verinfo_button_click(void)
{
    int id = compo_get_button_id();

    if (id == COMPO_ID_BTN_BACK) {
        func_switch_to(FUNC_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
    }
}

compo_form_t *func_verinfo_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_picturebox_t *pic;
    compo_textbox_t *txt;
    compo_button_t *btn;

    pic = compo_picturebox_create(frm, UI_VERINFO_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BACK);
    compo_picturebox_set_pos(pic, VERINFO_BACK_X, VERINFO_HEADER_Y);
    compo_picturebox_set_size(pic, VERINFO_LEFT_W, VERINFO_LEFT_H);
    compo_picturebox_set_visible(pic, false);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_BACK);
    compo_button_set_location(btn, 16, 28, 56, 40);

    txt = compo_textbox_create(frm, 16);
    compo_setid(txt, COMPO_ID_TITLE);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_align_center(txt, false);
    widget_set_align_center(txt->txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(txt->txt, false);
    compo_textbox_set_location(txt, VERINFO_TITLE_LEFT, VERINFO_HEADER_Y - VERINFO_TITLE_H / 2,
                               VERINFO_TITLE_W, VERINFO_TITLE_H);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, "Ver. Info.");

    pic = compo_picturebox_create(frm, UI_VERINFO_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BT);
    compo_picturebox_set_pos(pic, VERINFO_STATUS_BT_X, VERINFO_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_VERINFO_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_LOCK);
    compo_picturebox_set_pos(pic, VERINFO_STATUS_LOCK_X, VERINFO_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    compo_picturebox_set_visible(pic, false);

    pic = compo_picturebox_create(frm, UI_VERINFO_PLACEHOLDER);
    compo_setid(pic, COMPO_ID_PIC_BAT);
    compo_picturebox_set_pos(pic, VERINFO_STATUS_BAT_X, VERINFO_STATUS_Y);
    compo_picturebox_set_size(pic, HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
    compo_picturebox_set_visible(pic, false);

    func_verinfo_shape_create(frm, COMPO_ID_HEADER_DIVIDER, GUI_SCREEN_CENTER_X, VERINFO_DIVIDER_Y,
                              GUI_SCREEN_WIDTH, VERINFO_DIVIDER_H, VERINFO_COLOR_DIVIDER);

    txt = compo_textbox_create(frm, 32);
    compo_setid(txt, COMPO_ID_VERSION);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_NULL);
    widget_text_set_ellipsis(txt->txt, false);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, VERINFO_CONTENT_Y,
                               VERINFO_VERSION_W, VERINFO_VERSION_H);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, VERINFO_VERSION_STR);

    return frm;
}

static void func_verinfo_process(void)
{
    func_process();
}

static void func_verinfo_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_verinfo_button_click();
        break;

    case VERINFO_MSG_POWER:
        func_switch_to(FUNC_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_verinfo_enter(void)
{
    f_verinfo_t *f_verinfo;

    func_cb.f_cb = func_zalloc(sizeof(f_verinfo_t));
    func_cb.frm_main = func_verinfo_form_create();

    f_verinfo = (f_verinfo_t *)func_cb.f_cb;
    f_verinfo->pic_back = compo_getobj_byid(COMPO_ID_PIC_BACK);
    f_verinfo->btn_back = compo_getobj_byid(COMPO_ID_BTN_BACK);
    f_verinfo->pic_bt = compo_getobj_byid(COMPO_ID_PIC_BT);
    f_verinfo->pic_lock = compo_getobj_byid(COMPO_ID_PIC_LOCK);
    f_verinfo->pic_bat = compo_getobj_byid(COMPO_ID_PIC_BAT);
    f_verinfo->txt_version = compo_getobj_byid(COMPO_ID_VERSION);

    func_verinfo_gpu_flash_to_ram(verinfo_left_ram, VERINFO_LEFT_RAM_SIZE,
                                  UI_BUF_HOME_LEFT_BIN, UI_LEN_HOME_LEFT_BIN,
                                  f_verinfo->pic_back);
    func_verinfo_status_icons_apply(f_verinfo);
}

void func_verinfo_exit(void)
{
    func_cb.last = FUNC_VERINFO;
}

void func_verinfo(void)
{
    printf("%s\n", __func__);
    func_verinfo_enter();
    while (func_cb.sta == FUNC_VERINFO) {
        func_verinfo_process();
        func_verinfo_message(msg_dequeue());
    }
    func_verinfo_exit();
}
