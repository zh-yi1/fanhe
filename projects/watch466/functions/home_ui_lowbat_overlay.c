#include "include.h"
#include "home_ui_lowbat_overlay.h"
#include "home_ui_lock_overlay.h"
#include "home_ui_shared.h"
#include "new_home_icon_res.h"
#include "func.h"

#if ELUNCHBOX_PANEL_EN && HOME_UI_LOWBAT_OVERLAY_EN

#ifndef UI_BUF_NEW_UI_DIDIAN_BIN
#error "Missing didian.bin: add Output/bin/ui/new_ui/didian.png and run tools/gen_new_ui_icons.py + prebuild.bat"
#endif

#ifndef NEW_UI_DIDIAN_W
#define NEW_UI_DIDIAN_W                 GUI_SCREEN_WIDTH
#define NEW_UI_DIDIAN_H                 GUI_SCREEN_HEIGHT
#endif

#define HOME_UI_LOWBAT_OVERLAY_RAM_SIZE    (UI_LEN_NEW_UI_DIDIAN_BIN + 64)

enum {
    COMPO_ID_LOWBAT_OVERLAY_PIC = 0xFF03,
};

enum {
    LOWBAT_ST_IDLE = 0,
    LOWBAT_ST_SHOW,
    LOWBAT_ST_WAIT,
};

extern volatile u8 elunchbox_te_block_flag;

static compo_form_t *lowbat_overlay_frm;
static compo_picturebox_t *lowbat_overlay_pic;
static bool lowbat_overlay_visible;
static u8 lowbat_state;
static u32 lowbat_phase_tick;

static u8 *lowbat_overlay_ram_ptr;

static compo_picturebox_t *home_ui_lowbat_overlay_pic_create_hidden(compo_form_t *frm, u16 id)
{
    compo_picturebox_t *pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);

    if (pic == NULL) {
        return NULL;
    }
    pic->img = (void *)widget_image_create(frm->page_body, 0);
    pic->radix = 1;
    compo_setid(pic, id);
    compo_picturebox_set_visible(pic, false);
    return pic;
}

static void home_ui_lowbat_overlay_destroy(void)
{
    lowbat_overlay_pic = NULL;
    lowbat_overlay_frm = NULL;
    lowbat_overlay_visible = false;
}

static void home_ui_lowbat_overlay_bring_front_internal(void)
{
    if (lowbat_overlay_frm != NULL && lowbat_overlay_frm != func_cb.frm_main) {
        home_ui_lowbat_overlay_reset();
        lowbat_overlay_visible = false;
        return;
    }
    if (lowbat_overlay_pic != NULL && lowbat_overlay_pic->img != NULL) {
        widget_set_top(lowbat_overlay_pic->img, true);
    }
}

static void home_ui_lowbat_overlay_ensure(compo_form_t *frm)
{
    if (frm == NULL) {
        return;
    }
    if (lowbat_overlay_frm == frm && lowbat_overlay_pic != NULL) {
        return;
    }

    home_ui_lowbat_overlay_destroy();
    lowbat_overlay_pic = home_ui_lowbat_overlay_pic_create_hidden(frm, COMPO_ID_LOWBAT_OVERLAY_PIC);
    if (lowbat_overlay_pic == NULL) {
        return;
    }
    lowbat_overlay_frm = frm;
}

static bool home_ui_lowbat_overlay_load_icon(u16 *out_w, u16 *out_h)
{
    u32 addr = UI_BUF_NEW_UI_DIDIAN_BIN;
    u16 len = UI_LEN_NEW_UI_DIDIAN_BIN;

    if (len == 0 || len > HOME_UI_LOWBAT_OVERLAY_RAM_SIZE) {
        return false;
    }

    if (lowbat_overlay_ram_ptr == NULL) {
        lowbat_overlay_ram_ptr = (u8 *)ab_malloc(HOME_UI_LOWBAT_OVERLAY_RAM_SIZE);
    }
    if (lowbat_overlay_ram_ptr == NULL) {
        return false;
    }

    os_spiflash_read(lowbat_overlay_ram_ptr, addr, len);
    *out_w = NEW_UI_DIDIAN_W;
    *out_h = NEW_UI_DIDIAN_H;
    return gui_set_ram_check(lowbat_overlay_ram_ptr, __func__);
}

void home_ui_lowbat_overlay_reset(void)
{
    if (lowbat_overlay_pic != NULL) {
        compo_picturebox_set_visible(lowbat_overlay_pic, false);
        compo_picturebox_set_ram(lowbat_overlay_pic, NULL);
    }
    if (lowbat_overlay_ram_ptr != NULL) {
        ab_free(lowbat_overlay_ram_ptr);
        lowbat_overlay_ram_ptr = NULL;
    }
    home_ui_lowbat_overlay_destroy();
    lowbat_state = LOWBAT_ST_IDLE;
    lowbat_phase_tick = 0;
}

bool home_ui_lowbat_overlay_is_visible(void)
{
    return lowbat_overlay_visible;
}

void home_ui_lowbat_overlay_hide(void)
{
    lowbat_overlay_visible = false;
    if (lowbat_overlay_frm != NULL && lowbat_overlay_frm != func_cb.frm_main) {
        home_ui_lowbat_overlay_reset();
        return;
    }
    if (lowbat_overlay_pic != NULL) {
        compo_picturebox_set_visible(lowbat_overlay_pic, false);
    }
}

void home_ui_lowbat_overlay_bring_front(void)
{
    if (!lowbat_overlay_visible) {
        return;
    }
    home_ui_lowbat_overlay_bring_front_internal();
}

void home_ui_lowbat_overlay_show(void)
{
    u16 icon_w;
    u16 icon_h;
    u8 was_blocked;

    if (func_cb.frm_main == NULL || sys_cb.flag_swithing) {
        return;
    }
    if (home_ui_lock_overlay_is_visible()) {
        return;
    }

    if (lowbat_overlay_frm != NULL && lowbat_overlay_frm != func_cb.frm_main) {
        home_ui_lowbat_overlay_reset();
    }

    if (lowbat_overlay_visible && lowbat_overlay_frm == func_cb.frm_main
        && lowbat_overlay_pic != NULL) {
        return;
    }

    was_blocked = elunchbox_te_block_flag;
    if (!was_blocked) {
        elunchbox_te_block_flag = 1;
    }
    WDT_CLR();

    home_ui_lowbat_overlay_ensure(func_cb.frm_main);
    if (lowbat_overlay_pic == NULL) {
        if (!was_blocked) {
            elunchbox_te_block_flag = 0;
        }
        return;
    }

    if (!home_ui_lowbat_overlay_load_icon(&icon_w, &icon_h)) {
        home_ui_lowbat_overlay_hide();
        if (!was_blocked) {
            elunchbox_te_block_flag = 0;
        }
        return;
    }

    if (!was_blocked) {
        elunchbox_te_block_flag = 0;
    }

    compo_picturebox_set_ram(lowbat_overlay_pic, lowbat_overlay_ram_ptr);
    compo_picturebox_set_pos(lowbat_overlay_pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_size(lowbat_overlay_pic, icon_w, icon_h);
    compo_picturebox_set_visible(lowbat_overlay_pic, true);
    lowbat_overlay_visible = true;
    home_ui_lowbat_overlay_bring_front_internal();
    WDT_CLR();
}

void home_ui_lowbat_poll(void)
{
    if (!elunchbox_ui_is_live() || elunchbox_pwr_is_manual_off() || sys_cb.flag_swithing) {
        if (lowbat_state != LOWBAT_ST_IDLE || lowbat_overlay_visible) {
            home_ui_lowbat_overlay_hide();
            lowbat_state = LOWBAT_ST_IDLE;
        }
        return;
    }

    if (!home_ui_shared_battery_is_low() || home_ui_shared_battery_is_charging()) {
        if (lowbat_state != LOWBAT_ST_IDLE || lowbat_overlay_visible) {
            home_ui_lowbat_overlay_hide();
            lowbat_state = LOWBAT_ST_IDLE;
        }
        return;
    }

    switch (lowbat_state) {
    case LOWBAT_ST_IDLE:
        lowbat_state = LOWBAT_ST_SHOW;
        lowbat_phase_tick = tick_get();
        home_ui_lowbat_overlay_show();
        break;

    case LOWBAT_ST_SHOW:
        home_ui_lowbat_overlay_bring_front();
        if (tick_check_expire(lowbat_phase_tick, HOME_UI_LOWBAT_SHOW_MS)) {
            home_ui_lowbat_overlay_hide();
            lowbat_state = LOWBAT_ST_WAIT;
            lowbat_phase_tick = tick_get();
        }
        break;

    case LOWBAT_ST_WAIT:
        if (tick_check_expire(lowbat_phase_tick, HOME_UI_LOWBAT_INTERVAL_MS)) {
            lowbat_state = LOWBAT_ST_SHOW;
            lowbat_phase_tick = tick_get();
            home_ui_lowbat_overlay_show();
        }
        break;

    default:
        lowbat_state = LOWBAT_ST_IDLE;
        break;
    }
}

#endif /* ELUNCHBOX_PANEL_EN && HOME_UI_LOWBAT_OVERLAY_EN */
