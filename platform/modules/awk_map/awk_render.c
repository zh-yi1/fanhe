#include "include.h"
#include "awk.h"

static void awk_render_begin_drawing_adapter(uint32_t map_id, awk_render_context_t status)
{
    // printf("%s\n", __func__);
}

static void awk_render_commit_drawing_adapter(uint32_t map_id)
{
    // printf("%s\n", __func__);
}

static void awk_render_point_adapter(uint32_t map_id, awk_point_t *point, uint32_t point_size, const awk_paint_style_t *style)
{
    //  printf("%s\n", __func__);
}

static void awk_render_polyline_adapter(uint32_t map_id, awk_point_t *points, uint32_t point_size, const awk_paint_style_t *style)
{
    // printf("%s %d %x\n", __func__, style->width, style->color);
    // for (int i = 0; i < point_size; i++) {
    //     printf("%d %d\n", points[i].x, points[i].y);
    // }
}

static void awk_render_polygon_adapter(uint32_t map_id, awk_point_t *points, uint32_t point_size, const awk_paint_style_t *style)
{
    // printf("%s\n", __func__);
}

void awk_map_tile_updata(u32 x_s, u32 y_s, int32_t w, int32_t h, u8 *input_buf, u32 size);
static void awk_render_bitmap_adapter(uint32_t map_id, awk_rect_area_t area, awk_bitmap_t bitmap, const awk_paint_style_t *style)
{
    // printf("%s\n", __func__);
    // printf("pixel_mode:%d, buffer_size:%d, width:%d, heigh:%d, stride:%d, pre_multiplied:%d\n", bitmap.pixel_mode, bitmap.buffer_size, bitmap.width, bitmap.height, bitmap.stride, bitmap.pre_multiplied);
    // printf("area x:%d, y:%d, width:%d, height:%d\n", area.x, area.y, area.width, area.height);

    awk_map_tile_updata(area.x, area.y, area.width, area.height, bitmap.buffer, bitmap.buffer_size);
}

static void awk_render_color_adapter(uint32_t map_id, awk_rect_area_t area, const awk_paint_style_t *style)
{
    // printf("%s\n", __func__);
}

static void awk_render_text_adapter(uint32_t map_id, awk_point_t center, const char *text, const awk_paint_style_t *style)
{
    // printf("%s\n", __func__);
}

static bool awk_measure_text_adapter(uint32_t map_id, const char *text, const awk_paint_style_t *style, int32_t *width, int32_t *ascender, int32_t *descender)
{
    // printf("%s\n", __func__);
    return true;
}

static const awk_render_adapter_t render_adapter = {
    .begin_drawing = awk_render_begin_drawing_adapter,
    .commit_drawing = awk_render_commit_drawing_adapter,
    .draw_point = awk_render_point_adapter,
    .draw_polyline = awk_render_polyline_adapter,
    .draw_polygon = awk_render_polygon_adapter,
    .draw_bitmap = awk_render_bitmap_adapter,
    .draw_text = awk_render_text_adapter,
    .draw_color = awk_render_color_adapter,
    .measure_text = awk_measure_text_adapter,
};

const awk_render_adapter_t *awk_render_get_adapter(void)
{
    return &render_adapter;
}
