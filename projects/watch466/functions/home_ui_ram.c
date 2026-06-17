#include "include.h"
#include "home_ui_ram.h"

u8 home_ui_digit_ram[HOME_UI_DIGIT_SLOTS][HOME_DIGIT_RAM_MAX_SIZE] AT(.disp.home_ram);
/* Home 白冒号 / Heat 灰冒号互斥，共用一块 disp 缓冲 */
u8 home_ui_colon_ram[HOME_COLON_RAM_SIZE] AT(.disp.home_ram);

void home_ui_digit_pool_reset(void)
{
}
