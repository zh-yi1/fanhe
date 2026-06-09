#include "include.h"
#include "func.h"

void rtc_alarm_disable(void);

AT(.com_rodata.key.table)
const key_shake_tbl_t key_shake_table = {
    .scan_cnt = KEY_SCAN_TIMES,
    .up_cnt   = KEY_UP_TIMES,
    .long_cnt = KEY_LONG_TIMES,
    .hold_cnt = KEY_LONG_HOLD_TIMES,
};

#if USER_ADKEY
AT(.com_text.port.key)
static u8 get_adkey(u8 key_val)
{
    u8 num = 0;

    while (key_val > adkey_table[num].adc_val) {
        num++;
    }

    return adkey_table[num].usage_id;
}
#endif // USER_ADKEY

#if USER_PWRKEY
AT(.com_text.port.key)
static u8 get_pwrkey(void)
{
    u8 num = 0;
    u32 wko_val = saradc_get_value10(ADCCH_WKO);
    u16 key_val = ((u32)wko_val << 8) / adc_cb.vrtc_val;

    if (key_val > 0xff) {
        key_val = 0xff;
    }
    while ((u8)key_val > pwrkey_table[num].adc_val) {
        num++;
    }

    return pwrkey_table[num].usage_id;
}
#endif // USER_PWRKEY

u8 bsp_pwrkey_get_usage_id(void)
{
#if USER_PWRKEY
    return get_pwrkey();
#else
    return NO_KEY;
#endif
}

bool power_off_check(void)
{
#if CHARGE_EN
    u16 charge_cnt = 0;
#endif

#if USER_PWRKEY
    u32 pwron_press_nms;
    int pwrkey_pressed_flag, ticks = 0, up_cnt = 0;
    u8 restart_chk_en = 1;
    pwrkey_pressed_flag = 0;
    pwron_press_nms = PWRON_PRESS_TIME;
    if (pwron_press_nms == 0) {
        pwron_press_nms = 100;
    }
#endif // USER_PWRKEY

    //要等PWRKEY开关释放后再次按下才能重新开机, 否则充电过程中5分钟关机, 低电关机等异常
    if ((PWRKEY_2_HW_PWRON) && (sys_cb.poweron_flag)) {
#if USER_PWRKEY
        restart_chk_en = 0;
#endif // USER_PWRKEY
        sys_cb.poweron_flag = 0;
    }

    while (1) {
        WDT_CLR();
        delay_ms(5);
#if USER_PWRKEY
        if (get_pwrkey() == key_cb.pwr_usage_id) {
            up_cnt = 0;
            if (restart_chk_en) {
                if (!pwrkey_pressed_flag) {
                    ticks = tick_get();
                    pwrkey_pressed_flag = 1;
                    sys_cb.ms_ticks = tick_get();                                       //记录PWRKEY按键按下的时刻
                }
                if (!sys_cb.poweron_flag) {
                    if (tick_check_expire(ticks, pwron_press_nms)) {                    //长按开机时间配置
                        sys_cb.poweron_flag = 1;
                        sys_cb.pwrdwn_hw_flag = 0;                                      //清PWRKEY硬开关的关机标志
                    }
                }
            }
        } else {
            if (up_cnt < 3) {
                up_cnt++;
            }
            if (up_cnt == 3) {
                up_cnt = 10;
                sys_cb.poweron_flag = 0;
                pwrkey_pressed_flag = 0;
                restart_chk_en = 1;
            }
        }
#else
        sys_cb.poweron_flag = 1;
        sys_cb.pwrdwn_hw_flag = 0;                                      //清PWRKEY硬开关的关机标志
#endif // USER_PWRKEY

#if CHARGE_EN
        if (xcfg_cb.charge_en) {
			charge_cnt++;
			if (charge_cnt > 20) {
                charge_cnt = 0;
                charge_detect(0);
            }
        }
#endif // CHARGE_EN

        if (sys_cb.poweron_flag) {
            if ((CHARGE_DC_NOT_PWRON) && CHARGE_DC_IN()) {
                continue;
            }
            //长按PP/POWER开机
            dac_restart();
            bsp_change_volume(sys_cb.vol);
#if WARNING_POWER_ON
            mp3_res_play(RES_BUF_EN_POWERON_MP3, RES_LEN_EN_POWERON_MP3);
#endif // WARNING_POWER_ON
            func_cb.sta = DEFAULE_START_FUNC;
            return true;
        } else {
            if (CHARGE_DC_IN()) {
                continue;
            } else {
                return false;
            }
        }
    }
}

AT(.text.bsp.power)
bool is_powron_frist_enable(u32 rtccon9)
{
    if (!PWRON_FRIST_BAT_EN) {
        return false;
    }
    if (rtccon9 & BIT(2)) {                                             //WKO wakeup不能直接开机
        return false;
    }
    if (CHARGE_DC_IN() && (CHARGE_DC_NOT_PWRON)) {                      //VUSB充电禁止开机
        return false;
    }
    return true;
}

#if !LP_XOSC_CLOCK_EN
AT(.text.bsp.power)
void sniff_rc_init(void)
{
    CLKGAT0 |= BIT(18);                             //freqdet_clken
    CLKCON1 = (CLKCON1 & ~(3 << 12)) | (2 << 12);   //freq_clkin select xosc_clk
    CLKCON1 = (CLKCON1 & ~(3 << 14)) | (0 << 14);   //freqdet_clk select rtc_rc2m
    FREQDETCON = (2000-1) << 16;                    //timer pr

    RTCCON0 |= BIT(2);                              //rtc_rc2m enable
    RTCCON0 &= ~BIT(22);                            //rtc_rc2m select sniff_rc_rtc
    RTCCON0 = (RTCCON0 & ~(7 << 17)) | (0 << 17);   //RI_BIAS = 0
    RTCCON0 |= BIT(20);                             //RI_EN_SNIFF = 1
}

AT(.text.bsp.power)
void rtc_power_on_calibration(void)
{
    rtc_alarm_disable();
    if (cm_read8(PARAM_RTC_CAL_VALID) == 1) {
        sniff_rc_init();
        rtc_calibration_read(PARAM_RTC_CAL_ADDR);
        rtc_clock_calc_by_rc(1);
        rtc_printf();
    }
}
#endif

AT(.text.bsp.power)
void power_on_check(void)
{
#if CHARGE_EN
    u16 charge_cnt = 0;
#endif
    int pwrkey_pressed_flag = 0;
    u32 pwron_press_nms;
    u32 rtccon9 = RTCCON9;                                              //wakeup pending
    u32 rtccon10 = RTCCON10;

    RTCCON9 = 0xfff;                                                    //Clr pending
    RTCCON10 = 0xfff;                                                   //Clr pending
    RTCCON1 &= ~BIT(6);                                                 //wko pin low level wakeup

    if ((LVDCON & BIT(19)) && (rtccon10 & BIT(10)) && (sys_cb.vbat >LPWR_OFF_VBAT)) {
        if (IS_PWRKEY_PRESS()) {
            sys_cb.poweron_flag = 1;
        }
        rtc_alarm_disable();
        return;                                                         //长按PWRKEY 10S复位后直接开机。
    }

    if (rtccon9 & BIT(0) && (sys_cb.vbat >LPWR_OFF_VBAT)) {
        printf("alarm wakeup\n");
        rtc_alarm_disable();
        return;
    }

#if USER_PWRKEY
    int up_cnt = 0, ticks = 0;
    if (!IS_PWRKEY_PRESS()) {
        pwrkey_pressed_flag = 1;
        ticks = sys_cb.ms_ticks;
    }
#endif // USER_PWRKEY

    pwron_press_nms = PWRON_PRESS_TIME;
    if (pwron_press_nms < 100) {
        pwron_press_nms = 100;                                          //最小开机时间在100ms左右
    }

    //第一次上电是否直接开机
    if (is_powron_frist_enable(rtccon9) && (sys_cb.vbat >LPWR_OFF_VBAT)) {
        rtc_alarm_disable();
        return;
    }

    while (1) {
        WDT_CLR();
        delay_ms(1);
        bsp_saradc_tmr1ms_process();

#if CHARGE_EN
        // 实时更新电池电压会有100mv的误差   否则在while(1)循环中电池电压更新慢且不准
        if ((charge_cnt %5) == 0) {
            saradc_kick_start_do(saradc_cb.channel | BIT(ADCCH_VBAT), 0, 0);
        }
#endif

#if USER_PWRKEY
        u8 key_val = bsp_key_scan();
        if ((key_val & KEY_USAGE_MASK) == key_cb.pwr_usage_id) {
            up_cnt = 0;
            if (!pwrkey_pressed_flag) {
                ticks = tick_get();
                sys_cb.ms_ticks = ticks;                                //记录PWRKEY按键按下的时刻
                pwrkey_pressed_flag = 1;
            }
            if (!sys_cb.poweron_flag) {
                if (tick_check_expire(ticks, pwron_press_nms)) {        //长按开机时间配置
                    sys_cb.poweron_flag = 1;
                }
            }
        } else {
            if (up_cnt < 3*5) {
                up_cnt++;
            }
            if (up_cnt == 3*5) {
                up_cnt = 10*5;
                pwrkey_pressed_flag = 0;
                sys_cb.poweron_flag = 0;
            }
        }
#else
     pwrkey_pressed_flag = 1;
#endif // USER_PWRKEY

#if CHARGE_EN
        if (xcfg_cb.charge_en) {
			charge_cnt++;
			if ((charge_cnt % (20*5)) == 0) {
                charge_detect(0);
            }
            if (charge_cnt >= (200*5)) {  //1S
                charge_cnt = 0;
			}

            if (CHARGE_DC_IN() && (sys_cb.vbat > LPWR_OFF_VBAT)) {
                printf(" break sys_cb.vbat %d \n", sys_cb.vbat);
                break;
            }
#if CHARGE_VIO_0V_EN
            if (CHARGE_DC_IN() && bsp_charge_check_vio0v_bit()) {
                printf(" 0v startup, sys_cb.vbat %d\n", sys_cb.vbat);
                break;
            }
#endif

            //充满且蓝灯已灭，进入关机状态
            if ((sys_cb.charge_sta == 2) && (!sys_cb.charge_bled_flag)) {
                bsp_saradc_exit();
                sfunc_pwrdown(0);
            }
        }
#endif // CHARGE_EN

        if (sys_cb.poweron_flag) {
__pwron:
#if VBAT_DETECT_EN
            if (sys_cb.vbat <= LPWR_OFF_VBAT) {  //电压小于3.1v不开机
                continue;
            }
#endif
            if ((CHARGE_DC_NOT_PWRON) && CHARGE_DC_IN()) {
                continue;
            }
            //长按PP/POWER开机
#if !LP_XOSC_CLOCK_EN
            rtc_power_on_calibration();
#endif
            break;
        } else {
            //PWKKEY松开，立刻关机
            if (!pwrkey_pressed_flag) {
                if (!CHARGE_DC_IN()) {
                    if ((!SOFT_POWER_ON_OFF) || ((!USER_PWRKEY) && (!PWRKEY_2_HW_PWRON))) {
                        goto __pwron;       //没有软开关机功能，直接开机。
                    }
                    bsp_saradc_exit();
#if !LP_XOSC_CLOCK_EN
                    if (cm_read8(PARAM_RTC_CAL_VALID) == 1) {
                        sniff_rc_init();
                        rtc_calibration_read(PARAM_RTC_CAL_ADDR);
                        rtc_sleep_process();
                        rtc_printf();
                    }
                    sys_clk_set(SYS_24M);
#endif
                    sfunc_pwrdown(1);
                }
            }
        }
    }
}

void key_init(void)
{
    key_var_init();

#if USER_IOKEY
    io_key_init();
#endif

#if USER_ADKEY
    saradc_set_channel(BIT(ADKEY_CH));
//    adcch_io_pu10k_enable(ADKEY_CH);          //开内部10K上拉
#endif

#if USER_PWRKEY
    RTCCON1 |= BIT(4) | BIT(0);
    saradc_set_channel(BIT(ADCCH_VRTC) | BIT(ADCCH_WKO));
    key_cb.pwr_usage_id = pwrkey_table[USER_PWRON_KEY_SEL].usage_id; //开关机按键定义
#endif

#if VBAT_DETECT_EN
    saradc_set_channel(BIT(ADCCH_BGOP) | BIT(VBAT2_ADCCH));
#endif

#if USER_KEY_QDEC_EN || USER_ADKEY_QDEC_EN
    bsp_qdec_init();                            //旋转编码器初始化
#endif

    bsp_saradc_init();
    power_on_check();
    CRSTPND = 0x1ff0000;                        //clear reset pending
}

AT(.com_text.bsp.key)
bool get_poweron_flag(void)
{
    return sys_cb.poweron_flag;
}

AT(.com_text.bsp.key)
void set_poweron_flag(bool flag)
{
    sys_cb.poweron_flag = flag;
}

u8 get_pwroff_pressed_time(void)
{
    return PWROFF_PRESS_TIME;
}

u8 get_double_key_time(void)
{
    if (USER_MULTI_KEY_TIME > 8) {
        return 70;
    } else {
        return (u8)(USER_MULTI_KEY_TIME*20 + 1);
    }
}

//AT(.com_rodata.bsp.key)
//const char key_str[] = "enqueue: %04x\n";

AT(.com_text.bsp.key)
u16 bsp_key_process(u16 key_val)
{
    u16 key_return = NO_KEY, key;
	key_return = key_process(key_val);
    key = key_return;

    //多击处理
#if USER_MULTI_PRESS_EN
    key_return = key_multi_press_process(key_return);
    if (key != key_return && (key & KEY_TYPE_MASK) == KEY_SHORT_UP && (sys_cb.gui_sleep_sta == 0)) {
    	msg_enqueue(key);                   //short up key
//        printf(key_str, key, sys_cb.gui_sleep_sta, sys_cb.gui_need_wakeup);
    }
#endif
    return key_return;
}

AT(.com_text.bsp.key)
u8 bsp_key_scan(void)
{
    static bool flag_detach_delay_key = false;
    u8 key_val = NO_KEY;
    u16 key = NO_KEY;

#if VBAT_DETECT_EN
    if (adc_vbat2_flag) {
        adc_vbat2_flag = false;
        sys_cb.vbat = bsp_vbat_get_voltage(0);
    }
#endif

#if VUSB_DETECT_EN
    sys_cb.vusb = bsp_vusb_get_voltage();
#endif

#if USER_ADKEY
    if (key_val == NO_KEY) {
        key_val = get_adkey(saradc_get_value8(ADKEY_CH));
    }
#endif // USER_ADKEY

#if USER_PWRKEY
    if (key_val == NO_KEY) {
        key_val = get_pwrkey();
        //printf("key_val: %d\n", key_val);
    }
#endif // USER_PWRKEY

#if USER_IOKEY
    if (key_val == NO_KEY) {
        key_val = get_iokey();
    }
#endif // USER_IOKEY

    key = bsp_key_process(key_val);
    if (key != NO_KEY) {
        //防止enqueue多次HOLD消息
        if ((key & KEY_TYPE_MASK) == KEY_LONG) {
            sys_cb.kh_vol_msg = (key & KEY_USAGE_MASK) | KEY_HOLD;
        } else if ((key & KEY_TYPE_MASK) == KEY_LONG_UP) {
            msg_queue_detach(sys_cb.kh_vol_msg, 0);
            sys_cb.kh_vol_msg = NO_KEY;
        } else if (sys_cb.kh_vol_msg == key) {
            msg_queue_detach(key, 0);
        }
//        printf(key_str, key, sys_cb.gui_sleep_sta, sys_cb.gui_need_wakeup);
        if (sys_cb.gui_sleep_sta) {
            sys_cb.gui_need_wakeup = 1;
        }

        if (sys_cb.gui_sleep_sta == 0) {
            if (flag_detach_delay_key != true) {
                msg_enqueue(key);                   //short up key
            }

            flag_detach_delay_key = false;
        } else {
            flag_detach_delay_key = true;
        }

        reset_sleep_delay_all();
    }
    return key_val;
}

