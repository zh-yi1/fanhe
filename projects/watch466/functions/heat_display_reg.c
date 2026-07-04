#include "include.h"
#include "heat_display_reg.h"
#include "func_lunchbox_uart.h"

static heat_display_cb_t heat_display_cb;
static heat_display_info_t heat_display_last;
static bool heat_display_has_last;
static bool heat_display_charge_pending;  /* 充电中状态唤醒标志 */

static void heat_display_notify(void)
{
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
    u16 off = 0;
    u32 remain_min = 0;
    u16 temp_f = 0;
    bool got_remain = false;
    bool got_temp = false;
    bool got_enable = false;
    bool got_charge = false;
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
                printf("[LCD_REG] feed_dp: REMAIN_TIME=%u min\n", remain_min);
            }
            break;
        case LB_DPID_HEAT_TEMP:
            if (val_len >= 1) {
                temp_f = heat_display_temp_idx_to_f(val[0]);
                got_temp = true;
                printf("[LCD_REG] feed_dp: HEAT_TEMP idx=%u -> %u F\n", val[0], temp_f);
            }
            break;
        case LB_DPID_HEAT_ENABLE:
            if (val_len >= 1) {
                heating = (val[0] != 0);
                got_enable = true;
                printf("[LCD_REG] feed_dp: HEAT_ENABLE=%u (heating=%d)\n", val[0], heating);
            }
            break;
        case LB_DPID_CHARGE_STATUS:
            if (val_len >= 1) {
                charge_val = val[0];
                got_charge = true;
                printf("[LCD_REG] feed_dp: CHARGE_STATUS=%u\n", charge_val);
            }
            break;
        default:
            break;
        }
        off += 4 + val_len;
    }

    /* 加热已停止：清零 remain，让 heat_display_heating_active() 返回 false，避免阻止息屏/误唤醒 */
    if (got_enable && !heating) {
        printf("[LCD_REG] feed_dp: heating stopped, clear remain\n");
        if (heat_display_has_last && heat_display_last.remain_min > 0) {
            heat_display_last.remain_min = 0;
            heat_display_notify();
        }
        return;
    }
    if (got_remain && got_temp) {
        heat_display_show(remain_min, temp_f);
    } else if (got_remain) {
        heat_display_info_t last;

        if (heat_display_get_last(&last)) {
            heat_display_show(remain_min, last.temp_f);
        }
    } else if (got_temp) {
        heat_display_info_t last;

        if (heat_display_get_last(&last)) {
            heat_display_show(last.remain_min, temp_f);
        }
    }

    /* 充电中：通知 LCD 唤醒屏幕显示充电图标 */
    if (got_charge && charge_val == 1) {
        printf("[LCD_REG] feed_dp: charging, notify LCD\n");
        heat_display_notify();
        heat_display_charge_pending = true;  /* 主循环可用此标志唤醒息屏 */
    }
}

bool heat_display_charge_wake_pending(void)
{
    bool pending = heat_display_charge_pending;
    heat_display_charge_pending = false;
    return pending;
}
