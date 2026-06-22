// stubs_elunchbox.c — 饭盒项目不需要的手表功能桩函数
// 所有函数提供空实现以满足链接器，运行时不会被调用
#include "include.h"

// ============================================================================
// BT 通话音频回调（dev_vol, a2dp_vol）
// ============================================================================
bool dev_vol_set_cb(uint8_t dev_vol, uint8_t media_index, uint8_t setting_type) { return false; }
uint8_t a2dp_vol_adj_cb(uint8_t a2dp_vol, bool up_flag) { return 0; }
uint8_t a2dp_vol_get_init_cb(uint8_t vol_feat) { return 0; }

// ============================================================================
// BT 重拨
// ============================================================================
void bt_redial_init(void) {}
void bt_redial_reset(uint8_t index) {}
void bt_update_redial_number(uint8_t index, char *buf, u32 len) {}
void bt_init_lib(void) {}

// ============================================================================
// PLC (Packet Loss Concealment) — 通话丢包补偿
// ============================================================================
void plc_init_var(void) {}
void plc_init(void *cfg) {}
void plc_exit(void) {}
void plc_isr(void) {}
void plc_process(s16 *buf, u32 samples) {}

// ============================================================================
// 语音/DNN 算法处理（libvoices.a 中移除）
// ============================================================================
void dnn_fre_process(s32 *fft_in) {}
void aiaec_fre_process(void) {}
void dmdnn_fre_process(void) {}
void dmdnn_aiaec_fre_process(void) {}
void ains4_fre_process(void) {}
// ============================================================================
// FFT/IFFT 硬件加速
// ============================================================================
void fft_hw(void *cfg) {}
void ifft_hw(void *cfg) {}

// ============================================================================
// 数学协处理器中断
// ============================================================================
void math_isr(void) {}

// ============================================================================
// 万花筒表盘组件 (widget_icon_t *compo_kale_select_byidx(compo_kaleidoscope_t *kale, int idx))
// ============================================================================
#include "components/compo_kaleidoscope.h"
widget_icon_t *compo_kale_select_byidx(compo_kaleidoscope_t *kale, int idx) { return NULL; }

// ============================================================================
// 表盘时钟映射
// ============================================================================
u16 func_clock_time_map_r_get(bool is_sec) { return 0; }

// ============================================================================
// 天空菜单首索引
// ============================================================================
u8 func_menu_sub_skyrer_get_first_idx(void) { return 0; }

// ============================================================================
// libcodecs.a 桩函数 — 饭盒不需要音频/视频编解码
// ============================================================================

// MP3 解码
void mp3_dec_init(void *info) {}
int mp3_dec_frame(void *info) { return -1; }
u32 mp3_get_total_time(void *file) { return 0; }
void spi_mp3_dec_init(void) {}

// SBC 解码（BT A2DP）
void sbc_dec_init(void *cfg) {}
void sbc_dec_end(void) {}
int sbc_dec_frame(void *info) { return -1; }

// APE 解码（被 libplatform.a music.o 引用）
void ape_frame_jump_cal(void *f_dec, u32 offset) {}

// AVI / 视频播放
void avi_play_set_times_tsk(void) {}
void avi_recode_stop_tsk(void) {}
void avi_decode_stop_tsk(void) {}
void avi_play_audio_process_tsk(void) {}
void avi_play_video_process_tsk(void) {}
void avi_recode_process_tsk(void) {}
void avi_recode_circ_process(void) {}
void avi_encode_frame_ms(void) {}
void avi_video_frame_msec_tsk(void) {}
void avi_dac_mute_tsk(void) {}
void avi_recode_put_video_buff(void *buf, u32 size) {}
void avi_recode_put_video_buff_fast(void *buf, u32 size) {}
void api_video_play_frame_read_clean(void) {}
bool api_video_play_frame_read_value(void) { return false; }
void video_take_photo_callback(void) {}

// 其他编解码器
void bsp_pickup_isr(void) {}
void codec_info(void *info) {}
u32 avio_get_sample_index(void *avio) { return 0; }

// ============================================================================
// libapp.a 桩函数
// ============================================================================
void ble_uart_service_write(u8 *buff, u8 len) {}

// ============================================================================
// libimg.a 桩函数 — GIF 解码
// ============================================================================
void bsp_gif_lzw(void *ctx, u8 *out, int len) {}
void *stbi__gif_init(void *stbi) { return NULL; }
int stbi__gif_load_header(void *stbi, int *x, int *y, int *comp) { return 0; }
int stbi__gif_load_next(void *stbi, void *gif, int *delays) { return 0; }
void stbi__gif_ram_init(void *stbi, u8 *buf, int len) {}
void stbi__gif_register_lzw(void *stbi) {}
void stbi__gif_set_img_buf(void *stbi, u8 *buf, int len) {}
