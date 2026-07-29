#ifndef __ELUNCHBOX_LP_H
#define __ELUNCHBOX_LP_H

#include "include.h"

/* ============================================================
 * 饭盒低功耗状态机
 *
 * 息屏流程:
 *   5分钟无操作 → idle_tmr 到 0 → screen_off
 *   → sleep_delay 30s → guioff_sleep_ready → 深睡
 *
 * 唤醒: PE1/PF1/PF2 port_wakeup / 充电 / 预约加热
 * ============================================================ */

/* --- 状态查询 --- */
bool elunchbox_is_guioff(void);
bool elunchbox_pwr_gui_off_is_on(void);
bool elunchbox_guioff_sleep_ready(void);
bool elunchbox_guioff_idle_expired(void);

/* --- 操作 --- */
void elunchbox_user_activity_reset(void);   /* 有操作时重置空闲计时 */
void elunchbox_lp_user_activity_reset(void);  /* 重置空闲计时 (由 port_pt8028_key 桥接) */
void elunchbox_guioff_idle_tick(void);      /* 100ms tick: 空闲倒计时 */
void elunchbox_guioff_sleep_delay_tick(void); /* 100ms tick: 息屏→深睡倒计时 */
void elunchbox_guioff_sleep_delay_reset(void);
void elunchbox_guioff_sleep_arm_immediate(void); /* 息屏后立刻允许深睡 (TCH5 长按) */

void elunchbox_screen_off(void);            /* 关屏 */
void elunchbox_pwr_gui_wake(void);          /* 亮屏唤醒 */

/* 物理屏幕控制 (替代 gui_sleep/gui_wakeup，仅关物理屏，保持GPU链路) */
void lunchbox_display_off(void);
void lunchbox_display_on(void);

#endif /* __ELUNCHBOX_LP_H */
