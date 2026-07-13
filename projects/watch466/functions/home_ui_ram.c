#include "include.h"
#include "home_ui_ram.h"

home_ui_heat_pool_t home_ui_heat_pool AT(.disp.home_ram);
u8 home_ui_colon_ram[HOME_COLON_RAM_SIZE] AT(.disp.home_ram);
u8 home_ui_show_ram[NEW_HEAT_SHOW_RAM_SIZE];

void home_ui_digit_pool_reset(void)
{
}
