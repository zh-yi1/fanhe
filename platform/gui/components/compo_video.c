#include "include.h"

#if VIDEO_PLAY_EN

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/**
 * @brief 创建一个视频框组件
 * @param[in] frm : 窗体指针
 * @param[in] video_res_addr : 视频资源地址
 * @return 返回动画框指针
 **/
compo_video_t *compo_video_create(compo_form_t *frm, u32 video_res_addr)
{
    compo_video_t *video = compo_create(frm, COMPO_TYPE_VIDEO);
    widget_image_t *img = widget_image_create(frm->page_body, 0);
    video->img = img;
    video->res_addr = video_res_addr;
    video->obuf = NULL;
    video->fp = NULL;
    if (video_res_addr) {
        video->style = COMPO_VIDEO_TYPE_FLASH;
    } else {
        video->style = COMPO_VIDEO_TYPE_SD_FATFS;
    }
    return video;
}

void compo_video_set_flash_res_list(compo_video_t* video, const video_list_t* list, u32 size)
{
    if (video == NULL) {
//        printf("video handle err\n");
        return;
    }
    video->list = list;
    video->list_size = size;
}

void compo_video_set_play_style(compo_video_t* video, u8 style)
{
    if (video == NULL) {
//        printf("video handle err\n");
        return;
    }
    video->style = style;
}

bool compo_video_play_init_control(compo_video_t *video, const char* path[COMPO_VIDEO_SUPPORT_DISK_NUM], bool flags, bool not_exit_flags)
{
    if (video == NULL) {
//        printf("video handle err\n");
        return false;
    }

    bool can_exit = false;

    do {
        switch(video->style) {
        case COMPO_VIDEO_TYPE_SD_FATFS:
            if (flags == true) {
                bsp_sd_disk_mount(28);
                video->file_total = bsp_sd_disk_scan(path[COMPO_VIDEO_TYPE_SD_FATFS], "*.avi", NULL);
                if (video->file_total <= 0) {
                    TRACE("sd no avi file => to disk flash avi file\n");
                    compo_video_set_play_style(video, COMPO_VIDEO_TYPE_FLASH_FATFS);
                    bsp_sd_disk_unmount();
                    break;
                }
                video->file_num = 0;
                if (bsp_sd_disk_open_file_idx(video->file_num)) {
                    video->res_addr = 0;
    //                video->res_size = 0;
                    compo_video_play(video, &bsp_sd_disk_get_fatfs()->fp);
                }
            } else {
                bsp_sd_disk_unmount();
                TRACE("sd disk unmount => to disk flash avi file\n");
                compo_video_set_play_style(video, COMPO_VIDEO_TYPE_FLASH_FATFS);
                if (not_exit_flags) {
                    flags = true;
                }
                break;
            }
            can_exit = true;
            break;

        case COMPO_VIDEO_TYPE_FLASH_FATFS:
            if (flags == true) {
                bsp_flash_disk_mount();
                video->file_total = bsp_flash_disk_scan(path[COMPO_VIDEO_TYPE_FLASH_FATFS], "*.avi", NULL);
                if (video->file_total <= 0) {
                    TRACE("flash disk no avi file => to flash avi file\n");
                    compo_video_set_play_style(video, COMPO_VIDEO_TYPE_FLASH);
                    bsp_flash_disk_unmount();
                    break;
                }
                video->file_num = 0;
                if (bsp_flash_disk_open_file_idx(video->file_num)) {
                    video->res_addr = 0;
                    compo_video_play(video, &bsp_flash_disk_get_fatfs()->fp);
                }
            } else {
                bsp_flash_disk_unmount();
                TRACE("flash disk unmount => to flash video file\n");
                compo_video_set_play_style(video, COMPO_VIDEO_TYPE_FLASH);
                if (not_exit_flags) {
                    flags = true;
                }
                break;
            }
            can_exit = true;
            break;

        case COMPO_VIDEO_TYPE_FLASH:
            TRACE("play flash video file!!\n");
            video->file_num = 0;
            if (video->list) {
                video->file_total = video->list_size;
                video->res_addr = video->list[video->file_num].res_addr;
            } else if (video->res_addr) {
                video->file_total = 1;
            } else {
                video->file_total = 0;
            }
            if (video->file_total) {
                if (not_exit_flags) {
                    compo_video_play(video, NULL);
                }
            } else {
                return false;
            }
            can_exit = true;
            break;
        default:
            TRACE("video style: %d\n", video->style);
            can_exit = true;
            break;
        }
    } while (can_exit == false);

    return true;
}

bool compo_video_play_control(compo_video_t *video, u8 next)
{
    if (video == NULL) {
//        printf("video handle err\n");
        return false;
    }
    if (video->force_exit) {
        return false;
    }

    if (!sys_cb.sd_enter_flag && video->style != COMPO_VIDEO_TYPE_FLASH) {
        return 0;
    }

    do {
        if (next == 1) {
            video->file_num++;
            if (video->file_num > video->file_total - 1) {
                video->file_num = 0;
            }
        } else if (next == 0) {
            video->file_num--;
            if (video->file_num < 0) {
                video->file_num = video->file_total - 1;
            }
        } else {

        }

        if (video->style != COMPO_VIDEO_TYPE_FLASH) {
            if (video->style == COMPO_VIDEO_TYPE_SD_FATFS) {
                bsp_sd_disk_close_file();
                compo_video_exit(video);
                TRACE("total:%d, video->file_num:%d\n",video->file_total, video->file_num);
                if (bsp_sd_disk_open_file_idx(video->file_num)) {
                    compo_video_play(video, &bsp_sd_disk_get_fatfs()->fp);
                } else {
                    return false;
                }
            }
        } else {
            if(api_video_play_sta_get() >= AVI_STA_END) {
                compo_video_exit(video);
                TRACE("total:%d, video->file_num:%d\n",video->file_total, video->file_num);
                if (video->list) {
                    video->res_addr = video->list[video->file_num].res_addr;
        //            video->res_size = video->list[video->file_num].res_size;
                }
                compo_video_play(video, NULL);
            } else if (!video->file_total && !video->file_num) {        //重复播放只需设置时间即可
                api_video_play_set_times(0);
            }
        }
    } while(video->obuf == NULL);

    return true;
}

/**
 * @brief 开始播放
 * @param[in] video : 视频指针
 **/
void compo_video_play(compo_video_t *video, FIL* fp)
{
    if (video == NULL) {
//        printf("video handle err\n");
        return;
    }
    if (video->sta == true) {
        return;
    }
    video->sta = true;

//    widget_set_visible(video->img, true);
    video->fp = fp;
    bsp_video_play_init(video->res_addr);
    video->obuf = bsp_video_play(video->fp);
}

/**
 * @brief 退出组件
 * @param[in] video : 视频指针
 **/
void compo_video_exit(compo_video_t *video)
{
    if (video == NULL) {
//        printf("video handle err\n");
        return;
    }
    if (video->sta == false) {
        return;
    }
    video->sta = false;
//    bsp_video_stop();
    bsp_video_play_uninit();
//    widget_set_visible(video->img, true);
//    widget_image_set(video->img, 0);
    video->obuf = NULL;
}

void compo_video_exit_lock(compo_video_t *video)
{
    if (video == NULL) {
//        printf("video handle err\n");
        return;
    }
    if (video->force_exit == false) {
        printf("%s\n", __func__);
        video->force_exit = true;
        compo_video_exit(video);
    }
}

void compo_video_exit_unlock(compo_video_t *video)
{
    if (video == NULL) {
//        printf("video handle err\n");
        return;
    }
    if (video->force_exit == true) {
        printf("%s\n", __func__);
        video->force_exit = false;
//        compo_video_play_control(video, 2);
    }
}

/**
 * @brief 设置视频
          用于改变视频源
 * @param[in] video : 视频指针
 * @param[in] res_addr : 视频资源地址
 **/
//void compo_video_set(compo_video_t *video, u32 video_res_addr)
//{
//    bsp_video_set(video_res_addr);
//}

/**
 * @brief 视频进程
          用于视频播放
 * @param[in] video : 视频指针
 **/
void compo_video_process(compo_video_t *video)
{
    if (video == NULL) {
//        printf("video handle err\n");
        return;
    }
    if (video->force_exit) {
        return;
    }

    if((api_video_play_frame_read_value() == true) && (video->is_flush == 0) && (video->obuf != NULL)){
        video->is_flush = true;
        video->ticks = tick_get();
        widget_image_set_ram(video->img, 0);
        widget_image_set_ram(video->img, video->obuf);
        widget_set_size(video->img, video->location.wid, video->location.hei);
        widget_set_pos(video->img, video->location.x, video->location.y);
        if (!widget_get_visble(video->img)) {
            widget_set_visible(video->img, true);
            if (video->cover->img) {
                widget_set_visible(video->cover->img, false);
            }
        }
//        jpg->obuf = NULL;
    }

    if (api_video_play_frame_read_value() == false) {
        video->is_flush = false;
    }


	//按照30fps
    if(video->is_flush){
        if(tick_check_expire(video->ticks, 33)){
            video->is_flush = false;
        }
    }
}

/**
 * @brief 设置视频组件的坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] video : 视频指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_video_set_pos(compo_video_t *video, s16 x, s16 y)
{
    if (video == NULL) {
//        printf("video handle err\n");
        return;
    }
//    widget_set_pos(video->img, x, y);
    video->location.x = x;
    video->location.y = y;
}

/**
 * @brief 设置视频组件的大小
 * @param[in] video : 视频指针
 * @param[in] width : 视频宽度
 * @param[in] height : 视频高度
 **/
void compo_video_set_size(compo_video_t *video, s16 width, s16 height)
{
    if (video == NULL) {
//        printf("video handle err\n");
        return;
    }
    video->location.wid = width;
    video->location.hei = height;
}

/**
 * @brief 获取视频的坐标和大小
 * @param[in] video : 动画指针
 * @return 返回视频的坐标和大小
 **/
rect_t compo_video_get_location(compo_video_t *video)
{
    rect_t rect = {0};
    if (video == NULL) {
//        printf("video handle err\n");
        return rect;
    }
//    rect_t rect = widget_get_location(video->img);
    rect = video->location;
    return rect;
}
#endif

