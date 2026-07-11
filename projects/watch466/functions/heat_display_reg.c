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
    bool heating = false;
    u8 charge_val = 0;

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
    }
#endif

    /* 手动关机/息屏：仅更新缓存与充电唤醒标志，不触发 UI 回调 */
#if ELUNCHBOX_PANEL_EN
    if (!ui_ok) {
        if (got_enable && !heating && heat_display_has_last) {
            heat_display_last.remain_min = 0;
        }
        if (got_charge && charge_val == 1) {
            heat_display_charge_pending = true;
        }
        return;
    }

    /* 充电中且正在加热：立即刷新充电图标并跳转保温页 */
    if (func_heat_ui_is_heating()
        && heat_display_charging_now(got_charge, charge_val)) {
        home_ui_shared_battery_icon_refresh();
        if (func_elunchbox_charging_redirect_warm()) {
            return;
        }
    }
#endif

#if ELUNCHBOX_PANEL_EN
    /* 预约加热已由加热模块自动启动 → 跳转加热界面
     * UART 回调可能在 func_reservation_poll() 之前触发，必须在此处预设加热参数
     * (autostart + mode_to_heat)，避免 func_heat 进入后因没有 autostart 而跳转到设置页 */
    if (got_enable && heating && g_res.setup_done
        && func_cb.sta != FUNC_NEW_HEAT && func_cb.sta != FUNC_HEAT) {
        printf("[LCD_REG] feed_dp: reservation heating started by module, switch to heat panel\n");
        {
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
        }
        func_elunchbox_switch_to_heat_panel();
        return;
    }

    /* 串口保温指令：加热中+充电 → 立即切保温（勿等 heat_live_ready） */
    if (got_warm_mode && func_heat_ui_is_heating()
        && heat_display_charging_now(got_charge, charge_val)) {
        printf("[LCD_REG] feed_dp: warm cmd while charging+heating\n");
        func_elunchbox_enter_warm_from_charging();
        return;
    }

    if (got_warm_mode && func_heat_uart_finish_ok()) {
        func_elunchbox_enter_warm_from_heat();
        return;
    }

    /* 因充电进保温：拔电后收到退出指令 → Home */
    if (func_cb.sta == FUNC_NEW_WARM
        && func_elunchbox_warm_from_charging()
        && !heat_display_charging_now(got_charge, charge_val)) {
        if ((got_enable && !heating) || got_heat_stop) {
            if (!heat_display_warm_exit_try()) {
                return;
            }
            printf("[LCD_REG] feed_dp: charging-warm exit -> home\n");
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
            printf("[LCD_REG] feed_dp: warm page exit after charge\n");
            func_elunchbox_uart_stop_and_home();
            return;
        }
    }
#endif

    /* 加热已停止：清零 remain，让 heat_display_heating_active() 返回 false，避免阻止息屏/误唤醒 */
    if (got_enable && !heating) {
        printf("[LCD_REG] feed_dp: heating stopped, clear remain\n");
#if ELUNCHBOX_PANEL_EN
        if (func_cb.sta == FUNC_NEW_WARM && heat_display_charging_now(got_charge, charge_val)) {
            return;
        }
        if (func_cb.sta == FUNC_HEAT && heat_display_charging_now(got_charge, charge_val)
            && func_heat_ui_is_heating()) {
            func_elunchbox_charging_redirect_warm();
            return;
        }
        if (func_heat_uart_finish_ok()) {
            func_elunchbox_enter_warm_from_heat();
            return;
        }
#endif
        if (heat_display_has_last && heat_display_last.remain_min > 0) {
            heat_display_last.remain_min = 0;
            heat_display_notify();
        }
        return;
    }

#if ELUNCHBOX_PANEL_EN
    /* 保温页 + 充电中：模块 HeatEn=ON 为保温运行，勿跳回加热/设置页 */
    if (func_cb.sta == FUNC_NEW_WARM && heat_display_charging_now(got_charge, charge_val)) {
        if (got_charge) {
            home_ui_shared_battery_icon_refresh();
        }
        return;
    }

    /* 充电结束 + 加热指令：从保温页回到加热界面（仅非“充电转保温”场景） */
    if (got_enable && heating && func_cb.sta == FUNC_NEW_WARM
        && !heat_display_charging_now(got_charge, charge_val) && !got_warm_mode
        && !func_elunchbox_warm_from_charging()) {
        printf("[LCD_REG] feed_dp: charge ended, resume heat panel\n");
        func_elunchbox_warm_from_charging_set(false);
        lb_heat_autostart_set(true);
        func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        return;
    }

    /* 因充电进保温：拔电后串口下发继续加热 → 回加热界面 */
    if (got_enable && heating && func_cb.sta == FUNC_NEW_WARM
        && func_elunchbox_warm_from_charging()
        && !heat_display_charging_now(got_charge, charge_val) && !got_warm_mode) {
        printf("[LCD_REG] feed_dp: charging-warm resume heat panel\n");
        func_elunchbox_warm_from_charging_set(false);
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

    /* 充电中：刷新电量图标并唤醒息屏 */
    if (got_charge && charge_val != 0) {
        printf("[LCD_REG] feed_dp: charging, notify LCD\n");
        home_ui_shared_battery_icon_refresh();
        heat_display_notify();
        heat_display_charge_pending = true;
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
