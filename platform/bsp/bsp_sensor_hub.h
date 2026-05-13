#ifndef _SENSOR_HUB_H
#define _SENSOR_HUB_H

// CON
#define SENSHB_IIC_MAP_SHF      5
#define SENSHB_IE_SHF           4
#define SENSHB_IIC_EN_SHF       3
#define SENSHB_LPCLK_EN_SHF     2 // 对FPGA无影响，实际芯片需要使能
#define SENSHB_WKUP_EN_SHF      1
#define SENSHB_EN_SHF           0

// CLR
#define SENSHB_SDMACNT_CLR_SHF  6
#define SENSHB_SPAUSE_CLR_SHF   1
#define SENSHB_HOLD_CLR_SHF     0

// TCON
#define SENSHB_BCNT             16
#define SENSHB_STM_MSK          0x7
#define SENSHB_STM_SHF          6
#define SENSHB_TOID_MSK         0x7
#define SENSHB_TOID_SHF         3
#define SENSHB_HOLD_SET_SHF     2
#define SENSHB_TMR_HOLD_SHF     1
#define SENSHB_TMR_EN_SHF       0

// SCON
#define SENSHB_DMA_RCNT_SHF      24
#define SENSHB_DMA_CNT_SHF       16
#define SENSHB_IIC_DMAADR_UPT    14
#define SENSHB_SPAUSE_EN_SHF     13
#define SENSHB_SPTO_EN_SHF       12
#define SENSHB_STO_EN_SHF        11
#define SENSHB_SNACK_EN_SHF      10
#define SENSHB_SERR_EN_SHF       9
#define SENSHB_ADONE_EN_SHF      8
#define SENSHB_HDONE_EN_SHF      7
#define SENSHB_TRIG_MAP_SHF      2
#define SENSHB_MODE_SHF          1
#define SENSHB_SEN_SHF           0

// CPND
#define SENSHB_HOLD_PND_SHF     31
#define SENSHB_WKFG_PND_SHF     26
#define SENSHB_IIC_TOPND_SHF    25
#define SENSHB_TRIG_TOPND_SHF   20
#define SENSHB_NACKPND_SHF      15
#define SENSHB_ERRPND_SHF       10
#define SENSHB_ADONE_SHF        5
#define SENSHB_HDONE_SHF        0
#define SENSHB_PND_STEP         5
#define SENSHB_PND_CNT          5
#define SENSHB_CPND_ISR_MSK     ~(0x1f << SENSHB_WKFG_PND_SHF)

#define SENSHB_DEV_PND_MSK      (BIT(SENSHB_HDONE_SHF) | BIT(SENSHB_ADONE_SHF) | BIT(SENSHB_ERRPND_SHF) | BIT(SENSHB_NACKPND_SHF) | BIT(SENSHB_TRIG_TOPND_SHF))


#define USE_SENSHUB_MAX                    2                //at most 5

#define SENSHBMAP_G1                       1                //SCL:PE8 , SDA:PE7
#define SENSHBMAP_G2                       2                //SCL:PE10, SDA:PE9
#define SENSHBMAP_G3                       3                //SCL:PE12, SDA:PE11
#define SENSHBMAP_G4                       4                //SCL:PE4 , SDA:PE3

#define READ_CNT_MAX                       3                //每次采样读取寄存器最大次数

typedef enum{
    SENSHB_IICxCON0 = 0,
    SENSHB_IICxCON1,
    SENSHB_IICxCMDA,
    SENSHB_IICxDATA,
    SENSHB_IICxSSTS,
    SENSHB_IICxDMACNT,
    SENSHB_IICxDMAADR,
    SENSHB_CON,
    SENSHB_SPR0,
    SENSHB_SPR1,
    SENSHB_SPR2,
    SENSHB_SPR3,
    SENSHB_SPR4,

} SENSHUB_SFR;

typedef enum{
    SENSHB_WREG = 1,
    SENSHB_WAIT,
    SENSHB_CACK,
    SENSHB_UPAD,
    SENSHB_DONE = 7,
} SENSHUB_IIC_OPERATE;

typedef struct {
    uint16_t device_id;              //传感器设备ID（不带IIC读写位）
    uint16_t sample_rate;            //传感器采样率
    uint16_t sample_cnt;             //采样次数
    uint16_t read_size[READ_CNT_MAX];//单次采样数据连续读取的大小
    u8 *sample_buf;                  //采样数据buf
    void (*exec_callback)(void *senshub_table, uint8_t channel);
    void (*done_callback)(u8 *pbuf, uint16_t len);
} senshub_cfg_t;

void bsp_sensor_hub_init(void);
void bsp_sensor_hub_clk_init(void);
void bsp_sensor_hub_set_hold(bool en);
void bsp_sensor_hub_io_init(u8 mapping);
void bsp_sensorhub_process(void);
void bsp_senshb_isr(void);
void bsp_senshb_lp_process(void);
void bsp_sensor_hub_int_en(bool enable);
void bsp_sensor_hub_lowpower_en(bool enable);
u32 bsp_sensor_hub_config(u8 mapping, bool wku_en);

bool bsp_sensorhub_reg(const senshub_cfg_t *cfg);
uint32_t bsp_sensorhub_sample_period_set(uint8_t channel, u32 sample_rate, u32 timeout);
void bsp_senshb_set(void *sens, SENSHUB_SFR sfr, SENSHUB_IIC_OPERATE cmd, uint32_t data);

#endif // _SENSOR_HUB_H
