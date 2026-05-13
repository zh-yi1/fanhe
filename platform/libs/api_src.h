#ifndef _API_SRC_H
#define _API_SRC_H

//hardware peripheral src
//硬件支持两路SRC, 支持立体声24bit, 优先使用硬件SRC

typedef void (*psrc_callback_t)(u8 *ptr, u32 samples, u32 nch, bool is24b, void *param);

void psrc0_init(u32 spr_in, u32 spr_out, u32 channel);
void psrc0_stop(void);
void psrc0_adjust_speed(int speed);     //speed为有符号数, 调速精度: speed/(sample_rate_in/sample_rate_out*1920*2^12)
int psrc0_audio_input(u8 *buf, u32 in_samples, bool is24b);
void psrc0_audio_output_callback_set(psrc_callback_t callback);

void psrc1_init(u32 spr_in, u32 spr_out, u32 channel);
void psrc1_stop(void);
void psrc1_adjust_speed(int speed);     //speed为有符号数, 调速精度: speed/(sample_rate_in/sample_rate_out*1920*2^12)
int psrc1_audio_input(u8 *buf, u32 in_samples, bool is24b);
void psrc1_audio_output_callback_set(psrc_callback_t callback);

void psrc_var_init(void);


//software src, 支持两路单声道SRC, 支持单声道。也可合并为一路双声道处理
void src_init(u32 ch_index, int spr_in, int spr_out);
void src_phase_comp_set(u32 ch_index, int phase);
int src_frame_resample(u32 ch_index, short *src_in, short *src_out, int in_cnt, u32 max_nch);       //mono: max_nch = 1, stereo: max_nch = 2
int src_frame_resample_24bit(u32 ch_index, int *src_in, int *src_out, int in_cnt, u32 max_nch);     //mono: max_nch = 1, stereo: max_nch = 2

#endif // _API_SRC_H
