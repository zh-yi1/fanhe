#ifndef _HOME_UI_RAM_H
#define _HOME_UI_RAM_H

#include "home_icon_res.h"

/* 七段数字/冒号：必须在 .disp.home_ram，GPU 通过 set_ram 渲染 */
#define HOME_UI_DIGIT_SLOTS             4

extern u8 home_ui_digit_ram[HOME_UI_DIGIT_SLOTS][HOME_DIGIT_RAM_MAX_SIZE];
extern u8 home_ui_colon_ram[HOME_COLON_RAM_SIZE];

void home_ui_digit_pool_reset(void);

#endif
