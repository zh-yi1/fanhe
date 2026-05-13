#include "include.h"

#if USB_JPEG_CAMERA

#define  MAX_JPEG_BUF     (22*1024)
typedef struct {
    volatile u8 quality;                 //JPEG ENC Quality 0~100;
    volatile u8 jpeg_format;             //0:H1V1 1:H2V1(support, line:16) 2:H1V2 3:H2V2(support, line:32)
    volatile u8 jpeg_yuv_grb;            //0:YUV422, 1:RGB565
    volatile u16 photo_width;
    volatile u16 photo_height;
    volatile u8 *jpeg_buff;
    volatile u32 jpeg_buff_size;
    volatile u8 *jpeg_out_buff;
    volatile u32 jpeg_out_size;
    volatile u8 *jpeg_scale_buff;
    volatile bool enc_is_start;
    volatile bool take_photo_is_start;
    volatile u32 take_photo_GPU2CPU_tick;
    volatile bool is_continue;
}photo_jpeg_share_t;



usb_uvc_t usb_uvc;

extern photo_jpeg_share_t dvp_photo_jpeg_share;
void photo_encode_put(void);

void gpdma1_memcpy(void* dst, void *src, u32 size);
//活动记录功能事件处理
void hw_task_cal_timer2(void);

u32 tick;
AT(.com_rodata)
const char str_hw_task_cal_timer2[] = "hw_task_cal_timer2 cnt %d  len: %d tick %d buf: %x \n";
AT(.com_rodata)
const char str_hw_task_cal_timer3[] = "hw_task_cal_timer3  %x %x %x %x %x %x %x %x\n";
AT(.com_rodata)
const char str_hw_task_cal_timer4 [] = "hw_task_cal_timer4 cnt %d  decode_start: %d hwjpeg_buf %x rx_buf: %x\n";

AT(.com_rodata)
const char str_hw_task_cal_timer5[] = "hw_task_cal_timer2  tick %d buf:%x \n";

AT(.com_rodata)
const char str_hw_task_cal_timer6[] = "hw_task_cal_timer video  tick %d  \n";

u8 temp[1024];
u8 mjpeg_buff[MAX_JPEG_BUF*2]AT(.psram_video_recode.buf);
u8 mjpeg_video_buff[MAX_JPEG_BUF*2]AT(.psram_video_recode.buf);

AT(.com_text.isr)
void hw_task_cal_timer2(void)
{

    static bool hwjpeg_buf = 0;

    u16 len = uhv_video_data(&temp[0]);

    if (usb_uvc.frame_ready == 1) {   // svn 1970 不会出现C241问题 svn 2249会
        return;
    }
    // 解决边录像边显示出现C241问题 但是会出现帧数少一半的问题，原来是72ms一帧，更改后143ms左右一帧
    if (usb_uvc.frame_ready && dvp_video_jpeg_share.video_recode_is_start) {
        return;
    }

    if(len) {
        // 数据间隔 71-72ms  数据大小8k到20K
        if (temp[0] == 0xff && temp[1] == 0xd8 && usb_uvc.cnt ==0) {
            hwjpeg_buf ^= 0x01;
            usb_uvc.rx_buf = mjpeg_buff + MAX_JPEG_BUF * hwjpeg_buf;
            gpdma1_memcpy(usb_uvc.rx_buf + usb_uvc.cnt, temp, len);
            usb_uvc.cnt += len;
            usb_uvc.frame_ready = 0;

        } else if (temp[0] == 0xff && temp[1] == 0xd8 && usb_uvc.cnt !=0) {
            usb_uvc.frame_ready = 1;
            usb_uvc.out_buf = usb_uvc.rx_buf;

            if(dvp_photo_jpeg_share.take_photo_is_start){
                    printf(str_hw_task_cal_timer5, tick_get(), usb_uvc.out_buf);
                    dvp_photo_jpeg_share.take_photo_is_start  = false;
                    usb_uvc.video_buf =  mjpeg_video_buff + MAX_JPEG_BUF * hwjpeg_buf;
                    gpdma1_memcpy(usb_uvc.video_buf, usb_uvc.out_buf, usb_uvc.cnt);
                    dvp_photo_jpeg_share.jpeg_out_buff        = usb_uvc.video_buf;
                    dvp_photo_jpeg_share.jpeg_out_size        = usb_uvc.cnt;
                    dvp_photo_jpeg_share.take_photo_GPU2CPU_tick = 30;

                    dvp_gpu_share.gpu2cpu_bitmap |= BIT(GPU2CPU_PHOTO_JPEG_BIT);
                     MCPUSW1 = BIT(31) | (3);   // CPU 中断

            }

            if(dvp_video_jpeg_share.video_recode_is_start && usb_uvc.out_buf[usb_uvc.cnt-2] == 0xff && usb_uvc.out_buf[usb_uvc.cnt-1] == 0xd9){
                    my_printf(str_hw_task_cal_timer6, tick_get());

                    usb_uvc.video_buf =  mjpeg_video_buff + MAX_JPEG_BUF * hwjpeg_buf;
                    gpdma1_memcpy(usb_uvc.video_buf, usb_uvc.out_buf, usb_uvc.cnt);
                    dvp_video_jpeg_share.jpeg_out_buff        = usb_uvc.video_buf;

                    dvp_video_jpeg_share.jpeg_out_size        = usb_uvc.cnt;
                    dvp_gpu_share.gpu2cpu_bitmap |= BIT(GPU2CPU_VIDEO_JPEG_BIT);

                    MCPUSW1 = BIT(31) | (3);   // CPU 中断


            }
            hwjpeg_buf ^= 0x01;
            usb_uvc.rx_buf = mjpeg_buff + MAX_JPEG_BUF * hwjpeg_buf;
            gpdma1_memcpy(usb_uvc.rx_buf, temp, len);
            usb_uvc.total_cnt = usb_uvc.cnt;
            usb_uvc.cnt = len;

        } else if(usb_uvc.cnt != 0) {
            gpdma1_memcpy(usb_uvc.rx_buf + usb_uvc.cnt, temp, len);
            usb_uvc.cnt += len;
        }
    }

}

u16 usb_1902_frame_read_data(u8 *buf, u16 len)
{
    printf("%s line:%d len:%d decode_start: %d out_buf: %x tick:%d \n", __func__, __LINE__,  len,  usb_uvc.frame_ready, usb_uvc.out_buf, tick_get()- tick);
    tick = tick_get();
    //memcpy(buf, out_buf, len);
    gpdma1_memcpy(buf, usb_uvc.out_buf, len);
    return usb_uvc.total_cnt;

}

u8 usb_1902_frame_ready(void)
{
    return usb_uvc.frame_ready;
}

u32 usb_1902_frame_cnt(void)
{
    return usb_uvc.total_cnt;
}

void usb_1902_frame_ready_set(u8 state)
{
    usb_uvc.frame_ready = state;
    if(dvp_video_jpeg_share.video_recode_is_start){
        usb_uvc.cnt = 0;
    } else {
        usb_uvc.cnt = 0;
    }

}


void usb_1902_init(void)
{
    if(uhs_is_usb_uvc()) {

        bsp_hw_timer_set(HW_TIMER2, 125, hw_task_cal_timer2);
    } else {
        udisk_insert();
        bsp_image_sensor_set_state(true);
        if (dev_is_online(DEV_UDISK)) {
            if (dev_udisk_activation_try(1)) {
                sys_cb.cur_dev = DEV_UDISK;
            }
        }
        if(uhs_is_usb_uvc()) {
            bsp_hw_timer_set(HW_TIMER2, 125, hw_task_cal_timer2);
        }

    }
    usb_uvc.width = 640;
    usb_uvc.height = 480;
}

void usb_1902_deinit(void)
{
    if(uhs_is_usb_uvc()) {

        usb_host_uhv_video_close();
        bsp_image_sensor_set_state(false);
        udisk_remove();
    }
    bsp_hw_timer_del(HW_TIMER2);
    memset(&usb_uvc, 0, sizeof(usb_uvc_t));
    printf("hei: %d wid:%d \n", usb_uvc.height, usb_uvc.width);
}

#endif
