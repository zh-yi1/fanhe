#ifndef __API_PHOTO_VIEW_H__
#define __API_PHOTO_VIEW_H__


typedef struct {
    u8 *lseek_buff;
    u32 lseek_buff_size;
    u8 *jpg_buff;
    u32 jpg_buff_size;
    u8 *disp_buff;
    u32 disp_buff_size;
    u16 disp_width;
    u16 disp_height;
    u8 *photo_fifo_buff;
    u32 photo_fifo_buff_size;
    u8 *photo_fifo_buff_ex;
    u32 photo_fifo_buff_ex_size;
    u32 res_addr;
    u32 res_size;
    u32 seek_ofs;
    u8  type;
}photo_view_t;

enum {
    PHOTO_VIEW_FATFS,
    PHOTO_VIEW_FLASH,
};

typedef enum {
	PHOTO_OK              = 0,
    PHOTO_FILE_ERR        = 1,
    PHOTO_PARSE_ERR       = 2,
    PHOTO_DECODE_ERR      = 3,
    PHOTO_ENCODE_ERR      = 4,
} PHOTORESULT;

/**
 * 解析图片显示
 */
PHOTORESULT api_photo_view(photo_view_t *photo_cb);

/**
 * 解析图片+重压缩
 */
PHOTORESULT api_photo_recode_write(u8 *src_jpeg, u32 src_jpeg_size,
                                           u16 dst_width, u16 dst_hight,
                                           u8 *jpeg_obuff, u32 jpeg_obuff_size,
                                           u8 *jpeg_scale_buff, u32 jpeg_scale_buff_size,
                                           u8 *jpeg_dst_buff, u32 jpeg_dst_buff_size,
                                           bool (*write_back)(u8 *buff, u32 len));

/*
 * 大头贴解析
 */
bool api_stick_fill(u32 addr, u32 len, u8 *cache, u8 *img_buff, u16 *stick_width, u16 *stick_hight, u8 div);


#endif
