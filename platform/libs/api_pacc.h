#ifndef _API_PACC_H
#define _API_PACC_H

//pacc eq/drc
typedef enum{
    S16_Q15_SAT = 0,
    S32_Q23_SAT,
    S32_Q15,
    S32_Q23,
} pacc_fmt_t;

typedef struct pacc_cs_tag {
    u8 buf[36];
} pacc_cs_t;

typedef struct {
	int threshold;
	int gain;
	int slope;
} drc_coeff_t;

typedef struct __attribute__((packed)){
	int in_attack;
	int in_release;
	int out_attack;
	int out_release;
	int in_level;
	int out_gain;
	int tav;
	int in_rms;
	drc_coeff_t coeffs[4];
} drc_coef_t;

//bq模式
#define BQ_BAND_MAX_NB      10           //最大支持的band_num，必须跟库一致

typedef struct {
    drc_coef_t drc_coef;
	s32 zx[2][2*(BQ_BAND_MAX_NB+1)];    // 使用fade时需要分配2个
	s32 coef[2][5*BQ_BAND_MAX_NB+1];    // 使用fade时需要分配2个
} bq_bd_t;

//drc模式
#define DRC_BQ_MAX_NUM    8

//drc_mode0, drc
typedef struct {
	drc_coef_t drc_coef;                       //drc_coef[20]
} drc_bd_t;

//drc_mode1, drc->bq0
typedef struct {
	drc_coef_t drc_coef;                    //drc_coef[20]
	s32 zx0[DRC_BQ_MAX_NUM + 1][2];         //bq0_buf[2*(8+1)]
	s32 bq_coef0[5 * DRC_BQ_MAX_NUM+1];     //gain, bq0_coef[5*8]
} drc_bq_bd_t;

//drc_mode2, bq0->drc->bq1
typedef struct {
    drc_coef_t drc_coef;                    //drc_coef[20]
	s32 zx0[DRC_BQ_MAX_NUM + 1][2];         //pre_buf[2*(8+1)]
	s32 zx1[DRC_BQ_MAX_NUM + 1][2];         //pos_buf[2*(8+1)]
	s32 bq_coef0[5 * DRC_BQ_MAX_NUM+1];     //gain, pre_coef[5*8]
    s32 bq_coef1[5 * DRC_BQ_MAX_NUM+1];     //gain, pos_coef[5*8]
} mdrc_bd_t;

//dybq
#define DYBQ_BAND_MAX_NB    1
#define MAX_CH              2
#define DT_BAND             4

typedef struct {
    drc_coef_t drc_coef;
	s32 zx[DYBQ_BAND_MAX_NB + 1][2];
	s32 bq_coef[4][5*DYBQ_BAND_MAX_NB+1];
} dybq_sb_t;

//freq_shift
#define HIBERT_ORDER                                  160
typedef struct {
	s32 zx[HIBERT_ORDER];
	s32 Hilbert_hcoef[HIBERT_ORDER];
	s32 phase_step;
} freq_shift_bd_t;

//pacc api
void *pacc_ctl_init(uint link);                                     //初始化pacc外设，link=0/1
void pacc_ctl_exit(uint link);                                      //关闭pacc外设，link=0/1
void pacc_ctl_set_cs(void *ctl, pacc_cs_t *start_cs);               //注册pacc处理的cs链路（公共区）
void pacc_ctl_proc(void *ctl, void *dst, void *src, uint samples);  //按注册顺序处理cs链路（公共区）
void pacc_ctl_proc_cs(void *ctl, void *start_cs, void *dst, void *src, uint samples);   //将start_cs注册到ctl，并按顺序处理cs链路（公共区）

//cs api
void pacc_cs_set_next(pacc_cs_t *cs, pacc_cs_t *next_cs);                               //将next_cs链接到cs->next_ptr
void pacc_cs_set_buf_addr(pacc_cs_t *cs, void *buf_addr, void *coef_addr);              //设置cs的buf_addr和coef_addr
void pacc_cs_set_io_fmt(pacc_cs_t *cs, bool stereo, u8 in_fmt, u8 out_fmt);             //设置cs的输入输出格式
void pacc_cs_set_io_addr_fsize(pacc_cs_t *cs, void *out_addr, void *in_addr0, void *in_addr1, uint samples);//设置cs的输入输出buffer地址
//cs_list
void pacc_cs_list_set_io_fmt(pacc_cs_t *start_cs, bool stereo, u8 in_fmt, u8 out_fmt);  //设置cs链路输入输出格式
void pacc_cs_list_set_io_fsize(pacc_cs_t *start_cs, uint samples);                      //设置cs链路的处理帧长
void pacc_cs_list_set_io_addr_fsize(pacc_cs_t *start_cs, void *dst, void *src, uint samples); //设置cs链路输入输出buffer地址及整个链路的帧长（公共区）

//bq api
uint pacc_eq_init(pacc_cs_t *cs, bq_bd_t *bd, bool stereo, u8 in_fmt, u8 out_fmt);      //初始化bq_cs
uint pacc_eq_off(pacc_cs_t *cs);                                                        //关闭bq_cs
void pacc_eq_set_by_param(pacc_cs_t *cs, u8 band_cnt, const u32 *eq_param);
bool pacc_eq_set_by_res(pacc_cs_t *cs, u32 addr, u32 len);
void pacc_eq_set_post_gain(pacc_cs_t *cs, u32 gain);

//dybq api

//drc api
uint pacc_drc_init(pacc_cs_t *cs, drc_bd_t *bd, bool stereo, u8 in_fmt, u8 out_fmt);    //初始化drc_cs
uint pacc_drc_off(pacc_cs_t *cs);                                                       //关闭drc_cs
bool pacc_drc_set_by_res(pacc_cs_t *cs, u32 addr, u32 len);
void pacc_drc_set_by_param(pacc_cs_t *cs, u8 band_cnt, const u32 *drc_param);

//freq_shift api
uint pacc_fsh_init(pacc_cs_t *cs, freq_shift_bd_t *bd, bool stereo, u8 in_fmt, u8 out_fmt);
uint pacc_fsh_set_param(pacc_cs_t *cs, s32 phase_step);
uint pacc_fsh_off(pacc_cs_t *cs);


//pacc_effect处理类型
enum {
    PACC_EQ,            //mode0, bq
    PACC_EQ_DRC,        //mode1, bq->drc
    PACC_DRC,           //mode0, drc
    PACC_DRC_EQ,        //mode1, drc -> eq
    PACC_DYEQ,          //dybq
    PACC_FSH,           //freq_shift
};

typedef struct pacc_effect_cb_tag {
    u8 acc_id;
    u8 acc_mode;
    bool stereo;
    bool right_channel;
    void *data_in;
    void *data_in1;
    pacc_fmt_t fmt_in;
    void *data_out;
    pacc_fmt_t fmt_out;
    void *pacc_cs;
    void *pacc_sb;
} pacc_effect_cb;


//音乐使用PACC1 麦使用PACC0
void *pacc_effect_init(uint link);  //初始化pacc_effect，返回：ctl控制句柄
pacc_cs_t *pacc_effect_link(const pacc_effect_cb *cb_tbl, u8 max_cs, u16 frame_len);  //链表注册，返回cs链表的表头
pacc_cs_t *pacc_effect_relink(pacc_cs_t *cs_tbl, u8 *pacc_en, u8 max_cs);       //链表动态调整，返回cs链表的链头
void pacc_effect_process(void *ctl, pacc_cs_t *start_cs);                       //按链表顺序处理音频链路，每次处理frame_len个样点
void pacc_effect_exit(uint link);   //关闭pacc_effect

void pacc_effect_set_drc_param(void* coef, u8 *buf, u8 params);
void pacc_effect_set_eq_param(void* coef, u8 *buf, u8 params);
void pacc_effect_set_dyeq_param(void* coef, u8 *buf, u8 params);
void pacc_effect_set_mbdrc_bq_param(void* bq0, void* bq1, u8 *buf, u8 params, u8 flag);

#endif

