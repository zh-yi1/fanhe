#ifndef _HOME_UI_RAM_H
#define _HOME_UI_RAM_H

#include "home_icon_res.h"
#include "new_heat_res.h"

/* digit 与 heat_bg 互斥（new_heat 页 vs 加热面板页），共用 union 省 .disp */
#define HOME_UI_DIGIT_SLOTS             4

typedef union {
    u8 digit[HOME_UI_DIGIT_SLOTS][HOME_DIGIT_RAM_MAX_SIZE];
    u8 heat_bg[NEW_HEAT_NEW_PROGRESS_BG_RAM_SIZE];
} home_ui_heat_pool_t;

extern home_ui_heat_pool_t home_ui_heat_pool;
extern u8 home_ui_colon_ram[HOME_COLON_RAM_SIZE];
extern u8 home_ui_show_ram[NEW_HEAT_SHOW_RAM_SIZE];

#define home_ui_digit_ram               home_ui_heat_pool.digit
#define home_ui_heat_bg_ram             home_ui_heat_pool.heat_bg

void home_ui_digit_pool_reset(void);

#endif
