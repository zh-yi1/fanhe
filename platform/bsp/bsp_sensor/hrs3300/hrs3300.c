#include <include.h>

// #include <stdio.h>
// #include <stdbool.h>
//#include "SEGGER_RTT.h"
//#include "hrs3300_i2c.h"
//#include "app_timer.h"

#if (SENSOR_HR_SEL == SENSOR_HR_TYHX_HRS3300)

//////////////////////////////
#include "hrs3300.h"
//#include "hrs3300_alg.h"
#include "hrs3300_reg_init.h"
#include "fit_hrs3300.h"
//////////////////////////////
//#define GSENSER_DATA

#define HR_ADDR			(0x44 << 1)

#define nrf_delay_ms(ms)    delay_ms(ms)

// hrs3300 customer config
const uint8_t  hrs3300_bp_timeout_grade = 0;  // max 15
const uint8_t  hrs3300_agc_init_stage = 0x04;  // init AGC state
const uint8_t  hrs3300_bp_power_grade = 0;
const uint8_t  hrs3300_accurate_first_shot = 0;
const uint8_t  hrs3300_up_factor = 3;
const uint8_t  hrs3300_up_shift = 2;
const uint16_t hrs3300_AMP_LTH = 120;
const uint16_t hrs3300_hr_AMP_LTH = 150;
const uint16_t hrs3300_hr_PVAR_LTH = 10;
const uint8_t hrs3300_temp_detect_en = 1;
// hrs3300 customer config end

//20161117 added by ericy for "low power in no_touch state"
static bool hrs3300_power_up_flg = 0 ;
uint8_t reg_0x7f ;
uint8_t reg_0x80 ;
uint8_t reg_0x81 ;
uint8_t reg_0x82 ;
//20161117 added by ericy for "low power in no_touch state"

// static uint8_t i2c_data[1] = {0};
static uint8_t hrs3300_mode;

bool Hrs3300_write_reg(uint8_t addr, uint8_t data)
{
	uint32_t i2c_cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0 | WDATA | STOP_FLAG;
	bsp_hw_i2c_tx_byte(SENSOR_HR_IICX, i2c_cfg, HRS3300_WRITE_ADDR(HRS3300_ADDR_7BIT), addr, data);
    return 0;
}

uint8_t Hrs3300_read_reg(uint8_t addr)
{
    u8 rx_byte = 0;
	uint32_t i2c_cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0 | START_FLAG1 | DEV_ADDR1;
	bsp_hw_i2c_rx_buf(SENSOR_HR_IICX, i2c_cfg, HRS3300_READ_ADDR(HRS3300_ADDR_7BIT), addr, &rx_byte, 1);
    return rx_byte;
}

#ifdef MALLOC_MEMORY
void *hr_malloc(size_t size)
{
    return mem_malloc(size);
}

void hr_free(void * ptr)
{
    mem_free(ptr);
}
#endif

uint16_t Hrs3300_read_hrs(void)
{
    uint8_t  databuf[3];
    uint16_t data;

    databuf[0] = Hrs3300_read_reg(0x09);	// addr09, bit
    databuf[1] = Hrs3300_read_reg(0x0a);	// addr0a, bit
    databuf[2] = Hrs3300_read_reg(0x0f);	// addr0f, bit

    data = ((databuf[0]<<8) | ((databuf[1]&0x0F)<<4) | (databuf[2]&0x0F));

    return data;
}

uint16_t Hrs3300_read_als(void)
{
    uint8_t  databuf[3];
    uint16_t data;

    databuf[0] = Hrs3300_read_reg(0x08);	// addr09, bit [10:3]
    databuf[1] = Hrs3300_read_reg(0x0d);	// addr0a, bit [17:11]
    databuf[2] = Hrs3300_read_reg(0x0e);	// addr0f, bit [2:0]

    data = ((databuf[0]<<3) | ((databuf[1]&0x3F)<<11) | (databuf[2]&0x07));

    if (data > 32767) {
        data = 32767;                       // prevent overflow of other function
    }

    return data;
}

bool Hrs3300_chip_init(uint8_t hrss_mode)
{
    int i = 0;
    uint8_t id = 0;
    for (i = 0; i < 5; i++) {
        id = Hrs3300_read_reg(0x00);
        if (0x21 == id) {
            break;
        } else {
            nrf_delay_ms(10);
        }
    }
    printf("HRS3300 ID:0x%x\n",id);
    if (id != 0x21) {
        return false;
    }

    for (i = 0; i < INIT_ARRAY_SIZE; i++)
    {
        if (Hrs3300_write_reg(init_register_array[i][0], init_register_array[i][1]) != 0 ) {
            return false;
        }
    }

        //20161117 added by ericy for "low power in no_touch state"
        if (hrs3300_power_up_flg == 0) {
            reg_0x7f = Hrs3300_read_reg(0x7f);
            reg_0x80 = Hrs3300_read_reg(0x80);
            reg_0x81 = Hrs3300_read_reg(0x81);
            reg_0x82 = Hrs3300_read_reg(0x82);
            hrs3300_power_up_flg =  1;
        }
        //20161117 added by ericy for "low power in no_touch state"
    #ifdef RTT_PRINT
    DEBUG_PRINTF(">>> hrs3300 init id = %d\r\n", id);
    #endif

    hrs3300_mode = hrss_mode;
    return true;
// RTN:
//     return false;
}

void Hrs3300_chip_enable()
{
    Hrs3300_write_reg( 0x16, 0x78 );
    Hrs3300_write_reg( 0x01, 0xd0 );
    Hrs3300_write_reg( 0x0c, 0x2e );

    return ;
}

void Hrs3300_chip_disable()
{
    Hrs3300_write_reg( 0x01, 0x08 );
    Hrs3300_write_reg( 0x0c, 0x4e );

    return ;
}

uint8_t Oxygen_Process(u8 m_HRF)                  //假血氧
{
    const static uint8_t spo2h_buff0[6] = {88,89,90,91,92,93};
    const static uint8_t spo2h_buff1[4] = {94,95,96,97};
    const static uint8_t spo2h_buff2[3] = {97,98,99};

    uint8_t spo2h, num = get_random(10);

    if (m_HRF < 45) {
        spo2h =  0;
    } else if (m_HRF < 50){
        spo2h = spo2h_buff0[0];
    } else if (m_HRF < 60){
        num = num % 6;
        spo2h = spo2h_buff0[num];
    } else if (m_HRF < 70){
        num = num % 4;
        spo2h = spo2h_buff1[num];
    } else if (m_HRF <= 100){
        num = num % 3;
        spo2h = spo2h_buff2[num];
    } else {
        spo2h = spo2h_buff2[2];
    }
//    printf("spo2 = %d\n", spo2h);
    return spo2h;
}

void heart_rate_meas_timeout_handler(void *p_context)
{
    if (!get_hrs3300_sta()) return;

    // uint32_t        err_code;
    // uint16_t        heart_rate;
    uint16_t hrm_raw_data;
    uint16_t als_raw_data;
    // uint8_t gsen_data;
    hrs3300_results_t alg_results;
#ifdef BP_CUSTDOWN_ALG_LIB
    hrs3300_bp_results_t bp_alg_results ;
#endif
    static uint16_t timer_index = 0;
#ifdef GSENSER_DATA
    AxesRaw_t gsen_buf;
#endif

//    UNUSED_PARAMETER(p_context);
#ifdef GSENSER_DATA
    LIS3DH_GetAccAxesRaw(&gsen_buf);
#endif
    hrm_raw_data = Hrs3300_read_hrs();
    als_raw_data = Hrs3300_read_als();  // 20170430
#ifdef GSENSER_DATA
    Hrs3300_alg_send_data(hrm_raw_data, als_raw_data, gsen_buf.AXIS_X, gsen_buf.AXIS_Y, gsen_buf.AXIS_Z, 0);
#else
    Hrs3300_alg_send_data(hrm_raw_data, als_raw_data, 0, 0, 0, 0);
#endif
    alg_results = Hrs3300_alg_get_results();
    if (hrm_raw_data) {
        bsp_sensor_hrs_data_save(alg_results.hr_result);
    } else {
        bsp_sensor_hrs_data_save(0);
    }
//    printf("HRValue = %d\n", alg_results.hr_result);
//    printf("cnt[%d] raw[%d] mode[%d]\n", timer_index, hrm_raw_data, hrs3300_mode);

    timer_index++;
    if (timer_index >= 25)  {    // get result per second
        timer_index = 0;

        if (hrs3300_mode == HRSS_SPO2_MODE) {
            if (hrm_raw_data) {
                bsp_sensor_spo2_data_save(Oxygen_Process(alg_results.hr_result));
            } else {
                bsp_sensor_spo2_data_save(0);
            }
        }

        if (hrs3300_mode == HRSS_BP_MODE) {
            alg_results = Hrs3300_alg_get_results();
            if (alg_results.alg_status == MSG_NO_TOUCH) {
                //opr_display_hr(0);    // customer can print no touch information here
                bsp_sensor_bp_data_save(0, 0);
            } else if (alg_results.alg_status == MSG_PPG_LEN_TOO_SHORT) {
                //opr_display_wait( ((alg_results.data_cnt/100)%3)+1);  // customer can print waiting information here
            } else {
#ifdef BP_CUSTDOWN_ALG_LIB
                bp_alg_results = Hrs3300_alg_get_bp_results();
                if (bp_alg_results.sbp != 0) {
                   //opr_display_bp(bp_alg_results.sbp, bp_alg_results.dbp);
                }
                bsp_sensor_bp_data_save(bp_alg_results.sbp, bp_alg_results.dbp);
#endif
                //opr_display_hr(alg_results.hr_result);  // customer can print real heart rate here
            }
        }
    }
}

#endif //SENSOR_HR_TYHX_HRS3300
