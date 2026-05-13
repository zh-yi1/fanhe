#include "include.h"

#if (ASR_SELECT == ASR_YJ)

#if ET_WAKEUP_EN

#include "bsp_et_asr_ctrl.h"
#include <lib_et_asr.h>

#include "et_keyword.h"
#include "et_thres_list.h"
#if ET_ASR_KWS_LIST_BIN_EN
#include "et_kws_labels.h"
#endif
#if ET_COLLECT_BYTES_BIN_EN
#include "et_collects.h"
#endif

#if THIRD_UI_LIGHT_EN
#include "third_kws_event.h"
#include "third_func.h"

#endif // THIRD_UI_LIGHT_EN

#include "third_bsp_sys.h"

void et_func_kws_event(int event)
{
    #if 1
    if(event <=0 || event >=255) {
        return;
    }
    #else
    if(event ==0) {
        return;
    }
    #endif // 0
    #if ET_LOG_OPEN
    printf("ctrl event:%d\n", event);
    #endif // ET_LOG_OPEN

    #if THIRD_UI_LIGHT_EN
    //third_kws_event(event);
    set_et_kws_event(event);
    msg_enqueue(EVT_ET_FUNC_KWS_DEAL);
    #endif // THIRD_UI_LIGHT_EN
}

u32 et_asr_get_time_cnt()
{
    return tick_get();
}
#if 1
void bsp_et_asr_fft_user_init(void)
{
}

void bsp_et_asr_fft_forward(s32 *buf)
{
	extern void speaker_process_512(s32 *data);
	speaker_process_512(buf);
}

void bsp_et_asr_fft_inverse(s32* buf)
{
}
#endif // 0

ALIGNED(4) et_asr_kws_cfg_t et_kws_cfg  AT(.asr.save);
#define ET_SDK_BASE_VERSION "202408271131"

void et_bsp_ctrl_main_init()
{
    printf("-et_bsp_ctrl_main_init(),start-\n");
    //printf("len:%d\n", RES_LEN_WEIGHTS_PARM_BIN);
    printf("base[v:%s]\n", ET_SDK_BASE_VERSION);
    printf("ui[name:%s, v:%s]\n", ET_UI_PROJ_NAME, ET_UI_VERSION);
    memset(&et_kws_cfg,0,sizeof(et_asr_kws_cfg_t));
    //extern int get_vad_prefetch_num();
    et_kws_cfg.keyFile = ET_ASR_KEYWORD_BIG_STR;
    et_kws_cfg.keyLen = strlen(ET_ASR_KEYWORD_BIG_STR);
    et_kws_cfg.thresListFile = (char*)ET_ASR_THRES_LIST_BIG_STR;
    et_kws_cfg.thresListLen = strlen(ET_ASR_THRES_LIST_BIG_STR);
    #if ET_ASR_KWS_LIST_BIN_EN
    et_kws_cfg.kwsLabelsFile = (char*)et_asr_kws_labels_buf;
    et_kws_cfg.kwsLabelsLen = ET_ASR_KWS_LABELS_BUF_LEN;
    #endif // ET_ASR_KWS_LIST_BIN_EN
    #if ET_COLLECT_BYTES_BIN_EN
    et_kws_cfg.collectFile = (char*)et_asr_collects_buf;
    et_kws_cfg.collectLen = ET_ASR_COLLECTS_BUF_LEN;
    #endif // ET_COLLECT_BYTES_BIN_EN


    et_kws_cfg.weightFile = (char*)UI_BUF_ASR_WEIGHTS_BIN;
    et_kws_cfg.weightLen = UI_LEN_ASR_WEIGHTS_BIN;

    #if ET_USED_USBKEY
    et_kws_cfg.c_soft_key = (char*)xcfg_cb.asr_soft_key;
    #else
    et_kws_cfg.c_soft_key = NULL;
    #endif // ET_USED_USBKEY

    et_kws_cfg.time_cb = et_asr_get_time_cnt;

	#if ET_ASR_NEED_FFT_TYPE_EN
    et_kws_cfg.fft_init_cb = bsp_et_asr_fft_user_init;
    et_kws_cfg.forward_cb = bsp_et_asr_fft_forward;
    et_kws_cfg.inverse_cb = bsp_et_asr_fft_inverse;
	#endif // ET_ASR_NEED_FFT_TYPE_EN

	int ret = et_asr_wakeup_first_init(&et_kws_cfg, et_func_kws_event);
	printf("init ret:%d\n", ret);
	// if(ret == 0) {
    //     sys_cb.asr_en = 1;
	// }
	// else {
    //     sys_cb.asr_en = 0;
	// }
    printf("-et_bsp_ctrl_main_init(),end-\n");
}

void et_bsp_ctrl_exit()
{
    printf("-et_bsp_ctrl_exit(),start-\n");
    // sys_cb.asr_en = 0;
    et_stop_asr();
    et_asr_wakeup_exit();
    et_close_asr_record();
    printf("-et_bsp_ctrl_exit(),end-\n");
}

void et_init(void)
{
#if ET_WAKEUP_EN
    et_bsp_ctrl_main_init();
    #if ET_ASR_VAD_EN
        et_vad_init(VAD_POS_CNT_LIMT, VAD_NEG_CNT_LIMT, VAD_KWS_WORD_LIMT);
    #endif // ET_ASR_VAD_EN
    third_light_init();
    third_after_bsp_sys_init();
#endif // ET_WAKEUP_EN
}
#endif // ET_WAKEUP_EN

#if ET_ASR_VAD_EN

void et_asr_vad_callback(int16_t *ptr, int32_t samples, uint8_t step)
{
    et_asr_wakeup_buf_write((int16_t*)ptr, samples, step);
}

void et_asr_wordstart()
{
    sys_clk_set(SYS_192M);
    printf("word start\n");

    //PWM need reinit
#if HUART_DUMP_EN
    //need reinit
    //udet_init(1500000);
#endif // HUART_DUMP_EN
#if USER_UART0_EN
    //udet_init(115200);
#endif // USER_UART0_EN
}

void et_asr_wordend()
{
    printf("word end\n");
    #if 0
    if(!is_et_frame_refreshing()) {
        sys_clk_set(SYS_88M);
    }
    #endif // 0
    //PWM need reinit
#if HUART_DUMP_EN
    //need reinit
    //udet_init(1500000);
#endif // HUART_DUMP_EN
#if USER_UART0_EN
    //udet_init(115200);
#endif // USER_UART0_EN
}

#endif // ET_ASR_VAD_EN

#endif //ASR_SELECT
