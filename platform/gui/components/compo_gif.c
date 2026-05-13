#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/**
 * @brief 创建一个视频框组件
 * @param[in] frm : 窗体指针
 * @param[in] gif_res_addr : 视频资源地址
 * @return 返回动画框指针
 **/
compo_gif_t *compo_gif_create(compo_form_t *frm, u32 gif_res_addr, u32 gif_size)
{
    compo_gif_t *gif = compo_create(frm, COMPO_TYPE_GIF);
    widget_image_t *img = widget_image_create(frm->page_body, 0);
    gif->img = img;
    gif->res_addr = gif_res_addr;
    gif->res_size = gif_size;

    return gif;
}

/**
 * @brief 创建一个视频框组件
 * @param[in] frm : 窗体指针
 * @param[in] gif_res_addr : 视频资源地址
 * @return 返回动画框指针
 **/
void compo_gif_set_buf(compo_gif_t *gif, u8 *obuf, u8*ibuf)
{
    gif->ibuf = ibuf;
    gif->obuf = obuf;
    if ((u32)ibuf >= 0x20000000) {
        u8 *tbuf = ab_zalloc(4096);
        u32 res_addr = gif->res_addr;
        u8 *ibuf_ptr = gif->ibuf;
        for(u16 i=0;i<gif->res_size/4096;i++) {
            os_spiflash_read(tbuf, res_addr, 4096);
            memcpy(ibuf_ptr, tbuf, 4096);
            res_addr += 4096;
            ibuf_ptr += 4096;
            memset(tbuf, 0, 4096);
        }
        os_spiflash_read(tbuf, res_addr, gif->res_size%4096);
        memcpy(ibuf_ptr, tbuf, gif->res_size%4096);
        ab_free(tbuf);

    } else {
        os_spiflash_read(gif->ibuf, gif->res_addr, gif->res_size);
    }
}

/**
 * @brief 开始播放
 * @param[in] gif : 视频指针
 **/
void compo_gif_play(compo_gif_t *gif)
{
    if (gif == NULL) {
//        printf("gif handle err\n");
        return;
    }


    bsp_gif_play(gif->obuf, gif->ibuf, gif->res_size);
}

/**
 * @brief 退出组件
 * @param[in] gif : 视频指针
 **/
void compo_gif_exit(compo_gif_t *gif)
{
    if (gif == NULL) {
//        printf("gif handle err\n");
        return;
    }

    bsp_gif_stop();

}

/**
 * @brief 设置视频
          用于改变视频源
 * @param[in] gif : 视频指针
 * @param[in] res_addr : 视频资源地址
 **/
//void compo_gif_set(compo_gif_t *gif, u32 gif_res_addr)
//{
//    bsp_gif_set(gif_res_addr);
//}

/**
 * @brief 视频进程
          用于视频播放
 * @param[in] gif : 视频指针
 **/
void compo_gif_process(compo_gif_t *gif)
{
    if (gif == NULL) {
//        printf("gif handle err\n");
        return;
    }

    bool flush = bsp_gif_play_process(gif->obuf, gif->ibuf, gif->res_size);
    if (flush) {
        widget_image_set_ram(gif->img, gif->obuf);
    }
}
//
/**
 * @brief 设置视频组件的坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] gif : 视频指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_gif_set_pos(compo_gif_t *gif, s16 x, s16 y)
{
    if (gif == NULL) {
//        printf("gif handle err\n");
        return;
    }
    widget_set_pos(gif->img, x, y);
//    gif->location.x = x;
//    gif->location.y = y;

}
//
///**
// * @brief 设置视频组件的大小
// * @param[in] gif : 视频指针
// * @param[in] width : 视频宽度
// * @param[in] height : 视频高度
// **/
//void compo_gif_set_size(compo_gif_t *gif, s16 width, s16 height)
//{
//    if (gif == NULL) {
////        printf("gif handle err\n");
//        return;
//    }
//    gif->location.wid = width;
//    gif->location.hei = height;
//}
//
///**
// * @brief 获取视频的坐标和大小
// * @param[in] gif : 动画指针
// * @return 返回视频的坐标和大小
// **/
//rect_t compo_gif_get_location(compo_gif_t *gif)
//{
//    rect_t rect = {0};
//    if (gif == NULL) {
////        printf("gif handle err\n");
//        return rect;
//    }
////    rect_t rect = widget_get_location(gif->img);
//    rect = gif->location;
//    return rect;
//}
//#endif

