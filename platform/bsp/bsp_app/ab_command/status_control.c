#include "include.h"
#include "ab_common.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if (USE_APP_TYPE == APP_BLUE_FIT)

////星期日,一,二,三,四,五,六 -> BIT(6) BIT(0) BIT(1) BIT(2) BIT(3) BIT(4) BIT(5)
//const uint8_t rtc_weak_mask[7] = {0x40, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
//
//uint8_t wday_rtc_to_proc(uint8_t wday)
//{
//    uint8_t ret = 0;
//    if (wday < 7) {
//        ret = rtc_weak_mask[wday];
//    }
//    return ret;
//}

///状态控制类
//0x80:开关设置
uint32_t ble_get_universal_switch(void)
{
//    printf("%s:%x\n", __func__, app_data.sector0.sys_sw.u32_sw);
    return app_data.sector0.sys_sw.u32_sw;
}

void ble_set_universal_switch(uint32_t sw)
{
//    printf("sw[%x]\n", sw);
//    app_data.sector0.sys_sw.u32_sw = sw;
//    msg_enqueue(EVT_SWI_UPDATE);
}

//0x81:设备绑定
void app_data_clr(void)
{
//#if  (SENSOR_STEP_SEL == SENSOR_STEP_SC7A20)
//    sl_var_init();

//    if (app_data.sector0.sleep_check.all_day) {
//        sleep_data_clr();
//    }
//#endif
}

void app_data_day_clr(void)
{
//    printf("%s %d\n", __func__, app_data.dayclr_ban);
//    if (app_data.bind_sta < BIND_ING) {
//        if (app_data.dayclr_ban) {//app时间更新禁止清空数据
//            app_data.dayclr_ban = 0;
//        } else {
//            app_data_clr();
//        }
//    }
}

bool bind_flag_get(void)
{
    return app_data.sector0.bind_flag;
}

bool ab_app_is_connect(void)
{
//     TRACE("%s:%d\n", __func__, app_data.bind_sta);
    return (app_data.bind_sta == BIND_DONE);
}

void bind_flag_set(bool flag)
{
    app_data.sector0.bind_flag = flag;
}

void bind_sta_set(u8 sta)
{
    printf("set[%d] bind_sta[%d]\n", sta, app_data.bind_sta);
    if (sta == BIND_ING && app_data.bind_sta > BIND_ING) {
        return;
    }

    if ((sta + app_data.bind_sta) == (BIND_ING_DEV_DONE + BIND_ING_APP_DONE)) {	//设备和app双方绑定数据结束, 判断为绑定结束
        app_data.bind_sta = BIND_DONE;
    } else {
        app_data.bind_sta = sta;
    }
}

//0x83:闹钟协议
u8 alarm_num_get(void)
{
//    u8 num = 0;
//    for (u8 i = 0; i < ALARM_NUM_MAX; i++) {
//        if (app_data.sector0.alarm[i].cycle) {
//            num++;
//        }
//    }
//    return num;

//    TRACE("%s:%d\n", __func__, app_data.sector0.alarm_num);
    return app_data.sector0.alarm_num;
}

uint8_t alarm_free_index_get(void)
{
//    TRACE("%s\n", __func__);
//    for (u8 i = 0; i < ALARM_NUM_MAX; i++) {
//        if (app_data.sector0.alarm[i].cycle == 0) {
//            return i;
//        }
//    }
    return 0xff;
}

void alarm_equal(alarm_clock_t *alarm_t, bool enable, uint8_t cycle, uint8_t alarm_hour, uint8_t alarm_minute, uint8_t motor_mode, uint8_t remind_later)
{
//    alarm_t->swi          = enable;
//    alarm_t->cycle        = cycle;
//    alarm_t->hour         = alarm_hour;
//    alarm_t->minute       = alarm_minute;
//    alarm_t->moto_mode     = motor_mode;
//    alarm_t->remind_later = remind_later;
}

u8 ble_upload_alarm_data(void)
{
//    TRACE("%s\n", __func__);
//    uint8_t frame_seq = 1;
//    uint8_t upload_data_arr[13] = {0x00};
//    uint8_t frame_sum = alarm_num_get() + 1;
//    alarm_clock_t *alarm_p = {0};
//
//    for (u8 i = 0; i < ALARM_NUM_MAX; i++) {
//        if (app_data.sector0.alarm[i].cycle) {
//            alarm_p = &app_data.sector0.alarm[i];
////            upload_data_arr[0]     = ble_seq_num_get();
//            upload_data_arr[1]     = 0x83;
//            upload_data_arr[2]     = 0x02;
//            upload_data_arr[3]     = (frame_sum << 4) | frame_seq;
//            upload_data_arr[4]     = 8;
//            upload_data_arr[5]     = 0x00;
//            upload_data_arr[6]     = i;
//            memcpy(&upload_data_arr[7], alarm_p, sizeof(alarm_clock_t));
//            app_protocol_tx(upload_data_arr, 13);
//
//            frame_seq++;
//        }
//    }
    return 0;
}

bool alarm_check(tm_t *now_time)
{
//    uint8_t week_mask = wday_rtc_to_proc(now_time->day);
//    alarm_clock_t *alarm_p = {0};
//
//    if (!week_mask) {
//        return false;
//    }
//
//    for (uint8_t i = 0; i < ALARM_NUM_MAX + 1; i++) {
//
//        if (i == ALARM_NUM_MAX) {                       //贪睡闹钟
//            alarm_p = &app_data.alarm_later;
//        } else {                                        //用户闹钟
//            alarm_p = &app_data.sector0.alarm[i];
//        }
//
//        if (alarm_p->swi == 1) {//开关
//
//            printf("i[%d] num[%d] hour[%d]:minute[%d] = tm_hour[%d]:tm_min[%d]?\r\n",
//                i, ALARM_NUM_MAX + 1, alarm_p->hour, alarm_p->minute, now_time->hour, now_time->min);
//            printf("cycle[%x] mask[%x]\r\n", alarm_p->cycle, week_mask);
//
//            if ((alarm_p->cycle & week_mask) > 0 || (alarm_p->cycle >> 7)) {//当天或者只提醒一次
//                if ((alarm_p->hour == now_time->hour) && (alarm_p->minute == now_time->min)) {//到时间
//                    printf("alarm[%d] ring,[%d:%d]\n", i, alarm_p->hour, alarm_p->minute);
//                    app_data.alarm_ring = i;
//                    alarm_later_check(now_time);
//                    if (alarm_p->cycle >> 7) {
//                        alarm_p->swi = 0;
//                    }
//                    return true;
//                }
//            }
//        }
//    }

    return false;
}

u8 alarm_moto_set(u8 period, u8 level, u8 swi)
{
    return (period | (level << 5) | (swi << 7));    //[0, 3]:震动间隔(百毫秒), [4, 5]:震动强度, [7]:震动开关
}

void alarm_later_check(tm_t *now_time)
{
//    printf("%s:%d %d\n", __func__, app_data.alarm_ring,
//           app_data.sector0.alarm[app_data.alarm_ring].remind_later);

//    alarm_clock_t *alarm_p = &app_data.sector0.alarm[app_data.alarm_ring];
//    u8 hour_later = alarm_p->hour;
//	u8 min_later = alarm_p->minute + alarm_p->remind_later;
//
//    if(alarm_p->remind_later) {
//
//        if (min_later > 59) {
//            hour_later += 1;
//            min_later -= 60;
//        }
//
//        if (hour_later > 24) {
//            hour_later -= 24;
//        }
//
//        alarm_equal(&app_data.alarm_later, 1, 0x80, hour_later, min_later, alarm_moto_set(5, 3, 1), 0);
////        printf("%s [%d:%d]\n", __func__, hour_later, min_later);
//    }
    return;
}

u8 alarm_clock_set(uint8_t index, bool enable, uint8_t cycle, uint8_t alarm_hour, uint8_t alarm_minute, uint8_t motor_mode, uint8_t remind_later) //新增
{
//    alarm_clock_t alarm_t = {0};
//    alarm_equal(&alarm_t, enable, cycle, alarm_hour, alarm_minute, motor_mode, remind_later);
//
//    if (alarm_num_get() >= ALARM_NUM_MAX || app_data.sector0.alarm[index].cycle) {  //已满或编号已存在
////        printf("alarm have creat or full\n");
//        return CMD_RESULT_IDX_INVALID;
//    }
//
//    if (index == 0) {
//        app_data.sector0.alarm_num = 0;
//    }
//
//    app_data.sector0.alarm_num++;
//    memcpy(&app_data.sector0.alarm[index], &alarm_t, sizeof(alarm_clock_t));
//    printf("set[%d]: swi[%d] [%d:%d] clcyle[%x] later[%d]\n",index, alarm_t.swi, alarm_t.hour, alarm_t.minute, alarm_t.cycle, alarm_t.remind_later);

//    msg_enqueue(EVT_ALARM_UPDATE);
    return 0;

}

u8 alarm_clock_edit(uint8_t index, bool enable, uint8_t cycle, uint8_t alarm_hour, uint8_t alarm_minute, uint8_t motor_mode, uint8_t remind_later)
{
//    alarm_clock_t alarm_new   = {0};
//    alarm_equal(&alarm_new, enable, cycle, alarm_hour, alarm_minute, motor_mode, remind_later);
//
//    if (app_data.sector0.alarm[index].cycle == 0) {  //编号不存在
//        TRACE("alarm not exist\n");
//        return CMD_RESULT_IDX_INVALID;
//    }
//
//    memcpy(&app_data.sector0.alarm[index], &alarm_new, sizeof(alarm_clock_t));
//    printf("edit[%d]: swi[%d] [%d:%d] clcyle[%x] later[%d]\n",index, alarm_new.swi, alarm_new.hour, alarm_new.minute, alarm_new.cycle, alarm_new.remind_later);

//    msg_enqueue(EVT_ALARM_UPDATE);
    return 0;
}

u8 alarm_clock_delete(uint8_t index)
{
//    app_data.sector0.alarm_num--;
//    memset(&app_data.sector0.alarm[index], 0, sizeof(alarm_clock_t));
//    printf("[%d]:cycle[%d]\n", index, app_data.sector0.alarm[index].cycle);
//
//    msg_enqueue(EVT_ALARM_UPDATE);
    return 0;
}

u8 alarm_clock_delete_all(void)
{
//    app_data.sector0.alarm_num = 0;
//    memset(app_data.sector0.alarm, 0, sizeof(alarm_clock_t)*ALARM_NUM_MAX);
//    msg_enqueue(EVT_ALARM_UPDATE);
    return 0;
}

//0x85:提醒协议
u8 remind_num_get(void)
{
    u8 num = 0;
//    for (u8 i = 0; i < REMIND_NUM_MAX; i++) {
//        if (app_data.sector0.remind[i].re_switch != 0xff) {
//            num++;
//        }
//    }
//    TRACE("%s:%d\n", __func__, num);
    return num;
}

uint8_t ble_get_remind_free_index(void)
{
//    TRACE("%s\n", __func__);
//    for (u8 i = 0; i < REMIND_NUM_MAX; i++) {
//        if (app_data.sector0.remind[i].re_switch == 0xff) {
//            return i;
//        }
//    }
    return 0xff;
}

u8 ble_upload_remind_data(void)
{
//    uint8_t frame_seq = 1;
//    uint8_t upload_data_arr[14] = {0x00};
//    uint8_t frame_sum = remind_num_get() + 1;
//    normal_remind_t *remind_p = {0};
//
//    TRACE("%s:%d %d\r\n", __FUNCTION__, frame_sum, ble_get_gatt_mtu());
//    for (u8 i = 0; i < REMIND_NUM_MAX; i++) {
//        if (app_data.sector0.remind[i].re_switch != 0xff) {
//            remind_p = &app_data.sector0.remind[i];
//            upload_data_arr[0]     = frame_seq + 1;
//            upload_data_arr[1]     = 0x84;
//            upload_data_arr[2]     = 0x02;
//            upload_data_arr[3]     = (frame_sum << 4) | frame_seq;
//            upload_data_arr[4]     = 9;
//            upload_data_arr[5]     = 0x00;
//            upload_data_arr[6]     = i;
//            memcpy(&upload_data_arr[7], remind_p, 7);
//            app_protocol_tx(upload_data_arr, 14);
//            frame_seq++;
//        }
//    }
    return 0;
}

void remind_equal(normal_remind_t *remind_t, uint8_t id, bool enable, uint8_t cycle, uint8_t alarm_hour, uint8_t alarm_minute, uint8_t motor_mode, uint8_t remind_later)
{
//    remind_t->re_id               = id;
//    remind_t->re_switch           = enable;
//    remind_t->re_cycle            = cycle;
//    remind_t->re_hour             = alarm_hour;
//    remind_t->re_minute           = alarm_minute;
//    remind_t->re_mototype         = motor_mode;
//    remind_t->re_remind_later     = remind_later;
}

u8 remind_set(uint8_t index, uint8_t id, bool enable, uint8_t cycle, uint8_t alarm_hour, uint8_t alarm_minute, uint8_t motor_mode, uint8_t remind_later)
{
//    TRACE("%s\r\n", __FUNCTION__);
//    normal_remind_t remind_t = {0};
//    remind_equal(&remind_t, id, enable, cycle, alarm_hour, alarm_minute, motor_mode, remind_later);
//
//    if (remind_num_get() >= REMIND_NUM_MAX || app_data.sector0.remind[index].re_switch != 0xff) {  //已满或编号已存在
//        return 1;
//    }
//
//    memcpy(&app_data.sector0.remind[index], &remind_t, sizeof(normal_remind_t));

//    msg_enqueue(EVT_REMIND_UPDATE);
    return 0;
}

u8 remind_edit(uint8_t index, uint8_t id, bool enable, uint8_t cycle, uint8_t alarm_hour, uint8_t alarm_minute, uint8_t motor_mode, uint8_t remind_later)
{
////    TRACE("%s\r\n", __FUNCTION__);
//    normal_remind_t remind_new   = {0};
//    remind_equal(&remind_new, id, enable, cycle, alarm_hour, alarm_minute, motor_mode, remind_later);
//
//    if (app_data.sector0.remind[index].re_switch == 0xff) {  //编号不存在
//        return CMD_RESULT_IDX_INVALID;
//    }
//
//    memcpy(&app_data.sector0.remind[index], &remind_new, sizeof(normal_remind_t));

//    msg_enqueue(EVT_REMIND_UPDATE);
    return CMD_RESULT_OK;
}

u8 remind_delete(uint8_t index)
{
//    if (app_data.sector0.remind[index].re_switch != 0xff) {
//        memset(&app_data.sector0.remind[index], 0xff, sizeof(normal_remind_t));
//
////        msg_enqueue(EVT_REMIND_UPDATE);
//    }
    return CMD_RESULT_OK;
}

u8 remind_delete_all(void)
{
//    if (remind_num_get()) {
//    //    TRACE("%s\r\n", __FUNCTION__);
//        memset(app_data.sector0.remind, 0xff, sizeof(normal_remind_t)*REMIND_NUM_MAX);
//
//        msg_enqueue(EVT_REMIND_UPDATE);
//    }
    return 0;
}

//间隔提醒
void ble_get_interval_remind(uint8_t event_type, uint8_t *data)
{
//    interval_remind_t *interval_remind_p = {0};
//    TRACE("%s\r\n", __func__);
//
//    interval_remind_p = &app_data.sector0.remind_group[event_type];
//    interval_remind_p->id = event_type;
//    memcpy(data, interval_remind_p, sizeof(interval_remind_t));
}

u8 ble_set_interval_remind(uint8_t *buf, uint8_t len)
{
//    u8 struct_size = sizeof(interval_remind_t) - 4;
//    TRACE("%s\n", __func__);
//    //print_r(buf, len);
//    if (len - 1 < struct_size) {
//        TRACE("[%s]:length error [len - 1]:%d [struct_size]:%d\n", __func__, len - 1, struct_size);
//        return CMD_RESULT_LEN_INVALID;
//    }
//
//    memcpy(&app_data.sector0.remind_group[buf[0]], &buf[0], struct_size);
//
//    //printf("[%d, %d, %d, %d]\n", rtc_tm.tm_hour, rtc_tm.tm_min, buf[2], buf[3]);
//
//    if ((rtc_tm.hour < buf[2]) || (rtc_tm.min < buf[3])) {
//        //printf("-->OUT TIME\n");
//        app_data.sector0.remind_group[buf[0]].cnt_cycle = buf[6] - 1;      //如果开始的时间在未来，第一个开始时间默认提醒
//    } else {
//        //printf("-->IN TIME\n");
//        app_data.sector0.remind_group[buf[0]].cnt_cycle = 0;               //如果开始的时间在过去，过n分钟后提醒
//    }
//
//    printf("[id]:%d [switch]:%d [start_hour]:%d [start_min]:%d [end_hour]:%d [end_min]:%d [in_cycle]:%d\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6]);
    return CMD_RESULT_OK;
}

bool interval_remind_check(tm_t *now_time)
{
//    printf("%s\r\n", __FUNCTION__);
//    uint8_t week_mask = 0;
//    interval_remind_t *interval_remind_p = {0};
//    interval_remind_p = app_data.sector0.remind_group;
//
//    week_mask = wday_rtc_to_proc(now_time->day);
//    if(!week_mask){
//        return false;
//    }
//
//    for(u8 i = 0; i < REMIND_NUM_MAX; i++) {
//        if (interval_remind_p[i].cycle >> 7) { //开关开启
//
////            printf("id[%d] cycle[%x] start_hour[%d] start_min[%d] end_hour[%d] end_min[%d] interval[%d] week_mask[%x]\n",
////                   interval_remind_p[i].id, interval_remind_p[i].cycle, interval_remind_p[i].start_hour, interval_remind_p[i].start_minute,
////                   interval_remind_p[i].end_hour, interval_remind_p[i].end_minute, interval_remind_p[i].interval, week_mask);
//
//            if (interval_remind_p[i].cycle & week_mask) {
//                if (clock_in_time(now_time, interval_remind_p[i].start_hour, interval_remind_p[i].start_minute, interval_remind_p[i].end_hour, interval_remind_p[i].end_minute)) {//在时间范围内
//                    interval_remind_p[i].cnt_cycle++;
////                    printf("cnt_cycle[%d]\r\n", interval_remind_p[i].cnt_cycle);
//                    if (interval_remind_p[i].cnt_cycle == interval_remind_p[i].interval || interval_remind_p[i].interval == 0) {//单位min
//                        interval_remind_p[i].cnt_cycle = 0;
////                        sys_cb.cover_index = GCOVER_HEALTH_SEDENTARY + i;   //起始位置递增
////                        msg_enqueue(EVT_WATCH_SET_COVER);
////                        msg_enqueue(EVT_T3_PWM_ON);
//
//                        if (interval_remind_p[i].interval == 0x00) {
//                            interval_remind_p[i].cycle &= ~BIT(7);    //单次提醒关闭重复提醒
//                        }
//
//                        return true;
//                    }
//                }
//            }
//        }
//    }
    return false;
}

//0x86:开关表扩展
uint32_t ble_get_social_switch_list(void)
{
    return app_data.sector1.social_sw.u32_sw;
}

void ble_set_social_switch_list(uint32_t sw)
{
//    app_data.sector1.social_sw.u32_sw = sw;
}

//0x90:音乐控制
void ble_set_music_state(uint8_t music_state)
{
//    if (bt_is_connected() || music_state > GMUSIC_BLE_STATE_PLAY) {
//        return;
//    }
//
//    app_data.music_state = music_state;
//    printf("%s:%d\n", __func__, music_state);
//
//    if (func_cb.sta == FUNC_GMUSIC) {
//        msg_enqueue(EVT_MUSIC_STOP + music_state);
//    }
}

void ble_set_music_data(uint8_t *buf, uint8_t len)
{
//    printf("%s\n", __func__);
//    if (bt_is_connected()) {
//        return;
//    }
//
//    print_r(buf, len);
//    u16 offset = 0;
//    bool is_total_time = false;
//    while (1) {
//        u8 index = buf[offset];
//        offset += 1;
////        u16 length = ((u32)buf[offset + 1]) | (((u32)buf[offset]) << 8);
////        offset += 2;
//        u8 length = buf[offset];
//        offset += 1;
//        printf("%s:idx[%d] length[%d] ofs[%d] len[%d]\n",__func__, index, length, offset, len);
//
//        if(offset >= len){
//            break;
//        }
//
//        switch (index) {
//            case MUSIC_DATA_TITLE:
////                printf("name: ");
//            case MUSIC_DATA_ARTIST:
////                printf("artist:");
////                printf("%s\n", &buf[offset]);
////                print_r(&buf[offset], length);
//                lcd_gmusic_show_id3_tag(index, &buf[offset], length);
//                break;
//
//            case MUSIC_DATA_DURATION:
////                printf("total time:");
//                is_total_time = true;
//            case MUSIC_DATA_CUR_TIME:{
//			 	if (index == MUSIC_DATA_CUR_TIME) {
//                    is_total_time = false;
//                }
////                printf("cur time:");
////                printf("%s\n", &buf[offset]);
////                print_r(&buf[offset], length);
//                uint8_t *ptr = NULL;
//                uint32_t time = 0;
//                ptr = (uint8_t *)&time;
//                for (int i = 0; i < 4; i++) {
//                    ptr[i] = buf[offset + 3 - i];
//                }
//                lcd_gmusic_show_id3_time(is_total_time, time);
//                break;
//            }
//            default:
//                break;
//        }
//        offset += length;
//        if(offset >= len){
//            break;
//        }
//    }

//    u8 len = title_len > 128? 128 : title_len;
//    memset((char*)(app_data.music_title), 0, 128);
//    memcpy((char*)(app_data.music_title), (char *)music_title, len);
}

void ble_set_music_vol(uint8_t vol)
{
//    if (bt_is_connected()) {
//        return;
//    }
//    printf("%s: vol = %d\n", __func__, vol);
//    app_data.music_vol = vol;
//    sys_cb.vol = vol;
//    bsp_change_volume(sys_cb.vol);
//    printf("music_vol: %d\n", app_data.music_vol);
//    if (func_cb.sta == FUNC_GMUSIC) {
//        if (!gmusic_get_volume_sta()) {
//            gmusic_volume_swi(1);
//        }
//        gmusic_volume_update();
//    }
}

void ble_request_app_music_state(void)
{
//    if(!ble_is_connect()){
//        return;
//    }
//    u8 music_buf[6] = {0x00, 0x90, 0x02, 0x00, 0x01, 0x00};
//    app_protocol_tx(music_buf, sizeof(music_buf));
}

void ble_control_app_music(uint8_t control_type, uint8_t music_vol)
{
//    uint8_t music_buf[7] = {0x00, 0x90, 0x03, 0x00, 0x02, 0x00, 0x00};
//    if(control_type == 0x00){
//        app_protocol_tx(music_buf, 6);
//    }
//    else if(control_type == 0x01){
//        music_buf[5] = 0x01;
//        app_protocol_tx(music_buf, 6);
//    }
//    else if(control_type == 0x02){
//        music_buf[5] = 0x02;
//        app_protocol_tx(music_buf, 6);
//    }
//    else if(control_type == 0x03){
//        music_buf[5] = 0x03;
//        app_protocol_tx(music_buf, 6);
//    }
//    else if(control_type == 0x04){
//        music_buf[5] = 0x04;
//        app_protocol_tx(music_buf, 6);
//    }
//    else if(control_type == 0x05){
//        music_buf[5] = 0x05;
//        app_protocol_tx(music_buf, 6);
//    }
//    else if(control_type == 0x06){
//        music_buf[5] = 0x06;
//        music_buf[6] = music_vol;
//        app_protocol_tx(music_buf, 7);
//    }
}

//0x91:远程拍照
//extern u8 camera_cmd;
bool platform_get_camera_control(void)
{
//    return app_data.camera_control;
    return 0;
}

void platform_set_camera_control(bool en)
{
//    app_data.camera_control = en;
//    if (!en) {
////        camera_cmd = 0;
//    }
}

#endif
