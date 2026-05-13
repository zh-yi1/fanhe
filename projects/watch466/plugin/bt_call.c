#include "include.h"

#if BT_SCO_AINS4_EN
static ains4_cb_t ains4_cb AT(.sco_data.ains4);
#endif

#if BT_SCO_DNN_EN
static dnn_cb_t dnn_cb AT(.sco_data.dnn);
#endif

#if BT_SCO_DMIC_AI_EN
static dmns_cb_t dmns_cb AT(.sco_data.dmdnn);
#endif

#if BT_SCO_AIAEC_DNN_EN
static dnn_aec_ns_cb_t aiaec_dnn_cb AT(.sco_data.aiaec);
#endif

#if BT_SCO_DMIC_AIAEC_EN
static dmdnn_aiaec_cb_t dmdnn_aiaec_cb AT(.sco_data.dmdnn_aiaec);
#endif

#if BT_SCO_FAR_NR_EN
static far_nr_cfg_t far_nr_cfg AT(.sco_data.far);
#endif

extern const int mic_gain_tbl[16];

void bt_sco_dump_init(u8 *sysclk, call_cfg_t *p)
{
    nr_cb_t *nr = &p->nr;
#if BT_SCO_DUMP_EN
    nr->dump_en = DUMP_MIC_TALK | DUMP_MIC_NR;
#if BT_SCO_DMIC_EN
    nr->dump_en |= DUMP_MIC_FF;
#endif
#if BT_AEC_EN
    if (xcfg_cb.bt_aec_en) {
        nr->dump_en |= DUMP_AEC_FAR;
    }
#endif
#if BT_SCO_AIAEC_DNN_EN || BT_SCO_DMIC_AIAEC_EN
    nr->dump_en |= DUMP_AEC_FAR;
#endif
#elif BT_SCO_FAR_DUMP_EN
    nr->dump_en = DUMP_FAR_NR_INPUT | DUMP_FAR_NR_OUTPUT;
#elif BT_SCO_EQ_DUMP_EN
    nr->dump_en = DUMP_MIC_TALK | DUMP_MIC_NR | DUMP_EQ_OUTPUT;
#endif
    if (nr->dump_en != 0) {
        if (!xcfg_cb.huart_dump_en || !HUART_DUMP_EN) {
            printf("dump huart xcfg init err!\n");
            nr->dump_en = 0;
        }
    }
}

void bt_sco_aec_init(u8 *sysclk, aec_cfg_t *aec, alc_cb_t *alc)
{
    aec->far_offset     = 240 + BT_FAR_OFFSET;              //AIAEC会用到这个参数
    aec->ff_mic_ref_en  = BT_AEC_FF_MIC_REF_EN;

#if BT_AEC_EN
    if (xcfg_cb.bt_aec_en) {
        xcfg_cb.bt_alc_en   = 0;
        aec->aec_en         = 1;
        aec->mode           = 1;
        aec->nlp_bypass     = BT_AEC_NLP_BYPASS_EN;
        aec->nlp_only       = 0;
        aec->nlp_level      = BT_ECHO_LEVEL;
        aec->nlp_part       = 1;                            //0:xd, 1:de&xd, 2:xd|xe
        aec->comforn_level  = 10;
        aec->comforn_floor  = 300;
        aec->comforn_en     = 0;
        aec->xe_add_corr    = 16384;
        aec->upbin          = 63;                           //调节最高频率2000Hz
        aec->lowbin         = 7;                            //调节最低频率250Hz
        if (!bt_sco_is_msbc()) {
            aec->upbin *= 2;
            aec->lowbin *= 2;
        }
        aec->bandrange      = aec->upbin - aec->lowbin + 1;
        aec->diverge_th     = 3;
        aec->echo_th        = 66666;
        aec->gamma          = 27852;
        aec->aggrfact       = 22937;                        //bigger means more residual echo suppress
        aec->mic_ch         = 0;
        aec->mu_step        = 60;
        aec->qidx           = ((int)(0.35f * (1 << 15)) * aec->bandrange) >> 15;	//smaller means more residual echo suppress
        aec->dig_gain       = mic_gain_tbl[xcfg_cb.bt_aec_dig_gain];
        *sysclk = *sysclk < SYS_48M ? SYS_48M : *sysclk;
    }
#endif

#if BT_ALC_EN
    if (xcfg_cb.bt_alc_en) {
        alc->alc_en = 1;
        alc->fade_in_delay = BT_ALC_FADE_IN_DELAY;
        alc->fade_in_step = BT_ALC_FADE_IN_STEP;
        alc->fade_out_delay = BT_ALC_FADE_OUT_DELAY;
        alc->fade_out_step = BT_ALC_FADE_OUT_STEP;
        alc->far_voice_thr = BT_ALC_VOICE_THR;
        if (!bt_sco_is_msbc()) {
            alc->fade_in_delay >>= 1;
            alc->fade_out_delay >>= 1;
            alc->fade_in_step <<= 1;
            alc->fade_out_step <<= 1;
        }
        *sysclk = *sysclk < SYS_48M ? SYS_48M : *sysclk;
    }
#endif
}

//远端降噪
void bt_sco_far_nr_init(u8 *sysclk, nr_cb_t *nr)
{
#if BT_SCO_FAR_NR_EN
    nr->nr_cfg_en |= NR_CFG_FAR_EN;
    memset(&far_nr_cfg, 0, sizeof(far_nr_cfg_t));
    nr->far_nr = &far_nr_cfg;
    far_nr_cfg.enable           = BT_SCO_FAR_NR_EN;
    far_nr_cfg.level            = BT_SCO_FAR_NR_LEVEL;
    far_nr_cfg.noise_thr        = BT_SCO_FAR_NOISE_THR;
    far_nr_cfg.value_ns         = BT_SCO_FAR_VALUE_NS;
#endif
}

//MIC近端降噪
void bt_sco_nr_init(u8 *sysclk, nr_cb_t *nr)
{
    if (xcfg_cb.bt_sco_nr_en) {
        bt_sco_near_nr_init(sysclk, nr);
    }
}

void bt_sco_nr_exit(void)
{
    if (xcfg_cb.bt_sco_nr_en) {
        bt_sco_near_nr_exit();
    }
}

#if BT_SCO_AINS4_EN
void bt_sco_ains4_init(u8 *sysclk, nr_cb_t *nr)
{
    ains4_cb_t *cb           = &ains4_cb;
    nr->nr_type             = NR_TYPE_AINS4;

    memset(&ains4_cb, 0, sizeof(ains4_cb_t));

	cb->nr_suppress	        = BT_SCO_AINS4_LEVEL;
    cb->overdrive		    = 40960;
    cb->smooth_en	        = 0;
    cb->modelUpdatePars0    = 2;
    cb->delta_k_up		    = 2;
    cb->delta_k_down	    = 1310720;
    cb->prior_opt_idx       = 6;
    cb->low_fre_range       = 8;
    cb->spp_en				= 1;

    bt_ains4_init(&ains4_cb);

    *sysclk = *sysclk < SYS_48M ? SYS_48M : *sysclk;
}

void bt_sco_ains4_exit(void)
{
    bt_ains4_exit();
}
#endif

#if BT_SCO_DNN_EN
void bt_sco_dnn_init(u8 *sysclk, nr_cb_t *nr)
{
    dnn_cb_t *cb = &dnn_cb;
    nr->nr_type             = NR_TYPE_DNN;

    memset(&dnn_cb, 0, sizeof(dnn_cb_t));

    //cb->param_printf           = 1;
	//cb->nt                     = BT_SCO_DNN_LEVEL;
	//cb->nt_post                = 0; //0-6 >0才起效 0为不开gain指数化 开的话默认为3
	cb->noise_ps_rate          = 1;
	cb->prior_opt_idx	       = 3;
	cb->prior_opt_ada_en	   = 1;

	cb->low_fre_range          = 16; //
	cb->low_fre_range0         = 0;
	//cb->pitch_filter_en		   = 1;
	//cb->ps_lowlimt             = 0;
	cb->mask_floor			   = 1000;
	cb->noise_ceil			   = 0;
	cb->music_lev			   = 11;
	//cb->comforN_level		   = 1;
	cb->gain_expand			   = 1024;
	cb->nn_only				   = 0;
	cb->nn_only_len			   = 16;
	cb->gain_assign			   = 21666;
	cb->sin_gain_post_en	   = 0;
	cb->sin_gain_post_len	   = 128;
	cb->sin_gain_post_len_f	   = 256;
	cb->spp_thr				   = 8000;

	bt_dnn_init(&dnn_cb);

    *sysclk = *sysclk < SYS_144M ? SYS_144M : *sysclk;
}

void bt_sco_dnn_exit(void)
{
    bt_dnn_exit();
}
#endif

#if BT_SCO_DMIC_AI_EN
void bt_sco_dmns_init(u8 *sysclk, nr_cb_t *nr)
{
	dmns_cb_t *cb = &dmns_cb;
	memset(&dmns_cb, 0, sizeof(dmns_cb_t));
	nr->nr_type             = NR_TYPE_DMIC_AI;

	//辅助参数
	//cb->param_printf            = 1;
	cb->ai_postF_en             = 1;
	cb->show_out                = 0;  //0表示没有特殊输出,1:YAM,2:YBM,3:f1_tmp,4:f2_tmp,5:talk，6:YAM_R_w, 7:white_dB*x1, 8:bf2_used上之路YAM_R_tmp
									   // 9:风燥范围
	//基本参数
	cb->distance                = BT_SCO_DMNS_DISTANCE;
	cb->nt                      = BT_SCO_DMNS_LEVEL;

	//线性相位差参数
	cb->linear_en               = 1;     //开关，关掉的话highFcanc_256_to， sup180rtf，sup180都不起作用
	cb->linear_len              = 150;
	cb->linear_level            = 5;       //线性相位差线性度，越大越线性
	cb->grad_mu                 = 327;     //grad学习率，越小越慢，不能超过400

	// 风噪参数
	cb->wind_sig_choose		    = 1;     //0时代表副mic受风小  1代表主mic受风小
	cb->atf_smr                 = 30084;  //风噪，线性化的通道间tf的平滑率
	cb->wind_len_used           = 100;    //风噪抑制以及判断使用范围66
	cb->wind_sup_open           = 1;     //是否打开风噪抑制，风噪抑制是加在baseline上的，而非风噪会进入tf
	cb->wind_state_lim          = 25000; //风噪判断的阈值，越大判断风噪范围越大，即使没打开风噪抑制也需要判断是否是风噪
	cb->low_windstate_len       = 40;     //低频的风噪判断阈值范围
	cb->low_windstate_gain      = 1024;   //低频的风噪判断阈值增益
	cb->wind_or_en			    = 1;     //风噪判断的模式，1表示判断为风噪的范围更大
	cb->wind_sup_floor          = 23000; //风噪抑制floor
	cb->Gcoh_thr			    = 13666;
	cb->Gcoh_sum_thr		    = 9666;
	cb->wind_pick_mode          = 2;     //防止风噪放大情况，如果不为0则能量最大值会加以限制，2为选取f1f2最小值作为界限，1为最大值作为界限
	cb->after_tf_norm           = 324;    //1024 q10,越小谐波幅度可达到越高，但风噪有可能会突变
	cb->gcoh_gain               = 0;      //为0则关闭风噪低频压制模式， q10，太小了会导致压制过度，越大低频谐波越多

	//白噪编辑
	cb->white_detect            = 32767;   //白噪启动（检测）阈值，目标信号是噪声的倍数,越大white范围越小越严格
	cb->white_nomal_scale       = 4;       //白噪归一化尺度，越大白噪强度越小（但是会导致baseline相位差线性度削弱）,太大了会导致线性相位补偿效果弱
	cb->highFcanc_256_to        = 0;       //最大255，若检测声音方向是180°，高频白噪屏蔽，为255则关闭
	cb->all_smr                 = 26635;    //全额补偿平滑系数 X_AMAM_sm_w[i] X_BMBM_sm_w[i],对white形状很关键,当white_type_len使用gain3_save的时候这个无效
	cb->white_type_len		    = 0;    //使用gain3_save作为white判断的低频范围，低于这个值的频段相位线性会变弱波束也会变差

	// baseline参数
	cb->comp_mic_fre_L          = 0;
	cb->comp_mic_fre_H          = 255;     //幅度补偿增益上限
	cb->opt_limit               = 32767;   //调整baseline波束宽度，波束后滤波下之路最大增益，越大baseline输出被挖掉的下之路越多
	cb->bf_choose			    = 1;       //baseline的波叔形状选择
	cb->adp_smr                 = 26635;    //自适应补偿平滑系数 X_BMBM_sm[i] X_AMAM_sm[i]，X_BMBM_sm也是bf3噪声谱的重要判据，会影响baseline结果，越小越锐利，
	                                        // 降噪越好，但有可能杀谐波

	//RTF参数
	cb->tf_en				    = 1;
	cb->tf_len				    = 256; //smart: range:1-140   zoom:1:256
	cb->tf_norm_en			    = 1;
	cb->tf_norm_only_en		    = 0;
	cb->white_approach_shape    = 1;
	cb->v_white_en			    = 1;           //白噪开关
	cb->tf_SMrate               = 30000;       //最大32767  tf的平滑率
	cb->rtf_post_thres          = 2048;   //判断baseline结构是否为语音的阈值,越大判定噪声范围越大
	cb->sup180rtf               = 40;           //让180°入射tf更新快一点，越大越快,为0则关闭，与bf3的开启与否没关系

	//相位贴合
	cb->phase_to_w              = 0;  //风噪条件下，如果为0则最终相位为糅合，1为使用talk相位，2为使用ff相位， 3则使用自适应相位
	cb->phase_to_nw             = 0;  //非风噪条件下，如果为0则最终相位为糅合，1为使用talk相位，2为使用ff相位， 3则使用自适应相位

	// 波束增强模块选择
	cb->bfrein3_en		        = 1;//if set "1", the "cb->bfrein_en" must be "0".
	cb->bfrein3_intensity       = 1024;  //噪声估计强度，q10越大去噪越强最大不超过u16
	cb->sup180                  = 20;   //Q8//如果关掉bf3或者linear_en任意一个，sup180都失效，为0则关闭，越大抑制越大,越大180°方向噪声谱能量越大
	cb->grad_shift              = 0;  //bf波束宽度调节，越大波束越窄，可以为负数
	cb->frame_G_opt             = 0;    //帧级别低频范围，用来修正波束宽度，
	cb->bfrein3_ns		        = 3;  //波束增强模块gain的floor，越大floor越小，底噪越小

	cb->bfrein_en			    = 0;//if set "1", the "cb->bfrein3_en" must be "0".
	cb->b_frein2_floor		    = 19666;
	cb->bf2_used_len            = 0;   //bf2点级别，双上支路使用频段，必须大于bfrein2_len，为0时bfrein2_len也必为0，最大255,会有分层，主要是上之路选了更小的幅度
	// bf2_used_len还没有到理想状态，不能启用！

	//整形
	cb->high_tf_Llim            = 255;  //大于这个频率的tf用的gpost和平滑率略有不同 255为关闭
	cb->high_tf_rate            = 13000;
	cb->high_gpost_appd         = 10000;

	// AI参数
	cb->prior_opt_ada_en	    = 1;
	cb->noise_ps_rate           = 1;
	cb->music_lev			    = 11;
	cb->gain_expand			    = 1024;
	cb->nn_only				    = 0;
	cb->nn_only_len			    = 16;
	cb->hi_fre_en 			    = 1;
	cb->gain_assign			    = 26666;
	cb->sin_gain_post_en	    = 1;
	cb->sin_gain_post_len	    = 0;   //nn的sin化
	cb->sin_gain_post_len_f	    = 256;  //最终gain的sin化
	cb->enlarge_v			    = 1;//1 or 2 or 3
	cb->mask_floor			    = 1000;
	cb->mask_floor_hi		    = 160;
	cb->low_fre_range           = 16; //
	cb->low_fre_range0          = 0;
	cb->low_fre_ns0_range	    = 16;//range:1-200
	cb->prior_opt_idx           = 3;

	cb->sp_thres				= 12000;
	cb->mic1_gain_idx           = 10;
	cb->mic2_gain_idx           = 10;
	cb->smooth_en				= 1;

    bt_dmdnn_init(&dmns_cb);

    *sysclk = *sysclk < SYS_144M ? SYS_144M : *sysclk;
}

void bt_sco_dmdnn_exit(void)
{
    bt_dmdnn_exit();
}
#endif //BT_SCO_DMIC_AI_EN

#if BT_SCO_AIAEC_DNN_EN
void bt_sco_aiaec_dnn_init(u8 *sysclk, nr_cb_t *nr)
{
    dnn_aec_ns_cb_t *cb = &aiaec_dnn_cb;

    nr->nr_type = NR_TYPE_AIAEC;
    memset(&aiaec_dnn_cb, 0, sizeof(dnn_aec_ns_cb_t));

    //cb->param_printf           	= 0;
    cb->nt                     	= BT_SCO_AIAEC_DNN_LEVEL;
    cb->nlp_level               = BT_SCO_AIAEC_NLP_LEVEL;

    cb->gamma                   = 27852;
    cb->nlp_en                  = 1;
    cb->noise_ps_rate          	= 1;
    cb->prior_opt_idx	        = 10;
    cb->prior_opt_ada_en        = 1;
    cb->wind_level			    = 0;
    cb->wind_range			    = 0;
    cb->low_fre_range          	= 15;
    cb->mask_floor			    = 600;
    cb->music_lev			    = 11;
    cb->nn_only				   	= 0;
    cb->nn_only_len			   	= 16;
	cb->gain_assign				= 16666;
    cb->sin_gain_post_en	    = 0;
    cb->sin_gain_post_len	    = 128;
    cb->sin_gain_post_len_f		= 0;
    cb->smooth_en			    = 1;
    cb->far_post_thr		    = 1024;//Q8
	cb->dtd_post_thr		    = 80000000;//Q15

    cb->echo_gain_floor			= 100;
    cb->dtd_smooth			    = 29491;
    cb->single_floor		    = 0;
    cb->gain_assign_nlp			= 16000;
    cb->gain_st_thr				= 12000;
	cb->intensity			    = 1;
	cb->aec_gain_db				= 6;//AEC增益 0-30（放大0到30db）
	cb->nlms_en					= 1;
	cb->nlms_len				= 0;
	cb->index_lim				= 10000;
	cb->state_zero_en			= 1;
	cb->state_clr_en			= 1;

    bt_aiaec_dnn_init(cb);

    *sysclk = *sysclk < SYS_144M ? SYS_144M : *sysclk;
}

void bt_sco_aiaec_dnn_exit(void)
{
    bt_aiaec_dnn_exit();
}
#endif //BT_SCO_AIAEC_DNN_EN

#if BT_SCO_DMIC_AIAEC_EN
void bt_sco_dmdnn_aiaec_init(u8 *sysclk, nr_cb_t *nr)
{
    nr->nr_type = NR_TYPE_DM_AIAEC;
    dmdnn_aiaec_cb_t *cb = &dmdnn_aiaec_cb;

    memset(&dmdnn_aiaec_cb, 0, sizeof(dmdnn_aiaec_cb_t));

	//辅助参数
	//cb->param_printf            = 1;
	cb->ai_postF_en             = 1;
	cb->show_out                = 0;  //0表示没有特殊输出,1:YAM,2:YBM,3:f1_tmp,4:f2_tmp,5:talk，6:YAM_R_w, 7:white_dB*x1, 8:bf2_used上之路YAM_R_tmp
									   // 9:风燥范围
	//基本参数
	cb->distance                = BT_SCO_DMIC_AIAEC_DISTANCE;
	cb->nt                      = BT_SCO_DMIC_AIAEC_NT_LEVEL;
	cb->nlp_level               = BT_SCO_DMIC_AIAEC_ECHO_LEVEL;
	cb->nlp_choose				= BT_SCO_DMIC_AIAEC_NLP_REF;    //0代表主mic，1代表副mic

	// 风噪参数
	cb->wind_sig_choose		    = 1;     //0时代表副mic受风小  1代表主mic受风小
	cb->wind_len_used           = 66;    //风噪抑制以及判断使用范围66
	cb->wind_sup_open           = 1;     //是否打开风噪抑制，风噪抑制是加在baseline上的，而非风噪会进入tf
	cb->wind_state_lim          = 25000; //风噪判断的阈值，越大判断风噪范围越大，即使没打开风噪抑制也需要判断是否是风噪
	cb->wind_or_en			    = 1;     //风噪判断的模式，1表示判断为风噪的范围更大
	cb->wind_sup_floor          = 23000; //风噪抑制floor
	cb->Gcoh_thr			    = 13666;
	cb->Gcoh_sum_thr		    = 9666;
	cb->wind_pick_mode          = 0;     //防止风噪放大情况，如果不为0则能量最大值会加以限制，2为选取f1f2最小值作为界限，1为最大值作为界限

	//线性相位差参数
	cb->linear_en               = 1;  //开关，关掉的话highFcanc_256_to， sup180rtf，sup180都不起作用
	cb->linear_len              = 100;
	cb->linear_level            = 3;       //线性相位差线性度，越大越线性
	cb->white_detect            = 32767;    //白噪启动（检测）阈值，目标信号是噪声的倍数
	cb->white_nomal_scale       = 4;       //白噪归一化尺度，越大白噪强度越小（但是会导致baseline相位差线性度削弱）,太大了会导致线性相位补偿效果弱
	cb->highFcanc_256_to        = 0;    //最大255，若检测声音方向是180°，高频白噪屏蔽，为255则关闭

	// baseline参数
	cb->comp_mic_fre_L          = 0;
	cb->comp_mic_fre_H          = 256;     //幅度补偿增益上限
	cb->opt_limit               = 32767;   //波束后滤波下之路最大增益，越大baseline输出被挖掉的下之路越多
	cb->bf_choose			    = 1;       //baseline的波叔形状选择

	//RTF参数
	cb->tf_en				    = 1;
	cb->tf_len				    = 256; //smart: range:1-140   zoom:1:256
	cb->tf_norm_en			    = 1;
	cb->tf_norm_only_en		    = 0;
	cb->white_approach_shape    = 1;
	cb->v_white_en			    = 1;           //白噪开关
	cb->tf_SMrate               = 30000;       //最大32767  tf的平滑率
	cb->rtf_post_thres          = 2048;   //判断baseline结构是否为语音的阈值
	cb->white_type_len		    = 0;    //使用gain3_save作为white判断的低频范围，低于这个值的频段相位线性会变弱波束也会变差
	cb->sup180rtf               = 4;           //让180°入射tf更新快一点，越大越快,为0则关闭

	//相位贴合
	cb->phase_to                = 0;  //如果为0则最终相位为糅合，1为使用talk相位，2为使用ff相位

	// 波束增强模块选择
	cb->bfrein_en			    = 0;//if set "1", the "cb->bfrein3_en" must be "0".
	cb->b_frein2_floor		    = 19666;
	cb->bfrein3_en		        = 1;//if set "1", the "cb->bfrein_en" must be "0".
	cb->bfrein3_ns		        = 2;  //波束增强模块gain的floor，越大floor越小，底噪越小
	cb->bfrein3_intensity       = 1024;  //噪声估计强度，q10越大去噪越强最大不超过u16
	cb->sup180                  = 4;   ////如果关掉bf3或者linear_en任意一个，sup180都失效，为0则关闭，越大抑制越大,越大180°方向噪声谱能量越大
	cb->grad_shift              = 0;  //bf波束宽度调节，越大越容易进入非波束范围，波束越窄
	cb->bf2_used_len            = 0;   //bf2点级别，双上支路使用频段，必须大于bfrein2_len，为0时bfrein2_len也必为0，最大255,会有分层，主要是上之路选了更小的幅度
	// bf2_used_len还没有到理想状态，不能启用！

	// AI参数
	cb->prior_opt_ada_en	    = 1;
	cb->noise_ps_rate           = 1;
	cb->music_lev			    = 11;
	cb->gain_expand			    = 1024;
	cb->nn_only				    = 0;
	cb->nn_only_len			    = 16;
	cb->hi_fre_en 			    = 1;
	cb->gain_assign			    = 26666;
	cb->sin_gain_post_en	    = 0;
	cb->sin_gain_post_len	    = 128;
	cb->sin_gain_post_len_f	    = 256;
	cb->enlarge_v			    = 1;//1 or 2 or 3
	cb->mask_floor			    = 1000;
	cb->mask_floor_hi		    = 160;
	cb->low_fre_range           = 16; //
	cb->low_fre_range0          = 0;
	cb->low_fre_ns0_range	    = 16;//range:1-200
	cb->prior_opt_idx           = 3;

	cb->sp_thres				= 6000;
	cb->mic1_gain_idx           = 10;
	cb->mic2_gain_idx           = 10;
	cb->gamma                   = 27852;
	cb->nlp_en                  = 1;
	cb->far_post_thr		    = 1024;//Q8
	cb->dtd_post_thr		    = 80000000;//Q15
	cb->echo_gain_floor			= 100;
	cb->dtd_smooth			    = 29491;
	cb->single_floor		    = 0;
	cb->gain_assign_nlp			= 16000;
	cb->gain_st_thr				= 12000;
	cb->aec_gain_db				= 0;//AEC增益 0-30（放大0到30db）
	cb->refer_dfw				= 0;
	cb->nlms_en					= 1;
	cb->nlms_len				= 0;
	cb->smooth_en				= 1;

    bt_dmdnn_aiaec_init(cb);

    *sysclk = *sysclk < SYS_144M ? SYS_144M : *sysclk;
}

void bt_sco_dmdnn_aiaec_exit(void)
{
    bt_dmdnn_aiaec_exit();
}
#endif

#if BT_SCO_AGC_EN
void bt_sco_agc_init(void)
{
    int bit              = 16;
    int compress_agcDb   = 12;
    int target_agcDbfs   = 3;
    int sampleHzIn       = (bt_sco_is_msbc()) ? (16000) : (8000);

    bt_agc_init(sampleHzIn, bit, compress_agcDb, target_agcDbfs);
}
#endif //BT_SCO_AGC_EN
