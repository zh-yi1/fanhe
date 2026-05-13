#include "include.h"

void bt_aec_process(u8 *ptr, u32 samples, u32 pcm_mode);
void bt_adc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void bt_alc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void speaker_sdadc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void usbmic_sdadc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void recorder_sdadc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void mic_test_sdadc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void bsp_speech_recognition_sdadc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void opus_sdadc_process(u8 *ptr, u32 samples, u32 ch_mode);
u8 bsp_modem_get_spr(void);

#if VIDEO_RECODE_TAKE_PHOTO_EN
void video_recode_sdadc_proecess(u8 *ptr, u32 samples, u32 pcm_mode);
#endif // VIDEO_RECODE_TAKE_PHOTO_EN

#if UDE_MIC_EN
    #define usbmic_sdadc_callback   usbmic_sdadc_process
#else
    #define usbmic_sdadc_callback   sdadc_dummy
#endif // UDE_MIC_EN

#if BT_AEC_EN
    #define bt_sdadc_callback    bt_aec_process
#elif BT_ALC_EN
    #define bt_sdadc_callback    bt_alc_process
#else
    #define bt_sdadc_callback    bt_adc_process
#endif

#if FUNC_RECORDER_EN
    #define recorder_sdadc_callback     recorder_sdadc_process
#else
    #define recorder_sdadc_callback     sdadc_dummy
#endif

#if OPUS_ENC_EN
    #define opus_sdadc_callback   opus_sdadc_process
#else
    #define opus_sdadc_callback   sdadc_dummy
#endif

#if VIDEO_RECODE_TAKE_PHOTO_EN
#define video_recode_sdadc_callback     video_recode_sdadc_proecess
#else
#define video_recode_sdadc_callback     sdadc_dummy
#endif // VIDEO_RECODE_TAKE_PHOTO_EN

#if ASR_SELECT
    #define asr_sdadc_callback     bsp_speech_recognition_sdadc_process
#else
    #define asr_sdadc_callback     sdadc_dummy
#endif

//MIC analog gain配置3固定为12DB
//MIC数字增益0~36DB
const sdadc_cfg_t rec_cfg_tbl[] = {
/*   通道,       采样率,     模拟增益,  数字增益   BITS_MODE,    通路控制,    样点数,      回调函数*/
    {CH_MIC0,   SPR_16000,       3,          0,        1,        ADC2DAC_EN,     240,    sdadc_dummy},                   /* SPEAKER */
    {CH_MIC0,   SPR_8000,        3,          0,        1,        ADC2DAC_EN,     480,    bt_sdadc_callback},             /* BTMIC   */
    {CH_MIC0,   UDE_MIC_SPR,     3,          0, !UDE_MIC_24BIT,  ADC2DAC_EN,     256,    usbmic_sdadc_callback},         /* USB MIC */
    {CH_MIC0,   FUNC_REC_SPR,    3,          0,        1,        ADC2DAC_EN,     256,    recorder_sdadc_callback},       /* RECORDER*/
    {CH_MIC0,   SPR_16000,       3,          0,        1,        ADC2DAC_EN,     480,    bt_sdadc_callback},             /* IIS     */
    {CH_MIC0,   SPR_16000,       3,          0,        1,        ADC2DAC_EN,     128,    opus_sdadc_callback},           /* opus    */
    {CH_MIC0,   SPR_16000,       3,          0,        1,        ADC2DAC_EN,     256,    video_recode_sdadc_callback},   /* VIDEO REC*/
    {CH_MIC0,   SPR_16000,       3,      ASR_GAIN,     1,        ADC2DAC_EN,   ASR_SAMPLE,  asr_sdadc_callback},         /* ASR     */
};

/*****************************************************************************
 * 功能   : 根据Settings，获取mic通路
 * 输入   : audio_path_idx,用于判断是否为BT_MIC_PATH
 * 注意   : 若mic通路选取异常，会报err;BT_MIC_PATH可选单、双麦,其他模式只采用单麦即主麦
 * 返回   : 无
 *****************************************************************************/
static u16 bsp_mic_ch_getcfg(u8 audio_path)
{
    u8 mic_cnt = 0;
    u16 ch_sel = 0;
    u16 mic_list[2] = {xcfg_cb.bt_mmic_cfg, xcfg_cb.bt_smic_cfg};
    u8 mic_mapping_tbl[2] = {CH_MIC0, CH_MIC1};

    mic_cnt = MIC_DEFAULT_NCH;

#if BT_SCO_DMIC_EN
    if (audio_path == AUDIO_PATH_BTMIC) {
        mic_cnt = 2;
    }
#endif

    if (mic_cnt == 1) {
        printf("MMIC --> %d\n", mic_list[0]);
    } else {
        printf("MMIC --> %d, SMIC --> %d\n", mic_list[0], mic_list[1]);
    }

    for (u8 i = 0; i < mic_cnt; i++) {
        if ((mic_list[i] + 1) > MIC1) {
            continue;
        }
        ch_sel |= mic_mapping_tbl[mic_list[i]] << (8 * i);
    }
    return ch_sel;
}

void audio_path_init(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));
    sys_cb.audio_path = path_idx;
    if (path_idx == AUDIO_PATH_BTMIC) {
#if BT_AEC_EN
        if (xcfg_cb.bt_aec_en) {
            cfg.callback = bt_aec_process;
        } else
#endif
#if BT_ALC_EN
        if (xcfg_cb.bt_alc_en) {
            cfg.callback = bt_alc_process;
        } else
#endif
        {
            cfg.callback = bt_adc_process;
        }
        if (bt_sco_is_msbc() || bt_sco_dnn_is_en()) {               //如果开了msbc或dnn，则采样率设为16k
            cfg.sample_rate = SPR_16000;
        }
        cfg.channel = bsp_mic_ch_getcfg(path_idx);
        cfg.dig_gain = ((xcfg_cb.bt_mic0_dig_gain) |
                        (xcfg_cb.bt_mic1_dig_gain<<6));
        cfg.anl_gain = BT_ANL_GAIN | (BT_ANL_GAIN << 6);
    } else if (path_idx == AUDIO_PATH_USBMIC) {
        if (UDE_MIC_NCH == 2) {
            cfg.channel = CH_MIC0 | (CH_MIC1 << 8);
        }
        cfg.dig_gain = ((xcfg_cb.bt_mic0_dig_gain) |
                        (xcfg_cb.bt_mic1_dig_gain<<6));
        cfg.anl_gain = BT_ANL_GAIN | (BT_ANL_GAIN << 6);
    } else if (path_idx == AUDIO_PATH_MODEMMIC) {
#if MODEM_CAT1_EN
        cfg.sample_rate = bsp_modem_get_spr();
#endif
        cfg.dig_gain = ((xcfg_cb.bt_mic0_dig_gain) |
                        (xcfg_cb.bt_mic1_dig_gain<<6));
        cfg.anl_gain = BT_ANL_GAIN | (BT_ANL_GAIN << 6);
    } else if (path_idx == AUDIO_PATH_RECORDER) {

#if FUNC_REC_NR_EN
//        u16 opus_sample_size_cal(u32 spr);
        cfg.samples = 120;//opus_sample_size_cal(FUNC_REC_SPR) / FUNC_REC_NCH;
        printf("----------> audio samples:%d\n", cfg.samples);
#else
    #if FUNC_REC_OPUS_EN
        u16 opus_sample_size_cal(u32 spr);
        cfg.samples = opus_sample_size_cal(FUNC_REC_SPR) / FUNC_REC_NCH;
        printf("----------> audio samples:%d\n", cfg.samples);
    #endif // FUNC_REC_OPUS_EN
#endif // FUNC_REC_NR_EN
        cfg.sample_rate = FUNC_REC_SPR;
        cfg.channel = bsp_mic_ch_getcfg(path_idx);
        cfg.dig_gain = ((xcfg_cb.bt_mic0_dig_gain) |
                        (xcfg_cb.bt_mic1_dig_gain<<6));
        cfg.anl_gain = BT_ANL_GAIN | (BT_ANL_GAIN << 6);
    } else {
        cfg.channel = bsp_mic_ch_getcfg(path_idx);
        cfg.dig_gain = ((xcfg_cb.bt_mic0_dig_gain) |
                        (xcfg_cb.bt_mic1_dig_gain<<6));
        cfg.anl_gain = BT_ANL_GAIN | (BT_ANL_GAIN << 6);
    }
#if FPGA_EN
    bsp_auphy_mic_setup(cfg.channel);
    bsp_auphy_set_mic_analog_gain(cfg.channel, 8);
#endif
    sdadc_init(&cfg);
    if (path_idx == AUDIO_PATH_BTMIC) {
        if (!bt_sco_is_msbc() && bt_sco_dnn_is_en()) {              //dnn: 走窄带通话时，ADC为16K，DAC为8K
            dac_spr_set(SPR_8000);
        }
    }
}

void audio_path_start(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));
    if (path_idx == AUDIO_PATH_BTMIC) {
        cfg.channel = bsp_mic_ch_getcfg(path_idx);
    } else if (path_idx == AUDIO_PATH_USBMIC) {
        if (UDE_MIC_NCH == 2) {
            cfg.channel = CH_MIC0 | (CH_MIC1 << 8);
        }
    } else {
        cfg.channel = bsp_mic_ch_getcfg(path_idx);
    }
    sdadc_start(cfg.channel);
    sys_cb.audio_path = path_idx;
}

void audio_path_exit(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));
    if (path_idx == AUDIO_PATH_BTMIC) {
        cfg.channel = bsp_mic_ch_getcfg(path_idx);
    } else if (path_idx == AUDIO_PATH_USBMIC) {
        if (UDE_MIC_NCH == 2) {
            cfg.channel = CH_MIC0 | (CH_MIC1 << 8);
        }
    } else {
        cfg.channel = bsp_mic_ch_getcfg(path_idx);
    }
    sdadc_exit(cfg.channel);
    adpll_spr_set(DAC_OUT_SPR);
    sys_cb.audio_path = path_idx;
}

//使能mic
void mic_mute(bool en)
{
    sdadc_analog_mic_mute(en);
}


