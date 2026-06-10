#ifndef _HOME_TOP_TIME_H
#define _HOME_TOP_TIME_H

#include "include.h"
#include "home_icon_res.h"

/* 左上 RTC：0m..9m + colonm + AMm/PMm（ui/home -> ui.bin -> set_ram） */
#if (GUI_SCREEN_WIDTH == 320) && (GUI_SCREEN_HEIGHT == 240)
#define HOME_TOP_TIME_X                   12
#define HOME_TOP_TIME_Y                   20
#else
#define HOME_TOP_TIME_REF_W               466
#define HOME_TOP_TIME_REF_H               466
#define HOME_TOP_TIME_X                   ((s16)((s32)78 * GUI_SCREEN_WIDTH / HOME_TOP_TIME_REF_W))
#define HOME_TOP_TIME_Y                   ((s16)((s32)48 * GUI_SCREEN_HEIGHT / HOME_TOP_TIME_REF_H))
#endif
#define HOME_TOP_TIME_ELEM_GAP            2
#define HOME_TOP_TIME_AMPM_GAP            4

typedef struct home_top_time_ui_t_ {
    compo_picturebox_t *pic_h10;
    compo_picturebox_t *pic_h1;
    compo_picturebox_t *pic_colon;
    compo_picturebox_t *pic_m10;
    compo_picturebox_t *pic_m1;
    compo_picturebox_t *pic_ampm;
    u16 last_key;
} home_top_time_ui_t;

void home_top_time_create(compo_form_t *frm, u32 placeholder, const u16 id_h10,
                          u16 id_h1, u16 id_colon, u16 id_m10, u16 id_m1, u16 id_ampm);
void home_top_time_bind(home_top_time_ui_t *ui, u16 id_h10, u16 id_h1, u16 id_colon,
                        u16 id_m10, u16 id_m1, u16 id_ampm);
bool home_top_time_refresh(home_top_time_ui_t *ui, tm_t *tm);

#endif
