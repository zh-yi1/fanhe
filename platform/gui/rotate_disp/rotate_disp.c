#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

#if BOX_GUI_ROTATE_DISP

AT(.com_text.rotate_disp)
s16 ro_90_counterclockwise_get_x(s16 x, s16 y, s16 page_hei)
{
    s16 rotate_x = page_hei - y;
    return rotate_x;
}

AT(.com_text.rotate_disp)
s16 ro_90_counterclockwise_get_y(s16 x, s16 y)
{
    s16 rotate_y = x;
    return rotate_y;
}

void ro_90_counterclockwise_get_xy(s16 *x, s16 *y, s16 page_hei)
{
    s16 input_x = *x;
    s16 input_y = *y;

    *x = ro_90_counterclockwise_get_x(input_x, input_y, page_hei);
    *y = ro_90_counterclockwise_get_y(input_x, input_y);
}

void ro_90_counterclockwise_get_dxy_s16(s16 *dx, s16 *dy)
{
    s16 input_x = *dx;

    *dx = -*dy;
    *dy = input_x;
}

void ro_90_counterclockwise_get_dxy_s32(s32 *dx, s32 *dy)
{
    s32 input_x = *dx;

    *dx = -*dy;
    *dy = input_x;
}
#endif // BOX_GUI_ROTATE_DISP
