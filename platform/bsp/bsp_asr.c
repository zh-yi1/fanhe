#include "include.h"

#if ASR_SELECT

#if (ASR_SELECT == ASR_YJ && ET_WAKEUP_EN)
#include <lib_et_asr.h>
#endif // ET_WAKEUP_EN

#define MIC_TO_DAC_TEST                 0                               //mic直推dac测试
#define MEASURE_IO                      0                               //IO翻转测试
#define AUBUFDATA                       SFR_RW (SFR1_BASE + 0x01*4)
#define RECORD_BUFFER_SIZE              0x1000                          //mic数据流缓存大小
#define DI_MICAUD_ENGF(bit)             AUANGCON3 = (AUANGCON3 & ~(1<<28)) | ((bit)<<28)
#if (ASR_SELECT == ASR_WS || ASR_SELECT == ASR_WS_AIR)
#define SPEECH_BUF_SIZE                 640 * 2                         //从数据流中获取1280byte数据，每个样点2byte
#elif (ASR_SELECT == ASR_YJ)
#define SPEECH_BUF_SIZE                 ET_VAD_ONCE_BUF_SIZE * 4 * 2    //从数据流中获取1280byte数据，每个样点2byte
#endif

//通用数据流管理
au_stm_t record_pcm_stream AT(.asr_record);

//数据流内部缓存4K byte
u8 rec_obuf[RECORD_BUFFER_SIZE] AT(.asr_record);

//取数据临时buffer
unsigned char speech_buf[SPEECH_BUF_SIZE] AT(.asr_record);

volatile u8 asr_init_flag;
volatile u8 asr_api_sta;

void sco_dump_init(void);
void bt_sco_dump(u32 ch, u32 index, s16 *ptr, u32 len);

//在此安全获取原始mic数据(main线程)
AT(.text.record)
void bsp_speech_recognition_process(void)
{
    if (asr_init_flag == 0) {
        return;
    }

    #if ASR_API_CHECK_TIME
        u32 start_time = tick_get();
    #endif

    u8 *buf = speech_buf;
    if (!gets_stm_buf(&record_pcm_stream, buf, SPEECH_BUF_SIZE)) {//从数据流中获取1280byte数据，每个样点2byte
        // printf("pipe line data less than 640 * 2 byte\n");
        return;
    }

#if MIC_TO_DAC_TEST
    sdadc_pcm_2_dac(buf, SPEECH_BUF_SIZE / 2, 1); //pcm_mode == 1，单声道
#endif // MIC_TO_DAC_TEST

    asr_api_sta = 1;
    bt_thread_check_trigger();          //kick 蓝牙线程
    //这里调用识别库:
#if MEASURE_IO
    GPIOB &= ~BIT(3);
    #if (ASR_SELECT == ASR_WS)          //华镇
        asr_alg_process((int16_t *)buf);
    #elif (ASR_SELECT == ASR_YJ)        //友杰
    //----------------------------------------------------------------
        #if 0
            extern int getPcmDB(int16_t* data, int size);
            int tempDb = getPcmDB((int16_t*)buf, ET_VAD_ONCE_BUF_SIZE * 4);
            printf("tempDb:%d\n", tempDb);
        #endif // 0
        #if ET_ASR_VAD_EN
        et_bsp_vad_detect(buf, ET_VAD_ONCE_BUF_SIZE * 4, ET_VAD_ONCE_BUF_SIZE, et_asr_vad_callback, et_asr_wordstart, et_asr_wordend);
        #else
        et_asr_buf_write((int16_t*)buf, ET_VAD_ONCE_BUF_SIZE * 4, 3);
        #endif // ET_ASR_VAD_EN
    //-----------------------------------------------------------------
    #endif
    GPIOB |= BIT(3);
#else
    #if (ASR_SELECT == ASR_WS || ASR_SELECT == ASR_WS_AIR)          //华镇
        asr_alg_process((int16_t *)buf);
        #if ASR_API_CHECK_TIME
            printf("asr_ws_time:%d\n", tick_get() - start_time);
        #endif //ASR_API_CHECK_TIME
    #elif (ASR_SELECT == ASR_YJ)        //友杰
        #if 0
            extern int getPcmDB(int16_t* data, int size);
            int tempDb = getPcmDB((int16_t*)buf, ET_VAD_ONCE_BUF_SIZE * 4);
            printf("tempDb:%d\n", tempDb);
        #endif // 0
        #if ET_WAKEUP_EN
            #if ET_ASR_VAD_EN
            et_bsp_vad_detect((int16_t*)buf, ET_VAD_ONCE_BUF_SIZE * 4, ET_VAD_ONCE_BUF_SIZE, et_asr_vad_callback, et_asr_wordstart, et_asr_wordend);
            #else
            et_asr_buf_write((int16_t*)buf, ET_VAD_ONCE_BUF_SIZE * 4, 3);
            #endif // ET_ASR_VAD_EN
        #endif // ET_WAKEUP_EN
        #if ASR_API_CHECK_TIME
            printf("asr_yj_time:%d\n", tick_get() - start_time);
        #endif //ASR_API_CHECK_TIME
    #endif // ASR_SELECT
#endif // MEASURE_IO
    asr_api_sta = 0;
#if  ASR_MIC_DATA_DUMP_EN
    bt_sco_dump(0,0,(s16*)buf, SPEECH_BUF_SIZE>>1);
#endif // ASR_MIC_DATA_DUMP_EN
}

//mic数据输入数据流
AT(.com_text.record)
bool puts_mic_obuf(u8 *inbuf, u16 len)
{
    bool puts_ok = puts_stm_buf(&record_pcm_stream, inbuf, len);
    return puts_ok;
}

//sdadc数据回调，不做耗时操作（aupcm线程）
AT(.com_text.sdadc)
void bsp_speech_recognition_sdadc_process(u8 *ptr, u32 samples, u32 pcm_mode)
{
    bool is_24bit = pcm_mode & PCM_24BIT;
    uint nch = pcm_mode & PCM_CHMASK;

    puts_mic_obuf(ptr, (u16)(samples << (nch + is_24bit)));
}

//用于语音识别纯净mic数据采集
void bsp_speech_recognition_enter(void)
{
    // printf(">>>enter %s\n", __func__);

    //数据流初始化
    memset(&record_pcm_stream, 0, sizeof(record_pcm_stream));
    record_pcm_stream.buf = 0;
    record_pcm_stream.buf = record_pcm_stream.rptr = record_pcm_stream.wptr = rec_obuf;
    record_pcm_stream.size = REC_OBUF_SIZE;
#if  ASR_MIC_DATA_DUMP_EN
    sco_dump_init();      //初始化mic数据dump功能
#endif // ASR_MIC_DATA_DUMP_EN
#if (ASR_SELECT == ASR_WS)      //初始化华镇语音识别库
        asr_alg_start();
#elif (ASR_SELECT == ASR_YJ)    //初始化友杰语音识别库
    #if ET_WAKEUP_EN
        et_start_asr();
    #endif // ET_WAKEUP_EN
#elif (ASR_SELECT == ASR_WS_AIR)
    DI_MICAUD_ENGF(1);
    asr_alg_start();
#if IR_AIR_FUNC
    ir_pwm_io_init();
    air_data_init();
#endif
#endif

#if MEASURE_IO
    GPIOBDE |= BIT(3);
    GPIOBFEN &= ~BIT(3);
    GPIOBDIR &= ~BIT(3);
    GPIOBSET = BIT(3);
#endif
}

void os_task_sleep(uint32_t ms);
void thread_asr_entry(void *param)
{
    printf("enter asr thread\n");
    // bsp_speech_recognition_enter();
    // os_task_sleep(1000);

    while (1) {
        bsp_speech_recognition_process();
    }
}

///创建asr线程
void thread_asr_create(void)
{
    static os_thread_t pthd_asr;
    pthd_asr = os_thread_create(
        "asr",
        thread_asr_entry,
        0,
        OS_THREAD_ASR_STACK,
        OS_THREAD_ASR_PRIORITY,
        OS_THREAD_ASR_TICK);

    if (pthd_asr != 0) {
        printf("startup asr thread\n");
        os_thread_startup(pthd_asr);
    }
    #if (ASR_SELECT == ASR_YJ)
        int8_t tem[8];
        extern void matmul_s(int8_t *A, int8_t *B, int32_t *C, int A_rows, int A_cols, int B_cols);
        matmul_s(tem, tem, (int32_t*)tem, 1, 8, 1);
    #endif
}

// void asr_loop_test(void) {
//     printf("enter asr loop test\n");
//     bsp_speech_recognition_enter();

//     while (1) {
//         WDT_CLR();
//         bsp_speech_recognition_process();
//     }
// }

//mic供电
void bsp_asr_vddon(void)
{
    dac_set_power_on_off(1);
}

//初始化
void bsp_asr_init(void)
{
#if (ASR_SELECT == ASR_WS || ASR_SELECT == ASR_WS_AIR)
    if (check_uid_entryption() == false) {
        printf("Dongle authorization verification failed!\n");
    } else {
        printf("Dongle authorization verification successful!\n");
    }
#elif (ASR_SELECT == ASR_YJ)
    et_init();
#endif
#if ASR_FULL_SCENE
    bsp_asr_start();
#endif
}

//开始
void bsp_asr_start(void)
{
    if (asr_init_flag || func_cb.sta == FUNC_BT_CALL || func_cb.sta == FUNC_BT_RING) {
        if (sys_cb.audio_path != AUDIO_PATH_ASR) {
            audio_path_init(AUDIO_PATH_ASR);
            audio_path_start(AUDIO_PATH_ASR);
        }
        return;
    }

    printf("bsp_asr_start\n");
    CLKGAT1 |= BIT(4)|BIT(5)|BIT(27);
    CLKGAT2 |= BIT(18);

    audio_path_init(AUDIO_PATH_ASR);
    audio_path_start(AUDIO_PATH_ASR);

#if MIC_TO_DAC_TEST
    bsp_change_volume(16);
    dac_fade_in();
    aubuf0_dma_init(0);
#endif // MIC_TO_DAC_TEST

    bsp_asr_vddon();
    bsp_speech_recognition_enter();
    asr_init_flag = true;
}

void bsp_asr_pause(void)
{
    if ((!asr_init_flag)) {
        return ;
    }
    printf("bsp_asr_pause\n");

    if(get_music_dec_sta() == MUSIC_PLAYING ||(sys_cb.sco_state)) {

    } else {
        dac_fade_out();
        dac_aubuf_clr();
    }

#if ASR_AND_SIRI_PARALLEL_EN
    bt_siri_and_asr_parallel_en_setter(false);
#endif

    audio_path_exit(AUDIO_PATH_ASR);
    while(asr_api_sta) {
        os_task_sleep(1);
        WDT_CLR();
    }
    bsp_asr_voice_wake_sta_clr();
    asr_init_flag = false;
#if (ASR_SELECT == ASR_WS)
    asr_alg_stop();
#elif (ASR_SELECT == ASR_YJ)
    et_stop_asr();
#elif (ASR_SELECT == ASR_WS_AIR)
    asr_alg_stop();
#endif
}

//结束
void bsp_asr_stop(void)
{
    if ((!asr_init_flag)) {
        return;
    }
#if (ASR_SELECT == ASR_WS)
    ws_wake_clr();
#elif (ASR_SELECT == ASR_YJ)
    et_wake_clr();
#elif (ASR_SELECT == ASR_WS_AIR)
#if IR_AIR_FUNC
    isd_exit_search_remote();
#endif
    ws_air_wake_clr();
#endif
    bsp_asr_pause();
    printf("bsp_asr_stop\n");
}

//初始化状态
u8 bsp_asr_init_sta(void)
{
    return asr_init_flag;
}

//播放提示音
void bsp_asr_tone_play(u32 addr, u32 len)
{
    if ((!asr_init_flag)) {
        return ;
    }
    sys_cb.asr_mp3_addr = addr;
    sys_cb.asr_mp3_lens = len;
    sys_cb.asr_gain = sys_cb.gain_offset;
    dac_set_anl_offset(0);
    func_bt_mp3_res_play(addr, len);
    dac_set_anl_offset(sys_cb.asr_gain);
}

void func_clock_sub_dropdown_exit(void);
//切换任务
void bsp_asr_func_switch(u8 func)
{
    if ((!asr_init_flag)) {
        return ;
    }
    if (func_cb.sta == FUNC_CLOCK) {
        if (((f_clock_t *)func_cb.f_cb)->sta == FUNC_CLOCK_SUB_DROPDOWN) {
            ((f_clock_t *)func_cb.f_cb)->sta = FUNC_CLOCK_MAIN;
            func_clock_sub_dropdown_exit();
        }
    }
    if (func_cb.sta != func) {
        func_switch_to(func, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);
    }
}

//获取语音唤醒状态
u8 bsp_asr_voice_wake_sta_get(void)
{
    if ((!asr_init_flag)) {
        return 0;
    }
#if (ASR_SELECT == ASR_WS)
    return ws_wake_get();
#elif (ASR_SELECT == ASR_YJ)
    return et_wake_get();
#elif (ASR_SELECT == ASR_WS_AIR)
    return ws_air_wake_get();
#endif
    return 0;
}

//清除语音唤醒状态
void bsp_asr_voice_wake_sta_clr(void)
{
    if ((!asr_init_flag)) {
        return;
    }
#if (ASR_SELECT == ASR_WS)
    // ws_wake_clr();
#elif (ASR_SELECT == ASR_YJ)
    et_wake_clr();
#elif (ASR_SELECT == ASR_WS_AIR)
    // ws_air_wake_clr();
#endif
}

//ASR进程
void bsp_asr_process(void)
{
    //mai释放时间片给asr运行
    if(bsp_asr_init_sta()) {
        if (ctp_is_touch()) {   //优化触摸时的屏幕帧率
            delay_5ms(1);
        } else {
            delay_5ms(2);
        }
    }
    //离线语音任务
#if (ASR_SELECT == ASR_YJ && ASR_DEAL_TYPE)
    third_func_process();
#endif
#if (ASR_SELECT == ASR_WS && MONITOR_ASR_WAKEUP_EN)
    asr_event_process();
#endif
#if (ASR_SELECT == ASR_WS_AIR && MONITOR_ASR_WAKEUP_EN)
    asr_event_process();
#endif
}

#if ASR_AND_SIRI_PARALLEL_EN
static bool enable_siri_and_asr = 0;
AT(.bt_voice.sco)
bool bt_siri_and_asr_parallel_en(void)
{
    return enable_siri_and_asr;
}

AT(.bt_voice.sco)
void bt_siri_and_asr_parallel_en_setter(bool is_en)
{
    enable_siri_and_asr = is_en;
}

void bt_siri_and_asr_open(void)
{
    if(bt_is_connected()){                            //打开siri
        printf("bt_siri_and_asr_open\n");
        bt_hfp_siri_switch();
    }
}

void bt_siri_and_asr_off(void)
{
    if(bt_is_connected() && bt_get_siri_status()){   //关闭siri
        printf("bt_siri_and_asr_off\n");
        bt_hfp_siri_switch();
    }
}

#if ASR_SIRI_AUTO_CLOSE
static u32 asr_siri_tick = 0;
void bt_siri_asr_time_reset(void)
{
    asr_siri_tick = tick_get();
}

void bt_siri_asr_auto_close(void)
{
    if (!bsp_asr_voice_wake_sta_get()) return;
    if (tick_check_expire(asr_siri_tick, ASR_SIRI_AUTO_CLOSE_COUNTDOWN*1000)) {
        bt_siri_asr_time_reset();
        bt_siri_and_asr_off();
    }
}
#endif // ASR_SIRI_AUTO_CLOSE
#endif // ASR_AND_SIRI_PARALLEL_EN

#endif //ASR_SELECT
