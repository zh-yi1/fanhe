#include "include.h"
#include "func.h"
#include "func_fmrx.h"

#if FUNC_FMRX_EN
AT(.text.func.fmrx.msg)
void func_fmrx_message(u16 msg)
{
    switch (msg) {
        case KU_BACK:
            printf("==>FM KU_BACK\n");
            sys_cb.mute = 0;
            if (fmrx_cb.sta < FMRX_PLAY) {
                if(fmrx_cb.sta == FMRX_SEEK_PLAY) {
                    dac_fade_out();
                    dac_fade_wait();
                }
                fmrx_cb.sta = FMRX_SEEK_STOP;   //停止搜台
            } else {
                func_fmrx_pause_play();
            }
            break;

        case KL_BACK:
            printf("==>FM KL_BACK\n");
            sys_cb.mute = 0;
            if (fmrx_cb.sta == FMRX_PAUSE) {
                func_fmrx_pause_play();
            }
            if (fmrx_cb.sta == FMRX_PLAY) {
                fmrx_cb.sta = FMRX_SEEK_START;
            } else {
                if(fmrx_cb.sta == FMRX_SEEK_PLAY) {
                    dac_fade_out();
                    dac_fade_wait();
                }
                fmrx_cb.sta = FMRX_SEEK_STOP;
            }
            break;

        case KU_LEFT:
            printf("==>FM KU_LEFT\n");
            //切台过程中如果是mute状态要先解除mute（打开fmrx_digital）
            if (fmrx_cb.sta == FMRX_PAUSE) {
                func_fmrx_pause_play();
            }
            #if FMRX_TEST_CHANNEL
            fmrx_test_channel_switch(0);
            #else
            fmrx_switch_channel(0);
            //fmrx_switch_freq(0);
            #endif
            break;

        case KU_RIGHT:
            printf("==>FM KU_RIGHT\n");
            if (fmrx_cb.sta == FMRX_PAUSE) {
                func_fmrx_pause_play();
            }

            #if FMRX_TEST_CHANNEL
            fmrx_test_channel_switch(1);
            #else
            fmrx_switch_channel(1);
            //fmrx_switch_freq(1);
            #endif

            break;

#if FMRX_HALF_SEEK_EN
        case KL_LEFT:
            fmrx_half_seek_start(0);
            break;

        case KL_RIGHT:
            fmrx_half_seek_start(1);
            break;
#endif

//        case KU_MODE:
//            bsp_fmrx_logger_out();
//            break;

        default:
            //func_message(msg);
            break;
    }
}




AT(.text.func.fmrx)
void func_fmrx_bypass(void)
{
    dac_fade_out();
}

AT(.text.func.fmrx)
void func_fmrx_restore(void)
{
    dac_fade_in();
}

//先暂停FMRX再播放提示音
AT(.text.func.fmrx)
void func_fmrx_mp3_res_play(u32 addr, u32 len)
{
//    if (len == 0) {
//        return;
//    }
//    if (fmrx_cb.sta == FMRX_PAUSE) {
//        mp3_res_play(addr, len);
//    } else {
//        dac_fade_out();
//        dac_fade_wait();
//        fmrx_digital_stop();
//        mp3_res_play(addr, len);
//        fmrx_digital_start();
//        dac_fade_in();
//    }
}

//FMRX播放暂停控制
AT(.text.func.fmrx)
void func_fmrx_pause_play(void)
{
    if (fmrx_cb.sta == FMRX_PLAY) {
        dac_fade_out();
        dac_fade_wait();
        fmrx_digital_stop();
        fmrx_cb.sta = FMRX_PAUSE;
        led_idle();
    } else if (fmrx_cb.sta == FMRX_PAUSE) {
        fmrx_digital_start();
        dac_fade_in();
        fmrx_cb.sta = FMRX_PLAY;
    }
}

//调音量解除FMRX暂停
AT(.text.func.fmrx)
void func_fmrx_setvol_callback(u8 dir)
{
    if (fmrx_cb.sta == FMRX_PAUSE) {
        func_fmrx_pause_play();
    }
}

AT(.text.func.fmrx)
void save_ch_buf(u16 freq)
{
    unsigned char index, bit_pos;
    index = (freq - 875) / 16;
    bit_pos = (freq - 875) % 16;
    fmrx_cb.buf[index] |= BIT(bit_pos);
}

AT(.text.func.fmrx)
u16 get_ch_freq(u8 ch)
{
    u8 i, j;
    for (i = 0; i < 13; i++) {
        for (j = 0; j < 16; j++) {
            if (fmrx_cb.buf[i] & BIT(j)) {
                ch--;
                if (ch == 0) {
                    return ((j + i*16) + 875)*10;
                }
            }
        }
    }
    return FM_FREQ_MIN;
}

AT(.text.func.fmrx)
u16 get_ch_all(void)
{
    u8 i, j, ch = 0;
    for (i = 0; i < 13; i++) {
        for (j = 0; j < 16; j++) {
            if (fmrx_cb.buf[i] & BIT(j)) {
                ch++;
            }
        }
    }
    return ch;
}

AT(.text.func.fmrx)
void reset_fm_cb(void)
{
    fmrx_cb.freq = FM_FREQ_MIN;
    fmrx_cb.ch_cnt = 0;
    fmrx_cb.ch_cur = 1;
    memset(fmrx_cb.buf, 0, 26);
}

static void fmrx_set_curch_freq(void)
{
    param_fmrx_chcur_write();
    param_sync();
    fmrx_cb.freq = get_ch_freq(fmrx_cb.ch_cur);
    //gui_box_show_chan();
    bsp_fmrx_set_freq(fmrx_cb.freq);
    dac_fade_in();
    printf("ch freq[%d/%d]: %d\n", fmrx_cb.ch_cur, fmrx_cb.ch_cnt, fmrx_cb.freq);
}

AT(.text.func.fmrx)
void fmrx_switch_freq(u8 dir)
{
    if (dir > 0) {
        fmrx_cb.freq += 10;
        if (fmrx_cb.freq > 10800) {
            fmrx_cb.freq = 8750;
        }
    } else {
        fmrx_cb.freq -= 10;
        if (fmrx_cb.freq < 8750) {
            fmrx_cb.freq = 10800;
        }
    }
    bsp_fmrx_set_freq(fmrx_cb.freq);
    dac_fade_in();
    printf("switch freq: %d\n", fmrx_cb.freq);
}

AT(.text.func.fmrx)
void fmrx_switch_channel(u8 dir)
{
    sys_cb.mute = 0;
    if (fmrx_cb.ch_cnt == 0) {
        return;
    }

    if (dir > 0) {
        fmrx_cb.ch_cur++;
        if (fmrx_cb.ch_cur > fmrx_cb.ch_cnt) {
            fmrx_cb.ch_cur = 1;
        }
    } else {
        fmrx_cb.ch_cur--;
        if (fmrx_cb.ch_cur < 1) {
            fmrx_cb.ch_cur = fmrx_cb.ch_cnt;
        }
    }
    fmrx_set_curch_freq();
}


AT(.text.func.fmrx)
void fmrx_seek_start(void)
{
    printf("fmrx seek start\n");
    reset_fm_cb();
    fmrx_cb.seek_start = 1;
    fmrx_cb.seek_half_reset = 1;
    fmrx_cb.sta = FMRX_SEEKING;
    dac_fade_out();
    dac_fade_wait();
}

AT(.text.func.fmrx)
void fmrx_seek_stop(void)
{
    printf("fmrx seek stop\n");
    fmrx_cb.sta = FMRX_PLAY;
    fmrx_cb.freq = FM_FREQ_MIN;
    if (fmrx_cb.ch_cnt > 0) {
        fmrx_cb.ch_cur = 1;
        fmrx_cb.freq = get_ch_freq(fmrx_cb.ch_cur);
    }
    param_fmrx_chcur_write();
    param_fmrx_chcnt_write();
    param_fmrx_chbuf_write();
    param_sync();

    bsp_fmrx_set_freq(fmrx_cb.freq);
    dac_fade_in();
}

#if FMRX_HALF_SEEK_EN
AT(.text.func.fmrx)
void fmrx_half_seek_start(u8 dir)
{
    sys_cb.mute = 0;
    if (fmrx_cb.sta != FMRX_PLAY) {
        return;
    }
    fmrx_cb.seek_start = 2;
    fmrx_cb.hstep = -10;
    if (dir) {
        fmrx_cb.hstep = 10;
    }
    fmrx_cb.fcnt = 0;
    fmrx_cb.freq += fmrx_cb.hstep;
    if (fmrx_cb.freq > FM_FREQ_MAX) {
        fmrx_cb.freq = FM_FREQ_MIN;
    } else if (fmrx_cb.freq < FM_FREQ_MIN) {
        fmrx_cb.freq = FM_FREQ_MAX;
    }
    fmrx_cb.sta = FMRX_SEEKING_HALF;
    dac_fade_out();
}

AT(.text.func.fmrx)
void fmrx_half_seek_stop(void)
{
    fmrx_cb.seek_start = 0;
    fmrx_cb.sta = FMRX_PLAY;
    bsp_fmrx_set_freq(fmrx_cb.freq);
    dac_fade_in();
}
#endif // FMRX_HALF_SEEK_EN


AT(.text.fmrx_com)
void func_fmrx_process(void)
{
#if FMRX_INFO_PRINT
    fmrx_info_print();
#endif
    //func_process();
    WDT_CLR();
    switch (fmrx_cb.sta) {
        case FMRX_PLAY:
            break;

#if FMRX_HALF_SEEK_EN
        case FMRX_SEEKING_HALF:
            if( fmrx_cb.seek_half_reset){
                fmrx_cb.seek_half_reset = 0;
                #if 1
                fmrx_cb.freq = FM_FREQ_MIN;  //半自动搜台时不清除以前的电台
                #else
                reset_fm_cb();               //半自动搜台时清除以前的电台
                #endif
            }
            if (bsp_fmrx_check_freq(fmrx_cb.freq)) {
                fmrx_half_seek_stop();
                save_ch_buf(fmrx_cb.freq / 10);
                fmrx_cb.ch_cnt = get_ch_all();
                printf("all_ch = %d, cur freq: %d\n",fmrx_cb.ch_cnt, fmrx_cb.freq);
                fmrx_cb.ch_cur = fmrx_cb.ch_cnt;
                param_fmrx_chcur_write();
                param_fmrx_chcnt_write();
                param_fmrx_chbuf_write();
                param_sync();
                bsp_fmrx_set_freq(fmrx_cb.freq);
                dac_fade_in();
                break;
            }
            fmrx_cb.freq += fmrx_cb.hstep;
            if (fmrx_cb.freq > FM_FREQ_MAX) {
                fmrx_cb.freq = FM_FREQ_MIN;
            } else if (fmrx_cb.freq < FM_FREQ_MIN) {
                fmrx_cb.freq = FM_FREQ_MAX;
            }
            break;
#endif // FMRX_HALF_SEEK_EN

        case FMRX_SEEK_START:
            fmrx_seek_start();
            break;

        case FMRX_SEEKING:
//            if(bsp_res_is_playing()) {      //播报提示音时不搜台，避免搜台太慢引起提示音卡顿
//                break;
//            }
            if (bsp_fmrx_check_freq(fmrx_cb.freq)) {
                fmrx_cb.ch_cnt++;
                save_ch_buf(fmrx_cb.freq / 10);
                printf("seeked freq[%d]: %d\n",fmrx_cb.ch_cnt, fmrx_cb.freq);
                fmrx_cb.ch_cur = fmrx_cb.ch_cnt;
                bsp_fmrx_set_freq(fmrx_cb.freq);
                dac_fade_in();
                fmrx_cb.play_tick = tick_get();
                fmrx_cb.sta = FMRX_SEEK_PLAY;
                break;
            }

fmrx_seek_next:
            fmrx_cb.freq += 10;
            if (fmrx_cb.freq > FM_FREQ_MAX) {
                fmrx_cb.sta = FMRX_SEEK_STOP;
            }
            break;

        case FMRX_SEEK_PLAY:
            if(tick_check_expire(fmrx_cb.play_tick, 1500)) {
                dac_fade_out();
                dac_fade_wait();  //等声音完全淡出,防止设置下一电台时会有噪音出来
                fmrx_cb.sta = FMRX_SEEKING;
                goto fmrx_seek_next;
            }
            break;

        case FMRX_SEEK_STOP:
            fmrx_seek_stop();
            //bsp_fmrx_logger_out();
            break;
    }
}


AT(.text.func.fmrx)
void func_fmrx_enter(void)
{
	printf("%s\n");
    func_cb.sta = FUNC_FMRX;
#if 1
    func_bt_chk_off();           //确认关掉蓝牙,干扰会小些
    sys_clk_set(SYS_24M);        //系统时钟越小干扰越小
#endif
    memset(&fmrx_cb, 0, sizeof(fmrx_cb));
    func_cb.set_vol_callback = func_fmrx_setvol_callback;
    param_fmrx_chcur_read();
    param_fmrx_chcnt_read();
    param_fmrx_chbuf_read();

    //load_code_func();
    fmrx_cb.freq = get_ch_freq(fmrx_cb.ch_cur);
    fmrx_cb.seek_half_reset = 1;
    msg_queue_clear();
    AMPLIFIER_SEL_AB();
    bsp_loudspeaker_unmute();

#if WARNING_FUNC_FMRX
//    if (res_play_en) {
//        bsp_res_play(RES_IDX_FM_MODE);
//        bsp_res_w4_finish(true);
//    }
//    load_code_func();           //播完mp3提示音后code会被覆盖，需要重新load
#endif

    adpll_spr_set(DAC_OUT_48K);  //只支持48K的采样率
    fmrx_cb.phasecom_en = (DACDIGCON0>>6)&0x01;
    DACDIGCON0 |= BIT(6);
    PHASECOMP0 = 0;

    bsp_fmrx_init();

#if FMRX_OPTIMIZE_TRY
    fmrx_optimize_try_set();   //尝试时钟控制,CLK分频等
#endif
    fmrx_cb.sta = FMRX_PLAY;
#if FMRX_TEST_CHANNEL
    fmrx_cb.freq = fmrx_test_first_channel_get();
#endif
    fmrx_cb.freq = 8750;   //8750  //8770  //9150 //10380
    bsp_fmrx_set_freq(fmrx_cb.freq);
    dac_fade_in();
    printf("%s ok, freq[%d/%d] = %d\n", __func__ ,fmrx_cb.ch_cur, fmrx_cb.ch_cnt,fmrx_cb.freq);
}

AT(.text.func.fmrx)
void func_fmrx_exit(void)
{
    dac_fade_out();
    if (fmrx_cb.sta > FMRX_IDLE) {
        bsp_fmrx_exit();
    }
#if FMRX_OPTIMIZE_TRY
    fmrx_optimize_try_recover();
#endif
    sys_clk_set(SYS_CLK_SEL);
    adpll_spr_set(DAC_OUT_SPR);
    if(!fmrx_cb.phasecom_en) {
        DACDIGCON0 &= ~BIT(6);
    }
    AMPLIFIER_SEL_D();
    func_cb.last = FUNC_FMRX;
}

AT(.text.func.fmrx)
void func_fmrx(void)
{
    printf("%s\n", __func__);
    func_fmrx_enter();
    while (func_cb.sta == FUNC_FMRX) {
        func_fmrx_process();
        func_fmrx_message(msg_dequeue());
    }
    func_fmrx_exit();
}
#endif // FUNC_FMRX_EN
