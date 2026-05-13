#include <include.h>

#if (SENSOR_HR_SEL == SENSOR_HR_TYHX_HX3602)
//////////////////////////////

#include "hx3602.h"
#include "hx3602_reg_init.h"
#include "hx3602_hrs_driv.h"
#include "tyhx_hrs_alg.h"

#ifdef TYHX_DEMO
#include "SEGGER_RTT.h"
#include "app_timer.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "twi_master.h"
#include "drv_oled.h"
#include "button.h"
#include "demo_ctrl.h"
#endif


#ifdef TESTVEC
#include "testvec.h"
#endif

#ifdef GSENSER_DATA
#include "lis3dh_drv.h"
#endif

#ifdef TYHX_DEMO
extern volatile oled_display_t oled_dis;
#endif

uint8_t alg_ram[5 * 1024] __attribute__((aligned(4)));

// sucess return 0   fail  return 1
bool Hrs3602_write_reg(uint8_t addr, uint8_t data)
{
//    printf("%s reg:%x, data:%x, ra:%x\n", __func__, addr, data, __builtin_return_address(0));
	uint32_t i2c_cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0 | WDATA | STOP_FLAG;
	bsp_hw_i2c_tx_byte(SENSOR_HR_IICX, i2c_cfg, HX3602_WRITE_ADDR(HX3602_ADDR_7BIT), addr, data);

    return 0;
}

uint8_t Hrs3602_read_reg(uint8_t addr)
{
    uint8_t data_buf = 0;

	uint32_t i2c_cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0 | START_FLAG1 | DEV_ADDR1;
	bsp_hw_i2c_rx_buf(SENSOR_HR_IICX, i2c_cfg, HX3602_READ_ADDR(HX3602_ADDR_7BIT), addr, &data_buf, 1);
    return data_buf;
}

bool Hrs3602_brust_read_reg(uint8_t addr , uint8_t *buf, uint8_t length)
{
    uint32_t i2c_cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0 | START_FLAG1 | DEV_ADDR1;
    bsp_hw_i2c_rx_buf(SENSOR_HR_IICX, i2c_cfg, HX3602_READ_ADDR(HX3602_ADDR_7BIT), addr, buf, length);

    return true;
}
//void Hrs3602_read_data_packet()
//{
//    uint8_t  databuf1[6] = {0};
//    uint8_t  databuf2[6] = {0};
//    int32_t P0 = 0, P1 = 0,P2 = 0, P3 = 0;
//    Hrs3602_brust_read_reg(0xa0, databuf1, 6);
//    Hrs3602_brust_read_reg(0xa6, databuf2, 6);
//
//    P0= ((databuf1[0]) | (databuf1[1] << 8) | (databuf1[2] << 16));
//    P1 = ((databuf1[3]) | (databuf1[4] << 8) | (databuf1[5] << 16));
//    P2 = ((databuf2[0]) | (databuf2[1] << 8) | (databuf2[2] << 16));
//    P3  = ((databuf2[3]) | (databuf2[4] << 8) | (databuf2[5] << 16));
//    Hrs3602_brust_read_reg(0xa0, databuf1, 6);
//    Hrs3602_brust_read_reg(0xa6, databuf2, 6);
//    //DEBUG_PRINTF("%d %d %d %d\r\n" ,P0,P1,P2,P3);
//
//}

bool Hrs3602_read_hrs(int32_t *hrm_data, int32_t *als_data)
{
    uint8_t  databuf[6] = {0};
    int32_t P0 = 0, P1 = 0;
    Hrs3602_brust_read_reg(0xa0, databuf, 6);

    P0 = ((databuf[0])|(databuf[1]<<8)|(databuf[2]<<16));
    P1 = ((databuf[3])|(databuf[4]<<8)|(databuf[5]<<16));
    //DEBUG_PRINTF(0," %d %d \r\n" , P0, P1);
    if (P0 > P1)
    {
        *hrm_data = P0 - P1;
    }
    else
    {
        *hrm_data = 0;
    }
    *als_data = P0;

    return true;
}

bool Hrs3602_read_hrs_by_sensorhub(uint8_t *databuf, int32_t *hrm_data, int32_t *als_data)
{
    int32_t P0 = 0, P1 = 0;
//    Hrs3602_brust_read_reg(0xa0, databuf, 6);

    P0 = ((databuf[0])|(databuf[1]<<8)|(databuf[2]<<16));
    P1 = ((databuf[3])|(databuf[4]<<8)|(databuf[5]<<16));
    //DEBUG_PRINTF(0," %d %d \r\n" , P0, P1);
    if (P0 > P1)
    {
        *hrm_data = P0 - P1;
    }
    else
    {
        *hrm_data = 0;
    }
    *als_data = P0;

    return true;
}

bool Hrs3602_read_ps1_by_sensorhub(uint8_t *databuf, int32_t *infrared_data)
{

    int32_t P0 = 0, P1 = 0;
//    Hrs3602_brust_read_reg(0xa6, databuf, 6);
    P0 = ((databuf[0])|(databuf[1]<<8)|(databuf[2]<<16));
    P1 = ((databuf[3])|(databuf[4]<<8)|(databuf[5]<<16));
    if (P0 > P1)
    {
        *infrared_data = P0 - P1;
    }
    else
    {
        *infrared_data = 0;
    }
    return true;
}

void Hrs3602_Int_sensorhub_handle(uint8_t *hrs_data, uint8_t *ps1_data)
{
    int32_t        hrm_raw_data;
    int32_t        als_raw_data;
    int32_t        infrared_data;
    hrs_results_t  alg_results = {MSG_HRS_ALG_NOT_OPEN,0,0,0,0};
	hrs3602_wear_status_t wear_status = MSG_NO_TOUCH;

    #ifdef BP_CUSTDOWN_ALG_LIB
    bp_results_t bp_alg_results;
    #endif

    Hrs3602_read_hrs_by_sensorhub(hrs_data, &hrm_raw_data, &als_raw_data);
    Hrs3602_read_ps1_by_sensorhub(ps1_data, &infrared_data);

    #ifdef GSENSER_DATA
	AxesRaw_t gsen_buf = {0};
    LIS3DH_GetAccAxesRaw(&gsen_buf);
	#else
	GsensorRaw_t gsen_buf = {0};
    #endif

    wear_status = Hrs3602_agc(als_raw_data,infrared_data);
    if(wear_status == MSG_TOUCH)
    {
        tyhx_hrs_alg_send_data(&hrm_raw_data,1,&gsen_buf.AXIS_X, &gsen_buf.AXIS_Y, &gsen_buf.AXIS_Z);
        alg_results = tyhx_hrs_alg_get_results();
        printf("gr=%d, als=%d ir=%d status=%d  HR=%d\r\n" ,hrm_raw_data, als_raw_data, infrared_data, wear_status,alg_results.hr_result);
    }
}

bool Hrs3602_read_ps1(int32_t *infrared_data)
{
    uint8_t  databuf[6] = {0};
    int32_t P0 = 0, P1 = 0;
    Hrs3602_brust_read_reg(0xa6, databuf, 6);
    P0 = ((databuf[0])|(databuf[1]<<8)|(databuf[2]<<16));
    P1 = ((databuf[3])|(databuf[4]<<8)|(databuf[5]<<16));
    if (P0 > P1)
    {
        *infrared_data = P0 - P1;
    }
    else
    {
        *infrared_data = 0;
    }
    return true;
}

void Hrs3602_write_efuse()
{
    Hrs3602_write_reg( 0x85, 0x20 );
    Hrs3602_write_reg( 0x7f, 0x10 );
    Hrs3602_write_reg( 0x80, 0x08 );
    Hrs3602_write_reg( 0x81, 0x44 );
    Hrs3602_write_reg( 0x82, 0x40 );
    Hrs3602_write_reg( 0x85, 0x00 );
}


void Hrs3602_chip_disable(void)
{
    Hrs3602_write_reg( 0x09, 0x03 );
}


bool Hrs3602_chip_init()
{
    int i =0 ;
    uint8_t chip_id =0;

    chip_id = Hrs3602_read_reg(0x00) ;
    if(chip_id != 0x22)
    {
        printf("chip_id err:%x\n", chip_id);
        return false;
    }

    bsp_sensor_init_sta_set(SENSOR_INIT_HR);
    printf("%s ok, id:%x\n", __func__, chip_id);
    for(i = 0; i < INIT_ARRAY_SIZE; i++ )
    {
        if(Hrs3602_write_reg(init_register_array[i][0],init_register_array[i][1]) != 0)
        {
            return false;
        }
    }

    return true;
}

void Hrs3602_alg_config(void)
{
    /*para init begin ....*/
    uint32_t prf_temp_clk_num = 0;    /*temperatrue phase clk num in each prf */
    uint32_t prf_hrs_clk_num = 0;     /*hrs phase clk num in each prf */
    uint32_t prf_ps_clk_num =0;       /*ps phase clk num in each prf */
    uint32_t prf_wait_clk_num =0;     /*wait time clk num in each prf */
    uint32_t en2rst_delay_clk_num =0; /*en signal to first reset delay time clk num in each prf */
    uint16_t rst_clk_num = 0;
    uint16_t hrs_ckafe_clk_num = 0;
    uint16_t ps_ckafe_clk_num = 0;


    /*para init end ....*/

    /*chip config begin ....*/
    uint16_t sample_rate = 25;       /*config the data rate of chip frog2 ,uint is Hz*/
    uint32_t prf_clk_num = 2620000/sample_rate; /*period in clk num, num = Fclk/fs */

    uint8_t temperature_enable = 0;  /*temperature test function enable  , 1 mean enable ; 0 mean disable */
    uint8_t hrs_enable = 1;             /*hrs function enable  , 1 mean enable ; 0 mean disable */
    uint8_t ps_enable = 1;             /*ps function enable  , 1 mean enable ; 0 mean disable */

    uint8_t temperature_adc_osr = 0; /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ;*/
    uint8_t hrs_adc_osr = 3;           /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ;*/
    uint8_t ps_adc_osr = 3;               /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ;*/

    uint8_t led_dr_cfg = 3 ;  /* 0 = 12.5mA ; 1 = 25mA ; 2 =50mA(default) 3 = 100mA*/
    uint8_t tia_res = 2 ;     /* 0 = 54k(default) ; 1 = 108k ; 2 =216k 3 = 432k */

    en2rst_delay_clk_num = 64 ;   /* 0 ~ 127 clk num config */
    rst_clk_num = 6 ;             /* range from 0 to 7 ; 0=1 , 1=3, 2=7,....,7=255 */
    hrs_ckafe_clk_num = 128 ;     /* hrm phase led on time : 0~ 255 , 实际宽度10位，低两位默认为0*/
    ps_ckafe_clk_num = 10 ;       /* ps phase led on time : 0~ 255 , 实际宽度10位，低两位默认为0*/
																	/*红外离pd远的要适当调大， 比如苹果结构距离9mm的， 可以设置到96以上*/

    /*计算PRF_WAIT的时间*/
    if(temperature_enable == 1)
    {
        prf_temp_clk_num = (en2rst_delay_clk_num + 10) + 2*((2<<rst_clk_num) - 1) + 2*(128<<temperature_adc_osr);
    }

    if(hrs_enable == 1)
    {
        prf_hrs_clk_num = (en2rst_delay_clk_num + 10) +
                           2*(2*((2<<rst_clk_num) - 1) + (hrs_ckafe_clk_num<<4) + 10 + (128<<hrs_adc_osr));
    }

    if(ps_enable == 1)
    {
        prf_ps_clk_num = (en2rst_delay_clk_num + 10) +
                           2*(2*((2<<rst_clk_num) - 1) + (ps_ckafe_clk_num<<4) + 10 + (128<<ps_adc_osr));
    }

    prf_wait_clk_num = prf_clk_num - prf_temp_clk_num - prf_hrs_clk_num - prf_ps_clk_num ;
    /*计算PRF_WAIT的时间*/

    Hrs3602_write_reg(0xc1, 0x08);   // config external pd as input  and cap mode enable
    Hrs3602_write_reg(0x03, 0x8f);   // all phase use same external pd

    /*register configration begin ....*/

    Hrs3602_write_reg(0x01, (temperature_enable<<2)|temperature_adc_osr);
    Hrs3602_write_reg(0x02, (hrs_enable<<2)|(hrs_adc_osr)|(ps_adc_osr<<4)|(ps_enable<<6));

    //DEBUG_PRINTF(0," reg_0x02 is %x \r\n" , Hrs3602_read_reg(0x02));


    Hrs3602_write_reg(0x04, hrs_ckafe_clk_num);
    Hrs3602_write_reg(0x05, ps_ckafe_clk_num);

    //DEBUG_PRINTF(0," %x %x \r\n" , Hrs3602_read_reg(0x04) , Hrs3602_read_reg(0x05));

    Hrs3602_write_reg(0x0a, prf_wait_clk_num);
    Hrs3602_write_reg(0x0b, prf_wait_clk_num>>8);
    Hrs3602_write_reg(0x0c, prf_wait_clk_num>>16);

    Hrs3602_write_reg(0x11, rst_clk_num);
    Hrs3602_write_reg(0x12, en2rst_delay_clk_num);

    //GPIO config
    Hrs3602_write_reg(0xc0, (0x84 | led_dr_cfg)); /* led dr config , 0x84 = 12.5mA , 0x85 = 25mA ,0x86 = 50 mA ,0x87 = 100mA */
    Hrs3602_write_reg(0xc2, (0x00 | tia_res));    /* tia res config , 0x00 = 54K , 0x01 = 108K ,0x02 = 216K ,0x03 = 432K */
    /*register configration end ....*/
    Hrs3602_write_reg(0x09, 0x02 );

    #ifdef IR_CHECK_TOUCH
	Hrs3602_write_reg(0X14, 0x80);
    Hrs3602_write_reg(0X15, 0x40);
    #else
    Hrs3602_write_reg(0X14, 0x40);
    Hrs3602_write_reg(0X15, 0x40);
    #endif

#ifdef TIMER_READ_MODE
        Hrs3602_write_reg( 0x07, 0x00 );
        Hrs3602_write_reg( 0x08, 0x00 );  // self clear int

#else  //int mode
     Hrs3602_write_reg( 0x07, 0x01 );
     Hrs3602_write_reg( 0x08, 0x00 );  // self clear int
#endif
    return ;
}

void Hrs3602_wear_config(void)
{
    /*para init begin ....*/
    uint32_t prf_temp_clk_num = 0;    /*temperatrue phase clk num in each prf */
    uint32_t prf_hrs_clk_num = 0;     /*hrs phase clk num in each prf */
    uint32_t prf_ps_clk_num =0;       /*ps phase clk num in each prf */
    uint32_t prf_wait_clk_num =0;     /*wait time clk num in each prf */
    uint32_t en2rst_delay_clk_num =0; /*en signal to first reset delay time clk num in each prf */
    uint16_t rst_clk_num = 0;
    uint16_t hrs_ckafe_clk_num = 0;
    uint16_t ps_ckafe_clk_num = 0;

    uint16_t sample_rate = 25;       /*config the data rate of chip frog2 ,uint is Hz*/
    uint32_t prf_clk_num = 2620000/sample_rate; /*period in clk num, num = Fclk/fs */

    uint8_t temperature_enable = 0;  /*temperature test function enable  , 1 mean enable ; 0 mean disable */
    uint8_t hrs_enable = 0;             /*hrs function enable  , 1 mean enable ; 0 mean disable */
    uint8_t ps_enable = 1;             /*ps function enable  , 1 mean enable ; 0 mean disable */

    uint8_t temperature_adc_osr = 0; /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ;*/
    uint8_t hrs_adc_osr = 3;           /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ;*/
    uint8_t ps_adc_osr = 3;               /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ;*/

    uint8_t led_dr_cfg = 3 ;  /* 0 = 12.5mA ; 1 = 25mA ; 2 =50mA(default) 3 = 100mA*/
    uint8_t tia_res = 2 ;     /* 0 = 54k(default) ; 1 = 108k ; 2 =216k 3 = 432k */

    en2rst_delay_clk_num = 64 ;   /* 0 ~ 127 clk num config */
    rst_clk_num = 6 ;             /* range from 0 to 7 ; 0=1 , 1=3, 2=7,....,7=255 */
    hrs_ckafe_clk_num = 4 ;     /* hrm phase led on time : 0~ 255 , 实际宽度10位，低两位默认为0*/
    ps_ckafe_clk_num = 16 ;       /* ps phase led on time : 0~ 255 , 实际宽度10位，低两位默认为0*/


    /*计算PRF_WAIT的时间*/
    if(temperature_enable == 1)
    {
        prf_temp_clk_num = (en2rst_delay_clk_num + 10) + 2*((2<<rst_clk_num) - 1) + 2*(128<<temperature_adc_osr);
    }

    if(hrs_enable == 1)
    {
        prf_hrs_clk_num = (en2rst_delay_clk_num + 10) +
                           2*(2*((2<<rst_clk_num) - 1) + (hrs_ckafe_clk_num<<4) + 10 + (128<<hrs_adc_osr));
    }

    if(ps_enable == 1)
    {
        prf_ps_clk_num = (en2rst_delay_clk_num + 10) +
                           2*(2*((2<<rst_clk_num) - 1) + (ps_ckafe_clk_num<<4) + 10 + (128<<ps_adc_osr));
    }

    prf_wait_clk_num = prf_clk_num - prf_temp_clk_num - prf_hrs_clk_num - prf_ps_clk_num ;
    /*计算PRF_WAIT的时间*/

    Hrs3602_write_reg(0xc1, 0x08);   // config external pd as input  and cap mode enable
    Hrs3602_write_reg(0x03, 0x8f);   // all phase use same external pd

    /*register configration begin ....*/

    Hrs3602_write_reg(0x01, (temperature_enable<<2)|temperature_adc_osr);
    Hrs3602_write_reg(0x02, (hrs_enable<<2)|(hrs_adc_osr)|(ps_adc_osr<<4)|(ps_enable<<6));

    //DEBUG_PRINTF(0," reg_0x02 is %x \r\n" , Hrs3602_read_reg(0x02));


    Hrs3602_write_reg(0x04, hrs_ckafe_clk_num);
    Hrs3602_write_reg(0x05, ps_ckafe_clk_num);

    //DEBUG_PRINTF(0," %x %x \r\n" , Hrs3602_read_reg(0x04) , Hrs3602_read_reg(0x05));

    Hrs3602_write_reg(0x0a, prf_wait_clk_num);
    Hrs3602_write_reg(0x0b, prf_wait_clk_num>>8);
    Hrs3602_write_reg(0x0c, prf_wait_clk_num>>16);

    Hrs3602_write_reg(0x11, rst_clk_num);
    Hrs3602_write_reg(0x12, en2rst_delay_clk_num);

    //GPIO config
    Hrs3602_write_reg(0xc0, (0x84 | led_dr_cfg)); /* led dr config , 0x84 = 12.5mA , 0x85 = 25mA ,0x86 = 50 mA ,0x87 = 100mA */
    Hrs3602_write_reg(0xc2, (0x00 | tia_res));    /* tia res config , 0x00 = 54K , 0x01 = 108K ,0x02 = 216K ,0x03 = 432K */
    /*register configration end ....*/
    Hrs3602_write_reg(0x09, 0x02);

    #ifdef IR_CHECK_TOUCH
	Hrs3602_write_reg(0X14, 0x80);
    Hrs3602_write_reg(0X15, 0x40);
    #else
    Hrs3602_write_reg(0X14, 0x40);
    Hrs3602_write_reg(0X15, 0x40);
    #endif
    Hrs3602_write_reg(0x16, 0x04);
    Hrs3602_write_reg(0x07, 0x00);
    Hrs3602_write_reg(0x08, 0x00);

    return ;
}

void Hrs3602_int_clear()
{
	Hrs3602_write_reg(0x06, 0x7f);
    return ;
}

void Hrs3602_Int_handle()
{
    int32_t        hrm_raw_data;
    int32_t        als_raw_data;
    int32_t        infrared_data;
    hrs_results_t  alg_results = {MSG_HRS_ALG_NOT_OPEN,0,0,0,0};
	hrs3602_wear_status_t wear_status = MSG_NO_TOUCH;

    #ifdef BP_CUSTDOWN_ALG_LIB
    bp_results_t bp_alg_results;
    #endif

    Hrs3602_read_hrs(&hrm_raw_data, &als_raw_data);
    Hrs3602_read_ps1(&infrared_data);


    #ifdef GSENSER_DATA
	AxesRaw_t gsen_buf = {0};
    LIS3DH_GetAccAxesRaw(&gsen_buf);
	#else
	GsensorRaw_t gsen_buf = {0};
    #endif

    wear_status = Hrs3602_agc(als_raw_data,infrared_data);
    if(wear_status == MSG_TOUCH)
    {
        tyhx_hrs_alg_send_data(&hrm_raw_data,1,&gsen_buf.AXIS_X, &gsen_buf.AXIS_Y, &gsen_buf.AXIS_Z);
        alg_results = tyhx_hrs_alg_get_results();
        printf("gr=%d, als=%d ir=%d status=%d  HR=%d\r\n" ,hrm_raw_data, als_raw_data, infrared_data, wear_status,alg_results.hr_result);

    }


}/*void Hrs3602_Int_handle()*/

//extern uint32_t hx3602_wear_thre_high;
//extern uint32_t hx3602_wear_thre_low;
//void Hrs3602_wear_handle()
//{
//    int32_t infrared_data;
//	hrs3602_wear_status_t wear_status = MSG_CHECK_INIT;
//    Hrs3602_read_ps1(&infrared_data);
//    if(infrared_data > hx3602_wear_thre_high)
//    {
//        wear_status = MSG_TOUCH;
//    }
//    else if(infrared_data < hx3602_wear_thre_low)
//    {
//        wear_status = MSG_NO_TOUCH;
//    }
//}


void heart_rate_meas_timeout_handler(void * p_context)
{
	Hrs3602_Int_handle();
}

#endif
