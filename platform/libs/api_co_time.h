#ifndef _API_COTIME_H
#define _API_COTIME_H

#include "include.h"

//CO timer
typedef enum
{
    /// Only run once
    TIMER_ONE_SHOT,
    /// Repeat until stop it
    TIMER_REPEAT,
}co_timer_mode_t;

typedef enum
{
    /// Only run once
    LEVEL_LOW_PRI,
    /// Repeat until stop it
    LEVEL_HIGH_PRI,
}co_timer_level_t;


typedef struct _co_timer_t {
    struct _co_timer_t *next;
    co_timer_mode_t mode;
    uint32_t time_sorce;
    uint32_t time_out;
    uint32_t en : 1;
	uint32_t is_sleep_run : 1;
    void *func_cb;
    void *param;
}co_timer_t;


typedef void (*co_timer_callback_t)(co_timer_t *timer, void *param);

/**
 * @brief 创建软定时器
 * @param[in] timer : timer句柄 (必须是静态或全局)
 * @param[in] delay : 1ms为单位
 * @param[in] mode  : 执行一次或循环
 * @param[in] level : timer优先级，
                      0在直接在main线程执行
                      1在timer5ms线程执行（如果处理时间长，且处理的代码没在公共区，需要发消息到其他线程处理）
 * @param[in] callback: 回调函数
 * @param[in] param  :传参指针，默认为NUL
 *
 * @return false: 失败
 *         true: 成功
 **/
bool co_timer_set(co_timer_t *timer, uint32_t delay, co_timer_mode_t mode, co_timer_level_t level,
        co_timer_callback_t callback, void *param);

/**
 * @brief 删除软定时器函数
 *
 * @param[in] timer: 软定时器句柄
 *         
 **/
void co_timer_del(co_timer_t *timer);

/**
 * @brief 重置软定时器时间
 * @param[in] timer : 软定时器句柄
 *
 **/
bool co_timer_reset(co_timer_t *timer);

/**
 * @brief 是否使能定时器
          co_timer_set后默认就是使能状态
 * @param[in] timer : 软定时器句柄
   @param[in] en : true打开软定时器  false关闭软定时器
 *
 **/
bool co_timer_enable(co_timer_t *timer, bool en);

/**
 * @brief 设置对应的软定时器是否可以在sleep模式下运行
 *
 * @param[in] timer: 定时器初始化的句柄
 * @param[in] is_acitve: sleep下是否不退出休眠也执行定时器；
 * @return false:设置失败
 *         true: 设置成功
 **/
bool co_timer_set_sleep(co_timer_t *timer, bool is_acitve);

/**
 * @判断软定时器的handle是否有效
 * 设置为TIMER_ONE_SHOT后，被执行后也会返回false
 * @输入[in] 定时器初始化的句柄
 *
 **/
bool co_timer_handle_is_valid(co_timer_t *timer);

/**
 * @brief 设置软定时器最大数量
 * 默认是100个，即不调用这个函数之前，默认是100
 * @param[in] 最大数量
 *
 **/
void co_timer_max_num_set(u8 num);

/**
 * @brief 轮询运行timer函数
 *        
 **/
bool co_timer_pro(u8 run_type);

/**
 * @brief 获取列表里最近时间
 *
 * @param[in] type: timer类型
              ture: 全部类型时间
              flase:休眠下部执行的timer类
 * @return : 625us单位时间
 *         
 **/
u32 co_timer_get_min_time(bool type);

/**
 * @brief返回ms的tick
 *       使用蓝牙时钟源
 * @return : ms单位时间
 */
u32 co_tick_get(void);

/**
 * @brief 判断预设时间是否满足
 *          
 * @param[in] tick: 初始时间，ms单位时间
 * @param[in] co_expire: 期待多久，ms单位时间
 * @return false:超过设置的co_expire
 *         true: 没超过设置的co_expire
*/
bool co_check_expire(u32 tick, u32 co_expire);

#endif