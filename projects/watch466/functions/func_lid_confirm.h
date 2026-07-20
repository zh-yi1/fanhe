#ifndef _FUNC_LID_CONFIRM_H
#define _FUNC_LID_CONFIRM_H

#include "include.h"

compo_form_t *func_lid_confirm_form_create(void);
void func_lid_confirm_enter(void);
void func_lid_confirm_exit(void);
void func_lid_confirm(void);

#if ELUNCHBOX_PANEL_EN
/** 冷启动上电：武装盖确认（仅 MCU 报加热中时才弹） */
void elunchbox_lid_confirm_arm_boot(void);
/** 取消武装（非加热 / 已处理完毕） */
void elunchbox_lid_confirm_disarm(void);
bool elunchbox_lid_confirm_is_armed(void);
/**
 * 若处于上电武装：改切到盖确认页，返回 true。
 * 调用方本意是进加热页时使用（拦截 switch_to_heat_panel）。
 */
bool elunchbox_lid_confirm_try_redirect(void);
/** 上电在 Home 收到 MCU 加热上报时弹出 */
void func_elunchbox_switch_to_lid_confirm(void);

/**
 * 上电检测到 MCU 加热/保温：先保存模式/剩余时间/温度，再下发停热。
 * 可重复调用：已捕获则跳过。
 */
void elunchbox_lid_confirm_capture_and_stop(u8 mode, u32 remain_min, bool got_remain,
                                            u16 temp_f, bool got_temp);
/** 已保存快照（等待 YES/NO），停热应答勿取消弹窗 */
bool elunchbox_lid_confirm_snap_valid(void);
void elunchbox_lid_confirm_snap_clear(void);
#else
static inline void elunchbox_lid_confirm_arm_boot(void) {}
static inline void elunchbox_lid_confirm_disarm(void) {}
static inline bool elunchbox_lid_confirm_is_armed(void) { return false; }
static inline bool elunchbox_lid_confirm_try_redirect(void) { return false; }
static inline void func_elunchbox_switch_to_lid_confirm(void) {}
static inline void elunchbox_lid_confirm_capture_and_stop(u8 mode, u32 remain_min,
                                                          bool got_remain, u16 temp_f,
                                                          bool got_temp)
{
    (void)mode; (void)remain_min; (void)got_remain; (void)temp_f; (void)got_temp;
}
static inline bool elunchbox_lid_confirm_snap_valid(void) { return false; }
static inline void elunchbox_lid_confirm_snap_clear(void) {}
#endif

#endif
