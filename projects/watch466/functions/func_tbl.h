#ifndef __FUNC_TBL_H__
#define __FUNC_TBL_H__

#define FUNC_CREATE_CNT                       ((int)(sizeof(tbl_func_create) / sizeof(tbl_func_create[0])))
#define FUNC_ENTRY_CNT                        ((int)(sizeof(tbl_func_entry) / sizeof(tbl_func_entry[0])))
#define FUNC_ENTER_CNT                        ((int)(sizeof(tbl_func_enter) / sizeof(tbl_func_enter[0])))
#define FUNC_EXIT_CNT                        ((int)(sizeof(tbl_func_exit) / sizeof(tbl_func_exit[0])))


#define WATCH_FUNC_EN                      0   // 手表功能开关，饭盒项目设为0


#define WATCH_FUNC_EN                      0   // 手表功能开关，饭盒项目设为0

typedef struct func_t_ {
    int func_idx;
    void *func;
} func_t;


compo_form_t *func_menu_form_create(void);
compo_form_t *func_clock_form_create(void);
compo_form_t *func_clock_sub_sidebar_form_create(void);
compo_form_t *func_clock_sub_card_form_create(void);
compo_form_t *func_heartrate_form_create(void);
compo_form_t *func_bt_form_create(void);
compo_form_t *func_bt_outgoing_form_create(void);
compo_form_t *func_alarm_clock_form_create(void);
compo_form_t *func_alarm_clock_sub_set_form_create(void);
compo_form_t *func_alarm_clock_sub_repeat_form_create(void);
compo_form_t *func_alarm_clock_sub_edit_form_create(void);
compo_form_t *func_blood_oxygen_form_create(void);
compo_form_t *func_breathe_form_create(void);
compo_form_t *func_calculator_form_create(void);
compo_form_t *func_camera_form_create(void);
compo_form_t *func_light_form_create(void);
compo_form_t *func_timer_form_create(void);
compo_form_t *func_sleep_form_create(void);
compo_form_t *func_stopwatch_form_create(void);
compo_form_t *func_stopwatch_sub_record_form_create(void);
compo_form_t *func_weather_form_create(void);
compo_form_t *func_sport_form_create(void);
compo_form_t *func_sport_config_form_create(void);
compo_form_t *func_sport_sub_run_form_create(void);
compo_form_t *func_sport_switching_form_create(void);
compo_form_t *func_set_sub_disturd_form_create(void);
compo_form_t *func_disturd_sub_set_form_create(void);
compo_form_t *func_call_form_create(void);
compo_form_t *func_call_sub_record_form_create(void);
compo_form_t *func_call_sub_dial_form_create(void);
compo_form_t *func_game_form_create(void);
compo_form_t *func_style_form_create(void);
compo_form_t *func_findphone_form_create(void);
compo_form_t *func_altitude_form_create(void);
compo_form_t *func_map_form_create(void);
compo_form_t *func_message_form_create(void);
compo_form_t *func_scan_form_create(void);
compo_form_t *func_voice_form_create(void);
#if SECURITY_PAY_EN
compo_form_t *func_alipay_form_create(void);
#endif // SECURITY_PAY_EN
compo_form_t *func_compass_form_create(void);
compo_form_t *func_address_book_form_create(void);
compo_form_t *func_set_sub_list_form_create(void);
compo_form_t *func_set_sub_wrist_form_create(void);
compo_form_t *func_set_sub_sav_form_create(void);
compo_form_t *func_set_sub_dousing_form_create(void);
compo_form_t *func_set_sub_language_form_create(void);
compo_form_t *func_set_sub_time_form_create(void);
compo_form_t *func_set_sub_menu_navigation_form_create(void);
compo_form_t *func_time_sub_custom_form_create(void);
compo_form_t *func_set_sub_password_form_create(void);
compo_form_t *func_password_sub_disp_form_create(void);
compo_form_t *func_password_sub_select_form_create(void);
compo_form_t *func_set_sub_4g_form_create(void);
compo_form_t *func_set_sub_about_form_create(void);
compo_form_t *func_set_sub_restart_form_create(void);
compo_form_t *func_set_sub_rstfy_form_create(void);
compo_form_t *func_set_sub_off_form_create(void);
compo_form_t *func_calender_form_create(void);
compo_form_t *func_volume_form_create(void);
compo_form_t *func_activity_form_create(void);
compo_form_t *func_bloodsugar_form_create(void);
compo_form_t *func_bloodpressure_form_create(void);
compo_form_t *func_flashlight_form_create(void);
compo_form_t *func_brightness_form_create(void);
compo_form_t *func_charge_form_create(void);
compo_form_t *func_clock_preview_form_create(void);
compo_form_t *func_compo_select_form_create(void);
compo_form_t *func_compo_select_sub_form_create(void);
compo_form_t *func_debug_info_form_create(void);
compo_form_t *func_home_form_create(void);
compo_form_t *func_heat_form_create(void);
compo_form_t *func_mode_form_create(void);
compo_form_t *func_home_page_form_create(void);
compo_form_t *func_new_heat_form_create(void);
compo_form_t *func_new_warm_form_create(void);
compo_form_t *func_lowbat_form_create(void);
compo_form_t *func_new_mode_form_create(void);
compo_form_t *func_new_setup_form_create(void);
compo_form_t *func_new_language_form_create(void);
compo_form_t *func_new_verinfo_form_create(void);
compo_form_t *func_new_timeing_form_create(void);
compo_form_t *func_lid_confirm_form_create(void);
#if ELUNCHBOX_PANEL_EN
compo_form_t *func_new_reservation_form_create(void);
#else
compo_form_t *func_reservation_form_create(void);
#endif
compo_form_t *func_setup_form_create(void);
compo_form_t *func_timeing_form_create(void);
compo_form_t *func_languageing_form_create(void);
compo_form_t *func_verinfo_form_create(void);
compo_form_t * func_smartstack_form_create(void);
compo_form_t *func_music_form_create(void);
#if BT_EMIT_EN
compo_form_t *func_music_src_form_create(void);
compo_form_t * func_emit_list_form_create(void);
#endif
compo_form_t *func_usbdev_form_create(void);
compo_form_t *func_recorder_form_create(void);
compo_form_t *func_message_reply_form_create(void);
compo_form_t * func_mic_test_form_create(void);
compo_form_t *func_game_tetris_form_create(void);
compo_form_t *func_game_tetris_start_form_create(void);
compo_form_t *func_game_tetris_over_form_create(void);
compo_form_t *func_bird_form_create(void);
#if FUNC_BLE_GATTS_EN
compo_form_t *func_ble_gatts_form_create(void);
#endif

#if VIDEO_PLAY_EN
compo_form_t *func_video_play_form_create(void);
#endif // VIDEO_PLAY_EN
#if PHOTO_VIEW_EN
compo_form_t *func_photo_view_form_create(void);
#endif // PHOTO_VIEW_EN
#if AVI_DVP_DEMOLIST
compo_form_t *func_video_showlist_form_create(void);
#endif // AVI_DVP_DEMOLIST
#if VIDEO_RECODE_TAKE_PHOTO_EN
compo_form_t *func_video_recode_form_create(void);
compo_form_t *func_take_photo_form_create(void);
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
compo_form_t *func_gif_form_create(void);

const func_t tbl_func_create[] = {
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MENU,                         func_menu_form_create},
    {FUNC_MENUSTYLE,                    NULL},
    {FUNC_CLOCK,                        func_clock_form_create},
    {FUNC_CLOCK_PREVIEW,                func_clock_preview_form_create},
    {FUNC_SIDEBAR,                      func_clock_sub_sidebar_form_create},
    {FUNC_CARD,                         func_clock_sub_card_form_create},
    {FUNC_HEARTRATE,                    func_heartrate_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
    {FUNC_BT,                           func_bt_form_create},
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_BT_CALL,                      func_bt_outgoing_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
    {FUNC_COMPO_SELECT,                 func_compo_select_form_create},
    {FUNC_COMPO_SELECT_SUB,             func_compo_select_sub_form_create},
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_ALARM_CLOCK,                  func_alarm_clock_form_create},
    {FUNC_ALARM_CLOCK_SUB_SET,          func_alarm_clock_sub_set_form_create},
    {FUNC_ALARM_CLOCK_SUB_REPEAT,       func_alarm_clock_sub_repeat_form_create},
    {FUNC_ALARM_CLOCK_SUB_EDIT,         func_alarm_clock_sub_edit_form_create},
    {FUNC_BLOOD_OXYGEN,                 func_blood_oxygen_form_create},
    {FUNC_BLOODSUGAR,                   func_bloodsugar_form_create},
    {FUNC_BLOOD_PRESSURE,               func_bloodpressure_form_create},
    {FUNC_BREATHE,                      func_breathe_form_create},
    {FUNC_CALCULATOR,                   func_calculator_form_create},
    {FUNC_CAMERA,                       func_camera_form_create},
    {FUNC_TIMER,                        func_timer_form_create},
    {FUNC_SLEEP,                        func_sleep_form_create},
    {FUNC_STOPWATCH,                    func_stopwatch_form_create},
    {FUNC_STOPWATCH_SUB_RECORD,         func_stopwatch_sub_record_form_create},
    {FUNC_WEATHER,                      func_weather_form_create},
    {FUNC_SPORT,                        func_sport_form_create},
    {FUNC_SPORT_CONFIG,                 func_sport_config_form_create},
    {FUNC_SPORT_SUB_RUN,                func_sport_sub_run_form_create},
    {FUNC_SPORT_SWITCH,                 func_sport_switching_form_create},
    {FUNC_GAME,                         func_game_form_create},
    {FUNC_STYLE,                        func_style_form_create},
    {FUNC_FINDPHONE,                    func_findphone_form_create},
    {FUNC_ALTITUDE,                     func_altitude_form_create},  
    {FUNC_MAP,                          func_map_form_create},
    {FUNC_MESSAGE,                      func_message_form_create},
    {FUNC_SCAN,                         func_scan_form_create},
    {FUNC_VOICE,                        func_voice_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#if SECURITY_PAY_EN
    {FUNC_ALIPAY,                       func_alipay_form_create},
#endif // SECURITY_PAY_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_COMPASS,                      func_compass_form_create},
    {FUNC_ADDRESS_BOOK,                 func_address_book_form_create},
    {FUNC_CALL,                         func_call_form_create},
    {FUNC_CALL_SUB_RECORD,              func_call_sub_record_form_create},
    {FUNC_CALL_SUB_DIAL,                func_call_sub_dial_form_create},
    {FUNC_SETTING,                      func_set_sub_list_form_create},
    {FUNC_CALENDAER,                    func_calender_form_create},
    {FUNC_VOLUME,                       func_volume_form_create},
    {FUNC_ACTIVITY,                     func_activity_form_create},
    {FUNC_FLASHLIGHT,                   func_flashlight_form_create},
    {FUNC_BRIGHTNESS,                   func_brightness_form_create},
    {FUNC_LIGHT,                        func_light_form_create},
    {FUNC_SET_SUB_DOUSING,              func_set_sub_dousing_form_create},
    {FUNC_SET_SUB_WRIST,                func_set_sub_wrist_form_create},
    {FUNC_SET_SUB_DISTURD,              func_set_sub_disturd_form_create},
    {FUNC_DISTURD_SUB_SET,              func_disturd_sub_set_form_create},
    {FUNC_SET_SUB_LANGUAGE,             func_set_sub_language_form_create},
    {FUNC_SET_SUB_TIME,                 func_set_sub_time_form_create},
    {FUNC_SET_SUB_MENU_NAVIGATION,      func_set_sub_menu_navigation_form_create},
    {FUNC_TIME_SUB_CUSTOM,              func_time_sub_custom_form_create},
    {FUNC_SET_SUB_PASSWORD,             func_set_sub_password_form_create},
    {FUNC_PASSWORD_SUB_DISP,            func_password_sub_disp_form_create},
    {FUNC_PASSWORD_SUB_SELECT,          func_password_sub_select_form_create},
    {FUNC_SET_SUB_SAV,                  func_set_sub_sav_form_create},
    {FUNC_SET_SUB_ABOUT,                func_set_sub_about_form_create},
    {FUNC_SET_SUB_4G,                   func_set_sub_4g_form_create},
    {FUNC_SET_SUB_RESTART,              func_set_sub_restart_form_create},
    {FUNC_SET_SUB_RSTFY,                func_set_sub_rstfy_form_create},
    {FUNC_SET_SUB_OFF,                  func_set_sub_off_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
    {FUNC_CHARGE,                       func_charge_form_create},
    {FUNC_DEBUG_INFO,                   func_debug_info_form_create},
    {FUNC_HEAT,                         func_heat_form_create},
    {FUNC_HOME,                         func_home_form_create},
    {FUNC_MODE,                         func_mode_form_create},
    {FUNC_NEW_HEAT,                     func_new_heat_form_create},
    {FUNC_HOME_PAGE,                    func_home_page_form_create},
    {FUNC_NEW_WARM,                     func_new_warm_form_create},
    {FUNC_LOWBAT,                       func_lowbat_form_create},
    {FUNC_NEW_MODE,                     func_new_mode_form_create},
    {FUNC_NEW_SETUP,                    func_new_setup_form_create},
    {FUNC_NEW_LANGUAGE,                 func_new_language_form_create},
    {FUNC_NEW_VERINFO,                  func_new_verinfo_form_create},
    {FUNC_NEW_TIME,                     func_new_timeing_form_create},
    {FUNC_LID_CONFIRM,                  func_lid_confirm_form_create},
#if ELUNCHBOX_PANEL_EN
    {FUNC_RESERVATION,                  func_new_reservation_form_create},
#else
    {FUNC_RESERVATION,                  func_reservation_form_create},
#endif
    {FUNC_SETUP,                        func_setup_form_create},
    {FUNC_TIMEING,                      func_timeing_form_create},
    {FUNC_LANGUAGEING,                  func_languageing_form_create},
    {FUNC_VERINFO,                      func_verinfo_form_create},
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_SMARTSTACK,                   func_smartstack_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#if BT_EMIT_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MUSIC_SRC,                    func_music_src_form_create},
	{FUNC_EMIT_LIST,                    func_emit_list_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_MUSIC_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MUSIC,                        func_music_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_USBDEV_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_USBDEV,                       func_usbdev_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_RECORDER_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_RECORDER,                     func_recorder_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FLASHDB_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MESSAGE_REPLY,                func_message_reply_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif

    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_BIRD,                         func_bird_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#if FUNC_BLE_GATTS_EN
    {FUNC_BLE_GATTS,                    func_ble_gatts_form_create},
#endif
#if FUNC_GAME_TETRIS_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_GAME_TETRIS,                  func_game_tetris_form_create},
    {FUNC_GAME_TETRIS_START,            func_game_tetris_start_form_create},
    {FUNC_GAME_TETRIS_OVER,             func_game_tetris_over_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // FUNC_GAME_TETRIS_EN

#if VIDEO_PLAY_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_VIDEO_PLAY,                   func_video_play_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // VIDEO_PLAY_EN
#if PHOTO_VIEW_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_PHOTO_VIEW,                   func_photo_view_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // PHOTO_VIEW_EN
#if AVI_DVP_DEMOLIST
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_VIDEO_SHOWLIST,               func_video_showlist_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // AVI_DVP_DEMOLIST
#if VIDEO_RECODE_TAKE_PHOTO_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_VIDEO_RECODE,                 func_video_recode_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
    {FUNC_TAKE_PHOTO,                   func_take_photo_form_create},
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_GIF,                          func_gif_form_create},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN

};

extern void func_menu(void);
extern void func_menustyle(void);
extern void func_clock(void);
extern void func_clock_preview(void);
extern void func_clock_sub_sidebar(void);
extern void func_clock_sub_card(void);
extern void func_heartrate(void);
extern void func_compo_select(void);
extern void func_compo_select_sub(void);
extern void func_alarm_clock(void);
extern void func_alarm_clock_sub_set(void);
extern void func_alarm_clock_sub_repeat(void);
extern void func_alarm_clock_sub_edit(void);
extern void func_disturd_sub_set(void);
extern void func_blood_oxygen(void);
extern void func_breathe(void);
extern void func_calculator(void);
extern void func_camera(void);
extern void func_light(void);
extern void func_timer(void);
extern void func_sleep(void);
extern void func_stopwatch(void);
extern void func_stopwatch_sub_record(void);
extern void func_weather(void);
extern void func_sport(void);
extern void func_sport_config(void);
extern void func_sport_sub_run(void);
extern void func_calendar(void);
extern void func_call(void);
extern void func_call_sub_record(void);
extern void func_call_sub_dial(void);
extern void func_game(void);
extern void func_style(void);
extern void func_findphone(void);
extern void func_altitude(void);
extern void func_map(void);
extern void func_message_info(void);
extern void func_scan(void);
extern void func_voice(void);
#if SECURITY_PAY_EN
extern void func_alipay(void);
#endif // SECURITY_PAY_EN
extern void func_compass(void);
extern void func_address_book(void);
extern void func_set_sub_list(void);
extern void func_set_sub_sav(void);
extern void func_set_sub_dousing(void);
extern void func_set_sub_disturd(void);
extern void func_set_sub_language(void);
extern void func_set_sub_wrist(void);
extern void func_set_sub_time(void);
extern void func_set_sub_menu_navigation(void);
extern void func_time_sub_custom(void);
extern void func_set_sub_password(void);
extern void func_password_sub_disp(void);
extern void func_password_sub_select(void);
extern void func_set_sub_about(void);
extern void func_set_sub_restart(void);
extern void func_set_sub_rstfy(void);
extern void func_set_sub_off(void);
extern void func_set_sub_4g(void);
extern void func_switching_to_menu(void);
extern void func_volume(void);
extern void func_activity(void);
extern void func_sport_switching(void);
extern void func_bloodsugar(void);
extern void func_bloodpressure(void);
extern void func_flashlight(void);
extern void func_brightness(void);
extern void func_charge(void);
extern void func_debug_info(void);
extern void func_home(void);
extern void func_heat(void);
extern void func_mode(void);
extern void func_reservation(void);
extern void func_new_reservation(void);
extern void func_setup(void);
extern void func_new_heat(void);
extern void func_home_page(void);
extern void func_new_warm(void);
extern void func_lowbat(void);
extern void func_new_mode(void);
extern void func_new_setup(void);
extern void func_new_language(void);
extern void func_new_verinfo(void);
extern void func_new_timeing(void);
extern void func_lid_confirm(void);
extern void func_timeing(void);
extern void func_languageing(void);
extern void func_verinfo(void);
void func_home_process(void);
void func_home_message(size_msg_t msg);
void func_home_mode_key(void);
void func_home_confirm_key(void);

extern void func_music(void);
extern void func_music_src(void);
extern void func_idle(void);
extern void func_ota_ui(void);
extern void func_bt_update(void);
extern void func_bt(void);
extern void func_bt_ring(void);
extern void func_bt_call(void);
extern void func_bthid(void);
extern void func_usbdev(void);
extern void func_aux(void);
extern void func_smartstack(void);
extern void func_modem_call(void);
extern void func_modem_ring(void);
extern void func_message_reply_info(void);
extern void func_mic_test(void);
extern void func_emit_list(void);
#if FUNC_GAME_TETRIS_EN
extern void func_game_tetris(void);
extern void func_game_tetris_start(void);
extern void func_game_tetris_over(void);
#endif // FUNC_GAME_TETRIS_EN
extern void func_bird(void);
#if FUNC_BLE_GATTS_EN
extern void func_ble_gatts(void);
#endif

#if VIDEO_PLAY_EN
extern void func_video_play(void);
#endif // VIDEO_PLAY_EN
#if PHOTO_VIEW_EN
extern void func_photo_view(void);
#endif // PHOTO_VIEW_EN
#if AVI_DVP_DEMOLIST
extern void func_video_showlist(void);
#endif // AVI_DVP_DEMOLIST
#if VIDEO_RECODE_TAKE_PHOTO_EN
extern void func_video_recode(void);
extern void func_take_photo(void);
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
extern void func_gif(void);

const func_t tbl_func_entry[] = {
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MENU,                         func_menu},                     //主菜单(蜂窝)
    {FUNC_MENUSTYLE,                    func_menustyle},                //主菜单样式选择
    {FUNC_CLOCK,                        func_clock},                    //时钟表盘
    {FUNC_CLOCK_PREVIEW,                func_clock_preview},            //时钟表盘预览
    {FUNC_SIDEBAR,                      func_clock_sub_sidebar},        //表盘右滑
    {FUNC_CARD,                         func_clock_sub_card},           //表盘上拉
    {FUNC_HEARTRATE,                    func_heartrate},                //心率
    {FUNC_ALARM_CLOCK,                  func_alarm_clock},              //闹钟
    {FUNC_ALARM_CLOCK_SUB_SET,          func_alarm_clock_sub_set},      //闹钟--设置
    {FUNC_ALARM_CLOCK_SUB_REPEAT,       func_alarm_clock_sub_repeat},   //闹钟--重复
    {FUNC_ALARM_CLOCK_SUB_EDIT,         func_alarm_clock_sub_edit},     //闹钟--编辑
    {FUNC_BLOOD_OXYGEN,                 func_blood_oxygen},             //血氧
    {FUNC_BLOODSUGAR,                   func_bloodsugar},               //血糖
    {FUNC_BLOOD_PRESSURE,               func_bloodpressure},            //血压
    {FUNC_BREATHE,                      func_breathe},                  //呼吸
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
    {FUNC_COMPO_SELECT,                 func_compo_select},             //组件选择
    {FUNC_COMPO_SELECT_SUB,             func_compo_select_sub},         //组件选择子界面
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_CALCULATOR,                   func_calculator},               //计算器
    {FUNC_CAMERA,                       func_camera},                   //相机
    {FUNC_LIGHT,                        func_light},                    //亮度调节
    {FUNC_TIMER,                        func_timer},                    //定时器
    {FUNC_SLEEP,                        func_sleep},                    //睡眠
    {FUNC_STOPWATCH,                    func_stopwatch},                //秒表
    {FUNC_STOPWATCH_SUB_RECORD,         func_stopwatch_sub_record},     //秒表--秒表记录
    {FUNC_WEATHER,                      func_weather},                  //天气
    {FUNC_SPORT,                        func_sport},                    //运动
    {FUNC_SPORT_CONFIG,                 func_sport_config},             //运动配置
    {FUNC_SPORT_SUB_RUN,                func_sport_sub_run},            //运动--室内跑步
    {FUNC_SPORT_SWITCH,                 func_sport_switching},          //运动开启动画
    {FUNC_GAME,                         func_game},                     //游戏
    {FUNC_STYLE,                        func_style},                    //菜单风格
    {FUNC_ALTITUDE,                     func_altitude},                 //海拔
    {FUNC_FINDPHONE,                    func_findphone},                //寻找手机
    {FUNC_MAP,                          func_map},                      //地图
    {FUNC_MESSAGE,                      func_message_info},             //消息
    {FUNC_SCAN,                         func_scan},                     //扫一扫
    {FUNC_VOICE,                        func_voice},                    //语音助手
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#if SECURITY_PAY_EN
    {FUNC_ALIPAY,                       func_alipay},                   //支付宝
#endif // SECURITY_PAY_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_COMPASS,                      func_compass},                  //指南针
    {FUNC_ADDRESS_BOOK,                 func_address_book},             //电话簿
    {FUNC_CALENDAER,                    func_calendar},                 //日历
    {FUNC_CALL,                         func_call},                     //电话
    {FUNC_CALL_SUB_RECORD,              func_call_sub_record},          //电话--最近通话
    {FUNC_CALL_SUB_DIAL,                func_call_sub_dial},            //电话--拨号
    {FUNC_VOLUME,                       func_volume},                   //音量调节
    {FUNC_ACTIVITY,                     func_activity},                 //活动记录
    {FUNC_FLASHLIGHT,                   func_flashlight},               //手电筒
    {FUNC_BRIGHTNESS,                   func_brightness},               //亮度（全屏图标）
    {FUNC_SETTING,                      func_set_sub_list},             //设置
    {FUNC_SET_SUB_DOUSING,              func_set_sub_dousing},          //设置--熄屏
    {FUNC_SET_SUB_WRIST,                func_set_sub_wrist},            //设置--抬腕
    {FUNC_SET_SUB_DISTURD,              func_set_sub_disturd},          //设置--勿扰
    {FUNC_DISTURD_SUB_SET,              func_disturd_sub_set},          //勿扰--时间设置
    {FUNC_SET_SUB_SAV,                  func_set_sub_sav},              //设置--声音与振动
    {FUNC_SET_SUB_LANGUAGE,             func_set_sub_language},         //设置--语言
    {FUNC_SET_SUB_TIME,                 func_set_sub_time},             //设置--时间
    {FUNC_SET_SUB_MENU_NAVIGATION,      func_set_sub_menu_navigation},  //设置--菜单导航样式
    {FUNC_TIME_SUB_CUSTOM,              func_time_sub_custom},          //设置--自定义时间
    {FUNC_SET_SUB_PASSWORD,             func_set_sub_password},         //设置--密码锁
    {FUNC_PASSWORD_SUB_DISP,            func_password_sub_disp},        //设置--新密码锁设置
    {FUNC_PASSWORD_SUB_SELECT,          func_password_sub_select},      //设置--密码锁确认
    {FUNC_SET_SUB_ABOUT,                func_set_sub_about},            //设置--关于
    {FUNC_SET_SUB_4G,                   func_set_sub_4g},               //设置--4G
    {FUNC_SET_SUB_RESTART,              func_set_sub_restart},          //设置--重启
    {FUNC_SET_SUB_RSTFY,                func_set_sub_rstfy},            //设置--恢复出厂
    {FUNC_SET_SUB_OFF,                  func_set_sub_off},              //设置--关机
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
    {FUNC_CHARGE,                       func_charge},                   //充电
    {FUNC_DEBUG_INFO,                   func_debug_info},               //DEBUG
    {FUNC_HEAT,                         func_heat},                     //加热页
    {FUNC_HOME,                         func_home},                     //默认主页
    {FUNC_MODE,                         func_mode},                     //模式页
    {FUNC_NEW_HEAT,                     func_new_heat},                 //新主页→加热页
    {FUNC_HOME_PAGE,                    func_home_page},                //新UI主页
    {FUNC_NEW_WARM,                     func_new_warm},                 //新主页→保温页
    {FUNC_LOWBAT,                       func_lowbat},                   //低电模式
    {FUNC_NEW_MODE,                     func_new_mode},                 //新主页→模式页
    {FUNC_NEW_SETUP,                    func_new_setup},                //新主页→设置页
    {FUNC_NEW_LANGUAGE,                 func_new_language},             //新主页→语言页
    {FUNC_NEW_VERINFO,                  func_new_verinfo},              //新主页→版本信息页
    {FUNC_NEW_TIME,                     func_new_timeing},                 //新主页→时间页
    {FUNC_LID_CONFIRM,                  func_lid_confirm},              //上盖开启确认弹窗
#if ELUNCHBOX_PANEL_EN
    {FUNC_RESERVATION,                  func_new_reservation},          //预约页
#else
    {FUNC_RESERVATION,                  func_reservation},              //预约页
#endif
    {FUNC_SETUP,                        func_setup},                    //设置页
    {FUNC_TIMEING,                      func_timeing},                  //定时页
    {FUNC_LANGUAGEING,                  func_languageing},             //语言页
    {FUNC_VERINFO,                      func_verinfo},                  //版本信息页
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_SMARTSTACK,                   func_smartstack},               //智能堆栈
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#if FUNC_BT_EN
    {FUNC_BT,                           func_bt},
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_BT_RING,                      func_bt_ring},
    {FUNC_BT_CALL,                      func_bt_call},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // FUNC_BT_EN
#if FUNC_BT_DUT_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_BT_DUT,                       func_bt_dut},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // FUNC_BT_DUT_EN
#if BT_EMIT_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MUSIC_SRC,                    func_music_src},
    {FUNC_EMIT_LIST,                    func_emit_list},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_MUSIC_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MUSIC,                        func_music},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_FMRX_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_FMRX,                         func_fmrx},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_USBDEV_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_USBDEV,                       func_usbdev},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_RECORDER_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_RECORDER,                     func_recorder},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_IDLE_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_IDLE,                         func_idle},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FOTA_UI_EN
    {FUNC_OTA_UI_MODE,                  func_ota_ui},
#endif
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MODEM_CALL,                   func_modem_call},
    {FUNC_MODEM_RING,                   func_modem_ring},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
    {FUNC_BT_UPDATE,                    func_bt_update},
#if FLASHDB_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MESSAGE_REPLY,                func_message_reply_info},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_BIRD,                         func_bird},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#if FUNC_BLE_GATTS_EN
    {FUNC_BLE_GATTS,                    func_ble_gatts},
#endif
#if FUNC_GAME_TETRIS_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_GAME_TETRIS,                  func_game_tetris},
    {FUNC_GAME_TETRIS_START,            func_game_tetris_start},
    {FUNC_GAME_TETRIS_OVER,             func_game_tetris_over},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // FUNC_GAME_TETRIS_EN

#if VIDEO_PLAY_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_VIDEO_PLAY,                   func_video_play},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // VIDEO_PLAY_EN
#if PHOTO_VIEW_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_PHOTO_VIEW,                   func_photo_view},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // PHOTO_VIEW_EN
#if AVI_DVP_DEMOLIST
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_VIDEO_SHOWLIST,               func_video_showlist},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // AVI_DVP_DEMOLIST
#if VIDEO_RECODE_TAKE_PHOTO_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_VIDEO_RECODE,                 func_video_recode},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
    {FUNC_TAKE_PHOTO,                   func_take_photo},
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_GIF,                          func_gif},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN


};

void func_menu_enter(void);
void func_menustyle_enter(void);
void func_clock_enter(void);
void func_clock_preview_enter(void);
void func_clock_sub_sidebar_enter(void);
void func_clock_sub_card_enter(void);
void func_heartrate_enter(void);
void func_alarm_clock_enter(void);
void func_alarm_clock_sub_set_enter(void);
void func_alarm_clock_sub_repeat_enter(void);
void func_alarm_clock_sub_edit_enter(void);
void func_blood_oxygen_enter(void);
void func_bloodsugar_enter(void);
void func_bloodpressure_enter(void);
void func_breathe_enter(void);
void func_compo_select_enter(void);
void func_compo_select_sub_enter(void);
void func_calculator_enter(void);
void func_camera_enter(void);
void func_light_enter(void);
void func_timer_enter(void);
void func_sleep_enter(void);
void func_stopwatch_enter(void);
void func_stopwatch_sub_record_enter(void);
void func_weather_enter(void);
void func_sport_enter(void);
void func_sport_config_enter(void);
void func_sport_sub_run_enter(void);
void func_sport_switching_enter(void);
void func_game_enter(void);
void func_style_enter(void);
void func_altitude_enter(void);
void func_findphone_enter(void);
void func_map_enter(void);
void func_message_enter(void);
void func_scan_enter(void);
void func_voice_enter(void);
#if SECURITY_PAY_EN
void func_alipay_enter(void);
#endif // SECURITY_PAY_EN
void func_compass_enter(void);
void func_address_book_enter(void);
void func_calendar_enter(void);
void func_call_enter(void);
void func_call_sub_record_enter(void);
void func_call_sub_dial_enter(void);
void func_volume_enter(void);
void func_activity_enter(void);
void func_flashlight_enter(void);
void func_brightness_enter(void);
void func_set_sub_list_enter(void);
void func_set_sub_dousing_enter(void);
void func_set_sub_wrist_enter(void);
void func_set_sub_disturd_enter(void);
void func_disturd_sub_set_enter(void);
void func_set_sub_sav_enter(void);
void func_set_sub_language_enter(void);
void func_set_sub_time_enter(void);
void func_set_sub_menu_navigation_enter(void);
void func_time_sub_custom_enter(void);
void func_set_sub_password_enter(void);
void func_password_sub_disp_enter(void);
void func_password_sub_select_enter(void);
void func_set_sub_about_enter(void);
void func_set_sub_4g_enter(void);
void func_set_sub_restart_enter(void);
void func_set_sub_rstfy_enter(void);
void func_set_sub_off_enter(void);
void func_charge_enter(void);
void func_debug_enter(void);
void func_smartstack_enter(void);
void func_activity_enter(void);
#if FUNC_BT_EN
void func_bt_enter(void);
void func_bt_ring_enter(void);
void func_bt_call_enter(void);
#endif // FUNC_BT_EN
#if BT_EMIT_EN
void func_music_src_enter(void);
void func_emit_list_enter(void);
#endif
#if FUNC_MUSIC_EN
void func_music_enter(void);
#endif
#if FUNC_FMRX_EN
void func_fmrx_enter(void);
#endif
#if FUNC_USBDEV_EN
void func_usbdev_enter(void);
#endif
#if FUNC_RECORDER_EN
void func_recorder_enter(void);
#endif
#if FUNC_IDLE_EN
void func_idle_enter(void);
#endif
#if FOTA_UI_EN
void func_ota_ui_enter(void);
#endif
void func_modem_call_enter(void);
void func_modem_ring_enter(void);
void func_bt_update_enter(void);
#if FLASHDB_EN
void func_message_reply_info_enter(void);
#endif // FLASHDB_EN
void func_bird_enter(void);
#if FUNC_BLE_GATTS_EN
void func_ble_gatts_enter(void);
#endif
#if FUNC_GAME_TETRIS_EN
void func_game_tetris_enter(void);
void func_game_tetris_start_enter(void);
void func_game_tetris_over_enter(void);
#endif // FUNC_GAME_TETRIS_EN
#if VIDEO_PLAY_EN
void func_video_play_enter(void);
#endif // VIDEO_PLAY_EN
#if PHOTO_VIEW_EN
void func_photo_view_enter(void);
#endif // PHOTO_VIEW_EN
#if AVI_DVP_DEMOLIST
void func_video_showlist_enter(void);
#endif // AVI_DVP_DEMOLIST
#if VIDEO_RECODE_TAKE_PHOTO_EN
void func_video_recode_enter(void);
void func_take_photo_enter(void);
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
void func_gif_enter(void);
void func_heat_enter(void);
void func_heat_exit(void);
void func_mode_enter(void);
void func_mode_exit(void);
void func_reservation_enter(void);
void func_reservation_exit(void);
void func_new_reservation_enter(void);
void func_new_reservation_exit(void);
void func_home_page_enter(void);
void func_home_page_exit(void);
void func_new_heat_enter(void);
void func_new_heat_exit(void);
void func_new_warm_enter(void);
void func_lowbat_enter(void);
void func_new_warm_exit(void);
void func_lowbat_exit(void);
void func_new_mode_enter(void);
void func_new_mode_exit(void);
void func_new_mode_pre_leave_cleanup(void);
void func_new_setup_enter(void);
void func_new_setup_exit(void);
void func_new_language_enter(void);
void func_new_language_exit(void);
void func_new_verinfo_enter(void);
void func_new_verinfo_exit(void);
void func_new_timeing_enter(void);
void func_new_timeing_exit(void);
void func_lid_confirm_enter(void);
void func_lid_confirm_exit(void);
void func_setup_enter(void);
void func_setup_exit(void);
void func_timeing_enter(void);
void func_timeing_exit(void);
void func_languageing_enter(void);
void func_languageing_exit(void);
void func_verinfo_enter(void);
void func_verinfo_exit(void);
void func_mode_countdown_set(u8 hour, u8 min);
void func_mode_countdown_start(void);
void func_mode_countdown_stop(void);
void func_mode_temp_set_f(u16 temp_f);
void func_home_enter(void);
void func_heat_countdown_set(u8 hour, u8 min);
void func_heat_countdown_start(void);
void func_heat_countdown_stop(void);
u32 func_heat_countdown_remain_sec(void);
void func_heat_temp_set_f(u16 temp_f);

const func_t tbl_func_enter[] = {
    #if WATCH_FUNC_EN
    {FUNC_MENU,                         func_menu_enter},                     //主菜单(蜂窝)
    {FUNC_MENUSTYLE,                    func_menustyle_enter},                //主菜单样式选择
    {FUNC_CLOCK,                        func_clock_enter},                    //时钟表盘
    {FUNC_CLOCK_PREVIEW,                func_clock_preview_enter},            //时钟表盘预览
    {FUNC_SIDEBAR,                      func_clock_sub_sidebar_enter},        //表盘右滑
    {FUNC_CARD,                         func_clock_sub_card_enter},           //表盘上拉
    {FUNC_HEARTRATE,                    func_heartrate_enter},                //心率
    {FUNC_ALARM_CLOCK,                  func_alarm_clock_enter},              //闹钟
    {FUNC_ALARM_CLOCK_SUB_SET,          func_alarm_clock_sub_set_enter},      //闹钟--设置
    {FUNC_ALARM_CLOCK_SUB_REPEAT,       func_alarm_clock_sub_repeat_enter},   //闹钟--重复
    {FUNC_ALARM_CLOCK_SUB_EDIT,         func_alarm_clock_sub_edit_enter},     //闹钟--编辑
    {FUNC_BLOOD_OXYGEN,                 func_blood_oxygen_enter},             //血氧
    {FUNC_BLOODSUGAR,                   func_bloodsugar_enter},               //血糖
    {FUNC_BLOOD_PRESSURE,               func_bloodpressure_enter},            //血压
    {FUNC_BREATHE,                      func_breathe_enter},                  //呼吸
    #endif // WATCH_FUNC_EN
    {FUNC_COMPO_SELECT,                 func_compo_select_enter},             //组件选择
    {FUNC_COMPO_SELECT_SUB,             func_compo_select_sub_enter},         //组件选择子界面
    #if WATCH_FUNC_EN
    {FUNC_CALCULATOR,                   func_calculator_enter},               //计算器
    {FUNC_CAMERA,                       func_camera_enter},                   //相机
    {FUNC_LIGHT,                        func_light_enter},                    //亮度调节
    {FUNC_TIMER,                        func_timer_enter},                    //定时器
    {FUNC_SLEEP,                        func_sleep_enter},                    //睡眠
    {FUNC_STOPWATCH,                    func_stopwatch_enter},                //秒表
    {FUNC_STOPWATCH_SUB_RECORD,         func_stopwatch_sub_record_enter},     //秒表--秒表记录
    {FUNC_WEATHER,                      func_weather_enter},                  //天气
    {FUNC_SPORT,                        func_sport_enter},                    //运动
    {FUNC_SPORT_CONFIG,                 func_sport_config_enter},             //运动配置
    {FUNC_SPORT_SUB_RUN,                func_sport_sub_run_enter},            //运动--室内跑步
    {FUNC_SPORT_SWITCH,                 func_sport_switching_enter},          //运动开启动画
    {FUNC_GAME,                         func_game_enter},                     //游戏
    {FUNC_STYLE,                        func_style_enter},                    //菜单风格
    {FUNC_ALTITUDE,                     func_altitude_enter},                 //海拔
    {FUNC_FINDPHONE,                    func_findphone_enter},                //寻找手机
    {FUNC_MAP,                          func_map_enter},                      //地图
    {FUNC_MESSAGE,                      func_message_enter},             //消息
    {FUNC_SCAN,                         func_scan_enter},                     //扫一扫
    {FUNC_VOICE,                        func_voice_enter},                    //语音助手
    #endif // WATCH_FUNC_EN
#if SECURITY_PAY_EN
    {FUNC_ALIPAY,                       func_alipay_enter},                   //支付宝
#endif // SECURITY_PAY_EN
    #if WATCH_FUNC_EN
    {FUNC_COMPASS,                      func_compass_enter},                  //指南针
    {FUNC_ADDRESS_BOOK,                 func_address_book_enter},             //电话簿
    {FUNC_CALENDAER,                    func_calendar_enter},                 //日历
    {FUNC_CALL,                         func_call_enter},                     //电话
    {FUNC_CALL_SUB_RECORD,              func_call_sub_record_enter},          //电话--最近通话
    {FUNC_CALL_SUB_DIAL,                func_call_sub_dial_enter},            //电话--拨号
    {FUNC_VOLUME,                       func_volume_enter},                   //音量调节
    {FUNC_ACTIVITY,                     func_activity_enter},                 //活动记录
    {FUNC_FLASHLIGHT,                   func_flashlight_enter},               //手电筒
    {FUNC_BRIGHTNESS,                   func_brightness_enter},               //亮度（全屏图标）
    {FUNC_SETTING,                      func_set_sub_list_enter},             //设置
    {FUNC_SET_SUB_DOUSING,              func_set_sub_dousing_enter},          //设置--熄屏
    {FUNC_SET_SUB_WRIST,                func_set_sub_wrist_enter},            //设置--抬腕
    {FUNC_SET_SUB_DISTURD,              func_set_sub_disturd_enter},          //设置--勿扰
    {FUNC_DISTURD_SUB_SET,              func_disturd_sub_set_enter},          //勿扰--时间设置
    {FUNC_SET_SUB_SAV,                  func_set_sub_sav_enter},              //设置--声音与振动
    {FUNC_SET_SUB_LANGUAGE,             func_set_sub_language_enter},         //设置--语言
    {FUNC_SET_SUB_TIME,                 func_set_sub_time_enter},             //设置--时间
    {FUNC_SET_SUB_MENU_NAVIGATION,      func_set_sub_menu_navigation_enter},  //设置--菜单导航样式
    {FUNC_TIME_SUB_CUSTOM,              func_time_sub_custom_enter},          //设置--自定义时间
    {FUNC_SET_SUB_PASSWORD,             func_set_sub_password_enter},         //设置--密码锁
    {FUNC_PASSWORD_SUB_DISP,            func_password_sub_disp_enter},        //设置--新密码锁设置
    {FUNC_PASSWORD_SUB_SELECT,          func_password_sub_select_enter},      //设置--密码锁确认
    {FUNC_SET_SUB_ABOUT,                func_set_sub_about_enter},            //设置--关于
    {FUNC_SET_SUB_4G,                   func_set_sub_4g_enter},               //设置--4G
    {FUNC_SET_SUB_RESTART,              func_set_sub_restart_enter},          //设置--重启
    {FUNC_SET_SUB_RSTFY,                func_set_sub_rstfy_enter},            //设置--恢复出厂
    {FUNC_SET_SUB_OFF,                  func_set_sub_off_enter},              //设置--关机
    #endif // WATCH_FUNC_EN
    {FUNC_CHARGE,                       func_charge_enter},                   //充电
    {FUNC_DEBUG_INFO,                   func_debug_enter},               //DEBUG
    {FUNC_HEAT,                         func_heat_enter},                //加热页
    {FUNC_HOME,                         func_home_enter},               //默认主页
    {FUNC_MODE,                         func_mode_enter},               //模式页
    {FUNC_NEW_HEAT,                     func_new_heat_enter},           //新主页→加热页
    {FUNC_HOME_PAGE,                    func_home_page_enter},          //新UI主页
    {FUNC_NEW_WARM,                     func_new_warm_enter},           //新主页→保温页
    {FUNC_LOWBAT,                       func_lowbat_enter},             //低电模式
    {FUNC_NEW_MODE,                     func_new_mode_enter},           //新主页→模式页
    {FUNC_NEW_SETUP,                    func_new_setup_enter},          //新主页→设置页
    {FUNC_NEW_LANGUAGE,                 func_new_language_enter},       //新主页→语言页
    {FUNC_NEW_VERINFO,                  func_new_verinfo_enter},        //新主页→版本信息页
    {FUNC_NEW_TIME,                     func_new_timeing_enter},           //新主页→时间页
    {FUNC_LID_CONFIRM,                  func_lid_confirm_enter},        //上盖开启确认弹窗
#if ELUNCHBOX_PANEL_EN
    {FUNC_RESERVATION,                  func_new_reservation_enter},    //预约页
#else
    {FUNC_RESERVATION,                  func_reservation_enter},        //预约页
#endif
    {FUNC_SETUP,                        func_setup_enter},              //设置页
    {FUNC_TIMEING,                      func_timeing_enter},            //定时页
    {FUNC_LANGUAGEING,                  func_languageing_enter},       //语言页
    {FUNC_VERINFO,                      func_verinfo_enter},            //版本信息页
    #if WATCH_FUNC_EN
    {FUNC_SMARTSTACK,                   func_smartstack_enter},               //智能堆栈
    #endif // WATCH_FUNC_EN
#if FUNC_BT_EN
    {FUNC_BT,                           func_bt_enter},
    #if WATCH_FUNC_EN
    {FUNC_BT_RING,                      func_bt_ring_enter},
    {FUNC_BT_CALL,                      func_bt_call_enter},
    #endif // WATCH_FUNC_EN
#endif // FUNC_BT_EN
#if FUNC_BT_DUT_EN
    #if WATCH_FUNC_EN
    {FUNC_BT_DUT,                       NULL},
    #endif // WATCH_FUNC_EN
#endif // FUNC_BT_DUT_EN
#if BT_EMIT_EN
    #if WATCH_FUNC_EN
    {FUNC_MUSIC_SRC,                    func_music_src_enter},
    {FUNC_EMIT_LIST,                    func_emit_list_enter},
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_MUSIC_EN
    #if WATCH_FUNC_EN
    {FUNC_MUSIC,                        func_music_enter},
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_FMRX_EN
    #if WATCH_FUNC_EN
    {FUNC_FMRX,                         func_fmrx_enter},
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_USBDEV_EN
    #if WATCH_FUNC_EN
    {FUNC_USBDEV,                       func_usbdev_enter},
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_RECORDER_EN
    #if WATCH_FUNC_EN
    {FUNC_RECORDER,                     func_recorder_enter},
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_IDLE_EN
    #if WATCH_FUNC_EN
    {FUNC_IDLE,                         func_idle_enter},
    #endif // WATCH_FUNC_EN
#endif
#if FOTA_UI_EN
    {FUNC_OTA_UI_MODE,                  func_ota_ui_enter},
#endif
    #if WATCH_FUNC_EN
    {FUNC_MODEM_CALL,                   func_modem_call_enter},
    {FUNC_MODEM_RING,                   func_modem_ring_enter},
    #endif // WATCH_FUNC_EN
    {FUNC_BT_UPDATE,                    func_bt_update_enter},
#if FLASHDB_EN
    #if WATCH_FUNC_EN
    {FUNC_MESSAGE_REPLY,                func_message_reply_info_enter},
    #endif // WATCH_FUNC_EN
#endif
    #if WATCH_FUNC_EN
    {FUNC_BIRD,                         func_bird_enter},
    #endif // WATCH_FUNC_EN
#if FUNC_BLE_GATTS_EN
    {FUNC_BLE_GATTS,                    func_ble_gatts_enter},
#endif
#if FUNC_GAME_TETRIS_EN
    #if WATCH_FUNC_EN
    {FUNC_GAME_TETRIS,                  func_game_tetris_enter},
    {FUNC_GAME_TETRIS_START,            func_game_tetris_start_enter},
    {FUNC_GAME_TETRIS_OVER,             func_game_tetris_over_enter},
    #endif // WATCH_FUNC_EN
#endif // FUNC_GAME_TETRIS_EN

#if VIDEO_PLAY_EN
    #if WATCH_FUNC_EN
    {FUNC_VIDEO_PLAY,                   func_video_play_enter},
    #endif // WATCH_FUNC_EN
#endif // VIDEO_PLAY_EN
#if PHOTO_VIEW_EN
    #if WATCH_FUNC_EN
    {FUNC_PHOTO_VIEW,                   func_photo_view_enter},
    #endif // WATCH_FUNC_EN
#endif // PHOTO_VIEW_EN
#if AVI_DVP_DEMOLIST
    #if WATCH_FUNC_EN
    {FUNC_VIDEO_SHOWLIST,               func_video_showlist_enter},
    #endif // WATCH_FUNC_EN
#endif // AVI_DVP_DEMOLIST
#if VIDEO_RECODE_TAKE_PHOTO_EN
    #if WATCH_FUNC_EN
    {FUNC_VIDEO_RECODE,                 func_video_recode_enter},
    #endif // WATCH_FUNC_EN
    {FUNC_TAKE_PHOTO,                   func_take_photo_enter},
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
    #if WATCH_FUNC_EN
    {FUNC_GIF,                          func_gif_enter},
    #endif // WATCH_FUNC_EN
};


void func_menu_exit(void);
void func_menustyle_exit(void);
void func_clock_exit(void);
void func_clock_preview_exit(void);
void func_clock_sub_sidebar_exit(void);
void func_clock_sub_card_exit(void);
void func_heartrate_exit(void);
void func_alarm_clock_exit(void);
void func_alarm_clock_sub_set_exit(void);
void func_alarm_clock_sub_repeat_exit(void);
void func_alarm_clock_sub_edit_exit(void);
void func_blood_oxygen_exit(void);
void func_bloodsugar_exit(void);
void func_bloodpressure_exit(void);
void func_breathe_exit(void);
void func_compo_select_exit(void);
void func_compo_select_sub_exit(void);
void func_calculator_exit(void);
void func_camera_exit(void);
void func_light_exit(void);
void func_timer_exit(void);
void func_sleep_exit(void);
void func_stopwatch_exit(void);
void func_stopwatch_sub_record_exit(void);
void func_weather_exit(void);
void func_sport_exit(void);
void func_sport_config_exit(void);
void func_sport_sub_run_exit(void);
void func_sport_switching_exit(void);
void func_game_exit(void);
void func_style_exit(void);
void func_altitude_exit(void);
void func_findphone_exit(void);
void func_map_exit(void);
void func_message_exit(void);
void func_scan_exit(void);
void func_voice_exit(void);
#if SECURITY_PAY_EN
void func_alipay_exit(void);
#endif // SECURITY_PAY_EN
void func_compass_exit(void);
void func_address_book_exit(void);
void func_calendar_exit(void);
void func_call_exit(void);
void func_call_sub_record_exit(void);
void func_call_sub_dial_exit(void);
void func_volume_exit(void);
void func_activity_exit(void);
void func_flashlight_exit(void);
void func_brightness_exit(void);
void func_set_sub_exit(void);
void func_set_sub_dousing_exit(void);
void func_set_sub_wrist_exit(void);
void func_set_sub_disturd_exit(void);
void func_disturd_sub_set_exit(void);
void func_set_sub_sav_exit(void);
void func_set_sub_language_exit(void);
void func_set_sub_time_exit(void);
void func_set_sub_menu_navigation_exit(void);
void func_time_sub_custom_exit(void);
void func_set_sub_password_exit(void);
void func_password_sub_disp_exit(void);
void func_password_sub_select_exit(void);
void func_set_sub_about_exit(void);
void func_set_sub_4g_exit(void);
void func_set_sub_restart_exit(void);
void func_set_sub_rstfy_exit(void);
void func_set_sub_off_exit(void);
void func_charge_exit(void);
void func_debug_info_exit(void);
void func_heat_exit(void);
void func_home_exit(void);
void func_smartstack_exit(void);
#if FUNC_BT_EN
void func_bt_exit(void);
void func_bt_ring_exit(void);
void func_bt_call_exit(void);
#endif // FUNC_BT_EN
#if BT_EMIT_EN
void func_music_src_exit(void);
void func_emit_list_exit(void);
#endif
#if FUNC_MUSIC_EN
void func_music_exit(void);
#endif
#if FUNC_FMRX_EN
void func_fmrx_exit(void);
#endif
#if FUNC_USBDEV_EN
void func_usbdev_exit(void);
#endif
#if FUNC_RECORDER_EN
void func_recorder_exit(void);
#endif
#if FUNC_IDLE_EN
void func_idle_exit(void);
#endif
#if FOTA_UI_EN
void func_ota_ui_exit(void);
#endif
void func_modem_call_exit(void);
void func_modem_ring_exit(void);
void func_bt_update_exit(void);
void func_bird_exit(void);
#if FUNC_BLE_GATTS_EN
void func_ble_gatts_exit(void);
#endif
#if FUNC_GAME_TETRIS_EN
void func_game_tetris_exit(void);
void func_game_tetris_start_exit(void);
void func_game_tetris_over_exit(void);
#endif // FUNC_GAME_TETRIS_EN
#if VIDEO_PLAY_EN
void func_video_play_exit(void);
#endif // VIDEO_PLAY_EN
#if PHOTO_VIEW_EN
void func_photo_view_exit(void);
#endif // PHOTO_VIEW_EN
#if AVI_DVP_DEMOLIST
void func_video_showlist_exit(void);
#endif // AVI_DVP_DEMOLIST
#if VIDEO_RECODE_TAKE_PHOTO_EN
void func_video_recode_exit(void);
void func_take_photo_exit(void);
#endif // VIDEO_RECODE_TAKE_PHOTO_EN

void func_gif_exit(void);

const func_t tbl_func_exit[] = {
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MENU,                         func_menu_exit},                     //主菜单(蜂窝)
    {FUNC_MENUSTYLE,                    func_menustyle_exit},                //主菜单样式选择
    {FUNC_CLOCK,                        func_clock_exit},                    //时钟表盘
    {FUNC_CLOCK_PREVIEW,                func_clock_preview_exit},            //时钟表盘预览
    {FUNC_SIDEBAR,                      func_clock_sub_sidebar_exit},        //表盘右滑
    {FUNC_CARD,                         func_clock_sub_card_exit},           //表盘上拉
    {FUNC_HEARTRATE,                    func_heartrate_exit},                //心率
    {FUNC_ALARM_CLOCK,                  func_alarm_clock_exit},              //闹钟
    {FUNC_ALARM_CLOCK_SUB_SET,          func_alarm_clock_sub_set_exit},      //闹钟--设置
    {FUNC_ALARM_CLOCK_SUB_REPEAT,       func_alarm_clock_sub_repeat_exit},   //闹钟--重复
    {FUNC_ALARM_CLOCK_SUB_EDIT,         func_alarm_clock_sub_edit_exit},     //闹钟--编辑
    {FUNC_BLOOD_OXYGEN,                 func_blood_oxygen_exit},             //血氧
    {FUNC_BLOODSUGAR,                   func_bloodsugar_exit},               //血糖
    {FUNC_BLOOD_PRESSURE,               func_bloodpressure_exit},            //血压
    {FUNC_BREATHE,                      func_breathe_exit},                  //呼吸
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
    {FUNC_COMPO_SELECT,                 func_compo_select_exit},             //组件选择
    {FUNC_COMPO_SELECT_SUB,             func_compo_select_sub_exit},         //组件选择子界面
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_CALCULATOR,                   func_calculator_exit},               //计算器
    {FUNC_CAMERA,                       func_camera_exit},                   //相机
    {FUNC_LIGHT,                        func_light_exit},                    //亮度调节
    {FUNC_TIMER,                        func_timer_exit},                    //定时器
    {FUNC_SLEEP,                        func_sleep_exit},                    //睡眠
    {FUNC_STOPWATCH,                    func_stopwatch_exit},                //秒表
    {FUNC_STOPWATCH_SUB_RECORD,         func_stopwatch_sub_record_exit},     //秒表--秒表记录
    {FUNC_WEATHER,                      func_weather_exit},                  //天气
    {FUNC_SPORT,                        func_sport_exit},                    //运动
    {FUNC_SPORT_CONFIG,                 func_sport_config_exit},             //运动配置
    {FUNC_SPORT_SUB_RUN,                func_sport_sub_run_exit},            //运动--室内跑步
    {FUNC_SPORT_SWITCH,                 func_sport_switching_exit},          //运动开启动画
    {FUNC_GAME,                         func_game_exit},                     //游戏
    {FUNC_STYLE,                        func_style_exit},                    //菜单风格
    {FUNC_ALTITUDE,                     func_altitude_exit},                 //海拔
    {FUNC_FINDPHONE,                    func_findphone_exit},                //寻找手机
    {FUNC_MAP,                          func_map_exit},                      //地图
    {FUNC_MESSAGE,                      func_message_exit},             //消息
    {FUNC_SCAN,                         func_scan_exit},                     //扫一扫
    {FUNC_VOICE,                        func_voice_exit},                    //语音助手
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#if SECURITY_PAY_EN
    {FUNC_ALIPAY,                       func_alipay_exit},                   //支付宝
#endif // SECURITY_PAY_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_COMPASS,                      func_compass_exit},                  //指南针
    {FUNC_ADDRESS_BOOK,                 func_address_book_exit},             //电话簿
    {FUNC_CALENDAER,                    func_calendar_exit},                 //日历
    {FUNC_CALL,                         func_call_exit},                     //电话
    {FUNC_CALL_SUB_RECORD,              func_call_sub_record_exit},          //电话--最近通话
    {FUNC_CALL_SUB_DIAL,                func_call_sub_dial_exit},            //电话--拨号
    {FUNC_VOLUME,                       func_volume_exit},                   //音量调节
    {FUNC_ACTIVITY,                     func_activity_exit},                 //活动记录
    {FUNC_FLASHLIGHT,                   func_flashlight_exit},               //手电筒
    {FUNC_BRIGHTNESS,                   func_brightness_exit},               //亮度（全屏图标）
    {FUNC_SETTING,                      func_set_sub_exit},             //设置
    {FUNC_SET_SUB_DOUSING,              func_set_sub_dousing_exit},          //设置--熄屏
    {FUNC_SET_SUB_WRIST,                func_set_sub_wrist_exit},            //设置--抬腕
    {FUNC_SET_SUB_DISTURD,              func_set_sub_disturd_exit},          //设置--勿扰
    {FUNC_DISTURD_SUB_SET,              func_disturd_sub_set_exit},          //勿扰--时间设置
    {FUNC_SET_SUB_SAV,                  func_set_sub_sav_exit},              //设置--声音与振动
    {FUNC_SET_SUB_LANGUAGE,             func_set_sub_language_exit},         //设置--语言
    {FUNC_SET_SUB_TIME,                 func_set_sub_time_exit},             //设置--时间
    {FUNC_SET_SUB_MENU_NAVIGATION,      func_set_sub_menu_navigation_exit},  //设置--菜单导航样式
    {FUNC_TIME_SUB_CUSTOM,              func_time_sub_custom_exit},          //设置--自定义时间
    {FUNC_SET_SUB_PASSWORD,             func_set_sub_password_exit},         //设置--密码锁
    {FUNC_PASSWORD_SUB_DISP,            func_password_sub_disp_exit},        //设置--新密码锁设置
    {FUNC_PASSWORD_SUB_SELECT,          func_password_sub_select_exit},      //设置--密码锁确认
    {FUNC_SET_SUB_ABOUT,                func_set_sub_about_exit},            //设置--关于
    {FUNC_SET_SUB_4G,                   func_set_sub_4g_exit},               //设置--4G
    {FUNC_SET_SUB_RESTART,              func_set_sub_restart_exit},          //设置--重启
    {FUNC_SET_SUB_RSTFY,                func_set_sub_rstfy_exit},            //设置--恢复出厂
    {FUNC_SET_SUB_OFF,                  func_set_sub_off_exit},              //设置--关机
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
    {FUNC_CHARGE,                       func_charge_exit},                   //充电
    {FUNC_DEBUG_INFO,                   func_debug_info_exit},               //DEBUG
    {FUNC_HEAT,                         func_heat_exit},                    //加热页
    {FUNC_HOME,                         func_home_exit},                    //默认主页
    {FUNC_MODE,                         func_mode_exit},                    //模式页
    {FUNC_NEW_HEAT,                     func_new_heat_exit},                //新主页→加热页
    {FUNC_HOME_PAGE,                    func_home_page_exit},               //新UI主页
    {FUNC_NEW_WARM,                     func_new_warm_exit},                //新主页→保温页
    {FUNC_LOWBAT,                       func_lowbat_exit},                  //低电模式
    {FUNC_NEW_MODE,                     func_new_mode_exit},                //新主页→模式页
    {FUNC_NEW_SETUP,                    func_new_setup_exit},               //新主页→设置页
    {FUNC_NEW_LANGUAGE,                 func_new_language_exit},            //新主页→语言页
    {FUNC_NEW_VERINFO,                  func_new_verinfo_exit},             //新主页→版本信息页
    {FUNC_NEW_TIME,                     func_new_timeing_exit},                //新主页→时间页
    {FUNC_LID_CONFIRM,                  func_lid_confirm_exit},             //上盖开启确认弹窗
#if ELUNCHBOX_PANEL_EN
    {FUNC_RESERVATION,                  func_new_reservation_exit},         //预约页
#else
    {FUNC_RESERVATION,                  func_reservation_exit},             //预约页
#endif
    {FUNC_SETUP,                        func_setup_exit},                   //设置页
    {FUNC_TIMEING,                      func_timeing_exit},                 //定时页
    {FUNC_LANGUAGEING,                  func_languageing_exit},         //语言页
    {FUNC_VERINFO,                      func_verinfo_exit},                 //版本信息页
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_SMARTSTACK,                   func_smartstack_exit},               //智能堆栈
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#if FUNC_BT_EN
    {FUNC_BT,                           func_bt_exit},
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_BT_RING,                      func_bt_ring_exit},
    {FUNC_BT_CALL,                      func_bt_call_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // FUNC_BT_EN
#if FUNC_BT_DUT_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_BT_DUT,                       NULL},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // FUNC_BT_DUT_EN
#if BT_EMIT_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MUSIC_SRC,                    func_music_src_exit},
    {FUNC_EMIT_LIST,                    func_emit_list_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_MUSIC_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MUSIC,                        func_music_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_FMRX_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_FMRX,                         func_fmrx_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_USBDEV_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_USBDEV,                       func_usbdev_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_RECORDER_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_RECORDER,                     func_recorder_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FUNC_IDLE_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_IDLE,                         func_idle_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
#if FOTA_UI_EN
    {FUNC_OTA_UI_MODE,                  func_ota_ui_exit},
#endif
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MODEM_CALL,                   func_modem_call_exit},
    {FUNC_MODEM_RING,                   func_modem_ring_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
    {FUNC_BT_UPDATE,                    func_bt_update_exit},
#if FLASHDB_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_MESSAGE_REPLY,                NULL},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_BIRD,                         func_bird_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#if FUNC_BLE_GATTS_EN
    {FUNC_BLE_GATTS,                    func_ble_gatts_exit},
#endif
#if FUNC_GAME_TETRIS_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_GAME_TETRIS,                  func_game_tetris_exit},
    {FUNC_GAME_TETRIS_START,            func_game_tetris_start_exit},
    {FUNC_GAME_TETRIS_OVER,             func_game_tetris_over_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // FUNC_GAME_TETRIS_EN

#if VIDEO_PLAY_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_VIDEO_PLAY,                   func_video_play_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // VIDEO_PLAY_EN
#if PHOTO_VIEW_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_PHOTO_VIEW,                   func_photo_view_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // PHOTO_VIEW_EN
#if AVI_DVP_DEMOLIST
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_VIDEO_SHOWLIST,               func_video_showlist_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
#endif // AVI_DVP_DEMOLIST
#if VIDEO_RECODE_TAKE_PHOTO_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_VIDEO_RECODE,                 func_video_recode_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
    {FUNC_TAKE_PHOTO,                   func_take_photo_exit},
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
    #if WATCH_FUNC_EN
    #if WATCH_FUNC_EN
    {FUNC_GIF,                          func_gif_exit},
    #endif // WATCH_FUNC_EN
    #endif // WATCH_FUNC_EN
};

#endif // _FUNC_H
