#ifndef _THIRD_FUNC_H
#define _THIRD_FUNC_H

#if (ASR_SELECT == ASR_YJ)

enum{
	PARAM_START_ET=0xeb,	//注意要上面最后结束的地址开始
#if THIRD_UI_LIGHT_EN
	PARAM_ET_DEFAULT_OPEN,	//1 byte
	PARAM_ET_REG_LED_NUM,	//1 byte
	PARAM_ET_COLOR_SWT_MODE,	//1 byte
	PARAM_ET_LIGHT_UP	//1 byte
#endif
};

enum{
	EVT_ET_MESSAGE_START=0x740,	//0x7c0,	//songjian 1028
	EVT_ET_FUNC_KWS_DEAL,
	EVT_ET_FUNC_UART_DEAL,
	#if ET_ASR_FUNC_IO_CTRL_EN
	EVT_ET_ASR_FUNC_ON,
	EVT_ET_ASR_FUNC_OFF,
	#endif // ET_ASR_FUNC_IO_CTRL_EN
	EVT_ET_NO_OPEN_FAN,
	EVT_ET_NO_SUPPORT_FUNC,
	EVT_ET_EXIT_WAKE_WORD,
	EVT_ET_ALREADY_MUTE,
	EVT_ET_MAX_FENGSU,
	EVT_ET_MIN_FENGSU,
	EVT_ET_BOOT_XIAOMEIXIAOMEI,
	EVT_ET_BOOT_XIAOHEXIAOHE,
	EVT_ET_BOOT_XIAOAIXIAOAI,
	EVT_ET_BOOT_MSG_MAX,
	EVT_ET_MAX
};

void set_et_kws_event(u32 event);
void set_et_uart_event(u32 event);

#if THIRD_UI_LIGHT_EN
int third_func_message(u16 msg);
#endif // THIRD_UI_LIGHT_EN

unsigned char et_checksum(const unsigned char* data, uint16_t start, uint16_t len);
unsigned char et_crc_check(const unsigned char* data, uint16_t start, uint16_t len);
void uint_to_big_endian_byte_array(unsigned int value, unsigned char* array);

#endif //ASR_SELECT

#endif
