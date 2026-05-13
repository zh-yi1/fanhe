#include "asr_config.h"
#include "bsp_hw_timer.h"

#if (ASR_SELECT == ASR_WS_AIR && IR_AIR_FUNC)

//=======================================================================================
extern u8 asr_start_flag; //语音开启标志

static uint8_t  ir_prd, ir_times, ir_cnt;
static uint16_t dat_index, dat_len;
static uint8_t  ir_level_sta;
static uint32_t ir_times_h, ir_times_l;
static uint8_t  duty_h, duty_l; //载波高低占比 H-3, L-7

#define IR_TX_PORT    5 // 红外发送IO
#define IR_TX_VCT     IRQ_TMR3_VECTOR
#define IR_TX_CON     TMR3CON
#define IR_TX_CPD     TMR3CPND
#define IR_TX_CNT     TMR3CNT
#define IR_TX_PRD     TMR3PR


AT(.com_text.str)
const char ir_value[] = "ir stop...\n";
AT(.com_text.str)
const char ir_asr_start[] = "asr start...\n\n";


AT(.kws_ir.com)
void ir_pwm_io_init()
{
    GPIOFFEN &= ~BIT(IR_TX_PORT);    //PA9 作为 GPIO 使用
    GPIOFDE |= BIT(IR_TX_PORT);      //PA9 设置为数字 IO
    GPIOFDIR &= ~BIT(IR_TX_PORT);    //PA9 方向设置为输出
    GPIOFCLR = BIT(IR_TX_PORT);
}

AT(.com_text.isr.timer)
static void timer3pwm_isr(void)
{
    static uint8_t cnt;

    if (IR_TX_CON & BIT(9)) {
        IR_TX_CPD = BIT(9);
    }
#if 0
    static u8 s_flag = 0;
    if(s_flag) {
        s_flag = 0;
        GPIOACLR = BIT(IR_TX_PORT);
    }else{
        s_flag = 1;
        GPIOASET = BIT(IR_TX_PORT);
    }

#else
    if (dat_len && dat_index <= dat_len) {
        if (ir_level_sta) {
            if (ir_times_h--) {
                cnt++;
                if (cnt < duty_h) {
                    GPIOFSET = BIT(IR_TX_PORT);
                } else {
                    GPIOFCLR = BIT(IR_TX_PORT);
                    if (cnt >= duty_l) {
                        cnt = 0;
                    }
                }
            } else {
                cnt = 0;
                ir_times_l = pwm_out[dat_index++];
                ir_level_sta = 0;
            }
        } else {
            cnt = 0;
            if (ir_times_l--) {
                GPIOFCLR = BIT(IR_TX_PORT);
            } else {
                ir_level_sta = 1;
                ir_times_h = pwm_out[dat_index++];
            }
        }
    } else {
        if (ir_cnt < ir_times) {
            ir_times_h = pwm_out[0];
            dat_index = 1;
            ir_level_sta = 1;

            ir_cnt++;
        } else {
            GPIOFCLR = BIT(IR_TX_PORT);
            IR_TX_CON = 0;
            my_printf(ir_value);

            asr_start_flag = 1; //开启语音识别
            my_printf(ir_asr_start);
        }
    }
#endif
}


AT(.kws_ir.com)
void timer3_pwm_stop()
{
    IR_TX_CON = 0;
}


AT(.kws_ir.com)
void timer3_pwm_init()
{
    ir_pwm_io_init();

    register_isr(IR_TX_VCT, timer3pwm_isr);
    CLKGAT1 |= BIT(8+HW_TIMER3-2); //时钟使能寄存器
    IR_TX_CON = 0;
    PICPR &= ~BIT(IR_TX_VCT); //优先级中断 控制寄存器
    PICEN |= BIT(IR_TX_VCT);  //优先级中断 使能寄存器
    IR_TX_CNT = 0;
    IR_TX_PRD = (3 * 1) - 1;
    IR_TX_CON = (2 << 1) | (3 << 4) | BIT(7) | BIT(0); //24M div8
}

AT(.kws_ir.com)
void pwmr_data_init(uint16_t len, uint8_t times, uint32_t fre)
{
    ir_times = times;
    ir_cnt = 1;

    ir_level_sta = 1;
    ir_times_h = pwm_out[0];
    dat_index = 1;
    dat_len = len;

    ir_prd = 1000000 / fre;
    printf("ir_prd:%d\n", ir_prd);
    duty_h = ir_prd / 3;
    duty_l = ir_prd - 1; // -1是补偿定时器不准
}


AT(.kws_ir.com)
void isd_pwmr_send_wave(uint16_t* pbuf, uint32_t len, uint32_t frequency, uint32_t times)
{
    printf("ir send data len=%d,  frequency=%d,  times=%d\n", len, frequency, times);
    for (int i = 0; i < len; ) { 
        printf("%d, ", pbuf[i]); 
        i++;
        if(i%20 == 0) printf("\n");
    };
    printf("\n\n");

    timer3_pwm_stop();
    pwmr_data_init(len, times, frequency);
    timer3_pwm_init();

    printf("888888888888888888888888 \n\n");
}

#endif // ASR_WS_AIR