/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		vendor_api.c
 Author:			  sunll
 Version:			  V1.0
 Date:			    2023-05-10
 Description:	  fs interface definition
 History:

******************************************************************************/
#include "include.h"
#include "vendor_api.h"
#include "hed_private.h"
#if SECURITY_PAY_EN
#if (SECURITY_PAY_VENDOR == SECURITY_VENDOR_HED)
#define HSI2CBUF_LEN    1250
#define HED_I2C_WRITE_ADDR    (0x54)
#define HED_I2C_READ_ADDR     (0x55)


void hal_delay(unsigned int us)
{
    delay_us(us);
}

u32 hal_systick(void)
{
    return tick_get();
}

u8 i2c_write(unsigned char *pData, unsigned short wLen)
{
    printf("%s, wLen: 0x%x\n", __func__, wLen);
    print_r(pData, wLen);
    unsigned char byAddr = HED_I2C_WRITE_ADDR;
    sys_ctp_irq_disable(IRQ_CTP_VECTOR);
    port_irq_free(PORT_CTP_INT_VECTOR);
    bsp_hs_i2c_tx_buf(byAddr, pData, wLen);
    port_irq_register(PORT_CTP_INT_VECTOR, ctp_int_isr);
    sys_ctp_irq_enble(IRQ_CTP_VECTOR);
    return 0;
}

u8 i2c_read(unsigned char *pData, unsigned short wLen)
{
    unsigned char byAddr = HED_I2C_READ_ADDR;
    sys_ctp_irq_disable(IRQ_CTP_VECTOR);
    port_irq_free(PORT_CTP_INT_VECTOR);
    bsp_hs_i2c_rx_buf(byAddr, pData, wLen);
    printf("%s, wLen: 0x%x\n", __func__, wLen);
    print_r(pData, wLen);
    port_irq_register(PORT_CTP_INT_VECTOR, ctp_int_isr);
    sys_ctp_irq_enble(IRQ_CTP_VECTOR);
    return 0;
}

EXTERNC retval_t csi_user_cmd_process_sync(uint8_t *req, int32_t len_req, uint8_t *rsp, int32_t *len_rsp)
{
    printf("%s\n", __func__);
    se_error_t ret_code = SE_SUCCESS;
	uint16_t front_node = 0;
    uint8_t head = 0xA0;
    //参数判断
	if(req==NULL||len_req==0)
	{
		printf("failed to csi_user_cmd_process_sync input params!");
		return RV_UNKNOWN;
	}
	if(rsp==NULL||len_rsp==NULL)
	{
		printf("failed to csi_user_cmd_process_sync output params!");
		return RV_UNKNOWN;
	}
    //双端队列初始化
    queue_init(&g_queue_in);
    queue_init(&g_queue_out);

    //输入数据存入双端队列
    queue_rear_push((uint8_t *)req,len_req, &g_queue_in);

    queue_front_push(&head,1, &g_queue_in);

    //将输入双向队列中的数据按协议格式发送给设备，并将响应数据存储到输出双向队列
    ret_code = acl_transceive_queue((uint8_t *)&g_queue_in, queue_size(&g_queue_in), (uint8_t *)&g_queue_out, (uint32_t *)len_rsp);
    if(ret_code != SE_SUCCESS)
    {
        return ret_code;
    }

    //从双端队列拷贝回输出数据
    if(g_queue_out.q_buf_len < 2)
    {
        return 1;
    }
    g_queue_out.q_buf_len -= 2;//去掉状态字9000
    front_node = g_queue_out.front_node;
    memcpy(rsp,&g_queue_out.q_buf[front_node],g_queue_out.q_buf_len);
    *len_rsp = g_queue_out.q_buf_len;

    return ret_code;

}

EXTERNC void csi_enter_lpm(void *ctx)
{
    se_error_t ret = 0;

	//调用cmd层apdu_switch_mode接口
	ret = apdu_switch_mode (POWERDOWN);
	if(ret!=SE_SUCCESS)
	{
		LOGE("failed to call apdu_switch_mode!\n");
		return;
	}

	return;
}

EXTERNC void csi_exit_lpm(void *ctx)
{
    PORT_I2C_SE0_RST_LOW();
    hal_delay(PORT_I2C_SE_RST_LOW_DELAY);  //复位时，RST低电平持续时间
    PORT_I2C_SE0_RST_HIGH();
    hal_delay(PORT_I2C_SE_RST_HIGH_DELAY);  //复位后，RST高电平持续时间
}


se_error_t comm_test (void)
{
	uint8_t com_outbuf[300] = {0};
	uint32_t outlen =0;
	uint8_t atr[30] = {0};
	uint32_t atr_len = 0;
	uint32_t i = 0;
	se_error_t ret = 0;


	//---- 3. 连接，获取atr ----
	ret = api_connect(com_outbuf, &outlen);
	if(ret!=SE_SUCCESS)
	{
		 printf("failed to api_connect\n");
		 return ret;
	}
	printf("api_connect atr:\n");
	for(i = 0;i<outlen;i++)
	{
			printf("%02x",com_outbuf[i]);
	}
	printf("\n");

 /****************************************SE热复位，接收ATR*****************************************/
	atr_len = 0;

	ret = api_reset(atr, &atr_len);
	if(ret != SE_SUCCESS)
	{
		printf("failed to api_reset\n");
		return ret;
	}


	return SE_SUCCESS;

}

#endif
#endif
