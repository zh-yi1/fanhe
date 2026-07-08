#include "include.h"
#include "func.h"
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

AT(.sleep_backup.gui)
u8 sys_backup_buf[32 * 1024];

extern u8 *cache_backup;
extern u32 __dynamic_pool_start, __dynamic_pool_end;

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


AT(.sleep_text.sleep)
void sys_sleep_cb(u8 lpclk_type)
{
    //注意！！！！！！！！！！！！！！！！！
    //此函数只能调用sleep_text或com_text函数

    //此处关掉影响功耗的模块
    u32 gpiogde = GPIOGDE;
    if (gpiogde & BIT(6)) {
        GPIOGDE = BIT(2) | BIT(4) | BIT(6);         //SPICS, SPICLK
    } else {
        GPIOGDE = BIT(2) | BIT(4);                  //SPICS, SPICLK
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
    if (elunchbox_guioff_in_sleep_mode() && !elunchbox_pwr_is_manual_off()) {
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

//AT(.sleep_text.rodata.osc)
//const char lp_osc_str[] = "sleep_proc_delay: %d\n";

//此函数运行代码及调用子函数需要全部放到retention ram
AT(.sleep_text.sleep.proc)
bool sfunc_sleep_proc(void)
{
    uint32_t status, ret;
    u32 wkpnd;
    bool wko_wkup_flag;
    bool gui_need_wkp = false;
    u32 vddio_vddcore_level = PWRCON0 & 0x1ff;   // 包含vddio levle
    u8 vddio_level = vddio_sleep_level;
#if ELUNCHBOX_PANEL_EN
    bool manual_off = elunchbox_pwr_is_manual_off();
#endif

//    return gui_need_wkp;        //test
    cache_backup = sys_backup_buf;//(u8 *)ab_malloc(32 * 1024);  //用于sleep过程中备份cache ram
    if (cache_backup == NULL) {
        printf("==>sleep malloc memory failed!\n");
		return gui_need_wkp;
    }



#if ELUNCHBOX_PANEL_EN
    if (manual_off) {
        PWRCON0 = (PWRCON0 & ~0x1f) | 8;       //8: 0.9v (手动关机极限省电，保留retention)
    } else
#endif
    {
        PWRCON0 = (PWRCON0 & ~0x1f) | 12;      //12: 1v, 每档0.025v
    }
    if (vddio_level) {
        PWRCON0 = (PWRCON0 & ~(BIT(5)*0xF)) | BIT(5) * vddio_level;   // VDDIO电压，step=0.1V 0:2.4V
    }

    sys_cb.sleep_counter = 0;

    sys_cb.sleep_wakeup_time = -1L;
    while(bt_is_sleep()) {
        WDT_CLR();
#if ELUNCHBOX_PANEL_EN
        if (!manual_off)
#endif
            bt_thread_check_trigger();
        status = bt_sleep_proc();

#if ELUNCHBOX_PANEL_EN
        if (manual_off) {
            elunchbox_manual_off_sleep_poll();
            if (elunchbox_manual_wake_pending_peek()) {
                gui_need_wkp = true;
                break;
            }
        }
#endif

//        printf(lp_osc_str, get_sleep_proc_delay());
#if SENSOR_HUB_EN
        bsp_senshb_lp_process();
#endif
        if (status == 1) {
#if ELUNCHBOX_PANEL_EN
            if (manual_off) {
                /* 手动关机: 跳过 sleep_timer (省ADC/charge_detect), 仅低电退出 */
                if (++sys_cb.sleep_counter >= 60) {
                    sys_cb.sleep_counter = 0;
                    ret = sleep_timer();
                    if (ret == 2) break; /* 低电退出 */
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
        wkpnd = port_wakeup_get_status();
        wko_wkup_flag = port_wko_is_wakeup();

#if ELUNCHBOX_PANEL_EN
        if (!manual_off)
#endif
        {
            port_int_sleep_process(&wkpnd);
        }
#if ELUNCHBOX_PANEL_EN
        if (!manual_off)
#endif
        bsp_sensor_step_lowpwr_pro();

        if (wkpnd) {
#if ELUNCHBOX_PANEL_EN
            if (manual_off) {
                /* 手动关机：按键/PB9-UART唤醒 MCU 供主循环处理，但不亮屏 */
                break;
            }
#endif
            printf(port_wakeup_str, wkpnd);
            gui_need_wkp = true;
            break;
        }
#if ELUNCHBOX_PANEL_EN
        if (!manual_off)
#endif
        if ((RTCCON9 & BIT(2)) || (RTCCON10 & BIT(2)) || wko_wkup_flag) {
            printf(wko_wakeup_str);
#if ELUNCHBOX_PANEL_EN
            if (manual_off) {
                break;
            }
#endif
            gui_need_wkp = true;
            break;
        }
#if LE_EN
        if (ble_app_need_wakeup()) {
#if ELUNCHBOX_PANEL_EN
            if (manual_off) {
                /* 手动关机：BLE 事件不唤醒，继续等待 TCH5 长按 */
            } else
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
            if (manual_off) {
                /* 手动关机：co_timer 不唤醒，继续等待 TCH5 长按 */
            } else
#endif
            {
                printf(co_timer_wakeup_str);
                break;
            }
        }

		if (bt_cb.call_type) {
#if ELUNCHBOX_PANEL_EN
            if (manual_off) {
                /* 手动关机：来电不唤醒，继续等待 TCH5 长按 */
            } else
#endif
            {
                printf(call_wakeup_str);
                gui_need_wkp = true;
                break;
            }
		}
    }
    PWRCON0 = (PWRCON0 & ~0x1ff) | vddio_vddcore_level;
//    ab_free(cache_backup);

    return gui_need_wkp;
}

static void sfunc_sleep(void)
{
    uint32_t usbcon0, usbcon1;
    u16 pa_de, pb_de, pe_de, pf_de, pg_de;
    u16 adc_ch;
    uint32_t sysclk;
    u32 wkie;
    u32 clkgat0_bak = 0;
    bool gui_need_wkp = false;
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    bool elunchbox_guioff_slp = elunchbox_pwr_gui_off_is_on() && sys_cb.gui_sleep_sta;
    bool elunchbox_manual_off_slp = elunchbox_guioff_slp && elunchbox_pwr_is_manual_off();
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
    bt_enter_sleep();
    bt_audio_bypass();
    while(btstack_audio_is_busy());
#if LE_EN
    adv_interval = ble_get_adv_interval();
    if (elunchbox_manual_off_slp) {
        ble_adv_dis();
    } else {
        ble_set_adv_interval(1600);                  //interval: 500 * 0.625ms = 500ms
        if (ble_is_connect()) {                     //ble已连接
            interval = ble_get_conn_interval();
            latency = ble_get_conn_latency();
            tout = ble_get_conn_timeout();
            ble_update_conn_param(410, 0, 500);     //interval: 410*1.25ms = 512.5ms
        }
    }
#endif
#if ELUNCHBOX_PANEL_EN
    if (elunchbox_manual_off_slp && !bt_is_connected()) {
        bt_scan_disable();
    } else
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
    adda_clk_source_sel(1);                     //adda_clk48_a select xosc52m
    PLL0CON0 &= ~(BIT(18) | BIT(6));            //pll0 sdm & analog disable
    PLL1CON0 &= ~0x03;                          //disable pll1
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_manual_off_slp) {
        RTC_WDT_DIS();                          // 手动关机: 关闭RTC看门狗, 防止长休眠期间复位
    }
#endif
    rtc_sleep_enter();

    //io analog input
    pa_de = GPIOADE;
    pb_de = GPIOBDE;
    pe_de = GPIOEDE;
    pf_de = GPIOFDE;
    pg_de = GPIOGDE;
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
            GPIOBDE = BIT(3) | BIT(9);          /* PB3 日志 + PB9 UART1 RX，TX(PB8)掉电 */
        } else {
            GPIOBDE = BIT(3) | BIT(8) | BIT(9); /* PB3 日志 / PB8 PB9 UART1 */
        }
        GPIOEDE = (BIT(0) | BIT(1)); //PT8028 PE0+OUT_FLAG only, D0/D1/D2 off to prevent spurious wakeup
        GPIOFDE = 0;
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

#if ELUNCHBOX_PANEL_EN
    if (elunchbox_manual_off_slp) {
        /* 手动关机: 关闭非必要外设时钟，保留 UART1 供充电数据 RX */
        clkgat0_bak = CLKGAT0;
        CLKGAT0 &= ~(BIT(CLKGAT0_UART0_CLK_EN)  |
                     BIT(CLKGAT0_HSUT0_CLK_EN)  |
                     BIT(CLKGAT0_IIS_CLK_EN)    |
                     BIT(CLKGAT0_SPI0_CLK_EN)   |
                     BIT(CLKGAT0_SPI1_CLK_EN)   |
                     BIT(CLKGAT0_SPI2_CLK_EN)   |
                     BIT(CLKGAT0_TMR0_CLK_EN)   |
                     BIT(CLKGAT0_TMR1_CLK_EN)   |
                     BIT(CLKGAT0_TMR2_CLK_EN));
        /* UART1_CLK_EN 保留: 手动关机需 RX 接收加热模块充电数据 */
        /* 配置 PB9 (UART1 RX) 下降沿唤醒：加热模块发数据时唤醒 CPU 处理 */
        port_wakeup_init(IO_PB9, 1, 1);
    }
#endif

    sleep_wakeup_config();

    gui_need_wkp = sfunc_sleep_proc();          //进入休眠

#if ELUNCHBOX_PANEL_EN
    if (elunchbox_manual_off_slp && clkgat0_bak) {
        CLKGAT0 = clkgat0_bak;                  // 恢复手动关机前的外设时钟
    }
#endif

    RTCCON9 = BIT(7) | BIT(5) | BIT(2);         //clr port, bt, wko wakeup pending
    RTCCON3 &= ~(BIT(17) | BIT(13));            //disable port, bt wakeup
    sleep_wakeup_exit();
    if (cfg_bt_sniff_clk_sel != 5) {
        lp_xosc_en();
    }
    WKUPCON |= wkie;                            //还原WKIE

    GPIOADE = pa_de;
    GPIOBDE = pb_de;
    GPIOEDE = pe_de;
    GPIOFDE = pf_de;
    GPIOGDE = pg_de;
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

    if (gui_need_wkp) {
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
        if (elunchbox_guioff_slp) {
            elunchbox_guioff_sleep_post_wake(true);
        } else
#endif
        {
            printf("gui_wakeup\n");
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

#if LE_EN
    if (elunchbox_manual_off_slp && !gui_need_wkp) {
        ble_adv_dis();
    } else {
        ble_set_adv_interval(adv_interval);
        if (interval | latency | tout) {
            ble_update_conn_param(interval, latency, tout); //还原连接参数
        }
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
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (!elunchbox_manual_off_slp)
#endif
        printf("sleep_exit\n");
}

bool sleep_process(is_sleep_func is_sleep)
{
//    printf("%s->%d,%d\n", __func__, sys_cb.gui_need_wakeup, sys_cb.gui_sleep_sta);
#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_GUIOFF_SLEEP_EN
    if (elunchbox_pwr_gui_off_is_on() && sys_cb.gui_sleep_sta) {
        sys_cb.gui_need_wakeup = 0;
#if ELUNCHBOX_PANEL_EN
        /* 手动长按关机：直接进入深度休眠，sfunc_sleep() 内含 manual_off 低功耗路径 */
        if (elunchbox_pwr_is_manual_off()) {
            sfunc_sleep();
            reset_sleep_delay_all();
            reset_pwroff_delay();
            return false;
        }
        if (elunchbox_heating_blocks_idle()) {
            printf("elunchbox: sleep_process heating auto wake (not manual_off)\n");
            elunchbox_pwr_gui_wake_reason("sleep_process heating");
            reset_sleep_delay_all();
            return false;
        }
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        /* 自动息屏浅睡：按住 TCH5 时不进浅睡，让主循环累计 3 秒长按唤醒 */
        if (pt8028_is_power_key_held() || pt8028_boot_tch5_down()) {
            reset_sleep_delay();
            reset_pwroff_delay();
            return false;
        }
#endif
        if (elunchbox_guioff_sleep_ready() && (*is_sleep)()) {
            sfunc_sleep();
#if LE_EN
            /* 确保在手动关机浅睡的每次唤醒后都关闭广播，避免BT栈在sleep_exit时重新打开导致功耗升高 */
            ble_adv_dis();
#endif
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
    GPIOBDE = 0;
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

//硬开关方案，低电时，进入省电状态
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

//软开关方案，POWER键/低电时，进入关机状态
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
    GPIOEDE |= (BIT(0) | BIT(1)); //PE0+OUT_FLAG only, D0/D1/D2 off to prevent spurious wakeup
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
