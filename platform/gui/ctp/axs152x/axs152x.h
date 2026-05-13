#ifndef __AXS152X_H__
#define __AXS152X_H__


#define AXS_TOUCH_ONE_POINT_LEN             6
#define AXS_TOUCH_BUF_HEAD_LEN              2

#define AXS_TOUCH_GESTURE_POS               0
#define AXS_TOUCH_POINT_NUM                 1
#define AXS_TOUCH_EVENT_POS                 2
#define AXS_TOUCH_X_H_POS                   2
#define AXS_TOUCH_X_L_POS                   3
#define AXS_TOUCH_ID_POS                    4
#define AXS_TOUCH_Y_H_POS                   4
#define AXS_TOUCH_Y_L_POS                   5
#define AXS_TOUCH_WEIGHT_POS                6
#define AXS_TOUCH_AREA_POS                  7

#define AXS_GET_POINT_NUM(buf) buf[AXS_TOUCH_POINT_NUM]
#define AXS_GET_GESTURE_TYPE(buf)  buf[AXS_TOUCH_GESTURE_POS]
#define AXS_GET_POINT_X(buf) (((u16)(buf[AXS_TOUCH_X_H_POS] & 0x0F) <<8) + (u16)buf[AXS_TOUCH_X_L_POS])
#define AXS_GET_POINT_Y(buf) (((u16)(buf[AXS_TOUCH_Y_H_POS] & 0x0F) <<8) + (u16)buf[AXS_TOUCH_Y_L_POS])
#define AXS_GET_POINT_EVENT(buf) (buf[AXS_TOUCH_EVENT_POS] >> 6)


#define I2C_ADDR_AXS152X                 (0x3B)
#define I2C_AXS152X_WRITE_ADDR(ADDR)     		 ((ADDR) << 1)				//CTP IIC写地址
#define I2C_AXS152X_READ_ADDR(ADDR)      		 (((ADDR) << 1) +1)			//CTP IIC读地址

#define AXS_CMD_MAX_WRITE_BUF_LEN       1024
#define AXS_CMD_MAX_READ_BUF_LEN        1024

bool AXS152x_init(void);
void AXS152x_read_readkick(void);
u8 AXS152x_get_touch_point(s32 *x, s32 *y);


#endif // __BSP_I2C_CST716_H__
