#ifndef __API_GPDMA_H__
#define __API_GPDMA_H__


typedef enum
{
    GPDMA_FORMAT_NORMAL = 0,
    GPDMA_FORMAT_YUV422 = 1,
    GPDMA_FORMAT_YY     = 2,
    GPDMA_FORMAT_RGB565 = 3,
    GPDMA_FORMAT_RGB888 = 4,
    GPDMA_FORMAT_RGBA888= 5,
}GPDMA_FORMAT_TYPE;


typedef enum
{
    GPU_SR_RGB              = 0,
    GPU_SR_ARGB_TO_RGB      = 1,
    GPU_SR_ARGB_TO_ARGB     = 2,
}scaler_mode_type;


/**
 * 把cache回写到psram
 */
void dcache_writeback_region (void *addr, unsigned len);

/**
 * 读psram前调用
 */
void dcache_invalidate_region (void *addr, unsigned len);

/**
 * 通过DMA拷贝内存
 * dst:  目标地址
 * src： 源地址
 * size：拷贝大小
 */
void gpdma1_memcpy(void* dst, void *src, u32 size);


/**
 * 通过DMA拷贝图片(2维)
 * dst:  目标地址
 * src： 源地址
 * wight: 源图片宽度(多少像素)
 * hight: 源图片高度
 * src_stride ：一行有多少像素
   dst_stride : 一行有多少像素
   pixel_bytes: 一个像素多少bytes

   下图B图片拷贝到A BUF(2D Buff) 说明如下：
        (dst_stride)
----------------------------           (wight)
|                           |        (src_stride)
|                           |        ------------
|                           |       |            |
|                           |  <==  |            |(hight)
|                           |       |            |
|                           |       |            |
|                           |        ------------
----------------------------

            A                            B

                        (dst_stride)
                 ----------------------------
                 |      (src_stride)         |
                 |      ------------         |
                 |     |            |        |
                 |     |            |(hight) |
                 |     |            |        |
                 |     |            |        |
                 |      ------------         |
                 ----------------------------

                            A + B
 */
void gpdma1_2d_memcpy(void* dst, void *src, u32 wight, u32 hight, int32_t src_stride, int32_t dst_stride, u32 pixel_bytes);


/**
 * 通过DMA拷贝图片(2维)
 * dst:  目标地址
 * src： 源地址
 * wight: 源图片宽度(多少像素)
 * hight: 源图片高度
 * src_stride ：一行有多少像素
   dst_stride : 一行有多少像素
   format_dst : 图片格式，RGB565，YUY422 .....
   format_src : 图片格式，RGB565，YUY422 .....
   alpha      : RGB,YUV->RGBA888的参数，不需要就填写0xFF

   下图B图片拷贝到A BUF(2D Buff) 说明如下：
        (dst_stride)
----------------------------           (wight)
|                           |        (src_stride)
|                           |        ------------
|                           |       |            |
|                           |  <==  |            |(hight)
|                           |       |            |
|                           |       |            |
|                           |        ------------
----------------------------

            A                            B

                        (dst_stride)
                 ----------------------------
                 |      (src_stride)         |
                 |      ------------         |
                 |     |            |        |
                 |     |            |(hight) |
                 |     |            |        |
                 |     |            |        |
                 |      ------------         |
                 ----------------------------

                            A + B
 */
void gpdma1_image2fromat(void* dst, void *src, u16 wight, u16 hight, int32_t src_stride, int32_t dst_stride,
                            GPDMA_FORMAT_TYPE format_dst, GPDMA_FORMAT_TYPE format_src, u8 alpha);

/**
 * 硬件缩放
 */
bool hwde_scale(u16 *dst, u16 *src, scaler_mode_type mode,
                      u16 dst_w, u16 dst_h,
                      u16 src_w, u16 src_h);


#endif
