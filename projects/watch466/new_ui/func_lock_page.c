#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"
#include "general_ui.h"

#if ELUNCHBOX_PANEL_EN

/* ---- 锁屏弹窗（叠加在现有 form 上，不切换页面） ---- */

#define LOCK_RAM_SIZE  22000

static compo_shape_t      *g_lock_dim;
static compo_picturebox_t *g_lock_pic;
static u8                 *g_lock_ram;
static bool                g_lock_visible;

static void lock_cleanup(void)
{
    g_lock_dim = NULL;
    g_lock_pic = NULL;
    g_lock_visible = false;
}

static bool lock_load_icon(bool unlock_icon, u16 *out_w, u16 *out_h)
{
    u32 addr = unlock_icon ? UI_BUF_NEW_UI_NEW_UNLOCK_BIN
                           : UI_BUF_NEW_UI_NEW_LOCK_BIN;
    u32 len  = unlock_icon ? UI_LEN_NEW_UI_NEW_UNLOCK_BIN
                           : UI_LEN_NEW_UI_NEW_LOCK_BIN;

    *out_w = unlock_icon ? 106 : 80;
    *out_h = unlock_icon ? 100 : 80;

    if (len == 0 || len > LOCK_RAM_SIZE) return false;
    if (g_lock_ram == NULL) {
        g_lock_ram = (u8 *)ab_malloc(LOCK_RAM_SIZE);
    }
    if (g_lock_ram == NULL) return false;

    home_gpu_wait_idle();
    os_spiflash_read(g_lock_ram, addr, len);
    WDT_CLR();
    return gui_set_ram_check(g_lock_ram, __func__);
}

static bool lock_ensure_widgets(compo_form_t *frm)
{
    if (frm == NULL) return false;
    if (g_lock_dim != NULL && g_lock_pic != NULL) return true; /* already created */

    /* 半透明白色遮罩 */
    g_lock_dim = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    if (g_lock_dim == NULL) return false;
    compo_shape_set_color(g_lock_dim, COLOR_WHITE);
    compo_shape_set_location(g_lock_dim, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_alpha(g_lock_dim, 160);
    compo_shape_set_visible(g_lock_dim, false);

    /* 锁图标 — 照搬 overlay 创建方式 */
    g_lock_pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);
    if (g_lock_pic == NULL) return false;
    g_lock_pic->img = (void *)widget_image_create(frm->page_body, 0);
    g_lock_pic->radix = 1;
    compo_picturebox_set_pos(g_lock_pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_visible(g_lock_pic, false);

    return true;
}

void func_lock_page_show(bool unlock_icon)
{
    u16 icon_w, icon_h;

    printf("Enter the lock\n");

    if (func_cb.frm_main == NULL) return;

    /* form 切换后重建 */
    if (g_lock_visible && g_lock_dim == NULL) {
        lock_cleanup();
    }

    if (!lock_ensure_widgets(func_cb.frm_main)) return;

    if (!lock_load_icon(unlock_icon, &icon_w, &icon_h)) return;

    compo_shape_set_visible(g_lock_dim, true);
    compo_picturebox_set_ram(g_lock_pic, g_lock_ram);
    compo_picturebox_set_size(g_lock_pic, icon_w, icon_h);
    compo_picturebox_set_visible(g_lock_pic, true);

    /* 置顶 */
    if (g_lock_dim != NULL && g_lock_dim->rect != NULL)
        widget_set_top(g_lock_dim->rect, true);
    if (g_lock_pic != NULL && g_lock_pic->img != NULL)
        widget_set_top(g_lock_pic->img, true);

    g_lock_visible = true;
}

void func_lock_page_hide(void)
{
    printf("exit the lock\n");
    g_lock_visible = false;
    if (g_lock_dim != NULL) compo_shape_set_visible(g_lock_dim, false);
    if (g_lock_pic != NULL) compo_picturebox_set_visible(g_lock_pic, false);
}

#endif /* ELUNCHBOX_PANEL_EN */
