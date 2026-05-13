#ifndef _ROTATE_WIDGET_H
#define _ROTATE_WIDGET_H

//SCREEN
#define rotate_widget_set_pos(widget, x, y)                 ro_widget_set_pos(widget, x, y, GUI_GET_SCREEN_WIDTH, GUI_GET_SCREEN_HEIGHT)
#define rotate_widget_set_location(widget, x, y, w, h)      ro_widget_set_location(widget, x, y, w, h, GUI_GET_SCREEN_WIDTH, GUI_GET_SCREEN_HEIGHT)

void ro_widget_set_pos(widget_t *widget, s16 x, s16 y, s16 page_wid, s16 page_hei);
void ro_widget_set_location(widget_t *widget, s16 x, s16 y, s16 width, s16 height, s16 page_wid, s16 page_hei);

void rotate_widget_set_size(widget_t *widget, s16 width, s16 height);
void rotate_widget_page_set_client(widget_page_t *widget, s16 x, s16 y);
void rotate_widget_page_scale_to(widget_page_t *widget, s16 wid, s16 hei);

area_t rotate_gui_image_get_size(u32 res_addr);
area_t rotate_widget_image_get_size(widget_image_t *img);
rect_t rotate_widget_get_location(const widget_t *widget);
rect_t rotate_widget_get_absolute(const widget_t *widget);
void rotate_widget_image_set_rotation_center(void *img_ptr, s16 x, s16 y);
void rotate_widget_image_cut(widget_image_t *img, s16 x, s16 y, s16 wid, s16 hei, u8 total_cnt);

#endif  //_ROTATE_WIDGET_H
