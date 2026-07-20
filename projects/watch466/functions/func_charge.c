#include "include.h"
#include "func.h"

#if ELUNCHBOX_PANEL_EN

#include "home_ui_shared.h"
#include "new_home_icon_res.h"
#include "home_ui_gpu_detach.h"
#if USER_PANEL_LED
#include "port_panel_led.h"
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

/* 关机/关屏态充电：黑底 + 右上角充电跑马灯；拔电回主页 */
#define CHARGE_OFF_STATUS_Y               20
#define CHARGE_OFF_STATUS_RIGHT_MARGIN    10
#define CHARGE_OFF_BG_MARGIN              8

enum {
    COMPO_ID_CHARGE_OFF_BG = 1,
    COMPO_ID_CHARGE_OFF_BAT,
};

typedef struct {
    compo_shape_t *bg;
    compo_picturebox_t *pic_bat;
} f_charge_t;

extern volatile u8 elunchbox_te_block_flag;

/* new_ui 充电图按白底导出，黑屏页须把白底像素改成黑色，避免图标周围白块 */
static void charge_off_bat_white_to_black(compo_picturebox_t *pic)
{
    u8 *ram = home_ui_shared_status_bat_ram;
    u16 w;
    u16 h;
    u32 n;
    u32 i;

    if (pic == NULL || pic->img == NULL) {
        return;
    }
    w = (u16)GET_LE16(&ram[4]);
    h = (u16)GET_LE16(&ram[6]);
    if (w == 0 || h == 0 || (u32)w * (u32)h > (HOME_STATUS_BAT_RAM_SIZE - 8) / 2) {
        return;
    }
    n = (u32)w * (u32)h;
    for (i = 0; i < n; i++) {
        u16 c = (u16)GET_LE16(ram + 8 + i * 2);
        /* 白底及接近白的抗锯齿边 → 黑 */
        if (c >= 0xC618) {
            PUT_LE16(ram + 8 + i * 2, COLOR_BLACK);
        }
    }
    if (gui_set_ram_check(ram, __func__)) {
        compo_picturebox_set_ram(pic, ram);
        compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);
        compo_picturebox_set_visible(pic, true);
    }
}

#if USER_PANEL_LED
static void charge_off_led_all_off(void)
{
    panel_led_set_switch_latched(false);
    panel_led_set_lock_latched(false);
    panel_led_set_res_latched(false);
    panel_led_set_heat_latched(false);
    panel_led_all_off();
}
#else
static void charge_off_led_all_off(void) {}
#endif

bool elunchbox_charge_off_active(void)
{
    return func_cb.sta == FUNC_CHARGE;
}

void func_elunchbox_enter_charge_off_page(void)
{
    if (!home_ui_shared_battery_is_charging()) {
        return;
    }
    if (func_cb.sta == FUNC_CHARGE) {
        return;
    }
    if (sys_cb.flag_swithing) {
        return;
    }

    if (elunchbox_pwr_is_manual_off()
        || sys_cb.gui_sleep_sta
        || !elunchbox_ui_is_live()) {
        elunchbox_pwr_gui_wake_reason("charge_off_page");
    }

    printf("elunchbox: enter charge off page from sta=%u\n", func_cb.sta);
    func_switch_to(FUNC_CHARGE, FUNC_SWITCH_DIRECT | FUNC_SWITCH_AUTO);
}

static void charge_off_form_prepare(compo_form_t *frm)
{
    if (frm == NULL) {
        return;
    }
    compo_form_set_mode(frm, 0);
    if (frm->title != NULL) {
        compo_textbox_set_visible(frm->title, false);
    }
    if (frm->time != NULL) {
        widget_set_visible(frm->time, false);
    }
    if (frm->title_icon != NULL) {
        widget_set_visible(frm->title_icon, false);
    }
    if (frm->icon != NULL) {
        widget_set_visible(frm->icon, false);
    }
    if (frm->page_body != NULL) {
        widget_set_location(frm->page_body, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                            GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
        widget_page_set_client(frm->page_body, 0, 0);
    }
}

static void charge_off_bg_create(compo_form_t *frm)
{
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, COMPO_ID_CHARGE_OFF_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             (s16)(GUI_SCREEN_WIDTH + CHARGE_OFF_BG_MARGIN * 2),
                             (s16)(GUI_SCREEN_HEIGHT + CHARGE_OFF_BG_MARGIN * 2));
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_radius(bg, 0);
}

static compo_picturebox_t *charge_off_bat_create(compo_form_t *frm)
{
    compo_picturebox_t *pic;
    s16 bat_x;

    pic = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    if (pic == NULL) {
        return NULL;
    }
    bat_x = (s16)(GUI_SCREEN_WIDTH - CHARGE_OFF_STATUS_RIGHT_MARGIN - NEW_HOME_BAT_W / 2);
    compo_setid(pic, COMPO_ID_CHARGE_OFF_BAT);
    compo_picturebox_set_pos(pic, bat_x, CHARGE_OFF_STATUS_Y);
    compo_picturebox_set_size(pic, NEW_HOME_BAT_W, NEW_HOME_BAT_H);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

compo_form_t *func_charge_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);

    charge_off_form_prepare(frm);
    charge_off_bg_create(frm);
    return frm;
}

static void charge_off_try_exit_home(void)
{
    if (home_ui_shared_battery_is_charging()) {
        return;
    }
    if (sys_cb.flag_swithing) {
        return;
    }
    printf("elunchbox: charge off page unplug -> home\n");
    func_switch_to(FUNC_HOME, FUNC_SWITCH_DIRECT | FUNC_SWITCH_AUTO);
}

static void func_charge_process(void)
{
    f_charge_t *f = (f_charge_t *)func_cb.f_cb;

    WDT_CLR();
    charge_off_led_all_off();
    home_ui_shared_battery_chg_poll();
    if (f != NULL && f->pic_bat != NULL) {
        charge_off_bat_white_to_black(f->pic_bat);
    }
    tft_bglight_frist_set_check();
    func_process();
    charge_off_try_exit_home();
}

static void func_charge_message(size_msg_t msg)
{
    /* 黑屏充电页：吞掉触控/按键，仅拔电退出 */
    (void)msg;
}

void func_charge_enter(void)
{
    f_charge_t *f;
    u8 was_blocked;

    printf("func_charge_enter (charge off page)\n");
    charge_off_led_all_off();

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(1);
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif

    func_cb.f_cb = func_zalloc(sizeof(f_charge_t));
    f = (f_charge_t *)func_cb.f_cb;
    func_cb.frm_main = func_charge_form_create();
    charge_off_form_prepare(func_cb.frm_main);
    f->bg = (compo_shape_t *)compo_getobj_byid(COMPO_ID_CHARGE_OFF_BG);
    f->pic_bat = charge_off_bat_create(func_cb.frm_main);

    was_blocked = elunchbox_te_block_flag;
    if (!was_blocked) {
        elunchbox_te_block_flag = 1;
    }
    WDT_CLR();
    if (f->bg != NULL) {
        compo_shape_set_visible(f->bg, true);
        if (f->bg->rect != NULL) {
            widget_set_top(f->bg->rect, false);
        }
    }
    if (f->pic_bat != NULL) {
        home_ui_shared_battery_icon_refresh();
        home_ui_shared_battery_attach_pic(f->pic_bat);
        charge_off_bat_white_to_black(f->pic_bat);
        if (f->pic_bat->img != NULL) {
            widget_set_top(f->pic_bat->img, true);
        }
    }
    if (!was_blocked) {
        elunchbox_te_block_flag = 0;
    }

    tft_bglight_force_on();
    printf("func_charge_enter: ok bat=%p\n", f->pic_bat);
}

void func_charge_exit(void)
{
    f_charge_t *f = (f_charge_t *)func_cb.f_cb;

    printf("func_charge_exit\n");
    home_ui_shared_battery_detach_pic();
    /* 充电页把白底改成了黑，退出前从 Flash 重载，供主页白底使用 */
    home_ui_shared_battery_icon_refresh();
    if (f != NULL && f->pic_bat != NULL) {
        home_ui_gpu_pic_detach_light(f->pic_bat);
    }
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(0);
#endif
    func_cb.last = FUNC_CHARGE;
}

void func_charge(void)
{
    printf("%s\n", __func__);
    func_charge_enter();
    while (func_cb.sta == FUNC_CHARGE) {
        func_charge_process();
        func_charge_message(msg_dequeue());
    }
    func_charge_exit();
}

#else /* !ELUNCHBOX_PANEL_EN */

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_charge_t_ {
   u8 percent_bkp;
} f_charge_t;

compo_form_t *func_charge_form_create(void)
{
   return compo_form_create(true);
}

static void func_charge_process(void)
{
   func_process();
}

static void func_charge_message(size_msg_t msg)
{
   func_message(msg);
}

void func_charge_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_charge_t));
    func_cb.frm_main = func_charge_form_create();
}

void func_charge_exit(void)
{
    func_cb.last = FUNC_CHARGE;
}

void func_charge(void)
{
    printf("%s\n", __func__);
    func_charge_enter();
    while (func_cb.sta == FUNC_CHARGE) {
        func_charge_process();
        func_charge_message(msg_dequeue());
    }
    func_charge_exit();
}

bool elunchbox_charge_off_active(void)
{
    return false;
}

void func_elunchbox_enter_charge_off_page(void)
{
}

#endif /* ELUNCHBOX_PANEL_EN */
