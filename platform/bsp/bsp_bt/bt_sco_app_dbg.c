#include "bt_sco_app_dbg.h"

#if BT_SCO_APP_DBG_EN

#define TRACE_EN                1

#if TRACE_EN
    #define TRACE(...)              printf(__VA_ARGS__)
    #define TRACE_R(...)            print_r(__VA_ARGS__)
#else
    #define TRACE(...)
    #define TRACE_R(...)
#endif

#define SPP_TX_BUF_LEN      128     //spp tx buff

sco_app_cmd_recv_t sco_app_cmd_recv AT(.nr_buf.debug.recv);
sco_app_cmd_send_t sco_app_cmd_send AT(.nr_buf.debug.send);

AT(.text.sco_app.inf)
static const uint8_t chip_type_inf[] = { 'A', 'B', '5', '6', '3', 'X' };

AT(.text.sco_app.inf)
static const uint8_t sdk_version_inf[] = { '8', '9', '2', 'X', 'A', '2', '.' , 'v' , '0' , '1' , 'x', ' ', '&', ' ', '5', '6', '3', 'x'};

void bt_sco_dbg_connect_ack(void)
{
    bt_spp_tx(SPP_SERVICE_CH0, (uint8_t*)chip_type_inf, sizeof(chip_type_inf));
}

AT(.com_text.spp)
int spp_tx_buff_get_len(void)   //bt库的weak函数
{
    return SPP_TX_BUF_LEN;
}

AT(.text.sco_app_dbg.app.mtu)
u16 get_sco_spp_mtu_size(void)
{
    return spp_tx_buff_get_len();
}

AT(.text.sco_app_dbg.app.mtu)
static u16 sco_app_mtu_get(void)
{
    u16 mtu = 0;
    mtu = get_sco_spp_mtu_size();
    return mtu;
}

AT(.text.sco_app_dbg.app.response)
void sco_app_data_send(u8* buf, u16 len)
{
    u16 mtu = sco_app_mtu_get() - SOC_APP_HEADER_LEN;
    u8 total_frame = 0;
    u8 cur_frame = 0;
    u16 data_len = len - SOC_APP_HEADER_LEN;
    u8 *p_data = buf;
    u16 send_len;

    if(data_len > mtu){
        total_frame = (data_len / mtu) + (data_len % mtu ? 1 : 0) - 1;
    }
    sco_app_cmd_send.cmd_head.frame_total = total_frame;

    do{
        send_len = data_len > mtu ? mtu : data_len;
        sco_app_cmd_send.cmd_head.payload_len   = send_len;
        sco_app_cmd_send.cmd_head.frame_seq     = cur_frame++;
        memcpy(p_data, &sco_app_cmd_send.cmd_head, SOC_APP_HEADER_LEN);
        bt_spp_tx(SPP_SERVICE_CH0, p_data, send_len + SOC_APP_HEADER_LEN);
        TRACE("tx[%d][%d]:\n", total_frame, send_len);
        TRACE_R(p_data, send_len + SOC_APP_HEADER_LEN);
        sco_app_cmd_send.cmd_head.seq++;
        data_len    -= send_len;
        p_data      += send_len;
    }while(total_frame--);
}

AT(.text.sco_app_dbg.msg)
void sco_app_msg_deal(u8 msg)
{
    TRACE("sco_app_msg_deal:%d\n", msg);
    switch (msg) {
        case EVT_SCO_DBG_COMM_RSP:
            sco_app_data_send((u8*)&sco_app_cmd_send, SOC_APP_HEADER_LEN + 1);
            sco_app_request_tlv_payload_clear();
            break;
        case EVT_SCO_DBG_TLV_RSP:
            sco_app_data_send((u8*)&sco_app_cmd_send, (SOC_APP_HEADER_LEN + sco_app_cmd_send.total_len));
            sco_app_request_tlv_payload_clear();
            break;
        case EVT_SCO_DBG_NOTIFY:
            sco_app_data_send((u8*)&sco_app_cmd_send, SOC_APP_HEADER_LEN + sco_app_cmd_send.cmd_head.payload_len);
            break;
    }

}

//公共响应: 直接返回 SCO_APP_SUCCESS or SCO_APP_FAIL
AT(.text.sco_app_dbg.app.response)
void sco_app_request_common_response(sco_app_result_t result)
{
    TRACE("%s\n", __func__);
    sco_app_cmd_send.total_len      = 1;
    sco_app_cmd_send.payload[0]     = result;

    msg_enqueue(EVT_SCO_DBG_COMM_RSP);
//    sco_app_data_send((u8*)&sco_app_cmd_send, SOC_APP_HEADER_LEN + 1);
//    sco_app_request_tlv_payload_clear();
}

//response do: payload buf tx process
AT(.text.sco_app_dbg.app.response.tlv)
void sco_app_request_tlv_response_do(void)
{
    TRACE("%s\n", __func__);
    msg_enqueue(EVT_SCO_DBG_TLV_RSP);
//    sco_app_data_send((u8*)&sco_app_cmd_send, (SOC_APP_HEADER_LEN + sco_app_cmd_send.total_len));
//    sco_app_request_tlv_payload_clear();
}

//清空payload
AT(.text.sco_app_dbg.app.response.tlv)
void sco_app_request_tlv_payload_clear(void)
{
    TRACE("%s\n", __func__);
    memset(sco_app_cmd_send.payload, 0, sizeof(sco_app_cmd_send.payload));
    sco_app_cmd_send.total_len = 0;
}

//(SCO_APP_CMD) response: payload填充TLV包，输入为值
AT(.text.sco_app_dbg.app.response.tlv)
u16 sco_app_request_tlv_response_fill(u16 tlv_type, sco_app_result_t result)
{
    u16 ret_payload_len  = sco_app_cmd_send.total_len;
    u8 *ret_payload     = &sco_app_cmd_send.payload[ret_payload_len];
    u8  len = 1;

    ret_payload[0] = tlv_type;
    ret_payload[1] = 1;
    ret_payload[2] = result;

    sco_app_cmd_send.total_len += len + 2;
    TRACE("payload tlv fill[%d]:\n", sco_app_cmd_send.total_len);
    TRACE_R(ret_payload, len+2);
    return (len + 2);
}

//(SCO_INF_SEND) Information tx: payload填充TLV包，输入为值
AT(.text.sco_app_dbg.app.response.tlv)
u16 sco_app_tx_tlv_fill(enum SCO_INF_SEND inf, u16 len, u8 value)
{
    u16 ret_payload_len  = sco_app_cmd_send.total_len;
    u8 *ret_payload     = &sco_app_cmd_send.payload[ret_payload_len];

    ret_payload[0] = inf;
    ret_payload[1] = len;
    ret_payload[2] = (len) ? value : 0;

    sco_app_cmd_send.total_len += len + 2;
    TRACE("payload tlv fill[%d]:\n", sco_app_cmd_send.total_len);
    TRACE_R(ret_payload, len + 2);
    return (len + 2);
}

//(SCO_INF_SEND) Information tx: payload填充TLV包，输入为指针
AT(.text.sco_app_dbg.app.response.tlv)
u16 sco_app_tx_tlv_fill_ptr(enum SCO_INF_SEND inf, u16 len, const u8 *value)
{
    u16 ret_payload_len  = sco_app_cmd_send.total_len;
    u8 *ret_payload     = &sco_app_cmd_send.payload[ret_payload_len];

    ret_payload[0] = inf;
    ret_payload[1] = len;
    memcpy(&ret_payload[2], value, len);
    sco_app_cmd_send.total_len += len + 2;
    TRACE("payload tlv ptr fill[%d]:\n", sco_app_cmd_send.total_len);
    TRACE_R(ret_payload, len + 2);
    return (len + 2);
}

//(SCO_INF_SEND) Information tx: payload填充TLV嵌TLV的包，TLV包里面嵌TLV，输入为值
AT(.text.sco_app_dbg.app.response.tlv)
u16 sco_app_tx_tlv2tlv_fill(enum TLV_LAYER layer, enum SCO_INF_SEND inf, u16 len, u8 value)
{
    u16 ret_payload_len  = sco_app_cmd_send.total_len;
    u8 *ret_payload     = &sco_app_cmd_send.payload[ret_payload_len];

    ret_payload[0] = inf;
    ret_payload[1] = len;

    if (layer == TLV_LAYER0) {
        sco_app_cmd_send.total_len += 2;
        TRACE("payload tlv2tlv layer0 fill[%d]:\n", sco_app_cmd_send.total_len);
        TRACE_R(ret_payload, 2);
        return (2);
    } else if (layer == TLV_LAYER1) {
        ret_payload[2] = value;
        sco_app_cmd_send.total_len += len + 2;
        TRACE("payload tlv2tlv layer1 fill[%d]:\n", sco_app_cmd_send.total_len);
        TRACE_R(ret_payload, len + 2);
        return (len + 2);
    }
    return (0);
}

//(SCO_INF_SEND) Information tx: payload填充TLV嵌TLV的包，TLV包里面嵌TLV，输入为指针
AT(.text.sco_app_dbg.app.response.tlv)
u16 sco_app_tx_tlv2tlv_fill_ptr(enum TLV_LAYER layer, enum SCO_INF_SEND inf, u16 len, u8 *value)
{
    u16 ret_payload_len  = sco_app_cmd_send.total_len;
    u8 *ret_payload     = &sco_app_cmd_send.payload[ret_payload_len];

    ret_payload[0] = inf;
    ret_payload[1] = len;

    if (layer == TLV_LAYER0) {
        sco_app_cmd_send.total_len += 2;
        TRACE("payload tlv2tlv layer0 ptr fill[%d]:\n", sco_app_cmd_send.total_len);
        TRACE_R(ret_payload, 2);
        return (2);
    } else if (layer == TLV_LAYER1) {
        memcpy(&ret_payload[2], value, len);
        sco_app_cmd_send.total_len += len + 2;
        TRACE("payload tlv2tlv layer1 ptr fill[%d]:\n", sco_app_cmd_send.total_len);
        TRACE_R(ret_payload, len + 2);
        return (len + 2);
    }
    return (0);
}

//(SCO_INF_SEND) Information tx: payload重新填充
AT(.text.sco_app_dbg.app.response.tlv)
void sco_app_tx_tlv2tlv_rewrite(enum TLV_LAYER layer, enum SCO_INF_SEND inf, u16 trace_len, u16 len, u8 value)
{
    u16 ret_payload_len  = sco_app_cmd_send.total_len;
    u8 *ret_payload     = &sco_app_cmd_send.payload[ret_payload_len-trace_len];

    ret_payload[0] = inf;
    ret_payload[1] = len;

    if (layer == TLV_LAYER0) {
        TRACE("payload tlv2tlv layer0 rewrite[%d]:\n", sco_app_cmd_send.total_len);
        TRACE_R(ret_payload, 2);
    } else if (layer == TLV_LAYER1) {
        ret_payload[2] = value;
        TRACE("payload tlv2tlv layer1 rewrite[%d]:\n", sco_app_cmd_send.total_len);
        TRACE_R(ret_payload, len + 2);
    }
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_chip_type(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    sco_app_tx_tlv_fill_ptr(CMD_SCO_INF_CHIP_TYPE, sizeof(chip_type_inf), chip_type_inf);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_sdk_version(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    sco_app_tx_tlv_fill_ptr(CMD_SCO_INF_SDK_VERSION, sizeof(sdk_version_inf), sdk_version_inf);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_mtu_value(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u16 mtu = sco_app_mtu_get();
    u8 value[] = {(mtu&0xff), (mtu>>8)};
    sco_app_tx_tlv_fill_ptr(CMD_SCO_INF_MTU_VALUE, sizeof(value), value);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_sys_clk(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    sco_app_tx_tlv_fill(CMD_SCO_INF_SYS_CLK, 1, get_cur_sysclk());
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_anl_gain(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER0, CMD_SCO_INF_ANL_GAIN, 3*2, 0);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_MIC_ANL_GAIN_MICL, 1, sco_app_var.mic_anl_gain);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_MIC_ANL_GAIN_MICR, 1, sco_app_var.mic_anl_gain);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_dig_gain(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER0, CMD_SCO_INF_DIG_GAIN, 3*2, 0);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_MIC_DIG_GAIN_MICL, 1, sco_app_var.mic_dig_gain);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_MIC_DIG_GAIN_MICR, 1, sco_app_var.mic_dig_gain);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_post_gain(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    sco_app_tx_tlv_fill(CMD_SCO_INF_POST_GAIN, 1, sco_app_var.mic_post_gain);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_trumpet_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER0, CMD_SCO_INF_TRUMPET_PARAM, 3*TLV_TRUMPET_NR_LEVEL, 0);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_TRUMPET_NR_EN, 1, sco_app_var.bt_voice.nr.dump_en);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_TRUMPET_NR_LEVEL, 1, 0);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_far_nr_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER0, CMD_SCO_INF_FAR_NR_PARAM, 3*TLV_FAR_NR_LEVEL, 0);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_FAR_NR_EN, 1, (u8)bt_sco_dbg_get_alg_status(SCO_DBG_FAR_NR));
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_FAR_NR_TYPE, 1, SCO_APP_FAR_NR_ANS);
    far_nr_cfg_t *far_nr_cfg = (far_nr_cfg_t *)sco_app_var.bt_voice.nr.far_nr;
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_FAR_NR_LEVEL, 1, far_nr_cfg->level);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_aec_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u8 aec_en = bt_sco_dbg_get_alg_status(SCO_DBG_AEC_LP) & bt_sco_dbg_get_alg_status(SCO_DBG_AEC_NLP);

    sco_app_tx_tlv2tlv_fill(TLV_LAYER0, CMD_SCO_INF_AEC_PARAM, 3*TLV_AEC_AEC_MODE, 0);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_AEC_EN, 1, aec_en);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_AEC_TYPE, 1, SCO_APP_AEC);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_AEC_LP_EN, 1, bt_sco_dbg_get_alg_status(SCO_DBG_AEC_LP));
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_AEC_NLP_EN, 1, bt_sco_dbg_get_alg_status(SCO_DBG_AEC_NLP));
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_AEC_ECHO_LEVEL, 1, sco_app_var.bt_voice.aec.echo_th);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_AEC_FAR_OFFSET, 1, sco_app_var.bt_voice.aec.far_offset);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_AEC_AEC_MODE, 1, sco_app_var.bt_voice.nr.nr_type);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_enc_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER0, CMD_SCO_INF_ENC_PARAM, 3*TLV_ENC_TYPE, 0);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_ENC_EN, 1, (u8)bt_sco_dbg_get_alg_status(SCO_DBG_NR));
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_ENC_TYPE, 1, SCO_APP_AINS2);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_per_eq_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_per_drc_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_post_eq_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u16 tx_tlv_len = 0;
    u16 tx_trace_len = 0;

    u8 rx_tlv_type  = payload[0];
    u8 rx_tlv_len   = payload[1];
    u8 *rx_tlv_val  = &payload[2];
    s16 read_len    = 0;

    eq_param_t *eq_param = NULL;
    u32 *post_eq_param = NULL;
    bool eq_msbc_en = TLV_SCO_CODEC_MSBC;

    if (payload_len == 0) {     //当payload_len为空时，默认发MSBC的EQ曲线
        rx_tlv_type    = TLV_POST_EQ_MSBC;
        rx_tlv_val[0]  = TLV_SCO_CODEC_MSBC;
        payload_len = 1;
    }

    while (read_len < payload_len) {
        if (rx_tlv_type == TLV_POST_EQ_MSBC) {
            if (rx_tlv_val[0] == TLV_SCO_CODEC_CVSD) {
                eq_msbc_en = TLV_SCO_CODEC_CVSD;
                eq_param = &sco_app_var.post_eq_8k;
                post_eq_param = sco_app_var.post_eq_8k_param;
            } else if (rx_tlv_val[0] == TLV_SCO_CODEC_MSBC) {
                eq_msbc_en = TLV_SCO_CODEC_MSBC;
                eq_param = &sco_app_var.post_eq_16k;
                post_eq_param = sco_app_var.post_eq_16k_param;
            }

            if (eq_param != NULL) {
                tx_trace_len = sco_app_tx_tlv2tlv_fill(TLV_LAYER0, CMD_SCO_INF_POST_EQ, 2, 0);
                tx_tlv_len += sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_POST_EQ_EN, 1, (u8)bt_sco_dbg_get_alg_status(SCO_DBG_POST_EQ));
                tx_tlv_len += sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_POST_EQ_MSBC, 1, eq_msbc_en);
                tx_tlv_len += sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_POST_EQ_BAND_CNT, 1, eq_param->band_cnt);
                tx_tlv_len += sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_POST_EQ_VERSION, 1, eq_param->version);
//                tx_tlv_len += sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_POST_EQ_SAMPLE_RATE, 1, eq_param->sample);
                tx_tlv_len += sco_app_tx_tlv2tlv_fill_ptr(TLV_LAYER1, TLV_POST_EQ_GAIN, 4, (u8*)&post_eq_param[0]);
                if (eq_param->band_cnt && (post_eq_param != NULL)) {
                    u16 param_len = (eq_param->band_cnt*5)*sizeof(u32);
                    tx_tlv_len += sco_app_tx_tlv2tlv_fill_ptr(TLV_LAYER1, TLV_POST_EQ_PARAM, param_len, (u8*)&post_eq_param[1]);
                }
                tx_trace_len += tx_tlv_len;
                sco_app_tx_tlv2tlv_rewrite(TLV_LAYER0, CMD_SCO_INF_POST_EQ, tx_trace_len, tx_tlv_len, 0);
            }
        }

        read_len += (rx_tlv_len + 2);
        rx_tlv_type =  payload[read_len];
        rx_tlv_len  =  payload[read_len + 1];
        rx_tlv_val  = &payload[read_len + 2];
    }
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_post_drc_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u16 tx_tlv_len = 0;
    u16 tx_trace_len = 0;

    u8 rx_tlv_type  = payload[0];
    u8 rx_tlv_len   = payload[1];
    u8 *rx_tlv_val  = &payload[2];
    s16 read_len    = 0;

    u8 *post_drc_param = NULL;
    bool drc_msbc_en = TLV_SCO_CODEC_MSBC;
//    u8 sample_rate = 0x00;

    if (payload_len == 0) {     //当payload_len为空时，默认发MSBC的EQ曲线
        rx_tlv_type     = TLV_POST_DRC_MSBC;
        rx_tlv_val[0]   = TLV_SCO_CODEC_MSBC;
        payload_len     = 1;
    }

    while (read_len < payload_len) {
        if (rx_tlv_type == TLV_POST_DRC_MSBC) {
            if (rx_tlv_val[0] == TLV_SCO_CODEC_CVSD) {
                drc_msbc_en = TLV_SCO_CODEC_CVSD;
//                sample_rate = TLV_SCO_SAMPLE_RATE_8K;
                post_drc_param = sco_app_var.post_drc_8k_res;
            } else if (rx_tlv_val[0] == TLV_SCO_CODEC_MSBC) {
                drc_msbc_en = TLV_SCO_CODEC_MSBC;
//                sample_rate = TLV_SCO_SAMPLE_RATE_16K;
                post_drc_param = sco_app_var.post_drc_16k_res;
            }

            if (post_drc_param != NULL) {
                tx_trace_len = sco_app_tx_tlv2tlv_fill(TLV_LAYER0, CMD_SCO_INF_POST_DRC, 2, 0);
                tx_tlv_len += sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_POST_DRC_EN, 1, (u8)bt_sco_dbg_get_alg_status(SCO_DBG_POST_DRC));
                tx_tlv_len += sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_POST_DRC_CNT, 1, 4);
                tx_tlv_len += sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_POST_DRC_VERSION, 1, 0x03);
//                tx_tlv_len += sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_POST_DRC_SAMPLE_RATE, 1, sample_rate);
                tx_tlv_len += sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_POST_DRC_MSBC, 1, drc_msbc_en);
                if (bt_sco_dbg_get_alg_status(SCO_DBG_POST_DRC)) {
                    u16 param_len = sizeof(sco_app_var.post_drc_16k_res);
                    tx_tlv_len += sco_app_tx_tlv2tlv_fill_ptr(TLV_LAYER1, TLV_POST_DRC_PARAM, param_len, (u8*)post_drc_param);
                }
                tx_trace_len += tx_tlv_len;
                sco_app_tx_tlv2tlv_rewrite(TLV_LAYER0, CMD_SCO_INF_POST_DRC, tx_trace_len, tx_tlv_len, 0);
            }
        }

        read_len += (rx_tlv_len + 2);
        rx_tlv_type =  payload[read_len];
        rx_tlv_len  =  payload[read_len + 1];
        rx_tlv_val  = &payload[read_len + 2];
    }
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_ans_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_ains_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_ains2_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER0, CMD_SCO_INF_AINS2_PARAM, 3*TLV_ENC_AINS2_LEVEL, 0);
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_ENC_AINS2_EN, 1, (u8)bt_sco_dbg_get_alg_status(SCO_DBG_NR));
    sco_app_tx_tlv2tlv_fill(TLV_LAYER1, TLV_ENC_AINS2_LEVEL, 1, sco_app_var.bt_voice.nr.level);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_sndp_rnn_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_sndp_dnn_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_sndp_dmic_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_sndp_dmdnn_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_dnn_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_dmns_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.app)
static void sco_app_tx_dmdnn_param(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

static const void (*sco_app_inf_tx_func_api[])(u8 *payload,u8 payload_len) =
{
    sco_app_tx_chip_type,
    sco_app_tx_sdk_version,
    sco_app_tx_mtu_value,
    sco_app_tx_sys_clk,
    sco_app_tx_anl_gain,
    sco_app_tx_dig_gain,
    sco_app_tx_post_gain,
    sco_app_tx_trumpet_param,
    sco_app_tx_far_nr_param,
    sco_app_tx_aec_param,
    sco_app_tx_enc_param,
    sco_app_tx_per_eq_param,
    sco_app_tx_per_drc_param,
    sco_app_tx_post_eq_param,
    sco_app_tx_post_drc_param,
    sco_app_tx_ans_param,
    sco_app_tx_ains_param,
    sco_app_tx_ains2_param,
    sco_app_tx_sndp_rnn_param,
    sco_app_tx_sndp_dnn_param,
    sco_app_tx_sndp_dmic_param,
    sco_app_tx_sndp_dmdnn_param,
    sco_app_tx_dnn_param,
    sco_app_tx_dmns_param,
    sco_app_tx_dmdnn_param,
};

//SCO_INF_SEND 枚举变量空间映射到 sco_app_inf_tx_func_api 接口函数
AT(.text.sco_app_dbg.app)
static int get_inf_tx_func_api_idx(enum SCO_INF_SEND api_idx)
{
    int api_len1, api_len2, api_ret=0xff;
    api_len1 = CMD_SCO_INF_ENC_PARAM - CMD_SCO_INF_CHIP_TYPE + 1;
    api_len2 = api_len1 + (CMD_SCO_INF_POST_DRC - CMD_SCO_INF_PER_EQ + 1);

    if (api_idx <= CMD_SCO_INF_ENC_PARAM) {
        api_ret = (api_idx - (CMD_SCO_INF_CHIP_TYPE));
    } else if (api_idx <= CMD_SCO_INF_POST_DRC) {
        api_ret = (api_idx - (CMD_SCO_INF_PER_EQ) + api_len1);
    } else if (api_idx <= CMD_SCO_INF_DMDNN_PARAM) {
        api_ret = (api_idx - (CMD_SCO_INF_ANS_PARAM) + api_len2);
    }

    if (api_ret > (sizeof(sco_app_inf_tx_func_api)/4)) {
        printf("api_ret: %d, range:[%d, %d]\n", api_ret, 0, sizeof(sco_app_inf_tx_func_api)/4);
        printf("sco app information tx api ERROR: api_id out of range.(sco_app_inf_tx_func_api)\n");
        return -1;
    }
//    TRACE("api_ret:%d\n", api_ret);
    return api_ret;
}

AT(.text.sco_app_dbg.cmd)
void sco_app_inf_send(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u8 tlv_type     = payload[0];
    u8 tlv_len      = payload[1];
    u8 *tlv_val     = &payload[2];
    s16 read_len    = 0;
    int api_set;
    void (*inf_tx_func_api)(u8 *payload,u8 payload_len);

//    sco_app_cmd_send.total_len = 0;
    while (read_len < payload_len) {
        //TX API
        api_set = get_inf_tx_func_api_idx(tlv_type);
        if (api_set >= 0) {
            inf_tx_func_api = (void (*))sco_app_inf_tx_func_api[api_set];
            inf_tx_func_api(tlv_val, tlv_len);
        }

        read_len += (tlv_len + 2);
        tlv_type    =  payload[read_len];
        tlv_len     =  payload[read_len + 1];
        tlv_val     = &payload[read_len + 2];
    }

    sco_app_request_tlv_response_do();
}

AT(.text.sco_app_dbg.app)
static void sco_app_request_receive_proc(u8 cmd,u8 *payload,u8 payload_len)
{
    int api_set;
    void (*cmd_func_api)(u8 *payload,u8 payload_len);

    sco_app_cmd_send.cmd_head.cmd           = cmd;
    sco_app_cmd_send.cmd_head.cmd_type      = SCO_CMD_TYPE_RESPONSE;
    sco_app_cmd_send.cmd_head.frame_seq     = 0;
    sco_app_cmd_send.cmd_head.frame_total   = 0;
    sco_app_request_tlv_payload_clear();

    //CMD API
    api_set = get_cmd_func_api_idx(cmd);
    if (api_set >= 0) {
        cmd_func_api = (void (*))sco_app_cmd_func_api[api_set];
        cmd_func_api(payload, payload_len);
    }
}

AT(.text.sco_app_dbg.app)
static void sco_app_receive_proc_do(void)
{
    switch(sco_app_cmd_recv.cmd_head.cmd_type){
        case SCO_CMD_TYPE_REQUEST:
            sco_app_request_receive_proc(sco_app_cmd_recv.cmd_head.cmd, sco_app_cmd_recv.payload, sco_app_cmd_recv.total_len);
            break;
        default:
            break;
    }
}

AT(.text.sco_app_dbg.app.proc)
bool bt_sco_app_dbg_proc(u8 *data,u8 len)
{
    TRACE("bt_sco_app_dbg_proc[%d]:",len);
    TRACE_R(data, len);

    sco_app_cmd_head_t* cmd_head = (sco_app_cmd_head_t*)data;
    u8 *p_data = &data[SOC_APP_PAYLOAD_POS];
    u8 payload_len = cmd_head->payload_len;

    if(cmd_head->cmd_type > SCO_CMD_TYPE_NOTIFY){
        return false;
    }

    if(payload_len != (len - SOC_APP_HEADER_LEN)){
        return false;
    }

    if(sco_app_cmd_recv.next_header_seq != cmd_head->seq){
        TRACE("--->header_seq_err:%d,%d\n",sco_app_cmd_recv.next_header_seq,cmd_head->seq);
        sco_app_cmd_recv.next_header_seq = cmd_head->seq;
    }

    sco_app_cmd_recv.next_header_seq++;

    if(cmd_head->frame_total == 0){
        TRACE("sco_app_receive[%d]:",len);
        TRACE_R(data,len);
        if (payload_len > SCO_APP_RX_PAYLOAD_LEN) {
            TRACE("ERROR: rx payload_len overflow: [%d] > [%d]\n", payload_len, SCO_APP_RX_PAYLOAD_LEN);
        }
        memcpy(&sco_app_cmd_recv.cmd_head, cmd_head, SOC_APP_HEADER_LEN);
        memcpy(&sco_app_cmd_recv.payload, p_data, payload_len);
        sco_app_cmd_recv.total_len = payload_len;
        sco_app_receive_proc_do();
    }else{
        if(sco_app_cmd_recv.next_frame_seq != cmd_head->frame_seq){
            TRACE("--->frame_seq_err\n");
            sco_app_cmd_recv.next_frame_seq = 0;
            return false;
        }
        sco_app_cmd_recv.next_frame_seq++;
        memcpy(&sco_app_cmd_recv.payload[sco_app_cmd_recv.recv_len], p_data, payload_len);
        sco_app_cmd_recv.recv_len += payload_len;
        if(cmd_head->frame_seq == cmd_head->frame_total){
            memcpy(&sco_app_cmd_recv.cmd_head, cmd_head, SOC_APP_HEADER_LEN);
            sco_app_cmd_recv.total_len = sco_app_cmd_recv.recv_len;
            sco_app_cmd_recv.next_frame_seq = 0;
            sco_app_cmd_recv.recv_len = 0;
            sco_app_receive_proc_do();
        }
    }

    return true;
}


AT(.text.sco_app_dbg.app.init)
void bt_sco_app_dbg_init(void)
{
    printf("%s\n", __func__);
    memset(&sco_app_cmd_recv, 0, sizeof(sco_app_cmd_recv_t));
    memset(&sco_app_cmd_send, 0, sizeof(sco_app_cmd_send_t));

    bt_sco_nr_dbg_init();
}


///NOTIFY
AT(.text.sco_app_dbg.notify)
void bt_sco_app_dbg_notify_do(u8* payload,u8 len)
{
    sco_app_cmd_send.cmd_head.cmd = CMD_SCO_NOTIFY;
    sco_app_cmd_send.cmd_head.cmd_type = SCO_CMD_TYPE_NOTIFY;
    sco_app_cmd_send.cmd_head.frame_seq = 0;
    sco_app_cmd_send.cmd_head.frame_total = 0;
    sco_app_cmd_send.cmd_head.payload_len = len;
    memcpy(sco_app_cmd_send.payload, payload, len);

    msg_enqueue(EVT_SCO_DBG_NOTIFY);
//    sco_app_data_send((u8*)&sco_app_cmd_send, SOC_APP_HEADER_LEN + len);
}

AT(.text.sco_app_dbg.notify.call)
void bt_sco_app_dbg_notify_call_status(u8 status)
{
    u8 payload[3];
    u8 *tlv_type     = &payload[0];
    u8 *tlv_len      = &payload[1];
    u8 *tlv_val     = &payload[2];

    *tlv_type = CMD_SCO_NOTIFY_CALL_STATUS;
    *tlv_len  = 0x01;
    *tlv_val  = status;
    bt_sco_app_dbg_notify_do(payload, 3);
}

AT(.text.sco_app_dbg.notify.call.status)
void bt_sco_app_dbg_call_status(void)
{
    bt_sco_app_dbg_notify_call_status(TLV_CALL_STATUS_NONE);
}

#endif // BT_SCO_APP_DBG_EN
