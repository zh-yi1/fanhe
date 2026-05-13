#ifndef _BSP_PORT_INT_H
#define _BSP_PORT_INT_H

#define EXT_ISR_MAX_ID              8

typedef enum {
    RISE_EDGE,                      //上升沿
    FALL_EDGE,                      //下降沿
}ext_edge_type;

typedef enum {
    IOUD_SEL_NULL,                  //不开内部上拉
    IOUD_SEL_PU,                    //内部上拉
    IOUD_SEL_PD,                    //内部下拉
}ext_pupd_type;

typedef enum {
    MODE_ONLY_AWAKEN,               //唤醒时调用回调，休眠关闭(默认)
    MODE_BOTH_AWAKEN_SLEEP,         //唤醒休眠皆可调用回调,不具备唤醒
    MODE_BOTH_AWAKEN_SLEEP_PWK,     //唤醒休眠皆可调用回调,并具备唤醒
}ext_mode_type;

typedef struct {
    int io_num;
    int irq_num;
}ext_vector_sel;

typedef struct {
    u8 io_num;
	ext_edge_type edg;
    ext_pupd_type pupd_type;
    ext_mode_type mode;
    int vector;
    bool flag;
    bool stand;
    isr_t callback;
}port_int_cb_t;

//系统调用
void port_int_disable_to_sleep(void);
//系统调用
void port_int_enable_exit_sleep(void);
//系统调用
void port_int_sleep_process(u32 *wku);

/**
 * @brief set port ext ISR 打开IO外部中断功能，且设置中断回调
 * 注：建议使用的时候，一个IO单独使用一个中断向量号这种方式
 * @param[in] gpio_num  IO_PA0 IO_PA1 ...
 * @param[in] edg   触发边沿 根据 ext_edge_type 枚举类型选择
 * @param[in] callback_func 外部中断回调函数，必须放置在COM区，不需要回调执行可填NULL
 * @param[in] pupd_type   上下拉选择 默认不开内部上拉 根据 ext_pupd_type 枚举类型选择
 * 默认模式:MODE_ONLY_AWAKEN, 在休眠下关闭回调, 该模式可以调用extab_user_isr_mode_set设置
 * @return  false: set isr fail;
            true: set isr success;
 **/
bool extab_user_isr_set(u8 gpio_num, ext_edge_type edg, ext_pupd_type pupd_type, isr_t callback_func);

/**
 * @brief del ext isr 清除对应IO中断
 *注：清除后，重新使用中断需要调用 extab_user_isr_set 创建
 * @param[in] gpio_num  IO_PA0 IO_PA1 ...
 *
 * @return  false: clr isr fail;
            true: clr isr success;
 **/
bool extab_user_isr_clr(u8 gpio_num);

/**
 * @brief 设置外部中断调用模式
 *
 * @param[in] gpio_num  IO_PA0 IO_PA1 ...
 * @param[in] mode  MODE_ONLY_AWAKEN,               唤醒时调用回调，休眠关闭(默认)
                    MODE_BOTH_AWAKEN_SLEEP,         唤醒休眠皆可调用回调,不具备唤醒
                    MODE_BOTH_AWAKEN_SLEEP_PWK,     唤醒休眠皆可调用回调,并具备唤醒
 * @return  false: set isr mode fail;
            true: set isr mode success;
 **/
bool extab_user_isr_mode_set(u8 gpio_num, ext_mode_type mode);
#endif