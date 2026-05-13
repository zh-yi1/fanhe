#ifndef __HAL_BLE_H__
#define __HAL_BLE_H__

#include "hal_common.h"

/*蓝牙发送数据:
 * @param [in] data 待发送数据的指针
 * @param [in] len  待发送数据的大小
 * @return void
 */
hal_error_t hal_ble_write(uint8_t *data, uint16_t len);

#endif


