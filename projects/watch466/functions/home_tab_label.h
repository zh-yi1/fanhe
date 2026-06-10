#ifndef _HOME_TAB_LABEL_H
#define _HOME_TAB_LABEL_H

/* 底部 Tab 文字（与 func_mode.c Pasta/Chicken/Warm 同尺寸 5×7 点阵，绘制高 8px） */
#define HOME_TAB_LBL_FONT_W               5
#define HOME_TAB_LBL_FONT_H               7
#define HOME_TAB_LBL_CHAR_SP              1
#define HOME_TAB_LBL_CHAR_W               (HOME_TAB_LBL_FONT_W + HOME_TAB_LBL_CHAR_SP)
#define HOME_TAB_LBL_DRAW_H               8

u16 home_tab_label_text_width(const char *label);
u16 home_tab_label_ram_size(const char *label);
u16 home_tab_label_render(u8 *ram, u16 buf_size, const char *label, u16 bg_color);
void home_tab_label_apply(compo_picturebox_t *pic, u8 *ram, u16 buf_size,
                          const char *label, s16 cx, s16 cy, bool selected, u16 sel_bg_color);

#endif
