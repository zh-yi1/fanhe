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

//-----------------------------------------------------------------------------
// 镜像预写 (预测-校正机制)
//
// 本机发出改状态的命令后, 页面立刻切换, 而镜像要等模块应答才更新 ——
// 首屏会拿旧数据渲染。预写 = 命令发出成功的同时把"期望结果"写进镜像:
//   - seq++ 让页面立刻拿到新值
//   - 同步路由基线: 预写不产生路由边沿 (否则本地写 en 0→1 会自己触发跳页,
//     和页面手动切页打架; 1→0 会误判"自然结束进保温")
//   - 不触发 schedules_mark_dirty (那是给"模块自己启动=消费预约"用的)
// 校正: 模块应答(全量字段)回来, 值相同 → feed_dp 判无变化, 无痕;
//       值不同 → 打 ERROR + 正常覆盖, 页面按 seq 自纠。
// 自愈: 预写后 500ms 没等到任何 0x01 → 补发状态查询强制拉真相。
//-----------------------------------------------------------------------------

#define LB_PREDICT_TIMEOUT_MS   500

#define LB_PRED_PWR     BIT(0)
#define LB_PRED_MODE    BIT(1)
#define LB_PRED_TEMP    BIT(2)
#define LB_PRED_DUR     BIT(3)
#define LB_PRED_REMAIN  BIT(4)
#define LB_PRED_EN      BIT(5)

static struct {
    bool pending;       // 预写后还没等到模块 0x01 (应答/上报都算真相)
    u32  tick;          // 预写时刻 (自愈超时用)
    u8   mask;          // 本次预写了哪些字段 (LB_PRED_*)
    u8   pwr, mode, temp, en;
    u32  dur, remain;
} lb_predict;

/* 路由基线 (原 lb_ui_route_poll 的函数内 static, 预写要同步它所以提出来) */
static bool lb_route_inited;
static u8   lb_route_last_mode;
static u8   lb_route_last_enable;

/** @brief 预写落账: 路由基线对齐 + seq++ + 开校正窗口 */
static void lb_predict_commit(u8 mask)
{
    lb_route_inited      = true;         // 预写不产生路由边沿
    lb_route_last_mode   = lb_ui_state.heat_mode;
    lb_route_last_enable = lb_ui_state.heat_enable;

    /* 故意不置 valid: 正常运行时它本来就是真; 开机时序的 power_on 也走
     * 预写, 那一刻 lb_boot_lid_wait() 正在等"模块首帧"(以 valid 为判据),
     * 预写若置真会让等待提前结束, 盖盖弹窗判定拿到空镜像 */
    lb_ui_state.tick = tick_get();
    lb_ui_state.seq++;

    lb_predict.pending = true;
    lb_predict.tick    = tick_get();
    lb_predict.mask    = mask;
    printf("ui_state: predict pwr=%u mode=%u en=%u dur=%u remain=%u temp=%u\n",
           lb_ui_state.power_on, lb_ui_state.heat_mode, lb_ui_state.heat_enable,
           (unsigned)lb_ui_state.heat_duration,
           (unsigned)lb_ui_state.remain_time, lb_ui_state.heat_temp);
}

void lb_ui_state_predict_start(u8 mode, u8 temp_idx, u32 duration_min, u32 remain_min)
{
    lb_ui_state.power_on      = 1;
    lb_ui_state.heat_mode     = mode;
    lb_ui_state.heat_temp     = temp_idx;
    lb_ui_state.heat_duration = duration_min;
    lb_ui_state.remain_time   = remain_min;
    lb_ui_state.heat_enable   = 1;
    lb_predict.pwr  = 1;
    lb_predict.mode = mode;
    lb_predict.temp = temp_idx;
    lb_predict.dur  = duration_min;
    lb_predict.remain = remain_min;
    lb_predict.en   = 1;
    lb_predict_commit(LB_PRED_PWR | LB_PRED_MODE | LB_PRED_TEMP |
                      LB_PRED_DUR | LB_PRED_REMAIN | LB_PRED_EN);
}

void lb_ui_state_predict_stop(bool with_mode_off)
{
    lb_ui_state.heat_enable = 0;
    lb_predict.en = 0;
    u8 mask = LB_PRED_EN;
    if (with_mode_off) {
        lb_ui_state.heat_mode = LB_MODE_OFF;
        lb_predict.mode = LB_MODE_OFF;
        mask |= LB_PRED_MODE;
    }
    lb_predict_commit(mask);
}

void lb_ui_state_predict_power(bool on)
{
    lb_ui_state.power_on = on ? 1 : 0;
    lb_predict.pwr = lb_ui_state.power_on;
    lb_predict_commit(LB_PRED_PWR);
}

/** @brief 自愈轮询: 预写悬空超时没等到模块 0x01 → 补发状态查询拉真相 */
void lb_ui_state_predict_poll(void)
{
    if (!lb_predict.pending) {
        return;
    }
    if (tick_check_expire(lb_predict.tick, LB_PREDICT_TIMEOUT_MS)) {
        lb_predict.pending = false;
        printf("ui_state ERROR: predict no reply in %ums, query truth\n",
               LB_PREDICT_TIMEOUT_MS);
        lb_heat_cmd_status_query();
    }
}

/** @brief 校正窗口内逐 DP 核对: 模块回的值和预写值不一致 → 打 ERROR */
static void lb_predict_verify(u8 dpid, const u8 *val, u16 val_len)
{
    if (!lb_predict.pending) {
        return;
    }
    u32 got;
    switch (dpid) {
    case LB_DPID_POWER_SWITCH:
        if ((lb_predict.mask & LB_PRED_PWR) && val_len >= 1 && val[0] != lb_predict.pwr)
            printf("ui_state ERROR: DP1 sent=%u got=%u\n", lb_predict.pwr, val[0]);
        break;
    case LB_DPID_HEAT_MODE:
        if ((lb_predict.mask & LB_PRED_MODE) && val_len >= 1 && val[0] != lb_predict.mode)
            printf("ui_state ERROR: DP2 sent=%u got=%u\n", lb_predict.mode, val[0]);
        break;
    case LB_DPID_HEAT_TEMP:
        if ((lb_predict.mask & LB_PRED_TEMP) && val_len >= 1 && val[0] != lb_predict.temp)
            printf("ui_state ERROR: DP7 sent=%u got=%u\n", lb_predict.temp, val[0]);
        break;
    case LB_DPID_HEAT_DURATION:
        if ((lb_predict.mask & LB_PRED_DUR) && val_len >= 4) {
            got = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                | ((u32)val[2] << 8)  |  (u32)val[3];
            if (got != lb_predict.dur)
                printf("ui_state ERROR: DP5 sent=%u got=%u\n",
                       (unsigned)lb_predict.dur, (unsigned)got);
        }
        break;
    case LB_DPID_REMAIN_TIME:
        if ((lb_predict.mask & LB_PRED_REMAIN) && val_len >= 4) {
            got = ((u32)val[0] << 24) | ((u32)val[1] << 16)
                | ((u32)val[2] << 8)  |  (u32)val[3];
            if (got != lb_predict.remain)
                printf("ui_state ERROR: DP6 sent=%u got=%u\n",
                       (unsigned)lb_predict.remain, (unsigned)got);
        }
        break;
    case LB_DPID_HEAT_ENABLE:
        if ((lb_predict.mask & LB_PRED_EN) && val_len >= 1 && val[0] != lb_predict.en)
            printf("ui_state ERROR: DP10 sent=%u got=%u\n", lb_predict.en, val[0]);
        break;
    default:
        break;
    }
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
    u8   prev_enable = lb_ui_state.heat_enable;

    while (off + 4 <= len) {
        u8  dpid    = data[off];
        u16 val_len = ((u16)data[off + 2] << 8) | data[off + 3];

        if (off + 4 + val_len > len) {
            break;                       // 长度越界, 丢弃余下部分
        }
        const u8 *val = data + off + 4;

        lb_predict_verify(dpid, val, val_len);   // 预写校正窗口内核对

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

    // 加热使能 0→1: 若是预约到点, 模块侧已消费该条预约, 本地列表过期 → 自动重查。
    // 不区分模式 (执行预约时 DP2 回 4 还是回预约里存的模式未实测), 手动加热也顺带
    // 重查一次, 代价只是一条 0x02。预约灯的"到点熄灭"就靠这次重查兜底。
    if (!prev_enable && lb_ui_state.heat_enable) {
        lb_ui_schedules_mark_dirty();
    }

    lb_predict.pending = false;          // 真相已到 (不管对不对), 校正窗口关闭

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

u8 lb_ui_schedule_alloc_id(void)
{
    u16 used = 0;                        // bit1~bit12 对应 ID 1~12
    u8  i;

    for (i = 0; i < lb_ui_schedules.count && i < LB_SCHEDULE_MAX; i++) {
        u8 id = lb_ui_schedules.list[i].id;
        if (id >= 1 && id <= LB_SCHEDULE_ID_MAX) {
            used |= (u16)1 << id;
        }
    }
    for (i = 1; i <= LB_SCHEDULE_ID_MAX; i++) {
        if (!(used & ((u16)1 << i))) {
            return i;
        }
    }
    printf("schedules: no free id (all %u used)\n", LB_SCHEDULE_ID_MAX);
    return 0;
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

// 本机/APP 主动发过停止命令 → 接下来的"加热中→停止"是人停的, 不进保温。
// 不能用 DP6 剩余时间区分: 实测模块被动停止时也会把 Remain 清 0。
static bool lb_heat_stop_expected;

void lb_ui_heat_stop_expected(void)
{
    lb_heat_stop_expected = true;
}

lb_ui_route_t lb_ui_route_poll(void)
{
    /* 基线在文件级 (lb_route_*): 预写要同步它, 见 lb_predict_commit() */
    lb_ui_state_t *st = lb_ui_state_get();
    if (!st->valid) {
        return LB_UI_ROUTE_NONE;
    }

    // 只盯路由相关的两个字段, 电量/温度等变化不触发跳页
    if (lb_route_inited && st->heat_mode == lb_route_last_mode
        && st->heat_enable == lb_route_last_enable) {
        return LB_UI_ROUTE_NONE;
    }
    bool first = !lb_route_inited;
    u8 prev_enable = lb_route_last_enable;
    u8 prev_mode   = lb_route_last_mode;
    lb_route_inited = true;
    lb_route_last_mode = st->heat_mode;
    lb_route_last_enable = st->heat_enable;

    if (st->heat_enable) {
        lb_heat_stop_expected = false;       // 新任务开始, 旧的停止标志作废
        if (st->heat_mode == LB_MODE_WARM) {
            return LB_UI_ROUTE_WARM;         // 保温进行中
        }
        if (st->heat_mode >= LB_MODE_CUSTOM && st->heat_mode <= LB_MODE_RESERVE) {
            return LB_UI_ROUTE_HEAT;         // 自定义/鸡腿/意面/预约加热中
        }
        return LB_UI_ROUTE_NONE;             // 使能但模式未知, 不动
    }

    if (!first && prev_enable) {
        bool expected = lb_heat_stop_expected;
        lb_heat_stop_expected = false;
        // 加热(1~4)自然结束(没人发过停止) → 进保温;
        // 保温页 enter 看到模块没在保温, 会自动下发保温命令
        if (!expected && prev_mode >= LB_MODE_CUSTOM && prev_mode <= LB_MODE_RESERVE) {
            return LB_UI_ROUTE_WARM;
        }
        return LB_UI_ROUTE_HOME;             // 主动停止 / 保温结束, 回首页
    }
    return LB_UI_ROUTE_NONE;                 // 一直是停止态 (含首次上报即空闲)
}

//-----------------------------------------------------------------------------
// 状态查询快捷接口
//-----------------------------------------------------------------------------

bool lunchbox_reservation_pending(void)
{
    u8 i;

    if (!lb_ui_schedules.valid) {
        return false;
    }
    // 不要求 complete: 传输中按已收到的条目尽力判断, 收齐后自然修正
    for (i = 0; i < lb_ui_schedules.count && i < LB_SCHEDULE_MAX; i++) {
        if (lb_ui_schedules.list[i].enabled) {
            return true;
        }
    }
    return false;
}

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
