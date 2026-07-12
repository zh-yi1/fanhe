#include "include.h"
#include "heat_display_reg.h"
#include "func_lunchbox_uart.h"
#include "func.h"
#include "home_ui_shared.h"
#if ELUNCHBOX_PANEL_EN
#include "func_reservation.h"
#endif

static heat_display_cb_t heat_display_cb;
static heat_display_info_t heat_display_last;
static bool heat_display_has_last;
static bool heat_display_charge_pending;  /* 充电中状态唤醒标志 */
static bool heat_display_heat_pending;    /* 加热使能唤醒标志 */

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
}

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

/** 充电中 MCU 上报 DP02=5 时是否应切保温（须已有加热任务，空闲 Home 插电不跳） */
static bool heat_display_charging_warm_should_enter(bool got_enable, bool heating)
{
    if (func_cb.sta == FUNC_HEAT && func_heat_ui_is_heating()) {
        return true;
    }
    if (got_enable && heating) {
        return true;
    }
#if FUNC_LUNCHBOX_UART_EN
    /* 先充电后点加热：本地已标记加热任务但 UART 未下发 */
    if (func_cb.sta == FUNC_HEAT && lunchbox_heating_task_active()) {
        return true;
    }
#endif
    return false;
}

/** MCU 驱动从保温回加热：用 UART 快照预设参数，避免 func_heat 默认 176°F */
static void heat_display_preset_resume_heat(u32 remain_min, bool got_remain,
                                            u16 temp_f, bool got_temp,
                                            u32 duration_min, bool got_duration)
{
    heat_display_info_t last;
    u16 use_temp;
    u32 use_total;
    u32 use_remain;
    bool has_last;

    has_last = heat_display_get_last(&last);
    use_temp = got_temp ? temp_f : (has_last ? last.temp_f : 176);

    if (got_duration && duration_min > 0) {
        use_total = duration_min;
    } else if (got_remain && remain_min > 0) {
        use_total = remain_min;
    } else if (has_last && last.remain_min > 0) {
        use_total = last.remain_min;
    } else {
        use_total = 60;
    }

    use_remain = got_remain ? remain_min : (has_last ? last.remain_min : use_total);

    lb_mode_to_heat_set(1, use_temp,
                        (u8)(use_total / 60), (u8)(use_total % 60));
    heat_display_show(use_remain, use_temp);
    printf("[LCD_ROUTE] resume heat preset %uF total=%umin remain=%umin\n",
           use_temp, use_total, use_remain);
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

/**
 * @brief 从 DataPoint 数组提取剩余时间/温度，推送给加热页显示
 *
 * 解析协议 DataPoint 格式（dpid:1B + type:1B + len:2B-BE + val:lenB），
 * 提取 LB_DPID_REMAIN_TIME(6) / LB_DPID_HEAT_TEMP(7) / LB_DPID_HEAT_ENABLE(10)，
 * 调用 heat_display_show() 通知 LCD 刷新。
 *
 * 调用位置:
 *   - UART: lb_frame_parse() 收到 LB_UART_CMD_DYNAMIC (0x01)
 *   - BLE:  lunchbox_ble_rx_handle() 收到控制/状态相关 DataPoints
 */
void heat_display_feed_dp(u8 *data, u16 len)
{
#if ELUNCHBOX_PANEL_EN
    bool ui_ok = heat_display_ui_ok();
#endif
    u16 off = 0;
    u32 remain_min = 0;
    u16 temp_f = 0;
    bool got_remain = false;
    bool got_temp = false;
    bool got_enable = false;
    bool got_charge = false;
    bool got_warm_mode = false;
    bool got_heat_stop = false;
    bool got_duration = false;
    bool heating = false;
    u8 charge_val = 0;
    u32 duration_min = 0;

    while (off + 4 <= len) {
        u8  dpid    = data[off];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];

        if (off + 4 + val_len > len) {
            break;
        }
        u8 *val = data + off + 4;

        switch (dpid) {
        case LB_DPID_REMAIN_TIME:
            if (val_len >= 4) {
                remain_min = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                         | ((u32)val[2] << 8) | val[3];
                got_remain = true;
#if ELUNCHBOX_PANEL_EN
                if (ui_ok)
#endif
                printf("[LCD_REG] feed_dp: REMAIN_TIME=%u min\n", remain_min);
            }
            break;
        case LB_DPID_HEAT_TEMP:
            if (val_len >= 1) {
                temp_f = heat_display_temp_idx_to_f(val[0]);
                got_temp = true;
#if ELUNCHBOX_PANEL_EN
                if (ui_ok)
#endif
                printf("[LCD_REG] feed_dp: HEAT_TEMP idx=%u -> %u F\n", val[0], temp_f);
            }
            break;
        case LB_DPID_HEAT_DURATION:
            if (val_len >= 4) {
                duration_min = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                             | ((u32)val[2] << 8) | val[3];
                got_duration = true;
#if ELUNCHBOX_PANEL_EN
                if (ui_ok)
#endif
                printf("[LCD_REG] feed_dp: HEAT_DUR=%u min\n", duration_min);
            }
            break;
        case LB_DPID_HEAT_ENABLE:
            if (val_len >= 1) {
                heating = (val[0] != 0);
                got_enable = true;
#if ELUNCHBOX_PANEL_EN
                if (ui_ok)
#endif
                printf("[LCD_REG] feed_dp: HEAT_ENABLE=%u (heating=%d)\n", val[0], heating);
            }
            break;
        case LB_DPID_HEAT_MODE:
            if (val_len >= 1) {
                if (val[0] == 5) {
                    got_warm_mode = true;
#if ELUNCHBOX_PANEL_EN
                    if (ui_ok)
#endif
                    printf("[LCD_REG] feed_dp: HEAT_MODE=5 (warm)\n");
                } else if (val[0] == 0) {
                    got_heat_stop = true;
#if ELUNCHBOX_PANEL_EN
                    if (ui_ok)
#endif
                    printf("[LCD_REG] feed_dp: HEAT_MODE=0 (off)\n");
                }
            }
            break;
        case LB_DPID_CHARGE_STATUS:
            if (val_len >= 1) {
                charge_val = val[0];
                got_charge = true;
#if ELUNCHBOX_PANEL_EN
                if (ui_ok)
#endif
                printf("[LCD_REG] feed_dp: CHARGE_STATUS=%u\n", charge_val);
            }
            break;
        default:
            break;
        }
        off += 4 + val_len;
    }

    /* 加热模块主动上报 HEAT_ENABLE=1：用于熄屏唤醒和预约自动跳转 */
    if (got_enable && heating) {
        heat_display_heat_pending = true;
    }

#if ELUNCHBOX_PANEL_EN
    if (got_charge) {
        home_ui_shared_battery_charge_apply(charge_val);
        if (ui_ok) {
            home_ui_shared_battery_icon_refresh();
        }
        if (charge_val == 1) {
            heat_display_charge_pending = true;
        }
    }
#endif

    /* 预约加热已由加热模块自动启动 → 预设加热参数（必须在 !ui_ok 提前返回之前）
     * 关屏时 func_heat_enter() 需要 lb_heat_autostart 标志才能进入加热状态，
     * 否则会因为没有 autostart 而跳转到设置页，而不是正在加热界面 */
    if (got_enable && heating && g_res.setup_done
        && func_cb.sta != FUNC_NEW_HEAT && func_cb.sta != FUNC_HEAT) {
        u16 temp_f = lunchbox_temp_idx_to_f(g_res.temp_idx);
        u32 duration_min = (u32)g_res.heat_hour * 60 + (u32)g_res.heat_min;
        if (duration_min < LB_HEAT_DURATION_MIN_MIN) {
            duration_min = LB_HEAT_DURATION_MIN_MIN;
        } else if (duration_min > LB_HEAT_DURATION_MAX_MIN) {
            duration_min = LB_HEAT_DURATION_MAX_MIN;
        }
        lb_mode_to_heat_set(4, temp_f,
                            (u8)(duration_min / 60), (u8)(duration_min % 60));
        lb_heat_autostart_set(true);
        /* 加热模块已自发启动预约加热 → 立即切换预约阶段为加热中，关预约灯 LED4 */
        func_reservation_phase_enter_heating();
        printf("[LCD_REG] feed_dp: reservation heating, preset heat params "
               "temp=%uF dur=%umin ui_ok=%d sta=%u\n",
               temp_f, duration_min, ui_ok ? 1 : 0, func_cb.sta);
    }

    /* 手动关机/息屏：仅更新缓存与充电唤醒标志，不触发 UI 回调
     * 注意：加热参数预设 (lb_mode_to_heat_set/lb_heat_autostart_set) 已在上方完成，
     *       关屏唤醒后由 func_process() 中的 heat_display_heat_wake_pending 检查
     *       调用 func_elunchbox_switch_to_heat_panel() 完成页面跳转 */
#if ELUNCHBOX_PANEL_EN
    if (!ui_ok) {
        if (got_enable && !heating && heat_display_has_last) {
            heat_display_last.remain_min = 0;
        }
        if (got_charge && charge_val == 1) {
            heat_display_charge_pending = true;   //唤醒
        }
        return;
    }

    /* ── 充电保温路由（仅 UART 驱动，显示端充电时不发指令）────────────────
     * 1) 加热中插电 / 先充后加热：保持加热 UI，等 MCU 发 DP02=5 → 保温
     * 2) 保温中拔电：DP10=0 或 DP02=0 → Home；DP10=1 且非保温模式 → 加热
     * 3) 空闲 Home 仅 DP04：只刷新充电图标，不因 DP02=5 跳保温
     * ─────────────────────────────────────────────────────────────────── */

    /* 预约加热已由加热模块自动启动 → 亮屏时直接跳转加热界面
     * (加热参数已在 !ui_ok 之前预设，此处只需要触发页面跳转) */
    if (got_enable && heating && g_res.setup_done
        && func_cb.sta != FUNC_NEW_HEAT && func_cb.sta != FUNC_HEAT) {
        printf("[LCD_REG] feed_dp: reservation heating started by module, switch to heat panel\n");
        func_elunchbox_switch_to_heat_panel();
        return;
    }

    /* 充电中：MCU 上报 DP02=5 且存在加热任务 → 保温页 */
    if (got_warm_mode && heat_display_charging_now(got_charge, charge_val)) {
        if (func_cb.sta != FUNC_NEW_WARM
            && heat_display_charging_warm_should_enter(got_enable, heating)) {
            printf("[LCD_ROUTE] MCU DP02=5 charge+heat -> warm (sta=%u en=%u)\n",
                   func_cb.sta, heating);
            func_elunchbox_enter_warm_from_charging();
            return;
        }
        if (func_cb.sta == FUNC_NEW_WARM) {
            return;
        }
    }

    if (got_warm_mode && func_heat_uart_finish_ok()) {
        func_elunchbox_enter_warm_from_heat();
        return;
    }

    /* 因充电进保温：拔电后收到退出指令 → Home */
    if (func_cb.sta == FUNC_NEW_WARM
        && func_elunchbox_warm_from_charging()
        && !heat_display_charging_now(got_charge, charge_val)) {
        if (got_charge && charge_val == 0) {
            heat_display_warm_exit_reset();
        }
        if ((got_enable && !heating) || got_heat_stop) {
            if (!heat_display_warm_exit_try()) {
                return;
            }
            printf("[LCD_ROUTE] MCU unplug warm->home (sta=%u heating=%u stop=%u)\n",
                   func_cb.sta, heating, got_heat_stop);
            func_elunchbox_uart_stop_and_home();
            return;
        }
    }

    /* 保温页拔电后退出（自然结束保温场景） */
    if (func_cb.sta == FUNC_NEW_WARM
        && !func_elunchbox_warm_from_charging()
        && !heat_display_charging_now(got_charge, charge_val)) {
        if ((got_enable && !heating) || got_heat_stop) {
            if (!heat_display_warm_exit_try()) {
                return;
            }
            printf("[LCD_ROUTE] MCU warm exit -> home (sta=%u)\n", func_cb.sta);
            func_elunchbox_uart_stop_and_home();
            return;
        }
    }

    /* 加热已停止：清零 remain，让 heat_display_heating_active() 返回 false，避免阻止息屏/误唤醒 */
    if (got_enable && !heating) {
        printf("[LCD_REG] feed_dp: heating stopped, clear remain\n");
        if (func_cb.sta == FUNC_NEW_WARM && heat_display_charging_now(got_charge, charge_val)) {
            return;
        }
        if (func_heat_uart_finish_ok()) {
            func_elunchbox_enter_warm_from_heat();
            return;
        }
        if (heat_display_has_last && heat_display_last.remain_min > 0) {
            heat_display_last.remain_min = 0;
            heat_display_notify();
        }
        return;
    }

    /* 保温页 + 充电中：HeatEn=ON 为保温运行，勿跳回加热/设置页 */
    if (func_cb.sta == FUNC_NEW_WARM && heat_display_charging_now(got_charge, charge_val)) {
        return;
    }

    /* 因充电进保温：拔电后 MCU 下发继续加热 → 回加热界面（仅 UI，不回发 heat_start） */
    if (got_enable && heating && func_cb.sta == FUNC_NEW_WARM
        && func_elunchbox_warm_from_charging()
        && !heat_display_charging_now(got_charge, charge_val) && !got_warm_mode) {
        printf("[LCD_ROUTE] MCU unplug resume heat -> FUNC_HEAT (sta=%u DP10=1)\n", func_cb.sta);
        heat_display_preset_resume_heat(remain_min, got_remain, temp_f, got_temp,
                                      duration_min, got_duration);
        func_elunchbox_warm_from_charging_set(false);
        lb_heat_mcu_nav_set(false);
        lb_heat_uart_remote_set(true);
        lb_heat_autostart_set(true);
        func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        return;
    }

    /* 充电结束 + 加热指令：从保温页回到加热界面（仅非“充电转保温”场景） */
    if (got_enable && heating && func_cb.sta == FUNC_NEW_WARM
        && !heat_display_charging_now(got_charge, charge_val) && !got_warm_mode
        && !func_elunchbox_warm_from_charging()) {
        printf("[LCD_ROUTE] MCU charge end resume heat -> FUNC_HEAT (sta=%u)\n", func_cb.sta);
        heat_display_preset_resume_heat(remain_min, got_remain, temp_f, got_temp,
                                      duration_min, got_duration);
        func_elunchbox_warm_from_charging_set(false);
        lb_heat_mcu_nav_set(false);
        lb_heat_uart_remote_set(true);
        lb_heat_autostart_set(true);
        func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        return;
    }
#endif

    if (got_remain && got_temp) {
        heat_display_show(remain_min, temp_f);
#if ELUNCHBOX_PANEL_EN
        if (remain_min == 0 && func_heat_uart_finish_ok()) {
            func_elunchbox_enter_warm_from_heat();
            return;
        }
#endif
    } else if (got_remain) {
        heat_display_info_t last;

        if (heat_display_get_last(&last)) {
            heat_display_show(remain_min, last.temp_f);
        }
#if ELUNCHBOX_PANEL_EN
        if (remain_min == 0 && func_heat_uart_finish_ok()) {
            func_elunchbox_enter_warm_from_heat();
            return;
        }
#endif
    } else if (got_temp) {
        heat_display_info_t last;

        if (heat_display_get_last(&last)) {
            heat_display_show(last.remain_min, temp_f);
        }
    }

#if ELUNCHBOX_PANEL_EN
    if (got_charge && ui_ok) {
        heat_display_notify();
    }
#endif
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
