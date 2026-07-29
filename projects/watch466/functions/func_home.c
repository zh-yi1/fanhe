#include "include.h"
#include "func.h"

#if USER_PANEL_LED
#include "port_panel_led.h"
#endif

#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif

/*
 * home bin 已移除（Output/bin/ui/home 下的 bin 及 home_ui_* / home_top_time /
 * home_tab_label / home_icon_res 支持文件已删）。本文件仅保留按键/Tab/跳转
 * 逻辑骨架；图标 / UI_BUF_HOME_* / set_ram 相关代码已去掉，用系统字体文字代替。
 */

#define HOME_DBG(...)           printf(__VA_ARGS__)

enum {
    HOME_TAB_HEAT = 0,
    HOME_TAB_MODE,
    HOME_TAB_SETUP,
    HOME_TAB_CNT,
};

enum {
    COMPO_ID_TXT_TITLE = 1,
    COMPO_ID_TXT_CLOCK,
    COMPO_ID_TXT_TAB,
    COMPO_ID_TXT_RES_MARQUEE,
};

typedef struct f_home_t_ {
    u8 tab;
    u8 last_top_min;
    u8 last_top_sec;
    u16 last_cd_total_min;
    compo_textbox_t *txt_title;
    compo_textbox_t *txt_clock;
    compo_textbox_t *txt_tab;
    compo_textbox_t *txt_res_marquee;
} f_home_t;

static u32 home_countdown_remain_sec;
static bool home_countdown_running;
static bool home_countdown_inited;

static const char * const tbl_home_tab_label[HOME_TAB_CNT] = {
    "HEAT",
    "MODE",
    "SETUP",
};

#define HOME_PT8028_KEY_DEBOUNCE_MS     50

#if ELUNCHBOX_PANEL_EN
static u8 home_gui_dirty = 1;

void func_home_gui_mark_dirty(void)
{
    home_gui_dirty = 1;
}

bool func_home_gui_need_refresh(void)
{
    if (!home_gui_dirty) {
        return false;
    }
    home_gui_dirty = 0;
    return true;
}

static void func_home_draw_now(void)
{
    os_gui_draw_force();
    home_gpu_wait_idle();
}
#endif

static void func_home_countdown_set(u8 hour, u8 min)
{
    home_countdown_remain_sec = (u32)hour * 3600 + (u32)min * 60;
}

static void func_home_countdown_start(void)
{
    home_countdown_running = true;
}

static void func_home_countdown_get_display(u8 *hour, u8 *min)
{
    u32 sec = home_countdown_remain_sec;

    if (hour != NULL) {
        *hour = (u8)(sec / 3600);
    }
    if (min != NULL) {
        *min = (u8)((sec % 3600) / 60);
    }
}

static void func_home_countdown_tick(void)
{
    if (home_countdown_running && home_countdown_remain_sec > 0) {
        home_countdown_remain_sec--;
    }
}

static void func_home_countdown_adjust_min(f_home_t *f_home, s8 delta)
{
    u32 total_min;
    u8 hour, min;

    if (f_home == NULL) {
        return;
    }

    total_min = home_countdown_remain_sec / 60;
    if (delta < 0 && total_min < (u32)(-delta)) {
        total_min = 0;
    } else {
        total_min = (u32)((s32)total_min + delta);
    }
    if (total_min > 99 * 60 + 59) {
        total_min = 99 * 60 + 59;
    }

    home_countdown_remain_sec = total_min * 60;
    func_home_countdown_get_display(&hour, &min);
    if (f_home->txt_clock != NULL) {
        char buf[16];

        snprintf(buf, sizeof(buf), "%02u:%02u", hour, min);
        compo_textbox_set(f_home->txt_clock, buf);
    }
    f_home->last_cd_total_min = (u16)total_min;
#if ELUNCHBOX_PANEL_EN
    func_home_gui_mark_dirty();
    func_home_draw_now();
#endif
}

static void func_home_tab_select(f_home_t *f_home, u8 tab)
{
    if (f_home == NULL || tab >= HOME_TAB_CNT) {
        return;
    }
    f_home->tab = tab;
    if (f_home->txt_tab != NULL) {
        char buf[32];

        snprintf(buf, sizeof(buf), "Tab: %s", tbl_home_tab_label[tab]);
        compo_textbox_set(f_home->txt_tab, buf);
    }
#if ELUNCHBOX_PANEL_EN
    func_home_gui_mark_dirty();
#endif
}

static void func_home_mode_key(void)
{
    f_home_t *f_home = (f_home_t *)func_cb.f_cb;

    if (f_home == NULL) {
        return;
    }
    func_home_tab_select(f_home, (u8)((f_home->tab + 1) % HOME_TAB_CNT));
#if ELUNCHBOX_PANEL_EN
    func_home_draw_now();
#endif
}

static void func_home_tab_enter(f_home_t *f_home)
{
    if (f_home == NULL) {
        return;
    }

    home_gpu_wait_idle();

    switch (f_home->tab) {
    case HOME_TAB_HEAT:
        HOME_DBG("func_home_tab_enter: HOME_TAB_HEAT\n");
        func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    case HOME_TAB_MODE:
        HOME_DBG("func_home_tab_enter: HOME_TAB_MODE\n");
        func_switch_to(FUNC_MODE, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    case HOME_TAB_SETUP:
        HOME_DBG("func_home_tab_enter: HOME_TAB_SETUP\n");
        func_switch_to(FUNC_SETUP, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    default:
        break;
    }
}

static void func_home_res_marquee_refresh(f_home_t *f_home)
{
    if (f_home->txt_res_marquee == NULL) {
        return;
    }

    /*
     * 预约页已裁剪, 跑马灯暂无文本来源, 保持隐藏。页面重建后恢复:
     *   char buf[48];
     *   if (!func_reservation_is_waiting()) { 隐藏; return; }
     *   func_reservation_marquee_text(buf, sizeof(buf));
     *   compo_textbox_set(f_home->txt_res_marquee, buf);
     *   compo_textbox_set_visible(f_home->txt_res_marquee, true);
     */
    compo_textbox_set_visible(f_home->txt_res_marquee, false);
#if ELUNCHBOX_PANEL_EN
    func_home_gui_mark_dirty();
#endif
}

static void func_home_clock_update(f_home_t *f_home, u8 hour, u8 min)
{
    char buf[16];

    if (f_home == NULL || f_home->txt_clock == NULL) {
        return;
    }
    snprintf(buf, sizeof(buf), "%02u:%02u", hour, min);
    compo_textbox_set(f_home->txt_clock, buf);
}

static void func_home_status_refresh(f_home_t *f_home)
{
    if (func_cb.sta != FUNC_HOME || sys_cb.flag_swithing) {
        return;
    }

    tm_t tm = rtc_clock_get();

    if (f_home->last_top_min != tm.min || f_home->last_top_sec != tm.sec) {
        f_home->last_top_min = tm.min;
        f_home->last_top_sec = tm.sec;
        func_home_countdown_tick();
        func_home_res_marquee_refresh(f_home);
#if ELUNCHBOX_PANEL_EN
        func_home_gui_mark_dirty();
#endif
    }

    {
        u8 cd_hour, cd_min;
        u16 cd_total_min;

        func_home_countdown_get_display(&cd_hour, &cd_min);
        cd_total_min = (u16)cd_hour * 60 + cd_min;
        if (f_home->last_cd_total_min != cd_total_min) {
            f_home->last_cd_total_min = cd_total_min;
            func_home_clock_update(f_home, cd_hour, cd_min);
        }
    }
}

compo_form_t *func_home_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    txt = compo_textbox_create(frm, 32);
    compo_setid(txt, COMPO_ID_TXT_TITLE);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 30);
    compo_textbox_set(txt, "HOME (no bin)");

    txt = compo_textbox_create(frm, 16);
    compo_setid(txt, COMPO_ID_TXT_CLOCK);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 10);
    compo_textbox_set(txt, "00:00");

    txt = compo_textbox_create(frm, 32);
    compo_setid(txt, COMPO_ID_TXT_TAB);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 30);
    compo_textbox_set(txt, "Tab: HEAT");

    txt = compo_textbox_create(frm, 48);
    compo_setid(txt, COMPO_ID_TXT_RES_MARQUEE);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 55);
    compo_textbox_set_autoroll(txt, true);
    compo_textbox_set_autoroll_mode(txt, TEXT_AUTOROLL_MODE_SROLL_CIRC);
    compo_textbox_set_visible(txt, false);

    return frm;
}

#if ELUNCHBOX_PANEL_EN
u8 func_res_allow_switch;
#endif

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
static void func_home_drain_stale_key_msgs(void)
{
    msg_queue_detach(KU_NEXT, 0);
    msg_queue_detach(KU_MODE, 0);
    msg_queue_detach(KU_BACK, 0);
    msg_queue_detach(KU_PREV, 0);
    msg_queue_detach(KU_LEFT, 0);
    msg_queue_detach(KU_RIGHT, 0);
    msg_queue_detach(KU_VOL_UP, 0);
    msg_queue_detach(KU_VOL_DOWN, 0);
}

void func_home_switch_to_reservation(void)
{
    func_elunchbox_switch_to_reservation();
}

static void func_home_pt8028_do_confirm(f_home_t *f_home);
static void func_home_pt8028_handle_press(f_home_t *f_home, u8 tch);
static void func_home_pt8028_handle_release(f_home_t *f_home, u8 tch);

static void func_home_pt8028_keys_process(f_home_t *f_home)
{
    u8 act;
    u8 press_tch;
    u8 release_tch;

    if (f_home == NULL) {
        return;
    }

    act = pt8028_take_home_action();
    press_tch = pt8028_take_press_tch();
    release_tch = pt8028_take_release_tch();

    func_home_drain_stale_key_msgs();

    if (act == PT8028_HOME_ACT_CONFIRM) {
        func_home_pt8028_do_confirm(f_home);
    }

    if (press_tch <= PT8028_KEY_TCH6 &&
        press_tch != PT8028_KEY_TCH4) {
        u16 kd = (u16)(tbl_pt8028_bcd_to_key[press_tch] | KEY_SHORT);

        msg_queue_detach(kd, 0);
        func_home_pt8028_handle_press(f_home, press_tch);
    }
    if (release_tch <= PT8028_KEY_TCH6 &&
        release_tch != PT8028_KEY_TCH3 && release_tch != PT8028_KEY_TCH4) {
        u16 ku = (u16)(tbl_pt8028_bcd_to_key[release_tch] | KEY_SHORT_UP);

        msg_queue_detach(ku, 0);
        func_home_pt8028_handle_release(f_home, release_tch);
    }
}

static void func_home_pt8028_do_confirm(f_home_t *f_home)
{
    if (f_home == NULL) {
        return;
    }
    HOME_DBG("Home: 确认 -> 进入 Tab 子页 (tab=%d)\n", f_home->tab);
    func_home_tab_enter(f_home);
}

static void func_home_pt8028_handle_press(f_home_t *f_home, u8 tch)
{
    static u32 last_ms;
    static u8 last_tch = 0xff;

    if (f_home == NULL || tch > PT8028_KEY_TCH6) {
        return;
    }
    if (tch == PT8028_KEY_TCH4) {
        return;
    }
    if (tch == last_tch && !tick_check_expire(last_ms, HOME_PT8028_KEY_DEBOUNCE_MS)) {
        return;
    }
    last_ms = tick_get();
    last_tch = tch;

    switch (tch) {
    case PT8028_KEY_TCH3:
        HOME_DBG("Home: 模式键 -> Tab 切换\n");
        func_home_mode_key();
        break;

    case PT8028_KEY_TCH1:
        HOME_DBG("Home: TCH1 加热键按下\n");
        home_gpu_wait_idle();
        func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    case PT8028_KEY_TCH2:
        HOME_DBG("Home: TCH2 减号按下\n");
        func_home_countdown_adjust_min(f_home, -1);
        break;

    case PT8028_KEY_TCH6:
        HOME_DBG("Home: TCH6 加号按下\n");
        func_home_countdown_adjust_min(f_home, 1);
        break;

    case PT8028_KEY_TCH0:
        HOME_DBG("Home: TCH0 锁键按下\n");
        break;

    case PT8028_KEY_TCH5:
        HOME_DBG("Home: TCH5 开关键按下\n");
        break;

    default:
        break;
    }
}

static void func_home_pt8028_handle_release(f_home_t *f_home, u8 tch)
{
    static u32 last_ms;
    static u8 last_tch = 0xff;

    if (f_home == NULL || tch > PT8028_KEY_TCH6) {
        return;
    }
    if (tch == PT8028_KEY_TCH3 || tch == PT8028_KEY_TCH4) {
        return;
    }
    if (tch == last_tch && !tick_check_expire(last_ms, HOME_PT8028_KEY_DEBOUNCE_MS)) {
        return;
    }
    last_ms = tick_get();
    last_tch = tch;

    (void)tch;
}
#endif

void func_home_process(void)
{
    f_home_t *f_home = (f_home_t *)func_cb.f_cb;

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_gpio_ensure_periodic();
    pt8028_key_scan();
    func_home_pt8028_keys_process(f_home);
#endif

    func_process();

    if (f_home != NULL) {
        func_home_status_refresh(f_home);
    }
}

void func_home_message(size_msg_t msg)
{
    f_home_t *f_home = (f_home_t *)func_cb.f_cb;

    if (msg == NO_MSG) {
        return;
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        break;

    case KU_LEFT:
#if ELUNCHBOX_PANEL_EN
        break;
#else
        HOME_DBG("Home: 锁键\n");
#endif
        break;

    case KU_PREV:
        HOME_DBG("Home: 加热键 -> 跳转加热页\n");
        home_gpu_wait_idle();
        func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    case KU_MODE:
    case K_MODE:
#if ELUNCHBOX_PANEL_EN
        break;
#else
        HOME_DBG("Home msg: 模式键 KU_MODE\n");
        func_home_mode_key();
#endif
        break;

    case KU_BACK:
#if ELUNCHBOX_PANEL_EN
        break;
#else
        HOME_DBG("Home msg: 确认键 KU_BACK\n");
        func_home_tab_enter(f_home);
#endif
        break;

    case KU_RIGHT:
        HOME_DBG("Home: 开关键\n");
        break;

    case KU_NEXT:
        break;

    case KU_VOL_UP:
        HOME_DBG("Home msg: 加号\n");
        home_gpu_wait_idle();
        func_home_countdown_adjust_min(f_home, 1);
        break;

    case KU_VOL_DOWN:
        HOME_DBG("Home msg: 减号\n");
        home_gpu_wait_idle();
        func_home_countdown_adjust_min(f_home, -1);
        break;

    default:
#if ELUNCHBOX_PANEL_EN
        break;
#else
        func_message(msg);
        break;
#endif
    }
}

void func_home_enter(void)
{
    f_home_t *f_home;
    u8 hour, min;

#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif
    func_cb.f_cb = func_zalloc(sizeof(f_home_t));
    func_cb.frm_main = func_home_form_create();

    f_home = (f_home_t *)func_cb.f_cb;
    f_home->tab = HOME_TAB_HEAT;
    f_home->last_top_min = 0xff;
    f_home->last_top_sec = 0xff;
    f_home->last_cd_total_min = 0xffff;
    f_home->txt_title = compo_getobj_byid(COMPO_ID_TXT_TITLE);
    f_home->txt_clock = compo_getobj_byid(COMPO_ID_TXT_CLOCK);
    f_home->txt_tab = compo_getobj_byid(COMPO_ID_TXT_TAB);
    f_home->txt_res_marquee = compo_getobj_byid(COMPO_ID_TXT_RES_MARQUEE);

    os_gui_draw_force();
    home_gpu_wait_idle();
    tft_bglight_force_on();

    if (!home_countdown_inited) {
        func_home_countdown_set(90, 5);
        func_home_countdown_start();
        home_countdown_inited = true;
    }

    func_home_countdown_get_display(&hour, &min);
    func_home_clock_update(f_home, hour, min);
    func_home_tab_select(f_home, HOME_TAB_HEAT);
    func_home_status_refresh(f_home);
    func_home_res_marquee_refresh(f_home);

    os_gui_draw_force();
    home_gpu_wait_idle();
    tft_bglight_force_on();
#if ELUNCHBOX_PANEL_EN
    home_gui_dirty = 0;
    pt8028_release_clear();
#endif
}

void func_home_exit(void)
{
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(0);
    pt8028_release_clear();
#endif
#if USER_PANEL_LED
    panel_led_all_off();
#endif
    func_cb.last = FUNC_HOME;
}

void func_home(void)
{
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    pt8028_set_home_msg_block(1);
    pt8028_release_clear();
#endif
#if !ELUNCHBOX_PANEL_EN
    printf("%s\n", __func__);
#endif
    func_home_enter();
    while (func_cb.sta == FUNC_HOME) {
        func_home_process();
        func_home_message(msg_dequeue());
    }
    func_home_exit();
}
