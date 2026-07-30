#include "include.h"

void plugin_init(void)
{
    CLKGAT0 = BIT(0)|BIT(1)|BIT(2)|BIT(3)|BIT(4)|BIT(5)|BIT(6)|BIT(12)|BIT(14)|BIT(15)|BIT(16)|BIT(17)|BIT(24)|BIT(30)|BIT(31);
    CLKGAT1 = 0x3fffffef;
    CLKGAT2 = BIT(0) | BIT(1) | BIT(4) | BIT(9) | BIT(20) | BIT(21) | BIT(23) | BIT(24) | BIT(25) | BIT(28) | BIT(29) | BIT(30);
    CLKGAT3 = BIT(13) | BIT(15) |BIT(18);
}

void plugin_var_init(void)
{
#if CHARGE_EN
    if (!xcfg_cb.charge_en) {
        xcfg_cb.charge_dc_reset = 0;
        xcfg_cb.charge_dc_not_pwron = 0;
    }
#endif // CHARGE_EN

#if BT_HFP_INBAND_RING_EN
    if (xcfg_cb.bt_hfp_inband_ring_en) {
        xcfg_cb.bt_hfp_ring_number_en = 0;
    }
#endif

    //PWRKEY模拟硬开关,需要关闭长按10S复位， 第一次上电开机， 长按5秒进配对等功能。
    if (PWRKEY_2_HW_PWRON) {
        xcfg_cb.powkey_10s_reset = 0;
//        xcfg_cb.pwron_frist_bat_en = 0;
//        xcfg_cb.bt_pwrkey_nsec_discover = 0;
    }

#if LED_DISP_EN
	led_func_init();
#endif // LED_DISP_EN

    //MIC0与MIC1不能同时使用省RC方案
    if (xcfg_cb.mic0_en && xcfg_cb.mic1_en) {
        if (xcfg_cb.mic0_bias_method == 2 && xcfg_cb.mic1_bias_method == 2) {
            printf("-->mic bias method error!!\n");
            while(1);
        }
    }
    if (xcfg_cb.mic0_bias_method == 2) {
        xcfg_cb.mic0_pwr_sel = 0;
    }
    if (xcfg_cb.mic1_bias_method == 2) {
        xcfg_cb.mic1_pwr_sel = 0;
    }
}

AT(.com_text.plugin)
void plugin_tmr5ms_isr(void)
{

}

AT(.com_text.plugin)
void plugin_tmr1ms_isr(void)
{

}

//蓝牙休眠时，唤醒IO配置
void sleep_wakeup_config(void)
{
    WKUPCON |= BIT(17);                                 //wakup sniff enable
    wko_wakeup_init(1);                                 //wko下降沿唤醒

#if USER_ADKEY
    port_wakeup_init(get_adc_gpio_num(ADKEY_CH), 1, 1); //配置ADKEY IO下降沿唤醒。
#endif // USER_ADKEY

#if USER_IOKEY
    port_wakeup_init(IO_PF1, 1, 1);                     //开内部上拉，下降沿唤醒
    port_wakeup_init(IO_PF2, 1, 1);
#endif // USER_IOKEY

#if ELUNCHBOX_PANEL_EN
    port_wakeup_init(IO_PE1, 1, 1);                     //PE1(PT8028 OUT_FLAG) 下降沿唤醒
#endif

    port_int_disable_to_sleep();
}

void sleep_wakeup_exit(void)
{
    wko_wakeup_exit();

#if USER_ADKEY
    port_wakeup_exit(get_adc_gpio_num(ADKEY_CH));
#endif // USER_ADKEY

#if USER_IOKEY
    port_wakeup_exit(IO_PF1);
    port_wakeup_exit(IO_PF2);
    io_key_init();
#endif // USER_IOKEY

#if ELUNCHBOX_PANEL_EN
    port_wakeup_exit(IO_PE1);
#endif

    port_int_enable_exit_sleep();

    WKUPCON &= ~BIT(17);                        //wakup sniff disable
}

bool bt_hfp_ring_number_en(void)
{
    return xcfg_cb.bt_hfp_ring_number_en;
}

//设置piano提示音播放的数字音量 (0 ~ 0x7fff)
u32 get_piano_digvol(void)
{
    return 0x7fff;//0x6000;
}

//设置WAV RES提示音播放的数字音量 (0 ~ 0x7fff)
u32 get_wav_res_digvol(void)
{
    return 0x5000;
}

void maxvol_tone_play(void)
{
#if WARING_MAXVOL_MP3
    func_mp3_res_play(RES_BUF_MAX_VOL_MP3, RES_LEN_MAX_VOL_MP3);
#else // WARING_MAXVOL_MP3
#if WARNING_WAVRES_PLAY
        wav_res_play(RES_BUF_MAX_VOL_WAV, RES_LEN_MAX_VOL_WAV);
#else
        bsp_piano_warning_play(WARNING_TONE, TONE_MAX_VOL);
#endif

#endif // WARING_MAXVOL_MP3
}

void minvol_tone_play(void)
{

}

void plugin_playmode_warning(void)
{

}

void plugin_lowbat_vol_reduce(void)
{
#if LOWPWR_REDUCE_VOL_EN
    music_src_set_volume(0x50c0);       //设置音乐源音量达到整体降低系统音量 (范围：0~0x7fff)
#endif // LOWPWR_REDUCE_VOL_EN
}

void plugin_lowbat_vol_recover(void)
{
#if LOWPWR_REDUCE_VOL_EN
    music_src_set_volume(0x7fff);       //还原音量
#endif // LOWPWR_REDUCE_VOL_EN
}

//用于调DAC音质接口函数
void plugin_music_eq(void)
{
    bsp_eq_init();
    sys_cb.eq_mode = 0;
    if (cfg_dac_out_spr & BIT(1)) {         //dac out is 96k?
        music_set_eq_by_res(RES_BUF_EQ_DAC96K_EQ, RES_LEN_EQ_DAC96K_EQ);
    } else {
        music_set_eq_by_num(sys_cb.eq_mode);
#if DAC_DRC_EN
        music_set_drc_by_res(RES_BUF_DRC_DAC_DRC, RES_LEN_DRC_DAC_DRC);
#endif
    }
}

AT(.com_text.port.vbat)
void plugin_vbat_filter(u32 *vbat)
{
#if  VBAT_FILTER_USE_PEAK
    //电源波动比较大的音箱方案, 取一定时间内的电量"最大值"或"次大值",更能真实反应电量.
    #define VBAT_MAX_TIME  (3000/5)   //电量峰值判断时间 3S
    static u16 cnt = 0;
	static u16 vbat_max_cnt = 0;
    static u32 vbat_max[2] = {0,0};
    static u32 vbat_ret = 0;
    u32 vbat_cur = *vbat;
    if (cnt++  < VBAT_MAX_TIME) {
        if (vbat_max[0] < vbat_cur) {
            vbat_max[1] = vbat_max[0];  //vbat_max[1] is less max (次大值)
            vbat_max[0] = vbat_cur;     //vbat_max[0] is max(最大值)
            vbat_max_cnt = 0;
        } else if (vbat_max[0] == vbat_cur) {
            vbat_max_cnt ++;
        }
    } else {
        if (vbat_max_cnt >= VBAT_MAX_TIME/5) {  //总次数的(1/5)都采到最大值,则返回最大值.
            vbat_ret = vbat_max[0];
        } else if (vbat_max[1] != 0) {   //最大值次数较少,则返回次大值(舍弃最大值)
            vbat_ret = vbat_max[1];
        }
        vbat_max[0] = 0;
        vbat_max[1] = 0;
        vbat_max_cnt = 0;
        cnt = 0;
    }
    //返回值
    if (vbat_ret != 0) {
        *vbat = vbat_ret;
    }
#endif
}


