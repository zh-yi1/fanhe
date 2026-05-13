#include "include.h"

#if PHOTO_VIEW_EN

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

compo_jpg_t* compo_jpg_create(compo_form_t* frm, u32 jpg_res_addr, u32 jpg_res_size)
{
    compo_jpg_t* jpg = compo_create(frm, COMPO_TYPE_JPG);
    widget_image_t *img = widget_image_create(frm->page_body, 0);
    jpg->img = img;
    jpg->res_addr = jpg_res_addr;
    jpg->res_size = jpg_res_size;
    jpg->obuf = NULL;
    jpg->fp = NULL;
    jpg->zoom_num = 1.0;
    if (jpg_res_addr) {
        jpg->style = COMPO_JPG_TYPE_FLASH;
    } else {
        jpg->style = COMPO_JPG_TYPE_SD_FATFS;
    }
    return jpg;
}

void compo_jpg_set_flash_res_list(compo_jpg_t* jpg, const jpg_list_t* list, u32 size)
{
//    jpg->style = COMPO_JPG_TYPE_FLASH;
    jpg->list = list;
    jpg->list_size = size;
//    jpg->file_num = 0;
//    jpg->file_total = jpg->list_size;
}

void compo_jpg_set_view_style(compo_jpg_t* jpg, u8 style)
{
    jpg->style = style;
}


void compo_jpg_view_init_control(compo_jpg_t* jpg, const char* path[COMPO_JPG_SUPPORT_DISK_NUM], bool flags, bool exit_flags)
{
    bool can_exit = false;
    jpg->zoom_num = 1.0;

    do {
        switch (jpg->style) {
        case COMPO_JPG_TYPE_SD_FATFS:
            if (flags == true) {
                bsp_sd_disk_mount(28);
                jpg->file_total = bsp_sd_disk_scan(path[COMPO_JPG_TYPE_SD_FATFS], "*.jpg", NULL);
                if (jpg->file_total <= 0) {
                    printf("sd not jpg file => to disk flash jpg file\n");
                    compo_jpg_set_view_style(jpg, COMPO_JPG_TYPE_FLASH_FATFS);
                    break;
                }
                jpg->file_num = 0;
                if (bsp_sd_disk_open_file_idx(jpg->file_num)) {
                    jpg->res_addr = 0;
                    jpg->res_size = 0;
                    compo_jpg_view(jpg, &bsp_sd_disk_get_fatfs()->fp);
                }
            } else {
                bsp_sd_disk_unmount();
                printf("sd disk unmount => to disk flash jpg file\n");
                compo_jpg_set_view_style(jpg, COMPO_JPG_TYPE_FLASH_FATFS);
                if (exit_flags) {
                    flags = true;
                }
                break;
            }
            can_exit = true;
            break;

        case COMPO_JPG_TYPE_FLASH_FATFS:            //FLASH DISK
            if (flags == true) {
                bsp_flash_disk_mount();
                jpg->file_total = bsp_flash_disk_scan(path[COMPO_JPG_TYPE_FLASH_FATFS], "*.jpg", NULL);
                if (jpg->file_total <= 0) {
                    printf("flash disk not jpg file => to flash jpg file\n");
                    compo_jpg_set_view_style(jpg, COMPO_JPG_TYPE_FLASH);
                    break;
                }
                jpg->file_num = 0;
                if (bsp_flash_disk_open_file_idx(jpg->file_num)) {
                    jpg->res_addr = 0;
                    jpg->res_size = 0;
                    compo_jpg_view(jpg, &bsp_flash_disk_get_fatfs()->fp);
                }
            } else {
                bsp_flash_disk_unmount();
                printf("flash disk unmount => to flash jpg file\n");
                compo_jpg_set_view_style(jpg, COMPO_JPG_TYPE_FLASH);
                if (exit_flags) {
                    flags = true;
                }
                break;
            }
            can_exit = true;
            break;


        case COMPO_JPG_TYPE_FLASH:
            printf("view flash jpg file !!\n");
            jpg->file_num = 0;
            if (jpg->list) {
                jpg->file_total = jpg->list_size;
                jpg->res_addr = jpg->list[jpg->file_num].res_addr;
                jpg->res_size = jpg->list[jpg->file_num].res_size;
            } else if (jpg->res_addr) {
                jpg->file_total = 1;
            } else {
                jpg->file_total = 0;
            }
            if (jpg->file_total) {
                if (exit_flags) {
                    compo_jpg_view(jpg, NULL);
                }
            }
            can_exit = true;
            break;
        }
    } while (can_exit == false);
}

void compo_jpg_view_scale_control(compo_jpg_t* jpg, bool dir)
{
    if (dir) {
        jpg->zoom_num += 0.5;
    } else {
        jpg->zoom_num -= 0.5;
    }

    if (jpg->zoom_num <= 1.0) {
        jpg->zoom_num = 1.0;
    }
    if (jpg->zoom_num >= 2.5) {
        jpg->zoom_num = 2.5;
    }
    jpg->obuf = bsp_photo_get_disp_buff();
}

bool compo_jpg_view_control(compo_jpg_t* jpg, bool next)
{
    jpg->zoom_num = 1.0;


    do {
        if (next) {
            jpg->file_num++;
            if (jpg->file_num > jpg->file_total - 1) {
                jpg->file_num = 0;
            }
        } else {
            jpg->file_num--;
            if (jpg->file_num < 0) {
                jpg->file_num = jpg->file_total - 1;
            }
        }

        if (jpg->style != COMPO_JPG_TYPE_FLASH) {

            if (jpg->style == COMPO_JPG_TYPE_SD_FATFS) {
                bsp_sd_disk_close_file();
                compo_jpg_exit(jpg);
                if (bsp_sd_disk_open_file_idx(jpg->file_num)) {
                    compo_jpg_view(jpg, &bsp_sd_disk_get_fatfs()->fp);
                } else {
                    return false;
                }
            } else if (jpg->style == COMPO_JPG_TYPE_FLASH_FATFS) {
                bsp_flash_disk_close_file();
                compo_jpg_exit(jpg);
                if (bsp_flash_disk_open_file_idx(jpg->file_num)) {
                    compo_jpg_view(jpg, &bsp_flash_disk_get_fatfs()->fp);
                } else {
                    return false;
                }
            }
        } else {
            //只有一个文件, 返回不做切换
            if (jpg->file_total == 1) {
                return true;
            }

            compo_jpg_exit(jpg);
            if (jpg->list) {
                jpg->res_addr = jpg->list[jpg->file_num].res_addr;
                jpg->res_size = jpg->list[jpg->file_num].res_size;
            }
            compo_jpg_view(jpg, NULL);
        }
    } while (jpg->obuf == NULL);

    return true;
}

void compo_jpg_view(compo_jpg_t* jpg, FIL* fp)
{
    widget_set_visible(jpg->img, true);
    jpg->fp = fp;
    bsp_photo_view_init(jpg->res_addr, jpg->res_size);
    jpg->obuf = bsp_photo_view_start(jpg->fp);
}

void compo_jpg_exit(compo_jpg_t* jpg)
{
    bsp_photo_view_uninit();
    widget_set_visible(jpg->img, true);
    jpg->obuf = NULL;
}

void compo_jpg_process(compo_jpg_t* jpg)
{
    if (widget_get_visble(jpg->img) && jpg->obuf != NULL) {
        widget_image_set_ram(jpg->img, 0);
        widget_image_set_ram(jpg->img, jpg->obuf);
        widget_set_pos(jpg->img, jpg->location.x, jpg->location.y);
        widget_set_size(jpg->img, jpg->location.wid * jpg->zoom_num, jpg->location.hei * jpg->zoom_num);
        jpg->obuf = NULL;
    }
}

void compo_jpg_set_zoom(compo_jpg_t* jpg, float zoom)
{
    if (zoom <= 1.0) {
        zoom = 1.0;
    }
    if (zoom >= 2.5) {
        zoom = 2.5;
    }
    jpg->zoom_num = zoom;
}

void compo_jpg_set_pos(compo_jpg_t* jpg, s16 x, s16 y)
{
    jpg->location.x = x;
    jpg->location.y = y;
}

void compo_jpg_set_size(compo_jpg_t* jpg, s16 width, s16 height)
{
    jpg->location.wid = width;
    jpg->location.hei = height;
}

rect_t compo_jpg_get_location(compo_jpg_t* jpg)
{
    rect_t rect = jpg->location;
    return rect;
}

#endif // PHOTO_VIEW_EN
