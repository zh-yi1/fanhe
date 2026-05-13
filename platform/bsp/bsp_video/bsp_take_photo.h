#ifndef __BSP_TAKE_PHOTO_H__
#define __BSP_TAKE_PHOTO_H__

typedef enum {
    PHOTO_RESOLUT_VGA = 0,
    PHOTO_RESOLUT_2M,
    PHOTO_RESOLUT_3M,
    PHOTO_RESOLUT_5M,
    PHOTO_RESOLUT_8M,
    PHOTO_RESOLUT_10M,
    PHOTO_RESOLUT_12M,
    PHOTO_RESOLUT_15M,
    PHOTO_RESOLUT_20M,
    PHOTO_RESOLUT_30M,
    PHOTO_RESOLUT_32M,
    PHOTO_RESOLUT_48M,
}PHOTO_RESOLUT_TYPE;

/**
 * take photo
 */
void bsp_video_take_photo(void);

/**
 * 设置拍照分辨率
 */
void bsp_take_photo_set_resolut(PHOTO_RESOLUT_TYPE resolut);

/**
 * 获取拍照分辨率
 */
PHOTO_RESOLUT_TYPE bsp_take_photo_resolut_get_type(void);

/**
 * 水印添加
 */
void bsp_photo_recode_watermark_print(bool is_frist);

/**
 * 退出拍照
 */
void bsp_take_photo_exit(void);

/**
 * 退出时候等待系统完成
 */
void bsp_take_photo_wait_complete(void);

/**
 * 拍照是否正在进行
 */
bool bsp_take_photo_is_busy(void);

#endif
