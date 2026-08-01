#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"
#include "general_ui.h"

#if ELUNCHBOX_PANEL_EN

/* ---- 锁屏弹窗（叠加在现有 form 上，不切换页面） ---- */

#define LOCK_RAM_SIZE  22000

static compo_form_t       *g_lock_frm;   /* 控件所属 form；切页后必须重建 */
static compo_shape_t      *g_lock_dim;
static compo_picturebox_t *g_lock_pic;
static u8                 *g_lock_ram;
static bool                g_lock_visible;
static u32                 g_lock_cached_addr;  /* g_lock_ram 里缓存的图标资源地址, 0=无 */

static void lock_cleanup(void)
{
    /* 不主动 destroy：控件随 form 销毁；此处仅丢弃悬空指针 */
    g_lock_frm = NULL;
    g_lock_dim = NULL;
    g_lock_pic = NULL;
    g_lock_visible = false;
    g_lock_cached_addr = 0;      /* form 切换后 RAM 绑定失效, 重新读 */
}

static bool lock_load_icon(bool unlock_icon, u16 *out_w, u16 *out_h)
{
    u32 addr = unlock_icon ? UI_BUF_NEW_UI_NEW_UNLOCK_BIN
                           : UI_BUF_NEW_UI_NEW_LOCK_BIN;
    u32 len  = unlock_icon ? UI_LEN_NEW_UI_NEW_UNLOCK_BIN
                           : UI_LEN_NEW_UI_NEW_LOCK_BIN;

    *out_w = unlock_icon ? 106 : 80;
    *out_h = unlock_icon ? 100 : 80;

    /* 同一图标不重复读 Flash: 锁振荡时每帧都调 show, 22KB SPI 读 + GPU 等
     * 待没有 WDT_CLR 会让看门狗饿死 → WDT_RST。
     * 锁/解锁两张图共用 g_lock_ram, 必须比对地址, 图标切换时重新读 */
    if (g_lock_cached_addr == addr && g_lock_ram != NULL) {
        return gui_set_ram_check(g_lock_ram, __func__);
    }

    if (len == 0 || len > LOCK_RAM_SIZE) {
        return false;
    }
    if (g_lock_ram == NULL) {
        g_lock_ram = (u8 *)ab_malloc(LOCK_RAM_SIZE);
    }
    if (g_lock_ram == NULL) {
        return false;
    }

    WDT_CLR();
    home_gpu_wait_idle();
    WDT_CLR();
    os_spiflash_read(g_lock_ram, addr, len);
    WDT_CLR();
    if (gui_set_ram_check(g_lock_ram, __func__)) {
        g_lock_cached_addr = addr;
        return true;
    }
    return false;
}

static bool lock_ensure_widgets(compo_form_t *frm)
{
    if (frm == NULL) {
        return false;
    }

    /* 已有控件且仍属于当前 form → 复用 */
    if (g_lock_dim != NULL && g_lock_pic != NULL && g_lock_frm == frm) {
        return true;
    }

    /* form 已切换或首次创建：旧 form 销毁后指针失效，必须重建 */
    lock_cleanup();
    g_lock_frm = frm;

    /* 半透明白色遮罩 */
    g_lock_dim = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    if (g_lock_dim == NULL) {
        lock_cleanup();
        return false;
    }
    compo_shape_set_color(g_lock_dim, COLOR_WHITE);
    compo_shape_set_location(g_lock_dim, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_alpha(g_lock_dim, 160);
    compo_shape_set_visible(g_lock_dim, false);

    /* 锁图标 */
    g_lock_pic = (compo_picturebox_t *)compo_create(frm, COMPO_TYPE_PICTUREBOX);
    if (g_lock_pic == NULL) {
        lock_cleanup();
        return false;
    }
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

    if (func_cb.frm_main == NULL) {
        return;
    }

    if (!lock_ensure_widgets(func_cb.frm_main)) {
        return;
    }

    if (!lock_load_icon(unlock_icon, &icon_w, &icon_h)) {
        return;
    }

    /* set_ram 前再确认控件仍属于当前 form，避免悬空 → resource halt */
    if (g_lock_frm != func_cb.frm_main || g_lock_pic == NULL || g_lock_pic->img == NULL) {
        lock_cleanup();
        return;
    }

    home_gpu_wait_idle();
    compo_shape_set_visible(g_lock_dim, true);
    compo_picturebox_set_ram(g_lock_pic, g_lock_ram);
    compo_picturebox_set_size(g_lock_pic, icon_w, icon_h);
    compo_picturebox_set_visible(g_lock_pic, true);

    if (g_lock_dim->rect != NULL) {
        widget_set_top(g_lock_dim->rect, true);
    }
    if (g_lock_pic->img != NULL) {
        widget_set_top(g_lock_pic->img, true);
    }

    g_lock_visible = true;
}

void func_lock_page_hide(void)
{
    printf("exit the lock\n");
    g_lock_visible = false;

    /* form 已销毁时指针已在 cleanup 清掉，勿再访问 */
    if (g_lock_frm != func_cb.frm_main) {
        lock_cleanup();
        return;
    }
    if (g_lock_dim != NULL) {
        compo_shape_set_visible(g_lock_dim, false);
    }
    if (g_lock_pic != NULL) {
        compo_picturebox_set_visible(g_lock_pic, false);
    }
}

/* form 销毁前调用：丢弃悬空指针，下一页按需重建 */
void func_lock_page_on_form_destroy(void)
{
    lock_cleanup();
}

#endif /* ELUNCHBOX_PANEL_EN */
