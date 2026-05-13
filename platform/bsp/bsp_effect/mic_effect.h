#ifndef __MIC_EFFECT_H
#define __MIC_EFFECT_H

enum {
    LOC_MIC_PACC_EQ_CS          = 0,
    LOC_MIC_PACC_DRC_CS,

    LOC_MIC_PACC_MAX_CS,
};

enum {
    SCO_PACC_EQ_CS             = 0,
    SCO_PACC_DRC_CS,

    SCO_PACC_MAX_CS,
};

void loc_mic_pacc_init(void);
void loc_mic_pacc_set_param(void);
void loc_mic_pacc_enable(void);
void loc_mic_pacc_process(u8 *obuf, u8 *ibuf, u32 samples);
void loc_mic_pacc_exit(void);

void sco_pacc_init(void);
void sco_pacc_set_param(bool is_msbc);
void sco_pacc_enable(void);
void sco_pacc_process(u8 *obuf, u8 *ibuf, u32 samples);
void sco_pacc_exit(void);

bool mic_sco_pacc_init(bool is_msbc);
#define mic_sco_pacc_exit()     sco_pacc_exit()
#endif
