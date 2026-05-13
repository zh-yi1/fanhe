/***************************************************************************
* Include Header Files
***************************************************************************/
#include "include.h"
#include "hed_private.h"
#include "port_hed_i2c.h"

#if SECURITY_PAY_EN

/*********************************************************************************
Function:       port_i2c_periph_transmit
Description:   通过I2C接口向从设备periph发送多字节数据
               1.调用mcu hal库的HAL_I2C_Master_Transmit函数发送多字节数据
Input:       periph 设备句柄
		     inbuf 待发送数据的起始地址
		     inbuf_len 待发送数据的长度
Output:      no
Return:      函数操作状态码
Others:      no
*********************************************************************************/
se_error_t port_i2c_periph_transmit(uint8_t *inbuf, uint32_t  inbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	do
	{
		if((inbuf == NULL)||(inbuf_len == 0U))
		{
			ret_code =  SE_ERR_PARAM_INVALID;
			break;
		}

		ret_code = i2c_write(inbuf, inbuf_len);
	}while(0);

	return ret_code;
}

/*********************************************************************************
Function:       port_i2c_periph_receive
Description:   通过I2C 接口从从设备periph接收多字节数据
             1.调用mcu hal库的HAL_I2C_Master_Receive函数接收多字节数据
Input:       periph 设备句柄
		     outbuf 待接收数据的起始地址
		     outbuf_len 待接收数据的长度
Output:      no
Return:      函数操作状态码
Others:      no
*********************************************************************************/
se_error_t port_i2c_periph_receive(uint8_t *outbuf, uint16_t outbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	do
	{
		if((outbuf == NULL) || (outbuf_len == 0))
		{
			ret_code =  SE_ERR_PARAM_INVALID;
			printf("Failed:i2c param invalid!,  ErrCode-%08X.", ret_code);
			break;
		}
		ret_code = i2c_read(outbuf, outbuf_len);
	}while(0);

	return ret_code;
}
#endif
