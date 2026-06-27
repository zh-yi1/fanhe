#include "include.h"
#include "home_ui_gpu_detach.h"
#include "home_ui_shared.h"

#if ELUNCHBOX_PANEL_EN
/* ELUNCHBOX 模式：TE block 标志声明 */
extern volatile u8 elunchbox_te_block_flag;
#endif

void home_ui_gpu_pic_detach_light(compo_picturebox_t *pic)
{
    if (pic == NULL) {
        return;
    }
    compo_picturebox_set_visible(pic, false);
    compo_picturebox_set(pic, 0);
}

void home_ui_gpu_pic_detach(compo_picturebox_t *pic)
{
    if (pic == NULL) {
        return;
    }
    /* 安全 detach：使用 set(0) 清除资源绑定（适用于 flash 模式和 ram 模式），
     * 而不是 set_ram(0)（只适用于 ram 模式，可能会破坏 flash 模式的资源描述符）。
     * 在修改资源绑定之前，确保 GPU 完成前一帧操作。
     */
#if ELUNCHBOX_PANEL_EN
    u8 was_blocked = elunchbox_te_block_flag;
    if (!was_blocked) {
        elunchbox_te_block_flag = 1;
    }
#endif
    home_gpu_wait_idle();
    os_gui_draw_force();
    home_gpu_wait_idle();
    compo_picturebox_set_visible(pic, false);
    compo_picturebox_set(pic, 0);
    home_gpu_wait_idle();
#if ELUNCHBOX_PANEL_EN
    if (!was_blocked) {
        elunchbox_te_block_flag = 0;
    }
#endif
}

void home_ui_gpu_pics_detach(compo_picturebox_t * const *pics, u8 cnt)
{
    u8 i;

    if (pics == NULL || cnt == 0) {
        return;
    }
    home_gpu_wait_idle();
    for (i = 0; i < cnt; i++) {
        home_ui_gpu_pic_detach(pics[i]);
    }
    home_gpu_wait_idle();
}

void home_ui_pic_set_flash(compo_picturebox_t *pic, u32 flash_addr, u16 w, u16 h)
{
    if (pic == NULL || flash_addr == 0 || w == 0 || h == 0) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return;
    }
#if ELUNCHBOX_PANEL_EN
    /* ELUNCHBOX 模式：设置 TE block 标志（嵌套保护：如果已经设置则不重复设置） */
    u8 was_blocked = elunchbox_te_block_flag;
    if (!was_blocked) {
        elunchbox_te_block_flag = 1;
    }
#endif
    /* 确保GPU已完成使用该pic的前一帧资源，再改绑定，避免C241 */
    home_gpu_wait_idle();
    compo_picturebox_set(pic, flash_addr);
    compo_picturebox_set_size(pic, w, h);
    compo_picturebox_set_visible(pic, true);
    home_gpu_wait_idle();
#if ELUNCHBOX_PANEL_EN
    /* 清除 TE block 标志（只有本层设置的才清除） */
    if (!was_blocked) {
        elunchbox_te_block_flag = 0;
    }
#endif
}

void home_ui_status_apply_flash(compo_picturebox_t *pic_bt, compo_picturebox_t *pic_lock,
                                compo_picturebox_t *pic_bat, bool show_lock)
{
    home_ui_pic_set_flash(pic_bt, UI_BUF_HOME_BLUETOOTH_BIN, HOME_STATUS_BT_W, HOME_STATUS_BT_H);
    home_ui_pic_set_flash(pic_bat, home_ui_shared_battery_flash_addr(), HOME_STATUS_BAT_W, HOME_STATUS_BAT_H);
    if (show_lock) {
        home_ui_pic_set_flash(pic_lock, UI_BUF_HOME_LOCK_BIN, HOME_STATUS_LOCK_W, HOME_STATUS_LOCK_H);
    } else if (pic_lock != NULL) {
        compo_picturebox_set_visible(pic_lock, false);
    }
}
