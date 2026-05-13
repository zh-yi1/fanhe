
#include "include.h"
#include "msa310.h"

#if (SENSOR_STEP_SEL == SENSOR_STEP_MSA310)

#include "msa_app.h"

#define WRITE	0
#define READ 	1
#define abs(x) (((x) < 0) ? (-(x)) : (x))

uint8_t msa_id=0;

typedef struct
{
    short int AXIS_X;
    short int AXIS_Y;
    short int AXIS_Z;
} AxesRaw_t;

static unsigned int   SL_STEP                  		= 0;//计步值
static unsigned int   SL_LIGHT                 		= 0;//抬手亮屏
static unsigned char  SL_SWAY_STATUS           		= 0;//摇一摇拍照
static unsigned char  SL_SLEEP           		  	= 0;//睡眠状态
//static uint8_t slee_cnt = 0;//睡眠变量

char data_cnt;
unsigned char data_count = 0;

/***************************************************/
/*			software I2C APIs for motion sensor 				  */
/*=================================================*/
/***************************************************/
#if defined(GSENSOR_USE_SW_IIC)
static int8_t msa_i2c_addr = 0x62;

void MSA_SDA_Out(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = MSA_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(MSA_SDA_GPIO_Port, &GPIO_InitStruct);
}
void MSA_SDA_In(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = MSA_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(MSA_SDA_GPIO_Port, &GPIO_InitStruct);
}

void MSA_SCL_Out(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = MSA_SCL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(MSA_SCL_GPIO_Port, &GPIO_InitStruct);
}

#if 0
int16_t msa_sqrt(int32_t x)
{
	double a = 1.0;
	while(abs(a * a - x) > 100)
	{
		a = (a + x / a) / 2;
	}
	return (int16_t)a;
}
#endif

/******************************************
	us delay for STM32F429, HLK 168Mhz
	uint:us
*******************************************/

void st_delay_us(uint32_t delay)
{
	uint32_t i;
	while(delay--)
	{
		i =10;
		while(i--);
	}
}

/******************************************
	us delay for software I2C
	uint:us
*******************************************/
void SW_i2c_udelay(uint32_t delay)
{
		uint32_t i;
		for(i=0;i<delay;i++)
			st_delay_us(1);
}

/******************************************
	software I2C start bit
*******************************************/
void SW_i2c_start(void)
{
	MSA_SCL_Out();

	MSA_SDA_Out();

	MS_I2C_DATA_HIGH;
	MS_I2C_CLK_HIGH;
	SW_i2c_udelay(40);		//20
	MS_I2C_DATA_LOW;
	SW_i2c_udelay(20);		//10
	MS_I2C_CLK_LOW;
	SW_i2c_udelay(20);		//10
}

/******************************************
	software I2C stop bit
*******************************************/
void SW_i2c_stop(void)
{
	MSA_SCL_Out();
	MSA_SDA_Out();

	MS_I2C_DATA_LOW;
	SW_i2c_udelay(20);		//10
	MS_I2C_CLK_HIGH;
	SW_i2c_udelay(20);		//10
	MS_I2C_DATA_HIGH;
    SW_i2c_udelay(20);		//10
}

/******************************************
	software I2C one clock
*******************************************/
void SW_i2c_one_clk(void)
{
	SW_i2c_udelay(10);		//5
	MS_I2C_CLK_HIGH;
	SW_i2c_udelay(20);		//10
	MS_I2C_CLK_LOW;
	SW_i2c_udelay(10);		//5
}

/******************************************
	software I2C read byte with ack
*******************************************/
uint8_t ms_ReadByteAck(void)
{
	int8_t i;
	uint8_t data;

  MSA_SDA_In();
	data = 0;

	for (i=7; i>=0; i--)
	{
		if (MS_I2C_GET_BIT)
		{
			data |= (0x01<<i);
		}
		SW_i2c_one_clk();
	}

	MSA_SDA_Out();
	MS_I2C_DATA_LOW;
	SW_i2c_one_clk();

	return data;
}

/******************************************
	software I2C read byte without ack
*******************************************/
uint8_t ms_ReadByteNAck(void)
{
	int8_t i;
	uint8_t data;

	MSA_SDA_In();
	data = 0;

	for (i=7; i>=0; i--)
	{
		if (MS_I2C_GET_BIT)
		{
			data |= (0x01<<i);
		}
		SW_i2c_one_clk();
	}

	MSA_SDA_Out();
	MS_I2C_DATA_HIGH;
	SW_i2c_one_clk();

	return data;
}

/******************************************
	software I2C send byte
*******************************************/
void ms_SendByte(uint8_t sData)
{
	int8_t i;

	for (i=7; i>=0; i--)
	{
		if ((sData>>i)&0x01)
		{
			MS_I2C_DATA_HIGH;
		}
		else
		{
			MS_I2C_DATA_LOW;
		}
		SW_i2c_one_clk();
	}
}

/******************************************
	software I2C check ack bit
*******************************************/
uint8_t ms_ChkAck(void)
{
	MSA_SDA_In();

	SW_i2c_udelay(10);		//5
	MS_I2C_CLK_HIGH;
	SW_i2c_udelay(10);		//5

	if(MS_I2C_GET_BIT)		//Non-ack
	{
		SW_i2c_udelay(10);	//5
		MS_I2C_CLK_LOW;
		SW_i2c_udelay(10);	//5

		MSA_SDA_Out();
		MS_I2C_DATA_LOW;

		return false;
	}
	else					//Ack
	{
		SW_i2c_udelay(10);	//5
		MS_I2C_CLK_LOW;
		SW_i2c_udelay(10);	//5

		MSA_SDA_Out();
		MS_I2C_DATA_LOW;

		return true;
	}
}

/******************************************
	software I2C restart bit
*******************************************/
void ms_Restart(void)
{
	MSA_SCL_Out();
	MSA_SDA_Out();

	SW_i2c_udelay(40);
	MS_I2C_DATA_HIGH;
	SW_i2c_udelay(20);		//10
	MS_I2C_CLK_HIGH;
	SW_i2c_udelay(40);
	MS_I2C_DATA_LOW;
	SW_i2c_udelay(40);		//10
	MS_I2C_CLK_LOW;
	SW_i2c_udelay(20);		//10
}


/***************************************************/
/*			motion sensor read and write APIs 				  */
/*=================================================*/
/***************************************************/
/******************************************
	msa ms delay function
		uint: ms
*******************************************/
void msa_DelayMS(uint16_t delay)
{
	uint16_t i=0;

	for(i=0; i<delay; i++)
	{
		SW_i2c_udelay(1000);
	}
}

/******************************************
	msa read bytes
*******************************************/
uint8_t msa_ReadBytes(uint8_t* Data, uint8_t RegAddr)
{
	SW_i2c_start();						//start bit
	ms_SendByte(msa_i2c_addr << 1);		//slave address|write bit
	if(false == ms_ChkAck())		//check Ack bit
	{
        printf("I2C error!");
		//TO_DO: display ack check fail when send write id
		SW_i2c_stop();
		return false;
	}

	ms_SendByte(RegAddr);				//send RegAddr
	if(false == ms_ChkAck())		//check Ack bit
	{
        printf("I2C error!");
		//TO_DO: display ack check fail when send RegAddr
		SW_i2c_stop();
		return false;
	}

	ms_Restart();						//restart bit

	ms_SendByte(msa_i2c_addr<<1 | READ);		//slave address|read bit
	if(false == ms_ChkAck())
	{
        printf("I2C error!");
		//TO_DO: display ack check fail when send read id
		SW_i2c_stop();
		return false;
	}

	*Data = ms_ReadByteNAck();

	SW_i2c_stop();						//stop bit

	//TO_DO: add debug code to display the data received

	return true;

}


/******************************************
	msa write bytes
*******************************************/
uint8_t msa_WriteBytes(uint8_t RegAddr, uint8_t Data)
{
	SW_i2c_start();						//start bit

	ms_SendByte(msa_i2c_addr<<1 | WRITE);		//slave address|write bit
	if(false == ms_ChkAck())		//check Ack bit
	{
        printf("I2C error!");
		//TO_DO: display check ack fail when send write id
		SW_i2c_stop();
		return false;
	}

	ms_SendByte(RegAddr);				//send RegAddr
	if(false == ms_ChkAck())		//check Ack bit
	{
		//TO_DO: display check ack fail when send RegAddr
        printf("I2C error!");
		SW_i2c_stop();
		return false;
	}

	ms_SendByte(Data);					//send parameter
	if(false == ms_ChkAck())
	{
		//TO_DO: display check ack fail when send data
        printf("I2C error!");
		SW_i2c_stop();
		return false;
	}

	SW_i2c_stop();						//stop bit

	return true;
}

#else
bool msa_ConReadBytes(uint8_t* Data, uint8_t RegAddr, uint16_t Length)
{
	uint32_t i2c_cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0 | START_FLAG1 | DEV_ADDR1;
	bsp_hw_i2c_rx_buf(SENSOR_STEP_IICX, i2c_cfg, MSA310_READ_ADDR(MSA310_7BIT_IIC_ADDR), RegAddr, Data, Length);
	return true;
}

//QMA6981 write bytes
bool msa_WriteBytes(uint8_t RegAddr, uint8_t Data)
{
	uint32_t i2c_cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0 | WDATA | STOP_FLAG;
	bsp_hw_i2c_tx_byte(SENSOR_STEP_IICX, i2c_cfg, MSA310_WRITE_ADDR(MSA310_7BIT_IIC_ADDR), RegAddr, Data);
    return true;
}
#endif

/*return value: 0: is ok    other: is failed*/
int32_t     i2c_read_byte_data( uint8_t addr, uint8_t *data){

		int32_t ret = 0;


		ret = msa_ConReadBytes(data,addr,1);

		return (!ret);
}

/*return value: 0: is ok    other: is failed*/
int32_t  i2c_write_byte_data( uint8_t addr, uint8_t data){

		int32_t ret = 0;

		ret = msa_WriteBytes(addr,data);

		return (!ret);
}

/*return value: 0: is count    other: is failed*/
int32_t     i2c_read_block_data( uint8_t base_addr, uint8_t count, uint8_t *data){
	int32_t i = 0;

	for(i = 0; i < count;i++)
	{
		if(i2c_read_byte_data(base_addr+i,(data+i)))
		{
			return -1;

		}
	}
	return count;
}

int32_t msa_register_read( uint8_t addr, uint8_t *data){
    int32_t     res = 0;

    res = i2c_read_byte_data(addr, data);
    if(res != 0) {
          return res;
    }

    return res;
}

int32_t msa_register_write( uint8_t addr, uint8_t data){
    int32_t     res = 0;

    res = i2c_write_byte_data(addr, data);
    if(res != 0) {
         return res;
    }

    return res;
}

int32_t msa_register_read_continuously( uint8_t addr, uint8_t count, uint8_t *data)
{
    int32_t     res = 0;

    res = (count==i2c_read_block_data(addr, count, data)) ? 0 : 1;
    if(res != 0) {
			 return res;
    }

    return res;
}

int32_t msa_register_mask_write(uint8_t addr, uint8_t mask, uint8_t data)
{
    int32_t     res = 0;
    uint8_t      tmp_data;

    res = msa_register_read(addr, &tmp_data);
    if(res) {
        return res;
    }

    tmp_data &= ~mask;
    tmp_data |= data & mask;

    res = msa_register_write(addr, tmp_data);

    return res;
}

/*return value: 0: is ok    other: is failed*/
int32_t msa_read_data(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t    tmp_data[6] = {0};

    if (msa_register_read_continuously(MSA_REG_ACC_X_LSB, 6, tmp_data) != 0) {
  			return -1;
    }

    *x = ((short)(tmp_data[1] << 8 | tmp_data[0]))>> 4;
    *y = ((short)(tmp_data[3] << 8 | tmp_data[2]))>> 4;
    *z = ((short)(tmp_data[5] << 8 | tmp_data[4]))>> 4;

    return 0;
}

int32_t msa_read_fifo(int16_t *xBuf, int16_t *yBuf, int16_t *zBuf, int len)
{
    #if 0
    int i = 0;
    uint8_t temp_buf[384];
    //#else
    SW_i2c_start();						//start bit
	ms_SendByte(msa_i2c_addr << 1);		//slave address|write bit
	if(false == ms_ChkAck())		//check Ack bit
	{
        printf("I2C error!");
		//TO_DO: display ack check fail when send write id
		SW_i2c_stop();
		return false;
	}

	ms_SendByte(0xff);				//send RegAddr
	if(false == ms_ChkAck())		//check Ack bit
	{
        printf("I2C error!");
		//TO_DO: display ack check fail when send RegAddr
		SW_i2c_stop();
		return false;
	}

	ms_Restart();						//restart bit

    ms_SendByte(msa_i2c_addr<<1 | READ);		//slave address|read bit
	if(false == ms_ChkAck())
	{
		//TO_DO: display ack check fail when send read id
		SW_i2c_stop();
		return false;
	}

    for(i = 0; i < (len * 6);i++)
        //temp_buf[i] = ms_ReadByteNAck();
        temp_buf[i] = ms_ReadByteAck();

    for(i = 0; i < len; i++)
    {
        xBuf[i] = ((short)(temp_buf[i*6+1] << 8 | temp_buf[i*6+0]))>> 4;
        yBuf[i] = ((short)(temp_buf[i*6+3] << 8 | temp_buf[i*6+2]))>> 4;
        zBuf[i] = ((short)(temp_buf[i*6+5] << 8 | temp_buf[i*6+4]))>> 4;
    }

	SW_i2c_stop();						//stop bit

#endif
	return true;
}

// u8 MSA310_GetAccAxesRaw(AxesRaw_t* buff)
// {
	// uint8_t    tmp_data[6] = {0};
	// if (msa_register_read_continuously(MSA_REG_ACC_X_LSB, 6, tmp_data) != 0) {
	// 	return -1;
	// }

	// buff->AXIS_X = ((short)(tmp_data[3] << 8 | tmp_data[2]))>> 4;
	// buff->AXIS_Y = ((short)(tmp_data[1] << 8 | tmp_data[0]))>> 4;
	// buff->AXIS_Z = ((short)(tmp_data[5] << 8 | tmp_data[4]))>> 4;

	// return 0;
// }

/*return value: 0: is ok    other: is failed*/
uint8_t msa310_init(void)
{
	int32_t res = 0;

	msa_register_read(MSA_REG_WHO_AM_I,&msa_id);
	if (msa_id != 0x13) {
		bsp_sensor_init_sta_clr(SENSOR_INIT_STEP);
		printf("Msa_id err\n");
		return 1;
	}

	printf("Msa_id :0x%x\n", msa_id);
	res =  msa_register_mask_write(MSA_REG_SPI_I2C, 0x24, 0x24);

	delay_ms(10);

	res |= msa_register_mask_write(MSA_REG_G_RANGE, 0x03, 0x02);//lkk modify  0x10
	res |= msa_register_mask_write(MSA_REG_POWERMODE_BW, 0xFF, 0x5e);//lkk modify 0x5E
	res |= msa_register_mask_write(MSA_REG_ODR_AXIS_DISABLE, 0xFF, 0x04);	//15.63HZ data_count=8~10

	res |= msa_register_mask_write(MSA_REG_FIFO_CTRL, 0xFF, 0xbf);

	msa_param_init();

	bsp_sensor_init_sta_set(SENSOR_INIT_STEP);

    static co_timer_t msa310_500ms;
    co_timer_set(&msa310_500ms, 500, TIMER_REPEAT, LEVEL_LOW_PRI, msa310_process_500ms_callback, NULL);
    co_timer_set_sleep(&msa310_500ms, true);

	return 0;
}

uint8_t msa310_stop(void)
{
    int32_t res = 0;

    res |= msa_register_mask_write(MSA_REG_POWERMODE_BW, 0xFF, 0xDE); // lkk modify 0x5E
    res |= msa_register_mask_write(MSA_REG_FIFO_CTRL, 0xFF, 0x00);
	bsp_sensor_init_sta_clr(SENSOR_INIT_STEP);
    return 0;
}

AT(.text.lowpwr.sleep)
/**********************************************
 * @brief 		步数
 * @param x 	原始三轴x值
 * @param x 	原始三轴y值
 * @param x 	原始三轴z值
 * @return 		当前步数值
 *********************************************/
uint msa310_step(s16 x,s16 y,s16 z)
{
	return msa_step(x, y, z);
}

AT(.text.lowpwr.sleep)
/**********************************************
 * @brief 		抬腕亮屏
 * @param x 	原始三轴x值
 * @param x 	原始三轴y值
 * @param x 	原始三轴z值
 * @return 		0:NULL
 * 				1:on
 * 				2:off
 *********************************************/
uint8_t msa310_light(s16 x,s16 y,s16 z)
{
	//取到状态后就不用计算抬手亮屏了
	if ((SL_LIGHT == 1) || (SL_LIGHT == 2)) {

	} else {
		SL_LIGHT = msa_light(x, -y, z);
	}

	if (SL_LIGHT == 1) {		//亮
		// printf("LIGHT ON\n");
	} else if (SL_LIGHT == 2) {	//灭
		// printf("LIGHT OFF\n");
	}

	if (data_cnt == (data_count - 1)) {
		SL_LIGHT = 0;
		// printf("LIGHT CLR\n");
	}

	return SL_LIGHT;
}

AT(.text.lowpwr.sleep)
/**********************************************
 * @brief 		摇一摇拍照
 * @param x 	原始三轴x值
 * @param x 	原始三轴y值
 * @param x 	原始三轴z值
 * @return 		1:摇晃
 * 	 			0:静止
 *********************************************/
#define SHAKE_THRESHOLD_X		600
#define SHAKE_THRESHOLD_Y		600
#define SHAKE_THRESHOLD_Z		600

bool msa310_shake(short x,short y,short z)
{
#if 1
    static u8 cnt = 0;
    static AxesRaw_t xyz_axis[4];
    static short max_x,max_y,max_z;
    static short min_x,min_y,min_z;

    xyz_axis[cnt].AXIS_X = x;
    xyz_axis[cnt].AXIS_Y= y;
    xyz_axis[cnt].AXIS_Z= z;

    cnt++;
    if (cnt == 4) {
		cnt = 0;
		max_x = min_x = xyz_axis[cnt].AXIS_X;
		max_y = min_y = xyz_axis[cnt].AXIS_Y;
		max_z = min_z = xyz_axis[cnt].AXIS_Z;
		for (u8 i = 0;i < 4;i++) {
			if (xyz_axis[i].AXIS_X > max_x) {
				max_x = xyz_axis[i].AXIS_X;
			}

			if (xyz_axis[i].AXIS_X < min_x) {
				min_x = xyz_axis[i].AXIS_X;
			}

			if (xyz_axis[i].AXIS_Y > max_y) {
				max_y = xyz_axis[i].AXIS_Y;
			}

			if (xyz_axis[i].AXIS_Y < min_y) {
				min_y = xyz_axis[i].AXIS_Y;
			}

			if (xyz_axis[i].AXIS_Z > max_z) {
				max_z = xyz_axis[i].AXIS_Z;
			}

			if (xyz_axis[i].AXIS_Z < min_z) {
				min_z = xyz_axis[i].AXIS_Z;
			}
		}
		// printf("max_x:%d,min_x:%d\r\n",max_x,min_x);
		// printf("max_y:%d,min_y:%d\r\n",max_y,min_y);
		// printf("max_z:%d,min_z:%d\r\n",max_z,min_z);

		if (((max_x - min_x) > SHAKE_THRESHOLD_X) || ((max_y - min_y) > SHAKE_THRESHOLD_Y) || ((max_z - min_z) > SHAKE_THRESHOLD_Z)) {
			return true;
		} else {
			return false;
		}
	}
		return false;
#endif
}


static uint32_t times = 0;
static uint32_t awake_time_sec = 0;
static uint32_t light_time_sec = 0;
static uint32_t deep_time_sec  = 0;
static uint32_t getup_time_sec = 0;
AT(.text.lowpwr.sleep)
/**********************************************
 * @brief 		睡眠数据
 * @param x 	原始三轴x值
 * @param x 	原始三轴y值
 * @param x 	原始三轴z值
 * @return 		NULL
 *********************************************/
void msa_sleep_detect(short x,short y,short z)
{
	//2S采样一次数据
	if (++times % 4 == 0) {
		int sleep_status = msa_sleep(x,y,z);
		// printf("sleep_status : %d\n", sleep_status);
		if(sleep_status == MSA_RESTLESS) {
			SL_SLEEP = SLEEP_STATUS_AWAKE;
		} else if(sleep_status == MSA_ASLEEP) {
			SL_SLEEP = SLEEP_STATUS_LIGHT;
		} else if(sleep_status == MSA_ONTABLE) {
			SL_SLEEP = SLEEP_STATUS_DEEP;
		} else if(sleep_status == MSA_AWAKE) {
			SL_SLEEP = SLEEP_STATUS_GETUP;
		}
		// printf("SL_SLEEP : %d\n", SL_SLEEP);

		if (SL_SLEEP == SLEEP_STATUS_AWAKE) {
			awake_time_sec += 2;
		} else if (SL_SLEEP == SLEEP_STATUS_LIGHT) {
			light_time_sec += 2;
		} else if (SL_SLEEP == SLEEP_STATUS_DEEP) {
			deep_time_sec  += 2;
		} else if (SL_SLEEP == SLEEP_STATUS_GETUP) {
			getup_time_sec += 2;
		}
		// printf("sleep check ->awake[%d] light[%d] deep[%d] getup[%d] Sec\n", awake_time_sec, light_time_sec, deep_time_sec, getup_time_sec);
	}

	if (awake_time_sec % 60 == 0 && awake_time_sec != 0) {
		awake_time_sec = 0;
		// sys_data.user_data.sector0.sleep.awake_min++;
	}
	if (light_time_sec % 60 == 0 && light_time_sec != 0) {
		light_time_sec = 0;
		// sys_data.user_data.sector0.sleep.light_min++;
	}
	if (deep_time_sec % 60 == 0 && deep_time_sec != 0) {
		deep_time_sec = 0;
		// sys_data.user_data.sector0.sleep.deep_min++;
	}
	if (getup_time_sec % 60 == 0 && getup_time_sec != 0) {
		getup_time_sec = 0;
		// sys_data.user_data.sector0.sleep.getup_time++;
	}

	if (times % 120 == 0) {
		times = 0;
		// printf("sleep check ->awake[%d] light[%d] deep[%d] getup[%d] Min\n", sys_data.user_data.sector0.sleep.awake_min,
		// 	sys_data.user_data.sector0.sleep.light_min, sys_data.user_data.sector0.sleep.deep_min, sys_data.user_data.sector0.sleep.getup_time);
	}
}

AT(.text.lowpwr.sleep)
/**********************************************
 * @brief 		MSA310 process
 * 				500ms polling
 * @return 		NULL
 *********************************************/
void MSA310_GetFiFOAccAxesRaw(void)
{
	uint8_t temp_buf[72] = {0};			//change odr after must change temp_buf size, cur odr : 15.63HZ
	short x = 0,y = 0,z = 0;
	data_count = 0;
	unsigned char acc_count = 0;

	msa_register_read(0x0d,&data_count);
	data_count = (data_count & 0x7F);//+1;
	if (data_count > 32) {
		data_count = 32;
	}

	acc_count = data_count;
	// printf("data_count = %d \r\n",data_count);
	msa_ConReadBytes(temp_buf,0xff,acc_count*6);

	for (data_cnt = 0; data_cnt < data_count; data_cnt++) {
		y = ((short)(temp_buf[data_cnt*6+1] << 8 | temp_buf[data_cnt*6+0]))>> 4;
		x = ((short)(temp_buf[data_cnt*6+3] << 8 | temp_buf[data_cnt*6+2]))>> 4;
		z = ((short)(temp_buf[data_cnt*6+5] << 8 | temp_buf[data_cnt*6+4]))>> 4;
		// printf("x->%d, y->%d, z->%d, data->%d\n",x,y,z, data_count-data_cnt);

		//计步
		SL_STEP = msa310_step(x,y,z);

		//抬手亮屏
		msa310_light(x, y, z);

		//摇一摇拍照
		SL_SWAY_STATUS = msa310_shake(x,y,z);

		//睡眠
		if (data_cnt == 0) {
			msa_sleep_detect(x,y,z);
		}
	}
	printf("SL_STEP: %d, SL_SWAY: %d\n", SL_STEP, SL_SWAY_STATUS);

}

bool  is_msa310_chipid(void)
{
	if(msa_id==0x13)
		return true;
	else
		return false;
}

bool msa310_factory_check(void)
{
	u8 msa310_d =0;
	msa_register_read(MSA_REG_WHO_AM_I,&msa310_d);
	if(msa310_d==0x13)
		return true;
	else
		return false;
}

AT(.text.lowpwr.sleep)
void msa310_process_500ms_callback(co_timer_t *timer, void *param)
{
    if (!bsp_sensor_init_sta_get(SENSOR_INIT_STEP)) {
        return;
    }

	MSA310_GetFiFOAccAxesRaw();
}

#endif //(SENSOR_STEP_SEL == SENSOR_STEP_MSA310)
