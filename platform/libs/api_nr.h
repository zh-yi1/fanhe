#ifndef _API_NR_H
#define _API_NR_H

//数字音量
#define _MAX_GAIN               (0x8000)

//10^(n/20), n为DB数                                                  n
#define AEC_DIG_P0DB            (int)(_MAX_GAIN * 1.000000)           //0db
#define AEC_DIG_P0_5DB          (int)(_MAX_GAIN * 1.059254)           //0.5db
#define AEC_DIG_P1DB            (int)(_MAX_GAIN * 1.122018)           //1db
#define AEC_DIG_P1_5DB          (int)(_MAX_GAIN * 1.188502)           //1.5db
#define AEC_DIG_P2DB            (int)(_MAX_GAIN * 1.258925)           //2db
#define AEC_DIG_P2_5DB          (int)(_MAX_GAIN * 1.333521)           //2.5db
#define AEC_DIG_P3DB            (int)(_MAX_GAIN * 1.412538)           //3db
#define AEC_DIG_P3_5DB          (int)(_MAX_GAIN * 1.496236)           //3.5db
#define AEC_DIG_P4DB            (int)(_MAX_GAIN * 1.584893)           //4db
#define AEC_DIG_P4_5DB          (int)(_MAX_GAIN * 1.678804)           //4.5db
#define AEC_DIG_P5DB            (int)(_MAX_GAIN * 1.778279)           //5db
#define AEC_DIG_P5_5DB          (int)(_MAX_GAIN * 1.883649)           //5.5db
#define AEC_DIG_P6DB            (int)(_MAX_GAIN * 1.995262)           //6db
#define AEC_DIG_P6_5DB          (int)(_MAX_GAIN * 2.113489)           //6.5db
#define AEC_DIG_P7DB            (int)(_MAX_GAIN * 2.238721)           //7db
#define AEC_DIG_P7_5DB          (int)(_MAX_GAIN * 2.371374)           //7.5db
#define AEC_DIG_P8DB            (int)(_MAX_GAIN * 2.511886)           //8db
#define AEC_DIG_P8_5DB          (int)(_MAX_GAIN * 2.660725)           //8.5db
#define AEC_DIG_P9DB            (int)(_MAX_GAIN * 2.818383)           //9db
#define AEC_DIG_P9_5DB          (int)(_MAX_GAIN * 2.985383)           //9.5db
#define AEC_DIG_P10DB           (int)(_MAX_GAIN * 3.162278)           //10db
#define AEC_DIG_P10_5DB         (int)(_MAX_GAIN * 3.349654)           //10.5db
#define AEC_DIG_P11DB           (int)(_MAX_GAIN * 3.548134)           //11db
#define AEC_DIG_P11_5DB         (int)(_MAX_GAIN * 3.758374)           //11.5db
#define AEC_DIG_P12DB           (int)(_MAX_GAIN * 3.981072)           //12db
#define AEC_DIG_P12_5DB         (int)(_MAX_GAIN * 4.216965)           //12.5db
#define AEC_DIG_P13DB           (int)(_MAX_GAIN * 4.466836)           //13db
#define AEC_DIG_P13_5DB         (int)(_MAX_GAIN * 4.731513)           //13.5db
#define AEC_DIG_P14DB           (int)(_MAX_GAIN * 5.011872)           //14db
#define AEC_DIG_P14_5DB         (int)(_MAX_GAIN * 5.308844)           //14.5db
#define AEC_DIG_P15DB           (int)(_MAX_GAIN * 5.623413)           //15db
#define AEC_DIG_P15_5DB         (int)(_MAX_GAIN * 5.956621)           //15.5db
#define AEC_DIG_P16DB           (int)(_MAX_GAIN * 6.309573)           //16db
#define AEC_DIG_P16_5DB         (int)(_MAX_GAIN * 6.683439)           //16.5db
#define AEC_DIG_P17DB           (int)(_MAX_GAIN * 7.079458)           //17db
#define AEC_DIG_P17_5DB         (int)(_MAX_GAIN * 7.498942)           //17.5db
#define AEC_DIG_P18DB           (int)(_MAX_GAIN * 7.943282)           //18db
#define AEC_DIG_P18_5DB         (int)(_MAX_GAIN * 8.413951)           //18.5db
#define AEC_DIG_P19DB           (int)(_MAX_GAIN * 8.912509)           //19db
#define AEC_DIG_P19_5DB         (int)(_MAX_GAIN * 9.440609)           //19.5db
#define AEC_DIG_P20DB           (int)(_MAX_GAIN * 10.000000)          //20db
#define AEC_DIG_P20_5DB         (int)(_MAX_GAIN * 10.592537)          //20.5db
#define AEC_DIG_P21DB           (int)(_MAX_GAIN * 11.220185)          //21db
#define AEC_DIG_P21_5DB         (int)(_MAX_GAIN * 11.885022)          //21.5db
#define AEC_DIG_P22DB           (int)(_MAX_GAIN * 12.589254)          //22db
#define AEC_DIG_P22_5DB         (int)(_MAX_GAIN * 13.335214)          //22.5db
#define AEC_DIG_P23DB           (int)(_MAX_GAIN * 14.125375)          //23db
#define AEC_DIG_P23_5DB         (int)(_MAX_GAIN * 14.962357)          //23.5db
#define AEC_DIG_P24DB           (int)(_MAX_GAIN * 15.848932)          //24db
#define AEC_DIG_P24_5DB         (int)(_MAX_GAIN * 16.788040)          //24.5db
#define AEC_DIG_P25DB           (int)(_MAX_GAIN * 17.782794)          //25db
#define AEC_DIG_P25_5DB         (int)(_MAX_GAIN * 18.836491)          //25.5db
#define AEC_DIG_P26DB           (int)(_MAX_GAIN * 19.952623)          //26db
#define AEC_DIG_P26_5DB         (int)(_MAX_GAIN * 21.134890)          //26.5db
#define AEC_DIG_P27DB           (int)(_MAX_GAIN * 22.387211)          //27db
#define AEC_DIG_P27_5DB         (int)(_MAX_GAIN * 23.713737)          //27.5db
#define AEC_DIG_P28DB           (int)(_MAX_GAIN * 25.118864)          //28db
#define AEC_DIG_P28_5DB         (int)(_MAX_GAIN * 26.607251)          //28.5db
#define AEC_DIG_P29DB           (int)(_MAX_GAIN * 28.183829)          //29db
#define AEC_DIG_P29_5DB         (int)(_MAX_GAIN * 29.853826)          //29.5db
#define AEC_DIG_P30DB           (int)(_MAX_GAIN * 31.622777)          //30db
#define AEC_DIG_P30_5DB         (int)(_MAX_GAIN * 33.496544)          //30.5db
#define AEC_DIG_P31DB           (int)(_MAX_GAIN * 35.481339)          //31db
#define AEC_DIG_P31_5DB         (int)(_MAX_GAIN * 37.583740)          //31.5db

#define DUMP_MIC_TALK                       BIT(0)      //主MIC数据
#define DUMP_MIC_FF                         BIT(1)      //副MIC数据
#define DUMP_MIC_NR                         BIT(2)      //降噪后数据
#define DUMP_AEC_INPUT                      BIT(4)      //AEC输入数据
#define DUMP_AEC_FAR                        BIT(5)      //AEC远端数据
#define DUMP_AEC_OUTPUT                     BIT(6)      //AEC输出数据
#define DUMP_FAR_NR_INPUT                   BIT(8)      //远端降噪输入数据
#define DUMP_FAR_NR_OUTPUT                  BIT(9)      //远端降噪输出数据
#define DUMP_EQ_OUTPUT                      BIT(10)     //MIC EQ输出数据


//call param
enum NR_TYPE {
    NR_TYPE_NONE            = 0,
    NR_TYPE_AINS4,
    NR_TYPE_DNN,                                //自研单MIC降噪算法
    NR_TYPE_AIAEC,                              //自研单MIC AIAEC
    NR_TYPE_DMIC_AI,                            //自研双MIC降噪算法
    NR_TYPE_DM_AIAEC,                           //自研双MIC AIAEC
};

enum NR_CFG_EN {
    NR_CFG_FAR_EN           = BIT(0),           //使能远端降噪
    NR_CFG_SCO_FADE_EN      = BIT(3),           //使能通话前500ms淡入
};

typedef struct {
	s16	gamma;
	u8  nlp_en;
	u8  nlp_level;

	s16 nt;
	u8 param_printf;
	u8  music_lev;
	//s16 exp_range_H;
	//s16 exp_range_L;
	u8  noise_ps_rate;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
	u8  wind_level;
	u16 wind_range;
	u16 mask_floor;
	u16 low_fre_range;
	u8  nn_only;
	u16 nn_only_len;
	u8  sin_gain_post_en;
	u16 sin_gain_post_len;
	u16 sin_gain_post_len_f;
	u8  smooth_en;
	u16 far_post_thr;
	u32 dtd_post_thr;
	u16 gain_assign;
	u16 echo_gain_floor;
	s16 dtd_smooth;
	s16 single_floor;
	s16 gain_assign_nlp;
	u16 gain_st_thr;
	s32 intensity;
	s16 aec_gain_db;
	u8  nlms_en;
	u16 nlms_len;
	u32 index_lim;
	u8  state_zero_en;
	u8  state_clr_en;
} dnn_aec_ns_cb_t;

//typedef struct {
//	//u16 nt;
//	//s16 exp_range_H;
//	//s16 exp_range_L;
//	//u8  model_select;
//	u16 min_value;
//	u16 nostation_floor;
//	u8  wind_thr;
//	u8  wind_en;
//	u8  noise_ps_rate;
//	u8  prior_opt_idx;
//	u8  prior_opt_ada_en;
//    u8  param_printf;                           //使能参数打印
//   // u8  wind_level;
//	//u16 wind_range;
//	u16 low_fre_range;
//	u16 low_fre_range0;
//	u8  pitch_filter_en;
//	u16 mask_floor;
//	u16 noise_ceil;
//	//u16 ps_lowlimt;
//	u8  mask_floor_r;
//	u8  music_lev;
//	u16 gain_expand;
//	u8  nn_only;
//	u16 nn_only_len;
//	u16 gain_assign;
//	u8  sin_gain_post_en;
//	u16 sin_gain_post_len;
//	u16 sin_gain_post_len_f;
//	s16 spp_thr;
//	//u16 pitch_filter_range;
//} dnn_cb_t;
typedef struct {
	u16 nt;
	u8  nt_post;
	s16 exp_range_H;
	s16 exp_range_L;
	u8  model_select;
	u16 min_value;
	u16 nostation_floor;
	u8  wind_thr;
	u8  wind_en;
	u8  noise_ps_rate;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
    u8  param_printf;                           //使能参数打印
    u8  wind_level;
	u16 wind_range;
	u16 low_fre_range;
	u16 low_fre_range0;
	u8  pitch_filter_en;
	u16 mask_floor;
	u8  mask_floor_r;
	u8  music_lev;
	u16 gain_expand;
    u8  nn_only;
	u16 nn_only_len;
	u16 gain_assign;
	u8  preem_en;
} dnn_cb_t;


typedef struct {
	u8  param_printf;                           //使能参数打印
	u8  mic_cfg;                                //主副麦选择
	u8  bf_type;                                //beamforming类型
	s16 distance;				                //双麦间距
	s16 nt;						                //普通降噪量
	u8  nt_post;						        //后滤波降噪量
	s16 exp_range;
	//u8  fast_convergence_en;                    //快速收敛功能，对人声有影响，默认不开
	//u16 lowside_corr;                           //下支路相关系数

	//	u8  trumpet_en;                             //喇叭声抑制
	u8  cmp_tbl_dis;
	u8  dm_dnn_en;                              //是否使能双麦AI算法
	u8  noise_ps_rate;
	u8  comp_mic_fre;                           //双麦+AI的参数
	u16 comp_mic_fre_L;                         //传统双麦的参数
	u16 comp_mic_fre_H;                         //传统双麦的参数
	u8  prior_opt_idx;
	u16 low_fre_ns0_range;
	u8  wind_sig_choose;
	u8  bf_choose;
	u8  tf_en;
	u16 tf_len;
	u8  tf_norm_en;
	u8  tf_norm_only_en;
	s32 opt_limit;
	u8	music_lev;
	u16 gain_expand;
	u8  nn_only;
	u16 nn_only_len;
	u8  hi_fre_en;
	u8  prior_opt_ada_en;
	u16 gain_assign;
	u8  sin_gain_post_en;
	u16 b_frein2_floor;
	u16 Gcoh_thr;
	u16 Gcoh_sum_thr;
	u8  wind_or_en;
	u8  wind_pick_mode;
	s16 after_tf_norm;
	u8  v_white_en;
	//u8  wind_type;
	s16 sin_gain_post_len_f;
	s16 sin_gain_post_len;
	u8  enlarge_v;
	s16 mask_floor;
	u16 mask_floor_hi;
	u16 low_fre_range;
	u16 low_fre_range0;
	s16 wind_state_lim;
	s16 wind_sup_floor;
	u8  wind_sup_open;
	u8  bfrein_en;
	u8  bf2_used_len;
	//u8  bfrein2_len;
	//u16 bf2_gain_sm;
	//u16 bf2_frameG_floor;
	u8  bfrein3_ns;
	u16 bfrein3_intensity;
	u16 wind_len_used;
	u8  bfrein3_en;
	u8  ai_postF_en;
	u8  phase_to_w;
	u8  phase_to_nw;
	u8  show_out;
	u8  white_approach_shape;
	u8  linear_len;
	u8  linear_level;
	s32 white_detect;
	u8  white_nomal_scale;
	u8  highFcanc_256_to;
	s16 tf_SMrate;
	u8  sup180;
	u16 sup180rtf;
	u8  white_type_len;
	s16 sp_thres;
	u8  mic1_gain_idx;
	u8  mic2_gain_idx;
	s16 rtf_post_thres;
	u8  smooth_en;
	u8  linear_en;
	s32 grad_shift;
	s32 adp_smr;
	s32 all_smr;
	s32 atf_smr;
	s32 grad_mu;
	s16 high_tf_Llim;
	s32 high_tf_rate;
	s32 high_gpost_appd;
	u8  frame_G_opt;
	s16 gcoh_gain;
	u8  low_windstate_len;
	s16 low_windstate_gain;
} dmns_cb_t;

typedef struct {
	u8  param_printf;                           //使能参数打印
	u8  mic_cfg;                                //主副麦选择
	u8  bf_type;                                //beamforming类型
	s16 distance;				                //双麦间距
	s16 nt;						                //普通降噪量
	u8  nt_post;						        //后滤波降噪量
	s16 exp_range;
	//u8  fast_convergence_en;                    //快速收敛功能，对人声有影响，默认不开
	//u16 lowside_corr;                           //下支路相关系数

	//	u8  trumpet_en;                             //喇叭声抑制
	u8  cmp_tbl_dis;
	u8  dm_dnn_en;                              //是否使能双麦AI算法
	u8  noise_ps_rate;
	u8  comp_mic_fre;                           //双麦+AI的参数
	u16 comp_mic_fre_L;                         //传统双麦的参数
	u16 comp_mic_fre_H;                         //传统双麦的参数
	u8  prior_opt_idx;
	u16 low_fre_ns0_range;
	u8  wind_sig_choose;
	u8  bf_choose;
	u8  tf_en;
	u16 tf_len;
	u8  tf_norm_en;
	u8  tf_norm_only_en;
	s32 opt_limit;
	u8	music_lev;
	u16 gain_expand;
	u8  nn_only;
	u16 nn_only_len;
	u8  hi_fre_en;
	u8  prior_opt_ada_en;
	u16 gain_assign;
	u8  sin_gain_post_en;
	u16 b_frein2_floor;
	u16 Gcoh_thr;
	u16 Gcoh_sum_thr;
	u8  wind_or_en;
	u8  wind_pick_mode;
	u8  v_white_en;
	//u8  wind_type;
	s16 sin_gain_post_len_f;
	s16 sin_gain_post_len;
	u8  enlarge_v;
	s16 mask_floor;
	u16 mask_floor_hi;
	u16 low_fre_range;
	u16 low_fre_range0;
	s16 wind_state_lim;
	s16 wind_sup_floor;
	u8  wind_sup_open;
	u8  bfrein_en;
	u8  bf2_used_len;
	//u8  bfrein2_len;
	//u16 bf2_gain_sm;
	//u16 bf2_frameG_floor;
	u8  bfrein3_ns;
	u16 bfrein3_intensity;
	u16 wind_len_used;
	u8  bfrein3_en;
	u8  ai_postF_en;
	u8  phase_to;
	u8  show_out;
	u8  white_approach_shape;
	u8  linear_len;
	u8  linear_level;
	s32 white_detect;
	u8  white_nomal_scale;
	u8  highFcanc_256_to;
	s16 tf_SMrate;
	u8  sup180;
	u16 sup180rtf;
	u8  white_type_len;
	s16 sp_thres;
	u8  mic1_gain_idx;
	u8  mic2_gain_idx;
	s16 rtf_post_thres;
	u8  smooth_en;
	u8  nlp_choose;
	u8  linear_en;
	s32 grad_shift;

	s16	gamma;
	u8  nlp_en;
	u8  nlp_level;

	u16 far_post_thr;
	u32 dtd_post_thr;
	u16 echo_gain_floor;
	s16 dtd_smooth;
	s16 single_floor;
	s16 gain_assign_nlp;
	u16 gain_st_thr;
	s16 aec_gain_db;
	u8  nlms_en;
	u16 nlms_len;
	u8  refer_dfw;
} dmdnn_aiaec_cb_t;

typedef struct {
    s16 nt;
    u8  prior_opt_idx;
	u8	music_lev;
	u16 music_lev_hi_range;
	u16 music_lev_hi;
	u16 ns_ps_rate;
	u8	low_fre_lev;
	u16 low_fre_range;
	u16	ns_range_l;
	u16	ns_range_h;
	s32 noise_db;
	s32 noise_db2;
	s32 noise_db3;
	u8 smooth_en;
} ains3_cb_t;

typedef struct {
    s32 overdrive;
	s8  nr_suppress;
	u8  smooth_en;
	u8  modelUpdatePars0;
	u8  spp_en;
	s32 delta_k_down;
	s16 prior_opt_idx;
	u8  low_fre_range;
	u8  delta_k_up;
} ains4_cb_t;

typedef struct {
    u8 enable;
    u8 level;
    u8 noise_thr;
    s8 value_ns;
} far_nr_cfg_t;

typedef struct {
    u8 nr_type;                                 //近端降噪类型
    u8 nr_cfg_en;                               //远端降噪、喇叭声降噪、500ms淡入等降噪配置
    u8 level;                                   //降噪强度
    u8 resv;
    u32 dump_en;
    void *far_nr;                               //远端降噪算法配置
} nr_cb_t;

typedef struct {
    union {
        struct {
            u32 aec_en          : 1;
            u32 nlp_bypass      : 1;        //aec nlp bypass
            u32 nlp_only        : 1;        //aec nlp only select
            u32 comforn_en      : 1;        //comfortable noise enable
            u32 mic_ch          : 1;
            u32 mode            : 1;        //auto or manual mode
            u32 int_en          : 1;
            u32 nlp_level       : 4;        //aec nlp level
            u32 nlp_part        : 2;        //nlp mode choose
            u32 diverge_th      : 5;        //filter diverge threshold
            u32 comforn_level   : 4;        //range: 0~10
            u32 comforn_floor   : 10;       //range: 0~1000
        };
        u32 aeccon0;
    };
    u16 aggrfact;
    u16 xe_add_corr;
    u8 upbin;                               //hnl valid upbin
    u8 lowbin;                              //hnl valid lowbin
    u16 qidx;                               //ref bin
    u16 gamma;                              //sxd/de smooth parameter
    u32 dig_gain;                           //0DB: 0x8000
    u32 echo_th;
    u16 far_offset;
    u8  ff_mic_ref_en;
    u8  bandrange;
    u8  mu_step;
} aec_cfg_t;

typedef struct {
    u8 alc_en;
    u8 rfu[1];
    u8 fade_in_step;
    u8 fade_out_step;
    u8 fade_in_delay;
    u8 fade_out_delay;
    s32 far_voice_thr;
} alc_cb_t;

typedef struct {
    aec_cfg_t aec;
    alc_cb_t  alc;
    nr_cb_t   nr;
    u8 mic_eq_en;
    u8 mic_mav_en;
    u8 agc_en;
    u8 rfu[1];
    u32 post_gain;                  //后置数据增益
} call_cfg_t;

#if BT_SCO_APP_DBG_EN
void sco_audio_init(void);
void sco_audio_exit(void);

void bt_sco_app_dbg_init();
void bt_sco_dbg_connect_ack(void);
bool bt_sco_app_dbg_proc(u8 *data,u8 len);

void sco_app_msg_deal(u8 msg);
#endif // BT_SCO_APP_DBG_EN

bool bt_sco_dnn_is_en(void);
void bt_ains4_init(void *alg_cb);
void bt_ains4_exit(void);
void bt_dnn_init(void *alg_cb);
void bt_dnn_exit(void);
void bt_aiaec_dnn_init(void *alg_cb);
void bt_aiaec_dnn_exit(void);
void bt_dmdnn_init(void *alg_cb);
void bt_dmdnn_exit(void);
void bt_dmdnn_aiaec_init(void *alg_cb);
void bt_dmdnn_aiaec_exit(void);
void bt_agc_init(s32 sampleHzIn, s32 bit, s32 agcDb, s32 agcDbfs);

#endif // _API_NR_H
