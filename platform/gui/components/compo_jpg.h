#ifndef __COMPO_JPG_H__
#define __COMPO_JPG_H__

typedef struct {
    u32 res_addr;
    u32 res_size;
} jpg_list_t;

typedef struct compo_jpg_t_ {
    COMPO_STRUCT_COMMON;
    widget_image_t* img;
    u8 *obuf;
    rect_t location;
    float zoom_num;
    u32 res_addr;
    u32 res_size;

    const jpg_list_t* list;
    u32 list_size;

    FIL* fp;
    u8 style;
    s32 file_total;
    s32 file_num;
} compo_jpg_t;

enum {
    COMPO_JPG_TYPE_SD_FATFS,
    COMPO_JPG_TYPE_FLASH_FATFS,
    COMPO_JPG_TYPE_FLASH,
};
#define COMPO_JPG_SUPPORT_DISK_NUM      2

compo_jpg_t* compo_jpg_create(compo_form_t* frm, u32 jpg_res_addr, u32 jpg_res_size);
void compo_jpg_view_init_control(compo_jpg_t* jpg, const char* path[COMPO_JPG_SUPPORT_DISK_NUM], bool flags, bool exit_flags);
bool compo_jpg_view_control(compo_jpg_t* jpg, bool next);
void compo_jpg_view_scale_control(compo_jpg_t* jpg, bool dir);
void compo_jpg_view(compo_jpg_t* jpg, FIL* fp);
void compo_jpg_exit(compo_jpg_t* jpg);
void compo_jpg_process(compo_jpg_t* jpg);
void compo_jpg_set_zoom(compo_jpg_t* jpg, float zoom);
void compo_jpg_set_pos(compo_jpg_t* jpg, s16 x, s16 y);
void compo_jpg_set_size(compo_jpg_t* jpg, s16 width, s16 height);
rect_t compo_jpg_get_location(compo_jpg_t* jpg);
void compo_jpg_set_flash_res_list(compo_jpg_t* jpg, const jpg_list_t* list, u32 size);
void compo_jpg_set_view_style(compo_jpg_t* jpg, u8 style);
#endif // __COMPO_JPG_H__
