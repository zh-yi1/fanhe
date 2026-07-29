#include "include.h"

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN

#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#include "func_key_lock.h"
#include "func_key.h"
#include "func.h"

/*
 * 统一按键模块 — 所有按键处理集中于此。
 *
 * 参考 SDK key_process() 模式：轮询扫描 → 长按检测 → 童锁过滤 → 事件入队。
 *
 * func_key_poll() 每帧调用一次，内部处理：
 *   1. 底层扫描 (pt8028_key_scan_page)
 *   2. TCH0 长按 3s → func_key_lock_toggle()
 *   3. TCH5 长按 3s → 关机流程
 *   4. 锁定态 TCH5 按下/松开 → func_key_lock_on_pwr_key_*()
 *   5. 锁定态吞键 → func_key_lock_on_blocked_key()
 *   6. 未锁定键 → activity_reset + 入事件队列
 *
 * func_key_lock 模块不访问任何按键驱动，仅提供状态查询 + UI 回调。
 */

/* 内部事件队列 */
#define FUNC_KEY_QUEUE_SIZE 3

static func_key_event_t key_queue[FUNC_KEY_QUEUE_SIZE];
static u8 key_queue_head;
static u8 key_queue_count;

/* TCH0 长按检测状态 */
static u8  lock_lp_tch;           /* 当前跟踪的 TCH */
static u32 lock_lp_tick;          /* 按下时刻 */
static bool lock_lp_wait_rel;     /* 等待松手 */

/* TCH5 长按检测状态 */
static u8  pwr_lp_tch;
static u32 pwr_lp_tick;
static u8  pwr_lp_prev_held;      /* 上一帧是否按住 TCH5（用于检测松手沿） */

static void key_queue_push(func_key_event_t evt)
{
    if (key_queue_count >= FUNC_KEY_QUEUE_SIZE) {
        key_queue_head = (key_queue_head + 1) % FUNC_KEY_QUEUE_SIZE;
        key_queue_count--;
    }
    u8 idx = (key_queue_head + key_queue_count) % FUNC_KEY_QUEUE_SIZE;
    key_queue[idx] = evt;
    key_queue_count++;
}

/*===========================================================================
 * TCH0 长按检测 — 锁/解锁切换
 *===========================================================================*/

static void func_key_handle_lock_long(u8 held_tch)
{
    if (lock_lp_wait_rel) {
        /* 等待 TCH0 松手后再接受下一次长按 */
        if (held_tch != PT8028_KEY_TCH0) {
            lock_lp_wait_rel = false;
            lock_lp_tch = PT8028_KEY_NONE;
            lock_lp_tick = 0;
            func_key_lock_on_all_keys_released();
        }
        return;
    }

    if (held_tch == PT8028_KEY_TCH0) {
        if (lock_lp_tch != PT8028_KEY_TCH0) {
            /* TCH0 按下沿 */
            lock_lp_tch = PT8028_KEY_TCH0;
            lock_lp_tick = tick_get();
        } else if (tick_check_expire(lock_lp_tick, PT8028_LOCK_LONG_MS)) {
            /* 满 3s → 切换锁状态 */
            func_key_lock_toggle();
            lock_lp_wait_rel = true;
            lock_lp_tch = PT8028_KEY_NONE;
            lock_lp_tick = 0;
        }
    } else {
        /* TCH0 松手（未满 3s）→ 短按蜂鸣 */
        if (func_key_lock_is_active() && lock_lp_tch == PT8028_KEY_TCH0) {
            pt8028_defer_key_sound_tch(PT8028_KEY_TCH0);
            func_key_lock_on_blocked_key(PT8028_KEY_TCH0);
        }
        lock_lp_tch = PT8028_KEY_NONE;
        lock_lp_tick = 0;
    }
}

/*===========================================================================
 * TCH5 长按检测 — 关机
 *===========================================================================*/

static void func_key_handle_pwr_long(u8 held_tch)
{
    u8 prev = pwr_lp_prev_held;

    pwr_lp_prev_held = (held_tch == PT8028_KEY_TCH5) ? 1 : 0;

    if (held_tch == PT8028_KEY_TCH5) {
        if (pwr_lp_tch != PT8028_KEY_TCH5) {
            /* TCH5 按下沿 */
            pwr_lp_tch = PT8028_KEY_TCH5;
            pwr_lp_tick = tick_get();
            if (func_key_lock_is_active()) {
                func_key_lock_on_pwr_key_in_lock();
            }
        } else {
            /* TCH5 持续按住 — 长按关机由 bsp_pt8028_key 层检测 */
        }
    } else {
        /* TCH5 未按住 */
        if (prev) {
            /* 松手沿 */
            func_key_lock_on_pwr_key_release();
        }
        pwr_lp_tch = PT8028_KEY_NONE;
        pwr_lp_tick = 0;
    }
}

/*===========================================================================
 * 所有按键松开检测
 *===========================================================================*/

static bool func_key_any_key_held(void)
{
    return (pt8028_get_press_tch() <= PT8028_KEY_TCH7)
        || pt8028_is_press_active();
}

static u8 func_key_prev_any_held;

static void func_key_handle_all_keys_released(void)
{
    u8 cur = func_key_any_key_held() ? 1 : 0;

    if (func_key_prev_any_held && !cur) {
        /* 下降沿：所有键都已松开 */
        func_key_lock_on_all_keys_released();
    }
    func_key_prev_any_held = cur;
}

/*===========================================================================
 * 每帧轮询 — 按键处理总入口
 *===========================================================================*/

void func_key_poll(void)
{
    u8 held_tch;
    u8 press_tch;

    /* 1. 底层扫描 */
    pt8028_key_scan_page();

    /* 2. 获取当前按住键（供长按检测） */
    held_tch = pt8028_get_press_tch();

    /* 3. TCH5 长按/松手检测（关机 + 锁定态蜂鸣） */
    func_key_handle_pwr_long(held_tch);

    /* 4. TCH0 长按检测（锁/解锁切换） */
    func_key_handle_lock_long(held_tch);

    /* 5. 所有按键松开检测 */
    func_key_handle_all_keys_released();

    /* 6. 取按下沿事件 */
    press_tch = pt8028_take_press_tch();
    if (press_tch > PT8028_KEY_TCH7) {
        return;
    }

    /* 7. 童锁过滤 */
    if (func_key_lock_is_active()) {
        /* TCH0/TCH5 已由长按检测处理，此处吞掉避免重复 */
        if (press_tch == PT8028_KEY_TCH0 || press_tch == PT8028_KEY_TCH5) {
            return;
        }
        /* 其他键：吞掉 + 提示 */
        if (press_tch <= PT8028_KEY_TCH6 && press_tch != PT8028_KEY_TCH4) {
            elunchbox_user_activity_reset();
        }
        func_key_lock_on_blocked_key(press_tch);
        return;
    }

    /* 8. 非确认键重置用户活动计时器 */
    if (press_tch <= PT8028_KEY_TCH6 && press_tch != PT8028_KEY_TCH4) {
        elunchbox_user_activity_reset();
    }

    /* 9. 入队 */
    func_key_event_t evt;
    evt.type = FUNC_KEY_EVENT_PRESS;
    evt.tch = press_tch;
    key_queue_push(evt);

    /* 10. 清除 pt8028_key_scan 同时发到系统消息队列的 KU_* 消息，
     *     避免一次物理按键被 func_key handler 和 func_message 双重处理 */
    msg_queue_detach(KU_NEXT, 0);
    msg_queue_detach(KU_MODE, 0);
    msg_queue_detach(KU_BACK, 0);
    msg_queue_detach(KU_PREV, 0);
    msg_queue_detach(KU_LEFT, 0);
    msg_queue_detach(KU_RIGHT, 0);
    msg_queue_detach(KU_VOL_UP, 0);
    msg_queue_detach(KU_VOL_DOWN, 0);
}

/*===========================================================================
 * 事件获取
 *===========================================================================*/

bool func_key_get_event(func_key_event_t *evt)
{
    if (evt == NULL) return false;
    if (key_queue_count == 0) {
        evt->type = FUNC_KEY_EVENT_NONE;
        evt->tch = PT8028_KEY_NONE;
        return false;
    }
    *evt = key_queue[key_queue_head];
    key_queue_head = (key_queue_head + 1) % FUNC_KEY_QUEUE_SIZE;
    key_queue_count--;
    return true;
}

bool func_key_peek_event(func_key_event_t *evt)
{
    if (evt == NULL) return false;
    if (key_queue_count == 0) {
        evt->type = FUNC_KEY_EVENT_NONE;
        evt->tch = PT8028_KEY_NONE;
        return false;
    }
    *evt = key_queue[key_queue_head];
    return true;
}

bool func_key_has_event(void)
{
    return key_queue_count > 0;
}

/*===========================================================================
 * 逻辑映射
 *===========================================================================*/

func_key_logical_t func_key_map_logical(u8 tch)
{
    switch (tch) {
    case PT8028_KEY_TCH6: return FUNC_KEY_UP;           /* 加号短按 → 上一个 */
    case PT8028_KEY_TCH2: return FUNC_KEY_DOWN;         /* 减号短按 → 下一个 */
    case PT8028_KEY_TCH4: return FUNC_KEY_CONFIRM;      /* 确认短按 → 确定 */
    case PT8028_KEY_TCH5: return FUNC_KEY_BACK;         /* 开关短按 → 返回 */
    case PT8028_KEY_TCH3: return FUNC_KEY_MODE;         /* 模式短按 → 模式切换 */
    case PT8028_KEY_TCH1: return FUNC_KEY_HEAT;         /* 加热短按 → 加热页 */
    case PT8028_KEY_TCH7: return FUNC_KEY_RESERVATION;  /* 预约短按 → 预约页 */
    /* TCH0 锁键短按无效，只有长按3s才触发上锁/解锁（func_key_poll内部处理） */
    case PT8028_KEY_TCH0:
    default:                return FUNC_KEY_NONE;
    }
}

/*===========================================================================
 * 生命周期
 *===========================================================================*/

void func_key_flush(void)
{
    key_queue_head = 0;
    key_queue_count = 0;

    {
        u8 drain_tch;
        while (func_key_take_raw_press(&drain_tch)) {}
    }

    msg_queue_detach(KU_NEXT, 0);
    msg_queue_detach(KU_MODE, 0);
    msg_queue_detach(KU_BACK, 0);
    msg_queue_detach(KU_PREV, 0);
    msg_queue_detach(KU_LEFT, 0);
    msg_queue_detach(KU_RIGHT, 0);
    msg_queue_detach(KU_VOL_UP, 0);
    msg_queue_detach(KU_VOL_DOWN, 0);

    /* 重置长按状态 */
    lock_lp_tch = PT8028_KEY_NONE;
    lock_lp_tick = 0;
    lock_lp_wait_rel = false;
    pwr_lp_tch = PT8028_KEY_NONE;
    pwr_lp_tick = 0;
    pwr_lp_prev_held = 0;
    func_key_prev_any_held = 0;
}

void func_key_reset(void)
{
    key_queue_head = 0;
    key_queue_count = 0;
    lock_lp_tch = PT8028_KEY_NONE;
    lock_lp_tick = 0;
    lock_lp_wait_rel = false;
    pwr_lp_tch = PT8028_KEY_NONE;
    pwr_lp_tick = 0;
    pwr_lp_prev_held = 0;
    func_key_prev_any_held = 0;
}

/*===========================================================================
 * 底层访问 — 仅供 key 子系统内部模块使用（func_key_lock 兼容路径等）
 *===========================================================================*/

u8 func_key_get_held_tch(void)
{
    return pt8028_get_press_tch();
}

bool func_key_is_held(void)
{
    return pt8028_is_press_active();
}

bool func_key_peek_raw_press(u8 *tch)
{
    u8 val;
    if (tch == NULL) return false;
    val = pt8028_peek_press_tch();
    *tch = val;
    return (val <= PT8028_KEY_TCH7);
}

bool func_key_take_raw_press(u8 *tch)
{
    u8 val;
    if (tch == NULL) return false;
    val = pt8028_take_press_tch();
    *tch = val;
    return (val <= PT8028_KEY_TCH7);
}

void func_key_release_clear(void)
{
    pt8028_release_clear();
}

void func_key_defer_sound(u8 tch)
{
    pt8028_defer_key_sound_tch(tch);
}

void func_key_commit_pwr_long(void)
{
    pt8028_try_commit_pwr_long();
}

u8 func_key_tch_to_lunchbox_val(u8 tch)
{
    return pt8028_tch_to_lunchbox_key(tch);
}

#endif /* USER_PT8028_KEY && ELUNCHBOX_PANEL_EN */
