/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		comm.h
 Author:			liuch
 Version:			V1.0
 Date:			    2020-04-24
 Description:	    comm interface declearation
 History:

******************************************************************************/



#ifndef _HED_PRIVATE_H_
#define _HED_PRIVATE_H_

#include "include.h"
//#include "apdu.h"
#include <stdint.h>
#include <string.h>

//#include "port_stm32l433_i2c.h"




#ifdef _ASSERT_DEBUG
//#include <assert.h>
#define  ASSERT(int) assert(int)
#endif

#ifdef __cplusplus
extern "C" {
#endif

//typedef unsigned char    bool;
typedef  int32_t se_error_t;

#define     PORT_I2C_HAL_TIMEOUT	         50       //ms
#define		PORT_I2C_SE_RST_LOW_DELAY        200      //us	T7
#define		PORT_I2C_SE_RST_HIGH_DELAY       10000      //us	 T6

#define		PORT_I2C_SE_PWR_OFF_DEALY        5000      //us	 5ms
#define		PORT_I2C_SE_PWR_ON_DEALY         5000      //us	 5ms

#define	CRC_A 0
#define	CRC_B 1

#define PORT_I2C_SE0_RST_RESET()        {GPIOEFEN &= ~BIT(3); GPIOEDE  |= BIT(3); GPIOESET = BIT(3); GPIOEDIR &= ~BIT(3);}
#define PORT_I2C_SE0_RST_LOW()          GPIOECLR = BIT(3)
#define PORT_I2C_SE0_RST_HIGH()         GPIOESET = BIT(3)

#define  ASSERT(int)


#define SDK_VERSION_NUM 0x01020304
#define SDK_VERSION_STRING "CIU98_D SDK-AL[V1.2.3d-release]"



#ifndef NULL
#define NULL  0
#endif

#ifndef FALSE
#define FALSE 0
#define false 0

#endif

#ifndef TRUE
#define TRUE  1
#define true  1
#endif

#define  _DEBUG

#define  HAL_UART_PRINTF_ENABLE   0  //0-Jlink打印日志，1-串口打印日志

//SDK算法版本切换
#define ALG_V0278
//#define ALG_V0279
//#define ALG_V0280

//#define  _ASSERT_DEBUG




#define		I2C_PERIPHERAL_SE0       	       0
#define		I2C_PERIPHERAL_SE1       	       1




#define HED_I2C
#define HED_I2C_SE0

#define MAX_BUS_DRIVER 4

#define ACL_CRL_POWER           0x0000   //功耗控制

#define MAX_PERIPHERAL_DEVICE                   4
#define MAX_PERIPHERAL_BUS_DRIVER         4


#define ACL_TRANS_DATA_LEN_MIN        5
#define ACL_TRANS_DATA_LEN_MAX        261

#define DEQUE_MAX_SIZE   1300

#define USE_USB_CLKSOURCE_CRSHSI48 1
//#define USB
#if !defined (USE_USB_CLKSOURCE_PLL) && !defined (USE_USB_CLKSOURCE_CRSHSI48)
 #error "Missing USB clock definition"
#endif
/********************协议参数定义*******************/
#define  PROTO_I2C_PCB_L_BLOCK               0x40
#define  PROTO_I2C_PCB_R_BLOCK           	   0x80
#define  PROTO_I2C_PCB_R_NAK_BLOCK     0x81
#define  PROTO_I2C_PCB_ATR_BLOCK          0x30
#define  PROTO_I2C_PCB_S_BLOCK               0xC0
#define  PROTO_I2C_PCB_I_BLOCK	           0x20


#define  PROTO_I2C_NAD                                 0x31

#define PROTO_I2C_LR_FRAME_LENGTH         6
#define PROTO_I2C_FRAME_LENGTH_MINI     6
#define PROTO_I2C_CRC_LENGTH                     2
#define PROTO_I2C_NAD_PCB_LEN_LENGTH   4


#define PROTO_I2C_RETRY_NUM                     3
#define PROTO_I2C_NAK_NUM                         3
#define PROTO_I2C_WTX_NUM                        20
#define PROTO_I2C_ATR_MAX_LEN                32



/********************协议帧偏移*******************/
#define PROTO_I2C_NAD_OFFSET                 (0)
#define PROTO_I2C_PCB_OFFSET                  (1)
#define PROTO_I2C_LEN_OFFSET                  (2)
#define PROTO_I2C_DATA_OFFSET               (4)


#define PROTO_I2C_RECEVIE_FRAME_WAIT_TIME     700   //ms   ,  通信的帧等待时间:FWT
#define PROTO_I2C_COMM_MUTEX_WAIT_TIME         200   //ms


#define PROTO_I2C_RECEVIE_POLL_TIME                          1000  //delay us  帧间隔时间

#define PROTO_I2C_RECEIVE_TO_SEND_BGT                     200  //us
#define PROTO_I2C_POWERON_TO_SEND_BGT                  200  //us
#define PROTO_I2C_SEND_TO_RECEIVE_BGT                     200  //us
/* 外设类型*/
typedef enum _peripheral_type {

	PERIPHERAL_I2C,

}peripheral_type;

/* 外设 */
typedef struct _peripheral {
	peripheral_type type;
	uint8_t id;	/* 外设标识 */
}peripheral;



/**************************************************************************
* Global Type Definition
***************************************************************************/
typedef struct
{
    uint32_t  start;
    uint32_t  interval;
}i2c_timer_t;


 enum  PROTO_I2C_CTRL
 {
	 PROTO_I2C_CTRL_POWER =		 0x00000001,
	 PROTO_I2C_CTRL_RST =	                 0x00000002,
	 PROTO_I2C_CTRL_OTHER =		 0x0000000F
 } ;


#define i2c_time_get_diff(a, b)\
                ((a >= b) ? (a - b) : (0xffffffff -(b - a)))

/* 定义外设结构类型名 */
#define PERIPHERAL_STRUCT_DEFINE(periph_type) struct peripheral_##periph_type

/* 定义外设实例变量名 */
#define PERIPHERAL_NAME(periph_type, id) g_peripheral_device_##periph_type##id

/* 定义指向外设实例的指针*/
#define PERIPHERAL_POINTER(periph_type, id) p_peripheral_device_##periph_type##id


/* 定义外设开始 */
#define PERIPHERAL_DEFINE_BEGIN(periph_type, id) PERIPHERAL_STRUCT_DEFINE(periph_type) PERIPHERAL_NAME(periph_type, id) = {	\
	{periph_type, id},

/* 定义外设结束 */
#define PERIPHERAL_DEFINE_END()  };

/* 注册外设*/
//#define PERIPHERAL_REGISTER(periph_type, id, periph) const PERIPHERAL_STRUCT_DEFINE(periph_type) *PERIPHERAL_NAME(periph_type, id)_ = &periph
#define PERIPHERAL_REGISTER(periph_type, id, periph) PERIPHERAL_STRUCT_DEFINE(periph_type) *PERIPHERAL_POINTER(periph_type, id)= &periph


/* 获取外设*/
#define PERIPHERAL_GET(periph_type, id) PERIPHERAL_POINTER(periph_type, id)

/* 声明外设 */
//#define PERIPHERAL_DECLARE(periph_type, id) extern const PERIPHERAL_STRUCT_DEFINE(periph_type) *PERIPHERAL_NAME(periph_type, id)_
#define PERIPHERAL_DECLARE(periph_type, id) extern PERIPHERAL_STRUCT_DEFINE(periph_type) *PERIPHERAL_POINTER(periph_type, id)

#define HAL_I2C_PERIPHERAL_STRUCT_POINTER PERIPHERAL_STRUCT_DEFINE(PERIPHERAL_I2C)*

PERIPHERAL_STRUCT_DEFINE(PERIPHERAL_I2C) {
    peripheral periph;
    int32_t (*init)      (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);	/* 初始化*/
    int32_t (*deinit)	 (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);	/* 终止化*/
    int32_t (*power_on)  (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);	/* 上电*/
    int32_t (*power_off) (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);	/*下电*/
    int32_t (*lock)      (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);	/* 加锁(启用独占访问)*/
    int32_t (*unlock)    (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);	/* 解锁(解除独占访问)*/
    se_error_t (*transmit)  (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *data, uint32_t  data_len);  /*发送数据 */
    int32_t (*receive)   (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *data, uint32_t *data_len);         /* 接收数据 */
    se_error_t (*control)   (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint32_t ctrlcode, uint8_t *data, uint32_t  data_len);    /* 收发控制命令 */
    void *extra;                                               /* 保存客户自定义的参数 */
};

#ifndef _WIN32
#define SOAPI
#else
#define SOAPI __declspec(dllexport) __stdcall
#endif

#define DIR_FID (0xef00)
#define DIR_LEN (0x69)
#define PER_RECORD_LEN  (0x23)
#define RECORD_NAMELEN_OFFSET  (0x2)
#define RECORD_NAME_OFFSET  (0x3)

enum error_code
{
		SE_SUCCESS =                           0x00000000,  //Success

		SE_ERR_EXPECT_LENGTH =	               0x10006100,	//ISO7816 T0 Expect length
		SE_ERR_RETURN_DATA =                   0x10006281,	//Return Data Error
		SE_ERR_FILE_KEY_INVALID =              0x10006283,	//Select File Invalid/File/Key CRC ERROR
		SE_ERR_NUM_RETRY =                     0x10006300,	//last 1-bytes-Retry Num
		SE_ERR_DATA_REMAINING =	               0x10006310,	//Data Remainning
		SE_ERR_FLAG_UNCHANGED =	               0x10006400,	//Status Flag No Change
		SE_ERR_WRITE_FLASH =	                 0x10006581,	//Write FLASH Failure
	  SE_ERR_CODE_RUN =                      0x10006582,	//Code Run Error
		SE_ERR_LENGTH =	                       0x10006700,	//Length Error
		SE_ERR_CLA_NOT_MATCH =	               0x10006900,	//CLA Error
		SE_ERR_STATUS_INVALID =	               0x10006901,	//Status Invalid
		SE_ERR_STRUCTURSE_NOT_INVALID =	       0x10006981,	//Structure Not Invalid
		SE_ERR_SECURITY_VIOLATION =	           0x10006982,	//Security Violation
		SE_ERR_AUTHENTICATION_LOCKED =         0x10006983,	//Key Locked
		SE_ERR_CHALLENGSE_NOT_GET =	           0x10006984,	//No Challenge
		SE_ERR_OPERATION_VIOLATION =	         0x10006985,	//Operation Violation
		SE_ERR_FILE_NOT_SELECTED =	           0x10006986,	//File No Selected
		SE_ERR_SECURITY_MESSAGE =	             0x10006987,	//No Security Data
		SE_ERR_SECURITY_DATA_INCORRECT =       0x10006988,	//Security Data Incorrect
		SE_ERR_VERIFY_FAILURE =           	   0x10006A80,	//Data Structure Incorrect/Verify Failure
		SE_ERR_FUNCTION_UNSUPPORTED =	         0x10006A81,	//Function Unsupported
		SE_ERR_FILE_NOT_FOUND =	               0x10006A82,	//File Not Found
		SE_ERR_RECORD_NOT_FOUND =	             0x10006A83,	//Record Not Found
		SE_ERR_SPACSE_NOT_ENOUGH =	           0x10006A84,	//Space Not Enough
		SE_ERR_P1P2_INVALID =	                 0x10006A86,	//P1?P2 Error
		SE_ERR_OUT_OF_BOUND =	                 0x10006B00,	//Offset Error
		SE_ERR_DATA_RETURN_LENGTH =	           0x10006C00,	//Le Error/last 1-bytes-Return Length
		SE_ERR_APDU_UNSUPPORTED =	             0x10006D00,	//APDU Unsupported
		SE_ERR_CLA_INVALID =	                 0x10006E00,	//CLA Invalid
		SE_ERR_COMMAND_TURN_INVALID =          0x10006E01,	//Command Turn Invalid
		SE_ERR_SECURITY_INVALID =	             0x10006E02,	//Security Invaild
		SE_ERR_REFERENCE_DATA_INVALID =	       0x10006F00,	//Data Invaild
		SE_ERR_APPLICATION_LOCKED =	           0x10009303,	//Application Locked
		SE_ERR_ALG_UNSUPPORTED =	             0x10009401,	//Alg Unsupported
		SE_ERR_KEYTYPSE_UNSUPPORTED =	         0x10009402,	//Key Type Unsupported
		SE_ERR_KEY_NOT_FOUND =	               0x10009403,	//Key Not Found
		SE_ERR_ID_EXISTED =	                   0x10009404,	//ID Existed
		SE_ERR_KEYTYPE_EXISTED =	             0x10009405,	//Key Type Existed
		SE_ERR_MAC_INVALID =	                 0x10009406,	//MAC Invaild
		SE_ERR_DATA_TRANSMIT_LENGTH =	         0x10009500,	//last 1-bytes reference Data length

		SE_ERR_HANDLE_INVALID =                0x20000000,  //Function Handle Error
		SE_ERR_PARAM_INVALID =                 0x20000001,  //Function Param Error

		SE_ERR_INIT = 		                     0x30000000,  //Init error
		SE_ERR_BUSY=			                     0x30000001,	//Communication busy
		SE_ERR_TIMEOUT = 	                     0x30000002,	//Over Time
		SE_ERR_ATR = 	                         0x30000003,	//ATR Error
		SE_ERR_LRC_CRC =		                   0x30000004,  //LRC/CRC Error
		SE_ERR_LEN = 	                         0x30000005,  //Data Len Error
		SE_ERR_DATA = 	                       0x30000006,  //Data Error
		SE_ERR_COMM = 	                       0x30000007,  //Communication Error
		SE_ERR_NO_SELECT = 	                   0x30000008,  //No Select
};





//APDU协议类型
enum
{
	ISO_CASE_1 = 0x01,  // CLA INS P1 P2 00
	ISO_CASE_2 = 0x02,  // CLA INS P1 P2 Le
	ISO_CASE_3 = 0x03,  // CLA INS P1 P2 Lc Data
	ISO_CASE_4 = 0x04   // CLA INS P1 P2 Lc Data Le

};
//APDU命令参数
typedef struct
{
	uint8_t  isoCase;
	uint8_t  classbyte;
	uint8_t  instruction;
	uint8_t  p1;
	uint8_t  p2;
	uint32_t lc;
	uint32_t le;
} iso_command_apdu_t;

//CLA
enum
{
    CMD_CLA_CASE1 = 0x00,
	  CMD_CLA_CASE2 = 0x04,
    CMD_CLA_CASE3 = 0x80,
	  CMD_CLA_CASE4 = 0x84
};

//commandID
enum
{
  CMD_SCP_INIT_UPDATE = 0x00,   //发起安全通道会话
	CMD_SCP_AUTH,                 //通道认证
	CMD_PUT_KEY,
	CMD_GET_RANDOM,
	CMD_SELECT_FILE,
	CMD_WRITE_FILE,
	CMD_READ_FILE,
	CMD_GET_INFO,
	CMD_SWITCH_MODE,
    CMD_CLEAR_FILE,
};

typedef struct
{
	uint16_t front_node;    //队列头
	uint16_t rear_node;     //队列尾
	uint16_t q_buf_len;     //队列当前数据长度
	uint16_t capacity;      //队列容量
	uint8_t* q_buf;  //队列数组//[DEQUE_MAX_SIZE]

}double_queue_node,*double_queue;

//模式类型
typedef enum work_mode
{
	STANDBY    = 0x00,
	POWERDOWN  = 0x01
}work_mode;

//控制类别预留
typedef enum ctrl_type
{
	RFU = 0x00,
}ctrl_type;

//安全级别
 typedef enum privilege_level
{
	PRIVILEGE_LEVEL_0    = 0x00,
	PRIVILEGE_LEVEL_1    = 0x01,
	PRIVILEGE_LEVEL_2    = 0x02,
	PRIVILEGE_LEVEL_3    = 0x03
}privilege_level;


//SE信息类型
typedef enum info_type
{
	UNIQUE_NUMBER	= 0x00,
	PRODUCT_INFO	 = 0x01,
}info_type;




#define MAX_KEY_LEN  64
//基本密钥信息
typedef struct
{
	uint32_t alg;
    uint32_t id;
  	uint32_t val_len;
    uint8_t  val[MAX_KEY_LEN];
} key_t;

//公钥密钥信息
typedef  key_t  pub_key_t;

#define MAX_IV_LEN 16

//对称算法参数
typedef struct
{
	uint32_t mode;
	uint32_t iv_len;
	uint8_t iv[MAX_IV_LEN];
	uint32_t padding_type;
} alg_sym_param_t;

//非对称算法参数
typedef struct
{
	uint32_t hash_type;
	uint32_t padding_type;
    struct
	{
        uint32_t  enc_mode;
	} sm9;

} alg_asym_param_t;

#define MAX_ID_LEN 128
//用户ID信息
typedef struct
{
	uint32_t val_len;
    uint8_t val[MAX_ID_LEN];
} user_id_t;
//SE ID信息
typedef  user_id_t  se_id_t;

//SE信息
typedef struct
{
	uint8_t unique_number[13];
	uint8_t product_info[8];
}se_info_t;





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*总线类型 */

/* 从外设总线驱动抽象接口 */
typedef struct _peripheral_bus_driver {
	peripheral_type type;
	peripheral    *periph[MAX_PERIPHERAL_DEVICE];		/* 总线外设 */
	se_error_t (*init)		 (peripheral *periph);	/* 初始化 */
	se_error_t (*deinit)	 (peripheral *periph);	/* 终止化 */
	se_error_t (*open)      (peripheral *periph , uint8_t *rbuf, uint32_t *rlen);	/* 打开*/
	se_error_t (*close)     (peripheral *periph);	/* 关闭*/
	se_error_t (*transceive)(peripheral *periph ,  uint8_t *sbuf, uint32_t slen, uint8_t  *rbuf, uint32_t *rlen);	/*数据收发*/
	se_error_t (*reset)      (peripheral *periph , uint8_t *rbuf, uint32_t *rlen);	/*复位 */
	se_error_t (*control)   (peripheral *periph , uint32_t ctrlcode, uint8_t *sbuf, uint32_t slen, uint8_t  *rbuf, uint32_t *rlen);		/* 控制命令 */
	void *extra;
}peripheral_bus_driver;

/* 注册总线驱动 */
//#define PERIPHERAL_BUS_DRIVER_REGISTER(periph_type, driver) const  peripheral_bus_driver *g_peripheral_bus_driver_##periph_type = &driver
#define PERIPHERAL_BUS_DRIVER_REGISTER(periph_type, driver) peripheral_bus_driver *g_peripheral_bus_driver_##periph_type = &driver


/*获取总线外设驱动 */
#define PERIPHERAL_BUS_DRIVER_GET(periph_type) g_peripheral_bus_driver_##periph_type

/* 声明总线外设驱动*/
//#define PERIPHERAL_BUS_DRIVER_DECLARE(periph_type) extern const  peripheral_bus_driver *g_peripheral_bus_driver_##periph_type
#define PERIPHERAL_BUS_DRIVER_DECLARE(periph_type) extern  peripheral_bus_driver *g_peripheral_bus_driver_##periph_type


#define acl_register(periph_type, periph_id) _acl_register((peripheral_bus_driver *)PERIPHERAL_BUS_DRIVER_GET(periph_type), (peripheral *)PERIPHERAL_GET(periph_type, periph_id))
#define api_register(periph_type, periph_id) acl_register(periph_type, periph_id)

#define I2C_PERIPHERAL_NAME(id) PERIPHERAL_NAME(PERIPHERAL_I2C, id)

#define I2C_PERIPHERAL_DEFINE_BEGIN(id) PERIPHERAL_DEFINE_BEGIN(PERIPHERAL_I2C, id)

#define I2C_PERIPHERAL_DEFINE_END() PERIPHERAL_DEFINE_END()

#define I2C_PERIPHERAL_REGISTER(id) PERIPHERAL_REGISTER(PERIPHERAL_I2C, id, PERIPHERAL_NAME(PERIPHERAL_I2C, id))

#define I2C_PERIPHERAL_DECLARE(id) PERIPHERAL_DECLARE(PERIPHERAL_I2C, id)

/**************************************************************************
* Global Variable Declaration
***************************************************************************/
extern peripheral_bus_driver g_proto_i2c;
PERIPHERAL_BUS_DRIVER_DECLARE(PERIPHERAL_I2C);


extern double_queue_node g_queue_in ;
extern double_queue_node g_queue_out ;

#ifdef _DEBUG
//#include "port_stm32l433_util.h"

#define hal_printf_buf(info, buf, buflen)                              \
					hal_printf("\n");                      \
					hal_printf("%s", info);                   \
					for(uint16_t i=0; i<buflen; i++)     \
                                        hal_printf("%3.2x",  buf[i]);


#define LOG_PRINTF(...) hal_printf(__VA_ARGS__)

//#define LOG_TRACE(format, level, file, line, func, ...) hal_printf("\n%s - %s(%d) - %s: "format, level, file, line, func, ##__VA_ARGS__)
#define LOG_TRACE(format, level, file, line, func, ...)

#else
#define hal_printf(...)
#define hal_printf_buf(name, buf, buflen)

#define LOG_PRINTF(...)
#define LOG_TRACE(level, format, ...)
#endif

#define LOGV(format, ...) LOG_TRACE(format, "V", __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOGD(format, ...) LOG_TRACE(format, "D", __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOGI(format, ...) LOG_TRACE(format, "I", __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOGW(format, ...) LOG_TRACE(format, "W", __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOGE(format, ...) LOG_TRACE(format, "E", __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOGF(format, ...) LOG_TRACE(format, "F", __FILE__, __LINE__, __func__, __VA_ARGS__)

#ifndef LOGV
#define  LOGV(format, ...)
#endif

#ifndef LOGD
#define  LOGD(format, ...)
#endif

#ifndef LOGI
#define LOGI(format, ...)
#endif

#ifndef LOGW
#define  LOGW(format, ...)
#endif

#ifndef  LOGE
#define  LOGE(format, ...)
#endif

#ifndef  LOGF
#define  LOGF(format, ...)
#endif


se_error_t api_select (peripheral_type periph_type, uint32_t periph_id);


se_error_t api_connect (uint8_t *out_buf, uint32_t *out_buf_len);


se_error_t api_disconnect(void);


se_error_t api_transceive(const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);


se_error_t api_reset (uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t _acl_register(peripheral_bus_driver *driver, peripheral *periph);

se_error_t acl_select(peripheral_type periph_type, uint32_t periph_id);


se_error_t acl_connect (uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t acl_disconnect (void);

se_error_t acl_transceive(const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t acl_reset (uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t acl_control(uint32_t ctrlcode, uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t acl_transceive_queue(uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t api_switch_mode (work_mode  type);


se_error_t api_control(ctrl_type ctrlcode, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t apdu_scp_init_update(const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);


se_error_t apdu_scp_auth (uint32_t level, const uint8_t *in_buf, uint32_t in_buf_len);

se_error_t  apdu_put_key  (privilege_level level, const uint8_t *in_buf, uint32_t in_buf_len);

se_error_t  apdu_get_random  (uint32_t expected_len, uint8_t *random);

se_error_t  apdu_clear_file(uint32_t  fid,bool if_all);

se_error_t  apdu_select_file(uint32_t  fid);


se_error_t apdu_write_file (uint32_t offset,const uint8_t *in_buf, uint32_t in_buf_len);


se_error_t apdu_read_file  (uint32_t offset, uint32_t expect_len ,uint8_t *out_buf, uint32_t *out_buf_len);


se_error_t  apdu_get_info (info_type type, se_info_t * info);

se_error_t apdu_switch_mode (work_mode  type);


se_error_t apdu_control(ctrl_type ctrlcode, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);

iso_command_apdu_t* tpdu_init(iso_command_apdu_t *command, int32_t isoCase, int32_t cla, int32_t ins,int32_t p1, int32_t p2,int32_t lc, int32_t le);


iso_command_apdu_t* tpdu_init_with_id	(iso_command_apdu_t *command, uint8_t commandID);


iso_command_apdu_t* tpdu_set_cla  (iso_command_apdu_t *command, uint32_t cla);


iso_command_apdu_t* tpdu_set_p1p2 (iso_command_apdu_t *command, uint32_t p1, uint32_t p2);


iso_command_apdu_t* tpdu_set_le(iso_command_apdu_t *command, uint32_t le);


se_error_t tpdu_pack(iso_command_apdu_t *command, uint8_t *output, uint32_t *output_len);


se_error_t tpdu_unpack(uint8_t *output, uint32_t *output_len);


se_error_t tpdu_send(uint8_t *input, uint32_t input_len,uint8_t *output, uint32_t *output_len);


se_error_t tpdu_execute	(iso_command_apdu_t *command, uint8_t *input, uint32_t input_len, uint8_t *output, uint32_t *output_len);


se_error_t tpdu_execute_no_response	(iso_command_apdu_t *command, uint8_t *input, uint32_t input_len);


se_error_t tpdu_change_error_code(uint16_t status_word);

se_error_t  api_get_info (info_type type, se_info_t * info);


se_error_t  api_get_id (se_id_t *se_id );


se_error_t  api_sdk_version_number(uint32_t *num);


se_error_t  api_sdk_version_string (char *str);

extern void gpio_init(void);
extern void mcu_init(void);
extern void error_handler(void);
extern void SystemClock_Config(void);

extern double_queue queue_init(double_queue d_queue);
extern void queue_front_push(uint8_t*in_buf,uint32_t in_buf_len, double_queue d_queue);
extern void queue_front_pop(uint32_t in_buf_len ,double_queue d_queue);
extern void queue_rear_push(uint8_t*in_buf,uint32_t in_buf_len, double_queue d_queue);
extern void queue_rear_pop(uint32_t in_buf_len ,double_queue d_queue);
extern uint32_t queue_size (double_queue d_queue);

/**************************************************************************
* Global Functon Declaration
***************************************************************************/
extern se_error_t proto_i2c_init(peripheral *periph);
extern se_error_t proto_i2c_deinit(peripheral *periph);
extern se_error_t proto_i2c_open(peripheral *periph, uint8_t *rbuf, uint32_t *rlen);
extern se_error_t proto_i2c_close(peripheral *periph) ;
extern se_error_t proto_i2c_transceive(peripheral *periph, uint8_t *sbuf,    uint32_t  slen, uint8_t *rbuf, uint32_t *rlen);
extern se_error_t proto_i2c_reset(peripheral *periph, uint8_t *rbuf, uint32_t *rlen);
extern se_error_t proto_i2c_control(peripheral *periph , uint32_t ctrlcode, uint8_t *sbuf, uint32_t slen, uint8_t  *rbuf, uint32_t *rlen);



extern void HED_IIC_Init(void);
void hal_delay(unsigned int us);
u32 hal_systick(void);
void hed_rst_init(void);
#ifdef __cplusplus
}
#endif
#endif









