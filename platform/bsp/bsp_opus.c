#include "include.h"
#include "bsp_opus.h"

#if OPUS_ENC_EN

//支持
////16K采样率以下的  (20ms帧长)： 32kbps以下 的 单通道 压缩
#define OPUS_ONCE_SIZE          (opus_pcm_len() * 2)
#define OPUS_IN_BUF_SIZE        (320 * 2 * 2)
#define OPUS_OUT_BUF_SIZE       (80 * 2 * 5)

extern u16 opus_pcm_enc_len(void);     //压缩后一帧数据字节数输出
extern u16 opus_pcm_len(void);          //一次输入压缩的帧长
extern int opus_enc_frame(s16 *pcm, u8 *packet);
extern void opus_kick_start(void);
extern int sco_set_mic_gain_after_aec(void);
extern void mic_post_gain_process_s(s16 *ptr, int gain, int samples);
extern bool opus_enc_init(u32 spr, u32 nch, u32 bitrate);
extern void opus_enc_exit(void);


static u8 opus_sysclk, opus_init = 0;
static uint8_t opus_skip_frame = 0;


static u8 opus_in_buf[OPUS_IN_BUF_SIZE] AT(.opus.buf.bsp);
static u8 opus_out_buf[OPUS_OUT_BUF_SIZE] AT(.opus.buf.bsp);
static au_stm_t opus_in_stm AT(.opus.buf.bsp);
static au_stm_t opus_out_stm AT(.opus.buf.bsp);
static u8 opus_packet[320*2] AT(.opus.buf.bsp);

AT(.com_text.opus)
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


AT(.com_text.opus)
void opus_skip_frame_do(s16 *ptr, u32 samples)
{
    opus_skip_frame--;
    memset(ptr, 0, samples * 2);
}


//128点输入一次
AT(.com_text.sndp)
WEAK void opus_nr_process(s16 *buf)
{

}


AT(.com_text.opus)
static void opus_stm_put_do(u8 *buf, u16 len, au_stm_t *stm, char c)
{
    if ((stm->len + len) > stm->size) {
        remove_stm_buf(stm, len);
        uart_putchar(c);
    }

    puts_stm_buf(stm, buf, len);
}

AT(.com_text.opus)
static bool opus_stm_get_do(u8 *buf, u16 len, au_stm_t *stm)
{
    return gets_stm_buf(stm, buf, len);
}

AT(.com_text.opus)
void opus_put_pcm(u8 *buf, u16 len)
{
    opus_stm_put_do(buf, len, &opus_in_stm, '!');
}

AT(.com_text.opus)
static bool opus_get_pcm(u8 *buf, u16 len)
{
    return opus_stm_get_do(buf, len, &opus_in_stm);
}

//缓存压缩好的数据
AT(.com_text.opus)
void opus_put_frame(u8 *buf, u16 len)
{
    opus_stm_put_do(buf, len, &opus_out_stm, '@');
}

AT(.com_text.opus)
bool opus_get_frame(u8 *buf, u16 len)
{
	return opus_stm_get_do(buf, len, &opus_out_stm);
}


void opus_start_init(u8 nr_type)
{
    au_stm_t *stm = &opus_out_stm;
    stm->buf = stm->wptr = stm->rptr = opus_out_buf;
    stm->size = OPUS_OUT_BUF_SIZE;
    stm->len = 0;

    stm = &opus_in_stm;
    stm->buf = stm->wptr = stm->rptr = opus_in_buf;
    stm->size = OPUS_IN_BUF_SIZE;
    stm->len = 0;
}

void opus_stop_end(void)
{

}



AT(.com_text.opus)
WEAK void opus_sdadc_process(u8 *ptr, u32 samples, u32 ch_mode)
{
//    if (ch_mode > 1) {
//        uart_putchar('*');
//        return;
//    }


    if (opus_skip_frame) {
        opus_skip_frame_do((s16*)ptr, samples * (ch_mode & PCM_CHMASK));
        return;
    }

    opus_put_pcm(ptr, samples * 2 * (ch_mode & PCM_CHMASK));

    if (opus_in_stm.len >= OPUS_ONCE_SIZE) {  //当mic收集足够数据时，启动压缩
        opus_kick_start();
    }
}

AT(.com_text.opus)
void opus_enc_process(void)
{
    int rlen;
    if (!opus_get_pcm(opus_packet, OPUS_ONCE_SIZE)) {
        uart_putchar('#');
        return;
    }
    //print_r(opus_packet, OPUS_ONCE_SIZE);  //这一步收集pcm数据转mp3验证录音ok
    //return ;

    rlen = opus_enc_frame((void *)opus_packet, opus_packet);  //这一步进行编码

    // 640 / 8 = 80 //320 / 8 = 40
//    if(rlen < 0 || rlen != 80) {
    if(rlen < 0 || rlen != opus_pcm_enc_len()) {
        printf("opus encode failed: %d\n",rlen);
    } else {
//       print_r(opus_packet, rlen);
//       printf("opus:%d\n", rlen);
        opus_put_frame(opus_packet, rlen);
    }
}

AT(.text.opus.bsp)
bool bsp_opus_is_encode(void)
{
    return (opus_init == 1);
}

u16 opus_enc_data_len_get(void)
{
    return opus_out_stm.len;
}


AT(.text.opus.bsp)
void bsp_opus_encode_start(bool flag, u32 spr, u32 bitrate)
{
    // printf("opus_init:%d, bt_cb.disp_status:%d\n", opus_init, bt_cb.disp_status);
    if (opus_init == 0) { // && bt_cb.disp_status < BT_STA_INCOMING
        printf("--->bsp_opus_encode_start\n");
        if (flag == true) {
            bt_audio_bypass();
        }
//        if(opus_enc_init(SPR_16000, 1, 32000)) {
        if(opus_enc_init(spr, 1, bitrate)) {
            opus_sysclk = sys_clk_get();
            sys_clk_set(SYS_192M);
            opus_skip_frame = 125; //丢掉MIC刚启动时不稳定的数据
            opus_start_init(NR_TYPE_NONE);
            if (flag == true) {
                audio_path_init(AUDIO_PATH_OPUS);
                audio_path_start(AUDIO_PATH_OPUS);
            }
            opus_init = 1;
        }
     }
}

AT(.text.opus.bsp)
void bsp_opus_encode_stop(bool flag)
{
    if (opus_init) {
        printf("--->bsp_opus_encode_stop\n");
        opus_init = 0;
        opus_skip_frame = 0;
        if (flag == true) {
            audio_path_exit(AUDIO_PATH_OPUS);
        }
        opus_enc_exit();
        opus_stop_end();
        sys_clk_set(opus_sysclk);
        if (flag == true) {
            bt_audio_enable();
        }
    }
}

AT(.text.opus.bsp)
bool bsp_opus_get_enc_frame(u8 *buff, u16 len)
{
    if(opus_init && (buff != NULL)){
        return opus_get_frame(buff, len);
    }

    return false;
}

#endif  //OPUS_ENC_EN
