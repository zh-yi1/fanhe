#ifndef _BSP_RECORD_H
#define _BSP_RECORD_H

#define REC_OBUF_SIZE                0x800               //ADC PCM缓存BUF SIZE
#define REC_ENC_SIZE                 0x1A00              //录音压缩数据缓存BUF SIZE
#define REC_SYNC_TIMES               5000                //间隔5S录音时间同步一次

enum {
    REC_STOP,
    REC_PAUSE,
    REC_RECORDING,
    REC_STARTING,
};

typedef struct {
    u8 spr;                         //采样率索引
    u8 nchannel;                    //channel number
    u8 rec_type;                    //录音音频格式
    u32 bitrate;                    //码率
    void (*source_start)(void);
    void (*source_stop)(void);
} rec_src_t;

///通用数据流Buffer管理
typedef struct {
    u32 size;                       //stream buf size
volatile u32 len;                   //stream buf valid data bytes
    u8 *buf;                        //stream buf start address
    u8 *wptr;                       //stream buf write point
    u8 *rptr;                       //stream buf read point
} au_stm_t;

typedef struct {
    u8 sta;                         //record state machine
    u8 flag_dir     : 1,            //目录已创建标志
       flag_file    : 1,            //文件已创建并打开标志
       flag_play    : 1,            //录音完是否跳到播放
       pause        : 1,            //是否录音暂停
       first_flag   : 1,
       reserved     : 3;
    u16 trigger_len;
    u32 tick;                       //sync time控制
    int dig_gain;
    rec_src_t *src;                 //录音输入源
    au_stm_t *enc;                 //压缩数据BUF管理
    au_stm_t *pcm;                 //输入PCM数据BUF管理
    bool mode;                     //录音模式
    u32 start_time;
    u32 time;
    bool is_rec_path_start;
    bool is_enter;
    bool is_noise_alg_start;
    bool is_opus_alg_start;
    bool is_set_bitrate;
    bool is_set_dig_gain;
} rec_cb_t;

bool puts_rec_obuf(u8 *inbuf, u16 len);
bool gets_rec_obuf(u8 *buf, u16 len);
bool puts_stm_buf(void *scb, u8 *buf, u32 len);
bool gets_stm_buf(void *scb, u8 *buf, u32 len);
void pcm_soft_vol_process(s16 *ptr, int gain, int samples);

bool bsp_record_start(bool is_sco, u8 type, u32 spr, u32 bitrate);
void bsp_record_process(void);
void bsp_record_stop(void);
void bsp_record_var_init(void);
u8 get_bsp_record_sta(void);
void bsp_record_continue(void);
void bsp_record_pause(void);

void bsp_record_exit(void);
void bsp_record_enter(void);
u32 bsp_record_get_rec_sec(void);
void bsp_record_set_mode(bool mode);
bool bsp_record_get_mode(void);
u8 bsp_record_get_type(void);
u8 bsp_record_get_next_type(u8 rec_type);
void bsp_record_set_bitrate(u32 btrate);
void bsp_record_set_dig_gain(int dig_gain);
bool is_bsp_record_set_bitrate(void);
bool is_bsp_record_set_dig_gain(void);
u32 bsp_record_get_bitrate(void);
u32 bsp_record_get_dig_gain(void);

#endif //_BSP_RECORD_H
