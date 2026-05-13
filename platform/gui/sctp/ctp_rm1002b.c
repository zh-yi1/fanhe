#include "include.h"

#if SCTP_SELECT == SCTP_RM1002B
#define TRACE_EN            1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#ifndef DEFAULE_I2C_ADDR
#define DEFAULE_I2C_ADDR	(0x2b)
#endif

// ch0
#define PROXTH_CH0_CLS_REAL_VAL		500000  // CH0 接近阈值
#define PROXTH_CH0_FAR_REAL_VAL		500000  // CH0 远离阈值

// ch1
#define PROXTH_CH1_CLS_REAL_VAL		500000  // CH1 接近阈值
#define PROXTH_CH1_FAR_REAL_VAL		500000  // CH1 远离阈值

// ch2
#define PROXTH_CH2_CLS_REAL_VAL		500000  // CH2 接近阈值
#define PROXTH_CH2_FAR_REAL_VAL		500000  // CH2 远离阈值

// ch3
#define PROXTH_CH3_CLS_REAL_VAL		200000  // CH3 接近阈值
#define PROXTH_CH3_FAR_REAL_VAL		200000  // CH3 远离阈值

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
#define SLIDE_TIME_MIN                  5//ms
#define SLIDE_TIME_MAX                  1000//300//ms

#define RM1X01_PROXTH_CH0_CLS			0x49
#define RM1X01_PROXTH_CH0_FAR			0x51

#define RM1X01_RAW0_CH0					0X75
#define RM1X01_USE0_CH0					0X81
#define RM1X01_AVG0_CH0					0X8D
#define RM1X01_DIF0_CH0					0X99

#ifndef NO_KEY
#define NO_KEY 		0xff
#endif

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

static int32_t int_msec1 = 0;
static int32_t int_msec2 = 0;
//static int32_t int_msec3 = 0;
//static uint8_t earin_flag = 0;
static uint8_t slide_state = 0;
static uint8_t slide_flag = 0;
static uint8_t press_flag = NO_KEY;
static int32_t slide_lock_cnt = 0;
static int32_t key_scan_timer = 0;
static uint8_t state_ch3,state_ch1,state_ch2;
static uint8_t slideA_flag, slideB_flag, slideC_flag;
static int32_t slideA_tick, slideB_tick, slideC_tick;

static void clear_key_state(void);
static void touch_key_scan(void);

#if 1
struct RM1X01_REG_CFG rm1002_defaultcfg[] = {
    { 0x05, 0xf6 },
//	{ 0x07, 0x0b },

	{ 0x0c, 0x40 },
	{ 0x0d, 0x40 },
	{ 0x0e, 0x40 },
	{ 0x0f, 0x40 },
	//{ 0x14, 0x83 },

	{ 0x16, 0x0f },
	{ 0x17, 0x00 },
	{ 0x19, 0x02 },
	{ 0x1a, 0x02 },

	{ 0x1b, 0x00 },
	{ 0x1c, 0x00 },
//	{ 0x1c, 0x38 },

	{ 0x1d, 0x02 },
	{ 0x1e, 0x08 },
	{ 0x1f, 0x20 },
	{ 0x20, 0x80 },

	{ 0x21, 0x03 },
	{ 0x22, 0x03 },
	{ 0x23, 0x03 },
	{ 0x24, 0x03 },

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
bool sctp_rm1002b_reg_write(u8 reg_start_addr, u8 *reg_array, u8 num)
{
    bool sctp_iic_write_regs(u16 dev_addr, u32 reg_addr, u8 *wbuf, u16 wlen, bool stop);
    if (!sctp_iic_write_regs(DEFAULE_I2C_ADDR, reg_start_addr, reg_array, num, true)) {
        return false;
    }
    return true;
}

AT(.com_text.ctp)
bool sctp_rm1002b_reg_read(u8 reg_start_addr, u8 *reg_array, u8 num)
{
//    bool sctp_iic_read_regs2(u8 dev_addr, void *buf, u16 len, u8 clk_div);
//    bool sctp_iic_read_regs(u8 dev_addr, void *buf, u16 len, u8 clk_div);
//    bool sctp_iic_read_write_regs(u8 dev_addr, void *rbuf, int rlen, u16 w_addr, u8 *w_cmd, int wlen, u8 clk_div);
//    if (!sctp_iic_read_write_regs(DEFAULE_I2C_ADDR, reg_array, num, reg_start_addr, NULL, 0, 8)) {
//        return false;
//    }
//    return true;
bool sctp_iic_read_regs(u16 dev_addr, u32 reg_addr, u8 *wbuf, u16 wlen, u8 *rbuf, u16 rlen);

    if (!sctp_iic_read_regs(DEFAULE_I2C_ADDR, reg_start_addr, NULL, 0, reg_array, num)) {
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
uint32_t rm1002_adc_data_get(int dataType, int channel)
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

	if (reg_base_addr == 0) {
        return 0;
	}
//	uint32_t axirq = csi_irq_save();  // 屏蔽中断
    GLOBAL_INT_DISABLE();
	if(sctp_rm1002b_reg_read(reg_base_addr, rx_buf, 3) == true)
	{
		ret_data = rx_buf[0] | (rx_buf[1] << 8) | (rx_buf[2] << 16);
	}
	GLOBAL_INT_RESTORE();
//	csi_irq_restore(axirq); // 开启中断

	return ret_data;
}

AT(.com_text.ctp.sctp_rm1002b_set_config_print)
const char sctp_rm1002b_set_config_print[] = "reg[0x%x]: 0x%x != [0x%x]\n";

AT(.com_text.ctp)
void sctp_rm1002b_set_config(void)
{
	int i = 0;
	uint8_t tx_buf[4] = {0};

	for (i = 0; i < sizeof(rm1002_defaultcfg) / sizeof(rm1002_defaultcfg[0]); i++) {
		tx_buf[0] = rm1002_defaultcfg[i].reg_value;
		sctp_rm1002b_reg_write(rm1002_defaultcfg[i].reg_address, tx_buf, 1);
	}

	for (i = 0; i <= 0x68; i++) {
		if (sctp_rm1002b_reg_read(i, tx_buf, 1) == false) {
			printf("read reg error\n");
		}
	}

	for (i = 0; i < sizeof(rm1002_defaultcfg) / sizeof(rm1002_defaultcfg[0]); i++) {
		if (sctp_rm1002b_reg_read(rm1002_defaultcfg[i].reg_address, tx_buf, 1)) {
            if (tx_buf[0] != rm1002_defaultcfg[i].reg_value) {
                printf(sctp_rm1002b_set_config_print, rm1002_defaultcfg[i].reg_address, rm1002_defaultcfg[i].reg_value, tx_buf[0]);
            }
		}
	}



}

AT(.com_text.ctp.sctp_rm1002b_int_handler_print)
const char sctp_rm1002b_int_handler_print[] = "[RMX]---->>>>: rm1002b: %d %d %d %d -- %d\r\n";

AT(.com_text.ctp)
void sctp_rm1002b_int_handler(void)
{
    uint8_t streg = 0;
    uint8_t sta0,sta1,sta2,sta3;

    {
        sctp_rm1002b_reg_read(0x74, &streg, 1);
        sta0 = !!(streg & 0x01);
        sta1 = !!(streg & 0x02);
        sta2 = !!(streg & 0x04);
		sta3 = !!(streg & 0x08);
		slide_state =  sta1 | sta2 | sta3;

//        printf(sctp_rm1002b_int_handler_print, sta3, sta2, sta1, sta0, slide_flag);
//		sta3 = !!(streg & 0x08);
        if (sta0) {  // 按键按下
            press_flag = 1;
        } else { // 按键抬起
            slide_flag = 0;
            press_flag = NO_KEY;
        }

		if (state_ch1 != sta1) {  // B
			if(sta1 == 1) {
				slideA_tick = tick_get();
				slideA_flag = 1;
			}
			state_ch1 = sta1;
		}

		if (state_ch2 != sta2) {  // C
			if (sta2 == 1) {
				slideB_tick = tick_get();
				slideB_flag = 1;
			}
			state_ch2 = sta2;
		}


		if (state_ch3 != sta3) {  // A
			if (sta3 == 1) {
				slideC_tick = tick_get();
				slideC_flag = 1;
			}
			state_ch3 = sta3;
		}
    }
}
AT(.com_text.ctp)
int check_ab(void)
{
	if (slideA_tick > slideB_tick) {
		int_msec1 = slideA_tick - slideB_tick;
//		printf("[RMX]>>>>>>>>>>>>>> a-b:%d\r\n", int_msec1);
		if ((int_msec1 > SLIDE_TIME_MIN) && (int_msec1 < SLIDE_TIME_MAX)) {
			slide_flag = 0x14;
//			printf("[RMX]>>>>>>>>>>>>>> B->A\r\n");
		}
	} else if (slideB_tick > slideA_tick) {
		int_msec1 = slideB_tick - slideA_tick;
//		printf("[RMX]>>>>>>>>>>>>>> b-a:%d\r\n", int_msec1);
		if ((int_msec1 > SLIDE_TIME_MIN) && (int_msec1 < SLIDE_TIME_MAX)) {
			slide_flag = 0x24;
//			printf("[RMX]>>>>>>>>>>>>>> A->B\r\n");
		}
	} else {
		slide_flag = 0;
	}

//bc_end:
	slideA_tick = 0;
	slideB_tick = 0;
	slideA_flag = 0;
	slideB_flag = 0;

	return slide_flag;
}
AT(.com_text.ctp)
int check_bc(void)
{
	 if (slideB_tick > slideC_tick) {
		int_msec1 = slideB_tick - slideC_tick;
//		printf("[RMX]>>>>>>>>>>>>>> b-c:%d\r\n", int_msec1);
		if ((int_msec1 > SLIDE_TIME_MIN) && (int_msec1 < SLIDE_TIME_MAX)) {
			slide_flag = 0x15;
//			printf("[RMX]>>>>>>>>>>>>>> C->B\r\n");
		}
	}else if (slideC_tick > slideB_tick) {
		int_msec1 = slideC_tick - slideB_tick;
//		printf("[RMX]>>>>>>>>>>>>>> c-b:%d\r\n", int_msec1);
		if ((int_msec1 > SLIDE_TIME_MIN) && (int_msec1 < SLIDE_TIME_MAX)) {
			slide_flag = 0x25;
//			printf("[RMX]>>>>>>>>>>>>>> B->C\r\n");
		}
	}  else {
		slide_flag = 0;
	}

//bc_end:
	slideB_tick = 0;
	slideC_tick = 0;
	slideB_flag = 0;
	slideC_flag = 0;

	return slide_flag;
}
AT(.com_text.ctp)
int check_abc(void)
{
	if ((slideA_tick > slideB_tick) && (slideB_tick > slideC_tick)) {
		int_msec1 = slideA_tick - slideB_tick;
		int_msec2 = slideB_tick - slideC_tick;
//		printf("[RMX]>>>>>>>>>>>>>>a-b:%d b-c:%d\r\n", int_msec1, int_msec2);
		if ((int_msec1 > SLIDE_TIME_MIN) && (int_msec1 < SLIDE_TIME_MAX) \
		   && (int_msec2 > SLIDE_TIME_MIN) && (int_msec2 < SLIDE_TIME_MAX)){
			if (abs_s(int_msec2 - int_msec1) < 150) {
				slide_flag = 0x12;
//				printf("[RMX]>>>>>>>>>>>>>> C->B->A\r\n");
				goto abc_end;
			}
		}
		if ((int_msec1 > SLIDE_TIME_MIN) && (int_msec1 < SLIDE_TIME_MAX)) {
			slide_flag = 0x14;
//			printf("[RMX]>>>>>>>>>>>>>> B->A\r\n");
			goto abc_end;
		}
		if ((int_msec2 > SLIDE_TIME_MIN) && (int_msec2 < SLIDE_TIME_MAX)) {
			slide_flag = 0x15;
//			printf("[RMX]>>>>>>>>>>>>>> C->B\r\n");
			goto abc_end;
		}
	} else if ((slideC_tick > slideB_tick) && (slideB_tick > slideA_tick)) {
		int_msec1 = slideC_tick - slideB_tick;
		int_msec2 = slideB_tick - slideA_tick;
//		printf("[RMX]>>>>>>>>>>>>>>c-b:%d b-a:%d\r\n", int_msec1, int_msec2);
		if ((int_msec1 > SLIDE_TIME_MIN) && (int_msec1 < SLIDE_TIME_MAX)\
		   && (int_msec2 > SLIDE_TIME_MIN) && (int_msec2 < SLIDE_TIME_MAX)){
			if (abs_s(int_msec2 - int_msec1) < 150) {
				slide_flag = 0x22;
//				printf("[RMX]>>>>>>>>>>>>>> A->B->C\r\n");
				goto abc_end;
			}
		}
		if ((int_msec2 > SLIDE_TIME_MIN) && (int_msec2 < SLIDE_TIME_MAX)) {
			slide_flag = 0x24;
//			printf("[RMX]>>>>>>>>>>>>>> A->B\r\n");
			goto abc_end;
		}
		if ((int_msec1 > SLIDE_TIME_MIN) && (int_msec1 < SLIDE_TIME_MAX)) {
			slide_flag = 0x25;
//			printf("[RMX]>>>>>>>>>>>>>> B->C\r\n");
			goto abc_end;
		}
	} else {
		slide_flag = 0;
	}

abc_end:
	slideA_tick = 0;
	slideB_tick = 0;
	slideC_tick = 0;
	slideA_flag = 0;
	slideB_flag = 0;
	slideC_flag = 0;

	return slide_flag;
}

AT(.com_text.ctp.slide_check_print)
const char slide_check_print[] = "[RMX] slide >> 0x%x\r\n";

AT(.com_text.ctp)
int slide_check(void)
{
	int ret = 0;
//	int32_t int_msec1 = 0;
//	int32_t int_msec2 = 0;

	if (slide_state) {
		return 0;
	}

	if (slideA_flag == 1 && slideB_flag == 1 && slideC_flag == 1) {
		slide_flag = check_abc();
	} else if (slideA_flag == 1 && slideB_flag == 1 && slideC_flag == 0) {
		slide_flag = check_ab();
	} else if (slideA_flag == 0 && slideB_flag == 1 && slideC_flag == 1) {
		slide_flag = check_bc();
	}
	// 高四位为识别方向（左滑为1，右滑为2）
	// 低四位为哪几个按键响应（三个按键滑为1，前两个按键滑为2，后两个按键滑为3）
	if (slide_flag) {
//		printf(slide_check_print, slide_flag);
		clear_key_state();
		press_flag = NO_KEY;
		ret = slide_flag;
		if (slide_flag == 0x12) {
            msg_enqueue(MSG_CTP_LONG_RIGHT);
		} else if (slide_flag == 0x14 || slide_flag == 0x15) {
            msg_enqueue(MSG_CTP_SHORT_RIGHT);
		} else if (slide_flag == 0x22) {
            msg_enqueue(MSG_CTP_LONG_LEFT);
		} else if (slide_flag == 0x24 || slide_flag == 0x25) {
            msg_enqueue(MSG_CTP_SHORT_LEFT);
		}
		slide_flag = 0;
	}

	slideA_tick = 0;
	slideB_tick = 0;
	slideC_tick = 0;
	slideA_flag = 0;
	slideB_flag = 0;
	slideC_flag = 0;

	return ret;
}

// 进入低功耗模式
AT(.com_text.ctp)
bool rm1002_enterlowpower(void)
{
	uint8_t reg_addr = 0x16;
	uint8_t wdata = 0x02;
//	printf("-------------->>rm1101_enterlowpower\n");
	if (sctp_rm1002b_reg_write(reg_addr, &wdata, 1) != true)
	{
		return false;
	}
	reg_addr = 0x05;
	wdata = 0xfa;
	if (sctp_rm1002b_reg_write(reg_addr, &wdata, 1) != true)
	{
		return false;
	}
	reg_addr = 0x17;
    wdata = 0x17;
	if (sctp_rm1002b_reg_write(reg_addr, &wdata, 1) != true)
	{
		return false;
	}
	return true;
}

// 唤醒
AT(.com_text.ctp)
bool rm1002_wakeup(void)
{
	uint8_t reg_addr = 0x16;
	uint8_t wdata = 0x07;
	if (sctp_rm1002b_reg_write(reg_addr, &wdata, 1) != true)
	{
		return false;
	}
	reg_addr = 0x05;
	wdata = 0xf6;
	if (sctp_rm1002b_reg_write(reg_addr, &wdata, 1) != true)
	{
		return false;
	}
	reg_addr = 0x17;
    wdata = 0x05;
	if (sctp_rm1002b_reg_write(reg_addr, &wdata, 1) != true)
	{
		return false;
	}

	return true;
}

// 获取按键按下抬起状态
AT(.com_text.ctp)
uint8_t get_touch_sta(void)
{
	if (slide_lock_cnt > 0) {
        return NO_KEY;
    }
    return press_flag;
}


// GPIO中断回调函数
AT(.com_text.ctp)
void rm1002_irq_handler(void)
{
//	printf("%s %d\n", __func__, __LINE__);
	sctp_rm1002b_int_handler();
//	rm1002_debug_func();
}

// 10ms定时器中断，用于按键识别
AT(.com_text.ctp)
void rm1002_timer_handler(void)
{
	if (key_scan_timer == 0) {
		return;
	}
	slide_check();
	touch_key_scan();
}


bool sctp_rm1002b_init(void)
{
    printf("sctp_rm1002b_init\n");
    uint8_t wdata = 0xcc;
    sctp_rm1002b_reg_write(0x6F, &wdata, 1);
    delay_ms(2);
    wdata = 0x05;
    sctp_rm1002b_reg_write(0x17, &wdata, 1);
    delay_ms(14);
    sctp_rm1002b_set_config();
    wdata = 0xff;
    sctp_rm1002b_reg_write(0x2F, &wdata, 1);
    delay_ms(400);

    wdata = 0x05;
    sctp_rm1002b_reg_write(0x17, &wdata, 1);

    key_scan_timer = 1;

    return true;
}

/*****************************************************************************************************/
// 以下是调试代码
/*****************************************************************************************************/

void rm1002_debug_func(void)
{
	int32_t rawdata[4] = {0};
	int32_t difdata[4] = {0};
//	int32_t avgdata[4] = {0};
//	uint8_t stdata[4] = {0};
    u8 Channel = 0;
    u8 streg[4] = {0};

	while(1) {
        WDT_CLR();
		delay_ms(100);
		rawdata[0] = sign24To32(rm1002_adc_data_get(RAW_DATA, 0));
		rawdata[1] = sign24To32(rm1002_adc_data_get(RAW_DATA, 1));
		rawdata[2] = sign24To32(rm1002_adc_data_get(RAW_DATA, 2));
		rawdata[3] = sign24To32(rm1002_adc_data_get(RAW_DATA, 3));
//
		difdata[0] = sign24To32(rm1002_adc_data_get(DIF_DATA, 0));
		difdata[1] = sign24To32(rm1002_adc_data_get(DIF_DATA, 1));
		difdata[2] = sign24To32(rm1002_adc_data_get(DIF_DATA, 2));
		difdata[3] = sign24To32(rm1002_adc_data_get(DIF_DATA, 3));

		sctp_rm1002b_reg_read(0x16, &Channel, 1);
		sctp_rm1002b_reg_read(0x41, streg, 4);

//		printf("%d,%d,%d,%d,\r\n", stdata[0], stdata[1], stdata[2], stdata[3]);
		printf("%d,%d,%d,%d,%d,%d,%d,%d, channel:%d, streg[%d,%d,%d,%d]\r\n",
			rawdata[0], rawdata[1], rawdata[2], rawdata[3], difdata[0], difdata[1], difdata[2], difdata[3],
			Channel, streg[0], streg[1], streg[2], streg[3]);
//		printf("%d,%d,%d,%d,%d,%d,\r\n",
//			stdata[0]*PROXTH_CH0_CLS_REAL_VAL, stdata[1]*PROXTH_CH1_CLS_REAL_VAL, stdata[2]*PROXTH_CH2_CLS_REAL_VAL,
//			difdata[0], difdata[1], difdata[2]);
	 }
}

/// 按键识别
enum {
    RMKEY_EVENT_NONE,
    RMKEY_EVENT_CLICK,
    RMKEY_EVENT_LONG,
    RMKEY_EVENT_LONG1,
    RMKEY_EVENT_LONG2,
    RMKEY_EVENT_LONG3,
    RMKEY_EVENT_HOLD,
    RMKEY_EVENT_UP,
    RMKEY_EVENT_DOUBLE_CLICK,
    RMKEY_EVENT_TRIPLE_CLICK,
    RMKEY_EVENT_FOURTH_CLICK,
    RMKEY_EVENT_FIRTH_CLICK,
    RMKEY_EVENT_USER,
    RMKEY_EVENT_MAX,
};

struct tkey_driver_para {
    const uint32_t scan_time;	//按键扫描频率, 单位ms
    uint8_t last_key;  			//上一次get_value按键值
//== 用于消抖类参数
    uint8_t filter_value; 		//用于按键消抖
    uint8_t filter_cnt;  		//用于按键消抖时的累加值
    const uint8_t filter_time;	//当filter_cnt累加到base_cnt值时, 消抖有效
//== 用于判定长按和HOLD事件参数
    const uint8_t long_time;  	//按键判定长按数量
    const uint8_t long_hold_time1;  	//按键判定长按数量
    const uint8_t long_hold_time2;  	//按键判定长按数量
    const uint8_t long_hold_time3;  	//按键判定长按数量
    const uint8_t hold_time;  	//按键判定HOLD数量
	uint32_t hold_cnt;  	    // HOLD数量
    uint8_t press_cnt;  		 	//与long_time和hold_time对比, 判断long_event和hold_event
//== 用于判定连击事件参数
    uint8_t click_cnt;  			//单击次数
    uint8_t click_delay_cnt;  	//按键被抬起后等待连击事件延时计数
    const uint8_t click_delay_time;	////按键被抬起后等待连击事件延时数量
    uint8_t notify_value;  		//在延时的待发送按键值
};


static volatile uint8_t is_key_active = 0;
//static uint8_t last_key = NO_KEY;

struct tkey_driver_para tkey_scan_para = {
    .scan_time 	  	  = 10,				//按键扫描频率, 单位: ms
    .last_key 		  = NO_KEY,  		//上一次get_value按键值, 初始化为NO_KEY;
    .filter_time  	  = 1,				//按键消抖延时;
    .long_time 		  = 90,  			//按键判定长按数量
    .hold_time 		  = (90 + 10),  	//按键判定HOLD数量
    .long_hold_time1  = 10,  			//按键判定长按数量
    .long_hold_time2  = 20,  			//按键判定长按数量
    .long_hold_time3  = 30,  			//按键判定长按数量
    .click_delay_time = 50,				//按键被抬起后等待连击延时数量
//    .key_type		  = KEY_DRIVER_TYPE_IO,
//    .get_value 		  = io_get_key_value,
};

AT(.com_text.ctp)
void clear_key_state(void)
{
    tkey_scan_para.filter_cnt = 0;
    tkey_scan_para.last_key = NO_KEY;
    tkey_scan_para.click_cnt = 0;
}

AT(.com_text.ctp.touch_key_scan_print)
const char touch_key_scan_print[] = "key_value: 0x%x, event: %d\n";

AT(.com_text.ctp)
void touch_key_scan(void)
{
	uint8_t key_event = 0;
    uint8_t cur_key_value = NO_KEY;
    uint8_t key_value = 0;

	cur_key_value = get_touch_sta();

	if (cur_key_value == 1) {
		sctp_rm1002b_int_handler();
	}
	cur_key_value = get_touch_sta();


	//===== 按键消抖处理
    if (cur_key_value != tkey_scan_para.filter_value && tkey_scan_para.filter_time) {	//当前按键值与上一次按键值如果不相等, 重新消抖处理, 注意filter_time != 0;
        tkey_scan_para.filter_cnt = 0; 		//消抖次数清0, 重新开始消抖
        tkey_scan_para.filter_value = cur_key_value;	//记录上一次的按键值
        return; 		//第一次检测, 返回不做处理
    } 		//当前按键值与上一次按键值相等, filter_cnt开始累加;
    if (tkey_scan_para.filter_cnt < tkey_scan_para.filter_time) {
        tkey_scan_para.filter_cnt++;
        return;
    }
	if (cur_key_value != tkey_scan_para.last_key) {
		if (cur_key_value == NO_KEY) {  //cur_key = NO_KEY; last_key = valid_key . 按键被抬起
			if (tkey_scan_para.press_cnt >= tkey_scan_para.long_time) {  //长按/HOLD状态之后被按键抬起;
                key_event = RMKEY_EVENT_UP;
//				printf("[RM]---->> RMKEY_EVENT_UP\n");
                key_value = tkey_scan_para.last_key;
                goto _notify;  	//发送抬起消息
            }
			tkey_scan_para.click_delay_cnt = 1;  //按键等待下次连击延时开始
		} else {  //cur_key = valid_key, last_key = NO_KEY . 按键被按下
            tkey_scan_para.press_cnt = 1;  //用于判断long和hold事件的计数器重新开始计时;
			tkey_scan_para.hold_cnt = 0;
            if (cur_key_value != tkey_scan_para.notify_value) {  //第一次单击/连击时按下的是不同按键, 单击次数重新开始计数
                tkey_scan_para.click_cnt = 1;
                tkey_scan_para.notify_value = cur_key_value;
            } else {
                tkey_scan_para.click_cnt++;  //单击次数累加
            }
        }
		goto _scan_end;  //返回, 等待延时时间到
	} else {
		if (cur_key_value == NO_KEY) {  //last_key = NO_KEY; cur_key = NO_KEY . 没有按键按下
            if (tkey_scan_para.click_cnt > 0) {  //有按键需要消息需要处理
				if (tkey_scan_para.click_delay_cnt > tkey_scan_para.click_delay_time) { //按键被抬起后延时到
                    //TODO: 在此可以添加任意多击事件
                    if (tkey_scan_para.click_cnt >= 5) {
                        key_event = RMKEY_EVENT_FIRTH_CLICK;  //五击
//						printf("[RM]---->> nKEY_EVENT_FIRTH_CLICK\n");
                    } else if (tkey_scan_para.click_cnt >= 4) {
                        key_event = RMKEY_EVENT_FOURTH_CLICK;  //4击
//						printf("[RM]---->> KEY_EVENT_FOURTH_CLICK\n");
                    } else if (tkey_scan_para.click_cnt >= 3) {
                        key_event = RMKEY_EVENT_TRIPLE_CLICK;  //三击
//						printf("[RM]---->> KEY_EVENT_TRIPLE_CLICK\n");
                    } else if (tkey_scan_para.click_cnt >= 2) {
                        key_event = RMKEY_EVENT_DOUBLE_CLICK;  //双击
//						printf("[RM]---->> KEY_EVENT_DOUBLE_CLICK\n");
                    } else {
                        key_event = RMKEY_EVENT_CLICK;  //单击
//						printf("[RM]---->> KEY_EVENT_CLICK\n");
                    }
                    key_value = tkey_scan_para.notify_value;
                    goto _notify;
                } else {	//按键抬起后等待下次延时时间未到
                    tkey_scan_para.click_delay_cnt++;
                    goto _scan_end; //按键抬起后延时时间未到, 返回
                }
			} else {
                goto _scan_end;  //没有按键需要处理
            }
		} else {  //last_key = valid_key; cur_key = valid_key, press_cnt累加用于判断long和hold
            tkey_scan_para.press_cnt++;
            if (tkey_scan_para.press_cnt == tkey_scan_para.long_time) {
                key_event = RMKEY_EVENT_LONG;
//				printf("[RM]---->> KEY_EVENT_LONG\n");
            } else if (tkey_scan_para.press_cnt == tkey_scan_para.hold_time) {
                key_event = RMKEY_EVENT_HOLD;
//				printf("[RM]---->> KEY_EVENT_HOLD\n");
                tkey_scan_para.press_cnt = tkey_scan_para.long_time;
				tkey_scan_para.hold_cnt++;
            } else {
                goto _scan_end;  //press_cnt没到长按和HOLD次数, 返回
            }
            //press_cnt没到长按和HOLD次数, 发消息
            key_value = cur_key_value;
            goto _notify;
        }
	}

_notify:
	// 处理key_value对应长按事件
	if (key_event == RMKEY_EVENT_UP) {
		if (tkey_scan_para.hold_cnt >= tkey_scan_para.long_hold_time3) {
//			printf("[RM]---->> RMKEY_EVENT_LONG3 %d %d\n", tkey_scan_para.hold_cnt, tkey_scan_para.long_hold_time3);
		} else if (tkey_scan_para.hold_cnt >= tkey_scan_para.long_hold_time2) {
//			printf("[RM]---->> RMKEY_EVENT_LONG2 %d %d\n", tkey_scan_para.hold_cnt, tkey_scan_para.long_hold_time2);
		} else if (tkey_scan_para.hold_cnt >= tkey_scan_para.long_hold_time1) {
//			printf("[RM]---->> RMKEY_EVENT_LONG1 %d %d\n", tkey_scan_para.hold_cnt, tkey_scan_para.long_hold_time1);
		} else {
			;
		}
		msg_enqueue(MSG_CTP_LONG);
	}

	if (key_event) {
		slideA_flag = 0;
		slideB_flag = 0;
		slideC_flag = 0;
	}

    tkey_scan_para.click_cnt = 0;  //单击次数清0
    tkey_scan_para.notify_value = NO_KEY;

//    RMKEY_EVENT_NONE,
//    RMKEY_EVENT_CLICK,
//    RMKEY_EVENT_LONG,
//    RMKEY_EVENT_LONG1,
//    RMKEY_EVENT_LONG2,
//    RMKEY_EVENT_LONG3,
//    RMKEY_EVENT_HOLD,
//    RMKEY_EVENT_UP,
//    RMKEY_EVENT_DOUBLE_CLICK,
//    RMKEY_EVENT_TRIPLE_CLICK,
//    RMKEY_EVENT_FOURTH_CLICK,
//    RMKEY_EVENT_FIRTH_CLICK,
//    RMKEY_EVENT_USER,
//    RMKEY_EVENT_MAX,
    if (key_value == 1) {
        if (key_event == RMKEY_EVENT_CLICK) {
            msg_enqueue(MSG_CTP_CLICK);
        } else if (key_event == RMKEY_EVENT_DOUBLE_CLICK) {
            msg_enqueue(MSG_CTP_DOUBLE_CLICK);
        } else if (key_event == RMKEY_EVENT_TRIPLE_CLICK) {
            msg_enqueue(MSG_CTP_TRIPLE_CLICK);
        } else if (key_event == RMKEY_EVENT_FOURTH_CLICK) {
            msg_enqueue(MSG_CTP_FOURTH_CLICK);
        }
    }

//    printf(touch_key_scan_print, key_value, key_event);


_scan_end:
	tkey_scan_para.last_key = cur_key_value;
}

#endif // 0
