/**********************************************************************
*
*   strong_sco.c
*   定义库里面通话算法部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"

/*****************************************************************************
 * Module    : AEC算法强定义
 *****************************************************************************/
#if !BT_AEC_EN
AT(.com_text.isr.aec)
void aec_isr(void) {}
AT(.bt_voice.aec)
void aec_nlms_process(u8 mic_sel) {}
AT(.bt_voice.aec)
void aec_nlp_process(void) {}
void aec_init(void) {}
#endif

#if !BT_ALC_EN
void alc_init(void) {}
void alc_fade_in(s16 *buf) {}
void alc_fade_out(s16 *buf) {}
AT(.bt_voice.alc)
void bt_alc_process(u8 *ptr, u32 samples, u32 pcm_mode) {};
#endif

#if !BT_SCO_AINS4_EN
void bt_ains4_init(void *alg_cb) {}
void bt_ains4_exit(void) {}
s32 bt_ains4_nr_process(s32 *efw0, s32 *efw1, s32 *xfw) {return 0;}
void ains4_sm_process(void) {}
#endif

#if !BT_SCO_DNN_EN
void bt_dnn_init(void *alg_cb) {}
void bt_dnn_exit(void) {}
s32 bt_dnn_nr_process(s32 *efw0, s32 *efw1, s32 *xfw) {return 0;}
void dnn_sm_process(void) {}
#endif

#if !BT_SCO_DMIC_AI_EN
void bt_dmdnn_init(void *alg_cb) {}
void bt_dmdnn_exit(void) {}
s32 bt_dmdnn_nr_process(s32 *efw0, s32 *efw1, s32 *xfw) {return 0;}
void dmdnn_sm_process(void) {}
#endif

#if !BT_SCO_AIAEC_DNN_EN
void bt_aiaec_dnn_init(void *alg_cb) {}
void bt_aiaec_dnn_exit(void) {}
s32 bt_aiaec_process(s32 *efw0, s32 *efw1, s32 *xfw) {return 0;}
void aiaec_sm_process(void) {}
#endif

#if !BT_SCO_DMIC_AIAEC_EN
void bt_dmdnn_aiaec_init(void *alg_cb) {}
void dmdnn_aiaec_process(void) {}
void bt_dmdnn_aiaec_exit(void) {}
#endif

#if !BT_SCO_DNN_EN && !BT_SCO_AIAEC_DNN_EN && !BT_SCO_DMIC_AI_EN && !BT_SCO_DMIC_AIAEC_EN
AT(.com_text.bt_sco)
bool bt_sco_dnn_is_en(void) {return false;}
void dnn_far_upsample(s16 *out, s16 *in, u32 samples, u8 step) {}
u32 dnn_near_downsample(s16 *ptr, u32 samples) {return 0;}
#endif

