#ifndef _BSP_HW_TIMER_H
#define _BSP_HW_TIMER_H



typedef enum
{
	HW_TIMER1 = 0,
	HW_TIMER2,
	HW_TIMER3,
    HW_TIMER4,
    HW_TIMER_MAX,
}hw_timer_mode;


typedef void (*hw_timer_callback_t)(void);


/**
 * @brief 设置硬件定时器
 * @param[in] t_id 选择硬件定时器
 * @param[in] delay us为单位
 * @param[in] callback 中断回调
 *
 * @return  返回是否成功
 **/
bool bsp_hw_timer_set(hw_timer_mode t_id, uint32_t delay, hw_timer_callback_t callback);


/**
 * @brief 删除硬件定时器
 * @param[in] t_id 选择硬件定时器
 *
 * @return  返回是否成功
 **/
bool bsp_hw_timer_del(hw_timer_mode t_id);


//------------------------------------------------------------------------------------------
//用于测试运行时间函数 us为单位
//使用TIME3
/**
 * @brief 定时器初始化，可以用于测试代码运行时间
 *
 * @param[in] void
 *
 * @return  void
 **/
void cc_time_init(void);


/**
 * @brief 获取时间 us为单位
 * 注:休眠模式下测试不准确
 * @param[in] void
 *
 * @return  返回us时间
 **/
u32 cc_time_count(void);





#endif
