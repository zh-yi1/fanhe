#ifndef __ALIPAY_BIND_H__
#define __ALIPAY_BIND_H__

#include "alipay_common.h"

/**
 * 初始化该功能，启动设备或者进入支付宝前调用一次即可，若失败则表示功能不可用，不允许打开支付宝
 * @return  0：    初始化成功
 *          其他value：失败
 */
EXTERNC int alipay_pre_init(void);

/**
 * 获取绑定码
 * @param [out] result     存放绑定码的指针,建议buffer大小为256
 * @param [inout] len_result 绑定码数据大小，传入时为允许的最大长度，传出时为实际的长度
 * @return  RV_OK：    获取成功
 *          RV_WRONG_PARAM：入惨错误，result为空，或者*len_result为0
 *          RV_BUF_TOO_SHORT：缓存太小
 */
EXTERNC retval_e alipay_get_binding_code(PARAM_OUT char* result, PARAM_INOUT int *len_result);

/***
 * 轮询绑定结果
 * @param [out] status     当前的绑定状态 
 *                          ALIPAY_STATUS_UNKNOWN   还未开始绑定
 *                          ALIPAY_STATUS_START_BINDING 绑定中
 *                          ALIPAY_STATUS_BINDING_OK 绑定成功
 *                          ALIPAY_STATUS_BINDING_FAIL 绑定失败
 * @return  
 */
EXTERNC retval_e alipay_query_binding_result(PARAM_OUT binding_status_e* status);

/**
 * BLE数据接收的CallBack函数
 * @param [in] data 存放数据的指针
 * @param [in] len 收到的数据大小
 * @return void
 */
EXTERNC void alipay_ble_recv_data_handle(PARAM_IN const uint8_t *data, PARAM_IN uint32_t len);
#endif