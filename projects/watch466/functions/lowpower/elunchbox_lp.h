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
 *
 * manual_off 模式:
 *   用户长按电源键 → 仅保留 PE1+PB9 唤醒源 → 超低功耗深度休眠
 *   Buck→LDO / VDDTK 关闭进一步省电
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

/* ============================================================
 * manual_off 深度休眠管理
 * ============================================================ */

/* --- manual_off 状态 --- */
bool elunchbox_pwr_is_manual_off(void);
void elunchbox_pwr_manual_off_set(void);
void elunchbox_pwr_manual_off_clr(void);

/* --- 唤醒控制 --- */
void elunchbox_pwr_intentional_wake_set(bool v);
bool elunchbox_pwr_manual_off_gui_wake_ok(void);

/* --- 唤醒 pending (PE1 software edge detect) --- */
void elunchbox_manual_off_sleep_poll(void);
bool elunchbox_manual_wake_pending_peek(void);
void elunchbox_manual_wake_pending_take(void);

/* --- guioff 深睡服务 --- */
bool elunchbox_guioff_in_sleep_mode(void);
void elunchbox_guioff_sleep_mode_enter(void);
void elunchbox_guioff_sleep_service(void);
void elunchbox_guioff_sleep_post_wake(bool wkp);

/* --- 亮屏 --- */
void elunchbox_pwr_gui_wake_reason(const char *reason);

/* --- 加热/空闲判断 --- */
bool elunchbox_heating_blocks_idle(void);
bool elunchbox_pwr_manual_off_should_stay_awake(void);

/* --- UART TX block ---
 * FUNC_LUNCHBOX_UART_EN=1 时实体在 comm/lb_uart_app.c (原型与 lb_uart_app.h 一致),
 * =0 时 elunchbox_lp.c 里有空壳。声明不加条件, 免得 func_lowpwr.c 报隐式声明。 */
void lb_uart_tx_block(bool block);

/* --- pwroff_sent 重置 --- */
void elunchbox_pwroff_sent_reset(void);

#endif /* __ELUNCHBOX_LP_H__ */
