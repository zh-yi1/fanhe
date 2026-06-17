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
