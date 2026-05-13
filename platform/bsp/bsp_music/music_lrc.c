#include "include.h"

#if MUSIC_LRC_EN
#define FNAME_LEN               100

static u8 lrc_buf[512] AT(.lrc_buf);
static char lrc_lfn[FNAME_LEN] AT(.lrc_buf);
static FIL lrc_fp AT(.lrc_buf);
static u8 lrc_show[FNAME_LEN] AT(.lrc_buf);


FRESULT fs_read_lrc(void* buff, UINT btr, UINT* br)
{
    return fs_read(&lrc_fp, buff, btr, br);
}

FRESULT fs_lseek_lrc(DWORD ofs, u8 whence)
{
    int ofs_curr;

    if(whence == SEEK_SET){
        ofs_curr = ofs;
    }else if(whence == SEEK_CUR){
        ofs_curr = fs_tell(&lrc_fp);
        ofs_curr += ofs;
    }else{
        ofs_curr = fs_size(&lrc_fp);
        ofs_curr += ofs;
    }
    if(ofs_curr < 0){
        ofs_curr = 0;
    }

    return fs_lseek(&lrc_fp, ofs_curr);
}

AT(.text.lrc.init)
void bsp_lrc_init(void)
{
    FRESULT res;
    u8 *fname = bsp_music_name();

    msc_cb.lrc_sta = 0;
    memcpy(lrc_lfn, fname, FNAME_LEN);

    for(u8 i=0; i<FNAME_LEN; i++){
        if(lrc_lfn[i] == '.'){
            if(i > (FNAME_LEN - 5)){
                return;
            }
            lrc_lfn[i+1] = 'l';
            lrc_lfn[i+2] = 'r';
            lrc_lfn[i+3] = 'c';
            lrc_lfn[i+4] = '\0';
            break;
        }

        if(i >= (FNAME_LEN - 5)){
            return;
        }
    }
    printf("lrc:%s\n", lrc_lfn);
    f_chdirfno(bsp_music_fno());
    res = fs_open (&lrc_fp, lrc_lfn, FA_READ);
    printf("res:%d\n", res);
    if (res == FR_OK) {
        msc_cb.lrc_sta = 1;
        msc_cb.lrc_encoding = lrc_init(lrc_buf);
        printf("got lrc file: %d, %d\n", msc_cb.lrc_sta, msc_cb.lrc_encoding);
    }
}

AT(.text.lrc)
static void bsp_lrc_sta_process(void)
{
    if (msc_cb.lrc_sta && (msc_cb.lrc_sta != LRC_FNAME_DISP_SECS)) {
        msc_cb.lrc_sta++;
        if (msc_cb.lrc_sta == LRC_FNAME_DISP_SECS) {
            if (lrc_first_lable_time() <= music_get_cur_time()) {
                msc_cb.lrc_update = music_get_lrc((char *)lrc_show, lrc_first_lable_time());
            } else {
                msc_cb.lrc_sta = LRC_FNAME_DISP_SECS - 1;
            }
        }
    }
}

AT(.text.lrc)
static void show_lrc_default(char *lrc, u32 mtime)
{
    printf("show_lrc_default: [%02d:%02d.%d] %s\n", mtime/600, (mtime%600)/10, mtime%10, lrc);
}

AT(.text.lrc)
void bsp_lrc_get_content(void (*show_lrc_cb)(char*, u32))
{
    u32 mtime;
    static u32 ticks = 0;

    if(tick_check_expire(ticks, 1000)){
        ticks = tick_get();
        bsp_lrc_sta_process();
    }

    if (msc_cb.lrc_sta == LRC_FNAME_DISP_SECS) {
        mtime = music_get_cur_time() + 3;
        if (music_get_lrc((char *)lrc_show, mtime)) {
            msc_cb.lrc_update = 1;
            if (show_lrc_cb) {
                show_lrc_cb(lrc_show, mtime);
            } else {
                show_lrc_default(lrc_show, mtime);
            }
        }
    }
}

AT(.text.lrc)
bool bsp_lrc_is_ready(void)
{
    if (msc_cb.lrc_sta == LRC_FNAME_DISP_SECS) {
        return true;
    }
    return false;
}

#endif // MUSIC_LRC_EN
