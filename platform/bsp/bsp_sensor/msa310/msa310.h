#ifndef __MSA310_H__
#define __MSA310_H__

#if (SENSOR_STEP_SEL == SENSOR_STEP_MSA310)

#define MSA310_I2C_SLAVE_ADDR
#if defined(GSENSOR_USE_SW_IIC)
#define MS_I2C_CLK_HIGH				HAL_GPIO_WritePin(MSA_SCL_GPIO_Port, MSA_SCL_Pin, GPIO_PIN_SET)
#define MS_I2C_CLK_LOW				HAL_GPIO_WritePin(MSA_SCL_GPIO_Port, MSA_SCL_Pin, GPIO_PIN_RESET)
#define MS_I2C_DATA_HIGH			HAL_GPIO_WritePin(MSA_SDA_GPIO_Port, MSA_SDA_Pin, GPIO_PIN_SET)
#define MS_I2C_DATA_LOW				HAL_GPIO_WritePin(MSA_SDA_GPIO_Port, MSA_SDA_Pin, GPIO_PIN_RESET)
#define MS_I2C_GET_BIT              HAL_GPIO_ReadPin(MSA_SDA_GPIO_Port, MSA_SDA_Pin)
#endif

#define MSA310_7BIT_IIC_ADDR            0x62
#define MSA310_WRITE_ADDR(ADDR)     	((ADDR) << 1)
#define MSA310_READ_ADDR_UPDATE(ADDR)   ((ADDR) << 1 | 1)
#define MSA310_READ_ADDR(ADDR)      	((ADDR) << 1 | 1) << 8 | ((ADDR) << 1)

#define MSA_REG_SPI_I2C                 0x00
#define MSA_REG_WHO_AM_I                0x01
#define MSA_REG_ACC_X_LSB               0x02
#define MSA_REG_ACC_X_MSB               0x03
#define MSA_REG_ACC_Y_LSB               0x04
#define MSA_REG_ACC_Y_MSB               0x05
#define MSA_REG_ACC_Z_LSB               0x06
#define MSA_REG_ACC_Z_MSB               0x07

#define MSA_REG_MOTION_FLAG			    0x09
#define MSA_REG_G_RANGE                 0x0f
#define MSA_REG_ODR_AXIS_DISABLE        0x10
#define MSA_REG_POWERMODE_BW            0x11
#define MSA_REG_SWAP_POLARITY           0x12
#define MSA_REG_FIFO_CTRL               0x14
#define MSA_REG_INTERRUPT_SETTINGS1     0x16
#define MSA_REG_INTERRUPT_SETTINGS2     0x17
#define MSA_REG_INTERRUPT_MAPPING1      0x19
#define MSA_REG_INTERRUPT_MAPPING2      0x1a
#define MSA_REG_INTERRUPT_MAPPING3      0x1b
#define MSA_REG_INT_PIN_CONFIG          0x20
#define MSA_REG_INT_LATCH               0x21
#define MSA_REG_ACTIVE_DURATION         0x27
#define MSA_REG_ACTIVE_THRESHOLD        0x28
#define MSA_REG_TAP_DURATION            0x2A
#define MSA_REG_TAP_THRESHOLD           0x2B
#define MSA_REG_CUSTOM_OFFSET_X         0x38
#define MSA_REG_CUSTOM_OFFSET_Y         0x39
#define MSA_REG_CUSTOM_OFFSET_Z         0x3a
#define MSA_REG_ENGINEERING_MODE        0x7f
#define MSA_REG_SENSITIVITY_TRIM_X      0x80
#define MSA_REG_SENSITIVITY_TRIM_Y      0x81
#define MSA_REG_SENSITIVITY_TRIM_Z      0x82
#define MSA_REG_COARSE_OFFSET_TRIM_X    0x83
#define MSA_REG_COARSE_OFFSET_TRIM_Y    0x84
#define MSA_REG_COARSE_OFFSET_TRIM_Z    0x85
#define MSA_REG_FINE_OFFSET_TRIM_X      0x86
#define MSA_REG_FINE_OFFSET_TRIM_Y      0x87
#define MSA_REG_FINE_OFFSET_TRIM_Z      0x88
#define MSA_REG_SENS_COMP               0x8c
#define MSA_REG_SENS_COARSE_TRIM        0xd1

typedef unsigned char   uint8_t;
typedef signed char	int8_t;
typedef unsigned short  uint16_t;
typedef signed short	int16_t;

int32_t msa_read_data(int16_t *x, int16_t *y, int16_t *z);
int32_t msa_register_read_continuously( uint8_t addr, uint8_t count, uint8_t *data);
int32_t i2c_write_byte_data( uint8_t addr, uint8_t data);
bool msa_WriteBytes(uint8_t RegAddr, uint8_t Data);
bool msa_ReadBytes(uint8_t* Data, uint8_t RegAddr);
int32_t msa_read_fifo(int16_t *xBuf, int16_t *yBuf, int16_t *zBuf, int len);
uint8_t msa_ReadBytes_ACKEND(uint8_t* Data, uint8_t RegAddr);
uint8_t msa310_init(void);
uint8_t msa310_stop(void);

void MSA310_GetFiFOAccAxesRaw(void);
uint msa310_step(s16 x,s16 y,s16 z);
uint8_t msa310_light(s16 x,s16 y,s16 z);
bool msa310_shake(short x,short y,short z);
void msa_sleep_detect(short x,short y,short z);

void msa310_process_500ms_callback(co_timer_t *timer, void *param);

#endif //(SENSOR_STEP_SEL == SENSOR_STEP_MSA310)

#endif
