#ifndef _APP_AB_H_
#define _APP_AB_H_

///设备属性类
//电量同步
bool get_charge_sta(void);
u8 get_vbat_percent(void);
void ble_uplode_battery(u8 value, bool charge_sta);             //ble上报当前电量

//息屏时间
uint8_t ble_get_screen_brightness();
void ble_set_screen_brightness(uint8_t brightness_value);

//设备语言
uint8_t get_language_type(void);

//公英制格式
uint8_t ble_get_unit_type();
void ble_set_unit_type(uint8_t unit_type);

//勿扰模式
void ble_get_not_disturb(void *distrub);
void ble_set_not_disturb(uint8_t *distrub);
bool now_is_not_disturb(void);

///个人信息类
//个人信息
void user_info_get(void *user);
void user_info_set(void *user);

//开关设置
uint32_t ble_get_universal_switch(void);
void ble_set_universal_switch(uint32_t sw);
uint32_t ble_get_social_switch_list(void);
void ble_set_social_switch_list(uint32_t sw);

///状态设置类
//闹钟
u8 alarm_num_get(void);
u8 ble_upload_alarm_data(void);
uint8_t alarm_free_index_get(void);
u8 alarm_clock_set(uint8_t index, bool enable, uint8_t cycle, uint8_t alarm_hour, uint8_t alarm_minute, uint8_t motor_mode, uint8_t remind_later);
u8 alarm_clock_edit(uint8_t index, bool enable, uint8_t cycle, uint8_t alarm_hour, uint8_t alarm_minute, uint8_t motor_mode, uint8_t remind_later);
u8 alarm_clock_delete(uint8_t index);
u8 alarm_clock_delete_all(void);
bool alarm_check(tm_t *now_time);
void alarm_later_check(tm_t *now_time);

//单次提醒
u8 remind_num_get(void);
u8 ble_upload_remind_data(void);
uint8_t ble_get_remind_free_index(void);
u8 remind_set(uint8_t index, uint8_t id, bool enable, uint8_t cycle, uint8_t alarm_hour, uint8_t alarm_minute, uint8_t motor_mode, uint8_t remind_later);
u8 remind_edit(uint8_t index, uint8_t id, bool enable, uint8_t cycle, uint8_t alarm_hour, uint8_t alarm_minute, uint8_t motor_mode, uint8_t remind_later);
u8 remind_delete(uint8_t index);
u8 remind_delete_all(void);

//间隔提醒
void ble_get_interval_remind(uint8_t event_type, uint8_t *data);
u8 ble_set_interval_remind(uint8_t *buf, uint8_t len);

//预警
void ble_get_sedentary_remind(uint8_t *data);
void ble_set_sedentary_remind(uint8_t *data);

//音乐控制
void ble_set_music_state(uint8_t music_state);
void ble_set_music_data(uint8_t *buf, uint8_t len);
void ble_set_music_vol(uint8_t vol);

//拍照
bool ble_app_camera_control(uint8_t cmd);

//联系人
void contact_name_set(u8 index, u8 *name, u8 len);                      //保存联系人名称
void contact_number_set(u8 index, u8 *num, u8 buf_len, u8 num_len);     //保存联系人号码
u8 get_contact_num(void);                                               //获取联系人个数
void contact_delete_all(void);                                          //清空联系人数据

///运动相关类
//目标
void ble_get_daily_goals(uint8_t *data);
void ble_set_daily_goals(uint8_t *buf, uint8_t len);

//运动数据
void ble_upload_sport_online(u8 cmd_type);
bool ble_sport_record_upload(u8 cmd_type);
void sport_data_set(sport_record_t *sport);
void sport_data_save(sport_record_t *sport);
bool sport_data_is_valid(sport_record_t *sport);
u8 sport_data_num_get(void);
void sport_data_get_stamp(void);
void ble_step_data_del(void);

//睡眠数据
uint32_t ble_get_sleep_time(void);
void ble_set_sleep_time(uint32_t sleep_time);
bool ble_get_sleep_data(u8 *buf);
void ble_sleep_data_del(void);
bool ble_upload_sleep_data(void);
void get_sleep_check(uint8_t *data);
void set_sleep_check(uint8_t *data);

//健康数据
void ble_hr_data_del(uint8_t type);
bool ble_upload_hr_data(void);
void ble_get_auto_hr_interval(uint8_t *data);
void ble_set_auto_hr_interval(uint8_t *data);
void ble_get_auto_hr_alarm_range(uint8_t *data);
void ble_set_auto_hr_alarm_range(uint8_t *data);
void ble_get_hr_total(uint8_t *data);
void ble_get_hrv_total(uint8_t *data);
bool ble_upload_hrv_data(void);
void ble_get_base_cal_sw(uint8_t *data);
void ble_set_base_cal_sw(uint8_t *data);
void ble_get_workout_total(uint8_t *data);
bool ble_upload_workout_record_data(void);
void ble_work_record_del(void);
uint8_t time_format_get(void);

//测量相关
void ble_realtime_measure_upload(u8 type);              //type: 0:hr; 1:spo2; 2:sbp/dbp;
void ble_interval_measure_upload(u8 type);              //type: 0:hr; 1:spo2; 2:sbp/dbp;
void interval_measure_check(void);
u8 ble_interval_measure_query(u8 type);
void bpm_avg_reset(void);
u8 hrsensor_upload_swi(u8 type, u8 swi);
void platform_hr_save(u8 hr);
void platform_bp_save(u8 sbp, u8 dbp);
void platform_spo2_save(u8 spo2);
void ble_measure_upload(u8 cmd_type, u8 data_type);     //data_type: 0:step/cal/distance
void measure_check(void);
void gui_jump_measure_menu(u8 type, u8 swi);
u8 get_hr_swi_sta();

///数据传输类
void watch_diy_transmit_rsp(u8 *buf, u8 response);

#endif  //_APP_AB_H_
