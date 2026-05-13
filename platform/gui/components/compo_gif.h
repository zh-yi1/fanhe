#ifndef _COMPO_GIF_H
#define _COMPO_GIF_H



//typedef struct {
//    u32 res_addr;
//    u32 res_size;
//} gif_list_t;

typedef struct compo_gif_t_ {
    COMPO_STRUCT_COMMON;
    widget_image_t *img;
    u32 ticks;              //计时
    u32 res_addr;
    u32 res_size;
    u8 *ibuf;
    u8 *obuf;
//    compo_picturebox_t *cover;		//封面
} compo_gif_t;

/**
 * @brief 创建一个视频框组件
 * @param[in] frm : 窗体指针
 * @param[in] gif_res_addr : 视频资源地址
 * @return 返回动画框指针
 **/
compo_gif_t *compo_gif_create(compo_form_t *frm, u32 gif_res_addr, u32 gif_size);

/**
 * @brief 创建一个视频框组件
 * @param[in] frm : 窗体指针
 * @param[in] gif_res_addr : 视频资源地址
 * @return 返回动画框指针
 **/
void compo_gif_set_buf(compo_gif_t *gif, u8 *obuf, u8*ibuf);
/**
 * @brief 开始播放
 * @param[in] gif : 视频指针
 **/
void compo_gif_play(compo_gif_t *gif);

/**
 * @brief 退出组件
 * @param[in] gif : 视频指针
 **/
void compo_gif_exit(compo_gif_t *gif);

/**
 * @brief 视频进程
          用于视频播放
 * @param[in] gif : 视频指针
 **/
void compo_gif_process(compo_gif_t *gif);

/**
 * @brief 设置视频组件的坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] gif : 视频指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_gif_set_pos(compo_gif_t *gif, s16 x, s16 y);
#endif
