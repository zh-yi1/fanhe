#ifndef __BSP_IMAGE_SENSOR_H__
#define __BSP_IMAGE_SENSOR_H__


#define DVP_COL_SCALE_4         3
#define DVP_COL_SCALE_3         2
#define DVP_COL_SCALE_2         1
#define DVP_COL_SCALE_1         0

#define DVP_ROW_SCALE_4         3
#define DVP_ROW_SCALE_3         2
#define DVP_ROW_SCALE_2         1
#define DVP_ROW_SCALE_1         0

#define DVP_IN_YUYV             0
#define DVP_IN_YVYU             1
#define DVP_IN_UYVY             2
#define DVP_IN_VYUY             3

#define DVP_IN_RGRG             0
#define DVP_IN_GRGR             1
#define DVP_IN_BGBG             2
#define DVP_IN_GBGB             3

#define DVP_OUT_Y_ONLY          0
#define DVP_OUT_YUV422          1

// DVPCON
#define DVP_VSYNC_DONE_IE_SHF       19
#define DVP_O_HALF_IE_SHF           18
#define DVP_O_ALL_IE_SHF            17
#define DVP_SW_ERR_IE_SHF           16
#define DVP_DMA_ERR_IE_SHF          15
#define DVP_DMA_DONE_IE_SHF         14
#define DVP_CSI_ERR_IE              13
#define DVP_AFIFO_FULL_IE           12
#define DVP_OUTSRAM_SHF             11
#define DVP_TV2PC_EN_SHF            10
#define DVP_OUTFMT_SHF              9
#define DVP_YUV2RGB_FORMULA_SHF     8
#define DVP_SIZE_PROTECT_EN_SHF     7
#define DVP_VSYNC_POL_SHF           6
#define DVP_HSYNC_POL_SHF           5
#define DVP_YUV_SWAP_SHF            3
#define DVP_YUV_SWAP_MSK            0x3
#define DVP_DMA_EN_SHF              2
#define DVP_CROP_EN_SHF             1
#define DVP_EN_SHF                  0

// DVPSIZE
#define DVP_IMG_HEIGHT_SHF          16
#define DVP_IMG_HEIGHT_MSK          0x3FF
#define DVP_IMG_WIDTH_SHF           0
#define DVP_IMG_WIDTH_MSK           0x7FF

// DVPCROPH
#define DVP_CROW_H_SZ_SHF           30
#define DVP_CROP_H_END_SHF          16
#define DVP_CROP_H_END_MSK          0x7FF
#define DVP_CROP_H_OFF_SHF          0
#define DVP_CROP_H_OFF_MSK          0x7FF

// DVPCROPV
#define DVP_CROW_V_SZ_SHF           30
#define DVP_CROP_V_END_SHF          16
#define DVP_CROP_V_END_MSK          0x3FF
#define DVP_CROP_V_OFF_SHF          0
#define DVP_CROP_V_OFF_MSK          0x3FF

// DVPSTRIDE
#define DVP_HEIGHT_SEG_SHF          16
#define DVP_HEIGHT_SEG_MSK          0x3FF
#define DVP_STRIDE_SHF              0
#define DVP_STRIDE_MSK              0xFFF

// DVPPEND
#define DVP_VSYNC_DONE_SHF          7
#define DVP_O_ALL_PND_SHF           6
#define DVP_O_HALF_PND_SHF          5
#define DVP_SW_ERR_PND_SHF          4
#define DVP_DMA_ERR_SHF             3
#define DVP_DMA_DONE_SHF            2
#define DVP_SIZE_ERR_SHF            1
#define DVP_AFIFO_FULL_SHF          0
#define DVP_ERR_MSK                 (BIT(DVP_SW_ERR_PND_SHF) | BIT(DVP_DMA_ERR_SHF)| BIT(DVP_AFIFO_FULL_SHF))// | BIT(DVP_SIZE_ERR_SHF) )


typedef enum {
    IMG_IN_YUYV,
    IMG_IN_YVYU,
    IMG_IN_UYVY,
    IMG_IN_VYUY,
    IMG_OUT_YUV422,
    IMG_OUT_RGB565,
}img_format;

enum {
    IMG_TYPE_DVP,
    IMG_TYPE_SPICS,
};


typedef struct _img_sensor_drv_t {
    u32 speed_mhz;
    u32 image_height;
    u32 image_width;
    img_format in_format;
    img_format out_format;
    void (*imag_sensor_reg_init)(void);
    u32 (*imag_sensor_read_id)(void);
    bool vsync_is_high;  //0高有效，1低有效
    bool hsync_is_high;  //0高有效，1低有效
    u32 dev_id;          //有效ID值，为FF表示不判断ID
    u8 type;
}img_sensor_drv_t;

enum {
    GPU2CPU_DISP_READY_BIT = 0,
    GPU2CPU_VIDEO_JPEG_BIT,
    GPU2CPU_PHOTO_JPEG_BIT,
};

typedef struct {
    volatile u8 *dvp_buff;
    volatile u8 *disp_buff;
    volatile bool disp_is_ready;
    volatile u16 dvp_fifo_line;
    volatile u16 dvp_curr_line;
    volatile u16 image_width;
    volatile u16 image_hight;
    volatile u16 disp_width;
    volatile u16 disp_hight;
    volatile u16 disp_fifo_line;
    volatile u16 disp_curr_line;
    volatile u32 gpu2cpu_bitmap;
    volatile u8 *dvp_full_buff;
    volatile u16 dvp_full_width;
    volatile u16 dvp_full_height;
    volatile u8 *temp_buff;
    volatile u32 temp_buff_size;
    volatile u8 frame_buff_max;
    volatile u8 frame_rd;
    volatile u8 frame_wr;
    volatile u8 frame_buff_count;
    volatile u8 zoom_div;
    volatile u16 zoom_width;
    volatile u16 zoom_height;
    volatile u16 zoom_x0;
    volatile u16 zoom_y0;
    volatile u8 *zoom_buff;
    volatile bool is_rotate_90_cw;       //是否旋转90度
    volatile bool is_video_or_photo_mode;//拍照或录像模式，0录像，1拍照
    volatile bool is_frame_valid;
    volatile bool is_drv_ready;
    volatile bool is_recode_full_screen;
    volatile bool stick_is_ready;
    volatile u8 *stick_img;
    volatile u16 stick_x0;
    volatile u16 stick_y0;
    volatile u16 stick_width;
    volatile u16 stick_height;
    volatile u8 stick_div;
    volatile u8 drive_type;
}dvp_share_t;


extern dvp_share_t dvp_gpu_share;

#if USB_JPEG_CAMERA
typedef struct {
    u16 width;
    u16 height;
    u32 cnt;
    u32 total_cnt;
    u8 frame_ready;
    u8 *rx_buf;
    u8 *out_buf;
    u8 *video_buf;

}usb_uvc_t;

extern usb_uvc_t usb_uvc;
#endif
/**
 * drv wr
 */
int img_write_Reg8Data8(u8 id, u8 addr, u8 data);
int img_write_Reg8Data16(u8 id, u8 addr, u16 data);
int img_write_Reg16Data8(u8 id, u16 addr, u8 data);
int img_write_Reg16Data16(u8 id, u16 addr, u16 data);
int img_read_Reg8Data8(u8 id, u8 addr, u8 *data);
int img_read_Reg8Data16(u8 id, u8 addr, u16 *data);
int img_read_Reg16Data8(u8 id, u16 addr, u8 *data);
int img_read_Reg16Data16(u8 id, u16 addr, u16 *data);
bool image_sensor_drv_register(u8 idx);
void image_sensor_drv_unregister(void);
void image_sensor_drv_enter_pwdn(void);
void image_sensor_drv_change(void);
bool bsp_image_sensor_is_init(void);
void image_sensor_drv_auto_reg(void);
void ldo1_enable(u8 io_num);
void ldo1_disable(u8 io_num);
u8 image_sensor_get_drv_type(void);
/**
 *  img sensor drv
 */
extern img_sensor_drv_t *img_drv_t;
extern img_sensor_drv_t img_gc0308_drv;
extern img_sensor_drv_t img_spa0a39_drv;
extern img_sensor_drv_t img_bf30a2_drv;

void bsp_image_sensor_set_state(bool state);
/**
 * 设置获取Sensor的数据范围
 */
void bsp_dvp_set_crop(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h);

/**
 * 设置DVP DMA BUFF
 */
void bsp_dvp_set_dbuf(void *buf, uint32_t stride, uint32_t lines);

/**
 *  bsp_dvp_dma_start
 */
void bsp_dvp_dma_start(void);

/**
 *  bsp_dvp_dma_stop
 */
void bsp_dvp_dma_stop(void);

/**
 *  bsp_image_sensor_init
 */
void bsp_image_sensor_init(bool is_reset);

/**
 * bsp_image_sensor_uinit
 */
void bsp_image_sensor_uninit(void);

/**
 *sensor显示是否准备好
 */
bool dvp_disp_is_ready_and_clean(void);

/**
 *sensor显存获取
 */
u8* dvp_disp_get_buff(void);

/**
 *bsp_dvp_full_set
 */
void bsp_dvp_full_set(u16 width, u16 height);

/**
 *拍照或录像模式，0录像，1拍照
 */
void bsp_image_sensor_mode(bool video_or_photo);

/**
 *sensor 进入休眠
 */
void bsp_image_sensor_pwdn(void);

/**
 *sensor 退出休眠
 */
void bsp_image_sensor_wkup(void);

/**
 *切换前后摄像头
 */
void bsp_image_sensor_change(void);

/**
 *获取sensor width
 */
u16 bsp_image_sensor_width_get(void);

/**
 *获取sensor hight
 */
u16 bsp_image_sensor_hight_get(void);

/**
 *数码变焦拉进镜头
 */
void bsp_sensor_zoom_add(void);

/**
 *数码变焦拉远镜头
 */
void bsp_sensor_zoom_sub(void);

/**
 *获取当前倍数
 */
u8 bsp_sensor_zoom_get(void);

/**
 * 大头贴
 */
void bsp_image_sticker(u16 x0, u16 y0, u32 addr, u32 len, u8 div);

/**
 * 切换大头贴
 */
void bsp_image_sticker_add(void);
void bsp_image_sticker_reduce(void);

// usb部分
u16 usb_1902_frame_read_data(u8 *buf, u16 len);
u8 usb_1902_frame_ready(void);
u32 usb_1902_frame_cnt(void);
void usb_1902_frame_ready_set(u8 state);
void usb_1902_init(void);
void usb_1902_deinit(void);

#endif


