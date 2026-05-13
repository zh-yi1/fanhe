/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include "include.h"
#include "iotsec.h"
#include "utils.h"
#if SECURITY_PAY_EN
extern char g_mac_address[18];
u8 gatt_alipay_tx(u8 *buf, u16 len);
void ble_get_local_bd_addr(u8 *addr);

/*○ 功能描述
    ■
  获取设备ID号(以冒号分割的16进制mac地址)，要求内容以‘\0’结尾且长度不包含'\0'。所有字母大写，长度为17。例如：“AA:BB:CC:00:11:22”
  ○ 接口参数
    ■ buf_did - 存放设备ID数据地址
    ■ len_did - 存放设备ID长度地址
  ○ 返回值
    ■ 0表示成功，非0表示失败*/
csi_error_t csi_get_deviceid(uint8_t* buf_did, uint32_t* len_did)
{
    //*len_did= strlen(g_mac_address);
    // memcpy(buf_did, g_mac_address, strlen(g_mac_address));
    u8 addr_ble[6];
    ble_get_local_bd_addr(addr_ble);
    //char *buff_qr = func_zalloc(18);
    //char *buff_qr = ab_malloc(18);
    char *buff_qr = SECURITY_MALLOC(18);

    sprintf(buff_qr, "%02x:%02x:%02x:%02x:%02x:%02x", addr_ble[0], addr_ble[1], addr_ble[2], addr_ble[3], addr_ble[4], addr_ble[5]);
//    printf("csi_get_deviceid: %s\n", buff_qr);

    //const char * mac_address  = "18:7a:93:b3:9a:bc";
    const char * mac_address  = buff_qr;
    *len_did= strlen(mac_address);
    memcpy(buf_did, mac_address, strlen(mac_address));
    //func_free(buff_qr);
    //ab_free(buff_qr);
    SECURITY_FREE(buff_qr);
    return CSI_OK;
}

/*○ 功能描述
    ■ 获取设备product model
  ○ 接口参数
    ■ buffer - 存放设备product model数据地址
    ■ len - 存放设备product model长度地址
  ○ 返回值
    ■ 0表示成功，非0表示失败*/
csi_error_t csi_get_productmodel(uint8_t* buffer, uint32_t* len)
{
    const char* mock_model = "568x";
    *len = strlen(mock_model);
    memcpy(buffer, mock_model, strlen(mock_model));
    return CSI_OK;
}

/*○ 功能描述
    ■ 获取当前系统时间戳（Unix时间戳格式）
  ○ 接口参数
    ■ tm - 存放系统时间戳的变量地址
  ○ 返回值
    ■ 0表示成功，非0表示失败*/
csi_error_t csi_get_timestamp(uint32_t* tm)
{
    *tm = RTCCNT;   //UTC time
    u32 day_offset = (2020 - 1970)*365 + (2020 - 1970)/4;
    day_offset = day_offset*86400;
//    printf("%s: %d, %d\n", __func__, day_offset, RTCCNT);
    *tm = *tm + day_offset - 8*3600;
    printf("*tm: %d ,tm addr:%x\n", *tm, tm);
    return CSI_OK;
}

/*设置手表时间，设置之后要求及时生效，并且通过 alipay_get_timestamp 接口获取的是修改后的时间
 * 注:UTC时间戳，从1970开始的
 * @param [in] timestamp_s  时间戳，单位为s
 * @return
 */
void alipay_set_system_time(int32_t timestamp_s)
{
    printf("timestamp_s: %d\n", timestamp_s);
}

/*○ description:
    ■ get compile timestamp
  ○ param
  ○ return
    ■ compile timestamp*/
csi_error_t csi_get_compile_timestamp(uint32_t* timestamp)
{
    if (timestamp == NULL)
    {
        return CSI_ERROR;
    }

    *timestamp = get_compile_timestamp();
    return CSI_OK;
}

/*○ 功能描述
    ■ 获取设备SN(厂商印刷在卡片上的设备序列号)，长度不超过32个字符，只能包含大小写字母、数字、下划线。仅卡片类产品且有SN在小程序显示需求的厂商实现，其他厂商请输出""(空字符串)，len_sn=0
  ○ 接口参数
    ■ buf_sn - 存放设备SN数据地址
    ■ len_sn - 存放设备SN长度地址
  ○ 返回值
    ■ 0表示成功，非0表示失败*/
csi_error_t csi_get_sn(uint8_t* buf_sn, uint32_t* len_sn)
{
//    printf("%s\n", __func__);
    char mock_sn[20];
//    char *mock_sn2 = "01:02:03:04:05:08";
    sprintf(mock_sn, "%02x:%02x:%02x:%02x:%02x:%02x", xcfg_cb.serial_num[0], xcfg_cb.serial_num[1], xcfg_cb.serial_num[2], xcfg_cb.serial_num[3], xcfg_cb.serial_num[4], xcfg_cb.serial_num[5]);
//    sprintf(mock_sn, "%02x:%02x:%02x:%02x:%02x", xcfg_cb.serial_num[0], xcfg_cb.serial_num[1], xcfg_cb.serial_num[2], xcfg_cb.serial_num[3], xcfg_cb.serial_num[4]);
//    printf("mock_sn: %s  len: %d\n", mock_sn, strlen(mock_sn));
    *len_sn = strlen(mock_sn);
    memcpy(buf_sn, mock_sn, *len_sn);
    return CSI_OK;
}

/*○ 功能描述
    ■ 获取设备company name
  ○ 接口参数
    ■ buffer - 存放设备company name数据地址
    ■ len - 存放设备company name长度地址
  ○ 返回值
    ■ 0表示成功，非0表示失败*/
csi_error_t csi_get_companyname(uint8_t* buffer, uint32_t* len)
{
    const char* mock_company = "bluetrum";
    *len = strlen(mock_company);
    memcpy(buffer, mock_company, strlen(mock_company));

    return CSI_OK;
}


/*○ 功能描述
    ■ 打印日志信息
  ○ 接口参数
    ■ format - 格式化输出字符串
    ■ ... - 可变参数
  ○ 返回值
    ■ 无*/
void csi_log_ext(const char* format, va_list* val_list)
{
#if SECURITY_PAY_LOG
    a_vprintf(format, *val_list);
#endif // SECURITY_PAY_LOG
}

//int vsprintf(char *buffer, const char *format, va_list param)
//{
//    return 0;
//}

//void csi_log_ext(const char *format,...)
//{
//#if SECURITY_PAY_LOG
//    char log_buf[512] = {0};
//	va_list arg_list;
//	va_start(arg_list, format);
//	vsprintf(log_buf, format, arg_list);
//
//	printf("%s", log_buf);
//	va_end(arg_list);
//#endif // SECURITY_PAY_LOG
//}

/*○ 功能描述
    ■ 打印日志信息
  ○ 接口参数
    ■ level - 日志调试打印等级
    ■ format - 格式化输出字符串
    ■ value - 输出数据
  ○ 返回值
    ■ 无*/
void csi_log(int level, const char* format, int32_t value)
{
    printf("[level%d %d] %s\n", level, value, format);
}

/*○ 功能描述
    ■ 获取设备通讯协议类型
  ○ 接口参数
    ■ ptype - 存放设备通讯协议类型变量地址
  ○ 返回值
    ■ 0表示成功，非0表示失败*/
csi_error_t csi_get_protoctype(uint32_t* ptype)
{
    *ptype = 0;
    return CSI_OK;
}

/*○ 功能描述
    ■ 发送蓝牙数据
  ○ 接口参数
    ■ data - 存放发送数据地址
    ■ len - 存放发送数据长度(len<=20)
  ○ 返回值
    ■ 0表示成功，非0表示失败*/
csi_error_t csi_ble_write(uint8_t* data, uint16_t len)
{
    return gatt_alipay_tx(data, len);
}
//
//void __ltdf2(void){}
//void __subdf3(void){}
//void __unorddf2(void){}
//void __ledf2(void){}
//void __muldf3(void){}
//double strtod(const char *str, char **endptr){return 0;}
////void __gedf2(void){}		//使用软浮点库，需要注释掉此处
//void __fixdfsi(void){}

//WEAK void* sbrk(int inc){printf("###%s\n",__func__);return NULL;}

#endif
