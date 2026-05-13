#include "include.h"

#if  (SENSOR_STEP_SEL == SENSOR_STEP_SC7A20)

sc7a20_info_t sc7a20_info;
static co_timer_t sc7a20_500ms;

#if SC7A20_SLEEP_CHECK_EN
//static co_timer_t sc7a20_sleep;
static u8 sleep_check_exit_cnt; //非睡眠监测周期计数
#endif
/************需要进行IIC接口封包的函数************/
unsigned char SL_SC7A20_I2cAddr_Read(unsigned char iic_addr, unsigned char reg, unsigned char len, unsigned char *buf)
{
	uint32_t i2c_cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0 | START_FLAG1 | DEV_ADDR1;
	bsp_hw_i2c_rx_buf(SENSOR_STEP_IICX, i2c_cfg, SC7A20_READ_ADDR(SL_SC7A20_IIC_7BITS_ADDR), reg, buf, len);
    return 0;
}

unsigned char SL_SC7A20_I2c_Spi_Write(bool sl_spi_iic,unsigned char reg, unsigned char dat)
{
    // printf("%s\n", __func__);

	uint32_t i2c_cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0 | WDATA | STOP_FLAG;
	bsp_hw_i2c_tx_byte(SENSOR_STEP_IICX, i2c_cfg, SC7A20_WRITE_ADDR(SL_SC7A20_IIC_7BITS_ADDR), reg, dat);
    return 0;
}

unsigned char SL_SC7A20_I2c_Spi_Read(bool sl_spi_iic,unsigned char reg, unsigned char len, unsigned char *buf)
{
    // printf("%s\n", __func__);

	uint32_t i2c_cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0 | START_FLAG1 | DEV_ADDR1;
	bsp_hw_i2c_rx_buf(SENSOR_STEP_IICX, i2c_cfg, SC7A20_READ_ADDR(SL_SC7A20_IIC_7BITS_ADDR), reg, buf, len);
	return 0;
}

AT(.com_text.sc7a20)
void sc7a20_isr(void)
{
    if (!bsp_sensor_init_sta_get(SENSOR_INIT_STEP)) {
        return;
    }

    if (!bsensor.step.isr_kick) {
        bsensor.step.isr_kick = true;
    }
}

AT(.text.lowpwr.sleep)
void sc7a20_isr_process(void)
{
    if (!bsp_sensor_init_sta_get(SENSOR_INIT_STEP)) {
        return;
    }

    if (bsensor.step.isr_kick) {
        sc7a20_int_disable();
        bsensor.step.isr_kick = false;
    }
}

AT(.text.lowpwr.sleep)
void sc7a20_lowpwr_process(void)
{
    if (!bsp_sensor_init_sta_get(SENSOR_INIT_STEP)) {
        return;
    }

    bsp_sensor_step_pro_isr();
}

AT(.text.lowpwr.sleep)
void sc7a20_int_enable(void)
{
    if (bsensor.step.int_en || !bsp_sensor_init_sta_get(SENSOR_INIT_STEP)) {
        return;
    }

    printf("%s\n",__func__);
    bsensor.step.int_en = 1;

    bsensor.step.mutex = 1;
    SL_MCU_SLEEP_OPEN_SC7A20_INT();
    bsensor.step.mutex = 0;
}

AT(.text.lowpwr.sleep)
void sc7a20_int_disable(void)
{
    if (!bsensor.step.int_en || !bsp_sensor_init_sta_get(SENSOR_INIT_STEP)) {
        return;
    }

    printf("%s\n",__func__);
    bsensor.step.int_en = 0;

    bsensor.step.mutex = 1;
    SL_MCU_WAKE_CLOSE_SC7A20_INT();
    SL_SC7A20_PEDO_KCAL_WRIST_SLEEP_INT_SWAY_ALGO();
    bsensor.step.mutex = 0;
}

#if SC7A20_SLEEP_CHECK_EN

// const uint8_t rtc_weak_mask[7] = {0x40, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
// uint8_t wday_rtc_to_proc(uint8_t wday)
// {
//     uint8_t ret = 0;
//     if (wday < 7) {
//         ret = rtc_weak_mask[wday];
//     }
//     return ret;
// }

bool clock_intimerange(tm_t *nowtime, uint8_t begin_hour, uint8_t begin_minute, uint8_t end_hour, uint8_t end_minute)
{
    bool res = false;
    uint16_t now = 0, begin = 0, end = 0;
    now   = (nowtime->hour) * 60 + nowtime->min;
    begin = begin_hour * 60 + begin_minute;
    end   = end_hour *60 + end_minute;

    if (begin != end) {
        if (begin > end) {
            res = (now >= begin && now < 1440) || (now < end);
        } else {
            res = (now >= begin && now < end);
        }
    } else if (begin == 0 && end == 0) {	//00:00~00:00
        res = true;
    } else {
        res = false;
    }
    return res;
}

u32 b_timestamp_get(void)
{
    static uint8_t time_zone = 12;
    u32 timestamp = RTCCNT - (time_zone - 12)*3600;
    return timestamp;
}

void b_timestamp_data_save(b_timestamp_data_t *buf, u16 value)
{
    u32 rtccnt = b_timestamp_get();

    buf->timestamp = rtccnt;
    buf->value = value;
}

AT(.text.lowpwr.sleep)
void sleep_data_clr(void)
{
    printf("%s\n", __func__);
    memset(&sc7a20_info.sleep, 0, sizeof(b_sleep_record_t));
    memset(&sc7a20_info.sleep_sta, 0, sizeof(b_sleep_sta_t));
}

AT(.text.lowpwr.sleep)
void sleep_not_wear_clr(void)
{
    u8 sleep_status;
    u32 rtccnt = RTCCNT;
    b_sleep_record_t *sleep_p = &sc7a20_info.sleep;

    printf("%s\n", __func__);
//    print_r(&sc7a20_info.sleep_sta.sleep_last_sta, SLEEP_LAST_MAX);
    for (int i = 0; i < SLEEP_LAST_MAX; i++) {//删除无效记录
        sleep_status = sc7a20_info.sleep_sta.sleep_last_sta[i] & 0xf;
        if (sleep_status == SLEEP_AWAKE) {
            sleep_p->awake_min--;
        } else if (sleep_status == SLEEP_LIGHT) {
            sleep_p->light_min--;
        } else if (sleep_status == SLEEP_DEEP) {
            sleep_p->deep_min--;
        } else if (sleep_status == SLEEP_AWAKE) {
            sleep_p->wkup_time--;
            printf("SLEEP_AWAKE save\n");
        }
    }

    memset(&sc7a20_info.sleep_sta.sleep_last_sta[0], 0, sizeof(sc7a20_info.sleep_sta.sleep_last_sta));
    sc7a20_info.sleep_sta.data_valid = false;

    for (u8 i = 0; i < SLEEP_RECORD_MAX; i++) {
        if (rtccnt - sc7a20_info.sleep.change[i].timestamp < SLEEP_LAST_MAX*60) {//清除最近一段时间内的时间戳
            //printf("[%d][%x]\n", sc7a20_info.sleep.change[i].value, sc7a20_info.sleep.change[i].timestamp);
            sc7a20_info.sleep.change[i].timestamp = 0;
        }
    }
}

AT(.text.lowpwr.sleep)
static u8 sleep_change_save(u8 statue)
{
    u8 data_idx = 0;
    for (u8 i = 0; i < SCHANGE_LAST_MAX; i++) {
        if (sc7a20_info.sleep_sta.change_last[i].timestamp == 0) {
            data_idx = i;
            break;
        }
    }

    b_timestamp_data_save(&sc7a20_info.sleep_sta.change_last[data_idx], statue);
    printf("change_last->timestamp[%d] value[%d]\n", sc7a20_info.sleep_sta.change_last[data_idx].timestamp, sc7a20_info.sleep_sta.change_last[data_idx].value);

    return data_idx;
}

AT(.text.lowpwr.sleep)
static void sleep_last_save(u8 sta)//缓冲记录最近时间内的睡眠
{
    if (sta == SLEEP_NULL) {
        return;
    }

    sc7a20_info.sleep_sta.sleep_last_sta[sc7a20_info.sleep_sta.sleep_last_w] = sta;

    if (sta < SLEEP_CHANGE_MAX && sc7a20_info.sleep_sta.sleep_status_last != sta) {
        u8 idx = sleep_change_save(sta);
        sc7a20_info.sleep_sta.sleep_last_sta[sc7a20_info.sleep_sta.sleep_last_w] |= ((idx + 1) << 4);
        printf("change buffer[%d %d] %d\n", sta, sc7a20_info.sleep_sta.change_last[idx], idx);

        if (sta == SLEEP_AWAKE && sc7a20_info.sleep_sta.sleep_status_last > SLEEP_AWAKE) {
            sc7a20_info.sleep_sta.sleep_last_w++;
            sc7a20_info.sleep_sta.sleep_last_sta[sc7a20_info.sleep_sta.sleep_last_w] = SLEEP_WKUP;
            printf("SLEEP_WKUP buffer\n");
        }

        sc7a20_info.sleep_sta.sleep_status_last = sta;
    }

    sc7a20_info.sleep_sta.sleep_last_w++;
    if (sc7a20_info.sleep_sta.sleep_last_w >= SLEEP_LAST_MAX) {
        sc7a20_info.sleep_sta.sleep_last_w = 0;
        sc7a20_info.sleep_sta.data_valid = true;
    }
}

AT(.text.lowpwr.sleep)
static void sleep_sta_save(u8 sta)
{
    u8 sleep_last_r = sc7a20_info.sleep_sta.sleep_last_w;
    if (sta == SLEEP_NULL) {
        sleep_last_r = sc7a20_info.sleep_sta.sleep_last_w + sleep_check_exit_cnt;
    }

//    printf("cnt[%d] sta[%d] adom[%d] valid[%d]\n", sleep_last_r, sta, SL_Adom_GetStatus(), sc7a20_info.sleep_sta.data_valid);

    sleep_last_save(sta);//保存缓冲睡眠数据

//    if (sc7a20_info.sleep_sta.data_valid) {//确定已穿戴, 记录数据
        u8 sleep_status = sc7a20_info.sleep_sta.sleep_last_sta[sleep_last_r] & 0xf;
        u8 change_idx = sc7a20_info.sleep_sta.sleep_last_sta[sleep_last_r] >> 4;
        b_sleep_record_t *sleep_p = &sc7a20_info.sleep;

        if (sleep_status == SLEEP_AWAKE) {
            sleep_p->awake_min++;
        } else if (sleep_status == SLEEP_LIGHT) {
            sleep_p->light_min++;
        } else if (sleep_status == SLEEP_DEEP) {
            sleep_p->deep_min++;
        } else if (sleep_status == SLEEP_AWAKE) {
            sleep_p->wkup_time++;
            printf("SLEEP_AWAKE save\n");
        }

        if (change_idx) {
            memcpy(&sleep_p->change[sleep_p->index], &sc7a20_info.sleep_sta.change_last[change_idx - 1], sizeof(b_timestamp_data_t));
            printf("change save [%d %d]\n", sleep_p->change[sleep_p->index].value, sleep_p->change[sleep_p->index].timestamp);
            cycle_plus(&sleep_p->index, 0, SLEEP_RECORD_MAX - 1);
        }

        printf("cnt[%d] sta[%d] awake[%d] light[%d] deep[%d] wkup[%d] valid[%d] change[%d] adom[%d] active[%d]\n", sleep_last_r, sleep_status,
        sleep_p->awake_min, sleep_p->light_min, sleep_p->deep_min, sleep_p->wkup_time, sc7a20_info.sleep_sta.data_valid, change_idx, SL_Adom_GetStatus(), SL_Sleep_Active_Degree(1));
//    }
}

AT(.text.lowpwr.sleep)
void sc7a20_sleep_check(tm_t *now_time)
{
    static u8 sleep_check_sta;      //睡眠监测状态
    b_sleep_check_t *auto_sleep_p = &sc7a20_info.sleep_check;

    //auto_sleep_p->all_day = 1;
    if (auto_sleep_p->all_day ||
        clock_intimerange(now_time, auto_sleep_p->start_hour, auto_sleep_p->start_minute, auto_sleep_p->end_hour, auto_sleep_p->end_minute)) {

        sc7a20_info.sleep_status = SL_MCU_SLEEP_ALGO_FUNCTION() + 1;
//        sc7a20_info.sleep_status = get_random(SLEEP_DEEP + 1);

        if (sleep_check_sta == SLEEP_CHECK_NULL) {//首次进入睡眠监测状态
            sleep_data_clr();
            sleep_check_exit_cnt = 0;
            sleep_check_sta = SLEEP_CHECK_ING;
        }

        sleep_sta_save(sc7a20_info.sleep_status);

        if (!SL_Adom_GetStatus()) {
//            sleep_not_wear_clr();
        }

//        sleep_check_exit_cnt++;
//        if (sleep_check_exit_cnt% 80 == 0) {
//            adorn_sta = 0;
//        } else {
//            adorn_sta = 1;
//        }

    } else {//睡眠监测结束后一段时间内, 检查是否穿戴;

        if (sleep_check_sta == SLEEP_CHECK_ING) {
            sleep_check_exit_cnt++;

            if (SL_Adom_GetStatus()) {
                sleep_sta_save(SLEEP_NULL);
            } else {
//                 sleep_not_wear_clr();
            }

            if (sleep_check_exit_cnt == SLEEP_LAST_MAX) {
                sleep_check_sta = SLEEP_CHECK_NULL;
            }
        }
    }

    if (auto_sleep_p->all_day && now_time->hour == 20 && now_time->min == 0) {//全日睡眠监测时, 默认20点初始化睡眠数据
        sleep_check_sta = SLEEP_CHECK_NULL;
    }
}

AT(.text.lowpwr.sleep)
void sc7a20_sleep_1min_callback(co_timer_t *timer, void *param)
{
    if (!bsp_sensor_init_sta_get(SENSOR_INIT_STEP)) {
        return;
    }

    sc7a20_sleep_check(&compo_cb.tm);                            //睡眠检测
}

#endif

AT(.text.lowpwr.sleep)
void sc7a20_process_500ms_callback(co_timer_t *timer, void *param)
{
    if (!bsp_sensor_init_sta_get(SENSOR_INIT_STEP)) {
        return;
    }

    //互斥避免抢占死机
    if (!bsensor.step.int_en && !bsensor.step.mutex) {
        sl_data_update();
        SL_SC7A20_PEDO_KCAL_WRIST_SLEEP_INT_SWAY_ALGO();        //计步
    }
}


#if SENSOR_HUB_EN

AT(.com_text.sens_isr)
static void sc7a20_senshb_exec(void *_senshub, uint8_t channel)
{
    senshub_cfg_t **cfg = (senshub_cfg_t **)_senshub;
    uint32_t pr = bsp_sensorhub_sample_period_set(channel, (*cfg)->sample_rate, 4000);
    uint32_t iiccon0 = IIC0CON0;
    uint32_t reg_addr = 0;
    uint32_t r_dev_addr = (*cfg)->device_id << 1 | 1;
    uint32_t w_dev_addr = (*cfg)->device_id << 1;
    uint32_t iic_cmda = 0;
    uint32_t senshub_con = bsp_sensor_hub_config(SENSHBMAP_G1, true);


    bsp_senshb_set(_senshub, SENSHB_CON, SENSHB_WREG, senshub_con);
    bsp_senshb_set(_senshub, SENSHB_SPR0 + channel, SENSHB_WREG, pr);       //采样间隔

    reg_addr = 0xA8;
    iic_cmda = (u8)w_dev_addr | (u32)((u8)r_dev_addr) << 24 |
                              (u32)((u8)reg_addr) << 8 | (u32)((u8)(reg_addr >> 8)) << 16;
    bsp_senshb_set(_senshub, SENSHB_IICxCMDA, SENSHB_WREG, iic_cmda);     //配置iic 地址
    bsp_senshb_set(_senshub, SENSHB_IICxDMACNT, SENSHB_WREG, (((*cfg)->read_size[0] - 1) << 16) | BIT(0)); //单次读取寄存器数据大小
    bsp_senshb_set(_senshub, SENSHB_IICxDMAADR, SENSHB_WREG, DMA_ADR((*cfg)->sample_buf));  //配置读取buf
    bsp_senshb_set(_senshub, 0, SENSHB_UPAD, 0);
    bsp_senshb_set(_senshub, SENSHB_IICxCON1, SENSHB_WREG, NACK | STOP_FLAG | RDATA | DEV_ADDR1 | START_FLAG1 | REG_ADDR_0 | DEV_ADDR0 | START_FLAG0); //配置iic
    bsp_senshb_set(_senshub, SENSHB_IICxCON0, SENSHB_WREG, iiccon0 | BIT(28)); //开始采样
    bsp_senshb_set(_senshub, SENSHB_IICxCON0, SENSHB_WAIT, 0);


    //done
    bsp_senshb_set(_senshub, SENSHB_IICxCON0, SENSHB_DONE, 0);
}

AT(.com_text.sens_isr)
static void sc7a20_senshb_sample_done(u8 *pbuf, uint16_t len)
{
    uint16_t offset = 0;

    for (u8 i = 0; i < 10; i++) {
        print_r(&pbuf[offset], 6);
        offset += 6;
    }
    printf("offset = %d, len = %d\n", offset, len);
}

static u8 sc7a20_senshb_buf[6 * 10] AT(.sensorhub.buf);

const senshub_cfg_t sc7a20_sens_cfg = {
    .device_id = SL_SC7A20_IIC_7BITS_ADDR,
    .sample_rate = 25,      //25Hz
    .sample_cnt = 10,       //每次采样10组数据
    .read_size = {6},
    .sample_buf = sc7a20_senshb_buf,
    .exec_callback = sc7a20_senshb_exec,
    .done_callback = sc7a20_senshb_sample_done,
};


#endif



void sc7a20_init(void)
{
    printf("%s\n",__func__);
    //sl_var_init();                                  //数据初始化
    gsensor_use_info_set(1, 22, 60, 170, 1);        //用户信息导入
    char sc7a20_id = SL_SC7A20_PEDO_KCAL_WRIST_SLEEP_SWAY_INIT();    //sc7a20库函数初始化

#if SENSOR_HUB_EN
    bsp_sensorhub_reg(&sc7a20_sens_cfg);
#else

    if (sc7a20_id == SL_SC7A20_VERSION_VALUE || sc7a20_id == SL_SC7A20E_VERSION_VALUE) {
        bsp_sensor_init_sta_set(SENSOR_INIT_STEP);
    } else {
        bsp_sensor_init_sta_clr(SENSOR_INIT_STEP);
        return;
    }

    extab_user_isr_set(SENSOR_STEP_INT, FALL_EDGE, IOUD_SEL_PU, sc7a20_isr);
    extab_user_isr_mode_set(SENSOR_STEP_INT, MODE_BOTH_AWAKEN_SLEEP);

    bsp_sensor_step_wrist_set(true);                //抬腕亮屏
//
    co_timer_set(&sc7a20_500ms, 500, TIMER_REPEAT, LEVEL_LOW_PRI, sc7a20_process_500ms_callback, NULL);
    co_timer_set_sleep(&sc7a20_500ms, true);
#endif

//#if SC7A20_SLEEP_CHECK_EN
//    co_timer_set(&sc7a20_sleep, 1000 * 60, TIMER_REPEAT, LEVEL_LOW_PRI, sc7a20_sleep_1min_callback, NULL);
//    co_timer_set_sleep(&sc7a20_sleep, true);
//#endif
}
#endif //SENSOR_STEP_SEL
