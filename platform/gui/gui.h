#ifndef _GUI_H
#define _GUI_H

#include "components.h"
#include "tft.h"
#include "ctp.h"
#include "rotate_disp/rotate_disp.h"
#include "rotate_disp/rotate_widget.h"

#define UI_BTN_CLICK_EFFECT_ALPHA1    180
#define UI_BTN_CLICK_EFFECT_ALPHA     120
#define UI_BTN_NORMAL_EFFECT_ALPHA    255

void gpu_init(void);
void gpu_exit(void);
void gui_init(void);
void gui_sw_init(void);
void gui_sleep(bool is_gpu_exit);
void gui_wakeup(void);
void gui_halt(u32 halt_no);
void de_fill_rgb565(void *buf, u16 color, int cnt);
void de_fill_num(void *buf, u32 num, int ln);
bool gui_set_ram_check(void* ptr, const char* func_name);
void* psram_switch_cache(void* ptr);
u8 is_gpu_init(void);
#if GUI_USE_SCREENSHOOT
extern u8 cur_scbuf[GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*2+8] AT(.psram_buf.lcd);
extern u8 next_scbuf[GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*2+8] AT(.psram_buf.lcd);
#if GUI_USE_BLUR
extern u8 blur_obuf[GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*2+8] AT(.psram_buf.lcd);
extern u8 blur_tbuf[GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*2*2] AT(.psram_buf.lcd);
#endif
#endif
#endif
