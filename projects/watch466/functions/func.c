#include "include.h"
#include "func_tbl.h"
#include "func.h"
#include "new_ui/general_ui.h"      /* g_ui_sys(转include app_ui.h) + lb_ui_sync_pull */
#if ELUNCHBOX_PANEL_EN
#include "lowpower/elunchbox_lp.h"
#include "func_key_lock.h"
#endif
extern void func_confirm_overlay_show(void);
extern void func_confirm_overlay_hide(void);
extern bool func_confirm_overlay_visible(void);
extern bool func_confirm_overlay_poll(void);
//#include "func_reservation.h"
#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#endif
#if USER_PANEL_LED
#include "func_led.h"
#endif

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

void home_gpu_wait_idle(void)
{
    os_gui_draw_w4_done();
    os_gui_draw_w4_done();
}

func_cb_t func_cb AT(.buf.func_cb);

#if ELUNCHBOX_PANEL_EN
u8 func_res_allow_switch;
#endif

bool gui_get_auto_power_en(void);

#if ELUNCHBOX_PANEL_EN
static bool func_elunchbox_res_key_page_ok(void)
{
    switch (func_cb.sta) {
    case FUNC_HOME:
    case FUNC_MODE:
    case FUNC_SETUP:
    case FUNC_LANGUAGEING:
    case FUNC_TIMEING:
    case FUNC_VERINFO:
        return true;
    default:
        return false;
    }
}

void func_elunchbox_switch_to_reservation(void)
{
#if !FUNC_RESERVATION_UI_EN
    return;
#endif
    if (func_cb.sta == FUNC_RESERVATION) {
        return;
    }
    if (sys_cb.flag_swithing) {
        return;
    }
    func_res_allow_switch = 1;
    func_switch_to(FUNC_RESERVATION, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
    func_res_allow_switch = 0;
}

void func_elunchbox_res_key_poll(void)
{
#if USER_PT8028_KEY && FUNC_RESERVATION_UI_EN
    if (sys_cb.flag_swithing) {
        return;
    }
    if (pt8028_take_res_key_pending() && func_elunchbox_res_key_page_ok()) {
        func_elunchbox_switch_to_reservation();
    }
#endif
}
#endif

#if FUNC_LUNCHBOX_UART_EN
/**
 * @brief 加热模块状态变化 → 强制切页 (唯一调用点)
 *
 * lb_ui_route_poll() 是边沿触发且内部状态只能被消费一次, 全工程只准这里调。
 * 覆盖的场景: APP 远程启停加热、预约到点模块自己开始加热、加热结束。
 * 其余字段(电量/温度/剩余时间)变化不在这里管, 由各页面按 seq 自行刷新。
 */
/**
 * 关机时序走完 → 主板真断电
 *
 * 落地方式与长按 TCH5 3s 完全一致 (func_key.c: manual_off 超低功耗深睡,
 * 仅 PE1+PB9 唤醒), 不走 FUNC_PWROFF —— 那条路在充电时 func_pwroff() 会
 * return 不断电, 会陷入反复进关机页。
 *
 * 时序期间用户插上了充电线的情况: 这里再查一次 blocked, 命中就放弃断电。
 * 加热模块那边已经关了, 本机留着不关 —— 插线状态下本来就不该断电。
 */
/**
 * manual_off 深睡唤醒后的落页 (唤醒不重启, 老页面原地复活, 这里纠正去向)
 *
 * 闩锁在深睡循环里置入 (func_lowpwr.c):
 *   KEY  = TCH5 长按 2s → 补跑开机时序(模块还关着), 进主界面
 *   UART = 任何串口指令 → 主动发状态查询读模块数据, 开 3s 观察窗等应答:
 *          充电 → 黑屏充电页; 加热 → 加热页;
 *          都不是 → 留在原页, 5min 无操作自动关机兜底
 * (唤醒那一帧常因 UART 起得慢收残, 不能靠它判醒因, 所以醒来后主动查;
 *  曾在睡眠循环里偷听判帧, 实测收帧不完整, 已废弃)
 */
static void lb_wake_apply(void)
{
    static u32 wake_watch_tick;         /* UART 唤醒观察窗起点, 0=没开 */
    static u32 wake_query_tick;         /* 上次发状态查询的时刻 (窗内重发用) */

    if (sys_cb.flag_swithing) {
        return;                         /* 切页动画中不消费, 下一轮再取 */
    }
    switch (lunchbox_wake_reason_take()) {
    case LB_WAKE_KEY:
        printf("wake: key -> home\n");
        wake_watch_tick = 0;
        lunchbox_boot_seq_kick();       /* power_on + 查预约列表 */
        if (func_cb.sta != FUNC_HOME) {
            func_cb.sta = FUNC_HOME;
        }
        break;

    case LB_WAKE_UART:
        printf("wake: uart, query module status\n");
        lb_heat_cmd_status_query();
        wake_watch_tick = tick_get();
        wake_query_tick = wake_watch_tick;
        break;

    default:
        break;
    }

    if (wake_watch_tick == 0) {
        return;
    }
    if (tick_check_expire(wake_watch_tick, 3000)) {
        wake_watch_tick = 0;            /* 窗口过了还没等到充电/加热应答, 不动 */
        printf("wake: uart watch timeout, stay\n");
        return;
    }
    if (lunchbox_charging_now()) {
        wake_watch_tick = 0;
        printf("wake: charging -> black screen\n");
        if (func_cb.sta != FUNC_BLACK_SCREEN) {
            func_cb.sta = FUNC_BLACK_SCREEN;
        }
        return;
    }
    if (lunchbox_heating_task_active()) {
        wake_watch_tick = 0;
        printf("wake: module heating -> heat page\n");
        if (func_cb.sta != FUNC_NEW_HEAT_PAGE) {
            func_cb.sta = FUNC_NEW_HEAT_PAGE;
        }
        return;
    }
    if (tick_check_expire(wake_query_tick, 1000)) {
        wake_query_tick = tick_get();   /* 1s 没等到判据: 查询可能丢了, 再查一把 */
        lb_heat_cmd_status_query();
    }
}

/**
 * 盖盖上电确认 (开盖=整机断电, 盖盖=来电冷启动):
 * func_run 启动段(与"插线进黑屏页"同一位置)等模块首帧 ≤3s, 首帧带着
 * 加热/保温 = 加热中开过盖又盖上 → 暂存参数 + 置 pending;
 * 主页 enter 消费 pending, 把"继续加热?"弹窗直接合成进第一帧 ——
 * 亮屏即主页+弹窗, 无闪切。
 *
 * 弹窗期间**先不停加热**(容错: 用户觉得没盖好, 开盖重盖 → 再次冷启动时
 * 模块还在加热, 弹窗还会再来; 若一进弹窗就停, 重盖后就再也不弹了):
 *   - 挂满 1 分钟没表态 → lb_lid_popup_poll() 补发停止
 *   - NO → lb_lid_confirm_no() 立即停止, 留在主界面
 *   - YES → lb_lid_confirm_yes(): 还没停过 → 模块本来就在跑, 直接按暂存
 *     模式跳页(保温→保温页, 加热→加热页); 已超时停过 → 先 resume
 *     (带总/剩余时长)续跑再跳页
 * 弹窗决策期间路由边沿由 lb_ui_route_apply() 取走丢弃。
 * 深睡唤醒不重启不过 func_run, 不会再弹 (深睡唤醒不是盖盖)。
 */
#define LB_LID_POPUP_STOP_MS    60000   /* 弹窗挂多久没表态就停加热 */

static bool lb_lid_popup_pending;       /* 主页 enter 待弹"继续加热?" */
static u32  lb_lid_popup_tick;          /* 弹窗拉起时刻, 0=弹窗流程不在跑 */
static bool lb_lid_stopped;             /* 已发过停止 (超时/NO) */
static u8   lb_lid_mode;                /* 弹窗暂存: 开盖前的模式/温度 */
static u8   lb_lid_temp;
static u32  lb_lid_duration;            /* 弹窗暂存: 加热总时长(分钟) */
static u32  lb_lid_remain;              /* 弹窗暂存: 剩余时长(分钟) */

/** 主页 enter 取弹窗标记 (一次性); 命中即起 1 分钟表态计时 */
bool lb_lid_popup_pending_take(void)
{
    if (!lb_lid_popup_pending) {
        return false;
    }
    lb_lid_popup_pending = false;
    lb_lid_popup_tick    = tick_get() | 1;
    lb_lid_stopped       = false;
    return true;
}

/** 弹窗挂着时每轮调: 1 分钟没表态才停加热 (只发一次) */
void lb_lid_popup_poll(void)
{
    if (lb_lid_popup_tick == 0 || lb_lid_stopped) {
        return;
    }
    if (tick_check_expire(lb_lid_popup_tick, LB_LID_POPUP_STOP_MS)) {
        lb_lid_stopped = true;
        printf("lid: popup no answer in 1min -> stop heat\n");
        lb_heat_cmd_stop();
    }
}

/** 盖盖弹窗选 YES: 继续加热并按暂存数据跳页 */
void lb_lid_confirm_yes(void)
{
    lb_lid_popup_tick = 0;
    if (lb_lid_mode == LB_MODE_WARM) {
        /* 没停过: 保温页 enter 见模块仍在保温, 不会重发(计时不清零);
         * 超时停过: enter 见模块没在保温, 自动重发 194F/24h */
        func_cb.sta = FUNC_NEW_WARM_PAGE;
    } else if (lb_lid_mode != LB_MODE_OFF) {
        if (lb_lid_stopped) {
            /* 超时停过才需要续跑: 总/剩余时长分开带, 接着开盖前的进度 */
            lb_heat_cmd_resume(lb_lid_mode, lb_lid_temp, lb_lid_duration, lb_lid_remain);
        }
        func_cb.sta = FUNC_NEW_HEAT_PAGE;
    }
}

/** 盖盖弹窗选 NO: 立即停加热, 留在主界面 */
void lb_lid_confirm_no(void)
{
    lb_lid_popup_tick = 0;
    if (!lb_lid_stopped) {
        lb_lid_stopped = true;
        lb_heat_cmd_stop();
    }
}

/** func_run 启动段: 等模块首帧并判定盖盖弹窗 (阻塞 ≤3s, 喂狗) */
static void lb_boot_lid_wait(void)
{
    u32 t0 = tick_get();

    /* valid 在首个 0x01 帧喂进镜像时置位; 模块不在/不答 3s 后照常开机 */
    while (!lb_ui_state_get()->valid && !tick_check_expire(t0, 3000)) {
        WDT_CLR();
        lunchbox_uart_process();        /* 驱动开机时序 power_on → 首帧应答 */
        co_timer_pro(false);
    }

    /* 低电直接读镜像 —— g_ui_sys 要到主循环 lb_ui_sync_pull 才刷 */
    lb_ui_state_t *st = lb_ui_state_get();
    if (!st->valid || !lunchbox_heating_task_active()
        || st->fault == LB_FAULT_LOW_BATTERY) {
        return;                         /* 没在加热/低电 → 正常开机进主页 */
    }
    lb_lid_mode     = st->heat_mode;
    lb_lid_temp     = st->heat_temp;
    lb_lid_duration = st->heat_duration;
    lb_lid_remain   = st->remain_time ? st->remain_time : st->heat_duration;
    printf("boot: module heat/warm (mode=%u dur=%u remain=%u) -> lid popup\n",
           lb_lid_mode, (unsigned)lb_lid_duration, (unsigned)lb_lid_remain);
    lb_lid_popup_pending = true;        /* 主页 enter 把弹窗合成进第一帧;
                                         * 不停加热 —— 表态/超时才停 */
}

static void lb_shutdown_seq_apply(void)
{
    if (!lunchbox_shutdown_done_take()) {       /* 边沿只取一次, 取走即回 IDLE */
        return;
    }
    /* 时序已跑完: 加热停了、模块关了。落地按充电状态分流 ——
     * 充电中 → 黑屏充电页: 与真关机唯一的区别是主板不深睡, 还收串口状态
     *           (模块充电时仍上报 DP4), 屏上跑充电动画。拔线后页内自会真关机。
     * 未充电 → manual_off 深睡 (真关机)。 */
    if (lunchbox_charging_now()) {
        if (func_cb.sta != FUNC_BLACK_SCREEN && !sys_cb.flag_swithing) {
            printf("seq: shutdown done, charging -> black screen charge page\n");
            func_cb.sta = FUNC_BLACK_SCREEN;
        }
        return;
    }
#if ELUNCHBOX_PANEL_EN
    printf("seq: power down -> manual_off deep sleep\n");
    elunchbox_pwr_manual_off_set();
    elunchbox_screen_off();
    elunchbox_guioff_sleep_arm_immediate();     /* 不等 30s 空闲倒计时, 立刻允许深睡 */
#else
    printf("seq: power down -> FUNC_PWROFF (no elunchbox panel)\n");
    func_cb.sta = FUNC_PWROFF;
#endif
}

static void lb_ui_route_apply(void)
{
    /* 关机时序进行中: 不抢页, 也不消费边沿 —— 万一最后一刻被 blocked 拦下没关机,
     * "模块停了→回首页"这个跳页还是该生效的。 */
    if (lunchbox_shutdown_is_active()) {
        return;
    }

    /* 黑屏充电页: 只处理串口数据和开机键, 不被跳页拽走。
     * 边沿不消费 —— APP 这时远程开了加热, 用户按开机键回主界面后立刻路由到加热页 */
    if (func_cb.sta == FUNC_BLACK_SCREEN) {
        return;
    }

#if ELUNCHBOX_PANEL_EN
    /* 已进 manual_off (关机落地了): 不切页, 边沿取走丢弃。
     *
     * 必须丢弃: 关机时序第一步发的停止加热会让模块上报 HeatEn=OFF, 产生一个
     * "模块停了→回首页"边沿。这个边沿在 is_active() 期间被挂着, 断电那一刻
     * 状态回 IDLE, 下一句就把它放出来 —— 于是在关屏/存 keep_ram/gpu_exit 的
     * 同时重建首页 form 和锁屏浮层, 醒来渲染失效控件 → resource halt C245。 */
    if (elunchbox_pwr_is_manual_off()) {
        (void)lb_ui_route_poll();
        return;
    }
#endif

    /* 盖盖开机"继续加热?"弹窗决策期间: 边沿取走丢弃 ——
     * 去加热页还是停加热由弹窗结果定 (func_home_page.c), 不能让边沿抢先跳页 */
    if (g_ui_sys.lid_open || func_confirm_overlay_visible()) {
        (void)lb_ui_route_poll();
        return;
    }

    /* 切换动画进行中 / OTA 进行中: 不抢页, 也不消费边沿, 下一轮再来 */
    if (sys_cb.flag_swithing || lb_ota_is_active()) {
        return;
    }

    switch (lb_ui_route_poll()) {
    case LB_UI_ROUTE_HEAT:
        if (func_cb.sta != FUNC_NEW_HEAT_PAGE) {
            printf("route: module heating -> heat page\n");
            func_cb.sta = FUNC_NEW_HEAT_PAGE;
        }
        break;

    case LB_UI_ROUTE_WARM:
        if (func_cb.sta != FUNC_NEW_WARM_PAGE) {
            printf("route: module keep-warm -> warm page\n");
            func_cb.sta = FUNC_NEW_WARM_PAGE;
        }
        break;

    case LB_UI_ROUTE_HOME:
        /* 只把加热/保温页拉回首页, 用户正在别的页面时不打扰 */
        if (func_cb.sta == FUNC_NEW_HEAT_PAGE || func_cb.sta == FUNC_NEW_WARM_PAGE) {
            printf("route: module stopped -> home\n");
            func_cb.sta = FUNC_HOME_PAGE;
        }
        break;

    default:
        break;
    }
}
#endif // FUNC_LUNCHBOX_UART_EN

AT(.text.func.process)
void func_process(void)
{
#if ELUNCHBOX_PANEL_EN
    bool guioff = elunchbox_is_guioff();
#else
    bool guioff = sys_cb.gui_sleep_sta;
#endif

    if (gui_get_auto_power_en() && !guioff) {
        sys_clk_req(INDEX_GUI, SYS_192M);
    }

    WDT_CLR();

#if USER_PT8028_KEY
    pt8028_gpio_ensure_periodic();
    pt8028_key_scan();
    pt8028_log_flush();
    pt8028_poll_reinit();
#if PT8028_GPIO_MONITOR_EN
    pt8028_gpio_monitor();
#endif
#endif

#if (UART0_PRINTF_SEL != PRINTF_NONE)
    uart0_printf_ensure();
#endif

#if USER_PANEL_LED
    func_led_scan();
#endif

    /* ======== 空闲到期 ======== */
#if ELUNCHBOX_PANEL_EN && !FUNC_LUNCHBOX_UART_EN
    /* 无串口功能: 维持旧行为, 空闲只息屏 */
    if (!guioff && elunchbox_guioff_idle_expired()) {
        elunchbox_screen_off();
        guioff = true;
    }
#endif
    /* FUNC_LUNCHBOX_UART_EN=1 时空闲处理只在 sleep_process (func_lowpwr.c):
     * 5min 无操作 = 自动关机走关机时序 → manual_off 深睡。
     * 这里原来有一份"到期即息屏"(合并带入), 每轮跑在 sleep_process 之前,
     * 把机器抢进 auto-guioff 深睡 —— 那条路不关 RTC WDT, 睡下即被 RTC_WDT
     * 硬复位重启, 表现为"定时进低功耗后自己起来跑、功耗不对"。已删。 */

    /* ======== 息屏路径 ======== */
#if ELUNCHBOX_PANEL_EN
    if (guioff) {
        /* 空闲处理: 深睡由 sleep_process 统一入口,
         * 不在此阻塞, 唤醒后 sleep_process 返回 true */
        co_timer_pro(false);
        WDT_CLR();
    }
    else
#endif
    {
#if CPU_USAGE_MONITOT_EN
        cpu_trace_monitor();
#endif

#if (SD_SUPPORT_EN) && SD_SOFT_DETECT_EN
        sd_soft_cmd_detect(120);
#endif

        tft_bglight_frist_set_check();
    }

#if FUNC_LUNCHBOX_UART_EN
    /* 加热模块串口: 取字节 → 拼帧 → lb_uart_on_frame 分发。
     * 内部还驱动: 桥接应答超时重试 / 预约列表同步 / 开机关机时序 / 加热模块 OTA。
     * 息屏(guioff)时照常跑 —— 模块的心跳要回、预约到点要收。 */
    lunchbox_uart_process();

    lb_ui_sync_pull();                  /* 串口状态镜像 → g_ui_sys, 须在刷 UI 之前 */
    lb_wake_apply();                    /* 深睡唤醒原因 → 落页 (按键/充电/加热) */
    lb_shutdown_seq_apply();            /* 关机时序走完 → 主板断电 */
    lb_ui_route_apply();                /* 模块状态变化 → 强制切页 */
#endif

    /* GUI 更新 (不休眠时才更新) */
    if (!guioff && !sys_cb.flag_halt) {
#if ELUNCHBOX_PANEL_EN
        bool gui_do_refresh = !sys_cb.flag_swithing;
        compo_update();
        if (gui_do_refresh) {
            /* TE帧门控: 对齐TE推屏时隙消除切图撕裂 */
            if (tft_te_frame_gate()) {
                gui_process();
            }
        }
#if USER_PT8028_KEY && FUNC_RESERVATION_UI_EN
        func_elunchbox_res_key_poll();
#endif
#else
        compo_update();
        if (tft_te_frame_gate()) {
            gui_process();
        }
#endif
    }

    co_timer_pro(false);
    bsp_sensor_step_pro_isr();

    if (sys_cb.mp3_res_playing) {
        mp3_res_process();
    }

    if (sleep_process(bt_is_allow_sleep)) {
        bt_cb.disp_status = 0xff;
#if ELUNCHBOX_PANEL_EN
        if (elunchbox_pwr_gui_off_is_on()) {
            elunchbox_pwr_gui_wake();
        }
#endif
    }
#if ELUNCHBOX_PANEL_EN
    /* guioff 唤醒: gui_wakeup → lunchbox_display_on 顺序保证不花屏 */
    if (sys_cb.gui_sleep_sta && !elunchbox_pwr_gui_off_is_on()) {
        gui_wakeup();
        lunchbox_display_on();
    }
#endif

#if VBAT_DETECT_EN
    bsp_vbat_lpwr_process();
#endif

    if ((PWRKEY_2_HW_PWRON) && (sys_cb.pwrdwn_hw_flag)) {
        func_cb.sta = FUNC_PWROFF;
        sys_cb.pwrdwn_hw_flag = 0;
    }

#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_detect(1);
    }
#endif

    if (bt_cb.bt_is_inited) {
        bt_thread_check_trigger();
#if LE_EN
#if FUNC_LUNCHBOX_UART_EN
        /* 饭盒协议接收/分发在 functions/comm/lb_ble_app.c,
         * 不走 ble_app_process() → ble_app_watch_process() 那条老链路 */
        lunchbox_ble_process();
#else
        ble_app_process();
#endif
#endif
#if FUNC_LUNCHBOX_UART_EN
        lb_ota_process();               /* 主MCU OTA 状态机 */
#endif
    }

    if (g_ui_sys.lowbat && func_cb.sta != FUNC_LOWBAT && !sys_cb.flag_swithing) {
        func_cb.sta = FUNC_LOWBAT;
    }

    if (gui_get_auto_power_en() && !guioff) {
        sys_clk_free(INDEX_GUI);
    }
}

//根据任务名创建窗体。此处调用的创建窗体函数不要调用子任务的控制结构体
compo_form_t *func_create_form(u8 sta)
{
    compo_form_t *frm = NULL;
    compo_form_t *(*func_create)(void) = NULL;
    for (int i = 0; i < FUNC_CREATE_CNT; i++) {
        if (tbl_func_create[i].func_idx == sta) {
            func_create = tbl_func_create[i].func;
            if (func_create != NULL) {
                frm = func_create();
            }
            break;
        }
    }
    if (frm == NULL) {
        halt(HALT_FUNC_SORT);
    }
    return frm;

}

//获取当前任务顺序
static int func_get_order(u8 sta)
{
    int i;
    for (i=0; i<func_cb.sort_cnt; i++) {
        if (sta == func_cb.tbl_sort[i]) {
            return i;
        }
    }
    return -1;
}

//执行当前任务退出
void func_cur_sta_exit(void)
{
    void (*exit)(void) = NULL;
    for (int i = 0; i < FUNC_EXIT_CNT; i++) {
        if (tbl_func_exit[i].func_idx == func_cb.sta) {
            exit = tbl_func_exit[i].func;
            exit();
            break;
        }
    }
}


//切换到上一个任务
void func_switch_prev(bool flag_auto)
{
    compo_form_t *frm = NULL;
    u8 mode = func_get_switching_mode_byidx(sys_cb.nav_index, false);
    u16 switch_mode = flag_auto ? (mode | FUNC_SWITCH_AUTO) : mode;
    s8 idx = func_get_order(func_cb.sta);

    u8 sta;
    if (idx <= 0) {
        sta = task_stack_pop();
    } else {
        sta = func_cb.tbl_sort[idx - 1];
    }
#if !FUNC_RESERVATION_UI_EN
    if (sta == FUNC_RESERVATION) {
        return;
    }
#endif
    if (mode != FUNC_SWITCH_FADE_OUT) {
        frm = func_create_form(sta);                                  //创建下一个任务的窗体
    }

    bool res = func_switching(switch_mode, NULL);                     //切换动画

    if (frm) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }

    if (res) {
        if (sta == FUNC_CLOCK) {
            func_cb.flag_sort = false;
        }
        func_cb.sta = sta;
    }
}

//切换到下一个任务
void func_switch_next(bool flag_auto, bool flag_loop)
{
    compo_form_t *frm = NULL;
    u8 sta;
    u8 mode = func_get_switching_mode_byidx(sys_cb.nav_index, true);
    u16 switch_mode = flag_auto ? (mode | FUNC_SWITCH_AUTO) : mode;
    u8 idx = func_get_order(func_cb.sta);
    if (idx < 0) {
        return ;
    }

    if (idx >= func_cb.sort_cnt - 1) {
        if (flag_loop) {
            sta = func_cb.tbl_sort[0];
        } else {
            return ;
        }
    } else {
        sta = func_cb.tbl_sort[idx + 1];
    }

#if !FUNC_RESERVATION_UI_EN
    if (sta == FUNC_RESERVATION) {
        return;
    }
#endif

    if (mode != FUNC_SWITCH_FADE_OUT) {
        frm = func_create_form(sta);                                  //创建下一个任务的窗体
    }

    bool res = func_switching(switch_mode, NULL);                     //切换动画

    if (frm) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }

    if (res) {
        if (sta == FUNC_CLOCK) {
            func_cb.flag_sort = false;
        } else {
            func_cb.flag_sort = true;
        }
        func_cb.sta = sta;
    }
}


//切换
void func_switch_to(u8 sta, u16 switch_mode)
{
#if !FUNC_RESERVATION_UI_EN
    if (sta == FUNC_RESERVATION) {
        return;
    }
#endif
#if ELUNCHBOX_PANEL_EN
#if FUNC_RESERVATION_UI_EN
    if (sta == FUNC_RESERVATION && !func_res_allow_switch) {
        return;
    }
#endif
    if (sys_cb.flag_swithing) {
        return;
    }
    home_gpu_wait_idle();
#endif

    compo_form_t *frm = NULL;

    u8 mode = switch_mode & 0x7fff;
    if (mode != FUNC_SWITCH_FADE_OUT) {
        frm = func_create_form(sta);                                  //创建下一个任务的窗体
    }

    bool res = func_switching(switch_mode, NULL);         			  //切换动画

#if ELUNCHBOX_PANEL_EN
    if (res) {
        home_gpu_wait_idle();
    }
#endif

    if (frm) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }

    if (res) {
        func_cb.sta = sta;
    }
}

/* 手表表盘/菜单切换已移除：饭盒统一回 Home */
void func_switch_to_clock(void)
{
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
    func_cb.flag_sort = false;
}

void func_switch_to_menu(void)
{
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

void func_switch_to_football_menu(void)
{
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

void func_switching_to_menu(void)
{
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

//页面滑动回退功能
void func_backing_to(void)
{
    if (func_cb.sta == FUNC_BT_RING || func_cb.sta == FUNC_BT_CALL) {
        return;
    }

    u8 stack_top = task_stack_pop();

    if (!stack_top || stack_top == FUNC_MENU || stack_top == FUNC_SMARTSTACK) {
        stack_top = FUNC_HOME;
    }

    func_switch_to(stack_top, func_get_switching_mode_byidx(sys_cb.nav_index, false));
    if (func_cb.sta != stack_top) {
        task_stack_push(func_cb.sta);
    }
}

//页面按键回退功能
void func_back_to(void)
{
    if (func_cb.sta == FUNC_BT_RING || func_cb.sta == FUNC_BT_CALL) {
        return;
    }

    u8 stack_top = task_stack_pop();

    if (!stack_top || stack_top == FUNC_MENU || stack_top == FUNC_SMARTSTACK) {
        stack_top = FUNC_HOME;
    }

    func_switch_to(stack_top, func_get_switching_mode_byidx(sys_cb.nav_index, false) | FUNC_SWITCH_AUTO);
}

//页面直接回退,无动画效果
u8 func_directly_back_to(void)
{
    u8 stack_top = task_stack_pop();

    func_cb.sta = stack_top;

    return stack_top;
}

//static FATFS fat_fs;

void evt_message(size_msg_t msg)
{
    switch (msg) {
        case EVT_READY_EXCHANGE_MTU:
            ble_exchange_mtu_request();
            break;

#if BT_CONNECT_REQ_FROM_WATCH_EN
        case EVT_BT_CONNECT_ONCE:
            bt_connect();
            break;
#endif

#if MUSIC_UDISK_EN
        case EVT_UDISK_INSERT:
            if (dev_is_online(DEV_UDISK)) {
                if (dev_udisk_activation_try(1)) {
                    sys_cb.cur_dev = DEV_UDISK;
                    func_cb.sta = FUNC_MUSIC;
                }
            }
            break;
#endif // MUSIC_UDISK_EN

#if SD_SUPPORT_EN
        case EVT_SD_INSERT:
            if (dev_is_online(DEV_SDCARD)) {
                sys_cb.cur_dev = DEV_SDCARD;
                printf("EVT_SD_INSERT\n");
#if USB_SD_UPDATE_EN
                bsp_sd_disk_mount(24);
                func_update();                                  //尝试升级
#endif // USB_SD_UPDATE_EN

                msg_enqueue(EVT_SD_CARD_INTER);
            }
            break;

        case EVT_SD_REMOVE:
            if (!dev_is_online(DEV_SDCARD)) {
                printf("EVT_SD_REMOVE\n");

//                FRESULT res = fs_unmount ("B:");
//                printf("fs_unmount:%d\n", res);

                msg_enqueue(EVT_SD_CARD_OUT);
            }
            break;
#endif // SD_SUPPORT_EN

#if FUNC_USBDEV_EN
        case EVT_PC_INSERT:
            if (dev_is_online(DEV_USBPC)) {
                func_cb.sta = FUNC_USBDEV;
            }
            break;
#endif // FUNC_USBDEV_EN

        case EVT_HFP_SET_VOL:
            if(sys_cb.incall_flag & INCALL_FLAG_SCO){
                bsp_change_volume(bsp_bt_get_hfp_vol(sys_cb.hfp_vol));
                printf("HFP SET VOL: %d\n", sys_cb.hfp_vol);
            }
            break;

        case EVT_A2DP_SET_VOL:
            if ((sys_cb.incall_flag & INCALL_FLAG_SCO) == 0) {
                printf("A2DP SET VOL: %d\n", sys_cb.vol);
                bsp_change_volume(sys_cb.vol);
                param_sys_vol_write();
                sys_cb.cm_times = 0;
                sys_cb.cm_vol_change = 1;
            }
            break;

        case EVT_A2DP_MUSIC_PLAY:
            if (func_cb.sta == FUNC_VIDEO_PLAY) {
                bt_audio_bypass();
            } else {
                if (!sbc_is_bypass()) {
                    printf("EVT_A2DP_MUSIC_PLAY\n");
                    bsp_sys_unmute();
                }
                bt_cb.music_playing = true;
            }
            break;

        case EVT_A2DP_MUSIC_STOP:
            if (!sbc_is_bypass()) {
                printf("EVT_A2DP_MUSIC_STOP\n");
                bsp_sys_mute();
            }
            bt_cb.music_playing = false;
            break;

        case EVT_BT_SCAN_START:
            if (bt_get_status() < BT_STA_SCANNING) {
                bt_scan_enable();
            }
            break;
#if EQ_DBG_IN_UART || EQ_DBG_IN_SPP
        case EVT_ONLINE_SET_EQ:
            eq_parse_cmd();
            break;
#endif

#if EQ_MODE_EN
        case EVT_BT_SET_EQ:
            music_set_eq_by_num(sys_cb.eq_mode);
            break;
#endif

#if BT_SCO_APP_DBG_EN
        case EVT_SCO_DBG_COMM_RSP:
        case EVT_SCO_DBG_TLV_RSP:
        case EVT_SCO_DBG_NOTIFY:
            sco_app_msg_deal(msg);
            break;
#endif // BT_SCO_APP_DBG_EN

        case EVT_HALT:
            func_cb.sta = FUNC_BT_UPDATE;		//出现异常后要处理事情
            break;

#if FUNC_REC_AUTO_EN
        case EVT_VOX_RECORD_EN: {
            u8 sta = get_bsp_record_sta();
//            printf("EVT_VOX_RECORD_EN: %d\n", sta);
            if (sta == REC_PAUSE) {
                bsp_record_continue();
            } else if (sta == REC_STOP) {
                bsp_record_start(false, bsp_record_get_type(), FUNC_REC_SPR, FUNC_REC_BITRATE);
            }
        } break;

        case EVT_VOX_RECORD_DIS: {
            u8 sta = get_bsp_record_sta();
//            printf("EVT_VOX_RECORD_DIS\n");
            if (sta == REC_RECORDING) {
                bsp_record_pause();
            }
        } break;
#endif // FUNC_REC_AUTO_EN
    }
}

//func common message process
void func_message(size_msg_t msg)
{
#if ELUNCHBOX_PANEL_EN
    /* new_ui pages handle navigation via func_key, not old-style msg queue */
    if (func_cb.sta == FUNC_HOME || func_cb.sta == FUNC_HOME_PAGE
        || func_cb.sta >= FUNC_NEW_HEAT_SET) {
        switch (msg) {
        case MSG_CTP_SHORT_LEFT:
        case MSG_CTP_SHORT_RIGHT:
        case KU_BACK:
        case KU_PREV:
        case MSG_QDEC_FORWARD:
        case MSG_QDEC_BACKWARD:
            return;
        default:
            break;
        }
    }
#endif
    switch (msg) {
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_QDEC_FORWARD:
    case MSG_QDEC_BACKWARD:
        /* 手表触控/编码器切任务已移除 */
        break;

    case MSG_CTP_COVER:
#if !ELUNCHBOX_KEEP_AWAKE
        sys_cb.sleep_delay = 1; //100ms后进入休眠
#endif
        break;

    case KU_PREV:
        if (func_cb.sta != FUNC_HEAT) {
            func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
        break;

    case KU_NEXT:
        /* 饭盒：预约 UI 由 pt8028_take_res_key_pending 专用入口进入 */
        break;

    case KU_BACK:
        if (func_cb.sta == FUNC_HOME) {
            break;
        }
        func_back_to();
        break;

#if SOFT_POWER_ON_OFF
        case KLH_BACK:
        case KLH_LEFT:
        case KLH_RIGHT:
            printf("KLH: shutdown -> FUNC_PWROFF\n");
            func_cb.sta = FUNC_PWROFF;
            break;
#endif

    case KU_MODE:
            if (func_cb.sta == FUNC_HOME) {
                break;
            }
            if (func_cb.sta != FUNC_MODE &&
                       func_cb.sta != FUNC_SETUP && func_cb.sta != FUNC_RESERVATION &&
                       func_cb.sta != FUNC_TIMEING && func_cb.sta != FUNC_LANGUAGEING &&
                       func_cb.sta != FUNC_VERINFO) {
                func_cb.sta = FUNC_NULL;
            }
            break;

        case MSG_SYS_500MS:
            break;

        case MSG_SYS_1S:
#if BT_HFP_BAT_REPORT_EN
            bt_hfp_report_bat();
#endif
            break;

        default:
            evt_message(msg);
            break;
    }

    //调节音量，3秒后写入flash
    if ((sys_cb.cm_vol_change) && (sys_cb.cm_times >= 6)) {
        sys_cb.cm_vol_change = 0;
        cm_sync();
    }
}

///进入一个功能的总入口
extern u8 heap_func[HEAP_FUNC_SIZE] AT(.heap.func);

AT(.text.func)
void func_enter(void)
{
    //检查Func Heap
    u32 heap_size = func_heap_get_free_size();
    if (heap_size != HEAP_FUNC_SIZE) {
        printf("Func heap leak (%d -> %d): %d\n", func_cb.last, func_cb.sta, heap_size);
        printf("To Reset Func heap:%d\n", HEAP_FUNC_SIZE);
        func_heap_init(heap_func, HEAP_FUNC_SIZE);
//        halt(HALT_FUNC_HEAP);
    }

//    gui_box_clear();
    param_sync();
    reset_sleep_delay_all();
    reset_pwroff_delay();
    func_cb.mp3_res_play = mp3_res_play;
    func_cb.set_vol_callback = NULL;
//    bsp_clr_mute_sta();
//    sys_cb.voice_evt_brk_en = 1;    //播放提示音时，快速响应事件。
    AMPLIFIER_SEL_D();
}

AT(.text.func)
void func_exit(void)
{
    //销毁窗体
    if (func_cb.frm_main != NULL) {
#if ELUNCHBOX_PANEL_EN
        func_key_lock_on_form_destroy(); /* 先清锁 overlay，避免悬空 set_ram → C429 */
        home_gpu_wait_idle();
#endif
        compo_form_destroy(func_cb.frm_main);
    }
    //释放FUNC控制结构体
    if (func_cb.f_cb != NULL) {
        func_free(func_cb.f_cb);
    }
    func_cb.frm_main = NULL;
    func_cb.f_cb = NULL;
}

AT(.text.func)
void func_run(void)
{
#if CPU_USAGE_MONITOT_EN
    cpu_trace_monitor_init();
#endif

    void (*func_entry)(void) = NULL;
    printf("%s\n", __func__);

    /* 低功耗初始化 */
    sys_cb.sleep_en     = 1;
    sys_cb.sleep_time   = -1L;
    sys_cb.sleep_delay  = -1L;
    sys_cb.guioff_delay = -1L;
    sys_cb.pwroff_time  = -1L;
    sys_cb.pwroff_delay = -1L;
#if ELUNCHBOX_PANEL_EN
    elunchbox_user_activity_reset();
#endif

    memset(func_cb.tbl_sort, 0, sizeof(func_cb.tbl_sort));
    func_cb.tbl_sort[0] = FUNC_HOME;
    func_cb.sort_cnt = 1;
    func_cb.flag_sort = false;
    func_cb.sta = FUNC_HOME;
#if ELUNCHBOX_PANEL_EN && CHARGE_EN
    /* 冷启动(装电池/复位)时就插着充电线 → 进黑屏充电页, 不进主界面。
     * 注: 深睡中插线不走这里 —— manual_off 唤醒不重启, 深睡循环置 UART
     * 闩锁, 醒后 lb_wake_apply() 观察窗判充电 → 黑屏页。
     * 撤销上电自动 power_on: 黑屏页语义是"关机+充电", 模块保持关,
     * 长按开机键时再 lunchbox_boot_seq_kick() 补跑。 */
    if (CHARGE_DC_IN()) {
        printf("func_run: DC in at boot -> black screen charge page\n");
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_boot_seq_cancel();
#endif
        func_cb.sta = FUNC_BLACK_SCREEN;
    }
#if FUNC_LUNCHBOX_UART_EN
    else {
        /* 盖盖上电: 进页面前先把模块首帧等到手(≤3s), 加热中开过盖 →
         * 停加热+置 pending, 主页 enter 把"继续加热?"弹窗合成进第一帧 */
        lb_boot_lid_wait();
    }
#endif
#endif
    task_stack_init();  //任务堆栈
    latest_task_init(); //最近任务

    for (;;) {
        func_enter();
        for (int i = 0; i < FUNC_ENTRY_CNT; i++) {
            if (tbl_func_entry[i].func_idx == func_cb.sta) {
                task_stack_push(func_cb.sta);
                latest_task_add(func_cb.sta);
                func_entry = tbl_func_entry[i].func;
                func_entry();
                break;
            }
        }
        if (func_cb.sta == FUNC_PWROFF) {
            printf("func_run: -> func_pwroff\n");
            func_pwroff(1);
        }
        func_exit();
    }
}
