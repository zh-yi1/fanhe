#include "include.h"

#if SENSOR_HUB_EN

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define SENSHB_CMD_SHIFT                    5
#define SENSHB_CMD_CFG(sfr_num, cmd)        ((cmd << SENSHB_CMD_SHIFT) | sfr_num)
#define IIC_CMD_MIX(dev_addr, reg_addr)     (u8)dev_addr | ((u32)(dev_addr >> 8)) << 24 | (u32)((u8)reg_addr) << 8 | (u32)((u8)(reg_addr >> 8)) << 16
#define ROUND_DIV(a, d)                     (((a)+(d)/2) / (d))
#define LPM_EN                              0    //低功耗测试, CPU无法访问LPRAM

typedef struct {
    volatile unsigned long *scon;
    volatile unsigned long *spr;
    volatile unsigned long *adr;
} senshub_sfr_t;

const senshub_sfr_t SENSHB_SFR[] = {
    {&SENSHBSCON0, &SENSHBSPR0, &SENSHBADR0},
    {&SENSHBSCON1, &SENSHBSPR1, &SENSHBADR1},
    {&SENSHBSCON2, &SENSHBSPR2, &SENSHBADR2},
    {&SENSHBSCON3, &SENSHBSPR3, &SENSHBADR3},
    {&SENSHBSCON4, &SENSHBSPR4, &SENSHBADR4},
};

typedef struct
{
    u8 cmd_idx;
    u8 data_idx;
    uint8_t  cmd[30];
    uint32_t dat[30];
} senshb_instruct_t;

typedef struct {
    const senshub_cfg_t *cfg;
    senshb_instruct_t *instruct;
    u8 read_addr_idx[READ_CNT_MAX];
    u8 read_addr_cnt;
} bsp_senshub_t;


static senshb_instruct_t instruct_cache[USE_SENSHUB_MAX] AT(.sensorhub.buf);
static bsp_senshub_t senshub_table[USE_SENSHUB_MAX] = {0};
static u32 sniff_rc_clk;

AT(.com_rodata.sens.str)
const char c_str[] = "SENSHBCPND:%x, SENSHBTCON:%x, channel:%d\n";

AT(.com_rodata.sens.str)
const char err0_str[] = "[%d]channel err.\n";

AT(.com_rodata.sens.str)
const char err1_str[] = "[%d]cfg err.\n";


void dbg_clk_out2(u32 type, u32 div)
{
    /// clkcon0[15:13] output clock:
    /// 1 -> xosc26m, 2 -> xosc26m_32k, 3 -> osc32, 4 -> pll0div4_clk, 5 -> xosc52m
    /// 6 -> bt26m_clk, 7 -> rc2m, 8 -> rtc_rc2m, 9 -> sys_clk, 10 -> dac_clk
    if (!div) {
        div = 1;
    }
    CLKCON0 = (CLKCON0 & ~(0x0f << 27)) | (type << 27);        //select output source
    CLKDIVCON2 = (CLKDIVCON0 & ~(0x1f << 8)) | ((div - 1) << 8);  //clock output div

    GPIOEDE |= BIT(7);
    GPIOEDIR &= ~BIT(7);
    GPIOEFEN |= BIT(7);
    FUNCMCON1 = 6;
}

AT(.com_text.sensor_hub)
void bsp_sensor_hub_lowpower_en(bool enable)
{
    RTCCON11 = (RTCCON11 & ~BIT(12)) | (BIT(12) * enable);
    asm("nop");
    asm("nop");
//	delay_us(10);
}

AT(.com_text.bsp.key)
void bsp_sensor_hub_reg_dump(u8 n)
{
    printf("\n--------------------------------------------\n");
    printf("RTCCON0:%x, CLKGAT1:%x\n", RTCCON0, CLKGAT1);
    printf("SENSHBSCON0:%x, SENSHBSCON0:%x\n", SENSHBSCON0, SENSHBSCON1);
    printf("SENSHBCPND:%x\n", SENSHBCPND);
//    printf("SENSHBCLR:%x\n", SENSHBCLR);
    printf("SENSHBADR0:%x, SENSHBADR0:%x\n", SENSHBADR0, SENSHBADR1);
    printf("SENSHBCON:%x\n", SENSHBCON);
    printf("SENSHBTPR:%x\n", SENSHBTPR);
    printf("SENSHBTCON:%x\n", SENSHBTCON);
    printf("SENSHBSPR0:%x, SENSHBSPR1:%x\n", SENSHBSPR0, SENSHBSPR1);

    printf("IIC0CMDA:%x\n", IIC0CMDA);
    printf("IIC0CON0:%x\n", IIC0CON0);
    printf("IIC0CON1:%x\n", IIC0CON1);
    printf("IIC0DMACNT:%x\n", IIC0DMACNT);
    printf("IIC0DMAADR:%x\n", IIC0DMAADR);
    printf("GPIOEFEN:%x\n", GPIOEFEN);

    SENSHBTPR &= ~(0xffff0000);
    SENSHBTPR |= 0x00030000;

    for(u8 i=0;i<n;i++) {
        printf("[%d] cmd:%x, dat:%x\n", i, senshub_table[0].instruct->cmd[i], senshub_table[0].instruct->dat[i]);
    }

    printf("\n--------------------------------------------\n");
}

AT(.com_text.sens_isr)
void bsp_senshb_isr(void)
{
    u32 cpnd = SENSHBCPND;
    u8 channel = 0xff;
    if (cpnd) {
        for (u8 i = 0; i < 5; i++) {
            if (cpnd & (SENSHB_DEV_PND_MSK << i)) {
                channel = i;
                break;
            }
        }
        printf(c_str, cpnd, SENSHBTCON, channel);
        if (channel >= 5) {
            printf(err0_str, channel);
            return ;
        }
        if (senshub_table[channel].cfg == NULL) {
            printf(err1_str, channel);
            return ;
        }
        u8 *p_buf = senshub_table[channel].cfg->sample_buf;
        u16 sample_size = 0;
        for (u8 i = 0; i < READ_CNT_MAX; i++) {
            sample_size += senshub_table[channel].cfg->read_size[i];
        }
        sample_size *= senshub_table[channel].cfg->sample_cnt;

        if (cpnd & BIT(SENSHB_HDONE_SHF + channel)) {
            SENSHBCPND = BIT(SENSHB_HDONE_SHF + channel);
            //print_r(p_buf, sample_size);
        }

        if(cpnd & BIT(SENSHB_ADONE_SHF + channel)) {
            //print_r(p_buf, sample_size);
            if (senshub_table[channel].cfg->done_callback) {
                senshub_table[channel].cfg->done_callback(p_buf, sample_size);
            }
            memset(p_buf, 0, sample_size);
            //reset buf addr
            for (u8 i = 0; i < senshub_table[channel].read_addr_cnt; i++) {
                u16 dat_idx = senshub_table[channel].read_addr_idx[i];
                senshub_table[channel].instruct->dat[dat_idx] = DMA_ADR(p_buf);
                p_buf += (senshub_table[channel].cfg->read_size[i] * senshub_table[channel].cfg->sample_cnt);
            }
            SENSHBCPND = BIT(SENSHB_ADONE_SHF + channel);
            SENSHBCLR = BIT(1 + channel);
        }
    }

	if (cpnd & BIT(SENSHB_WKFG_PND_SHF + channel)) {
        SENSHBCPND = BIT(SENSHB_WKFG_PND_SHF + channel);
	}
}

AT(.com_text.bsp.key)
void bsp_senshb_lp_process(void)
{
    bsp_sensor_hub_lowpower_en(false);
    bsp_senshb_isr();
    bsp_sensor_hub_lowpower_en(true);
}

static void sensor_hub_init(void)
{
    SENSHBCON &= ~BIT(SENSHB_EN_SHF); // RESET
    SENSHBTCON = 0; // RESET
    delay_ms(100);
    SENSHBCON |= BIT(SENSHB_EN_SHF);
    SENSHBCLR = -1UL;
    SENSHBCPND = -1UL;
}

static uint32_t bsp_sensor_hub_dev_period_init(uint8_t channel, bool enable, uint32_t ms, uint32_t iic_timeout_us, u32 clk)
{
    uint32_t timeout = 0;
    uint32_t pr      = (uint32_t)ROUND_DIV((uint64_t)ms * clk, ((SENSHBTPR + 1) * 1000 ) & 0xffff); // 偏差最大还是有可能到basepr/2
    uint32_t pr_cfg  = pr;

    *SENSHB_SFR[channel].scon  &= ~BIT(SENSHB_MODE_SHF);

    if (iic_timeout_us) {
        timeout = (uint32_t)ROUND_DIV((uint64_t)iic_timeout_us * clk, 1000000) & 0xffff;
        pr_cfg |= timeout << 16;
    }
    *SENSHB_SFR[channel].spr = pr_cfg;
    printf("%s set timer trig mode period %d us, timeout %d us\n", __func__,
           (uint32_t)((uint64_t)pr      * ((SENSHBTPR&0xffff)+1) * 1000000 / clk),
           (uint32_t)((uint64_t)timeout * 1000000 / clk));

    return pr;
}

static void bsp_sensor_hub_dev_en(uint8_t channel, bool en)
{
    if (en)
        *SENSHB_SFR[channel].scon |= BIT(SENSHB_SEN_SHF);
    else
        *SENSHB_SFR[channel].scon &= ~BIT(SENSHB_SEN_SHF);
}

static void bsp_sensor_hub_dev_dma_init(uint8_t channel, uint8_t *cmd, uint32_t *dat, uint32_t round)
{

    *SENSHB_SFR[channel].scon = ((*SENSHB_SFR[channel].scon) & ~(BIT(SENSHB_DMA_CNT_SHF) * 0xff)) | BIT(SENSHB_DMA_CNT_SHF) * round | BIT(SENSHB_IIC_DMAADR_UPT);
    *SENSHB_SFR[channel].adr = (DMA_ADR(cmd) & 0xffff) | ((DMA_ADR(dat) & 0xffff) << 16);
    printf("%s channel:%d, CMD %08x, DAT %08x, round %d\n", __func__,channel, cmd, dat, round);
}

static void bsp_sensor_hub_dev_wake_en(uint8_t channel, bool half, bool all)
{

    *SENSHB_SFR[channel].scon &= ~(BIT(SENSHB_HDONE_EN_SHF) | BIT(SENSHB_ADONE_EN_SHF));
    *SENSHB_SFR[channel].scon |= BIT(SENSHB_HDONE_EN_SHF) * half |
                      BIT(SENSHB_ADONE_EN_SHF) * all;
}

static void bsp_sensor_hub_dev_except_en(uint8_t channel, bool err, bool nack, bool timeout)
{

    *SENSHB_SFR[channel].scon = ((*SENSHB_SFR[channel].scon) & ~(0x7 << SENSHB_SERR_EN_SHF)) |
                     BIT(SENSHB_SERR_EN_SHF)   * err   |
                     BIT(SENSHB_SNACK_EN_SHF)  * nack  |
                     BIT(SENSHB_STO_EN_SHF)    * timeout;
}

static void bsp_sensor_hub_dev_adone_pause_en(uint8_t channel, bool en)
{
    if (en){
        *SENSHB_SFR[channel].scon |= BIT(SENSHB_SPAUSE_EN_SHF);
    }else{
        *SENSHB_SFR[channel].scon &= ~BIT(SENSHB_SPAUSE_EN_SHF);
    }
}

static void bsp_sensorhub_channel_init(uint8_t channel, uint8_t *cmd, uint32_t *dat, uint32_t rounds)
{
    printf("%s,chanel:%d\n",__func__,channel);
    bsp_sensor_hub_dev_en(channel, true);
    bsp_sensor_hub_dev_dma_init(channel, cmd, dat, rounds);
    bsp_sensor_hub_dev_except_en(channel, true, true, true);
    bsp_sensor_hub_dev_wake_en(channel, true, true);
    bsp_sensor_hub_dev_adone_pause_en(channel, true);
}

uint32_t bsp_sensorhub_sample_period_set(uint8_t channel, u32 sample_rate, u32 timeout)
{
    return bsp_sensor_hub_dev_period_init(channel, true, 1000 / sample_rate, timeout, sniff_rc_clk);
}

void bsp_sensor_bub_base_timer_init(bool enable, uint32_t pr)
{
    SENSHBTPR  = (pr - 1) | 0xffff0000;
    printf("### SENSHBTPR:%x\n", SENSHBTPR);
}

uint64_t bsp_sensor_hub_base_pr_set(uint32_t us, u32 clk)
{
//    uint64_t pr = (uint64_t)us * clk / 1000000;
    uint64_t pr = ROUND_DIV((uint64_t)us * clk, 1000000);
    printf("pr:%d\n", pr);
    printf("main timer set period %d us\n", (uint64_t)pr * 1000000 / clk);

    bsp_sensor_bub_base_timer_init(true, pr);
    return pr;
}

void bsp_sensor_hub_tmr_start(void)
{
    SENSHBTCON |= BIT(SENSHB_TMR_EN_SHF);
}

void bsp_sensor_hub_tmr_hold(bool hold)
{
    if (hold){
        SENSHBTCON |= BIT(SENSHB_TMR_HOLD_SHF);
    }else{
        SENSHBTCON &= ~BIT(SENSHB_TMR_HOLD_SHF);
    }
}

bool bsp_is_sensor_hub_tmr_hold(void)
{
    if (!(SENSHBCON & BIT(SENSHB_EN_SHF))) {
        return true;
    }

    bool senhb_iic_en = !!(SENSHBCON & BIT(SENSHB_IIC_EN_SHF+0));
    bool senhb_tmr_hold_en = !!(SENSHBTCON & BIT(SENSHB_TMR_HOLD_SHF));

    printf("[%d, %d]\n", !senhb_iic_en, senhb_tmr_hold_en);

    return (!senhb_iic_en && senhb_tmr_hold_en);
}

AT(.com_text.sensor_hub)
void bsp_sensor_hub_int_en(bool enable)
{
    SENSHBCON = (SENSHBCON & ~BIT(SENSHB_IE_SHF)) |
               BIT(SENSHB_IE_SHF) * enable;
    if (enable) {
        sys_irq_init(IRQ_I2C_VECTOR, 0, bsp_senshb_isr);
    }
}

void bsp_sensor_hub_wakeup_en(bool enable)
{
    SENSHBCON = (SENSHBCON & ~BIT(SENSHB_WKUP_EN_SHF)) | BIT(SENSHB_WKUP_EN_SHF) * enable;
}

void bsp_sensor_hub_iic_en(uint32_t niic, bool enable)
{
    if (enable) {
        SENSHBCON |= BIT(SENSHB_IIC_EN_SHF+niic);
    } else {
        SENSHBCON &= ~BIT(SENSHB_IIC_EN_SHF+niic);
    }
}

void bsp_sensor_hub_resume(void)
{
    SENSHBCLR |= BIT(SENSHB_HOLD_CLR_SHF);
}

void bsp_sensor_hub_hold(void)
{
    SENSHBTCON |= BIT(SENSHB_HOLD_SET_SHF);
    if (SENSHBCON & BIT(SENSHB_IE_SHF)){

    }else{
        while(!(SENSHBCPND & BIT(SENSHB_HOLD_PND_SHF)));
        SENSHBCPND = BIT(SENSHB_HOLD_PND_SHF);
    }
}

WEAK void bsp_sensor_hub_set_hold(bool en)
{
    if (en) {
        SENSHBCON &= ~BIT(SENSHB_IE_SHF);
        bsp_sensor_hub_tmr_hold(true);
        bsp_sensor_hub_hold();
        bsp_sensor_hub_iic_en(0, false);
        IIC0DMACNT = 0;                         // Disable DMA
    } else {
        bsp_sensor_hub_resume();
        bsp_sensor_hub_iic_en(0, true);
        SENSHBCON |= BIT(SENSHB_IE_SHF);
        bsp_sensor_hub_tmr_hold(false);
    }
    printf("senshbcon:0x%08x\n", SENSHBCON);
}

u32 bsp_sensor_hub_config(u8 mapping, bool wku_en)
{
    uint32_t con = BIT(SENSHB_IIC_MAP_SHF) * mapping    |
           BIT(SENSHB_IIC_EN_SHF)                       |
           BIT(SENSHB_LPCLK_EN_SHF)                     |
           BIT(SENSHB_WKUP_EN_SHF)        * wku_en      |
           BIT(SENSHB_EN_SHF);

    return con;
}

void bsp_senshb_set(void *sens, SENSHUB_SFR sfr, SENSHUB_IIC_OPERATE cmd, uint32_t data)
{
    bsp_senshub_t *sens_tmp = (bsp_senshub_t *)sens;
    senshb_instruct_t *ins = sens_tmp->instruct;
    ins->cmd[ins->cmd_idx] = SENSHB_CMD_CFG(sfr, cmd);

    if (data) {
        ins->dat[ins->data_idx] = data;
        if (SENSHB_IICxDMAADR == sfr && SENSHB_WREG == cmd) {
            sens_tmp->read_addr_idx[sens_tmp->read_addr_cnt++] = ins->data_idx;
        }
        ins->data_idx++;
    }
    ins->cmd_idx++;
    printf("### cmd[%d]:%x, dat[%d]:%x\n", ins->cmd_idx-1, ins->cmd[ins->cmd_idx-1], ins->data_idx-1, ins->dat[ins->data_idx-1]);
}

void bsp_sensor_hub_init(void)
{
    sniff_rc_clk = rtc_get_freq() / 100;
    printf("sniff_rc_clk:%d\n", sniff_rc_clk);
    delay_5ms(40);
    //dbg_clk_out2(3, 0);

    SENSHBCPND = 0xffffffff;
    sensor_hub_init();
    bsp_sensor_hub_wakeup_en(true);
    bsp_sensor_hub_base_pr_set(1000, sniff_rc_clk);
    bool have_sens = false;

    for (int i = 0; i < USE_SENSHUB_MAX; i++) {
        if (senshub_table[i].cfg) {
            printf("cfg[%d]:device_id=%d,sample_rate=%d,sample_cnt=%d\n",
                i,senshub_table[i].cfg->device_id, senshub_table[i].cfg->sample_rate,senshub_table[i].cfg->sample_cnt);

            //sensub data buf init
            u16 sample_size = 0;
            for (u8 j = 0; j < READ_CNT_MAX; j++) {
                sample_size += senshub_table[i].cfg->read_size[j];
            }
            sample_size *= senshub_table[i].cfg->sample_cnt;
            memset(senshub_table[i].cfg->sample_buf, 0, sample_size);

            //instruc cfg
            senshub_table[i].instruct = &instruct_cache[i];
            memset(senshub_table[i].instruct, 0, sizeof(senshb_instruct_t));
            memset(senshub_table[i].read_addr_idx, 0, sizeof(senshub_table[i].read_addr_idx));
            senshub_table[i].read_addr_cnt = 0;

            //exec cfg
            if (senshub_table[i].cfg->exec_callback) {
                senshub_table[i].cfg->exec_callback((void *)&senshub_table[i], i);
            }

            //loop
            u32 cmd_adr = DMA_ADR(&senshub_table[i].instruct->cmd);
            u32 data_adr = DMA_ADR(&senshub_table[i].instruct->dat);
            bsp_sensorhub_channel_init(i, (uint8_t *)cmd_adr, (uint32_t *)data_adr, senshub_table[i].cfg->sample_cnt);

            have_sens = true;
        }
    }

    if (have_sens) {
        bsp_sensor_hub_tmr_start();
    }
#if TRACE_EN
    bsp_sensor_hub_reg_dump(20);
#endif
}

void bsp_sensorhub_process(void)
{
    static u32 ticks = 0;
    if (tick_check_expire(ticks, 20)) {
        ticks = tick_get();
        WDT_CLR();
    #if LPM_EN
        bsp_sensor_hub_int_en(false);
        bsp_sensor_hub_lowpower_en(true);
        GLOBAL_INT_DISABLE();     //关总中断
        WDT_DIS();
        RTCCON12 |= (0x3<<6);                       //DIS RTC_WDT
        LPMCON |= BIT(0);
        asm("nop");asm("nop");asm("nop");
        GLOBAL_INT_RESTORE();     //开总中断
        bsp_senshb_isr();
    #else
        bsp_senshb_isr();
    #endif
    }
}

bool bsp_sensorhub_reg(const senshub_cfg_t *cfg)
{
    if (cfg == NULL) {
        TRACE("%s, cfg is null\n", __func__);
    }
    for (u8 i = 0; i < USE_SENSHUB_MAX; i++) {
        if (senshub_table[i].cfg == NULL) {
            senshub_table[i].cfg = cfg;
            return true;
        }
    }
    TRACE("%s, sensorhub is full\n", __func__);

    return false;
}


#endif //SENSOR_HUB_EN

void bsp_sensor_hub_io_init(u8 mapping)
{
    printf("%s:%d\n", __func__, mapping);
    switch (mapping) {
    case SENSHBMAP_G1:
        GPIOEDIR |= BIT(7) | BIT(8);                    //SCL SDA
        GPIOEPU |= BIT(7) | BIT(8);
        GPIOEDE |= BIT(7) | BIT(8);
        GPIOEFEN &= ~(BIT(7) | BIT(8));
        break;

    case SENSHBMAP_G2:
        GPIOEDIR |= BIT(9) | BIT(10);                    //SCL SDA
        GPIOEPU |= BIT(9) | BIT(10);
        GPIOEDE |= BIT(9) | BIT(10);
        GPIOEFEN &= ~(BIT(9) | BIT(10));
        break;

    case SENSHBMAP_G3:
        GPIOEDIR |= BIT(11) | BIT(12);                    //SCL SDA
        GPIOEPU |= BIT(11) | BIT(12);
        GPIOEDE |= BIT(11) | BIT(12);
        GPIOEFEN &= ~(BIT(11) | BIT(12));
        break;

    case SENSHBMAP_G4:
        GPIOEDIR |= BIT(3) | BIT(4);                    //SCL SDA
        GPIOEPU |= BIT(3) | BIT(4);
        GPIOEDE |= BIT(3) | BIT(4);
        GPIOEFEN &= ~(BIT(3) | BIT(4));
        break;

    default:
        break;
    }

    SENSHBCON = BIT(SENSHB_IIC_MAP_SHF) * mapping | BIT(SENSHB_IIC_EN_SHF) | BIT(SENSHB_LPCLK_EN_SHF);
    printf("senshbcon:0x%08x\n", SENSHBCON);
}

void bsp_sensor_hub_clk_init(void)
{
    printf("%s\n", __func__);
    CLKGAT1 |= BIT(3);
    RTCCON0 |= BIT(0);      //RTC en
    RTCCON0 |= BIT(2);      //RTC clk to module
//    RTCCON0 &= ~BIT(22);     //SNF_RC_EN
    RTCCON0 |= BIT(20);     //SNF_RC_EN
    RTCCON0 = (RTCCON0 & ~(BIT(14) | BIT(15))) | BIT(15);	//sniff rc
    RTCCON0 = (RTCCON0 & ~(BIT(8) | BIT(9))) | BIT(9);		//sniff rc
    SENSHBCON |= BIT(5+0*4) * 1 | BIT(3+0) | BIT(SENSHB_LPCLK_EN_SHF);
    printf("senshbcon:0x%08x\n", SENSHBCON);
}




