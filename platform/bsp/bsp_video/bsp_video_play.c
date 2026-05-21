#include "include.h"


#if VIDEO_PLAY_EN

#if CHIP_PACKAGE_SUPPORT_PSRAM
#   if ((GUI_SCREEN_WIDTH < IMAGE_SENSOR_WIDTH) || (GUI_SCREEN_HEIGHT < IMAGE_SENSOR_HEIGHT))
#       define VIDEO_PLAY_DISP_WID_MAX     IMAGE_SENSOR_WIDTH
#       define VIDEO_PLAY_DISP_HEI_MAX     IMAGE_SENSOR_HEIGHT
#   else
#       define VIDEO_PLAY_DISP_WID_MAX     GUI_SCREEN_WIDTH
#       define VIDEO_PLAY_DISP_HEI_MAX     GUI_SCREEN_HEIGHT
#   endif
#else
#   define VIDEO_PLAY_DISP_WID_MAX     240
#   define VIDEO_PLAY_DISP_HEI_MAX     200
#endif // CHIP_PACKAGE_SUPPORT_PSRAM
static FIL* video_play_fp;

#define AUDIO_BUFF_MAX          0x2000
#define LSEEK_MAP_BUFF_MAX      1024
#define AVI_FRAME_LEN           0x10000
#define AVI_JPEG_FIFO_LEN       0x1f000

static u8 avi_pcm_buff[AUDIO_BUFF_MAX] AT(.video_play.buf);
static u8 avi_lseek_buff[LSEEK_MAP_BUFF_MAX] AT(.video_play.buf);
static video_play_t video_play_cb AT(.video_play.buf);

#if PSRAM_SIZE
static u8 avi_frame_psram_buff[AVI_FRAME_LEN] AT(.psram_video_play.buf);
#else
static u8 avi_frame_buff[AVI_FRAME_LEN] AT(.video_play_buf.buf);
#endif

#if PSRAM_SIZE
static u8 video_play_disp_buff[VIDEO_PLAY_DISP_WID_MAX * VIDEO_PLAY_DISP_HEI_MAX * 2 + 8] AT(.psram_video_play_disp.buf);
#endif // PSRAM_SIZE

static u8 avi_jpeg_fifo_buff[AVI_JPEG_FIFO_LEN] AT(.avi_play_fifo.buff);    //AT(.psram_video_play_disp.buf);//��spram�������һЩ, ��Ҫpsram_switch_cacheת��һ��
static bool avi_is_init;


/*
 *������ü��ܣ��ײ�ص����������ȡKEY
 */
u32 video_play_key_get(void)
{
    return 7852;
}

/*
 * ���º����������
 */
u8 avi_file_read(u8 *buf, u32 btr)
{
    if (video_play_cb.type == VIDEO_PLAY_FATFS) {
        UINT br;
        return fs_read (video_play_fp, buf, btr, &br);
    } else if (video_play_cb.type == VIDEO_PLAY_FLASH) {
        if ((u32)buf >= 0x20000000) {
            u8 *rbuf = ab_malloc(btr);
            os_spiflash_read(rbuf, video_play_cb.res_addr + video_play_cb.seek_ofs, btr);
            memcpy(buf, rbuf, btr);
            ab_free(rbuf);
        } else {
            os_spiflash_read(buf, video_play_cb.res_addr + video_play_cb.seek_ofs, btr);
        }

        video_play_cb.seek_ofs += btr;
        return 0;
    }
    return 0;
}
u8 avi_file_write(u8* buf, u32 btw)
{
    if (video_play_cb.type == VIDEO_PLAY_FATFS) {
        UINT bw;
        return fs_write (video_play_fp, buf, btw, &bw);
    } else if (video_play_cb.type == VIDEO_PLAY_FLASH) {
        return 0;
    }
    return 0;
}

u8 avi_file_lseek(u32 ofs)
{
    if (video_play_cb.type == VIDEO_PLAY_FATFS) {
        return fs_lseek (video_play_fp, ofs);
    } else if (video_play_cb.type == VIDEO_PLAY_FLASH) {
        video_play_cb.seek_ofs = ofs;
        return 0;
    }
    return 0;

}
u32 avi_file_tell(void)
{
    if (video_play_cb.type == VIDEO_PLAY_FATFS) {
        return fs_tell(video_play_fp);
    } else if (video_play_cb.type == VIDEO_PLAY_FLASH) {
        return video_play_cb.seek_ofs;
    }
    return 0;
}

u8 avi_file_close(void)
{
    if (video_play_cb.type == VIDEO_PLAY_FATFS) {
        return fs_close(video_play_fp);
    } else if (video_play_cb.type == VIDEO_PLAY_FLASH) {
        video_play_cb.seek_ofs = 0;
        return 0;
    }
    return 0;
}

u8 avi_file_creat_fastmap(void* buff, u32 len)
{
    if (video_play_cb.type == VIDEO_PLAY_FATFS) {
        return fs_creat_fastmap(video_play_fp, buff, len);
    } else if (video_play_cb.type == VIDEO_PLAY_FLASH) {
        return 0;
    }
    return 0;
}


void *avi_malloc(u32 size)
{
    return ab_malloc(size);
}

void avi_free(void *ptr)
{

    //printf("free:%x %x\n", ptr, __builtin_return_address(0));
    ab_free(ptr);
}

void *avi_realloc(void *ptr, u32 new_size)
{
    return ab_realloc(ptr, new_size);
}


/**
 * video play init
 */
void bsp_video_play_init(u32 res_addr)
{

    if (avi_is_init) {
        return;
    }

    u16 wid = VIDEO_PLAY_DISP_WID_MAX;
    u16 hei = VIDEO_PLAY_DISP_HEI_MAX;

    memset(&video_play_cb, 0, sizeof(video_play_t));
    memset(avi_pcm_buff, 0, sizeof(avi_pcm_buff));
    memset(avi_lseek_buff, 0, sizeof(avi_lseek_buff));
    video_play_cb.avi_pcm_buff      = avi_pcm_buff;
    video_play_cb.avi_pcm_buff_size = AUDIO_BUFF_MAX;
    video_play_cb.lseek_buff        = avi_lseek_buff;
    video_play_cb.lseek_buff_size   = LSEEK_MAP_BUFF_MAX;
#if PSRAM_SIZE
    memset(avi_frame_psram_buff, 0, sizeof(avi_frame_psram_buff));
    video_play_cb.avi_frame_buff     = (u8*)psram_switch_cache(avi_frame_psram_buff);
    video_play_cb.avi_frame_buff_size= sizeof(avi_frame_psram_buff);
#else
    memset(avi_frame_buff, 0, sizeof(avi_frame_buff));
    video_play_cb.avi_frame_buff     = avi_frame_buff;
    video_play_cb.avi_frame_buff_size= AVI_FRAME_LEN;
#endif

    if (DVP_DISP_WIDTH && DVP_DISP_HIGHT) {
#if PSRAM_SIZE == 0
        void customer_heap_init(void);
        customer_heap_init();
        video_play_cb.disp_buff = ab_malloc(wid*hei*2 + 8);
#else
        memset(psram_switch_cache(video_play_disp_buff), 0, sizeof(video_play_disp_buff));
        video_play_cb.disp_buff = (u8*)psram_switch_cache(video_play_disp_buff);
#endif // PSRAM_SIZE

        printf("video play disp<%d>(%d*%d):%dKB <%x>\n", PSRAM_SIZE, wid, hei, (wid*hei*2 + 8)/1024, video_play_cb.disp_buff);

        if (video_play_cb.disp_buff == NULL) {
            printf("bsp_video_play_init disp_buff ERR\n");
            halt(HALT_MALLOC);
        }
        video_play_cb.disp_buff_size     = wid*hei*2 + 8;
        video_play_cb.disp_width         = wid;
        video_play_cb.disp_height        = hei;
        if (res_addr != 0) {
            video_play_cb.type = VIDEO_PLAY_FLASH;
            video_play_cb.res_addr = res_addr;
        } else {
            video_play_cb.type = VIDEO_PLAY_FATFS;
        }
    }

    video_play_cb.avi_jpeg_fifo_buff = avi_jpeg_fifo_buff;
    video_play_cb.avi_jpeg_fifo_buff_size = AVI_JPEG_FIFO_LEN;
    bsp_jpgdec_init(1);
    sys_clk_req(INDEX_VIDEO, VIDEO_CLK_SEL);
    avi_is_init = true;

    //�����������ڲ���
    if (bsp_bt_disp_status() <= BT_STA_CONNECTED) {
        bsp_sys_mute();
    }
}


/**
 * video play uninit
 */
void bsp_video_play_uninit(void)
{
    if (avi_is_init == false) {
        return;
    }

    bsp_video_stop();
    bsp_jpgdec_exit();

    if(video_play_cb.disp_buff != NULL){
        printf("disp_buff:%x\n", video_play_cb.disp_buff);
    	os_gui_draw_w4_done();
//        ab_free(disp_buff);
    }

#if PSRAM_SIZE == 0
    if(video_play_cb.disp_buff != NULL){
//        ab_free(video_play_cb.disp_buff);
        void customer_heap_init(void);
        customer_heap_init();
    }
#endif // PSRAM_SIZE

    video_play_cb.avi_pcm_buff   = NULL;
    video_play_cb.lseek_buff     = NULL;
    video_play_cb.avi_frame_buff = NULL;
    video_play_cb.disp_buff      = NULL;

    sys_clk_free(INDEX_VIDEO);
    avi_is_init = false;
    printf("bsp_video_play_uninit\n");
}

//bool bsp_video_is_init(void)
//{
//    return avi_is_init;
//}

void avi_play_start_callback(bool audio_exit)
{
    if (audio_exit && !sbc_is_bypass() /*&& bsp_bt_disp_status() > BT_STA_CONNECTED*/) {
        printf("-->bypass\n");
        bt_audio_bypass();
        bsp_sys_mute();
    }
}

/**
 * bsp video play
 */
void *bsp_video_play(FIL* fp)
{
    if (sys_cb.mp3_res_playing) {
        music_control(MUSIC_MSG_STOP);
        while(mp3_res_process()) {          //wait mp3 stop
            WDT_CLR();
            vusb4s_reset_clr_cnt();
        }
    }

    video_play_fp = fp;
    u8 *show_buff = api_video_play_start(fp, &video_play_cb);

    if(show_buff != NULL){
        u32 ticks = tick_get();
        do {
            if (tick_check_expire(ticks, 500)) {
                printf("bsp_video_play<0x%x>:%d*%d\n", GET_LE32(&show_buff[0]), GET_LE16(&show_buff[4]), GET_LE16(&show_buff[6]));
                printf("show err\n");
                return NULL;
            }
            func_process();
        } while (GET_LE32(&show_buff[0]) != 0x24150 || GET_LE16(&show_buff[4]) == 0 || GET_LE16(&show_buff[6]) == 0);
//        printf("bsp_video_play<0x%x>:%d*%d\n", GET_LE32(&show_buff[0]), GET_LE16(&show_buff[4]), GET_LE16(&show_buff[6]));
        if (avi_audio_exsit_get()) {
            bsp_sys_unmute();
        }
    }

    if((u32)show_buff >= 0x30000000){
        dcache_writeback_region ((void*)show_buff, 8);
    }


    return (void*)show_buff;
}


/**
 * bsp video play stop
 */
void bsp_video_stop(void)
{

    if (video_play_cb.type == VIDEO_PLAY_FLASH) {
        avi_file_close();
    }

    api_video_play_stop();


//    bt_audio_enable();
}


/*
 *avi���Ź����У�������Դ������Ƶ,�����Ƶ����
 */
void bsp_video_mp3_res_play(u32 addr, u32 len)
{
    AVISTA  sta;

    if (len == 0 || sys_cb.mp3_res_playing) {
        return;
    }
    sta = api_video_play_sta_get();
    if(sta == AVI_STA_PLAYING){
        os_gui_draw_w4_done();
        api_video_play_set_pp(1);
        delay_5ms(20);
    }

	mp3_res_play(addr, len);

    if(sta == AVI_STA_PLAYING){
        while(mp3_res_process()) {
            WDT_CLR();
            vusb4s_reset_clr_cnt();
        }
        api_video_play_set_pp(0);
    }
}

#endif // VIDEO_PLAY_EN

