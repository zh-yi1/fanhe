/**
 * lowpwr.c — 低功耗模块实现
 *
 * 从 func_lowpwr.c 重构:
 *   - sys_cb 分散字段 → lowpwr_state_t
 *   - func.h / func_cb 耦合 → lowpwr_app_t 回调
 *   - sleep_wakeup_config/exits → app->on_wakeup_config/on_wakeup_exit
 *   - BSP API (bt_/ble_/gui_/saradc_ 等) 通过 include.h 保留
 *
 * === 进入低功耗 ===
 *   5分钟无操作: tick 递减 sleep_delay → 到0 → sleep_process → enter_deepsleep
 *   长按开关机:   KLH_BACK → FUNC_PWROFF → lowpwr_pwroff()
 *
 * === 唤醒 ===
 *   按键 PF1/PF2: port_wakeup_init(下降沿+上拉) → port_wakeup_get_status()
 *   WKO 脚:       wko_wakeup_init(下降沿) → RTCCON9/10 & BIT(2)
 *   串口/BLE/来电: BSP 自带 → ble_app_need_wakeup / bt_cb.call_type
 *
 * === IO 休眠配置 ===
 *   GPIOADE = BIT(7)或0   PA7 rst
 *   GPIOBDE = BIT(3)      PB3 UART TX
 *   GPIOGDE = 0x3F        MCP FLASH
 *   GPIOEDE = sensor?I2C:0
 *   GPIOFDE = pf_keep     sensor PG + modem
 *
 * 注意: sys_enter_sleep_vddio_level / sys_sleep_cb / sys_sleep_backup_cb
 *       / sys_sleep_restore_cb 是 BSP 层固定签名的回调, 保留为全局函数。
 *       vddio_sleep_level 保留为全局变量。
 */

#include "include.h"
#include "lowpwr.h"
#if ELUNCHBOX_PANEL_EN
#include "elunchbox_lp.h"
#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#endif
#endif

/* ============================================================
 * BSP 回调所需的全局变量
 * ============================================================ */

/* VDDIO 休眠电压等级 (BSP sys_enter_sleep_vddio_level 返回此值)
 * step=0.1V, 0=2.4V (默认), 非0时不灭屏 */
u8 vddio_sleep_level = 0;

/* ============================================================
 * 内部引用 — 仅保留 SDK 头文件未声明的函数
 * ============================================================ */
extern u8 *cache_backup;
extern u32 __dynamic_pool_start, __dynamic_pool_end;
extern bool lp_xosc_check(void);
extern u32  get_sleep_proc_delay(void);
extern bool btstack_audio_is_busy(void);
extern bool keep_ram_tbl_load(void);
extern bool keep_ram_tbl_restore(void);
extern void rtc_sleep_enter(void);
extern void rtc_sleep_exit(void);
extern void rtc_sleep_process(void);
extern void rtc_rc_sleep_enter(void);
extern void sniff_rc_init(void);
extern void lp_xosc_en(void);
extern void rtc_printf(void);
extern void image_sensor_drv_enter_pwdn(void);
extern void image_sensor_drv_auto_reg(void);
extern void bsp_image_sensor_pwdn(void);
extern bool bsp_image_sensor_is_init(void);
extern void bsp_video_play_uninit(void);
extern void charge_set_detect_cnt(u8 cnt);
extern void dac_aubuf_init(void);
extern void fpga_uart_reinit(void);
extern void customer_heap_init(void);
extern u8 cfg_bt_sniff_clk_sel;

/* RAM 备份使用 func_lowpwr.c 的 sys_backup_buf[32K] */

/* ============================================================
 * 模块初始化
 * ============================================================ */
void lowpwr_init(lowpwr_t *ctx, lowpwr_state_t *state,
                 const lowpwr_app_t *app, uint32_t sleep_time_100ms)
{
    memset(state, 0, sizeof(*state));
    ctx->state = state;
    ctx->app   = app;

    state->sleep_en     = 1;   /* 默认允许休眠, ELUNCHBOX_KEEP_AWAKE 可覆盖 */
    state->pwroff_time  = -1L;
    state->pwroff_delay = -1L;

    if (sleep_time_100ms != 0) {
        state->sleep_time   = sleep_time_100ms * 10;
        state->sleep_delay  = sleep_time_100ms * 10;  /* 倒计时从 sleep_time 开始 */
        state->guioff_delay = sleep_time_100ms * 10;
    } else {
        state->sleep_time   = -1L;
        state->sleep_delay  = -1L;
        state->guioff_delay = -1L;
    }
}

/* ============================================================
 * 100ms tick — 倒计时递减
 * ============================================================ */
AT(.com_text.sleep)
void lowpwr_tick(lowpwr_t *ctx)
{
    /* func_run() 初始化前 g_lowpwr.state 为 NULL, 提前返回 */
    if (ctx == NULL || ctx->state == NULL) return;

    lowpwr_state_t *s = ctx->state;

    if (s->sleep_delay != -1L && s->sleep_delay > 0) {
        s->sleep_delay--;
    }
    if (s->guioff_delay != -1L && s->guioff_delay > 0) {
        s->guioff_delay--;
    }
    if (s->pwroff_delay != -1L && s->pwroff_delay > 0) {
        s->pwroff_delay--;
    }

    /* 回写到 sys_cb, 兼容旧宏 + BSP 回调 (sys_sleep_check 等读取 sys_cb 字段) */
    sys_cb.sleep_delay       = s->sleep_delay;
    sys_cb.guioff_delay      = s->guioff_delay;
    sys_cb.pwroff_delay      = s->pwroff_delay;
    sys_cb.sleep_en          = s->sleep_en;
    sys_cb.sleep_wakeup_time = s->sleep_wakeup_time;

#if ELUNCHBOX_PANEL_EN
    elunchbox_guioff_sleep_delay_tick();
    elunchbox_guioff_idle_tick();
#endif
}

/* ============================================================
 * 休眠时间校验
 * ============================================================ */
AT(.com_text.sleep)
bool lowpwr_sleep_time_check(lowpwr_t *ctx, u32 *sleep_time)
{
    u32 co_min = co_timer_get_min_time(true) * 2;

    if (*sleep_time > co_min) {
        *sleep_time = co_min;
    }
    if (*sleep_time < 4) {
        *sleep_time = 4;
    }
    if (*sleep_time > ctx->state->sleep_wakeup_time) {
        *sleep_time = ctx->state->sleep_wakeup_time;
        return true;
    }
    return false;
}

/* ============================================================
 * BLE 休眠参数检查
 * ============================================================ */
AT(.sleep_text.sleep)
static void sleep_ble_param_check(lowpwr_t *ctx)
{
    lowpwr_state_t *s = ctx->state;

    if (s->flag_sleep_ble != ble_is_connect()) {
        if ((s->flag_sleep_ble == false) && ble_is_connect()) {
            ble_update_conn_param(410, 0, 500);  /* 512.5ms interval */
        }
        s->flag_sleep_ble = ble_is_connect();
    }
}

/* ============================================================
 * 休眠内 500ms 定时器回调 (由 BSP sleep 循环调用)
 *
 * 返回值: 0=继续休眠  1=关机  2=低电唤醒
 * ============================================================ */
AT(.sleep_text.sleep.timer)
static uint32_t sleep_timer_cb(lowpwr_t *ctx)
{
    lowpwr_state_t *s = ctx->state;
    uint32_t ret = 0;

    s->sleep_counter++;
    if (s->sleep_counter == 10) {
        s->sleep_counter = 0;
        sys_cb.sleep_counter = 0;

#if LPWR_WARNING_VBAT
        u32 rtccon8 = RTCCON8;
#if CHARGE_VOL_DYNAMIC_DET
        if (RTCCON & BIT(20)) {                 /* vusb online? */
            gradient_process(1);
            RTCCON8 |= BIT(1);                  /* charge stop */
            delay_5ms(2);
            gradient_process(0);
        }
#endif
        saradc_init();
        saradc_adc15_analog_channel_sleep();
        saradc_adc15_ana_set_channel(ADCCH15_ANA_BG);
        delay_us(600);
        saradc_kick_start(0, 0);
        while (!bsp_saradc_process(0));
        s->vbat = bsp_vbat_get_voltage(1);
        RTCCON8 = rtccon8;
        if (s->vbat < LPWR_WARNING_VBAT) {
            ret = 2; /* 低电需要唤醒 */
        }
        saradc_exit();
#endif
    }

    if (ctx->charge_enabled) {
        charge_detect(0);
    }

    s->sleep_wakeup_time = -1L;
    sys_cb.sleep_wakeup_time = -1L;

    if (s->pwroff_delay != -1L) {
        if (s->pwroff_delay > 5) {
            s->pwroff_delay -= 5;
        } else {
            s->pwroff_delay = 0;
            return 1;  /* 自动关机 */
        }
    }

    if (ctx->pwrkey_2_hw_pwron && (!IS_PWRKEY_PRESS())) {
        ret = 1;
    }

    rtc_sleep_process();
    sleep_ble_param_check(ctx);

    return ret;
}

/* ============================================================
 * 休眠主循环 — 全部代码在 retention RAM 中运行
 *
 * 返回: true = GUI 需要唤醒
 * ============================================================ */
AT(.sleep_text.rodata.wakeup)
static const char str_port_wkup[]  = "port wakeup: %x\n";
AT(.sleep_text.rodata.wakeup)
static const char str_wko_wkup[]   = "wko wakeup\n";
AT(.sleep_text.rodata.wakeup)
static const char str_app_wkup[]   = "ble_app_need_wakeup\n";
AT(.sleep_text.rodata.wakeup)
static const char str_cotimer_wkup[] = "co_timer_pro_wakeup\n";
AT(.sleep_text.rodata.wakeup)
static const char str_call_wkup[]  = "call_wakeup\n";

AT(.sleep_text.sleep.proc)
static bool sleep_proc_loop(lowpwr_t *ctx)
{
    lowpwr_state_t *s = ctx->state;
    uint32_t status, ret;
    u32 wkpnd;
    bool wko_wkup_flag;
    bool gui_need_wkp = false;
    u32  vddio_vddcore_level = PWRCON0 & 0x1ff;
    u8   vddio_level = vddio_sleep_level;

    extern u8 sys_backup_buf[];
    cache_backup = sys_backup_buf;
    if (cache_backup == NULL) {
        printf("==>sleep malloc memory failed!\n");
        return gui_need_wkp;
    }

    /* 休眠期间降核心电压至 1.0V */
    PWRCON0 = (PWRCON0 & ~0x1f) | 12;           /* 12: 1.0V, 0.025V/step */
    if (vddio_level) {
        PWRCON0 = (PWRCON0 & ~(BIT(5) * 0xF)) | BIT(5) * vddio_level;
    }

    s->sleep_counter = 0;
    s->sleep_wakeup_time = -1L;
    sys_cb.sleep_counter = 0;           /* BSP bt_sleep_proc 读 sys_cb, 同步 */
    sys_cb.sleep_wakeup_time = -1L;

    while (bt_is_sleep()) {
        WDT_CLR();
        bt_thread_check_trigger();
        status = bt_sleep_proc();

#if SENSOR_HUB_EN
        bsp_senshb_lp_process();
#endif
        if (status == 1) {
            ret = sleep_timer_cb(ctx);
            if (ret) {
                if (ret == 1) {
                    ctx->app->set_ui_sta(FUNC_PWROFF);
                }
                break;
            }
        }

        wkpnd = port_wakeup_get_status();
        wko_wkup_flag = port_wko_is_wakeup();

        port_int_sleep_process(&wkpnd);
        bsp_sensor_step_lowpwr_pro();

        if (wkpnd) {
            printf(str_port_wkup, wkpnd);
            gui_need_wkp = true;
            break;
        }
        if ((RTCCON9 & BIT(2)) || (RTCCON10 & BIT(2)) || wko_wkup_flag) {
            printf(str_wko_wkup);
            gui_need_wkp = true;
            break;
        }
#if LE_EN
        if (ble_app_need_wakeup()) {
            printf(str_app_wkup);
            gui_need_wkp = true;
            break;
        }
#endif
        if (co_timer_pro(true)) {
            printf(str_cotimer_wkup);
            break;
        }
        if (bt_cb.call_type) {
            printf(str_call_wkup);
            gui_need_wkp = true;
            break;
        }
    }

    PWRCON0 = (PWRCON0 & ~0x1ff) | vddio_vddcore_level;
    return gui_need_wkp;
}

/* ============================================================
 * sys_sleep_cb — BSP sleep 进出回调
 * ============================================================ */
AT(.sleep_text.sleep.backup)
void sys_sleep_backup_cb(void)
{
    /* 应用层可在此补充 retention RAM 备份 */
}

AT(.sleep_text.sleep.restore)
void sys_sleep_restore_cb(void)
{
    /* 应用层可在此补充恢复 */
}

/* VDDIO 休眠电压 — BSP 回调, 固定签名 */
AT(.sleep_text.sleep)
u8 sys_enter_sleep_vddio_level(void)
{
    return vddio_sleep_level;
}

/* BLE 唤醒准入 — BSP 回调, 固定签名 */
AT(.com_text.sleep)
bool ble_is_allow_wkup(void)
{
    return LE_ALLOW_WKUP_EN;
}

AT(.sleep_text.sleep)
void sys_sleep_cb(u8 lpclk_type)
{
    /* 此函数只能调用 sleep_text 或 com_text 函数 */
    u32 gpiogde = GPIOGDE;
    if (gpiogde & BIT(6)) {
        GPIOGDE = BIT(2) | BIT(4) | BIT(6);     /* SPICS, SPICLK */
    } else {
        GPIOGDE = BIT(2) | BIT(4);
    }

    sys_enter_sleep(lpclk_type);                /* 进入硬件休眠 */

    GPIOGDE = gpiogde;
}

/* 休眠态切系统时钟 (用于抬高主频跑算法, 用完需切回 24M) */
void sleep_set_sysclk(uint8_t sys_clk)
{
    uint8_t cur_sys_clk = sys_clk_get();

    if (sys_clk < SYS_24M || cur_sys_clk == sys_clk) {
        return;
    }

    if (sys_clk > SYS_24M) {
        if (cur_sys_clk <= SYS_24M) {
            CLKCON0 = (CLKCON0 & ~(0x03 << 2)) | (0x01 << 2);
            RSTCON0 &= ~BIT(4);
            adpll_init(DAC_OUT_SPR);
            adda_clk_source_sel(0);
        }
        sys_clk_set(sys_clk);
    } else {
        sys_clk_set(SYS_24M);
        DACDIGCON0 &= ~BIT(0);
        adda_clk_source_sel(1);
        PLL0CON0 &= ~(BIT(18) | BIT(6));
    }
}

/* ============================================================
 * lowpwr_enter_deepsleep — 进入深度休眠
 * ============================================================ */
void lowpwr_enter_deepsleep(lowpwr_t *ctx)
{
    lowpwr_state_t *s = ctx->state;
    uint32_t usbcon0, usbcon1;
    u16 pa_de, pb_de, pe_de, pf_de, pg_de;
    u16 adc_ch;
    uint32_t sysclk;
    u32 wkie;
    bool gui_need_wkp = false;
#if LE_EN
    u16 interval = 0;
    u16 latency  = 0;
    u16 tout     = 0;
    u16 adv_interval = 0;
#endif
    s->flag_sleep_ble = ble_is_connect();
    u8 dac_status = dac_get_power_status();

#if VBAT_DETECT_EN
    if (bsp_vbat_get_lpwr_status()) {
        return;  /* 低电不进 sniff mode */
    }
#endif

    printf("%s\n", __func__);

#if VIDEO_PLAY_EN
    if ((api_video_play_sta_get() != AVI_STA_STOP)
        && (ctx->app->get_ui_sta() == FUNC_CLOCK)) {
        bsp_video_play_uninit();
    }
#endif

#if AVI_DVP_USE_CAMERA && IMG_SENSOR_SELECT
    bsp_image_sensor_pwdn();
#endif

    sleep_cb.sys_is_sleep = true;
    s->gui_need_wakeup = 0;

    bt_enter_sleep();
    bt_audio_bypass();
    while (btstack_audio_is_busy());

#if LE_EN
    adv_interval = ble_get_adv_interval();
    ble_set_adv_interval(1600);                  /* 500 * 0.625ms = 500ms */
    if (ble_is_connect()) {
        interval = ble_get_conn_interval();
        latency  = ble_get_conn_latency();
        tout     = ble_get_conn_timeout();
        ble_update_conn_param(410, 0, 500);      /* 512.5ms */
    }
#endif

#if BT_SINGLE_SLEEP_LPW_EN
    if (!bt_is_connected()) {
        if (bt_get_scan()) {
            bt_update_bt_scan_param(4096, 8, 4096, 12);
        } else {
            bt_update_bt_scan_param(4096, 0, 4096, 0);
            bt_scan_disable();
        }
    }
#else
    if (!bt_is_connected()) {
        bt_update_bt_scan_param(4096, 12, 2048, 12);
    }
#endif

#if DAC_DNR_EN
    u8 dnr_sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

#if (ASR_SELECT && ASR_FULL_SCENE)
    bsp_asr_stop();
#endif

    if (dac_status) {
        dac_power_off();
    }

    sys_set_tmr_enable(0, 0);

    adc_ch = bsp_saradc_exit();
    saradc_clr_channel(adc_ch);
    saradc_set_channel(BIT(ADCCH_VBAT) | BIT(ADCCH_BGOP));
    saradc_adc15_ana_set_channel(ADCCH15_ANA_BG);

#if CHARGE_EN
    bsp_charge_set_stop_time(3600);
    charge_set_detect_cnt(1);
#if CHARGE_VOL_DYNAMIC_DET
    if (s->chg_on) {
        gradient_process(0);
        RTCCON8 &= ~BIT(1);
    }
#endif
#endif

    usbcon0 = USBCON0;
    usbcon1 = USBCON1;
    USBCON0 = BIT(5);
    USBCON1 = 0;

#if SD_SUPPORT_EN
    SD0_LDO_DIS();
#endif

    gui_sleep(true);

#if MODEM_CAT1_EN
    bsp_modem_sleep_enter();
#endif

#if USER_KEY_QDEC_EN || USER_ADKEY_QDEC_EN
    bsp_qdec_exit();
#endif

    /* 切到 24M 再关 PLL */
    sysclk = sys_clk_get();
    sys_clk_set(SYS_24M);
    DACDIGCON0 &= ~BIT(0);
    adda_clk_source_sel(1);
    PLL0CON0 &= ~(BIT(18) | BIT(6));
    PLL1CON0 &= ~0x03;
    rtc_sleep_enter();

    /* IO 模拟输入配置 */
    pa_de = GPIOADE;
    pb_de = GPIOBDE;
    pe_de = GPIOEDE;
    pf_de = GPIOFDE;
    pg_de = GPIOGDE;

    if (vddio_sleep_level) {
        GPIOADE = BIT(7);  /* PA7 rst */
    } else {
        GPIOADE = 0;
    }
    GPIOBDE = BIT(3);
    GPIOGDE = 0x3F;        /* MCP FLASH */

    u32 pf_keep = 0;
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    /* 与 elunchbox_enter_sleep / lowpower 一致：息屏深睡保留 PE0/PE1 */
    if (elunchbox_pwr_gui_off_is_on()) {
        GPIOBDE = BIT(3) | BIT(8) | BIT(9);
        GPIOEDE = (BIT(0) | BIT(1));
        GPIOFDE = 0;
        pf_keep = 0;
    } else
#endif
    {
        u8 sensor_type = bsp_sensor_init_sta_get(SENSOR_INIT_ALL);
        if (sensor_type) {
            bool hr   = (sensor_type & SENSOR_INIT_HR);
            bool step = (sensor_type & SENSOR_INIT_STEP);
            u32 gpioede = (BIT(4) | BIT(3)) * hr | (BIT(8) | BIT(7)) * step;
            gpioede |= BIT(4) | BIT(3);
            pf_keep |= BIT(2);
            GPIOEDE = gpioede;
        } else {
            GPIOEDE = 0;
        }
#if MODEM_CAT1_EN
        if (bsp_modem_get_init_flag()) {
            pf_keep |= BIT(1) | BIT(2) | BIT(3);
        } else {
            pf_keep |= BIT(3);
        }
#endif
        GPIOFDE = pf_keep;
    }

#if AVI_DVP_USE_CAMERA && IMG_SENSOR_SELECT
    image_sensor_drv_enter_pwdn();
#endif

    wkie = WKUPCON & BIT(16);
    WKUPCON &= ~BIT(16);                        /* 休眠时关 WKIE */
    ctx->app->on_wakeup_config();               /* 配置 PF1/PF2 下降沿 + WKO 唤醒 */

    gui_need_wkp = sleep_proc_loop(ctx);        /* === 进入休眠主循环 === */

    /* --- 唤醒后恢复 --- */
    RTCCON9 = BIT(7) | BIT(5) | BIT(2);
    RTCCON3 &= ~(BIT(17) | BIT(13));
    ctx->app->on_wakeup_exit();                 /* 恢复 IO + 关 sniff wakeup */
    if (cfg_bt_sniff_clk_sel != 5) {
        lp_xosc_en();
    }
    WKUPCON |= wkie;

    GPIOADE = pa_de;
    GPIOBDE = pb_de;
    GPIOEDE = pe_de;
    GPIOFDE = pf_de;
    GPIOGDE = pg_de;
    USBCON0 = usbcon0;
    USBCON1 = usbcon1;

    bt_update_bt_scan_param_default();
#if BT_SINGLE_SLEEP_LPW_EN
    if (!bt_is_connected() && bt_get_scan()) {
        bt_scan_enable();
    }
#endif
#if SD_SUPPORT_EN
    SD0_LDO_EN();
#endif
    rtc_sleep_exit();
    CLKCON0 = (CLKCON0 & ~(0x03 << 2)) | (0x01 << 2);
    RSTCON0 &= ~BIT(4);
    adpll_init(DAC_OUT_SPR);
    adda_clk_source_sel(0);
    DACDIGCON0 |= BIT(0);
#if FPGA_EN
    fpga_uart_reinit();
#endif
    dac_aubuf_init();
    sys_clk_set(sysclk);
    saradc_set_channel(adc_ch);
    bsp_saradc_init();
#if USER_KEY_QDEC_EN || USER_ADKEY_QDEC_EN
    bsp_qdec_init();
#endif
#if CHARGE_EN
    bsp_charge_set_stop_time(18000);
    bsp_charge_sta(s->charge_sta);
    charge_set_detect_cnt(5);
#if CHARGE_VOL_DYNAMIC_DET
    if (s->chg_on) {
        gradient_process(0);
        RTCCON8 &= ~BIT(1);
    }
#endif
#endif
    sys_set_tmr_enable(1, 1);
    if (DAC_FAST_SETUP_EN) {
        bsp_loudspeaker_mute();
    }
#if MODEM_CAT1_EN
    bsp_modem_sleep_exit();
#endif
#if NOC_FLASH_EN || NOC_PSRAM_EN
    noc_init((NOC_PSRAM_EN << 1) | NOC_FLASH_EN);
#endif

    if (gui_need_wkp) {
        printf("gui_wakeup\n");
        gui_wakeup();
    } else {
        gpu_init();
    }

#if LE_EN
    ble_set_adv_interval(adv_interval);
    if (interval | latency | tout) {
        ble_update_conn_param(interval, latency, tout);
    }
#endif

    if (dac_status) {
        dac_restart();
    }
    plugin_music_eq();
    bsp_change_volume(s->vol);

#if (ASR_SELECT && ASR_FULL_SCENE)
    bsp_asr_start();
#endif
#if DAC_DNR_EN
    dac_dnr_set_sta(dnr_sta);
#endif

    bt_exit_sleep();
    bt_audio_enable();

#if AVI_DVP_USE_CAMERA && IMG_SENSOR_SELECT
    if (gui_need_wkp) {
        image_sensor_drv_auto_reg();
    }
#endif

    if (((u32)&__dynamic_pool_end > 0x60000) && is_gpu_init()) {
        customer_heap_init();
    }

    sleep_cb.sys_is_sleep = false;
    printf("sleep_exit\n");
}

/* ============================================================
 * lowpwr_sleep_process — 休眠决策主函数 (主循环调用)
 * ============================================================ */
bool lowpwr_sleep_process(lowpwr_t *ctx)
{
    lowpwr_state_t *s = ctx->state;

    /* ======== ELUNCHBOX 息屏路径 ======== */
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_pwr_gui_off_is_on() || sys_cb.gui_sleep_sta) {
        s->gui_need_wakeup = 0;

        if (!elunchbox_guioff_sleep_ready()) {
            return false;
        }
#if USER_PT8028_KEY
        /* 与 lowpower 一致：电源键仍按住时不进深睡，避免进睡瞬间 bts 异常 */
        if (pt8028_get_press_tch() == PT8028_KEY_TCH5) {
            return false;
        }
#endif
        /* 必须等 BT 允许休眠再 bt_enter_sleep，否则 bts 任务易 ERR:80 */
        if (!bt_is_allow_sleep()) {
            return false;
        }

        extern void elunchbox_enter_sleep(void);
        printf("elunchbox: deep sleep enter (bt ok)\n");
        elunchbox_enter_sleep();
        LPWR_DELAY_INIT_ALL(ctx);
        LPWR_PWROFF_DELAY_KILL(ctx);
        return true;
    }
#endif /* ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN */

    /* ======== 手表原始路径 (非 ELUNCHBOX) ======== */

    /* GUI 在休眠态但有唤醒请求 → 亮屏 */
    if (s->gui_need_wakeup && s->gui_sleep_sta) {
        gui_wakeup();
        LPWR_DELAY_INIT_ALL(ctx);
        s->gui_need_wakeup = 0;
        return false;
    }

#if VIDEO_PLAY_EN
    if ((api_video_play_sta_get() != AVI_STA_STOP)
        && (ctx->app->get_ui_sta() != FUNC_CLOCK)) {
        LPWR_DELAY_INIT_ALL(ctx);
        return false;
    }
#endif

#if VIDEO_RECODE_TAKE_PHOTO_EN
    if (bsp_video_recode_is_start()) {
        LPWR_PWROFF_RESET(ctx);
    }
#endif

#if AVI_DVP_USE_CAMERA
    if (bsp_image_sensor_is_init()) {
        return false;
    }
#endif

#if LE_EN
    if (ble_app_need_wakeup()) {
        LPWR_DELAY_INIT_ALL(ctx);
        LPWR_PWROFF_RESET(ctx);
        return false;
    }
#endif

    /* 应用层允许休眠 且 不是仅熄屏模式 */
    if (ctx->app->is_allow_sleep()
        && !LPWR_IS_GUI_OFF_ONLY(ctx)
#if VIDEO_RECODE_TAKE_PHOTO_EN
        && !bsp_video_recode_is_start()
#endif
        ) {
        if (!s->sleep_en) {
            LPWR_DELAY_INIT_ALL(ctx);
            return false;
        }
        if (s->sleep_delay == 0) {
            ctx->app->gui_sleep_psram_check();
            lowpwr_enter_deepsleep(ctx);     /* 熄屏 + 深度休眠 */
            LPWR_DELAY_INIT_ALL(ctx);
            LPWR_PWROFF_DELAY_KILL(ctx);
            return true;
        }
    } else {
        /* 仅熄屏 (不进深度休眠) */
        if (s->guioff_delay == 0 && !s->gui_sleep_sta) {
            printf("LP: gui_sleep(off only)\n");
            gui_sleep(false);
        }
        LPWR_SLEEP_RESET(ctx);
        LPWR_PWROFF_RESET(ctx);
    }

    return false;
}

/* ============================================================
 * 省电关机 — 关外设进入 sleep mode / power down mode
 * ============================================================ */

AT(.text.pwroff.vusb)
static u8 pwrdown_w4_vusb_offline(void)
{
    return 0;  /* 不等待 VUSB 掉线 */
}

AT(.text.pwroff.enter)
static void power_save_enter(lowpwr_t *ctx)
{
    lowpwr_state_t *s = ctx->state;

#if LP_XOSC_CLOCK_EN
    bool lp_xosc_clock_err = lp_xosc_check();
#endif

    sys_clk_set(SYS_24M);
    LOUDSPEAKER_MUTE_DIS();
    WDT_CLR();

    if (ctx->charge_enabled) {
        bsp_charge_off();
    }

    set_buck_mode(0);                            /* 切回 LDO mode */
    vusb_delay_rst_dis();
    RTCCON4 |= BIT(5) | BIT(6);
    delay_us(100);
    RTCCON4 &= ~BIT(6);
    USBCON0 = BIT(5);
    USBCON1 = 0;
    GPIOADE  = 0;
    GPIOBDE  = 0;
    GPIOEDE  = 0;
    GPIOFDE  = 0;
    GPIOGDE  = 0x3F;
    GPIOHDE  = 0;
    RTCCON8 &= ~BIT(15);

#if !LP_XOSC_CLOCK_EN
    RTCCON15 &= ~(3 << 13);
    if (s->flag_shipping) {
        RTCCON0 &= ~(BIT(0) | BIT(20) | BIT(2));
    }
#else
    RTCCON0 &= ~(BIT(0) | BIT(20) | BIT(2));
    if (lp_xosc_clock_err) {
        RTCCON15 &= ~(3 << 13);
    }
#endif

    PICCONCLR = BIT(0);
    CLKCON0 |= BIT(0);
    CLKCON0 = (CLKCON0 & ~(0x3 << 2));

    RSTCON0 &= ~(BIT(4) | BIT(6));
    CLKGAT2 &= ~(BIT(0) | BIT(1));
    PLL0CON0 &= ~(BIT(6) | BIT(18));
    XOSCCON0 &= ~((0xf << 24) | (3 << 6) | (7 << 10) | BIT(5));
    PLL0CON0 &= ~(BIT(12) | BIT(0) | BIT(1));
    RTCCON9 = 0xfff;
}

/* sfunc_pwrdown_do — 进入 power down 模式 */
AT(.text.pwroff.pwrdwn)
static void pwrdown_do(lowpwr_t *ctx, u8 vusb_wakeup_en)
{
    lowpwr_state_t *s = ctx->state;

#if !LP_XOSC_CLOCK_EN
    if (!s->flag_shipping) {
        rtc_set_alarm_wakeup(900);               /* 15min 后唤醒 */
    }
#endif

    sys_set_tmr_enable(0, 0);
    power_save_enter(ctx);

    if (!vusb_wakeup_en) {
        RTCCON8 = (RTCCON8 & ~BIT(6)) | BIT(1);
        vusb_wakeup_en = pwrdown_w4_vusb_offline();
    }
    RTCCON1 &= ~(BIT(5) | BIT(7));
    RTCCON11 = (RTCCON11 & ~0x03) | BIT(2);      /* WK PIN filter 8ms */

    uint rtccon3 = RTCCON3 & ~BIT(11);
    uint rtccon1 = RTCCON1 & ~0x1f;

    if (ctx->charge_enabled && vusb_wakeup_en) {
        rtccon3 |= BIT(11);                      /* VUSB wakeup */
    }

    LPPGCON0 = 0;
    RTCCON0 &= ~BIT(5);
    RTCCON0 &= ~BIT(4);
    RTCCON1 &= ~BIT(9);

    RTCCON3 = rtccon3 & ~BIT(10);                /* 清 WK PIN pending */
    rtccon3 = RTCCON3 & ~0x7;
    rtccon3 &= ~BIT(3);
    rtccon3 |= BIT(6) | BIT(4) | BIT(19);        /* PDCORE */
    rtccon3 |= BIT(10);                          /* WK pin wakeup */
#if SOFT_POWER_VDDIO_EN
    rtccon3 |= BIT(7);
#endif
    rtccon1 |= BIT(0) | BIT(4);                  /* WK 90K pull-up */
    WDT_DIS();
    RTCCON12 |= (0x3 << 6);
    BTCON2 |= BIT(23);
    BTCON2 &= ~(3 << 10);
    RTCCON &= ~(0xf << 7);
    QDECCON &= ~BIT(2);
    WKUPCPND = (0xff << 16);
    WKUPCON &= ~BIT(17);
    RTCCON &= ~(3 << 1);
    RTCCON |= BIT(5);
    RTCCON1 = rtccon1;
    RTCCON3 = rtccon3;
    LPMCON |= BIT(0);
    LPMCON |= BIT(1);
    asm("nop"); asm("nop"); asm("nop");
    while (1);
}

/* sfunc_lowbat_do — 低电关机进入 sleep mode */
AT(.text.pwroff)
static void lowbat_do(lowpwr_t *ctx)
{
    power_save_enter(ctx);
    WDT_DIS();
    RTCCON3 |= BIT(11);                          /* VUSB wakeup */
    while (1) {
        LPMCON |= BIT(0);
        asm("nop"); asm("nop"); asm("nop");
        if (RTCCON10 & BIT(3)) {
            WDT_RST();
        }
    }
}

void lowpwr_pwrdown(lowpwr_t *ctx, uint8_t vusb_wakeup_en)
{
    ctx->app->on_pwroff_lock();
    if (ctx->app->on_pwrdown_wake_prep) {
        ctx->app->on_pwrdown_wake_prep();
    }
    pwrdown_do(ctx, vusb_wakeup_en);
}

void lowpwr_lowbat_shutdown(lowpwr_t *ctx)
{
    ctx->app->on_pwroff_lock();
    lowbat_do(ctx);
}

/* ============================================================
 * lowpwr_pwroff — 软关机
 * ============================================================ */
void lowpwr_pwroff(lowpwr_t *ctx, int pwroff_tone_en)
{
    lowpwr_state_t *s = ctx->state;
    printf("%s\n", __func__);

    /* 断开 BT/BLE */
    if (bt_cb.bt_is_inited) {
        if (ble_is_connected()) {
            ble_disconnect();
        }
        if (bt_is_connected()) {
            bt_disconnect(0);
        }
        bt_off();
        bt_cb.bt_is_inited = 0;
    }

#if (ASR_SELECT && ASR_FULL_SCENE)
    bsp_asr_stop();
#endif

#if !LP_XOSC_CLOCK_EN
    if (!s->flag_shipping) {
        sniff_rc_init();
        rtc_rc_sleep_enter();
        rtc_calibration_write(PARAM_RTC_CAL_ADDR);
        cm_write8(PARAM_RTC_CAL_VALID, 1);
        cm_sync();
        rtc_printf();
    }
#endif

#if WARNING_POWER_OFF
    if (ctx->soft_power_on_off) {
        mp3_res_play(RES_BUF_POWEROFF_MP3, RES_LEN_POWEROFF_MP3);
    }
#endif

    gui_sleep(true);

    if (ctx->soft_power_on_off) {
        if (!ctx->pwrkey_2_hw_pwron) {
            while (IS_PWRKEY_PRESS()) {
                delay_5ms(1);
                WDT_CLR();
            }
        }
        u8 dac_status = dac_get_power_status();
        if (dac_status) {
            dac_power_off();
        }
        if (CHARGE_DC_IN()) {
            if (ctx->app->power_off_check()) {
                return;  /* 充电中不允许关机 */
            }
        }
        bsp_saradc_exit();
        if (ctx->pwrkey_2_hw_pwron && s->poweron_flag) {
            RTCCON1 |= BIT(6);                   /* WK PIN high level wakeup */
        }
        lowpwr_pwrdown(ctx, 1);
    } else {
        dac_power_off();
        bsp_saradc_exit();
        lowpwr_lowbat_shutdown(ctx);
    }
}
