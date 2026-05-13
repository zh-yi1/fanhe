/*
 * 文件名称: mic_effect.c
 * 功能描述: 复用PACC0模块，处理EQ/DRC等音效
 *
 * 注意事项：PACC0能分时处理不同cs链路，但要避免多线程同时调用pacc_ctl_proc_cs
 *
 */

#include "include.h"
#include "effect_idx.h"
#include "mic_effect.h"


#if BT_SCO_EQ_DRC_EN
//------------------------------------------------------------------------------------------
//通话mic EQ/DRC链路

#define SCO_PACC_FRAME_LEN		                240                 //算法处理帧长

static struct {
    void *pacc_ctl;                                                 //pacc硬件模块的控制句柄
    pacc_cs_t *start_cs;                                            //pacc链表处理的表头
    pacc_cs_t pacc_cs[SCO_PACC_MAX_CS];                             //pacc_cs列表
    u8 cs_en[SCO_PACC_MAX_CS];                                      //cs列表使能标志
    u8 cache[SCO_PACC_FRAME_LEN*4];                                 //pacc中间缓存
} sco_pacc AT(.sco_alg_buf.sco_pacc.link);

static bq_bd_t sco_bq_db; //AT(.sco_alg_buf.sco_pacc);              //coef与buf放到不同的sram速度会快点
static drc_bd_t sco_drc_db; //AT(.sco_alg_buf.sco_pacc);

static const pacc_effect_cb sco_pacc_cb_tbl[SCO_PACC_MAX_CS] = {
    {SCO_PACC_EQ_CS,   PACC_EQ,    false, false,  sco_pacc.cache,        NULL, S16_Q15_SAT, sco_pacc.cache,  S32_Q23, &sco_pacc.pacc_cs[SCO_PACC_EQ_CS],        &sco_bq_db},
    {SCO_PACC_DRC_CS,  PACC_DRC,   false, false,  sco_pacc.cache,        NULL, S32_Q23, sco_pacc.cache,  S16_Q15_SAT, &sco_pacc.pacc_cs[SCO_PACC_DRC_CS],       &sco_drc_db},
};


void sco_pacc_init(void)
{
    printf("%s\n", __func__);

    //temp ram init
    memset(sco_pacc.cache, 0, sizeof(sco_pacc.cache));

    //初始链路模块状态全部关闭
    memset(sco_pacc.cs_en, 0x0, SCO_PACC_MAX_CS);

    //ram init
    memset(&sco_drc_db, 0, sizeof(drc_bd_t));
    memset(&sco_bq_db, 0, sizeof(bq_bd_t));

    sco_pacc.pacc_ctl = pacc_effect_init(0); //麦使用PACC0
    sco_pacc.start_cs = pacc_effect_link((void*)sco_pacc_cb_tbl, SCO_PACC_MAX_CS, SCO_PACC_FRAME_LEN);
}

static void sco_pacc_eq_set_param(bool is_msbc)
{
    u32 eq_addr, eq_len;

    if (is_msbc) {
        eq_addr = RES_BUF_EQ_BT_MIC_16K_EQ;
        eq_len  = RES_LEN_EQ_BT_MIC_16K_EQ;
    } else {
        eq_addr = RES_BUF_EQ_BT_MIC_8K_EQ;
        eq_len  = RES_LEN_EQ_BT_MIC_8K_EQ;
    }

    if (pacc_eq_set_by_res(&sco_pacc.pacc_cs[SCO_PACC_EQ_CS], eq_addr, eq_len) == ERROR_NO) {
        sco_pacc.cs_en[SCO_PACC_EQ_CS] = 1;
    }
}

static void sco_pacc_drc_set_param(bool is_msbc)
{
    u32 drc_addr, drc_len;

    if (is_msbc) {
        drc_addr = RES_BUF_DRC_BT_MIC_16K_DRC;
        drc_len = RES_LEN_DRC_BT_MIC_16K_DRC;
    } else {
        drc_addr = RES_BUF_DRC_BT_MIC_8K_DRC;
        drc_len = RES_LEN_DRC_BT_MIC_8K_DRC;
    }

    if (pacc_drc_set_by_res(&sco_pacc.pacc_cs[SCO_PACC_DRC_CS], drc_addr, drc_len) == ERROR_NO) {
        sco_pacc.cs_en[SCO_PACC_DRC_CS] = 1;
    }
}

void sco_pacc_set_param(bool is_msbc)
{
    sco_pacc_eq_set_param(is_msbc);
    sco_pacc_drc_set_param(is_msbc);
}

void sco_pacc_enable(void)
{
    printf("%s\n", __func__);

    //根据使能的链路，重新链接cs链路
    sco_pacc.start_cs = pacc_effect_relink(sco_pacc.pacc_cs, sco_pacc.cs_en, SCO_PACC_MAX_CS);

    //重新设置链头和链尾的数据格式
    pacc_cs_list_set_io_fmt(sco_pacc.start_cs, false, S16_Q15_SAT, S16_Q15_SAT);
}

AT(.bt_voice.sco_pacc)
void sco_pacc_process(u8 *obuf, u8 *ibuf, u32 samples)
{
    pacc_ctl_proc_cs(sco_pacc.pacc_ctl, sco_pacc.start_cs, obuf, ibuf, samples);
}

void sco_pacc_exit(void)
{
    pacc_effect_exit(0);                //麦使用PACC0
}

bool mic_sco_pacc_init(bool is_msbc)
{
    //先初始化PACC链路
    sco_pacc_init();

    //然后设置参数
    sco_pacc_set_param(is_msbc);

    //最后使能PACC
    sco_pacc_enable();

    return (sco_pacc.start_cs != NULL);
}
#endif // BT_SCO_EQ_DRC_EN
