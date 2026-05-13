#include "include.h"
#include "ab_common.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if (USE_APP_TYPE == APP_BLUE_FIT)

///设备属性类
//0x50:时间同步
void ab_app_rtc_clock_set(u32 time_stamp, u8 time_zone)
{
    if (time_zone >= UTC_NULL) {
        return;
    }

    int tzone_diff = 0;
    if (app_data.time_zone != UTC_NULL) {
        tzone_diff = time_zone - app_data.time_zone;
        if (tzone_diff >= 0) {                              //切换时区禁止每日清空
            if (compo_cb.tm.hour + tzone_diff > 24) {
                app_data.dayclr_ban = 1;
            }
        } else {
            if (compo_cb.tm.hour + tzone_diff < 0) {
                app_data.dayclr_ban = 1;
            }
        }
    }

    if (app_data.time_zone != time_zone) {                  //时区变化
        app_data.time_zone = time_zone;
    }

    rtc_clock_timestamp_set(time_stamp, time_zone);

#if TRACE_EN
    tm_t tm = time_to_tm(RTCCNT);
    TRACE("%d-%d-%d %d:%d:%d\n", tm.year, tm.mon, tm.day, tm.hour, tm.min, tm.sec);
    TRACE("time_stamp[%d] time_zone[%d->%d] tzone_diff[%d] RTCCNT[%d]\n", time_stamp, app_data.time_zone, time_zone, tzone_diff, RTCCNT);
#endif
}

//0x51:电量获取
bool ab_app_charge_sta_get(void)
{
    bool charge_sta = sys_cb.charge_sta == 1 ? true : false;

    TRACE("%s: %d\n", __func__, charge_sta);
    return charge_sta;
}

u8 ab_app_vbat_percent_get(void)
{
    TRACE("%s: %d\n", __func__, sys_cb.vbat_percent);
    return sys_cb.vbat_percent;
}

//0x52:屏幕亮度
uint8_t ble_get_screen_brightness()
{
    return app_data.brightness_level;
}

void ble_set_screen_brightness(uint8_t brightness_value)
{
//    app_data.brightness_level = brightness_value;
//    lcd_bl_set(app_data.brightness_level);
//    msg_enqueue(EVT_BRIGHT_UPDATE);
}

//0x53:设备语言
uint8_t get_language_type(void)
{
    return app_data.sector1.lang;
}

//0x54:设备单位格式
uint8_t app_distance_unit_get(void)
{
    return app_data.distance_unit;
}

void ble_set_unit_type(uint8_t unit_type)
{
//    app_data.distance_unit = unit_type;
//    comm_refrash();         //马上更新

}

//0x56:设备亮屏时间
uint32_t ble_get_sleep_time(void)
{
    return sys_cb.sleep_time;
}

void ble_set_sleep_time(uint32_t sleep_time)
{
//    sys_cb.sleep_time = sleep_time * 10;    //单位100ms
//    TRACE("sleep_time[%ds][%dms]\n", (int)sleep_time, (int)sys_cb.sleep_time * 100);
}

//0x57:勿扰模式
void ble_get_not_disturb(void *disturb)
{
//    period_t *period_sys = &app_data.sector1.not_disturb_info.period;
//    not_disturb *disturb_get = disturb;
//
//    if (app_data.sector1.not_disturb_info.all_day) {
//        disturb_get->sw = app_data.sector1.not_disturb_info.all_day;
//    } else {
//        disturb_get->sw = app_data.sector1.not_disturb_info.sw;
//    }
//    period_set(&disturb_get->period, period_sys->start_hour, period_sys->start_minute, period_sys->end_hour, period_sys->end_minute);
//    printf("get: not_disturb swi[%d] start[%d:%d] end[%d:%d]\n", disturb_get->sw, disturb_get->period.start_hour,
//           disturb_get->period.start_minute, disturb_get->period.end_hour, disturb_get->period.end_minute);
}

void ble_set_not_disturb(uint8_t *disturb)
{
//    app_data.sector1.not_disturb_info.sw           = disturb[0];
//    period_set(&app_data.sector1.not_disturb_info.period, disturb[1], disturb[2], disturb[3], disturb[4]);
//
////    printf("set: not_disturb swi[%d] start[%d:%d] end[%d:%d]\n", disturb[0], disturb[1], disturb[2], disturb[3], disturb[4]);
//    if (app_data.sector1.not_disturb_info.sw) {//若app打开定时勿扰，则手表上关闭全天勿扰
//        if (app_data.sector1.not_disturb_info.all_day) {
//            app_data.sector1.not_disturb_info.all_day = 0;
//        }
//    } else {//app上关闭勿扰模式，手表清空定时时间
//        memset(&app_data.sector1.not_disturb_info, 0, sizeof(app_data.sector1.not_disturb_info));
//    }
}

bool now_is_not_disturb(void)
{
//    if (app_data.sector1.not_disturb_info.all_day || (app_data.sector1.not_disturb_info.sw &&
//        clock_in_time_range(&app_data.sector1.not_disturb_info.period))) {
//        return true;
//    }

    return false;
}

//0x59: 查找手机
void ab_app_search_phone(bool en)
{
    if (ab_app_is_connect()) {
        ab_app_tx_search_phone(en);
    }
}

void phone_find_sta_set(uint8_t val)
{
//    app_data.looking_ring = val;
//    msg_enqueue(EVT_PHONE_LOOK_UPDATE);
}

//0x5a:天气单位设置
void weather_set_unit(bool unit)
{
//    app_data.sector1.weather_unit = unit;
//    msg_enqueue(EVT_WEATHER_UNIT_UPDATE);
}

bool weather_get_unit(void)
{
    return app_data.sector1.weather_unit;
}

uint8_t weather_unit_convert(uint8_t weather_value)//天气单位°C->°F转换, weather_value: 当前°C
{
//    printf("weather_unit_convert\r\n");
//    weather_value = (weather_value * 9 / 5) + 32;
    return weather_value;
}

//0x5b: 12h/24h进制切换
uint8_t time_format_get(void)
{
    return app_data.sector1.time_format;
}

void time_format_set(uint8_t format)
{
//    app_data.sector1.time_format = format;
//    printf("time_format[%d %d]\r\n", app_data.sector1.time_format, format);
//    msg_enqueue(EVT_TIME_FORMAT_UPDATE);
}

uint8_t sys_get_time_format(uint8_t* hour)
{
//    if(hour == NULL)
//        return SYS_TIME_FORMAT_NULL;
//    if(app_data.sector1.time_format == 0x01) {    //12h
//        if(*hour == 0){
//            //*hour = 12;
//            return SYS_TIME_FORMAT_AM;
//        }
//        else if(*hour <12){
//            return SYS_TIME_FORMAT_AM;
//        }
//        else if(*hour == 12){
//            return SYS_TIME_FORMAT_PM;
//        }
//        else {
//            *hour = *hour - 12;
//            return SYS_TIME_FORMAT_PM;
//        }
//    }
    return SYS_TIME_FORMAT_NULL;
}

char *show_time_text_with_format(char *buf, uint8_t hour, uint8_t minute, bool format, bool with_ampm)
{
//    uint8_t now_hour = hour;
//    uint8_t now_minute = minute;
//
//    if (format) {
//        uint8_t hour_format = sys_get_time_format(&now_hour);
////        printf("hour_format:%d\r\n", hour_format);
//        if (with_ampm) {
//            if (hour_format == SYS_TIME_FORMAT_AM) {
//                sprintf(buf, "%d:%02d AM", now_hour, now_minute);
//                //printf("AM[%s]:%s\r\n", __FUNCTION__, buf);
//            } else if (hour_format == SYS_TIME_FORMAT_PM ){
//                sprintf(buf, "%d:%02d PM", now_hour, now_minute);
//                //printf("PM[%s]:%s\r\n", __FUNCTION__, buf);
//            } else {
//                sprintf(buf, "%02d:%02d", now_hour, now_minute);
//                //printf("0[%s]:%s\r\n", __FUNCTION__, buf);
//            }
//        } else {
//            sprintf(buf, "%02d:%02d", now_hour, now_minute);
//            //printf("1[%s]:%s\r\n", __FUNCTION__, buf);
//        }
//    } else {
//        sprintf(buf, "%02d:%02d", now_hour, now_minute);
//        //printf("3[%s]:%s\r\n", __FUNCTION__, buf);
//    }
    return buf;
}

char *show_ampm_text_with_format(char *buf, uint8_t hour)
{
//    uint8_t now_hour = hour;
//
//    uint8_t hour_format = sys_get_time_format(&now_hour);
//    if (hour_format == SYS_TIME_FORMAT_AM) {
//        sprintf(buf, "AM");
//    } else if (hour_format == SYS_TIME_FORMAT_PM) {
//        sprintf(buf, "PM");
//    } else {
//        sprintf(buf, "  ");
//    }
    return buf;
}

//0x5c:设备信息上报
bool ble_uplode_watch_info(u8 type)
{
    u32 dev_lang = DEVICE_LANG;
    u32 func_suport = DEVICE_FUNC;
    u32 func_detection = DETECTION_FUNC;
    u32 device_msg = DEVICE_MSG;
    u32 device_swi = DEVICE_SWI;
    u32 device_sport = DEVICE_SPORT;

    TRACE("type[%d] lang[%x] func[%x] det[%x] msg[%x] swi[%x] sport[%x]\n",
           WATCH_TYPE, dev_lang, func_suport, func_detection, device_msg, device_swi, device_sport);

    u8 buf[30] = {0x00, type, CMD_TYPE_NOTIFY, 0x00, 0x1d};
    buf[5] = WATCH_TYPE;//手表
    memcpy(&buf[6], &dev_lang, 4);
    memcpy(&buf[10], &func_suport, 4);
    memcpy(&buf[14], &func_detection, 4);
    memcpy(&buf[18], &device_msg, 4);
    memcpy(&buf[22], &device_swi, 4);
    memcpy(&buf[26], &device_sport, 4);

    return app_protocol_tx(buf, sizeof(buf));
}

bool ble_get_watch_info(u8 *buf)
{
    u32 lang = DEVICE_LANG;
    buf[2] = 0x02;

    TRACE("ble_get_watch_info:%d\n", buf[6]);
    switch (buf[6]) {
    case 0x00:  //手表类型
        buf[7] = WATCH_TYPE;
        buf[4] = 0x03;
        break;

    case 0x01:  //设备支持的语言
        memcpy(&buf[7], &lang, sizeof(lang));
        buf[4] = 0x06;
        break;

    case 0x02:  //序列号
        printf("serial_num:");
        print_r(xcfg_cb.serial_num, 6);
        printf("\n");
        memcpy(&buf[7], xcfg_cb.serial_num, sizeof(xcfg_cb.serial_num));
        buf[4] = 0x08;
        break;
    }

    print_r(buf, 5 + buf[4]);
    return app_protocol_tx(buf, 5 + buf[4]);
}

//0x5d:应用端信息
void app_phone_type_set(u8 type)
{
    app_data.ble_app_ios = type;
}

u8 app_phone_type_get(void)
{
    return app_data.ble_app_ios;
}

#endif

