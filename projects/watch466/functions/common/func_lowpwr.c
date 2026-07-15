#include "include.h"
#include "func.h"
#if ELUNCHBOX_PANEL_EN
#include "func_lunchbox_uart_internal.h"
extern bool lb_send_waiting;
#endif
#if ELUNCHBOX_PANEL_EN && FUNC_RESERVATION_UI_EN
#include "func_reservation.h"
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

AT(.sleep_backup.gui)
u8 sys_backup_buf[32 * 1024 - 8];

/* manual_off 休眠标志：sys_sleep_cb 在 sys_enter_sleep 前关 BT wakeup */
static bool elunchbox_manual_off_in_sleep;
/* PB9(UART/门铃)唤醒标志：sfunc_sleep 内直接亮屏，不走 3s 长按 */
static bool elunchbox_pb9_wake_source;
/* 非TCH5按键按住期间：PE1 wakeup已切为上升沿，等松手后恢复下降沿，解决PE1=LOW时清pending无法深睡的问题 */
static bool elunchbox_waiting_key_release;

extern u8 *cache_backup;
extern u32 __dynamic_pool_start, __dynamic_pool_end;
extern u32 elunchbox_saved_clkgat0;   /* func.c: 熄屏前保存, 唤醒时恢复, 防 CLKGAT0=0→8001 */

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

/* 【休眠/关机倒计时】500ms 定时器回调：递减 sleep_delay / guioff_delay / pwroff_delay
 *   sleep_delay → 0: 触发深度休眠(sfunc_sleep)
 *   guioff_delay → 0: 触发仅熄屏(gui_sleep)
 *   pwroff_delay → 0: 触发自动关机(func_pwroff)
 */
AT(.com_text.sleep)
void lowpwr_tout_ticks(void)
{
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

AT(.com_text.sleep)
bool sys_sleep_check(u32 *sleep_time)
{
	u32 co_min = co_timer_get_min_time(true)*2;

    if(*sleep_time > co_min) {
        *sleep_time = co_min;
    }

	if(*sleep_time < 4){
		*sleep_time = 4;
	}

    if(*sleep_time > sys_cb.sleep_wakeup_time) {
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
    //注意！！！！！！！！！！！！！！！！！
    //此函数只能调用sleep_text或com_text函数

    s_lpclk_type_saved = lpclk_type;            /* 保存供 manual_off 强制睡使用 */

    //此处关掉影响功耗的模块
    u32 gpiogde = GPIOGDE;
    if (gpiogde & BIT(6)) {
        GPIOGDE = BIT(2) | BIT(4) | BIT(6);         //SPICS, SPICLK
    } else {
        GPIOGDE = BIT(2) | BIT(4);                  //SPICS, SPICLK
    }

    /* manual_off: 最后一刻关所有非 GPIO 唤醒源。
     * RTCCON3[13] 不够 —— BT 栈还用 BTCON2/RTCCON 做 RTC 闹钟唤醒。
     * 硬关机路径(sfunc_pwrdown_do)也是关这三处。只留 PE1+PB9 port wakeup。 */
    if (elunchbox_manual_off_in_sleep) {
        RTCCON3 &= ~BIT(13);        /* disable bt wakeup */
        BTCON2 &= ~(3 << 10);       /* disable bt sleep wakeup */
        RTCCON  &= ~(0xf << 7);     /* disable rtc sleep wakeup */
        RTCCON9 = BIT(7) | BIT(5) | BIT(2); /* clr bt/wko/port pending, 防残留立即唤醒 */
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
        && (!elunchbox_pwr_is_manual_off()
#if FUNC_RESERVATION_UI_EN
            || func_reservation_is_waiting()
#endif
            )) {
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

//AT(.sleep_text.rodata.osc)
//const char lp_osc_str[] = "sleep_proc_delay: %d\n";

/* ---- manual_off 休眠内 helper (retention RAM) ---- */

/* 临时使能 PE2~PE4 数字输入，读 BCD 键值和 PE1 电平，然后恢复 GPIOEDE。
 * 返回 PE1 是否 LOW；*bcd 输出 BCD 值 (0~7)。
 * 调用前 PE1 必须已是数字输入（GPIOEDE 含 BIT(1)）。
 *
 * 【防假唤醒】PE2~4 在休眠时被切模拟+清上下拉以省电。
 * 此处临时切回数字输入时若不加下拉，floating 电平随机读成 5(TCH5 的 BCD 值)
 * → pe1_lo && bcd==5 → 假唤醒 → 20mA。加上下拉确保 BCD 读 0 而非 5。 */
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
    /* 弱上拉/下拉一起清，防止残留 pull 干扰 BCD 读数 */
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

    /* VDDCORE: 统一 1.0V。0.9V 可能导致 BT 数字逻辑/射频无法正常休眠。 */
    PWRCON0 = (PWRCON0 & ~0x1f) | 12;           // 1.0V
    if (vddio_level) {
        PWRCON0 = (PWRCON0 & ~(BIT(5)*0xF)) | BIT(5) * vddio_level;
    }

    /* 【低功耗优化】manual_off: 关闭 VDDTK LDO.
     * PT8028 自带电源完成触摸检测，通过 PE1 FLAG 输出数字电平；
     * JL5790T 内部触控控制器在休眠期间完全不使用，关掉省 10-50μA. */
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
     * 若不处理→退出 while → 整个 sfunc_sleep 重进(gui_sleep/gpu_exit) → 死循环。
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
            printf("lp: bt_sleep_proc %u→%u loop=%u sleep_cnt=%u bt_sleep=%u\n",
                   lp_last_status, status, lp_loop_cnt, lp_sleep_cnt, bt_is_sleep() ? 1u : 0u);
            lp_last_status = (u8)status;
        }
        /* heartbeat: loop spinning without sleep (every 256 iters) */
        if ((lp_loop_cnt & 0xFF) == 0 && status != 1) {
            printf("lp: SPIN loop=%u status=%u sleep_cnt=%u bt_sleep=%u\n",
                   lp_loop_cnt, status, lp_sleep_cnt, bt_is_sleep() ? 1u : 0u);
        }

#if SENSOR_HUB_EN
        bsp_senshb_lp_process();
#endif

        if (status == 1) {
            lp_sleep_cnt++;
#if ELUNCHBOX_PANEL_EN
            force_spin_cnt = 0;     /* BT 栈正常睡了 → 复位强睡计数 */
            if (manual_off) {
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
            /* bt_sleep_proc 连续返回非1 → BT栈不睡 → 绕过直接强睡.
             * 首次上电 BT 栈状态干净可正常睡; 唤醒后状态变化可能永远不睡.
             * 用 sys_sleep_cb 保存的 lpclk_type 直接 sys_enter_sleep. */
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
         * manual_off 唤醒源: 仅 PE1(TCH5按键) + PB9(UART RX) 两个。
         *   - PB9 LOW → 门铃唤醒
         *   - TCH5(bcd=5) 持续 LOW≥2s → 开机唤醒
         *   - 非TCH5/轻触: 清pending → continue(不离while, bt_sleep_proc重睡)
         * ================================================================ */
#if ELUNCHBOX_PANEL_EN && USER_PT8028_KEY
        if (manual_off) {
            bool sw_has_event;
            bool hw_has_event;

            /* 【非TCH5按键-松手检测】PE1已切为上升沿，唤醒=松手→恢复下降沿→重睡 */
            if (elunchbox_waiting_key_release) {
                wkpnd = port_wakeup_get_status();
                bool pb9_lo = ((GPIOB >> 9) & 1) == 0;
                u8 flag = pt8028_read_flag_raw();

                if (pb9_lo) {
                    /* PB9(门铃)在等松手期间唤醒 → 真唤醒亮屏 */
                    printf("lp: -> PB9 wake during wait-release\n");
                    gui_need_wkp = true;
                    elunchbox_pb9_wake_source = true;
                    break;
                }

                if (flag != 0) {
                    /* PE1 HIGH → 按键已松手 → 恢复下降沿唤醒，准备下一次按键 */
                    printf("lp: -> key released, restore falling edge\n");
                    port_wakeup_init(PT8028_GPIO_OUT_FLAG, 1, 1);
                    elunchbox_waiting_key_release = false;
                }
                /* flag==0: PE1仍LOW，杂波唤醒 → 忽略，继续等松手 */

                RTCCON9 = BIT(7) | BIT(5) | BIT(2);
                elunchbox_manual_wake_pending_take();
                continue;
            }

            /* Step 1: 软件轮询 PE1 FLAG 下降沿 */
            elunchbox_manual_off_sleep_poll();
            sw_has_event = elunchbox_manual_wake_pending_peek();

            /* Step 2: 硬件 port wakeup pending */
            wkpnd = port_wakeup_get_status();
            hw_has_event = (wkpnd != 0);

            if (sw_has_event || hw_has_event) {
                /* Step 3: 读 PB9 + PE1/BCD */
                bool pb9_lo = ((GPIOB >> 9) & 1) == 0;
                u8 bcd;
                bool pe1_lo = sleep_read_bcd_pe1(&bcd);
                printf("lp: EVENT sw=%d hw=%d pb9=%d pe1=%d bcd=%d\n",
                       sw_has_event, hw_has_event, pb9_lo, pe1_lo, bcd);

                if (pb9_lo) {
                    printf("lp: -> UART/PB9 wake\n");
                    gui_need_wkp = true;
                    elunchbox_pb9_wake_source = true;
                    break;
                }
                if (pe1_lo && bcd == 5) {
                    /* TCH5: 要求持续 LOW≥2s 才当有效唤醒 */
                    int tch5_cnt = 0;
                    while (tch5_cnt < 400) {  /* 400*5ms=2000ms=2s */
                        delay_5ms(1);
                        WDT_CLR();
                        if (pt8028_read_flag_raw() != 0) {
                            break;  /* PE1 回 HIGH → 未按够 2s */
                        }
                        tch5_cnt++;
                    }
                    if (tch5_cnt >= 400) {
                        printf("lp: -> TCH5 wake (held 2s)\n");
                        gui_need_wkp = true;
                        break;
                    }
                    printf("lp: -> TCH5 brief (<2s), back to sleep\n");
                } else if (pe1_lo) {
                    printf("lp: -> non-TCH5 (bcd=%d), back to sleep\n", bcd);
                } else {
                    printf("lp: -> stale/noise, ignore\n");
                }

                /* 无效唤醒: 清 pending → 切上升沿等松手 → continue 等 bt_sleep_proc 重睡。
                 *   非TCH5按下/轻触弹跳(stale/noise)/TCH5短按，统一切上升沿：
                 *   PE1 仍 LOW → 等松手上升沿 → 恢复下降沿 → 重睡
                 *   PE1 已 HIGH(轻触弹跳) → 无上升沿 → 直接睡；下次唤醒时恢复下降沿 */
                RTCCON9 = BIT(7) | BIT(5) | BIT(2);
                elunchbox_manual_wake_pending_take();
                port_wakeup_init(PT8028_GPIO_OUT_FLAG, 0, 1);
                elunchbox_waiting_key_release = true;
                printf("lp: -> sw to rising edge, wait release\n");
                continue;
            }
        }
#endif

        /* ---- 以下与原厂逻辑一致，manual_off/normal 共用 ---- */
        wkpnd = port_wakeup_get_status();
        wko_wkup_flag = port_wko_is_wakeup();
        port_int_sleep_process(&wkpnd);
        bsp_sensor_step_lowpwr_pro();

        /* Port wakeup (manual_off: 上面已处理过, 此处是新事件) */
        if (wkpnd) {
#if ELUNCHBOX_PANEL_EN
            if (manual_off) {
                bool pb9_lo = ((GPIOB >> 9) & 1) == 0;
                u8 bcd;
                bool pe1_lo = sleep_read_bcd_pe1(&bcd);
                if (pb9_lo) {
                    elunchbox_pb9_wake_source = true;
                    gui_need_wkp = true;
                    break;
                }
                if (pe1_lo && bcd == 5) {
                    /* TCH5: 2s hold check */
                    int tch5_cnt = 0;
                    while (tch5_cnt < 400) {
                        delay_5ms(1);
                        WDT_CLR();
                        if (pt8028_read_flag_raw() != 0) break;
                        tch5_cnt++;
                    }
                    if (tch5_cnt >= 400) {
                        gui_need_wkp = true;
                        break;
                    }
                }
                /* 无效唤醒: 清 pending → continue */
                printf("lp: port_wkp2 non-wake (bcd=%d pe1=%d pb9=%d)\n",
                       bcd, pe1_lo ? 1 : 0, pb9_lo ? 1 : 0);
                /* 同第一处：清 pending → 切上升沿等松手 → 重睡 */
                RTCCON9 = BIT(7) | BIT(5) | BIT(2);
                port_wakeup_init(PT8028_GPIO_OUT_FLAG, 0, 1);
                elunchbox_waiting_key_release = true;
                printf("lp: -> sw to rising edge, wait release\n");
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
    printf("lp: while exit loop=%u sleep_cnt=%u wkp=%u\n",
           lp_loop_cnt, lp_sleep_cnt, gui_need_wkp ? 1u : 0u);
    /* 真唤醒 → 重置松手等待标志，确保下次进 manual_off 休眠使用下降沿唤醒 */
    if (gui_need_wkp) {
        elunchbox_waiting_key_release = false;
    }
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

/* 【休眠】同一轮 manual_off 会话只发一次 PowerSwitch=OFF。
 * sfunc_sleep 可能被反复调用(睡→醒→睡)，每次都发会触发加热模块 ACK，
 * ACK 落在 PB9 drain 之后 → 唤醒 → 死循环 → 500μA+。
 * 重置由 elunchbox_pwroff_sent_reset() 负责 — 仅在屏幕真实亮起时调。 */
static bool s_pwroff_sent = false;

void elunchbox_pwroff_sent_reset(void)
{
    s_pwroff_sent = false;
    /* manual_off 休眠后 TX 保持 block, 确认真唤醒时在此恢复.
     * 调用路径: elunchbox_screen_wake() / TCH5 3s 长按 都经过此处. */
    lb_uart_tx_block(false);
}

/* 【休眠主函数】sfunc_sleep — 熄屏 + 关外设 + sfunc_sleep_proc 深度休眠
 *   自动息屏: guioff_slp=1, manual_off_slp=0 → BLE 降参数, 保留部分唤醒源
 *   手动关机: guioff_slp=1, manual_off_slp=1 → 关 BLE 广播, 关 RTC WDT, 仅保留 UART RX+按键唤醒
 *   普通休眠: guioff_slp=0 → gui_sleep 熄屏后进入深度休眠
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
    elunchbox_pb9_wake_source = false;
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    bool elunchbox_guioff_slp = elunchbox_pwr_gui_off_is_on();  /* auto guioff or manual off */
    /* 【手动关机标志】guioff_slp && manual_off → 进入手动关机低功耗深度休眠路径 */
    bool elunchbox_manual_off_slp = elunchbox_guioff_slp && elunchbox_pwr_is_manual_off();
    printf("elunchbox: sfunc_sleep manual_off_slp=%u\n",
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
    /* 【休眠】发送 PowerSwitch=OFF 给加热模块，通知进入低功耗。
     * 同一轮 manual_off 会话只发一次：反复睡→醒→睡时若每次都发，
     * 加热模块回 ACK → PB9 LOW → drain 后唤醒 → 死循环 → 500μA+。
     * 真唤醒(gui_need_wkp=true)后重置标记，下次进 manual_off 再发。 */
    if (elunchbox_guioff_slp && !s_pwroff_sent) {
        /* 两步握手: ①发停加热→等ACK ②发关开关→等ACK
         * 确保加热模块完全处理完才进休眠, 避免残留应答导致睡/醒风暴. */
        int timeout;

        /* Step ①: 停加热 */
        {
            u8 data[8];
            u16 len = lb_dp_encode_bool(data, LB_DPID_HEAT_ENABLE, 0);
            lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, len, false);  /* false=等ACK */
            printf("elunchbox: sfunc_sleep step1 HeatEnable=0 (wait ACK)\n");
            timeout = 0;
            while (lb_send_waiting && timeout < 200) {
                lunchbox_uart_process();
                delay_5ms(5);
                timeout++;
            }
            if (lb_send_waiting) {
                printf("elunchbox: HeatEnable=0 ACK timeout, force clear\n");
                lb_send_waiting = false;
            }
        }

        /* Step ②: 关总开关 */
        {
            u8 data[8];
            u16 len = lb_dp_encode_bool(data, LB_DPID_POWER_SWITCH, 0);
            lb_uart_send_raw(LB_UART_CMD_DYNAMIC, data, len, false);  /* false=等ACK */
            printf("elunchbox: sfunc_sleep step2 PowerSwitch=OFF (wait ACK)\n");
            timeout = 0;
            while (lb_send_waiting && timeout < 200) {
                lunchbox_uart_process();
                delay_5ms(5);
                timeout++;
            }
            if (lb_send_waiting) {
                printf("elunchbox: PowerSwitch=OFF ACK timeout, force clear\n");
                lb_send_waiting = false;
            }
        }

        s_pwroff_sent = true;
        printf("elunchbox: sfunc_sleep handshake done, entering sleep\n");
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
    bt_enter_sleep();
    bt_audio_bypass();
    while(btstack_audio_is_busy());
#if LE_EN
    adv_interval = ble_get_adv_interval();
    /* manual_off: 不能关广播！ble_adv_dis() 会让 BT 栈进入等完成状态
     * → bt_sleep_proc() 永远返回 0 → sys_enter_sleep 永远不会被调。
     * 跟工厂一样只拉长间隔，BT 栈就能正常 sleep。BLE 唤醒已被忽略。 */
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
    /* manual_off: 强制关 BT scan.
     * 第一次上电时 scan 未开启所以 bt_scan_disable 生效 → bt_sleep_proc 可睡。
     * 唤醒亮屏后 scan 被 bt_update_bt_scan_param_default 恢复 → 第二次进
     * manual_off 时 bt_get_scan()=true → 上面只调参不关 scan → bt_sleep_proc
     * 不睡 → 10mA。此处强制关掉。 */
    if (elunchbox_manual_off_slp) {
        bt_scan_disable();
    }
#endif

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

    usbcon0 = USBCON0;                          //需要先关中断再保存
    usbcon1 = USBCON1;
    USBCON0 = BIT(5);
    USBCON1 = 0;
#if SD_SUPPORT_EN
    SD0_LDO_DIS();
#endif

    /* 【熄屏】非饭盒路径：休眠前先关屏；饭盒路径已在 sleep_process 中提前熄屏，此处跳过 */
    if (!elunchbox_guioff_slp) {
        gui_sleep(true);
    }

#if MODEM_CAT1_EN
    bsp_modem_sleep_enter();
#endif

#if USER_KEY_QDEC_EN || USER_ADKEY_QDEC_EN
    bsp_qdec_exit();                            //旋转编码器exit
#endif

    sysclk = sys_clk_get();
    sys_clk_set(SYS_24M);
    DACDIGCON0 &= ~BIT(0);                      //disable digital dac
    adda_clk_source_sel(1);                 //adda_clk48_a select xosc52m
    PLL0CON0 &= ~(BIT(18) | BIT(6));            //pll0 sdm & analog disable
    PLL1CON0 &= ~0x03;                          //disable pll1
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_manual_off_slp) {
        RTC_WDT_DIS();
    }
#endif
    rtc_sleep_enter();

    //io analog input
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
            printf("elunchbox: sfunc_sleep manual_off GPIOE PE1~4 digital, GPIOB PB8+PB9 only\n");
            /* PE1(FLAG) + PE2~4(D0~D2) 全数字输入。
             * PE2~4 保留 200K 弱上拉 —— 匹配 PT8028 空闲态 BCD=111,
             * 防止 PT8028 扫描间隙 tri-state 时引脚浮空漏电。
             * PT8028 推挽输出, idle 全高 → 上拉无 DC 冲突。
             * 按下时个别线拉低 → 3.3V/200K=16.5μA/pin, 瞬态可接受。 */
            GPIOEDE = BIT(1) | BIT(2) | BIT(3) | BIT(4);
            GPIOBDE = BIT(9) | BIT(8);              /* PB9 门铃唤醒 + PB8 TX 防浮空; PB3 analog */
            /* PB8+PB9: 数字输入+上拉, 防浮空漏电.
             * PB9 下降沿唤醒(门铃). PB8(UART TX) 加热模块断电后浮空, 上拉固定高电平. */
            GPIOBDIR = 0;
            GPIOBPU  = BIT(9) | BIT(8);
            GPIOBPD  = 0;
            GPIOBPU200K = 0;
            GPIOBPD200K = 0;
            GPIOBPU300  = 0;
            GPIOBPD300  = 0;
            /* PE2~4: 200K 弱上拉, 清强上下拉. */
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
            GPIOBPU200K = 0;
            GPIOBPD200K = 0;
            GPIOBPU300  = 0;
            GPIOBPD300  = 0;
        } else {
            GPIOBDE = BIT(3) | BIT(8) | BIT(9); /* PB3 日志 / PB8 PB9 UART1 */
        }
        /* manual_off: PE1~4 digital (PE2~4 200K 上拉防浮空); PE0 analog 省 3-5μA
         * auto guioff: PE0+PE1 digital — UART/I2C 交互需要 */
        GPIOEDE = elunchbox_manual_off_slp ? (BIT(1) | BIT(2) | BIT(3) | BIT(4)) : (BIT(0) | BIT(1));
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
//        printf("hr: %d, step:%d\n", sensor_type_hr, sensor_type_step);
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
        /* 【UART 应答排空】两步握手已等 ACK, 此处排空残留.
         * Poll PB9 until HIGH (UART idle), timeout ~400ms. */
        delay_5ms(6);

        {
            int drain = 0;
            while (((GPIOB >> 9) & 1) == 0 && drain < 4000) {
                delay_us(100);
                drain++;
            }
        }

        /* 【UART 排空】握手后加热模块可能发状态更新, 处理掉并 ACK,
         * 确保对方收到回应 → 安心进休眠, 不再重试 → PB9 不会误唤醒. */
        {
            int drain = 0;
            while (drain < 20) {
                lunchbox_uart_process();
                delay_5ms(1);
                drain++;
            }
        }

        if (elunchbox_manual_off_slp) {
            /* 手动关机: block TX → 关 UART 硬件(释放 PB9 从 UART RX 回到 GPIO).
             * PB9 做"门铃": 只检测边沿唤醒, 不收数据, 第一包丢了.
             * 对方发数据, 起始位下降沿(HIGH→LOW)触发 port wakeup → 唤醒芯片.
             * 唤醒后 UART resume, TX 保持 block, 主循环确认真开机后开 TX. */
            lb_uart_tx_block(true);
            lunchbox_uart_suspend();   /* 先关 UART + 释放 FUNCMCON0, PB9 回到 GPIO */
            delay_5ms(10);
            printf("elunchbox: sfunc_sleep manual_off UART suspended\n");
        }
    }
#endif

    sleep_wakeup_config();
    RTCCON9 = BIT(2);   /* clr spurious port pending from wakeup config */

#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_guioff_slp) {
        /* manual_off: 先关 BT wakeup，再配 port wakeup 仅 PE1+PB9.
         * 必须在 sleep_wakeup_config() 之后 —— SDK 内部会覆盖 port wakeup 配置.
         * BT wakeup 也在 sys_sleep_cb() 内最后一刻再清(防 BT 栈内部重开). */
        if (elunchbox_manual_off_slp) {
            RTCCON3 |= BIT(17);         /* re-enable port wakeup (上个循环 line932 关了) */
            RTCCON3 &= ~BIT(13);        /* disable bt wakeup */
            BTCON2 &= ~(3 << 10);       /* disable bt sleep wakeup */
            RTCCON  &= ~(0xf << 7);     /* disable rtc sleep wakeup */
        }
        port_wakeup_init(PT8028_GPIO_OUT_FLAG, 1, 1);
        port_wakeup_all_init(IO_PB9, 1, 1);
        printf("elunchbox: sfunc_sleep %s wakeup PE1+PB9 configured\n",
               elunchbox_manual_off_slp ? "manual_off" : "auto_guioff");
    }
#endif

    /* 【低功耗优化】manual_off: 切 BUCK→LDO 模式.
     * 此时所有外设已关(GPIO模拟/PLL/USB/DAC/SD0)、BT参数已降、wakeup已配，
     * BT栈即将通过 bt_sleep_proc→sys_enter_sleep 进入硬件深度休眠。
     * 在此时切 LDO：BUCK DCDC 在 μA 级休眠电流下效率 <50%，LDO 省 50-200μA.
     * 必须在 sleep_wakeup_config 之后——切换过程如有 GPIO 扰动，wakeup 配置不受影响.
     * 唤醒后由 adpll_init + set_buck_mode(1) 恢复 BUCK 模式. */
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
    dac_aubuf_init();
    /* 【低功耗优化】恢复 BUCK 模式.
     * set_buck_mode(1) 依赖 ADPLL 为 PMU 模拟部分提供参考时钟,
     * 故必须在 adpll_init() 之后. */
#if LPWR_BUCK_TO_LDO_EN && ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (buck_saved) {
        set_buck_mode(1);
    }
#endif
    sys_clk_set(sysclk);
    /* manual_off: 恢复 UART1 硬件 (sfunc_sleep 中 suspend 的) */
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    lunchbox_uart_resume();
    if (!elunchbox_manual_off_slp) {
        lb_uart_tx_block(false);    /* auto guioff: 恢复 TX */
    }
    /* manual_off: TX 保持 block, 主循环确认真开机(on TCH5/充电/加热事件)后再开 */
#endif
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

    /* 【ELUNCHBOX FIX】elunchbox_guioff_sleep_post_wake() 内 screen_wake 依赖
     * CLKGAT0 已恢复，否则 pt8028_port_gpio_init / key_scan 访问未钟控外设 →
     * CLKGAT0=0 → halt:8001 蓝屏。
     * 参考 lunchbox_display_off 先保存 CLKGAT0 再关屏，此处先恢复 CLKGAT0
     * 再做 gui_wakeup，确保所有外设时钟就位。 */
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_guioff_slp && elunchbox_pwr_is_manual_off()) {
        CLKGAT0 = elunchbox_saved_clkgat0;
    }
#endif

    if (gui_need_wkp) {
        /* 不在这里重置 s_pwroff_sent — 短暂按键唤醒(非真亮屏)不应触发
         * PowerSwitch=OFF 重发。改为屏幕实际亮起时由 elunchbox_pwroff_sent_reset() 重置。 */
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
        if (elunchbox_guioff_slp) {
            elunchbox_guioff_sleep_post_wake(true);
            /* PB9(UART/门铃)唤醒 → 直接亮屏，不走 3s 长按 */
            if (elunchbox_pb9_wake_source && elunchbox_manual_off_slp) {
                printf("elunchbox: PB9 wake → screen on\n");
                elunchbox_pwr_gui_wake_reason("PB9");
                elunchbox_pwroff_sent_reset();
            }
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

    /* 【ELUNCHBOX】manual_off key wake: restore GPU now in sfunc_sleep() context.
     * At this point the system is still single-threaded — no ISR/timer races.
     * lunchbox_display_on() deferred to main loop after 3s hold verification.
     * Calling gui_wakeup() from the main loop (via elunchbox_screen_wake) causes
     * a timer ISR crash at 0x1001201e (halt:8001) due to post-sleep race.
     *
     * NOTE: gui_wakeup() checks elunchbox_pwr_manual_off_gui_wake_ok() which
     * requires elunchbox_pwr_intentional_wake=true when manual_off is set.
     * Set it temporarily so the GPU restore actually runs; clear it after. */
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_manual_off_slp && gui_need_wkp && sys_cb.gui_sleep_sta) {
        elunchbox_pwr_intentional_wake_set(true);
        gui_wakeup();
        elunchbox_pwr_intentional_wake_set(false);
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
//    printf("%s->%d,%d\n", __func__, sys_cb.gui_need_wakeup, sys_cb.gui_sleep_sta);
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    /* gui_sleep_sta: classic path sets it; elunchbox_pwr_gui_off: auto guioff path sets it.
     * Either one means screen is off → eligible for shallow/deep sleep. */
    if (elunchbox_pwr_gui_off_is_on() || sys_cb.gui_sleep_sta) {
        sys_cb.gui_need_wakeup = 0;

        /* Auto guioff: heating in progress → wake screen instead of sleeping */
        if (!elunchbox_pwr_is_manual_off() && elunchbox_heating_blocks_idle()) {
            printf("elunchbox: sleep_process heating auto wake (not manual_off)\n");
            elunchbox_pwr_gui_wake_reason("sleep_process heating");
            reset_sleep_delay_all();
            return false;
        }

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        /* Auto guioff: hold TCH5 → stay awake, let main loop accumulate 3s for manual_off */
        if (!elunchbox_pwr_is_manual_off()) {
            if (pt8028_is_power_key_held() || pt8028_boot_tch5_down()) {
                reset_sleep_delay();
                reset_pwroff_delay();
                return false;
            }
        }
#endif

        /* Manual off: force enter deep sleep immediately (bypass sleep_ready).
         * But stay awake if TCH5 fresh press needs main-loop tick for long-press timer:
         * tick_get() doesn't advance during deep sleep, so 2s long-press timer would stall. */
        bool force_lowpwr = false;
        if (elunchbox_pwr_is_manual_off()) {
            if (elunchbox_pwr_manual_off_should_stay_awake()) {
                reset_sleep_delay();
                reset_pwroff_delay();
                return false;
            }
            force_lowpwr = true;
        }

        /* Enter shallow sleep: auto guioff needs sleep_ready + bt_is_allow_sleep;
         * manual_off forces entry regardless (BT stack may refuse but we go anyway). */
        if ((elunchbox_guioff_sleep_ready() && (*is_sleep)()) || force_lowpwr) {
            if (force_lowpwr) {
                printf("elunchbox: sleep_process force_lowpwr -> sfunc_sleep\n");
                /* 手动关机深度休眠前须退出 GPU。
                 * 正常路径 sfunc_sleep 内 gui_sleep(true) → keep_ram → gpu_exit，
                 * 但 elunchbox guioff_slp=true 跳过了此步骤。
                 * lunchbox_display_off() 只关物理屏(背光+VDDLCD)，GPU 仍在跑 →
                 * 带电进休眠白白耗电。在此补齐 gui_sleep(true)。 */
                if (!sys_cb.gui_sleep_sta) {
                    gui_sleep(true);
                }
            }
            sfunc_sleep();
            /* 不再调 ble_adv_dis() — 异步等完成会阻塞下次 bt_sleep_proc()。
             * 广播间隔已拉长到 1600，功耗可忽略。 */
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
    /* 饭盒：独立 idle 计时到 0 即息屏（不依赖 guioff_delay / sleep_en） */
    if (elunchbox_guioff_idle_expired()) {
        elunchbox_pwr_gui_off_activate();
        return false;
    }
    /* 已息屏但加热仍进行：自动亮回，不进浅睡 */
    if (elunchbox_pwr_gui_off_is_on() && sys_cb.gui_sleep_sta && elunchbox_heating_blocks_idle()) {
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
    RTCCON1 &= ~(BIT(5) | BIT(7));		        //BIT(7): VRTC voltage for ADC, BIT(5):WK pin analog enable, output WKO voltage for ADC
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
    GPIOEDE |= (BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4)); //PE0+OUT_FLAG+D0/D1/D2, 冷启动即读键值
    pt8028_port_pwrdown_wake_prep();            //恢复 OUT_FLAG 上拉与数字使能
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

/* 【最终关机入口】func_pwroff: 断开蓝牙 → 熄屏 → sfunc_pwrdown 硬关机 / sfunc_lowbat 低电休眠
 *   调用路径：系统关机 / 低电关机 / 手动长按关机（最终阶段）
 */
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
        pt8028_wait_out_flag_release();
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
