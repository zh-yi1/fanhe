#include "include.h"
#include "new_heat_point_util.h"
#include "home_ui_shared.h"

#define NEW_HEAT_POINT_BLUE565          0x42FF
#define NEW_HEAT_POINT_BLUE_R10         61
#define NEW_HEAT_POINT_RING_OUTER_R10   103

static u32 new_heat_point_dist2_x10(u16 x, u16 y, u16 cc)
{
    s32 dx = ((s32)x - (s32)cc) * 10 + 5;
    s32 dy = ((s32)y - (s32)cc) * 10 + 5;

    return (u32)(dx * dx + dy * dy);
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

static u16 new_heat_point_pixel_color(u16 x, u16 y, u16 cc, s16 sx, s16 sy,
                                      const new_heat_point_bg_t *bg)
{
    u32 d2;
    u32 blue_r2;
    u32 ring_r2;

    d2 = new_heat_point_dist2_x10(x, y, cc);
    blue_r2 = (u32)NEW_HEAT_POINT_BLUE_R10 * NEW_HEAT_POINT_BLUE_R10;
    ring_r2 = (u32)NEW_HEAT_POINT_RING_OUTER_R10 * NEW_HEAT_POINT_RING_OUTER_R10;

    if (d2 <= blue_r2) {
        return NEW_HEAT_POINT_BLUE565;
    }
    if (d2 <= ring_r2) {
        return 0xFFFF;
    }
    return new_heat_point_bg_at(bg, sx, sy);
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
