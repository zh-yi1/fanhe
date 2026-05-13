#ifndef __VENDOR_BLE_H__
#define __VENDOR_BLE_H__

#include "alipay_common.h"

/*蓝牙发送数据:
 * @param [in] data 待发送数据的指针
 * @param [in] len  待发送数据的大小
 * @return void
 */
EXTERNC void alipay_ble_write(PARAM_IN uint8_t *data,PARAM_IN uint16_t len);

#endif
