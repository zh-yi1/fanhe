#ifndef _NEW_HOME_TOP_TIME_H
#define _NEW_HOME_TOP_TIME_H

#include "home_top_time.h"

/* 白底主页：使用 new_ui 目录下 new_0m..new_9m 等 bin（白底合成） */
void new_home_top_time_create(compo_form_t *frm, u32 placeholder,
                              u16 id_h10, u16 id_h1, u16 id_colon,
                              u16 id_m10, u16 id_m1, u16 id_ampm);
void new_home_top_time_bind(home_top_time_ui_t *ui, u16 id_h10, u16 id_h1, u16 id_colon,
                            u16 id_m10, u16 id_m1, u16 id_ampm);
bool new_home_top_time_refresh(home_top_time_ui_t *ui, tm_t *tm);
void new_home_top_time_gpu_detach(home_top_time_ui_t *ui);

#endif
