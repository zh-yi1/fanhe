#ifndef __FUNC_API_AIR_H_
#define __FUNC_API_AIR_H_

#if (ASR_SELECT == ASR_WS_AIR && IR_AIR_FUNC)

/**
 * @brief 初始化遥控器参数，需要在其他所有初始化完成后再调用该函数 
 */
void air_remote_data_init();

/**
 * @brief 匹配空调遥控器
 * @return 0: OK    -1:error
 */
int match_remote_process(void);


/**
 * @brief 切换空调遥控器
 * @return 0: OK    -1:error
 */
int event_next_air_remote();

/**
 * @brief 打开空调
 * @return none
 */

int event_air_power_on();
 
/**
 * @brief 关闭空调
 * @return none
 */
int event_air_power_off();

/**
 * @brief 制冷模式
 * @return none
 */
int event_air_mode_cool();

/**
 * @brief 加热模式
 * @return none
 */
int event_air_mode_hot();

/**
 * @brief 自动模式
 * @return none
 */
int event_air_mode_auto();

/**
 * @brief 送风模式
 * @return none
 */
int event_air_mode_wind();

/**
 * @brief 除湿模式
 * @return none
 */
int event_air_mode_wetted();

/**
 * @brief 空调温度设置
 * @param[in] {value}    温度[16-30]
 * @return 0: OK    1: out of range  -1: Can't adjust
 */
int event_air_temperature_set(unsigned char value);

/**
 * @brief 调高温度
 * @return 0: OK    1: max  -1: Can't adjust
 */
int event_air_temperature_increase();

/**
 * @brief 调低温度
 * @return 0: OK    1: min  -1: Can't adjust
 */
int event_air_temperature_reduce();

/**
 * @brief 自动风量
 * @return none
 */
int event_air_wind_speed_auto();

/**
 * @brief 最小风量
 * @return none
 */
int event_air_wind_speed_min();

/**
 * @brief 中等风量
 * @return none
 */
int event_air_wind_speed_mid();

/**
 * @brief 最大风量
 * @return none
 */
int event_air_wind_speed_max();

/**
 * @brief 上下扫风
 * @return none
 */
int event_air_mode_direction();

/**
 * @brief 停止扫风
 * @return none
 */
int event_air_mode_stop_direction();

int event_air_timer_set(unsigned int timer);

#endif //ASR_WS_AIR

#endif