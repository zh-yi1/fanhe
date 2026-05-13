#include <include.h>

#if (SENSOR_HR_SEL == SENSOR_HR_TYHX_HX3605)

#include "hx3605.h"
#include "hx3605_spo2_agc.h"

#ifdef TYHX_DEMO
#include "twi_master.h"
#include "SEGGER_RTT.h"
#endif

#ifdef SPO2_ALG_LIB

extern const uint8_t  hx3605_spo2_agc_red_idac;  // 6,7,8...
extern const uint8_t  hx3605_spo2_agc_ir_idac;  // 6,7,8...
extern const uint8_t  hx3605_spo2_agc_green_idac;  // 6,7,8...
extern const uint8_t  green_led_max_init;
extern const uint8_t  red_led_max_init;
extern const uint8_t  ir_led_max_init;
extern uint8_t low_vbat_flag;

//SPO2_INFRARED_THRES
extern const int32_t  spo2_ir_unwear_thres;
extern const int32_t  spo2_ir_wear_thres;
extern uint8_t read_fifo_first_flg;

static uint8_t s_ppg_state = 0;
static uint8_t s_cal_state = 0;
//static int32_t s_buf[64] = {0};
static int32_t agc_buf[64] = {0};

static uint8_t cal_delay = CAL_DELAY_COUNT;
static SPO2_CAL_SET_T  calReg;

static hx3605_spo2_wear_msg_code_t spo2_wear_status = MSG_SPO2_NO_WEAR;
static hx3605_spo2_wear_msg_code_t spo2_wear_status_pre = MSG_SPO2_NO_WEAR;

void Init_Spo2_PPG_Calibration_Routine(SPO2_CAL_SET_T *calR,uint8_t led)
{
    calR->flag = CAL_FLG_LED_DR|CAL_FLG_LED_DAC|CAL_FLG_AMB_DAC|CAL_FLG_RF;
    calR->G_LEDDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->R_LEDDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->IR_LEDDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->AMBDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->R_RF = 5;       /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    calR->IR_RF = 5;       /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    calR->R_LED = SPO2_CAL_INIT_LED_RED;
    calR->IR_LED = SPO2_CAL_INIT_LED_IR;
    calR->state = sCalStart;
    calR->int_cnt = 0;
    calR->cur255_cnt =0;
    if(low_vbat_flag == 1)
    {
        calR->red_idac = (hx3605_spo2_agc_red_idac*3)>>2;
        calR->ir_idac = (hx3605_spo2_agc_ir_idac*3)>>2;
        calR->red_max_cur = (red_led_max_init*3)>>2;
        calR->ir_max_cur = (ir_led_max_init*3)>>2;
    }
    else
    {
        calR->red_idac = hx3605_spo2_agc_red_idac;
        calR->ir_idac = hx3605_spo2_agc_ir_idac;
        calR->red_max_cur = red_led_max_init;
        calR->ir_max_cur = ir_led_max_init;
    }
}

void Restart_Spo2_PPG_Calibration_Routine(SPO2_CAL_SET_T *calR)
{
    calR->flag = CAL_FLG_LED_DAC;
    calR->state = sCalLed;
    calR->int_cnt = 0;
}

bool hx3605_spo2_check_unwear(WORK_MODE_T mode,int32_t infrared_data)
{
    //if(infrared_data > spo2_ir_wear_thres)
		if(calReg.IR_LEDDAC < 10)
    {
        spo2_wear_status = MSG_SPO2_NO_WEAR;
        if(spo2_wear_status_pre != MSG_SPO2_NO_WEAR)
        {
            spo2_wear_status_pre = MSG_SPO2_NO_WEAR;
            hx3605_spo2_low_power();
						return true;
        }
    }
    return false;
}

bool hx3605_spo2_check_wear(WORK_MODE_T mode,int32_t infrared_data)
{
    if(infrared_data > spo2_ir_wear_thres)
    {
        spo2_wear_status = MSG_SPO2_WEAR;
        if(spo2_wear_status_pre != MSG_SPO2_WEAR)
        {
						//AGC_LOG("hx3605_spo2_check_wear ir = %d\r\n", infrared_data);
            spo2_wear_status_pre = MSG_SPO2_WEAR;
            hx3605_spo2_alg_open_deep();
						return true;
        }
    }
    return false;
}


void PPG_Spo2_Calibration_Routine(SPO2_CAL_SET_T *calR, int32_t r_led, int32_t amb, int32_t ir_led)
{
    int32_t r_led_temp = 0, ir_led_temp = 0;

    switch(calR->state)
    {
				case sCalStart:
					  calR->state = sCalStart1;
						break;

        case sCalStart1:
            if(spo2_wear_status == MSG_SPO2_NO_WEAR)
            {
                calR->state = sCalFinish;
                break;
            }
            if(amb>270000)
						{
							calR->AMBDAC = (amb-270000)/2800;
						}
						else
						{
							calR->AMBDAC = 0;
						}
            if(r_led>amb+128)
            {
                calR->R_led_step = (r_led-amb)/SPO2_CAL_INIT_LED_RED;
								r_led_temp = 4000*(hx3605_spo2_agc_red_idac+calR->AMBDAC)/calR->R_led_step;
								if(r_led_temp>calR->red_max_cur)
								{
									r_led_temp = calR->red_max_cur;
								}
								calR->R_LED = r_led_temp;
								if(calR->R_LED != calR->red_max_cur)
								{
									calR->R_LEDDAC = (hx3605_spo2_agc_red_idac*3)>>2;
								}
								else
								{
									calR->R_LEDDAC = hx3605_spo2_agc_red_idac>>1;
								}
            }
            if(ir_led>amb+128)
            {
                calR->IR_led_step = (ir_led-amb)/SPO2_CAL_INIT_LED_IR;
								ir_led_temp = 4000*(hx3605_spo2_agc_ir_idac+calR->AMBDAC)/calR->IR_led_step;
								if(ir_led_temp>calR->ir_max_cur)
								{
									ir_led_temp = calR->ir_max_cur;
								}
								calR->IR_LED = ir_led_temp;
								if(calR->IR_LED != calR->ir_max_cur)
								{
									calR->IR_LEDDAC = (hx3605_spo2_agc_ir_idac*3)>>2;
								}
								else
								{
									calR->IR_LEDDAC = hx3605_spo2_agc_ir_idac>>1;
								}
            }
						calR->R_RF = 64;
						calR->IR_RF = 64;
						calR->flag = CAL_FLG_LED_DR|CAL_FLG_LED_DAC|CAL_FLG_AMB_DAC|CAL_FLG_RF;
            calR->state = sCalLed;
            break;

        case sCalLed:
            if(r_led>450000 && calR->R_LEDDAC < 200)
            {
               calR->R_LEDDAC = calR->R_LEDDAC+6;
            }

            if(ir_led>450000 && calR->IR_LEDDAC < 200)
            {
               calR->IR_LEDDAC = calR->IR_LEDDAC+6;
            }

            if(ir_led<=450000 && r_led<=450000)
            {
              calR->state = sCalLed2;
            }
						else if(calR->IR_LEDDAC >= 200 || calR->R_LEDDAC >= 200)
						{
							calR->state = sCalFinish;
						}
            else
            {
              calR->state = sCalLed;
            }
            calR->flag = CAL_FLG_LED_DAC;
            break;

        case sCalLed2:
            if(r_led>320000 && calR->R_LEDDAC < 200)
            {
               calR->R_LEDDAC = calR->R_LEDDAC+2;
            }
						else if(r_led<200000 && calR->R_LEDDAC > 1)
            {
               calR->R_LEDDAC = calR->R_LEDDAC-2;
            }
						else if(r_led<200000 && calR->R_LEDDAC > 0)
            {
               calR->R_LEDDAC = calR->R_LEDDAC-1;
            }

            if(ir_led>320000 && calR->IR_LEDDAC < 200)
            {
               calR->IR_LEDDAC = calR->IR_LEDDAC+2;
            }
						else if(ir_led<200000 && calR->IR_LEDDAC > 1)
            {
               calR->IR_LEDDAC = calR->IR_LEDDAC-2;
            }
						else if(ir_led<200000 && calR->IR_LEDDAC > 0)
            {
               calR->IR_LEDDAC = calR->IR_LEDDAC-1;
            }

            if(ir_led<320000 && ir_led>200000 && r_led<320000 && r_led>200000)
            {
              calR->state = sCalFinish;
            }
						else if(calR->IR_LEDDAC >= 200 || calR->R_LEDDAC >= 200)
            {
              calR->state = sCalFinish;
            }
            else
            {
              calR->state = sCalLed2;
            }
            calR->flag = CAL_FLG_LED_DAC;
            break;

        case sCalRfEnd:
            calR->state = sCalFinish;
            break;

        default:
            break;

    }
    AGC_LOG("AGC: R_cur=%d,Ir_cur=%d,ambDac=%d,R_Dac=%d,Ir_Dac=%d,R_step=%d,Ir_step=%d,state=%d\r\n",\
            calR->R_LED,calR->IR_LED,calR->AMBDAC,calR->R_LEDDAC,calR->IR_LEDDAC,calR->R_led_step,calR->IR_led_step,calR->state);
}

SPO2_CAL_SET_T PPG_spo2_agc(void)
{
    int32_t r_led_val, amb_val;
    int32_t ir_led_val;// green_led_val;

    calReg.work = false;
    if (!s_cal_state)
    {
        return calReg;
    }

		#ifdef INT_MODE
    calReg.int_cnt ++;
    if(calReg.int_cnt < 8)
    {
         return calReg;
    }
    calReg.int_cnt = 0;
		hx3605_gpioint_cfg(false);
		#endif

    calReg.work = true;
    read_data_packet(agc_buf);
    amb_val = agc_buf[1];
    r_led_val = agc_buf[0];
    ir_led_val = agc_buf[2];

    AGC_LOG("cal dat ch1=%d,ch2=%d,ch3=%d,ch4=%d\r\n", agc_buf[0], agc_buf[1], agc_buf[2], agc_buf[3]);

    PPG_Spo2_Calibration_Routine(&calReg, r_led_val, amb_val, ir_led_val);

    if (calReg.state == sCalFinish)
    {
        hx3605_spo2_set_mode(CAL_OFF);
        if(spo2_wear_status == MSG_SPO2_NO_WEAR)
        {
            hx3605_spo2_low_power();
        }
				#if defined(TIMMER_MODE)
				#else
				hx3605_gpioint_cfg(true);
				#endif
    }
    else
    {
        hx3605_spo2_updata_reg();
				#if defined(INT_MODE)
				hx3605_gpioint_cfg(true);
				#endif
    }
    return  calReg;
}


void hx3605_spo2_cal_init(void) // 20200615 ericy afe cali offline
{
    uint16_t sample_rate = 200;                      /*config the data rate of chip alps2_fm ,uint is Hz*/
    uint32_t prf_clk_num = 32000/sample_rate;        /*period in clk num, num = Fclk/fs */
    uint8_t ps1_cp_avg_num_sel= 0;
    uint8_t ps0_cp_avg_num_sel= 0;
    uint8_t thres_int =0;    //thres int enable
    uint8_t data_rdy_int =8;    //[3]:ps1_data2 [2]:ps1_data1 [1]:ps0_data2 [0]:ps0_data1

    hx3605_write_reg(0X11, (uint8_t)prf_clk_num);    // prf bit<7:0>6   default 0x00
    hx3605_write_reg(0X12, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>  default 0x03
    hx3605_write_reg(0X16, (ps1_cp_avg_num_sel<<4)|(ps0_cp_avg_num_sel));  //default 0x00

    hx3605_write_reg(0x2d,0x00);     //bits<3:0> fifo data sel, 0000 = p1;0001= p2;0010=p3;0011=p4;bits<7> fifo enble
    hx3605_write_reg(0X26, (thres_int<<4|data_rdy_int));   //default 0x0f

    #if defined(INT_MODE)
    hx3605_write_reg(0X27, 0x00);   // int sel,01=prf ,04=enable almost full
    #endif

    hx3605_write_reg(0X68, 0X01);  //soft reset
    hx3605_delay(5);
    hx3605_write_reg(0X68, 0X00);  //release
    hx3605_delay(5);             //Delay for reset time
}



void hx3605_spo2_cal_off(void) // 20200615 ericy afe cali offline
{
    uint16_t sample_rate = 25;                      /*config the data rate of chip alps2_fm ,uint is Hz*/
    uint32_t prf_clk_num = 32000/sample_rate;        /*period in clk num, num = Fclk/fs */
    uint8_t ps1_cp_avg_num_sel= 0;
    uint8_t ps0_cp_avg_num_sel= 1;
    uint8_t thres_int =0;    //thres int enable
    uint8_t data_rdy_int =0;    //[3]:ps1_data2 [2]:ps1_data1 [1]:ps0_data2 [0]:ps0_data1
    //		uint8_t databuf[3] = {0};

    hx3605_write_reg(0X11, (uint8_t)prf_clk_num);    // prf bit<7:0>6   default 0x00
    hx3605_write_reg(0X12, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>  default 0x03
    hx3605_write_reg(0X16, (ps1_cp_avg_num_sel<<4)|(ps0_cp_avg_num_sel));  //default 0x00

    hx3605_write_reg(0x2d,0x83);     //bits<3:0> fifo data sel, 0000 = p1;0001= p2;0010=p3;0011=p4;bits<7> fifo enble
    hx3605_write_reg(0X26, (thres_int<<4|data_rdy_int));   //default 0x0f

    #if defined(INT_MODE)
    hx3605_write_reg(0x27,0x20);
    #endif

    hx3605_write_reg(0X68, 0X01);  //soft reset
    hx3605_delay(5);
    hx3605_write_reg(0X68, 0X00);  //release
    hx3605_delay(5);
    read_fifo_first_flg = 1;
}

void hx3605_spo2_read_data(int32_t *buf) // 20200615 ericy read reg_data phase1 and phase3
{
    uint8_t  databuf1[6] = {0};
    uint8_t  databuf2[6] = {0};
    uint32_t P1 = 0,P2 = 0 ,P3 = 0 ,P4 =0 ;

    hx3605_brust_read_reg(0x03, databuf1, 6);
    hx3605_brust_read_reg(0x09, databuf2, 6);

    P1 = ((databuf1[0])|(databuf1[1]<<8)|(databuf1[2]<<16));
    P3 = ((databuf1[3])|(databuf1[4]<<8)|(databuf1[5]<<16));
    P4 = ((databuf2[0])|(databuf2[1]<<8)|(databuf2[2]<<16));
    P2 = ((databuf2[3])|(databuf2[4]<<8)|(databuf2[5]<<16));

    buf[0] = P1;
    buf[1] = P2;
    buf[2] = P3;
    buf[3] = P4;
}

void hx3605_spo2_low_power(void)
{
    uint16_t sample_rate = 10;                      /*config the data rate of chip alps2_fm ,uint is Hz*/
    uint32_t prf_clk_num = 32000/sample_rate;        /*period in clk num, num = Fclk/fs */
    uint8_t ps1_cp_avg_num_sel= 0;
    uint8_t ps0_cp_avg_num_sel= 0;
    uint8_t thres_int =0;    //thres int enable
    uint8_t data_rdy_int =0;    //[3]:ps1_data2 [2]:ps1_data1 [1]:ps0_data2 [0]:ps0_data1
    uint8_t led_on_time  = 5;
    uint8_t ldrsel_ps1 =2;      //ps1 LDR SELECT 01:ldr0-IR 02:ldr1-RLED  04:ldr2-GLED
    uint8_t ldrsel_ps0 =0;      //ps0 LDR SELECT 01:ldr0-IR 02:ldr1-RLED  04:ldr2-GLED

    uint8_t dccancel_ps0_data1 =0;    //offset idac   BIT[9:0]  0 31.25nA, 1023 32uA, step 31.25nA
    uint8_t dccancel_ps0_data2 =0;    //offset idac   BIT[9:0]  0 31.25nA, 1023 32uA, step 31.25nA
    uint8_t dccancel_ps1_data1 =0;    //offset idac   BIT[9:0]  0 31.25nA, 1023 32uA, step 31.25nA
    uint8_t dccancel_ps1_data2 =0;    //offset idac   BIT[9:0]  0 31.25nA, 1023 32uA, step 31.25nA
    uint8_t ir_pden_ps0 =0;     //IR_PDEN_PS0
    //    uint8_t ext_pden_ps0 =0;    //EXT_PDEN_PS0
    uint8_t pddrive_ps0 =0;     //PDDRIVE_PS0  0-63
    uint8_t ir_pden_ps1 =0;     //IR_PDEN_PS0
    //    uint8_t ext_pden_ps1 =0;    //EXT_PDEN_PS0
    uint8_t pddrive_ps1 =5;     //PDDRIVE_PS0  0-63

    hx3605_write_reg(0X11, (uint8_t)prf_clk_num);    // prf bit<7:0>6   default 0x00
    hx3605_write_reg(0X12, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>  default 0x03
    hx3605_write_reg(0X16, (ps1_cp_avg_num_sel<<4)|(ps0_cp_avg_num_sel));  //default 0x00
    hx3605_write_reg(0X20, (ir_pden_ps0<<7|ir_pden_ps0<<6|pddrive_ps0));  //default 0x00
    hx3605_write_reg(0X21, (ir_pden_ps1<<7|ir_pden_ps1<<6|pddrive_ps1));  //default 0x00
    hx3605_write_reg(0X18, (uint8_t)dccancel_ps0_data1);     //default 0x00
    hx3605_write_reg(0X19, (uint8_t)(dccancel_ps0_data1>>8));   //default 0x00
    hx3605_write_reg(0X1a, (uint8_t)dccancel_ps0_data2);   //default 0x00
    hx3605_write_reg(0X1b, (uint8_t)(dccancel_ps0_data2>>8));  //default 0x00
    hx3605_write_reg(0X1c, (uint8_t)dccancel_ps1_data1);   //default 0x00
    hx3605_write_reg(0X1d, (uint8_t)(dccancel_ps1_data1>>8));   //default 0x00
    hx3605_write_reg(0X1e, (uint8_t)dccancel_ps1_data2 );   //default 0x00
    hx3605_write_reg(0X1f, (uint8_t)(dccancel_ps1_data2 >>8));   //default 0x00
    hx3605_write_reg(0X22, (ldrsel_ps1<<4|ldrsel_ps0));      //default 0x00
    hx3605_write_reg(0X15, led_on_time);

    #ifdef INT_MODE
    hx3605_write_reg(0X12,0x14);   // fifo almostfull cfg ,max=0x40;
    hx3605_write_reg(0x13,0x31); //FIFO bypass mode enable
    hx3605_write_reg(0x23,0x00); //phase3 convertion ready disable
    #else
    hx3605_write_reg(0x2d,0x83);     //bits<3:0> fifo data sel, 0000 = p1;0001= p2;0010=p3;0011=p4;bits<7> fifo enble
    hx3605_write_reg(0X26, (thres_int<<4|data_rdy_int));   //default 0x0f
    #endif

    hx3605_write_reg(0X68, 0X01);  //soft reset
    hx3605_delay(5);
    hx3605_write_reg(0X68, 0X00);  //release
    hx3605_delay(5);

    calReg.R_LED = 0;
    calReg.IR_LED = 2;
    calReg.R_LEDDAC = 0;
    calReg.IR_LEDDAC = 0;

    AGC_LOG(" chip go to low power mode  \r\n" );

}

void hx3605_spo2_updata_reg(void)
{
    if (calReg.flag & CAL_FLG_LED_DR)
    {
        hx3605_write_reg(0X20, calReg.R_LED);
        hx3605_write_reg(0X21, calReg.IR_LED);
    }

    if (calReg.flag & CAL_FLG_LED_DAC)
    {
        hx3605_write_reg(0X18, (uint8_t)calReg.R_LEDDAC);
        hx3605_write_reg(0X19, (uint8_t)(calReg.R_LEDDAC>>8));
        hx3605_write_reg(0X1c, (uint8_t)calReg.IR_LEDDAC);
        hx3605_write_reg(0X1d, (uint8_t)(calReg.IR_LEDDAC>>8));
    }

    if (calReg.flag & CAL_FLG_AMB_DAC)
    {
        hx3605_write_reg(0X1a, (uint8_t)calReg.AMBDAC);
        hx3605_write_reg(0X1b, (uint8_t)(calReg.AMBDAC>>8));
        hx3605_write_reg(0X1e, (uint8_t)calReg.AMBDAC);
        hx3605_write_reg(0X1f, (uint8_t)(calReg.AMBDAC>>8));
    }

    if (calReg.flag & CAL_FLG_RF)
    {
        hx3605_write_reg(0X15, calReg.R_RF);
    }
}
void hx3605_spo2_set_mode(uint8_t mode_cmd)
{
    switch (mode_cmd)
    {
        case PPG_INIT:
            hx3605_spo2_ppg_init();
            #if defined(TIMMER_MODE)
                hx3605_320ms_timer_cfg(true);
                #if defined(GSEN_40MS_TIMMER)
                hx3605_40ms_timer_cfg(true);
                #endif
            #else
                hx3605_gpioint_cfg(true);
            #endif
            s_ppg_state = 1;
            AGC_LOG("ppg init mode\r\n");
            break;

        case PPG_OFF:
            hx3605_ppg_off();
            s_ppg_state = 0;
            AGC_LOG("ppg off mode\r\n");
            break;

        case PPG_LED_OFF:
            hx3605_spo2_low_power();
            s_ppg_state = 0;
            AGC_LOG("ppg led off mode\r\n");
            break;

        case CAL_INIT:
            Init_Spo2_PPG_Calibration_Routine(&calReg,64);
            hx3605_spo2_cal_init();
            hx3605_spo2_updata_reg();
            #if defined(TIMMER_MODE)
                #if defined(GSEN_40MS_TIMMER)
                #else
                hx3605_40ms_timer_cfg(true);
                #endif
            #endif
            s_cal_state = 1;
            AGC_LOG("cal init mode\r\n");
            break;

        case RECAL_INIT:
            Restart_Spo2_PPG_Calibration_Routine(&calReg);
            hx3605_spo2_cal_init();
            hx3605_spo2_updata_reg();
            #if defined(TIMMER_MODE)
                #if defined(GSEN_40MS_TIMMER)
                #else
                hx3605_40ms_timer_cfg(true);
                #endif
            #endif
            s_cal_state = 1;
            AGC_LOG("recal init mode\r\n");
            break;

        case CAL_OFF:
            #if defined(TIMMER_MODE)
                #if defined(GSEN_40MS_TIMMER)
                #else
                hx3605_40ms_timer_cfg(false);
                #endif
            #endif
            hx3605_spo2_cal_off();
            s_cal_state = 0;
            AGC_LOG("cal off mode\r\n");
            break;

        default:
            break;
    }
}

SENSOR_ERROR_T hx3605_spo2_enable(void)
{
    if (!hx3605_chip_check())
    {
        AGC_LOG("hx3690l check id failed!\r\n");
        return SENSOR_OP_FAILED;
    }

    AGC_LOG("hx3690l check id success!\r\n");

    if (s_ppg_state)
    {
        AGC_LOG("ppg already on!\r\n");
        return SENSOR_OP_FAILED;
    }

    if(!hx3605_spo2_alg_open())
    {
        AGC_LOG("spo2 alg open fail,or dynamic ram not enough!\r\n");
        return SENSOR_OP_FAILED;
    }
    spo2_wear_status = MSG_SPO2_NO_WEAR;
    spo2_wear_status_pre = MSG_SPO2_NO_WEAR;
    hx3605_spo2_set_mode(PPG_INIT);

    AGC_LOG("hx3690l enable!\r\n");

    return SENSOR_OK;
}

void hx3605_spo2_disable(void)
{
    #if defined(TIMMER_MODE)
    hx3605_320ms_timer_cfg(false);
    hx3605_40ms_timer_cfg(false);
    #elif defined(INT_MODE)
    hx3605_gpioint_cfg(false);
    #endif
    hx3605_spo2_set_mode(PPG_OFF);
    s_ppg_state = 0;
    s_cal_state = 0;
    hx3605_spo2_alg_close();

    AGC_LOG("hx3690l disable!\r\n");
}

void hx3605_spo2_data_reset(void)
{
    s_ppg_state = 0;
    s_cal_state = 0;
    hx3605_spo2_alg_close();
}

hx3605_spo2_wear_msg_code_t hx3605_spo2_get_wear_status(void)
{
    return  spo2_wear_status;
}

SPO2_CAL_SET_T get_spo2_agc_status(void)
{
    SPO2_CAL_SET_T cal;

    cal.flag = calReg.flag;
    cal.int_cnt = calReg.int_cnt;
    cal.R_LED = calReg.R_LED;     // phasex led driver config
    cal.IR_LED = calReg.IR_LED;     // phasex led driver config
    cal.R_LEDDAC = calReg.R_LEDDAC;  // phasex led offset idac cfg
    cal.IR_LEDDAC = calReg.IR_LEDDAC;  // phasex led offset idac cfg
	  cal.G_LEDDAC = calReg.G_LEDDAC; // phasex led offset idac cfg
    cal.AMBDAC = calReg.AMBDAC;  // phasex offset idac cfg
    cal.R_RF = calReg.R_RF;      // phasex tia feed back resister cfg
    cal.IR_RF = calReg.IR_RF;      // phasex tia feed back resister cfg
    cal.R_led_step = calReg.R_led_step;
    cal.IR_led_step = calReg.IR_led_step;
    cal.state = calReg.state;
    cal.red_idac = calReg.red_idac;
    cal.ir_idac = calReg.ir_idac;

    return cal;
}

void hx3605_spo2_read_fifo_data(uint8_t read_fifo_size,int32_t *buf)
{
    uint8_t data_flg = 127;
    int32_t data;
    uint8_t databuf[3];
    uint8_t ii=0;
    for(ii=0; ii<read_fifo_size; ii++)
    {
        hx3605_write_reg(0x17, 0x00); // write any walue to 0x17 will update a new data
        hx3605_delay_us(100);

        databuf[2]=hx3605_read_reg(0x17);
        databuf[1]=hx3605_read_reg(0x16);
        databuf[0]=hx3605_read_reg(0x15);
        data_flg = databuf[2]>>5;
        data = (int32_t)(databuf[0]|(databuf[1]<<8)|((databuf[2]&0x1f)<<16));

        if(ii==0)
				{
						if(data_flg ==3)
						{
							ii=3;
							buf[0] = 0;
							buf[1] = 0;
							buf[2] = 0;
						}
						if(data_flg ==2)
						{
							ii=2;
							buf[0] = 0;
							buf[1] = 0;
						}
						if(data_flg ==1)
						{
							ii=1;
							buf[0] = 0;
						}
				}

				if(data_flg == 0)
				{
						buf[ii]= data;
				}
				else if(data_flg == 1)
				{
						buf[ii]= data;
				}
				else if(data_flg == 2)
				{
						buf[ii]= data;
				}
				else if(data_flg == 3)
				{
						buf[ii]= data;
				}
    }
}

uint8_t hx3605_spo2_read(spo2_sensor_data_t * s_dat)
{
    int32_t Red_src_data;
    int32_t Ir_src_data;
    bool recal = false;
    //    uint8_t size = 0;
    //    uint8_t size_byte = 0;
    int32_t *red_buf =  &(s_dat->red_data[0]);
    int32_t *ir_buf =  &(s_dat->ir_data[0]);
    int32_t *s_buf =  &(s_dat->s_buf[0]);
    s_dat->agc_red =  calReg.R_LED;
    s_dat->agc_ir =  calReg.IR_LED;
    int32_t ps_data[4]={0};
    int32_t ps_red_data = 0;
    int32_t ps_ir_data = 0;
    uint8_t data_count = 0;
    if (!s_ppg_state || s_cal_state)
    {
//        return NULL;
        return 0;
    }
		data_count = hx3605_read_fifo_data(s_buf,2,1);
		s_dat->count =  data_count;
		for (uint8_t i=0; i<data_count; i++)
		{
				Red_src_data = s_buf[i*2];
				Ir_src_data = s_buf[i*2+1];

				red_buf[i] = 2*Red_src_data;
				ir_buf[i] = 2*Ir_src_data;
				if(spo2_wear_status==MSG_SPO2_WEAR)
				{
						if(hx3605_spo2_check_unwear(SPO2_MODE,Ir_src_data))
						{
//							return NULL;
							return 0;
						}
				}
				else
				{
						if(hx3605_spo2_check_wear(SPO2_MODE,Ir_src_data))
						{
//							return NULL;
							return 0;
						}
				}
				//SEGGER_RTT_printf(0,"hx3605_hrs_wear_mode_check:::%d %d %d\r\n", hx3605_read_reg(0X2a),hx3605_read_reg(0X2b),hx3605_read_reg(0X2c));
				//SEGGER_RTT_printf(0,"%d/%d %d %d\r\n" ,1+i,data_count,red_buf[i],ir_buf[i]);
		}
		read_data_packet(ps_data);
		ps_red_data = ps_data[0];
		ps_ir_data = ps_data[2];
		if (ps_red_data<150000 || ps_red_data>400000 || ps_ir_data<150000 || ps_ir_data>400000)
		{
				recal = true;

//				if(spo2_wear_status==MSG_HRS_NO_WEAR)
                if(spo2_wear_status==MSG_SPO2_NO_WEAR)
				{
						 recal = false;
				}
		}
		if (recal)
		//if (0)
		{
				cal_delay--;
				if (cal_delay <= 0)
				{
						cal_delay = CAL_DELAY_COUNT;
						hx3605_spo2_set_mode(RECAL_INIT);
				}
		}
		else
		{
				cal_delay = CAL_DELAY_COUNT;
		}
    return 1;
}
#endif

#endif //SENSOR_HR_TYHX_HX3605
