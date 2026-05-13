#include "include.h"
#include "third_func.h"

#if (ASR_SELECT == ASR_YJ)

#if THIRD_UI_LIGHT_EN
#include <lib_et_ui_io.h>
#include <lib_et_asr.h>
#include "third_bsp_sys.h"
#include "bsp_et_asr_ctrl.h"
#include "third_kws_event.h"

void play_sound(u32 eventId, u8 sound_en);
void third_et_func_kws_deal(u32 event);
void compo_form_set_ai_voice_anim(u8 sta);
static u32 et_kws_event = 0;

void set_et_kws_event(u32 event) {
    et_kws_event = event;
}

u8 et_wake_get(void)
{
    return t_pwn.et_wake_kws_en;
}

void et_wake_clr(void)
{
    if (!bsp_asr_init_sta()) {
        return;
    }
    //如果已经响应，则关闭唤醒状态
    if ((et_kws_event > UI_EVENT_XIAO_LAN_XIAO_LAN && et_kws_event < UI_EVENT_MAX) || t_pwn.et_wake_kws_t_cnt) {
        t_pwn.et_wake_kws_en = 0;
        t_pwn.et_wake_kws_t_cnt = 0;
#if (ASR_SELECT && ASR_VOICE_BALL_ANIM)
        compo_form_set_ai_voice_anim(false);
#endif
#if ASR_AND_SIRI_PARALLEL_EN
        bt_siri_and_asr_off();
#endif
    }
}

#if ET_A_WAKE_WORD_EN
void et_wake_kws_timer_tick();
#endif // ET_A_WAKE_WORD_EN

#if !ASR_DEAL_TYPE
int third_func_message(u16 msg)
{
    if(sys_cb.mp3_res_playing || !bsp_asr_init_sta())
    {
        return 0;
    }

    if (bsp_asr_init_sta() && !dac_get_power_status()) {
        bsp_asr_vddon();
    }

    int ret = 1;
    if(msg != 0x00 && msg != 0x7ff && msg != MSG_SYS_500MS && msg != MSG_SYS_1S ) {
        printf("msg:%d,%x\n",msg, msg);
    }

    u32 temp_event;
    switch (msg) {
        case EVT_ET_FUNC_KWS_DEAL:
        if(et_kws_event > 0) {
            temp_event = et_kws_event;
            et_kws_event = 0;
            third_et_func_kws_deal(temp_event);
        }
        break;
        case MSG_SYS_1S:
            #if ET_A_WAKE_WORD_EN
            et_wake_kws_timer_tick();
            #endif // ET_A_WAKE_WORD_EN
            break;
        default:
            ret = 0;
            break;
    }
    return ret;
}
#else
int third_func_process(void)
{
    if(sys_cb.mp3_res_playing || !bsp_asr_init_sta())
    {
        return 0;
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
    u32 temp_event;
    if(et_kws_event > 0) {
        temp_event = et_kws_event;
        et_kws_event = 0;
        third_et_func_kws_deal(temp_event);
    }

    static u32 et_kws_tick = 0;
    if (tick_check_expire(et_kws_tick, 1000)) {
        et_kws_tick = tick_get();
        #if ET_A_WAKE_WORD_EN
        et_wake_kws_timer_tick();
        #endif // ET_A_WAKE_WORD_EN
    }

    return 1;
}
#endif

#if ET_A_WAKE_WORD_EN
void et_wake_kws_timer_tick()
{
    if (t_pwn.et_wake_kws_en)
    {
        ET_LOGI("et_wake_kws_t_cnt:%d\n", t_pwn.et_wake_kws_t_cnt);
        if (t_pwn.et_wake_kws_t_cnt > 0) {
            t_pwn.et_wake_kws_t_cnt--;
        }
        else {
#if (ASR_SELECT && ASR_VOICE_BALL_ANIM)
            compo_form_set_ai_voice_anim(false);
#endif
            t_pwn.et_wake_kws_en = 0;
#if ET_VAD_WAKE_WORD_HOLD_EN && ET_ASR_VAD_EN
            extern u8 is_et_vad_need_hold();
            extern void set_et_vad_need_hold(u8 flag);
            set_et_vad_need_hold(0);
#endif // ET_VAD_WAKE_WORD_HOLD_EN

#if 0//ET_USER_UART0_EN
            et_uart_send_msg(ET_DAT_RECV_SHIYONGSHIQINGZAI, 1);
#endif // ET_USER_UART0_EN

#if ET_RX_REC_LOCAL_DEBUG_EN
#if ET_WARNING_TONE_EN
            //play_sound(ET_DAT_WOXIANTUIXIALE, t_pwn.sound_en);
            play_sound(ET_DAT_RECV_SHIYONGSHIQINGZAI, t_pwn.sound_en);
#endif // ET_WARNING_TONE_EN
#endif // ET_RX_REC_LOCAL_DEBUG_EN

        }
    }
}
#endif // ET_A_WAKE_WORD_EN



void third_et_func_kws_deal(u32 event)
{
    ET_LOGI("--kws_deal event:%d,:%x\n", event, event);
    #if ET_A_WAKE_WORD_EN
    //huanxingcichuli
    if((event >= UI_EVENT_XIAO_LAN_XIAO_LAN && event <= UI_EVENT_XIAO_LAN_XIAO_LAN)
       || event == UI_EVENT_XIAO_LAN_XIAO_LAN) {
        if(t_pwn.et_wake_kws_busy) {
            ET_LOGI("EVT_ET_A_WAKE_WORD busy!\n");
            return;
        }

        t_pwn.et_wake_kws_busy = 1;

        //third_ui_light_flash_times(1);

        #if ET_VAD_WAKE_WORD_HOLD_EN && ET_ASR_VAD_EN
        extern void set_et_vad_need_hold(u8 flag);
        set_et_vad_need_hold(1);
        sys_clk_set(SYS_192M);
        #endif // ET_VAD_WAKE_WORD_HOLD_EN


        #if ET_USER_UART0_EN && ET_ASR_VAD_EN
        //    udet_init(115200);
        udet_init(9600);
        register_uart0_isr_init(uart0_isr);
        //my_printf_init(et_bsp_empty_putchar_byte);//½«ËùÓÐ´òÓ¡printfÓ³Éäµ½¿Õº¯Êý
        #endif

        #if ET_USER_UART0_EN
        et_uart_send_msg(event, t_pwn.need_uart_send);
        #endif // ET_USER_UART0_EN
#if ASR_AND_SIRI_PARALLEL_EN
    #if ASR_SIRI_AUTO_CLOSE
        bt_siri_asr_time_reset();
    #endif
        bt_siri_and_asr_open();
#endif
#if (ASR_SELECT && ASR_VOICE_BALL_ANIM)
        compo_form_set_ai_voice_anim(true);
#endif
        #if ET_WARNING_TONE_EN
        play_sound(UI_EVENT_XIAO_LAN_XIAO_LAN, t_pwn.sound_en);
        #endif // ET_WARNING_TONE_EN

        t_pwn.et_wake_kws_t_cnt = ET_WAKE_TIME_DEFAULT;
        t_pwn.et_wake_kws_en = 1;
        t_pwn.et_wake_kws_busy = 0;
        return;
    }
    #endif // ET_A_WAKE_WORD_EN

    #if ET_A_WAKE_WORD_EN
    if(t_pwn.et_wake_kws_busy || !t_pwn.et_wake_kws_en) {
        return;
    }
    t_pwn.et_wake_kws_t_cnt = ET_WAKE_TIME_DEFAULT;
    t_pwn.et_wake_kws_en = 1;
    #endif // ET_A_WAKE_WORD_EN

    et_wake_clr();

    switch(event) {
    case UI_EVENT_DA_KAI_SHE_ZHI:
        {
            ET_LOGI("UI_EVENT_DA_KAI_SHE_ZHI\n");
            bsp_asr_func_switch(FUNC_SETTING);
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_SHE_ZHI, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_YIN_YUE:
        {
            ET_LOGI("UI_EVENT_DA_KAI_YIN_YUE\n");
            bsp_asr_func_switch(FUNC_BT);
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_YIN_YUE, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DUAN_SHI_PIN_YAO_KONG:
        {
            ET_LOGI("UI_EVENT_DUAN_SHI_PIN_YAO_KONG\n");
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DUAN_SHI_PIN_YAO_KONG, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_TIAN_QI:
        {
            ET_LOGI("UI_EVENT_DA_KAI_TIAN_QI\n");
            bsp_asr_func_switch(FUNC_WEATHER);
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_TIAN_QI, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_XUE_YANG:
        {
            ET_LOGI("UI_EVENT_DA_KAI_XUE_YANG\n");
            bsp_asr_func_switch(FUNC_BLOOD_OXYGEN);
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_XUE_YANG, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
#if 0
	case UI_EVENT_DA_KAI_XUE_YA:
        {
            ET_LOGI("UI_EVENT_DA_KAI_XUE_YA\n");
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_XUE_YA, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
#endif // 0
    case UI_EVENT_DA_KAI_SHUI_MIAN:
        {
            ET_LOGI("UI_EVENT_DA_KAI_SHUI_MIAN\n");
            bsp_asr_func_switch(FUNC_SLEEP);
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_SHUI_MIAN, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_NAO_ZHONG:
        {
            ET_LOGI("UI_EVENT_DA_KAI_NAO_ZHONG\n");
            bsp_asr_func_switch(FUNC_ALARM_CLOCK);
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_NAO_ZHONG, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_XIN_LV:
        {
            ET_LOGI("UI_EVENT_DA_KAI_XIN_LV\n");
            bsp_asr_func_switch(FUNC_HEARTRATE);
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_XIN_LV, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_ZHI_FU_BAO:
        {
            ET_LOGI("UI_EVENT_DA_KAI_ZHI_FU_BAO\n");
            #if SECURITY_PAY_EN
            bsp_asr_func_switch(FUNC_ALIPAY);
            #endif // SECURITY_PAY_EN
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_ZHI_FU_BAO, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_WEI_XIN_ZHI_FU:
        {
            ET_LOGI("UI_EVENT_DA_KAI_WEI_XIN_ZHI_FU\n");
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_WEI_XIN_ZHI_FU, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_SHOU_KUAN_MA:
        {
            ET_LOGI("UI_EVENT_DA_KAI_SHOU_KUAN_MA\n");
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_SHOU_KUAN_MA, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_CHENG_CHE_MA:
        {
            ET_LOGI("UI_EVENT_DA_KAI_CHENG_CHE_MA\n");
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_CHENG_CHE_MA, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_YU_YIN_ZHU_SHOU:
        {
            ET_LOGI("UI_EVENT_DA_KAI_YU_YIN_ZHU_SHOU\n");
            bsp_asr_func_switch(FUNC_VOICE);
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_YU_YIN_ZHU_SHOU, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_JI_PI_TI:
        {
            ET_LOGI("UI_EVENT_DA_KAI_JI_PI_TI\n");
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_JI_PI_TI, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_YUN_DONG:
        {
            ET_LOGI("UI_EVENT_DA_KAI_YUN_DONG\n");
            bsp_asr_func_switch(FUNC_SPORT);
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_YUN_DONG, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_TING_ZHI_YUN_DONG:
        {
            ET_LOGI("UI_EVENT_TING_ZHI_YUN_DONG\n");
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_TING_ZHI_YUN_DONG, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_JI_XU_YUN_DONG:
        {
            ET_LOGI("UI_EVENT_JI_XU_YUN_DONG\n");
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_JI_XU_YUN_DONG, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_ZAN_TING_YUN_DONG:
        {
            ET_LOGI("UI_EVENT_ZAN_TING_YUN_DONG\n");
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_ZAN_TING_YUN_DONG, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_CHANG_LIANG_MO_SHI:
        {
            ET_LOGI("UI_EVENT_DA_KAI_CHANG_LIANG_MO_SHI\n");
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_CHANG_LIANG_MO_SHI, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_GUAN_BI_CHANG_LIANG_MO_SHI:
        {
            ET_LOGI("UI_EVENT_GUAN_BI_CHANG_LIANG_MO_SHI\n");
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_GUAN_BI_CHANG_LIANG_MO_SHI, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_DA_KAI_SAO_MA_LIAN_JIE:
        {
            ET_LOGI("UI_EVENT_DA_KAI_SAO_MA_LIAN_JIE\n");
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_DA_KAI_SAO_MA_LIAN_JIE, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_CHA_ZHAO_SHOU_JI:
        {
            ET_LOGI("UI_EVENT_CHA_ZHAO_SHOU_JI\n");
            bsp_asr_func_switch(FUNC_FINDPHONE);
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_CHA_ZHAO_SHOU_JI, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    case UI_EVENT_FAN_HUI_BIAO_PAN:
        {
            ET_LOGI("UI_EVENT_FAN_HUI_BIAO_PAN\n");
            bsp_asr_func_switch(FUNC_CLOCK);
            #if ET_WARNING_TONE_EN
            play_sound(UI_EVENT_FAN_HUI_BIAO_PAN, t_pwn.sound_en);
            #endif // ET_WARNING_TONE_EN
        }
        break;
    default:
        break;
    }
}


void play_sound(u32 evendId, u8 sound_en) {
    printf("play_sound(),e:%x,e:%d, s:%d\n", evendId, evendId, sound_en);
    if(!sound_en) {
        return;
    }

    #if ET_WARNING_MP3_PLAY
    switch(evendId) {
    case UI_EVENT_XIAO_LAN_XIAO_LAN:
        //我在
        bsp_asr_tone_play(RES_BUF_ASR_K001_MP3, RES_LEN_ASR_K001_MP3);
        break;
    case UI_EVENT_TING_ZHI_YUN_DONG:
        //运动已停止
        bsp_asr_tone_play(RES_BUF_ASR_K002_MP3, RES_LEN_ASR_K002_MP3);
        break;
    case UI_EVENT_JI_XU_YUN_DONG:
        //运动已继续
        bsp_asr_tone_play(RES_BUF_ASR_K003_MP3, RES_LEN_ASR_K003_MP3);
        break;
    case UI_EVENT_ZAN_TING_YUN_DONG:
        //运动已暂停
        bsp_asr_tone_play(RES_BUF_ASR_K004_MP3, RES_LEN_ASR_K004_MP3);
        break;
    case UI_EVENT_DA_KAI_SHE_ZHI:
    case UI_EVENT_DA_KAI_YIN_YUE:
    case UI_EVENT_DUAN_SHI_PIN_YAO_KONG:
    case UI_EVENT_DA_KAI_TIAN_QI:
    case UI_EVENT_DA_KAI_XUE_YANG:
#if 0
    case UI_EVENT_DA_KAI_XUE_YA:
#endif // 0
    case UI_EVENT_DA_KAI_SHUI_MIAN:
    case UI_EVENT_DA_KAI_NAO_ZHONG:
    case UI_EVENT_DA_KAI_XIN_LV:
    case UI_EVENT_DA_KAI_ZHI_FU_BAO:
    case UI_EVENT_DA_KAI_WEI_XIN_ZHI_FU:
    case UI_EVENT_DA_KAI_SHOU_KUAN_MA:
    case UI_EVENT_DA_KAI_CHENG_CHE_MA:
    case UI_EVENT_DA_KAI_YU_YIN_ZHU_SHOU:
    case UI_EVENT_DA_KAI_JI_PI_TI:
    case UI_EVENT_DA_KAI_YUN_DONG:
    case UI_EVENT_DA_KAI_CHANG_LIANG_MO_SHI:
    case UI_EVENT_DA_KAI_SAO_MA_LIAN_JIE:
        //已打开
        bsp_asr_tone_play(RES_BUF_ASR_K005_MP3, RES_LEN_ASR_K005_MP3);
        break;
    case UI_EVENT_GUAN_BI_CHANG_LIANG_MO_SHI:
        //已关闭
        bsp_asr_tone_play(RES_BUF_ASR_K006_MP3, RES_LEN_ASR_K006_MP3);
        break;
    case UI_EVENT_CHA_ZHAO_SHOU_JI:
        //为你查找手机
        bsp_asr_tone_play(RES_BUF_ASR_K007_MP3, RES_LEN_ASR_K007_MP3);
        break;
    case UI_EVENT_FAN_HUI_BIAO_PAN:
        //已返回
        bsp_asr_tone_play(RES_BUF_ASR_K008_MP3, RES_LEN_ASR_K008_MP3);
        break;
    default:
        break;
    }
    #endif // ET_WARNING_MP3_PLAY

}

int is_et_ui_filter_keyword(int index, uint8_t  prefix_flag)
{
    return 0;
}

int is_et_ui_delay_keyword(int index, uint8_t prefix_flag)
{
#if 1
    if(prefix_flag == 254) {
        return 1;
    }
#endif
    return 0;
}

#endif //THIRD_UI_LIGHT_EN

#endif //ASR_SELECT
