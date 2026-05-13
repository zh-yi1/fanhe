#ifndef _THIRD_BSP_SYS_H__
#define _THIRD_BSP_SYS_H__

#include "include.h"

#if (ASR_SELECT == ASR_YJ)

#if THIRD_UI_LIGHT_EN

void third_light_init();
void third_after_bsp_sys_init();
void third_light_exit();

#endif // THIRD_UI_LIGHT_EN

#endif //ASR_SELECT

#endif
