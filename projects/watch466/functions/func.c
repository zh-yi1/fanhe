#include "include.h"
#include "func_menu.h"
#include "func_tbl.h"
#include "func.h"
#include "func_reservation.h"
#include "func_new_home.h"
#include "heat_display_reg.h"
#include "func_lunchbox_off.h"
#include "func_lunchbox_ota.h"
#include "func_lowbat.h"
#include "func_lunchbox_wake.h"
#include "func_lowpwr.h"
#include "func_lid_confirm.h"
#if ELUNCHBOX_PANEL_EN
#include "home_ui_shared.h"
#include "home_ui_lowbat_overlay.h"
#include "func_lunchbox_uart.h"
/* ELUNCHBOX 模式：TE block 标志声明 */
extern volatile u8 elunchbox_te_block_flag;
#endif
#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif
#if USER_PANEL_LED
#include "port_panel_led.h"
#endif

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

bool func_music_is_play(void);
void func_music_play(bool sta);
void func_call_mgr_process(void);
u8 func_menu_sub_skyrer_get_first_idx(void);
compo_form_t *func_clock_form_create_by_screenshoot(void);

func_cb_t func_cb AT(.buf.func_cb);

#if BT_BACKSTAGE_EN
AT(.text.func.process)
void func_watch_bt_process(void)
{
    uint disp_status = bsp_bt_disp_status();

    if (disp_status == BT_STA_OTA) {
        sfunc_bt_ota();
    }
#if !CALL_MGR_EN
    else if (sys_cb.reject_tick) {
        if (tick_check_expire(sys_cb.reject_tick, 3000) || disp_status < BT_STA_INCOMING) {
            sys_cb.reject_tick = 0;
        }
#if BT_VOIP_REJECT_EN
    } else if (bt_cb.disp_status == BT_STA_INCOMING && bt_cb.call_type == CALL_TYPE_PHONE) {
#else
    } else if (bt_cb.disp_status == BT_STA_INCOMING) {
#endif
        func_cb.sta = FUNC_BT_RING;

#if BT_VOIP_REJECT_EN
    } else if (bt_cb.disp_status >= BT_STA_OUTGOING && bt_cb.call_type == CALL_TYPE_PHONE) {
#else
    } else if (bt_cb.disp_status >= BT_STA_OUTGOING) {
#endif
        func_cb.sta = FUNC_BT_CALL;
    }
#endif
}
#endif // BT_BACKSTAGE_EN
bool gui_get_auto_power_en(void);

#if ELUNCHBOX_PANEL_EN
static bool func_elunchbox_res_key_page_ok(void)
{
    switch (func_cb.sta) {
    case FUNC_HOME:
    case FUNC_MODE:
    case FUNC_SETUP:
    case FUNC_HEAT:
    case FUNC_LANGUAGEING:
    case FUNC_TIMEING:
    case FUNC_VERINFO:
    case FUNC_NEW_HEAT:
    case FUNC_NEW_MODE:
    case FUNC_NEW_SETUP:
    case FUNC_NEW_WARM:
    case FUNC_LOWBAT:
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
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif
    /* 与加热/模式/设置子页一致：FADE_OUT 切页 + func_exit 统一销毁源 frm。
     * 勿在 func_home_process 内抢先 destroy/free（易 use-after-free / 长时间阻塞触发 WDT）。
     */
    home_gpu_wait_idle();
    WDT_CLR();
    func_res_allow_switch = 1;
    /* 与模式页进预约一致：DIRECT 避免 FADE_OUT 在 frm 已销毁时 compo_form_set_alpha → 5142 */
    func_switch_to(FUNC_RESERVATION, FUNC_SWITCH_DIRECT | FUNC_SWITCH_AUTO);
    func_res_allow_switch = 0;
}

void func_elunchbox_switch_to_heat(void)
{
#if ELUNCHBOX_PANEL_EN
    if (func_cb.sta == FUNC_NEW_HEAT || func_cb.sta == FUNC_HEAT) {
        return;
    }
#else
    if (func_cb.sta == FUNC_HEAT) {
        return;
    }
#endif
    if (sys_cb.flag_swithing) {
        return;
    }
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif
    home_gpu_wait_idle();
    WDT_CLR();
#if ELUNCHBOX_PANEL_EN
    func_switch_to(FUNC_NEW_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
#else
    func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
#endif
}

#if ELUNCHBOX_PANEL_EN
static bool elunchbox_pwr_intentional_wake; /* manual_off 下允许 gui_wakeup（前置声明） */
static u8   elunchbox_ble_pending_sta;      /* BLE 触发的延后切页（避免 flag_swithing 时丢失） */
static u8   elunchbox_ble_pending_home;     /* BLE 停止加热：延后回 Home */
static bool elunchbox_warm_from_charging;   /* 因充电进入保温（非加热自然结束） */

void func_elunchbox_switch_to_home(void)
{
#if ELUNCHBOX_PANEL_EN
    if (elunchbox_lowbat_active() || elunchbox_lowbat_should_block_ui_route()) {
        printf("elunchbox: switch home blocked (lowbat sta=%u)\n", func_cb.sta);
        return;
    }
    if (elunchbox_charge_off_active() && home_ui_shared_battery_is_charging()) {
        printf("elunchbox: switch home blocked (charge off page)\n");
        return;
    }
    if (func_cb.sta == FUNC_HOME) {
        return;
    }
    if (sys_cb.flag_swithing) {
        elunchbox_ble_pending_home = 1;
        return;
    }
    elunchbox_ble_pending_sta = 0;
    elunchbox_ble_pending_home = 0;
#if USER_PT8028_KEY
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif
    if (func_cb.sta == FUNC_HEAT) {
        func_heat_prepare_ble_stop();
    }
    home_gpu_wait_idle();
    WDT_CLR();
    printf("elunchbox: switch to home from sta=%u\n", func_cb.sta);
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
#endif
}

void func_elunchbox_ble_cancel_pending_switch(void)
{
#if ELUNCHBOX_PANEL_EN
    elunchbox_ble_pending_sta = 0;
    elunchbox_ble_pending_home = 0;
#endif
}

void func_elunchbox_ble_pending_set(u8 sta)
{
#if ELUNCHBOX_PANEL_EN
    elunchbox_ble_pending_sta = sta;
#endif
}

void func_elunchbox_uart_stop_and_home(void)
{
#if ELUNCHBOX_PANEL_EN
    if (elunchbox_lowbat_should_block_ui_route()) {
        printf("[LCD_ROUTE] MCU stop ignored: lowbat active/pending (sta=%u)\n", func_cb.sta);
        return;
    }
    printf("[LCD_ROUTE] MCU stop heat -> home (sta=%u)\n", func_cb.sta);
    func_elunchbox_warm_from_charging_set(false);
    lb_heat_mcu_nav_set(false);
    func_elunchbox_ble_cancel_pending_switch();
#if FUNC_LUNCHBOX_UART_EN
    heat_display_unregister();
    lunchbox_heat_clear_local();
#endif
    if (func_cb.sta == FUNC_HEAT) {
        func_heat_prepare_ble_stop();
    }
    if (func_cb.sta != FUNC_HOME) {
        func_elunchbox_switch_to_home();
    }
#endif
}

static void elunchbox_ble_pending_sta_poll(void)
{
    u8 sta;

    if (elunchbox_ble_pending_home) {
        if (sys_cb.flag_swithing) {
            return;
        }
        elunchbox_ble_pending_home = 0;
        func_elunchbox_switch_to_home();
        return;
    }
    if (elunchbox_ble_pending_sta == 0) {
        return;
    }
    if (sys_cb.flag_swithing) {
        return;
    }
    sta = elunchbox_ble_pending_sta;
    elunchbox_ble_pending_sta = 0;
    if (sta == func_cb.sta) {
        if (sta == FUNC_HEAT) {
            func_heat_ble_remote_restart();
        } else if (sta == FUNC_NEW_WARM) {
            func_new_warm_ble_restart();
        }
        return;
    }
    printf("elunchbox: deferred switch sta=%u from=%u\n", sta, func_cb.sta);
#if USER_PT8028_KEY
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif
    home_gpu_wait_idle();
    WDT_CLR();
    func_switch_to(sta, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
    if (sta == FUNC_HEAT) {
        home_gpu_wait_idle();
        WDT_CLR();
    }
}
#endif

void func_elunchbox_switch_to_heat_panel(void)
{
#if ELUNCHBOX_PANEL_EN
    /* 已进入加热流程：开机充电窗口失效，插电应走保温而非黑屏跑马灯 */
    elunchbox_boot_charge_check_clear();
    if (elunchbox_lid_confirm_try_redirect()) {
        printf("elunchbox: heat panel redirected to lid confirm\n");
        return;
    }
    if (!elunchbox_pwr_is_manual_off()
        && (elunchbox_pwr_gui_off_is_on() || sys_cb.gui_sleep_sta)) {
        elunchbox_pwr_intentional_wake = elunchbox_pwr_is_manual_off();
        elunchbox_pwr_gui_wake_reason("ble heat start");
        elunchbox_pwr_intentional_wake = false;
    }
    if (func_cb.sta == FUNC_HEAT && func_cb.f_cb != NULL) {
        func_heat_ble_remote_restart();
        return;
    }
    elunchbox_ble_pending_sta = FUNC_HEAT;
    printf("elunchbox: heat panel pending (cur_sta=%u switching=%u)\n",
           func_cb.sta, sys_cb.flag_swithing ? 1u : 0u);
#endif
}

void func_elunchbox_switch_to_warm_panel(void)
{
#if ELUNCHBOX_PANEL_EN
    if (!elunchbox_pwr_is_manual_off()
        && (elunchbox_pwr_gui_off_is_on() || sys_cb.gui_sleep_sta)) {
        elunchbox_pwr_intentional_wake = elunchbox_pwr_is_manual_off();
        elunchbox_pwr_gui_wake_reason("ble warm start");
        elunchbox_pwr_intentional_wake = false;
    }
    if (func_cb.sta == FUNC_NEW_WARM && func_cb.f_cb != NULL) {
        func_new_warm_ble_restart();
        return;
    }
    if (func_cb.sta == FUNC_HEAT && func_cb.f_cb != NULL) {
        func_elunchbox_enter_warm_from_heat();
        return;
    }
    elunchbox_ble_pending_sta = FUNC_NEW_WARM;
    printf("elunchbox: warm panel pending (cur_sta=%u switching=%u)\n",
           func_cb.sta, sys_cb.flag_swithing ? 1u : 0u);
#endif
}

bool func_elunchbox_charging_redirect_warm(void)
{
#if ELUNCHBOX_PANEL_EN
    /* 充电中界面跳转只跟 UART 上报，本地不再主动切保温 */
    if (home_ui_shared_battery_is_charging() && func_cb.sta == FUNC_NEW_WARM) {
        home_ui_shared_battery_icon_refresh();
    }
#endif
    return false;
}

bool func_elunchbox_warm_from_charging(void)
{
#if ELUNCHBOX_PANEL_EN
    return elunchbox_warm_from_charging;
#else
    return false;
#endif
}

void func_elunchbox_warm_from_charging_set(bool on)
{
#if ELUNCHBOX_PANEL_EN
    elunchbox_warm_from_charging = on;
#endif
}

#if ELUNCHBOX_PANEL_EN
static void elunchbox_subpage_gpu_recycle_after_leave(void)
{
    WDT_CLR();
    if (func_cb.frm_main != NULL) {
        home_gpu_wait_idle();
        WDT_CLR();
        compo_form_destroy(func_cb.frm_main);
        home_gpu_wait_idle();
        WDT_CLR();
        func_cb.frm_main = NULL;
    }
    compos_init();
    home_gpu_wait_idle();
    WDT_CLR();
}

static bool elunchbox_subpage_sta(u8 sta)
{
    return sta == FUNC_NEW_HEAT || sta == FUNC_NEW_WARM || sta == FUNC_NEW_MODE
        || sta == FUNC_NEW_SETUP || sta == FUNC_NEW_LANGUAGE || sta == FUNC_NEW_VERINFO
        || sta == FUNC_NEW_TIME || sta == FUNC_LID_CONFIRM || sta == FUNC_RESERVATION;
}
#endif

void func_elunchbox_res_key_poll(void)
{
#if USER_PT8028_KEY && FUNC_RESERVATION_UI_EN
    if (sys_cb.flag_swithing) {
        return;
    }
#if ELUNCHBOX_PANEL_EN
    if (func_cb.sta == FUNC_RESERVATION && !func_new_reservation_key_ready()) {
        return;
    }
#endif
    if (pt8028_take_res_key_pending()) {
        if (func_key_lock_is_active()) {
            return;
        }
        if (func_elunchbox_res_key_page_ok()) {
            func_elunchbox_switch_to_reservation();
        }
    }
#endif
}
#endif /* ELUNCHBOX_PANEL_EN */

#if USER_PT8028_KEY && SOFT_POWER_ON_OFF
#if ELUNCHBOX_PANEL_EN
static bool elunchbox_pwr_gui_off;
static bool elunchbox_pwr_manual_off;   /* 长按3s手动关机：浅睡态，长按再开 */
static bool elunchbox_pwr_wake_armed;   /* manual off 后允许再次长按 3s 唤醒 */
static bool elunchbox_pwr_need_fresh_press; /* 关机松手后须新一次按下才计 3s 唤醒 */
static bool elunchbox_manual_wake_pending; /* 浅睡内检测到长按，退出 sleep 后再亮屏 */
static bool elunchbox_boot_power_sent;
static bool elunchbox_pwr_hw_off;       /* 协议/BLE 关机已向加热模块发 PowerSwitch=OFF */
static s32 elunchbox_guioff_sleep_delay = -1L;
static u8 elunchbox_guioff_sleep_mode;
static u32 elunchbox_idle_tmr = (u32)ELUNCHBOX_GUIOFF_TIME_SEC * 10;  /* 100ms 单位，独立于 sys_cb.guioff_delay */
static bool elunchbox_pwr_pending_auto_shutdown;  /* 空闲定时器到期，延迟执行 manual_shutdown */
u32  elunchbox_saved_clkgat0;               /* 关机时保存 CLKGAT0，唤醒后恢复 */
static u32  elunchbox_manual_wake_home_tick;       /* 手动关机后 TCH5 唤醒：短时抑制自动跳加热页 */
static u32  elunchbox_uart_listen_tick;            /* PB9 门铃唤醒后 UART 收包窗口起点 */
static u32  elunchbox_uart_listen_query_tick;      /* 窗口内上次主动查询时刻 */
static bool elunchbox_uart_listen_on;
static bool elunchbox_uart_listen_probed;          /* 窗口内已开 TX 并完成首次查询 */

static u32  elunchbox_boot_tick;                   /* 上电时刻：开机窗口内插电进黑屏充电页 */
static bool elunchbox_boot_charge_check;           /* 开机后短时检测充电 → 黑屏跑马灯 */

#ifndef ELUNCHBOX_MANUAL_WAKE_HOME_HOLD_MS
#define ELUNCHBOX_MANUAL_WAKE_HOME_HOLD_MS  3000
#endif

/* 门铃首包丢失：唤醒后保持清醒，主动查询充电状态 */
#ifndef ELUNCHBOX_UART_LISTEN_MS
#define ELUNCHBOX_UART_LISTEN_MS           5000
#endif
#ifndef ELUNCHBOX_UART_LISTEN_QUERY_MS
#define ELUNCHBOX_UART_LISTEN_QUERY_MS     800
#endif
/* 上电后若检测到充电（含关机插电导致复位重启），进黑屏跑马灯 */
#ifndef ELUNCHBOX_BOOT_CHARGE_WINDOW_MS
#define ELUNCHBOX_BOOT_CHARGE_WINDOW_MS    15000
#endif

/* need_fresh_press 超时兜底：TCH5 硬件卡住时强制清零，防止主循环死等不休眠 */
#ifndef ELUNCHBOX_FRESH_PRESS_TIMEOUT_MS
#define ELUNCHBOX_FRESH_PRESS_TIMEOUT_MS   10000
#endif

void elunchbox_manual_off_uart_listen_arm(void);
bool elunchbox_manual_off_uart_listening(void);
#if ELUNCHBOX_PANEL_EN && FUNC_LUNCHBOX_UART_EN
static void elunchbox_manual_off_uart_listen_probe(void);
#endif

/** 主页已完全显示：清除开机充电窗口，后续充电只更新主页电池图标，不切黑屏页 */
void elunchbox_boot_charge_check_clear(void)
{
    elunchbox_boot_charge_check = false;
}

static bool elunchbox_manual_wake_home_active(void)
{
    if (elunchbox_manual_wake_home_tick == 0) {
        return false;
    }
    return !tick_check_expire(elunchbox_manual_wake_home_tick, ELUNCHBOX_MANUAL_WAKE_HOME_HOLD_MS);
}

static bool elunchbox_is_charging(void)
{
    return home_ui_shared_battery_is_charging();
}
#if USER_PT8028_KEY
static void func_elunchbox_guioff_wake_poll(void);
#endif
#if SOFT_POWER_ON_OFF
static void func_elunchbox_pwr_long_poll(void);
#endif

void elunchbox_guioff_sleep_delay_reset(void)
{
#if ELUNCHBOX_GUIOFF_SLEEP_EN
    elunchbox_guioff_sleep_delay = (s32)ELUNCHBOX_GUIOFF_SLEEP_DELAY_SEC * 10;
#else
    elunchbox_guioff_sleep_delay = -1L;
#endif
}

void elunchbox_guioff_sleep_delay_tick(void)
{
    if (elunchbox_guioff_sleep_delay > 0) {
        elunchbox_guioff_sleep_delay--;
    }
}

bool elunchbox_guioff_sleep_ready(void)
{
#if ELUNCHBOX_GUIOFF_SLEEP_EN
    return elunchbox_guioff_sleep_delay == 0;
#else
    return false;
#endif
}

void elunchbox_guioff_sleep_delay_rearm(void)
{
#if ELUNCHBOX_GUIOFF_SLEEP_EN
    elunchbox_guioff_sleep_delay = 10;             /* 浅睡唤醒后 1s 再允许入睡 */
#endif
}

bool elunchbox_guioff_in_sleep_mode(void)
{
    return elunchbox_guioff_sleep_mode != 0;
}

void elunchbox_guioff_sleep_service(void)
{
#if FUNC_LUNCHBOX_UART_EN
    if (!elunchbox_pwr_is_manual_off()) {
        lunchbox_uart_process();
        lunchbox_keep_warm_poll();
    } else if (func_reservation_is_waiting()) {
        lunchbox_uart_process();
    }
#endif
    if (!elunchbox_pwr_is_manual_off() || func_reservation_is_waiting()) {
        func_reservation_poll();
    }
}

void elunchbox_guioff_sleep_mode_enter(void)
{
    elunchbox_guioff_sleep_mode = 1;
}

bool elunchbox_heating_blocks_idle(void)
{
#if ELUNCHBOX_PANEL_EN
    /* 手动关机(长按3s)：除用户再次长按开机外，不因残留加热/预约状态自动唤醒 */
    if (elunchbox_pwr_is_manual_off()) {
        return false;
    }
#endif
    /* OTA 升级进行中：阻止空闲计时器倒计时 → 防止关屏断蓝牙/WDT 复位 */
    if (bt_get_status() == BT_STA_OTA) {
        return true;
    }
#if FUNC_LUNCHBOX_UART_EN
    if (lb_ota_is_active()) {
        return true;
    }
    if (lunchbox_heating_task_active()) {
        return true;
    }
#endif
    if (heat_display_heating_active()) {
        return true;
    }
    if (func_reservation_is_heating()) {
        return true;
    }
    if (func_cb.sta == FUNC_HEAT && func_heat_ui_is_heating()) {
        return true;
    }
    if (func_cb.sta == FUNC_MODE && func_mode_ui_is_heating()) {
        return true;
    }
    if (func_cb.sta == FUNC_NEW_WARM) {
        return true;
    }
#if FUNC_LUNCHBOX_UART_EN
    if (lunchbox_keep_warm_is_active()) {
        return true;
    }
#endif
    return false;
}

static void elunchbox_pwr_gui_off_exit(void);

bool elunchbox_pwr_gui_off_is_on(void)
{
    return elunchbox_pwr_gui_off;
}

bool elunchbox_ui_is_live(void)
{
#if ELUNCHBOX_PANEL_EN
    if (elunchbox_pwr_is_manual_off()) {
        return false;
    }
#endif
    return !sys_cb.gui_sleep_sta && !elunchbox_pwr_gui_off;
}

bool elunchbox_pwr_is_manual_off(void)
{
#if ELUNCHBOX_PANEL_EN
    return elunchbox_pwr_manual_off;
#else
    return false;
#endif
}

bool elunchbox_pwr_manual_off_wake_pressing(void)
{
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    if (!elunchbox_pwr_is_manual_off() || !elunchbox_pwr_wake_armed) {
        return false;
    }
    if (elunchbox_pwr_need_fresh_press) {
        return false;
    }
    return pt8028_is_power_key_held() || pt8028_boot_tch5_down();
#else
    return false;
#endif
}

/* 【手动关机-唤醒保持】need_fresh_press 时：
 *   TCH5 按住 → 不进休眠(stay awake)，让 tick 推进等松手
 *   TCH5 松开 → 清 need_fresh_press，允许进休眠
 *   need_fresh_press 已清后：TCH5 按住 → 不进休眠(准备 2s 长按唤醒) */
bool elunchbox_pwr_manual_off_should_stay_awake(void)
{
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    if (!elunchbox_pwr_is_manual_off() || !elunchbox_pwr_wake_armed) {
        return false;
    }
    if (elunchbox_manual_off_uart_listening()) {
        return true;   /* PB9 门铃后收包窗口：勿立刻 force_lowpwr */
    }
    if (elunchbox_pwr_need_fresh_press) {
        if (pt8028_is_power_key_held() || pt8028_boot_tch5_down()) {
            return true;   /* 还按着 → stay awake，不进休眠 */
        }
        /* 松开了 → 清标志，允许进休眠 */
        elunchbox_pwr_need_fresh_press = false;
        printf("elunchbox: fresh_press cleared (released)\n");
        return false;
    }
    return pt8028_is_power_key_held() || pt8028_boot_tch5_down();
#else
    return false;
#endif
}

#if ELUNCHBOX_PANEL_EN && FUNC_LUNCHBOX_UART_EN
extern u16 lb_dp_encode_bool(u8 *buf, u8 dpid, u8 val);
extern u16 lb_dp_encode_value(u8 *buf, u8 dpid, u32 val);
extern void lb_uart_send_raw(u8 uart_cmd, u8 *data, u16 data_len, bool no_wait);

/** 主动查加热模块动态属性（含 DP04 充电），门铃首包丢失后靠此拿状态 */
static void elunchbox_manual_off_uart_query_charge(void)
{
    u8 data[24];
    u8 *p = data;
    u16 len;

    lb_uart_tx_block(false);
    p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, lb_get_unix_time());
    p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);
    len = (u16)(p - data);
    lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, len, true);
    elunchbox_uart_listen_query_tick = tick_get();
    printf("elunchbox: uart listen query DYNAMIC (charge)\n");
}

void elunchbox_manual_off_uart_listen_arm(void)
{
    elunchbox_uart_listen_on = true;
    elunchbox_uart_listen_tick = tick_get();
    elunchbox_uart_listen_query_tick = 0;
    elunchbox_uart_listen_probed = false;
    printf("elunchbox: uart listen arm %ums (charge/heat)\n",
           (unsigned)ELUNCHBOX_UART_LISTEN_MS);
}

bool elunchbox_manual_off_uart_listening(void)
{
    if (!elunchbox_uart_listen_on) {
        return false;
    }
#if CHARGE_EN
    /* 本机已插电：延长窗口，直到进充电页或拔电 */
    if (CHARGE_DC_IN()) {
        elunchbox_uart_listen_tick = tick_get();
        return true;
    }
#endif
    if (tick_check_expire(elunchbox_uart_listen_tick, ELUNCHBOX_UART_LISTEN_MS)) {
        elunchbox_uart_listen_on = false;
        if (elunchbox_pwr_is_manual_off() && !home_ui_shared_battery_is_charging()) {
            lb_uart_tx_block(true);
            printf("elunchbox: uart listen expire, re-block TX\n");
        }
        return false;
    }
    return true;
}

static void elunchbox_manual_off_uart_listen_probe(void)
{
    if (!elunchbox_uart_listen_probed) {
        elunchbox_uart_listen_probed = true;
        elunchbox_manual_off_uart_query_charge();
        return;
    }
    /* 窗口内周期性重查：门铃丢首包后模块可能只回一次 */
    if (tick_check_expire(elunchbox_uart_listen_query_tick, ELUNCHBOX_UART_LISTEN_QUERY_MS)) {
        elunchbox_manual_off_uart_query_charge();
    }
}
#else
void elunchbox_manual_off_uart_listen_arm(void)
{
}

bool elunchbox_manual_off_uart_listening(void)
{
    return false;
}
#endif

bool elunchbox_is_device_powered(void)
{
    return true;
}

static void elunchbox_pwr_manual_shutdown(void);

void elunchbox_pwr_gui_off_activate(void)
{
    if (elunchbox_pwr_gui_off && sys_cb.gui_sleep_sta) {
        return;
    }
    if (elunchbox_heating_blocks_idle() || elunchbox_is_charging()) {
        elunchbox_user_activity_reset();
        return;
    }
    printf("elunchbox: auto full-shutdown after %ds idle\n", ELUNCHBOX_GUIOFF_TIME_SEC);
    elunchbox_pwr_pending_auto_shutdown = true;
}

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static bool pt8028_pwr_key_fully_released(void)
{
    pt8028_gpio_ensure();
    return pt8028_out_flag_is_idle() && !pt8028_boot_tch5_down()
        && !pt8028_is_power_key_held() && !pt8028_is_press_active();
}

static void elunchbox_pwr_key_dbg(const char *tag)
{
    printf("elunchbox: keydbg [%s] idle=%u boot5=%u held=%u active=%u longrdy=%u fresh=%u armed=%u\n",
           tag,
           pt8028_out_flag_is_idle() ? 1u : 0u,
           pt8028_boot_tch5_down() ? 1u : 0u,
           pt8028_is_power_key_held() ? 1u : 0u,
           pt8028_is_press_active() ? 1u : 0u,
           pt8028_pwr_key_long_ready() ? 1u : 0u,
           elunchbox_pwr_need_fresh_press ? 1u : 0u,
           elunchbox_pwr_wake_armed ? 1u : 0u);
}
#endif

bool elunchbox_pwr_manual_off_gui_wake_ok(void)
{
#if ELUNCHBOX_PANEL_EN
    if (!elunchbox_pwr_is_manual_off()) {
        return true;
    }
    return elunchbox_pwr_intentional_wake;
#else
    return true;
#endif
}

#if ELUNCHBOX_PANEL_EN
void elunchbox_pwr_intentional_wake_set(bool on)
{
    elunchbox_pwr_intentional_wake = on;
}

/**
 * 关机态进黑屏充电页专用唤醒：恢复 GPU/时钟，但先不开背光。
 * 调用方切到 FUNC_CHARGE 后再 lunchbox_display_on()，避免先露出主界面。
 */
void elunchbox_pwr_wake_for_charge_off(void)
{
    printf("elunchbox: wake_for_charge_off manual=%u guioff=%u sleep=%u\n",
           elunchbox_pwr_is_manual_off() ? 1u : 0u,
           elunchbox_pwr_gui_off ? 1u : 0u,
           sys_cb.gui_sleep_sta ? 1u : 0u);

    elunchbox_uart_listen_on = false;
    elunchbox_pwr_gui_off = false;
    elunchbox_pwr_manual_off = false;
    elunchbox_guioff_sleep_delay_reset();
    elunchbox_pwroff_sent_reset();
    CLKGAT0 = elunchbox_saved_clkgat0;

    elunchbox_pwr_intentional_wake = true;
    if (sys_cb.gui_sleep_sta) {
        gui_wakeup();
    }
    elunchbox_pwr_intentional_wake = false;

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_pwr_long_consume();
    pt8028_release_clear();
    pt8028_set_home_msg_block(1);
#endif
#if FUNC_LUNCHBOX_UART_EN
    lb_uart_tx_block(false);
#endif
}
#endif

static void elunchbox_pwr_shutdown_yield(void)
{
    WDT_CLR();
    co_timer_pro(false);
}

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static void elunchbox_pwr_off_arm_wake_keys(const char *tag)
{
    pt8028_pwr_long_consume();
    pt8028_pwr_manual_off_arm();
    pt8028_key_scan();
    if (!pt8028_pwr_key_fully_released()) {
        printf("elunchbox: %s still holding -> need_fresh_press\n", tag);
        elunchbox_pwr_key_dbg("off_arm holding");
    } else {
        printf("elunchbox: %s already released\n", tag);
        elunchbox_pwr_key_dbg("off_arm released");
    }
    pt8028_set_home_msg_block(0);
    func_home_drain_stale_key_msgs();
    elunchbox_pwr_wake_armed = true;
    elunchbox_pwr_need_fresh_press = true;
}
#endif

/* 纯关屏：仅熄屏。完整关机见 elunchbox_do_full_shutdown（先停加热再关 MCU）。 */
static void elunchbox_screen_off(void)
{
    if (elunchbox_pwr_gui_off && sys_cb.gui_sleep_sta) {
        return;
    }
    if (elunchbox_is_charging()) {
        elunchbox_pwr_pending_auto_shutdown = false;
        elunchbox_user_activity_reset();
        printf("elunchbox: screen off blocked (charging)\n");
        return;
    }
    if (bt_get_status() == BT_STA_OTA) {
        printf("elunchbox: screen off blocked (BLE OTA in progress)\n");
        return;
    }
#if FUNC_LUNCHBOX_UART_EN
    if (lb_ota_is_active()) {
        printf("elunchbox: screen off blocked (UART OTA in progress)\n");
        return;
    }
#endif
#if USER_PANEL_LED
    panel_led_all_off();
    panel_led_set_switch_latched(false);
#endif
    elunchbox_pwr_gui_off = true;
    elunchbox_guioff_sleep_delay = 0;
    elunchbox_saved_clkgat0 = CLKGAT0;  // 唤醒时恢复，否则 CLKGAT0=0→8001 蓝屏

    WDT_CLR();
    lunchbox_display_off();  // 等 GPU 帧完成 → 关背光 → 关 VDDLCD（不设 gui_sleep_sta，保持 GPU 链路）
    WDT_CLR();

    printf("elunchbox: screen off (BLE/UART/heating keep alive)\n");
}

/**
 * 完整关机：与长按 3s / 5 分钟无操作相同。
 * Step1 HeatEnable=0 → Step2 PowerSwitch=OFF → 熄屏 → manual_off 深睡（再长按开机）。
 */
static void elunchbox_do_full_shutdown(const char *reason)
{
    const char *tag = (reason != NULL) ? reason : "?";

    printf("elunchbox: full shutdown begin (%s)\n", tag);
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_mcu_shutdown_sequence();
    elunchbox_pwroff_sent_mark();   /* 休眠握手勿重复下发 */
#endif
    elunchbox_pwr_manual_off = true;
    elunchbox_screen_off();
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_pwr_manual_off_arm();
    elunchbox_pwr_wake_armed = true;
    elunchbox_pwr_need_fresh_press = true;
#endif
    printf("elunchbox: full shutdown done (%s)\n", tag);
}

/* 保留旧函数名兼容，内部转调纯关屏 */
static void elunchbox_pwr_manual_shutdown(void)
{
    elunchbox_screen_off();
}

void elunchbox_pwr_ble_switch(bool on)
{
#if FUNC_LUNCHBOX_UART_EN
    if (on) {
        if (!elunchbox_pwr_gui_off && !sys_cb.gui_sleep_sta) {
            printf("elunchbox: BLE power on ignored (already on)\n");
            return;
        }
        elunchbox_pwr_gui_wake_reason("ble power on");
    } else {
        if (elunchbox_is_charging()) {
            printf("elunchbox: BLE power off blocked (charging)\n");
            return;
        }
        if (elunchbox_pwr_gui_off || sys_cb.gui_sleep_sta) {
            printf("elunchbox: BLE power off ignored (already off)\n");
            return;
        }
        /* 纯关屏：不断 BLE，不封 UART，不停加热 */
        elunchbox_screen_off();
        printf("elunchbox: BLE power off -> screen off (short press to wake)\n");
    }
#else
    (void)on;
#endif
}

static bool elunchbox_is_guioff(void);

void elunchbox_guioff_sleep_post_wake(bool key_wake)
{
    if (!elunchbox_pwr_is_manual_off()) {
        printf("elunchbox: sleep_post_wake key_wake=%u pending=%u\n",
               key_wake ? 1u : 0u,
               elunchbox_manual_wake_pending_peek() ? 1u : 0u);
    } else {
        printf("elunchbox: sleep_post_wake manual_off key_wake=%u pending=%u\n",
               key_wake ? 1u : 0u,
               elunchbox_manual_wake_pending_peek() ? 1u : 0u);
    }
    elunchbox_guioff_sleep_mode = 0;
    pt8028_port_gpio_init();
    pt8028_key_scan();
    /* manual_off: 立刻丢弃唤醒时读到的按键状态，防止亮屏后主循环
     * pt8028_key_scan() 看到松手边沿 → 误入队 → 误触发加热等界面。
     * 真正的 3s 长按判断在主循环通过 GPIO 电平独立判断，不依赖此处状态。 */
    if (elunchbox_pwr_is_manual_off()) {
        pt8028_release_clear();
    }
    /* 浅睡循环内 manual_off_sleep_poll 检测到 FLAG 下降沿 → 设 pending → 退出深睡。
     * manual_off: 只退出睡眠，不亮屏。主循环 func_elunchbox_guioff_wake_poll()
     *   执行 3 秒长按判断后才调 elunchbox_pwr_gui_wake()。
     * auto guioff: 短按直接亮屏（保持原有行为）。 */
    if (elunchbox_manual_wake_pending_take()) {
        if (elunchbox_is_guioff() && !elunchbox_pwr_is_manual_off()) {
            printf("elunchbox: sleep_post_wake -> gui_wake (key_wake=%u)\n", key_wake ? 1u : 0u);
            elunchbox_pwr_gui_off = false;
            elunchbox_pwr_manual_off = false;
            if (sys_cb.gui_sleep_sta) {
                gui_wakeup();
            }
            lunchbox_display_on();
            elunchbox_user_activity_reset();
        }
    }
#if USER_PT8028_KEY
    /* manual_off: 3s 长按判断延后到主循环，此处不重复启动计时 */
    if (elunchbox_is_guioff() && !elunchbox_pwr_is_manual_off()) {
        func_elunchbox_guioff_wake_poll();
#if SOFT_POWER_ON_OFF
        func_elunchbox_pwr_long_poll();
#endif
    }
#endif
    if (!elunchbox_pwr_is_manual_off()) {
        elunchbox_guioff_sleep_service();
    }
#if ELUNCHBOX_PANEL_EN
    /* 非按键唤醒(RX UART): 预约到时 + 加热模块 UART 确认加热 → 唤醒并跳转
     * 预约到点先进加热页，加热页上再由 MCU 充电转保温 */
    if (!key_wake && func_reservation_is_heating() && lunchbox_heating_task_active()) {
        if (heat_display_reservation_can_switch_heat()) {
            printf("reservation: RX wake + heating confirmed, switch to heat panel\n");
            func_elunchbox_switch_to_heat_panel();
        }
        heat_display_warm_charge_route_poll();
    }
#endif
    if (key_wake) {
        elunchbox_guioff_sleep_delay_reset();
    } else if (elunchbox_pwr_is_manual_off()) {
        elunchbox_guioff_sleep_delay = 0;   /* 手动关机：浅睡返回后立即再入睡 */
    } else {
        elunchbox_guioff_sleep_delay_rearm();
    }
}

void elunchbox_user_activity_reset(void)
{
    if (elunchbox_pwr_gui_off_is_on() || sys_cb.gui_sleep_sta) {
        return;
    }
    elunchbox_idle_tmr = (u32)ELUNCHBOX_GUIOFF_TIME_SEC * 10;
}

void elunchbox_guioff_idle_tick(void)
{
    if (elunchbox_pwr_gui_off_is_on() || sys_cb.gui_sleep_sta) {
        return;
    }
    if (elunchbox_heating_blocks_idle() || elunchbox_is_charging()) {
        elunchbox_idle_tmr = (u32)ELUNCHBOX_GUIOFF_TIME_SEC * 10;
        elunchbox_pwr_pending_auto_shutdown = false;
        return;
    }
    if (elunchbox_idle_tmr > 0) {
        elunchbox_idle_tmr--;
    }
}

bool elunchbox_guioff_idle_expired(void)
{
    if (elunchbox_pwr_gui_off_is_on() || sys_cb.gui_sleep_sta) {
        return false;
    }
    if (elunchbox_heating_blocks_idle() || elunchbox_is_charging()) {
        return false;
    }
    return elunchbox_idle_tmr == 0;
}

void elunchbox_pwr_gui_wake(void)
{
    elunchbox_pwr_gui_wake_reason("unspecified");
}

void elunchbox_pwr_gui_wake_reason(const char *reason)
{
    const char *tag = (reason != NULL) ? reason : "?";

    printf("elunchbox: WAKE try reason=%s manual=%u guioff=%u sleep=%u fresh=%u armed=%u sta=%u\n",
           tag,
           elunchbox_pwr_is_manual_off() ? 1u : 0u,
           elunchbox_pwr_gui_off ? 1u : 0u,
           sys_cb.gui_sleep_sta ? 1u : 0u,
           elunchbox_pwr_need_fresh_press ? 1u : 0u,
           elunchbox_pwr_wake_armed ? 1u : 0u,
           (unsigned)func_cb.sta);
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    elunchbox_pwr_key_dbg("wake_try");
#endif
    if (!elunchbox_pwr_gui_off && !sys_cb.gui_sleep_sta) {
        printf("elunchbox: WAKE ignored (already on) reason=%s\n", tag);
        return;
    }
    elunchbox_pwr_intentional_wake = true;
    printf("elunchbox: WAKE execute reason=%s\n", tag);
    elunchbox_pwr_gui_off_exit();
    elunchbox_pwr_intentional_wake = false;
    printf("elunchbox: [DBG] WAKE done reason=%s final: guioff=%u manual=%u sleep=%u sta=%u\n",
           tag,
           elunchbox_pwr_gui_off ? 1u : 0u,
           elunchbox_pwr_is_manual_off() ? 1u : 0u,
           sys_cb.gui_sleep_sta ? 1u : 0u,
           (unsigned)func_cb.sta);
}

static void elunchbox_screen_wake(void)
{
    printf("elunchbox: [DBG] screen_wake enter sta=%u guioff=%u sleep=%u\n",
           (unsigned)func_cb.sta,
           elunchbox_pwr_gui_off ? 1u : 0u,
           sys_cb.gui_sleep_sta ? 1u : 0u);

    elunchbox_pwr_gui_off = false;
    elunchbox_pwr_manual_off = false;
    elunchbox_guioff_sleep_delay_reset();
    elunchbox_pwroff_sent_reset();  /* 屏真亮了 → 允许下次 manual_off 重发 PowerSwitch=OFF */

    CLKGAT0 = elunchbox_saved_clkgat0;

    if (sys_cb.gui_sleep_sta) {
        printf("elunchbox: [DBG] screen_wake calling gui_wakeup()\n");
        gui_wakeup();
    }
    printf("elunchbox: [DBG] screen_wake calling lunchbox_display_on()\n");
    lunchbox_display_on();
    printf("elunchbox: [DBG] screen_wake display_on done\n");
    elunchbox_user_activity_reset();

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_pwr_long_consume();
    pt8028_release_clear();
    pt8028_set_home_msg_block(1);
#endif

#if ELUNCHBOX_PANEL_EN
    home_ui_shared_status_inited = false;
    home_ui_shared_status_lock_preloaded = false;
    bool bat_was_charging = home_ui_shared_battery_is_charging();
    home_ui_shared_battery_boot_init();
    if (bat_was_charging) {
        home_ui_shared_battery_charge_apply(1);
    }
#endif
#if BT_BACKSTAGE_EN
    bt_update_bt_scan_param_default();
#endif

#if USER_PANEL_LED
    panel_led_scan();
#endif
    printf("elunchbox: [DBG] screen_wake COMPLETE\n");
}

static void elunchbox_pwr_gui_off_exit(void)
{
    elunchbox_screen_wake();
}

void elunchbox_panel_boot_power_on(void)
{
    if (elunchbox_boot_power_sent) {
        return;
    }
    elunchbox_boot_power_sent = true;
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_power_on();
    lunchbox_query_reservation_list();
#endif
}

static bool elunchbox_is_guioff(void)
{
    return sys_cb.gui_sleep_sta || elunchbox_pwr_gui_off;
}

static void elunchbox_guioff_idle_process(void)
{
#if FUNC_LUNCHBOX_UART_EN
    if (!elunchbox_pwr_is_manual_off()) {
        lunchbox_uart_process();
        lunchbox_keep_warm_poll();
    } else if (func_reservation_is_waiting()) {
        lunchbox_uart_process();
    }
#endif
    if (!elunchbox_pwr_is_manual_off() || func_reservation_is_waiting()) {
        func_reservation_poll();
    }
}

static bool elunchbox_manual_off_wake_ready(void)
{
    static u8  fresh_wait_logged;
    static u32 fresh_press_start;   /* need_fresh_press 开始时刻，用于超时兜底 */

    if (!elunchbox_pwr_wake_armed) {
        fresh_press_start = 0;
        return false;
    }
    if (elunchbox_pwr_need_fresh_press) {
        if (pt8028_boot_tch5_down() || pt8028_is_power_key_held()) {
            if (fresh_press_start == 0) {
                fresh_press_start = tick_get();
            }
            if (!fresh_wait_logged) {
                fresh_wait_logged = 1;
                printf("elunchbox: wake blocked, wait TCH5 release (fresh_press)\n");
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
                elunchbox_pwr_key_dbg("fresh_wait_release");
#endif
            }
            /* 超时兜底：TCH5 硬件卡住时强制清零，允许进深度休眠防电池耗光 */
            if (tick_check_expire(fresh_press_start, ELUNCHBOX_FRESH_PRESS_TIMEOUT_MS)) {
                elunchbox_pwr_need_fresh_press = false;
                fresh_wait_logged = 0;
                fresh_press_start = 0;
                printf("elunchbox: fresh_press force cleared (timeout %ums, TCH5 stuck?)\n",
                       ELUNCHBOX_FRESH_PRESS_TIMEOUT_MS);
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
                elunchbox_pwr_key_dbg("fresh_timeout");
#endif
                return true;
            }
            return false;
        }
        elunchbox_pwr_need_fresh_press = false;
        fresh_wait_logged = 0;
        fresh_press_start = 0;
        printf("elunchbox: fresh_press cleared, ready for new 3s hold\n");
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        elunchbox_pwr_key_dbg("fresh_cleared");
#endif
    }
    return true;
}

/* 【手动关机唤醒-轮询】仅由 func_elunchbox_guioff_wake_poll / func_elunchbox_pwr_long_poll
 * 在主循环内检测 FLAG + BCD 并判断长按，不再在浅睡循环内识别键值。 */

/* 【熄屏唤醒轮询】检测按键长按/短按，区分手动关机态和普通熄屏态：
 * - 手动关机态：TCH5/电源键长按 3 秒 → 设 manual_wake_pending 标志，由 func_process 调用方唤醒
 * - 普通熄屏态：TCH5/电源键短按 → 直接唤醒 GUI
 */
static void func_elunchbox_guioff_wake_poll(void)
{
    static u32 hold_start;
    static u8  hold_log_once;

    if (!elunchbox_is_guioff()) {
        hold_start = 0;
        hold_log_once = 0;
        return;
    }
#if ELUNCHBOX_PANEL_EN
    /* --- 手动关机态：长按 3 秒唤醒 --- */
    if (elunchbox_pwr_is_manual_off()) {
        if (!elunchbox_pwr_wake_armed) {
            hold_start = 0;
            hold_log_once = 0;
            if (pt8028_take_pwr_long_pending()) {
                printf("elunchbox: discard pwr_long_pending (!wake_armed)\n");
                pt8028_pwr_long_consume();
            }
            return;
        }
        if (!elunchbox_manual_off_wake_ready()) {
            hold_start = 0;
            hold_log_once = 0;
            if (pt8028_take_pwr_long_pending()) {
                printf("elunchbox: discard pwr_long_pending (!wake_ready poll)\n");
                pt8028_pwr_long_consume();
            }
            return;
        }
        /* 直接读 GPIO(OUT_FLAG+BCD)，不依赖 press_emitted / 浅睡内驱动状态 */
        if (pt8028_boot_tch5_down() || pt8028_is_power_key_held()) {
            if (hold_start == 0) {
                hold_start = tick_get();
                if (!hold_log_once) {
                    hold_log_once = 1;
                    printf("elunchbox: manual off hold begin (need %ums) boot5=%u held=%u\n",
                           PT8028_PWR_LONG_MS,
                           pt8028_boot_tch5_down() ? 1u : 0u,
                           pt8028_is_power_key_held() ? 1u : 0u);
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
                    elunchbox_pwr_key_dbg("hold_begin");
#endif
                }
            } else if (tick_check_expire(hold_start, PT8028_PWR_LONG_MS)) {
                hold_start = 0;
                hold_log_once = 0;
                /* 只设标志，由调用方统一唤醒 */
                printf("elunchbox: [DBG] manual off hold COMPLETE -> set manual_wake_pending\n");
                elunchbox_manual_wake_pending = true;
            }
        } else {
            if (hold_start != 0) {
                printf("elunchbox: manual off hold cancelled (released)\n");
            }
            hold_start = 0;
            hold_log_once = 0;
        }
        return;
    }
#endif
    /* --- 普通熄屏态：短按唤醒 GUI --- */
    if (pt8028_is_power_key_held() || pt8028_boot_tch5_down()) {
        if (hold_start == 0) {
            hold_start = tick_get();
            printf("elunchbox: [DBG] auto_guioff key down detected, waiting %ums for wake\n", PT8028_PWR_WAKE_MS);
        } else if (tick_check_expire(hold_start, PT8028_PWR_WAKE_MS)) {
            hold_start = 0;
            printf("elunchbox: [DBG] auto_guioff short press -> wake\n");
            elunchbox_pwr_gui_wake_reason("guioff_poll auto_guioff");
        }
    } else {
        hold_start = 0;
    }
}

/* 【电源键长按处理】消费 PT8028 驱动的 pwr_long_pending 事件：
 * - 普通亮屏态（!guioff）：长按 3 秒 → 纯关屏
 * - 熄屏态（guioff）：直接返回（唤醒由 guioff_wake_poll 的短按处理）
 */
static void func_elunchbox_pwr_long_poll(void)
{
    if (!pt8028_take_pwr_long_pending()) {
        return;
    }
    printf("elunchbox: pwr_long_pending taken guioff=%u\n",
           elunchbox_is_guioff() ? 1u : 0u);
    pt8028_pwr_long_consume();
    if (elunchbox_is_guioff()) {
        /* 熄屏态：长按唤醒已由 guioff_wake_poll 短按处理，此处忽略 */
        return;
    }
    /* 亮屏充电中长按关机：进黑屏充电跑马灯页（不深睡），拔电回主页
     * 加热/保温中长按仍走完整关机，勿误进跑马灯 */
    if (elunchbox_is_charging() && !elunchbox_heating_blocks_idle()) {
        printf("elunchbox: pwr_long while charging -> charge off page\n");
        func_elunchbox_enter_charge_off_page();
        return;
    }
    /* 普通亮屏态：长按 3 秒 = 关机（停加热→MCU关机→熄屏深睡） */
    printf("elunchbox: pwr_long_pending -> full shutdown\n");
    elunchbox_do_full_shutdown("pwr_long_3s");
}

void elunchbox_manual_off_sleep_poll(void)
{
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    static u8  last_flag = 1;
    static bool primed = false;       /* 首次进入时同步硬件状态，防 PT8028 松手去抖误唤醒 */
    static u8  last_sleep_mode = 0;   /* 追踪 guioff_sleep_mode 0→1 跳变，每轮休眠复位 primed */

    if (!elunchbox_pwr_is_manual_off() || !elunchbox_pwr_wake_armed) {
        last_flag = 1;
        primed = false;
        last_sleep_mode = 0;
        return;
    }
    /* 每轮深度休眠 guioff_sleep_mode 0→1 时复位 primed。
     * primed 不能在 guard 里靠 !manual_off 复位：manual_off 被 screen_wake 清 false
     * 后本函数不再被调用，guard 永远执行不到 → primed 残留为 true → 下次进休眠跳同步。 */
    {
        u8 cur = elunchbox_guioff_in_sleep_mode() ? 1u : 0u;
        if (cur && !last_sleep_mode) {
            primed = false;
        }
        last_sleep_mode = cur;
    }
    WDT_CLR();
    /* 【手动关机-浅睡轮询】只读 FLAG(PE1)，不读 BCD(PE2~PE4)：
     *   sfunc_sleep 已把 PE2~PE4 切为模拟以防误唤醒、降功耗；
     *   若在此调 pt8028_key_scan()→get_pt8028_key()→pt8028_gpio_bcd_ensure()
     *   会把 BCD 线重新拉回数字输入，浪费功耗且引入 BCD 跳变误唤醒。
     *   唤醒分两步：
     *     Step 1: FLAG 下降沿(键按下) → 退出浅睡
     *     Step 2: 主循环 sfunc_sleep 退出后 pt8028_port_gpio_init 重开 BCD
     *             → 读键值 → 判断 TCH5 → 2s 长按 → 真正唤醒 */
    u8 flag = pt8028_read_flag_raw();  //获取状态
    //printf("%s: %d->flag=%d\n",__func__,__LINE__, flag);
    if (!primed) {
        /* 首次轮询：同步 last_flag 到当前硬件电平。
         * 关机长按松手后 BCD 先恢复但 OUT_FLAG 可能仍为 LOW（PT8028 去抖延迟），
         * 若 last_flag 保持初始值 1 而 flag=0，会误判为下降沿 → 立即唤醒。
         * 同步后：flag=LOW 时 last_flag=LOW → 等真正松手(HIGH)后再按下(LOW)才唤醒。 */
        last_flag = flag;
        primed = true;
        return;
    }
    if (flag == 0 && last_flag == 1) {  //xing
        elunchbox_manual_wake_pending = true;
    }
    last_flag = flag;
#endif
}

bool elunchbox_manual_wake_pending_take(void)
{
    bool pending = elunchbox_manual_wake_pending;
    elunchbox_manual_wake_pending = false;
    return pending;
}

bool elunchbox_manual_wake_pending_peek(void)
{
    return elunchbox_manual_wake_pending;
}
#endif /* ELUNCHBOX_PANEL_EN */

#if USER_PT8028_KEY && SOFT_POWER_ON_OFF && !ELUNCHBOX_PANEL_EN
static void func_elunchbox_pwr_long_poll(void)
{
    if (!pt8028_take_pwr_long_pending()) {
        return;
    }
    func_cb.sta = FUNC_PWROFF;
}
#endif

#endif /* USER_PT8028_KEY && SOFT_POWER_ON_OFF */

#if USER_PT8028_KEY && FUNC_LUNCHBOX_UART_EN
static void func_elunchbox_key_notify_poll(void)
{
    u8 tch;
    u8 key_val;

    tch = pt8028_take_key_sound_defer_tch();
    if (tch > PT8028_KEY_TCH7) {
        return;
    }
    key_val = pt8028_tch_to_lunchbox_key(tch);
    if (key_val != 0) {
        /* 童锁误触也发 KEY_NOTIFY，加热模块仅蜂鸣反馈 */
        lunchbox_key_notify(key_val);
    }
}
#endif /* USER_PT8028_KEY && FUNC_LUNCHBOX_UART_EN */

AT(.text.func.process)
void func_process(void)
{
#if ELUNCHBOX_PANEL_EN
    bool guioff = elunchbox_is_guioff();
#else
    bool guioff = sys_cb.gui_sleep_sta;
#endif

#if ELUNCHBOX_PANEL_EN
    if (guioff && elunchbox_pwr_is_manual_off()) {  //手动关机
        {
            static u32 manual_loop_hb;
            if (tick_check_expire(manual_loop_hb, 2000)) {
                manual_loop_hb = tick_get();
                printf("elunchbox: func_process manual_off loop guioff=%u\n", guioff ? 1u : 0u);
            }
        }
        WDT_CLR();  //喂狗->防止系统复位
#if USER_PT8028_KEY
        pt8028_set_home_msg_block(0);
        pt8028_gpio_ensure_periodic();        //确保摁键周期性扫描
        pt8028_key_scan();                    //检测是否有长按唤醒
        func_elunchbox_guioff_wake_poll();    //轮询唤醒
#if SOFT_POWER_ON_OFF
        func_elunchbox_pwr_long_poll();   //轮询检查电源键长按（3 秒开机）
#endif
        reset_sleep_delay();    //重置休眠倒计时 → 阻止系统进入休眠->USER_PT8028_KEY有操作
        reset_pwroff_delay();   //重置关机倒计时 → 阻止系统自动关机->USER_PT8028_KEY有操作
#else
        reset_sleep_delay();
        reset_pwroff_delay();
#endif
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_uart_process();    //轮询接收充电模块发来的数据

        /* PB9 门铃唤醒后：发 PowerSwitch=ON 探测，并保持清醒收充电包 */
#if ELUNCHBOX_PANEL_EN
        if (elunchbox_manual_off_uart_listening()) {
            elunchbox_manual_off_uart_listen_probe();
            lunchbox_uart_process();
        }
#endif

#if ELUNCHBOX_LOWBAT_MODE_EN
        elunchbox_lowbat_poll();
        if (elunchbox_lowbat_active()) {
            return;
        }
#endif

        /* 用户 TCH5 长按唤醒：先清 manual_off → gui_wakeup() 不阻塞 → 再亮屏 */
        if (elunchbox_manual_wake_pending_take()) {
            printf("elunchbox: TCH5 3s hold wakes screen from manual off\n");
            elunchbox_pwr_gui_off = false;
            elunchbox_pwr_manual_off = false;
            elunchbox_uart_listen_on = false;
            elunchbox_guioff_sleep_delay_reset();
            elunchbox_pwroff_sent_reset();  /* 屏真亮了 → 允许下次 manual_off 重发 PowerSwitch=OFF */
            CLKGAT0 = elunchbox_saved_clkgat0;
            if (sys_cb.gui_sleep_sta) {
                gui_wakeup();
            }
            lunchbox_display_on();
#if FUNC_LUNCHBOX_UART_EN
            /* 关机时已发 PowerSwitch=OFF，唤醒须重新开 MCU */
            lunchbox_power_on();
#endif
            elunchbox_user_activity_reset();
            return;
        }

        /* 关机态插电 → 黑屏充电跑马灯；UART 充电 / 本机 DC_IN 均可进入 */
        if (heat_display_charge_wake_pending()
            || home_ui_shared_battery_is_charging()
#if CHARGE_EN
            || CHARGE_DC_IN()
#endif
            ) {
#if CHARGE_EN
            if (CHARGE_DC_IN() && !home_ui_shared_battery_is_charging()) {
                /* 本机已检测到 DC，先置充电态以便跑马灯；UART 随后校正 */
                home_ui_shared_battery_charge_apply(1);
            }
#endif
            printf("elunchbox: charge from manual off -> charge off page (chg=%u dc=%u)\n",
                   home_ui_shared_battery_is_charging() ? 1u : 0u,
#if CHARGE_EN
                   CHARGE_DC_IN() ? 1u : 0u
#else
                   0u
#endif
                   );
            elunchbox_uart_listen_on = false;
            lb_uart_tx_block(false);
            func_elunchbox_enter_charge_off_page();
            return;
        }
        if (heat_display_warm_charge_pending_active()) {
            printf("elunchbox: warm_charge DP wakes screen from manual off\n");
            elunchbox_pwr_gui_wake();
            return;
        }
        /* 预约时间到 → 加热模块自发加热 → UART 上报 HEAT_ENABLE=1 → 唤醒
         * 预约到点先进加热页，加热页上再由 MCU 充电转保温 */
        {
            bool heat_pending = heat_display_heat_wake_pending();
            if (heat_pending && g_res.setup_done) {
                elunchbox_pwr_gui_wake();
                if (heat_display_reservation_can_switch_heat()) {
                    printf("elunchbox: reservation heating wakes screen from manual off\n");
                    func_elunchbox_switch_to_heat_panel();
                }
                heat_display_warm_charge_route_poll();
                return;
            }
            if (heat_pending && !g_res.setup_done) {
                printf("elunchbox: [DEBUG] manual_off heat_pending=1 BUT setup_done=0, skip wake\n");
            }
        }
#endif
#if FUNC_RESERVATION_UI_EN
        func_reservation_poll();
        if (func_reservation_is_heating()) {
            return;
        }
#endif

        co_timer_pro(false);
        WDT_CLR();

        sleep_process(bt_is_allow_sleep);  //手动关机→深度休眠
        //lunchbox_display_off();  // 已在 elunchbox_pwr_manual_shutdown() 中调用，循环里无需重复
        return;
    }
#endif

#if ELUNCHBOX_PANEL_EN
    if (elunchbox_lowbat_active()) {
        WDT_CLR();
        elunchbox_lowbat_poll();
        if (!guioff) {
            tft_bglight_frist_set_check();
            if (func_cb.frm_main != NULL) {
                compo_update();
                gui_process();
            }
        }
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_uart_process();
#endif
        co_timer_pro(false);
        return;
    }
#endif

    if (gui_get_auto_power_en() && !guioff) {    //唤醒  !guioff
        sys_clk_req(INDEX_GUI, SYS_192M);        //恢复主频率
    }

    WDT_CLR();

#if CPU_USAGE_MONITOT_EN
    cpu_trace_monitor();
#endif

#if (SD_SUPPORT_EN) && SD_SOFT_DETECT_EN
    sd_soft_cmd_detect(120);
#endif

#if ELUNCHBOX_PANEL_EN
    if (!guioff || !sys_cb.gui_sleep_sta)   //
#endif
    tft_bglight_frist_set_check();  //背光检测

    // gui 没有休眠才更新
	#if FOTA_UI_EN
    if ((!guioff) && (func_cb.sta != FUNC_OTA_UI_MODE) && !sys_cb.flag_halt) {
	#else
	if (!guioff && !sys_cb.flag_halt) {  //是否执行GUI更新
	#endif

#if ELUNCHBOX_PANEL_EN
        bool gui_do_refresh = true;   // 默认允许刷新

        if (sys_cb.flag_swithing) {   // 如果正在切换页面/模式
            gui_do_refresh = false;   // 跳过本次刷新
        }
#if USER_PT8028_KEY
        /* Home 在 func_home_process 内扫键；子页（加热/模式/设置/预约等）须在此扫键 */
        if (func_cb.sta != FUNC_HOME) {
            u8 press_tch;

            pt8028_gpio_ensure_periodic();
            pt8028_key_scan();
            press_tch = pt8028_peek_press_tch();
            if (press_tch <= PT8028_KEY_TCH6) {
                elunchbox_user_activity_reset();
            }
        }
#endif
#if USER_PANEL_LED && USER_PT8028_KEY
        /* Home 已在 func_home_process 扫 LED；此处跳过避免重复 GPIO 采样 */
        if (func_cb.sta != FUNC_HOME) {
            panel_led_scan();
        }
#endif
        if (func_cb.frm_main != NULL) {
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
            if (!func_key_lock_pre_gui_poll())
#endif
            {
                compo_update();
#if ELUNCHBOX_PANEL_EN && LE_EN
                home_ui_shared_ble_status_poll();
#endif
#if ELUNCHBOX_PANEL_EN
                home_ui_shared_battery_chg_poll();
#endif
                if (gui_do_refresh) {
                    gui_process();    // 实际刷新屏幕
                }
            }
        }
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        func_key_lock_poll();
#endif
#if USER_PT8028_KEY && FUNC_LUNCHBOX_UART_EN
        /* 按键音走 UART：须在 gui_process 之后；在 key_lock_poll 之后以便童锁吞键也能蜂鸣 */
        func_elunchbox_key_notify_poll();
#endif
#if ELUNCHBOX_PANEL_EN
        home_ui_lowbat_poll();
#endif
#if USER_PT8028_KEY && FUNC_RESERVATION_UI_EN
        func_elunchbox_res_key_poll(); //预约
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        func_heat_key_poll();
#endif
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_keep_warm_poll();
#endif
        func_reservation_poll();  //预约定时
#else
        compo_update();                                     //更新组件

        gui_process();                                      //刷新UI
        func_reservation_poll();
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_keep_warm_poll();
#endif
#endif

    } else if (guioff) {  //guioff && !manual_off 自动息屏
#if ELUNCHBOX_PANEL_EN
        /* [DEBUG] 心跳打印：每约 2 秒一次，确认自动息屏循环在运行 */
        {
            static u32 dbg_guioff_heartbeat;
            if (tick_check_expire(dbg_guioff_heartbeat, 2000)) {
                dbg_guioff_heartbeat = tick_get();
                printf("elunchbox: [DBG] auto_guioff loop alive tch5=%u pwrkey=%u sleep=%u\n",
                       pt8028_boot_tch5_down() ? 1u : 0u,
                       pt8028_is_power_key_held() ? 1u : 0u,
                       sys_cb.gui_sleep_sta ? 1u : 0u);
            }
        }
        elunchbox_guioff_idle_process();   //熄屏空闲处理(UART收数据→可能设 charge/heat pending)
        /* 加热中充电进保温优先于黑屏跑马灯 */
        if (heat_display_warm_charge_pending_active()) {
            printf("elunchbox: warm_charge DP wakes screen from guioff\n");
            elunchbox_pwr_gui_wake();
            return;
        }
        /* 熄屏态插电 → 黑屏充电跑马灯；拔电回主页 */
        if (heat_display_charge_wake_pending()
            || home_ui_shared_battery_is_charging()) {
            if (elunchbox_heating_blocks_idle()) {
                printf("elunchbox: charge while heating from guioff -> wake\n");
                elunchbox_pwr_gui_wake();
                return;
            }
            printf("elunchbox: charge from guioff -> charge off page\n");
            func_elunchbox_enter_charge_off_page();
            return;
        }
        /* 预约加热已由加热模块自动启动 → 唤醒；预约到点先进加热页 */
        {
            bool heat_pending = heat_display_heat_wake_pending();
            if (heat_pending && g_res.setup_done) {
                elunchbox_pwr_gui_wake();
                if (heat_display_reservation_can_switch_heat()) {
                    printf("elunchbox: reservation heating wakes screen from guioff\n");
                    func_elunchbox_switch_to_heat_panel();
                }
                heat_display_warm_charge_route_poll();
                return;
            }
            if (heat_pending && !g_res.setup_done) {
                printf("elunchbox: [DEBUG] guioff heat_pending=1 BUT setup_done=0, skip wake\n");
            }
        }
        pt8028_gpio_ensure_periodic();
        pt8028_key_scan();
        func_elunchbox_guioff_wake_poll();  //轮询唤醒
#endif
    }

//#if OPUS_ENC_EN
//    if (bsp_opus_is_encode()) {
//        reset_sleep_delay_all();
//        u8 temp_buf[80];
//        memset(temp_buf, 0, sizeof(temp_buf));
//        if (bsp_opus_get_enc_frame(temp_buf, sizeof(temp_buf))) {
//            print_r(temp_buf, sizeof(temp_buf));
//        }
//    }
//#endif//OPUS_ENC_EN

    if (!elunchbox_pwr_is_manual_off()) {  
        co_timer_pro(false);
        bsp_sensor_step_pro_isr(); //wu guan

        if (sys_cb.mp3_res_playing) {
            mp3_res_process();    //wuguan                //提示音后台处理
        }
    }

    

#if FUNC_LUNCHBOX_UART_EN
    /* UART 必须在 sleep_process 之前处理：预约加热来的指令先解析
       → lb_attr_heat_enable=1 → elunchbox_heating_blocks_idle()=true
       → sleep_process 内的自动关屏判断才能正确跳过，避免"先关屏再唤醒"的抖动 */
    if (!guioff) {
        lunchbox_uart_process();
    }
#endif

    // sleep_process 内部已处理 guioff/非guioff 两路，勿用 !guioff 堵入口
    if (sleep_process(bt_is_allow_sleep)) {
        bt_cb.disp_status = 0xff;
    }
#if ELUNCHBOX_PANEL_EN
    /* 5 分钟无操作 = 完整关机（同长按 3s：停加热→MCU关机→熄屏深睡） */
    if (elunchbox_pwr_pending_auto_shutdown) {
        elunchbox_pwr_pending_auto_shutdown = false;
        if (elunchbox_pwr_is_manual_off()) {
            printf("elunchbox: auto shutdown skipped (already manual_off)\n");
        } else {
            elunchbox_do_full_shutdown("auto_idle_5min");
        }
    }
#endif
#if ELUNCHBOX_KEEP_AWAKE && ELUNCHBOX_PANEL_EN
    sys_cb.sleep_en = 0;
#endif

#if VBAT_DETECT_EN //各种子系统->到1302
    if (!elunchbox_pwr_is_manual_off()) {  
        bsp_vbat_lpwr_process();
    }
#endif

#if BT_BACKSTAGE_EN
    func_watch_bt_process();
#endif

#if CALL_MGR_EN
    func_call_mgr_process();
#endif
    //PWRKEY模拟硬开关关机处理
    if ((PWRKEY_2_HW_PWRON) && (sys_cb.pwrdwn_hw_flag)) {
        func_cb.sta = FUNC_PWROFF;
        sys_cb.pwrdwn_hw_flag = 0;
    }

#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_detect(1);
    }
#endif // CHARGE_EN

    if(bt_cb.bt_is_inited && !elunchbox_pwr_is_manual_off()) {
        bt_thread_check_trigger(); //经典蓝牙线程
#if LE_EN
        ble_app_process();//ble
#endif
#if LE_AB_FOT_EN
    	bsp_fot_process();
#endif
#if FUNC_LUNCHBOX_UART_EN
    	lb_ota_process();
#endif
    }

//#if MUSCI_BACKSTAGE_EN
//    bsp_music_process();
//#endif

#if ASR_SELECT
    if (!elunchbox_pwr_is_manual_off()) {
        bsp_asr_process();
    }
#endif

#if SENSOR_HUB_EN
    if (!elunchbox_pwr_is_manual_off()) {
        bsp_sensorhub_process();
    }
#endif

#if VBAT_ADC_EN
    {
        static u32 ticks = 0;
        if (!elunchbox_pwr_is_manual_off()) {
            u32 vadc_process(void);
            if (tick_check_expire(ticks, 500)) {
                ticks = tick_get();
                /*u32 val = */vadc_process();
        //        printf("vadc:%d uv, vbat:%d mv\n", val, sys_cb.vbat);
        //        void vadc_test(void);
        //        vadc_test();
            }
        }
    }
#endif

   if (gui_get_auto_power_en() && !guioff) {
        sys_clk_free(INDEX_GUI); // 释放 GUI 时钟
   }

#if FUNC_LUNCHBOX_UART_EN
    /* 开机窗口：关机插电导致复位后 UART 报充电 → 进黑屏跑马灯（勿停在主页）
     * 注意：主页完全显示后 elunchbox_boot_charge_check 已被清除，后续充电只更新图标
     * 加热/上盖确认等任务活跃时勿抢路由（应走充电→保温） */
    if (elunchbox_boot_charge_check && !guioff
        && !elunchbox_charge_off_active()
        && !elunchbox_lowbat_should_block_ui_route()
        && !sys_cb.flag_swithing
        && !elunchbox_heating_blocks_idle()) {
        if (tick_check_expire(elunchbox_boot_tick, ELUNCHBOX_BOOT_CHARGE_WINDOW_MS)) {
            elunchbox_boot_charge_check = false;
        } else if (home_ui_shared_battery_is_charging()
#if CHARGE_EN
                   || CHARGE_DC_IN()
#endif
                   ) {
            printf("elunchbox: boot+charging -> charge off page (black marquee)\n");
            elunchbox_boot_charge_check = false;
            func_elunchbox_enter_charge_off_page();
            return;
        }
    }

    /* 预约到点 / 上电 MCU 加热：先进盖确认或加热页 */
    {
        bool heat_pending = heat_display_heat_wake_pending();
        if (elunchbox_manual_wake_home_active()) {
            if (heat_pending) {
                printf("elunchbox: heat_wake ignored (manual wake -> home)\n");
            }
        } else if (!elunchbox_pwr_is_manual_off() && heat_pending
                   && heat_display_reservation_can_switch_heat()) {
            if (elunchbox_lid_confirm_is_armed()) {
                heat_display_info_t last;
                u8 mode = heat_display_get_mcu_mode();

                printf("elunchbox: power-on heating -> lid confirm\n");
                if (heat_display_get_last(&last)) {
                    elunchbox_lid_confirm_capture_and_stop(mode, last.remain_min, true,
                                                          last.temp_f, true);
                } else if (!elunchbox_lid_confirm_snap_valid()) {
                    elunchbox_lid_confirm_capture_and_stop(mode, 0, false, 176, false);
                }
                func_elunchbox_switch_to_lid_confirm();
            } else if (g_res.setup_done) {
                printf("elunchbox: reservation heating confirmed, switch to heat panel (awake)\n");
                func_elunchbox_switch_to_heat_panel();
            } else {
                printf("elunchbox: [DEBUG] awake heat_pending=1 BUT setup_done=0 sta=%u, skip\n",
                       func_cb.sta);
            }
        } else if (heat_pending && !g_res.setup_done
                   && !elunchbox_lid_confirm_is_armed()) {
            printf("elunchbox: [DEBUG] awake heat_pending=1 BUT setup_done=0 sta=%u, skip\n",
                   func_cb.sta);
        }
    }
#endif

#if USER_PT8028_KEY && SOFT_POWER_ON_OFF
    func_elunchbox_pwr_long_poll();  // 电源键长按检测
#endif
#if ELUNCHBOX_PANEL_EN
    elunchbox_ble_pending_sta_poll(); /* 预约/加热切页 */
#endif
#if FUNC_LUNCHBOX_UART_EN
    /* 须在加热页切完后再执行：预约与直接加热共用充电进保温 */
    heat_display_warm_charge_route_poll();
#endif
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
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN

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
#if VIDEO_PLAY_EN
        compo_video_exit_unlock(video);
#endif
        return;
    }
#endif
#if GUI_USE_SCREENSHOOT
    compo_form_t *frm_cur = NULL;
    func_switching3d_form_create(switch_mode, sta, &frm_cur, &frm);
    bool res = func_switching(switch_mode, NULL);                     //切换动画
    if (frm != NULL) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }
    compo_form_destroy(frm_cur);                                      //切换完成或取消，销毁窗体

#else
    if (mode != FUNC_SWITCH_FADE_OUT) {
        frm = func_create_form(sta);                                  //创建下一个任务的窗体
    }

    bool res = func_switching(switch_mode, NULL);                     //切换动画

    if (frm) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }
#endif

#if GUI_USE_SCREENSHOOT
//    func_cb.frm_main = func_create_form(func_cb.sta);                 //换回真实的窗体
    void (*enter)(void) = NULL;
    for (int i = 0; i < FUNC_ENTER_CNT; i++) {
        if (tbl_func_enter[i].func_idx == func_cb.sta) {
            enter = tbl_func_enter[i].func;
            enter();
            break;
        }
    }
#endif

    if (res) {
        if (sta == FUNC_CLOCK) {
            func_cb.flag_sort = false;
        }
        func_cb.sta = sta;
    }

#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}

//切换到下一个任务
void func_switch_next(bool flag_auto, bool flag_loop)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN

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
#if VIDEO_PLAY_EN
        compo_video_exit_unlock(video);
#endif
        return;
    }
#endif

#if GUI_USE_SCREENSHOOT
    compo_form_t *frm_cur = NULL;
    func_switching3d_form_create(switch_mode, sta, &frm_cur, &frm);
    bool res = func_switching(switch_mode, NULL);                     //切换动画
    if (NULL != frm) {
        compo_form_destroy(frm);                                      	  //切换完成或取消，销毁窗体
    }
    compo_form_destroy(frm_cur);                                      //切换完成或取消，销毁窗体
#else
    if (mode != FUNC_SWITCH_FADE_OUT) {
        frm = func_create_form(sta);                                  //创建下一个任务的窗体
    }

    bool res = func_switching(switch_mode, NULL);                     //切换动画

    if (frm) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }
#endif

#if GUI_USE_SCREENSHOOT
//    func_cb.frm_main = func_create_form(func_cb.sta);                 //换回真实的窗体
    void (*enter)(void) = NULL;
    for (int i = 0; i < FUNC_ENTER_CNT; i++) {
        if (tbl_func_enter[i].func_idx == func_cb.sta) {
            enter = tbl_func_enter[i].func;
            enter();
            break;
        }
    }
#endif

    if (res) {
        if (sta == FUNC_CLOCK) {
            func_cb.flag_sort = false;
        } else {
            func_cb.flag_sort = true;
        }
        func_cb.sta = sta;
    }

#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
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
    if (elunchbox_lowbat_active() && sta != FUNC_LOWBAT && sta != FUNC_PWROFF) {
        return;
    }
    /* 低电页优先：允许在 manual_off 下切入 FUNC_LOWBAT（先由 lowbat_enter 唤醒） */
    if (elunchbox_pwr_is_manual_off() && sta != FUNC_LOWBAT) {
        return;
    }
    if (sys_cb.flag_swithing) {
        return;
    }
    WDT_CLR();
#endif
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN

    compo_form_t *frm = NULL;

#if GUI_USE_SCREENSHOOT
    compo_form_t *frm_cur = NULL;
    if (sta == FUNC_SMARTSTACK) {
        frm = func_create_form(sta);
        switch_mode = (switch_mode & 0x8000) | FUNC_SWITCH_DIRECT;
    } else {
        func_switching3d_form_create(switch_mode, sta, &frm_cur, &frm);
    }
    bool res = func_switching(switch_mode, NULL);                     //切换动画
    if (frm) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }
    compo_form_destroy(frm_cur);                                      //切换完成或取消，销毁窗体
#else
    u8 mode = switch_mode & 0x7fff;
    /* ELUNCHBOX：DIRECT 和 FADE_OUT 都不在此预创建目标窗体。 */
    if (mode != FUNC_SWITCH_FADE_OUT && mode != FUNC_SWITCH_DIRECT) {
        frm = func_create_form(sta);
    }

    bool res = func_switching(switch_mode, NULL);

#if ELUNCHBOX_PANEL_EN
    if (res && mode != FUNC_SWITCH_DIRECT && mode != FUNC_SWITCH_FADE_OUT) {
        home_gpu_wait_idle();
    }
#endif

    if (frm) {
        compo_form_destroy(frm);
    }

#if ELUNCHBOX_PANEL_EN
    /* 子页：Home 控件多，destroy + compos_init 清软件池；勿 gpu_exit（keep_ram 恢复过期状态 → WDT） */
    if ((mode == FUNC_SWITCH_DIRECT || mode == FUNC_SWITCH_FADE_OUT) && func_cb.frm_main != NULL) {
        WDT_CLR();
        if (elunchbox_subpage_sta(sta)) {
            elunchbox_subpage_gpu_recycle_after_leave();
        } else {
            if (func_cb.sta == FUNC_NEW_MODE) {
                func_new_mode_pre_leave_cleanup();
            }
            home_gpu_wait_idle();
            WDT_CLR();
            func_key_lock_on_form_destroy();
            if (sta != FUNC_LOWBAT) {
                home_ui_lowbat_overlay_reset();
            }
            compo_form_destroy(func_cb.frm_main);
            compos_init();
            func_cb.frm_main = NULL;
        }
        if (res) {
            func_cb.sta = sta;
        }
    }
#endif

#endif

#if GUI_USE_SCREENSHOOT
//    func_cb.frm_main = func_create_form(func_cb.sta);                 //换回真实的窗体
    if (sta != FUNC_SMARTSTACK) {
        void (*enter)(void) = NULL;
        for (int i = 0; i < FUNC_ENTER_CNT; i++) {
            if (tbl_func_enter[i].func_idx == func_cb.sta) {
                enter = tbl_func_enter[i].func;
                enter();
                break;
            }
        }
    }
#endif

    if (res) {
        func_cb.sta = sta;
    }

#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}

//切换回主时钟
void func_switch_to_clock(void)
{
    func_switch_to(FUNC_CLOCK, FUNC_SWITCH_LR_ZOOM_RIGHT | FUNC_SWITCH_AUTO);
    func_cb.flag_sort = false;
}


//退回到主菜单
void func_switch_to_menu(void)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN
    u16 switch_mode;
    bool flag_frm_menu;                                                         //是否需要创建菜单窗体
    flag_frm_menu = true;
    if (func_cb.menu_style == MENU_STYLE_FOOTBALL) {
        switch_mode = FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO;;
        flag_frm_menu = false;
    } else if (func_cb.sta != FUNC_CLOCK || func_cb.menu_style == MENU_STYLE_HONEYCOMB) {
        switch_mode = FUNC_SWITCH_ZOOM_EXIT | FUNC_SWITCH_AUTO;
    } else if (func_cb.menu_style == MENU_STYLE_WATERFALL) {
        switch_mode = FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO;
        func_cb.flag_animation = true;                                          //淡出后进入入场动画
        flag_frm_menu = false;
    } else {
        switch_mode = FUNC_SWITCH_ZOOM_FADE_EXIT | FUNC_SWITCH_AUTO;
    }
    if (flag_frm_menu) {
        widget_icon_t *icon;
        compo_form_t *frm = func_create_form(FUNC_MENU);                        //创建下一个任务的窗体
#if (ASR_SELECT && ASR_VOICE_BALL_ANIM)
        component_t *compo = compo_get_next((component_t *)frm->anim);
#else
        component_t *compo = compo_get_next((component_t *)frm->title);
#endif

        if (compo->type == COMPO_TYPE_ICONLIST) {
            compo_iconlist_t *iconlist = (compo_iconlist_t *)compo;
            icon = compo_iconlist_select_byidx(iconlist, func_cb.menu_idx);
        } else if (compo->type == COMPO_TYPE_LISTBOX) {
            compo_listbox_t *listbox = (compo_listbox_t *)compo;
            icon = compo_listbox_select_byidx(listbox, func_cb.menu_idx);
        } else if (compo->type == COMPO_TYPE_DISKLIST) {
            compo_disklist_t *disklist = (compo_disklist_t *)compo;
            icon = compo_disklist_select_byidx(disklist, func_cb.menu_idx);
        } else if (compo->type == COMPO_TYPE_KALEIDOSCOPE) {
            compo_kaleidoscope_t *kale = (compo_kaleidoscope_t *)compo;
            icon = compo_kale_select_byidx(kale, func_cb.menu_idx);
        } else if (compo->type == COMPO_TYPE_RINGS) {
            compo_rings_t *rings = (compo_rings_t *)compo;
            icon = compo_rings_select_byidx(rings, func_cb.menu_idx);
            if (icon == NULL) {
                func_cb.menu_idx = func_menu_sub_skyrer_get_first_idx();
                icon = compo_rings_select_byidx(rings, func_cb.menu_idx);
            }
        } else {
//            printf("%s\n", __func__);
//            halt(HALT_GUI_COMPO_ICONLIST_TYPE);
//            return;
            icon = NULL;
        }
        if (icon == NULL) {
            switch_mode = FUNC_SWITCH_FADE_OUT;
        }
        func_switching(switch_mode, icon);                                      //退出动画
        compo_form_destroy(frm);                                                //切换完成或取消，销毁窗体
    } else {
        func_switching(switch_mode, NULL);                                      //退出动画
    }
    func_cb.sta = FUNC_MENU;
#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}

//上滑进入足球菜单
void func_switch_to_football_menu(void)
{
    func_cb.menu_style = MENU_STYLE_FOOTBALL;
    func_switch_to_menu();
}

//手动退回到主菜单
void func_switching_to_menu(void)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN

    widget_icon_t *icon;
    u16 switch_mode;
    compo_form_t *frm = func_create_form(FUNC_MENU);                            //创建下一个任务的窗体
#if (ASR_SELECT && ASR_VOICE_BALL_ANIM)
        component_t *compo = compo_get_next((component_t *)frm->anim);
#else
        component_t *compo = compo_get_next((component_t *)frm->title);
#endif
    if (compo->type == COMPO_TYPE_ICONLIST) {
        compo_iconlist_t *iconlist = (compo_iconlist_t *)compo;
        icon = compo_iconlist_select_byidx(iconlist, func_cb.menu_idx);
    } else if (compo->type == COMPO_TYPE_LISTBOX) {
        compo_listbox_t *listbox = (compo_listbox_t *)compo;
        icon = compo_listbox_select_byidx(listbox, func_cb.menu_idx);
    } else if (compo->type == COMPO_TYPE_DISKLIST) {
        compo_disklist_t *disklist = (compo_disklist_t *)compo;
        icon = compo_disklist_select_byidx(disklist, func_cb.menu_idx);
    } else if (compo->type == COMPO_TYPE_KALEIDOSCOPE) {
        compo_kaleidoscope_t *kale = (compo_kaleidoscope_t *)compo;
        icon = compo_kale_select_byidx(kale, func_cb.menu_idx);
    } else if (compo->type == COMPO_TYPE_RINGS) {
            compo_rings_t *rings = (compo_rings_t *)compo;
            icon = compo_rings_select_byidx(rings, func_cb.menu_idx);
            if (icon == NULL) {
                func_cb.menu_idx = func_menu_sub_skyrer_get_first_idx();
                icon = compo_rings_select_byidx(rings, func_cb.menu_idx);
            }
    } else {
//            printf("%s\n", __func__);
//            halt(HALT_GUI_COMPO_ICONLIST_TYPE);
//            return;
            icon = NULL;
    }
    if (func_cb.sta != FUNC_CLOCK || func_cb.menu_style == MENU_STYLE_HONEYCOMB) {
        switch_mode = FUNC_SWITCH_ZOOM_EXIT;
    } else {
        switch_mode = FUNC_SWITCH_ZOOM_FADE_EXIT;
    }

    if (icon == NULL) {
        switch_mode = FUNC_SWITCH_FADE_OUT;
    }

    bool res = func_switching(switch_mode, icon);                               //退出动画
    compo_form_destroy(frm);                                                    //切换完成或取消，销毁窗体
    if (res) {
        func_cb.sta = FUNC_MENU;
    }
#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}


//页面滑动回退功能
void func_backing_to(void)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN
    if (func_cb.sta == FUNC_BT_RING || func_cb.sta == FUNC_BT_CALL) {

        return;
    }

    u8 stack_top = task_stack_pop();

    if (!stack_top) {
#if ELUNCHBOX_PANEL_EN
        stack_top = FUNC_HOME;                                  //异常返回 Home
#else
        stack_top = FUNC_CLOCK;                                 //异常返回表盘
#endif
    }

    if (stack_top == FUNC_MENU
    ) {
        func_switching_to_menu();                               //右滑缓慢退出任务
    } else if (stack_top == FUNC_SMARTSTACK
    ) {
        func_switch_to(stack_top, FUNC_SWITCH_LR_ZOOM_RIGHT);   //返回上一个界面
    } else {
        func_switch_to(stack_top, func_get_switching_mode_byidx(sys_cb.nav_index, false));   //返回上一个界面
    }
    if (func_cb.sta != stack_top) {                             //如果页面没切换需要重新入栈
        task_stack_push(func_cb.sta);
    }
#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}

//页面按键回退功能
void func_back_to(void)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN
    if (func_cb.sta == FUNC_BT_RING || func_cb.sta == FUNC_BT_CALL) {
        return;
    }

    u8 stack_top = task_stack_pop();

    if (!stack_top) {
#if ELUNCHBOX_PANEL_EN
        stack_top = FUNC_HOME;                                  //异常返回 Home
#else
        stack_top = FUNC_CLOCK;                                 //异常返回表盘
#endif
    }

    if (stack_top == FUNC_MENU
    ) {
        func_switch_to_menu();                                  //返回主菜单
    } else if (stack_top == FUNC_SMARTSTACK
    ) {
        func_switch_to(stack_top, FUNC_SWITCH_LR_ZOOM_RIGHT | FUNC_SWITCH_AUTO);  //返回上一个界面
    }  else {
        func_switch_to(stack_top, func_get_switching_mode_byidx(sys_cb.nav_index, false) | FUNC_SWITCH_AUTO);    //返回上一个界面
    }
#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
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

//                CLKGAT0 |= BIT(16);                   //sd0_clken
//                CLKGAT0 |= BIT(21);
//                SD0_LDO_EN();
//
//                sd0_set_speed_mhz(2);
//                FRESULT res = fs_mount (&fat_fs, "B:", 1);
//                printf("f_mount:%d\n", res);

//#if FUNC_MUSIC_EN
//				func_cb.sta = FUNC_MUSIC;
//#endif // FUNC_MUSIC_EN
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
    #if BT_EMIT_EN
                    music_control(MUSIC_MSG_BT_PLAY);
    #endif
                    bsp_sys_unmute();
                }
                bt_cb.music_playing = true;
            }
            break;

        case EVT_A2DP_MUSIC_STOP:
#if !BT_EMIT_EN
            if (!sbc_is_bypass()) {
                printf("EVT_A2DP_MUSIC_STOP\n");
                bsp_sys_mute();
            }
            bt_cb.music_playing = false;
#endif
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

/* 表盘子界面（下拉/侧边/转盘等）仍占用 func_cb.sta==FUNC_CLOCK，但 f_clk->sta!=MAIN。
 * 此时若仍走 func_switch_next / func_switch_to_menu 等，会与 sub_frm、转场状态机冲突，易 WDT。 */
static bool func_clock_subui_active(void)
{
    if (func_cb.sta != FUNC_CLOCK || func_cb.f_cb == NULL) {
        return false;
    }
    return ((f_clock_t *)func_cb.f_cb)->sta != FUNC_CLOCK_MAIN;
}

//func common message process
void func_message(size_msg_t msg)
{
#if ELUNCHBOX_PANEL_EN && USER_PT8028_KEY
    if (func_key_lock_ku_blocked(msg)) {
        return;
    }
#endif
    switch (msg) {
    case MSG_CTP_SHORT_LEFT:
        if (func_cb.sta == FUNC_CLOCK) {
            if (!func_clock_subui_active()) {
                func_switch_next(false, true);                    //切到下一个任务
            }
        } else if (func_cb.flag_sort) {
            func_switch_next(false, true);
        }
        break;

    case MSG_CTP_SHORT_RIGHT:
        if (func_cb.sta == FUNC_CLOCK && func_clock_subui_active()) {
            break;
        }
        if (func_cb.flag_sort){
            func_switch_prev(false);                    //切到上一个任务
        } else if(func_cb.menu_style == MENU_STYLE_FOOTBALL) {
            func_back_to();
        } else /*if (ctp_get_sxy().x <= 32)*/ {
            func_backing_to();							//右滑缓慢退出任务
        }
        break;

    case MSG_CTP_COVER:
#if !ELUNCHBOX_KEEP_AWAKE
        sys_cb.sleep_delay = 1; //100ms后进入休眠
#endif
        break;

    case MSG_QDEC_FORWARD:
        if (func_cb.sta == FUNC_CLOCK) {
            if (!func_clock_subui_active()) {
                msg_queue_detach(MSG_QDEC_FORWARD, 0);  //防止不停滚动
                func_switch_next(true, true);                     //切到下一个任务
            }
        } else if (func_cb.flag_sort) {
            msg_queue_detach(MSG_QDEC_FORWARD, 0);
            func_switch_next(true, true);
        }
        break;

    case MSG_QDEC_BACKWARD:
        if (func_cb.sta == FUNC_CLOCK || func_cb.flag_sort) {
            msg_queue_detach(MSG_QDEC_BACKWARD, 0);
            if (func_cb.flag_sort != 0) {
                func_switch_prev(true);                     //切到下一个任务
            }
        }
        break;

    case KU_PREV:
        if (func_cb.sta != FUNC_NEW_HEAT) {
            func_switch_to(FUNC_NEW_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
        break;

    case KU_NEXT:
#if !FUNC_RESERVATION_UI_EN
        break;
#elif ELUNCHBOX_PANEL_EN
        /* 预约键：统一走 DIRECT + allow，避免 FADE_OUT 无 allow 被拒 */
        if (func_cb.sta != FUNC_RESERVATION) {
            func_elunchbox_switch_to_reservation();
        }
        break;
#else
        if (func_cb.sta != FUNC_RESERVATION) {
            func_switch_to(FUNC_RESERVATION, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
        break;
#endif

    case KU_BACK:
#if ELUNCHBOX_PANEL_EN
        if (func_cb.sta == FUNC_HOME) {
            break;
        }
        if (func_cb.sta == FUNC_NEW_HEAT || func_cb.sta == FUNC_NEW_WARM
            || func_cb.sta == FUNC_NEW_MODE || func_cb.sta == FUNC_NEW_SETUP
            || func_cb.sta == FUNC_NEW_LANGUAGE || func_cb.sta == FUNC_NEW_VERINFO
            || func_cb.sta == FUNC_NEW_TIME || func_cb.sta == FUNC_LID_CONFIRM) {
            break;
        }
#endif
        if (func_cb.flag_sort) {
            func_switch_to_clock();                     //切换回主时钟
        } else if (func_cb.sta == FUNC_CLOCK) {
            if (!func_clock_subui_active()) {
                func_switch_to_menu();                  //退回到主菜单
            }
        } else {
            func_back_to();								//直接退出任务
        }
        break;

#if SOFT_POWER_ON_OFF
        case KLH_BACK:
        case KLH_LEFT:
        case KLH_RIGHT:
#if ELUNCHBOX_PANEL_EN
            /* 饭盒：开关键长按由 func_elunchbox_pwr_long_poll 软开/软关 */
            break;
#else
            func_cb.sta = FUNC_PWROFF;
            break;
#endif
#endif

    case KU_MODE:
#if ELUNCHBOX_PANEL_EN
            /* 面板端：所有新 UI 页面模式键统一跳转到模式选择页 */
            func_switch_to(FUNC_NEW_MODE, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
            break;
#endif
            if (func_cb.sta == FUNC_HOME) {
                func_home_mode_key();
            } else if (func_cb.sta != FUNC_HEAT && func_cb.sta != FUNC_MODE &&
                       func_cb.sta != FUNC_SETUP && func_cb.sta != FUNC_RESERVATION &&
                       func_cb.sta != FUNC_TIMEING && func_cb.sta != FUNC_LANGUAGEING &&
                       func_cb.sta != FUNC_VERINFO) {
                func_cb.sta = FUNC_NULL;
            }
            break;

        case KL_BACK:   //堆栈后台
            /* 下拉等子界面内勿切智能堆栈：避免 sub_frm 与切换栈交错导致异常 PC（如 0xfff9xxxx） */
            if (func_cb.sta == FUNC_CLOCK && func_clock_subui_active()) {
                break;
            }
            if (bt_cb.disp_status < BT_STA_INCOMING && func_cb.sta != FUNC_MENUSTYLE) {
                if (func_cb.sta == FUNC_CLOCK) {
                    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
                    /* 仅在主表盘态销毁 sub_frm。FUNC_CLOCK_SUB_* 子界面在自有 while 内仍持有 sub_frm，
                     * 若此处销毁，子界面 exit 会二次 compo_form_destroy -> 堆损坏 / WDT。 */
                    if (f_clk != NULL && f_clk->sta == FUNC_CLOCK_MAIN && f_clk->sub_frm != NULL) {
                        compo_form_destroy(f_clk->sub_frm);     //下拉界面存在双窗体
                    }
                }

                if (func_cb.menu_style == MENU_STYLE_FOOTBALL) {
                    func_switching(FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO, NULL);
                    func_cb.sta = FUNC_SMARTSTACK;
                } else {
                    func_switch_to(FUNC_SMARTSTACK, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);
                }
            }
            break;


//        case KU_LEFT:
//            ble_bt_connect();               //ios一键双连测试
//            printf("send sm req\n");
//            break;

//        case KU_LEFT:
//            bt_audio_enable();
//            printf("bt_audio_enable\n");
//            break;
//
//        case KU_RIGHT:
//            bt_audio_bypass();
//            printf("bt_audio_bypass\n");
//            break;

//#if EQ_MODE_EN
//        case KU_EQ:
//            sys_set_eq();
//            break;
//#endif // EQ_MODE_EN
//
//        case KU_MUTE:
//            if (sys_cb.mute) {
//                bsp_sys_unmute();
//            } else {
//                bsp_sys_mute();
//            }
//            break;
//
        case MSG_SYS_500MS:
            break;

        case MSG_SYS_1S:
#if CALL_MGR_EN
            bsp_bt_call_times_inc();
            bsp_modem_call_times_inc();
#endif
#if BT_HFP_BAT_REPORT_EN
            bt_hfp_report_bat();
#endif
            break;

        // case EVT_4G_INIT_DONE:
        //     modem_test_machine();
        //     break;
#if BT_EMIT_EN
        case EVT_BT_DISCONNECT:
            if (func_music_is_play()) {
                func_music_play(false);
            }
            break;
#endif // BT_EMIT_EN

        default:
            evt_message(msg);
            break;
    }

#if (ASR_SELECT == ASR_YJ && !ASR_DEAL_TYPE)
    third_func_message(msg);
#endif

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
#if ELUNCHBOX_PANEL_EN
    elunchbox_user_activity_reset();
    printf("enter: sta=%d\n", func_cb.sta);
#else
    reset_sleep_delay_all();
#endif
    reset_pwroff_delay();
    printf("enter: chk1\n");
    func_cb.mp3_res_play = mp3_res_play;
    func_cb.set_vol_callback = NULL;
    printf("enter: chk2\n");
//    bsp_clr_mute_sta();
//    sys_cb.voice_evt_brk_en = 1;    //播放提示音时，快速响应事件。
    AMPLIFIER_SEL_D();
    printf("enter: chk3 before key_lock\n");
#if ELUNCHBOX_PANEL_EN
    func_key_lock_on_page_change();
#endif
    printf("enter: done\n");
}

AT(.text.func)
void func_exit(void)
{
#if ASR_SELECT
    bsp_asr_voice_wake_sta_clr();
#endif
#if ELUNCHBOX_PANEL_EN
    bool warm_from_heat = (func_cb.last == FUNC_HEAT && func_cb.sta == FUNC_NEW_WARM);
    printf("exit: frm_main=%p f_cb=%p last=%d sta=%d warm_from_heat=%d\n",
           func_cb.frm_main, func_cb.f_cb, func_cb.last, func_cb.sta, warm_from_heat);
    if (elunchbox_te_block_flag) {
        elunchbox_te_block_flag = 0;
    }
    if (!warm_from_heat) {
        home_gpu_wait_idle();
    }
    func_key_lock_on_form_destroy();
    if (func_cb.sta != FUNC_LOWBAT) {
        home_ui_lowbat_overlay_reset();
    }
    home_ui_shared_bt_detach_pic();
#endif
    //销毁窗体
    if (func_cb.frm_main != NULL) {
#if ELUNCHBOX_PANEL_EN
        printf("exit: destroy form\n");
#endif
            compo_form_destroy(func_cb.frm_main);
#if ELUNCHBOX_PANEL_EN
            printf("exit: destroyed\n");
            if (!warm_from_heat) {
                home_gpu_wait_idle();
                printf("exit: wait2 done\n");
            }
            compos_init();
            printf("exit: compos_init done\n");
            if (!warm_from_heat) {
                home_gpu_wait_idle();
                printf("exit: wait3 done\n");
            }
#endif
    }
    //释放FUNC控制结构体
    if (func_cb.f_cb != NULL) {
        func_free(func_cb.f_cb);
    }
    func_cb.frm_main = NULL;
    func_cb.f_cb = NULL;

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    if (func_cb.last == FUNC_ALIPAY) {
        void gui_sw_init(void);
        gui_sw_init();
        printf("gui_reinit\n");
    }
#endif

}

#if ELUNCHBOX_PANEL_EN && FUNC_LUNCHBOX_UART_EN
extern u16 lb_dp_encode_bool(u8 *buf, u8 dpid, u8 val);
extern u16 lb_dp_encode_value(u8 *buf, u8 dpid, u32 val);
extern void lb_uart_send_raw(u8 uart_cmd, u8 *data, u16 data_len, bool no_wait);

/**
 * 上电先问加热模块是否在充电：是则直接进黑屏跑马灯，避免先闪主页再跳。
 * （关机插电常触发 WKUP/WDT 整机重启，只能在开机路径直接落地充电页）
 */
static bool elunchbox_boot_probe_charging(void)
{
    u8 data[24];
    u8 *p = data;
    u16 len;
    int i;

#if CHARGE_EN
    if (CHARGE_DC_IN()) {
        home_ui_shared_battery_charge_apply(1);
        printf("elunchbox: boot probe DC_IN=1\n");
        return true;
    }
#endif

    lb_uart_tx_block(false);
    p += lb_dp_encode_value(p, LB_DPID_TIME_SYNC, lb_get_unix_time());
    p += lb_dp_encode_bool(p, LB_DPID_POWER_SWITCH, 1);
    len = (u16)(p - data);
    lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, len, true);
    printf("elunchbox: boot probe query DYNAMIC\n");

    for (i = 0; i < 100; i++) {   /* 最长约 1s */
        WDT_CLR();
        lunchbox_uart_process();
        if (home_ui_shared_battery_is_charging()) {
            printf("elunchbox: boot probe Charge=1 (i=%d)\n", i);
            return true;
        }
        delay_5ms(1);
    }
    printf("elunchbox: boot probe no charge\n");
    return false;
}
#endif

AT(.text.func)
void func_run(void)
{
#if CPU_USAGE_MONITOT_EN
    cpu_trace_monitor_init();
#endif

    void (*func_entry)(void) = NULL;
    printf("%s\n", __func__);
    memset(func_cb.tbl_sort, 0, sizeof(func_cb.tbl_sort));
#if ELUNCHBOX_PANEL_EN
    func_cb.tbl_sort[0] = FUNC_HOME;
    func_cb.sort_cnt = 1;
    func_cb.flag_sort = false;
    func_cb.sta = FUNC_HOME;
#else
    func_cb.tbl_sort[0] = FUNC_HOME;
    func_cb.tbl_sort[1] = FUNC_VIDEO_SHOWLIST;
    func_cb.tbl_sort[2] = FUNC_ACTIVITY;
    func_cb.tbl_sort[3] = FUNC_SLEEP;
    func_cb.tbl_sort[4] = FUNC_BLOOD_OXYGEN;
    func_cb.tbl_sort[5] = FUNC_BT;
    func_cb.tbl_sort[6] = FUNC_COMPO_SELECT;
    func_cb.sort_cnt = 7;
    func_cb.sta = DEFAULE_START_FUNC;
#endif
    task_stack_init();  //任务堆栈
    latest_task_init(); //最近任务
#if ELUNCHBOX_PANEL_EN
    elunchbox_pwr_gui_off = false;
    elunchbox_pwr_manual_off = false;
    home_ui_shared_battery_boot_init();
    elunchbox_user_activity_reset();
    elunchbox_lid_confirm_arm_boot();
    elunchbox_boot_tick = tick_get();
    elunchbox_boot_charge_check = true;
#if FUNC_LUNCHBOX_UART_EN
    /* 开机先探测充电：直接进黑屏跑马灯，不先进 Home */
    if (elunchbox_boot_probe_charging()) {
        home_ui_shared_battery_charge_apply(1);
        func_cb.sta = FUNC_CHARGE;
        elunchbox_boot_charge_check = false;
        printf("elunchbox: boot -> FUNC_CHARGE direct (black marquee)\n");
    } else {
        printf("elunchbox: boot charge check window %ums (fallback)\n",
               (unsigned)ELUNCHBOX_BOOT_CHARGE_WINDOW_MS);
    }
#else
    printf("elunchbox: boot charge check window %ums\n",
           (unsigned)ELUNCHBOX_BOOT_CHARGE_WINDOW_MS);
#endif
#endif
    // func.c
    
    for (;;) {
#if !ELUNCHBOX_PANEL_EN
        printf("func_enter <<\n");
#endif
#if ELUNCHBOX_PANEL_EN
        printf("run: enter sta=%d frm=%p f_cb=%p\n", func_cb.sta, func_cb.frm_main, func_cb.f_cb);
#endif
        func_enter();
#if !ELUNCHBOX_PANEL_EN
        printf("pwrkey usage_id: %d\n", bsp_pwrkey_get_usage_id());
#endif
#if ELUNCHBOX_PANEL_EN
        printf("run: find_entry sta=%d\n", func_cb.sta);
#endif
        for (int i = 0; i < FUNC_ENTRY_CNT; i++) {
            if (tbl_func_entry[i].func_idx == func_cb.sta) {
                task_stack_push(func_cb.sta);
                latest_task_add(func_cb.sta);
                func_entry = tbl_func_entry[i].func;
                WDT_CLR();
                printf("run: call func_entry=%p\n", func_entry);
                func_entry();
                printf("run: func_entry returned\n");
                break;
            }
        }
#if !ELUNCHBOX_PANEL_EN
        printf("func_cb.sta:%d\n", func_cb.sta);
        if (func_cb.sta == FUNC_PWROFF) {
            printf("func_pwroff <<\n");
            func_pwroff(1);
            printf("func_pwroff >>\n");
        }
        printf("func_exit <<\n");
#endif
        if (func_cb.sta == FUNC_PWROFF) {
            func_pwroff(1);
        }
        func_exit();
#if !ELUNCHBOX_PANEL_EN
        printf("func_exit >>\n");
#endif
    }
}
