#include "include.h"

#if FUNC_MUSIC_EN

#define FS_CRC_SEED         0xffff

extern uint calc_crc(void *buf, uint len, uint seed);
extern void music_slot_kick(void);

static u8 file_fast_buff[4096] AT(.music_play.buf);
static fatfs_t music_fatfs;
bsp_msc_t msc_cb AT(.music_play.buf);


/**
 * 通过idx打开文件
 */
static bool bsp_music_open_idx(u32 idx, FIL* fp)
{
    FRESULT res = fs_node2file_info (&music_fatfs.stdscan, idx, &music_fatfs.fno);
    if(res == FR_OK){
        res = fs_openfno (fp, &music_fatfs.fno, FA_READ);
        if(res == FR_OK){
            fs_creat_fastmap(fp, file_fast_buff, sizeof(file_fast_buff));
        }
    }

    return (res == FR_OK);
}

AT(.text.stream)
int stream_read(void *buf, unsigned int size)
{
    UINT len;
    u8 res = fs_read(&music_fatfs.fp, buf, size, &len);
    if (res == FR_OK) {
#if MUSIC_ENCRYPT_EN
        if (msc_cb.encrypt) {
            music_stream_decrypt(buf, len);
        }
#endif
        return len;
    } else {
        return -1;
    }
}


AT(.text.stream)
bool stream_seek(unsigned int ofs, int whence)
{
    int ofs_curr;
#if MUSIC_ENCRYPT_EN
    if (msc_cb.encrypt) {
        if (whence == SEEK_SET) {
            ofs += 1;
        }
    }
#endif

    if(whence == SEEK_SET){
        ofs_curr = ofs;
    }else if(whence == SEEK_CUR){
        ofs_curr = fs_tell(&music_fatfs.fp);
        ofs_curr += ofs;
    }else{
        ofs_curr = fs_size(&music_fatfs.fp);
        ofs_curr += ofs;
    }
    if(ofs_curr < 0){
        ofs_curr = 0;
    }

    u8 res = fs_lseek(&music_fatfs.fp, ofs_curr);
    if (res == FR_OK) {
        return true;
    }
    return false;
}



/**
 * 音乐文件过滤器
 */
bool music_match_func(BYTE attr, char *name)
{
    if(!(attr & D_DIR)){
        if (strstr(name, ".mp3") != NULL) {
            return true;
        }
    #if MUSIC_AAC_SUPPORT
        if (strstr(name, ".aac") != NULL) {
            return true;
        }
    #endif
    #if MUSIC_WAV_SUPPORT
        if (strstr(name, ".wav") != NULL) {
            return true;
        }
    #endif
    #if MUSIC_APE_SUPPORT
        if (strstr(name, ".ape") != NULL) {
            return true;
        }
    #endif
    #if MUSIC_APE_SUPPORT
        if (strstr(name, ".fla") != NULL) {
            return true;
        }
    #endif
    }
    return false;
}


/**
 * 扫描磁盘
 */
bool bsp_music_scan_disk(u8 device)
{
    FRESULT res;
    static DWORD filenum, foldnum;

    msc_cb.cur_dev = device;
    music_fatfs.stdscan.fbuff     = music_fatfs.scanbuff;
    music_fatfs.stdscan.fbuff_len = sizeof(music_fatfs.scanbuff);
    music_fatfs.stdscan.foldbuff  = music_fatfs.scanfoldbuff;
    music_fatfs.stdscan.foldbuff_len  = sizeof(music_fatfs.scanfoldbuff);
    if(device == DEV_SDCARD){
        music_fatfs.stdscan.path    =  "B:\\MUSIC\0";
    }else if(device == DEV_SPIFLASH){
        music_fatfs.stdscan.path    =  "A:\\MUSIC\0";
    }else if(device == DEV_UDISK){
        music_fatfs.stdscan.path    =  "C:\\MUSIC\0";
    }
//    strcpy(music_fatfs.stdscan.pattern, "*.mp3");

    music_fatfs.stdscan.match_func = music_match_func;
    fs_chdrive (music_fatfs.stdscan.path);
    res = fs_scan_disk(&music_fatfs.stdscan, &filenum, &foldnum);
    printf("scan_disk:%d %d %d\n", res, filenum, foldnum);

    for (DWORD i=0; i<filenum; i++){
        res = fs_node2file_info (&music_fatfs.stdscan, i, &music_fatfs.fno);
        if(res == FR_OK){
            printf("info:%d : %s\n", i, music_fatfs.fno.fname);
        }
    }

    if((filenum == 0) || res){
        return false;
    }
    param_msc_num_read();
    msc_cb.file_num     = 0;
    msc_cb.file_total   = filenum;          //文件个数
    msc_cb.dir_num      = 0;
    msc_cb.dir_total    = foldnum;
    return true;
}


/**
 * 通过文件idx播放文件
 */
bool bsp_music_idx_play(u16 idx)
{
    music_control(MUSIC_MSG_STOP);

    printf("music idx:%d\n", idx);
    if(bsp_music_open_idx(idx, &music_fatfs.fp)){
        msc_cb.file_num = idx;
        fs_file2folder_index (&music_fatfs.stdscan, msc_cb.file_num, &msc_cb.dir_num);  //获取当前文件所在文件夹编号
        memcpy(msc_cb.fname, music_fatfs.fno.fname, sizeof(msc_cb.fname));
        msc_cb.alltime.min = 0xff;
        msc_cb.alltime.sec = 0;
        msc_cb.curtime.min = 0;
        msc_cb.curtime.sec = 0;
        msc_cb.pause = 0;
        msc_cb.encrypt = 0;
        msc_cb.type = NONE_TYPE;

    #if MUSIC_AAC_SUPPORT
        if (strstr(msc_cb.fname, ".aac") != NULL) {
            msc_cb.type = AACIO_TYPE;
        }
    #endif

    #if MUSIC_ENCRYPT_EN
        if (!music_is_encrypt(MUSIC_ENCRYPT_KEY, &music_fatfs.fp)) {
            msc_cb.encrypt = 1;
        }
    #endif

        if (func_cb.sta == FUNC_MUSIC) { //历史遗留的垃圾代码
            msc_cb.fname_update = 1;
        }

    #if MUSIC_LRC_EN
        bsp_lrc_init();
    #endif

    #if MUSIC_AAC_SUPPORT
        if (msc_cb.type == AACIO_TYPE) {
            if (!aacio_decode_init()) {
                msc_cb.type = NONE_TYPE;
            }
        }
    #endif

        if (msc_cb.type == NONE_TYPE) {
            msc_cb.type = music_decode_init();
        }

        if (msc_cb.type != NONE_TYPE) {
    #if MUSIC_ID3_TAG_EN
            if (msc_cb.type == MP3_TYPE) {
                get_mp3_id3_tag();
            } else if (msc_cb.type == WMA_TYPE) {
                get_wma_id3_tag();
            }
    #endif
            printf("music decoding [%s]\n", music_fatfs.fno.fname);
            printf("new file number: %d, %d\n", msc_cb.file_num, msc_cb.file_total);
            param_msc_num_write();
            music_breakpoint_init();
            param_sync();
            music_control(MUSIC_MSG_PLAY);             //开始播放
            delay_5ms(1);
        }
    }
    msc_cb.brkpt_flag = 0;

    return true;
}

/**
 * direction: 0->上一曲,    1->下一曲
 */
AT(.text.func.music)
void bsp_music_switch_file(u8 direction)
{
    u32 dir_snum, dir_lnum, dir_idx;
    music_control(MUSIC_MSG_STOP);

    printf("music sw:%d %d\n", direction, sys_cb.play_mode);
    switch (sys_cb.play_mode) {
        case SINGLE_MODE:
            break;
        case NORMAL_MODE:
            if (direction) {
                msc_cb.file_num++;
                if (msc_cb.file_num >= msc_cb.file_total) {
                    msc_cb.file_num = 0;
                }
            } else {
                if((msc_cb.file_num == 0) || (msc_cb.file_num >= msc_cb.file_total)){
                    msc_cb.file_num = msc_cb.file_total - 1;
                }else{
                    msc_cb.file_num--;
                }
            }
            break;

        case FLODER_MODE:
            fs_file2folder_index (&music_fatfs.stdscan, msc_cb.file_num, &dir_idx);
            fs_folder2file_index (&music_fatfs.stdscan, dir_idx, &dir_snum);
            fs_numoffolder (&music_fatfs.stdscan, dir_idx, &dir_lnum);
            dir_lnum = dir_snum + dir_lnum - 1;                     //获取当前文件夹结束文件编号

            if (direction) {
                msc_cb.file_num++;
                if (msc_cb.file_num > dir_lnum) {
                    msc_cb.file_num = dir_snum;
                }
            } else {
                msc_cb.file_num--;
                if ((msc_cb.file_num < dir_snum) || (msc_cb.file_num > dir_lnum)) {
                    msc_cb.file_num = dir_lnum;
                }
            }
            printf("Floder play mode: %d,%d\n", msc_cb.file_num, dir_lnum);
            break;

        case RANDOM_MODE:
            msc_cb.file_num = get_random(msc_cb.file_total);
            break;
    }

    bsp_music_idx_play(msc_cb.file_num);
    if (direction) {
        msc_cb.prev_flag = 0;
    } else {
        msc_cb.prev_flag = 1;
    }
}

/**
 * 播放暂停
 */
bool bsp_music_play(void)
{
    if (!msc_cb.pause) {
        return false;
    }
    msc_cb.pause = 0;
    music_control(MUSIC_MSG_PLAY);
    led_music_play();
    printf("%s\n", __func__);
    return true;
}

bool bsp_music_pause(void)
{

    if (msc_cb.pause) {
        return false;
    }

    msc_cb.pause = 1;
    bsp_clr_mute_sta();
    music_control(MUSIC_MSG_PAUSE);
    led_idle();
    printf("%s\n", __func__);
    return true;
}

void bsp_music_play_pause(void)
{
    if (msc_cb.pause) {
		bsp_music_play();
    } else {
        bsp_music_pause();
    }
}

/**
 * 播放时间
 */
AT(.text.func.music)
void bsp_music_get_curtime(void)
{
    u16 cur_sec, min, sec;
    cur_sec = music_get_cur_time() / 10;
    min = cur_sec / 60;
    sec = cur_sec % 60;
    if (msc_cb.curtime.min != min || msc_cb.curtime.sec != sec) {
        msc_cb.curtime.min = min;
        msc_cb.curtime.sec = sec;
    }
}

/**
 * 播放处理
 */
AT(.text.func.music)
void bsp_music_process(bool first_enter)
{
    bsp_music_get_curtime();
    if (music_get_total_time()) {
        music_slot_kick();
    }

#if MUSIC_LRC_EN
    static u32 lrc_tick = 0;
    if (get_music_dec_sta() == MUSIC_PLAYING  && tick_check_expire(lrc_tick, 500)) {
        lrc_tick = tick_get();
        bsp_lrc_get_content(msc_cb.lrc_show_callback);
    }
#endif

    if (get_music_dec_sta() == MUSIC_STOP) {
        if (dev_is_online(msc_cb.cur_dev) || (msc_cb.cur_dev == DEV_SPIFLASH)) {          //设备拔出结束解码不自动切换下一曲
            music_breakpoint_clr();
			if (first_enter) {
                bsp_music_idx_play(msc_cb.file_num);
            } else {
                if ((msc_cb.prev_flag) && ((msc_cb.alltime.min == 0xff)
                    || ((msc_cb.curtime.min == 0) && (msc_cb.curtime.sec == 0)))) {
                    //错误文件或播放小于2S保持切换方向
                    bsp_music_switch_file(0);
                } else {
                    bsp_music_switch_file(1);
                }
            }
        }
    }

    if (msc_cb.alltime.min == 0xff) {
        u16 total_time = music_get_total_time();
        if (total_time != 0xffff) {
            msc_cb.alltime.min = total_time / 60;
            msc_cb.alltime.sec = total_time % 60;
            printf("[%s] total time: %02d:%02d\n\n", msc_cb.fname, msc_cb.alltime.min, msc_cb.alltime.sec);
        }
    }
}


/**
 * 获取当前的播放文件名字
 */
u8* bsp_music_name(void)
{
    return (u8*)music_fatfs.fno.fname;
}



/**
 * 获取当前的播放文件fno
 */
FILINFO* bsp_music_fno(void)
{
    return &music_fatfs.fno;
}


#if MUSIC_BREAKPOINT_EN
/**
 * 以下是断点保存
 */
void bsp_music_breakpoint_clr(void)
{
    msc_cb.brkpt.file_ptr = 0;
    msc_cb.brkpt.frame_count = 0;
    param_msc_breakpoint_write();
    param_sync();
    //printf("%s\n", __func__);
}

void bsp_music_breakpoint_init(void)
{
    int clr_flag = 0;
    if (msc_cb.brkpt_flag) {
        msc_cb.brkpt_flag = 0;
        param_msc_breakpoint_read();
        if (calc_crc(msc_cb.fname, 8, FS_CRC_SEED) == msc_cb.brkpt.fname_crc) {
            music_set_jump(&msc_cb.brkpt);
        } else {
            clr_flag = 1;
        }
    }
    msc_cb.brkpt.fname_crc = calc_crc(msc_cb.fname, 8, FS_CRC_SEED);
    msc_cb.brkpt.file_ptr = 0;
    msc_cb.brkpt.frame_count = 0;
    if (clr_flag) {
        param_msc_breakpoint_write();
    }
}

void bsp_music_breakpoint_save(void)
{
    music_get_breakpiont(&msc_cb.brkpt);
    param_msc_breakpoint_write();
    param_sync();
}
#endif // MUSIC_BREAKPOINT_EN


/**
 * MP3提示音播放
 */
AT(.text.func.music)
void bsp_func_music_mp3_res_play(u32 addr, u32 len)
{
    u32 cur_time;

    if (len == 0) {
        return;
    }

    msc_breakpiont_t brkpt;
    music_get_breakpiont(&brkpt);           //保存当前播放位置
    cur_time = music_get_cur_time();
    music_control(MUSIC_MSG_STOP);

    mp3_res_play(addr, len);

    music_decode_init();
    music_set_jump(&brkpt);                 //恢复播放位置
    music_set_cur_time(cur_time);
    if (msc_cb.pause) {
        music_control(MUSIC_MSG_PAUSE);
    } else {
        music_control(MUSIC_MSG_PLAY);
    }
}

/**
 * 下面三个函数会被底层调用
 */
static u32 f_tell;
u32 fs_get_file_size(void)
{
    return fs_size(&music_fatfs.fp);
}

void fs_save_file_info(unsigned char *buf)
{
    f_tell = fs_tell(&music_fatfs.fp);
}

void fs_load_file_info(unsigned char *buf)
{
    fs_lseek(&music_fatfs.fp, f_tell);
}
#endif

