#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>


#ifdef TYHX_DEMO
#include "SEGGER_RTT.h"
#include "nrf_delay.h"
#endif

//////////////////////////////

#include "nrf_delay.h"
#include "hrs3602.h"
#include "hx3602_factory_test.h"

void hx3602_factory_ft_card_test_config()//灰卡10mm高度测试配置
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
    
    uint8_t led_dr_cfg = 2;  /* 0 = 12.5mA ; 1 = 25mA ; 2 =50mA(default) 3 = 100mA*/
    uint8_t tia_res = 2 ;     /* 0 = 54k(default) ; 1 = 108k ; 2 =216k 3 = 432k */    
    
    en2rst_delay_clk_num = 64 ;   /* 0 ~ 127 clk num config */
    rst_clk_num = 6 ;             /* range from 0 to 7 ; 0=1 , 1=3, 2=7,....,7=255 */
    hrs_ckafe_clk_num =64 ;     /* hrm phase led on time : 0~ 255 */
    ps_ckafe_clk_num = 32 ;     /* ps phase led on time : 0~ 255 */
        
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
    
    /*register configration begin ....*/
    
    Hrs3602_write_reg(0x01, (temperature_enable<<2)|temperature_adc_osr);
    Hrs3602_write_reg(0x02, (hrs_enable<<2)|(hrs_adc_osr)|(ps_adc_osr<<4)|(ps_enable<<6));  
        
	Hrs3602_write_reg(0xc1, 0x0f);   // config external pd as input  and tia mode enable  
	Hrs3602_write_reg(0x03, 0x8f);   // all phase use same external pd 

    Hrs3602_write_reg(0x04, hrs_ckafe_clk_num);
    Hrs3602_write_reg(0x05, ps_ckafe_clk_num);
    
    Hrs3602_write_reg(0x0a, prf_wait_clk_num);    
    Hrs3602_write_reg(0x0b, prf_wait_clk_num>>8);    
    Hrs3602_write_reg(0x0c, prf_wait_clk_num>>16);
    
    Hrs3602_write_reg(0x11, rst_clk_num);
    Hrs3602_write_reg(0x12, en2rst_delay_clk_num);
    
    //GPIO config 
    Hrs3602_write_reg(0xc0, (0x84 | led_dr_cfg)); /* led dr config , 0x84 = 12.5mA , 0x85 = 25mA ,0x86 = 50 mA ,0x87 = 100mA */
    Hrs3602_write_reg(0xc2, (0x00 | tia_res));    /* tia res config , 0x00 = 54K , 0x01 = 108K ,0x02 = 216K ,0x03 = 432K */
    /*register configration end ....*/
    Hrs3602_write_reg(0xc2, 0X00); 
    Hrs3602_write_reg(0x09, 0x02);  
    Hrs3602_write_reg(0x16, 0x00);  		
  

    Hrs3602_write_reg(0x14,0x80);//ps
    Hrs3602_write_reg(0x15,0x40);//hrs
    
    #ifdef TIMER_READ_MODE
        Hrs3602_write_reg( 0x07, 0x00 );  
        Hrs3602_write_reg( 0x08, 0x00 );  // self clear int 
          
#else  //int mode
     Hrs3602_write_reg( 0x07, 0x01 ); 
     Hrs3602_write_reg( 0x08, 0x00 );  // self clear int 
#endif 
         
    return ;    
}

void hx3602_factory_ft_leak_light_test_config()//漏光测试配置
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
    uint8_t tia_res = 3 ;     /* 0 = 54k(default) ; 1 = 108k ; 2 =216k 3 = 432k */    
    
    en2rst_delay_clk_num = 64 ;   /* 0 ~ 127 clk num config */
    rst_clk_num = 6 ;             /* range from 0 to 7 ; 0=1 , 1=3, 2=7,....,7=255 */
    hrs_ckafe_clk_num = 128 ;     /* hrm phase led on time : 0~ 255 , 实际宽度10位，低两位默认为0*/
    ps_ckafe_clk_num = 32 ;       /* ps phase led on time : 0~ 255 , 实际宽度10位，低两位默认为0*/
    
    
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

    Hrs3602_write_reg(0xc1, 0x0c);   // config external pd as input  and tia mode enable  
    Hrs3602_write_reg(0x03, 0x8f);   // all phase use same external pd 

    Hrs3602_write_reg(0x01, (temperature_enable<<2)|temperature_adc_osr);
    Hrs3602_write_reg(0x02, (hrs_enable<<2)|(hrs_adc_osr)|(ps_adc_osr<<4)|(ps_enable<<6));

    Hrs3602_write_reg(0x04, hrs_ckafe_clk_num);
    Hrs3602_write_reg(0x05, ps_ckafe_clk_num);

    Hrs3602_write_reg(0x0a, prf_wait_clk_num);    
    Hrs3602_write_reg(0x0b, prf_wait_clk_num>>8);    
    Hrs3602_write_reg(0x0c, prf_wait_clk_num>>16);
    
    Hrs3602_write_reg(0x11, rst_clk_num);
    Hrs3602_write_reg(0x12, en2rst_delay_clk_num);
    
    //GPIO config 
    Hrs3602_write_reg(0xc0, (0x84 | led_dr_cfg)); /* led dr config , 0x84 = 12.5mA , 0x85 = 25mA ,0x86 = 50 mA ,0x87 = 100mA */
    Hrs3602_write_reg(0xc2, (0x00 | tia_res));    /* tia res config , 0x00 = 54K , 0x01 = 108K ,0x02 = 216K ,0x03 = 432K */
    Hrs3602_write_reg(0x09, 0x02);  
    
    Hrs3602_write_reg(0x14, 0x80);
    Hrs3602_write_reg(0x15, 0x40);
#ifdef TIMER_READ_MODE
        Hrs3602_write_reg( 0x07, 0x00 );  
        Hrs3602_write_reg( 0x08, 0x00 );  // self clear int 
          
#else  //int mode
     Hrs3602_write_reg( 0x07, 0x01 ); 
     Hrs3602_write_reg( 0x08, 0x00 );  // self clear int 
#endif  
			 
	return ;
}



bool hx3602_factory_test_read(int32_t *phase_data)
{
    uint8_t  databuf1[6] = {0};
    uint8_t  databuf2[6] = {0};
    uint32_t P1 = 0, P2 = 0, P4 = 0, P3 = 0;
    
    Hrs3602_brust_read_reg(0xa0, databuf1, 6);
	Hrs3602_brust_read_reg(0xa6, databuf2, 6);

    P1 = ((databuf1[0]) | (databuf1[1] << 8) | (databuf1[2] << 16));
    P2 = ((databuf1[3]) | (databuf1[4] << 8) | (databuf1[5] << 16));
    P3 = ((databuf2[0]) | (databuf2[1] << 8) | (databuf2[2] << 16));
    P4 = ((databuf2[3]) | (databuf2[4] << 8) | (databuf2[5] << 16));

    phase_data[0] = P1 ;
    phase_data[1] = P2 ;	
    phase_data[2] = P3 ;
    phase_data[3] = P4 ;	
    DEBUG_PRINTF("%d %d %d %d\r\n",phase_data[0],phase_data[1],phase_data[2],phase_data[3]);

    return true; 	
}


bool hx3602_chip_check_id(void)
{
    uint8_t i = 0;   
	uint8_t chip_id =0;
    
    Hrs3602_write_reg(0x09, 0x02);
    nrf_delay_ms(5);
    for(i=0;i<10;i++)
    {
        chip_id = Hrs3602_read_reg(0x00);  
        if (chip_id == 0x22)
        {
            return true;
        }        
    }
    return false;
}

int32_t hx3602_sort_cal(int32_t *data)
{
    uint8_t ii,jj;
    int32_t data_temp = 0;
    int32_t data_final = 0;
    for(ii=0;ii<3;ii++)
	{
		for(jj=0;jj<3-ii;jj++)
		{
			if(data[jj]>=data[jj+1])
			{
				data_temp = data[jj];
				data[jj] = data[jj+1];
				data[jj+1] = data_temp;
			}
		}
	}
    data_final = (data[1]+data[2])>>1;
    return data_final;
}

FT_RESULTS_t hx3602_factroy_test(TEST_MODE_t test_mode)
{
    FT_RESULTS_t ft_result = {0,0};
	int32_t gr_buffer[4] = {0,0,0,0};
	int32_t ir_buffer[4] = {0,0,0,0};
	int32_t phase_data[4] = {0,0,0,0};
	uint8_t ii = 0;

	if (hx3602_chip_check_id() == false)
	{
		return ft_result;
	}
	switch(test_mode)
	{
        case LEAK_LIGHT_TEST:
            hx3602_factory_ft_leak_light_test_config();
            break; 
        
        case GRAY_CARD_TEST:
            hx3602_factory_ft_card_test_config();
            break;
        
        default:
            break;
	}
	nrf_delay_ms(100);
	
	for(ii=0;ii<4;ii++)   //40ms read data
	{
        hx3602_factory_test_read(phase_data);		
        gr_buffer[ii] = phase_data[0] - phase_data[1];	
        ir_buffer[ii] = phase_data[2] - phase_data[3];
        nrf_delay_ms(40);
	}
	ft_result.gr_data_final = hx3602_sort_cal(gr_buffer);
    ft_result.ir_data_final = hx3602_sort_cal(ir_buffer);
	DEBUG_PRINTF("%d %d %x %x\r\n",ft_result.gr_data_final,ft_result.ir_data_final,Hrs3602_read_reg(0x14),Hrs3602_read_reg(0x15));
    
	return ft_result;
}
