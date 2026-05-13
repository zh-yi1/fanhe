#ifndef __BSP_ASR_H
#define __BSP_ASR_H

void bsp_speech_recognition_enter(void);
void bsp_speech_recognition_process(void);
void bsp_asr_init(void);
void bsp_asr_start(void);
void bsp_asr_stop(void);
void bsp_asr_pause(void);
u8 bsp_asr_init_sta(void);
void thread_asr_create(void);
void bsp_asr_vddon(void);
void bsp_asr_tone_play(u32 addr, u32 len);
void bsp_asr_func_switch(u8 func);
u8 bsp_asr_voice_wake_sta_get(void);
void bsp_asr_voice_wake_sta_clr(void);
void bt_siri_and_asr_parallel_en_setter(bool is_en);
bool bt_siri_and_asr_parallel_en(void);
void bt_siri_asr_auto_close(void);
void bt_siri_asr_time_reset(void);
void bt_siri_and_asr_open(void);
void bt_siri_and_asr_off(void);
void bsp_asr_process(void);

/* 华镇 相关函数*/
void asr_alg_start(void);
void asr_alg_stop(void);
u32 asr_alg_process(short *ptr);
void ws_asr_usbkey_check(void);
void asr_event_process();
void ws_wake_clr(void);
u8 ws_wake_get(void);

/* 友杰 相关函数*/
void et_init(void);
void et_start_asr();
void et_stop_asr();
void set_et_frame_refreshing(uint8_t flag);
int third_func_message(u16 msg);
int third_func_process(void);
u8 et_wake_get(void);
void et_wake_clr(void);
void et_asr_vad_callback(int16_t *ptr, int32_t samples, uint8_t step);
void et_asr_wordstart();
void et_asr_wordend();

/* 华镇空调伴侣 相关函数 */
u8 ws_air_wake_get(void);
void ws_air_wake_clr(void);
void ir_pwm_io_init();
void air_data_init();
void mp3_end_handle();
void isd_exit_search_remote(void);
#endif
