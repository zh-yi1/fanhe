#include "asr_config.h"

#if (ASR_SELECT == ASR_WS_AIR && IR_AIR_FUNC)

AIR_STA_T air_sta_data;

//空调遥控按键类型
enum {
    AIR_PRESS_ID_NULL,
    AIR_PRESS_ID_POWER,
    AIR_PRESS_ID_MODE,
    AIR_PRESS_ID_TEMP0,
    AIR_PRESS_ID_TEMP1,
    AIR_PRESS_ID_SPEED,
    AIR_PRESS_ID_WIND,
    AIR_PRESS_ID_DERICT,
};

//空调模式
enum {
    AIR_MODE_COOL,
    AIR_MODE_HOT,
    AIR_MODE_AUTO,
    AIR_MODE_FAN,
    AIR_MODE_DRY,
};

//超强 8，定时开 9，定时关 10，灯光 11，辅热 12，左右风向 21，睡眠 22
enum {
    AIR_EXT_FID_SUPER = 8,//超强
    AIR_EXT_FID_TIMER_ON,//定时开
    AIR_EXT_FID_TIMER_OFF,//定时关
    AIR_EXT_FID_LIGHT,//灯光
    AIR_EXT_FID_HOT,//辅热
    AIR_EXT_FID_WIND_LEFT_RIGHT = 21,//左右风向
    AIR_EXT_FID_SLEEP,//睡眠
};


/**
 * @brief 初始化遥控器参数，需要在其他所有初始化完成后再调用该函数
 * @return 0: OK
 */
AT(.kws_ir.com)
void air_remote_data_init()
{
    air_sta_data._power = 0;
    air_sta_data._wind_direction = 1;
    air_sta_data._mode = AIR_MODE_COOL;
    air_sta_data._temperature = 25;
    air_sta_data._wind_speed = 1;
    air_sta_data._light = 1;
    air_sta_data._super = 0;
    printf("air para init Ok!!!\n");
}


/**
 * @brief 打开空调
 * @return none
 */
AT(.kws_ir.com)
int event_air_power_on()
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._power = 0;
    ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_POWER);
    if (ret < 0) {
        printf("air_control error\n");
        return -2;
    }
    air_sta_data = temp;
    return 0;
}

/**
 * @brief 关闭空调
 * @return none
 */
AT(.kws_ir.com)
int event_air_power_off()
{
    int ret = 0;
    AIR_STA_T temp = air_sta_data;
    temp._power = 1;
    ret = air_control(temp, 0, AIR_PRESS_ID_POWER);
    if (ret < 0) {
        return ret;
    }
    air_sta_data = temp;
    return 0;
}

/**
 * @brief 开关空调
 * @return none
 */
AT(.kws_ir.com)
int event_air_power_onoff(int power)
{
    int ret = 0;
    AIR_STA_T temp = air_sta_data;
    temp._power = power;
    ret = air_control(temp, 0, AIR_PRESS_ID_POWER);
    if (ret < 0) {
        return ret;
    }
    air_sta_data = temp;
    return 0;
}

/**
 * @brief 制冷模式
 * @return none
 */
AT(.kws_ir.com)
int event_air_mode_cool()
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._mode = AIR_MODE_COOL;// 0;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -2) {
        return -1;
    }

    ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_MODE);
    if (ret < 0) {
        return -2;
    }
    air_sta_data = temp;
    return 0;
}

/**
 * @brief 加热模式
 * @return none
 */
AT(.kws_ir.com)
int event_air_mode_hot()
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._mode = AIR_MODE_HOT;//1;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -2) {
        return -1;
    }
    //ws add
    if(ret == -4) {
        printf("不支持的风速，风速调到最小，再次获取...\n");
        temp._wind_speed = 1;
        remote_support_info(temp, &wind_direct_flag);
    }

    ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_MODE);
    if (ret < 0) {
        return -2;
    }
    air_sta_data = temp;
    return 0;
}

/**
 * @brief 自动模式
 * @return none
 */
AT(.kws_ir.com)
int event_air_mode_auto()
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._mode = AIR_MODE_AUTO;//2;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -2) {
        return -1;
    }

    ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_MODE);
    if (ret < 0) {
        return -2;
    }
    air_sta_data = temp;
    return 0;
}

/**
 * @brief 送风模式
 * @return none
 */
AT(.kws_ir.com)
int event_air_mode_wind()
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._mode = AIR_MODE_FAN;//3;
    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -2) {
        return -1;
    }

    ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_MODE);
    if (ret < 0) {
        return -2;
    }
    air_sta_data = temp;
    return 0;
}

/**
 * @brief 除湿模式
 * @return none
 */
AT(.kws_ir.com)
int event_air_mode_wetted()
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._mode = AIR_MODE_DRY;//4;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -2) {
        return -1;
    }
    //ws add
    if(ret == -4) {
        printf("不支持的风速，风速调到最小，再次获取...\n");
        temp._wind_speed = 1;
        remote_support_info(temp, &wind_direct_flag);
    }

    ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_MODE);
    if (ret < 0) {
        return -2;
    }
    air_sta_data = temp;
    return 0;
}

/**
 * @brief 设置模式
 * @return none
 */
AT(.kws_ir.com)
int event_air_mode_set(int mode)
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;

    temp._mode = mode;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -2) {
        return -1;
    }

    ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_MODE);
    if (ret < 0) {
        return -2;
    }
    air_sta_data = temp;
    return 0;
}

/**
 * @brief 空调温度设置
 * @param[in] {value}    温度[16-30]
 * @return 0: OK    1: out of range  -1: Can't adjust
 */
AT(.kws_ir.com)
int event_air_temperature_set(unsigned char value)
{
    printf("tmpr= %d\n", value);
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;

    if (value >= 16 && value <= 30) {
        temp._temperature = value;

        ret = remote_support_info(temp, &wind_direct_flag);
        if (ret == -3) {
            return -1;
        }

        ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_NULL);
        if (ret < 0) {
            return -2;
        }
        air_sta_data = temp;
    } else {
        printf("\nTemperature out of range\n");
        ret = 1;
    }
    return ret;
}

/**
 * @brief 自动风量
 * @return none
 */
AT(.kws_ir.com)
int event_air_wind_speed_auto()
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._wind_speed = 0;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -4) {
        return -1;
    }

    ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_SPEED);
    if (ret < 0) {
        return -2;
    }
    air_sta_data = temp;

    return ret;
}

/**
 * @brief 最小风量
 * @return none
 */
AT(.kws_ir.com)
int event_air_wind_speed_min()
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._wind_speed = 1;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -4) {
        return -1;
    }

    ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_SPEED);
    if (ret < 0) {
        return ret;
    }
    air_sta_data = temp;

    return ret;
}

/**
 * @brief 中等风量
 * @return none
 */
AT(.kws_ir.com)
int event_air_wind_speed_mid()
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._wind_speed = 2;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -4) {
        return -1;
    }

    ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_SPEED);
    if (ret < 0) {
        return ret;
    }
    air_sta_data = temp;

    return ret;
}

/**
 * @brief 最大风量
 * @return none
 */
AT(.kws_ir.com)
int event_air_wind_speed_max()
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._wind_speed = 3;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -4) {
        return -1;
    }

    ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_SPEED);
    if (ret < 0) {
        return ret;
    }
    air_sta_data = temp;

    return ret;
}

/**
 * @brief 设置风量
 * @return none
 */
AT(.kws_ir.com)
int event_air_wind_speed_set(char speed)
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._wind_speed = speed;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -4) {
        return -1;
    }

    ret = air_control(temp, wind_direct_flag, AIR_PRESS_ID_SPEED);
    if (ret < 0) {
        return ret;
    }
    air_sta_data = temp;

    return ret;
}

/**
 * @brief 上下扫风
 * @return none
 */
AT(.kws_ir.com)
int event_air_mode_direction()
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._wind_direction = 0;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -1 || wind_direct_flag == 0) {
        return -1;
    }
    //ret = air_control(temp, wind_direct_flag,AIR_PRESS_ID_WIND);
   // if(wind_direct_flag==1)
    ret = air_direct_ctrl(temp, wind_direct_flag, AIR_PRESS_ID_WIND);
    //    else
    //        ret = air_direct_ctrl(temp, wind_direct_flag,AIR_PRESS_ID_DERICT);
    if (ret < 0) {
        return -2;
    }
    air_sta_data = temp;

    return ret;
}

/**
 * @brief 停止扫风
 * @return none
 */
AT(.kws_ir.com)
int event_air_mode_stop_direction()
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._wind_direction = 1;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -1 || wind_direct_flag == 0) {
        return -1;
    }
    //ret = air_control(temp, wind_direct_flag,AIR_PRESS_ID_DERICT);
    ret = air_direct_ctrl(temp, wind_direct_flag, AIR_PRESS_ID_DERICT);
    if (ret < 0) {
        return -3;
    }
    air_sta_data = temp;

    return ret;
}

/**
 * @brief 上下扫风设置
 * @return none
 */
AT(.kws_ir.com)
int event_air_wind_direction_set(unsigned char wind)
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;
    temp._wind_direction = wind;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -1 || wind_direct_flag == 0) {
        return -1;
    }
    if (temp._wind_direction == 0)
        ret = air_direct_ctrl(temp, wind_direct_flag, AIR_PRESS_ID_WIND);
    else
        ret = air_direct_ctrl(temp, wind_direct_flag, AIR_PRESS_ID_DERICT);
    if (ret < 0) {
        return -3;
    }

    air_sta_data = temp;

    return ret;
}


//==========================================================================================
/**
 * @brief 设置定时
 * @return none
 */
 //==========================================================================================
uint8_t air_time_set_flag;
uint32_t air_time_set_tick;
uint16_t air_set_time;

uint8_t air_time_search_flag;
uint32_t air_time_search_tick;
uint16_t air_search_time;

/**
 * @brief 空调计时重置
 * @param part 1:定时关机； 2:搜索空调
 */
AT(.kws_ir.com)
void air_time_clear(uint8_t part)
{
    if(part == AIR_OFF){
        // air_time_set_tick = 0;
        air_time_set_tick = tick_get();
    }
    else if(part == AIR_SEARCH){
        // air_time_search_tick = 0;
        air_time_search_tick = tick_get();
    }
}

/**
 * @brief 空调定时开始
 * @param part 1:定时关机； 2:搜索空调
 */
AT(.kws_ir.com)
void air_time_start(uint8_t part, uint8_t en)
{
    air_time_clear(part);
    if(part == AIR_OFF) {
        air_time_set_tick = tick_get();
        // air_time_set_tick = 0;
        air_time_set_flag = en;

        printf("air timeout start %d...\n", en);
    }
    else if(part == AIR_SEARCH) {
        air_time_search_tick = tick_get();
        // air_time_search_tick = 0;
        air_time_search_flag = en;

        printf("air time search start %d...\n", en);
    }
}

/**
 * @brief 达到定时时间
 * @param part 1:定时关机； 2:搜索空调
 * @param timeOut 秒
 * @return int
 */
AT(.kws_ir.com)
int air_is_timeOut(uint8_t part, uint32_t timeOut)
{
    int ret = 0;
    if(part == AIR_OFF) {
        // ret = air_time_set_tick > timeOut ? 1 : 0;
        ret = tick_check_expire(air_time_set_tick, timeOut*1000);   //毫秒级
    }
    else if(part == AIR_SEARCH) {
        // ret = air_time_search_tick > timeOut ? 1 : 0;
        ret = tick_check_expire(air_time_search_tick, timeOut*1000); //毫秒级
    }
    return ret;
}

AT(.kws_ir.com)
void air_time_process()
{
    if (air_time_set_flag && air_is_timeOut(AIR_OFF, air_set_time)) {
        air_time_set_flag = 0;
        air_time_clear(AIR_OFF);

        event_air_power_off();

        printf("air timeout close...\n");
    }
    if (air_time_search_flag && air_is_timeOut(AIR_SEARCH, AIR_SEARCH_TIME)) {
        air_time_search_flag = 0;
        air_time_clear(AIR_SEARCH);

        bsp_asr_tone_play(RES_BUF_ASR_33_MP3, RES_LEN_ASR_33_MP3);
        // air_match_result_flashing(EFFECT_SEARCH);
        
        extern u8 mp3_end_oper;
        mp3_end_oper = 3;
        // isd_get_next_remote();

        printf("air search timeout cut...\n");
    }
}

AT(.kws_ir.com)
int event_air_timer_set(unsigned int timer)
{
    if(timer == 0) {
        air_time_start(AIR_OFF, 0);
    }
    else{
        air_set_time = timer*10*60; //秒
        air_time_clear(AIR_OFF);
        air_time_start(AIR_OFF, 1);
    }
    return 0;
}




//==========================================================================================
/**
 * @brief 设置灯光
 * @return none
 */
AT(.kws_ir.com)
int event_air_light_set(int light)
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;

    temp._light = light;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -2) {
        return -1;
    }

    // ret = air_control_ext(temp, wind_direct_flag,AIR_EXT_FID_LIGHT,light);
    if (ret < 0) {
        return -2;
    }
    air_sta_data = temp;
    return 0;
}

/**
 * @brief 设置超强
 * @return none
 */
AT(.kws_ir.com)
int event_air_super_set(int super)
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;

    temp._super = super;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -2) {
        return -1;
    }
    // ret = air_control_ext(temp, wind_direct_flag,AIR_EXT_FID_SUPER,super);
    if (ret < 0) {
        return -2;
    }
    air_sta_data = temp;
    return 0;
}

/**
 * @brief 设置左右扫风
 * @return none
 */
AT(.kws_ir.com)
int event_air_wind_left_right_set(int wind)
{
    int ret = 0;
    unsigned char wind_direct_flag = 0;
    AIR_STA_T temp = air_sta_data;

    temp._wind_left_right = wind;

    ret = remote_support_info(temp, &wind_direct_flag);
    if (ret == -2) {
        return -1;
    }
    // ret = air_control_ext(temp, wind_direct_flag,AIR_EXT_FID_WIND_LEFT_RIGHT,wind);
    if (ret < 0) {
        return -2;
    }
    air_sta_data = temp;
    return 0;
}

#endif //AIR_WS_AIR