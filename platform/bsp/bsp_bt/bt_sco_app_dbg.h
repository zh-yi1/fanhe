#ifndef __BT_SCO_APP_DBG_H
#define __BT_SCO_APP_DBG_H

#include "include.h"

#define SOC_APP_HEADER_LEN      5
#define SOC_APP_PAYLOAD_POS     5
#define SOC_APP_CM_TAG          0xab23

#define SCO_APP_RX_PAYLOAD_LEN      (256)
#define SCO_APP_TX_PAYLOAD_LEN      (512)

#define SCO_DBG_ALG_ALL_EN          (0xffff)
#define SCO_DBG_ALG_ALL_DISEN       (0x0000)

typedef enum {
    SCO_DBG_PER_EQ      = BIT(0),               //算法前EQ
    SCO_DBG_PER_DRC     = BIT(1),               //算法前DRC
    SCO_DBG_AEC_LP      = BIT(2),               //AEC线性
    SCO_DBG_AEC_NLP     = BIT(3),               //AEC非线性
    SCO_DBG_NR          = BIT(4),               //降噪
    SCO_DBG_TRUMPET     = BIT(5),               //汽车喇叭声降噪
    SCO_DBG_POST_EQ     = BIT(6),               //算法后EQ
    SCO_DBG_POST_DRC    = BIT(7),               //算法后DRC
    SCO_DBG_FAR_NR      = BIT(8),               //远端降噪
    SCO_DBG_SNDP_AINS2  = BIT(9),               //声加算法前的AINS2预处理
} sco_dbg_t;

enum SCO_APP_CMD {
    CMD_SCO_INF_GET         = 0x27,     //设置通话的基本参数(芯片型号、SDK版本、MTU值、时钟、增益、汽车喇叭声降噪量、远端降噪、AEC参数)
    CMD_SCO_NOTIFY          = 0x28,     //设备端上报设备信息

    CMD_SYS_CLK_SET         = 0xB0,     //设置通话时的系统主频
    CMD_MIC_ANL_GAIN_SET,               //设置MIC模拟增益
    CMD_MIC_DIG_GAIN_SET,               //设置MIC数字增益
    CMD_TRUMPET_NR_SET,                 //设置汽车喇叭声降噪参数
    CMD_POST_GIAN_SET,                  //设置后置增益
    CMD_FAR_NR_SET,                     //设置远端降噪参数
    CMD_AEC_SET,                        //设置AEC参数
    CMD_ENC_SET,                        //设置ENC参数

    CMD_PRE_EQ_SET          = 0xC0,     //设置算法前EQ曲线
    CMD_PRE_DRC_SET,                    //设置算法前DRC曲线
    CMD_POST_EQ_SET,                    //设置算法后EQ曲线
    CMD_POST_DRC_SET,                   //设置算法后DRC曲线

    CMD_NR_ANS_SET          = 0xD0,     //设置ANS降噪参数
    CMD_NR_AINS_SET,                    //设置AINS降噪参数
    CMD_NR_AINS2_SET,                   //设置AINS2降噪参数
    CMD_NR_SNDP_RNN_SET,                //设置声加RNN降噪参数
    CMD_NR_SNDP_DNN_SET,                //设置声加DNN降噪参数
    CMD_NR_SNDP_DMIC_SET,               //设置声加传统双麦降噪参数
    CMD_NR_SNDP_DMIC_DNN_SET,           //设置声加双麦+AI降噪参数
    CMD_NR_DNN_SET,                     //设置DNN降噪参数
    CMD_NR_DMNS_SET,                    //设置DMNS降噪参数
    CMD_NR_DMDNN_SET,                   //设置DMDNN降噪参数
};

//CMD_SCO_REQUEST
enum SCO_INF_SEND {
    CMD_SCO_INF_CHIP_TYPE               = 0xb1,
    CMD_SCO_INF_SDK_VERSION,
    CMD_SCO_INF_MTU_VALUE,
    CMD_SCO_INF_SYS_CLK,
    CMD_SCO_INF_ANL_GAIN,
    CMD_SCO_INF_DIG_GAIN,
    CMD_SCO_INF_POST_GAIN,
    CMD_SCO_INF_TRUMPET_PARAM,
    CMD_SCO_INF_FAR_NR_PARAM,
    CMD_SCO_INF_AEC_PARAM,
    CMD_SCO_INF_ENC_PARAM,

    CMD_SCO_INF_PER_EQ                  = 0xc0,
    CMD_SCO_INF_PER_DRC,
    CMD_SCO_INF_POST_EQ,
    CMD_SCO_INF_POST_DRC,

    CMD_SCO_INF_ANS_PARAM               = 0xd0,
    CMD_SCO_INF_AINS_PARAM,
    CMD_SCO_INF_AINS2_PARAM,
    CMD_SCO_INF_SNDP_RNN_PARAM,
    CMD_SCO_INF_SNDP_DNN_PARAM,
    CMD_SCO_INF_SNDP_DMIC_PARAM,
    CMD_SCO_INF_SNDP_DMIC_DNN_PARAM,
    CMD_SCO_INF_DNN_PARAM,
    CMD_SCO_INF_DMNS_PARAM,
    CMD_SCO_INF_DMDNN_PARAM,
};


enum CMD_SCO_NOTIFY {
    CMD_SCO_NOTIFY_CALL_STATUS          = 0xb0,
};

enum CMD_SCO_NOTIFY_CALL_STATUS {
    TLV_CALL_STATUS_NONE    = 0x00,
    TLV_CALL_STATUS_CVSD,
    TLV_CALL_STATUS_MSBC,
};

enum CMD_SCO_INF_ANL_GAIN {
    TLV_MIC_ANL_GAIN_MICL   = 0x01,
    TLV_MIC_ANL_GAIN_MICR,
};

enum CMD_SCO_INF_DIG_GAIN {
    TLV_MIC_DIG_GAIN_MICL   = 0x01,
    TLV_MIC_DIG_GAIN_MICR,
};

enum CMD_SCO_INF_TRUMPET_PARAM {
    TLV_TRUMPET_NR_EN       = 0x01,
    TLV_TRUMPET_NR_LEVEL,
};

enum CMD_SCO_INF_FAR_NR_PARAM {
    TLV_FAR_NR_EN           = 0x01,
    TLV_FAR_NR_TYPE,
    TLV_FAR_NR_LEVEL,
};

enum CMD_SCO_INF_AEC_PARAM {
    TLV_AEC_EN              = 0x01,
    TLV_AEC_TYPE,
    TLV_AEC_LP_EN,
    TLV_AEC_NLP_EN,
    TLV_AEC_ECHO_LEVEL,
    TLV_AEC_FAR_OFFSET,
    TLV_AEC_AEC_MODE,
};

enum CMD_SCO_INF_POST_EQ_PARAM {
    TLV_POST_EQ_EN          = 0x01,
    TLV_POST_EQ_MSBC,
    TLV_POST_EQ_BAND_CNT,
    TLV_POST_EQ_VERSION,
    TLV_POST_EQ_SAMPLE_RATE,
    TLV_POST_EQ_RFU,
    TLV_POST_EQ_GAIN,
    TLV_POST_EQ_PARAM,
};

enum CMD_SCO_INF_POST_DRC_PARAM {
    TLV_POST_DRC_EN          = 0x01,
    TLV_POST_DRC_VERSION,
    TLV_POST_DRC_SAMPLE_RATE,
    TLV_POST_DRC_CNT,
    TLV_POST_DRC_PARAM,
    TLV_POST_DRC_MSBC,
};

enum TLV_SCO_CODEC_TYPE {
    TLV_SCO_CODEC_CVSD          = 0x00,
    TLV_SCO_CODEC_MSBC,
};

enum TLV_SCO_SAMPLE_RATE_TYPE {
    TLV_SCO_SAMPLE_RATE_8K      = 0x00,
    TLV_SCO_SAMPLE_RATE_16K,
    TLV_SCO_SAMPLE_RATE_44_1K,
    TLV_SCO_SAMPLE_RATE_48K,
};

enum CMD_SCO_INF_ENC_PARAM {
    TLV_ENC_EN              = 0x01,
    TLV_ENC_TYPE,
};

enum CMD_SCO_INF_ANS_PARAM {
    TLV_ENC_ANS_EN          = 0x01,
    TLV_ENC_ANS_LEVEL,
};

enum CMD_SCO_INF_AINS_PARAM {
    TLV_ENC_AINS_EN         = 0x01,
    TLV_ENC_AINS_LEVEL,
};

enum CMD_SCO_INF_AINS2_PARAM {
    TLV_ENC_AINS2_EN        = 0x01,
    TLV_ENC_AINS2_LEVEL,
};

enum CMD_SCO_INF_SNDP_RNN_PARAM {
    TLV_ENC_SNDP_RNN_EN     = 0x01,
    TLV_ENC_SNDP_RNN_LEVEL,
    TLV_ENC_SNDP_RNN_PER_PROC_EN,
};

enum CMD_SCO_INF_SNDP_DNN_PARAM {
    TLV_ENC_SNDP_DNN_EN     = 0x01,
    TLV_ENC_SNDP_DNN_MAX_LEVEL,
    TLV_ENC_SNDP_DNN_MIN_LEVEL,
    TLV_ENC_SNDP_DNN_PER_PROC_EN,
};

enum CMD_SCO_INF_SNDP_DMIC_PARAM {
    TLV_ENC_SNDP_DMIC_EN    = 0x01,
    TLV_ENC_SNDP_DMIC_DISTANCE,
    TLV_ENC_SNDP_DMIC_DEGREE,
    TLV_ENC_SNDP_DMIC_MAX_DEGREE,
    TLV_ENC_SNDP_DMIC_LEVEL,
    TLV_ENC_SNDP_DMIC_WIND_LEVEL,
    TLV_ENC_SNDP_DMIC_SNR_X0,
    TLV_ENC_SNDP_DMIC_SNR_X1,
    TLV_ENC_SNDP_DMIC_MAXIR,
};

enum CMD_SCO_INF_SNDP_DMIC_DNN_PARAM {
    TLV_ENC_SNDP_DMIC_DNN_EN = 0x01,
    TLV_ENC_SNDP_DMIC_DNN_NLMS_EN,
    TLV_ENC_SNDP_DMIC_DNN_DISTANCE,
    TLV_ENC_SNDP_DMIC_DNN_DEGREE,
    TLV_ENC_SNDP_DMIC_DNN_MAX_DEGREE,
    TLV_ENC_SNDP_DMIC_DNN_LEVEL,
    TLV_ENC_SNDP_DMIC_DNN_DNN_LEVEL,
    TLV_ENC_SNDP_DMIC_DNN_WIND_LEVEL,
};

enum CMD_SCO_INF_DNN_PARAM {
    TLV_ENC_DNN_EN          = 0x01,
    TLV_ENC_DNN_MODEL,
    TLV_ENC_DNN_LEVEL,
    TLV_ENC_DNN_MIN_VALUE,
    TLV_ENC_DNN_NOSTATION_FLOOR,
    TLV_ENC_DNN_NOISE_PS_RATE,
};

enum CMD_SCO_INF_DMNS_PARAM {
    TLV_ENC_DMNS_EN         = 0x01,
    TLV_ENC_DMNS_DISTANCE,
    TLV_ENC_DMNS_DEGREE,
    TLV_ENC_DMNS_LEVEL,
    TLV_ENC_DMNS_BF_LEVEL,
    TLV_ENC_DMNS_LOWSIDE_CORR,
    TLV_ENC_DMNS_FAST_CONVERGENCE_EN,
    TLV_ENC_DMNS_FILTER_CONSTRAINT,
    TLV_ENC_DMNS_CMP_TBL_DIS,
    TLV_ENC_DMNS_MIN_VALUE,
    TLV_ENC_DMNS_NOISE_PS_RATE,
    TLV_ENC_DMNS_COMP_MIC_FRE,
    TLV_ENC_DMNS_BF_TYPE,
};

enum CMD_SCO_INF_DMDNN_PARAM {
    TLV_ENC_DMDNN_EN        = 0x01,
    TLV_ENC_DMDNN_DISTANCE,
    TLV_ENC_DMDNN_DEGREE,
    TLV_ENC_DMDNN_LEVEL,
    TLV_ENC_DMDNN_BF_LEVEL,
    TLV_ENC_DMDNN_LOWSIDE_CORR,
    TLV_ENC_DMDNN_FAST_CONVERGENCE_EN,
    TLV_ENC_DMDNN_FILTER_CONSTRAINT,
    TLV_ENC_DMDNN_CMP_TBL_DIS,
    TLV_ENC_DMDNN_MIN_VALUE,
    TLV_ENC_DMDNN_NOISE_PS_RATE,
    TLV_ENC_DMDNN_COMP_MIC_FRE,
    TLV_ENC_DMDNN_BF_TYPE,
    TLV_ENC_DMDNN_MODEL,
};

enum TLV_LAYER {
    TLV_LAYER0   = 0x00,
    TLV_LAYER1,
};

enum SCO_APP_CMD_TYPE {
    SCO_CMD_TYPE_REQUEST = 1,
    SCO_CMD_TYPE_RESPONSE,
    SCO_CMD_TYPE_NOTIFY,
};

typedef enum {
    SCO_APP_SUCCESS = 0,
    SCO_APP_FAIL,
}sco_app_result_t;

enum SCO_APP_AEC_TYPE {
    SCO_APP_AEC = 0,
    SCO_APP_NLMS,
    SCO_APP_FDNLMS,
    SCO_APP_TDAEC,
};

enum SCO_APP_NR_TYPE {
    SCO_APP_ANS = 0,
    SCO_APP_AINS,
    SCO_APP_AINS2,
    SCO_APP_SNDP_RNN,
    SCO_APP_SNDP_DNN,
    SCO_APP_SNDP_DMIC,
    SCO_APP_SNDP_DMIC_DNN,
    SCO_APP_DNN,
    SCO_APP_DMNS,
    SCO_APP_DMDNN,
};

enum SCO_APP_FAR_NR_TYPE {
    SCO_APP_FAR_NR_ANS = 0x00,
    SCO_APP_FAR_NR_AINS2,
};

typedef struct __attribute__((packed)){
    u32 seq     : 4;
    u32 reserve : 3;
    u32 encrypt : 1;
    u8  cmd;
    u8  cmd_type;
    u32 frame_seq : 4;
    u32 frame_total : 4;
    u8  payload_len;
} sco_app_cmd_head_t;

typedef struct __attribute__((packed)){
    sco_app_cmd_head_t cmd_head;
    u8  *payload;
} sco_app_cmd_t;

typedef struct __attribute__((packed)){
    sco_app_cmd_head_t cmd_head;
    u8  payload[SCO_APP_RX_PAYLOAD_LEN];
    u32 next_header_seq     : 4;
    u32 next_frame_seq      : 4;
    u8  recv_len;
    u16 total_len;
} sco_app_cmd_recv_t;

typedef struct __attribute__((packed)){
    sco_app_cmd_head_t cmd_head;
    u8  payload[SCO_APP_TX_PAYLOAD_LEN];
    u16 total_len;
} sco_app_cmd_send_t;

typedef struct {
    u8 band_cnt;
    u8 version;
    u8 sample;
    u8 rfu[1];
    const u32 *param;
} eq_param_t;

typedef struct{
    u8  sys_clk;
    u8  mic_anl_gain;
    u8  mic_dig_gain;
    u8  mic_post_gain;

    call_cfg_t bt_voice;

    eq_param_t post_eq_8k;
    eq_param_t post_eq_16k;
    u32 post_eq_8k_param[8*5+1];
    u32 post_eq_16k_param[8*5+1];

    u8 post_drc_8k_res[42 + 17 * 4];
    u8 post_drc_16k_res[42 + 17 * 4];
} sco_app_var_t;

extern sco_app_cmd_recv_t sco_app_cmd_recv;
extern sco_app_cmd_send_t sco_app_cmd_send;

extern sco_app_var_t sco_app_var;
extern call_cfg_t *bt_voice_ptr;

void sdadc_set_digital_gain(u8 gain);
uint calc_crc(void *buf, uint len, uint seed);
bool bt_sco_is_msbc(void);
bool sco_is_connected(void);

u8 get_cur_sysclk(void);
bool mic_set_eq(u8 band_cnt, const u32 *eq_param);

void sco_app_inf_send(u8 *payload,u8 payload_len);
extern const void (*sco_app_cmd_func_api[])(u8 *payload,u8 payload_len);
int get_cmd_func_api_idx(enum SCO_APP_CMD api_idx);

int bt_spp_tx(uint8_t ch, uint8_t *packet, uint16_t len);
bool bt_sco_dbg_get_alg_status(sco_dbg_t type);
void bt_sco_nr_dbg_init(void);

//用于获取应用层的值
u8 bt_sco_dbg_call_var_get(u8 index);

//通话APP设置MIC模拟增益
void sco_app_set_anl_gain(u8 gain);
//通话APP设置MIC数字增益
void sco_app_set_dig_gain(u8 gain);
//通话APP设置后置增益
void sco_app_set_post_gain(u8 gain);
//通话APP设置降噪算法使能
void sco_app_set_nr_en(bool en);
//通话APP设置降噪算法参数
void sco_app_set_nr_level(u8 level);
//通话APP设置汽车喇叭声降噪使能
void sco_app_set_trumpet_nr_en(bool en);
//通话APP设置AEC算法使能
void sco_app_set_aec_en(bool en);
//通话APP设置AEC回声消除级别
void sco_app_set_aec_echo_level(u8 level);
//通话APP设置AEC远端补偿值
void sco_app_set_aec_far_offset(u8 offset);

//公共响应: 直接返回 SCO_APP_SUCCESS or SCO_APP_FAIL
void sco_app_request_common_response(sco_app_result_t result);
//清空payload
void sco_app_request_tlv_payload_clear(void);
//(SCO_APP_CMD) response: payload填充TLV包，输入为值
u16 sco_app_request_tlv_response_fill(u16 tlv_type, sco_app_result_t result);
//(SCO_INF_SEND) Information tx: payload填充TLV包，输入为值
u16 sco_app_tx_tlv_fill(enum SCO_INF_SEND inf, u16 len, u8 value);
//(SCO_INF_SEND) Information tx: payload填充TLV包，输入为指针
u16 sco_app_tx_tlv_fill_ptr(enum SCO_INF_SEND inf, u16 len, const u8 *value);
//(SCO_INF_SEND) Information tx: payload填充TLV嵌TLV的包，TLV包里面嵌TLV，输入为值
u16 sco_app_tx_tlv2tlv_fill(enum TLV_LAYER layer, enum SCO_INF_SEND inf, u16 len, u8 value);
//(SCO_INF_SEND) Information tx: payload填充TLV嵌TLV的包，TLV包里面嵌TLV，输入为指针
u16 sco_app_tx_tlv2tlv_fill_ptr(enum TLV_LAYER layer, enum SCO_INF_SEND inf, u16 len, u8 *value);
//(SCO_INF_SEND) Information tx: payload重新填充
void sco_app_tx_tlv2tlv_rewrite(enum TLV_LAYER layer, enum SCO_INF_SEND inf, u16 trace_len, u16 len, u8 value);
//response do: payload buf tx process
void sco_app_request_tlv_response_do(void);

//NOTIFY
void bt_sco_app_dbg_notify_do(u8* payload,u8 len);
void bt_sco_app_dbg_notify_call_status(u8 status);

#endif
