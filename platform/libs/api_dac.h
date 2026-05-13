#ifndef _API_DAC_H_
#define _API_DAC_H_

#define DAC_OBUF_SIZE       576
#define DAC_OBUF_EXSIZE     512

#define PCM_OUT_24BITS              BIT(0)
#define PCM_OUT_MONO                BIT(1)

extern uint16_t cfg_pcm_out_format;
extern uint8_t cfg_dac_out_spr;
extern uint8_t cfg_spk_mute_en;

void dac_cb_init(u32 dac_cfg);
void dac_set_volume(u8 vol);
void dac_set_dvol(u16 vol);
void dac_fade_process(void);
void dac_aubuf_clr(void);
void dac_fade_out(void);
void dac_fade_in(void);
void dac_fade_wait(void);
void dac_set_fade_flag(u8 flag);
bool dac_get_fade_sta(void);
void dac_analog_fade_in(void);
void dac_spr_set(uint spr);
void dac_aubuf_init(void);
void dac_src1_init(void);
void dac_power_on(void);
void dac_restart(void);
void dac_power_off(void);
void dac_set_mono(void);
void dac_mono_init(bool dual_en, bool lr_sel);
void dac_put_sample_16bit(s16 left, s16 right);
void dac_put_sample_24bit(s32 left, s32 right);
void dac1_put_sample_16bit(s16 left, s16 right);
void dac1_put_sample_24bit(s32 left, s32 right);
void adpll_init(u8 out_spr);
bool adpll_spr_set(u8 out48k_flag);
void dac_unmute_set_delay(u16 delay);
void dac_dnr_process(void);
u16 dac_pcm_pow_get(void);                      //20ms间隔自动计算一次, 可随时调用
void music_src_set_volume(u16 vol);
void dac_channel_configure(void);
void dac_channel_exchange(void);                //DAC左右声道交换
void dac_set_power_on_off(u32 status);          //dac模拟开关控制。status: 1 -> dac analog power on, 0 -> dac analog power off
u8 dac_get_power_status(void);                  //获取dac当前状态
void dac_set_balance(u16 l_vol, u16 r_vol);

void aubuf0_dma_kick(void *ptr, u32 samples, u32 nch, bool is_24bit);
void aubuf0_dma_init(u32 isr_en);
void aubuf0_dma_w4_done(void);

void aubuf1_dma_kick(void *ptr, u32 samples, u32 nch, bool is_24bit);
void aubuf1_dma_init(u32 isr_en);
void aubuf1_dma_w4_done(void);

void au0_dma_start(void);
void au0_dma_stop(void);
void au0_dma_set_callback(void (*out_cb)(u8 *buf, u32 samples, bool is_24bit));

bool music_set_eq_by_res(u32 addr, u32 len);
void music_set_eq_by_num(u8 num);
void music_eq_off(void);
bool music_set_eq(u8 band_cnt, const u32 *eq_param);
bool music_set_eq_is_done(void);    //判断上一次设置EQ是否完成，1：已完成
void music_drc_set_param(u32 ch, u32 band_cnt, const u32 *tbl);
void music_set_drc(u8 band_cnt, const u32 *drc_param);
bool music_set_drc_by_res(u32 addr, u32 len);
void music_set_drc_by_online(u8 band_cnt, const u32 *drc_param);
void music_drc_off(void);

void bass_treble_coef_cal(void *eq_coef, int gain, int mode);   //gain:-12dB~12dB, mode:0(bass), 1(treble)
void eq_coef_cal(void *eq_coef, int gain);           //index:0~7（8条EQ）, gain:-12dB~12dB
#endif
