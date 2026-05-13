/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 	  private.c
 Author:		  sunll
 Version:		  V1.0
 Date:			  2023-12-15
 Description:	  comm interface definition
 History:

******************************************************************************/
#include "include.h"
#include "hed_private.h"
#include "port_hed_i2c.h"
#include <stdint.h>
#include <string.h>
#if SECURITY_PAY_EN
#if (SECURITY_PAY_VENDOR == SECURITY_VENDOR_HED)
#define COMMAND_LIST_NUM 60
#define VAR 0xFF


uint8_t g_buf[2600] AT(.alipay.hed_HEDI2CBuf);  //主端开的buf

double_queue_node g_queue_in ;
double_queue_node g_queue_out ;

uint8_t* IIC_Master_Init(void)
{
    void bsp_alipay_i2c_init(void);
    bsp_alipay_i2c_init();
    memset(g_buf, 0, 2600);
    return g_buf;
}
void HED_IIC_Init(void)
{
	g_queue_in.q_buf = IIC_Master_Init();
    g_queue_out.q_buf= g_queue_in.q_buf + DEQUE_MAX_SIZE ;
}

void hed_rst_init(void)
{
    PORT_I2C_SE0_RST_RESET();
}

/*************************************************
Function:	  api_connect
Description:  SE连接应用
Input:	      out_buf         ATR数据
			  out_buf_len     ATR数据长度
Output:	      no
Return:	      参见error.h
Others:       no
*************************************************/
se_error_t api_connect (uint8_t *out_buf, uint32_t *out_buf_len)
{
    printf("%s\n", __func__);
	se_error_t ret = 0;
	//参数判断
	if(out_buf==NULL||out_buf_len==NULL)
	{
		printf("failed to api_connect output params!");
		return SE_ERR_PARAM_INVALID;
	}
	//调用acl层acl_connect接口
	ret = acl_connect(out_buf,out_buf_len);
	if(ret!=SE_SUCCESS)
	{
		printf("failed to call acl_connect!\n");
		return ret;
	};

	return SE_SUCCESS;
}

/*************************************************
Function:	  api_transceive
Description:  SE命令传输应用
Input:	      in_buf      APDU命令流
			  in_buf_len  APDU命令流长度
Output:	      out_buf     APDU响应
			  out_buf_len APDU响应数据长度
Return:	      参见error.h
Others:       no
*************************************************/
se_error_t api_transceive(const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret = 0;

	//参数判断
	if(in_buf==NULL||in_buf_len==0)
	{
		LOGE("failed to api_transceive input params!");
		return SE_ERR_PARAM_INVALID;
	}
	if(out_buf==NULL||out_buf_len==NULL)
	{
		LOGE("failed to api_transceive output params!");
		return SE_ERR_PARAM_INVALID;
	}
	//调用acl层acl_transceive接口
	ret = acl_transceive(in_buf,in_buf_len,out_buf,out_buf_len);
	if(ret!=SE_SUCCESS)
	{
		LOGE("failed to call acl_transceive!\n");
		return ret;
	};

	return SE_SUCCESS;
}

/*************************************************
Function:	  api_reset
Description:  SE复位应用
Input:	      out_buf         ATR数据
			  out_buf_len     ATR数据长度
Output:	      no
Return:	      参见error.h
Others:       no
*************************************************/
se_error_t api_reset (uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret = 0;
	//参数判断
	if(out_buf==NULL||out_buf_len==NULL)
	{
		LOGE("failed to api_reset output params!");
		return SE_ERR_PARAM_INVALID;
	}
	//调用acl层acl_reset接口
	ret = acl_reset(out_buf,out_buf_len);
	if(ret!=SE_SUCCESS)
	{
		LOGE("failed to call acl_reset!\n");
		return ret;
	};

	return SE_SUCCESS;

}

//*************************************************
//info.c
//*************************************************
/*************************************************
Function:	  api_sdk_version_number
Description:  获取整型sdk版本号
Input:	      no
Output:	      num 整型版本号
Return:	      参见error.h
Others:	      no
*************************************************/
se_error_t  api_sdk_version_number(uint32_t *num)
{

	//参数检查
	if(num==NULL)
	{
		LOGE("failed to api_sdk_version_number pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	*num = SDK_VERSION_NUM;

	return SE_SUCCESS;
}

/*************************************************
Function:	  api_sdk_version_string
Description:  获取字符串版本号
Input:	      no
Output:	      str 字符串版本号
Return:	      参见error.h
Others:	      no
*************************************************/
se_error_t  api_sdk_version_string (char *str)
{

	//参数检查
	if(str==NULL)
	{
		LOGE("failed to api_sdk_version_string pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	strcpy(str,SDK_VERSION_STRING);

	return SE_SUCCESS;
}
//*************************************************
//ctrl.c
//*************************************************
/*************************************************
Function:	  api_switch_mode
Description:  切换工作模式应用
Input:	      type         工作模式
Output:	      no
Return:	      参见error.h
Others:       no
*************************************************/
//se_error_t api_switch_mode (work_mode  type)
//{
//	se_error_t ret = 0;
//	//参数检查
//	if(type!=STANDBY&&type!=POWERDOWN)
//	{
//		LOGE("failed to api_switch_mode input params!\n");
//		return SE_ERR_PARAM_INVALID;
//	}
//	//调用cmd层apdu_switch_mode接口
//	ret = apdu_switch_mode (type);
//	if(ret!=SE_SUCCESS)
//	{
//		LOGE("failed to call apdu_switch_mode!\n");
//		return ret;
//	}

//	return SE_SUCCESS;

//}
/*************************************************
Function:	  api_control
Description:  命令控制应用
Input:	      ctrlcode        控制类型
			  in_buf       输入数据
				  in_buf_len   输入数据长度
Output:	      out_buf      输出数据
			  out_buf_len  输出数据长度
Return:	      参见error.h
Others:       no
*************************************************/
se_error_t api_control(ctrl_type ctrlcode, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret = 0;
	//参数检查
	if(in_buf==NULL||in_buf_len==0)
	{
		LOGE("failed to api_control input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(out_buf==NULL||out_buf_len==NULL)
	{
		LOGE("failed to api_control output params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	//调用cmd层apdu_control接口
	ret = apdu_control(ctrlcode, in_buf, in_buf_len,out_buf,out_buf_len);
	if(ret!=SE_SUCCESS)
	{
		LOGE("failed to call apdu_control!\n");
		return ret;
	}

	return SE_SUCCESS;
}
//*************************************************
//tudu.c
//*************************************************
/*************************************************
	Function:	 tpdu_init
	Description: 设置命令头子函数
	Input:	     isoCase  APDU协议类型
			     cla      CLA
	ins          INS
	p1           P1
	p2           P2
	lc           Lc
	le           LE
	Output:	     command 命令结构体
	Return:	     参见技术规格返回码
	Others:	     no
*************************************************/
iso_command_apdu_t* tpdu_init(iso_command_apdu_t *command, int32_t isoCase, int32_t cla, int32_t ins,int32_t p1, int32_t p2,int32_t lc, int32_t le)
{
	#ifdef _ASSERT_DEBUG
	ASSERT(command!=NULL);
	#endif

	//赋值APDU各参数
	if ( isoCase >= 0 )
		command->isoCase = isoCase;
	if ( cla >= 0 )
		command->classbyte = cla;
	if ( ins >= 0 )
		command->instruction = ins;
	if ( p1  >= 0 )
		command->p1 = p1;
	if ( p2  >= 0 )
		command->p2 = p2;
	if ( lc  >= 0 )
		command->lc = lc;
	if ( le  >  0 )
		command->le = le;

	return command;
}

/*************************************************
	Function:	  tpdu_init_with_id
	Description:  根据命令头初始表设计命令头
	Input:	      commandID 命令ID号
	Output:	      command 命令结构体
	Return:	      参见技术规格返回码
	Others:	      no
*************************************************/
iso_command_apdu_t* tpdu_init_with_id(iso_command_apdu_t *command, uint8_t commandID)
{
	typedef struct
	{
		uint8_t  isoCase;
		uint8_t  classbyte;
		uint8_t  instruction;
		uint8_t  p1;
		uint8_t  p2;
	}command_info_t;
	command_info_t *pc = NULL;
	//命令初始化表，方便后续更新命令
	command_info_t command_list[COMMAND_LIST_NUM]=
	{
		{ISO_CASE_3, 0x80, 0x50, 0x00, 0x00},   //CMD_SCP_INIT_UPDATE
		{ISO_CASE_3, 0x84, 0x82, VAR, 0x00},    //CMD_SCP_AUTH
        {ISO_CASE_3, VAR, 0xD8, 0x00, 0x00},    //CMD_PUT_KEY
		{ISO_CASE_2, 0x00, 0x84, 0x00,0x00},    //CMD_GET_RANDOM
		{ISO_CASE_3, 0x00, 0xA4, 0x00,0x00},    //CMD_SELECT_FILE
		{ISO_CASE_3, VAR, 0xD6, VAR, VAR},      //CMD_WRITE_FILE
		{ISO_CASE_3, VAR, 0xB0, VAR, VAR},      //CMD_READ_FILE
        {ISO_CASE_2, 0x80,0xC8, VAR, 0x00},     //CMD_GET_INFO
		{ISO_CASE_3, 0x80, 0xE1,0x00,VAR},      //CMD_SWITCH_MODE
        {ISO_CASE_3, 0x80, 0xCE,VAR,0x00},      //CMD_CLEAR_FILE
	};
	#ifdef _ASSERT_DEBUG
    ASSERT(command!=NULL);
		ASSERT(commandID<sizeof(command_list));
	#endif
	pc = &command_list[commandID];
	//根据command ID指令赋值各个参数
    tpdu_init(command,
        pc->isoCase,
        pc->classbyte,
        pc->instruction,
        pc->p1,
        pc->p2,
        -1,
        -1
    );

    return command;
}
/*************************************************
	Function:	  tpdu_set_cla
	Description:  修改CLA
	Input:	      cla      CLA
	Output:	      command 命令结构体
	Return:	      参见技术规格返回码
	Others:	      no
*************************************************/
iso_command_apdu_t* tpdu_set_cla  (iso_command_apdu_t *command, uint32_t cla)
{
	#ifdef _ASSERT_DEBUG
	ASSERT(command!=NULL);
	#endif
	//更新CLA参数
	tpdu_init(command, -1, cla, -1, -1, -1,  -1, -1);
    return command;

}
/*************************************************
	Function:	  tpdu_set_p1p2
	Description:  修改P1P2
	Input:	      p1       P1
	              p2       P2
	Output:	      command 命令结构体
	Return:	      参见技术规格返回码
	Others:	      no
*************************************************/
iso_command_apdu_t* tpdu_set_p1p2 (iso_command_apdu_t *command, uint32_t p1, uint32_t p2)
{
	#ifdef _ASSERT_DEBUG
	ASSERT(command!=NULL);
	#endif
	//更新P1P2
	tpdu_init(command, -1, -1, -1, p1, p2, -1, -1);
    return command;
}
/*************************************************
	Function:	  tpdu_set_le
	Description:  修改Le
	Input:	      le       LE
	Output:	      command 命令结构体
	Return:	      参见技术规格返回码
	Others:       no
*************************************************/
iso_command_apdu_t* tpdu_set_le(iso_command_apdu_t *command, uint32_t le)
{
	#ifdef _ASSERT_DEBUG
	ASSERT(command!=NULL);
	#endif
	//更新Le
	tpdu_init(command, -1, -1, -1, -1, -1, -1, le);
    return command;
}

/*************************************************
	Function:	  tpdu_pack
	Description:  子函数，将APDU命令结构体设置到双端队列中
	Input:	      command命令结构体
	Output:	      output  输出数据
	              output_len  输出数据长度
	Return:	      参见技术规格返回码
	Others:	      no
*************************************************/
se_error_t tpdu_pack(iso_command_apdu_t *command, uint8_t *output, uint32_t *output_len)
{
	//output 为双端队列
	uint8_t front_buffer[5] = {0};
	uint8_t rear_buffer[2] = {0};
	double_queue queue_out = (double_queue)output;

	//参数判断
	if(command==NULL||output==NULL||output_len==NULL)
	{
		LOGE("failed to tpdu_pack input params!");
		return SE_ERR_PARAM_INVALID;
	}


	//拷贝到缓存区，方便放入双向队列
	front_buffer[0] = command->classbyte;
	front_buffer[1] = command->instruction;
	front_buffer[2] = command->p1;
	front_buffer[3] = command->p2;
	//根据APDU协议类型拼装4种数据
	//ISO_CASE_1 = 0x01,  // CLA INS P1 P2 00
	//ISO_CASE_2 = 0x02,  // CLA INS P1 P2 Le
	//ISO_CASE_3 = 0x03,  // CLA INS P1 P2 Lc Data
	//ISO_CASE_4 = 0x04   // CLA INS P1 P2 Lc Data Le

	switch ( command->isoCase ) {
		case ISO_CASE_1:
			front_buffer[4] = 0x00;
			queue_front_push(front_buffer,5, queue_out);
			*output_len = 5;
			break;

		case ISO_CASE_2:
			front_buffer[4] = (command->le)&0xFF;
			*output_len = 5;
			queue_front_push(front_buffer,5, queue_out);
			break;

		case ISO_CASE_3:
			front_buffer[4] = queue_size(queue_out)&0xFF;
			queue_front_push(front_buffer,5, queue_out);
			*output_len = queue_size(queue_out);
			break;

		case ISO_CASE_4:
			front_buffer[4] = queue_size(queue_out)&0xFF;
			*output_len = 5+queue_size(queue_out)+1;
			queue_front_push(front_buffer,5, queue_out);
			rear_buffer[1]=(command->le)&0xFF;
			queue_rear_push(rear_buffer,1, queue_out);
			break;
	}
	output = (uint8_t*)queue_out;

	return SE_SUCCESS;

}
/*************************************************
	Function:	  tpdu_unpack
	Description:  子函数，拆分状态字和数据
	Input:	       no
	Output:	       response-> queue_out   输出双端队列
			       response-> status_word 状态字
	Return:	       参见技术规格返回码
	Others:	       no
*************************************************/
se_error_t tpdu_unpack(uint8_t *output, uint32_t *output_len)
{
	double_queue queue_out=(double_queue)output;
	//参数判断
	if(output==NULL||output_len==NULL)
	{
		LOGE("failed to tpdu_unpack output params!");
		return SE_ERR_PARAM_INVALID;
	}
	//去除状态字
	queue_rear_pop(2 ,queue_out);
	output = (uint8_t *)queue_out;
	*output_len = queue_out->q_buf_len;

	return SE_SUCCESS;
}

/*************************************************
	Function:	   tpdu_send
	Description:   apdu命令发送接收
	Input:	       in_buf      输入数据
	               in_buf_len  输入数据长度
	Output:	       output  输出数据
	output_len     输出数据长度
	Return:	       参见技术规格返回码
	Others:	       no
*************************************************/
se_error_t tpdu_send(uint8_t *input, uint32_t input_len, uint8_t *output, uint32_t *output_len)
{

	double_queue queue_in=(double_queue)input;
	double_queue queue_out=(double_queue)output;
	uint8_t temp_buffer[5]={0x00};
	se_error_t ret = 0;
	uint32_t temp_len = 0;

	//参数判断
	if(input==NULL||input_len==0)
	{
		LOGE("failed to tpdu_send input params!");
		return SE_ERR_PARAM_INVALID;
	}
	if(output==NULL||output_len==NULL)
	{
		LOGE("failed to tpdu_send output params!");
		return SE_ERR_PARAM_INVALID;
	}


	//备份命令处理61XX和6CXX情况
	if(queue_in->q_buf_len==5)
	{
		 memcpy(temp_buffer,queue_in->q_buf+queue_in->front_node,5);
	}

	while(1)
	{
		ret=acl_transceive_queue((uint8_t*)queue_in,queue_in->q_buf_len, (uint8_t*)queue_out,&temp_len);
		//处理61XX和6CXX情况
		if((temp_len==2)&&(queue_out->q_buf[queue_out->rear_node-2]==0x61))
		{
				memcpy(temp_buffer,"\x00\xC0\x00\x00",4);
				temp_buffer[4] = queue_out->q_buf[queue_out->rear_node-1];
				queue_init(queue_in);
				queue_init(queue_out);
				queue_rear_push(temp_buffer,5, queue_in);
				continue;
		}
		if((temp_len==2)&&(queue_out->q_buf[queue_out->rear_node-2]==0x6C))
		{
				temp_buffer[4] = queue_out->q_buf[queue_out->rear_node-1];
				queue_init(queue_in);
				queue_init(queue_out);
				queue_rear_push(temp_buffer,5, queue_in);
				continue;
		}
		break;
	}
	output = (uint8_t *)queue_out;
	*output_len = queue_out->q_buf_len;
	if(ret!=SE_SUCCESS)
		return ret;


	return SE_SUCCESS;
}
/*************************************************
	Function:	   tpdu_execute
	Description:   apdu命令处理及发送接收
	Input:	       command     command命令结构体
	               in_buf      输入数据
	               in_buf_len  输入数据长度
	Output:	       output      输出数据
	               output_len  输出数据长度
	Return:        参见技术规格返回码
	Others:   	   no
*************************************************/
se_error_t tpdu_execute	(iso_command_apdu_t *command, uint8_t *input, uint32_t input_len, uint8_t *output, uint32_t *output_len)
{

	se_error_t ret = 0;
	uint16_t status_word = 0;
	double_queue queue_in=(double_queue)input;
	double_queue queue_out=(double_queue)output;
	uint32_t in_len = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	uint32_t change_status = 0;

	//参数判断
	if(command==NULL||input==NULL)
	{
		LOGE("failed to tpdu_execute input params!");
		return SE_ERR_PARAM_INVALID;
	}
	if(output==NULL||output_len==NULL)
	{
		LOGE("failed to tpdu_execute output params!");
		return SE_ERR_PARAM_INVALID;
	}
	//将APDU命令放入双向队列
	ret = tpdu_pack(command,(uint8_t *) queue_in,&in_len);
	if(ret!=SE_SUCCESS)
		return ret;
	//发送接收APDU命令
	ret = tpdu_send((uint8_t *) queue_in,queue_in->q_buf_len,(uint8_t *)queue_out,&out_len);
	//先判断返回是否正确
	if(ret!=SE_SUCCESS)
	{
		output = (uint8_t *)queue_out;
	  *output_len = out_len;
		return ret;
	}
	//将状态字取出
	off = queue_out->rear_node;
	status_word=((queue_out->q_buf[off-2]<< 8) & 0xFF00) | (queue_out->q_buf[off-1] & 0xFF);
	//6310需要处理，先将数据返回
	if(status_word==0x6310)
	{
		change_status = tpdu_change_error_code(status_word);
		ret = tpdu_unpack((uint8_t *)queue_out,&out_len);
	    output = (uint8_t *)queue_out;
	    *output_len = out_len;
		return change_status;
	}
	//其他返回，直接返回错误码，正确继续处理去状态字
	if(status_word!=0x9000)
		return tpdu_change_error_code(status_word);

	//通信且返回都正常，将状态字从数据中去除，并返回有效数据
	ret = tpdu_unpack((uint8_t *)queue_out,&out_len);
	if(ret!=SE_SUCCESS)
		return ret;
	output = (uint8_t *)queue_out;
	*output_len = out_len;

	return SE_SUCCESS;

}
/*************************************************
	Function:	   tpdu_execute_no_response
	Description:   apdu命令处理及发送接收，无返回数据
	Input:	       command     command命令结构体
	               in_buf      输入数据
	               in_buf_len  输入数据长度
	Output:	       no
	Return:        参见技术规格返回码
	Others:   	   no
*************************************************/
se_error_t tpdu_execute_no_response	(iso_command_apdu_t *command, uint8_t *input, uint32_t input_len)
{

	se_error_t ret = 0;
	uint16_t status_word = 0;
	double_queue queue_in=(double_queue)input;
	//double_queue_node queue_out={0};
	uint32_t in_len = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;

	//参数判断
	if(command==NULL||input==NULL)
	{
		LOGE("failed to tpdu_execute_no_response input params!");
		return SE_ERR_PARAM_INVALID;
	}
	//将APDU命令放入双向队列
	queue_init(&g_queue_out);
	ret = tpdu_pack(command,(uint8_t *) queue_in,&in_len);
	if(ret!=SE_SUCCESS)
		return ret;
	//发送APDU命令，无返回
	ret = tpdu_send((uint8_t *) queue_in,queue_in->q_buf_len,(uint8_t *)&g_queue_out,&out_len);
	off = g_queue_out.front_node;
	status_word=((g_queue_out.q_buf[off]<< 8) & 0xFF00) | (g_queue_out.q_buf[off+1] & 0xFF);
	//判断返回是否正确
	if(ret!=SE_SUCCESS)
		return ret;
	//判断状态字是否正确
	if(status_word!=0x9000)
		return tpdu_change_error_code(status_word);

	return SE_SUCCESS;

}


/*************************************************
	Function:	  tpdu_change_error_code
	Description:  状态码转换
	Input:	      status_word 源状态吗
	Output:	      no
	Return:	      参见技术规格返回码
	Others:	      no
*************************************************/
se_error_t tpdu_change_error_code(uint16_t status_word)
{
	se_error_t error = (se_error_t)(status_word&(0x0000FFFF));
	//状态码装换，参见error.h
	if(status_word==0x9000)
		return SE_SUCCESS;
	else
		return (error|(0x10000000));

}
//*************************************************
//apdu.c
//*************************************************
/*************************************************
	Function:	  apdu_switch_mode
	Description:  切换工作模式命令封装
	Input:	      type         工作模式
	Output:	      no
	Return:	      参见error.h
	Others:       no
*************************************************/
se_error_t apdu_switch_mode (work_mode  type)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	//double_queue_node queue_in ={0} ;
	uint32_t p2 = 0;
	if(type!=STANDBY&&type!=POWERDOWN)
	{
		LOGE("failed to apdu_switch_mode input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//双端队列初始化
	queue_init(&g_queue_in);
	//设置命令头
	tpdu_init_with_id(&command,CMD_SWITCH_MODE);
	//设置P1P2
	p2 = (type==STANDBY)?0x00:0x02;
	tpdu_set_p1p2 (&command,0x00,p2);

	ret = tpdu_execute_no_response(&command, (uint8_t *)&g_queue_in, queue_size (&g_queue_in));
	if(ret!=SE_SUCCESS)
		return ret;


	return SE_SUCCESS;
}

/*************************************************
	Function:	  apdu_control
	Description:  命令控制命令封装
	Input:	      ctrlcode     控制类型
	              in_buf       输入数据
			      in_buf_len   输入数据长度
	Output:	      out_buf      输出数据
	              out_buf_len  输出数据长度
	Return:	      参见error.h
	Others:       no
*************************************************/
se_error_t apdu_control(ctrl_type ctrlcode, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	return SE_SUCCESS;
}

/*********************************************************************************
Function:		acl_connect
Description:	连接外设及获取外设ATR
			1. 检查参数，加载选择外设的驱动，若只注册了一个外设，
			且未选择此外设，加载第一个注册的设备
			2. 根据加载的外设驱动，调用init 初始化函数和open函数
Input:		no
Output: 	out_buf 输出的起始地址
		  	out_buf_len  输出数据长度的起始地址
Return: 		函数操作状态码
Others: 		no
*********************************************************************************/
se_error_t acl_connect (uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	peripheral *periph = NULL;
    printf("%s\n", __func__);
	do
	{
		if ((out_buf == NULL) || (out_buf_len == NULL))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		//3.打开设备
		ret_code = proto_i2c_open(periph, out_buf , out_buf_len); //打开设备,获取ATR
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

	}while(0);

	return ret_code;
}

/******************************************************************
Function:		acl_transceive_queue
Description:	以双向队列格式，向从设备发送命令及接收响应
                1.检查参数有效性
		        2.加载总线驱动句柄和设备句柄
                3.通过加载的句柄，调用proto的transceiver函数，发送数据及接收数据响应
Input:	   in_buf 输入双向队列的起始地址
		   in_buf_len 输入双向队列的输入数据长度
Output: 	out_buf 输出双向队列的起始地址
		   out_buf_len  输出双向队列的输出数据长度的起始地址
Return: 	   函数操作状态码
Others: 	   no
******************************************************************/
se_error_t acl_transceive_queue(uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
    printf("%s\n", __func__);
	se_error_t ret_code = SE_SUCCESS;
	peripheral *periph = NULL;

	do
	{
		//检查参数
		if ((in_buf == NULL)||(in_buf_len == 0U) || (out_buf == NULL)||(out_buf_len==NULL))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}



		//---调用proto中的proto_spi_transceive或proto_i2c_transceive,
		//向选择的设备发送命令，并接收命令响应数据
		ret_code = proto_i2c_transceive(periph, in_buf, in_buf_len, out_buf, out_buf_len);
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

	}while(0);

	return ret_code;
}


/******************************************************************
Function:		acl_transceive
Description:	将输入数据存储到双向队列，向外设设备发送数据及接收响应，并从双向队列取出响应数据
                1.输入存储存档到输入双向队列
		        2.调用acl_transceive_queue 函数，以双向队列格式发送和接收数据
                3.从输出双向队列取出响应数据
Input:	   in_buf 输入数据的起始地址
		   in_buf_len 输入数据长度
Output:    out_buf 输出的起始地址
		   out_buf_len  输出数据长度的起始地址
Return:    函数操作状态码
Others:    no
******************************************************************/
se_error_t acl_transceive(const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	uint32_t out_len = 0;
	uint16_t front_node = 0;

	//double_queue_node queue_in ={0} ;
	//double_queue_node queue_out ={0} ;

	do
	{
		//检查参数
		if ((in_buf == NULL) || (out_buf == NULL)||(out_buf_len==NULL))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		if ((in_buf_len<ACL_TRANS_DATA_LEN_MIN)||(in_buf_len>ACL_TRANS_DATA_LEN_MAX))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		//双端队列初始化
		queue_init(&g_queue_in);
		queue_init(&g_queue_out);

		//输入数据存入双端队列
		queue_rear_push((uint8_t *)in_buf,in_buf_len, &g_queue_in);

		//将输入双向队列中的数据按协议格式发送给设备，并将响应数据存储到输出双向队列
		ret_code = acl_transceive_queue((uint8_t *)&g_queue_in, queue_size(&g_queue_in), (uint8_t *)&g_queue_out, &out_len);
		if(ret_code != SE_SUCCESS)
		{
			return ret_code;
		}

		//从双端队列拷贝回输出数据
		front_node = g_queue_out.front_node;
		memcpy(out_buf,&g_queue_out.q_buf[front_node],g_queue_out.q_buf_len);
		*out_buf_len = g_queue_out.q_buf_len;

	}while(0);

	return ret_code;
}


/*************************************************
Function:	   queue_init
Description:   双端队列初始化
Input:	       no
Output:	       d_queue     双端队列结构体
Return:	       参见技术规格返回码
Others:	       no
*************************************************/
double_queue queue_init(double_queue d_queue)
{
	d_queue ->q_buf_len =0;
	d_queue ->capacity=DEQUE_MAX_SIZE;
	d_queue ->front_node=20;
	d_queue ->rear_node =20;
	memset(d_queue->q_buf,0x00,DEQUE_MAX_SIZE);
	return d_queue;
}

/*************************************************
Function:	   queue_front_push
Description:   双端队列头增加数据
Input:	       in_buf       输入数据
               in_buf_len   输入数据长度
Output:	       d_queue      双端队列结构体
Return:	       no
Others:	       no
*************************************************/
void queue_front_push(uint8_t*in_buf,uint32_t in_buf_len, double_queue d_queue)
{
  	uint32_t off = 0;

	if(d_queue ->front_node==0)
	{

		d_queue ->front_node = d_queue ->capacity- in_buf_len;

	}
	else
	{
		d_queue ->front_node-= in_buf_len;
	}
        off = d_queue ->front_node;
        memcpy(d_queue->q_buf+off, in_buf, in_buf_len);
        d_queue ->q_buf_len += in_buf_len;

}

/*************************************************
Function:	   queue_front_pop
Description:   双端队列头删除数据
Input:	       in_buf_len   删除数据长度
Output:	       d_queue      双端队列结构体
Return:	       no
Others:	       no
*************************************************/
void queue_front_pop(uint32_t in_buf_len ,double_queue d_queue)
{
	//uint8_t item[DEQUE_MAX_SIZE] = {0x00};
       // uint32_t off = d_queue->front_node;
	if(d_queue ->front_node ==(d_queue ->capacity- in_buf_len))
	{
		//memcpy(item,d_queue->q_buf,in_buf_len);
		d_queue ->front_node =0;

	}
	else
	{
		//memcpy(item,d_queue->q_buf+off,in_buf_len);
		d_queue ->front_node += in_buf_len;
	}

	d_queue ->q_buf_len -= in_buf_len;

	return;
}


/*************************************************
Function:	   queue_rear_push
Description:   双端队列尾增加数据
Input:	       in_buf       输入数据
               in_buf_len   输入数据长度
Output:	       d_queue      双端队列结构体
Return:	       no
Others:	       no
*************************************************/
void queue_rear_push(uint8_t*in_buf,uint32_t in_buf_len, double_queue d_queue)
{
	uint32_t off = d_queue->rear_node;
	memcpy(d_queue->q_buf+off, in_buf, in_buf_len);
	if(d_queue ->rear_node== d_queue ->capacity- in_buf_len)
	{
		d_queue ->rear_node = 0;
	}
	else
	{
		d_queue ->rear_node += in_buf_len;
	}


	d_queue ->q_buf_len += in_buf_len;

}

/*************************************************
Function:	   queue_rear_pop
Description:   双端队列尾删除数据
Input:	       in_buf_len   删除数据长度
Output:	       d_queue      双端队列结构体
Return:	       删除的数据
Others:	       no
*************************************************/
void queue_rear_pop(uint32_t in_buf_len ,double_queue d_queue)
{
	//uint8_t item[DEQUE_MAX_SIZE] = {0x00};
//        uint32_t off = d_queue->rear_node;
	if(d_queue ->rear_node == 0)
	{
		//off=d_queue ->capacity-in_buf_len;
		//memcpy(item,d_queue->q_buf+off,in_buf_len);
		d_queue ->capacity-= in_buf_len;
	}
	else
	{
		//memcpy(item,d_queue->q_buf+off,in_buf_len);
		d_queue ->rear_node -= in_buf_len;
	}

		d_queue ->q_buf_len -= in_buf_len;

	   return;
}

/*************************************************
Function:	   queue_rear_pop
Description:   双端队列尾删除数据
Input:	       in_buf_len   删除数据长度
Output:	       d_queue      双端队列结构体
Return:	       no
Others:	       no
*************************************************/
uint32_t queue_size (double_queue d_queue)
{
	if(d_queue ->rear_node> d_queue ->front_node)
	{
	   return (d_queue ->rear_node- d_queue ->front_node);
	}
	else
	{
		return (d_queue ->front_node- d_queue ->rear_node);
	}
}

//***************************************************
//proto_i2c_impl.c
//***************************************************
/*************************************************
  Function:	  proto_i2c_crc16
  Description:  计算指定长度指定数据的CRC结果
  Input:
            CRCType：CRC计算类sing
            Length：要计算的数据长度
            Data：要计算的数据的起始地址
  Return:	crc计算结果
  Others:
*************************************************/
uint16_t proto_i2c_crc16(uint32_t CRCType, uint32_t Length , uint8_t *Data)
{
	uint8_t chBlock = 0;
	uint16_t wCrc = 0;

	wCrc = (CRCType == CRC_A) ? 0x6363 : 0xFFFF;	// CRC_A : ITU-V.41 , CRC_B : ISO 3309

	do
	{
		chBlock = *Data++;
		chBlock = (chBlock^(uint8_t)(wCrc & 0x00FF));
		chBlock = (chBlock^(chBlock<<4));
		wCrc= (wCrc >> 8)^((uint16_t)chBlock << 8)^((uint16_t)chBlock<<3)^((uint16_t)chBlock>>4);
	} while (--Length);

	if (CRCType != CRC_A)
	{
		wCrc = ~wCrc; // ISO 3309
	}

	return wCrc;
}


/******************************************************************
Function:		porto_i2c_queue_in
Description:	根据HED I2C协议格式，向双向队列中增加帧头和帧尾数据
		1.向双向队列头部添加 NAD, PCB , LEN
		2.向双向队列尾部添加CRC
Input:	  periph 设备句柄
		  inbuf 输入双向队列的起始地址
		  inbuf_len 输入双向队列的输入数据长度
Output: 	  no
Return: 	  函数操作状态码
Others: 	   no
******************************************************************/
se_error_t porto_i2c_queue_in(uint8_t* inbuf, uint16_t inbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	double_queue queue_in = (double_queue)inbuf;

	uint8_t i2c_data[4] = {0};
	uint8_t offset = 0;
	uint16_t i2c_crc = 0;

	do
	{

		if ((inbuf == NULL) ||(inbuf_len == 0U))
		{
			ret_code =  SE_ERR_PARAM_INVALID;
			break;
		}

		//向双向队列分别添加NAD, PCB, LENH,LENL
		i2c_data[offset++] = PROTO_I2C_NAD;
		i2c_data[offset++] = PROTO_I2C_PCB_I_BLOCK;
		i2c_data[offset++] = inbuf_len>>8;
		i2c_data[offset++] = inbuf_len;
		queue_front_push(i2c_data, offset, queue_in);

		//计算CRC16，并向双向队列添加 CRC
		i2c_crc = proto_i2c_crc16(CRC_B,queue_size(queue_in), &queue_in->q_buf[queue_in->front_node]);	//4 //计算 CRC
		queue_rear_push((uint8_t *)&i2c_crc, 2, queue_in);

	}while(0);

	return ret_code;
}


/******************************************************************
Function:		porto_i2c_send_iblock
Description:	通过I2C 接口发送I 块数据
		1.通过port层设备注册的函数列表指针，调用port层i2c接口的
			发送函数transmit，发送I 块数据
Input:	  periph 设备句柄
		  inbuf 输入双向队列的起始地址
		  inbuf_len 输入双向队列的输入数据长度
Output: 	  no
Return: 	  函数操作状态码
Others: 	   no
******************************************************************/
se_error_t porto_i2c_send_iblock(peripheral *periph, uint8_t* inbuf, uint16_t inbuf_len)
{
    printf("%d%s\n", __LINE__, __func__);
	se_error_t ret_code = SE_SUCCESS;

	do
	{
		if((inbuf == NULL) ||(inbuf_len == 0U))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		hal_delay(PROTO_I2C_RECEIVE_TO_SEND_BGT);

		//发送数据
		ret_code = port_i2c_periph_transmit(inbuf, inbuf_len);

	}while(0);

	return ret_code;

}

/******************************************************************
Function:		porto_i2c_send_lblock
Description:	通过I2C 接口发送L , R或ATR 块
		1.根据块类型，组织块格式的帧数据
		2.通过port层设备注册的函数列表指针，调用port层i2c接口的
			发送函数transmit，发送块数据
Input:	  periph   设备句柄
          block_type  块类型
		  data_len   块类型为L 块时，表示后续要发送的I块的数据长度，
		  块类型为其它块时，其值为0
Output: 	  no
Return:       函数操作状态码
Others: 	   no
******************************************************************/
se_error_t porto_i2c_send_block(uint8_t block_type, uint16_t data_len)
{
	se_error_t ret_code = SE_SUCCESS;
	uint16_t i2c_crc = 0;
	uint8_t i2c_buf[PROTO_I2C_LR_FRAME_LENGTH] = {0};
	uint8_t offset=0;


	//拼接发送数据
	i2c_buf[offset++] = PROTO_I2C_NAD;
	i2c_buf[offset++] = block_type;
	i2c_buf[offset++] = data_len>>8;
	i2c_buf[offset++] = data_len;

	//计算CRC16
	i2c_crc = proto_i2c_crc16(CRC_B,offset, i2c_buf); 	//4 // NAD PCB LEN1 LEN2 CRC
	i2c_buf[offset++] = i2c_crc;
	i2c_buf[offset++] = i2c_crc>>8;

	ret_code = port_i2c_periph_transmit(i2c_buf, offset);

	return ret_code;
}


/******************************************************************
Function:		porto_i2c_send_lblock
Description:	通过I2C 接口发送L 块数据
Input:	  periph 设备句柄
		  data_len L 块的长度值，后续要发送的I块的数据长度
Output:   no
Return:   函数操作状态码
Others:   no
******************************************************************/
se_error_t porto_i2c_send_lblock(peripheral *periph, uint16_t data_len)
{
	hal_delay(PROTO_I2C_RECEIVE_TO_SEND_BGT);
	return porto_i2c_send_block(PROTO_I2C_PCB_L_BLOCK, data_len);
}


/******************************************************************
Function:		porto_i2c_send_rblock
Description:	通过I2C 接口发送R 块数据
Input:	  periph 设备句柄
Output:   no
Return:   函数操作状态码
Others:   no
******************************************************************/
se_error_t porto_i2c_send_rblock(peripheral *periph)
{
	hal_delay(PROTO_I2C_RECEIVE_TO_SEND_BGT);
	return porto_i2c_send_block(PROTO_I2C_PCB_R_BLOCK, 0);
}


/******************************************************************
Function:		porto_i2c_send_atr_block
Description:	通过I2C 接口发送ATR 请求块
Input:	  periph 设备句柄
Output:   no
Return:   函数操作状态码
Others:   no
******************************************************************/
se_error_t porto_i2c_send_atr_block(peripheral *periph)
{
	hal_delay(PROTO_I2C_RECEIVE_TO_SEND_BGT);
	return porto_i2c_send_block(PROTO_I2C_PCB_ATR_BLOCK, 0);
}


/******************************************************************
Function:		porto_i2c_receive_block
Description:	通过I2C 接口接收块数据
		        1.通过port层设备注册的函数列表指针，调用port层i2c接口的
			接收函数receive，接收块数据
Input:	      periph 设备句柄
Output: 	  rbuf 接收块数据的存储地址
              rlen 块信息的长度
Return: 	  函数操作状态码
Others: 	   no
******************************************************************/
se_error_t porto_i2c_receive_block(uint8_t *rbuf, uint16_t rlen)
{
    printf("%d%s\n", __LINE__, __func__);
	se_error_t ret_code = SE_SUCCESS;
	i2c_timer_t timer = {0};
	uint8_t crc_err_count=0;
	uint16_t crc_value = 0;

	if((rbuf == NULL)||(rlen < PROTO_I2C_FRAME_LENGTH_MINI))
	{
		return SE_ERR_PARAM_INVALID;
	}

	//设置接收数据时帧等待的超时时间
	timer.interval = PROTO_I2C_RECEVIE_FRAME_WAIT_TIME;
	timer.start = hal_systick();

	do
	{
		do
		{
			hal_delay(PROTO_I2C_RECEVIE_POLL_TIME);   //delay poll time

			ret_code = port_i2c_periph_receive(rbuf, rlen);
			if(ret_code == SE_SUCCESS)
			{
				break;
			}

			if(i2c_time_get_diff(hal_systick(), timer.start) >= timer.interval)
			{
				LOGE("Failed:receive frame overtime,  ErrCode-%08X.", SE_ERR_TIMEOUT);
				return SE_ERR_TIMEOUT;
			}

		}while(1);


		if(rbuf[PROTO_I2C_PCB_OFFSET] == PROTO_I2C_PCB_I_BLOCK)
		{
			//compare CRC16
			crc_value = proto_i2c_crc16(CRC_B, rlen -PROTO_I2C_CRC_LENGTH, rbuf);
			if((rbuf[rlen -PROTO_I2C_CRC_LENGTH]==(crc_value&0xff))&&(rbuf[rlen-PROTO_I2C_CRC_LENGTH+1]==crc_value>>8))
			{
				return ret_code;  //receive data ok
			}
		}
		else if((rbuf[PROTO_I2C_PCB_OFFSET]  == PROTO_I2C_PCB_R_BLOCK) ||
			   (rbuf[PROTO_I2C_PCB_OFFSET]  == PROTO_I2C_PCB_R_NAK_BLOCK) ||
			   (rbuf[PROTO_I2C_PCB_OFFSET]  == PROTO_I2C_PCB_L_BLOCK) ||
			   (rbuf[PROTO_I2C_PCB_OFFSET]  == PROTO_I2C_PCB_S_BLOCK))
		{
			//compare CRC16
			crc_value = proto_i2c_crc16(CRC_B, PROTO_I2C_LR_FRAME_LENGTH -PROTO_I2C_CRC_LENGTH, rbuf);
			if((rbuf[PROTO_I2C_LR_FRAME_LENGTH -PROTO_I2C_CRC_LENGTH]==(crc_value&0xff))&&(rbuf[PROTO_I2C_LR_FRAME_LENGTH-PROTO_I2C_CRC_LENGTH+1]==crc_value>>8))
			{
				return ret_code;  //receive data ok
			}
		}
		else if(rbuf[PROTO_I2C_PCB_OFFSET] == 0xFF)
		{
			continue;
		}
		else
		{
		    printf("%d%s\n", __LINE__, __func__);
			return SE_ERR_PARAM_INVALID;
		}

		crc_err_count++;  //crc  err
	}while(crc_err_count <= PROTO_I2C_RETRY_NUM);

	return SE_ERR_LRC_CRC;
}



/******************************************************************
Function:		porto_i2c_ receive_lrblock
Description:	通过I2C 接口接收L块或R块，L块长度有效，R块长度为0
		        1.调用porto_i2c_receive_block 函数接收L块、R 块或 S 块
			    2.解析块数据
Input:	  periph 设备句柄
Output:   block_type 块数据类型
          block_len 块类型为L块，表示 后续要接收 I  块信息的长度
Return: 	  函数操作状态码
Others: 	   no
******************************************************************/
se_error_t porto_i2c_receive_lrblock(peripheral *periph, uint8_t *block_type, uint16_t *block_len)
{
    printf("%s\n", __func__);
	se_error_t ret_code = SE_SUCCESS;
	uint8_t i2c_buf[PROTO_I2C_LR_FRAME_LENGTH];
	uint8_t nad = 0;
	uint8_t pcb = 0;
	uint16_t rec_wtx_count = 0;

	do
	{
		ret_code = porto_i2c_receive_block(i2c_buf, PROTO_I2C_LR_FRAME_LENGTH);
		if(ret_code != SE_SUCCESS)
		{
			return ret_code;
		}

		nad = i2c_buf[PROTO_I2C_NAD_OFFSET];
		pcb = i2c_buf[PROTO_I2C_PCB_OFFSET];

		//the NAD recieved is the NAD sended whose high-low bit is changed
		if(((nad>>4)&0x0f)!=(PROTO_I2C_NAD&0x0f) ||
		((nad<<4)&0xf0)!=(PROTO_I2C_NAD&0xf0))
		{
			return SE_ERR_PARAM_INVALID;
		}

		if((pcb == PROTO_I2C_PCB_R_BLOCK) ||(pcb == PROTO_I2C_PCB_R_NAK_BLOCK) || (pcb == PROTO_I2C_PCB_L_BLOCK))
		{
			*block_type = pcb;
			break;
		}

		else if(pcb == PROTO_I2C_PCB_S_BLOCK)   //S块  需要重新计时及接收数据
		{
			if(rec_wtx_count>=PROTO_I2C_WTX_NUM)
			{
				return SE_ERR_COMM;    //接收WTX请求帧超过20次将返回错误，以防止SE不间断发送请求导致无法退出循环
			}
			rec_wtx_count++;
			continue;
		}

		else
		{
			return SE_ERR_PARAM_INVALID;
		}
//		break;
	}while(1);

	*block_len = i2c_buf[PROTO_I2C_LEN_OFFSET]*256 + i2c_buf[PROTO_I2C_LEN_OFFSET+1];	   //cal output len
	printf("i2c_buf\n");
	print_r(i2c_buf, PROTO_I2C_LR_FRAME_LENGTH);
	printf("block_len: %d\n", *block_len);

	return  ret_code;
}



/******************************************************************
Function:		proto_i2c_receive_iblock
Description:	通过I2C 接口接收I 块数据或R块
		        1.调用porto_i2c_receive_block 函数接收I 块数据或R 块
Input:	  periph 设备句柄
          data_len 接收信息块数据的长度
Output:   block_type 块数据类型
          outbuf 输出双向队列的起始地址
Return:   函数操作状态码
Others:   no
******************************************************************/
se_error_t proto_i2c_receive_irblock(peripheral *periph, uint8_t *block_type, uint8_t *outbuf, uint16_t data_len)
{
	se_error_t ret_code = SE_SUCCESS;
	uint8_t nad = 0;
	uint8_t pcb = 0;
	uint16_t  receive_len = 0;

	if((outbuf == NULL)||(data_len == 0U))
	{
		return SE_ERR_PARAM_INVALID;
	}

	do
	{
		ret_code = porto_i2c_receive_block(outbuf, data_len+PROTO_I2C_FRAME_LENGTH_MINI);
		if(ret_code != SE_SUCCESS)
		{
			return ret_code;
		}

		nad = outbuf[PROTO_I2C_NAD_OFFSET];
		pcb = outbuf[PROTO_I2C_PCB_OFFSET];

		//the NAD recieved is the NAD sended whose high-low bit is changed
		if(((nad>>4)&0x0f)!=(PROTO_I2C_NAD&0x0f) ||
		((nad<<4)&0xf0)!=(PROTO_I2C_NAD&0xf0))
		{
			return SE_ERR_PARAM_INVALID;
		}

		//到此应为I 块或 NAK块，否则报错
		if(pcb == PROTO_I2C_PCB_R_NAK_BLOCK)
		{
			*block_type = pcb;
			return ret_code;
		}
		else if(pcb == PROTO_I2C_PCB_I_BLOCK)
		{
			*block_type = pcb;
			break;
		}
		else
		{
			return SE_ERR_PARAM_INVALID;
		}

	}while(0);

	receive_len = outbuf[PROTO_I2C_LEN_OFFSET]*256+outbuf[PROTO_I2C_LEN_OFFSET+1];
	if(data_len != receive_len) //L块长度与I块长度不一致
	{
		return SE_ERR_LEN;
	}

	return ret_code;
}

/*********************************************************************************
Function:       proto_i2c_handle
Description:    按照HED I2C 通信协议的帧格式与SE进行收发数据交互，期间根据
		      接收帧类型及LRC校验等，发送延时帧、NAK帧及错误时帧重发操作
                      主要调用了如下函数:
		      1. proto_spi_send_frame
		      2. proto_spi_receive_frame_head
		      3. proto_spi_receive_frame_data
Input:          handle 设备句柄
		   input  输入数据指针
		   input_len 输入数据长度
Output:    output输出数据存储地址
		   outputlen输出数据长度
Return:         no
Others:         no
*********************************************************************************/
se_error_t proto_i2c_handle(peripheral *periph, uint8_t* inbuf, uint16_t inbuf_len, uint8_t* outbuf, uint16_t* outbuf_len)
{
    printf("%s\n", __func__);
	se_error_t ret_code = SE_SUCCESS;
	uint16_t block_len = 0;
	uint8_t nak_count = 0;
	uint8_t block_type = 0;

	do
	{
		if(nak_count > PROTO_I2C_NAK_NUM)
		{
		    printf("%d%s\n", __LINE__, __func__);
			return SE_ERR_LRC_CRC;
		}
		nak_count++;

		ret_code = porto_i2c_send_lblock(periph, inbuf_len);//发送L块
		if(ret_code != SE_SUCCESS)
		{
		    printf("%d%s  ret_code: %d\n", __LINE__, __func__, ret_code);
			return ret_code;
		}

		ret_code = porto_i2c_receive_lrblock(periph, &block_type, &block_len);  //接收R块
		printf("%d%s  ret_code: %d, block_type: %d\n", __LINE__, __func__, ret_code, block_type);
		if(ret_code != SE_SUCCESS)
		{
			return ret_code;
		}

	}while(block_type == PROTO_I2C_PCB_R_NAK_BLOCK);

	nak_count = 0;
	do
	{
		if(nak_count > PROTO_I2C_NAK_NUM)
		{
            printf("%d%s\n", __LINE__, __func__);
			return SE_ERR_LRC_CRC;
		}
		nak_count++;
        print_r(inbuf, inbuf_len+PROTO_I2C_FRAME_LENGTH_MINI);
		ret_code = porto_i2c_send_iblock(periph, inbuf, inbuf_len+PROTO_I2C_FRAME_LENGTH_MINI); //发送I块
		if(ret_code != SE_SUCCESS)
		{
			return ret_code;
		}

		ret_code = porto_i2c_receive_lrblock(periph, &block_type, &block_len);  //接收L块 或 R 块

		if(ret_code != SE_SUCCESS)
		{
			return ret_code;
		}
	}while(block_type == PROTO_I2C_PCB_R_NAK_BLOCK);

	if((block_type==PROTO_I2C_PCB_L_BLOCK) && (block_len!=0))
	{
		//正确收到L块,发送R块
		nak_count = 0;
		do
		{
			if(nak_count > PROTO_I2C_NAK_NUM)
			{
				return SE_ERR_LRC_CRC;
			}
			nak_count++;

			//正确收到L块,发送R块
			ret_code = porto_i2c_send_rblock(periph);
			if(ret_code != SE_SUCCESS)
			{
				 return ret_code;
			}

			ret_code = proto_i2c_receive_irblock(periph, &block_type, outbuf, block_len);//接收I块或R块
			if(ret_code != SE_SUCCESS)
			{
				 return ret_code;
			}
		}while(block_type == PROTO_I2C_PCB_R_NAK_BLOCK);
	}
	else
	{
		//应该收到L块，却收到了R块
		return SE_ERR_COMM;
	}

	*outbuf_len = block_len+PROTO_I2C_FRAME_LENGTH_MINI;
	return ret_code;
}



/***********************************************************************
Function:       proto_i2c_get_atr
Description:   SPI通信协议之打开设备的操作
                     1. 调用proto_i2c_send_atr_block函数，向SE发送ATR请求块。
                     2. 调用proto_i2c_receive_lrblock接收L块或R块。
                     3. 若接收到为NAK的R块，继续调用proto_i2c_send_atr_block函数发
                            送ATR请求块，并调用proto_i2c_receive_lrblock函数接收L块或R块。
                     4. 若接收到I块，调用proto_i2c_receive_iblock函数接收ATR数据。
Input:        periph 设备句柄
Output:      rbuf  待接收ATR的起始地址
		     rlen  待接收ATR的长度
Return:      函数操作状态码
Others:      no
************************************************************************/
se_error_t proto_i2c_get_atr(peripheral *periph, uint8_t *rbuf, uint32_t *rlen)
{
    printf("%s\n", __func__);
	se_error_t ret_code = SE_SUCCESS;
	uint16_t block_len = 0;
	uint8_t nak_count = 0;
	uint8_t block_type = 0;


	if ((rbuf == NULL) ||(rlen == NULL))
	{
		return SE_ERR_PARAM_INVALID;
	}

	do
	{
		if(nak_count > PROTO_I2C_NAK_NUM)
		{
			return SE_ERR_LRC_CRC;
		}
		nak_count++;

		ret_code = porto_i2c_send_atr_block(periph);  //发送ATR 请求块
		if(ret_code != SE_SUCCESS)
		{
			return ret_code;
		}

		ret_code = porto_i2c_receive_lrblock(periph, &block_type, &block_len);  //接收 L 块 或R 块
		if(ret_code != SE_SUCCESS)
		{
			return ret_code;
		}
	}while(block_type == PROTO_I2C_PCB_R_NAK_BLOCK);

	if((block_type==PROTO_I2C_PCB_L_BLOCK) && (block_len!=0))
	{
		if(block_len>(PROTO_I2C_ATR_MAX_LEN+PROTO_I2C_FRAME_LENGTH_MINI))
		{
			return SE_ERR_LEN;
		}
		nak_count = 0;
		do
		{
			if(nak_count > PROTO_I2C_NAK_NUM)
			{
				return SE_ERR_LRC_CRC;
			}
			nak_count++;

			//正确收到L块,发送R块
			ret_code = porto_i2c_send_rblock(periph);
			if(ret_code != SE_SUCCESS)
			{
				 return ret_code;
			}

			ret_code = proto_i2c_receive_irblock(periph, &block_type, rbuf, block_len);//接收I块( ATR 值)或R块
			if(ret_code != SE_SUCCESS)
			{
				 return ret_code;
			}

		}while(block_type == PROTO_I2C_PCB_R_NAK_BLOCK);

	}
	else
	{
		//应该收到L块，却收到了R块
		return SE_ERR_COMM;
	}

	*rlen = block_len;

	return ret_code;
}


/***********************************************************************
Function:       proto_i2c_open
Description:   SPI通信协议之打开设备的操作
	             1. 通过port层设备注册的函数列表指针，调用port层的lock函数上锁
		         2. 通过port层设备注册的函数列表指针，调用port层i2c接口的函数power_on对设备上电
                 3. 通过port层设备注册的函数列表指针，调用port层i2c接口的函数对设备复位
                 4. 调用proto_i2c_get_atr函数，获取ATR。
                 5. 通过port层设备注册的函数列表指针，调用unlock函数解锁。
Input:        periph 设备句柄
Output:      rbuf  待接收ATR的起始地址
		     rlen  待接收ATR的长度
Return:      函数操作状态码
Others:      no
************************************************************************/
se_error_t proto_i2c_open(peripheral *periph, uint8_t *rbuf, uint32_t *rlen)
{
    printf("%s\n", __func__);
	se_error_t ret_code = SE_SUCCESS;
	i2c_timer_t timer = {0};
	uint8_t atr_buf[PROTO_I2C_ATR_MAX_LEN+PROTO_I2C_FRAME_LENGTH_MINI] = {0};


	if ((rbuf == NULL) || (rlen == NULL))
	{
		return  SE_ERR_PARAM_INVALID;
	}

	//设置锁等待的超时时间
	timer.interval = PROTO_I2C_COMM_MUTEX_WAIT_TIME;
	timer.start = hal_systick();

	do
	{
		if(i2c_time_get_diff(hal_systick(), timer.start) >= timer.interval)
		{
			ret_code = SE_ERR_TIMEOUT;
			LOGE("Failed:open periph mutex,  ErrCode-%08X.", ret_code);
			break;
		}


		ret_code = proto_i2c_get_atr(periph, atr_buf, rlen);
		if(ret_code == SE_ERR_BUSY)
		{
			continue;
		}
		else if(ret_code != SE_SUCCESS)
		{
			LOGE("Failed:protocol get atr,  ErrCode-%08X.", ret_code);
			break;
		}

		memcpy(rbuf, atr_buf+PROTO_I2C_DATA_OFFSET, *rlen);
		LOGI("Open Periph Success!");
		break;

	}while(1);


	return ret_code;
}

/******************************************************************
Function:		proto_i2c_transceive
Description:	通过I2C 接口向从设备命令发送命令及接收命令响应时，调用此函数
			1. 通过port层设备注册的函数列表指针，调用port层的lock函数上锁
			2、根据 HED_I2C 通信协议，调用proto_i2c_handle函数，发送及接收协议帧数据。
			3．通过port层设备注册的函数列表指针，调用unlock函数解锁。
Input:	   periph 设备句柄
		   sbuf 输入双向队列的起始地址
		   slen 输入双向队列的输入数据长度
Output:    rbuf 输出双向队列的起始地址
		   rlen  输出双向队列的输出数据长度的起始地址
Return:        函数操作状态码
Others:  no
******************************************************************/
se_error_t proto_i2c_transceive(peripheral *periph, uint8_t *sbuf,  uint32_t  slen, uint8_t *rbuf, uint32_t *rlen)
{
    printf("%s\n", __func__);
	se_error_t ret_code = SE_SUCCESS;
//	se_error_t ret_code_bak = SE_SUCCESS;
	i2c_timer_t timer = {0};
	double_queue queue_in = (double_queue)sbuf;
	double_queue queue_out = (double_queue)rbuf;
	uint16_t send_len = 0;
	uint16_t rec_len = 0;
	uint8_t* p_output = NULL;
	uint8_t* p_input = NULL;

	if ((sbuf == NULL) || (rbuf == NULL) || (slen == 0U) || (rlen == NULL))
	{
		return  SE_ERR_PARAM_INVALID;
	}

    printf("%d%s\n", __LINE__, __func__);
    print_r(sbuf, slen);
	//准备收发的数据
	send_len = (uint16_t)slen;
	ret_code =porto_i2c_queue_in(sbuf, slen);
	if(ret_code != SE_SUCCESS)
	{
		return ret_code;
	}

	p_input = &queue_in->q_buf[queue_in->front_node];
	p_output = &queue_out->q_buf[queue_out->front_node];

	//设置锁等待的超时时间
	timer.interval = PROTO_I2C_COMM_MUTEX_WAIT_TIME;
	timer.start = hal_systick();

	do
	{
		if(i2c_time_get_diff(hal_systick(), timer.start) >= timer.interval)
		{
			ret_code = SE_ERR_TIMEOUT;
			LOGE("Failed:communication mutex,  ErrCode-%08X.\n", ret_code);
			break;
		}

        printf("%d%s\n", __LINE__, __func__);
        print_r(p_input, send_len);
		//启动I2C  数据的收发
		ret_code = proto_i2c_handle(periph, p_input, send_len, p_output,&rec_len);
		if(ret_code == SE_ERR_BUSY)
		{
			continue;
		}

		else if(ret_code == SE_ERR_LRC_CRC)
		{
			printf("Failed:check crc,  ErrCode-%08X.\n", ret_code);
			//控制SE 的RST引脚进行复位操作
			//ret_code_bak = p_i2c_periph->control(p_i2c_periph, PROTO_I2C_CTRL_RST, NULL, NULL);
			//if(ret_code_bak != SE_SUCCESS)
			//{
			//	LOGE("Failed:protocol communication control,  ErrCode-%08X.", ret_code_bak);
			//}
			//p_i2c_periph->power_off(p_i2c_periph);
			break;
		}

		else if(ret_code != SE_SUCCESS)
		{
			printf("Failed:protocol communication,  ErrCode-%08X.", ret_code);
			break;
		}

		queue_out->q_buf_len = rec_len;
		queue_out->rear_node =  queue_out->front_node + rec_len;

		queue_front_pop(PROTO_I2C_NAD_PCB_LEN_LENGTH, queue_out); //移除NAD, PCB, LEN
		queue_rear_pop(PROTO_I2C_CRC_LENGTH, queue_out);//移除CRC
		*rlen = queue_size(queue_out);
		printf("Communication Success!");
		break;
	}while(1);

	return ret_code;
}

#endif
#endif
