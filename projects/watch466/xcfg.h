//该头文件由软件自动生成，请勿随意修改！
#ifndef _XCFG_H
#define _XCFG_H

#define XCFG_EN             1

typedef struct __attribute__((packed)) _xcfg_cb_t {
    u32 powkey_10s_reset                 : 1;   //POWKEY 10s复位系统
    u16 sys_sleep_time;                         //自动休眠时间: 不休眠: 0, 10秒钟后: 10, 20秒钟后: 20, 30秒钟后: 30, 45秒钟后: 45, 1分钟后: 60, 2分钟后: 120, 3分钟后: 180, 4分钟后: 240, 5分钟后: 300, 6分钟后: 360, 7分钟后: 420, 8分钟后: 480, 9分钟后: 540, 10分钟后: 600, 15分钟后: 900, 20分钟后: 1200, 25分钟后: 1500, 30分钟后: 1800, 45分钟后: 2700, 1小时后: 3600
    u8 vol_max;                                 //音量级数: 0:16级音量, 1:32级音量
    u8 sys_init_vol;                            //开机默认音量
    u8 warning_volume;                          //提示音播放音量
    u8 osc_both_cap;                            //OSC基础电容: 0:0PF, 1:6PF
    u8 uosci_cap;                               //自定义OSCI电容
    u8 uosco_cap;                               //自定义OSCO电容
    u32 ft_osc_cap_en                    : 1;   //优先使用产测电容值
    u32 eq_dgb_spp_en                    : 1;   //EQ调试（蓝牙串口）
    u32 huart_dump_en                    : 1;   //HUART调试(EQ/FCC)
    u32 huart_dump_sel                   : 4;   //HUART串口选择: PB3: 1, VUSB: 2, PA10: 4, PB0: 5, PB1: 6, PB8: 7, PB9: 8, PE0: 9, PE1: 10, PE4: 11, PE5: 12, PE13: 13
    u32 fmrx_cfg_en                      : 1;   //FM收音配置
    u32 buck_mode_en                     : 1;   //BUCK MODE
    u32 vddbt_capless_en                 : 1;   //VDDBT省电容
    u32 vddio_sel                        : 4;   //VDDIO电压: 3.1V: 7, 3.2V: 8, 3.3V: 9, 3.4V: 10, 3.5V: 11, 3.6V: 12
    u32 vddbt_sel                        : 5;   //VDDBT电压: 1.35V: 10, 1.4V: 11, 1.45V: 12, 1.5V: 13, 1.55V: 14, 1.6V: 15, 1.65V: 16, 1.7V: 17, 1.95V: 22, 2.0V: 23
    u32 dac_sel                          : 2;   //DAC声道选择: 单端双声道: 0, 单端单声道: 1
    u32 dacaud_ldo_sel                   : 3;   //VDDDAC电压: 2.5V: 0, 2.6V: 1, 2.7V: 2, 2.8V: 3, 2.9V: 4, 3.0V: 5, 3.1V: 6, 3.2V: 7
    u32 dacaud_bypass_en                 : 1;   //省VDDDAC方案
    u8 dac_max_gain;                            //DAC最大音量: 0:0DB, 1:-1DB, 2:-2DB, 3:-3DB, 4:-4DB, 5:-5DB, 6:-6DB, 7:-7DB
    u8 bt_call_max_gain;                        //通话最大音量: 0:0DB, 1:-1DB, 2:-2DB, 3:-3DB, 4:-4DB, 5:-5DB, 6:-6DB, 7:-7DB
    u32 bt_connect_name_en               : 1;   //限制蓝牙名连接
    char bt_connect_name[32];                   //可连接蓝牙名
    u32 charge_en                        : 1;   //充电使能
    u32 charge_trick_en                  : 1;   //涓流充电使能
    u32 charge_dc_reset                  : 1;   //插入DC复位系统
    u32 charge_dc_not_pwron              : 1;   //插入DC禁止软开机
    u32 charge_stop_curr                 : 4;   //充电截止电流: 2.5mA: 1, 5mA: 2, 7.5mA: 3, 10mA: 4, 12.5mA: 5, 15mA: 6, 17.5mA: 7, 20mA: 8, 25mA: 10, 30mA: 12, 35mA: 14
    u32 charge_constant_curr             : 7;   //恒流充电电流: 30mA: 5, 40mA: 7, 50mA: 9, 60mA: 11, 70mA: 13, 80mA: 15, 90mA: 17, 100mA: 19, 110mA: 21, 120mA: 23, 130mA: 25, 140mA: 27, 150mA: 29, 160mA: 31, 170mA: 33, 180mA: 35, 190mA: 37, 200mA: 39, 210mA: 41, 220mA: 43, 230mA: 45, 240mA: 47, 250mA: 49, 260mA: 51, 270mA: 53, 280mA: 55, 290mA: 57, 300mA: 59, 310mA: 61, 320mA: 63, 330mA: 65, 340mA: 67, 350mA: 69, 360mA: 71, 370mA: 73, 380mA: 75, 390mA: 77, 400mA: 79
    u32 charge_trickle_curr              : 6;   //涓流充电电流: 5mA: 0, 10mA: 1, 20mA: 3, 25mA: 4, 30mA: 5, 35mA: 6, 40mA: 7
    char bt_name[32];                           //蓝牙名称
    u8 bt_addr[6];                              //蓝牙地址
    u32 bt_rf_txpwr_recon                : 3;   //降低回连TXPWR: 不降低: 0, 降低3dbm: 1, 降低6dbm: 2, 降低9dbm: 3
    u32 bt_a2dp_en                       : 1;   //音乐播放功能
    u32 bt_a2dp_vol_ctrl_en              : 1;   //音乐音量同步
    u32 bt_sco_en                        : 1;   //通话功能
    u32 bt_hfp_private_en                : 1;   //私密接听功能
    u32 bt_hfp_ring_number_en            : 1;   //来电报号功能
    u32 bt_hfp_inband_ring_en            : 1;   //来电播放手机铃声
    u32 bt_spp_en                        : 1;   //串口功能
    u32 ble_en                           : 1;   //BLE控制功能
    char le_name[32];                           //BLE名称
    u8 bt_rf_pwrdec;                            //降低预置RF参数发射功率
    u32 ft_rf_param_en                   : 1;   //优先使用FT的RF参数
    u32 bt_rf_param_en                   : 1;   //自定义RF参数
    u8 rf_pa_gain;                              //PA_GAIN0
    u8 rf_mix_gain;                             //MIX_GAIN0
    u8 rf_dig_gain;                             //DIG_GAIN0
    u8 rf_pa_cap;                               //GL_PA_CAP
    u8 rf_mix_cap;                              //GL_MIX_CAP
    u8 rf_txdbm;                                //GL_TX_DBM
    u32 bt_mmic_cfg                      : 3;   //通话主MIC配置: None: 5, MIC0: 0, MIC1: 1
    u32 bt_smic_cfg                      : 3;   //通话副MIC配置: None: 5, MIC0: 0, MIC1: 1
    u32 mic_post_gain                    : 4;   //通话MIC后置数字增益
    u32 mic0_en                          : 1;   //MIC0
    u32 mic0_bias_method                 : 2;   //MIC0偏置电路配置: 单端MIC外部电阻电容: 0, 差分MIC: 1, MIC省电阻电容: 2
    u32 mic0_pwr_sel                     : 2;   //供电IO选择: None: 0, PA11(MICBIAS0): 1, PA13(MICBIAS1): 2
    u32 bt_mic0_dig_gain                 : 6;   //MIC0数字增益(0~36DB)
    u32 mic1_en                          : 1;   //MIC1
    u32 mic1_bias_method                 : 2;   //MIC1偏置电路配置: 单端MIC外部电阻电容: 0, 差分MIC: 1, MIC省电阻电容: 2
    u32 mic1_pwr_sel                     : 2;   //供电IO选择: None: 0, PA11(MICBIAS0): 1, PA13(MICBIAS1): 2
    u32 bt_mic1_dig_gain                 : 6;   //MIC1数字增益(0~36DB)
    u32 bt_sco_nr_en                     : 1;   //近端降噪
    u32 bt_sco_nr_level                  : 5;   //近端降噪级别
    u32 bt_aec_en                        : 1;   //AEC功能
    u32 bt_echo_level                    : 4;   //AEC回声消除级别
    u32 bt_aec_dig_gain                  : 4;   //AEC数字增益
    u8 bt_far_offset;                           //AEC远端补偿值
    u32 bt_alc_en                        : 1;   //ALC功能
    u8 fmrx_r_val;                              //FMRX_THD_R_VAL
    u32 fmrx_z_val                       : 13;  //FMRX_THD_Z_VAL
    u32 fmrx_fz_val                      : 13;  //FMRX_THD_FZ_VAL
    u32 fmrx_d_val                       : 13;  //FMRX_THD_D_VAL
    u32 fmrx_cs_filter_fixed             : 1;   //固定CS_FILTER
    u32 fmrx_cs_filter_sel               : 2;   //CS_FILTER: 125K: 0, 75K: 1, 50K: 2, 25K: 3
    u32 fmrx_audio_ch                    : 2;   //FM声道选则: 单声道: 0, 双声道: 1
    char serial_2[8];                           //序列号前两个字节
    u8 serial_num[6];                           //序列号
    u8 xm_keep_start[0];                        //For Keep Area Start
    u8 osci_cap;                                //产测OSCI电容
    u8 osco_cap;                                //产测OSCO电容
    u8 soft_key[20];                            //授权密钥
    u8 asr_soft_key[20];                        //语音识别授权密钥
    u8 xm_keep_end[0];                          //For Keep Area End
} xcfg_cb_t;

extern xcfg_cb_t xcfg_cb;
#endif
