#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

void ro_widget_set_pos(widget_t *widget, s16 x, s16 y, s16 page_wid, s16 page_hei)
{
    s16 x_pos = RO_DISP_GET_X(x, y, page_hei);
    s16 y_pos = RO_DISP_GET_Y(x, y, page_wid);
    widget_set_pos(widget, x_pos, y_pos);

}

void ro_widget_set_location(widget_t *widget, s16 x, s16 y, s16 width, s16 height, s16 page_wid, s16 page_hei)
{
    s16 x_pos = RO_DISP_GET_X(x, y, page_hei);
    s16 y_pos = RO_DISP_GET_Y(x, y, page_wid);
    s16 wid = COMPO_GET_WID(width, height);
    s16 hei = COMPO_GET_HEI(width, height);

//    printf("xy(%d, %d, %d, %d)->(%d, %d);  wid,hei[%d, %d]->[%d, %d]\n",
//           x, y, page_wid, page_hei, x_pos, y_pos, width, height, wid, hei);
    widget_set_location(widget, x_pos, y_pos, wid, hei);
}

void rotate_widget_set_size(widget_t *widget, s16 width, s16 height)
{
    s16 wid = COMPO_GET_WID(width, height);
    s16 hei = COMPO_GET_HEI(width, height);

    widget_set_size(widget, wid, hei);
}

void rotate_widget_page_set_client(widget_page_t *widget, s16 x, s16 y)
{
#if BOX_GUI_ROTATE_DISP
    widget_page_set_client(widget, -y, x);
#else
    widget_page_set_client(widget, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

void rotate_widget_page_scale_to(widget_page_t *widget, s16 wid, s16 hei)
{
    s16 width = COMPO_GET_WID(wid, hei);
    s16 height = COMPO_GET_HEI(wid, hei);

    widget_page_scale_to(widget, width, height);
}

area_t rotate_gui_image_get_size(u32 res_addr)
{
    area_t area = gui_image_get_size(res_addr);
    s16 wid = COMPO_GET_WID(area.wid, area.hei);
    s16 hei = COMPO_GET_HEI(area.wid, area.hei);
    area.wid = wid;
    area.hei = hei;
    return area;
}

area_t rotate_widget_image_get_size(widget_image_t *img)
{
    area_t area = widget_image_get_size(img);
    s16 wid = COMPO_GET_WID(area.wid, area.hei);
    s16 hei = COMPO_GET_HEI(area.wid, area.hei);
    area.wid = wid;
    area.hei = hei;
    return area;
}

rect_t rotate_widget_get_location(const widget_t *widget)
{
    rect_t rect = widget_get_location(widget);
#if BOX_GUI_ROTATE_DISP
    rect_t rect_tmp;
    rect_tmp.x = rect.y;
    rect_tmp.y = GUI_GET_SCREEN_HEIGHT - rect.x;
    rect_tmp.wid = rect.hei;
    rect_tmp.hei = rect.wid;
    rect = rect_tmp;
#endif
    return rect;
}

rect_t rotate_widget_get_absolute(const widget_t *widget)
{
    rect_t rect = widget_get_absolute(widget);
#if BOX_GUI_ROTATE_DISP
    rect_t rect_tmp;
    rect_tmp.x = rect.y;
    rect_tmp.y = GUI_GET_SCREEN_HEIGHT - rect.x;
    rect_tmp.wid = rect.hei;
    rect_tmp.hei = rect.wid;
    rect = rect_tmp;
#endif
    return rect;
}

void rotate_widget_image_set_rotation_center(void *img_ptr, s16 x, s16 y)
{
    s16 x_pos = x;
    s16 y_pos = y;
#if BOX_GUI_ROTATE_DISP
    x_pos = y;
    y_pos = x;
#endif
    widget_image_set_rotation_center(img_ptr, x_pos, y_pos);
}

void rotate_widget_image_cut(widget_image_t *img, s16 x, s16 y, s16 wid, s16 hei, u8 total_cnt)
{
    s16 width = COMPO_GET_WID(wid, hei);
    s16 height = COMPO_GET_HEI(wid, hei);
    s16 x_pos = x;
    s16 y_pos = y;
#if BOX_GUI_ROTATE_DISP
    x_pos = 110;//((total_cnt - 1) * width) - y;
    y_pos = 0;//x;
#endif
    printf("---------> cut: %d,%d,%d,%d\n", x_pos, y_pos, width, height);
    widget_image_cut(img, x_pos, y_pos, width, height);
}
