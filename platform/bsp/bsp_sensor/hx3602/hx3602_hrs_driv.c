#include "include.h"
#include "hx3602.h"
#include "hx3602_hrs_driv.h"
#include "tyhx_hrs_alg.h"

#if (SENSOR_HR_SEL == SENSOR_HR_TYHX_HX3602)

static hrs3602_wear_status_t wear_status = MSG_CHECK_INIT;
static hrs3602_wear_status_t wear_status_pre = MSG_CHECK_INIT;

uint32_t hx3602_wear_thre_high = 5000;
uint32_t hx3602_wear_thre_low = 4000;
uint32_t hx3602_agc_adj_thre_high = 450000;
uint32_t hx3602_agc_adj_thre_low = 150000;

static int32_t check_touch_data_min = 300000;
static int32_t check_touch_data_max = 0;
static int32_t check_touch_data_fifo[8] = {0};

uint8_t agc_cnt = 0;
uint8_t agc_offset = 0;
uint8_t check_touch_cnt = 0;

void Hrs3602_driv_init(void)
{
    wear_status = MSG_CHECK_INIT;
    wear_status_pre = MSG_CHECK_INIT;
    agc_cnt = 0;
    agc_offset = 0;
    check_touch_cnt = 10;
}

void Hrs3602_check_touch(int32_t infrared_data)
{
    if(infrared_data > hx3602_wear_thre_high)
    {
        if(check_touch_cnt>=20)
        {
            check_touch_cnt = 20;
            wear_status = MSG_TOUCH;
            if(wear_status != wear_status_pre)
            {
                Hrs3602_normal_power();
                tyhx_hrs_alg_open_deep();
            }
        }
        else
        {
            check_touch_cnt++;
        }
    }
    else if(infrared_data < hx3602_wear_thre_low)
    {
        if(check_touch_cnt<=0)
        {
            check_touch_cnt = 0;
            wear_status = MSG_NO_TOUCH;
            if(wear_status != wear_status_pre)
            {
                Hrs3602_low_power();
            }
        }
        else
        {
            check_touch_cnt--;
        }
    }
}

void Hrs3602_check_touch_diff(int32_t infrared_data)
{
    uint8_t ii=0,jj=0;
    int32_t ir_data_temp = 0;
    int32_t check_fifo[8];
    int32_t check_data_sum = 0;
    int32_t check_data_ave = 0;

    for(ii=0;ii<7;ii++)
    {
        check_touch_data_fifo[ii] = check_touch_data_fifo[ii+1];
        check_fifo[ii] = check_touch_data_fifo[ii];
        check_data_sum = check_data_sum+check_touch_data_fifo[ii];
    }
    check_touch_data_fifo[7] = infrared_data;
    check_fifo[7] = check_touch_data_fifo[7];
    check_data_sum = check_data_sum+check_touch_data_fifo[7];

    for(ii=0;ii<7;ii++)
    {
        for(jj=0;jj<7-ii;jj++)
        {
            if(check_fifo[jj] > check_fifo[jj+1])
            {
                ir_data_temp = check_fifo[jj];
                check_fifo[jj] = check_fifo[jj+1];
                check_fifo[jj+1] = ir_data_temp;
            }
        }
    }
    if(abs(check_fifo[7]-check_fifo[0])<4000 && check_fifo[0]>0)
    {
        check_data_ave =  check_data_sum>>3;
    }

    if(check_data_ave < check_touch_data_min && check_data_ave > 0)
    {
        check_touch_data_min = check_data_ave;
    }
    if(check_data_ave > check_touch_data_max && check_data_ave > 0)
    {
        check_touch_data_max = check_data_ave;
    }


    if(check_touch_data_max>0 && check_touch_data_min>0 && check_touch_data_max>check_touch_data_min+hx3602_wear_thre_high)
    {
        if(infrared_data > check_touch_data_min+hx3602_wear_thre_high)
        {
            if(check_touch_cnt>=20)
            {
                check_touch_cnt = 20;
                wear_status = MSG_TOUCH;
                if(wear_status != wear_status_pre)
                {
                    Hrs3602_normal_power();
                    tyhx_hrs_alg_open_deep();
                }
            }
            else
            {
                check_touch_cnt++;
            }
        }
        else if(infrared_data < check_touch_data_min+hx3602_wear_thre_low)
        {
            if(check_touch_cnt<=0)
            {
                check_touch_cnt = 0;
                wear_status = MSG_NO_TOUCH;
                if(wear_status != wear_status_pre)
                {
                    Hrs3602_low_power();
                }
            }
            else
            {
                check_touch_cnt--;
            }
        }
    }
    else
    {
        if(infrared_data > hx3602_wear_thre_high)
        {
            if(check_touch_cnt>=20)
            {
                check_touch_cnt = 20;
                wear_status = MSG_TOUCH;
                if(wear_status != wear_status_pre)
                {
                    Hrs3602_normal_power();
                    tyhx_hrs_alg_open_deep();
                }
            }
            else
            {
                check_touch_cnt++;
            }
        }
        else if(infrared_data < hx3602_wear_thre_low)
        {
            if(check_touch_cnt<=0)
            {
                check_touch_cnt = 0;
                wear_status = MSG_NO_TOUCH;
                if(wear_status != wear_status_pre)
                {
                    Hrs3602_low_power();
                }
            }
            else
            {
                check_touch_cnt--;
            }
        }
    }
}

hrs3602_wear_status_t Hrs3602_agc(int32_t als_raw_data , int32_t infrared_data)
{
    #ifdef CHECK_TOUCH_DIFF
        Hrs3602_check_touch_diff(infrared_data);
    #else
        Hrs3602_check_touch(infrared_data);
    #endif

    if(wear_status == MSG_TOUCH)
    {
        if(als_raw_data > hx3602_agc_adj_thre_high)
        {
            if(agc_cnt==3)
            {
                agc_cnt = 0;
                if(agc_offset<32)
                {
                    agc_offset++;
                    Hrs3602_write_reg(0x15,(0x40|agc_offset));
                }
            }
            else
            {
                agc_cnt++;
            }
        }
        else if(als_raw_data < hx3602_agc_adj_thre_low)
        {
            if(agc_cnt==3)
            {
                agc_cnt = 0;
                if(agc_offset>0)
                {
                    agc_offset--;
                    Hrs3602_write_reg(0x15,(0x40|agc_offset));
                }
            }
            else
            {
                agc_cnt++;
            }
        }
    }
    else
    {
        agc_cnt = 0;
        agc_offset = 0;
    }
    wear_status_pre = wear_status;
//    DEBUG_PRINTF("min=%d ir=%d status=%d offset=%d\r\n" ,check_touch_data_min, infrared_data, wear_status, agc_offset);
    return wear_status;
}

void Hrs3602_low_power(void)
{
    printf("Hrs3602_low_power\n");
    Hrs3602_write_reg(0x02, 0x70);
    Hrs3602_write_reg(0x15, 0x40);
//    Hrs3602_write_reg(0xc0, 0x87);  /* led dr config , 0x84 = 12.5mA , 0x85 = 25mA ,0x86 = 50 mA ,0x87 = 100mA */
    Hrs3602_write_reg(0x16, 0x31);	// detect interval is 1s when loss

}

void Hrs3602_normal_power(void)
{
    printf("Hrs3602_normal_power\n");
	Hrs3602_write_reg(0x02, 0x77);
//    Hrs3602_write_reg(0xc0, 0x87);
    Hrs3602_write_reg(0x16, 0x00);   // detect interval is 40mS

}
#endif
