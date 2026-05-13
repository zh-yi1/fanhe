#ifndef __IR_H__
#define __IR_H__

#if (ASR_SELECT == ASR_WS_AIR && IR_AIR_FUNC)

/******************************************************************************
                               码库
******************************************************************************/
/**
 * 码库匹配流程枚举
 */
typedef enum
{
    MATCH_OVER = 0,
    MATCH_START_PLAY,
    MATCH_LEARN_START_PLAY,
    MATCH_LEARN_START,
    MATCH_LEARN_SUCCESS_PLAY,
    MATCH_LEARN_FAILURE_PLAY,
    MATCH_LEARN_SUCCESS,
    MATCH_LEARN_FAILURE,
    MATCH_SUCCESS_PLAY,
    MATCH_FAILURE_PLAY,
    MATCH_SUCCESS,
    MATCH_FAILURE,
    MATCH_NEXT_ONE_SUCCESS_PLYA,
    MATCH_NEXT_ONE_FAILURE_PLYA,
} MATCH_REMOTE_FLAG;

/**
 * 码库遥控器索引
 */
typedef struct
{
    unsigned char key[20];
    unsigned char _flag;
    unsigned short _num;     //返回根据传入的电平搜索到的遥控码总套数
    unsigned short _id;      //控码的id
    unsigned short _handle;  //遥控码的句柄
    unsigned short _len;     //返回第一套遥控码数据长度
    int _offset;             //遥控码在数据文件中的偏移量
    unsigned short _cur_num; //遥控码在数据文件中的偏移量

    unsigned short _one_id;     //第一套遥控码的id
    unsigned short _one_handle; //遥控码的句柄
    int _one_offset;            //遥控码在数据文件中的偏移量
    unsigned short _one_len;    //返回第一套遥控码数据长度
} REMOTE_PARA_T;

/**
 * 空调状态
 */
typedef struct
{
    unsigned char key[20];
    unsigned char _power;          //开关 [0 开机， 1 关机]
    unsigned char _mode;           //模式 [0-4]对应制冷，制热，自动，送风，除湿
    unsigned char _temperature;    //温度 [16-30]
    unsigned char _wind_speed;     //风速 [0-3]对应自动，低，中，高
    unsigned char _wind_direction; //风向 上下
    unsigned char _wind_left_right; //左右扫风
    unsigned char _timer; //
    unsigned char _light; //
    unsigned char _super; //

    unsigned short _frequency;   //载波频率
    unsigned char _isd_ble_flag; //ble 发送数据标志
} AIR_STA_T;


/**
 * @brief 遥控器功能支持信息获取
 * @param[in] {AIR_STA_T air_ctr}    空调遥控器状态
 * @param[out] {wind_direct_flag}    风向支持
 * @return 0: OK    -1:获取数据失败 -2：不支持的模式 -3：不支持的温度 -4：不支持的风速
 */
int remote_support_info(AIR_STA_T air_ctr, unsigned char *wind_direct_flag);

/**
 * @brief 空调风向控制
 * @param[in] {AIR_STA_T air_ctr}    空调遥控器状态
 * @param[in] {wind_direct_flag}     风向支持标志位
 * @return 0: OK    -1:没有可用的遥控器，需要先匹配  -2：获取遥控码失败
 */
int air_direct_ctrl(AIR_STA_T air_ctr, unsigned char wind_direct_flag,unsigned char press_fid);

/**
 * @brief 空调状态控制
 * @param[in] {AIR_STA_T air_ctr}    空调遥控器状态
 * @param[in] {wind_direct_flag}     风向支持标志位
 * @return 0: OK    -1:没有可用的遥控器，需要先匹配  -2：获取遥控码失败
 */
int air_control(AIR_STA_T air_ctr, unsigned char wind_direct_flag,unsigned char press_fid);

/**
 * @brief 在码库读取遥控器数据
 * @param [in] {device_type}     设备号
 * @return 0: OK    -1:error
 */
int isd_read_remote_code(unsigned char device_type);

#endif //ASR_WS_AIR

#endif