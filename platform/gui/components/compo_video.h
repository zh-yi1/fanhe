#ifndef _COMPO_VEDIO_H
#define _COMPO_VEDIO_H

//#define COMPO_video_CLICK_NUM            6 //区域点击动画最大个数


typedef struct {
    u32 res_addr;
    u32 res_size;
} video_list_t;

typedef struct compo_video_t_ {
    COMPO_STRUCT_COMMON;
    widget_image_t *img;
    u8 *obuf;
    u32 ticks;              //计时
    u32 res_addr;
    bool is_flush;
    rect_t location;

    const video_list_t* list;
    u32 list_size;

    FIL* fp;
    u8 style;
    s32 file_total;
    s32 file_num;

    u8 sta;
    compo_picturebox_t *cover;		//封面
    bool force_exit;
} compo_video_t;

enum {
    COMPO_VIDEO_TYPE_SD_FATFS,
    COMPO_VIDEO_TYPE_FLASH_FATFS,
    COMPO_VIDEO_TYPE_FLASH,
};
#define COMPO_VIDEO_SUPPORT_DISK_NUM          2


bool compo_video_play_control(compo_video_t *video, u8 next);
bool compo_video_play_init_control(compo_video_t *video, const char* path[COMPO_VIDEO_SUPPORT_DISK_NUM], bool flags, bool not_exit_flags);

/**
 * @brief 创建一个视频框组件
 * @param[in] frm : 窗体指针
 * @param[in] video_res_addr : 视频资源地址
 * @return 返回动画框指针
 **/
compo_video_t *compo_video_create(compo_form_t *frm, u32 video_res_addr);

/**
 * @brief 开始播放
 * @param[in] video : 视频指针
 **/
void compo_video_play(compo_video_t *video, FIL* fp);

/**
 * @brief 退出组件
 * @param[in] video : 视频指针
 **/
void compo_video_exit(compo_video_t *video);

/**
 * @brief 设置视频
          用于改变视频源
 * @param[in] video : 视频指针
 * @param[in] res_addr : 视频资源地址
 **/
void compo_video_set(compo_video_t *video, u32 video_res_addr);
/**
 * @brief 视频进程
          用于视频播放
 * @param[in] video : 视频指针
 **/
void compo_video_process(compo_video_t *video);

/**
 * @brief 设置视频组件的坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] video : 视频指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_video_set_pos(compo_video_t *video, s16 x, s16 y);

/**
 * @brief 设置视频组件的大小
 * @param[in] video : 视频指针
 * @param[in] width : 视频宽度
 * @param[in] height : 视频高度
 **/
void compo_video_set_size(compo_video_t *video, s16 width, s16 height);

/**
 * @brief 获取视频的坐标和大小
 * @param[in] video : 动画指针
 * @return 返回视频的坐标和大小
 **/
rect_t compo_video_get_location(compo_video_t *video);
void compo_video_set_flash_res_list(compo_video_t* video, const video_list_t* list, u32 size);
void compo_video_set_play_style(compo_video_t* video, u8 style);
void compo_video_exit_unlock(compo_video_t *video);
void compo_video_exit_lock(compo_video_t *video);
#endif
