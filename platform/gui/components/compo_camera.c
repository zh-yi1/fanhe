#include "include.h"

#if VIDEO_RECODE_TAKE_PHOTO_EN

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


compo_camera_t* compo_camera_create(compo_form_t* frm)
{
    compo_camera_t* camera = compo_create(frm, COMPO_TYPE_CAMERA);
    widget_image_t* img = widget_image_create(frm->page_body, 0);
    camera->img = img;
    camera->dev_sta = 0;
    camera->obuf = NULL;
    return camera;
}

void compo_camera_init_control(compo_camera_t* camera, u8 mode, u8 flags)
{
    if (flags == COMPO_CAMERA_INIT || flags == COMPO_CAMERA_SD_CARD_INTER) {
        bsp_sd_disk_mount(28);                          //尝试挂载sd卡
        if (bsp_sd_mount_state() == true) {
            camera->dev_sta |= MASK_SD_MOUNT_STA;       //sd卡在线
        } else {
            camera->dev_sta &= ~MASK_SD_MOUNT_STA;      //sd卡离线
        }


        if (flags == COMPO_CAMERA_INIT) {
            camera->mode = mode;                            //模式
            //  usb jpeg

#if !USB_JPEG_CAMERA
            bsp_image_sensor_init(true);
            bsp_image_sensor_mode(mode);
#endif // USB_JPEG_CAMERA
            if (bsp_image_sensor_is_init() == true) {
                camera->dev_sta |= MASK_CAMERA_INIT_STA;
            } else {
                camera->dev_sta &= ~MASK_CAMERA_INIT_STA;
            }
        }

        if (camera->mode == 0) {
            bsp_video_recode_set_resolut(sys_cb.videosize_id);       //获取录像分辨率
        } else if (camera->mode == 1) {
            if ((camera->dev_sta & MASK_SD_MOUNT_STA) == 0) {   //尝试挂载flash disk
                bsp_flash_disk_mount();
                if (bsp_flash_mount_state() == true) {
                    camera->dev_sta |= MASK_FLASH_MOUNT_STA;
                } else {
                    camera->dev_sta &= ~MASK_FLASH_MOUNT_STA;
                }
            } else {
                if (bsp_flash_mount_state() == true) {
                    printf("sd disk online => unmount flash disk!!\n");
                    bsp_flash_disk_unmount();
                    camera->dev_sta &= ~MASK_FLASH_MOUNT_STA;
                }
            }
            bsp_take_photo_set_resolut(sys_cb.photosize_id);        //获取图片的大小
        }


    } else if (flags == COMPO_CAMERA_EXIT || flags == COMPO_CAMERA_SD_CARD_OUT) {
        if (flags == COMPO_CAMERA_EXIT) {
            bsp_image_sensor_uninit();
            if (mode == 1) {
                bsp_take_photo_exit();
            } else if (mode == 0) {
                if(bsp_video_recode_is_start()){
                    bsp_video_recode_stop();
                }
            }
            camera->dev_sta &= ~MASK_CAMERA_INIT_STA;


            if (camera->dev_sta & MASK_FLASH_MOUNT_STA) {
                bsp_flash_disk_unmount();
            }
            camera->dev_sta &= ~MASK_FLASH_MOUNT_STA;      //FLASH DISK离线
        } else {
            bsp_flash_disk_mount();
            if (bsp_flash_mount_state() == true) {
                camera->dev_sta |= MASK_FLASH_MOUNT_STA;
            } else {
                camera->dev_sta &= ~MASK_FLASH_MOUNT_STA;
            }
        }

        if (camera->dev_sta & MASK_SD_MOUNT_STA) {
            bsp_sd_disk_unmount();
        }
        camera->dev_sta &= ~MASK_SD_MOUNT_STA;      //sd卡离线

    }
}

void compo_camera_work_do(compo_camera_t* camera)
{
    if ((camera->dev_sta & MASK_CAMERA_INIT_STA) == 0) {
        return;
    }

    switch (camera->mode) {
    case 0: //录像
        if (camera->dev_sta & MASK_SD_MOUNT_STA) {
            if (bsp_sd_disk_get_free() > 0) {
                if (bsp_video_recode_is_start()) {
                    bsp_video_recode_stop();
                    printf("###%s=>recode stop\n", __func__);
                } else {
                    bsp_video_recode_start();
                    printf("###%s=>recode start\n", __func__);
                }
            } else {
                printf("###%s=>recode sd not free space\n", __func__);
            }
        } else {
            printf("###%s=>recode sd not online\n", __func__);
        }
        break;

    case 1: //拍照
        if ((bsp_sd_disk_get_free() > 0) || (bsp_flash_disk_get_free() > 0) || ((camera->dev_sta & MASK_BLE_SEND_JPG) && ble_is_connect())) {
            os_gui_draw_w4_done();
            if (camera->take_photo_flags == false) {
                bsp_video_take_photo();
    //            delay_5ms(100); //用延时解决花屏
                camera->take_photo_flags = true;
                camera->mask_ticks = 0;
            }
        } else {
            printf("###%s=>take photo not save dev\n", __func__);
        }
        break;
    }
}

void compo_camera_zoom_do(compo_camera_t* camera, bool dir)
{
    if ((camera->dev_sta & MASK_CAMERA_INIT_STA) == 0) {
        return;
    }

    if (dir == false) {
        bsp_sensor_zoom_sub();
    } else {
        bsp_sensor_zoom_add();
    }
}

void compo_camera_set_take_photo_ble_send(compo_camera_t* camera, void* ble_send_func)
{
    if ((camera->dev_sta & MASK_CAMERA_INIT_STA) == 0) {
        return;
    }

    camera->dev_sta |= MASK_BLE_SEND_JPG;
    sys_cb.camera_take_photo_ble_send = ble_send_func;
}

void compo_camera_set_video_record_ble_send(compo_camera_t* camera, void* ble_send_func)
{
    if ((camera->dev_sta & MASK_CAMERA_INIT_STA) == 0) {
        return;
    }

    camera->dev_sta |= MASK_BLE_SEND_FRAME;
    sys_cb.camera_video_record_ble_send = ble_send_func;
}

bool compo_camera_is_working(compo_camera_t* camera)
{
    if ((camera->dev_sta & MASK_CAMERA_INIT_STA) == 0) {
        return false;
    }

    switch (camera->mode) {
    case 0: //录像
        return bsp_video_recode_is_start();

    case 1: //拍照
        return (camera->take_photo_flags || bsp_take_photo_is_busy());
    }

    return false;
}
extern u8 avi_encoding;
void compo_camera_view_frame_process(compo_camera_t* camera)
{
    if ((camera->dev_sta & MASK_CAMERA_INIT_STA) == 0) {
        return;
    }

    if (dvp_disp_is_ready_and_clean()) {
        if (camera->mode == 1) {
            if (bsp_take_photo_is_busy()) {
                return;
            }

            if (camera->take_photo_flags) {
                if (camera->mask_ticks++ > 1) {
                    camera->take_photo_flags = false;
                    return;
                }
                widget_image_set_ram(camera->img, 0);
                return;
            }

        }

        widget_image_set_ram(camera->img, 0);
        camera->obuf = dvp_disp_get_buff();
        widget_image_set_ram(camera->img, camera->obuf);   // 画面显示
        widget_set_pos(camera->img, camera->location.x, camera->location.y);
        widget_set_size(camera->img, camera->location.wid, camera->location.hei);


        if (camera->mode == 0) {                                //录像状态
            if ((camera->dev_sta & MASK_BLE_SEND_FRAME) && ble_is_connect()) {        //ble发送原始帧数据
                if (GET_LE32(camera->obuf) == 0x24150) {
                    u16 frame_wid = GET_LE16(&camera->obuf[4]);
                    u16 frame_hei = GET_LE16(&camera->obuf[6]);
                    if (sys_cb.camera_video_record_ble_send) {
                        sys_cb.camera_video_record_ble_send(camera->obuf, frame_wid * frame_hei * 2 + 8);
                    }
                }
            }
        }


    }
#if USB_JPEG_CAMERA
    if (usb_1902_frame_ready() ) {

        // 完成jpg数据转成 img控件可以使用的数据 然后供 compo_jpg_process 使用

        bsp_photo_view_uninit();

         bsp_photo_view_init(0x02, usb_1902_frame_cnt()); // 223ab0
         avi_encoding = 1;
         camera->obuf = bsp_photo_view_start(NULL);
         avi_encoding = 0;
         if(camera->obuf ==NULL) {
             usb_1902_frame_ready_set(0);
             printf("%s line:%d \n", __func__, __LINE__);
             return;
         }
         //compo_jpg_process(jpg);
        widget_image_set_ram(camera->img, 0);

        widget_image_set_ram(camera->img, camera->obuf);   // 画面显示

        widget_set_pos(camera->img, camera->location.x, camera->location.y);

        widget_set_size(camera->img, camera->location.wid, camera->location.hei);

        //compo_jpg_view(jpg, NULL);
        // 清除拍照Flag 方便下次拍照
        if (camera->take_photo_flags) {
            if (camera->mask_ticks++ > 1) {
                camera->take_photo_flags = false;
                printf("%s line:%d \n", __func__, __LINE__);
            }
        }
        usb_1902_frame_ready_set(0);
    }
#endif
}


void compo_camera_set_pos(compo_camera_t* camera, s16 x, s16 y)
{
    camera->location.x = x;
    camera->location.y = y;
}

void compo_camera_set_size(compo_camera_t* camera, s16 width, s16 height)
{
    camera->location.wid = width;
    camera->location.hei = height;
}

rect_t compo_camera_get_location(compo_camera_t* camera)
{
    rect_t rect = camera->location;
    return rect;
}


#endif // VIDEO_RECODE_TAKE_PHOTO_EN

