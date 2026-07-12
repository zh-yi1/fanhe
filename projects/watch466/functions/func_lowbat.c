#include "include.h"
#include "func.h"
#include "func_lowbat.h"
#include "new_home_icon_res.h"
#include "home_ui_shared.h"
#include "func_lunchbox_uart.h"
#include "func_lunchbox_wake.h"
#include "home_ui_gpu_detach.h"
#if USER_PANEL_LED
#include "port_panel_led.h"
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_LOWBAT_MODE_EN

#ifndef UI_BUF_NEW_UI_DIDIAN_BIN
#error "Missing didian.bin: add Output/bin/ui/new_ui/didian.png and run tools/gen_new_ui_icons.py + prebuild.bat"
#endif

/* 仅 DP09 fault_code=0x0A 进入低电页；进入后只显示 UI，不下发任何 UART 指令 */
#define LB_FAULT_CODE_LOW_BAT           0x0Au

#define LOWBAT_ICON_DRAW_W              NEW_UI_DIDIAN_W
#define LOWBAT_ICON_DRAW_H              NEW_UI_DIDIAN_H
#define LOWBAT_BG_MARGIN                8

enum {
    COMPO_ID_LOWBAT_BG = 1,
    COMPO_ID_LOWBAT_PIC,
};

typedef struct {
    compo_picturebox_t *pic_icon;
    compo_shape_t *bg;
    bool icon_applied;
} f_lowbat_t;

static bool elunchbox_lowbat_latched;
static bool elunchbox_lowbat_mcu_fault;

static u8 *lowbat_icon_ram_ptr;

extern volatile u8 elunchbox_te_block_flag;

static void lowbat_icon_ram_free(void);

#if USER_PANEL_LED
static void lowbat_led_all_off(void)
{
    panel_led_set_switch_latched(false);
    panel_led_set_lock_latched(false);
    panel_led_set_res_latched(false);
    panel_led_set_heat_latched(false);
    panel_led_all_off();
}
#else
static void lowbat_led_all_off(void) {}
#endif

static void lowbat_form_prepare(compo_form_t *frm)
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

static void lowbat_bg_create(compo_form_t *frm)
{
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);

    compo_setid(bg, COMPO_ID_LOWBAT_BG);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             (s16)(GUI_SCREEN_WIDTH + LOWBAT_BG_MARGIN * 2),
                             (s16)(GUI_SCREEN_HEIGHT + LOWBAT_BG_MARGIN * 2));
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_radius(bg, 0);
}

static compo_picturebox_t *lowbat_pic_create(compo_form_t *frm)
{
    compo_picturebox_t *pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);

    if (pic == NULL) {
        return NULL;
    }
    pic->img = (void *)widget_image_create(frm->page_body, 0);
    pic->radix = 1;
    compo_setid(pic, COMPO_ID_LOWBAT_PIC);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

compo_form_t *func_lowbat_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);

    lowbat_form_prepare(frm);
    lowbat_bg_create(frm);
    return frm;
}

bool elunchbox_lowbat_active(void)
{
    return elunchbox_lowbat_latched || func_cb.sta == FUNC_LOWBAT;
}

bool elunchbox_lowbat_should_block_ui_route(void)
{
    if (elunchbox_lowbat_active()) {
        return true;
    }
    if (home_ui_shared_battery_is_charging()) {
        return false;
    }
    return elunchbox_lowbat_mcu_fault;
}

static bool elunchbox_lowbat_should_enter(void)
{
    if (home_ui_shared_battery_is_charging()) {
        return false;
    }
    return elunchbox_lowbat_mcu_fault;
}

static void elunchbox_lowbat_enter_now(void)
{
    if (elunchbox_lowbat_latched && func_cb.sta == FUNC_LOWBAT) {
        return;
    }
    if (sys_cb.flag_swithing) {
        return;
    }

    elunchbox_lowbat_latched = true;
    lowbat_led_all_off();

    if (elunchbox_pwr_is_manual_off()
        || sys_cb.gui_sleep_sta
        || !elunchbox_ui_is_live()) {
        elunchbox_pwr_gui_wake_reason("lowbat");
    }

    if (func_cb.sta == FUNC_LOWBAT) {
        return;
    }

    printf("lowbat: enter (sta=%u fault=%u)\n",
           func_cb.sta, elunchbox_lowbat_mcu_fault ? 1u : 0u);
    func_switch_to(FUNC_LOWBAT, FUNC_SWITCH_DIRECT | FUNC_SWITCH_AUTO);
}

static void elunchbox_lowbat_try_exit(void)
{
    if (func_cb.sta != FUNC_LOWBAT || elunchbox_lowbat_mcu_fault) {
        return;
    }
    elunchbox_lowbat_latched = false;
    printf("lowbat: fault cleared, exit to home\n");
    func_switch_to(FUNC_HOME, FUNC_SWITCH_DIRECT | FUNC_SWITCH_AUTO);
}

void elunchbox_lowbat_feed_dp(u8 *data, u16 len)
{
    u16 off = 0;
    bool got_fault = false;
    u8 fault = 0;

    if (data == NULL || len < 4) {
        return;
    }

    while (off + 4 <= len) {
        u8  dpid    = data[off];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];

        if (off + 4 + val_len > len) {
            break;
        }
        u8 *val = data + off + 4;

        if (dpid == LB_DPID_FAULT && val_len >= 1) {
            fault = val[0];
            got_fault = true;
            elunchbox_lowbat_mcu_fault = (fault == LB_FAULT_CODE_LOW_BAT);
        }
        off += 4 + val_len;
    }

    if (got_fault && !elunchbox_lowbat_mcu_fault) {
        elunchbox_lowbat_try_exit();
    }
}

void elunchbox_lowbat_poll(void)
{
    if (elunchbox_lowbat_should_enter()) {
        elunchbox_lowbat_enter_now();
        return;
    }
    elunchbox_lowbat_try_exit();
}

void elunchbox_lowbat_reset(void)
{
    elunchbox_lowbat_mcu_fault = false;
    elunchbox_lowbat_latched = false;
    lowbat_icon_ram_free();
}

static void lowbat_icon_ram_free(void)
{
    if (lowbat_icon_ram_ptr != NULL) {
        ab_free(lowbat_icon_ram_ptr);
        lowbat_icon_ram_ptr = NULL;
    }
}

static void lowbat_bring_front(f_lowbat_t *f)
{
    if (f == NULL) {
        return;
    }
    if (f->bg != NULL && f->bg->rect != NULL) {
        widget_set_top(f->bg->rect, false);
    }
    if (f->pic_icon != NULL && f->pic_icon->img != NULL) {
        widget_set_top(f->pic_icon->img, true);
    }
}

static bool lowbat_icon_ram_load(void)
{
    if (UI_LEN_NEW_UI_DIDIAN_BIN == 0
        || UI_LEN_NEW_UI_DIDIAN_BIN > NEW_UI_DIDIAN_RAM_SIZE) {
        printf("lowbat: bad didian len %u cap %u\n",
               UI_LEN_NEW_UI_DIDIAN_BIN, NEW_UI_DIDIAN_RAM_SIZE);
        return false;
    }
    if (lowbat_icon_ram_ptr == NULL) {
        lowbat_icon_ram_ptr = (u8 *)ab_malloc(NEW_UI_DIDIAN_RAM_SIZE);
    }
    if (lowbat_icon_ram_ptr == NULL) {
        printf("lowbat: ab_malloc failed %u\n", NEW_UI_DIDIAN_RAM_SIZE);
        return false;
    }
    os_spiflash_read(lowbat_icon_ram_ptr, UI_BUF_NEW_UI_DIDIAN_BIN, UI_LEN_NEW_UI_DIDIAN_BIN);
    if (!gui_set_ram_check(lowbat_icon_ram_ptr, __func__)) {
        printf("lowbat: gui_set_ram_check failed\n");
        return false;
    }
    return true;
}

static bool lowbat_icon_apply(f_lowbat_t *f)
{
    u16 iw;
    u16 ih;

    if (f == NULL || f->pic_icon == NULL) {
        return false;
    }
    if (!lowbat_icon_ram_load()) {
        return false;
    }

    if (f->bg != NULL) {
        compo_shape_set_visible(f->bg, true);
    }

    iw = (u16)GET_LE16(&lowbat_icon_ram_ptr[4]);
    ih = (u16)GET_LE16(&lowbat_icon_ram_ptr[6]);
    if (iw == 0 || ih == 0) {
        iw = LOWBAT_ICON_DRAW_W;
        ih = LOWBAT_ICON_DRAW_H;
    }
    compo_picturebox_set_ram(f->pic_icon, lowbat_icon_ram_ptr);
    compo_picturebox_set_pos(f->pic_icon, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_size(f->pic_icon, (s16)iw, (s16)ih);
    compo_picturebox_set_visible(f->pic_icon, true);
    lowbat_bring_front(f);
    f->icon_applied = true;
    return true;
}

static void lowbat_icon_show(f_lowbat_t *f)
{
    u8 was_blocked;

    if (f == NULL || f->pic_icon == NULL) {
        return;
    }

    was_blocked = elunchbox_te_block_flag;
    if (!was_blocked) {
        elunchbox_te_block_flag = 1;
    }
    WDT_CLR();

    if (!lowbat_icon_apply(f)) {
        printf("lowbat: icon apply failed\n");
    }

    if (!was_blocked) {
        elunchbox_te_block_flag = 0;
    }
}

void func_lowbat_enter(void)
{
    f_lowbat_t *f;

    printf("func_lowbat_enter\n");
    elunchbox_lowbat_latched = true;
    lowbat_led_all_off();

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(1);
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif

    func_cb.f_cb = func_zalloc(sizeof(f_lowbat_t));
    f = (f_lowbat_t *)func_cb.f_cb;
    func_cb.frm_main = func_lowbat_form_create();
    lowbat_form_prepare(func_cb.frm_main);
    f->bg = (compo_shape_t *)compo_getobj_byid(COMPO_ID_LOWBAT_BG);
    f->pic_icon = lowbat_pic_create(func_cb.frm_main);
    f->icon_applied = false;

    lowbat_icon_show(f);
    tft_bglight_force_on();
    printf("func_lowbat_enter: ok pic=%p ram=%p\n", f->pic_icon, lowbat_icon_ram_ptr);
}

void func_lowbat_exit(void)
{
    f_lowbat_t *f = (f_lowbat_t *)func_cb.f_cb;

    printf("func_lowbat_exit\n");
    if (f != NULL && f->pic_icon != NULL) {
        home_ui_gpu_pic_detach_light(f->pic_icon);
    }
    lowbat_icon_ram_free();
    func_cb.last = FUNC_LOWBAT;
}

static void func_lowbat_message(size_msg_t msg)
{
    (void)msg;
}

static void func_lowbat_process(void)
{
    f_lowbat_t *f = (f_lowbat_t *)func_cb.f_cb;

    WDT_CLR();
    lowbat_led_all_off();
    if (f != NULL && f->pic_icon != NULL
        && (!f->icon_applied || !compo_picturebox_get_visible(f->pic_icon))) {
        lowbat_icon_show(f);
    }
    tft_bglight_frist_set_check();
    if (func_cb.frm_main != NULL) {
        compo_update();
        gui_process();
    }
#if FUNC_LUNCHBOX_UART_EN
    /* 仅收包更新 DP09，lb_send_frame 内已禁止低电态 TX */
    lunchbox_uart_process();
#endif
    co_timer_pro(false);
}

void func_lowbat(void)
{
    func_lowbat_enter();
    while (func_cb.sta == FUNC_LOWBAT) {
        func_lowbat_process();
        func_lowbat_message(msg_dequeue());
    }
    func_lowbat_exit();
}

#endif /* ELUNCHBOX_PANEL_EN && ELUNCHBOX_LOWBAT_MODE_EN */
