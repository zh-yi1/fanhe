#ifndef __WANSON_IR_H__
#define __WANSON_IR_H__

#if (ASR_SELECT == ASR_WS_AIR && IR_AIR_FUNC)

#ifdef _MSC_VER
#ifdef DLL_EXPORTS
#define SO_PUBLIC __declspec(dllexport)
#elif defined(DLL_IMPORTS)
#define SO_PUBLIC __declspec(dllimport)
#else
#define SO_PUBLIC
#endif
#else
#ifdef _HIDE_PRIVATE_FUNC_
#define SO_PUBLIC __attribute__ ((visibility("default")))
#else
#define SO_PUBLIC
#endif
#endif
 

#include "include.h"

#ifdef __cplusplus
extern "C" {
#endif

// 得到状态码空调遥控器(码值由当前的状态动态组合而成)状态支持情况，支持的模式，风向，每个模式支持的温度和风速范围
// data遥控器数据首地址, data_len遥控器数据总长度
// mode_flags 长度至少包含5个uint16_t元素，0-4一共5个uint16_t分别对应制冷，制热，自动，送风，除湿模式的信息
// mode_flags的每个uint16_t包含信息的规则为：
// 为0则表示不支持该模式，从低到高0-3bit存储该模式支持的温度最小值与16的差(制冷模式为例：(mode_flags[0]&0x0F)+16),
// 从低到高4-7bit存储该模式支持的温度最大值与16的差(制冷模式为例：((mode_flags[0]>>4)&0x0F)+16)
// 最小最大值都为0则表示不支持温度调节(制冷模式为例：(mode_flags[0]&0xFF)==0)
// 从低到高8-11bit每bit分别对应风速自动，低，中，高，为1则表示支持该风速，为0则表示不支持
// (制冷模式为例：((mode_flags[0]>>8)&1) 自动， ((mode_flags[0]>>9)&1) 低，((mode_flags[0]>>10)&1) 中，((mode_flags[0]>>11)&1) 高)
// wind_direct_flag 一个字节长度，返回遥控器上下风向支持情况，*wind_direct_flag = 0 表示不支持上下风向调节(get_ac_state_pulse接口wind_direct传-1），
// *wind_direct_flag = 1表示上下风向只有一组码，发这组码空调的上下风向在1-n个状态之间来回切换(get_ac_state_pulse接口wind_direct传1），
// 除此之外则表示空调上下风向有多组码（码值组数根据码组序号最大值和最小值确定），每组码对应空调上下风向的一个状态，0-3bit返回风向码组序号的最小值（(*wind_direct_flag) & 0x0F, 从0开始，0表示发该组码空调进入扫风状态），
// 4-7bit返回风向码组序号的最大值（((*wind_direct_flag)>>4) & 0x0F),(get_ac_state_pulse接口wind_direct传想发送的码组序号即可，比如传0返回扫风的码组）
//
// 返回0成功，-1 data指向的数据不合法
SO_PUBLIC int32_t wanson_get_ac_info(const uint8_t *data, uint16_t data_len,
    uint16_t *mode_flags, uint8_t *wind_direct_flag);

// 得到指定状态的红外脉冲数据, data遥控器数据缓冲区首地址, data_len data缓冲区中有效字节数,
// power 开关机[0 开机， 1 关机], mode 模式[0-4对应制冷，制热，自动，送风，除湿],
// temperature 温度[16-30]，不支持调节温度传-1，wind_speed 风速[0-3对应自动，低，中，高]，不支持调节风量传-1，
// wind_direct 上下风向状态值，请参考get_ac_info接口中wind_direct_flag参数的说明
// pressed_fid 对应用户使用物理遥控器当前按压的按键id[电源 1，模式 2，温度+ 3，温度- 4，风量 5，扫风 6，定向风 7，不确定则传0]，frequency返回频率比如38000，
// out 调用方准备好的返回高低电平缓冲区（单位：脉冲个数），out_max_num out允许存放的最大元素个数(out指向的可用内存空间/sizeof(uint16_t))，
// ret_num 编码完成后在out内存空间中实际写了多少个uint16_t元素，frame_count 通过out返回的高低电平数组（一帧）需要重复的次数
//
// 返回值 0 成功，-1 data指向的数据不合法，-2 power 参数取值不在[01]范围内, -3 mode 参数取值不在[0-4]范围内， -4 遥控器不支持mode参数指定的模式
// -99 out缓冲区空间不足
// 注意： temperature和win_speed参数会自动使用最近的有效值，比如temperature传入15则会当16处理，传入31则会当30处理
// wind_speed传入4会当3处理
SO_PUBLIC int32_t wanson_get_ac_state_pulse(const uint8_t *data, uint16_t data_len,
    uint8_t power, uint8_t mode, int8_t temperature, int8_t wind_speed,
    int8_t wind_direct, int8_t pressed_fid, uint16_t *frequency, uint16_t *out,
    uint16_t out_max_num, uint16_t *ret_num, uint8_t *frame_count);

// 根据学习到的红外高低电平宽度数组(单位us)搜索码库
// data 索引数据首地址, data_len索引数据总长度
// pattern 学习到的红外高低电平宽度数组(单位us), pattern_num pattern缓冲区内高低电平个数
// device_type 搜索的遥控器设备类型，5 空调， 8 风扇
// remote_num 返回根据传入的电平搜索到的遥控码总套数，remote_handle 返回第一套遥控码的句柄（用来获取下一套，不需要取下一套则传NULL）
// remote_id 第一套遥控码的id,不需要则传NULL， remote_data_offset 返回第一套遥控码在数据文件中的偏移量
// remote_data_len返回第一套遥控码数据长度
//
// 返回值 0 成功， -1 索引数据错误， -2 红外数据无法识别，-3 索引文件找不到匹配的遥控码, -4 device_type参数值不正确
SO_PUBLIC int32_t wanson_search_remote(const uint8_t *data, uint16_t data_len,
    int32_t *pattern, int32_t pattern_num, uint8_t device_type,
    uint16_t *remote_num, uint16_t *remote_handle, uint16_t *remote_id,
    int32_t *remote_data_offset, uint16_t *remote_data_len);

// 根据电器品牌ID搜索码库
// data 索引数据首地址, data_len索引数据总长度
// brand_id 品牌唯一标识ID
// remote_num 返回根据传入的电平搜索到的遥控码总套数，remote_handle 返回第一套遥控码的句柄（用来获取下一套，不需要取下一套则传NULL）
// remote_id 第一套遥控码的id,不需要则传NULL， remote_data_offset 返回第一套遥控码在数据文件中的偏移量
// remote_data_len返回第一套遥控码数据长度
//
// 返回值 0 成功， -1 索引数据错误， -2 品牌ID不存在
SO_PUBLIC int32_t wanson_get_remote_by_brand(const uint8_t *data, uint16_t data_len,
    uint16_t brand_id, uint16_t *remote_num, uint16_t *remote_handle,
    uint16_t *remote_id, int32_t *remote_data_offset, uint16_t *remote_data_len);

// 遍历码库
// data 索引数据首地址, data_len索引数据总长度
// remote_num 返回遍历码库索引文件遥控码总套数，remote_handle 返回第一套遥控码的句柄（用来获取下一套，不需要取下一套则传NULL）
// remote_id 第一套遥控码的id,不需要则传NULL， remote_data_offset 返回第一套遥控码在数据文件中的偏移量
// remote_data_len返回第一套遥控码数据长度
//
// 返回值 0 成功， -1 索引数据错误
SO_PUBLIC int32_t wanson_get_remote_one_by_one(const uint8_t *data, uint16_t data_len,
    uint16_t *remote_num, uint16_t *remote_handle, uint16_t *remote_id,
    int32_t *remote_data_offset, uint16_t *remote_data_len);

// 根据当前遥控器的句柄获取下一套遥控器（调用者请注意总套数，调用次数不要越界）
// data 索引数据首地址, data_len索引数据总长度
// cur_remote_handle 当前遥控器的句柄，next_remote_handle 返回下一套遥控码的句柄（不需要则传NULL）
// remote_id 下一套遥控码的id（不需要则传NULL），remote_data_offset 返回下一套遥控码在数据文件中的偏移量
// remote_data_len返回下一套遥控码数据长度
//
// 返回值 0 成功， -1 索引数据错误
SO_PUBLIC int32_t wanson_get_next_remote(const uint8_t *data, uint16_t data_len,
    uint16_t cur_remote_handle, uint16_t *next_remote_handle,
    uint16_t *remote_id, int32_t *remote_data_offset, uint16_t *remote_data_len);

// 根据当前遥控器的句柄获取上一套遥控器（调用者请注意总套数，调用次数不要越界）
// data 索引数据首地址, data_len索引数据总长度
// cur_remote_handle 当前遥控器的句柄，pre_remote_handle 返回上一套遥控码的句柄（不需要则传NULL）
// remote_id 上一套遥控码的id（不需要则传NULL），remote_data_offset 返回上一套遥控码在数据文件中的偏移量
// remote_data_len返回上一套遥控码数据长度
//
// 返回值 0 成功， -1 索引数据错误
SO_PUBLIC int32_t wanson_get_pre_remote(const uint8_t *data, uint16_t data_len,
    uint16_t cur_remote_handle, uint16_t *pre_remote_handle,
    uint16_t *remote_id, int32_t *remote_data_offset, uint16_t *remote_data_len);

// 得到状态码空调之外的固定按键码遥控器（一个按键对应一个固定的红外码值）信息
// data遥控器数据首地址, data_len遥控器数据总长度,
// device_type 搜索的遥控器设备类型，2 电视， 8 风扇
// flag_buf 用来存放每个功能的红外码标识，一个字节对应一个fid功能，fid减一对应下标值
// flag_buf_size flag_buf一共有多少个有效字节数，电视最少12，风扇最少5
// 分设备类型说明如下：
// 电视  device_type = 2
// fid功能按鍵id，定义如下
// 电源 1,音量加减 2,静音 3,上 4,下 5,左 6,7 右,8 OK/确定,9 信号源,10 主页,11 返回,12 退出
// power_flag返回1表示开关机同一个码，返回2表示开关机分开2个码
//
// volume_flag返回0表示无音量调节功能，返回2表示音量+和音量-2个码
//
// mute_flag返回0表示无静音功能，返回1表示静音只有一个码
//
// navigate_up_flag返回0表示无上键，返回1表示上键只有一个码
//
// navigate_down_flag返回0表示无下键，返回1表示下键只有一个码
//
// navigate_left_flag返回0表示无左键，返回1表示左键只有一个码
//
// navigate_right_flag返回0表示无右键，返回1表示右键只有一个码
//
// ok_flag返回0表示无ok键，返回1表示ok键只有一个码
//
// input_flag返回0表示无信号源切换功能，返回1表示信号源键只有一个码，返回2表示信号源+和信号源-2个码
//
// homepage_flag返回0表示无主页键，返回1表示主页键只有一个码
//
// back_flag返回0表示无返回键，返回1表示返回键只有一个码
//
// exit_flag返回0表示无退出键，返回1表示退出键只有一个码
//
// 风扇  device_type = 8
// fid功能按鍵id，定义如下
// 电源 1,风速 2,摇头 3,定时 4,模式 5
// power_flag返回1表示开关机同一个码，返回2表示开关机分开2个码
//
// speed_flag返回0表示无风速调节功能，返回1表示风速只有一个码，返回2表示风速+和风速-2个码，
// 返回3表示风速分低中高或者1,2,3之类的档位（码值不同），此时调用get_fan_pulse需要传入state参数
// 用来保存每次获取到风速相关的第几个码，用来下次获取下一个码。响应风速+顺序为（低->中->高或者1->2->3)
// 响应风速-顺序相反
//
// swing_flag返回0表示无摇头功能，返回1表示摇头只有一个码，返回2表示摇头和停止摇头2个码
//
// timer_flag返回0表示无定时功能，返回1表示定时只有一个码，返回2表示定时+和定时-2个码
// 返回3表示定时分定时1,3,6之类的档位（码值不同），此时调用get_fan_pulse需要传入state参数
// 用来保存每次获取到定时相关的第几个码，用来下次获取下一个码。响应定时+顺序为（1->2->3之类)
// 响应定时-顺序相反
//
// mode_flag返回0表示无模式功能，返回1表示模式只有一个码
//
// 返回0成功，-1 data指向的数据不合法， -2 不支持的device_type，-3 flag_buf_size太小
SO_PUBLIC int32_t wanson_get_other_remote_info(const uint8_t *data, uint16_t data_len,
    uint8_t device_type, uint8_t *flag_buf, uint8_t flag_buf_size);

// 得到得到状态码空调之外的固定按键码遥控器的红外脉冲数据
// data遥控器数据首地址, data_len遥控器数据总长度,
// device_type 搜索的遥控器设备类型，2 电视， 8 风扇
//
// fid功能按鍵id，定义如下
// 电视：电源 1,音量加减 2,静音 3,上 4,下 5,左 6,7 右,8 OK/确定,9 信号源,10 主页,11 返回,12 退出
// 风扇：电源 1,风速 2,摇头 3,定时 4,模式 5
//
// direct 1 向上方向比如风速+，开机，定时+，音量+，信号源+， 0 向下方向比如风速-，关机，定时-，音量-，信号源-
// get_other_remote_info接口对应flag=1时传0或者1任何值都可
// get_other_remote_info接口对应flag=2或者3时需要传入0或者1
//
// loop 是否循环，比如风速低中高之后下次调用是否返回低 。 0 不循环， 1 循环
// get_other_remote_info接口对应flag=1或者2时传0或者1任何值都可
// get_other_remote_info接口对应flag=3时根据具体应用需要传0或者1控制是否循环
//
// press_count该套遥控码当前是第几次按键（用来处理翻转码）， frequency返回频率比如38000，
// out 调用方准备好的返回高低电平缓冲区（单位：脉冲个数），
// out_max_num out允许存放的最大元素个数(out指向的可用内存空间/sizeof(uint16_t))，
// ret_num 编码完成后在out 内存空间中实际写了多少个uint16_t元素，
// frame_count 通过out返回的高低电平数组（一帧）需要重复的次数
// state用来处理flag=3的情况，
// 第一次调用state传0即可，后续每次调用传入上次调用返回的值即可,flag不为3可以传NULL
//
// 返回0成功，-1 data指向的数据不合法，-2 找不到fid传入的键对应的码值, -3 flag为3的时候state传递了NULL，-4 不支持的device_type
SO_PUBLIC int32_t wanson_get_other_remote_pulse(const uint8_t *data, uint16_t data_len,
    uint8_t device_type, int32_t fid, uint8_t direct, uint8_t loop,
    uint8_t press_count, uint16_t *frequency, uint16_t *out,
    uint16_t out_max_num, uint16_t *ret_num, uint8_t *frame_count,
    uint8_t *state);

// 返回版本号，递增的整数，从1开始
SO_PUBLIC int32_t wanson_get_version();

#ifdef __cplusplus
}
#endif

#endif //ASR_WS_AIR

#endif // wanson_ir
