#ifndef __LOWPWR_H
#define __LOWPWR_H

#include <stdbool.h>
#include <stdint.h>

/*
 * 类型依赖: u8, u16, u32, bool 等由 include.h 提供。
 * 本头文件假设上下文已包含这些类型 (通过 include.h)。
 * 若独立使用, 需先 include <stdint.h> 并自行 typedef。
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 低功耗模块 — 从 func_lowpwr.c 重构
 *
 * === 进入低功耗的两种方式 ===
 *
 * 1) 5分钟无操作自动深睡:
 *    lowpwr_tick() 在 100ms 中断递减 guioff_delay/sleep_delay
 *    → 倒计时到0 → lowpwr_sleep_process() → bt_is_allow_sleep() 允许
 *    → lowpwr_enter_deepsleep()
 *
 * 2) 长按开关机键关机:
 *    PT8028 TCH5 长按 → func_cb.sta = FUNC_PWROFF
 *    → lowpwr_pwroff() → 断BT → 熄屏 → lowpwr_pwrdown()
 *
 * === 唤醒源 (3种) ===
 *
 *   ┌──────────┬──────────────────────────────┬─────────────────────┐
 *   │ 唤醒源   │ IO 配置 (sleep_wakeup_config) │ 检测方式            │
 *   ├──────────┼──────────────────────────────┼─────────────────────┤
 *   │ 按键     │ port_wakeup_init(PF1, 上拉,   │ port_wakeup_get_    │
 *   │ PF1/PF2  │              下降沿)          │ status() → wkpnd    │
 *   ├──────────┼──────────────────────────────┼─────────────────────┤
 *   │ WKO 脚   │ wko_wakeup_init(下降沿)       │ RTCCON9/10 BIT(2)   │
 *   │          │                              │ + port_wko_is_wkup() │
 *   ├──────────┼──────────────────────────────┼─────────────────────┤
 *   │ 串口/BLE │ BSP 自带 (co_timer / bt)      │ ble_app_need_wkup() │
 *   │ /来电    │                              │ / bt_cb.call_type   │
 *   └──────────┴──────────────────────────────┴─────────────────────┘
 *
 * === 休眠时 IO 状态 (lowpwr_enter_deepsleep 内处理) ===
 *
 *   GPIOADE = BIT(7) 或 0    PA7 rst 保留
 *   GPIOBDE = BIT(3)         PB3 UART TX
 *   GPIOGDE = 0x3F           MCP FLASH
 *   GPIOEDE = sensor?I2C:0   sensor I2C 保留或全关
 *   GPIOFDE = pf_keep        sensor PG + modem 脚
 *   WKUPCON &= ~BIT(16)      关 WKIE (防误唤醒)
 *   → on_wakeup_config()     配置按键/WKO 唤醒源
 *   → 休眠主循环...
 *   → on_wakeup_exit()       恢复 IO
 *   WKUPCON |= wkie          恢复 WKIE
 *
 * 依赖: JL SDK BSP 层 (include.h 提供的 BT/BLE/GUI/SARADC 等 API)
 * 去掉了对 func.h / sys_cb / func_cb 的直接耦合,
 * 状态和回调由应用层通过 lowpwr_t 注入。
 *
 * 全局变量: vddio_sleep_level (BSP 回调 sys_enter_sleep_vddio_level 使用)
 *           应用层直接设置此全局变量即可控制休眠 VDDIO 电压等级。
 * ============================================================ */

/* VDDIO 休眠电压等级, step=0.1V, 0=2.4V (默认), 非0时不灭屏 */
extern u8 vddio_sleep_level;

/* --- 休眠状态 (原 sys_cb 分散字段) --- */
typedef struct {
    int32_t  sleep_delay;        /* 深度休眠倒计时 (100ms tick) */
    int32_t  guioff_delay;       /* 熄屏倒计时 (100ms tick) */
    int32_t  pwroff_delay;       /* 自动关机倒计时 (100ms tick) */
    uint32_t sleep_time;         /* 配置: 休眠时间 */
    uint32_t pwroff_time;         /* 配置: 自动关机时间 */
    uint32_t sleep_wakeup_time;  /* 休眠最大时长 */
    uint32_t sleep_counter;      /* 休眠内 500ms 计数 */
    uint16_t vbat;               /* 电池电压 mV */
    uint8_t  sleep_en      : 1;  /* 是否允许进入休眠 */
    uint8_t  is_sleep_off_gui : 1; /* 仅熄屏,不进深度休眠 */
    uint8_t  gui_sleep_sta : 1;  /* GUI 是否在休眠态 */
    uint8_t  gui_need_wakeup : 1;/* 需要唤醒 GUI */
    uint8_t  flag_sleep_ble : 1; /* BLE 连接状态标记 */
    uint8_t  flag_shipping : 1;  /* 船运模式 */
    uint8_t  poweron_flag  : 1;  /* PWRKEY 开机标志 */
    uint8_t  chg_on        : 1;  /* 充电打开状态 */
    uint8_t  charge_sta;         /* 充电状态: 0=关 1=开 2=满 */
    uint8_t  vol;                /* 系统音量 */
} lowpwr_state_t;

/* --- 应用层回调 (替代 func.h / func_cb 耦合) --- */
typedef struct {
    /* 当前是否允许休眠 (例如: 加热中 bt_is_allow_sleep 返回 false) */
    bool (*is_allow_sleep)(void);
    /* 获取当前界面状态码 (原 func_cb.sta) */
    uint8_t (*get_ui_sta)(void);
    /* 设置界面状态码 (例如关机时设置 FUNC_PWROFF) */
    void (*set_ui_sta)(uint8_t sta);
    /* 关机前的准备 (锁存编码器状态等，原 lock_code_pwrsave) */
    void (*on_pwroff_lock)(void);
    /* 检查充电中是否允许关机 */
    bool (*power_off_check)(void);
    /* gui_sleep 前的 psram 检查 */
    void (*gui_sleep_psram_check)(void);

    /* ======== 唤醒 IO 配置 (原 plugin.c 的 sleep_wakeup_config/exits) ======== */

    /* 进入深睡前配置唤醒 IO (下降沿 + 上拉)
     * 典型实现:
     *   WKUPCON |= BIT(17);                           // wakup sniff enable
     *   wko_wakeup_init(1);                           // WKO 下降沿唤醒
     *   port_wakeup_init(IO_PF1, 1, 1);               // PF1 上拉, 下降沿
     *   port_wakeup_init(IO_PF2, 1, 1);               // PF2 上拉, 下降沿
     *   port_int_disable_to_sleep();
     */
    void (*on_wakeup_config)(void);

    /* 退出深睡后恢复 IO
     * 典型实现:
     *   wko_wakeup_exit();
     *   port_wakeup_exit(IO_PF1);
     *   port_wakeup_exit(IO_PF2);
     *   io_key_init();
     *   port_int_enable_exit_sleep();
     *   WKUPCON &= ~BIT(17);                           // wakup sniff disable
     */
    void (*on_wakeup_exit)(void);

    /* 关机前准备 wakeup IO (原 pt8028_port_pwrdown_wake_prep)
     * 配置 PE1(OUT_FLAG)/PE0+BCD 脚的数字输入+上拉,
     * 确保冷启动后能读到按键状态 */
    void (*on_pwrdown_wake_prep)(void);

} lowpwr_app_t;

/* --- 模块上下文 --- */
typedef struct lowpwr_s {
    lowpwr_state_t *state;
    const lowpwr_app_t *app;
    /* 充电功能是否使能 */
    bool charge_enabled;
    /* 是否 LP_XOSC 时钟模式 */
    bool lp_xosc_clock_en;
    /* 是否软开关方案 */
    bool soft_power_on_off;
    /* 是否 PWRKEY 模拟硬开关 */
    bool pwrkey_2_hw_pwron;
} lowpwr_t;

/* 说明: FUNC_PWROFF / FUNC_CLOCK 由 func.h 枚举提供,
 *        lowpwr.h 通过 func.h 被 include, 直接使用即可 */

/* --- 便利宏 --- */
#define LPWR_GUIOFF_DELAY_INIT(ctx)  ((ctx)->state->guioff_delay  = (ctx)->state->sleep_time)
#define LPWR_SLEEP_DELAY_INIT(ctx)   ((ctx)->state->sleep_delay   = (ctx)->state->sleep_time)
#define LPWR_DELAY_INIT_ALL(ctx)     do { \
    (ctx)->state->sleep_delay  = (ctx)->state->sleep_time; \
    (ctx)->state->guioff_delay = (ctx)->state->sleep_time; \
} while(0)
#define LPWR_PWROFF_DELAY_INIT(ctx)  ((ctx)->state->pwroff_delay  = (ctx)->state->pwroff_time)
#define LPWR_PWROFF_DELAY_KILL(ctx)  ((ctx)->state->pwroff_delay  = -1L)
#define LPWR_DELAY_KILL(ctx)         ((ctx)->state->sleep_delay   = -1L)

#define LPWR_PWROFF_RESET(ctx)       ((ctx)->state->pwroff_delay = (ctx)->state->pwroff_time)
#define LPWR_SLEEP_RESET(ctx)        ((ctx)->state->sleep_delay  = (ctx)->state->sleep_time)

/* 仅熄屏不休眠 */
#define LPWR_GUI_OFF_ONLY(ctx)       ((ctx)->state->is_sleep_off_gui = true)
#define LPWR_GUI_OFF_ONLY_DIS(ctx)   ((ctx)->state->is_sleep_off_gui = false)
#define LPWR_IS_GUI_OFF_ONLY(ctx)    ((ctx)->state->is_sleep_off_gui)

/* ============================================================
 * 公开 API
 * ============================================================ */

/**
 * 初始化 lowpwr 模块
 * @param ctx       模块上下文 (应用层分配)
 * @param state     状态结构 (应用层分配, 持久保留)
 * @param app       应用回调 (含 wakeup IO 配置)
 * @param sleep_time_100ms  配置的休眠时间 (0=禁用, 单位100ms)
 */
void lowpwr_init(lowpwr_t *ctx, lowpwr_state_t *state,
                 const lowpwr_app_t *app, uint32_t sleep_time_100ms);

/**
 * 100ms 定时 tick — 递减各倒计时
 * 由 BSP 层 100ms 定时中断调用 (原 lowpwr_tout_ticks)
 */
void lowpwr_tick(lowpwr_t *ctx);

/**
 * 休眠决策 + 执行 — 主循环中调用 (原 sleep_process)
 *
 * 逻辑:
 *   1. GUI 有待唤醒请求 → gui_wakeup() 亮屏
 *   2. BLE/视频/摄像头 活跃 → 重置倒计时, 不进休眠
 *   3. is_allow_sleep() 允许 且 sleep_delay==0 → lowpwr_enter_deepsleep()
 *   4. guioff_delay==0 且 GUI未睡 → gui_sleep(false) 仅熄屏
 *
 * @return true  已进入并退出深睡
 *         false 未深睡或仅熄屏
 */
bool lowpwr_sleep_process(lowpwr_t *ctx);

/**
 * 进入深度休眠 (熄屏 + 关外设 + 等唤醒) (原 sfunc_sleep)
 *
 * 流程:
 *   BT 进sleep → BLE 降参数 → 关 DAC/ADC → 关 PLL
 *   → 保存 GPIO → 仅保留关键 IO (PA7/PB3/Gx)
 *   → on_wakeup_config() 配置按键/WKO 唤醒
 *   → 休眠主循环 (等 port_wakeup / wko / ble / timer / call 唤醒)
 *   → on_wakeup_exit() 恢复 IO
 *   → 恢复 PLL/DAC/ADC/BT → gui_wakeup()
 */
void lowpwr_enter_deepsleep(lowpwr_t *ctx);

/**
 * 进入省电关机 — power down mode (原 sfunc_pwrdown)
 * @param vusb_wakeup_en  是否使能 VUSB 充电唤醒
 */
void lowpwr_pwrdown(lowpwr_t *ctx, uint8_t vusb_wakeup_en);

/**
 * 低电关机 — sleep mode (原 sfunc_lowbat)
 */
void lowpwr_lowbat_shutdown(lowpwr_t *ctx);

/**
 * 软关机流程 (原 func_pwroff)
 *
 * 流程:
 *   on_pwrdown_wake_prep() 准备唤醒 IO
 *   → 断开 BT/BLE → 熄屏 → 等待 PWRKEY 松开
 *   → 充电检测 → lowpwr_pwrdown() 或 lowpwr_lowbat_shutdown()
 *
 * @param pwroff_tone_en  是否播放关机提示音
 */
void lowpwr_pwroff(lowpwr_t *ctx, int pwroff_tone_en);

#ifdef __cplusplus
}
#endif

#endif /* __LOWPWR_H */
