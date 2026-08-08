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
static u8   lb_route_last_fault;
static u8   lb_route_last_charge;

/* 加热中插电 → 模块把这份任务改成保温, 但 DP5(时长)/DP6(剩余) 仍是原来那份,
 * 剩余时间继续往下走。记住原加热模式和转保温前的温度档位 (转完 DP7 就变成
 * 保温温度了), 拔线时按 DP6 的剩余分钟数续跑:
 *   DP6 > 0  → 回加热页, 补一条加热命令把剩下的时间跑完
 *   DP6 == 0 → 加热时长已经用完, 回主界面
 * 加热账在保温里耗完 (模块把 DP6 走到 0 自停, 路由补 24h 续保温) 时标记
 * 随之清掉 —— 账没了, 之后拔线走"手动保温"路线留在保温页, 不再续加热。
 * 别的保温 (APP 下发 / 加热自然结束转的) 不置 active, 不适用这套。
 * (声明放在预写函数之前: 预写路径不产生路由边沿, 标记的记录/清理要在预写里做) */
static struct {
    bool active;
    u8   mode;          // 被转走的加热模式 (1~4)
    u8   temp;          // 转保温之前的温度档位
} lb_warm_from_heat;

static u8 lb_last_heat_temp;    // 加热中持续记录, 转保温后 DP7 会变成保温温度

/* 本机/APP 主动发过停止命令 → 接下来的"加热中→停止"是人停的, 不进保温。
 * 不能用 DP6 剩余时间区分: 实测模块被动停止时也会把 Remain 清 0。
 * 声明放预写区之前: 本机停止走预写, en 1→0 边沿被基线同步吃掉, 路由的
 * 消费清零执行不到, 标志会残留; 只能靠下一次 predict_start 清 (漏清的
 * 实测现象: 1H 自然结束被误判成主动停止, 直接回主界面不进保温)。 */
static bool lb_heat_stop_expected;

/** @brief 预写落账: 路由基线对齐 + seq++ + 开校正窗口 */
static void lb_predict_commit(u8 mask)
{
    lb_route_inited      = true;         // 预写不产生路由边沿
    lb_route_last_mode   = lb_ui_state.heat_mode;
    lb_route_last_enable = lb_ui_state.heat_enable;
    lb_route_last_fault  = lb_ui_state.fault;   // 预写不碰 fault/charge, 对齐防万一
    lb_route_last_charge = lb_ui_state.charge;

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
    /* 预写把路由基线一起同步掉, lb_ui_route_poll 看不到这次启动的边沿,
     * 它里面"新加热任务 → 记温度/清转保温标记"那两行不会执行, 在这补:
     * 否则拔线续跑会用上一份加热的旧温度 (实测日志: 140F 续成了 158F)。
     * 保温启动(mode=5)不清标记 —— 加热中插电转保温期间, APP/页面若下发
     * 保温也不该抹掉"拔线续加热"的资格; 加热账耗完的清理由路由的续保温
     * 分支自己做 (那边清完才发 resume)。 */
    if (mode >= LB_MODE_CUSTOM && mode <= LB_MODE_RESERVE) {
        lb_last_heat_temp = temp_idx;
        lb_warm_from_heat.active = false;
    }
    /* 路由 "st->heat_enable → 旧停止标志作废" 那句同样被基线同步跳过, 在这补:
     * 上一次手动退出加热/保温页残留的停止标志, 不能污染这份新任务的自然结束 */
    lb_heat_stop_expected = false;
    lb_predict_commit(LB_PRED_PWR | LB_PRED_MODE | LB_PRED_TEMP |
                      LB_PRED_DUR | LB_PRED_REMAIN | LB_PRED_EN);
}

void lb_ui_state_predict_stop(bool with_mode_off)
{
    /* 主动停止 = 这份加热/保温会话终结, 转保温标记随之作废。
     * 必须在这清: 停止走预写, 路由基线被同步, en 1→0 的边沿路由看不到,
     * 它里面的清理执行不到 → 残留标记会污染下一份手动保温, 拔线时被
     * 误判成"加热转来的"而自动续跑一条不存在的加热 (实测日志 15:52:44)。 */
    lb_warm_from_heat.active = false;
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
        case LB_DPID_HEAT_TRIGGER:
            if (val_len >= 1) changed |= lb_ui_set_u8(&lb_ui_state.heat_trigger, val[0]);
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

// lb_heat_stop_expected 声明在预写区之前 (predict_start 要清它, 见那边注释)
void lb_ui_heat_stop_expected(void)
{
    lb_heat_stop_expected = true;
}

/*
 * 路由意义上的"故障": 模块报了 DP9 且不是低电。
 * 低电有独立页面 (func.c 按 g_ui_sys.lowbat 切 FUNC_LOWBAT), 不走这里,
 * 否则会跟低电页抢跳页。
 * 注: 模块若把 DP9 当 0/1 两值回 (见 lb_proto.h 的说明), 这里同样成立。
 */
static bool lb_fault_active(u8 fault)
{
    return fault != LB_FAULT_NONE && fault != LB_FAULT_LOW_BATTERY;
}

/* 充电线在不在: DP4 0=未充电 1=充电中 2=已充满 —— 充满时线还插着, 算在充电。
 * 用模块的 DP4 而不是本机 CHARGE_DC_IN(): 模块侧已消抖 (见黑屏充电页拔线处理),
 * 且路由本来就只吃镜像这一份数据。 */
/* 保温页每帧报上来的"已保温"显示值: 补发保温命令时要把它写进 DP6, 让模块
 * 的账接着屏幕走。页面不在保温页时这个值是上次留下的残值, 只有上面那条
 * "补发"分支会读它, 而那条分支只在保温进行中触发, 值必然是新鲜的。 */
static u32 lb_warm_elapsed_min;

void lb_ui_warm_elapsed_set(u32 minutes)
{
    lb_warm_elapsed_min = minutes;
}

static bool lb_charge_plugged(u8 charge)
{
    return charge != 0;
}

lb_ui_route_t lb_ui_route_poll(void)
{
    /* 基线在文件级 (lb_route_*): 预写要同步它, 见 lb_predict_commit() */
    lb_ui_state_t *st = lb_ui_state_get();
    if (!st->valid) {
        return LB_UI_ROUTE_NONE;
    }

    // 只盯路由相关的四个字段, 电量/温度等变化不触发跳页
    if (lb_route_inited && st->heat_mode == lb_route_last_mode
        && st->heat_enable == lb_route_last_enable
        && st->fault == lb_route_last_fault
        && st->charge == lb_route_last_charge) {
        return LB_UI_ROUTE_NONE;
    }
    bool first = !lb_route_inited;
    u8 prev_enable = lb_route_last_enable;
    u8 prev_mode   = lb_route_last_mode;
    u8 prev_fault  = lb_route_last_fault;
    u8 prev_charge = lb_route_last_charge;
    lb_route_inited = true;
    lb_route_last_mode = st->heat_mode;
    lb_route_last_enable = st->heat_enable;
    lb_route_last_fault = st->fault;
    lb_route_last_charge = st->charge;

    bool fault_now = lb_fault_active(st->fault);
    bool plugged   = lb_charge_plugged(st->charge);

    /* 模块报故障 → 停止加热 + 回主界面, 不进保温。
     * 故障与 DP10=0 通常在同一帧上报, 这里先判故障边沿把边沿吃掉;
     * 若故障晚一帧到, 下面"自然结束"分支也会因 fault_now 改判回主界面。
     *
     * 模块只报故障没停加热时本机补一条停止 (DP10=0 + DP1=1, 与退出加热页
     * 同一条): 不能只靠页面 exit —— 用户在设置页等非加热页时故障根本不跳页,
     * 加热就停不下来。stop 内部已置停止标志并预写镜像 heat_enable=0,
     * 所以加热页 exit 那条补发不会重复触发。
     * 关机时序进行中不插队: 那边正按 heat_off → power_off 一步步走。 */
    if (!first && fault_now && !lb_fault_active(prev_fault)) {
        bool stop_sent = false;
        lb_heat_stop_expected = true;
        if (st->heat_enable && !lunchbox_shutdown_is_active()
            && !lunchbox_shutdown_is_done()) {
            stop_sent = lb_heat_cmd_stop();
        }
        printf("route: module fault=%u -> home (stop %s)\n", st->fault,
               stop_sent ? "sent" : "skip");
        return LB_UI_ROUTE_HOME;
    }

    /* 拔充电线, 按"这份保温是哪来的"分三条:
     *   加热中插电转来的, 还有剩余时间 → 回加热页, 补命令把剩下的跑完
     *   加热中插电转来的, 时间已用完   → 回主界面 (加热本来就该结束了)
     *   人自己选的保温, 还在跑         → 留在保温页, 拔线不打断
     *   其余 (保温已停 / 空闲)         → 回主界面
     * 加热中拔线不动 —— 那是电池供电继续加热, 不该被拔线打断。
     * 黑屏充电页拔线是另一条路(走关机时序), 这里的边沿在 apply 里
     * 只作用于加热页/保温页, 碰不到它。 */
    if (!first && !plugged && lb_charge_plugged(prev_charge)
        && !(st->heat_enable && st->heat_mode >= LB_MODE_CUSTOM
             && st->heat_mode <= LB_MODE_RESERVE)) {
        if (lb_warm_from_heat.active && st->remain_time > 0) {
            u8  mode = lb_warm_from_heat.mode;
            u8  temp = lb_warm_from_heat.temp;
            u32 left = st->remain_time;
            lb_warm_from_heat.active = false;
            printf("route: unplugged, %lu min heat left -> resume heat (mode=%u temp=%u)\n",
                   (unsigned long)left, mode, temp);
            /* 必须补这条命令, 不能只跳页:
             *   ① 模块这会儿在保温, 不发它就一直保温, 加热页的倒计时是假的;
             *   ② 保温页 exit 有一条"还在保温就停保温", 发了这条之后镜像预写
             *      成 mode=加热, 那条判据不成立, 不会把刚续上的加热掐掉。 */
            lb_heat_cmd_start(mode, temp, left);
            return LB_UI_ROUTE_HEAT;
        }
        bool was_from_heat = lb_warm_from_heat.active;
        lb_warm_from_heat.active = false;
        /* 人自己选的保温 (模式页下发 / APP 下发) 还在跑 → 拔线不打断,
         * 留在保温页接着显示, 等它自己结束或用户退出。
         * 加热转来的那份不适用: 它的时长是那份加热的, 跑完就该回主界面
         * (上面 remain>0 已经先接走了续加热那条)。 */
        if (!was_from_heat && st->heat_enable && st->heat_mode == LB_MODE_WARM) {
            printf("route: unplugged, manual keep-warm still running -> stay\n");
            return LB_UI_ROUTE_NONE;
        }
        printf("route: charger unplugged -> home\n");
        return LB_UI_ROUTE_HOME;
    }

    if (st->heat_enable) {
        lb_heat_stop_expected = false;       // 新任务开始, 旧的停止标志作废
        if (st->heat_mode == LB_MODE_WARM) {
            /* 加热中插电 → 模块转保温, DP5/DP6 仍是那份加热的, 剩余继续走。
             * 记下原模式和转保温前的温度, 拔线时续跑。 */
            if (!first && plugged && prev_enable
                && prev_mode >= LB_MODE_CUSTOM && prev_mode <= LB_MODE_RESERVE) {
                lb_warm_from_heat.active = true;
                lb_warm_from_heat.mode   = prev_mode;
                lb_warm_from_heat.temp   = lb_last_heat_temp;
                printf("route: heating(mode=%u) -> keep-warm on charge, %lu min left\n",
                       prev_mode, (unsigned long)st->remain_time);
            }
            return LB_UI_ROUTE_WARM;         // 保温进行中
        }
        if (st->heat_mode >= LB_MODE_CUSTOM && st->heat_mode <= LB_MODE_RESERVE) {
            lb_last_heat_temp = st->heat_temp;   // 转保温后 DP7 会变, 先记着
            lb_warm_from_heat.active = false;    // 新的加热任务, 旧标记作废
            return LB_UI_ROUTE_HEAT;         // 自定义/鸡腿/意面/预约加热中
        }
        return LB_UI_ROUTE_NONE;             // 使能但模式未知, 不动
    }

    if (!first && prev_enable) {
        bool expected  = lb_heat_stop_expected;
        bool from_heat = lb_warm_from_heat.active;
        lb_heat_stop_expected = false;
        // 加热(1~4)自然结束(没人发过停止, 且模块没在故障) → 进保温;
        // 保温页 enter 看到模块没在保温, 会自动下发保温命令
        if (!expected && !fault_now
            && prev_mode >= LB_MODE_CUSTOM && prev_mode <= LB_MODE_RESERVE) {
            lb_warm_from_heat.active = false;
            return LB_UI_ROUTE_WARM;
        }
        /* 充电中由加热转来的保温会话到点停了 (实测 2026-08-08: 模块不是短计时
         * 自停, 而是把那份加热账 DP5/DP6 一路走到 0 才停 —— 这一下就是"加热
         * 时长在保温里耗完了")。线还插着就补一条 194F/24h 接着保温, 页面留在
         * 保温界面。
         *
         * 用 resume 而不是 start: 带上 DP6 = 24h − 已保温, 让模块的账
         * (DP5−DP6) 一上来就等于屏幕显示值, 而不是从 0 重开。
         * 协议 §4 标 DP6 "APP下发 = ×", 但实测模块认: 下 1440/1386 应答原样
         * 回 1440/1386, 保温时间无缝接续。
         *
         * from_heat 必须清: 加热账已耗完, 这份保温从此按"手动保温"路线走,
         * 之后拔线留在保温页 (:648), 不再续加热。不清的话拔线会拿保温账的
         * DP6 当"加热剩余"续出一条超长加热 (实测日志 15:19:02: 凭空续出
         * 1366 分钟 Custom 加热并跳回加热页)。 */
        if (!expected && !fault_now && prev_mode == LB_MODE_WARM
            && plugged && from_heat) {
            u32 done = lb_warm_elapsed_min;
            u32 left = (done < LB_WARM_DURATION_MIN)
                     ? LB_WARM_DURATION_MIN - done : 1;
            lb_warm_from_heat.active = false;
            printf("route: heat quota used up in keep-warm -> resume 194F, done=%lu left=%lu\n",
                   (unsigned long)done, (unsigned long)left);
            lb_heat_cmd_resume(LB_MODE_WARM,
                               lunchbox_temp_f_to_idx(LB_WARM_TEMP_F),
                               LB_WARM_DURATION_MIN, left);
            return LB_UI_ROUTE_NONE;
        }
        lb_warm_from_heat.active = false;
        // 充电中普通保温结束 → 页面留在保温界面, 等拔线才回首页。
        // 未充电时保温结束仍按老规矩回首页。
        if (!expected && !fault_now && prev_mode == LB_MODE_WARM && plugged) {
            printf("route: keep-warm ended while charging, stay on warm page\n");
            return LB_UI_ROUTE_NONE;
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
