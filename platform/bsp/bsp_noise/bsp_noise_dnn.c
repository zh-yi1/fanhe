#include "include.h"

#if BT_SCO_DNN_EN

typedef enum {
    RDFT_128 = 0,
    RDFT_256,
    RDFT_512,
} RDFT_LEN;

typedef struct {
    void *in_addr;
    void *out_addr;
    RDFT_LEN size;					//size:0(128), 1(256), 2(512)
    u8 window_en        :1;         //只有fft 512有效
    u8 input_type       :1;         //input type:0,half word; 1,word
    u8 isr_en           :1;         //是否打开中断
} fft_cfg_t;

typedef struct {
    void *in_addr;
    void *out_addr;
    RDFT_LEN size;                  //size:0(128), 1(256), 2(512)
    u8 window_en        :1;         //只有ifft 512有效
    u8 output_type      :1;         //output type:0,half word; 1,word
    u8 overlap_en       :1;
    u8 overlap_len      :1;         //ola长度：0:240,1:320（512点有效）
    u8 isr_en           :1;         //是否打开中断
} ifft_cfg_t;

typedef struct {
    fft_cfg_t   fft_cft;
    ifft_cfg_t  ifft_cft;
    s16 input[256];
    s16 output[256];
    s16 fft_in[512];
    s32 fft_out[512];
    s16 ifft_out[512];
    u8 iptr;
    u8 optr;

    u8 talk_cnt;
    u8 dnn_cnt;
    s16 mem_hp_x;
} dnn_rdft_t;

void dnn_init(dnn_cb_t *dnn_cb);
void noise_dnn_kick_start(void);
void dnn_fre_process(s32 *fft_in);
void fft_hw(fft_cfg_t *cfg);
void ifft_hw(ifft_cfg_t *cfg);
void rdft_init(void);
void rdft_exit(void);

extern u32 __dnn_vma, __dnn_lma, __dnn_size;

static dnn_rdft_t dnn_rdft AT(.dnn_data.fft);
static volatile bool noise_nr_alg_start = false;

AT(.bt_voice.sco.dnn) WEAK
void noise_dnn_sm_process(void)
{
    if (noise_nr_alg_start == false) {
        return;
    }
    //int i;

//    nr_cb_t *nr = &bt_voice_cfg->nr;
    fft_cfg_t   *fft   = &dnn_rdft.fft_cft;
    ifft_cfg_t  *ifft  = &dnn_rdft.ifft_cft;

//    if (nr->dump_en & BIT(0)) {
//        sco_huart_putcs(0, dnn_rdft.talk_cnt++, &dnn_rdft.fft_in[256],256 * 2);
//    }

    fft_hw(fft);

    dnn_fre_process(dnn_rdft.fft_out);

    ifft_hw(ifft);

//    if (nr->dump_en & BIT(1)) {
//        sco_huart_putcs(1, dnn_rdft.dnn_cnt++,dnn_rdft.ifft_out,256 * 2);
//    }
//    memcpy(&dnn_rdft.fft_in[0], &dnn_rdft.fft_in[256], 256 * 2);
    memcpy(&dnn_rdft.fft_in[0], &dnn_rdft.fft_in[240], 240 * 2);
}

typedef int16_t pcm_sample_t;
#define INT16_MAX   32767
#define INT16_MIN   -32768
AT(.com_text.noise_dnn)
u32 pcm_double2single_mix(pcm_sample_t *in_double_ch, pcm_sample_t *out_single_ch, u32 pcm_total_len)
{

#if 1
    // 1. 同方法1，参数合法性校验
    if (in_double_ch == NULL || out_single_ch == NULL || pcm_total_len == 0) {
        return 0;
    }
    u32 sample_bytes = sizeof(pcm_sample_t);
    u32 double_frame_bytes = 2 * sample_bytes;
    if (pcm_total_len % double_frame_bytes != 0) {
        return 0;
    }

    // 2. 计算样本总数
    u32 double_sample_count = pcm_total_len / sample_bytes;
    u32 single_sample_count = double_sample_count / 2;

    // 3. 左右声道等权混合（避免溢出：先转为32位整型计算，再转回16位）
    for (u32 i = 0, j = 0; i < double_sample_count; i += 2, j++) {
        // 提取左右声道样本
        int32_t ch0 = (int32_t)in_double_ch[i];    // 左声道
        int32_t ch1 = (int32_t)in_double_ch[i+1];  // 右声道

        // 等权平均计算（(ch0 + ch1) / 2），先转32位防止16位数据溢出
        int32_t mix_sample = (ch0 + ch1) / 2;

        // 限幅处理（确保结果在16bit有符号整型范围内，防止失真）
        if (mix_sample > INT16_MAX) {
            mix_sample = INT16_MAX;
        } else if (mix_sample < INT16_MIN) {
            mix_sample = INT16_MIN;
        }

        // 存入单通道缓冲区
        out_single_ch[j] = (pcm_sample_t)mix_sample;
    }

    // 4. 返回单通道总字节数
    return single_sample_count * sample_bytes;

#else
    // 1. 校验参数合法性（嵌入式场景必备，防止内存越界）
    if (in_double_ch == NULL || out_single_ch == NULL || pcm_total_len == 0) {
        return 0;
    }
    // 单样本字节数（16bit=2字节）
    u32 sample_bytes = sizeof(pcm_sample_t);
    // 双通道单帧字节数（左+右=2*单样本字节数）
    u32 double_frame_bytes = 2 * sample_bytes;
    // 校验总字节数是否合法（必须是双通道单帧字节数的整数倍）
    if (pcm_total_len % double_frame_bytes != 0) {
        return 0;
    }

    // 2. 计算样本总数（双通道总样本数=总字节数/单样本字节数，单通道样本数=双通道总样本数/2）
    u32 double_sample_count = pcm_total_len / sample_bytes;
    u32 single_sample_count = double_sample_count / 2;

    // 3. 提取左声道（交错存储中，偶数索引为左声道：0,2,4...，奇数索引为右声道：1,3,5...）
    for (u32 i = 0, j = 0; i < double_sample_count; i += 2, j++) {
        out_single_ch[j] = in_double_ch[i];  // 提取左声道，丢弃右声道（in_double_ch[i+1]）
//        out_single_ch[j] = in_double_ch[i+1]; //若需提取右声道，替换为：
    }

    // 4. 返回单通道总字节数（单样本数*单样本字节数）
    return single_sample_count * sample_bytes;
#endif // 0
}

static pcm_sample_t noise_obuf[60];

//AT(.bt_voice.noise.dmdnn.proc)
AT(.com_text.noise_dnn)
void noise_dnn_process(s16 *buf, u32 samples, u8 ch)
{
//    printf("^");

    if (noise_nr_alg_start == false) {
        return;
    }

    if (samples != 60) {
        printf("^");
        return;
    }


    s16* obuf = NULL;

//    memset(obuf, 0, samples*sizeof(pcm_sample_t));
//    printf("1");
    ///双通道->单通道数据
//    printf("ch:%d\n", ch);
    if (ch > 1) {
//        printf(".");
        obuf = (s16*)noise_obuf;
        pcm_double2single_mix(buf, obuf, samples * ch * 2);
    } else {
        obuf = buf;
//        printf("&");
//        memcpy(obuf, buf, samples*sizeof(pcm_sample_t));
    }

//    printf("2");

//    memcpy(&dnn_rdft.input[dnn_rdft.iptr * 60], buf, 60 * 2);
    memcpy(&dnn_rdft.input[dnn_rdft.iptr * 60], obuf, 60 * 2);

//    printf("3");

    dnn_rdft.iptr++;
    if (dnn_rdft.iptr >= 4) {

        dnn_rdft.iptr = 0;
        memcpy(dnn_rdft.output, dnn_rdft.ifft_out, 240 * 2);
        memcpy(&dnn_rdft.fft_in[240], dnn_rdft.input, 240 * 2);
        noise_dnn_kick_start();
        dnn_rdft.optr = 0;
    }

//    memcpy(buf, &dnn_rdft.output[dnn_rdft.optr * 60], 60 * 2);
    memcpy(obuf, &dnn_rdft.output[dnn_rdft.optr * 60], 60 * 2);
    if (ch > 1) {
        memcpy(buf, obuf, 60 * 2);
    }

    dnn_rdft.optr++;
    if (dnn_rdft.optr >= 4) {
        dnn_rdft.optr = 0;
    }
}

WEAK
void noise_dnn_init(dnn_cb_t *dnn_cb)
{
    printf("noise dnn init\n");

    fft_cfg_t   *fft    = &dnn_rdft.fft_cft;
    ifft_cfg_t  *ifft   = &dnn_rdft.ifft_cft;
    memset(&dnn_rdft, 0, sizeof(dnn_rdft_t));

    fft->size           = RDFT_512;
	fft->input_type     = 0;
    fft->window_en      = 1;
    fft->isr_en         = 1;
    fft->in_addr        = dnn_rdft.fft_in;
    fft->out_addr       = dnn_rdft.fft_out;

    ifft->size          = RDFT_512;
    ifft->output_type   = 0;
    ifft->window_en     = 1;
    ifft->isr_en        = 1;
    ifft->overlap_en    = 1;
    ifft->in_addr       = dnn_rdft.fft_out;
    ifft->out_addr      = dnn_rdft.ifft_out;
    rdft_init();

    printf("bt_dnn_init: %x, %x, %x\n", &__dnn_vma, &__dnn_lma, (u32)&__dnn_size);
    memcpy(&__dnn_vma, &__dnn_lma, (u32)&__dnn_size);
    dnn_init((dnn_cb_t *)dnn_cb);
    noise_nr_alg_start = true;
}

WEAK
void noise_dnn_exit(void)
{
    noise_nr_alg_start = false;
    rdft_exit();
}

#endif
