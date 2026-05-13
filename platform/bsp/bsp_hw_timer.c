#include "include.h"


/**
 * @brief 寄存器CON
 **/
static const volatile uint32_t *TIMER_N_CON[HW_TIMER_MAX] = {
    &TMR1CON, &TMR2CON, &TMR3CON, &TMR4CON
};

static const int TIMER_N_VECTOR[HW_TIMER_MAX] = {
    IRQ_TMR1_VECTOR, IRQ_TMR2_VECTOR, IRQ_TMR3_VECTOR, IRQ_TMR4_VECTOR
};

static hw_timer_callback_t isr_cb[HW_TIMER_MAX];

/**
 * @brief ISR
 **/
AT(.com_text.isr)
static void timer_isr_callback(void)
{
    if(TMR1CON & BIT(9)){
        TMR1CPND = BIT(9);
        if(isr_cb[HW_TIMER1]){
            isr_cb[HW_TIMER1]();
        }
    }

    if(TMR2CON & BIT(9)){
        TMR2CPND = BIT(9);
        if(isr_cb[HW_TIMER2]){
            isr_cb[HW_TIMER2]();
        }
    }

    if(TMR3CON & BIT(9)){
        TMR3CPND = BIT(9);
        if(isr_cb[HW_TIMER3]){
            isr_cb[HW_TIMER3]();
        }
    }

    if(TMR4CON & BIT(9)){
        TMR4CPND = BIT(9);
        if(isr_cb[HW_TIMER4]){
            isr_cb[HW_TIMER4]();
        }
    }
}


/**
 * @brief 设置硬件定时器
 * @param[in] t_id 选择硬件定时器
 * @param[in] delay us为单位
 * @param[in] callback 中断回调
 *
 * @return  返回是否成功
 **/
bool bsp_hw_timer_set(hw_timer_mode t_id, uint32_t delay, hw_timer_callback_t callback)
{
    volatile uint32_t *timer_con, *timer_cpnd, *timer_cnt, *timer_pr;
    int vector;

	if((t_id >= HW_TIMER_MAX) || (delay == 0)) {
		return false;
	}

    if((u32)callback >= 0x10000000){
        printf("hw_timer_set ERR : callback function in flash\n");
        return false;
    }

    vector     = TIMER_N_VECTOR[t_id];
    timer_con  = (void *)TIMER_N_CON[t_id];
    timer_cpnd = (void *)TIMER_N_CON[t_id] + 4;
    timer_cnt  = (void *)TIMER_N_CON[t_id] + 8;
    timer_pr   = (void *)TIMER_N_CON[t_id] + 12;

    CLKGAT0 |= BIT(1+t_id);

    *timer_con = 0;
    PICPR &= ~BIT(vector);
	PICEN |= BIT(vector);
    register_isr(vector, timer_isr_callback);

    isr_cb[t_id]= callback;

	*timer_cnt  = 0;
	*timer_pr   = 12 * delay - 1;
    *timer_cpnd = BIT(9) | BIT(10) | BIT(12);
    *timer_con  = BIT(7) | (1<<4) | (2<<1)  | BIT(0);  // xosc26_div2

    return true;
}


/**
 * @brief 删除硬件定时器
 * @param[in] t_id 选择硬件定时器
 *
 * @return  返回是否成功
 **/
bool bsp_hw_timer_del(hw_timer_mode t_id)
{
    uint32_t *timer_con;
    int vector;

	if(t_id >= HW_TIMER_MAX) {
		return false;
	}

    vector     = TIMER_N_VECTOR[t_id];
    timer_con  = (void *)TIMER_N_CON[t_id];

    PICEN &= ~BIT(vector);
    *timer_con = 0x00;  // xosc26_div2

    return true;
}

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
void cc_time_init(void)
{
    CLKGAT0 |= BIT(3);                      //timer3 clkgat
    delay_us(3);

    TMR3CON = 0;
	TMR3CNT = 0;
	TMR3PR  = -1;
    TMR3CON = BIT(0) | BIT(2) | BIT(4);       //Timer RC2M div2
}

/**
 * @brief 获取时间 us为单位
 * 注:休眠模式下测试不准确
 * @param[in] void
 *
 * @return  返回us时间
 **/
u32 cc_time_count(void)
{
	return TMR3CNT/12;
}


