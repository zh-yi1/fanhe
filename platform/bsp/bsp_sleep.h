#ifndef _BSP_SLEEP_H_
#define _BSP_SLEEP_H_


typedef void (*sleep_callback_func)(void *param);


typedef struct _sleep_cb_t{
    sleep_callback_func sleep_enter_callback_func;
    sleep_callback_func sleep_exit_callback_func;
    bool sleep_terminate;
    bool sys_is_sleep;
    bool adpll_is_sleep;
}sleep_cb_t;


extern sleep_cb_t sleep_cb;





/**
 *   设置进入和退出休眠的回调函数
     sleep是省功耗的模式，该模式下会关闭ADC和部分PLL、硬件定时器，以及节省功耗；
     BLE和EDR还是保持连接，BLE正常交互不会退出sleep模式；EDR动作会退出，比如播放音乐，来电等动作。
 *   enter_callback : 进入休眠会被调用，可以在里面放置休眠下不需要执行的功能的反初始化，IO销毁等动作，以达到省功耗；
 *   exit_callback  : 退出休眠会被调用，可以在里面放置退出休眠需要重新初始化、恢复硬件等功能；
 */
void bsp_sleep_callbck_set(sleep_callback_func enter_callback,  sleep_callback_func exit_callback);


/**
 *  设置是否允许进入休眠
 *  is_allow : 如果当前在RUN模式，设置为true，指示系统可以进入sleep模式（会调用 enter_callback）；已经在sleep模式，设置为true没有意义;
               如果当前在RUN模式，设置false，指示系统不允许进入sleep模式；当已经sleep模式，设置为false，立马会退出sleep模式（会调用exit_callback）；
 */
void bsp_sleep_permit_set(bool is_allow);


/**
 *  获取当前是否在休眠模式
 *  返回值：true，在sleep模式；
            false 在run模式
 */
bool bsp_system_is_sleep(void);





#endif
