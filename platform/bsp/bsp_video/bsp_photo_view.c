#include "include.h"

#if PHOTO_VIEW_EN

#if CHIP_PACKAGE_SUPPORT_PSRAM
#   if ((GUI_SCREEN_WIDTH < IMAGE_SENSOR_WIDTH) || (GUI_SCREEN_HEIGHT < IMAGE_SENSOR_HEIGHT))
#       define PHOTO_VIEW_DISP_WID_MAX     IMAGE_SENSOR_WIDTH
#       define PHOTO_VIEW_DISP_HEI_MAX     IMAGE_SENSOR_HEIGHT
#   else
#       define PHOTO_VIEW_DISP_WID_MAX     GUI_SCREEN_WIDTH
#       define PHOTO_VIEW_DISP_HEI_MAX     GUI_SCREEN_HEIGHT
#   endif
#else
#   define PHOTO_VIEW_DISP_WID_MAX     240
#   define PHOTO_VIEW_DISP_HEI_MAX     200
#endif // CHIP_PACKAGE_SUPPORT_PSRAM

#define PHOTO_FRAME_LEN           0x10000
#define PHOTO_JPEG_FIFO_LEN       0x1f000

static FIL* photo_view_fp;
static bool photo_view_is_running;
static photo_view_t photo_cb AT(.photo_view.buf);
static u8 photo_lseek_buff[1024] AT(.photo_view.buf);
#if PSRAM_SIZE == 0
static u8 photo_jpeg_buff[PHOTO_FRAME_LEN] AT(.photo_view_buf.buf);
#endif
static u8 photo_jpeg_fifo_buff[PHOTO_JPEG_FIFO_LEN] AT(.photo_view_fifo.buff);

#if PSRAM_SIZE
static u8 photo_jpg_psram_buff[1024*1024] AT(.psram_photo_view.buf);
static u8 photo_fifo_psram_buff_ex[512*1024] AT(.psram_photo_view.buf);
static u8 photo_disp_buff[PHOTO_VIEW_DISP_WID_MAX * PHOTO_VIEW_DISP_HEI_MAX * 2 + 8] AT(.psram_photo_view_disp.buf);
#endif

/**
 * photo view init
 */
void bsp_photo_view_init(u32 res_addr, u32 res_size)
{
    u16 wid = PHOTO_VIEW_DISP_WID_MAX;
    u16 hei = PHOTO_VIEW_DISP_HEI_MAX;

    memset(&photo_cb, 0, sizeof(photo_cb));
    photo_view_is_running = false;

    photo_cb.lseek_buff         = photo_lseek_buff;
    photo_cb.lseek_buff_size    = sizeof(photo_lseek_buff);
#if PSRAM_SIZE == 0
    photo_cb.jpg_buff           = photo_jpeg_buff;
    photo_cb.jpg_buff_size      = PHOTO_FRAME_LEN;
#else
    photo_cb.jpg_buff           = (u8*)psram_switch_cache(photo_jpg_psram_buff);
    photo_cb.jpg_buff_size      = sizeof(photo_jpg_psram_buff);
#endif
    if (DVP_DISP_WIDTH && DVP_DISP_HIGHT) {
#if PSRAM_SIZE
        photo_cb.disp_buff          = (u8*)psram_switch_cache(photo_disp_buff);
        photo_cb.disp_buff_size     = sizeof(photo_disp_buff);
#else
        void customer_heap_init(void);
        customer_heap_init();
        photo_cb.disp_buff          = ab_malloc(wid*hei*2 + 8);
        photo_cb.disp_buff_size     = wid*hei*2 + 8;
#endif // PSRAM_SIZE

        printf("photo view disp<%d>(%d*%d):%dKB <%x>\n", PSRAM_SIZE, wid, hei, (wid*hei*2 + 8)/1024, photo_cb.disp_buff);

        if (photo_cb.disp_buff == NULL) {
            printf("%s disp_buff ERR\n", __func__);
            halt(HALT_MALLOC);
        }

        photo_cb.disp_width         = wid;
        photo_cb.disp_height        = hei;

        if (res_addr != 0) {
            photo_cb.type = VIDEO_PLAY_FLASH;
            photo_cb.res_addr = res_addr;
            photo_cb.res_size = res_size;
        } else {
            photo_cb.type = VIDEO_PLAY_FATFS;
        }
    }

    photo_cb.photo_fifo_buff    = photo_jpeg_fifo_buff;
    photo_cb.photo_fifo_buff_size = PHOTO_JPEG_FIFO_LEN;
#if PSRAM_SIZE
    photo_cb.photo_fifo_buff_ex      = (u8*)psram_switch_cache(photo_fifo_psram_buff_ex);
    photo_cb.photo_fifo_buff_ex_size = sizeof(photo_fifo_psram_buff_ex);
#endif
    bsp_jpgdec_init(1);
}


/**
 * photo view uninit
 */
void bsp_photo_view_uninit(void)
{
    printf("bsp_photo_view_uninit\n");

#if PSRAM_SIZE == 0
    if(photo_cb.disp_buff != NULL){

        printf("bsp_photo_view_uninit %x\n", photo_cb.disp_buff);
//        ab_free(photo_cb.disp_buff);
        void customer_heap_init(void);
        customer_heap_init();
    }
#endif // PSRAM_SIZE

    photo_cb.lseek_buff     = NULL;
    photo_cb.jpg_buff       = NULL;
    photo_cb.disp_buff      = NULL;
    photo_cb.photo_fifo_buff_ex = NULL;
    printf("bsp_photo_view_uninit end\n");
}


/**
 * photo view
 */
void *bsp_photo_view_start(FIL* fp)
{
    PHOTORESULT ret;

    if(photo_view_is_running == true){
        return NULL;
    }

    photo_view_is_running = true;
    u32 disp_buff = (u32)photo_cb.disp_buff;
    photo_view_fp = fp;

    sys_clk_req(INDEX_VIDEO, PHOTO_CLK_SEL);
    ret = api_photo_view(&photo_cb);
    photo_view_is_running = false;
    sys_clk_free(INDEX_VIDEO);

    if(ret != PHOTO_OK){
        return NULL;
    }

    u32 ticks = tick_get();
    // 等待解码完成 将jpeg的格式转成可以显示的图片格式
    do {
        printf("bsp_photo_view<0x%x>:%d*%d\n", GET_LE32(&photo_cb.disp_buff[0]), GET_LE16(&photo_cb.disp_buff[4]), GET_LE16(&photo_cb.disp_buff[6]));

        if (tick_check_expire(ticks, 500)) {
            printf("show err\n");
            return NULL;
        }
    } while( (disp_buff == (u32)photo_cb.disp_buff ) && (GET_LE32(&photo_cb.disp_buff[0]) != 0x24150 || GET_LE16(&photo_cb.disp_buff[4]) == 0 || GET_LE16(&photo_cb.disp_buff[6]) == 0));

     if (disp_buff != (u32)photo_cb.disp_buff ) {
         return NULL;
     }
    if((u32)photo_cb.disp_buff >= 0x30000000){
        dcache_writeback_region ((void*)photo_cb.disp_buff, 8);
    }

    return photo_cb.disp_buff;
}

void *bsp_photo_get_disp_buff(void)
{
    if (gui_set_ram_check(photo_cb.disp_buff, __func__)) {
        return photo_cb.disp_buff;
    } else {
        return NULL;
    }
}



/*
 * 以下函数供库调用
 */


u8 photo_view_file_read(u8 *buf, u32 btr)
{
    if (photo_cb.type == PHOTO_VIEW_FATFS) {
        UINT br;
        return fs_read (photo_view_fp, buf, btr, &br);
    } else if (photo_cb.type == PHOTO_VIEW_FLASH) {
        u8* ptr = ab_malloc(btr);
        if (ptr) {
#if USB_JPEG_CAMERA
            if (usb_1902_frame_ready()== 1) {
                usb_1902_frame_read_data(ptr, btr);
            }  else{
                os_spiflash_read(ptr, photo_cb.res_addr + photo_cb.seek_ofs, btr);
            }
#else
			os_spiflash_read(ptr, photo_cb.res_addr + photo_cb.seek_ofs, btr);
#endif
            photo_cb.seek_ofs += btr;
            //memcpy(buf, ptr, btr);
            gpdma1_memcpy(buf, ptr, btr);
            ab_free(ptr);
        }
        return 0;
    }
    return 0;
}

u8 photo_view_file_lseek(u32 ofs)
{
    if (photo_cb.type == PHOTO_VIEW_FATFS) {
        return fs_lseek (photo_view_fp, ofs);
    } else if (photo_cb.type == PHOTO_VIEW_FLASH) {
        photo_cb.seek_ofs = ofs;
        return 0;
    }
    return 0;
}

u32 photo_view_file_tell(void)
{
    if (photo_cb.type == PHOTO_VIEW_FATFS) {
        return fs_tell(photo_view_fp);
    } else if (photo_cb.type == PHOTO_VIEW_FLASH) {
        return photo_cb.seek_ofs;
    }
    return 0;
}

u64 photo_view_file_size(void)
{
    if (photo_cb.type == PHOTO_VIEW_FATFS) {
        return fs_size (photo_view_fp);
    } else if (photo_cb.type == PHOTO_VIEW_FLASH) {
        return photo_cb.res_size;
    }
    return 0;
}

u8 photo_view_file_close(void)
{
    if (photo_cb.type == PHOTO_VIEW_FATFS) {
        return fs_close(photo_view_fp);
    } else if (photo_cb.type == PHOTO_VIEW_FLASH) {
        return 0;
    }
    return 0;
}

u8 photo_view_file_creat_fastmap(void* buff, u32 len)
{
    if (photo_cb.type == PHOTO_VIEW_FATFS) {
        return fs_creat_fastmap(photo_view_fp, buff, len);
    } else if (photo_cb.type == PHOTO_VIEW_FLASH) {
        return 0;
    }
    return 0;
}


#endif // PHOTO_VIEW_EN
