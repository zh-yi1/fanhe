#ifndef _HOME_TOP_TIME_TXT_H
#define _HOME_TOP_TIME_TXT_H

#include "home_top_time.h"

typedef struct home_top_time_txt_t_ {
    compo_textbox_t *txt;
    u16 last_key;
    bool font_ready;
} home_top_time_txt_t;

void home_top_time_txt_create(compo_form_t *frm, u16 id);
void home_top_time_txt_bind(home_top_time_txt_t *ui, u16 id);
bool home_top_time_txt_refresh(home_top_time_txt_t *ui, tm_t *tm);
bool home_top_time_txt_tick(home_top_time_txt_t *ui, u8 *last_min, u8 *last_sec);
void home_top_time_txt_force(home_top_time_txt_t *ui, u8 *last_min, u8 *last_sec);
void home_top_time_txt_keep_visible(home_top_time_txt_t *ui);
void home_top_time_txt_bring_front(home_top_time_txt_t *ui);

#endif
