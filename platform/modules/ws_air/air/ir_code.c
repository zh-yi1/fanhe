#include "asr_config.h"

#if (ASR_SELECT == ASR_WS_AIR && IR_AIR_FUNC)

void isd_pwmr_send_wave(uint16_t* pbuf, uint32_t len, uint32_t frequency, uint32_t times);

AT(.kws_ir.com)
static unsigned short trans_pulse(unsigned short* out, unsigned short ret_num,
    unsigned short frequency, unsigned char frame_count)
{
    unsigned short i;
    int pattern_value;
    double pulse;

    pulse = (double)1000000 / frequency;
    for (i = 0; i < ret_num; i++) {
        pattern_value = pulse * out[i];
        if (pattern_value > 0xFFFF) {
            pattern_value = 0xFFFF;
        }

        out[i] = pattern_value;
    }
    if (frame_count > 1) {
        for (i = 1; i < frame_count; i++) {
            memcpy(&out[ret_num * i], out, ret_num * sizeof(out[0]));
        }
    }
    return ret_num * frame_count;
}


/**
 * @brief 遥控器功能支持信息获取
 * @param [in]  {AIR_STA_T air_ctr}    空调遥控器状态
 * @param [out] {wind_direct_flag}     风向支持
 * @return 0:OK  -1:获取数据失败  -2:不支持的模式  -3:不支持的温度  -4:不支持的风速
 */
AT(.kws_ir.com)
int remote_support_info(AIR_STA_T air_ctr, unsigned char* wind_direct_flag)
{
    isd_read_remote_code(5); //停止语音识别

    int ret = 0;
    int i = 0;

    unsigned char t_max = 0;
    unsigned char t_min = 0;
    unsigned char wind = 0;
    unsigned short info[5] = { 0 };
#ifdef LOG_TEST
    printf("get_ac_info-->_offset: %d\n", SPI_FLASH_DATA_BIN_ADDR + remote_data._offset);
#endif
    ret = wanson_get_ac_info(p_remote_dat, remote_data_len, info, wind_direct_flag);
    if (ret < 0) {
        printf("\nGet info error!!!\n");
        asr_start_stop(1); //打开语音识别
        return -1;
    }
#ifdef LOG_TEST
    printf("\nGet data:wind_direct_flag %d\n", *wind_direct_flag);
    for (int i = 0; i < 5; i++) {
        printf("%02X;", info[i]);
    }
    printf("info mode : %d; %02X\n", air_ctr._mode, info[air_ctr._mode]);
#endif
    if (info[air_ctr._mode] == 0) {
        asr_start_stop(1); //打开语音识别
        return -2;
    }

    t_min = (info[air_ctr._mode] & 0x0F) + 16;
    t_max = ((info[air_ctr._mode] >> 4) & 0x0F) + 16;
    if (air_ctr._temperature < t_min || air_ctr._temperature > t_max || t_min == t_max) {
        asr_start_stop(1); //打开语音识别
        return -3;
    }

    wind = (info[air_ctr._mode] >> 8) & 0x0F;
    for (; i < 4; i++) {
        if ((wind & 0x01) != 1) {
            if (air_ctr._wind_speed == i) {
                asr_start_stop(1); //打开语音识别
                return -4;
            }
        }
        wind = wind >> 1;
    }
    return 0;
}

AT(.kws_ir.com)
int air_direct_ctrl(AIR_STA_T air_ctr, unsigned char wind_direct_flag, unsigned char press_fid)
{
    isd_read_remote_code(5); //停止语音识别

    unsigned short ret_num = 0;
    unsigned char frame_count = 0;
    int ret = 0;
    unsigned char temp_direction = 0;
    temp_direction = air_ctr._wind_direction;

    //如果要设置成局部变量,需要增加 asr task  stack_size
    //获取对应功能脉冲数量
    //需要注意 控制什么功能  pressed_fid 选择对应的案件
    ret = wanson_get_ac_state_pulse(p_remote_dat, remote_data_len,
        air_ctr._power, air_ctr._mode, air_ctr._temperature, air_ctr._wind_speed,
        temp_direction, press_fid,
        &air_ctr._frequency,
        pwm_out, 1024,
        &ret_num, &frame_count);
    if (ret < 0) {
        printf("get_ac_state_pulse error,ret=%d\n", ret);
        asr_start_stop(1); //打开语音识别
        return -2;
    }
    ret_num = trans_pulse(pwm_out, ret_num, air_ctr._frequency, frame_count);
    isd_pwmr_send_wave(pwm_out, ret_num, air_ctr._frequency, 1);

    return 0;
}

/**
 * @brief 空调状态控制
 * @param[in] {AIR_STA_T air_ctr}    空调遥控器状态
 * @param[in] {wind_direct_flag}     风向支持标志位
 * @return 0: OK    -1:没有可用的遥控器，需要先匹配  -2：获取遥控码失败
 */
AT(.kws_ir.com)
int air_control(AIR_STA_T air_ctr, unsigned char wind_direct_flag, unsigned char press_fid)
{
    isd_read_remote_code(5); //停止语音识别

    unsigned short ret_num = 0;
    unsigned char frame_count = 0;
    int ret = 0;
    unsigned char temp_direction = 0;

    temp_direction = air_ctr._wind_direction;

    ret = wanson_get_ac_state_pulse(p_remote_dat, remote_data_len,
        air_ctr._power, air_ctr._mode, air_ctr._temperature, air_ctr._wind_speed, temp_direction, 
        press_fid,
        &air_ctr._frequency, pwm_out, 1024, &ret_num, &frame_count);
        
    if (ret < 0) {
        printf("get_ac_state_pulse error,ret=%d\n", ret);
        asr_start_stop(1); //打开语音识别
        return -2;
    }

    ret_num = trans_pulse(pwm_out, ret_num, air_ctr._frequency, frame_count);
    isd_pwmr_send_wave(pwm_out, ret_num, air_ctr._frequency, 1);

    return 0;
}

#endif //ASR_WS_AIR