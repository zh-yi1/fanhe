//#include "alipay_common.h"
//#include "vendor_ble.h"
//#include <stdio.h>
//
//#define ALIPAY_TEST_LOG(format, ...) \
//    printf("[ALIPAY_TEST]"#format"\n", ##__VA_ARGS__)
//
//#define TEST_CASE(func_name) \
//    bool TEST_##func_name()
//
//#define TEST_TAG ALIPAY_TEST_LOG("[ALIPAY_TEST]===================TEST:[%s]\n", __func__)
//
//#define TEST_IT(func)                                         \
//do {                                                            \
//    ALIPAY_TEST_LOG("==============BEGIN %s", #func);           \
//    ret = TEST_##func(); \
//    if (ret == true) ALIPAY_TEST_LOG("SUCCESS: %s", #func); \
//    else ALIPAY_TEST_LOG("FAILURE: %s",#func); \
//    ALIPAY_TEST_LOG("==============END %s", #func); \
//} while(0)
