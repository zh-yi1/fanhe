#ifndef _BSP_MUSIC_H
#define _BSP_MUSIC_H

#include "music_lrc.h"
#include "music_id3_tag.h"
#include "music_res.h"

#define SEEK_SET            0
#define SEEK_CUR            1
#define SEEK_END            2

enum {
    NORMAL_MODE,
    SINGLE_MODE,
    FLODER_MODE,
    RANDOM_MODE,
};

enum {
    MUSIC_SRC_BT,
    MUSIC_SRC_WATCH,
    MUSIC_SRC_EARPHONE,
};

typedef struct {
    u8 min;                     //minute
    u8 sec;                     //second
} msc_time_t;

typedef struct {
    u32 frame_count;            //current frame count
    u32 file_ptr;               //file ptr of current frame
    u16 fname_crc;              //file name crc
} msc_breakpiont_t;

typedef struct {
    u8 pause        : 1,
       file_change  : 1,
       dev_change   : 2,
       brkpt_flag   : 1,
       prev_flag    : 1;
    u8 cur_dev;

    u8 rec_type     : 1,
       rec_scan     : 2,
       encrypt      : 1;

    u8 type;                    //音乐格式
    u32 bit_rate;               //音乐码率

    char fname[FF_LFN_BUF + 1];

    msc_time_t alltime;         //music file total time
    msc_time_t curtime;         //music current playing time

    u32 file_num;               //文件编号
    u32 file_total;             //文件个数

    u32 dir_num;                //directory current number
    u32 dir_total;              //directory total number

#if MUSIC_BREAKPOINT_EN
    msc_breakpiont_t brkpt;     //music breakpoint info
#endif // MUSIC_BREAKPOINT_EN

#if MUSIC_LRC_EN
    u8 lrc_sta;
    u8 lrc_encoding;
    void (*lrc_show_callback)(char*, u32);
#endif // MUSIC_LRC_EN

    u8 fname_update : 1,
       lrc_update   : 1;
} bsp_msc_t;
extern bsp_msc_t msc_cb;


/**
 * 扫描磁盘
 */
bool bsp_music_scan_disk(u8 device);

/**
 * 通过文件idx播放文件
 */
bool bsp_music_idx_play(u16 idx);

/**
 * direction: 0->上一曲,    1->下一曲
 */
void bsp_music_switch_file(u8 direction);

/**
 * 播放暂停
 */
void bsp_music_play_pause(void);
bool bsp_music_play(void);
bool bsp_music_pause(void);

/**
 * 播放时间
 */
void bsp_music_get_curtime(void);

/**
 * 播放处理
 */
void bsp_music_process(bool first_enter);

/**
 * 获取当前的播放文件名字
 */
u8* bsp_music_name(void);

/**
 * 获取当前的播放文件fno
 */
FILINFO* bsp_music_fno(void);


/**
 * 音乐播放模式下提示音
 */
void bsp_func_music_mp3_res_play(u32 addr, u32 len);

#define bsp_music_prev()            bsp_music_switch_file(0)        //上一曲
#define bsp_music_next()            bsp_music_switch_file(1)        //下一曲

#if MUSIC_BREAKPOINT_EN
void bsp_music_breakpoint_init(void);
void bsp_music_breakpoint_save(void);
void bsp_music_breakpoint_clr(void);

#define music_breakpoint_init()     bsp_music_breakpoint_init()
#define music_breakpoint_save()     bsp_music_breakpoint_save()
#define music_breakpoint_clr()      bsp_music_breakpoint_clr()
#else
#define music_breakpoint_init()
#define music_breakpoint_save()
#define music_breakpoint_clr()
#endif // MUSIC_BREAKPOINT_EN

#endif //_BSP_MUSIC_H
