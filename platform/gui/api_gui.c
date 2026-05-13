#include "include.h"

#if 0//!GUI_SELECT
void de_memset_dword(u32 *dst, const u32 value, uint cnt){}
void os_gui_init(const gui_init_param_t *param){}
void os_gui_draw(void){}
void os_gui_flush_lcd(void){}
void gui_process(void){}
void gui_set_te_margin(u8 margin){}
void gui_widget_refresh(void){}
void os_gui_draw_w4_done(void){}
area_t gui_image_get_size(u32 res_addr){area_t area = {0}; return area;}
area_t widget_image_get_size(widget_image_t *img){area_t area = {0}; return area;}
u16 make_color(u8 r, u8 g, u8 b){return 0;}
widget_page_t *widget_pool_create(bool flag_top){return NULL;}
void widget_pool_clear(widget_page_t *widget){}
void widget_pool_set_top(u8 top_num){}
void *widget_get_head(void){return NULL;}
widget_t *widget_get_next(const widget_t *widget){return NULL;}
widget_page_t *widget_get_parent(const widget_t *widget){return NULL;}
void widget_set_alpha(widget_t *widget, u8 alpha){}
u8 widget_get_alpha(widget_t *widget){return 0;}
void widget_set_align_center(widget_t *widget, bool align_center){}
bool widget_get_align_center(widget_t *widget){return 0;}
void widget_set_top(widget_t *widget, bool flag_top){}
void widget_set_pos(widget_t *widget, s16 x, s16 y){}
void widget_set_size(widget_t *widget, s16 width, s16 height){}
void widget_set_location(widget_t *widget, s16 x, s16 y, s16 width, s16 height){}
rect_t widget_get_location(const widget_t *widget){rect_t rect = {0}; return rect;}
rect_t widget_get_absolute(const widget_t *widget){rect_t rect = {0}; return rect;}
void widget_set_visible(widget_t *widget, bool visible){}
bool widget_get_visble(widget_t *widget){return 0;}
widget_page_t *widget_page_create(widget_page_t *parent){return NULL;}
bool widget_is_page(const void *widget){return 0;}
void widget_page_set_client(widget_page_t *widget, s16 x, s16 y){}
void widget_page_scale_to(widget_page_t *widget, s16 wid, s16 hei){}
void widget_page_update(void){}
widget_image_t *widget_image_create(widget_page_t *parent, u32 res_addr){return NULL;}
void widget_image_set(widget_image_t *img, u32 res_addr){}
void widget_image_set_ram(widget_image_t *icon, void *buf){}
void widget_image_set_rotation(void *img_ptr, s16 angle){}
void widget_image_set_rotation_center(void *img_ptr, s16 x, s16 y){}
void widget_image_cut(widget_image_t *img, s16 x, s16 y, s16 wid, s16 hei){}
void widget_image_set_color(widget_image_t *img, u16 color){}
widget_image3d_t *widget_image3d_create(widget_page_t *parent, u32 res_addr){return NULL;}
void widget_image3d_set_axis(widget_image3d_t *img, widget_axis3d_t *axis){}
void widget_image3d_set(widget_image3d_t *img, u32 res_addr){}
void widget_image3d_set_ram(widget_image3d_t *img, void *buf){}
void widget_image3d_set_r(widget_image3d_t *img, s16 r){}
void widget_image3d_set_polar(widget_image3d_t *img, s16 angle){}
void widget_image3d_set_azimuth(widget_image3d_t *img, s16 angle){}
void widget_image3d_set_rotation(widget_image3d_t *img, s16 angle){}
void widget_image3d_set_rotation_center(widget_image3d_t *img, s16 x, s16 y){}
bool widget_image3d_is_front(widget_image3d_t *img){return 0;}
bool widget_image3d_contains(widget_image3d_t *img, s16 x, s16 y){return 0;}
widget_axis3d_t *widget_axis3d_create(widget_page_t *parent){return NULL;}
void widget_axis3d_set_distance(widget_axis3d_t *axis, s16 distance){}
void widget_axis3d_set_overlook(widget_axis3d_t *axis, s16 angle){}
void widget_axis3d_set_pos(widget_axis3d_t *axis, s16 x, s16 y, s16 z){}
void widget_axis3d_set_sph(widget_axis3d_t *axis, sph_t sph){}
void widget_axis3d_set_polar(widget_axis3d_t *axis, s16 angle){}
void widget_axis3d_set_azimuth(widget_axis3d_t *axis, s16 angle){}
void widget_axis3d_set_rotation(widget_axis3d_t *axis, s16 angle){}
sph_t widget_axis3d_roll(widget_axis3d_t *axis, s16 roll_polar, s16 roll_azimuth){sph_t sp = {0}; return sp;}
sph_t widget_axis3d_roll_from(widget_axis3d_t *axis, sph_t sph, s16 roll_polar, s16 roll_azimuth){sph_t sp = {0}; return sp;}
widget_icon_t *widget_icon_create(widget_page_t *parent, u32 res_addr){return NULL;}
bool widget_is_icon(widget_icon_t *icon){return 0;}
void widget_icon_set(widget_icon_t *icon, u32 res_addr){}
void widget_icon_set_ram(widget_icon_t *icon, void *buf){}
widget_text_t *widget_text_create(widget_page_t *parent, u16 max_word_cnt){return NULL;}
void widget_text_set_font(widget_text_t *txt, u32 font_res_addr){}
void widget_text_clear(widget_text_t *txt){}
void widget_text_set(widget_text_t *txt, const char *text){}
void widget_text_set_with_type(widget_text_t *txt, u16 *ubuf, u8 *tbuf){}
void widget_text_set_color(widget_text_t *txt, u16 color){}
void widget_text_set_hspace(widget_text_t *txt, s8 hspace){}
void widget_text_set_wholewrap(widget_text_t *txt, bool wholewrap){}
void widget_text_set_equal_hspace(widget_text_t *txt, bool equal_hspace){}
void widget_text_set_autosize(widget_text_t *txt, bool autosize){}
void widget_text_set_wordwrap(widget_text_t *txt, bool wordwrap){}
void widget_text_set_direction(widget_text_t *txt, bool direction){}
bool widget_text_get_direction(widget_text_t *txt){return 0;}
void widget_text_set_ellipsis(widget_text_t *txt, bool ellipsis){}
bool widget_text_get_ellipsis(widget_text_t *txt){return 0;}
void widget_text_set_client(widget_text_t *txt, s16 rel_x, s16 rel_y){}
void widget_text_get_client(widget_text_t *txt, s16 *rel_x, s16 *rel_y){}
area_t widget_text_get_area(widget_text_t *txt){area_t area = {0}; return area;}
area_t widget_text_get_box_area_rel(widget_text_t *txt){area_t area = {0}; return area;}
u8 widget_text_get_line_cnt(widget_text_t *txt){return 0;}
u16 widget_text_get_autoroll_circ_pixel(widget_text_t *txt){return 0;}
u8 widget_text_get_height(void){return 0;}
void widget_text_set_right_align(widget_text_t *txt, bool right_align){}
void widget_text_set_autoroll_mode(widget_text_t *txt, u8 autoroll_mode){}
void widget_text_set_autoroll_circ_space(widget_text_t *txt, u16 space_pixel){}
bool widget_text_get_right_align(widget_text_t *txt){return 0;}
int widget_text_get_layout(widget_text_t *txt){return 0;}
widget_rect_t *widget_rect_create(widget_page_t *parent){return NULL;}
void widget_rect_set_color(widget_rect_t *rect, u16 color){}
void widget_rect_set_radius(widget_rect_t *rect, u16 r){}
widget_qrcode_t *widget_qrcode_create(widget_page_t *parent, u8 qr_type, u16 max_ch_cnt){return NULL;}
void widget_qrcode_set(widget_qrcode_t *qrcode, const char *str){}
void widget_qrcode_2d_set(widget_qrcode_t *qrcode, const char *code, int len){}
void widget_qrcode_set_bitwid(widget_qrcode_t *qrcode, u8 bit_wid){}
void widget_qrcode_set_bitwid_by_qrwid(widget_qrcode_t *qrcode, u16 qr_wid){}
void widget_qrcode_set_maxwid(widget_qrcode_t *qrcode, u16 max_wid){}
int widget_qrcode_get_bit_cnt(widget_qrcode_t *qrcode){return 0;}
void widget_qrcode_set_level(widget_qrcode_t *qrcode, u8 level){}
void widget_barcode_set_type(widget_qrcode_t *qrcode, u8 type){}
widget_arc_t *widget_arc_create(widget_page_t *parent){return NULL;}
void widget_arc_set_angles(widget_arc_t *arc, u16 start_angle, u16 end_angle){}
void widget_arc_set_color(widget_arc_t *arc, u16 color_intra, u16 color_outre){}
void widget_arc_set_alpha(widget_arc_t *arc, u8 alpha_intra, u8 alpha_outre){}
void widget_arc_set_width(widget_arc_t *arc, u16 arc_width){}
void widget_arc_set_edge_circle(widget_arc_t *arc, bool start_onoff, bool end_onoff){}
widget_circle_t *widget_circle_create(widget_page_t *parent){return NULL;}
void widget_circle_set_acolor(widget_circle_t *circle, u16 color_intra, u8 alpha_intra){}
widget_chart_t *widget_chart_create(widget_page_t *parent, WGT_CHART_TYPE type, u8 max_num){return NULL;}
void widget_chart_set_range(widget_chart_t *chart, u16 range_x, u16 range_y){}
bool widget_chart_set_real_num(widget_chart_t *chart, u8 real_num){return 0;}
bool widget_chart_set_value(widget_chart_t *chart, u8 id, u16 x_start, u16 x_end, u16 y_start, u16 y_end, u16 color){return 0;}
widget_line_t *widget_line_create(widget_page_t *parent, u16 item_max){return NULL;}
bool widget_line_set_item(widget_line_t *line, point_t *item_buff, u16 item_num){return 0;}
bool widget_line_add_item(widget_line_t *line, point_t item_point){return 0;}
bool widget_line_add_item_xy(widget_line_t *line, s16 x, s16 y){return 0;}
bool widget_line_modify_item(widget_line_t *line, u16 item_id, point_t item_point){return 0;}
bool widget_line_set_color(widget_line_t *line, u16 color){return 0;}
bool widget_line_set_width(widget_line_t *line, u16 width){return 0;}
bool widget_line_set_circle(widget_line_t *line, bool onof){return 0;}
u16 widget_line_get_item_num(widget_line_t *line){return 0;}
widget_bar_t *widget_bar_create(widget_page_t *parent){return NULL;}
void widget_bar_set_range(widget_bar_t *bar, int32_t min, int32_t max){}
void widget_bar_set_value(widget_bar_t *bar, int32_t value){}
void widget_bar_set_color(widget_bar_t *bar, u16 color1, u16 color2){}
void widget_bar_set_vh(widget_bar_t *bar, u16 vh){}
void widget_bar_set_rabardius(widget_bar_t *bar, u16 r){}
#endif
