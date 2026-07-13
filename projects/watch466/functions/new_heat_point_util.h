#ifndef _NEW_HEAT_POINT_UTIL_H
#define _NEW_HEAT_POINT_UTIL_H

#include "include.h"

/* 圆点下方背景（轨道或弧形灰轨 RAM），圆环外像素与此同色 */
typedef struct {
    const u8 *bg_ram;
    u16 bg_ram_len;
    u16 bg_w;
    u16 bg_h;
    s16 bg_anchor_x;
    s16 bg_anchor_y;
} new_heat_point_bg_t;

bool new_heat_point_gpu_ram_bind(compo_picturebox_t *pic,
                                 u8 *ram, u16 ram_cap,
                                 u16 point_w, u16 point_h,
                                 s16 px, s16 py,
                                 const new_heat_point_bg_t *bg);

#endif
