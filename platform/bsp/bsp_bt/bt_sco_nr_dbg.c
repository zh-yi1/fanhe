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

static u16 cfg_sco_debug = 0xffff;              //通话调试全局变量，每一BIT的作用参考bt_sco.h中的SCO_DBG枚举
static u16 cfg_sco_debug_init_val;              //备份初始值，当对应功能的初始值为0时，不能开关该功能
sco_app_var_t sco_app_var;
static bool cfg_sco_dbg_init_first = true;
static void sco_app_dbg_var_set_do(u32 type);

AT(.text.sco_app_dbg.res_tbl)
const u32 soc_app_res_tbl[4][2] = {
    {(u32)&RES_BUF_EQ_BT_MIC_8K_EQ, (u32)&RES_LEN_EQ_BT_MIC_8K_EQ},
    {(u32)&RES_BUF_EQ_BT_MIC_16K_EQ, (u32)&RES_LEN_EQ_BT_MIC_16K_EQ},
    {(u32)&RES_BUF_DRC_BT_MIC_8K_DRC, (u32)&RES_LEN_DRC_BT_MIC_8K_DRC},
    {(u32)&RES_BUF_DRC_BT_MIC_16K_DRC, (u32)&RES_LEN_DRC_BT_MIC_16K_DRC},
};

AT(.text.sco_app_dbg.nr_func)
static bool bt_sco_dbg_alg_en(sco_dbg_t type, bool enable)
{
    if (cfg_sco_debug_init_val & type) {        //当对应功能的初始值为1时，可以开关该功能
        if (enable) {
            cfg_sco_debug |= type;
//            TRACE("Alg[%x] open\n", type);
        } else {
            cfg_sco_debug &= ~type;
//            TRACE("Alg[%x] close\n", type);
        }
        return true;
    } else {                                    //当对应功能的初始值为0时，不能开关该功能
        return false;
    }
}

AT(.bt_voice.sco.debug.alg_status)
bool bt_sco_dbg_get_alg_status(sco_dbg_t type)
{
    if (cfg_sco_debug & type) {
//        TRACE("Alg[%x] is opened\n", type);
        return true;
    } else {
//        TRACE("Alg[%x] is closed\n", type);
        //output unchanged
        return false;
    }
}

//通话APP设置当前系统时钟
AT(.text.sco_app_dbg.cmd)
static void sco_app_sys_clk_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    if (sco_is_connected()) {
        u8 sys_clk = payload[0];
        sys_clk_set(sys_clk);
        sco_app_request_common_response(SCO_APP_SUCCESS);
        TRACE("sys_clk: %d\n", get_cur_sysclk());
    } else {
        sco_app_request_common_response(SCO_APP_FAIL);
    }
}

//通话APP设置MIC模拟增益
AT(.text.sco_app_dbg.cmd)
static void sco_app_mic_anl_gain_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u8 tlv_type     = payload[0];
    u8 tlv_len      = payload[1];
    u8 *tlv_val     = &payload[2];
    s16 read_len    = 0;

    while (read_len < payload_len) {
        switch (tlv_type) {
            case TLV_MIC_ANL_GAIN_MICL:
                sco_app_var.mic_anl_gain = tlv_val[0];
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            case TLV_MIC_ANL_GAIN_MICR:
                sco_app_var.mic_anl_gain = tlv_val[0];
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            default:
                sco_app_request_tlv_response_fill(0x00, SCO_APP_FAIL);
        }
        read_len   += (tlv_len + 2);
        tlv_type    =  payload[read_len];
        tlv_len     =  payload[read_len + 1];
        tlv_val     = &payload[read_len + 2];
    }

    sco_app_dbg_var_set_do(CMD_MIC_ANL_GAIN_SET);
    sco_app_request_tlv_response_do();
}

//通话APP设置MIC数字增益
AT(.text.sco_app_dbg.cmd)
static void sco_app_mic_dig_gain_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u8 tlv_type     = payload[0];
    u8 tlv_len      = payload[1];
    u8 *tlv_val     = &payload[2];
    s16 read_len    = 0;

    while (read_len < payload_len) {
        switch (tlv_type) {
            case TLV_MIC_DIG_GAIN_MICL:
                sco_app_var.mic_dig_gain = tlv_val[0];
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            case TLV_MIC_DIG_GAIN_MICR:
                sco_app_var.mic_dig_gain = tlv_val[0];
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            default:
                sco_app_request_tlv_response_fill(0x00, SCO_APP_FAIL);
                break;
        }
        read_len   += (tlv_len + 2);
        tlv_type    =  payload[read_len];
        tlv_len     =  payload[read_len + 1];
        tlv_val     = &payload[read_len + 2];
    }

    sco_app_dbg_var_set_do(CMD_MIC_DIG_GAIN_SET);
    sco_app_request_tlv_response_do();
}

//通话APP设置汽车喇叭声降噪参数
AT(.text.sco_app_dbg.cmd)
static void sco_app_trumpet_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u8 tlv_type     = payload[0];
    u8 tlv_len      = payload[1];
    u8 *tlv_val     = &payload[2];
    s16 read_len    = 0;
    bool alg_en;

    while (read_len < payload_len) {
        switch (tlv_type) {
            case TLV_TRUMPET_NR_EN:
                //sco_app_var.bt_voice.near_nr.trumpet_en = tlv_val[0];
                alg_en = (bool)tlv_val[0];
                if (bt_sco_dbg_alg_en(SCO_DBG_TRUMPET, alg_en)) {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                } else {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                }
                break;

            case TLV_TRUMPET_NR_LEVEL:
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                break;

            default:
                sco_app_request_tlv_response_fill(0x00, SCO_APP_FAIL);
                break;
        }
        read_len   += (tlv_len + 2);
        tlv_type    =  payload[read_len];
        tlv_len     =  payload[read_len + 1];
        tlv_val     = &payload[read_len + 2];
    }

    sco_app_dbg_var_set_do(CMD_TRUMPET_NR_SET);
    sco_app_request_tlv_response_do();
}

//通话APP设置后置增益
AT(.text.sco_app_dbg.cmd)
static void sco_app_post_gain_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    sco_app_var.mic_post_gain = payload[0];
    sco_app_request_tlv_response_fill(0x01, SCO_APP_SUCCESS);

    sco_app_dbg_var_set_do(CMD_POST_GIAN_SET);
    sco_app_request_tlv_response_do();
}

//通话APP设置远端降噪算法参数
AT(.text.sco_app_dbg.cmd)
static void sco_app_far_nr_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u8 tlv_type     = payload[0];
    u8 tlv_len      = payload[1];
    u8 *tlv_val     = &payload[2];
    s16 read_len    = 0;
    bool alg_en;

    while (read_len < payload_len) {
        switch (tlv_type) {
            case TLV_FAR_NR_EN:
                alg_en = (bool)tlv_val[0];
                if (bt_sco_dbg_alg_en(SCO_DBG_FAR_NR, alg_en)) {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                } else {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                }
                break;

            case TLV_FAR_NR_TYPE:
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                break;

            case TLV_FAR_NR_LEVEL:
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                break;

            default:
                sco_app_request_tlv_response_fill(0x00, SCO_APP_FAIL);
                break;
        }
        read_len   += (tlv_len + 2);
        tlv_type    =  payload[read_len];
        tlv_len     =  payload[read_len + 1];
        tlv_val     = &payload[read_len + 2];
    }

    sco_app_dbg_var_set_do(CMD_FAR_NR_SET);
    sco_app_request_tlv_response_do();
}

//通话APP设置AEC算法参数
AT(.text.sco_app_dbg.cmd)
static void sco_app_aec_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u8 tlv_type     = payload[0];
    u8 tlv_len      = payload[1];
    u8 *tlv_val     = &payload[2];
    s16 read_len    = 0;
    bool alg_en;

    while (read_len < payload_len) {
        switch (tlv_type) {
            case TLV_AEC_EN:
                alg_en = (bool)tlv_val[0];
                if (bt_sco_dbg_alg_en(SCO_DBG_AEC_LP, alg_en) && bt_sco_dbg_alg_en(SCO_DBG_AEC_NLP, alg_en)) {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                } else {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                }
                break;

            case TLV_AEC_TYPE:
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                break;

            case TLV_AEC_LP_EN:
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                break;

            case TLV_AEC_NLP_EN:
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                break;

            case TLV_AEC_ECHO_LEVEL:
                sco_app_var.bt_voice.aec.nlp_level = tlv_val[0];
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            case TLV_AEC_FAR_OFFSET:
                sco_app_var.bt_voice.aec.far_offset = tlv_val[0];
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            case TLV_AEC_AEC_MODE:
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                break;

            default:
                sco_app_request_tlv_response_fill(0x00, SCO_APP_FAIL);
                break;
        }
        read_len   += (tlv_len + 2);
        tlv_type    =  payload[read_len];
        tlv_len     =  payload[read_len + 1];
        tlv_val     = &payload[read_len + 2];
    }

    sco_app_dbg_var_set_do(CMD_AEC_SET);
    sco_app_request_tlv_response_do();
}

//通话APP设置降噪算法参数
AT(.text.sco_app_dbg.cmd)
static void sco_app_enc_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u8 tlv_type     = payload[0];
    u8 tlv_len      = payload[1];
    u8 *tlv_val     = &payload[2];
    s16 read_len    = 0;
    bool alg_en;

    while (read_len < payload_len) {
        switch (tlv_type) {
            case TLV_ENC_EN:
                alg_en = (bool)tlv_val[0];
                if (bt_sco_dbg_alg_en(SCO_DBG_NR, alg_en)) {
                    sco_app_var.bt_voice.nr.nr_cfg_en = tlv_val[0];
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                } else {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                }
                break;

            case TLV_ENC_TYPE:
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                break;

            default:
                sco_app_request_tlv_response_fill(0x00, SCO_APP_FAIL);
                break;
        }
        read_len   += (tlv_len + 2);
        tlv_type    =  payload[read_len];
        tlv_len     =  payload[read_len + 1];
        tlv_val     = &payload[read_len + 2];
    }

    sco_app_dbg_var_set_do(CMD_ENC_SET);
    sco_app_request_tlv_response_do();
}

AT(.text.sco_app_dbg.cmd)
static void sco_app_pre_eq_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.cmd)
static void sco_app_pre_drc_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

//AT(.text.sco_app_dbg.eq_convert)
//static void sco_app_post_eq_param_convert(u8 band_cnt, u8 *rptr)
//{
//    int i;
//    u32 (*ptr)[5] = (u32 (*)[5])rptr;
//    u32 temp[5];
//    for (i=0; i<band_cnt; i++) {
//        memcpy(temp, ptr[i], sizeof(temp));
//        ptr[i][0] = temp[0];
//        ptr[i][1] = temp[2];
//        ptr[i][2] = temp[3];
//        ptr[i][3] = temp[1];
//        ptr[i][4] = temp[4];
//    }
//}

//通话APP设置算法后EQ
AT(.text.sco_app_dbg.cmd)
static void sco_app_post_eq_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u8 tlv_type     = payload[0];
    u8 tlv_len      = payload[1];
    u8 *tlv_val     = &payload[2];
    s16 read_len    = 0;
    bool alg_en;
    bool eq_msbc_en = 0;
//    u8 sample_rate  = 1;
    u8 band_cnt     = 0;
    u32 gain        = 0;
    u8 *rptr, *wptr;

    while (read_len < payload_len) {
        switch (tlv_type) {
            case TLV_POST_EQ_EN:
                alg_en = (bool)tlv_val[0];
                if (bt_sco_dbg_alg_en(SCO_DBG_POST_EQ, alg_en)) {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                } else {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                }
                break;

            case TLV_POST_EQ_MSBC:
                eq_msbc_en = (bool)tlv_val[0];
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            case TLV_POST_EQ_BAND_CNT:
                band_cnt = tlv_val[0];
                band_cnt = (band_cnt > 8) ? 8 : band_cnt;
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            case TLV_POST_EQ_VERSION:
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;
            case TLV_POST_EQ_SAMPLE_RATE:
//                sample_rate = tlv_val[0];
//                if (sample_rate == 0) {             //根据EQ采样率判断当前设置的EQ类型
//                    eq_msbc_en = 0;
//                } else if (sample_rate == 1) {
//                    eq_msbc_en = 1;
//                }
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;
            case TLV_POST_EQ_RFU:
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            case TLV_POST_EQ_GAIN:
                gain = ((u32*)tlv_val)[0];
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            case TLV_POST_EQ_PARAM:
                rptr = (u8*)&tlv_val[0];
                if (eq_msbc_en == TLV_SCO_CODEC_MSBC) {
                    sco_app_var.post_eq_16k.band_cnt = band_cnt;
                    sco_app_var.post_eq_16k_param[0] = gain;
                    wptr = (u8*)&sco_app_var.post_eq_16k_param[1];
                    memcpy(wptr, rptr, sco_app_var.post_eq_16k.band_cnt*(5*4));
                } else {
                    sco_app_var.post_eq_8k.band_cnt = band_cnt;
                    sco_app_var.post_eq_8k_param[0] = gain;
                    wptr = (u8*)&sco_app_var.post_eq_8k_param[1];
                    memcpy(wptr, rptr, sco_app_var.post_eq_8k.band_cnt*(5*4));
                }
//                sco_app_post_eq_param_convert(band_cnt, wptr);
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            default:
                sco_app_request_tlv_response_fill(0x00, SCO_APP_FAIL);
                break;
        }
        read_len   += (tlv_len + 2);
        tlv_type    =  payload[read_len];
        tlv_len     =  payload[read_len + 1];
        tlv_val     = &payload[read_len + 2];
    }

    sco_app_dbg_var_set_do(CMD_POST_EQ_SET);
    sco_app_request_tlv_response_do();
}

//通话APP设置算法后DRC
AT(.text.sco_app_dbg.cmd)
static void sco_app_post_drc_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u8 tlv_type     = payload[0];
    u8 tlv_len      = payload[1];
    u8 *tlv_val     = &payload[2];
    s16 read_len    = 0;
    bool alg_en;
    bool drc_msbc_en    = 0;
//    u8 sample_rate  = 1;
    u8 *rptr, *wptr;

    while (read_len < payload_len) {
        switch (tlv_type) {
            case TLV_POST_DRC_EN:
                alg_en = (bool)tlv_val[0];
                if (bt_sco_dbg_alg_en(SCO_DBG_POST_DRC, alg_en)) {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                } else {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                }
                break;

            case TLV_POST_DRC_VERSION:
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            case TLV_POST_DRC_SAMPLE_RATE:
//                sample_rate = tlv_val[0];
//                if (sample_rate == 0) {             //根据EQ采样率判断当前设置的EQ类型
//                    drc_msbc_en = 0;
//                } else if (sample_rate == 1) {
//                    drc_msbc_en = 1;
//                }
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            case TLV_POST_DRC_CNT:
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            case TLV_POST_DRC_MSBC:
                drc_msbc_en = tlv_val[0];
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            case TLV_POST_DRC_PARAM:
                rptr = (u8*)&tlv_val[0];
                if (drc_msbc_en == TLV_SCO_CODEC_MSBC) {
                    wptr = (u8*)sco_app_var.post_drc_16k_res;
                    memcpy(wptr, rptr, sizeof(sco_app_var.post_drc_16k_res));
                } else {
                    wptr = (u8*)sco_app_var.post_drc_8k_res;
                    memcpy(wptr, rptr, sizeof(sco_app_var.post_drc_8k_res));
                }
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            default:
                sco_app_request_tlv_response_fill(0x00, SCO_APP_FAIL);
                break;
        }
        read_len   += (tlv_len + 2);
        tlv_type    =  payload[read_len];
        tlv_len     =  payload[read_len + 1];
        tlv_val     = &payload[read_len + 2];
    }

    sco_app_dbg_var_set_do(CMD_POST_DRC_SET);
    sco_app_request_tlv_response_do();
}

AT(.text.sco_app_dbg.cmd)
static void sco_app_ans_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.cmd)
static void sco_app_ains_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.cmd)
static void sco_app_ains2_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
    u8 tlv_type     = payload[0];
    u8 tlv_len      = payload[1];
    u8 *tlv_val     = &payload[2];
    s16 read_len    = 0;
    bool alg_en;

    while (read_len < payload_len) {
        switch (tlv_type) {
            case TLV_ENC_AINS2_EN:
                alg_en = (bool)tlv_val[0];
                if (bt_sco_dbg_alg_en(SCO_DBG_NR, alg_en)) {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                } else {
                    sco_app_request_tlv_response_fill(tlv_type, SCO_APP_FAIL);
                }
                break;

            case TLV_ENC_AINS2_LEVEL:
                sco_app_var.bt_voice.nr.level = tlv_val[0];
                sco_app_request_tlv_response_fill(tlv_type, SCO_APP_SUCCESS);
                break;

            default:
                sco_app_request_tlv_response_fill(0x00, SCO_APP_FAIL);
                break;
        }
        read_len   += (tlv_len + 2);
        tlv_type    =  payload[read_len];
        tlv_len     =  payload[read_len + 1];
        tlv_val     = &payload[read_len + 2];
    }

    sco_app_dbg_var_set_do(CMD_ENC_SET);
    sco_app_request_tlv_response_do();
}

AT(.text.sco_app_dbg.cmd)
static void sco_app_sndp_rnn_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.cmd)
static void sco_app_sndp_dnn_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.cmd)
static void sco_app_sndp_dmic_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.cmd)
static void sco_app_sndp_dmdnn_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.cmd)
static void sco_app_dnn_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);

}

AT(.text.sco_app_dbg.cmd)
static void sco_app_dmns_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.cmd)
static void sco_app_dmdnn_set(u8 *payload,u8 payload_len)
{
    TRACE("%s\n", __func__);
}

//所有的CMD响应函数的API都放到这个表，因为算法设置的命令基本都在这里，所以放在这个文件
const void (*sco_app_cmd_func_api[])(u8 *payload,u8 payload_len) =
{
    sco_app_inf_send,
    sco_app_sys_clk_set,
    sco_app_mic_anl_gain_set,
    sco_app_mic_dig_gain_set,
    sco_app_trumpet_set,
    sco_app_post_gain_set,
    sco_app_far_nr_set,
    sco_app_aec_set,
    sco_app_enc_set,
    sco_app_pre_eq_set,
    sco_app_pre_drc_set,
    sco_app_post_eq_set,
    sco_app_post_drc_set,
    sco_app_ans_set,
    sco_app_ains_set,
    sco_app_ains2_set,
    sco_app_sndp_rnn_set,
    sco_app_sndp_dnn_set,
    sco_app_sndp_dmic_set,
    sco_app_sndp_dmdnn_set,
    sco_app_dnn_set,
    sco_app_dmns_set,
    sco_app_dmdnn_set,
};

//SCO_APP_CMD 枚举变量空间映射到 sco_app_cmd_func_api 接口函数
AT(.text.sco_app_dbg.api)
int get_cmd_func_api_idx(enum SCO_APP_CMD api_idx)
{
    int api_len1, api_len2, api_len3, api_ret=0xff;
    api_len1 = 1;
    api_len2 = api_len1 + (CMD_ENC_SET - CMD_SYS_CLK_SET + 1);
    api_len3 = api_len2 + (CMD_POST_DRC_SET - CMD_PRE_EQ_SET + 1);

    if (api_idx == CMD_SCO_INF_GET) {
        api_ret = 0;
    } else if (api_idx <= CMD_ENC_SET) {
        api_ret = (api_idx - (CMD_SYS_CLK_SET) + api_len1);
    } else if (api_idx <= CMD_POST_DRC_SET) {
        api_ret = (api_idx - (CMD_PRE_EQ_SET) + api_len2);
    } else if (api_idx <= CMD_NR_DMDNN_SET) {
        api_ret = (api_idx - (CMD_NR_ANS_SET) + api_len3);
    }

    if (api_ret > (sizeof(sco_app_cmd_func_api)/4)) {
        printf("api_ret: %d, range:[%d, %d]\n", api_ret, 0, sizeof(sco_app_cmd_func_api)/4);
        printf("sco app cmd api ERROR: api_id out of range.(get_cmd_func_api_idx)\n");
        return -1;
    }
//    TRACE("api_ret:%d\n", api_ret);
    return api_ret;
}

AT(.text.sco_app_dbg.nr_init)
static void sco_app_anl_gain_init(void)
{
    TRACE("%s\n", __func__);

    //重新初始化模拟增益
    set_mic_analog_gain(sco_app_var.mic_anl_gain, CH_MIC0);
    //bt_voice_init之后才初始化ADC，所以要在这里修改ADC初始化的增益，不然在非通话模式下修改增益会不起效
    sco_app_set_anl_gain(sco_app_var.mic_anl_gain);
}

AT(.text.sco_app_dbg.nr_init)
static void sco_app_dig_gain_init(void)
{
    TRACE("%s\n", __func__);
    u8 gain;

    //重新初始化数字增益
    gain = sco_app_var.mic_dig_gain;
    sdadc_set_digital_gain(gain & 0x3f);

    sco_app_set_dig_gain(sco_app_var.mic_dig_gain);
}

AT(.text.sco_app_dbg.nr_init)
static void sco_app_post_gain_init(void)
{
    TRACE("%s\n", __func__);
    //重新初始化后置增益
    sco_app_set_post_gain(sco_app_var.mic_post_gain);
}

AT(.text.sco_app_dbg.nr_init)
static void sco_app_trumpet_nr_init(void)
{
    TRACE("%s\n", __func__);
    //sco_app_set_trumpet_nr_en(sco_app_var.bt_voice.near_nr.trumpet_en);
}

AT(.text.sco_app_dbg.nr_init)
static void sco_app_aec_init(void)
{
    TRACE("%s\n", __func__);
    sco_app_set_aec_en(sco_app_var.bt_voice.aec.aec_en);
    sco_app_set_aec_echo_level(sco_app_var.bt_voice.aec.nlp_level);
    sco_app_set_aec_far_offset(sco_app_var.bt_voice.aec.far_offset);
}

AT(.text.sco_app_dbg.nr_init)
static void sco_app_enc_init(void)
{
    TRACE("%s\n", __func__);
    sco_app_set_nr_en(sco_app_var.bt_voice.nr.nr_cfg_en);
    sco_app_set_nr_level(sco_app_var.bt_voice.nr.level);
}

AT(.text.sco_app_dbg.nr_init)
static void sco_app_far_nr_init(void)
{
    TRACE("%s\n", __func__);
}

AT(.text.sco_app_dbg.nr_init)
static bool sco_app_eq_res_crc_check(u8 *ptr, eq_param_t *eq)
{
    u32 res_crc, cal_crc, band;
    if (ptr[0] == 'E' && ptr[1] == 'Q') {                               //V1: Q23
        band = ptr[3];
        res_crc = ptr[band*27+11] << 8 | ptr[band*27+10];
        cal_crc = calc_crc(ptr, band*27+10, 0xffff);
        eq->version = 0;
        eq->param = (u32 *)&ptr[4];
        eq->sample = ptr[2] & 0x07;
    } else if (ptr[0] == 'E' && ptr[1] == 'q' && ptr[2] == 0x01) {      //V2: Q27
        band = ptr[7];
        res_crc = ptr[band*27+15] << 8 | ptr[band*27+14];
        cal_crc = calc_crc(ptr, band*27+14, 0xffff);
        eq->version = ptr[2];
        eq->param = (u32 *)&ptr[8];
        eq->sample = ptr[3];
    } else {
        return false;
    }
    eq->band_cnt = band;
    if (res_crc == cal_crc) {
        return true;
    } else {
        return false;
    }
}

AT(.text.sco_app_dbg.nr_init)
static bool sco_app_eq_res_get_value(u32 addr, u32 len, u8 msbc_en)
{
    if (addr == 0 || (len) == 0) {
        return false;
    }
    u8 *ptr = (u8 *)(addr);
    eq_param_t *eq_param;
    if (msbc_en) {
        eq_param = &sco_app_var.post_eq_16k;
    } else {
        eq_param = &sco_app_var.post_eq_8k;
    }
    //验证CRC
    if (!sco_app_eq_res_crc_check(ptr, eq_param)) {
        return false;
    }
    if (eq_param->band_cnt == 0) {
        return false;
    }
    if (msbc_en) {
        memcpy(sco_app_var.post_eq_16k_param, sco_app_var.post_eq_16k.param, (sco_app_var.post_eq_16k.band_cnt*5+1)*sizeof(u32));
    } else {
        memcpy(sco_app_var.post_eq_8k_param, sco_app_var.post_eq_8k.param, (sco_app_var.post_eq_8k.band_cnt*5+1)*sizeof(u32));
    }
    return true;
}

AT(.text.sco_app_dbg.nr_init)
static void sco_app_post_eq_init(void)
{
    TRACE("%s\n", __func__);
    if (bt_sco_is_msbc()) {
        mic_set_eq(sco_app_var.post_eq_16k.band_cnt, sco_app_var.post_eq_16k_param);
    } else {
        mic_set_eq(sco_app_var.post_eq_8k.band_cnt, sco_app_var.post_eq_8k_param);
    }
}

AT(.text.sco_app_dbg.nr_init)
static void sco_app_post_drc_init(void)
{
    TRACE("%s\n", __func__);
    if (bt_sco_is_msbc()) {
        mic_set_drc_by_res((u32)sco_app_var.post_drc_16k_res, sizeof(sco_app_var.post_drc_16k_res));
    } else {
        mic_set_drc_by_res((u32)sco_app_var.post_drc_8k_res, sizeof(sco_app_var.post_drc_8k_res));
    }
}

AT(.text.sco_app_dbg.nr_init)
static void sco_app_dbg_var_set_do(u32 type)
{
    u16 temp0, temp1;
    if (sco_is_connected()) {
        temp0 = cfg_sco_debug_init_val;
        temp1 = cfg_sco_debug;
        cfg_sco_debug_init_val = 0;
        cfg_sco_debug = 0;

        switch (type) {
        case CMD_MIC_ANL_GAIN_SET:
            sco_app_anl_gain_init();
            break;
        case CMD_MIC_DIG_GAIN_SET:
            sco_app_dig_gain_init();
            break;
        case CMD_TRUMPET_NR_SET:
            sco_app_trumpet_nr_init();
            break;
        case CMD_POST_GIAN_SET:
            sco_app_post_gain_init();
            break;
        case CMD_FAR_NR_SET:
            sco_app_far_nr_init();
            break;
        case CMD_AEC_SET:
            sco_app_aec_init();
            break;
        case CMD_ENC_SET:
            sco_app_enc_init();
            break;
        case CMD_POST_EQ_SET:
            sco_app_post_eq_init();
            break;
        case CMD_POST_DRC_SET:
            sco_app_post_drc_init();
            break;
        default:
            break;
        }
        delay_us(10);

        cfg_sco_debug_init_val = temp0;
        cfg_sco_debug = temp1;
    }
}

AT(.text.sco_app_dbg.init)
void bt_sco_nr_dbg_init(void)
{
    TRACE("%s\n", __func__);
    if (cfg_sco_dbg_init_first) {                               //上电之后只取一次默认值，避免通话挂断之后参数恢复默认值
        TRACE("bt_sco_nr_dbg_init first time!\n");

        memset(&sco_app_var, 0, sizeof(sco_app_var_t));
        cfg_sco_debug_init_val  = SCO_DBG_ALG_ALL_EN;           //先打开所有算法，让 cfg_sco_debug 使能初始化值
        cfg_sco_debug           = SCO_DBG_ALG_ALL_DISEN;

        memcpy(&sco_app_var.bt_voice, bt_voice_ptr, sizeof(call_cfg_t));

        sco_app_var.mic_anl_gain                = bt_sco_dbg_call_var_get(0);
        sco_app_var.mic_dig_gain                = bt_sco_dbg_call_var_get(1);
        sco_app_var.mic_post_gain               = bt_sco_dbg_call_var_get(2);



        bt_sco_dbg_alg_en(SCO_DBG_AEC_LP, 1);
        bt_sco_dbg_alg_en(SCO_DBG_AEC_NLP, 1);
        bt_sco_dbg_alg_en(SCO_DBG_NR, 1);
        bt_sco_dbg_alg_en(SCO_DBG_TRUMPET, 1);
        bt_sco_dbg_alg_en(SCO_DBG_POST_EQ, 1);
        bt_sco_dbg_alg_en(SCO_DBG_POST_DRC, 1);
        bt_sco_dbg_alg_en(SCO_DBG_FAR_NR, 0);

        if (!sco_app_eq_res_get_value((u32)*(u32*)soc_app_res_tbl[0][0], (u32)*(u32*)soc_app_res_tbl[0][1], 0)) {
            TRACE("sco_app_eq_res_get_value 8K error!\n");
        }
        sco_app_var.post_eq_8k.version = 0x01;
        sco_app_var.post_eq_8k.sample = TLV_SCO_SAMPLE_RATE_8K;
        sco_app_var.post_eq_8k.param = sco_app_var.post_eq_8k_param;

        if (!sco_app_eq_res_get_value((u32)*(u32*)soc_app_res_tbl[1][0], (u32)*(u32*)soc_app_res_tbl[1][1], 1)) {
            TRACE("sco_app_eq_res_get_value 16K error!\n");
        }
        sco_app_var.post_eq_16k.version = 0x01;
        sco_app_var.post_eq_16k.sample = TLV_SCO_SAMPLE_RATE_16K;
        sco_app_var.post_eq_16k.param = sco_app_var.post_eq_16k_param;


        int res_len_8k = sizeof(sco_app_var.post_drc_8k_res);
        int res_len_16k = sizeof(sco_app_var.post_drc_16k_res);
        res_len_8k = (*(u32*)soc_app_res_tbl[2][1] > res_len_8k) ? res_len_8k : *(u32*)soc_app_res_tbl[2][1];
        res_len_16k = (*(u32*)soc_app_res_tbl[3][1] > res_len_16k) ? res_len_16k : *(u32*)soc_app_res_tbl[3][1];
        memcpy(sco_app_var.post_drc_8k_res, (u8*)*(u32*)soc_app_res_tbl[2][0], res_len_8k);
        memcpy(sco_app_var.post_drc_16k_res, (u8*)*(u32*)soc_app_res_tbl[3][0], res_len_16k);

        cfg_sco_debug_init_val = cfg_sco_debug;     //备份 cfg_sco_debug 初始值

    } else {
        //NOTIFY
        if (!bt_sco_is_msbc()) {
            bt_sco_app_dbg_notify_call_status(TLV_CALL_STATUS_CVSD);
        } else {
            bt_sco_app_dbg_notify_call_status(TLV_CALL_STATUS_MSBC);
        }
    }
    cfg_sco_dbg_init_first = false;

    //重新初始化一遍APP修改后的参数
    sco_app_anl_gain_init();
    sco_app_dig_gain_init();
    sco_app_post_gain_init();
    sco_app_trumpet_nr_init();
    sco_app_aec_init();
    sco_app_enc_init();
    sco_app_far_nr_init();
    sco_app_post_eq_init();
    sco_app_post_drc_init();
}

#endif // BT_SCO_APP_DBG_EN
