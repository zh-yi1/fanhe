#include "include.h"
#include "asr.h"

#if (ASR_SELECT == ASR_WS_AIR)

#if ASR_AIR_PREFETCH_EN
#define TDNN_PARAMS_PRELOAD_TO_RAM          0 //权重计算完成后的调节参数是否提前放到ram中，可提升一点速度
#define BASE_ADDR   UI_BUF_ASR_WEIGHT_BIN
#define BASE_LEN    UI_LEN_ASR_WEIGHT_BIN
#define W7OD        488

extern const int16_t tdnn_mean_1[128];
extern const int16_t tdnn_bias_1[128];
extern const float tdnn_var_1[128];
extern const int16_t tdnn_mean_2[128];
extern const int16_t tdnn_bias_2[128];
extern const float tdnn_var_2[128];
extern const int16_t tdnn_mean_3[128];
extern const int16_t tdnn_bias_3[128];
extern const float tdnn_var_3[128];
extern const int16_t tdnn_mean_4[128];
extern const int16_t tdnn_bias_4[128];
extern const float tdnn_var_4[128];
extern const int16_t tdnn_mean_5[128];
extern const int16_t tdnn_bias_5[128];
extern const float tdnn_var_5[128];
extern const int16_t tdnn_mean_6[128];
extern const int16_t tdnn_bias_6[128];
extern const float tdnn_var_6[128];
extern const int16_t tdnn_bias_7[W7OD];
extern const float tdnn_scale_7[W7OD];

#if TDNN_PARAMS_PRELOAD_TO_RAM
int16_t tdnn_ram_mean1[128] AT(.ws_asr.test);
int16_t tdnn_ram_bias1[128] AT(.ws_asr.test);
float tdnn_ram_var1[128] AT(.ws_asr.test);
int16_t tdnn_ram_mean2[128] AT(.ws_asr.test);
int16_t tdnn_ram_bias2[128] AT(.ws_asr.test);
float tdnn_ram_var2[128] AT(.ws_asr.test);
int16_t tdnn_ram_mean3[128] AT(.ws_asr.test);
int16_t tdnn_ram_bias3[128] AT(.ws_asr.test);
float tdnn_ram_var3[128] AT(.ws_asr.test);
int16_t tdnn_ram_mean4[128] AT(.ws_asr.test);
int16_t tdnn_ram_bias4[128] AT(.ws_asr.test);
float tdnn_ram_var4[128] AT(.ws_asr.test);
int16_t tdnn_ram_mean5[128] AT(.ws_asr.test);
int16_t tdnn_ram_bias5[128] AT(.ws_asr.test);
float tdnn_ram_var5[128] AT(.ws_asr.test);
int16_t tdnn_ram_mean6[128] AT(.ws_asr.test);
int16_t tdnn_ram_bias6[128] AT(.ws_asr.test);
float tdnn_ram_var6[128] AT(.ws_asr.test);
int16_t tdnn_ram_bias7[W7OD] AT(.ws_asr.test);
float tdnn_ram_scale7[W7OD] AT(.ws_asr.test);
#endif // TDNN_PARAMS_PRELOAD_TO_RAM

typedef struct {
    u32 load_addr;
} tdnn_pretetch_t;
tdnn_pretetch_t tdnn_pretetch AT(.ws_asr.test);

volatile u8 asr_prefetch_kisck;
int32_t sum_buffer[W7OD] AT(.ws_asr.test);
int32_t matrix_sum AT(.ws_asr.test);
#define tdnn_round(x) ((x)>=0?((x)+0.5):((x)-0.5))

AT(.com_text.tdnn)
void tdnn_compute(int8_t *in_buf, float in_scale, int in_dim, int out_dim,
                  const int8_t *tdnn_weight, const int16_t *tdnn_bias,
                  const int16_t *tdnn_mean, const float *tdnn_var, const float *tdnn_scale,
                  float *out_buf, int last_layer) {
    int i;
    if (asr_prefetch_kisck) {
        return;
    }
#if TDNN_PARAMS_PRELOAD_TO_RAM
    if (tdnn_mean == tdnn_mean_1) {
        tdnn_mean = tdnn_ram_mean1;
        tdnn_bias = tdnn_ram_bias1;
        tdnn_var = tdnn_ram_var1;
    } else if (tdnn_mean == tdnn_mean_2) {
        tdnn_mean = tdnn_ram_mean2;
        tdnn_bias = tdnn_ram_bias2;
        tdnn_var = tdnn_ram_var2;
    } else if (tdnn_mean == tdnn_mean_3) {
        tdnn_mean = tdnn_ram_mean3;
        tdnn_bias = tdnn_ram_bias3;
        tdnn_var = tdnn_ram_var3;
    } else if (tdnn_mean == tdnn_mean_4) {
        tdnn_mean = tdnn_ram_mean4;
        tdnn_bias = tdnn_ram_bias4;
        tdnn_var = tdnn_ram_var4;
    } else if (tdnn_mean == tdnn_mean_5) {
        tdnn_mean = tdnn_ram_mean5;
        tdnn_bias = tdnn_ram_bias5;
        tdnn_var = tdnn_ram_var5;
    } else if (tdnn_mean == tdnn_mean_6) {
        tdnn_mean = tdnn_ram_mean6;
        tdnn_bias = tdnn_ram_bias6;
        tdnn_var = tdnn_ram_var6;
    } else {
        tdnn_scale = tdnn_ram_scale7;
        tdnn_bias = tdnn_ram_bias7;
    }
#endif //TDNN_PARAMS_PRELOAD_TO_RAM
    tdnn_pretetch_t *t = &tdnn_pretetch;
    u32 frame_len = (out_dim * in_dim);
    memset(sum_buffer, 0,sizeof(sum_buffer));

    tdnn_start();//tdnn使能后，不能使用DMA发送SPI命令、地址
    tdnn_set_dimwnsion(out_dim, in_dim);
    tdnn_kick(sum_buffer, in_buf);
    spiflash_lock();
    spiflash_read_kick((u32*)0x80000000, t->load_addr, frame_len);
    tdnn_wait();
    tdnn_finish();
    spiflash_read_wait();
    spiflash_unlock();

	t->load_addr += frame_len;
    if (t->load_addr >= (BASE_ADDR + BASE_LEN)) {
        t->load_addr = BASE_ADDR;
    }

    for (i = 0; i < out_dim; i++) {
        matrix_sum = sum_buffer[i];
        int32_t q = tdnn_round(matrix_sum * in_scale);
        int32_t z = q + tdnn_bias[i];
        if (!last_layer) {
            if (z <= 0) {
                z = 0;
            }
            out_buf[i] = (z - tdnn_mean[i]) * tdnn_var[i];
        } else {
            out_buf[i] = z * tdnn_scale[i];
        }
    }
}

ALIGNED(512)
void asr_prefetch_init_do(void)
{
    tdnn_pretetch_t *t = &tdnn_pretetch;
    memset(t, 0, sizeof(tdnn_pretetch_t));
    t->load_addr = BASE_ADDR;
#if TDNN_PARAMS_PRELOAD_TO_RAM
    memcpy(tdnn_ram_mean1,     tdnn_mean_1,     sizeof(tdnn_mean_1));
    memcpy(tdnn_ram_bias1,     tdnn_bias_1,     sizeof(tdnn_bias_1));
    memcpy(tdnn_ram_var1,      tdnn_var_1,      sizeof(tdnn_var_1));
    memcpy(tdnn_ram_mean2,     tdnn_mean_2,     sizeof(tdnn_mean_2));
    memcpy(tdnn_ram_bias2,     tdnn_bias_2,     sizeof(tdnn_bias_2));
    memcpy(tdnn_ram_var2,      tdnn_var_2,      sizeof(tdnn_var_2));
    memcpy(tdnn_ram_mean3,     tdnn_mean_3,     sizeof(tdnn_mean_3));
    memcpy(tdnn_ram_bias3,     tdnn_bias_3,     sizeof(tdnn_bias_3));
    memcpy(tdnn_ram_var3,      tdnn_var_3,      sizeof(tdnn_var_3));
    memcpy(tdnn_ram_mean4,     tdnn_mean_4,     sizeof(tdnn_mean_4));
    memcpy(tdnn_ram_bias4,     tdnn_bias_4,     sizeof(tdnn_bias_4));
    memcpy(tdnn_ram_var4,      tdnn_var_4,      sizeof(tdnn_var_4));
    memcpy(tdnn_ram_mean5,     tdnn_mean_5,     sizeof(tdnn_mean_5));
    memcpy(tdnn_ram_bias5,     tdnn_bias_5,     sizeof(tdnn_bias_5));
    memcpy(tdnn_ram_var5,      tdnn_var_5,      sizeof(tdnn_var_5));
    memcpy(tdnn_ram_mean6,     tdnn_mean_6,     sizeof(tdnn_mean_6));
    memcpy(tdnn_ram_bias6,     tdnn_bias_6,     sizeof(tdnn_bias_6));
    memcpy(tdnn_ram_var6,      tdnn_var_6,      sizeof(tdnn_var_6));
    memcpy(tdnn_ram_bias7,     tdnn_bias_7,     sizeof(tdnn_bias_7));
    memcpy(tdnn_ram_scale7,    tdnn_scale_7,    sizeof(tdnn_scale_7));
#endif // TDNN_PARAMS_PRELOAD_TO_RAM
}

void asr_prefetch_init(void)
{
    asr_prefetch_init_do();
    asr_prefetch_kisck = 5;
    printf(">>>>>>>>>>>>>>>>>>>enter %s\n", __func__);
}
#endif // ASR_AIR_PREFETCH_EN

#if !ASR_AIR_PREFETCH_EN
//汇编原型
//int vector_multadd(int8_t *in_buf, const int8_t *w, int in_dim)
//{
// 	int sum = 0;
//	for (int i = 0; i < in_dim; i++) {
//		sum += in_buf[i] * w[i];
//	}
//	return sum;
//}
int vector_multadd(int8_t *in_buf, const int8_t *w, int in_dim);
AT(.com_text.tdnn)
void tdnn_compute(int8_t *in_buf, float in_scale, int in_dim, int out_dim,
                  const int8_t *tdnn_weight, const int16_t *tdnn_bias,
                  const int16_t *tdnn_mean, const float *tdnn_var, const float *tdnn_scale,
                  float *out_buf, int last_layer)
{
    for(int i = 0; i < out_dim; i++){
        const int8_t *w = tdnn_weight + i * in_dim;
        int32_t dot_prod = vector_multadd(in_buf, w, in_dim);
        int32_t q = dot_prod * in_scale;
        int32_t z = q + tdnn_bias[i];
        if(!last_layer){
            if(z <= 0){
                z = 0;
            }
            out_buf[i] = (z - tdnn_mean[i]) * tdnn_var[i];
        } else {
            out_buf[i] = z * tdnn_scale[i];
        }
    }
}
#endif

#endif //ASR_WS_AIR