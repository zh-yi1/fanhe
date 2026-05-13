#ifndef _LIB_ET_ASR_H__
#define _LIB_ET_ASR_H__

#if (ASR_SELECT == ASR_YJ)

#include "include.h"
#define ET_VAD_ONCE_BUF_SIZE    (ASR_SAMPLE >> 1) //
#define ET_VAD_ONCE_BUF_NUM    6 //
#define ET_ASR_KWS_LIST_BIN_EN    1 //kws_labels.bin
#define ET_COLLECT_BYTES_BIN_EN    0 //z_et_collects.bin;
#define ET_CTC2_BIN_EN    0 //recheck_idx.bin;

typedef void (*et_asr_wakeup_event_cb_t)(int event);
typedef uint32_t (*et_asr_time_cb_t)();

typedef void (*et_asr_vad_cb_t)(int16_t *ptr, int32_t length, uint8_t step);
typedef void (*et_vad_wordstart_cb_t)();
typedef void (*et_vad_wordend_cb_t)();

typedef struct {
    char* keyFile;
    int keyLen;
    char* weightFile;
    int weightLen;
    char* parmFile;
    int parmLen;
    char* thresListFile;
    int thresListLen;
    char* recheckIdxFile;
    int recheckIdxLen;
    char* c_soft_key;
    #if ET_ASR_KWS_LIST_BIN_EN
    char* kwsLabelsFile;
    int kwsLabelsLen;
    #endif // ET_ASR_KWS_LIST_BIN_EN

    char* collectFile;
    int collectLen;

    et_asr_time_cb_t time_cb;

} et_asr_kws_cfg_t;


uint32_t get_et_asr_time_cnt();

int et_asr_wakeup_first_init(et_asr_kws_cfg_t* cfg, et_asr_wakeup_event_cb_t cb);

int et_asr_wakeup_init();
void et_asr_wakeup_exit();

int et_asr_wakeup_buf_write(int16_t *ptr, uint32_t samples, uint8_t step);
int et_asr_buf_write(int16_t *ptr, uint32_t samples, uint8_t step);
void et_print_fbank_begin();
int get_et_fbank_begin();
void et_reset_asr_state();
void et_start_asr();
void et_stop_asr();

uint8_t is_et_asr_work();
uint8_t is_et_asr_parm_suc();
uint8_t is_et_asr_initing();
uint8_t is_et_asr_exitting();
uint8_t et_search_kws_list_bin(int row, int col);

et_asr_kws_cfg_t* get_et_asr_kws_cfg_p();

void et_vad_init(int pos_cnt_limit, int neg_cnt_limit, int kws_word_limit);
void et_bsp_vad_detect(int16_t *ptr, int length, int vad_unit_len,
                    et_asr_vad_cb_t asr_vad_cb, et_vad_wordstart_cb_t wordstart_cb, et_vad_wordend_cb_t wordend_cb);
void set_et_vad_need_hold(uint8_t flag);
void set_et_noise_env(uint8_t flag);
uint8_t get_et_noise_env();
uint8_t is_et_vad_wake(void);

int et_asr_data_process(int16_t *ptr, uint32_t samples, uint8_t step);
int et_asr_check_one_loop();
void set_et_frame_refreshing(uint8_t flag);
uint8_t is_et_frame_refreshing();
void set_et_play_tip_mode(uint8_t flag);
uint8_t is_et_play_tip_mode();

void et_open_mem_pools_init(); //20KB availabel
void et_open_mem_pools_destroy();
void* et_open_mem_allocate(int size);
void* et_open_mem_zllocate(int size);
void et_open_mem_free(void* ptr);

#if ET_ASR_KWS_LIST_BIN_EN
uint8_t* get_et_kws_labels(int row, uint8_t* labels);
#endif // ET_ASR_KWS_LIST_BIN_EN

#endif //ASR_SELECT

#endif
