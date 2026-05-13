#ifndef _API_SDADC_H
#define _API_SDADC_H

#define CHANNEL_L           0x00ff
#define CHANNEL_R           0xff00

#define ADC2DAC_EN          0x01        //ADC-->DAC
#define ADC2SRC_EN          0x02        //ADC-->SRC
#define ADC2IRQ_EN          0x03        //ADC-->IRQ（测试用）
#define ADC2DIR_EN          0x04        //ADC-->DAC（测试用）
#define ADC2SANC_EN         0x05        //ADC-->SANC
#define ADC2ASR_EN          0x06        //ADC-->ASR
#define ADC_ALIGN_EN        0x07        //ADC DMA ALIGN

enum {
    PCM_CHMASK      = 0x000f,
    PCM_STEREO      = BIT(6),
    PCM_24BIT       = BIT(7),
    PCM_DMACH       = 0x0f00,
    //max 16bit
};

typedef void (*pcm_callback_t)(u8 *ptr, u32 samples, u32 pcm_mode);

typedef struct {
    u16 channel;
    u8 sample_rate;
    u32 anl_gain;
    u32 dig_gain;
    u8 bits_mode;                       //ADC BITS选择；0: 24bits, 1: 16bits, 0xff: 跟随DAC的BIT MODE
    u8 out_ctrl;
    u16 samples;
    pcm_callback_t callback;
} sdadc_cfg_t;

enum {
    SPR_48000,
    SPR_44100,
    SPR_38000,
    SPR_32000,
    SPR_24000,
    SPR_22050,
    SPR_16000,
    SPR_12000,
    SPR_11025,
    SPR_8000,
    SPR_6000,
    SPR_4000,

    SPR_96000,
    SPR_88200,
    SPR_76000,
    SPR_64000,
    SPR_384000,
    SPR_352800,
    SPR_192000,
    SPR_176400,
};

void sdadc_dummy(u8 *ptr, u32 samples, u32 pcm_mode);
void sdadc_pcm_2_dac(u8 *ptr, u32 samples, u32 pcm_mode);
void sdadc_pcm_2_src1(u8 *ptr, u32 samples, u32 pcm_mode);
void sdadc_var_init(void);

int sdadc_init(const sdadc_cfg_t *p_cfg);
int sdadc_start(u16 channel);
int sdadc_exit(u16 channel);

#endif //_API_SDADC_H

