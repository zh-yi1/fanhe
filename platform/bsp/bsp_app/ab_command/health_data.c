#include "include.h"
#include "ab_common.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if (USE_APP_TYPE == APP_BLUE_FIT)

///健康相关类
//0xb0:目标设置
void ble_get_daily_goals(uint8_t *data)
{
//    memcpy(data, (char*)(&app_data.daily_goal), sizeof(daily_goals_t) - 2);
}

void ble_set_daily_goals(uint8_t *buf, uint8_t len)
{
//    TRACE("%s\n", __func__);
//    uint8_t offset = 0, goal_type = 0, goal_len = 2;
//    daily_goals_t *daily_goal = &app_data.daily_goal;
//
//    memcpy(&daily_goal->swi, &buf[offset], 2);
//    offset += 2;
//    while (offset < len) {
//        goal_type = buf[offset++];
//        goal_len = buf[offset++];
////        memcpy((daily_goal + 2 + (goal_type+1)*2, &buf[offset], 2);
//        switch (goal_type) {
//        case 0x00:
//            memcpy(&daily_goal->step, &buf[offset], goal_len);
//            break;
//        case 0x01:
//            memcpy(&daily_goal->kcal, &buf[offset], goal_len);
//            break;
//        case 0x02:
//            memcpy(&daily_goal->distance, &buf[offset], goal_len);
//            break;
//        case 0x03:
//            memcpy(&daily_goal->sleep, &buf[offset], goal_len);
//            break;
//        case 0x04:
//            memcpy(&daily_goal->duration, &buf[offset], goal_len);
//            break;
//        }
//        offset += goal_len;
//    }
//
//    printf("goal: swi[%x] step[%x] kcal[%x] distance[%x] sleep[%x] duration[%x]\n",daily_goal->swi, daily_goal->step,
//           daily_goal->kcal, daily_goal->distance, daily_goal->sleep, daily_goal->duration);
}

//0xb3: 运动数据
//static u32 s_rtccnt;
//sport_type_t sport_get_type(u8 sport_index)
//{
//    sport_type_t sport_type = RUNNING;
//    if (sport_index == 1) {
//        sport_type = JUMPROPE;
//    } else if (sport_index == 2) {
//        sport_type = SITUP;
//    }
//    return sport_type;
//}

void sport_data_set(sport_record_t *sport)
{
//    sport->sport_type = sys_cb.sport_index;
//    sport->timestamp = s_rtccnt;
//    sport->step = sport_col_step_get();
//    sport->cal = sport_col_cal_get() * 100;
//    sport->distance = sport_col_distance_get();     //千米
//    sport->duration = gui_get_tim_sec(TIM_1);
//    sport->avg_hr = app_data.bpm_avg;
//    sport->base_cal = 0;
//    TRACE("SET:type[%x] stamp[%04x] step[%d] cal[%d] dist[%d] dura[%d] avg[%x]\n",
//           sport->sport_type, sport->timestamp, sport->step,
//           sport->cal, sport->distance, sport->duration, sport->avg_hr);
}

u8 sport_data_num_get(void)
{
    u8 num = 0;
//    for (u8 i = 0; i < SPORT_RECORD_NUM_MAX; i++) {
//        if (app_data.sector0.sport[i].timestamp) {
//            //printf("found %d--->%lx\n", i, app_data.sector0.sport[i].timestamp);
//            num++;
//        }
//    }
    return num;
}

void sport_data_get_stamp(void)
{
//    s_rtccnt = RTCCNT;
//    printf("%s:stamp[%x]\n", __func__, s_rtccnt);
}

void sport_data_save(sport_record_t *sport)
{
//    printf("%s:idx[%d]\n", __func__, app_data.sector0.sport_idx, sizeof(sport_record_t));
//
//    memcpy(&app_data.sector0.sport[app_data.sector0.sport_idx], sport, sizeof(sport_record_t));
//    printf("timestamp:%lx\n", app_data.sector0.sport[app_data.sector0.sport_idx].timestamp);
//    cycle_plus(&app_data.sector0.sport_idx, 0, SPORT_RECORD_NUM_MAX - 1);
}

bool ble_upload_sport(u8 cmd_type, sport_record_t *sport, u8 frame_seq)
{
    uint8_t upload_data[32] = {0x00, 0xb3};
//    uint8_t sport_size = sizeof(sport_record_t);
//    upload_data[2] = cmd_type;
//    upload_data[3] = frame_seq;
//    upload_data[4] = sport_size + 1;
//    upload_data[5] = 0;
//    memcpy(&upload_data[6], sport, sport_size);
//    u32 timestamp = sport->timestamp - (app_data.time_zone - 12)*3600;
//    memset(&upload_data[7], 0, 4);
//    memcpy(&upload_data[7], &timestamp, 4);

//    printf("TX:type[%x] stamp[%04x] step[%d] cal[%d] dist[%d] dura[%d] avg[%x]\n",
//       sport->sport_type, sport->timestamp, sport->step,
//       sport->cal, sport->distance, sport->duration, sport->avg_hr);

    return app_protocol_tx(upload_data, upload_data[4] + INSTRUCT_HEADER_LEN);
}

bool sport_data_is_valid(sport_record_t *sport)
{
//    bool sport_valid = false, speed_judge = false;
//    u8 sport_speed = 10;
//
//    if (sport->sport_type == RUNNING || sport->sport_type == WALK1 || sport->sport_type == CLIMBING) {
//        sport_speed = sport->distance*10/sport->duration;
//        speed_judge = true;
//    }
//
//    if (speed_judge) {
//        if (sport->duration > SPORT_RECORD_VALID_TIME && 5 < sport_speed &&  sport_speed < 200) {
//            sport_valid = true;
//        }
//        printf("duration[%d] sport_speed[%d] record?[%d]\n", sport->duration, sport_speed, sport_valid);
//    } else {
//        if (sport->duration > SPORT_RECORD_VALID_TIME) {
//            sport_valid = true;
//        }
//    }

//    return sport_valid;
    return false;
}

void ble_upload_sport_online(u8 cmd_type)
{
//    if (ble_is_connect()) {
//        uint8_t idx = app_data.sector0.sport_idx == 0?SPORT_RECORD_NUM_MAX - 1:app_data.sector0.sport_idx - 1;//索引往回退一格
//        ble_upload_sport(cmd_type, &app_data.sector0.sport[idx], 0);
//    }
}

bool ble_sport_record_upload(u8 cmd_type)//查询
{
//    uint8_t data_num = sport_data_num_get();
//
//    if (!data_num) {
//        memset(&app_data.sector0.sport[0], 0, sizeof(sport_record_t));
//        app_data.sector0.sport[0].sport_type = 0xff;//无运动数据
//        ble_upload_sport(cmd_type, &app_data.sector0.sport[0], 0);
//        return false;
//    }
//
//    for (u8 i = 0; i < SPORT_RECORD_NUM_MAX; i++) {
////        printf("i[%d] %d %d\n", i, app_data.sector0.sport[i].timestamp, app_data.sector0.sport[i].duration);
//        if (app_data.sector0.sport[i].timestamp) {
//            ble_upload_sport(cmd_type, &app_data.sector0.sport[i], (data_num << 4) | i);
////            app_data.sector0.sport[i].timestamp = 0;
//        }
//    }
//    param_pm_write(PM_SYNC_SECTOR0);
    return true;
}

//0xb4:运动数据删除(预留)
void ble_step_data_del(void)
{
//    for (u8 i = 0; i < SPORT_RECORD_NUM_MAX; i++) {
//        app_data.sector0.sport[i].timestamp = 0;
//    }
}

//0xb5:睡眠数据
u8 sleep_change_num_get(void)
{
    u8 num = 0;
//    for (u8 i = 0; i < SLEEP_RECORD_MAX; i++) {
//        if (app_data.sector1.sleep.change[i].timestamp) {
//            num++;
//        }
//    }
    return num;
}

void ble_sleep_data_del(void)
{
//    TRACE("%s\n", __func__);
//    memset(&app_data.sector1.sleep, 0, sizeof(sleep_record_t));
}

bool ble_get_sleep_data(u8 *buf)
{
//    switch (buf[5]) {
//    case 0x00:  //查询睡眠时间记录
//        buf[4] = 0x08;
//        memcpy(&buf[6], &app_data.sector1.sleep.awake_min, 7);
//        printf("awake[%d] light[%d] deep[%d] wkup[%d]\n",
//            app_data.sector1.sleep.awake_min, app_data.sector1.sleep.light_min,
//            app_data.sector1.sleep.deep_min, app_data.sector1.sleep.wkup_time);
//        break;
//
//    case 0x01:{  //查询睡眠变化记录
//        u8 offset = 6;
//        if (!sleep_change_num_get()) {
//            buf[offset] = 0;
//            offset++;
//            memset(&buf[offset], 0, 4);
//            offset += 4;
//        } else {
//            for (u8 i = 0; i < SLEEP_RECORD_MAX; i++) {
//                timestamp_data_t *change = &app_data.sector1.sleep.change[i];
//                if (change->timestamp) {
//                    buf[offset++] = change->value;
//                    memcpy(&buf[offset], &change->timestamp, 4);
//                    offset += 4;
//                    printf("value[%d] rtc[%d]\n", change->value, change->timestamp);
//                }
//            }
//        }
//        buf[4] = offset - INSTRUCT_HEADER_LEN;
//        break;
//    }
//
//    case 0x02:  //删除睡眠记录
//        buf[4] = 2;
////#if  (SENSOR_STEP_SEL == SENSOR_STEP_SC7A20)
////        sleep_data_clr();
////#endif
//        break;
//
//    }
    return app_protocol_tx(buf, 5 + buf[4]);
}

bool ble_upload_sleep_data(void)
{
    return false;
}

//0xb6:自动睡眠监测
void get_sleep_check(uint8_t *data)
{
//    memcpy(data, &app_data.sector1.sleep_check, sizeof(sleep_check_t));
}

void set_sleep_check(uint8_t *data)
{
//    memcpy(&app_data.sector1.sleep_check, data, sizeof(sleep_check_t));
//
//    sleep_check_t *auto_sleep_p = &app_data.sector0.sleep_check;
//    TRACE("SET: all_day[%x] start_hour[%d] start_min[%d] end_hour[%d] end_min[%d]\n",
//           auto_sleep_p->all_day, auto_sleep_p->start_hour, auto_sleep_p->start_minute,
//           auto_sleep_p->end_hour, auto_sleep_p->end_minute);
}

//0xC5:实时测量控制(心率/血氧)
//static u32 bpm_total;
//static u16 bpm_save_cnt;
//static u16 sec_ticks[3] = {0};

void bpm_avg_reset(void)
{
//    bpm_total = 0;
//    bpm_save_cnt = 0;
}

void ticks_reset_realtime_upload(u8 type)
{
//    sec_ticks[type] = sec_tick_get();
//    printf("%s:%d %d\n", __func__, sec_ticks[type], sec_tick_get());
}

void ble_realtime_min_upload(u8 type)
{
//    printf("%d %d\n", sec_ticks[type], app_data.sec_cnt);
//    if (sec_tick_check_expire(sec_ticks[type], 60)) {//60s上传一次
//        sec_ticks[type] = sec_tick_get();
//        ble_interval_measure_upload(type);
//    }
}


void platform_hr_save(u8 bpm)
{
//    if (app_data.bpm != bpm) {
//        TRACE("%s: %d\n", __func__, bpm);
//        app_data.bpm = bpm;
////        msg_enqueue(EVT_HRINFO_UPDATE);
//    }
//
//    if (bpm) {
//        bpm_total += bpm;
//        bpm_save_cnt++;
//        app_data.bpm_avg = bpm_total/bpm_save_cnt;
//
//        app_data.sector1.hr.bpm_last = bpm;
//        interval_measure_t *int_measure_p = &app_data.sector0.int_measure;
////        TRACE("*%s: %d upload_swi[%d] period[%d]\n",
////              __func__, bpm, app_data.com_sw.sw.ble_bpm_upload, int_measure_p->cnt_cycle[HRSS_HRS_MODE]);
//        if (app_data.com_sw.sw.ble_bpm_upload) {//实时测量
//            ble_realtime_min_upload(HRSS_HRS_MODE);
//        }else if (int_measure_p->period[HRSS_HRS_MODE] && app_data.int_measure_swi[HRSS_HRS_MODE] && func_cb.sta != FUNC_HEART_RATE) {//测量监测
//            printf("IM: hr_pe[%d]\n", int_measure_p->period[HRSS_HRS_MODE]);
//            limit_sub(&app_data.hr_sampling, 0);
//            app_data.int_measure_swi[HRSS_HRS_MODE] = false;
//            if (ble_is_connect()) {
//                ble_interval_measure_upload(0);
//            } else {
//                TRACE("interval save: bmp_record[%d] %d\n", int_measure_p->wptr[HRSS_HRS_MODE]);
//                timestamp_data_save(&int_measure_p->interval[HRSS_HRS_MODE][int_measure_p->wptr[HRSS_HRS_MODE]], bpm);
//                cycle_plus(&int_measure_p->wptr[HRSS_HRS_MODE], 0, HR_RECORD_NUM_MAX - 1);
//            }
//        }
//    }
}

void platform_spo2_save(u8 spo2)
{
//    if (app_data.spo2 != spo2) {
//        printf("%s: %d\n", __func__, spo2);
//        app_data.spo2 = spo2;
//
////        msg_enqueue(EVT_HRINFO_UPDATE);
//    }
//
//    if (spo2) {
//        TRACE("*%s: %d %d\n", __func__, spo2, app_data.com_sw.sw.ble_spo2_upload);
//        app_data.sector1.bo.bo_last = spo2;
//        interval_measure_t *int_measure_p = &app_data.sector0.int_measure;
//        if (app_data.com_sw.sw.ble_spo2_upload) {//实时测量
//            ble_realtime_min_upload(HRSS_SPO2_MODE);
//        } else if(int_measure_p->period[HRSS_SPO2_MODE] && app_data.int_measure_swi[HRSS_SPO2_MODE] && func_cb.sta != FUNC_BLOOD_OXYGEN) {//测量监测
//            printf("IM: bo_pe[%d]\n", int_measure_p->period[HRSS_SPO2_MODE]);
//            limit_sub(&app_data.hr_sampling, 0);
//            app_data.int_measure_swi[HRSS_SPO2_MODE] = false;
//            if (ble_is_connect()) {
//                ble_interval_measure_upload(1);
//            } else {
//                TRACE("interval save: bo_record[%d]\n", int_measure_p->wptr[HRSS_SPO2_MODE]);
//                timestamp_data_save(&int_measure_p->interval[HRSS_SPO2_MODE][int_measure_p->wptr[HRSS_SPO2_MODE]], spo2);
//                cycle_plus(&int_measure_p->wptr[HRSS_SPO2_MODE], 0, HR_RECORD_NUM_MAX - 1);
//            }
//        }
//    }
}

void platform_bp_save(u8 sbp, u8 dbp)
{
//    if (app_data.sbp != sbp || app_data.dbp != dbp) {
//        app_data.sbp = sbp;
//        app_data.dbp = dbp;
////        printf("*%s: %d %d\n", __func__, sbp, dbp);
////        msg_enqueue(EVT_DP_UPDATE);
//    }
//
//    if (sbp || dbp) {
//        TRACE("*%s: %d %d\n", __func__, sbp, dbp);
//        app_data.sector1.bp.sbp_last = sbp;
//        app_data.sector1.bp.dbp_last = dbp;
//        interval_measure_t *int_measure_p = &app_data.sector0.int_measure;
//        if (app_data.com_sw.sw.ble_bp_upload) {//实时测量
//            ble_realtime_min_upload(HRSS_BP_MODE);
//        } else if(int_measure_p->period[HRSS_BP_MODE] && app_data.int_measure_swi[HRSS_BP_MODE] && func_cb.sta != FUNC_BLOOD_PRESSURE) {//测量监测
//            printf("IM: bp_pe[%d]\n", int_measure_p->period[HRSS_BP_MODE]);
//            limit_sub(&app_data.hr_sampling, 0);
//            app_data.int_measure_swi[HRSS_BP_MODE] = false;
//            if (ble_is_connect()) {
//                ble_interval_measure_upload(2);
//            } else {
//                TRACE("interval save: bp_record[%d]\n", int_measure_p->wptr[HRSS_BP_MODE]);
//                timestamp_data_save(&int_measure_p->interval[HRSS_BP_MODE][int_measure_p->wptr[HRSS_BP_MODE]], sbp | (dbp << 8));
//                cycle_plus(&int_measure_p->wptr[HRSS_BP_MODE], 0, HR_RECORD_NUM_MAX - 1);
//            }
//        }
//    }
}

//extern u8 bp_test_wait;
//extern u8 bo_test_wait;
u8 hrsensor_upload_swi(u8 type, u8 swi)
{
    u8 ret = CMD_RESULT_OK;

//    //实时上传开关
//    ticks_reset_realtime_upload(type);
//    switch (type) {
//    case HRSS_HRS_MODE:
//        app_data.com_sw.sw.ble_bpm_upload = swi;
//        break;
//    case HRSS_SPO2_MODE:
//        app_data.com_sw.sw.ble_spo2_upload = swi;
////        bo_test_wait = 0;
//        break;
//    case HRSS_BP_MODE:
//        app_data.com_sw.sw.ble_bp_upload = swi;
////        bp_test_wait = 0;
//        break;
//    }

    //传感器开关
//    if (swi & 0x01) {
//        ret = !(gui_hr_sensor_start(type));
//    } else {
//        gui_hr_sensor_stop();
//    }
    return ret;
}

//0xC6: 自动测量监测
void interval_measure_check(void)
{
//    interval_measure_t *int_measure_p = &app_data.sector0.int_measure;
//
//    for (u8 i = 0; i < HRSS_MODE_MAX; i++) {
//        if (int_measure_p->period[i]) {
//            int_measure_p->cnt_cycle[i]++;
//            printf("++[%d]:cn[%d] pe[%d] sp[%d]\n",i, int_measure_p->cnt_cycle[i], int_measure_p->period[i], app_data.hr_sampling);
//            if (int_measure_p->cnt_cycle[i] >= int_measure_p->period[i]*60) {
//                gui_hr_sensor_start(i);
//                app_data.int_measure_swi[i] = true;
//                int_measure_p->cnt_cycle[i] = 0;
//                if (sys_cb.sleep_status) {
//                    sys_cb.hr_wkup = true;
//                    //msg_enqueue(EVT_PER_WAKEUP);
//                }
//            }
//
//            if (app_data.hr_sampling && int_measure_p->cnt_cycle[i] == HR_INTERVAL_MEASURE_TIMEOUT) {
//                limit_sub(&app_data.hr_sampling, 0);
//                app_data.int_measure_swi[i] = false;
//                int_measure_p->cnt_cycle[i] = 0;
////                printf("[%d]:sp[%d]\n",i, app_data.hr_sampling);
//            }
//        }
//    }
//
//    if (!app_data.hr_sampling) {
//        gui_hr_sensor_stop();
//        bo_test_wait = 0;
//        bp_test_wait = 0;
//    }
}

void interval_measure_set(u8 type, u8 period)
{
//    app_data.sector0.int_measure.period[type] = period;
//    TRACE("%s:type[%d] period[%d]\n", __func__, type, period);
}

u8 ble_interval_measure_query(u8 type)
{
//    return app_data.sector0.int_measure.period[type];
    return 0;
}

//0xc7: 一键测量
void measure_check(void)
{
//    static u8 min_cnt = 0;
//    //上传测量数据
//    min_cnt++;
//    if (min_cnt % MEASURE_UPLOAD_T == 0) {
//        ble_measure_upload(CMD_TYPE_NOTIFY, 0x00);
////        ble_measure_upload(CMD_TYPE_NOTIFY, 0x01);
//        min_cnt = 0;
//    }
}

u32 ble_app_get_steps(void)
{
    return app_data.sector1.step;
}

u32 ble_app_get_calorie(void)
{
    return app_data.sector1.cal;
}

u32 app_get_distance(void)
{
    return app_data.sector1.distance;
}

u8 ble_app_get_hr(void)
{
    return app_data.sector1.hr.bpm_last;
}

u8 ble_app_get_bo(void)
{
    return app_data.sector1.bo.bo_last;
}

u8 app_get_sbp(void)
{
//    return app_data.sbp;
    return 0x64;
}

u8 app_get_dbp(void)
{
//    return app_data.dbp;
    return 0x50;
}

u16 ble_app_get_bp(void)
{
    return 0x6450;  //收缩压100 舒张压80
}

void gui_jump_measure_menu(u8 type, u8 swi)
{
//    printf("%s: %d %d\n", __func__, type, swi);
//    if (swi == 0) {     //结束测量
//        hrsensor_upload_swi(type, swi);
//        if ((type == 0 && func_cb.sta == FUNC_HEART_RATE) || (type == 1 && func_cb.sta == FUNC_BLOOD_OXYGEN) || (type == 2 && func_cb.sta == FUNC_BLOOD_PRESSURE)) {
//            gui_break_recover();
//        }
//    } else {            //开始测量
//        if (is_fot_start() ||  //fot升级时,只测量,不跳到对应的测量界面
//            ((bt_is_connected() && ((f_bt.disp_status == BT_STA_INCOMING) || (f_bt.disp_status == BT_STA_OUTGOING) || (f_bt.disp_status == BT_STA_INCALL)))) ||  //通话返回
//            (bsp_charge_is_force_show())) {        //充电返回
//            return;
//        }
//
//        if (app_data.screensaver_status == true) {
//            sys_screensaver_break();
//            gcover_break_recover();
//        }
//        if (!sys_cb.watch_per_is_sleep && !sys_cb.lcd_bl_sta) {    //未休眠熄屏
//            msg_enqueue(EVT_T3_PWM_ON);		//开背光
//        } else if (sys_cb.watch_per_is_sleep) {
//            msg_enqueue(EVT_PER_WAKEUP);
//            msg_enqueue(EVT_T3_PWM_ON);		//开背光
//        }
//
//        if (type == 0) {           //心率
//            gui_break_jump(func_cb.sta, FUNC_HEART_RATE);
//        } else if (type == 1) {    //血氧
//            gui_break_jump(func_cb.sta, FUNC_BLOOD_OXYGEN);
//        } else if (type == 2) {    //收缩压/舒张压
//            gui_break_jump(func_cb.sta, FUNC_BLOOD_PRESSURE);
//        }
//    }
}

u8 get_hr_swi_sta(void)
{
#if SENSOR_HR_SELECT
    return sys_cb.hr_init;
#else
    return 0;
#endif
}

#endif
