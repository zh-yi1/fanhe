#include "include.h"
#include "alipay_common.h"
#include "hal_ble.h"
#include "csi_ble.h"
#include "vendor_ble.h"
#if SECURITY_PAY_EN

EXTERNC void alipay_log_ext(const char *format, ...);

void alipay_ble_write(uint8_t *data, uint16_t len) {
    alipay_log_ext("alipay_ble_write: %d", len);
    hal_error_t ret = hal_ble_write(data, len);
    if (ret != HAL_OK) {
        alipay_log_ext("ble_write failed: %d", ret);
    }
}

/*-------------------------------    HAL     --------------------------------------*/

hal_error_t hal_ble_write(uint8_t *data, uint16_t len)
{
    csi_error_t ret;

    if(!data || !len) {
        return HAL_INVALID_PARAM;
    }

    ret = csi_ble_write(data, len);
    if(ret) {
        return HAL_ERROR;
    }

    return HAL_OK;
}
#endif // SECURITY_PAY_EN
