#include "include.h"
#include "func.h"
#if ELUNCHBOX_PANEL_EN
#include "lowpower/elunchbox_lp.h"
#if USER_PT8028_KEY
#include "port_pt8028_key.h"
#include "bsp_pt8028_key.h"
#endif
#endif

/* 休眠循环详细日志: 1=开启(调试用, UART活跃→mA级功耗) 0=关闭(省电) */
#ifndef LP_SLEEP_VERBOSE
#define LP_SLEEP_VERBOSE 0
#endif

#if ELUNCHBOX_PANEL_EN && FUNC_LUNCHBOX_UART_EN
/**
 * manual_off 中 PB9(串口)唤醒过滤:
 *   1. 恢复 UART (PB9 唤醒首帧已丢失, 恢复后主动发查询)
 *   2. 发送 0x01 动态属性查询 → 等加热模块应答
 *   3. 扫描应答帧 DataPoints:
 *        DP4 (充电状态)==1  → 唤醒
 *        DP10(是否加热)==1  → 唤醒
 *   4. 都不满足/超时 → suspend UART 回去继续睡
 *
 * 注: 大缓冲用 static 省栈 (sleep 上下文栈仅 ~1200B)
 */
AT(.sleep_text.sleep.proc)
static bool manual_off_uart_wake_check(void)
{
    static lb_proto_parser_t parser;      /* 260B, 省栈 */
    static u8 tx_buf[LB_TXBUF_SIZE];      /* 256B, 省栈 */
    lb_rx_frame_t frame;
    u32 deadline;
    bool got_frame = false;

    /* 深睡时 UART 已 suspend, PB9 切回 GPIO 做下降沿唤醒源; 这里恢复 */
    lb_link_resume();
    lb_proto_parser_reset(&parser);

    /* 唤醒首帧已丢失: 主动发查询(空数据, 不发时间戳避免扰乱加热模块时钟) */
    {
        u16 total = lb_proto_build_frame(tx_buf, LB_UART_CMD_DYNAMIC,
                                          0x80, LB_ERR_SUCCESS, NULL, 0);
        if (total > 0) {
            lb_link_tx(tx_buf, total);
            printf("lp: uart query sent, waiting response\n");
        }
    }

    /* 等应答帧 (500ms 超时) */
    deadline = tick_get();
    while (!tick_check_expire(deadline, 500)) {
        u8 ch;
        while (lb_link_getc(&ch)) {
            if (lb_proto_parser_feed(&parser, ch, &frame)) {
                got_frame = true;
                break;
            }
        }
        if (got_frame) break;
        WDT_CLR();
    }

    if (got_frame) {
        u8 val;

        /* DP4 充电状态: 1=充电中 */
        if (lb_dp_scan_bool(frame.data, frame.data_len,
                            LB_DPID_CHARGE_STATUS, &val) && val == 1) {
            printf("lp: uart wake DP4=charging -> wake up\n");
            lunchbox_wake_reason_set_uart();
            return true;
        }
        /* DP10 是否加热: 1=立即加热 */
        if (lb_dp_scan_bool(frame.data, frame.data_len,
                            LB_DPID_HEAT_ENABLE, &val) && val == 1) {
            printf("lp: uart wake DP10=heating -> wake up\n");
            lunchbox_wake_reason_set_uart();
            return true;
        }
        printf("lp: uart frame ignored (DP4/DP10 not match)\n");
    } else {
        printf("lp: uart wake timeout, no response\n");
    }

    /* 未命中或超时: suspend UART, PB9 还给 GPIO 做下次唤醒源 */
    lb_link_suspend();
    return false;
}
#else
#define manual_off_uart_wake_check()    true    /* 无串口功能: 任何 PB9 都唤醒 */
#endif

AT(.sleep_backup.gui)
u8 sys_backup_buf[32 * 1024 - 8];

/* manual_off 深度休眠标志：sys_sleep_cb 在 sys_enter_sleep 前关 BT+RTC wakeup */
static bool elunchbox_manual_off_in_sleep;
/* 非TCH5按键按住期间：PE1 wakeup已切为上升沿，等松手后恢复下降沿 */
static bool elunchbox_waiting_key_release;

extern u8 *cache_backup;
extern u32 __dynamic_pool_start, __dynamic_pool_end;
#if ELUNCHBOX_PANEL_EN
extern u32 elunchbox_saved_clkgat0;   /* elunchbox_lp.c: 息屏前保存，唤醒须先恢复 */
#endif

bool power_off_check(void);
void lock_code_pwrsave(void);
void unlock_code_pwrsave(void);
bool lp_xosc_check(void);
u32 get_sleep_proc_delay(void);
void gui_sleep_psram_check(void);
bool btstack_audio_is_busy(void);
bool keep_ram_tbl_load(void);
bool keep_ram_tbl_restore(void);

// //获取vddio ldo是否打开（平台库调用，默认false），true:打开vddio_ldo，避免动态开关vddhr产生纹波, 但功耗会增加40uA；false：不打开vddio_ldo，省功耗但会影响vddhr产生纹波
// AT(.sleep_text.lp.sleep.proc)
// bool get_sleep_vddio_ldo_en(void)
// {
//     return true;
// }

/* 【休眠/关机倒计时】5ms 定时器回调，内部 /20 → 100ms 间隔 */
AT(.com_text.sleep)
void lowpwr_tout_ticks(void)
{
    // static u8 div;  /* 5ms * 20 = 100ms 分频 */

    // if (++div < 20) return;
    // div = 0;

    if(sys_cb.sleep_delay != -1L && sys_cb.sleep_delay > 0) {
        sys_cb.sleep_delay--;
    }
    if(sys_cb.guioff_delay != -1L && sys_cb.guioff_delay > 0) {
        sys_cb.guioff_delay--;
    }
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    elunchbox_guioff_sleep_delay_tick();
#endif
#if ELUNCHBOX_PANEL_EN
    elunchbox_guioff_idle_tick();
#endif
    if(sys_cb.pwroff_delay != -1L && sys_cb.pwroff_delay > 0) {
        sys_cb.pwroff_delay--;
    }
}

/* sys_sleep_check — BSP btstack 调用, 固定签名 */
AT(.com_text.sleep)
bool sys_sleep_check(u32 *sleep_time)
{
    u32 co_min = co_timer_get_min_time(true) * 2;

    if (*sleep_time > co_min) {
        *sleep_time = co_min;
    }
    if (*sleep_time < 4) {
        *sleep_time = 4;
    }
    if (*sleep_time > sys_cb.sleep_wakeup_time) {
        *sleep_time = sys_cb.sleep_wakeup_time;
        return true;
    }
    return false;
}

//休眠中ble断开/连接/传输是否需要退出休眠
AT(.com_text.sleep)
bool ble_is_allow_wkup(void)
{
    return LE_ALLOW_WKUP_EN;
}

//sleep前补充备份到retention ram
AT(.sleep_text.sleep.backup)
void sys_sleep_backup_cb(void)
{

}

//wakeup后补充恢复
AT(.sleep_text.sleep.restore)
void sys_sleep_restore_cb(void)
{

}

// 非0的时候休眠不灭屏   vddio电压不能太低   VDDIO电压，step=0.1V 0:2.4V
u8 vddio_sleep_level = 0;
AT(.sleep_text.sleep)
u8 sys_enter_sleep_vddio_level(void)
{
    return vddio_sleep_level;
}

/* 保存最近一次 sys_sleep_cb 用的 lpclk_type, manual_off 强制睡时复用 */
static u8 s_lpclk_type_saved = 0;

AT(.sleep_text.sleep)
void sys_sleep_cb(u8 lpclk_type)
{
    s_lpclk_type_saved = lpclk_type;            /* 保存供 manual_off 强制睡使用 */

    //此处关掉影响功耗的模块
    u32 gpiogde = GPIOGDE;
    if (gpiogde & BIT(6)) {
        GPIOGDE = BIT(2) | BIT(4) | BIT(6);         //SPICS, SPICLK
    } else {
        GPIOGDE = BIT(2) | BIT(4);                  //SPICS, SPICLK
    }

    /* manual_off: 最后一刻关 BT+RTC 唤醒，仅保留 PE1+PB9 port wakeup */
    if (elunchbox_manual_off_in_sleep) {
        RTCCON3 &= ~BIT(13);        /* disable bt wakeup */
        BTCON2 &= ~(3 << 10);       /* disable bt sleep wakeup */
        RTCCON  &= ~(0xf << 7);     /* disable rtc sleep wakeup */
#if CHARGE_EN
        RTCCON3 |= BIT(11);         /* VUSB 插电可唤醒: 关机后充电进黑屏充电页 */
#endif
        RTCCON9 = BIT(7) | BIT(5) | BIT(2); /* clr bt/wko/port pending */
    }

    sys_enter_sleep(lpclk_type);                //enter sleep

    //唤醒后，恢复模块功能
    GPIOGDE = gpiogde;
}

void sleep_set_sysclk(uint8_t sys_clk)
{
    uint8_t cur_sys_clk = sys_clk_get();

    if (sys_clk < SYS_24M || cur_sys_clk == sys_clk) {
        return;
    }

    if (sys_clk > SYS_24M) {
        if (cur_sys_clk <= SYS_24M) {
            CLKCON0 = (CLKCON0 & ~(0x03 << 2)) | (0x01 << 2); //sysclk select xosc26m_clk
            RSTCON0 &= ~BIT(4);                         //pllsdm disable
            adpll_init(DAC_OUT_SPR);                    //enable adpll
            adda_clk_source_sel(0);                     //adda_clk48_a select pll0
        }
        sys_clk_set(sys_clk);
    } else {
        sys_clk_set(SYS_24M);
        DACDIGCON0 &= ~BIT(0);                      //disable digital dac
        adda_clk_source_sel(1);                     //adda_clk48_a select xosc52m
        PLL0CON0 &= ~(BIT(18) | BIT(6));             //pll0 sdm & analog disable
    }
}

//用于未连接ble休眠后,当ble连接上后更新连接参数用
AT(.sleep_text.sleep)
void sleep_ble_param_check(void)
{
    if (sys_cb.flag_sleep_ble_status != ble_is_connect()) {
        if ((sys_cb.flag_sleep_ble_status == false) && ble_is_connect()) {
            ble_update_conn_param(410, 0, 500);     //interval: 410*1.25ms = 512.5ms
        }
        sys_cb.flag_sleep_ble_status = ble_is_connect();
    }
}


//休眠定时器，500ms进一次
AT(.sleep_text.sleep.timer)
uint32_t sleep_timer(void)
{
    uint32_t ret = 0;

    sys_cb.sleep_counter++;
    if (sys_cb.sleep_counter == 10) {
        sys_cb.sleep_counter = 0;

#if LPWR_WARNING_VBAT
        u32 rtccon8 = RTCCON8;
#if CHARGE_VOL_DYNAMIC_DET
        if (RTCCON & BIT(20)) {                 //vusb is online?
            gradient_process(1);
            RTCCON8 |= BIT(1);                  //charge stop
            delay_5ms(2);
            gradient_process(0);
        }
#endif
        saradc_init();
        saradc_adc15_analog_channel_sleep();
        saradc_adc15_ana_set_channel(ADCCH15_ANA_BG);
        delay_us(600);
        saradc_kick_start(0, 0);
        while(!bsp_saradc_process(0));
        sys_cb.vbat = bsp_vbat_get_voltage(1);
        RTCCON8 = rtccon8;
        if (sys_cb.vbat < LPWR_WARNING_VBAT) {
            //低电需要唤醒sniff mode
            ret = 2;
        }
        saradc_exit();
#endif
    }

#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_detect(0);
    }
#endif // CHARGE_EN

    sys_cb.sleep_wakeup_time = -1L;

    if(sys_cb.pwroff_delay != -1L) {
        if(sys_cb.pwroff_delay > 5) {
            sys_cb.pwroff_delay -= 5;
        } else {
            sys_cb.pwroff_delay = 0;
            return 1;
        }
    }

    if ((PWRKEY_2_HW_PWRON) && (!IS_PWRKEY_PRESS())){
        ret = 1;
    }

    rtc_sleep_process();
    sleep_ble_param_check();

#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_guioff_in_sleep_mode()
        && !elunchbox_pwr_is_manual_off()) {
        elunchbox_guioff_sleep_service();
    }
#endif

    return ret;
}

AT(.sleep_text.rodata.wakeup)
const char port_wakeup_str[] = "port wakeup: %x\n";

AT(.sleep_text.rodata.wakeup)
const char wko_wakeup_str[] = "wko wakeup\n";

AT(.sleep_text.rodata.wakeup)
const char app_wakeup_str[] = "ble_app_need_wakeup\n";

AT(.sleep_text.rodata.wakeup)
const char co_timer_wakeup_str[] = "co_timer_pro_wakeup\n";

AT(.sleep_text.rodata.wakeup)
const char call_wakeup_str[] = "call_wakeup\n";

AT(.sleep_text.rodata.wakeup)
const char lp_enter_str[] = "lp: enter m=%u\n";

AT(.sleep_text.rodata.wakeup)
const char lp_exit_str[] = "lp: exit wkp=%u m=%u\n";

/* ---- manual_off 休眠内 helper (retention RAM) ---- */

/* 临时使能 PE2~PE4 数字输入，读 BCD 键值和 PE1 电平，然后恢复 GPIOEDE。
 * 返回 PE1 是否 LOW；*bcd 输出 BCD 值 (0~7)。
 * 【防假唤醒】PE2~4 在休眠时被切模拟+清上下拉以省电。
 * 此处临时切回数字输入时加下拉，防止 floating 电平随机读成 5(TCH5 的 BCD 值)
 * → 假唤醒 → 20mA。加上下拉确保 BCD 读 0 而非 5。 */
AT(.sleep_text.sleep.proc)
static bool sleep_read_bcd_pe1(u8 *bcd)
{
    u32 saved = GPIOEDE;
    u32 saved_pu = GPIOEPU;
    u32 saved_pd = GPIOEPD;
    u32 saved_pu200k = GPIOEPU200K;
    u32 saved_pd200k = GPIOEPD200K;
    u32 saved_pu300  = GPIOEPU300;
    u32 saved_pd300  = GPIOEPD300;
    GPIOEDE |= BIT(2) | BIT(3) | BIT(4);   /* PE2~4 临时数字输入 */
    GPIOEPU &= ~(BIT(2) | BIT(3) | BIT(4)); /* 清上拉 */
    GPIOEPD |= (BIT(2) | BIT(3) | BIT(4));  /* 加下拉 → BCD 稳定读 0 (非 TCH5=5) */
    GPIOEPU200K &= ~(BIT(2) | BIT(3) | BIT(4));
    GPIOEPD200K &= ~(BIT(2) | BIT(3) | BIT(4));
    GPIOEPU300  &= ~(BIT(2) | BIT(3) | BIT(4));
    GPIOEPD300  &= ~(BIT(2) | BIT(3) | BIT(4));
    delay_us(30);                           /* BCD 稳定 */
    u8 pe = (u8)(GPIOE & 0x1f);
    *bcd = (u8)(((pe >> 4) & 1) << 2) | (((pe >> 3) & 1) << 1) | ((pe >> 2) & 1);
    bool pe1_lo = ((pe >> 1) & 1) == 0;
    GPIOEDE = saved;
    GPIOEPU = saved_pu;
    GPIOEPD = saved_pd;
    GPIOEPU200K = saved_pu200k;
    GPIOEPD200K = saved_pd200k;
    GPIOEPU300  = saved_pu300;
    GPIOEPD300  = saved_pd300;
    return pe1_lo;
}

//此函数运行代码及调用子函数需要全部放到retention ram
AT(.sleep_text.sleep.proc)
bool sfunc_sleep_proc(void)
{
    uint32_t status, ret;
    u32 wkpnd;
    bool wko_wkup_flag;
    bool gui_need_wkp = false;
    u32 vddio_vddcore_level = PWRCON0 & 0x1ff;
    u8 vddio_level = vddio_sleep_level;
#if ELUNCHBOX_PANEL_EN
    bool manual_off = elunchbox_pwr_is_manual_off();
#endif

    cache_backup = sys_backup_buf;
    if (cache_backup == NULL) {
        printf("==>sleep malloc memory failed!\n");
        return gui_need_wkp;
    }

    /* VDDCORE: 统一 1.0V。0.9V 可能导致 BT 数字逻辑/射频无法正常休眠 */
    PWRCON0 = (PWRCON0 & ~0x1f) | 12;           // 1.0V
    if (vddio_level) {
        PWRCON0 = (PWRCON0 & ~(BIT(5)*0xF)) | BIT(5) * vddio_level;
    }

    /* 【低功耗优化】manual_off: 关闭 VDDTK LDO, 省 10-50μA */
#if LPWR_VDDTK_OFF_EN && ELUNCHBOX_PANEL_EN
    u32 rtccon1_vddtk_saved = 0;
    if (manual_off) {
        rtccon1_vddtk_saved = RTCCON1 & BIT(9);
        RTCCON1 &= ~BIT(9);
    }
#endif

    sys_cb.sleep_counter = 0;
    sys_cb.sleep_wakeup_time = -1L;

#if ELUNCHBOX_PANEL_EN
    printf(lp_enter_str, manual_off ? 1u : 0u);
#endif

    u32 lp_loop_cnt = 0;
    u32 lp_sleep_cnt = 0;
    u8  lp_last_status = 0xFF;
#if ELUNCHBOX_PANEL_EN
    u8  force_spin_cnt = 0;   /* manual_off: bt_sleep_proc 连续不睡计数 */
#endif
    /* manual_off: BT 栈可能因 BLE 定时醒来后 bt_is_sleep() 变 false，
     * 若不处理→退出 while → sfunc_sleep 重进 → 死循环。
     * manual_off 时忽略 bt_is_sleep() 的 false，继续 bt_sleep_proc()，
     * 只靠 PE1/PB9 事件或低电退出。 */
    while (bt_is_sleep()
#if ELUNCHBOX_PANEL_EN
           || manual_off
#endif
           ) {
        WDT_CLR();
        bt_thread_check_trigger();
        status = bt_sleep_proc();
        lp_loop_cnt++;

        /* debug: track BT sleep state transitions */
        if (status != lp_last_status) {
#if LP_SLEEP_VERBOSE
            printf("lp: bt_sleep_proc %u→%u loop=%u sleep_cnt=%u bt_sleep=%u\n",
                   lp_last_status, status, lp_loop_cnt, lp_sleep_cnt, bt_is_sleep() ? 1u : 0u);
#endif
            lp_last_status = (u8)status;
        }
        /* heartbeat: loop spinning without sleep (every 256 iters) */
        if ((lp_loop_cnt & 0xFF) == 0 && status != 1) {
#if LP_SLEEP_VERBOSE
            printf("lp: SPIN loop=%u status=%u sleep_cnt=%u bt_sleep=%u\n",
                   lp_loop_cnt, status, lp_sleep_cnt, bt_is_sleep() ? 1u : 0u);
#endif
        }

#if SENSOR_HUB_EN
        bsp_senshb_lp_process();
#endif

        if (status == 1) {
            lp_sleep_cnt++;
#if ELUNCHBOX_PANEL_EN
            force_spin_cnt = 0;     /* BT 栈正常睡了 → 复位强睡计数 */
            if (manual_off) {
#if CHARGE_EN
                /* 本机 DC 插电 → 醒去黑屏充电页 (demo6 同款: 每次 sniff ~2s 一查,
                 * 兜底 PB9 门铃首包丢失; charge_dc_detect 只读寄存器, 开销忽略) */
                if (CHARGE_DC_IN()) {
                    printf("lp: DC_IN while sniff -> wake for charge page\n");
#if FUNC_LUNCHBOX_UART_EN
                    lunchbox_wake_reason_set_uart();
#endif
                    gui_need_wkp = true;
                    break;
                }
#endif
                /* Reduced ADC: every 60 rounds (~30s), low-battery only */
                if (++sys_cb.sleep_counter >= 60) {
                    sys_cb.sleep_counter = 0;
                    ret = sleep_timer();
                    if (ret == 2) { break; }        /* low battery → wake */
                }
            } else
#endif
            {
                ret = sleep_timer();
                if (ret) {
                    if (ret == 1) {
                        func_cb.sta = FUNC_PWROFF;
                    }
                    break;
                }
            }
        }
#if ELUNCHBOX_PANEL_EN
        else if (manual_off) {
            /* bt_sleep_proc 连续返回非1 → BT栈不睡 → 绕过直接强睡 */
            force_spin_cnt++;
            if (force_spin_cnt >= 5) {
                force_spin_cnt = 0;
                u32 gpiogde = GPIOGDE;
                if (gpiogde & BIT(6)) {
                    GPIOGDE = BIT(2) | BIT(4) | BIT(6);
                } else {
                    GPIOGDE = BIT(2) | BIT(4);
                }
                RTCCON3 &= ~BIT(13);        /* disable bt wakeup */
                BTCON2 &= ~(3 << 10);       /* disable bt sleep wakeup */
                RTCCON  &= ~(0xf << 7);     /* disable rtc sleep wakeup */
                RTCCON9 = BIT(7) | BIT(5) | BIT(2); /* clr bt/wko/port pending */
                sys_enter_sleep(s_lpclk_type_saved);
                GPIOGDE = gpiogde;
                lp_sleep_cnt++;
            }
        }
#endif

        /* ================================================================
         * manual_off 唤醒源: 仅 PE1(TCH5按键) + PB9(UART RX)
         *   - PB9 LOW → UART唤醒
         *   - TCH5(bcd=5) 持续 LOW≥2s → 开机唤醒
         *   - 非TCH5/轻触: 清pending → continue(不离while, bt_sleep_proc重睡)
         * ================================================================ */
#if ELUNCHBOX_PANEL_EN && USER_PT8028_KEY
        if (manual_off) {
            bool sw_has_event;
            bool hw_has_event;

#if CHARGE_EN
            /* 本机 VUSB 已插电: 退出深睡去黑屏充电页 (demo6 同款,
             * 事件路径也查 —— 按键/PB9 事件与插电同时发生时充电优先) */
            if (CHARGE_DC_IN()) {
                printf("lp: -> VUSB/DC_IN wake (manual_off)\n");
#if FUNC_LUNCHBOX_UART_EN
                lunchbox_wake_reason_set_uart();
#endif
                gui_need_wkp = true;
                break;
            }
#endif

            /* Step 1: 软件轮询 PE1 FLAG 下降沿 */
            elunchbox_manual_off_sleep_poll();
            sw_has_event = elunchbox_manual_wake_pending_peek();

            /* Step 2: 硬件 port wakeup pending */
            wkpnd = port_wakeup_get_status();
            hw_has_event = (wkpnd != 0);

            /* Step 2.5: 非TCH5按键松手检测
             * PE1已切为上升沿等松手，唤醒=松手→恢复下降沿→重睡 */
            if (elunchbox_waiting_key_release) {
                bool pb9_lo = ((GPIOB >> 9) & 1) == 0;
                u8 pe1 = ((GPIOE >> 1) & 1);

                if (pb9_lo) {
                    if (manual_off_uart_wake_check()) {
                        printf("lp: -> PB9 wake during wait-release\n");
                        gui_need_wkp = true;
                        break;
                    }
                    /* 无关帧: 不唤醒, 落到下面继续等松手/重睡 */
                }

                if (pe1 != 0) {
                    /* PE1 HIGH → 按键已松手 → 恢复下降沿唤醒 */
#if LP_SLEEP_VERBOSE
                    printf("lp: -> key released, restore falling edge\n");
#endif
                    port_wakeup_init(PT8028_GPIO_OUT_FLAG, 1, 1);
                    elunchbox_waiting_key_release = false;
                }
                /* PE1仍LOW：杂波唤醒 → 忽略，继续等松手 */

                RTCCON9 = BIT(7) | BIT(5) | BIT(2);
                elunchbox_manual_wake_pending_take();
                continue;
            }

            if (sw_has_event || hw_has_event) {
                /* Step 3: 读 PB9 + PE1/BCD */
                bool pb9_lo = ((GPIOB >> 9) & 1) == 0;
                u8 bcd;
                bool pe1_lo = sleep_read_bcd_pe1(&bcd);
#if LP_SLEEP_VERBOSE
                printf("lp: EVENT sw=%d hw=%d pb9=%d pe1=%d bcd=%d\n",
                       sw_has_event, hw_has_event, pb9_lo, pe1_lo, bcd);
#endif

                if (pb9_lo) {
                    if (manual_off_uart_wake_check()) {
                        printf("lp: -> UART/PB9 wake\n");
                        gui_need_wkp = true;
                        break;
                    }
                    /* 无关帧(心跳等): 清 pending 回去继续睡 */
                    RTCCON9 = BIT(7) | BIT(5) | BIT(2);
                    elunchbox_manual_wake_pending_take();
                    continue;
                }
                if (pe1_lo && bcd == 5) {
                    /* TCH5: 要求持续 LOW≥2s 才当有效唤醒 */
                    int tch5_cnt = 0;
                    while (tch5_cnt < 400) {  /* 400*5ms=2000ms=2s */
                        delay_5ms(1);
                        WDT_CLR();
                        if (((GPIOE >> 1) & 1) != 0) {
                            break;  /* PE1 回 HIGH → 未按够 2s */
                        }
                        tch5_cnt++;
                    }
                    if (tch5_cnt >= 400) {
                        printf("lp: -> TCH5 wake (held 2s)\n");
#if FUNC_LUNCHBOX_UART_EN
                        lunchbox_wake_reason_set_key();
#endif
                        gui_need_wkp = true;
                        break;
                    }
#if LP_SLEEP_VERBOSE
                    printf("lp: -> TCH5 brief (<2s), back to sleep\n");
#endif
                    /* 误触: 不走共享cleanup, 自己清理+统一上升沿+强睡标定, 同lowpower风格 */
                    RTCCON9 = BIT(7) | BIT(5) | BIT(2);
                    elunchbox_manual_wake_pending_take();
                    port_wakeup_init(PT8028_GPIO_OUT_FLAG, 0, 1);
                    elunchbox_waiting_key_release = true;
                    force_spin_cnt = 4;
                    continue;
                } else if (pe1_lo) {
#if LP_SLEEP_VERBOSE
                    printf("lp: -> non-TCH5 (bcd=%d), back to sleep\n", bcd);
#endif
                } else {
#if LP_SLEEP_VERBOSE
                    printf("lp: -> stale/noise, ignore\n");
#endif
                }

                /* 无效唤醒: PE1仍LOW(按键还按着)→切上升沿等松手;
                 * PE1已HIGH(键已松开, 上升沿已错过)→恢复下降沿直接继续睡.
                 * 不能在 PE1=HIGH 时切上升沿: 没边沿了 → sys_enter_sleep 再唤不醒. */
                RTCCON9 = BIT(7) | BIT(5) | BIT(2);
                elunchbox_manual_wake_pending_take();
                if (((GPIOE >> 1) & 1) == 0) {
                    /* PE1 仍 LOW: 按键按住中, 切上升沿等松手 */
                    port_wakeup_init(PT8028_GPIO_OUT_FLAG, 0, 1);
                    elunchbox_waiting_key_release = true;
#if LP_SLEEP_VERBOSE
                    printf("lp: -> sw to rising edge, wait release\n");
#endif
                } else {
                    /* PE1 已 HIGH: 键已松开, 保持下降沿, 下次按键正常唤醒 */
                    elunchbox_waiting_key_release = false;
#if LP_SLEEP_VERBOSE
                    printf("lp: -> key already released, stay falling edge\n");
#endif
                }
                continue;
            }
        }
#endif

        /* ---- 以下与原厂逻辑一致，manual_off/normal 共用 ---- */
        wkpnd = port_wakeup_get_status();
        wko_wkup_flag = port_wko_is_wakeup();
        port_int_sleep_process(&wkpnd);
        bsp_sensor_step_lowpwr_pro();

        /* Port wakeup */
        if (wkpnd) {
#if ELUNCHBOX_PANEL_EN
            if (manual_off) {
                bool pb9_lo = ((GPIOB >> 9) & 1) == 0;
                u8 bcd;
                bool pe1_lo = sleep_read_bcd_pe1(&bcd);
                if (pb9_lo) {
                    if (manual_off_uart_wake_check()) {
                        gui_need_wkp = true;
                        break;
                    }
                    /* 无关帧: 清 pending 回去继续睡 */
                    RTCCON9 = BIT(7) | BIT(5) | BIT(2);
                    continue;
                }
                if (pe1_lo && bcd == 5) {
                    int tch5_cnt = 0;
                    while (tch5_cnt < 400) {
                        delay_5ms(1);
                        WDT_CLR();
                        if (((GPIOE >> 1) & 1) != 0) break;
                        tch5_cnt++;
                    }
                    if (tch5_cnt >= 400) {
#if FUNC_LUNCHBOX_UART_EN
                        lunchbox_wake_reason_set_key();
#endif
                        gui_need_wkp = true;
                        break;
                    }
                    /* 误触: 不走共享cleanup, 自己清理+统一上升沿+强睡标定 */
                    RTCCON9 = BIT(7) | BIT(5) | BIT(2);
                    port_wakeup_init(PT8028_GPIO_OUT_FLAG, 0, 1);
                    elunchbox_waiting_key_release = true;
                    force_spin_cnt = 4;
                    continue;
                }
                /* 非TCH5/无效: PE1仍LOW→切上升沿等松手; PE1已HIGH→保持下降沿 */
                RTCCON9 = BIT(7) | BIT(5) | BIT(2);
                if (((GPIOE >> 1) & 1) == 0) {
                    port_wakeup_init(PT8028_GPIO_OUT_FLAG, 0, 1);
                    elunchbox_waiting_key_release = true;
                } else {
                    elunchbox_waiting_key_release = false;
                }
                continue;
            } else
#endif
            {
                printf(port_wakeup_str, wkpnd);
                gui_need_wkp = true;
                break;
            }
        }

        /* WKO / RTC wakeup */
        if ((RTCCON9 & BIT(2)) || (RTCCON10 & BIT(2)) || wko_wkup_flag) {
#if ELUNCHBOX_PANEL_EN
            if (manual_off) { /* skip */ } else
#endif
            {
                printf(wko_wakeup_str);
                gui_need_wkp = true;
                break;
            }
        }

#if LE_EN
        if (ble_app_need_wakeup()) {
#if ELUNCHBOX_PANEL_EN
            if (manual_off) { /* skip */ } else
#endif
            {
                printf(app_wakeup_str);
                gui_need_wkp = true;
                break;
            }
        }
#endif

        if (co_timer_pro(true)) {
#if ELUNCHBOX_PANEL_EN
            if (manual_off) { /* skip */ } else
#endif
            {
                printf(co_timer_wakeup_str);
                break;
            }
        }

        if (bt_cb.call_type) {
#if ELUNCHBOX_PANEL_EN
            if (manual_off) { /* skip */ } else
#endif
            {
                printf(call_wakeup_str);
                gui_need_wkp = true;
                break;
            }
        }
    }
    /* 真唤醒 → 重置松手等待标志，确保下次进 manual_off 休眠使用下降沿唤醒 */
    if (gui_need_wkp) {
        elunchbox_waiting_key_release = false;
    }
    printf("lp: while exit loop=%u sleep_cnt=%u wkp=%u\n",
           lp_loop_cnt, lp_sleep_cnt, gui_need_wkp ? 1u : 0u);
    PWRCON0 = (PWRCON0 & ~0x1ff) | vddio_vddcore_level;

    /* 【低功耗优化】恢复 VDDTK LDO */
#if LPWR_VDDTK_OFF_EN && ELUNCHBOX_PANEL_EN
    if (manual_off && rtccon1_vddtk_saved) {
        RTCCON1 |= BIT(9);
    }
#endif

#if ELUNCHBOX_PANEL_EN
    printf(lp_exit_str, gui_need_wkp ? 1u : 0u, manual_off ? 1u : 0u);
#endif

    return gui_need_wkp;
}

/* 【休眠】同一轮 manual_off 会话只发一次通知，防反复睡→醒→睡 风暴 */
static bool s_pwroff_sent = false;

void elunchbox_pwroff_sent_reset(void)
{
    s_pwroff_sent = false;
    lb_uart_tx_block(false);
}

/* 【休眠主函数】sfunc_sleep — 熄屏 + 关外设 + sfunc_sleep_proc 深度休眠
 *   auto_guioff: guioff_slp=1, manual_off=0 → BLE 降参数, 保留部分唤醒源
 *   manual_off:  guioff_slp=1, manual_off=1 → 关 BT wakeup, 关 RTC WDT, 仅保留 PE1+PB9 唤醒
 *   normal:      guioff_slp=0 → gui_sleep 熄屏后进入深度休眠
 */
static void sfunc_sleep(void)
{
    uint32_t usbcon0, usbcon1;
    u16 pa_de, pb_de, pe_de, pf_de, pg_de, ph_de;
    u16 pb_dir, pb_pu, pb_pd;
    u16 pe_pu, pe_pd, pf_pu, pf_pd, ph_pu, ph_pd;
    u16 pb_pu200k, pb_pd200k, pb_pu300, pb_pd300;
    u16 pe_pu200k, pe_pd200k, pe_pu300, pe_pd300;
    u16 adc_ch;
    uint32_t sysclk;
    u32 wkie;
    bool gui_need_wkp = false;
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    bool elunchbox_guioff_slp = elunchbox_pwr_gui_off_is_on();
    bool elunchbox_manual_off_slp = elunchbox_guioff_slp && elunchbox_pwr_is_manual_off();
    printf("elunchbox: sfunc_sleep manual_off_slp=%u [BUILD 0731-1100]\n",
           elunchbox_manual_off_slp ? 1u : 0u);
#else
    bool elunchbox_guioff_slp = false;
    bool elunchbox_manual_off_slp = false;
#endif
#if LE_EN
    u16 interval = 0;
    u16 latency = 0;
    u16 tout = 0;
    u16 adv_interval = 0;
#endif
    sys_cb.flag_sleep_ble_status = ble_is_connect();
    u8 dac_status = dac_get_power_status();
#if LPWR_BUCK_TO_LDO_EN && ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    u8 buck_saved = 0;
#endif

#if VBAT_DETECT_EN
    if (bsp_vbat_get_lpwr_status()) {           //低电不进sniff mode
        return;
    }
#endif

#if ELUNCHBOX_PANEL_EN
    if (!elunchbox_manual_off_slp)
#endif
        printf("%s%s\n", __func__, elunchbox_guioff_slp ? "(elunchbox guioff)" : "");
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_guioff_slp) {
        elunchbox_guioff_sleep_mode_enter();
    }
#endif

#if VIDEO_PLAY_EN
    if ((api_video_play_sta_get() != AVI_STA_STOP) && (func_cb.sta == FUNC_CLOCK)) {
        bsp_video_play_uninit();
    }
#endif

#if AVI_DVP_USE_CAMERA && IMG_SENSOR_SELECT
    bsp_image_sensor_pwdn();
#endif
    sleep_cb.sys_is_sleep = true;
    sys_cb.gui_need_wakeup = 0;
    printf("slp: A (bt_enter_sleep)\n");
    bt_enter_sleep();
    bt_audio_bypass();
    while(btstack_audio_is_busy());
    printf("slp: B (audio idle)\n");
#if LE_EN
    adv_interval = ble_get_adv_interval();
    /* manual_off: 不能关广播！ble_adv_dis() 会让 BT 栈进入等完成状态
     * → bt_sleep_proc() 永远返回 0 → sys_enter_sleep 永远不会被调。
     * 只拉长间隔，BT 栈就能正常 sleep。BLE 唤醒已被忽略。 */
    ble_set_adv_interval(1600);                  //interval: 500 * 0.625ms = 500ms
    if (ble_is_connect()) {                     //ble已连接
        interval = ble_get_conn_interval();
        latency = ble_get_conn_latency();
        tout = ble_get_conn_timeout();
        ble_update_conn_param(410, 0, 500);     //interval: 410*1.25ms = 512.5ms
    }
#endif
#if BT_SINGLE_SLEEP_LPW_EN
    if (!bt_is_connected()){                    //蓝牙未连接
        if (bt_get_scan()) {                    //双模休眠
            bt_update_bt_scan_param(4096, 8, 4096, 12);
        } else {
            bt_update_bt_scan_param(4096, 0, 4096, 0);
            bt_scan_disable();
        }
    }
#else
    if (!bt_is_connected()){                    //蓝牙未连接
        bt_update_bt_scan_param(4096, 12, 2048, 12);
    }
#endif

#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    /* manual_off: 强制关 BT scan。
     * 第一次上电时 scan 未开启所以 bt_scan_disable 生效 → bt_sleep_proc 可睡。
     * 唤醒亮屏后 scan 被 bt_update_bt_scan_param_default 恢复 → 第二次进
     * manual_off 时 bt_get_scan()=true → 上面只调参不关 scan → bt_sleep_proc
     * 不睡 → 10mA。此处强制关掉。 */
    if (elunchbox_manual_off_slp) {
        bt_scan_disable();
    }
#endif
    printf("slp: C (bt param/scan done)\n");

#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

#if (ASR_SELECT && ASR_FULL_SCENE)
    bsp_asr_stop();
#endif

    if (dac_status) {
        dac_power_off();                            //dac power down
    }

    sys_set_tmr_enable(0, 0);

    adc_ch = bsp_saradc_exit();                //close saradc及相关通路模拟
    saradc_clr_channel(adc_ch);
    saradc_set_channel(BIT(ADCCH_VBAT) | BIT(ADCCH_BGOP));
    saradc_adc15_ana_set_channel(ADCCH15_ANA_BG);

#if CHARGE_EN
    bsp_charge_set_stop_time(3600);
    charge_set_detect_cnt(1);
#if CHARGE_VOL_DYNAMIC_DET
    if(sys_cb.chg_on){
        gradient_process(0);
        RTCCON8 &= ~BIT(1);                                  //charge open
    }
#endif
#endif

    printf("slp: D (dac/adc/charge done, dac_was=%u)\n", dac_status);
    usbcon0 = USBCON0;                          //需要先关中断再保存
    usbcon1 = USBCON1;
    USBCON0 = BIT(5);
    USBCON1 = 0;
    printf("slp: D1 (usb off)\n");
#if SD_SUPPORT_EN
    SD0_LDO_DIS();
#endif

    /* 【熄屏】非饭盒路径：休眠前先关屏；饭盒路径已在 sleep_process 中提前熄屏 */
    if (!elunchbox_guioff_slp) {
        gui_sleep(true);
    }
    printf("slp: D2 (gui_sleep/sd done)\n");

#if MODEM_CAT1_EN
    bsp_modem_sleep_enter();
#endif

#if USER_KEY_QDEC_EN || USER_ADKEY_QDEC_EN
    bsp_qdec_exit();                            //旋转编码器exit
#endif

    sysclk = sys_clk_get();
    printf("slp: D3 (sysclk=%u -> 24M)\n", (unsigned)sysclk);
    sys_clk_set(SYS_24M);
    printf("slp: D4 (clk switched)\n");
    DACDIGCON0 &= ~BIT(0);                      //disable digital dac
    printf("slp: E1 (dacdig off)\n");
    adda_clk_source_sel(1);                     //adda_clk48_a select xosc52m
    printf("slp: E2 (adda clk sel)\n");
    /* PLL0/PLL1 不在此处手动关断。实测 bt_enter_sleep 后关 PLL0
     * 会导致 AHB 总线挂死 → RTC_WDT 复位。让 rtc_sleep_enter →
     * sys_enter_sleep 硬件序列去关 PLL, 省掉这段手动关断。
     * 唤醒端 adpll_init() 会重新使能 PLL0, 不受影响。 */
    printf("slp: E3 (pll0 off skipped)\n");
#if FUNC_LUNCHBOX_UART_EN
    /* 关 UART1, 释放 PB8/PB9 回 GPIO, 以便下面 rtc_sleep_enter 后
     * GPIO 配置段自由设置引脚状态 (digital + pull)。*/
    lunchbox_uart_suspend();
    printf("slp: D0 (uart1 off)\n");
#endif
    printf("slp: E (before rtc_sleep_enter)\n");
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_manual_off_slp) {
        RTC_WDT_DIS();
    }
#endif
    rtc_sleep_enter();
    printf("slp: F (rtc_sleep_enter ret)\n");

    //io analog input — 完整保存所有 GPIO 配置
    pa_de = GPIOADE;
    pb_de = GPIOBDE;
    pe_de = GPIOEDE;
    pf_de = GPIOFDE;
    pg_de = GPIOGDE;
    ph_de = GPIOHDE;
    pb_dir = GPIOBDIR;
    pb_pu  = GPIOBPU;
    pb_pd  = GPIOBPD;
    pe_pu  = GPIOEPU;
    pe_pd  = GPIOEPD;
    pf_pu  = GPIOFPU;
    pf_pd  = GPIOFPD;
    ph_pu  = GPIOHPU;
    ph_pd  = GPIOHPD;
    pb_pu200k = GPIOBPU200K;
    pb_pd200k = GPIOBPD200K;
    pb_pu300  = GPIOBPU300;
    pb_pd300  = GPIOBPD300;
    pe_pu200k = GPIOEPU200K;
    pe_pd200k = GPIOEPD200K;
    pe_pu300  = GPIOEPU300;
    pe_pd300  = GPIOEPD300;
    if(vddio_sleep_level) {
        GPIOADE = BIT(7);
    } else {
        GPIOADE = 0; // PA7 rst
    }

    GPIOBDE = BIT(3);
    GPIOGDE = 0x3F;                             //MCP FLASH

    u32 pf_keep = 0;

#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_guioff_slp) {
        if (elunchbox_manual_off_slp) {
            /* PE1(FLAG) + PE2~4(D0~D2) 全数字输入。
             * PE2~4 保留 200K 弱上拉 —— 匹配 PT8028 空闲态 BCD=111,
             * 防止 PT8028 扫描间隙 tri-state 时引脚浮空漏电。 */
            GPIOEDE = BIT(1) | BIT(2) | BIT(3) | BIT(4);
            /* DEBUG(0731): PB3 暂保留数字 —— 日志 UART TX 在 PB3(UART0_PRINTF_SEL=
             * PRINTF_PB3), 此处切模拟会让 F 之后的定位打印全部丢失。
             * 验证完成后恢复为 BIT(9)|BIT(8) 省电。 */
            GPIOBDE = BIT(3) | BIT(9) | BIT(8);     /* PB3 日志 + PB9 门铃 + PB8 防浮空 */
            /* PB8+PB9: 数字输入+上拉, 防浮空漏电 */
            GPIOBDIR = 0;
            GPIOBPU  = BIT(9) | BIT(8);
            GPIOBPD  = 0;
            GPIOBPU200K = 0;
            GPIOBPD200K = 0;
            GPIOBPU300  = 0;
            GPIOBPD300  = 0;
            /* PE2~4: 200K 弱上拉, 清强上下拉 */
            GPIOEPU &= ~(BIT(2) | BIT(3) | BIT(4));
            GPIOEPD &= ~(BIT(2) | BIT(3) | BIT(4));
            GPIOEPU200K |= (BIT(2) | BIT(3) | BIT(4));
            GPIOEPD200K &= ~(BIT(2) | BIT(3) | BIT(4));
            GPIOEPU300  &= ~(BIT(2) | BIT(3) | BIT(4));
            GPIOEPD300  &= ~(BIT(2) | BIT(3) | BIT(4));
            GPIOFPU = 0;
            GPIOFPD = 0;
            GPIOHPU = 0;
            GPIOHPD = 0;
        } else {
            GPIOBDE = BIT(3) | BIT(8) | BIT(9); /* PB3 日志 / PB8 PB9 UART1 */
            GPIOEDE = BIT(0) | BIT(1);            /* PE0+PE1: UART/I2C 交互需要 */
        }
        GPIOFDE = 0;
        GPIOHDE = 0;                                 /* Port H all analog, save power */
    } else
#endif
    {
        u8 sensor_type = bsp_sensor_init_sta_get(SENSOR_INIT_ALL);
        if (sensor_type) {
            bool sensor_type_hr = (sensor_type & SENSOR_INIT_HR);
            bool sensor_type_step = (sensor_type & SENSOR_INIT_STEP);
            u32 gpioede = (BIT(4) | BIT(3))*sensor_type_hr | (BIT(8) | BIT(7))*sensor_type_step;
            gpioede |= BIT(4) | BIT(3);          //SENSOR I2C
            pf_keep |= BIT(2);                      //SENSOR PG
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
    WKUPCON &= ~BIT(16);                        //休眠时关掉WKIE

#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_guioff_slp) {
        /* UART 应答排空: poll PB9 until HIGH, timeout ~400ms */
        delay_5ms(6);
        {
            int drain = 0;
            while (((GPIOB >> 9) & 1) == 0 && drain < 4000) {
                delay_us(100);
                WDT_CLR();
                drain++;
            }
        }
        printf("slp: H (drain done)\n");
    }
#endif
    printf("slp: I (gpio/uart cfg done)\n");

    sleep_wakeup_config();
    RTCCON9 = BIT(2);   /* clr spurious port pending from wakeup config */
    printf("slp: J (sleep_wakeup_config done)\n");

#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_guioff_slp) {
        /* manual_off: 先关 BT wakeup，再配 port wakeup 仅 PE1+PB9 */
        if (elunchbox_manual_off_slp) {
            RTCCON3 |= BIT(17);         /* re-enable port wakeup */
            RTCCON3 &= ~BIT(13);        /* disable bt wakeup */
            BTCON2 &= ~(3 << 10);       /* disable bt sleep wakeup */
            RTCCON  &= ~(0xf << 7);     /* disable rtc sleep wakeup */
        }
        /* PE1: 检测当前电平, LOW则用上升沿等松手, HIGH则用下降沿等按下 */
        {
            u8 pe1_edge = ((GPIOE >> 1) & 1) ? 1 : 0;  /* HIGH→下降沿(1), LOW→上升沿(0) */
            port_wakeup_init(PT8028_GPIO_OUT_FLAG, pe1_edge, 1);
            elunchbox_waiting_key_release = (pe1_edge == 0);  /* 上升沿=等松手 */
        }
        port_wakeup_all_init(IO_PB9, 1, 1);
        printf("elunchbox: sfunc_sleep %s PE1=%s PB9=%s wakeup configured\n",
               elunchbox_manual_off_slp ? "manual_off" : "auto_guioff",
               ((GPIOE >> 1) & 1) ? "HI" : "LO",
               ((GPIOB >> 9) & 1) ? "HI" : "LO");
    }
#endif

    /* 【低功耗优化】manual_off: 切 BUCK→LDO 模式, 省 50-200μA */
#if LPWR_BUCK_TO_LDO_EN && ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_manual_off_slp) {
        buck_saved = 1;
        set_buck_mode(0);
    }
#endif

    elunchbox_manual_off_in_sleep = elunchbox_manual_off_slp;
    printf("elunchbox: [DBG] -> sfunc_sleep_proc manual_off=%u bt_sleep=%u\n",
           elunchbox_manual_off_slp ? 1u : 0u, bt_is_sleep() ? 1u : 0u);
    gui_need_wkp = sfunc_sleep_proc();          //进入休眠
    printf("elunchbox: [DBG] <- sfunc_sleep_proc gui_need_wkp=%u\n",
           gui_need_wkp ? 1u : 0u);
    elunchbox_manual_off_in_sleep = false;

    RTCCON9 = BIT(7) | BIT(5) | BIT(2);         //clr port, bt, wko wakeup pending
    RTCCON3 &= ~(BIT(17) | BIT(13));            //disable port, bt wakeup
    sleep_wakeup_exit();
    if (cfg_bt_sniff_clk_sel != 5) {
        lp_xosc_en();
    }
    WKUPCON |= wkie;                            //还原WKIE

    GPIOADE = pa_de;
    GPIOBDE = pb_de & ~BIT(3);               /* PB3 暂缓，等亮屏再开 */
    GPIOEDE = pe_de;
    GPIOFDE = pf_de;
    GPIOGDE = pg_de;
    GPIOHDE = ph_de;
    GPIOBDIR = pb_dir;
    GPIOBPU  = pb_pu;
    GPIOBPD  = pb_pd;
    GPIOEPU  = pe_pu;
    GPIOEPD  = pe_pd;
    GPIOFPU  = pf_pu;
    GPIOFPD  = pf_pd;
    GPIOHPU  = ph_pu;
    GPIOHPD  = ph_pd;
    GPIOBPU200K = pb_pu200k;
    GPIOBPD200K = pb_pd200k;
    GPIOBPU300  = pb_pu300;
    GPIOBPD300  = pb_pd300;
    GPIOEPU200K = pe_pu200k;
    GPIOEPD200K = pe_pd200k;
    GPIOEPU300  = pe_pu300;
    GPIOEPD300  = pe_pd300;
    USBCON0 = usbcon0;
    USBCON1 = usbcon1;

#if ELUNCHBOX_PANEL_EN
    if (!elunchbox_manual_off_slp || gui_need_wkp)
#endif
    {
        bt_update_bt_scan_param_default();
#if BT_SINGLE_SLEEP_LPW_EN
        if(!bt_is_connected() && bt_get_scan()){    //单模
            bt_scan_enable();
        }
#endif
    }
#if ELUNCHBOX_PANEL_EN
    else if (!bt_is_connected()) {
        bt_scan_disable();
    }
#endif
#if SD_SUPPORT_EN
    SD0_LDO_EN();
#endif
    rtc_sleep_exit();
    CLKCON0 = (CLKCON0 & ~(0x03 << 2)) | (0x01 << 2); //sysclk select xosc26m_clk
    RSTCON0 &= ~BIT(4);                         //pllsdm disable
    adpll_init(DAC_OUT_SPR);                    //enable adpll
    adda_clk_source_sel(0);                     //adda_clk48_a select pll0
    DACDIGCON0 |= BIT(0);                      //enable digital dac
#if FPGA_EN
    fpga_uart_reinit();
#endif
#if FUNC_LUNCHBOX_UART_EN
    /* UART1 在睡下前 suspend 过，这里恢复。须在系统时钟恢复后 (adpll_init) 再做，
     * 因为 lb_link_init → uart_init 用 sys_clk 算波特率。 */
    lunchbox_uart_resume();
#endif
    dac_aubuf_init();
    /* 【低功耗优化】恢复 BUCK 模式 */
#if LPWR_BUCK_TO_LDO_EN && ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (buck_saved) {
        set_buck_mode(1);
    }
#endif
    sys_clk_set(sysclk);
    saradc_set_channel(adc_ch);
    bsp_saradc_init();
#if USER_KEY_QDEC_EN || USER_ADKEY_QDEC_EN
    bsp_qdec_init();                            //旋转编码器初始化
#endif
#if CHARGE_EN
    bsp_charge_set_stop_time(18000);
    bsp_charge_sta(sys_cb.charge_sta);          //update充灯状态
    charge_set_detect_cnt(5);
#if CHARGE_VOL_DYNAMIC_DET
    if(sys_cb.chg_on){
        gradient_process(0);
        RTCCON8 &= ~BIT(1);                     //charge open
    }
#endif
#endif // CHARGE_EN
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

    /* ELUNCHBOX FIX: screen_wake 依赖 CLKGAT0 已恢复 */
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_guioff_slp && elunchbox_pwr_is_manual_off()) {
        CLKGAT0 = elunchbox_saved_clkgat0;
    }
#endif

    if (gui_need_wkp) {
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
        if (elunchbox_guioff_slp) {
            elunchbox_guioff_sleep_post_wake(true);
            /* manual_off + gui_need_wkp: 恢复 GPU → 等主循环亮屏 */
        } else
#endif
        {
//        func_create_form(func_cb.sta);
            gui_wakeup();
        }
    } else {
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
        if (elunchbox_guioff_slp) {
            elunchbox_guioff_sleep_post_wake(false);
        } else
#endif
        {
            gpu_init();
        }
    }

    /* guioff wake: restore GPU now (auto_guioff + manual_off 统一, display on 由主循环排队) */
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_guioff_slp && gui_need_wkp && sys_cb.gui_sleep_sta) {
        elunchbox_pwr_intentional_wake_set(true);
        gui_wakeup();
        elunchbox_pwr_intentional_wake_set(false);
        /* lunchbox_display_on 由主循环 elunchbox_pwr_gui_wake 调用,
         * 确保在 gui_wakeup 推完帧之后才开屏 */
    }
#endif

#if LE_EN
    ble_set_adv_interval(adv_interval);
    if (interval | latency | tout) {
        ble_update_conn_param(interval, latency, tout); //还原连接参数
    }
#endif

    if (dac_status) {
        dac_restart();
    }
    plugin_music_eq();
    bsp_change_volume(sys_cb.vol);

#if (ASR_SELECT && ASR_FULL_SCENE)
    bsp_asr_start();
#endif

#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif
    bt_exit_sleep();
    bt_audio_enable();

#if AVI_DVP_USE_CAMERA && IMG_SENSOR_SELECT
    if (gui_need_wkp) {
        image_sensor_drv_auto_reg();
    }
#endif

    if (((u32)&__dynamic_pool_end > 0x60000) && is_gpu_init()) {     //如果ab_malloc跨sram和gpuram将会清空malloc内存
        void customer_heap_init(void);
        customer_heap_init();
    }

    sleep_cb.sys_is_sleep = false;
    /* 亮屏后再开 PB3 调试打印 */
    GPIOBDE = pb_de;

#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    printf("sleep_exit manual_off_slp=%u\n", elunchbox_manual_off_slp ? 1u : 0u);
#else
    printf("sleep_exit\n");
#endif
}

bool sleep_process(is_sleep_func is_sleep)
{
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    /* guioff 或 classic gui_sleep_sta → 屏幕已关 → 可以进浅睡/深睡 */
    if (elunchbox_pwr_gui_off_is_on() || sys_cb.gui_sleep_sta) {
        sys_cb.gui_need_wakeup = 0;

        /* Auto guioff: 加热进行中 → 唤醒屏幕 */
        if (!elunchbox_pwr_is_manual_off() && elunchbox_heating_blocks_idle()) {
            printf("elunchbox: sleep_process heating auto wake (not manual_off)\n");
            elunchbox_pwr_gui_wake_reason("sleep_process heating");
            reset_sleep_delay_all();
            return false;
        }

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        /* Auto guioff: TCH5 按住中 → 保持唤醒等长按计时 */
        if (!elunchbox_pwr_is_manual_off()) {
            if (pt8028_get_press_tch() == 5) {
                reset_sleep_delay();
                reset_pwroff_delay();
                return false;
            }
        }
#endif

        /* Manual off / auto_guioff: 强制进深度休眠 (绕过 bt_is_allow_sleep) */
        bool force_lowpwr = false;
        if (elunchbox_pwr_is_manual_off()) {
            if (elunchbox_pwr_manual_off_should_stay_awake()) {
                reset_sleep_delay();
                reset_pwroff_delay();
                return false;
            }
            force_lowpwr = true;
        }
        if (elunchbox_guioff_sleep_ready()) {
            force_lowpwr = true;
        }

        /* 进浅睡/深睡 */
        if (force_lowpwr) {
            printf("elunchbox: sleep_process force_lowpwr -> sfunc_sleep\n");
            /* 深度休眠前须退出 GPU (auto_guioff + manual_off 统一) */
            if (!sys_cb.gui_sleep_sta) {
                gui_sleep(true);
            }
            sfunc_sleep();
            reset_sleep_delay_all();
            reset_pwroff_delay();
            return false;
        }
        reset_sleep_delay();
        reset_pwroff_delay();
        return false;
    } else
#endif
#if ELUNCHBOX_PANEL_EN
    if (elunchbox_pwr_gui_off_is_on() && sys_cb.gui_sleep_sta) {
        sys_cb.gui_need_wakeup = 0;
    } else
#endif
    if (sys_cb.gui_need_wakeup && sys_cb.gui_sleep_sta) {
        printf("elunchbox: sleep_process gui_need_wakeup=1 manual=%u\n",
               elunchbox_pwr_is_manual_off() ? 1u : 0u);
        gui_wakeup();                   //按键亮屏
        reset_sleep_delay_all();
        sys_cb.gui_need_wakeup = 0;
        return false;
    }


#if VIDEO_PLAY_EN
    if((api_video_play_sta_get() != AVI_STA_STOP) && (func_cb.sta != FUNC_CLOCK)){
        reset_sleep_delay_all();
        return false;
    }
#endif // VIDEO_PLAY_EN

#if VIDEO_RECODE_TAKE_PHOTO_EN
    if(bsp_video_recode_is_start()){
        reset_pwroff_delay();
    }
#endif // VIDEO_RECODE_TAKE_PHOTO_EN

#if AVI_DVP_USE_CAMERA
    if(bsp_image_sensor_is_init()) {
        return false;
    }
#endif // AVI_DVP_USE_CAMERA

#if LE_EN
    if (ble_app_need_wakeup()) {
        reset_sleep_delay_all();
        reset_pwroff_delay();
        return false;
    }
#endif
#if ELUNCHBOX_PANEL_EN
    /* 饭盒：5min 无操作 = 自动关机, 与长按 TCH5 一样立 manual_off 标志,
     * 进 sfunc_sleep 的代码完全一致 (PE1+PB9 port wakeup / RTC_WDT_DIS).
     * 多一句 lb_heat_cmd_heat_off() 通知模块停加热, 一发即走不等应答.
     * 不可走 lunchbox_shutdown_start: 异步等 ack 期间 bt_is_allow_sleep 可能
     * 先跑进正常深睡(未配 PE1/PB9 wakeup) → 唤不醒. */
    if (elunchbox_guioff_idle_expired()) {
#if FUNC_LUNCHBOX_UART_EN
        if (func_cb.sta == FUNC_BLACK_SCREEN) {
            elunchbox_lp_user_activity_reset();
            return false;
        }
        if (!elunchbox_pwr_is_manual_off()) {
            printf("elunchbox: idle %us -> manual_off deep sleep\n",
                   (unsigned)ELUNCHBOX_GUIOFF_TIME_SEC);
            lb_heat_cmd_heat_off();               /* 停加热, 不等应答 */
            lb_heat_cmd_power(false);             /* 关模块, 不等应答 */
            elunchbox_pwr_manual_off_set();       /* 立手动关机标志 → sfunc_sleep 配 PE1+PB9 唤醒 */
            elunchbox_screen_off();
            elunchbox_guioff_sleep_arm_immediate();
        }
#else
        printf("elunchbox: idle %us -> manual_off deep sleep\n",
               (unsigned)ELUNCHBOX_GUIOFF_TIME_SEC);
        elunchbox_pwr_manual_off_set();
        elunchbox_screen_off();
        elunchbox_guioff_sleep_arm_immediate();
#endif
        return false;
    }
    /* 已息屏但加热进行中：自动亮回 */
    if (elunchbox_pwr_gui_off_is_on() && sys_cb.gui_sleep_sta
        && elunchbox_heating_blocks_idle()) {
        printf("elunchbox: sleep_process guioff heating wake manual=%u\n",
               elunchbox_pwr_is_manual_off() ? 1u : 0u);
        elunchbox_pwr_gui_wake_reason("sleep_process guioff_heating");
        reset_sleep_delay_all();
        return false;
    }
#endif
    if ((*is_sleep)() && (sleep_is_only_gui_off() == false)
#if VIDEO_RECODE_TAKE_PHOTO_EN
        && (bsp_video_recode_is_start() == false)
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
        ) {
#if ELUNCHBOX_PANEL_EN
        if (!sys_cb.sleep_en) {
            /* KEEP_AWAKE：不进深度休眠；息屏由 elunchbox_idle_tmr 统一处理 */
            reset_sleep_delay();
            reset_pwroff_delay();
            return false;
        }
#else
        if (!sys_cb.sleep_en) {
            reset_sleep_delay_all();
            return false;
        }
#endif
        /* 经典深度休眠入口：sleep_delay 倒计时到 0 → gui_sleep + sfunc_sleep */
        if (sys_cb.sleep_delay == 0) {
            gui_sleep_psram_check();
            sfunc_sleep();              //熄屏且进入休眠
            reset_sleep_delay_all();
            reset_pwroff_delay();
            return true;
        }
    } else {
#if !ELUNCHBOX_PANEL_EN
        if (sys_cb.guioff_delay == 0 && !sys_cb.gui_sleep_sta) {
            gui_sleep(false);                //仅熄屏
        }
#endif
        reset_sleep_delay();
        reset_pwroff_delay();
    }
    return false;
}

////红外关机进入sleep mode
//AT(.text.lowpwr.sleep)
//void func_sleepmode(void)
//{
//
//}

///解决充满关机等待5V掉电，超时1分钟
AT(.text.pwroff.vusb)
u8 sfunc_pwrdown_w4_vusb_offline(void)
{
//    u32 timeout = 10000, off_cnt = 0;
//    while (timeout--) {
//        WDT_CLR();
//        delay_us(330);                          //5ms
//        if (!CHARGE_DC_IN()) {
//            off_cnt++;
//        } else {
//            off_cnt = 0;
//        }
//        if (off_cnt > 5) {
//            return 1;                           //VUSB已掉线，打开VUSB唤醒
//        }
//    }
    return 0;                                   //关VUSB唤醒
}

AT(.text.pwroff.enter)
void sfunc_power_save_enter(void)
{
#if LP_XOSC_CLOCK_EN
    bool lp_xosc_clock_err = lp_xosc_check();
#endif

    sys_clk_set(SYS_24M);
    LOUDSPEAKER_MUTE_DIS();
    WDT_CLR();
#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        bsp_charge_off();
    }
#endif
    set_buck_mode(0);                           //关机前切回ldo mode
    vusb_delay_rst_dis();
//    RTCCON4 = (RTCCON4 & ~(3 << 5)) | (1 << 5); //RI_VCORE_SVBT = 0, RI_VCORE_SVIO = 1
    RTCCON4 |= BIT(5) | BIT(6);
    delay_us(100);
    RTCCON4 &= ~BIT(6);
    USBCON0 = BIT(5);
    USBCON1 = 0;
    GPIOADE = 0;
    GPIOBDE = 0; //高阻态，模拟输入->省电
    GPIOEDE = 0;
    GPIOFDE = 0;
    GPIOGDE = 0x3F;                             //MCP FLASH
    GPIOHDE = 0;
    RTCCON8 &= ~BIT(15);                        //RI_EN_VUSBDIV = 0

#if !LP_XOSC_CLOCK_EN
    RTCCON15 &= ~(3 << 13);                     //disable LP_XOSC
    if (sys_cb.flag_shipping_mode) {
        RTCCON0 &= ~(BIT(0) | BIT(20) | BIT(2)); //RI_EN_SNIFF = 0, RI_EN_RINGOSC = 0, 关掉RTC_RC
    }
#else
    RTCCON0 &= ~(BIT(0) | BIT(20) | BIT(2));    //RI_EN_SNIFF = 0, RI_EN_RINGOSC = 0, 关掉RTC_RC
    if (lp_xosc_clock_err) {
        RTCCON15 &= ~(3 << 13);                 //disable LP_XOSC
    }
#endif

    PICCONCLR = BIT(0);                         //Globle IE disable
    CLKCON0 |= BIT(0);                          //enable RC
    CLKCON0 = (CLKCON0 & ~(0x3 << 2));          //system clk select RC

    RSTCON0 &= ~(BIT(4) | BIT(6));              //pllsdm, adda_clk
    CLKGAT2 &= ~(BIT(0) | BIT(1));              //disable dac clk
    PLL0CON0 &= ~(BIT(6) | BIT(18));             //PLL0 disable
    XOSCCON0 &= ~((0xf<<24) | (3 << 6) | (7 << 10) | BIT(5));//X24 output, xosc, xosc24m disable
    PLL0CON0 &= ~(BIT(12) | BIT(0) | BIT(1));    //PLL0 & XOSC关电
    RTCCON9 = 0xfff;                            //Clr pending
}

/* 【低电关机-硬开关方案】关闭所有外设 → sleep mode，仅等 VUSB 充电唤醒 */
AT(.text.pwroff)
void sfunc_lowbat_do(void)
{
    sfunc_power_save_enter();
    WDT_DIS();
    RTCCON3 |= BIT(11);                         //VUSB Wakeup enable
    while(1) {
        LPMCON |= BIT(0);                       //Sleep mode enable
        asm("nop");asm("nop");asm("nop");
        if (RTCCON10 & BIT(3)) {                //VUSB wake up pending
            WDT_RST();
        }
    }
}

/* 【软关机-最终断电】关闭所有外设/时钟/PLL → power down mode，等 WK 引脚/VUSB 唤醒 */
AT(.text.pwroff.pwrdwn)
void sfunc_pwrdown_do(u8 vusb_wakeup_en)
{
#if !LP_XOSC_CLOCK_EN
    if (!sys_cb.flag_shipping_mode) {
        rtc_set_alarm_wakeup(900);              //15min
    }
#endif

    sys_set_tmr_enable(0, 0);
    sfunc_power_save_enter();
    if (!vusb_wakeup_en) {
        RTCCON8 = (RTCCON8 & ~BIT(6)) | BIT(1); //disable charger function
        vusb_wakeup_en = sfunc_pwrdown_w4_vusb_offline();
    }
    RTCCON1 &= ~(BIT(5) | BIT(7));              //BIT(7): VRTC voltage for ADC, BIT(5):WK pin analog enable, output WKO voltage for ADC
    RTCCON11 = (RTCCON11 & ~0x03) | BIT(2);     //WK PIN filter select 8ms

    uint rtccon3 = RTCCON3 & ~BIT(11);
    uint rtccon1 = RTCCON1 & ~0x1f;
#if CHARGE_EN
    if ((xcfg_cb.charge_en) && (vusb_wakeup_en)) {
        rtccon3 |= BIT(11);                     //VUSB wakeup enable
    }
#endif
    LPPGCON0 = 0;                               //disable PG
    RTCCON0 &= ~BIT(5);                         //TKSW_RSTN disable
    RTCCON0 &= ~BIT(4);                         //tkif_en disable
    RTCCON1 &= ~BIT(9);                         //VDDTK disable

    RTCCON3 = rtccon3 & ~BIT(10);               //关WK PIN，再打开，以清除Pending
    rtccon3 = RTCCON3 & ~0x7;                  //Disable VDDCORE VDDIO VDDBUCK VDDIO_AON
    rtccon3 &= ~BIT(3);                         //RI_EN_VDD11 = 0
    rtccon3 |= BIT(6) | BIT(4) | BIT(19);       //PDCORE, PDCORE2, PDCORE3
    rtccon3 |= BIT(10);                         //WK pin wake up enable
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    rtccon3 |= BIT(7);                          //VDDIO AON：PT8028 与 PE 口在硬关机态仍需供电
    GPIOEDE |= (BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4));
    port_wakeup_init(PT8028_GPIO_OUT_FLAG, 1, 1);
    rtccon3 |= BIT(17);                         //port io wakeup enable
#elif SOFT_POWER_VDDIO_EN
    rtccon3 |= BIT(7);                          //VDDIO AON enable
#endif
//    rtccon3 |= BIT(16);                         //RTC_WDT wake ep enable
    rtccon1 |= BIT(0) | BIT(4);                 //WK 90K pull up enable
    WDT_DIS();
    RTCCON12 |= (0x3<<6);                       //DIS RTC_WDT
    BTCON2 |= BIT(23);                          //clear pending
    BTCON2 &= ~(3 << 10);                       //disable sleep wakeup
    RTCCON &= ~(0xf << 7);                      //disable sleep wakeup
    QDECCON &= ~BIT(2);                         //disable sleep wakeup
    WKUPCPND = (0xff << 16);                    //clear pending
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    WKUPCON |= BIT(17);                         //port io 唤醒（PE1/TCH5 OUT_FLAG 下降沿）
#else
    WKUPCON &= ~BIT(17);                        //disable sleep wakeup
#endif
    RTCCON &= ~(3 << 1);                        //避免关机时rtc配置来不及生效问题
    RTCCON |= BIT(5);                           //PowerDown Reset，如果有Pending，则马上Reset
    RTCCON1 = rtccon1;
    RTCCON3 = rtccon3;
    LPMCON |= BIT(0);                           //sleep mode
    LPMCON |= BIT(1);                           //idle mode
    asm("nop");asm("nop");asm("nop");
    while (1);
}

void sfunc_pwrdown(u8 vusb_wakeup_en)
{
    lock_code_pwrsave();
    sfunc_pwrdown_do(vusb_wakeup_en);
}

void sfunc_lowbat(void)
{
    lock_code_pwrsave();
    sfunc_lowbat_do();
}

void func_pwroff(int pwroff_tone_en)
{
    printf("%s\n", __func__);

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
//    gui_sleep();
#if !LP_XOSC_CLOCK_EN
    if (!sys_cb.flag_shipping_mode) {

        sniff_rc_init();
        rtc_rc_sleep_enter();
        rtc_calibration_write(PARAM_RTC_CAL_ADDR);
        cm_write8(PARAM_RTC_CAL_VALID, 1);
        cm_sync();
        rtc_printf();
    }
#endif

#if WARNING_POWER_OFF
    if (SOFT_POWER_ON_OFF) {
        mp3_res_play(RES_BUF_POWEROFF_MP3, RES_LEN_POWEROFF_MP3);
    }
#endif // WARNING_POWER_OFF

    gui_sleep(true);

    if (SOFT_POWER_ON_OFF) {
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        printf("elunchbox: wait OUT_FLAG release\n");
        /* 等待 TCH5 松手后再关机 */
        while (((GPIOE >> 1) & 1) == 0) {
            delay_5ms(1);
            WDT_CLR();
        }
        printf("elunchbox: enter sfunc_pwrdown\n");
#elif !PWRKEY_2_HW_PWRON
        while (IS_PWRKEY_PRESS()) {     //等待PWRKWY松开
            delay_5ms(1);
            WDT_CLR();
        }
#endif
        u8 dac_status = dac_get_power_status();
        if (dac_status) {
            dac_power_off();                    //dac power down
        }
        if (CHARGE_DC_IN()) {
            if (power_off_check()) {        //充电过程中等待结束再关机
                return;
            }
        }
        bsp_saradc_exit();                  //close saradc及相关通路模拟
        if ((PWRKEY_2_HW_PWRON) && (sys_cb.poweron_flag)) {
            RTCCON1 |= BIT(6);              //WK PIN High level wakeup
        }
        sfunc_pwrdown(1);
    } else {
        dac_power_off();                    //dac power down
        bsp_saradc_exit();                  //close saradc及相关通路模拟

        sfunc_lowbat();                     //低电关机进入Sleep Mode
    }
}
