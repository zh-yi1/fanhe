#ifndef _FUNC_KEY_H
#define _FUNC_KEY_H

#include "include.h"

/*
 * 统一按键事件模块
 *
 * 参考 SDK key_process() 模式：轮询扫描 → 类型化事件 → 页面分发。
 * 封装 pt8028 底层细节（scan_page / take_press / key_lock / 按键音），
 * 页面只需 poll + get_event 即可拿到已过滤的按键事件。
 *
 * 物理 TCH 映射（PT8028S 触控面板）：
 *   TCH0=锁键  TCH1=加热  TCH2=减号  TCH3=模式
 *   TCH4=确认  TCH5=开关  TCH6=加号  TCH7=预约
 */

/* 按键事件类型 */
typedef enum {
    FUNC_KEY_EVENT_NONE = 0,
    FUNC_KEY_EVENT_PRESS,        /* 短按按下沿 */
    FUNC_KEY_EVENT_RELEASE,      /* 短按释放沿（预留，当前 PT8028 主要用 PRESS） */
    FUNC_KEY_EVENT_LONG,         /* 长按 3s（TCH0 童锁 / TCH5 关机） */
} func_key_event_type_t;

/* 统一按键事件 */
typedef struct {
    func_key_event_type_t type;
    u8 tch;                      /* PT8028_KEY_TCH0~7，无效时为 PT8028_KEY_NONE */
} func_key_event_t;

/*
 * 逻辑按键 — 反映按键的实际行为（非物理名字）。
 *
 * 物理 → 逻辑映射规则：
 *   TCH6(加号) 短按 → UP        切换/上一个
 *   TCH2(减号) 短按 → DOWN      切换/下一个
 *   TCH4(确认) 短按 → CONFIRM   确定/进入
 *   TCH5(开关) 短按 → BACK      返回
 *   TCH5(开关) 长按3s → POWER   开关机（func_key 内部处理）
 *   TCH0(锁键) 长按3s → LOCK    上锁/解锁（func_key 内部处理）
 *   TCH0(锁键) 短按 → 无效
 *   TCH1(加热) 短按 → HEAT      跳转加热页
 *   TCH3(模式) 短按 → MODE      模式切换
 *   TCH7(预约) 短按 → RESERVATION  跳转预约页
 */
typedef enum {
    FUNC_KEY_NONE = 0,
    FUNC_KEY_UP,                 /* 上/下一个（TCH6 短按） */
    FUNC_KEY_DOWN,               /* 下/上一个（TCH2 短按） */
    FUNC_KEY_CONFIRM,            /* 确定（TCH4 短按） */
    FUNC_KEY_BACK,               /* 返回（TCH5 短按） */
    FUNC_KEY_MODE,               /* 模式切换（TCH3 短按） */
    FUNC_KEY_HEAT,               /* 跳转加热（TCH1 短按） */
    FUNC_KEY_RESERVATION,        /* 跳转预约（TCH7 短按） */
    FUNC_KEY_POWER,              /* 开关机（TCH5 长按3s，func_key内部处理） */
    FUNC_KEY_LOCK,               /* 上锁/解锁（TCH0 长按3s，func_key内部处理） */
} func_key_logical_t;

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN

/*===========================================================================
 * 每帧轮询 — 在页面 process 开头调用（func_process 之前）
 *===========================================================================*/

/* 扫描按键并填充内部事件队列（已过滤童锁、已处理 activity reset） */
void func_key_poll(void);

/*===========================================================================
 * 事件获取
 *===========================================================================*/

/* 取下一个按键事件，返回 true 表示有效；无事件时返回 false */
bool func_key_get_event(func_key_event_t *evt);

/* 偷看下一个事件但不消费 */
bool func_key_peek_event(func_key_event_t *evt);

/* 当前是否有待处理事件 */
bool func_key_has_event(void);

/*===========================================================================
 * 逻辑映射 — 把物理 TCH 转成方向/确认/返回等语义
 *===========================================================================*/

/* 将物理 TCH 一对一映射为逻辑按键 */
func_key_logical_t func_key_map_logical(u8 tch);

/*===========================================================================
 * 生命周期
 *===========================================================================*/

/* 清空事件队列 + 排空消息队列残留（切页时调用） */
void func_key_flush(void);

/* 重置模块状态（页面 enter 时调用） */
void func_key_reset(void);

/*===========================================================================
 * 底层访问 — 仅供 key 子系统内部模块使用（func_key_lock 等），页面勿调
 *===========================================================================*/

/* 当前按住的 TCH（无键按下返回 PT8028_KEY_NONE） */
u8 func_key_get_held_tch(void);

/* 是否有键正在按下 */
bool func_key_is_held(void);

/* 偷看底层按下沿事件（不消费） */
bool func_key_peek_raw_press(u8 *tch);

/* 取走底层按下沿事件（消费） */
bool func_key_take_raw_press(u8 *tch);

/* 清空底层 press/release pending */
void func_key_release_clear(void);

/* 登记按键音（供锁模块在吞键后手动触发蜂鸣） */
void func_key_defer_sound(u8 tch);

/* 兜底提交 TCH5 长按关机 */
void func_key_commit_pwr_long(void);

/* TCH → lunchbox UART 按键值 */
u8 func_key_tch_to_lunchbox_val(u8 tch);

#else /* !(USER_PT8028_KEY && ELUNCHBOX_PANEL_EN) */

/* Stub — 非 ELUNCHBOX 面板模式 */

static inline void func_key_poll(void) {}
static inline bool func_key_get_event(func_key_event_t *evt)
{
    if (evt != NULL) { evt->type = FUNC_KEY_EVENT_NONE; evt->tch = 0xff; }
    return false;
}
static inline bool func_key_peek_event(func_key_event_t *evt)
{
    if (evt != NULL) { evt->type = FUNC_KEY_EVENT_NONE; evt->tch = 0xff; }
    return false;
}
static inline bool func_key_has_event(void) { return false; }
static inline func_key_logical_t func_key_map_logical(u8 tch) { (void)tch; return FUNC_KEY_NONE; }
static inline void func_key_flush(void) {}
static inline void func_key_reset(void) {}

static inline u8 func_key_get_held_tch(void) { return 0xff; }
static inline bool func_key_is_held(void) { return false; }
static inline bool func_key_peek_raw_press(u8 *tch) { if (tch) *tch = 0xff; return false; }
static inline bool func_key_take_raw_press(u8 *tch) { if (tch) *tch = 0xff; return false; }
static inline void func_key_release_clear(void) {}
static inline void func_key_defer_sound(u8 tch) { (void)tch; }
static inline void func_key_commit_pwr_long(void) {}
static inline u8 func_key_tch_to_lunchbox_val(u8 tch) { (void)tch; return 0; }

#endif /* USER_PT8028_KEY && ELUNCHBOX_PANEL_EN */

#endif /* _FUNC_KEY_H */
