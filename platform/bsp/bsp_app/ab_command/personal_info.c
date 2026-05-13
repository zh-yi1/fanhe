#include "include.h"
#include "ab_common.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if (USE_APP_TYPE == APP_BLUE_FIT)

///个人信息类
//0x70:个人信息
void user_info_get(void *user)
{
    memcpy(user, &app_data.sector0.userinfo, 4);
//    TRACE("get-user_sex[%d] user_age[%d] user_height[%d] user_weight[%d]\r\n", user_t->user_sex, user_t->user_age, user_t->user_height, user_t->user_weight);
}

void user_info_set(void *user)
{
//    print_r(user, 4);
    memcpy(&app_data.sector0.userinfo, user, 4);
    printf("set-user_sex[%d] user_age[%d] user_height[%d] user_weight[%d]\r\n",
           app_data.sector0.userinfo.user_sex, app_data.sector0.userinfo.user_age, app_data.sector0.userinfo.user_height, app_data.sector0.userinfo.user_weight);

#if  (SENSOR_STEP_SEL == SENSOR_STEP_SC7A20)
    u8 user_info_para[4] = {0};
    user_info_para[0] = app_data.sector0.userinfo.user_height;
    user_info_para[1] = app_data.sector0.userinfo.user_weight;
    user_info_para[2] = app_data.sector0.userinfo.user_age;
    user_info_para[3] = app_data.sector0.userinfo.user_sex;
    if ((user_info_para[0] >= 0x1e && user_info_para[0] <= 0xfa) &&
        (user_info_para[1] >= 0x0a && user_info_para[1] <= 0xc8) &&
        (user_info_para[2] >= 0x03 && user_info_para[2] <= 0x96)) {
//        SL_Pedo_Person_Inf_Init(&user_info_para[0]);
//        printf("user info height[%d] weight[%d] age[%d] sex[%d]\r\n",
//              user_info_para[0], user_info_para[1], user_info_para[2], user_info_para[3]);
    }
#endif
}

#endif
