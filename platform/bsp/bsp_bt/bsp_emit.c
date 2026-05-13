#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

#if BT_EMIT_EN

#define EMIT_PCM_BUF_SIZE       4096*2*2
#define EMIT_SBCEN_BUF_SIZE     4096*2*2   //容易卡可加大此BUFFER

typedef struct {
    au_stm_t pcm;
    au_stm_t sbcen;
} emit_stm_t;

typedef struct {
    int fix_cnt;     //发包数修正
    u8 cnt;          //单个周期的发包计数
    u16 tick;        //发包定时器, 单位(ms)
    bool pending;    //是否需要挂起解码线程
    bool fix_allow;  //首个周期跳过修正
} emit_cb_t;

emit_stm_t emit_stm;
emit_cb_t emit_cb;
u8 emit_pcm_buf[EMIT_PCM_BUF_SIZE] AT(.custom_buf.platform);
u8 emit_sbcen_buf[EMIT_SBCEN_BUF_SIZE] AT(.custom_buf.platform);
static co_timer_t emit_timer;
void emit_fix_cnt_clr(void);
void sbc_encode_process(void);
void bt_put_ext_link_info(void *buf, u16 addr, u16 size);

//////////////////////////////emit info//////////////////////////////////



//AT(.com_rodata.bsp)
//const char sbcen_put[] = "sbcen put %d,%d\n";
//
//AT(.com_rodata.bsp)
//const char sbcen_get[] = "sbcen get %d,%d tick%d\n";
//////////////////////////////emit stm/////////////////////////////////
#if TRACE_EN


AT(.com_rodata.bsp)
const char pcm_put[] = "pcm put %d,%d tick%d\n";

//AT(.com_rodata.bsp)
//const char pcm_get[] = "pcm get %d tick%d\n";

//AT(.com_rodata.bsp)
//const char my_no_err_str[] = "use len:%d\n";
#endif
AT(.com_rodata.bsp)
const char sbcen_full[] = "sbcen full\n";

AT(.com_rodata.bsp)
const char pcm_str[] = "losing a frame, %d\n";

u16 sbcen_get_data_size(void);
void os_emit_sem_post(void);

//AT(.com_text.bsp.emit)
//bool bsp_puts_stm_buf(au_stm_t *stm, u8 *buf, u16 len)
//{
//    if ((stm->len + len) > stm->size) {
//        printf(pcm_str, 2);
//        remove_stm_buf(stm, len);
//    }
//    return puts_stm_buf(stm, buf, len);
//}

AT(.com_text.bsp.emit)
static void remove_stm_buf(au_stm_t *stm, u16 len)
{
    GLOBAL_INT_DISABLE();
    stm->wptr -= len;
    stm->len -= len;
    if (stm->wptr < stm->buf) {
        stm->wptr = stm->wptr + stm->size;
    }
    GLOBAL_INT_RESTORE();
}

AT(.com_text.bsp.emit)
bool sbcen_puts_buf(u8 *buf, u16 len)
{
//    printf(sbcen_put, len, sbcen_get_data_size());
#if BT_EMIT_HFP_EN
    if(bt_emit_sco_bypass){
        return false;
    }
#endif // BT_EMIT_HFP_EN
    au_stm_t *sbcen_stm = &emit_stm.sbcen;
    if ((sbcen_stm->len + len) > sbcen_stm->size) {  //fulling, losing a frame
//        remove_stm_buf(sbcen_stm,len);
        GLOBAL_INT_DISABLE();
        //防止一直卡
        sbcen_stm->buf = sbcen_stm->wptr = sbcen_stm->rptr = emit_sbcen_buf;
        sbcen_stm->len = 0;
        emit_fix_cnt_clr();
        GLOBAL_INT_RESTORE();
        printf(sbcen_full);
    }
    return puts_stm_buf(sbcen_stm, buf, len);
}

AT(.com_text.bsp.emit)
bool sbcen_gets_buf(u8 *buf, u16 len)
{
//    printf(sbcen_get, len, sbcen_get_data_size(), tick_get());
#if BT_EMIT_HFP_EN
    if(bt_emit_sco_bypass){
        return false;
    }
#endif // BT_EMIT_HFP_EN
    au_stm_t *sbcen_stm = &emit_stm.sbcen;
    return gets_stm_buf(sbcen_stm, buf, len);
}

AT(.com_text.sbcen)
u16 sbcen_get_data_size(void)
{
#if BT_EMIT_HFP_EN
    if(bt_emit_sco_bypass){
        return 0;
    }
#endif // BT_EMIT_HFP_EN
    au_stm_t *sbcen_stm = &emit_stm.sbcen;
    return sbcen_stm->len;
}

AT(.com_text.bsp.emit)
void emit_puts_pcm_obuf(u8 *inbuf, u16 len)
{
    if (get_music_dec_sta() != MUSIC_MSG_PLAY) {
//        TRACE(pcm_str, len);
        return;
    }

    au_stm_t *pcm_stm = &emit_stm.pcm;
    if ((pcm_stm->len + len) > pcm_stm->size) {  //fulling, losing a frame
        printf(pcm_str, pcm_stm->len);
        remove_stm_buf(pcm_stm, len);
        emit_cb.fix_cnt = 0;
    } else {
//        TRACE(my_no_err_str, pcm_stm->len + len);
    }
    puts_stm_buf(pcm_stm, inbuf, len);
    TRACE(pcm_put, len, pcm_stm->len, tick_get());

//    music_enc_control(ENC_MSG_SBC);
}

//读取音源pcm数据
AT(.com_text.bsp.emit)
bool emit_gets_pcm_obuf(u8 *buf, u16 len)
{
#if BT_EMIT_HFP_EN
    if(bt_emit_sco_bypass){
        return false;
    }
#endif // BT_EMIT_HFP_EN
    bool ret = false;
    au_stm_t *pcm_stm = &emit_stm.pcm;

//    TRACE(pcm_get, len, tick_get());
    ret = gets_stm_buf(pcm_stm, buf, len);
    return ret;
}

//读取音源pcm数据的长度
AT(.com_text.bsp.emit)
u16 pcm_get_data_size(void)
{
    au_stm_t *pcm_stm = &emit_stm.pcm;
    return pcm_stm->len;
}
//////////////////////////////emit stream ctrl/////////////////////////////////
//---双声道 16bit 44.1K---
//1s PCM数据量: 44100*4byte(pcm)
//一包emit数据量 = 5包sbc = 512*5byte(pcm)
//每秒发的emit数 = (44100*4)/(512*5) = 68.9cnt
//---双声道 16bit 48K-----
//每秒发的emit数 = (48000*4)/(512*5) = 75cnt
#define EMIT_CNT_LIMIT_DIVIDE       40                                              //emit发包时间均分份数
#define EMIT_CNT_LIMIT_PR_1S        1000                                            //每个调整周期时间, 单位(ms), 如1s, 固定发射44.1K, 则为68.9包emit
#define EMIT_CNT_LIMIT_PR           (EMIT_CNT_LIMIT_PR_1S/EMIT_CNT_LIMIT_DIVIDE)    //均等每份时间(ms)

#if (EMIT_CNT_LIMIT_DIVIDE == 10)
AT(.com_rodata.emit)
u8 emit_cnt_limit_divide[] = { 7, 14, 21, 28, 35, 42, 49, 56, 63, 69};              //1s 10均等分(10pr: 7 7 7 7 7 7 7 7 7 6)
#elif (EMIT_CNT_LIMIT_DIVIDE == 20)
AT(.com_rodata.emit)
u8 emit_cnt_limit_divide[] = { 3,  7, 10, 14, 17, 21, 24, 28, 31, 35, 38, 42, 45, 49, 52, 56,
                              59, 63, 66, 69};                                      //1s 20均等分(20pr: (3+4)*9 3+3)
#elif (EMIT_CNT_LIMIT_DIVIDE == 40)
AT(.com_rodata.emit)
u8 emit_cnt_limit_divide[] = { 1,  3,  5,  7,  8, 10, 12, 14, 15, 17, 19, 21, 22, 24, 26, 28,
                              29, 31, 33, 35, 36, 38, 40, 42, 43, 45, 47, 49, 50, 52, 54, 56,
                              57, 59, 61, 63, 64, 66, 67, 69};                      //1s 40均等分
#endif

AT(.com_rodata.bsp)
const char emit_1s_check_str[] = "ecnt[%d] init[%d] fix[%d] tick[%d], sbcen size:%d, pcm size:%d, scan:%x\n";

#if TRACE_EN

AT(.com_rodata.bsp)
const char emit_total_cnt[] = "s_cnt[%d] e_cnt[%d] tick[%d]\n";
AT(.com_rodata.bsp)
const char emit_10s_check_str[] = "10s_fix: limit[%d] fix[%d]\n";
#endif // TRACE_EN

//AT(.com_rodata.tx)
//static const char tx_str[] = "-->tx cnt:%d, tick:%d\n";

//AT(.com_rodata.tx)
//static const char tx2_str[] = "-->tx cnt 1S:%d\n";

AT(.com_text.bsp.emit)
void emit_count_add(void)       //发包计数
{
//    printf("emit tick[%d]\n", tick_get());
    int emit_cnt_limit = (int)emit_cnt_limit_divide[emit_cb.tick/EMIT_CNT_LIMIT_PR];

    //fix
    if (emit_cnt_limit + emit_cb.fix_cnt < 0) {
        emit_cnt_limit = 0;
    } else {
        emit_cnt_limit += emit_cb.fix_cnt;
    }

    //add for check limit
    emit_cb.cnt++;
//    printf(tx_str, emit_cb.cnt, tick_get());
    if (emit_cb.cnt >= emit_cnt_limit) {
        if (!emit_cb.pending) {
            emit_cb.pending = true;
        }
    }
}

bool emit_count_pending(void)
{
    return emit_cb.pending;
}

void emit_fix_cnt_clr(void)
{
    emit_cb.fix_cnt = 0;
}


AT(.com_text.bsp.emit)
void emit_aubuf_adjust(void)        //根据单位时间发包数进行调整
{
    if (emit_cb.fix_allow) {
        emit_cb.fix_cnt += (69 - emit_cb.cnt);
    } else {
        emit_cb.fix_allow = true;
    }

    printf(emit_1s_check_str, emit_cb.cnt, emit_cb.fix_allow, emit_cb.fix_cnt, tick_get(), sbcen_get_data_size(), pcm_get_data_size(), bt_get_scan());
    emit_cb.cnt = 0;                //清空发包计数
}

AT(.com_text.bsp.emit)
static void emit_timer_callback(co_timer_t *timer, void *param)
{
//    printf("emit_timer_callback tick:%d\n", tick_get());
    static u8 sec_cnt = 0;
    emit_cb.tick += EMIT_CNT_LIMIT_PR;

    if (get_music_dec_sta() != MUSIC_MSG_PLAY) {
        return;
    }

    int emit_cnt_limit = emit_cnt_limit_divide[emit_cb.tick/EMIT_CNT_LIMIT_PR];
    if (emit_cnt_limit + emit_cb.fix_cnt > 0 && (emit_cnt_limit > emit_cb.cnt)) {
        if (emit_cb.pending) {
            emit_cb.pending = false;
            os_emit_sem_post();
        }
    }

    if (emit_cb.tick % EMIT_CNT_LIMIT_PR_1S == 0) {

        static u32 s_cnt = 0;
        static u32 e_cnt = 0;
        s_cnt += 69;
        e_cnt += emit_cb.cnt;
        TRACE(emit_total_cnt, s_cnt, e_cnt, tick_get());

        emit_cb.tick = 0;
//        printf(tx2_str, emit_cb.cnt);
        emit_aubuf_adjust();
        sec_cnt++;
    }

    if (sec_cnt && sec_cnt % 10 == 0) {
        sec_cnt = 0;
        emit_cb.fix_cnt -= 1;
        TRACE(emit_10s_check_str, emit_cnt_limit, emit_cb.fix_cnt);
    }
}

void emit_timer_reset(void)
{
//    printf("%s:%d\n", __func__, tick_get());
    co_timer_enable(&emit_timer, true);
    memset(&emit_cb, 0, sizeof(emit_cb_t));
}
////////////////////////////////////////////////////////////////////

//dac dmaout callback
AT(.com_text.emit.cb)
void msc_puts_pcm_obuf(u8 *buf, u32 samples, bool is24b)
{
    u32 size = samples << (1+is24b);

    //emit stream ctrl
    if (emit_count_pending()) {
        //os_emit_sem_pend();
    }
    emit_puts_pcm_obuf(buf, size);

	//sbcen
    sbc_encode_process();
}

void bt_emit_init(void)
{
    if (cfg_bt_emit_mode) {
        printf("bt_emit_init\n");
//        bt_init();
//        func_bt_init();
        bt_audio_enable();
        bt_scan_disable();
        dac_dnr_set_sta(0);
        //dac_power_off();
//        DAC_OUTPUT_DIS();
    }

    memset(&emit_stm, 0, sizeof(emit_stm_t));
    memset(&emit_cb, 0, sizeof(emit_cb_t));

    au_stm_t *sbcen_stm = &emit_stm.sbcen;
    au_stm_t *pcm_stm = &emit_stm.pcm;

    memset(emit_sbcen_buf, 0x00, sizeof(emit_sbcen_buf));
    memset(emit_pcm_buf, 0x00, sizeof(emit_pcm_buf));

    sbcen_stm->buf = sbcen_stm->wptr = sbcen_stm->rptr = emit_sbcen_buf;
    sbcen_stm->size = EMIT_SBCEN_BUF_SIZE;
    pcm_stm->buf = pcm_stm->wptr = pcm_stm->rptr = emit_pcm_buf;
    pcm_stm->size = EMIT_PCM_BUF_SIZE;

    sbc_encode_init(SPR_44100, 0x02);
    au0_dma_set_callback(msc_puts_pcm_obuf);
    au0_dma_start();

	co_timer_set(&emit_timer, EMIT_CNT_LIMIT_PR, TIMER_REPEAT, LEVEL_HIGH_PRI, emit_timer_callback, NULL);
	co_timer_enable(&emit_timer, false);

	//ble_set_adv_interval(3200);
}

void bsp_emit_start(void)
{
	bt_music_avdtp_start();
    delay_5ms(2);
    sys_cb.vol = 10;
    bsp_set_volume(sys_cb.vol);
    bsp_bt_vol_change();
    bt_music_slave_set_volume();
    bt_emit_enable();

}
#endif
