#ifndef __ALIPAY_BLE_COMMON_H__
#define __ALIPAY_BLE_COMMON_H__

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
    RV_UNKNOWN
} retval_t;

//typedef unsigned short     uint16_t;
//typedef unsigned char      uint8_t;
////typedef char               int8_t;
//typedef long long          int64_t;
//typedef unsigned long long uint64_t;
//typedef int                int32_t;
//typedef unsigned int       uint32_t;
//// typedef unsigned int        size_t;


#if !defined(__cplusplus)
#ifndef bool
typedef unsigned char bool;
#endif
#ifndef false
#define false ((bool)0)
#define true  ((bool)1)
#endif
#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#if 0
#define STATIC static
#else
#define STATIC_DEFED
#define STATIC
#endif

#ifndef NULL
#define NULL 0
#endif

#define PARAM_IN
#define PARAM_OUT
#define PARAM_INOUT


#define LOG(level, format, ...)
#define LOG_DBG(format, ...)
#define LOG_INFO(format, ...)
#define LOG_WARN(format, ...)
#define LOG_ERROR(format, ...)
#define LOG_FATAL(format, ...)

// #define CHECK_BOOL_RET_VAL(val, ret)
// #define CHECK_BOOL_RET(val)
// #define CHECK_NULL_RET_VAL(val, ret)
// #define CHECK_NULL_RET(val)
// #define CHECK_FUNC_RET_VAL(func, ret)
// #define CHECK_FUNC_RET(func)
// #define CHECK_NULL_RET_NULL(val)
// #define CHECK_FUNC_RET_NULL(func)
// #define CHECK_BOOL_RET_NULL(val)

#define CHECK_BOOL_RET_VAL(val, ret)                \
do {                                                \
    if (!(val)) {                                   \
        LOG_ERROR("%s WRONG.", #val);               \
        return ret;                                 \
    }                                               \
} while(0)

#define CHECK_BOOL_RET(val)                     \
CHECK_BOOL_RET_VAL(val, RV_UNKNOWN)

#define CHECK_NULL_RET_VAL(val, ret)            \
CHECK_BOOL_RET_VAL(NULL!=(val), ret)

#define CHECK_NULL_RET(val)                     \
CHECK_NULL_RET_VAL(val, RV_UNKNOWN)

#define CHECK_FUNC_RET_VAL(func, ret)           \
CHECK_BOOL_RET_VAL(RV_OK==(func), ret)

#define CHECK_FUNC_RET(func)                    \
CHECK_FUNC_RET_VAL(func, RV_UNKNOWN)

#define CHECK_NULL_RET_NULL(val) CHECK_NULL_RET_VAL(val, NULL)
#define CHECK_FUNC_RET_NULL(func) CHECK_FUNC_RET_VAL(func, NULL)
#define CHECK_BOOL_RET_NULL(val) CHECK_BOOL_RET_VAL(val, NULL)

#define CHECK_FUNC_RET_VAL_AND_RET(func)           \
CHECK_BOOL_RET_VAL(RV_OK==(ret = func), ret)





#endif


