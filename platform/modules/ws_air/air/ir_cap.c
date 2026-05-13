#include "include.h"

#if (ASR_SELECT == ASR_WS_AIR && IR_AIR_FUNC)

#include "ir_cap.h"
#include "asr_config.h"

extern u8  asr_start_flag;

static u32 tmrlast;
static u8  ir_match_flag;
static u16 cap_index;
u8  ir_cap_flag;
u32 ir_cap_tick;
u16 ir_data_len;



AT(.com_text.str)
const char ir_rx[] = "ir_rx: %d\n";

int isd_check_ir_dat(void);




//===========================================================================================
AT(.kws_ir.pwm_rcv)
void ir_cap_timer_stop()
{
    ir_match_flag = 0;
    TMR3CON &= ~(BIT(8) | BIT(0));
}


AT(.kws_ir.pwm_rcv)
int isd_get_air_match_status()
{
    return ir_match_flag;
}


AT(.kws_ir.pwm_rcv)
int check_ir_data_error(int32_t *buf, u16 len)
{
    u32 err_cnt = 0, i = 0;
    for(i = 0; i < len; i++) {
        if(buf_prd[0] < 200)
            return -1;
        if(buf_prd[i] < 200)
        {
            err_cnt++;
        }
    }
    if(err_cnt > 2) {
        return -err_cnt;
    }
    if(len < 70 && buf_prd[0] < 4000) {
        return -1;
    }
    return 0;
}


AT(.kws_ir.pwm_rcv)
void ir_timeout()
{
    printf("\n___========================== %d\n", cap_index);
    buf_prd[cap_index++] = 0;

    for (u16 i = 0; i < cap_index; )
    {
        buf_prd[i] *= 1.34; // 分频补偿 24M时钟 32分频
        printf("%d, ", buf_prd[i]);
        i++;
        if(i%20 == 0) printf("\n");
    }
    printf("\n");

    if(cap_index > 20)
    {
        if(check_ir_data_error(buf_prd, cap_index) == 0)
        {
            printf("cap succ\n");
            ir_data_len = cap_index;

            if(isd_check_ir_dat() == 0) {
                printf("match succ\n");
                ir_cap_timer_stop();
                // air_match_result_flashing(EFFECT_MATCH_SUCC);
                bsp_asr_tone_play(RES_BUF_ASR_31_MP3, RES_LEN_ASR_31_MP3);
            } else {
                // air_match_result_flashing(EFFECT_MATCH_FAIL);
                bsp_asr_tone_play(RES_BUF_ASR_32_MP3, RES_LEN_ASR_32_MP3);
            }
        }
    }
    cap_index = 0;
    // ir_cap_tick = 0;
    ir_cap_flag = 0;
    ir_cap_tick = tick_get();

    asr_start_stop(1);
}


//===========================================================================================
AT(.com_text.isr.timer)
static void timercap_isr(void)
{
    u32 tmrcnt;
    u32 tmrpr;
    if(TMR3CON & BIT(10)) 
    {
        asr_start_flag = 0;

        if(ir_cap_flag < 2) {
            ir_cap_flag++;
        }
        // ir_cap_tick = 0;
        ir_cap_tick = tick_get();

        // timer1 capture interrupt
        tmrpr = TMR3CPT;
        if (tmrlast < tmrpr) {
            tmrcnt = tmrpr - tmrlast;
        } else {
            tmrcnt = (0xffff - tmrlast) + tmrpr;
        }
        tmrlast = tmrpr;
        TMR3CPND = BIT(10);
        // tmrcnt /= TMR2_RCLK; //convert to ms

        if(cap_index > 1024) {
            return;
        }

        if(ir_cap_flag == 2) { //丢弃第一个数据，从第二个开始存入
            buf_prd[cap_index++] = tmrcnt;
        }
        // my_printf(ir_rx, tmrcnt);
    } else {
        return;
    }
}


AT(.kws_ir.pwm_rcv)
void timercap_init(void)
{
    // sys_irq_init(IRQ_TMR3_VECTOR, 0, timer3_isr);
    register_isr(IRQ_TMR3_VECTOR, timercap_isr);
    CLKGAT1 |= BIT(8+HW_TIMER3-2); //时钟使能寄存器
    TMR3CON = 0;
    PICPR &= ~BIT(IRQ_TMR3_VECTOR);
	PICEN |= BIT(IRQ_TMR3_VECTOR);
	TMR3CNT = 0;
    TMR3PR = 0xffffffff;


    /*
    * (2 << 1) 24m clk
    * (5 << 4) 32 div
    * (3 << 14) capture rising or falling
    * bit(8) capture interrupt enable
    * bit(0) timer enable 
    */
    TMR3CON = (2 << 1) | (5 << 4) | (3 << 14) | BIT(8) | BIT(0) ;
}


AT(.kws_ir.pwm_rcv)
void irrx_sw_init(void)
{
    IR_CAPTURE_PORT();
    // FUNCMCON2 = (1 << 4);           //enable timer3 map
    FUNCMCON2 = (9 << 4);           //enable timer3 map
    FUNCINCON = (IRRX_MAPPING - 1);
    tmrlast = 0;
    timercap_init();
}


AT(.kws_ir.pwm_rcv)
void ir_cap_timer_start(u8 type)
{
    irrx_sw_init();
    ir_match_flag = type;
    my_printf("cap start......\n");
}

#endif // ASR_WS_AIR