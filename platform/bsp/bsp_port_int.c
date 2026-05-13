#include "include.h"

#define EXT_ISR_MAX_VECTOR_MAP          ((int)(sizeof(ext_vector_cb) / sizeof(ext_vector_sel)))

static const ext_vector_sel ext_vector_cb[] = {
    {IO_WKO,  PORT_INT0_VECTOR},{IO_PA0,  PORT_INT0_VECTOR},{IO_PA1,  PORT_INT0_VECTOR},{IO_PA13, PORT_INT0_VECTOR},
    {IO_PA5,  PORT_INT1_VECTOR},{IO_PB7,  PORT_INT1_VECTOR},{IO_PA14, PORT_INT1_VECTOR},{IO_PA15, PORT_INT1_VECTOR},
    {IO_PA6,  PORT_INT2_VECTOR},{IO_PE9,  PORT_INT2_VECTOR},{IO_PB3,  PORT_INT2_VECTOR},{IO_PB4,  PORT_INT2_VECTOR},
    {IO_PA10, PORT_INT3_VECTOR},{IO_PA11, PORT_INT3_VECTOR},{IO_PB10, PORT_INT3_VECTOR},{IO_PB8,  PORT_INT3_VECTOR},
    {IO_PE1,  PORT_INT4_VECTOR},{IO_PE9,  PORT_INT4_VECTOR},{IO_PB0,  PORT_INT4_VECTOR},{IO_PB1,  PORT_INT4_VECTOR},
    {IO_PE10, PORT_INT5_VECTOR},{IO_PE2,  PORT_INT5_VECTOR},{IO_PB6,  PORT_INT5_VECTOR},{IO_PB5,  PORT_INT5_VECTOR},
    {IO_PB2,  PORT_INT6_VECTOR},{IO_PE5,  PORT_INT6_VECTOR},{IO_PE6,  PORT_INT6_VECTOR},{IO_EDGE_FALL, PORT_INT6_VECTOR},
    {IO_PE7,  PORT_INT7_VECTOR},{IO_PE8,  PORT_INT7_VECTOR},{IO_PE11, PORT_INT7_VECTOR},{IO_EDGE_RISE, PORT_INT7_VECTOR},
};

static port_int_cb_t port_int_cb[EXT_ISR_MAX_ID];
static bool is_sleep_mode = 0;

//根据中断向量号查询中断结构体
AT(.com_text.portint.irq)
port_int_cb_t *extab_user_isr_inqure_cb_from_irq(int irq_num)
{
    port_int_cb_t *pi_cb = NULL;
    for (u8 i = 0; i < EXT_ISR_MAX_ID; i++) {
        pi_cb = &port_int_cb[i];
        if ((pi_cb->flag == true) && (pi_cb->vector == irq_num)) {
            return pi_cb;
        }
    }
    return NULL;
}

//根据GPIO查询中断结构体
AT(.com_text.portint.gpio)
port_int_cb_t *extab_user_isr_inqure_cb_from_gpio(u8 io_num)
{
    port_int_cb_t *pi_cb = NULL;
    for (u8 i = 0; i < EXT_ISR_MAX_ID; i++) {
        pi_cb = &port_int_cb[i];
        if ((pi_cb->flag == true) && (pi_cb->io_num == io_num)) {
            return pi_cb;
        }
    }
    return NULL;
}

//设置中断
void extab_user_isr_set_do(port_int_cb_t *pi_cb, bool reg_en)
{
    if (reg_en) {
        port_irq_register(pi_cb->vector, pi_cb->callback);
    }

    if (pi_cb->stand) {
        port_wakeup_init(pi_cb->io_num, pi_cb->edg, pi_cb->pupd_type);
    } else {
        port_wakeup_all_init(pi_cb->io_num, pi_cb->edg, pi_cb->pupd_type);
    }
}

//清除中断
void extab_user_isr_clr_do(port_int_cb_t *pi_cb)
{
    port_irq_free(pi_cb->vector);
    port_wakeup_exit(pi_cb->io_num);
}

bool extab_user_isr_set(u8 gpio_num, ext_edge_type edg, ext_pupd_type pupd_type, isr_t callback_func)
{
    if ((gpio_num == IO_NONE) || (gpio_num > IO_MAX_NUM)) {
        return false;
    }
    //是否已经建立
    port_int_cb_t *p = NULL;
    for (u8 i = 0; i < EXT_ISR_MAX_ID; i++) {
        p = &port_int_cb[i];
        if ((p->io_num == gpio_num) && (p->flag == true)) {
            p->callback = callback_func;
            return true;
        }
    }

    for (u8 i = 0; i < EXT_ISR_MAX_ID; i++) {
        p = &port_int_cb[i];
        if (p->flag == false) {
            //向量号查询
            for (u8 k = 0; k < EXT_ISR_MAX_VECTOR_MAP; k++) {
                if (gpio_num == ext_vector_cb[k].io_num) {
                    p->vector = ext_vector_cb[k].irq_num;
                    p->stand = true;
                    break;
                }
            }
            //非独立中断，采用通用中断
            if (!p->stand) {
                p->vector = (edg == RISE_EDGE ? PORT_INT7_VECTOR : PORT_INT6_VECTOR);
            }

            p->pupd_type = pupd_type;               //上下拉模式
            p->callback  = callback_func;           //回调
			p->io_num    = gpio_num;                //引脚
			p->edg       = edg;                     //触发边沿
			p->flag      = true;                    //设置标志
            p->mode      = MODE_ONLY_AWAKEN;        //默认模式
            extab_user_isr_set_do(p, true);
            break;
        }
    }

    return true;
}

bool extab_user_isr_clr(u8 gpio_num)
{
    port_int_cb_t *p = NULL;
    p = extab_user_isr_inqure_cb_from_gpio(gpio_num);
    if (p != NULL) {
        extab_user_isr_clr_do(p);
        memset(p, 0, sizeof(port_int_cb_t));
        return true;
    }
    return false;
}

bool extab_user_isr_mode_set(u8 gpio_num, ext_mode_type mode)
{
    port_int_cb_t *p = NULL;
    p = extab_user_isr_inqure_cb_from_gpio(gpio_num);
    if (p != NULL) {
        p->mode = mode;
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------
//中断进休眠设置
void port_int_disable_to_sleep(void)
{
    port_int_cb_t *p = NULL;
    for(u8 i = 0; i < EXT_ISR_MAX_ID; i++) {
        p = &port_int_cb[i];
        if ((p->flag == true)) {
			if (p->mode == MODE_ONLY_AWAKEN) {
                extab_user_isr_clr_do(p);
            } else if (p->mode >= MODE_BOTH_AWAKEN_SLEEP) {
                extab_user_isr_set_do(p, false);
            }
        }
    }

    is_sleep_mode = true;
}

//中断退休眠设置
void port_int_enable_exit_sleep(void)
{
    is_sleep_mode = false;

    port_int_cb_t *p = NULL;
    for(u8 i = 0; i < EXT_ISR_MAX_ID; i++) {
        p = &port_int_cb[i];
        if ((p->flag == true)) {
            extab_user_isr_set_do(p, true);
        }
    }
}

//中断休眠处理
AT(.sleep_text.sleep.port_int)
void port_int_sleep_process(u32 *wku)
{
    if (!is_sleep_mode) {
        return;
    }

    int bit = 0;
    u32 weakpu, pending, status;
    weakpu = pending = status = *(u32 *)wku;
    port_int_cb_t *p = NULL;

    while (status) {
        bit = xbs_ff1(status);
        p = extab_user_isr_inqure_cb_from_irq(bit);
        status &= ~BIT(bit);
        weakpu &= ~BIT(bit);
        if (p != NULL) {
            if (p->callback != NULL && p->mode >= MODE_BOTH_AWAKEN_SLEEP) {
                p->callback();
            }
            if (p->mode == MODE_BOTH_AWAKEN_SLEEP_PWK) {
                pending &= ~BIT(bit);
                weakpu |= BIT(bit);
            }
        } else {
            pending &= ~BIT(bit);
            weakpu |= BIT(bit);
        }
    }

    if (*(u32 *)wku) {
        WKUPCPND = ((u8)pending) << 16;
        *(u32 *)wku = ((u8)weakpu);
    }
}
