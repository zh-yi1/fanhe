#ifndef __AXS5106_H__
#define __AXS5106_H__

#define         TOUCH_UNKOWN        0
#define         TOUCH_PRESS         BIT(0)
#define         TOUCH_REALESE       BIT(1)
#define         TOUCH_HOLD          BIT(2)
#define         TOUCH_MOVE_UP       BIT(3)
#define         TOUCH_MOVE_DOWN     BIT(4)
#define         TOUCH_MOVE_LEFT     BIT(5)
#define         TOUCH_MOVE_RIGHT    BIT(6)

#define TRUE  true
#define FALSE false

bool  AXS5106_init(void);
void AXS5106_isr(void);
void AXS5106_read_readkick(void);
bool AXS5106_get_touch_point(s32 *x, s32 *y);

void AXS5106_hynitron_update_check(void);

#endif // __BSP_I2C_CST716_H__
