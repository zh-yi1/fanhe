#ifndef _BSP_PT8028_KEY_H
#define _BSP_PT8028_KEY_H

#include "bsp_key.h"

typedef enum {
    PT8028_KEY_NONE = 0xFF,
    PT8028_KEY_TCH0 = 0,
    PT8028_KEY_TCH1 = 1,
    PT8028_KEY_TCH2 = 2,
    PT8028_KEY_TCH3 = 3,
    PT8028_KEY_TCH4 = 4,
    PT8028_KEY_TCH5 = 5,
    PT8028_KEY_TCH6 = 6,
    PT8028_KEY_TCH7 = 7,
} pt8028_key_id_t;

void pt8028_key_init(void);
u8 get_pt8028_key(void);

#endif // _BSP_PT8028_KEY_H
