#include "include.h"

#if FUNC_REC_EN

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

static rec_src_t rec_src AT(.buf.record);
static rec_cb_t  rec_cb AT(.buf.record);
static au_stm_t rec_enc_stm AT(.buf.record);
static au_stm_t rec_pcm_stm AT(.buf.record);

static u8 rec_obuf[REC_OBUF_SIZE] AT(.rec_buf.obuf);
static u8 rec_encbuf[REC_ENC_SIZE] AT(.rec_buf.enc);
static u8 wr_buff[512];


u8 frame_cnt = 0;

void music_enc_control(u8 msg);
bool sbc_encode_init(u8 spr, u8 nch);
void sbc_encode_exit(void);
bool adpcm_encode_init(u8 nch);
bool mpa_encode_init(u32 spr, u32 nchannel, u32 bitrate);
void mpa_encode_exit(void);
void mpa_encode_kick_start(void);
void bt_call_alg_init(void);
void bt_call_alg_exit(void);
void recorder_sdadc_process(u8 *ptr, u32 samples, u32 ch_mode);
void bt_sco_pcm_buf_rec_cb_init(void(*pcm_callback)(u8 *ptr, u32 samples, u32 ch_mode));

#if FUNC_REC_OPUS_EN
void opus_sdadc_process(u8 *ptr, u32 samples, u32 ch_mode);
static u8 opus_buff[512];
#endif // FUNC_REC_OPUS_EN

static void voice_control_mic_rec_start(void)
{
    if (rec_cb.mode == true) {
        printf("[%s]\n", __func__);
        audio_path_init(AUDIO_PATH_RECORDER);
        audio_path_start(AUDIO_PATH_RECORDER);
        rec_cb.is_rec_path_start = true;
    }
}

static void voice_control_mic_rec_stop(void)
{
    if (rec_cb.mode == true) {
        printf("[%s]\n", __func__);
        audio_path_exit(AUDIO_PATH_RECORDER);
        rec_cb.is_rec_path_start = false;
    }
}

static void mic_rec_start(void)
{
    if (rec_cb.mode == false) {
        printf("[%s]\n", __func__);
        audio_path_init(AUDIO_PATH_RECORDER);
        audio_path_start(AUDIO_PATH_RECORDER);
        rec_cb.is_rec_path_start = true;
    }
}

static void mic_rec_stop(void)
{
    if (rec_cb.mode == false) {
        printf("[%s]\n", __func__);
        audio_path_exit(AUDIO_PATH_RECORDER);
        rec_cb.is_rec_path_start = false;
    }
}

AT(.com_text.record)
bool puts_rec_obuf(u8 *inbuf, u16 len)
{
    rec_cb_t *rec = &rec_cb;
    if (!rec->pcm || !rec->src || rec->sta != REC_RECORDING) {
        return false;
    }

    bool puts_ok  = false;
    puts_ok = puts_stm_buf(rec->pcm, inbuf, len);

#if REC_WAV_SUPPORT
    if (rec->src->rec_type == REC_WAV) {
        music_enc_control(ENC_MSG_WAV);
    }
#endif
#if REC_ADPCM_SUPPORT
    if (rec->src->rec_type == REC_ADPCM) {
        music_enc_control(ENC_MSG_ADPCM);
    }
#endif
#if REC_SBC_SUPPORT
    if (rec->src->rec_type == REC_SBC) {
        music_enc_control(ENC_MSG_SBC);
    }
#endif
#if REC_MP3_SUPPORT
    if (rec->src->rec_type == REC_MP3) {
        if (rec_cb.pcm->len >= rec_cb.trigger_len) {
            mpa_encode_kick_start();
        }
    }
#endif

#if REC_OPUS_SUPPORT

#endif // REC_OPUS_SUPPORT

    return puts_ok;
}

//读取len的ADC数据
AT(.com_text.record)
bool gets_rec_obuf(u8 *buf, u16 len)
{
    if (gets_stm_buf(rec_cb.pcm, buf, len)) {
        pcm_soft_vol_process((s16 *)buf, rec_cb.dig_gain, len >> 1);    //软件数字GAIN
        return true;
    }
    return false;
}

AT(.com_text.record)
bool puts_rec_encbuf(u8 *buf, u16 len)
{
    return puts_stm_buf(rec_cb.enc, buf, len);
}

AT(.com_text.record)
bool gets_rec_encbuf(u8 *buf, u16 len)
{
    return gets_stm_buf(rec_cb.enc, buf, len);
}

#if REC_WAV_SUPPORT
//wave只需将数据从pcmbuf搬到encbuf
void wave_encode_process(void)
{

//    printf("%s\n", __func__);
    u8 *buf = wr_buff;
    memset(buf, 0, 512);
    if (buf == NULL) {
        return;
    }

    if (gets_rec_obuf(buf, 512)) {
        puts_rec_encbuf(buf, 512);
    }

    rec_cb_t *rec = &rec_cb;
    if (rec->sta == REC_RECORDING) {
        reset_sleep_delay_all();

#if FUNC_REC_OPUS_EN
        bool ble_conn_status = ble_is_connect();
        bool opus_status = bsp_opus_is_encode();
        u16 remain_len = opus_enc_data_len_get();
        u16 mtu = (ble_conn_status == true) ? ble_get_gatt_mtu() : FUNC_REC_OPUS_FRAME_LEN;
        if (FUNC_REC_OPUS_BLE_FIX) {
            mtu = FUNC_REC_OPUS_BLE_FIX;
        }
        u8* opus_buf = opus_buff;
        bool opus_encode_flags = false;
        if (remain_len != 0 && opus_status) {
            if (remain_len > mtu) {
                while (remain_len > mtu) {
                    memset(opus_buf, 0, 512);
                    opus_encode_flags = bsp_opus_get_enc_frame(opus_buf, mtu);
                    if (opus_encode_flags) {
#if TRACE_EN
                        printf("==================================================\n");
                                            print_r(opus_buf, mtu);
                        printf("==================================================\n");
#endif // TRACE_EN
                        if (rec->src->rec_type == REC_OPUS) {
                            if (!rec_file_write(opus_buf, mtu, rec->src->rec_type)) {
                                bsp_record_stop();
                                goto __exit;
                            }
                        }
                    }
#if FUNC_REC_OPUS_BLE
                    if (ble_conn_status && opus_encode_flags) {
                    #if (USE_APP_TYPE == APP_AB_LINK)
                        extern bool ble_app_ab_link_send_packet(u8 *buf, u16 len);
//                        ble_app_ab_link_send_packet(opus_buf, mtu);             //用户自行修改发送函数
                        user_common_ble_send(opus_buf, mtu, ble_app_ab_link_send_packet);
                    #endif // USE_APP_TYPE
                    }
#endif // FUNC_REC_OPUS_BLE
                    remain_len = opus_enc_data_len_get();
                }
            }
        }
#endif

        if (rec->src->rec_type != REC_OPUS) {
            if (gets_rec_encbuf(buf, 512)) {
                if (!rec_file_write(buf, 512, rec->src->rec_type)) {
                    bsp_record_stop();
                    goto __exit;
                }
            }
        }

        if (tick_check_expire(rec->tick, REC_SYNC_TIMES)) {
            rec->tick = tick_get();
            bool sync = rec_file_updata(rec->src->rec_type);
            if (sync == false) {
                bsp_record_stop();
            }
        }
    }

__exit:
}
#endif

AT(.text.record)
void bsp_record_process(void)
{
    rec_cb_t *rec = &rec_cb;

    if (rec->sta != REC_RECORDING || rec->src->rec_type == REC_WAV) {
        return;
    }

    u8 *buf = wr_buff;
    memset(buf, 0, 512);
    if (buf == NULL) {
        return;
    }

    if (rec->sta == REC_RECORDING) {
        reset_sleep_delay_all();

#if FUNC_REC_OPUS_EN
        bool ble_conn_status = ble_is_connect();
        bool opus_status = bsp_opus_is_encode();
        u16 remain_len = opus_enc_data_len_get();
        u16 mtu = (ble_conn_status == true) ? ble_get_gatt_mtu() : FUNC_REC_OPUS_FRAME_LEN;
        if (FUNC_REC_OPUS_BLE_FIX) {
            mtu = FUNC_REC_OPUS_BLE_FIX;
        }
        u8* opus_buf = opus_buff;
        bool opus_encode_flags = false;
        if (remain_len != 0 && opus_status) {
            if (remain_len > mtu) {
                while (remain_len > mtu) {
                    memset(opus_buf, 0, 512);
                    opus_encode_flags = bsp_opus_get_enc_frame(opus_buf, mtu);
                    if (opus_encode_flags) {
#if TRACE_EN
                        printf("==================================================\n");
                                            print_r(opus_buf, mtu);
                        printf("==================================================\n");
#endif // TRACE_EN
                        if (rec->src->rec_type == REC_OPUS) {
                            if (!rec_file_write(opus_buf, mtu, rec->src->rec_type)) {
                                bsp_record_stop();
                                goto __exit;
                            }
                        }
                    }
#if FUNC_REC_OPUS_BLE
                    if (ble_conn_status && opus_encode_flags) {
                    #if (USE_APP_TYPE == APP_AB_LINK)
                        extern bool ble_app_ab_link_send_packet(u8 *buf, u16 len);
//                        ble_app_ab_link_send_packet(opus_buf, mtu);             //用户自行修改发送函数
                        user_common_ble_send(opus_buf, mtu, ble_app_ab_link_send_packet);
//                        printf("mtu:%d\n", mtu);
                    #endif
                    }
#endif // FUNC_REC_OPUS_BLE
                    remain_len = opus_enc_data_len_get();
                }
            }
        }
#endif

         if (rec->src->rec_type != REC_OPUS) {
            if (gets_rec_encbuf(buf, 512)) {
                if (!rec_file_write(buf, 512, rec->src->rec_type)) {
                    bsp_record_stop();
                    goto __exit;
                }
            }
        }

        if (tick_check_expire(rec->tick, REC_SYNC_TIMES)) {
            rec->tick = tick_get();
            bool sync = rec_file_updata(rec->src->rec_type);
            if (sync == false) {
                bsp_record_stop();
            }
        }
    }

__exit:

}

AT(.com_text.printf)
const char print[] = "pow [%d,%d]\n";

AT(.com_text.printf1)
const char print1[] = "recorder_sdadc_process:%d\n";

AT(.com_text.printf2)
const char print2[] = "puts_rec_obuf:%d\n";


AT(.com_text.recorder)
void recorder_sdadc_process(u8 *ptr, u32 samples, u32 ch_mode)
{
#if FUNC_REC_AUTO_EN
    static u16 stand_by_time_out = 0;
    static u32 mic_dnr_tick = 0;
    static u32 print_tick = 0;
    static u16 mic_maxpow = 0;
    static u8 ret = 0;
#endif // FUNC_REC_AUTO_EN

    rec_cb_t *rec = &rec_cb;
//    s16 *r_ptr = (s16*)ptr;
//    u8 *d_ptr;

    if (rec->sta == REC_RECORDING) {
#if FUNC_REC_NR_EN
        void noise_dnn_process(s16 *buf, u32 samples, u8 ch);
        u8 ch = (ch_mode & PCM_CHMASK);
//        printf(print2, ch);
        noise_dnn_process((s16*)ptr, samples, ch);
        ch_mode = (ch_mode & ~(PCM_CHMASK)) | (1);
//        rec->src.nchannel = 1;
#endif // FUNC_REC_NR_EN

        if (frame_cnt < 4) {
            frame_cnt++;
        }
        if (frame_cnt > 3) {
            puts_rec_obuf(ptr, (u16)(samples * 2 * (ch_mode & PCM_CHMASK)));//pcm_mode & PCM_CHMASK
        }
#if FUNC_REC_OPUS_EN
        opus_sdadc_process(ptr, samples, ch_mode);
#endif // FUNC_REC_OPUS_EN
    }

#if FUNC_REC_AUTO_EN
    if (tick_check_expire(print_tick, 1000)) {
        print_tick = tick_get();
        printf(print1, rec->sta == REC_RECORDING);
        printf(print, mic_maxpow, ret);
    }

    if (rec->mode == true) {
        extern u8* dac_get_obuf(uint32_t* size);
        extern u16 dnr_buf_maxpow(void *ptr, u16 len);
        extern u8 mic_noise_detect(u16 pow);

        if (tick_check_expire(mic_dnr_tick, 10)) {      //10mS检测一次
            mic_dnr_tick = tick_get();

//            u16 mic_maxpow = dnr_buf_maxpow((u16*)ptr, samples);
            mic_maxpow = bt_get_mic_pmaxow(CH_MIC0, 256);
            ret = mic_noise_detect(mic_maxpow);
//                printf(print, mic_maxpow, ret);
            if (ret == 1) {     //此处再置标志位进行录音即可

                msg_enqueue(EVT_VOX_RECORD_EN);
            } else if (ret == 2) {
                stand_by_time_out++;
                if (stand_by_time_out > 300) {
                    msg_enqueue(EVT_VOX_RECORD_DIS);//此处可实现无声时退出录音
                    stand_by_time_out = 0;
                }
            }
        }
    }
#endif // FUNC_REC_AUTO_EN

//#if MIC_EQ_EN
//    sdadc_pcm_peri_eq(ptr,samples);
//#endif

#if FUNC_REC_PCM2DAC_OUT
    sdadc_pcm_2_dac(ptr, samples, (ch_mode & PCM_CHMASK));               //推DAC播放
#endif // FUNC_REC_PCM2DAC_OUT
}
#if FUNC_REC_NR_EN
static dnn_cb_t dnn_cb;
#endif // FUNC_REC_NR_EN

void bsp_record_noise_alg_init(void)
{
    rec_cb_t *rec = &rec_cb;
    if (rec->is_noise_alg_start == false) {
#if FUNC_REC_NR_EN
//        bsp_change_volume(5);
//        dac_fade_in();
//        dac_spr_set(rec->src->spr);

        memset(&dnn_cb, 0, sizeof(dnn_cb_t));
        dnn_cb_t *cb = &dnn_cb;
        cb->param_printf           = 1;
        cb->nt                     = FUNC_REC_DNN_LEVEL;
        cb->nt_post                = 0; //0-6 >0才起效 0为不开gain指数化 开的话默认为3
        cb->noise_ps_rate          = 1;
        cb->prior_opt_idx	       = 3;
        cb->prior_opt_ada_en	   = 1;

        cb->low_fre_range          = 16; //
        cb->low_fre_range0         = 0;
        cb->pitch_filter_en		   = 1;
//        cb->ps_lowlimt             = 0;
        cb->mask_floor			   = 1000;
//        cb->noise_ceil			   = 0;
        cb->music_lev			   = 11;
//        cb->comforN_level		   = 1;
        cb->gain_expand			   = 1024;
        cb->nn_only				   = 0;
        cb->nn_only_len			   = 16;
        cb->gain_assign			   = 21666;
//        cb->sin_gain_post_en	   = 0;
//        cb->sin_gain_post_len	   = 128;
//        cb->sin_gain_post_len_f	   = 256;
//        cb->spp_thr				   = 8000;
        void noise_dnn_init(dnn_cb_t *dnn_cb);
        noise_dnn_init(&dnn_cb);
        rec->is_noise_alg_start = true;
//        bt_call_alg_init();
//        dac_set_anl_offset(1);
//        bsp_change_volume(bsp_bt_get_hfp_vol(sys_cb.hfp_vol));
//        dac_spr_set(spr);
//        bt_sco_pcm_buf_rec_cb_init(recorder_sdadc_process);
//        rec->is_noise_alg_start = true;
#endif
    }

    if (rec->is_opus_alg_start == false) {
#if FUNC_REC_OPUS_EN
        bsp_opus_encode_start(false, rec->src->spr, rec->src->bitrate);
        rec->is_opus_alg_start = true;
#endif // FUNC_REC_OPUS_EN
    }
}

void bsp_record_noise_alg_exit(void)
{
    rec_cb_t *rec = &rec_cb;
    if (rec->is_noise_alg_start == true) {
#if FUNC_REC_NR_EN
        dac_fade_out();
        dac_fade_wait();
        void noise_dnn_exit(void);
        noise_dnn_exit();
        rec->is_noise_alg_start = false;

//        dac_fade_out();
//        dac_aubuf_clr();
//        bt_call_alg_exit();
//        dac_set_anl_offset(0);
//        bsp_change_volume(sys_cb.vol);
//        rec->is_noise_alg_start = false;
//        bt_sco_pcm_buf_rec_cb_init(NULL);
#endif
    }

    if (rec->is_opus_alg_start == true) {
#if FUNC_REC_OPUS_EN
        bsp_opus_encode_stop(false);
        rec->is_opus_alg_start = false;
#endif // FUNC_REC_OPUS_EN
    }
}

AT(.text.func.record)
u8 get_bsp_record_sta(void)
{
    return rec_cb.sta;
}

void bsp_record_stop(void)
{
    printf("%s\n", __func__);
    rec_cb_t *rec = &rec_cb;
    if (rec->sta == REC_STOP) {
        return;
    }
    frame_cnt = 0;


    if (rec->src && rec->src->source_stop != NULL) {
        rec->src->source_stop();
    }

    ///写入缓存BUFFER残留的数据
    if (rec->enc->len > 0) {
        u32 wlen = rec->enc->len;
        u8 *buf = wr_buff;
        memset(buf, 0, 512);
        if (gets_rec_encbuf(buf, wlen)) {
            if (rec_file_write(buf, wlen, rec->src->rec_type)) {
                rec_file_updata(rec->src->rec_type);
            }
        }
    }

    rec_file_updata(rec->src->rec_type);

    if (rec->flag_file) {
        rec_file_close(rec->src->rec_type);
    }

#if REC_MP3_SUPPORT
    if (rec->src->rec_type == REC_MP3) {
        mpa_encode_exit();
    }
#endif
#if REC_SBC_SUPPORT
    if (rec->src->rec_type == REC_SBC) {
        sbc_encode_exit();
    }
#endif
    rec->flag_file = 0;
    rec->flag_dir = 0;
    rec->sta = REC_STOP;
    rec->flag_play = 1;

#if FUNC_REC_TO_SD
    if (dev_is_online(DEV_SDCARD)) {
        sd0_stop(1);
    }
#endif // FUNC_REC_TO_SD

    // 降噪算法初始化，与通话相关
    if (rec->mode == false) {
        bsp_record_noise_alg_exit();
    }

    rec_cb.start_time = 0;
    rec_cb.time = 0;
}

u8 bsp_record_get_type(void)
{
    if (REC_MP3_SUPPORT) {
        return REC_MP3;
    } else if (REC_WAV_SUPPORT) {
        return REC_WAV;
    } else if (REC_ADPCM_SUPPORT) {
        return REC_ADPCM;
    } else if (REC_SBC_SUPPORT) {
        return REC_SBC;
    } else if (REC_OPUS_SUPPORT) {
        return REC_OPUS;
    }

    return REC_MP3;
}

u8 bsp_record_get_next_type(u8 rec_type)
{

    u8 next = rec_type+1;

    while(1) {
        switch (next) {
        case REC_MP3:
            if (REC_MP3_SUPPORT) {
                return next;
            } else {
                next++;
            }
            break;
        case REC_WAV:
            if (REC_WAV_SUPPORT) {
                return next;
            } else {
                next++;
            }
            break;
        case REC_ADPCM:
            if (REC_ADPCM_SUPPORT) {
                return next;
            } else {
                next++;
            }
            break;
        case REC_SBC:
            if (REC_SBC_SUPPORT) {
                return next;
            } else {
                next++;
            }
        case REC_OPUS:
            if (REC_OPUS_SUPPORT) {
                return next;
            } else {
                next++;
            }
            break;
        default:
            next = REC_WAV;
            break;
        }
    }

    return REC_MP3;
}

//void bsp_record_set_bitrate(u32 btrate)
//{
//    rec_cb_t *rec = &rec_cb;
//    if (rec->sta == REC_STOP) {
//        rec->src->bitrate = btrate;
//        rec->is_set_bitrate = true;
//    } else {
//        printf("[%s] fail\n");
//    }
//}

void bsp_record_set_dig_gain(int dig_gain)
{
    rec_cb_t *rec = &rec_cb;
    if (rec->sta == REC_STOP) {
        rec->dig_gain = dig_gain;
        rec->is_set_dig_gain = true;
    } else {
        printf("[%s] fail\n");
    }
}

//bool is_bsp_record_set_bitrate(void)
//{
//    rec_cb_t *rec = &rec_cb;
//    return rec->is_set_bitrate;
//}

bool is_bsp_record_set_dig_gain(void)
{
    rec_cb_t *rec = &rec_cb;
    return rec->is_set_dig_gain;
}

u32 bsp_record_get_bitrate(void)
{
    rec_cb_t *rec = &rec_cb;
    return rec->src->bitrate;
}

u32 bsp_record_get_dig_gain(void)
{
    rec_cb_t *rec = &rec_cb;
    return rec->dig_gain;
}

bool bsp_record_start(bool is_sco, u8 type, u32 spr, u32 bitrate)
{
#if FUNC_REC_NR_EN
    dac_fade_in();
    dac_spr_set(spr);
#endif // FUNC_REC_NR_EN

#if FUNC_REC_PCM2DAC_OUT
    bsp_change_volume(5);
    bool mute_bkp = bsp_get_mute_sta();
    if (mute_bkp) {
        bsp_sys_unmute();
    } else {
        dac_fade_in();
    }
    dac_spr_set(spr);
#endif // FUNC_REC_PCM2DAC_OUT



    printf("%s\n", __func__);
    rec_cb_t *rec = &rec_cb;
    rec->src = &rec_src;
    rec_src.spr = spr;
#if FUNC_REC_NR_EN
    rec_src.nchannel = 1;
#else
    rec_src.nchannel = FUNC_REC_NCH;
#endif // FUNC_REC_NR_EN
    rec_src.rec_type = type;

#if FUNC_REC_OPUS_BT_MUSIC_EN
    if (rec_src.rec_type != REC_OPUS) {
        printf("rec_type err => please check to opus\n");
        printf("auto check => opus\n");
        rec_src.rec_type = REC_OPUS;
    }
#endif // FUNC_REC_OPUS_BT_MUSIC_EN

    //rec_src.rec_type = REC_WAV;
//    if (!rec->is_set_bitrate) {
        rec_src.bitrate = bitrate;        //用户设置bitrate
//    }
    if (!is_sco) {
        rec_src.source_start = mic_rec_start;
        rec_src.source_stop  = mic_rec_stop;
    } else {
        rec_src.source_start = NULL;
        rec_src.source_stop  = NULL;
    }
    frame_cnt = 0;
    rec->sta = REC_STARTING;
    if (!rec->is_set_dig_gain) {
        rec->dig_gain = DIG_N0DB;       //用户没有设置增益,使用系统默认增益
    }

#if FUNC_REC_TO_SD
    if (dev_is_online(DEV_SDCARD)) {
        sys_cb.cur_dev = DEV_SDCARD;
    } else if (dev_is_online(DEV_UDISK)) {
        sys_cb.cur_dev = DEV_UDISK;
    }
    if (!rec->flag_file) {
        bsp_sd_disk_mount(24);
    }
#endif // FUNC_REC_TO_SD


    //创建录音文件夹
    if (!rec->flag_dir) {
        if (rec_file_creat(rec->src->rec_type) == false) {
            printf("rec_file_creat error\n");
            return false;
        }
        rec->flag_dir = 1;
        rec->flag_file = 1;
    }

#if REC_WAV_SUPPORT
    if (rec->src->rec_type == REC_WAV) {
        if (!record_wav_init(rec->src->nchannel & 0x03, rec->src->spr, rec->src->rec_type)) {
            return false;
        }
    }
#endif
#if REC_ADPCM_SUPPORT
    if (rec->src->rec_type == REC_ADPCM) {
        if (!record_wav_init(rec->src->nchannel & 0x03, rec->src->spr, rec->src->rec_type)) {
            return false;
        }
        adpcm_encode_init(rec->src->nchannel & 0x03);
    }
#endif
#if REC_MP3_SUPPORT
    if (rec->src->rec_type == REC_MP3) {
        rec->trigger_len = 384 << (rec->src->nchannel & 0x03);

        if (!mpa_encode_init(rec->src->spr, rec->src->nchannel, rec->src->bitrate)) {
            return false;
        }
    }
#endif
#if REC_SBC_SUPPORT
    if (rec->src->rec_type == REC_SBC) {
        if (!sbc_encode_init(rec->src->spr, rec->src->nchannel & 0x03)) {
            return false;
        }
    }
#endif

    memset(&rec_enc_stm, 0, sizeof(rec_enc_stm));                       //output coded data buffer init
    memset(&rec_pcm_stm, 0, sizeof(rec_pcm_stm));                       //input pcm data buffer init
    rec->enc = &rec_enc_stm;
    rec->pcm = &rec_pcm_stm;
    rec->enc->buf = rec->enc->rptr = rec->enc->wptr = rec_encbuf;
    rec->pcm->buf = rec->pcm->rptr = rec->pcm->wptr = rec_obuf;
    rec->enc->size = REC_ENC_SIZE;
    rec->pcm->size = REC_OBUF_SIZE;
    if (rec->src && rec->src->source_start != NULL) {
        rec->src->source_start();
    }
    rec->tick = tick_get();
    rec->sta = REC_RECORDING;

    // 压缩ops算法，降噪算法初始化，与通话相关
    bsp_record_noise_alg_init();
    rec->start_time = RTCCNT;

    return true;
}

void bsp_record_pause(void)
{
    printf("%s\n", __func__);
    rec_cb_t *rec = &rec_cb;
    if (rec->sta != REC_RECORDING) {
        return;
    }
    printf("record pause\n");
    if (rec->src) {
        rec->src->source_stop();
    }
#if REC_MP3_SUPPORT
    if (rec->src->rec_type == REC_MP3) {
        mpa_encode_exit();
    }
#endif
#if REC_SBC_SUPPORT
    if (rec->src->rec_type == REC_SBC) {
        sbc_encode_exit();
    }
#endif
//    rec->src = 0;
    rec->sta = REC_PAUSE;
    rec->flag_play = 1;

#if FUNC_REC_TO_SD
    if (dev_is_online(DEV_SDCARD)) {
        printf("sd0 stop 1!\n");
        sd0_stop(1);
    }
#endif // FUNC_REC_TO_SD

    // 降噪算法初始化，与通话相关
    if (rec->mode == false) {
        bsp_record_noise_alg_exit();
    }

    rec->time += RTCCNT - rec->start_time;
}

AT(.text.func.record)
void bsp_record_continue(void)
{
    printf("%s\n", __func__);
    if (rec_cb.sta == REC_PAUSE) {
        printf("record continue\n");
        bsp_record_start(false, rec_src.rec_type, FUNC_REC_SPR, FUNC_REC_BITRATE);
    }
}


void bsp_record_var_init(void)
{
    memset(&rec_cb, 0, sizeof(rec_cb));
    memset(&rec_src, 0, sizeof(rec_src));
    rec_cb.src = &rec_src;
    rec_cb.first_flag = 1;
//    rec_cb.mode = mode;
//    printf("[%s] rec_cb.mode: %d\n", __func__, rec_cb.mode);
}

static bool save_fade_sta = 0;
void bsp_record_enter(void)
{

    save_fade_sta = dac_get_fade_sta();
    printf("save_fade_sta:%d\n", save_fade_sta);

#if !FUNC_REC_OPUS_BT_MUSIC_EN
#if BT_BACKSTAGE_MUSIC_EN
    bt_audio_bypass();
#endif
#endif

    //mic_dnr_init(x, y, z, k);连续超过x次大于y就认为有声，连续超过z次低于k就认为无声
    //vor_en控制录音开关，目前之前大概配置一下，具体灵敏度需要实际调试确定
    extern void mic_dnr_init(u8 v_cnt, u16 v_pow, u8 s_cnt, u16 s_pow);
    mic_dnr_init(FUNC_REC_VOICED_CNT, FUNC_REC_VOICED_POW_VALUE, FUNC_REC_SILENT_CNT, FUNC_REC_SILENT_POW_VALUE);
    printf("[%s] rec_cb.mode: %d\n", __func__, rec_cb.mode);
    if (rec_cb.mode == true) {
        voice_control_mic_rec_start();
        bsp_record_noise_alg_init();
    }
    rec_cb.is_enter = true;
}

void bsp_record_exit(void)
{

#if FUNC_REC_PCM2DAC_OUT
    dac_fade_out();
    dac_fade_wait();
    bsp_change_volume(sys_cb.vol);
    bool mute_bkp = bsp_get_mute_sta();
    if (mute_bkp) {
        bsp_sys_mute();
    }
#endif // FUNC_REC_PCM2DAC_OUT

    if (rec_cb.mode == true) {
        voice_control_mic_rec_stop();
//        if (rec_cb.sta == REC_RECORDING || rec_cb.sta == REC_PAUSE) {
//            bsp_record_stop();
//        }
    }

    bsp_record_stop();
    bsp_record_noise_alg_exit();

//#if !FUNC_REC_OPUS_BT_MUSIC_EN
#if BT_BACKSTAGE_MUSIC_EN
    bt_audio_enable();
#endif
//#endif

    if (save_fade_sta == 1 || bt_is_playing()) {
        dac_fade_in();
    } else {
        dac_fade_wait();
        dac_fade_out();
    }

    bsp_sd_disk_unmount();
    rec_cb.is_enter = false;

    memset(&rec_cb, 0, sizeof(rec_cb));
}

u32 bsp_record_get_rec_sec(void)
{
    if (rec_cb.sta == REC_RECORDING) {
        rec_cb.time += (compo_cb.rtc_cnt - rec_cb.start_time);
        rec_cb.start_time = compo_cb.rtc_cnt;
        return rec_cb.time;
    } else {
        return 0;
    }
}

void bsp_record_set_mode(bool mode)
{
    rec_cb.mode = mode;
    if (rec_cb.is_enter) {
        if (rec_cb.mode == true) {
            if (rec_cb.is_rec_path_start == false) {
                voice_control_mic_rec_start();
            }
            bsp_record_noise_alg_init();
        } else {
            if (rec_cb.sta != REC_RECORDING) {
                bsp_record_noise_alg_exit();
            }
        }
    }
    printf("[%s] rec_cb.mode: %d\n", __func__, rec_cb.mode);
}

bool bsp_record_get_mode(void)
{
    return rec_cb.mode;
}

#endif // FUNC_REC_EN



//#if FUNC_REC_EX_TEST
//
//u8 rec_test_buf[512];
//
//void rec_file_test(void)
//{
//
//    if (dev_is_online(DEV_SDCARD)) {
//        sys_cb.cur_dev = DEV_SDCARD;
//    } else if (dev_is_online(DEV_UDISK)) {
//        sys_cb.cur_dev = DEV_UDISK;
//    }
//    bsp_sd_disk_mount(24);
//
//    //创建录音文件夹
//    if (!rec_file_creat(REC_MP3) {
//        printf("rec_file_creat error\n");
//        return;
//    }
//
//    u8 cest_data = 0x55;
//    for (int i=0; i<10; i++) {
//        memset(rec_test_buf, cest_data, 512);
//
//        if (rec_file_write(rec_test_buf, 512, REC_MP3) != true) {
//            printf("write fail\n");
//            while(1) WDT_CLR();
//        }
//
//        record_file_sync(REC_MP3);
//        cest_data++;
//    }
//
//    rec_file_close(REC_MP3);
//
//    if (dev_is_online(DEV_SDCARD)) {
//        bsp_sd_disk_unmount();
//    }
//}
//
//
//#endif // FUNC_REC_EX_TEST



