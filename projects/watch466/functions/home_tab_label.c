#include "include.h"
#include "home_tab_label.h"

typedef struct {
    char c;
    u8 g[HOME_TAB_LBL_FONT_H];
} home_tab_lbl_glyph_t;

static const home_tab_lbl_glyph_t tbl_home_tab_lbl_glyph[] = {
    {'A', {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00}},
    {'C', {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}},
    {'D', {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}},
    {'E', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}},
    {'H', {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {'I', {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}},
    {'K', {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}},
    {'M', {0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x00}},
    {'N', {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x00}},
    {'O', {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {'P', {0x1F, 0x11, 0x11, 0x1F, 0x10, 0x10, 0x10}},
    {'R', {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}},
    {'S', {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}},
    {'T', {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
    {'U', {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {'W', {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11}},
    {'a', {0x00, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F}},
    {'c', {0x00, 0x00, 0x0E, 0x10, 0x10, 0x11, 0x0E}},
    {'d', {0x00, 0x00, 0x01, 0x0F, 0x11, 0x11, 0x0F}},
    {'e', {0x00, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0E}},
    {'h', {0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11}},
    {'i', {0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x0E}},
    {'k', {0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12}},
    {'m', {0x00, 0x00, 0x1A, 0x15, 0x15, 0x11, 0x00}},
    {'n', {0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11}},
    {'o', {0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E}},
    {'p', {0x00, 0x00, 0x1E, 0x11, 0x11, 0x1E, 0x10}},
    {'r', {0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10}},
    {'s', {0x00, 0x00, 0x0E, 0x10, 0x0E, 0x01, 0x1E}},
    {'t', {0x04, 0x04, 0x0E, 0x04, 0x04, 0x04, 0x06}},
    {'u', {0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0D}},
    {'w', {0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A}},
};

static const u8 *home_tab_lbl_glyph(char c)
{
    u8 i;

    for (i = 0; i < (sizeof(tbl_home_tab_lbl_glyph) / sizeof(tbl_home_tab_lbl_glyph[0])); i++) {
        if (tbl_home_tab_lbl_glyph[i].c == c) {
            return tbl_home_tab_lbl_glyph[i].g;
        }
    }
    return NULL;
}

u16 home_tab_label_text_width(const char *label)
{
    u16 len;

    if (label == NULL) {
        return 0;
    }
    len = (u16)strlen(label);
    if (len == 0) {
        return 0;
    }
    return (u16)(len * HOME_TAB_LBL_CHAR_W - HOME_TAB_LBL_CHAR_SP);
}

u16 home_tab_label_ram_size(const char *label)
{
    u16 tw = home_tab_label_text_width(label);

    if (tw == 0) {
        return 0;
    }
    return (u16)(8 + (u32)tw * HOME_TAB_LBL_DRAW_H * 2);
}

static void home_tab_lbl_ram_pixel(u8 *ram, u16 buf_w, s16 x, s16 y, u16 color)
{
    u16 buf_h = GET_LE16(&ram[6]);

    if (x < 0 || y < 0 || x >= buf_w || y >= buf_h) {
        return;
    }
    PUT_LE16(&ram[8 + ((u32)y * buf_w + (u32)x) * 2], color);
}

static void home_tab_lbl_draw_char(u8 *ram, u16 buf_w, s16 x0, s16 y0, char c)
{
    const u8 *glyph = home_tab_lbl_glyph(c);
    u8 row;
    u8 col;

    if (glyph == NULL) {
        return;
    }

    for (row = 0; row < HOME_TAB_LBL_FONT_H; row++) {
        u8 bits = glyph[row];

        for (col = 0; col < HOME_TAB_LBL_FONT_W; col++) {
            if (bits & (1 << (HOME_TAB_LBL_FONT_W - 1 - col))) {
                home_tab_lbl_ram_pixel(ram, buf_w, (s16)(x0 + col), (s16)(y0 + row), COLOR_WHITE);
            }
        }
    }
}

u16 home_tab_label_render(u8 *ram, u16 buf_size, const char *label, u16 bg_color)
{
    u16 tw = home_tab_label_text_width(label);
    u16 th = HOME_TAB_LBL_DRAW_H;
    u16 data_len;
    u32 pix_cnt;
    u32 i;
    s16 x;
    s16 y;

    if (ram == NULL || label == NULL || tw == 0) {
        return 0;
    }

    data_len = (u16)(8 + (u32)tw * th * 2);
    if (data_len > buf_size) {
        return 0;
    }

    PUT_LE32(&ram[0], 0x24150);
    PUT_LE16(&ram[4], tw);
    PUT_LE16(&ram[6], th);
    pix_cnt = (u32)tw * th;
    for (i = 0; i < pix_cnt; i++) {
        PUT_LE16(&ram[8 + i * 2], bg_color);
    }

    x = 0;
    y = (s16)((th - HOME_TAB_LBL_FONT_H) / 2);
    while (*label != '\0') {
        home_tab_lbl_draw_char(ram, tw, x, y, *label);
        x += HOME_TAB_LBL_CHAR_W;
        label++;
    }
    return data_len;
}

static bool home_tab_label_gpu_set(u8 *ram, u16 buf_size, u16 data_len, compo_picturebox_t *pic)
{
    u16 need;

    if (ram == NULL || data_len == 0 || data_len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    if (!gui_set_ram_check(ram, __func__)) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    need = (u16)(8 + (u32)GET_LE16(&ram[4]) * GET_LE16(&ram[6]) * 2);
    if (need > data_len || need > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    if (pic != NULL) {
        compo_picturebox_set_ram(pic, ram);
        compo_picturebox_set_visible(pic, true);
    }
    return true;
}

void home_tab_label_apply(compo_picturebox_t *pic, u8 *ram, u16 buf_size,
                          const char *label, s16 cx, s16 cy, bool selected, u16 sel_bg_color)
{
    u16 tw = home_tab_label_text_width(label);
    u16 bg_color = selected ? sel_bg_color : COLOR_BLACK;
    u16 data_len;

    if (pic == NULL || ram == NULL || tw == 0) {
        return;
    }

    data_len = home_tab_label_render(ram, buf_size, label, bg_color);
    if (data_len == 0) {
        compo_picturebox_set_visible(pic, false);
        return;
    }

    if (home_tab_label_gpu_set(ram, buf_size, data_len, pic)) {
        compo_picturebox_set_size(pic, tw, HOME_TAB_LBL_DRAW_H);
        compo_picturebox_set_pos(pic, cx, cy);
        compo_picturebox_set_visible(pic, true);
    }
}
