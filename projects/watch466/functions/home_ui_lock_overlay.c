#include "include.h"
#include "home_ui_lock_overlay.h"
#include "new_home_icon_res.h"
#include "home_ui_lock_res.h"
#include "home_ui_shared.h"
#include "home_ui_ram.h"
#include "new_heat_res.h"

#if ELUNCHBOX_PANEL_EN

#ifndef UI_BUF_NEW_UI_NEW_LOCK_BIN
#error "Missing new_lock.bin: add Output/bin/ui/new_ui/new_lock.png and run tools/gen_new_ui_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_NEW_UI_NEW_UNLOCK_BIN
#error "Missing new_unlock.bin: add Output/bin/ui/new_ui/new_unlock.png and run tools/gen_new_ui_icons.py + prebuild.bat"
#endif

#define HOME_UI_LOCK_OVERLAY_ALPHA      160

enum {
    COMPO_ID_LOCK_OVERLAY_DIM = 0xFF01,
    COMPO_ID_LOCK_OVERLAY_PIC = 0xFF02,
};

extern volatile u8 elunchbox_te_block_flag;

static compo_form_t *lock_overlay_frm;
static compo_shape_t *lock_overlay_dim;
static compo_picturebox_t *lock_overlay_pic;
static bool lock_overlay_visible;

/* 复用 heat_bg（Home 等页未用），勿 +21KB BSS 导致启动失败 */
#define lock_overlay_ram                home_ui_heat_bg_ram
#define HOME_UI_LOCK_OVERLAY_RAM_CAP      NEW_HEAT_NEW_PROGRESS_BG_RAM_SIZE

static compo_picturebox_t *home_ui_lock_overlay_pic_create_hidden(compo_form_t *frm, u16 id)
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

static void home_ui_lock_overlay_destroy(void)
{
    lock_overlay_dim = NULL;
    lock_overlay_pic = NULL;
    lock_overlay_frm = NULL;
    lock_overlay_visible = false;
}

static void home_ui_lock_overlay_bring_front_internal(void)
{
    if (lock_overlay_dim != NULL && lock_overlay_dim->rect != NULL) {
        widget_set_top(lock_overlay_dim->rect, true);
    }
    if (lock_overlay_pic != NULL && lock_overlay_pic->img != NULL) {
        widget_set_top(lock_overlay_pic->img, true);
    }
}

static void home_ui_lock_overlay_ensure(compo_form_t *frm)
{
    compo_shape_t *dim;

    if (frm == NULL) {
        return;
    }
    if (lock_overlay_frm == frm && lock_overlay_dim != NULL && lock_overlay_pic != NULL) {
        return;
    }

    home_ui_lock_overlay_destroy();

    dim = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    if (dim == NULL) {
        return;
    }
    compo_setid(dim, COMPO_ID_LOCK_OVERLAY_DIM);
    compo_shape_set_location(dim, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(dim, COLOR_WHITE);
    compo_shape_set_radius(dim, 0);
    compo_shape_set_alpha(dim, HOME_UI_LOCK_OVERLAY_ALPHA);
    compo_shape_set_visible(dim, false);

    lock_overlay_pic = home_ui_lock_overlay_pic_create_hidden(frm, COMPO_ID_LOCK_OVERLAY_PIC);
    if (lock_overlay_pic == NULL) {
        lock_overlay_dim = NULL;
        lock_overlay_frm = NULL;
        return;
    }

    lock_overlay_dim = dim;
    lock_overlay_frm = frm;
}

static bool home_ui_lock_overlay_load_icon(bool unlock_icon, u16 *out_w, u16 *out_h)
{
    u32 addr;
    u16 len;

    if (unlock_icon) {
        addr = UI_BUF_NEW_UI_NEW_UNLOCK_BIN;
        len = UI_LEN_NEW_UI_NEW_UNLOCK_BIN;
        *out_w = NEW_UI_UNLOCK_W;
        *out_h = NEW_UI_UNLOCK_H;
    } else {
        addr = UI_BUF_NEW_UI_NEW_LOCK_BIN;
        len = UI_LEN_NEW_UI_NEW_LOCK_BIN;
        *out_w = NEW_UI_LOCK_W;
        *out_h = NEW_UI_LOCK_H;
    }

    if (len == 0 || len > HOME_UI_LOCK_OVERLAY_RAM_CAP) {
        return false;
    }

    os_spiflash_read(lock_overlay_ram, addr, len);
    return gui_set_ram_check(lock_overlay_ram, __func__);
}

void home_ui_lock_overlay_prepare(compo_form_t *frm)
{
    /* 勿在 form_create 调用：widget 池在 boot 时已满，仅 show 时懒创建 */
    (void)frm;
}

void home_ui_lock_overlay_bring_front(void)
{
    if (!lock_overlay_visible) {
        return;
    }
    home_ui_lock_overlay_bring_front_internal();
}

void home_ui_lock_overlay_reset(void)
{
    home_ui_lock_overlay_destroy();
}

void home_ui_lock_overlay_hide(void)
{
    lock_overlay_visible = false;
    if (lock_overlay_dim != NULL) {
        compo_shape_set_visible(lock_overlay_dim, false);
    }
    if (lock_overlay_pic != NULL) {
        compo_picturebox_set_visible(lock_overlay_pic, false);
    }
}

void home_ui_lock_overlay_show(bool unlock_icon)
{
    u16 icon_w;
    u16 icon_h;
    u8 was_blocked;

    if (func_cb.frm_main == NULL) {
        return;
    }

    if (lock_overlay_frm != NULL && lock_overlay_frm != func_cb.frm_main) {
        home_ui_lock_overlay_reset();
    }

    was_blocked = elunchbox_te_block_flag;
    if (!was_blocked) {
        elunchbox_te_block_flag = 1;
    }
    WDT_CLR();

    home_ui_lock_overlay_ensure(func_cb.frm_main);
    if (lock_overlay_dim == NULL || lock_overlay_pic == NULL) {
        if (!was_blocked) {
            elunchbox_te_block_flag = 0;
        }
        return;
    }

    if (!home_ui_lock_overlay_load_icon(unlock_icon, &icon_w, &icon_h)) {
        home_ui_lock_overlay_hide();
        if (!was_blocked) {
            elunchbox_te_block_flag = 0;
        }
        return;
    }

    if (!was_blocked) {
        elunchbox_te_block_flag = 0;
    }

    compo_shape_set_visible(lock_overlay_dim, true);
    compo_picturebox_set_ram(lock_overlay_pic, lock_overlay_ram);
    compo_picturebox_set_pos(lock_overlay_pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_size(lock_overlay_pic, icon_w, icon_h);
    compo_picturebox_set_visible(lock_overlay_pic, true);
    lock_overlay_visible = true;
    home_ui_lock_overlay_bring_front_internal();

    /* 让主循环中的 compo_update() + gui_process() 自然渲染 overlay，
       避免在此处强制 home_gpu_wait_idle() + os_gui_draw_force()
       阻塞 GUI 线程导致 "gui thread miss" 及锁图标显示延迟 */
    WDT_CLR();
}

#endif
