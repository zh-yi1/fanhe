#include "include.h"

void mp3_res_play_kick(u32 addr, u32 len);
void wav_res_play_kick(u32 addr, u32 len);
void wav_res_dec_process(void);
bool wav_res_is_play(void);
bool wav_res_stop(void);
void mp3_res_play_exit(void);

#if DAC_DNR_EN
static u8 msc_dnr_sta;
#endif
static bool mute_sta = false;

u8 mp3_res_process(void)
{
    if (sys_cb.mp3_res_playing) {
        if (get_music_dec_sta() == MUSIC_STOP) {
            printf("mp3 ring stop:%d\n", sys_cb.mute);
            sys_cb.mp3_res_playing = false;
            bsp_change_volume(sys_cb.vol);
            music_control(MUSIC_MSG_STOP);

            if (music_set_eq_is_done()) {
                music_set_eq_by_num(sys_cb.eq_mode); // 恢复 EQ
            }
            mp3_res_play_exit();

            if (mute_sta) {
                bsp_sys_mute();
                mute_sta = sys_cb.mute;
            }

#if DAC_DNR_EN
            dac_dnr_set_sta(msc_dnr_sta);
#endif
            func_bt_mp3_play_restore();
            return 0;
        }

    }
    return 1;
}

void mp3_res_play_do(u32 addr, u32 len, bool sync)
{
    if (len == 0) {
        return;
    }

    printf("mp3 ring: %x, %x, %d\n", addr, len, sys_cb.mute);
#if DAC_DNR_EN
    msc_dnr_sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

    if (get_music_dec_sta() != MUSIC_STOP) { //避免来电响铃/报号未完成，影响get_music_dec_sta()状态
        music_control(MUSIC_MSG_STOP);
    }

    bsp_change_volume(WARNING_VOLUME);

    if(sys_cb.mute) {
        mute_sta = sys_cb.mute;
        bsp_sys_unmute();
    }

    if (music_set_eq_is_done()) {
        music_set_eq_by_num(0); // EQ 设置为 normal
    }
    mp3_res_play_kick(addr, len);
    sys_cb.mp3_res_playing = true;
}

void mp3_res_play(u32 addr, u32 len)
{
    mp3_res_play_do(addr, len, 0);
}

void mp3_res_play_w4_done(void)
{
    if (sys_cb.mp3_res_playing) {
        while(mp3_res_process()) {
            WDT_CLR();
            vusb4s_reset_clr_cnt();
        }
    }
}

void mp3_res_play_block(u32 addr, u32 len)
{
    bt_audio_bypass();

	mp3_res_play(addr, len);
	while(mp3_res_process()) {
        bt_thread_check_trigger();
        WDT_CLR();
        vusb4s_reset_clr_cnt();
    }

    bt_audio_enable();
}

//int mp3_read_func(void *buf, u32 addr, u32 len)
//{
//    return os_spiflash_read(buf, addr, len);
//}
//
//void mp3_play_flash(u32 addr, u32 len)
//{
//	register_spi_read_function(mp3_read_func);
//	mp3_res_play_block(addr, len);
//	register_spi_read_function(NULL);
//}


#if WARNING_WAVRES_PLAY
/**
 *  播放wav，会等待wav播放完成
 */
static void wav_res_play_do(u32 addr, u32 len, bool sync)
{
    if (len == 0 || sys_param_cb.camera_vol == 0) {
        return;
    }
    if (api_video_play_sta_get() < AVI_STA_PAUSE && !func_video_allow_warning_tone()) {
        return;
    }

#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif
    if (sys_cb.mute) {
        bsp_loudspeaker_unmute();
		dac_fade_in();
        dac_fade_wait();
    }
    wav_res_play_kick(addr, len);
    while (wav_res_is_play()) {
        bt_thread_check_trigger();
        wav_res_dec_process();
        WDT_CLR();
        vusb4s_reset_clr_cnt();
    }
    wav_res_stop();

	if (sys_cb.mute) {
		bsp_loudspeaker_mute();
		dac_fade_out();
	}

#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif
}

void wav_res_play(u32 addr, u32 len)
{
    wav_res_play_do(addr, len, 0);
}


/**
 *  播放wav，不会会等待wav播放完成
 */
static bool is_wav_play = 0;
static void wav_res_play_no_wait(u32 addr, u32 len, bool sync)
{
    if (len == 0) {
        return;
    }
    if(is_wav_play){
        return;
    }
    is_wav_play = 1;
#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif
    if (sys_cb.mute) {
        bsp_loudspeaker_unmute();
		dac_fade_in();
        dac_fade_wait();
    }
    wav_res_play_kick(addr, len);
#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif
}

void wav_res_play_ex(u32 addr, u32 len)
{
    wav_res_play_no_wait(addr, len, 0);
}

void wav_res_play_process(void)
{
    if(is_wav_play){
        if(!wav_res_is_play()){
            wav_res_stop();
			if (sys_cb.mute) {
				dac_fade_out();
				bsp_loudspeaker_mute();
			}
            is_wav_play = 0;
        }else{
            wav_res_dec_process();

        }
    }
}

bool bsp_wav_is_playing(void)
{
	return is_wav_play;
}


/**
 *  播放wav，会执行callback
 */
void wav_res_play_with_process(u32 addr, u32 len, void (*process)(void))
{
    if (len == 0 || sys_param_cb.camera_vol == 0) {
        return;
    }
    if(api_video_play_sta_get() < AVI_STA_PAUSE){
        return;
    }

#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif
    if (sys_cb.mute) {
		u8 delay = 50;
        loudspeaker_unmute_no_wait();
		dac_fade_in();
        dac_fade_wait();
		while(delay--){
			process();
			delay_ms(1);
		}
    }
    wav_res_play_kick(addr, len);
    while (wav_res_is_play()) {
        wav_res_dec_process();
        WDT_CLR();
        vusb4s_reset_clr_cnt();
		process();
    }
    wav_res_stop();

	if (sys_cb.mute) {
		bsp_loudspeaker_mute();
		dac_fade_out();
	}

#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif
}


#endif

