#include "include.h"

#if (ASR_SELECT == ASR_WS_AIR)

#include "../ws_air/air/ir_cap.h"
#include "../ws_air/air/func_api_air.h"
///语音识别处理例程，客户根据需要修改命令列表和处理流程
#define ASR_CMD_LEN         19

u8  wakeup_flag;
u32 asr_wakeup_tick_timer;
u8  asr_out_timer = 10;

extern uint8_t isd_search_type;

// 1-匹配空调，2-搜索空调，3-下一套
u8 mp3_end_oper;


u8 asr_start_flag = 1; //语音开启标志

int isd_set_ir_brand(int index);
void ir_cap_timer_stop();
void isd_exit_search_remote(void);
int isd_brand_get_next_remote(uint8_t type);
void isd_search_remote_onebyone(void);
void isd_get_next_remote(void);
void compo_form_set_ai_voice_anim(u8 sta);

// 语音开始、停止
void asr_start_stop(u8 flag)
{
    if (flag && !asr_start_flag) {
        printf("asr start...\n\n");
        asr_start_flag = 1;
        asr_alg_start();
    } else if (!flag && asr_start_flag) {
        printf("asr stop...\n\n");
        asr_start_flag = 0;
        asr_alg_stop();
    }
}

// 唤醒定时清零
void asr_timer_cnt_clear(void)
{
    // asr_wakeup_tick_timer = 0;
    asr_wakeup_tick_timer = tick_get();
}

void air_timer_set(u8 timer)
{
    asr_out_timer = timer;
}

int asr_is_timeout(unsigned int timeOut)
{
    // return asr_wakeup_tick_timer > timeOut ? 1 : 0;
    return tick_check_expire(asr_wakeup_tick_timer, timeOut*1000);
}


enum {
#if MONITOR_ASR_WAKEUP_EN
    ASR_CMD_WAKEUP = 0,
    ASR_CMD_WAKEUP1,
#endif
    ASR_AIR_01,		//*01*/ {"LG空调"},
    ASR_AIR_02,		//*02*/ {"TCL空调"},
    ASR_AIR_03,		//*03*/ {"奥克斯空调"},
    ASR_AIR_04,		//*04*/ {"志高空调"},
    ASR_AIR_05,		//*05*/ {"松下空调"},
    ASR_AIR_06,		//*06*/ {"格力空调"},
    ASR_AIR_07,		//*07*/ {"海信空调"},
    ASR_AIR_08,		//*08*/ {"海尔空调"},
    ASR_AIR_09,		//*09*/ {"科龙空调"},
    ASR_AIR_10,		//*10*/ {"美的空调"},
    ASR_AIR_11,		//*11*/ {"长虹空调"},
    ASR_AIR_12,		//*12*/ {"日立空调"},
    ASR_AIR_13,		//*13*/ {"大金空调"},
    ASR_AIR_14,		//*14*/ {"三菱空调"},
    ASR_AIR_15,		//*15*/ {"小米空调"},
    ASR_AIR_16,		//*16*/ {"惠而浦空调"},
    ASR_AIR_17,		//*17*/ {"格兰仕空调"},
    ASR_AIR_18,		//*18*/ {"富士通空调"},
    ASR_AIR_19,		//*19*/ {"山水空调"},
    ASR_AIR_20,		//*20*/ {"约克空调"},
    ASR_AIR_21,		//*21*/ {"创维空调"},
    ASR_AIR_22,		//*22*/ {"新科空调"},
    ASR_AIR_23,		//*23*/ {"春兰空调 "},
    ASR_AIR_24,		//*24*/ {"樱花空调"},
    ASR_AIR_25,		//*25*/ {"三星空调"},
    ASR_AIR_26,		//*26*/ {"澳柯玛空调"},
    ASR_AIR_27,		//*27*/ {"新飞空调"},
    ASR_AIR_28,		//*28*/ {"东芝空调"},
    ASR_AIR_29,		//*29*/ {"声宝空调"},
    ASR_AIR_30,		//*30*/ {"月兔空调"},
    ASR_AIR_31,		//*31*/ {"扬子空调"},

    ASR_CMD_001,	///*01*/ {"匹配空调"},
    ASR_CMD_002,	///*02*/ {"退出匹配"},
    ASR_CMD_003,	///*03*/ {"打开空调"},
    ASR_CMD_004,	///*04*/ {"关闭空调"},
    ASR_CMD_005,	///*05*/ {"自动风速"},
    ASR_CMD_006,	///*06*/ {"自动模式"},
    ASR_CMD_007,	///*07*/ {"制冷模式"},
    ASR_CMD_008,	///*08*/ {"加热模式"},
    ASR_CMD_009,	///*09*/ {"制热模式"},
    ASR_CMD_010,	///*10*/ {"除湿模式"},
    ASR_CMD_011,	///*11*/ {"送风模式"},
    ASR_CMD_012,	///*12*/ {"中等风速"},
    ASR_CMD_013,	///*13*/ {"最小风速"},
    ASR_CMD_014,	///*14*/ {"最大风速"},
    ASR_CMD_015,	///*15*/ {"三十度"},
    ASR_CMD_016,	///*16*/ {"二十九度"},
    ASR_CMD_017,	///*17*/ {"二十八度"},
    ASR_CMD_018,	///*18*/ {"二十七度"},
    ASR_CMD_019,	///*19*/ {"二十六度"},
    ASR_CMD_020,	///*20*/ {"二十五度"},
    ASR_CMD_021,	///*21*/ {"二十四度"},
    ASR_CMD_022,	///*22*/ {"二十三度"},
    ASR_CMD_023,	///*23*/ {"二十二度"},
    ASR_CMD_024,	///*24*/ {"二十一度"},
    ASR_CMD_025,	///*25*/ {"二十度"},
    ASR_CMD_026,	///*26*/ {"十九度"},
    ASR_CMD_027,	///*27*/ {"十八度"},
    ASR_CMD_028,	///*28*/ {"十七度"},
    ASR_CMD_029,	///*29*/ {"十六度"},
    ASR_CMD_030,	///*30*/ {"打开扫风"},
    ASR_CMD_031,	///*31*/ {"关闭扫风"},
    ASR_CMD_032,	///*32*/ {"定时一小时"},
    ASR_CMD_033,	///*33*/ {"定时二小时"},
    ASR_CMD_034,	///*34*/ {"定时两小时"},
    ASR_CMD_035,	///*35*/ {"定时三小时"},
    ASR_CMD_036,	///*36*/ {"定时四小时"},
    ASR_CMD_037,	///*37*/ {"定时五小时"},
    ASR_CMD_038,	///*38*/ {"定时六小时"},
    ASR_CMD_039,	///*39*/ {"定时七小时"},
    ASR_CMD_040,	///*40*/ {"定时八小时"},
    ASR_CMD_041,	///*41*/ {"取消定时"},
    ASR_CMD_042,	///*42*/ {"搜索空调"},
    ASR_CMD_043,	///*43*/ {"停止搜索"},
    ASR_CMD_044,	///*44*/ {"下一套遥控码"},

    ASR_CMD_045,	//*45*/ {"打开设置"},
	ASR_CMD_046,	//*46*/ {"打开天气"},
	ASR_CMD_047,	//*47*/ {"打开血压"},
	ASR_CMD_048,	//*48*/ {"打开睡眠"},
	ASR_CMD_049,	//*49*/ {"打开闹钟"},
	ASR_CMD_050,	//*50*/ {"打开心率"},
	ASR_CMD_051,	//*51*/ {"查看运动记录"},
	ASR_CMD_052,	//*52*/ {"打开支付宝"},
	// ASR_CMD_053,	//*53*/ {"打开付款码"},
	ASR_CMD_055,	//*55*/ {"打开语音助手"},
    ASR_CMD_061,	//*61*/ {"查找手机"},
	ASR_CMD_062,	//*62*/ {"打开拨号"},
	ASR_CMD_063,	//*63*/ {"打开血氧"},
	ASR_CMD_064,	//*64*/ {"打开血糖"},
	ASR_CMD_065,	//*65*/ {"打开计算器"},
	ASR_CMD_066,	//*66*/ {"打开日历"},
    ASR_CMD_067,	//*67*/ {"打开音乐"},
    ASR_CMD_068,	//*68*/ {"打开短视频遥控器"},
    ASR_CMD_069,	//*69*/ {"打开微信支付"},
    ASR_CMD_070,	//*70*/ {"打开收款码"},
    ASR_CMD_071,	//*71*/ {"打开乘车码"},
    ASR_CMD_072,	//*72*/ {"打开GPT"},
    ASR_CMD_073,	//*74*/ {"打开常亮模式"},
    ASR_CMD_074,	//*74*/ {"关闭常亮模式"},
    ASR_CMD_075,	//*75*/ {"打开扫码连接"},
    ASR_CMD_076,	//*76*/ {"返回表盘"},

	ASR_CMD_057,	//*57*/ {"开始运动"},
	ASR_CMD_058,	//*58*/ {"停止运动"},
	ASR_CMD_059,	//*59*/ {"继续运动"},
	ASR_CMD_X59,	//*59*/ {"继续运动"},
	ASR_CMD_060,	//*60*/ {"暂停运动"},

    ASR_CMD_MAX
};

static const char str_asr_cmd[ASR_CMD_MAX][ASR_CMD_LEN] = {
#if MONITOR_ASR_WAKEUP_EN
    /*00*/ {"小智同学"},
    /*00*/ {"AI同学"},
#endif
    /*01*/ {"乐金空调"},
    /*02*/ {"TCL空调"},
    /*03*/ {"奥克斯空调"},
    /*04*/ {"志高空调"},
    /*05*/ {"松下空调"},
    /*06*/ {"格力空调"},
    /*07*/ {"海信空调"},
    /*08*/ {"海尔空调"},
    /*09*/ {"科龙空调"},
    /*10*/ {"美的空调"},
    /*11*/ {"长虹空调"},
    /*12*/ {"日立空调"},
    /*13*/ {"大金空调"},
    /*14*/ {"三菱空调"},
    /*15*/ {"小米空调"},
    /*16*/ {"惠而浦空调"},
    /*17*/ {"格兰仕空调"},
    /*18*/ {"富士通空调"},
    /*19*/ {"夏普空调"},
    /*20*/ {"三菱重工"},// {"约克空调"},
    /*21*/ {"创维空调"},
    /*22*/ {"新科空调"},
    /*23*/ {"春兰空调"},
    /*24*/ {"樱花空调"},//
    /*25*/ {"三星空调"},
    /*26*/ {"澳柯玛空调"},
    /*27*/ {"新飞空调"},
    /*28*/ {"东芝空调"},//
    /*29*/ {"声宝空调"},//
    /*30*/ {"月兔空调"},//
    /*31*/ {"扬子空调"},

	/*01*/ {"匹配空调"},
	/*02*/ {"退出匹配"},

	/*03*/ {"打开空调"},
	/*04*/ {"关闭空调"},
	/*05*/ {"自动风速"},
	/*06*/ {"自动模式"},
	/*07*/ {"制冷模式"},
	/*08*/ {"加热模式"},
	/*09*/ {"制热模式"},
	/*10*/ {"除湿模式"},
	/*11*/ {"送风模式"},
	/*12*/ {"中等风速"},
	/*13*/ {"最小风速"},
	/*14*/ {"最大风速"},
	/*15*/ {"三十度"},
	/*16*/ {"二十九度"},
	/*17*/ {"二十八度"},
	/*18*/ {"二十七度"},
	/*19*/ {"二十六度"},
	/*20*/ {"二十五度"},
	/*21*/ {"二十四度"},
	/*22*/ {"二十三度"},
	/*23*/ {"二十二度"},
	/*24*/ {"二十一度"},
	/*25*/ {"二十度"},
	/*26*/ {"十九度"},
	/*27*/ {"十八度"},
	/*28*/ {"十七度"},
	/*29*/ {"十六度"},
	/*30*/ {"打开扫风"},
	/*31*/ {"关闭扫风"},
	/*32*/ {"定时一小时"},
	/*33*/ {"定时二小时"},
	/*34*/ {"定时两小时"},
	/*35*/ {"定时三小时"},
	/*36*/ {"定时四小时"},
	/*37*/ {"定时五小时"},
	/*38*/ {"定时六小时"},
	/*39*/ {"定时七小时"},
	/*40*/ {"定时八小时"},
	/*41*/ {"取消定时"},
	/*42*/ {"搜索空调"},
	/*43*/ {"停止搜索"},
	/*44*/ {"下一套遥控码"},

    // watch
    /*45*/ {"打开设置"},
	/*46*/ {"打开天气"},
	/*47*/ {"打开血压"},
	/*48*/ {"打开睡眠"},
	/*49*/ {"打开闹钟"},
	/*50*/ {"打开心率"},
	/*51*/ {"查看运动记录"},
	/*52*/ {"打开支付宝"},
	// /*53*/ {"打开付款码"},
	/*54*/ {"打开语音助手"},
    /*55*/ {"查找手机"},
	/*56*/ {"打开拨号"},
	/*57*/ {"打开血氧"},
	/*58*/ {"打开血糖"},
	/*59*/ {"打开计算器"},
	/*60*/ {"打开日历"},
    /*61*/ {"打开音乐"},
    /*62*/ {"打开短视频"},
    /*63*/ {"打开微信支付"},
    /*64*/ {"打开收款码"},
    /*65*/ {"打开乘车码"},
    /*66*/ {"打开GPT"},
    /*67*/ {"打开常亮模式"},
    /*68*/ {"关闭常亮模式"},
    /*69*/ {"打开扫码连接"},
    /*70*/ {"返回表盘"},

	/*00*/ {"打开运动"},
	/*00*/ {"停止运动"},
	/*00*/ {"继续运动"},
	/*00*/ {"续运动"},
	/*00*/ {"暂停运动"},
};

void air_brand_play(u8 index)
{
    switch (index)
    {
    case ASR_AIR_01: bsp_asr_tone_play(RES_BUF_ASR_63_MP3, RES_LEN_ASR_63_MP3); break;
    case ASR_AIR_02: bsp_asr_tone_play(RES_BUF_ASR_64_MP3, RES_LEN_ASR_64_MP3); break;
    case ASR_AIR_03: bsp_asr_tone_play(RES_BUF_ASR_65_MP3, RES_LEN_ASR_65_MP3); break;
    case ASR_AIR_04: bsp_asr_tone_play(RES_BUF_ASR_66_MP3, RES_LEN_ASR_66_MP3); break;
    case ASR_AIR_05: bsp_asr_tone_play(RES_BUF_ASR_67_MP3, RES_LEN_ASR_67_MP3); break;
    case ASR_AIR_06: bsp_asr_tone_play(RES_BUF_ASR_68_MP3, RES_LEN_ASR_68_MP3); break;
    case ASR_AIR_07: bsp_asr_tone_play(RES_BUF_ASR_69_MP3, RES_LEN_ASR_69_MP3); break;
    case ASR_AIR_08: bsp_asr_tone_play(RES_BUF_ASR_70_MP3, RES_LEN_ASR_70_MP3); break;
    case ASR_AIR_09: bsp_asr_tone_play(RES_BUF_ASR_71_MP3, RES_LEN_ASR_71_MP3); break;
    case ASR_AIR_10: bsp_asr_tone_play(RES_BUF_ASR_72_MP3, RES_LEN_ASR_72_MP3); break;
    case ASR_AIR_11: bsp_asr_tone_play(RES_BUF_ASR_73_MP3, RES_LEN_ASR_73_MP3); break;
    case ASR_AIR_12: bsp_asr_tone_play(RES_BUF_ASR_74_MP3, RES_LEN_ASR_74_MP3); break;
    case ASR_AIR_13: bsp_asr_tone_play(RES_BUF_ASR_75_MP3, RES_LEN_ASR_75_MP3); break;
    case ASR_AIR_14: bsp_asr_tone_play(RES_BUF_ASR_76_MP3, RES_LEN_ASR_76_MP3); break;
    case ASR_AIR_15: bsp_asr_tone_play(RES_BUF_ASR_77_MP3, RES_LEN_ASR_77_MP3); break;
    case ASR_AIR_16: bsp_asr_tone_play(RES_BUF_ASR_78_MP3, RES_LEN_ASR_78_MP3); break;
    case ASR_AIR_17: bsp_asr_tone_play(RES_BUF_ASR_79_MP3, RES_LEN_ASR_79_MP3); break;
    case ASR_AIR_18: bsp_asr_tone_play(RES_BUF_ASR_80_MP3, RES_LEN_ASR_80_MP3); break;
    case ASR_AIR_19: bsp_asr_tone_play(RES_BUF_ASR_81_MP3, RES_LEN_ASR_81_MP3); break;
    case ASR_AIR_20: bsp_asr_tone_play(RES_BUF_ASR_82_MP3, RES_LEN_ASR_82_MP3); break;
    case ASR_AIR_21: bsp_asr_tone_play(RES_BUF_ASR_83_MP3, RES_LEN_ASR_83_MP3); break;
    case ASR_AIR_22: bsp_asr_tone_play(RES_BUF_ASR_84_MP3, RES_LEN_ASR_84_MP3); break;
    case ASR_AIR_23: bsp_asr_tone_play(RES_BUF_ASR_85_MP3, RES_LEN_ASR_85_MP3); break;
    case ASR_AIR_24: bsp_asr_tone_play(RES_BUF_ASR_86_MP3, RES_LEN_ASR_86_MP3); break;
    case ASR_AIR_25: bsp_asr_tone_play(RES_BUF_ASR_87_MP3, RES_LEN_ASR_87_MP3); break;
    case ASR_AIR_26: bsp_asr_tone_play(RES_BUF_ASR_88_MP3, RES_LEN_ASR_88_MP3); break;
    case ASR_AIR_27: bsp_asr_tone_play(RES_BUF_ASR_89_MP3, RES_LEN_ASR_89_MP3); break;
    case ASR_AIR_28: bsp_asr_tone_play(RES_BUF_ASR_90_MP3, RES_LEN_ASR_90_MP3); break;
    case ASR_AIR_29: bsp_asr_tone_play(RES_BUF_ASR_91_MP3, RES_LEN_ASR_91_MP3); break;
    case ASR_AIR_30: bsp_asr_tone_play(RES_BUF_ASR_92_MP3, RES_LEN_ASR_92_MP3); break;
    case ASR_AIR_31: bsp_asr_tone_play(RES_BUF_ASR_93_MP3, RES_LEN_ASR_93_MP3); break;

    default:
        break;
    }
}

u8  wakeup_flag;
volatile u8 ws_asr_cmd = ASR_CMD_MAX;

u8 ws_air_wake_get(void)
{
    return wakeup_flag;
}

AT(.text.func.process)
void ws_air_wake_clr(void)
{
    if (!bsp_asr_init_sta()) {
        return;
    }

    if (wakeup_flag) {
        #if (ASR_SELECT && ASR_VOICE_BALL_ANIM)
            compo_form_set_ai_voice_anim(false);
        #endif
    }
    wakeup_flag = 0;
    air_timer_set(10);
    asr_timer_cnt_clear();

#if ASR_AND_SIRI_PARALLEL_EN
        bt_siri_and_asr_off();
#endif

    printf("wake down...\r\n");
}

///处理识别结果，注意耗时不要太长，耗时太长的操作可以放到main process中进行
void asr_result_deal(const char *result)
{
    int i;
    for (i = 0; i < ASR_CMD_MAX; i++) {
        if (strcmp(result, str_asr_cmd[i]) == 0) {
            ws_asr_cmd = i;
            break;
        }
    }
    // printf("ws_asr_cmd:%d\n", i);
}

void asr_result_deal_process(void)
{
    if(ws_asr_cmd >= ASR_CMD_MAX)  return;

    u8 temp_cmd = ws_asr_cmd;
#if MONITOR_ASR_WAKEUP_EN
    if (ASR_AIR_01 <= temp_cmd && temp_cmd < ASR_CMD_MAX) {
        if (wakeup_flag) { //如果在唤醒状态，识别了控制指令则重新计时唤醒时间
            asr_timer_cnt_clear();
            air_timer_set(10);
            // ws_air_wake_clr(); //响应一次就关闭
            // if (temp_cmd == ASR_CMD_001 || temp_cmd == ASR_CMD_042) {      //匹配搜索空调则保持继续唤醒
            //     wakeup_flag = 1;
            // }
        } else { //在非唤醒状态，识别非唤醒词则不作处理
            ws_asr_cmd = ASR_CMD_MAX;
            return;
        }
    }
#endif
    ws_asr_cmd = ASR_CMD_MAX;
    // 空调匹配
#if IR_AIR_FUNC
    if (temp_cmd >= ASR_AIR_01 && temp_cmd <= ASR_AIR_31)
    {
        if (isd_get_air_match_status() == 5)
        {
            isd_set_ir_brand(temp_cmd - ASR_AIR_01);
            // bsp_asr_tone_play(RES_BUF_ASR_31_MP3, RES_LEN_ASR_31_MP3);
            air_brand_play(temp_cmd);
        }
        ir_cap_timer_stop();
        return;
    }
#endif

    switch (temp_cmd) {
    case ASR_CMD_WAKEUP:
    case ASR_CMD_WAKEUP1:
        wakeup_flag = 1;
        asr_timer_cnt_clear();
        printf("wakeup...\r\n");
        #if ASR_AND_SIRI_PARALLEL_EN
            #if ASR_SIRI_AUTO_CLOSE
                bt_siri_asr_time_reset();
            #endif
            bt_siri_and_asr_open();
        #endif
        #if (ASR_SELECT && ASR_VOICE_BALL_ANIM)
        compo_form_set_ai_voice_anim(true);
        #endif
        bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
        break;
#if IR_AIR_FUNC
    case ASR_CMD_001: ///*01*/ {"匹配空调"},
        mp3_end_oper = 1;
        bsp_asr_tone_play(RES_BUF_ASR_27_MP3, RES_LEN_ASR_27_MP3);
        asr_timer_cnt_clear();
        air_timer_set(20);

        // ir_cap_timer_start(5);
        break;
    case ASR_CMD_002: ///*02*/ {"退出匹配"},
        ir_cap_timer_stop();
        bsp_asr_tone_play(RES_BUF_ASR_28_MP3, RES_LEN_ASR_28_MP3);
        break;
    case ASR_CMD_003: ///*03*/ {"打开空调"},
        event_air_power_on();
        bsp_asr_tone_play(RES_BUF_ASR_05_MP3, RES_LEN_ASR_05_MP3);
        break;
    case ASR_CMD_004: ///*04*/ {"关闭空调"},
        event_air_power_off();
        bsp_asr_tone_play(RES_BUF_ASR_06_MP3, RES_LEN_ASR_06_MP3);
        break;
    case ASR_CMD_005: ///*05*/ {"自动风速"},
        event_air_wind_speed_auto();
        bsp_asr_tone_play(RES_BUF_ASR_07_MP3, RES_LEN_ASR_07_MP3);
        break;
    case ASR_CMD_006: ///*06*/ {"自动模式"},
        event_air_mode_auto();
        bsp_asr_tone_play(RES_BUF_ASR_08_MP3, RES_LEN_ASR_08_MP3);
        break;
    case ASR_CMD_007: ///*07*/ {"制冷模式"},
        event_air_mode_cool();
        bsp_asr_tone_play(RES_BUF_ASR_09_MP3, RES_LEN_ASR_09_MP3);
        break;
    case ASR_CMD_008: ///*08*/ {"加热模式"},
    case ASR_CMD_009: ///*09*/ {"制热模式"},
        event_air_mode_hot();
        bsp_asr_tone_play(RES_BUF_ASR_10_MP3, RES_LEN_ASR_10_MP3);
        break;
    case ASR_CMD_010: ///*10*/ {"除湿模式"},
        event_air_mode_wetted();
        bsp_asr_tone_play(RES_BUF_ASR_11_MP3, RES_LEN_ASR_11_MP3);
        break;
    case ASR_CMD_011: ///*11*/ {"送风模式"},
        event_air_mode_wind();
        bsp_asr_tone_play(RES_BUF_ASR_12_MP3, RES_LEN_ASR_12_MP3);
        break;

    case ASR_CMD_012: ///*12*/ {"中等风速"},
        event_air_wind_speed_mid();
        bsp_asr_tone_play(RES_BUF_ASR_13_MP3, RES_LEN_ASR_13_MP3);
        break;
    case ASR_CMD_013: ///*13*/ {"最小风速"},
        event_air_wind_speed_min();
        bsp_asr_tone_play(RES_BUF_ASR_14_MP3, RES_LEN_ASR_14_MP3);
        break;
    case ASR_CMD_014: ///*14*/ {"最大风速"},
        event_air_wind_speed_max();
        bsp_asr_tone_play(RES_BUF_ASR_15_MP3, RES_LEN_ASR_15_MP3);
        break;
    case ASR_CMD_015: ///*15*/ {"三十度"},
    case ASR_CMD_016: ///*16*/ {"二十九度"},
    case ASR_CMD_017: ///*17*/ {"二十八度"},
    case ASR_CMD_018: ///*18*/ {"二十七度"},
    case ASR_CMD_019: ///*19*/ {"二十六度"},
    case ASR_CMD_020: ///*20*/ {"二十五度"},
    case ASR_CMD_021: ///*21*/ {"二十四度"},
    case ASR_CMD_022: ///*22*/ {"二十三度"},
    case ASR_CMD_023: ///*23*/ {"二十二度"},
    case ASR_CMD_024: ///*24*/ {"二十一度"},
    case ASR_CMD_025: ///*25*/ {"二十度"},
    case ASR_CMD_026: ///*26*/ {"十九度"},
    case ASR_CMD_027: ///*27*/ {"十八度"},
    case ASR_CMD_028: ///*28*/ {"十七度"},
    case ASR_CMD_029: ///*29*/ {"十六度"},
        event_air_temperature_set(16 + ASR_CMD_029 - temp_cmd);
        bsp_asr_tone_play(RES_BUF_ASR_04_MP3, RES_LEN_ASR_04_MP3);
        break;

    case ASR_CMD_030: ///*30*/ {"打开扫风"},
        event_air_mode_direction();
        bsp_asr_tone_play(RES_BUF_ASR_16_MP3, RES_LEN_ASR_16_MP3);
        break;
    case ASR_CMD_031: ///*31*/ {"关闭扫风"},
        event_air_mode_stop_direction();
        bsp_asr_tone_play(RES_BUF_ASR_17_MP3, RES_LEN_ASR_17_MP3);
        break;
    case ASR_CMD_032: ///*32*/ {"定时一小时"},
        event_air_timer_set(1*6);
        bsp_asr_tone_play(RES_BUF_ASR_18_MP3, RES_LEN_ASR_18_MP3);
        break;
    case ASR_CMD_033: ///*33*/ {"定时二小时"},
    case ASR_CMD_034: ///*34*/ {"定时两小时"},
        event_air_timer_set(2*6);
        bsp_asr_tone_play(RES_BUF_ASR_19_MP3, RES_LEN_ASR_19_MP3);
        break;
    case ASR_CMD_035: ///*35*/ {"定时三小时"},
        event_air_timer_set(3*6);
        bsp_asr_tone_play(RES_BUF_ASR_20_MP3, RES_LEN_ASR_20_MP3);
        break;
    case ASR_CMD_036: ///*36*/ {"定时四小时"},
        event_air_timer_set(4*6);
        bsp_asr_tone_play(RES_BUF_ASR_21_MP3, RES_LEN_ASR_21_MP3);
        break;
    case ASR_CMD_037: ///*37*/ {"定时五小时"},
        event_air_timer_set(5*6);
        bsp_asr_tone_play(RES_BUF_ASR_22_MP3, RES_LEN_ASR_22_MP3);
        break;
    case ASR_CMD_038: ///*38*/ {"定时六小时"},
        event_air_timer_set(6*6);
        bsp_asr_tone_play(RES_BUF_ASR_23_MP3, RES_LEN_ASR_23_MP3);
        break;
    case ASR_CMD_039: ///*39*/ {"定时七小时"},
        event_air_timer_set(7*6);
        bsp_asr_tone_play(RES_BUF_ASR_24_MP3, RES_LEN_ASR_24_MP3);
        break;
    case ASR_CMD_040: ///*40*/ {"定时八小时"},
        event_air_timer_set(8*6);
        bsp_asr_tone_play(RES_BUF_ASR_25_MP3, RES_LEN_ASR_25_MP3);
        break;
    case ASR_CMD_041: ///*41*/ {"取消定时"},
        event_air_timer_set(0);
        bsp_asr_tone_play(RES_BUF_ASR_26_MP3, RES_LEN_ASR_26_MP3);
        break;
    case ASR_CMD_042: ///*42*/ {"搜索空调"},
        ir_cap_timer_stop();
        mp3_end_oper = 2;
        bsp_asr_tone_play(RES_BUF_ASR_29_MP3, RES_LEN_ASR_29_MP3);
        // isd_search_remote_onebyone();
        break;
    case ASR_CMD_043: ///*43*/ {"停止搜索"},
        isd_exit_search_remote();
        bsp_asr_tone_play(RES_BUF_ASR_30_MP3, RES_LEN_ASR_30_MP3);
        break;
    case ASR_CMD_044: ///*44*/ {"下一套遥控码"},
        isd_brand_get_next_remote(isd_search_type);
        bsp_asr_tone_play(RES_BUF_ASR_04_MP3, RES_LEN_ASR_04_MP3);
        break;
#endif //IR_AIR_FUNC


    case ASR_CMD_045: //*45*/ {"打开设置"},
        bsp_asr_func_switch(FUNC_SETTING);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_046: //*46*/ {"打开天气"},
        bsp_asr_func_switch(FUNC_WEATHER);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_047: //*47*/ {"打开血压"},
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_048: //*48*/ {"打开睡眠"},
        bsp_asr_func_switch(FUNC_SLEEP);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_049: //*49*/ {"打开闹钟"},
        bsp_asr_func_switch(FUNC_ALARM_CLOCK);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_050: //*50*/ {"打开心率"},
        bsp_asr_func_switch(FUNC_HEARTRATE);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_051: //*51*/ {"查看运动记录"},
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_052: //*52*/ {"打开支付宝"},
        #if SECURITY_PAY_EN
        bsp_asr_func_switch(FUNC_ALIPAY);
        #endif // SECURITY_PAY_EN
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    // case ASR_CMD_053: //*53*/ {"打开付款码"},
    //     bsp_asr_tone_play(RES_BUF_ASR_42_MP3, RES_LEN_ASR_42_MP3);
    //     break;

    case ASR_CMD_055: //*55*/ {"打开语音助手"},
        bsp_asr_func_switch(FUNC_VOICE);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;

    case ASR_CMD_057: //*57*/ {"开始运动"},
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_058: //*58*/ {"停止运动"},
        bsp_asr_tone_play(RES_BUF_ASR_58_MP3, RES_LEN_ASR_58_MP3);
        break;
    case ASR_CMD_059: //*59*/ {"继续运动"},
    case ASR_CMD_X59: //*59*/ {"续运动"},
        bsp_asr_tone_play(RES_BUF_ASR_48_MP3, RES_LEN_ASR_48_MP3);
        break;
    case ASR_CMD_060: //*60*/ {"暂停运动"},
        bsp_asr_tone_play(RES_BUF_ASR_59_MP3, RES_LEN_ASR_59_MP3);
        // bsp_asr_tone_play(RES_BUF_ASR_58_MP3, RES_LEN_ASR_58_MP3);
        break;
    case ASR_CMD_061: //*61*/ {"查找手机"},
        bsp_asr_func_switch(FUNC_FINDPHONE);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;

    case ASR_CMD_062: //*45*/ {"打开拨号"},
        bsp_asr_func_switch(FUNC_CALL_SUB_DIAL);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_063: //*46*/ {"打开血氧"},
        bsp_asr_func_switch(FUNC_BLOOD_OXYGEN);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_064: //*47*/ {"打开血糖"},
        bsp_asr_func_switch(FUNC_BLOODSUGAR);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_065: //*48*/ {"打开计算器"},
        bsp_asr_func_switch(FUNC_CALCULATOR);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_066: //*49*/ {"打开日历"},
        bsp_asr_func_switch(FUNC_CALENDAER);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;


    case ASR_CMD_067: //*47*/ {"打开音乐"},
        bsp_asr_func_switch(FUNC_BT);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_068: //*48*/ {"打开短视频遥控器"},
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_069: //*49*/ {"打开微信支付"},
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_070: //*47*/ {"打开收款码"},
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    // case ASR_CMD_071: //*48*/ {"打开乘车码"},
    //     bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
    //     break;
    case ASR_CMD_072: //*49*/ {"打开GPT"},
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_073: //*48*/ {"打开常亮模式"},
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_074: //*48*/ {"关闭常亮模式"},
        bsp_asr_tone_play(RES_BUF_ASR_60_MP3, RES_LEN_ASR_60_MP3);
        break;
    case ASR_CMD_075: //*49*/ {"打开扫码连接"},
        bsp_asr_func_switch(FUNC_SCAN);
        bsp_asr_tone_play(RES_BUF_ASR_57_MP3, RES_LEN_ASR_57_MP3);
        break;
    case ASR_CMD_076: //*49*/ {"返回表盘"},
        bsp_asr_func_switch(FUNC_CLOCK);
        bsp_asr_tone_play(RES_BUF_ASR_62_MP3, RES_LEN_ASR_62_MP3);
        break;

    default:
        break;
    }
}


void asr_event_process()
{
    if(sys_cb.mp3_res_playing || !bsp_asr_init_sta()) {
        return;
    }

    if (bsp_asr_init_sta() && !dac_get_power_status()) {
        bsp_asr_vddon();
    }

#if ASR_AND_SIRI_PARALLEL_EN
#if ASR_SIRI_AUTO_CLOSE
    bt_siri_asr_auto_close();
#endif
    if (sys_cb.asr_mp3_delay && !sco_get_incall_flag(INCALL_FLAG_SCO)) {
        sys_cb.asr_mp3_delay = 0;
        bsp_asr_tone_play(sys_cb.asr_mp3_addr, sys_cb.asr_mp3_lens);
    }
#endif

    //asr 启动/停止
    static u8 s_asr_start_flag = 1;
    if(s_asr_start_flag != asr_start_flag) {
        s_asr_start_flag = asr_start_flag;
        if(s_asr_start_flag) {
            extern void asr_alg_start(void);
            asr_alg_start();
        }
    }

//asr唤醒
#if MONITOR_ASR_WAKEUP_EN
    extern int asr_is_timeout(unsigned int timeOut);
    extern u8 wakeup_flag;
    if (wakeup_flag && asr_is_timeout(asr_out_timer)) { //退出唤醒
        // wakeup_flag = 0;
        // asr_timer_cnt_clear();
        // air_timer_set(10);
        ws_air_wake_clr();
        printf("wake down...\r\n");
#if IR_AIR_FUNC
        if (isd_get_air_match_status() == 5) { //匹配超时
            ir_cap_timer_stop();
            bsp_asr_tone_play(RES_BUF_ASR_51_MP3, RES_LEN_ASR_51_MP3);
        } else {
            bsp_asr_tone_play(RES_BUF_ASR_03_MP3, RES_LEN_ASR_03_MP3);
        }
#endif
    }
#endif


//空伴
#if IR_AIR_FUNC
    extern u8 ir_cap_flag;
    extern u32 ir_cap_tick;
    if(ir_cap_flag == 2) {
        if (tick_check_expire(ir_cap_tick, 200)) {
            ir_cap_flag = 3;
            ir_cap_tick = tick_get();
        }
    }
    if(ir_cap_flag == 3) {
        ir_cap_flag = 4;
        ir_timeout();
    }
    extern void air_time_process();
    air_time_process();
#endif

    asr_result_deal_process();

}

#if IR_AIR_FUNC
// 等待提示音播报结束，空调匹配、搜索才开始执行
void mp3_end_handle()
{
    if(mp3_end_oper == 1) { // 匹配空调
        asr_timer_cnt_clear();
        air_timer_set(20);
        ir_cap_timer_start(5);
    }
    else if(mp3_end_oper == 2) { // 搜索空调
        asr_timer_cnt_clear();
        air_timer_set(20);
        isd_search_remote_onebyone();
    }
    else if(mp3_end_oper == 3) { // 下一套
        asr_timer_cnt_clear();
        air_timer_set(20);
        isd_get_next_remote();
    }
    mp3_end_oper = 0;
}
#endif

#endif //ASR_WS_AIR
