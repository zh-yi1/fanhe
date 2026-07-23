#ifndef _FUNC_HEAT_PANEL_H
#define _FUNC_HEAT_PANEL_H

#include "include.h"

#if ELUNCHBOX_PANEL_EN

struct f_heat_t_;

compo_form_t *func_heat_panel_form_create(void);
void func_heat_panel_bind(struct f_heat_t_ *f_heat);
void func_heat_panel_enter(struct f_heat_t_ *f_heat);
void func_heat_panel_exit(void);
void func_heat_panel_exit_to_warm(void);
bool func_heat_panel_track_ram_valid(void);
bool func_heat_panel_ui_ready(void);
void func_heat_panel_track_ram_consume(void);
void func_heat_panel_mark_dirty(struct f_heat_t_ *f_heat);
void func_heat_panel_process(struct f_heat_t_ *f_heat);
void func_heat_panel_status_refresh(struct f_heat_t_ *f_heat);

/** 加热中：推送剩余时长和温度（UART/协议层 → UI 回调） */
void func_heat_panel_push_live(u32 heat_remain_min, u16 temp_f);
/** 进加热页同步 MCU 快照后调用，避免开局残留 remain=0 误判结束 */
void func_heat_panel_ack_mcu_live(u32 remain_min);

#endif

#endif
