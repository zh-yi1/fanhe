/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef IOT_SEC_H
#define IOT_SEC_H

#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>
#include "alipay_common.h"

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef enum {
    RV_OK = 0,
    RV_OPEN_ERROR,
    RV_WRITE_ERROR,
    RV_READ_ERROR,
    RV_DEL_ERROR,
    RV_NOT_FOUND,
    RV_DECRYPT_ERROR,
    RV_HOTP_ERROR,
    RV_DATA_CORRUPT,
    RV_PARAM_INVALID,
    RV_COMMUVERS_ERROR,
    RV_SDKVERS_ERROR,
    RV_UNSUPPORT,
    RV_NO_PERMISSION,
    RV_ECC_GENERATE_ERROR,
    RV_ECC_SHARE_ERROR,
    RV_GET_PRIVATE_KEY_ERROR,
    RV_GET_CERT_ERROR,
    RV_SIGN_ERROR,
    RV_UNKNOWN
} retval_t;

EXTERNC retval_t csi_user_cmd_process_sync(uint8_t *req, int32_t len_req, uint8_t *rsp, int32_t *len_rsp);
EXTERNC void csi_enter_lpm(void *ctx);
EXTERNC void csi_exit_lpm(void *ctx);
#endif
