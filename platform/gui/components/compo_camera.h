#ifndef __COMPO_CAMERA_H__
#define __COMPO_CAMERA_H__

enum {
    MASK_SD_MOUNT_STA       = BIT(0),   //设备是否插入了SD卡存储器件, 插入拍照或录像会全部存储到SD卡
    MASK_BLE_SEND_JPG       = BIT(1),   //设备通过ble发送通过摄像头拍照处理生成的jpg图片
    MASK_BLE_SEND_FRAME     = BIT(2),   //设备通过ble发送通过摄像头录像处理生成的原始每帧的图片数据
    MASK_CAMERA_INIT_STA    = BIT(3),   //摄像头设备是否在线
    MASK_FLASH_MOUNT_STA    = BIT(4),   //设备使用FLASH DISK储存, 由于FLASH DISK较小, 只储存图片
};

enum {
    COMPO_CAMERA_EXIT,
    COMPO_CAMERA_INIT,
    COMPO_CAMERA_SD_CARD_INTER,
    COMPO_CAMERA_SD_CARD_OUT,
};

typedef struct {
    u32 res_addr;
    u32 res_size;
    u16 x;
    u16 y;
} sticker_pic_t;

typedef struct {
    COMPO_STRUCT_COMMON;

    u8 dev_sta;     //设备状态
    u8 mode;        //模式: 0,录像； 1,拍照
    const sticker_pic_t* sticker_pic;   //大头贴特效

    widget_image_t* img;        //显示
    u8* obuf;                   //原始图像颜色数据
    rect_t location;            //位置

    u32 mask_ticks;
    bool take_photo_flags;

} compo_camera_t;

compo_camera_t* compo_camera_create(compo_form_t* frm);
void compo_camera_init_control(compo_camera_t* camera, u8 mode, u8 flags);
void compo_camera_work_do(compo_camera_t* camera);
void compo_camera_zoom_do(compo_camera_t* camera, bool dir);
void compo_camera_set_take_photo_ble_send(compo_camera_t* camera, void* ble_send_func);
void compo_camera_set_video_record_ble_send(compo_camera_t* camera, void* ble_send_func);
void compo_camera_view_frame_process(compo_camera_t* camera);
void compo_camera_set_pos(compo_camera_t* camera, s16 x, s16 y);
void compo_camera_set_size(compo_camera_t* camera, s16 width, s16 height);
rect_t compo_camera_get_location(compo_camera_t* camera);
bool compo_camera_is_working(compo_camera_t* camera);
#endif // __COMPO_CAMERA_H__
