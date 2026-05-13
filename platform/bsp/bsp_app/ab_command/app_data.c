#include "include.h"
#include "ab_common.h"

#if (USE_APP_TYPE == APP_BLUE_FIT)
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
AT(.qr_buff)
#endif
app_data_t app_data AT(.unuse); //unuse

void sector0_var_init(void)
{
    //开关
    app_data.sector0.sys_sw.sw.wrist = 1;
    app_data.sector0.sys_sw.sw.sleep = 1;
    app_data.sector0.sys_sw.sw.notification = 1;
    app_data.sector0.sys_sw.sw.notify_screen_light = 1;
    app_data.sector0.sys_sw.sw.shake = 1;
    app_data.sector0.sys_sw.sw.message_vibration = 1;
    app_data.sector0.sys_sw.sw.sound = 1;

    //用户信息
    app_data.sector0.userinfo.user_sex    = 1;
    app_data.sector0.userinfo.user_age    = 24;
    app_data.sector0.userinfo.user_weight = 65;
    app_data.sector0.userinfo.user_height = 180;

    //表盘市场
//    app_data.sector0.watch_diy_cb.flag = false;
//    app_data.sector0.watch_diy_cb.addr = FLASH_MARKET_DIAL_ADDR;
}

void sector1_var_init(void)
{
    //gsensor相关变量初始化
//    sl_var_init();

    app_data.sector1.time_format = 1;                       //24h制
    app_data.sector1.password = 0xffff;

    //睡眠监测
//    app_data.sector1.sleep_check.end_hour = 21;
//    app_data.sector1.sleep_check.end_minute = 0;
//    app_data.sector1.sleep_check.end_hour = 9;
//    app_data.sector1.sleep_check.end_minute = 0;
    app_data.sector1.sleep_check.all_day = 1;

    app_data.sector1.sleep_time = 0xffffffff;

    //app_data.sector1.factory_mack = true;                 //开机进入语言设置界面
}
#endif
