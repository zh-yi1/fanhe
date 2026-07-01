#ifndef _FUNC_HEAT_PANEL_H
#define _FUNC_HEAT_PANEL_H

#include "include.h"

#if ELUNCHBOX_PANEL_EN

struct f_heat_t_;

compo_form_t *func_heat_panel_form_create(void);
void func_heat_panel_bind(struct f_heat_t_ *f_heat);
void func_heat_panel_enter(struct f_heat_t_ *f_heat);
void func_heat_panel_exit(void);
void func_heat_panel_mark_dirty(struct f_heat_t_ *f_heat);
void func_heat_panel_process(struct f_heat_t_ *f_heat);
void func_heat_panel_status_refresh(struct f_heat_t_ *f_heat);

#endif

#endif
