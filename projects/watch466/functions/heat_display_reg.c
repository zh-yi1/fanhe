#include "include.h"
#include "heat_display_reg.h"
#include "func_lunchbox_uart.h"
#include "func.h"
#include "home_ui_shared.h"
#include "func_lid_confirm.h"
#if ELUNCHBOX_PANEL_EN
#include "func_reservation.h"
#include "func_lowbat.h"
#endif
#include "func_lunchbox_uart_internal.h"

static heat_display_cb_t heat_display_cb;
static heat_display_info_t heat_display_last;
static bool heat_display_has_last;
static bool heat_display_charge_pending;  /* 充电中状态唤醒标志 */
static bool heat_display_heat_pending;    /* 加热使能唤醒标志 */
#if ELUNCHBOX_PANEL_EN
static bool heat_display_warm_charge_remain_zero; /* 充电转保温后 remain=0 标志: 拔电后回主页 */
static bool heat_display_warm_charge_pending; /* 熄屏时充电+保温：唤醒后进保温页 */
static bool heat_display_charge_keep_warm_sent; /* 充电中加热结束后已下发 24h 保温 */
static u8 heat_display_cached_mcu_mode;       /* 最近 MCU 上报的 DP02 */
static bool heat_display_has_cached_mcu_mode;
#endif

static bool heat_display_ui_ok(void)
{
#if ELUNCHBOX_PANEL_EN
    return elunchbox_ui_is_live() && is_gpu_init();
#else
    return true;
#endif
}

#if ELUNCHBOX_PANEL_EN
static bool heat_display_charging_now(bool got_charge, u8 charge_val)
{
    if (got_charge) {
        return charge_val != 0;
    }
    return home_ui_shared_battery_is_charging();
}

static bool heat_display_warm_exit_pending;

void heat_display_warm_exit_reset(void)
{
    heat_display_warm_exit_pending = false;
    heat_display_warm_charge_remain_zero = false;
#if ELUNCHBOX_PANEL_EN
    heat_display_charge_keep_warm_sent = false;
#endif
}

#if ELUNCHBOX_PANEL_EN
/**
 * 充电中加热结束(MCU HeatEn=OFF)且仍插电：下发 24h 保温。
 * 充电仅切保温页时不下发，避免与 MCU 加热过程抢控。
 */
static void heat_display_try_send_charge_keep_warm(bool charging)
{
    if (!charging || heat_display_charge_keep_warm_sent) {
        return;
    }
    if (func_cb.sta != FUNC_NEW_WARM && func_cb.sta != FUNC_HEAT
        && !func_elunchbox_warm_from_charging()) {
#if FUNC_LUNCHBOX_UART_EN
        if (!lunchbox_heating_task_active() && !lunchbox_keep_warm_is_active()) {
            return;
        }
#else
        return;
#endif
    }
    heat_display_charge_keep_warm_sent = true;
#if FUNC_LUNCHBOX_UART_EN
    lb_heat_user_uart_tx_force_set(true);
    (void)lb_heat_uart_remote_consume();
    lunchbox_keep_warm_apply();
    printf("[LCD_REG] charging heat end -> keep_warm 24h\n");
#endif
}
#endif

/** 保温页 UART 退出去重：同一次拔电/停止只触发一次 stop_and_home */
static bool heat_display_warm_exit_try(void)
{
    if (func_cb.sta != FUNC_NEW_WARM) {
        heat_display_warm_exit_pending = false;
        return false;
    }
    if (heat_display_warm_exit_pending) {
        return false;
    }
    heat_display_warm_exit_pending = true;
    return true;
}

/** 解析 MCU 加热模式 DP02（本包未带时用缓存） */
static u8 heat_display_mcu_mode_resolve(bool got_mode, u8 mode_val)
{
    if (got_mode) {
        heat_display_cached_mcu_mode = mode_val;
        heat_display_has_cached_mcu_mode = true;
        return mode_val;
    }
    if (heat_display_has_cached_mcu_mode) {
        return heat_display_cached_mcu_mode;
    }
    return lunchbox_get_heat_mode();
}

static bool heat_display_mcu_mode_is_heating(u8 mode)
{
    return mode >= 1 && mode <= 4;
}

static bool heat_display_mcu_mode_is_warm(u8 mode)
{
    return mode == 5;
}

static bool heat_display_mcu_mode_is_off(u8 mode)
{
    return mode == 0;
}

/** 直接加热 / 预约到点：MCU 加热任务是否活跃 */
static bool heat_display_heat_task_active(bool got_enable, bool heating)
{
#if FUNC_LUNCHBOX_UART_EN
    if (lunchbox_heating_task_active()) {
        return true;
    }
#endif
    if (got_enable && heating) {
        return true;
    }
    if (heat_display_heating_active()) {
        return true;
    }
    if (func_cb.sta == FUNC_HEAT && func_heat_ui_is_heating()) {
        return true;
    }
    return false;
}

/** 充电进保温：须在加热页且加热任务活跃（预约与直接加热相同） */
static bool heat_display_charging_warm_should_enter(bool got_enable, bool heating)
{
    if (func_cb.sta != FUNC_HEAT) {
        return false;
    }
    return heat_display_heat_task_active(got_enable, heating);
}

/** DP02=5 + 充电 → 保温；亮屏立即切页，熄屏记 pending */
static bool heat_display_try_charging_warm_route(bool got_mode, u8 mcu_mode,
                                                 bool got_charge, u8 charge_val,
                                                 bool got_enable, bool heating)
{
    u8 mode = heat_display_mcu_mode_resolve(got_mode, mcu_mode);

    if (!heat_display_mcu_mode_is_warm(mode)
        || !heat_display_charging_now(got_charge, charge_val)) {
        return false;
    }
    if (func_cb.sta == FUNC_NEW_WARM) {
        return true;
    }
    if (func_cb.sta == FUNC_LID_CONFIRM || elunchbox_lid_confirm_snap_valid()) {
        return true;
    }
    if (elunchbox_charge_off_active()) {
        return true;
    }
    /* 预约到点/Home等：亮屏直接进保温（预约加热+充电先保温，拔电后回加热），熄屏记 pending */
    if (func_cb.sta != FUNC_HEAT) {
        if (heat_display_heat_task_active(got_enable, heating)) {
            heat_display_heat_pending = false;
            if (heat_display_ui_ok()) {
                printf("[LCD_ROUTE] MCU mode=5 charge -> warm (sta=%u)\n", func_cb.sta);
                func_elunchbox_enter_warm_from_charging();
                /* 充电转保温时记录剩余时间: 若 remain 已为 0，标记拔电后应回主页 */
                if (heat_display_has_last && heat_display_last.remain_min == 0) {
                    heat_display_warm_charge_remain_zero = true;
                    printf("[LCD_ROUTE] warm from charging with remain=0, flag set\n");
                }
            } else {
                heat_display_warm_charge_pending = true;
                printf("[LCD_ROUTE] MCU mode=5 charge -> warm pending (await sta=%u)\n",
                       func_cb.sta);
            }
            return true;
        }
        return false;
    }
    if (!heat_display_charging_warm_should_enter(got_enable, heating)) {
        if (heat_display_heat_task_active(got_enable, heating)) {
            heat_display_warm_charge_pending = true;
        }
        return heat_display_warm_charge_pending;
    }
    if (!func_heat_panel_ready_for_charge_warm()) {
        heat_display_warm_charge_pending = true;
        printf("[LCD_ROUTE] MCU mode=5 charge -> warm pending (heat not ready sta=%u)\n",
               func_cb.sta);
        return true;
    }
    heat_display_heat_pending = false;
    if (heat_display_ui_ok()) {
        printf("[LCD_ROUTE] MCU mode=5 charge -> warm (sta=%u)\n", func_cb.sta);
        func_elunchbox_enter_warm_from_charging();
        /* 充电转保温时记录剩余时间: 若 remain 已为 0，标记拔电后应回主页 */
        if (heat_display_has_last && heat_display_last.remain_min == 0) {
            heat_display_warm_charge_remain_zero = true;
            printf("[LCD_ROUTE] warm from charging with remain=0, flag set\n");
        }
    } else {
        heat_display_warm_charge_pending = true;
        printf("[LCD_ROUTE] MCU mode=5 charge -> warm pending (guioff sta=%u)\n",
               func_cb.sta);
    }
    return true;
}

/** MCU 驱动从保温回加热：用 UART 快照预设参数，避免 func_heat 默认 176°F */
static void heat_display_preset_resume_heat(u32 remain_min, bool got_remain,
                                            u16 temp_f, bool got_temp,
                                            u32 duration_min, bool got_duration,
                                            u8 proto_mode)
{
    heat_display_info_t last;
    u16 use_temp;
    u32 use_total;
    u32 use_remain;
    bool has_last;

    has_last = heat_display_get_last(&last);
    use_temp = got_temp ? temp_f : (has_last ? last.temp_f : 176);
#if ELUNCHBOX_PANEL_EN
    if (!got_temp && !has_last && g_res.setup_done) {
        use_temp = lunchbox_temp_idx_to_f(g_res.temp_idx);
    }
#endif

    if (got_duration && duration_min > 0) {
        use_total = duration_min;
#if ELUNCHBOX_PANEL_EN
    } else if (g_res.setup_done) {
        use_total = (u32)g_res.heat_hour * 60 + (u32)g_res.heat_min;
        if (use_total < LB_HEAT_DURATION_MIN_MIN) {
            use_total = LB_HEAT_DURATION_MIN_MIN;
        } else if (use_total > LB_HEAT_DURATION_MAX_MIN) {
            use_total = LB_HEAT_DURATION_MAX_MIN;
        }
#endif
    } else if (has_last && last.remain_min > 0) {
        use_total = last.remain_min;
    } else {
        use_total = 60;
    }

    use_remain = got_remain ? remain_min : (has_last ? last.remain_min : use_total);

    lb_mode_to_heat_set(proto_mode, use_temp,
                        (u8)(use_total / 60), (u8)(use_total % 60));
    heat_display_show(use_remain, use_temp);
    printf("[LCD_ROUTE] resume heat preset mode=%u %uF total=%umin remain=%umin\n",
           proto_mode, use_temp, use_total, use_remain);
}

/** 本包无 DP10 时，用 UART 属性判断 MCU 是否仍在加热 */
static bool heat_display_mcu_still_heating(bool got_enable, bool heating)
{
    if (got_enable) {
        return heating;
    }
#if FUNC_LUNCHBOX_UART_EN
    return lunchbox_heating_task_active();
#else
    return false;
#endif
}

/** 拔电回加热页时使用的协议模式（预约须为 4） */
static u8 heat_display_resume_proto_mode(u8 mode)
{
    if (heat_display_mcu_mode_is_heating(mode)) {
        return mode;
    }
#if ELUNCHBOX_PANEL_EN
    if (g_res.setup_done && func_reservation_is_heating()) {
        return 4;
    }
#endif
    return 1;
}

/** 充电进保温后拔电：是否应回加热页 */
static bool heat_display_unplug_should_resume_heat(u8 mode, bool got_enable, bool heating)
{
    if (heat_display_mcu_mode_is_off(mode)) {
        return false;
    }
    if (got_enable && !heating) {
        return false;
    }
    if (!heat_display_mcu_still_heating(got_enable, heating)) {
        return false;
    }
    if (heat_display_mcu_mode_is_heating(mode)) {
        return true;
    }
    /* mode=5(保温) 仅在充电转保温后拔电时才应回加热页；
     * 加热自然结束进保温时 MCU 也会回复 mode=5，此乃保温正常状态，勿跳回加热 */
    if (heat_display_mcu_mode_is_warm(mode) && func_elunchbox_warm_from_charging()) {
        return true;
    }
#if ELUNCHBOX_PANEL_EN
    /* 预约加热中：Mode=1~4 表示 MCU 仍在加热，应从保温回加热；
     * 但 Mode=5(保温) 表示加热已结束，不应回加热（否则保温→加热→保温死循环） */
    if (g_res.setup_done && func_reservation_is_heating()
        && !heat_display_mcu_mode_is_warm(mode)) {
        return true;
    }
#endif
    return false;
}

static void heat_display_route_resume_heat(u8 mode,
                                           u32 remain_min, bool got_remain,
                                           u16 temp_f, bool got_temp,
                                           u32 duration_min, bool got_duration)
{
    u8 proto_mode = heat_display_resume_proto_mode(mode);

    heat_display_preset_resume_heat(remain_min, got_remain, temp_f, got_temp,
                                    duration_min, got_duration, proto_mode);
    func_elunchbox_warm_from_charging_set(false);
    heat_display_warm_charge_remain_zero = false;
    lb_heat_mcu_nav_set(true);
    lb_heat_uart_remote_set(true);
    lb_heat_autostart_set(true);
    printf("[LCD_ROUTE] resume heat proto=%u (mcu_mode=%u res=%u)\n",
           proto_mode, mode,
#if ELUNCHBOX_PANEL_EN
           func_reservation_is_heating() ? 1u : 0u
#else
           0u
#endif
           );
    func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

/** 开始加热后：按 MCU DP02 模式 + 充电状态路由界面 */
static bool heat_display_mcu_mode_route(bool got_mode, u8 mcu_mode,
                                        u32 remain_min, bool got_remain,
                                        u16 temp_f, bool got_temp,
                                        u32 duration_min, bool got_duration,
                                        bool got_enable, bool heating,
                                        bool got_charge, u8 charge_val)
{
    u8 mode = heat_display_mcu_mode_resolve(got_mode, mcu_mode);
    bool charging = heat_display_charging_now(got_charge, charge_val);

    /* DP02=5 + 充电：加热页 → 保温 */
    if (heat_display_try_charging_warm_route(got_mode, mcu_mode, got_charge, charge_val,
                                             got_enable, heating)) {
        return true;
    }

    /* 加热自然结束：DP02=5 且非充电 */
    if (heat_display_mcu_mode_is_warm(mode) && !charging) {
        if (func_cb.sta == FUNC_HEAT && func_heat_uart_finish_ok()) {
            printf("[LCD_ROUTE] MCU mode=5 -> warm (sta=FUNC_HEAT live_ok=1)\n");
            func_elunchbox_enter_warm_from_heat();
            return true;
        }
        printf("[LCD_ROUTE] MCU mode=5 warm skipped: "
               "sta=%u is_heat=%d live_ok=%d charging=%d\n",
               func_cb.sta,
               (func_cb.sta == FUNC_HEAT) ? 1 : 0,
               func_heat_uart_finish_ok() ? 1 : 0,
               charging ? 1 : 0);
    }

    /* 充电保温中拔电：
     * - 加热进行中(remain>0) → 回加热页
     * - 加热已结束 / 已下发 24h 保温 → 停加热回主页
     * 仅处理因充电进入保温的场景；加热自然结束进保温不受拔电影响。 */
    if (!charging && func_cb.sta == FUNC_NEW_WARM
        && func_elunchbox_warm_from_charging()) {
        /* 先保存标志：warm_exit_reset 会清掉 charge_keep_warm_sent */
        bool keep_warm_sent = heat_display_charge_keep_warm_sent;
        bool remain_is_zero;

        if (keep_warm_sent) {
            /* 已强制下发 24h 保温：加热已结束，拔电停热回主页（勿因 remain>0 回加热） */
            remain_is_zero = true;
        } else if (got_remain && remain_min > 0) {
            /* MCU 明确上报 remain>0: 加热仍在进行，忽略任何遗留的 remain=0 标志 */
            remain_is_zero = false;
        } else {
            remain_is_zero = (got_remain && remain_min == 0)
                           || heat_display_warm_charge_remain_zero;
        }
        if (got_charge && charge_val == 0) {
            heat_display_warm_exit_reset();
        }
        /* 拔电后加热已结束 / 已发 24h 保温 → 停加热回主界面 */
        if (remain_is_zero) {
            if (!heat_display_warm_exit_try()) {
                return true;
            }
            printf("[LCD_ROUTE] unplug warm->home (sta=%u keep_warm=%d remain0_flag=%d)\n",
                   func_cb.sta, keep_warm_sent ? 1 : 0,
                   heat_display_warm_charge_remain_zero ? 1 : 0);
            if (keep_warm_sent) {
                /* 本机已启动 24h 保温，须下发停热给 MCU */
                lunchbox_keep_warm_stop_user();
            }
            func_elunchbox_uart_stop_and_home();
            return true;
        }
        /* 拔电后 remain>0 → 加热还在进行，回加热页 */
        if (heat_display_unplug_should_resume_heat(mode, got_enable, heating)) {
            printf("[LCD_ROUTE] unplug remain>0 warm->heat "
                   "mode=%u remain=%u (sta=%u)\n",
                   mode, got_remain ? remain_min : 0xffffffff, func_cb.sta);
            heat_display_route_resume_heat(mode, remain_min, got_remain, temp_f, got_temp,
                                           duration_min, got_duration);
            return true;
        }
        return false;
    }

    return false;
}
#endif

static void heat_display_notify(void)
{
    if (!heat_display_ui_ok()) {
        return;
    }
    if (heat_display_cb != NULL) {
        heat_display_cb(&heat_display_last);
    }
}

void heat_display_register(heat_display_cb_t cb)
{
    printf("[LCD_REG] register cb=%p\n", cb);
    heat_display_cb = cb;
}

void heat_display_unregister(void)
{
    printf("[LCD_REG] unregister cb=%p\n", heat_display_cb);
    heat_display_cb = NULL;
}

bool heat_display_get_last(heat_display_info_t *out)
{
    if (out == NULL || !heat_display_has_last) {
        return false;
    }
    *out = heat_display_last;
    return true;
}

bool heat_display_heating_active(void)
{
    return heat_display_has_last && heat_display_last.remain_min > 0;
}

void heat_display_show_schedule(u32 schedule_min)
{
    if (schedule_min > 5999) {
        schedule_min = 5999;
    }

    if (heat_display_has_last && heat_display_last.schedule_min == schedule_min) {
        return;
    }

    heat_display_last.schedule_min = schedule_min;
    heat_display_has_last = true;
    heat_display_notify();
}

void heat_display_show(u32 remain_min, u16 temp_f)
{
    bool changed;

    if (remain_min > 5999) {
        remain_min = 5999;
    }
    if (temp_f > 999) {
        temp_f = 999;
    }

    /* 剩余时间以 MCU 为准：仅温度相同时也须刷新（避免 live 未就绪时显示旧 total） */
    changed = (!heat_display_has_last
               || heat_display_last.remain_min != remain_min
               || heat_display_last.temp_f != temp_f);
    if (!changed) {
        return;
    }

    heat_display_last.remain_min = remain_min;
    heat_display_last.temp_f = temp_f;
    heat_display_has_last = true;
    heat_display_notify();
}

void heat_display_show_all(u32 schedule_min, u32 remain_min, u16 temp_f)
{
    bool changed;

    if (schedule_min > 5999) {
        schedule_min = 5999;
    }
    if (remain_min > 5999) {
        remain_min = 5999;
    }
    if (temp_f > 999) {
        temp_f = 999;
    }

    changed = (!heat_display_has_last
               || heat_display_last.schedule_min != schedule_min
               || heat_display_last.remain_min != remain_min
               || heat_display_last.temp_f != temp_f);
    if (!changed) {
        return;
    }

    heat_display_last.schedule_min = schedule_min;
    heat_display_last.remain_min = remain_min;
    heat_display_last.temp_f = temp_f;
    heat_display_has_last = true;
    heat_display_notify();
}

/** @brief 温度档位 → 华氏度 (协议: 0=40°C ~ 6=100°C) */
static u16 heat_display_temp_idx_to_f(u8 idx)
{
    u16 temp_c;

    if (idx > 6) {
        idx = 6;
    }
    temp_c = 40 + (u16)idx * 10;
    return (u16)(temp_c * 9 / 5 + 32);
}

/** 将 DP06/DP07 写入 heat_display_last；跳页前须缓存，供 func_heat_sync_mcu_snapshot 读取 */
static void heat_display_feed_apply(u32 remain_min, bool got_remain,
                                    u16 temp_f, bool got_temp)
{
    heat_display_info_t last;
    u16 use_temp;

    if (got_remain && got_temp) {
        heat_display_show(remain_min, temp_f);
        return;
    }
    if (got_remain) {
        if (heat_display_get_last(&last)) {
            use_temp = last.temp_f;
        }
#if ELUNCHBOX_PANEL_EN
        else if (g_res.setup_done) {
            use_temp = lunchbox_temp_idx_to_f(g_res.temp_idx);
        }
#endif
        else {
            use_temp = 176;
        }
        heat_display_show(remain_min, use_temp);
        return;
    }
    if (got_temp && heat_display_get_last(&last)) {
        heat_display_show(last.remain_min, temp_f);
    }
}


bool heat_display_charge_wake_pending(void)
{
    bool pending = heat_display_charge_pending;
    heat_display_charge_pending = false;
    return pending;
}

bool heat_display_heat_wake_pending(void)
{
    bool pending = heat_display_heat_pending;
    heat_display_heat_pending = false;
    return pending;
}

#if ELUNCHBOX_PANEL_EN
/** 熄屏时收到充电+保温 DP，唤醒后应进保温页而非加热页 */
bool heat_display_warm_charge_wake_pending(void)
{
    bool pending = heat_display_warm_charge_pending;
    heat_display_warm_charge_pending = false;
    return pending;
}

bool heat_display_warm_charge_pending_active(void)
{
    return heat_display_warm_charge_pending;
}

void heat_display_warm_charge_route_poll(void)
{
    if (!elunchbox_ui_is_live()) {
        return;
    }
    if (func_cb.sta != FUNC_HEAT) {
        return;
    }
    if (!heat_display_warm_charge_pending
        && !(heat_display_has_cached_mcu_mode
             && heat_display_mcu_mode_is_warm(heat_display_cached_mcu_mode)
             && heat_display_charging_now(false, 0)
             && heat_display_heat_task_active(false, false))) {
        return;
    }
    if (!func_heat_panel_ready_for_charge_warm()) {
        return;
    }
    if (heat_display_warm_charge_pending) {
        heat_display_warm_charge_pending = false;
        heat_display_heat_pending = false;
        printf("[LCD_ROUTE] warm_charge pending -> enter warm (sta=%u)\n", func_cb.sta);
        func_elunchbox_enter_warm_from_charging();
        /* 充电转保温(熄屏唤醒)时记录剩余时间: 若 remain 已为 0，标记拔电后应回主页 */
        if (heat_display_has_last && heat_display_last.remain_min == 0) {
            heat_display_warm_charge_remain_zero = true;
            printf("[LCD_ROUTE] warm from charging (deferred) with remain=0, flag set\n");
        }
        return;
    }
    /* 预约进加热页后：用缓存 MCU 模式补触发（与直接加热插电进保温相同） */
    (void)heat_display_try_charging_warm_route(false, 0, false, 0, false, false);
}

bool heat_display_reservation_can_switch_heat(void)
{
    if (func_cb.sta == FUNC_NEW_WARM || func_cb.sta == FUNC_HEAT) {
        return false;
    }
    if (func_elunchbox_warm_from_charging()) {
        return false;
    }
    return true;
}

u8 heat_display_get_mcu_mode(void)
{
    if (heat_display_has_cached_mcu_mode) {
        return heat_display_cached_mcu_mode;
    }
    return lunchbox_get_heat_mode();
}
#endif
