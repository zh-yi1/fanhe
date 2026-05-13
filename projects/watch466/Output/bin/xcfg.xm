depend(0x01020300);

config(SUB, "系统配置", "系统的相关配置");
config(CHECK, "POWKEY 10s复位系统", "是否长按POWKEY 10s复位系统，用于防止系统死机", POWKEY_10S_RESET, 0);
config(LISTVAL, "自动休眠时间", "设置自动休眠时间", SYS_SLEEP_TIME, SHORT, 45, 21, ("不休眠", 0), ("10秒钟后", 10), ("20秒钟后", 20), ("30秒钟后", 30), ("45秒钟后", 45), ("1分钟后", 60), ("2分钟后", 120), ("3分钟后", 180), ("4分钟后", 240), ("5分钟后", 300), ("6分钟后", 360), ("7分钟后", 420), ("8分钟后", 480), ("9分钟后", 540), ("10分钟后", 600), ("15分钟后", 900), ("20分钟后", 1200), ("25分钟后", 1500), ("30分钟后", 1800), ("45分钟后", 2700), ("1小时后", 3600));
#config(LISTVAL, "自动关机时间", "设置自动关机时间", SYS_OFF_TIME, 	SHORT, 0, 18, ("不关机", 0), ("30秒钟后", 30), ("1分钟后", 60),  ("2分钟后", 120), ("3分钟后", 180), ("4分钟后", 240), ("5分钟后", 300), ("6分钟后", 360), ("7分钟后", 420), ("8分钟后", 480), ("9分钟后", 540), ("10分钟后", 600), ("15分钟后", 900), ("20分钟后", 1200), ("25分钟后", 1500), ("30分钟后", 1800), ("45分钟后", 2700), ("1小时后", 3600));
config(LIST, "音量级数", "选择系统音量级数", VOL_MAX, 2, "16级音量", "32级音量", 0);
config(BYTE, "开机默认音量", "开机默认音量级数", SYS_INIT_VOL, 0, 32, 8);
config(BYTE, "提示音播放音量", "提示音播放音量级数", WARNING_VOLUME, 0, 32, 8);
config(LIST, "OSC基础电容", "同时配置26M晶振OSCI与OSCO负载电容", OSC_BOTH_CAP, 2, "0PF", "6PF", 0);
config(BYTE, "自定义OSCI电容", "独立配置26M晶振OSCI负载电容，单位0.25PF。OSCI电容大小：n * 0.25PF + 基础电容", UOSCI_CAP, 0, 63, 23);
config(BYTE, "自定义OSCO电容", "独立配置26M晶振OSCO负载电容，单位0.25PF。OSCO电容大小：n * 0.25PF + 基础电容", UOSCO_CAP, 0, 63, 23);
config(CHECK,  "优先使用产测电容值",  "是否优先使用产测校准的OSC电容值，没过产测或者关闭时使用<自定义OSCI电容>、<自定义OSCO电容>",  FT_OSC_CAP_EN,  0);
config(CHECK, "EQ调试（蓝牙串口）", "是否使用SPP调试EQ功能", EQ_DGB_SPP_EN, 0);
config(CHECK, "HUART调试(EQ/FCC)", "是否使用HUART调试功能", HUART_DUMP_EN, 0);
config(LISTVAL, "HUART串口选择", "选择HUART的IO", HUART_DUMP_SEL, BIT, 4, 1, 12, ("PB3", 1), ("VUSB", 2), ("PA10", 4), ("PB0", 5), ("PB1", 6), ("PB8", 7), ("PB9", 8), ("PE0", 9), ("PE1", 10), ("PE4", 11), ("PE5", 12), ("PE13", 13), HUART_DUMP_EN);
config(CHECK, "FM收音配置", "是否支持FM收音功能", FMRX_CFG_EN, 0);

config(SUB, "电源配置", "电源的相关配置");
config(CHECK, "BUCK MODE", "是否设置为BUCK MODE", BUCK_MODE_EN, 0);
config(CHECK, "VDDBT省电容", "是否VDDBT省电容，只能LDO模式才能capless", VDDBT_CAPLESS_EN, 0);
config(LISTVAL, "VDDIO电压", "选择VDDIO电压", VDDIO_SEL, BIT, 4, 9, 6, ("3.1V", 7), ("3.2V", 8), ("3.3V", 9), ("3.4V", 10), ("3.5V", 11), ("3.6V", 12));
config(LISTVAL, "VDDBT电压", "选择VDDBT电压", VDDBT_SEL, BIT, 5, 12, 10, ("1.35V", 10), ("1.4V", 11), ("1.45V", 12), ("1.5V", 13), ("1.55V", 14), ("1.6V", 15), ("1.65V", 16), ("1.7V", 17), ("1.95V", 22), ("2.0V", 23));

config(SUB, "DAC配置", "DAC的相关配置");
config(LISTVAL, "DAC声道选择", "选择DAC的输出方式", DAC_SEL, BIT, 2, 1, 2, ("单端双声道", 0), ("单端单声道", 1));
config(LEVEL, 0x00);
config(LISTVAL, "VDDDAC电压", "选择VDDDAC电压", DACAUD_LDO_SEL, BIT, 3, 5, 8, ("2.5V", 0), ("2.6V", 1), ("2.7V", 2), ("2.8V", 3), ("2.9V", 4), ("3.0V", 5), ("3.1V", 6), ("3.2V", 7));
config(LEVEL, 0x03);
config(CHECK, "省VDDDAC方案", "是否VDDDAC Bypass，内部VDDDAC与VDDIO短接。", DACAUD_BYPASS_EN, 0);
config(LIST, "DAC最大音量", "配置系统最大模拟增益，自动调整模拟音量表。", DAC_MAX_GAIN, 8, "0DB", "-1DB", "-2DB", "-3DB", "-4DB", "-5DB", "-6DB", "-7DB", 0);
config(LIST, "通话最大音量", "配置通话时最大模拟增益，自动调整模拟音量表。", BT_CALL_MAX_GAIN, 8, "0DB", "-1DB", "-2DB", "-3DB", "-4DB", "-5DB", "-6DB", "-7DB", 0);

#if BT_EMIT_EN
config(SUB, "BT发射配置", "蓝牙发射器相关配置");
config(CHECK, "限制蓝牙名连接", "是否打开限制蓝牙名连接功能", BT_CONNECT_NAME_EN, 0);
config(TEXT, "可连接蓝牙名", "只能连接这个蓝牙名字的蓝牙设备", BT_CONNECT_NAME, 32, "BT-BOX",BT_CONNECT_NAME_EN);
#endif

config(SUB, "充电配置", "充电功能的相关配置");
config(CHECK, "充电使能", "是否打开充电功能", CHARGE_EN, 1);
config(CHECK, "涓流充电使能", "是否使能涓流充电", CHARGE_TRICK_EN, 1, CHARGE_EN);
config(CHECK, "插入DC复位系统", "是否插入DC充电复位系统，用于防止系统死机", CHARGE_DC_RESET, 1, CHARGE_EN);
config(CHECK, "插入DC禁止软开机", "PWRKEY软开机，DC Online时，禁止软开机", CHARGE_DC_NOT_PWRON, 1, CHARGE_EN);
config(LISTVAL, "充电截止电流", "选择充电截止电流", CHARGE_STOP_CURR, BIT, 4, 5, 11, ("2.5mA", 1), ("5mA", 2), ("7.5mA", 3), ("10mA", 4), ("12.5mA", 5), ("15mA", 6), ("17.5mA", 7), ("20mA", 8), ("25mA", 10), ("30mA", 12), ("35mA", 14), CHARGE_EN);
config(LISTVAL, "恒流充电电流", "恒流充电电流", CHARGE_CONSTANT_CURR, BIT, 7, 19, 38, ("30mA", 5), ("40mA", 7), ("50mA", 9), ("60mA", 11), ("70mA", 13), ("80mA", 15), ("90mA", 17), ("100mA", 19), ("110mA", 21), ("120mA", 23), ("130mA", 25), ("140mA", 27), ("150mA", 29), ("160mA", 31), ("170mA", 33), ("180mA", 35), ("190mA", 37), ("200mA", 39), ("210mA", 41), ("220mA", 43), ("230mA", 45), ("240mA", 47), ("250mA", 49), ("260mA", 51), ("270mA", 53), ("280mA", 55), ("290mA", 57), ("300mA", 59), ("310mA", 61), ("320mA", 63), ("330mA", 65), ("340mA", 67), ("350mA", 69), ("360mA", 71), ("370mA", 73), ("380mA", 75),("390mA", 77), ("400mA", 79), CHARGE_EN);
config(LISTVAL, "涓流充电电流", "涓流充电电流", CHARGE_TRICKLE_CURR,  BIT, 6, 5, 7, ("5mA", 0), ("10mA", 1), ("20mA", 3), ("25mA", 4), ("30mA", 5), ("35mA", 6), ("40mA", 7), CHARGE_TRICK_EN);

config(SUB, "蓝牙配置", "蓝牙的相关配置");
config(TEXT, "蓝牙名称", "手机上可以看到的蓝牙名称", BT_NAME, 32, "BT-AB569X");
config(MAC, "蓝牙地址", "蓝牙的MAC地址", BT_ADDR, 6, 41:42:00:00:00:00, 41:42:00:00:00:FF, 41:42:00:00:00:01);
config(LISTVAL, "降低回连TXPWR", "是否降低回连TXPWR", BT_RF_TXPWR_RECON, BIT, 3, 0, 4, ("不降低", 0), ("降低3dbm", 1), ("降低6dbm", 2), ("降低9dbm", 3));
config(CHECK, "音乐播放功能", "是否支持蓝牙音乐播放功能", BT_A2DP_EN, 1);
config(CHECK, "音乐音量同步", "是否支持A2DP音量与手机同步", BT_A2DP_VOL_CTRL_EN, 1, BT_A2DP_EN);
config(CHECK, "通话功能", "是否支持蓝牙通话的功能", BT_SCO_EN, 1);
config(CHECK, "私密接听功能", "是否使用手动私密接听（切换到手机端接听）", BT_HFP_PRIVATE_EN, 1, BT_SCO_EN);
config(CHECK, "来电报号功能", "是否支持来电报号功能", BT_HFP_RING_NUMBER_EN, 1, BT_SCO_EN);
config(CHECK, "来电播放手机铃声", "打开此功能后，可以播放手机铃声，例如ios", BT_HFP_INBAND_RING_EN, 0, BT_SCO_EN);
config(CHECK, "串口功能", "是否支持蓝牙串口的功能", BT_SPP_EN, 1);
config(CHECK, "BLE控制功能", "是否支持BLE音乐控制的功能", BLE_EN, 0);
config(TEXT, "BLE名称", "手机上可以看到的BLE蓝牙名称", LE_NAME, 32, "LE-AB569X", BLE_EN);

config(SUB, "蓝牙RF配置", "蓝牙RF相关配置");
config(S8, "降低预置RF参数发射功率", "微调<预置RF参数>或<FT的RF参数>的发射功率，每级可降低0.5dbm发射功率（负数则增加功率）", BT_RF_PWRDEC, -6, 32, 0);
config(CHECK,  "优先使用FT的RF参数",  "是否优先使用FT校准的RF参数，没过FT或关闭时使用<预置RF参数>或<自定义RF参数>",  FT_RF_PARAM_EN,  1);
config(CHECK, "自定义RF参数", "打开后，有FT的芯片优先使用<FT的RF参数>，否则使用<自定义RF参数>", BT_RF_PARAM_EN, 0);
config(BYTE, "PA_GAIN0",  "PA_GAIN0",  RF_PA_GAIN, 0, 7, 7, BT_RF_PARAM_EN);
config(BYTE, "MIX_GAIN0", "MIX_GAIN0", RF_MIX_GAIN, 0, 7, 6, BT_RF_PARAM_EN);
config(BYTE, "DIG_GAIN0", "DIG_GAIN0", RF_DIG_GAIN, 20, 57, 55, BT_RF_PARAM_EN);
config(LEVEL, 0x00);
config(BYTE, "GL_PA_CAP", "PA_CAP, 参考值4",     RF_PA_CAP,     0, 15,  4, BT_RF_PARAM_EN);
config(BYTE, "GL_MIX_CAP","MIX_CAP, 参考值7~9",  RF_MIX_CAP,    0, 15,  8, BT_RF_PARAM_EN);
config(BYTE, "GL_TX_DBM", "Cable实测dbm值",      RF_TXDBM,      0, 12,  8, BT_RF_PARAM_EN);
config(LEVEL, 0x03);

config(SUB, "MIC参数", "蓝牙MIC参数");
config(LISTVAL, "通话主MIC配置", "选择通话主MIC，注意配置对应MIC通路参数", BT_MMIC_CFG, BIT, 3, 0, 3,  ("None", 5), ("MIC0", 0), ("MIC1", 1), BT_SCO_EN);
config(LISTVAL, "通话副MIC配置", "选择通话副MIC，注意配置对应MIC通路参数", BT_SMIC_CFG, BIT, 3, 0, 3,  ("None", 5), ("MIC0", 0), ("MIC1", 1), BT_SCO_EN);
config(BIT, "通话MIC后置数字增益", "调节算法后置MIC数字增益", MIC_POST_GAIN, 4, 0, 15, 0, BT_SCO_EN);

config(CHECK, "MIC0", "是否打开MIC0", MIC0_EN, 0);
config(LISTVAL, "MIC0偏置电路配置", "MIC0电路BIAS配置，省电阻电容配置", MIC0_BIAS_METHOD, BIT, 2, 0, 3, ("单端MIC外部电阻电容", 0), ("差分MIC", 1), ("MIC省电阻电容", 2), MIC0_EN);
config(RELATE, MIC0_BIAS_METHOD, <2);
config(LISTVAL, "供电IO选择", "选择对应IO给MIC0供电", MIC0_PWR_SEL, BIT, 2, 0, 3, ("None", 0), ("PA11(MICBIAS0)", 1), ("PA13(MICBIAS1)", 2), MIC0_EN);
config(RELATE)
config(BIT, "MIC0数字增益(0~36DB)", "MIC0 SDADC数字增益, Step 1DB, 总共(0~36DB)", BT_MIC0_DIG_GAIN, 6, 0, 36, 10, MIC0_EN);

config(CHECK, "MIC1", "是否打开MIC0", MIC1_EN, 0);
config(LISTVAL, "MIC1偏置电路配置", "MIC1电路BIAS配置，省电阻电容配置", MIC1_BIAS_METHOD, BIT, 2, 0, 3, ("单端MIC外部电阻电容", 0), ("差分MIC", 1), ("MIC省电阻电容", 2), MIC1_EN);
config(RELATE, MIC1_BIAS_METHOD, <2);
config(LISTVAL, "供电IO选择", "选择对应IO给MIC1供电", MIC1_PWR_SEL, BIT, 2, 0, 3, ("None", 0), ("PA11(MICBIAS0)", 1), ("PA13(MICBIAS1)", 2), MIC1_EN);
config(RELATE)
config(BIT, "MIC1数字增益(0~36DB)", "MIC1 SDADC数字增益, Step 1DB, 总共(0~36DB)", BT_MIC1_DIG_GAIN, 6, 0, 36, 10, MIC1_EN);

config(SUB, "通话参数", "蓝牙的通话参数", BT_SCO_EN);
config(CHECK, "近端降噪", "是否使能近端降噪功能", BT_SCO_NR_EN, 1, BT_SCO_EN);
config(BIT, "近端降噪级别", "近端降噪深度, 值越大降噪越猛。", BT_SCO_NR_LEVEL, 5, 0, 30, 6, BT_SCO_NR_EN);
config(CHECK, "AEC功能", "是否支持AEC功能，AEC/ALC只能二选一", BT_AEC_EN, 1, BT_SCO_EN);
config(BIT, "AEC回声消除级别", "回声消除级别(级别越高，回声衰减越明显，但通话效果越差)", BT_ECHO_LEVEL, 4, 0, 15, 2, BT_AEC_EN);
config(BIT, "AEC数字增益", "AEC频域数字增益(0~15DB), Step 1DB", BT_AEC_DIG_GAIN, 4, 0, 15, 9, BT_AEC_EN);
config(BYTE, "AEC远端补偿值", "远端补偿值(0~255)", BT_FAR_OFFSET, 0, 255, 42, BT_AEC_EN);
config(CHECK, "ALC功能", "是否支持ALC功能，AEC/ALC只能二选一", BT_ALC_EN, 0, BT_SCO_EN);

config(SUB, "FM收音配置", "FM收音参数的相关配置",FMRX_CFG_EN);
config(BYTE, "FMRX_THD_R_VAL", "FM搜台参数:RSSI阈值,大于该阈值才会被判断为真台,该值越小台越多(默认值128)", FMRX_R_VAL,  0, 255, 128);
config(BIT, "FMRX_THD_Z_VAL", "FM搜台参数:音质Z阈值,小于该阈值才会判断为真台,该值越大台越多(默认值1100).", FMRX_Z_VAL,  13, 0, 8191, 1100);
config(BIT, "FMRX_THD_FZ_VAL","FM搜台参数:音质FZ阈值,小于该阈值才会判断为真台,该值越大台越多(默认值600).", FMRX_FZ_VAL, 13, 0, 8191, 600);
config(BIT, "FMRX_THD_D_VAL", "FM搜台参数:音质D阈值,真台一般在3000内,此参数一般不用更改.该值越大台越多(默认值3000).", FMRX_D_VAL, 13, 0, 8191, 3000);
config(CHECK, "固定CS_FILTER", "FM指标测试需要固定CS_FILTER,避免自动切换带来其它影响,正常使用一般关该功能,让弱台自动使用小一些的CS_FILER杂音更小", FMRX_CS_FILTER_FIXED, 1);
config(LISTVAL, "CS_FILTER", "CS_FILTER选择", FMRX_CS_FILTER_SEL, BIT, 2, 2, 4, ("125K", 0), ("75K", 1), ("50K", 2), ("25K", 3)),FMRX_CS_FILTER_FIXED);
config(LISTVAL, "FM声道选则", "FM声道输出声道,一般单声道比双声道噪音会小些", FMRX_AUDIO_CH, BIT, 2, 0, 2, ("单声道", 0), ("双声道", 1));


config(SUB, "生产配置", "生产配置");
config(TEXT, "序列号前两个字节", "序列号前两个字节", SERIAL_2, 8, "ABAB");
config(MAC, "序列号", "烧录的序列号", SERIAL_NUM, 6, 00:00:00:00:00:01, 00:00:FF:FF:FF:FF, 00:00:00:00:00:01);

config(LEVEL, 0x0E);
config(SUB, "产测参数", "产测校准的一些参数，不要改动！");
config(KEEP);
config(BYTE, "产测OSCI电容", "产测校准的24M晶振OSCI负载电容，单位0.25PF。不要改动默认值0。", OSCI_CAP, 0, 63, 0);
config(BYTE, "产测OSCO电容", "产测校准的24M晶振OSCO负载电容，单位0.25PF。不要改动默认值0。", OSCO_CAP, 0, 63, 0);
#if DONGLE_AUTH_EN
config(BUF, "授权密钥", "程序运行需要判断的合法授权密钥", soft_key, 20);
#endif
#if ASR_USBKEY_PSD
config(BUF, "语音识别授权密钥", "语音识别程序运行需要判断的合法授权密钥", asr_soft_key, 20);
#endif
config(KEEPEND)

config(LEVEL, 0x100);
makecfgfile(xcfg.bin);
makecfgdef(xcfg.h);
xcopy(xcfg.h, ../../xcfg.h);
