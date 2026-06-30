#ifndef _HOME_UI_GPU_DETACH_H
#define _HOME_UI_GPU_DETACH_H

void home_ui_gpu_pic_detach(compo_picturebox_t *pic);
/* 轻量 detach：无逐 pic draw_force，切子页前释放共享 RAM 绑定用 */
void home_ui_gpu_pic_detach_light(compo_picturebox_t *pic);
void home_ui_gpu_pics_detach(compo_picturebox_t * const *pics, u8 cnt);
/* 批量 light detach + 单次 draw_force，切页 exit 用，避免逐 pic force 触发 gui thread miss */
void home_ui_gpu_pics_detach_light_flush(compo_picturebox_t * const *pics, u8 cnt);
void home_ui_pic_set_flash(compo_picturebox_t *pic, u32 flash_addr, u16 w, u16 h);
void home_ui_status_apply_flash(compo_picturebox_t *pic_bt, compo_picturebox_t *pic_lock,
                                compo_picturebox_t *pic_bat, bool show_lock);

#endif
