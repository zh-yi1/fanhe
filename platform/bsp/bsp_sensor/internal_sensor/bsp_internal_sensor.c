#include "include.h"

extern  u32 sys_get_rc2m_clkhz(u32 type);
#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif
#if (SENSOR_STEP_SEL == SENSOR_STEP_INTER)


//时钟源选择
#define SENS_CLK_RC2M_RTC       1
#define SENS_CLK_SNF_RC_RTC     2
#define IR_CLK_X26MDIV16_RTC    3      // 休眠后会造成死机
#define SENS_CLK          SENS_CLK_SNF_RC_RTC  // SENS_CLK_RC2M_RTC //IR_CLK_X26MDIV16_RTC
#define SENS_CLK_DIV      50

u32 sensor_clk;

#define SENS_CLK_FREQ           (sensor_clk/(SENS_CLK_DIV))

//SENSCON
#define SENSOR_SENS_EN          0
#define SENSOR_HIGH_IE          1
#define SENSOR_LOW_IE           2
#define SENSOR_VLD_LVL_SEL      3
#define SENSOR_FIFO_IE          5
#define SENSOR_WKUP_EN          6
#define SENSOR_FIFO_FULL_IE     7
#define SENSOR_FIFO_FULL_PND    12
#define SENSOR_FIFO_PND         13
#define SENSOR_LOW_PND          14
#define SENSOR_HIGH_PND         15
#define SENSOR_VLDLEN           16

//SENSCON1
#define SENSOR_SENS_DIV         0
#define SENSOR_PCLK_EN          0

//最小有效电平时间，ms
#define MIN_VALID_TIME          20
#define VLDLEN                  (SENS_CLK_FREQ * MIN_VALID_TIME / 1000)

// 模块说明
/*
数据可以记录高有效也可以记录低有效    BIT(SENSOR_VLD_LVL_SEL)   *  HIGH_VLD
有数据的时候产生中断可以右三种选择：
        数据记录满，         bsp_internal_sensor_fifofullIe_en(1);
        高数据               bsp_internal_sensor_highIe_en(1);
        低数据触发           bsp_internal_sensor_lowIe_en(1);

高低数据都有效的时候，在休眠的时候记录的数据无法区分是高还是低(没有休眠可以区分)

模块开启后功耗回增加 初步测试回增加350uA左右，实际跟对应脚位漏电造成的

休眠后注意对应的gpio脚设定，否则会导致模块功能失效

*/
volatile static u32 sensor_level_mask;  // 最多保存32个数据，每一位表示是高电平还是低电平，为1表示高，为0表示低
volatile static u8 sensor_data_cnt = 0; // 记录fifo的数据，方便记录每位数据是高还是低，实际测试进入休眠后，中断不能及时响应会有问题
static co_timer_t inter_sensor_500ms;  // 阈值是20ms，最大保存数据是32个，即数据存满至少需要640ms，因此可以不使用 SENSOR_FIFO_FULL_PND

static const u8 gpio_num_table[] = {IO_PE6, IO_PE3, IO_PB7};

void bsp_internal_sensor_lowIe_en(uint8_t flag)
{
    if (flag)
        SENSCON |=  BIT(SENSOR_LOW_IE);
    else
        SENSCON &= ~BIT(SENSOR_LOW_IE);
}

void bsp_internal_sensor_highIe_en(uint8_t flag)
{
    if (flag)
        SENSCON |=  BIT(SENSOR_HIGH_IE);
    else
        SENSCON &= ~BIT(SENSOR_HIGH_IE);
}


void bsp_internal_sensor_vld_sel(sensor_vld_t sel)
{
    SENSCON  = (SENSCON & ~(0x3 << SENSOR_VLD_LVL_SEL)) | (sel << SENSOR_VLD_LVL_SEL);
}

void bsp_internal_sensor_fifoIe_en(uint8_t flag)
{
    if (flag)
        SENSCON |=  BIT(SENSOR_FIFO_IE);
    else
        SENSCON &= ~BIT(SENSOR_FIFO_IE);
}

void bsp_internal_sensor_fifofullIe_en(uint8_t flag)
{
    if (flag)
        SENSCON |=  BIT(SENSOR_FIFO_FULL_IE);
    else
        SENSCON &= ~BIT(SENSOR_FIFO_FULL_IE);
}

void bsp_internal_sensor_wake_en(uint8_t flag)
{
    if (flag)
        SENSCON |=  BIT(SENSOR_WKUP_EN);
    else
        SENSCON &= ~BIT(SENSOR_WKUP_EN);
}

void bsp_internal_sensor_vldlen_set(uint16_t vldlen)
{
    SENSCON  = (SENSCON & ~(0xffff << SENSOR_VLDLEN)) | (vldlen << SENSOR_VLDLEN);
}

#if TRACE_EN
AT(.com_rodata)
const char bsp_internal_sensor_fiq_isr[] = " sensor_level_mask: %x cnt:%8d \n";
#endif

AT(.com_text.sensor)
void bsp_internal_sensor_fiq(void)
{
   // TRACE(bsp_internal_sensor_fiq_isr,  sensor_level_mask, sensor_data_cnt );

/*
    if(SENSCON & BIT(SENSOR_FIFO_FULL_PND))
    {
        SENSCPND = BIT(SENSOR_FIFO_FULL_PND);
    }
*/

    if(SENSCON & BIT(SENSOR_HIGH_PND))
    {
        SENSCPND = BIT(SENSOR_HIGH_PND);
        sensor_level_mask |= BIT(sensor_data_cnt);
        //sensor_data_cnt ++;
    }

    if(SENSCON & BIT(SENSOR_LOW_PND))
    {
        SENSCPND = BIT(SENSOR_LOW_PND);
        //sensor_data_cnt ++;
    }
    sensor_data_cnt ++;

}

////////////////////////////////////////////////////////////////////////////////////
void bsp_internal_sensor_dump_reg(void)
{
    TRACE("SENSCON         = %08x\n",SENSCON);
    TRACE("SENSCON1        = %08x\n",SENSCON1);
    TRACE("SENS_CLK_FREQ   = %d\n",SENS_CLK_FREQ);
    TRACE("RTCCON0         = %08x\n", RTCCON0);
}
// 设定sensor 对应的IO
void bsp_internal_sensor_io_init(u8 gpio_num,  u8 group)
{
    gpio_t gpio;
    gpio_cfg_init(&gpio, gpio_num);
    TRACE("gpio.num: %d group: %d \n", gpio.num, group);
    gpio.sfr[GPIOxDE] |= BIT(gpio.num);
    gpio.sfr[GPIOxSET] |= BIT(gpio.num);
    gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
    gpio.sfr[GPIOxPU] |= BIT(gpio.num);

    SENSCON1 = (SENSCON1 & ~(BIT(8) &0x03)) |(BIT(8)*group);
}

void bsp_internal_sensor_clk_init(u8 clk_sel, u8 div)
{
    RTCCON0 = (RTCCON0 & (~(BIT(10)*0x03)))| (BIT(10) * clk_sel);
    SENSCON1 |= (BIT(7) |div);
}

void bsp_internal_sensor_process(co_timer_t *timer, void *param)
{
    TRACE("sensor_level_mask: %x sensor_data_cnt: %d \n", sensor_level_mask, sensor_data_cnt);
    GLOBAL_INT_DISABLE();
    //u8 data_cnt = sensor_data_cnt;
    u32 level_mask = sensor_level_mask;
    sensor_data_cnt = 0;
    sensor_level_mask = 0;
    GLOBAL_INT_RESTORE();

    u8 i = 0;
    while (SENSCON & BIT(SENSOR_FIFO_PND)) {
        u32 timer_len = (SENSCNT & 0x0000ffff); //    timer_len 为0xffff的时候表示一直处于高或者低
        TRACE("is_high:%d times %8d ms  timer_len: %x \n", ((level_mask & BIT(i))>> i), timer_len * 1000 / SENS_CLK_FREQ, timer_len);
        i++;
    }

}


void bsp_internal_sensor_init(void)
{

//不同时钟下的实际频率
#if SENS_CLK == SENS_CLK_RC2M_RTC
    sensor_clk = sys_get_rc2m_clkhz(1);
#elif SENS_CLK == SENS_CLK_SNF_RC_RTC
    sensor_clk = sys_get_rc2m_clkhz(2);
#elif SENS_CLK == IR_CLK_X26MDIV16_RTC
    sensor_clk = 1625000;
#endif
    TRACE("%s sensor_clk: %d \n",__func__, sensor_clk);
    bsp_internal_sensor_clk_init(SENS_CLK, SENS_CLK_DIV-1);
    bsp_internal_sensor_io_init(gpio_num_table[INTER_SENSOR_PB7], INTER_SENSOR_PB7); // PB7

	SENSCON = 0 |
		BIT(SENSOR_SENS_EN)       *  1  |
		BIT(SENSOR_VLD_LVL_SEL)   *  LOW_VLD  |
		BIT(SENSOR_WKUP_EN)       *  1  |
		BIT(SENSOR_VLDLEN)        *  VLDLEN
		;

    u8 result = sys_irq_init(23, 0, bsp_internal_sensor_fiq);//  bsp_register_fiq(23, bsp_internal_sensor_fiq);
    TRACE("%s result: %d \n",__func__, result);

    // bsp_internal_sensor_fifo_en(0);
   // bsp_internal_sensor_vldlen_set(3);
    bsp_internal_sensor_fifofullIe_en(0);
    bsp_internal_sensor_fifoIe_en(0);
    bsp_internal_sensor_highIe_en(0);
    bsp_internal_sensor_lowIe_en(0);
    bsp_internal_sensor_dump_reg();
    sensor_level_mask = 0;
    sensor_data_cnt = 0;

    TRACE("start  SENSCON: %x  RTCCON0: %x SENSCON1: %x \n", SENSCON,  RTCCON0, SENSCON1);
    co_timer_set(&inter_sensor_500ms, 500, TIMER_REPEAT, LEVEL_LOW_PRI, bsp_internal_sensor_process, NULL);
    co_timer_set_sleep(&inter_sensor_500ms, true);
}

void bsp_internal_sensor_stop(void)
{
    TRACE("bsp_internal_sensor_stop \n");
    SENSCON = 0 |
        BIT(SENSOR_SENS_EN)       *  0  |
        BIT(SENSOR_VLD_LVL_SEL)   *  0  |
        BIT(SENSOR_WKUP_EN)       *  0  |
        BIT(SENSOR_VLDLEN)        *  VLDLEN
        ;
    co_timer_del(&inter_sensor_500ms);
}

#endif
