#ifndef _BSP_PT8028_KEY_H
#define _BSP_PT8028_KEY_H

#include "bsp_key.h"

/*
 * PT8028S BCD 通道号，与 D2:D1:D0 数值一致（表2）
 * 上电空闲 D=111(BCD7) 但 OUT_FLAG=1 时不作按键
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

void pt8028_key_init(void);

/* 供 bsp_key_scan 轮询：按下期间 NO_KEY 或 KEY_* */
u8 get_pt8028_key(void);

/* 释放沿产生的 KU_*，取走后清零；无则返回 NO_KEY */
u16 pt8028_pop_short_up(void);

/* OUT_FLAG 低或本次按键尚未处理完，用于屏蔽 PWRKEY 误抢 */
bool pt8028_key_busy(void);

/* 按键消息路径：有效按压会话内返回 TCH0~7 */
u8 pt8028_get_press_tch(void);

/* LED 联动：OUT_FLAG=0 且 BCD 有效时返回 TCH0~7，松开即 NONE */
u8 pt8028_get_led_tch(void);

void pt8028_get_raw_state(u8 *out_flag, u8 *bcd, u8 *d0, u8 *d1, u8 *d2);
const char *pt8028_get_last_key_str(void);

/* 主线程：打印 ISR 里缓存的按键/边沿日志 */
void pt8028_log_flush(void);

/* 主线程：GPIO 变化时打印原始电平 */
void pt8028_gpio_monitor(void);

/* 周期检查 PE1~4 数字使能，防止被 init 清掉 */
void pt8028_gpio_ensure(void);

/* 周期重新配置 PT8028 GPIO，防止被 SD/LCD 复用覆盖 */
void pt8028_poll_reinit(void);

#endif // _BSP_PT8028_KEY_H
