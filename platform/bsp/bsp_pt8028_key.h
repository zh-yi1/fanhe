#ifndef _BSP_PT8028_KEY_H
#define _BSP_PT8028_KEY_H

#include "bsp_key.h"

/*
 * PT8028S BCD 通道号，与 D2:D1:D0 数值一致（表2）
 *
 * 表2 状态机（规格书）：
 *   OUT_FLAG=1, D=111  -> 上电/空闲（不是按键；D 线 Hold 上次值时 OUT_FLAG 必为 1）
 *   OUT_FLAG=0, D=0~7  -> 对应 TCH0~TCH7 按下（仅在 OUT_FLAG=0 时读 D 判键）
 *   OUT_FLAG=1, D=Hold -> 释放沿，用 Hold 或按下阶段 hist 触发释放逻辑
 *
 * OUT_FLAG 边沿（PE1，低有效）：
 *   下降沿 1→0：按下，采样 BCD 并锁存
 *   上升沿 0→1：释放，Hold=按下锁存，判键/发消息
 */
typedef enum {
    PT8028_KEY_NONE = 0xFF,
    PT8028_KEY_TCH0 = 0,
    PT8028_KEY_TCH1 = 1,
    PT8028_KEY_TCH2 = 2,
    PT8028_KEY_TCH3 = 3,
    PT8028_KEY_TCH4 = 4,
    PT8028_KEY_TCH5 = 5,
    PT8028_KEY_TCH6 = 6,
    PT8028_KEY_TCH7 = 7,
} pt8028_key_id_t;

/* Home 页：一次按键只产生一个动作 */
#define PT8028_HOME_ACT_NONE            0
#define PT8028_HOME_ACT_MODE            1
#define PT8028_HOME_ACT_CONFIRM         2

void pt8028_key_init(void);

/* 供 bsp_key_scan 轮询：按下期间 NO_KEY 或 KEY_* */
u8 get_pt8028_key(void);

/* 释放沿产生的 KU_*，取走后清零；无则返回 NO_KEY */
u16 pt8028_pop_short_up(void);

/* OUT_FLAG 低或本次按键尚未处理完，用于屏蔽 PWRKEY 误抢 */
bool pt8028_key_busy(void);

/* 按键消息路径：有效按压会话内返回 TCH0~7 */
u8 pt8028_get_press_tch(void);

/* LED 联动：仅 OUT_FLAG=0（按下）时根据 session/BCD 亮对应灯，松开即灭 */
u8 pt8028_get_led_tch(void);

void pt8028_get_raw_state(u8 *out_flag, u8 *bcd, u8 *d0, u8 *d1, u8 *d2);
const char *pt8028_get_last_key_str(void);

/* 主线程：打印 ISR 里缓存的按键/边沿日志 */
void pt8028_log_flush(void);

/* 主线程：GPIO 变化时打印原始电平 */
void pt8028_gpio_monitor(void);

/* 仅在 GPIO 未配置时恢复 PE1~4（热路径无 GPIOEDE 读） */
void pt8028_gpio_ensure(void);

/* 主线程 500ms 检测 GPIOEDE 是否被 LCD/SD 等改写 */
void pt8028_gpio_ensure_periodic(void);

/* pt8028_port_gpio_init 完成后标记已配置 */
void pt8028_gpio_mark_configured(void);

/* LCD/GUI 等可能清 GPIO 前置无效，下次 ensure 会恢复 */
void pt8028_gpio_invalidate(void);

/* 周期重新配置 PT8028 GPIO，防止被 SD/LCD 复用覆盖 */
void pt8028_poll_reinit(void);

#if ELUNCHBOX_PANEL_EN
/* 饭盒：主线程 5ms 节拍扫描（勿放 5ms 中断，避免 tmr thread miss） */
void pt8028_key_scan(void);

/* Home 页由 func_home 取 release_tch 处理，屏蔽消息队列 */
void pt8028_set_home_msg_block(u8 en);
void pt8028_release_clear(void);
u8 pt8028_take_press_tch(void);
u8 pt8028_take_release_tch(void);
u8 pt8028_take_home_action(void);
bool pt8028_take_res_key_pending(void);
/* 主线程取走待上报的按键 TCH，无则返回 0xff */
u8 pt8028_take_key_notify_tch(void);
#endif

#endif // _BSP_PT8028_KEY_H
