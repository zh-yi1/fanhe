#include "include.h"
#include "new_heat_point_util.h"
#include "home_ui_shared.h"

#define NEW_HEAT_POINT_BLUE565          0x42FF
#define NEW_HEAT_POINT_BLUE_R10         61
#define NEW_HEAT_POINT_RING_OUTER_R10   103
/* 4x4 超采样：半径换算到 1/40 像素（x10 * 4） */
#define NEW_HEAT_POINT_SS               4
#define NEW_HEAT_POINT_BLUE_R40         ((s32)NEW_HEAT_POINT_BLUE_R10 * NEW_HEAT_POINT_SS)
#define NEW_HEAT_POINT_RING_R40         ((s32)NEW_HEAT_POINT_RING_OUTER_R10 * NEW_HEAT_POINT_SS)

static void new_heat_point_rgb565_split(u16 c, u16 *r, u16 *g, u16 *b)
{
    *r = (u16)((c >> 11) & 0x1f);
    *g = (u16)((c >> 5) & 0x3f);
    *b = (u16)(c & 0x1f);
}

static u16 new_heat_point_rgb565_pack(u32 r, u32 g, u32 b)
{
    return (u16)(((r & 0x1f) << 11) | ((g & 0x3f) << 5) | (b & 0x1f));
}

static u16 new_heat_point_bg_at(const new_heat_point_bg_t *bg, s16 sx, s16 sy)
{
    s16 tx;
    s16 ty;
    u32 di;

    if (bg == NULL || bg->bg_ram == NULL || bg->bg_ram_len < 8 ||
        bg->bg_w == 0 || bg->bg_h == 0) {
        return COLOR_WHITE;
    }

    tx = sx - (bg->bg_anchor_x - (s16)bg->bg_w / 2);
    ty = sy - (bg->bg_anchor_y - (s16)bg->bg_h / 2);
    if (tx < 0 || ty < 0 || tx >= (s16)bg->bg_w || ty >= (s16)bg->bg_h) {
        return COLOR_WHITE;
    }

    di = 8 + ((u32)ty * bg->bg_w + (u32)tx) * 2;
    if (di + 1 >= bg->bg_ram_len) {
        return COLOR_WHITE;
    }
    return GET_LE16(&bg->bg_ram[di]);
}

/*
 * 以画布几何中心为圆心做欧氏距离 4x4 超采样，硬阈值改为覆盖率混合，
 * 避免 ~6px 半径光栅化后出现明显棱角/六边形观感。
 */
static u16 new_heat_point_pixel_color(u16 x, u16 y, u16 cc, s16 sx, s16 sy,
                                      const new_heat_point_bg_t *bg)
{
    u32 blue_r2;
    u32 ring_r2;
    u32 sum_r = 0;
    u32 sum_g = 0;
    u32 sum_b = 0;
    u8 si;
    u8 sj;
    s32 cx40;
    s32 cy40;
    u16 bg_c;
    u16 cr;
    u16 cg;
    u16 cb;

    blue_r2 = (u32)NEW_HEAT_POINT_BLUE_R40 * (u32)NEW_HEAT_POINT_BLUE_R40;
    ring_r2 = (u32)NEW_HEAT_POINT_RING_R40 * (u32)NEW_HEAT_POINT_RING_R40;
    /* 像素中心坐标系：真正圆心在 cc+0.5 → x40 下为 cc*40+20 */
    cx40 = (s32)cc * 40 + 20;
    cy40 = cx40;
    bg_c = new_heat_point_bg_at(bg, sx, sy);

    for (si = 0; si < NEW_HEAT_POINT_SS; si++) {
        for (sj = 0; sj < NEW_HEAT_POINT_SS; sj++) {
            s32 px40 = (s32)x * 40 + (s32)sj * 10 + 5;
            s32 py40 = (s32)y * 40 + (s32)si * 10 + 5;
            s32 dx = px40 - cx40;
            s32 dy = py40 - cy40;
            u32 d2 = (u32)(dx * dx + dy * dy);
            u16 c;

            if (d2 <= blue_r2) {
                c = NEW_HEAT_POINT_BLUE565;
            } else if (d2 <= ring_r2) {
                c = 0xFFFF;
            } else {
                c = bg_c;
            }
            new_heat_point_rgb565_split(c, &cr, &cg, &cb);
            sum_r += cr;
            sum_g += cg;
            sum_b += cb;
        }
    }

    return new_heat_point_rgb565_pack(sum_r / 16, sum_g / 16, sum_b / 16);
}

static bool new_heat_point_ram_build(u8 *ram, u16 ram_cap, u16 pw, u16 ph,
                                     s16 px, s16 py, const new_heat_point_bg_t *bg)
{
    u16 cc;
    u16 y;
    u16 x;
    u32 need;
    s16 ox;
    s16 oy;

    if (ram == NULL || pw == 0 || ph == 0) {
        return false;
    }

    need = 8 + (u32)pw * ph * 2;
    if (need > ram_cap) {
        return false;
    }

    cc = pw / 2;
    ox = px - (s16)pw / 2;
    oy = py - (s16)ph / 2;

    PUT_LE32(&ram[0], 0x24150);
    PUT_LE16(&ram[4], pw);
    PUT_LE16(&ram[6], ph);

    for (y = 0; y < ph; y++) {
        for (x = 0; x < pw; x++) {
            u16 c = new_heat_point_pixel_color(x, y, cc,
                                               (s16)(ox + (s16)x),
                                               (s16)(oy + (s16)y), bg);
            u32 di = 8 + ((u32)y * pw + (u32)x) * 2;

            PUT_LE16(&ram[di], c);
        }
    }
    return true;
}

bool new_heat_point_gpu_ram_bind(compo_picturebox_t *pic,
                                 u8 *ram, u16 ram_cap,
                                 u16 point_w, u16 point_h,
                                 s16 px, s16 py,
                                 const new_heat_point_bg_t *bg)
{
    if (pic == NULL || ram == NULL) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    home_gpu_wait_idle();
    if (!new_heat_point_ram_build(ram, ram_cap, point_w, point_h, px, py, bg)) {
        compo_picturebox_set_visible(pic, false);
        return false;
    }
    WDT_CLR();
    if (!gui_set_ram_check(ram, __func__)) {
        compo_picturebox_set_visible(pic, false);
        return false;
    }

    home_gpu_wait_idle();
    compo_picturebox_set_ram(pic, ram);
    compo_picturebox_set_size(pic, point_w, point_h);
    compo_picturebox_set_pos(pic, px, py);
    compo_picturebox_set_visible(pic, true);
    return true;
}
