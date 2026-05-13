#include "include.h"

#if (ASR_SELECT == ASR_WS)

///语音识别处理例程，客户根据需要修改命令列表和处理流程
#define ASR_CMD_LEN         25 //3 * 字数 + 1

u8  wakeup_flag;
u32 asr_wakeup_tick_timer;

void compo_form_set_ai_voice_anim(u8 sta);

void asr_timer_cnt_clear(void)
{
    // asr_wakeup_tick_timer = 0;
    asr_wakeup_tick_timer = tick_get();
}
int asr_is_timeout(unsigned int timeOut)
{
    // return asr_wakeup_tick_timer > timeOut ? 1 : 0;
    return tick_check_expire(asr_wakeup_tick_timer, timeOut*1000);
}



enum {
    ASR_CMD_WAKEUP,
    ASR_CMD_01,
    ASR_CMD_02,
    ASR_CMD_03,
    ASR_CMD_04,
    ASR_CMD_05,
    ASR_CMD_06,
    ASR_CMD_07,
    ASR_CMD_08,
    ASR_CMD_09,
    ASR_CMD_10,
    ASR_CMD_11,
    ASR_CMD_12,
    ASR_CMD_13,
    ASR_CMD_14,
    ASR_CMD_15,
    ASR_CMD_16,
    ASR_CMD_17,
    ASR_CMD_18,
    ASR_CMD_19,
    ASR_CMD_20,
    ASR_CMD_21,

    ASR_CMD_MAX
};

static const char str_asr_cmd[ASR_CMD_MAX][ASR_CMD_LEN] = {
	/*00*/ {"小天管家"},

	/*01*/ {"打开设置"},
	/*02*/ {"打开音乐"},
	/*03*/ {"打开短视频遥控器"},
	/*04*/ {"打开天气"},
	/*05*/ {"打开血氧"},
	/*06*/ {"打开睡眠"},
	/*07*/ {"打开闹钟"},
	/*08*/ {"打开心率"},
	/*09*/ {"打开支付宝"},
	/*10*/ {"打开微信支付"},
	/*11*/ {"打开收款码"},
	/*12*/ {"打开语音助手"},
	/*13*/ {"打开运动"},
	/*14*/ {"停止运动"},
	/*15*/ {"继续运动"},
	/*16*/ {"暂停运动"},
	/*17*/ {"打开常亮模式"},
	/*18*/ {"关闭常亮模式"},
	/*19*/ {"打开扫码连接"},
	/*20*/ {"查找手机"},
    /*21*/ {"返回表盘"},
};

u8 ws_wake_get(void)
{
    return wakeup_flag;
}

AT(.text.func.process)
void ws_wake_clr(void)
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
    asr_timer_cnt_clear();
#if ASR_AND_SIRI_PARALLEL_EN
        bt_siri_and_asr_off();
#endif
    printf("wake down...\r\n");
}

volatile u8 ws_asr_cmd = ASR_CMD_MAX;

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

AT(.text.func.process)
void asr_event_process()
{
    if(sys_cb.mp3_res_playing || !bsp_asr_init_sta())
    {
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
    // if(ws_asr_cmd >= ASR_CMD_MAX) return;
    u8 temp_cmd = ws_asr_cmd;
#if MONITOR_ASR_WAKEUP_EN
    if (ASR_CMD_WAKEUP < ws_asr_cmd && ws_asr_cmd < ASR_CMD_MAX) {
        if (wakeup_flag) { //如果在唤醒状态，识别了控制指令则重新计时唤醒时间
            asr_timer_cnt_clear();
            // ws_wake_clr();  //响应一次就关闭
        } else { //在非唤醒状态，识别非唤醒词则不作处理
            ws_asr_cmd = ASR_CMD_MAX;
            return;
        }
    }
#endif
    ws_asr_cmd = ASR_CMD_MAX;
    switch (temp_cmd) {
        case ASR_CMD_WAKEUP:
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
            bsp_asr_tone_play(RES_BUF_ASR_01_MP3, RES_LEN_ASR_01_MP3);
            break;

        case ASR_CMD_01:
            bsp_asr_func_switch(FUNC_SETTING);
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_02:
            bsp_asr_func_switch(FUNC_BT);
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_03:
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_04:
            bsp_asr_func_switch(FUNC_WEATHER);
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_05:
            bsp_asr_func_switch(FUNC_BLOOD_OXYGEN);
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_06:
            bsp_asr_func_switch(FUNC_SLEEP);
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_07:
            bsp_asr_func_switch(FUNC_ALARM_CLOCK);
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_08:
            bsp_asr_func_switch(FUNC_HEARTRATE);
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_09:
            #if SECURITY_PAY_EN
            bsp_asr_func_switch(FUNC_ALIPAY);
            #endif // SECURITY_PAY_EN
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_10:
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_11:
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_12:
            bsp_asr_func_switch(FUNC_VOICE);
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_13:
            bsp_asr_func_switch(FUNC_SPORT);
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_14:
            bsp_asr_tone_play(RES_BUF_ASR_04_MP3, RES_LEN_ASR_04_MP3);
            break;
        case ASR_CMD_15:
            bsp_asr_tone_play(RES_BUF_ASR_05_MP3, RES_LEN_ASR_05_MP3);
            break;
        case ASR_CMD_16:
            bsp_asr_tone_play(RES_BUF_ASR_06_MP3, RES_LEN_ASR_06_MP3);
            break;
        case ASR_CMD_17:
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_18:
            bsp_asr_tone_play(RES_BUF_ASR_03_MP3, RES_LEN_ASR_03_MP3);
            break;
        case ASR_CMD_19:
            bsp_asr_tone_play(RES_BUF_ASR_02_MP3, RES_LEN_ASR_02_MP3);
            break;
        case ASR_CMD_20:
            bsp_asr_func_switch(FUNC_FINDPHONE);
            bsp_asr_tone_play(RES_BUF_ASR_08_MP3, RES_LEN_ASR_08_MP3);
            break;
        case ASR_CMD_21:
            bsp_asr_func_switch(FUNC_CLOCK);
            bsp_asr_tone_play(RES_BUF_ASR_09_MP3, RES_LEN_ASR_09_MP3);
            break;
        ///...

        default:
            break;
    }

//asr唤醒
#if MONITOR_ASR_WAKEUP_EN
    if (wakeup_flag && asr_is_timeout(MONITOR_ASR_WAKEUP_TIME)) { //退出唤醒
        ws_wake_clr();
    }
#endif
}

#endif //ASR_SELECT