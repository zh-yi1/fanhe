#include "include.h"

#if 1


#ifndef DEFAULE_I2C_ADDR
#define DEFAULE_I2C_ADDR	(0x2a)
#endif

// ch0
#define PROXTH_CH0_CLS_REAL_VAL		500000
#define PROXTH_CH0_FAR_REAL_VAL		500000

// ch1
#define PROXTH_CH1_CLS_REAL_VAL		500000
#define PROXTH_CH1_FAR_REAL_VAL		500000

// ch2
#define PROXTH_CH2_CLS_REAL_VAL		120000
#define PROXTH_CH2_FAR_REAL_VAL		120000

// ch3
#define PROXTH_CH3_CLS_REAL_VAL		120000
#define PROXTH_CH3_FAR_REAL_VAL		120000

// 通道0
#define PROXTH_CH0_CLS			(PROXTH_CH0_CLS_REAL_VAL >> 8)
#define PROXTH_CH0_FAR			(PROXTH_CH0_FAR_REAL_VAL >> 8)
// 通道1
#define PROXTH_CH1_CLS			(PROXTH_CH1_CLS_REAL_VAL >> 8)
#define PROXTH_CH1_FAR			(PROXTH_CH1_FAR_REAL_VAL >> 8)
// 通道2
#define PROXTH_CH2_CLS			(PROXTH_CH2_CLS_REAL_VAL >> 8)
#define PROXTH_CH2_FAR			(PROXTH_CH2_FAR_REAL_VAL >> 8)
// 通道3
#define PROXTH_CH3_CLS			(PROXTH_CH3_CLS_REAL_VAL >> 8)
#define PROXTH_CH3_FAR			(PROXTH_CH3_FAR_REAL_VAL >> 8)

//最大和最小滑动时间
#define SLIDE_TIME_MIN                     5//ms
#define SLIDE_TIME_MAX                     500//ms

#define RM1X01_PROXTH_CH0_CLS		0x49
#define RM1X01_PROXTH_CH0_FAR		0x51

#define RM1X01_RAW0_CH0					0X75
#define RM1X01_USE0_CH0					0X81
#define RM1X01_AVG0_CH0					0X8D
#define RM1X01_DIF0_CH0					0X99

#define s_abs(a)        ((a > 0) ? a : -a)

typedef enum{
    RAW_DATA,
    USE_DATA,
    AVG_DATA,
    DIF_DATA
} RM1X01_DataType;

struct RM1X01_REG_CFG {
	uint8_t reg_address;
	uint8_t reg_value;
};


typedef enum{
	MUX_CHANNEL0 = 0X01,
	MUX_CHANNEL1 = 0X02,
	MUX_CHANNEL2 = 0X04,
	MUX_CHANNEL3 = 0X08
}RmMuxChannel;

//static uint8_t earin_flag = 0;
//static uint8_t slide_flag = 0;
//static uint8_t press_flag = 0xff;
//static int32_t slide_lock_cnt = 0;
static int32_t key_scan_timer = 0;

//static void clear_key_state(void);
//static void touch_key_scan(void);

#if 1
struct RM1X01_REG_CFG rm1015B_defaultcfg[] = {
    { 0x05, 0xf6 },
	{ 0x07, 0x0b },

	{ 0x0c, 0x40 },
	{ 0x0d, 0x40 },
	{ 0x0e, 0x40 },
	{ 0x0f, 0x40 },
	//{ 0x14, 0x83 },

	{ 0x16, 0x03 },
	{ 0x17, 0x00 },
	{ 0x18, 0x27 },
	{ 0x19, 0x03 },
	{ 0x1a, 0x03 },

	{ 0x1b, 0x00 },
	{ 0x1c, 0x00 },
//	{ 0x1c, 0x38 },
//单端
	// { 0x1d, 0x02 },
	// { 0x1e, 0x08 },
	// { 0x1f, 0x20 },
	// { 0x20, 0x80 },
//差分
	{ 0x1d, 0x06 },
    { 0x1e, 0x60 },
    { 0x1f, 0x00 },
    { 0x20, 0x00 },

	{ 0x21, 0x06 },
	{ 0x22, 0x06 },
	{ 0x23, 0x06 },
	{ 0x24, 0x06 },

	{ 0x25, 0x00 },
	{ 0x26, 0x01 },
	{ 0x27, 0x01 },
	{ 0x28, 0x01 },
	{ 0x29, 0x01 },

	{ 0x2a, 0x80 },
	// { 0x2b, 0x00 },
	// { 0x2c, 0x00 },
	{ 0x2b, 0x81 },
	{ 0x2c, 0x81 },
	{ 0x2d, 0x81 },
	{ 0x2e, 0x81 },

	{ 0x2f, 0x0f },
	//{ 0x30, 0x41 },
	{ 0x30, 0x41 },

	{ 0x39, 0x2a },
	{ 0x3a, 0x2a },
	{ 0x3b, 0x2a },
	{ 0x3c, 0x2a },

	{ 0x49, 0xff & PROXTH_CH0_CLS },
	{ 0x4a, 0xff & (PROXTH_CH0_CLS >> 8)},
	{ 0x4b, 0xff & PROXTH_CH1_CLS },
	{ 0x4c, 0xff & (PROXTH_CH1_CLS >> 8)},
	{ 0x4d, 0xff & PROXTH_CH2_CLS },
	{ 0x4e, 0xff & (PROXTH_CH2_CLS >> 8) },
	{ 0x4f, 0xff & PROXTH_CH3_CLS },
	{ 0x50, 0xff & (PROXTH_CH3_CLS >> 8) },

	{ 0x51, 0xff & PROXTH_CH0_FAR },
	{ 0x52, 0xff & (PROXTH_CH0_FAR >> 8) },
	{ 0x53, 0xff & PROXTH_CH1_FAR },
	{ 0x54, 0xff & (PROXTH_CH1_FAR >> 8) },
	{ 0x55, 0xff & PROXTH_CH2_FAR },
	{ 0x56, 0xff & (PROXTH_CH2_FAR >> 8) },
	{ 0x57, 0xff & PROXTH_CH3_FAR },
	{ 0x58, 0xff & (PROXTH_CH3_FAR >> 8) },

//	{ 0x65, 0x00 },
//	{ 0x66, 0x10 },

	{ 0x65, 0xff },
	{ 0x66, 0x00 },
	{ 0x67, 0x07 },
	{ 0x68, 0x01 },
};

#else


#endif


AT(.com_text.ctp)
bool rm1015B_reg_write(uint8_t reg_start_addr, uint8_t *reg_array, uint8_t num)
{
    bool ectp_iic_write_regs(u16 dev_addr, u32 reg_addr, u8 *wbuf, u16 wlen, bool stop);
    if (!ectp_iic_write_regs(DEFAULE_I2C_ADDR, reg_start_addr, reg_array, num, true)) {
        return false;
    }
    return true;
}

AT(.com_text.ctp)
bool rm1015B_reg_read(uint8_t reg_start_addr, uint8_t *reg_array, uint8_t num)
{
    bool ectp_iic_read_regs(u16 dev_addr, u32 reg_addr, u8 *wbuf, u16 wlen, u8 *rbuf, u16 rlen);

    if (!ectp_iic_read_regs(DEFAULE_I2C_ADDR, reg_start_addr, NULL, 0, reg_array, num)) {
        return false;
    }
    return true;
}

AT(.com_text.ctp)
static int32_t sign24To32(uint32_t bit24)
{
	if ((bit24 & 0x800000) == 0x800000) {
		bit24 |= 0xff000000;
	}

	return bit24;
}

AT(.com_text.ctp)
uint32_t rm1015B_adc_data_get(int dataType, int channel)
{
	uint8_t rx_buf[3] = {0};
	uint8_t reg_base_addr = 0;
	uint32_t ret_data = 0;

	switch (dataType)
	{
		case RAW_DATA:
			reg_base_addr = channel * 3 + RM1X01_RAW0_CH0;
			break;
		case USE_DATA:
			reg_base_addr = channel * 3 + RM1X01_USE0_CH0;
			break;
		case AVG_DATA:
			reg_base_addr = channel * 3 + RM1X01_AVG0_CH0;
			break;
		case DIF_DATA:
			reg_base_addr = channel * 3 + RM1X01_DIF0_CH0;
			break;
		default:
			break;
	}

//	uint32_t axirq = csi_irq_save();  // 屏蔽中断
    GLOBAL_INT_DISABLE();
	if(rm1015B_reg_read(reg_base_addr, rx_buf, 3) == true)
	{
		ret_data = rx_buf[0] | (rx_buf[1] << 8) | (rx_buf[2] << 16);
	}
    GLOBAL_INT_RESTORE();
//	csi_irq_restore(axirq); // 开启中断

	return ret_data;
}

AT(.com_text.ctp.rm1015B_setconfig_print)
const char rm1015B_setconfig_print[] = "reg[0x%x]: 0x%x != [0x%x]\n";


AT(.com_text.ctp)
void rm1015B_setconfig(void)
{
	int i = 0;
	uint8_t tx_buf[4] = {0};

	for (i = 0; i < sizeof(rm1015B_defaultcfg) / sizeof(rm1015B_defaultcfg[0]); i++) {
		tx_buf[0] = rm1015B_defaultcfg[i].reg_value;
		rm1015B_reg_write(rm1015B_defaultcfg[i].reg_address, tx_buf, 1);
	}

	for (i = 0; i <= 0x68; i++) {
		if (rm1015B_reg_read(i, tx_buf, 1) == false) {
			printf("read reg error\n");
		}
	}

    for (i = 0; i < sizeof(rm1015B_defaultcfg) / sizeof(rm1015B_defaultcfg[0]); i++) {
		if (rm1015B_reg_read(rm1015B_defaultcfg[i].reg_address, tx_buf, 1)) {
            if (tx_buf[0] != rm1015B_defaultcfg[i].reg_value) {
                printf(rm1015B_setconfig_print, rm1015B_defaultcfg[i].reg_address, rm1015B_defaultcfg[i].reg_value, tx_buf[0]);
            }
		}
	}
}

AT(.com_text.ctp)
void rm1015B_set_proxth(uint32_t channel,int32_t proxth_cls, int32_t proxth_far)
{
	uint8_t tx_buf[4] = {0};
	uint8_t reg_base_addr = 0;

//	printf("%s %d: %d %d", __func__, __LINE__, proxth_cls, proxth_far);

	reg_base_addr = channel * 2 + RM1X01_PROXTH_CH0_CLS;
	tx_buf[0] = proxth_cls >> 8;
	tx_buf[1] = proxth_cls >> 16;
	if(rm1015B_reg_write(reg_base_addr, tx_buf, 2) == false)
	{
//		printf("%s %d", __func__, __LINE__);
	}

	reg_base_addr = channel * 2 + RM1X01_PROXTH_CH0_FAR;
	tx_buf[0] = proxth_far >> 8;
	tx_buf[1] = proxth_far >> 16;
	if(rm1015B_reg_write(reg_base_addr, tx_buf, 2) == false)
	{
//		printf("%s %d", __func__, __LINE__);
	}
}

AT(.com_text.ctp.rm1015B_intr_handler)
const char rm1015B_intr_handler_print[] = "sta:[%d,%d]\n";

AT(.com_text.ctp)
void rm1015B_intr_handler(void)
{
    uint8_t streg = 0;
    uint8_t sta0,sta1;//sta2,sta3;
//    static uint8_t state_ch0,state_ch1,state_ch2;
//    static int32_t slideA_tick1,slideB_tick1,slideC_tick1;

        rm1015B_reg_read(0x74, &streg, 1);
        sta0 = !!(streg & 0x01);
        sta1 = !!(streg & 0x02);
        //sta2 = !!(streg & 0x04);
		//sta3 = !!(streg & 0x08);
//		earin_flag = !!(streg & 0x08);


        printf(rm1015B_intr_handler_print, sta0, sta1);
		if (sta0) {
			msg_enqueue(MSG_CTP_LEFT_INEAR);
		} else {
            msg_enqueue(MSG_CTP_LEFT_OUTEAR);
		}
		if (sta1) {
            msg_enqueue(MSG_CTP_RIGHT_INEAR);
		} else {
            msg_enqueue(MSG_CTP_RIGHT_OUTEAR);
		}


//	printf("streg:%d\r\n",streg);

}

void rm1015B_init(void)
{
    printf("%s\n", __func__);
//	int i = 0;
//	uint8_t coff[2] = {0};
//	uint8_t rxbuf[4] = {0};
	uint8_t wdata = 0xcc;

	rm1015B_reg_write(0x6f, &wdata, 1);
	delay_ms(1);
	wdata = 0x05;
	rm1015B_reg_write(0x17, &wdata, 1);

	delay_ms(70);
	rm1015B_setconfig();

	wdata = 0x3f;
	rm1015B_reg_write(0x2f, &wdata, 1);

	delay_ms(100);

	wdata = 0x0f;
	rm1015B_reg_write(0x17, &wdata, 1);
}

AT(.com_text.ctp)
bool rm1015B_enterlowpower(void)
{

	uint8_t reg_addr = 0x17;
	uint8_t wdata = 0x17;
	if (rm1015B_reg_write(reg_addr, &wdata, 1) != true)
	{
		return false;
	}

	 reg_addr = 0x16;
	 wdata = 0x00;
//	puts("-------------->>rm1101_enterlowpower\n");
	if (rm1015B_reg_write(reg_addr, &wdata, 1) != true)
	{
		return false;
	}


	return true;
}

AT(.com_text.ctp)
bool rm1015B_wakeup(void)
{
	uint8_t reg_addr = 0x17;
	uint8_t wdata = 0x0f;
	if (rm1015B_reg_write(reg_addr, &wdata, 1) != true)
	{
		return false;
	}

	 reg_addr = 0x16;
	 wdata = 0x03;
	if (rm1015B_reg_write(reg_addr, &wdata, 1) != true)
	{
		return false;
	}



	return true;
}



AT(.com_text.ctp)
void rm1015B_irq_handler(void)
{
	key_scan_timer = 1;
//	uart_printf("%s %d\n", __func__, __LINE__);
	rm1015B_intr_handler();
//	rm1015B_debug_func();
}

AT(.com_text.ctp)
void rm1015B_timer_handler(uint32_t ms)
{
//	if (key_scan_timer)
		//touch_key_scan();
}

/*****************************************************************************************************/
// 以下是调试代码
/*****************************************************************************************************/

void rm1015B_debug_func(void)
{
	int32_t rawdata[4] = {0};
	int32_t difdata[4] = {0};
    u8 Channel = 0;
    u8 streg[4] = {0};

	 while(1)
	 {
        WDT_CLR();
		delay_ms(50);
		rawdata[0] = sign24To32(rm1015B_adc_data_get(RAW_DATA, 0));
		rawdata[1] = sign24To32(rm1015B_adc_data_get(RAW_DATA, 1));
		rawdata[2] = sign24To32(rm1015B_adc_data_get(RAW_DATA, 2));
		rawdata[3] = sign24To32(rm1015B_adc_data_get(RAW_DATA, 3));
//
		difdata[0] = sign24To32(rm1015B_adc_data_get(DIF_DATA, 0));
		difdata[1] = sign24To32(rm1015B_adc_data_get(DIF_DATA, 1));
		difdata[2] = sign24To32(rm1015B_adc_data_get(DIF_DATA, 2));
		difdata[3] = sign24To32(rm1015B_adc_data_get(DIF_DATA, 3));

		rm1015B_reg_read(0x16, &Channel, 1);
		rm1015B_reg_read(0x41, streg, 4);

//		uart_printf("%d,%d,%d,%d,\r\n", stdata[0], stdata[1], stdata[2], stdata[3]);
		printf("%d,%d,%d,%d,%d,%d,%d,%d, channel:%d, streg[%d,%d,%d,%d]\r\n",
			rawdata[0], rawdata[1], rawdata[2], rawdata[3], difdata[0], difdata[1], difdata[2], difdata[3],
			Channel, streg[0], streg[1], streg[2], streg[3]);//		uart_printf("%d,%d,%d,%d,%d,%d,\r\n",
//			stdata[0]*PROXTH_CH0_CLS_REAL_VAL, stdata[1]*PROXTH_CH1_CLS_REAL_VAL, stdata[2]*PROXTH_CH2_CLS_REAL_VAL,
//			difdata[0], difdata[1], difdata[2]);
	 }
}



/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/



#endif
