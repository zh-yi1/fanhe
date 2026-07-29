/**
 * @file    lb_ui_state.c
 * @brief   设备状态结构体实现 — 见 lb_ui_state.h
 */
#include "include.h"
#include "lb_proto.h"
#include "lb_ui_state.h"
#include "lb_heat_cmd.h"    // 列表同步状态机自己发 0x02 查询

#if FUNC_LUNCHBOX_UART_EN

static lb_ui_state_t lb_ui_state;

lb_ui_state_t *lb_ui_state_get(void)
{
    return &lb_ui_state;
}

void lb_ui_state_reset(void)
{
    // 蓝牙连接标志由 BLE 回调维护, 与模块上报无关, 复位时保留
    bool ble = lb_ui_state.ble_connected;
    memset(&lb_ui_state, 0, sizeof(lb_ui_state));
    lb_ui_state.ble_connected = ble;
}

void lb_ui_ble_link_set(bool connected)
{
    if (lb_ui_state.ble_connected == connected) {
        return;
    }
    lb_ui_state.ble_connected = connected;
    lb_ui_state.seq++;                   // 页面按 seq 刷新时连带刷新蓝牙图标
    printf("BLE link: %s\n", connected ? "connected" : "disconnected");
}

bool lb_ui_ble_is_connected(void)
{
    return lb_ui_state.ble_connected;
}

/** @brief 更新 u8 字段, 变化时返回 true */
static bool lb_ui_set_u8(u8 *field, u8 val)
{
    if (*field == val) {
        return false;
    }
    *field = val;
    return true;
}

/** @brief 更新 u32 字段, 变化时返回 true */
static bool lb_ui_set_u32(u32 *field, u32 val)
{
    if (*field == val) {
        return false;
    }
    *field = val;
    return true;
}

bool lb_ui_state_feed_dp(const u8 *data, u16 len)
{
    if (!data || len < 4) {
        return false;
    }

    bool changed = false;
    u16  off = 0;

    while (off + 4 <= len) {
        u8  dpid    = data[off];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];

        if (off + 4 + val_len > len) {
            break;                       // 长度越界, 丢弃余下部分
        }
        const u8 *val = data + off + 4;

        switch (dpid) {
        case LB_DPID_POWER_SWITCH:
            if (val_len >= 1) changed |= lb_ui_set_u8(&lb_ui_state.power_on, val[0]);
            break;
        case LB_DPID_HEAT_MODE:
            if (val_len >= 1) changed |= lb_ui_set_u8(&lb_ui_state.heat_mode, val[0]);
            break;
        case LB_DPID_BATTERY:
            if (val_len >= 1) changed |= lb_ui_set_u8(&lb_ui_state.battery, val[0]);
            break;
        case LB_DPID_CHARGE_STATUS:
            if (val_len >= 1) changed |= lb_ui_set_u8(&lb_ui_state.charge, val[0]);
            break;
        case LB_DPID_HEAT_DURATION:
            if (val_len >= 4) {
                u32 v = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                      | ((u32)val[2] << 8)  |  (u32)val[3];
                changed |= lb_ui_set_u32(&lb_ui_state.heat_duration, v);
            }
            break;
        case LB_DPID_REMAIN_TIME:
            if (val_len >= 4) {
                u32 v = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                      | ((u32)val[2] << 8)  |  (u32)val[3];
                changed |= lb_ui_set_u32(&lb_ui_state.remain_time, v);
            }
            break;
        case LB_DPID_HEAT_TEMP:
            if (val_len >= 1) changed |= lb_ui_set_u8(&lb_ui_state.heat_temp, val[0]);
            break;
        case LB_DPID_FAULT:
            if (val_len >= 1) changed |= lb_ui_set_u8(&lb_ui_state.fault, val[0]);
            break;
        case LB_DPID_HEAT_ENABLE:
            if (val_len >= 1) changed |= lb_ui_set_u8(&lb_ui_state.heat_enable, val[0]);
            break;
        default:
            break;                       // DP11/12/13/14: 时间/按键/版本, 非 UI 状态
        }

        off += 4 + val_len;
    }

    lb_ui_state.tick = tick_get();
    if (!lb_ui_state.valid) {
        lb_ui_state.valid = true;
        changed = true;                  // 首次上报必刷新
    }
    if (changed) {
        lb_ui_state.seq++;
    }
    return changed;
}

//-----------------------------------------------------------------------------
// 预约列表镜像
//-----------------------------------------------------------------------------

static lb_ui_schedules_t lb_ui_schedules;

lb_ui_schedules_t *lb_ui_schedules_get(void)
{
    return &lb_ui_schedules;
}

//-----------------------------------------------------------------------------
// 列表同步状态机 — 标脏后自动重查, 超时重发, 用尽置 FAIL
//
// 时序: dirty ──> 发 0x02 ──> 等首帧/后续帧 ──> 收齐(complete) ──> IDLE
//                    ↑           │ 静默超过 LB_SCH_SYNC_TIMEOUT_MS
//                    └───────────┘ 重发, 满 LB_SCH_SYNC_MAX_RETRY 次 → FAIL
//-----------------------------------------------------------------------------

#define LB_SCH_SYNC_TIMEOUT_MS  1000    // 发出查询/收到上一帧后的静默上限
#define LB_SCH_SYNC_MAX_RETRY   3       // 超时重发次数, 用尽置 FAIL

static struct {
    bool dirty;         // 有待办的重查请求
    bool waiting;       // 查询已发出, 等应答中
    bool failed;        // 重试用尽
    u8   retry;         // 本轮已重发次数
    u8   gen;           // 标脏世代号: 每次 mark_dirty/refresh 自增
    u8   sent_gen;      // 当前在途查询发出时的世代号
    u32  tick;          // 发出查询 / 收到上一帧的时刻
} lb_sch_sync;

/** @brief 置重查请求 (世代号自增, 使在途的旧查询结果不再算数) */
static void lb_sch_sync_request(void)
{
    lb_sch_sync.dirty = true;
    lb_sch_sync.failed = false;
    lb_sch_sync.gen++;
}

/** @brief 发出查询, 进入等待 */
static void lb_sch_sync_send(void)
{
    if (!lb_heat_cmd_schedule_query()) {
        return;                          // TX 被阻断, 下一轮再试
    }
    lb_sch_sync.waiting  = true;
    lb_sch_sync.sent_gen = lb_sch_sync.gen;
    lb_sch_sync.tick     = tick_get();
}

void lb_ui_schedules_refresh(void)
{
    lb_sch_sync_request();
}

lb_sch_sync_t lb_ui_schedules_sync_state(void)
{
    if (lb_sch_sync.failed) {
        return LB_SCH_SYNC_FAIL;
    }
    return (lb_sch_sync.waiting || lb_sch_sync.dirty) ? LB_SCH_SYNC_BUSY
                                                      : LB_SCH_SYNC_IDLE;
}

void lb_ui_schedules_sync_process(void)
{
    if (lb_sch_sync.waiting) {
        if (!tick_check_expire(lb_sch_sync.tick, LB_SCH_SYNC_TIMEOUT_MS)) {
            return;                      // 还在等, 帧到达时会刷新 tick
        }
        lb_sch_sync.waiting = false;
        if (lb_sch_sync.retry >= LB_SCH_SYNC_MAX_RETRY) {
            lb_sch_sync.dirty  = false;  // 别再自动重试, 等下次 refresh
            lb_sch_sync.failed = true;
            lb_sch_sync.retry  = 0;
            printf("schedules: sync FAILED (no reply)\n");
            return;
        }
        lb_sch_sync.retry++;
        printf("schedules: sync timeout, retry %u/%u\n",
               lb_sch_sync.retry, LB_SCH_SYNC_MAX_RETRY);
        lb_sch_sync_send();
        return;
    }

    if (lb_sch_sync.dirty) {
        lb_sch_sync.retry = 0;
        lb_sch_sync_send();
    }
}

void lb_ui_schedules_mark_dirty(void)
{
    lb_ui_schedules.complete = false;
    lb_ui_schedules.seq++;
    lb_sch_sync_request();               // 增/删/改生效 → 自动重查
}

bool lb_ui_schedules_feed_entry(const u8 *data, u16 len)
{
    // 条目格式(44B): total(1)+seq(1)+mode(1)+id(1)+name(32)+time(4,BE)
    //               +temp(1)+duration(1)+enabled(1)+repeat(1)
    if (!data || len < 44) {
        return false;
    }

    u8 total = data[0];
    u8 seq   = data[1];

    if (total == 0) {                    // 空列表: 一帧结束
        lb_ui_schedules.count = 0;
        lb_ui_schedules.total = 0;
        lb_ui_schedules.complete = true;
    } else {
        if (seq == 1) {                  // 新一轮传输, 重开列表
            lb_ui_schedules.count = 0;
            lb_ui_schedules.complete = false;
        }
        if (lb_ui_schedules.count < LB_SCHEDULE_MAX) {
            lb_ui_schedule_t *e = &lb_ui_schedules.list[lb_ui_schedules.count];
            e->mode     = data[2];
            e->id       = data[3];
            memcpy(e->name, data + 4, 32);
            e->time     = ((u32)data[36] << 24) | ((u32)data[37] << 16)
                        | ((u32)data[38] << 8)  |  (u32)data[39];
            e->temp     = data[40];
            e->duration = data[41];
            e->enabled  = data[42];
            e->repeat   = data[43];
            lb_ui_schedules.count++;
        }
        lb_ui_schedules.total = total;
        if (seq >= total) {
            lb_ui_schedules.complete = true;
        }
    }

    lb_ui_schedules.valid = true;
    lb_ui_schedules.tick = tick_get();
    lb_ui_schedules.seq++;

    // 同步状态机: 每收一帧刷新静默计时, 收齐则本轮结束
    // (APP 自己查列表时 waiting=false, 应答照常入镜像, 不影响状态机)
    if (lb_sch_sync.waiting) {
        lb_sch_sync.tick = tick_get();
        if (lb_ui_schedules.complete) {
            lb_sch_sync.waiting = false;
            lb_sch_sync.failed  = false;
            lb_sch_sync.retry   = 0;
            // 收齐期间又被标脏 → 这份已过时, 保留 dirty 让下一轮再查
            if (lb_sch_sync.sent_gen == lb_sch_sync.gen) {
                lb_sch_sync.dirty = false;
            }
        }
    }
    return true;
}

//-----------------------------------------------------------------------------
// 自动跳页路由
//-----------------------------------------------------------------------------

lb_ui_route_t lb_ui_route_poll(void)
{
    static bool inited;
    static u8   last_mode;
    static u8   last_enable;

    lb_ui_state_t *st = lb_ui_state_get();
    if (!st->valid) {
        return LB_UI_ROUTE_NONE;
    }

    // 只盯路由相关的两个字段, 电量/温度等变化不触发跳页
    if (inited && st->heat_mode == last_mode && st->heat_enable == last_enable) {
        return LB_UI_ROUTE_NONE;
    }
    bool first = !inited;
    u8 prev_enable = last_enable;
    inited = true;
    last_mode = st->heat_mode;
    last_enable = st->heat_enable;

    if (st->heat_enable) {
        if (st->heat_mode == LB_MODE_WARM) {
            return LB_UI_ROUTE_WARM;         // 保温进行中
        }
        if (st->heat_mode >= LB_MODE_CUSTOM && st->heat_mode <= LB_MODE_RESERVE) {
            return LB_UI_ROUTE_HEAT;         // 自定义/鸡腿/意面/预约加热中
        }
        return LB_UI_ROUTE_NONE;             // 使能但模式未知, 不动
    }

    if (!first && prev_enable) {
        return LB_UI_ROUTE_HOME;             // 加热/保温 → 停止, 回首页
    }
    return LB_UI_ROUTE_NONE;                 // 一直是停止态 (含首次上报即空闲)
}

//-----------------------------------------------------------------------------
// 状态查询快捷接口
//-----------------------------------------------------------------------------

bool lunchbox_heating_task_active(void)
{
    if (!lb_ui_state.valid) {
        return false;
    }
    return lb_ui_state.heat_enable != 0;
}

u8 lunchbox_get_heat_mode(void)
{
    if (!lb_ui_state.valid) {
        return 0;
    }
    return lb_ui_state.heat_mode;
}

u8 lunchbox_get_heat_enable(void)
{
    if (!lb_ui_state.valid) {
        return 0;
    }
    return lb_ui_state.heat_enable;
}

#endif // FUNC_LUNCHBOX_UART_EN
