#include "include.h"
#include "func.h"
#include "func_menu.h"
#include "func_clock.h"
#include "func_music.h"

/*
 * Lunchbox build: stub implementations for removed watch pages / symbols.
 * Entering a stubbed page immediately returns to FUNC_HOME.
 */

static compo_form_t *stub_form_create(void)
{
    return compo_form_create(true);
}

static void stub_go_home(void)
{
    func_cb.sta = FUNC_HOME;
}

/* ---------- framework helpers still referenced by func.c / platform ---------- */

bool func_music_is_play(void)
{
    return false;
}

void func_music_play(bool sta)
{
    (void)sta;
}

bool func_video_allow_warning_tone(void)
{
    return true;
}

u8 func_menu_sub_skyrer_get_first_idx(void)
{
    return 0;
}

compo_form_t *func_clock_form_create_by_screenshoot(void)
{
    return stub_form_create();
}

void func_call_mgr_process(void)
{
}

/* music / recorder / fmrx / usbdev APIs pulled via headers */
void func_music_mp3_res_play(u32 addr, u32 len)
{
    (void)addr;
    (void)len;
}

bool func_music_filter_switch(u8 rec_type)
{
    (void)rec_type;
    return false;
}

void func_music_file_navigation(void)
{
}

void func_music_insert_device(u8 dev)
{
    (void)dev;
}

void func_music_remove_device(u8 dev)
{
    (void)dev;
}

/* clock helpers declared in func_clock.h */
u16 func_clock_time_map_r_get(bool is_sec)
{
    (void)is_sec;
    return 0;
}

void func_clock_time_map_r_set(bool is_sec, u16 value)
{
    (void)is_sec;
    (void)value;
}

void func_clock_sub_process(void)
{
}

void func_clock_sub_message(size_msg_t msg)
{
    (void)msg;
}

void func_clock_sub_dropdown(void)
{
    stub_go_home();
}

void func_clock_sub_pullup(void)
{
    stub_go_home();
}

void func_clock_sub_side(void)
{
    stub_go_home();
}

compo_form_t *func_clock_butterfly_form_create(void)
{
    return stub_form_create();
}

void func_clock_butterfly_set_light_visible(bool visible)
{
    (void)visible;
}

void func_clock_sub_rotary(void)
{
    stub_go_home();
}

u32 func_clock_get_dialplate_cube_idx(void)
{
    return 0;
}

u32 func_clock_get_dialplate_butterfly_idx(void)
{
    return 0;
}

void func_clock_swipe_up_to_football_menu(void)
{
    stub_go_home();
}

/* menu style form creators declared in func_menu.h */
#define STUB_MENU_FORM(name) \
compo_form_t *name(void) { return stub_form_create(); }

STUB_MENU_FORM(func_menu_sub_football_form_create)
STUB_MENU_FORM(func_menu_sub_honeycomb_form_create)
STUB_MENU_FORM(func_menu_sub_waterfall_form_create)
STUB_MENU_FORM(func_menu_sub_list_form_create)
STUB_MENU_FORM(func_menu_sub_sudoku_form_create)
STUB_MENU_FORM(func_menu_sub_grid_form_create)
STUB_MENU_FORM(func_menu_sub_disk_form_create)
STUB_MENU_FORM(func_menu_sub_ring_form_create)
STUB_MENU_FORM(func_menu_sub_kale_form_create)
STUB_MENU_FORM(func_menu_sub_skyrer_form_create)
STUB_MENU_FORM(func_menu_sub_cum_sudoku_form_create)
STUB_MENU_FORM(func_menu_sub_hexagon_form_create)

#define STUB_MENU_ENTRY(name) \
void name(void) { stub_go_home(); }

STUB_MENU_ENTRY(func_menu_sub_football)
STUB_MENU_ENTRY(func_menu_sub_honeycomb)
STUB_MENU_ENTRY(func_menu_sub_waterfall)
STUB_MENU_ENTRY(func_menu_sub_list)
STUB_MENU_ENTRY(func_menu_sub_sudoku)
STUB_MENU_ENTRY(func_menu_sub_grid)
STUB_MENU_ENTRY(func_menu_sub_disk)
STUB_MENU_ENTRY(func_menu_sub_ring)
STUB_MENU_ENTRY(func_menu_sub_kale)
STUB_MENU_ENTRY(func_menu_sub_skyrer)
STUB_MENU_ENTRY(func_menu_sub_cum_sudoku)
STUB_MENU_ENTRY(func_menu_sub_hexagon)

void func_menu_sub_message(size_msg_t msg)
{
    (void)msg;
    func_message(msg);
}

void func_menu_sub_exit(void)
{
}

/* ---------- generic page stub macro ---------- */
#define STUB_PAGE(prefix) \
compo_form_t *func_##prefix##_form_create(void) { return stub_form_create(); } \
void func_##prefix##_enter(void) { func_cb.frm_main = stub_form_create(); } \
void func_##prefix##_exit(void) { } \
void func_##prefix(void) { stub_go_home(); }

STUB_PAGE(menu)
STUB_PAGE(menustyle)
STUB_PAGE(clock)
STUB_PAGE(clock_preview)
STUB_PAGE(clock_sub_sidebar)
STUB_PAGE(clock_sub_card)
STUB_PAGE(heartrate)
STUB_PAGE(compo_select)
STUB_PAGE(compo_select_sub)
STUB_PAGE(alarm_clock)
STUB_PAGE(alarm_clock_sub_set)
STUB_PAGE(alarm_clock_sub_repeat)
STUB_PAGE(alarm_clock_sub_edit)
STUB_PAGE(blood_oxygen)
STUB_PAGE(bloodsugar)
STUB_PAGE(bloodpressure)
STUB_PAGE(breathe)
STUB_PAGE(calculator)
STUB_PAGE(camera)
STUB_PAGE(light)
STUB_PAGE(timer)
STUB_PAGE(sleep)
STUB_PAGE(stopwatch)
STUB_PAGE(stopwatch_sub_record)
STUB_PAGE(weather)
STUB_PAGE(sport)
STUB_PAGE(sport_config)
STUB_PAGE(sport_sub_run)
STUB_PAGE(sport_switching)
STUB_PAGE(game)
STUB_PAGE(style)
STUB_PAGE(findphone)
STUB_PAGE(altitude)
STUB_PAGE(map)
STUB_PAGE(scan)
STUB_PAGE(voice)
STUB_PAGE(compass)
STUB_PAGE(address_book)
STUB_PAGE(call)
STUB_PAGE(call_sub_record)
STUB_PAGE(call_sub_dial)
STUB_PAGE(volume)
STUB_PAGE(activity)
STUB_PAGE(flashlight)
STUB_PAGE(brightness)
STUB_PAGE(smartstack)
STUB_PAGE(bird)
STUB_PAGE(gif)
STUB_PAGE(modem_call)
STUB_PAGE(modem_ring)

#if SECURITY_PAY_EN
STUB_PAGE(alipay)
#endif

#if FUNC_MUSIC_EN
STUB_PAGE(music)
#endif

#if BT_EMIT_EN
STUB_PAGE(music_src)
STUB_PAGE(emit_list)
#endif

#if FUNC_USBDEV_EN
STUB_PAGE(usbdev)
#endif

#if FUNC_RECORDER_EN
STUB_PAGE(recorder)
#endif

#if FUNC_FMRX_EN
STUB_PAGE(fmrx)
#endif

#if FUNC_GAME_TETRIS_EN
STUB_PAGE(game_tetris)
STUB_PAGE(game_tetris_start)
STUB_PAGE(game_tetris_over)
#endif

#if VIDEO_PLAY_EN
STUB_PAGE(video_play)
#endif

#if PHOTO_VIEW_EN
STUB_PAGE(photo_view)
#endif

#if AVI_DVP_DEMOLIST
STUB_PAGE(video_showlist)
#endif

#if VIDEO_RECODE_TAKE_PHOTO_EN
STUB_PAGE(video_recode)
STUB_PAGE(take_photo)
#endif

#if FUNC_BLE_GATTS_EN
/* real implementation kept in func_ble_gatts.c */
#endif

/* setting / password / disturd pages — naming differs from STUB_PAGE */
#define STUB_NAMED(form_fn, enter_fn, exit_fn, loop_fn) \
compo_form_t *form_fn(void) { return stub_form_create(); } \
void enter_fn(void) { func_cb.frm_main = stub_form_create(); } \
void exit_fn(void) { } \
void loop_fn(void) { stub_go_home(); }

STUB_NAMED(func_set_sub_list_form_create, func_set_sub_list_enter, func_set_sub_exit, func_set_sub_list)
STUB_NAMED(func_set_sub_wrist_form_create, func_set_sub_wrist_enter, func_set_sub_wrist_exit, func_set_sub_wrist)
STUB_NAMED(func_set_sub_sav_form_create, func_set_sub_sav_enter, func_set_sub_sav_exit, func_set_sub_sav)
STUB_NAMED(func_set_sub_dousing_form_create, func_set_sub_dousing_enter, func_set_sub_dousing_exit, func_set_sub_dousing)
STUB_NAMED(func_set_sub_language_form_create, func_set_sub_language_enter, func_set_sub_language_exit, func_set_sub_language)
STUB_NAMED(func_set_sub_time_form_create, func_set_sub_time_enter, func_set_sub_time_exit, func_set_sub_time)
STUB_NAMED(func_set_sub_menu_navigation_form_create, func_set_sub_menu_navigation_enter, func_set_sub_menu_navigation_exit, func_set_sub_menu_navigation)
STUB_NAMED(func_time_sub_custom_form_create, func_time_sub_custom_enter, func_time_sub_custom_exit, func_time_sub_custom)
STUB_NAMED(func_set_sub_password_form_create, func_set_sub_password_enter, func_set_sub_password_exit, func_set_sub_password)
STUB_NAMED(func_password_sub_disp_form_create, func_password_sub_disp_enter, func_password_sub_disp_exit, func_password_sub_disp)
STUB_NAMED(func_password_sub_select_form_create, func_password_sub_select_enter, func_password_sub_select_exit, func_password_sub_select)
STUB_NAMED(func_set_sub_4g_form_create, func_set_sub_4g_enter, func_set_sub_4g_exit, func_set_sub_4g)
STUB_NAMED(func_set_sub_about_form_create, func_set_sub_about_enter, func_set_sub_about_exit, func_set_sub_about)
STUB_NAMED(func_set_sub_restart_form_create, func_set_sub_restart_enter, func_set_sub_restart_exit, func_set_sub_restart)
STUB_NAMED(func_set_sub_rstfy_form_create, func_set_sub_rstfy_enter, func_set_sub_rstfy_exit, func_set_sub_rstfy)
STUB_NAMED(func_set_sub_off_form_create, func_set_sub_off_enter, func_set_sub_off_exit, func_set_sub_off)
STUB_NAMED(func_set_sub_disturd_form_create, func_set_sub_disturd_enter, func_set_sub_disturd_exit, func_set_sub_disturd)
STUB_NAMED(func_disturd_sub_set_form_create, func_disturd_sub_set_enter, func_disturd_sub_set_exit, func_disturd_sub_set)
STUB_NAMED(func_calender_form_create, func_calendar_enter, func_calendar_exit, func_calendar)
STUB_NAMED(func_debug_info_form_create, func_debug_enter, func_debug_info_exit, func_debug_info)
STUB_NAMED(func_message_form_create, func_message_enter, func_message_exit, func_message_info)

#if FLASHDB_EN
compo_form_t *func_message_reply_form_create(void)
{
    return stub_form_create();
}

void func_message_reply_info_enter(void)
{
    func_cb.frm_main = stub_form_create();
}

void func_message_reply_info(void)
{
    stub_go_home();
}
#endif

compo_form_t *func_mic_test_form_create(void)
{
    return stub_form_create();
}

void func_mic_test(void)
{
    stub_go_home();
}

void func_aux(void)
{
    stub_go_home();
}

void func_bthid(void)
{
    stub_go_home();
}

/* optional helpers some pages referenced */
u8 func_setting_get_bit(u32 n)
{
    (void)n;
    return 0;
}
