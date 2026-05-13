#include "include.h"

#if USER_KEY_QDEC_EN

AT(.com_text.qdec)
void qdec_key_msg_enqueue(u8 msg)
{
    reset_sleep_delay_all();        //复位休眠计数
    msg_enqueue(msg);
}

/*
*   旋转编码器Quadrate Decode
*   QDEC_MAP_G1: A -> PB0, B -> PB1
*   QDEC_MAP_G2: A -> PE1, B -> PE2
*   QDEC_MAP_G3: A -> PE6, B -> PE7
*/

AT(.com_text.qdec)
void qdec_isr(void)
{
    u32 qdeccon = QDECCON;
    if (qdeccon & BIT(1)) {                 //interrupt is enbale?
        if (qdeccon & BIT(30)) {            //forward
            QDECCPND = BIT(30);
            qdec_key_msg_enqueue(MSG_QDEC_FORWARD);
        }
        if (qdeccon & BIT(31)) {            //reverse
            QDECCPND = BIT(31);
            qdec_key_msg_enqueue(MSG_QDEC_BACKWARD);
        }
    }
}

void bsp_qdec_init(void)
{
    CLKGAT0 |= BIT(22);                      //qdec clk enable
#if (USER_QDEC_MAPPING == QDEC_MAP_G1)
    GPIOADIR |= 0x03;
    GPIOADE |= 0x03;
    GPIOAFEN |= 0x03;
    GPIOAPU |= 0x03;
#elif (USER_QDEC_MAPPING == QDEC_MAP_G2)
    GPIOBDIR |= (3 << 3)
    GPIOBDE |= (3 << 3)
    GPIOBFEN |= (3 << 3)
    GPIOBPU |= (3 << 3)
#elif (USER_QDEC_MAPPING == QDEC_MAP_G3)
    GPIOEDIR |= (3 << 9);
    GPIOEDE |= (3 << 9);
    GPIOEFEN |= (3 << 9);
    GPIOEPU |= (3 << 9);
#elif (USER_QDEC_MAPPING == QDEC_MAP_G4)
    GPIOEDIR |= (3 << 13);
    GPIOEDE |= (3 << 13);
    GPIOEFEN |= (3 << 13);
    GPIOEPU |= (3 << 13);
#elif (USER_QDEC_MAPPING == QDEC_MAP_G5)
    GPIOADIR |= (3 << 12);
    GPIOADE |= (3 << 12);
    GPIOAFEN |= (3 << 12);
    GPIOAPU |= (3 << 12);
    CH0_FUNI_SEL(FI_PA12);
    CH1_FUNI_SEL(FI_PA13);
#elif (USER_QDEC_MAPPING == QDEC_MAP_G6)
    GPIOEDIR |= (3 << 2);
    GPIOEDE |= (3 << 2);
    GPIOEFEN |= (3 << 2);
    GPIOEPU |= (3 << 2);
#elif (USER_QDEC_MAPPING == QDEC_MAP_G7)
    GPIOBDIR |= (3 << 0);
    GPIOBDE |= (3 << 0);
    GPIOBFEN |= (3 << 0);
    GPIOBPU |= (3 << 0);
#endif
    sys_irq_init(IRQ_QDEC_VECTOR, 0, qdec_isr);
    FUNCMCON2 = USER_QDEC_MAPPING;
    QDECCON = 45 << 3;                      //quadrate decode filter length
    QDECCPND = BIT(30) | BIT(31);
    QDECCON = BIT(0) | BIT(1);              //qdec decode enable, interrupt enable
}

void bsp_qdec_exit(void)
{
    QDECCON = 0;
    CLKGAT0 &= ~BIT(22);
#if (USER_QDEC_MAPPING == QDEC_MAP_G1)
    GPIOADE &= ~0x03;
    GPIOAPU &= ~0x03;
#elif (USER_QDEC_MAPPING == QDEC_MAP_G2)
    GPIOBDE &= ~(3 << 3)
    GPIOBPU &= ~(3 << 3)
#elif (USER_QDEC_MAPPING == QDEC_MAP_G3)
    GPIOEDE &= ~(3 << 9);
    GPIOEPU &= ~(3 << 9);
#elif (USER_QDEC_MAPPING == QDEC_MAP_G4)
    GPIOEDE &= ~(3 << 13);
    GPIOEPU &= ~(3 << 13);
#elif (USER_QDEC_MAPPING == QDEC_MAP_G5)
    GPIOADE &= ~(3 << 12);
    GPIOAPU &= ~(3 << 12);
    CH0_FUNI_SEL(FI_PA12);
    CH1_FUNI_SEL(FI_PA13);
#elif (USER_QDEC_MAPPING == QDEC_MAP_G6)
    GPIOEDE &= ~(3 << 2);
    GPIOEPU ~= ~(3 << 2);
#elif (USER_QDEC_MAPPING == QDEC_MAP_G7)
    GPIOBDE &= ~(3 << 0);
    GPIOBPU &= ~(3 << 0);
#endif
}

#elif USER_ADKEY_QDEC_EN

/*
*   QDEC_A串接5.1K，QEC_B串接10K，接到同一IO上。IO开内部10K上拉，用ADC来采电压。
*   正旋：3.3v -- 1.113v -- 0.825v -- 1.65v -- 3.3v
*   正旋: AH_BH - AL_BH -- AL_BL -- AH_BL -- AH_BH
*   反旋：3.3v -- 1.65v -- 0.825v -- 1.113v -- 3.3v
*   反旋：AH_BH - AH_BL -- AL_BL -- AL_BH -- AH_BH
*/

enum {
    QEC_AH_BH,                              //A输出高电平，B输出高电平，空闲状态
    QEC_AL_BH,                              //A输出低电平，B输出高电平
    QEC_AL_BL,                              //A输出低电平，B输出低电平
    QEC_AH_BL,                              //A输出高电平，B输出低电平
};

typedef struct {
    u8 status[3];
    u8 cnt;
    u8 pre_sta;
} qdec_cb_t;
static qdec_cb_t qdec_cb;

AT(.com_rodata.pwrkey.table)
const adkey_tbl_t qdec_adkey_table[6] = {
    {0x30, QEC_AH_BH},
    {0x4C, QEC_AL_BL},
    {0x6A, QEC_AL_BH},
    {0x8A, QEC_AH_BL},
    {0xFF, QEC_AH_BH},
};

void bsp_qdec_init(void)
{
    u8 io_num = get_adc_gpio_num(USER_QDEC_ADCH);

    memset(&qdec_cb, 0, sizeof(qdec_cb_t));
    if (io_num != IO_NONE) {
        gpio_t gpio;
        gpio_cfg_init(&gpio, io_num);
        gpio.sfr[GPIOxDE] |= gpio.pin;
        gpio.sfr[GPIOxDIR] |= gpio.pin;
        gpio.sfr[GPIOxPU] |= gpio.pin;
        saradc_set_channel(BIT(USER_QDEC_ADCH));
    }
}

AT(.com_text.qdec)
void qdec_key_msg_enqueue(u8 msg)
{
    reset_sleep_delay_all();        //复位休眠计数
    msg_enqueue(msg);
}

//每毫秒ADC采样值进行处理
AT(.com_text.qdec)
void bsp_qdec_adc_process(u8 adc_val)
{
    u8 num = 0;
    u8 qdec_cur_sta;

    while ((u8)adc_val > qdec_adkey_table[num].adc_val) {
        num++;
    }
    qdec_cur_sta = qdec_adkey_table[num].usage_id;
    if (qdec_cur_sta != qdec_cb.pre_sta) {
        if (qdec_cur_sta != QEC_AH_BH) {
            if (qdec_cb.pre_sta == QEC_AH_BH) {
                qdec_cb.cnt = 0;
            }
            if (qdec_cb.cnt < 3) {
                qdec_cb.status[qdec_cb.cnt++] = qdec_cur_sta;
            }
        }
        qdec_cb.pre_sta = qdec_cur_sta;
    } else {
        if (qdec_cur_sta == QEC_AH_BH) {                //结束

#if !USER_ADKEY_QDEC_NO_STD
            if (qdec_cb.cnt == 3 && qdec_cb.status[1] == QEC_AL_BL) {
                if (qdec_cb.status[0] == QEC_AL_BH && qdec_cb.status[2] == QEC_AH_BL) {
                    qdec_key_msg_enqueue(MSG_QDEC_FORWARD);
//                    uart_putchar('F');
                } else if (qdec_cb.status[2] == QEC_AL_BH && qdec_cb.status[0] == QEC_AH_BL) {
                    qdec_key_msg_enqueue(MSG_QDEC_BACKWARD);
//                    uart_putchar('R');
                }
            }
#else
            if (qdec_cb.cnt >= 2) {
                if (qdec_cb.status[0] == QEC_AL_BH) {
                    if (qdec_cb.status[1] == QEC_AL_BL) {
                        qdec_key_msg_enqueue(MSG_QDEC_FORWARD);
//                        uart_putchar('F');
                    }
                } else if (qdec_cb.status[0] == QEC_AH_BL) {
                    if (qdec_cb.status[1] == QEC_AL_BL) {
                        qdec_key_msg_enqueue(MSG_QDEC_BACKWARD);
//                        uart_putchar('R');
                    }
                }
            }
#endif
            memset(&qdec_cb, 0, sizeof(qdec_cb_t));
        }
    }
}

void bsp_qdec_exit(void)
{
    u8 io_num = get_adc_gpio_num(USER_QDEC_ADCH);
    if (io_num != IO_NONE) {
        gpio_t gpio;
        gpio_cfg_init(&gpio, io_num);
        gpio.sfr[GPIOxDE] &= ~gpio.pin;
        gpio.sfr[GPIOxPU] &= ~gpio.pin;
        saradc_clr_channel(BIT(USER_QDEC_ADCH));
    }
}
#else
void bsp_qdec_init(void)
{

}

void bsp_qdec_exit(void)
{

}
#endif // USER_ADKEY_QDEC_EN
