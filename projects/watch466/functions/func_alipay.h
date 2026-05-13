#ifndef __FUNC_GALIPAY_H
#define __FUNC_GALIPAY_H

#include "include.h"
#include "func.h"
#if SECURITY_PAY_EN


#include "alipay_common.h"
#include "alipay_bind.h"
#include "alipay_pay.h"
#include "hs_private.h"
#include "hed_private.h"
#include "iotsec.h"
#include "list.h"
#include "alipay_account_manage.h"
#include "aid_code.h"
#include "vendor_api.h"
#define ALIPAY_DETECT_TIME      (1)                        //1使用500ms检测, 2使用1秒检测
#define ALIPAY_BIND_TIMEOUT     (30*ALIPAY_DETECT_TIME)    //绑定超时时间                     单位：s
#define ALIPAY_FAIL_TIMEOUT     (8*ALIPAY_DETECT_TIME)     //失败界面超时时间                 单位：s
#define ALIPAY_SUCCESS_TIMEOUT  (2*ALIPAY_DETECT_TIME)     //绑定成功界面展示时间             单位：s
#define ALIPAY_LOGO_TIMEOUT     (1)                        //支付宝logo界面展示时间           单位：500ms
#define ALIPAY_API_RETRY_CNT    (5)                        //支付宝信息接口获取失败重试次数
#define ALIPAY_CODE_REFRESH_TIMEOUT     (60)               //支付宝支付二维码与条形码刷新时间间隔     单位：s

void func_alipay_form_create_by_page(u8 page_sta);



#endif // SECURITY_PAY_EN

#endif // __FUNC_GALIPAY_H
