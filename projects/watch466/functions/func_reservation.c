#include "include.h"
#include "func.h"
#include "func_reservation.h"

/* home bin removed: keep poll/waiting/marquee APIs; UI is text skeleton */

typedef struct {
    bool setup_done;
    reservation_phase_t phase;
    u8 appt_hour;
    u8 appt_min;
    u8 heat_hour;
    u8 heat_min;
    u8 temp_idx;
    u8 last_poll_min;
} reservation_global_t;

static reservation_global_t g_res;

enum {
    COMPO_ID_TXT_TITLE = 1,
    COMPO_ID_TXT_INFO,
};

typedef struct f_reservation_t_ {
    u8 appt_hour;
    u8 appt_min;
    u8 heat_hour;
    u8 heat_min;
    u8 temp_idx;
    u8 focus; /* 0=appt 1=heat 2=done */
    compo_textbox_t *txt_info;
} f_reservation_t;

reservation_phase_t func_reservation_get_phase(void)
{
    return g_res.phase;
}

bool func_reservation_is_waiting(void)
{
    return g_res.setup_done && (g_res.phase == RES_PHASE_WAITING);
}

static u32 func_res_seconds_until_appt(void)
{
    tm_t tm = rtc_clock_get();
    u32 now = (u32)tm.hour * 3600 + (u32)tm.min * 60 + (u32)tm.sec;
    u32 appt = (u32)g_res.appt_hour * 3600 + (u32)g_res.appt_min * 60;

    if (appt <= now) {
        appt += 24 * 3600;
    }
    return appt - now;
}

void func_reservation_marquee_text(char *buf, u16 buf_len)
{
    u32 sec;
    u32 hours;
    u32 mins;

    if (buf == NULL || buf_len == 0) {
        return;
    }

    if (!func_reservation_is_waiting()) {
        buf[0] = '\0';
        return;
    }

    sec = func_res_seconds_until_appt();
    hours = sec / 3600;
    mins = (sec % 3600) / 60;

    if (hours > 0 && mins > 0) {
        snprintf(buf, buf_len, "Starts in %lu hours %lu min",
                 (unsigned long)hours, (unsigned long)mins);
    } else if (hours > 0) {
        snprintf(buf, buf_len, "Starts in %lu hour%s",
                 (unsigned long)hours, (hours > 1) ? "s" : "");
    } else if (mins > 0) {
        snprintf(buf, buf_len, "Starts in %lu min", (unsigned long)mins);
    } else {
        snprintf(buf, buf_len, "Starts soon");
    }
}

static void func_res_info_refresh(f_reservation_t *f_res)
{
    char buf[48];

    if (f_res == NULL || f_res->txt_info == NULL) {
        return;
    }

    snprintf(buf, sizeof(buf), "Appt %02u:%02u Heat %02u:%02u",
             f_res->appt_hour, f_res->appt_min, f_res->heat_hour, f_res->heat_min);
    compo_textbox_set(f_res->txt_info, buf);
}

void func_reservation_force_heating_enter(void)
{
    g_res.phase = RES_PHASE_HEATING;
}

static void func_res_save_and_go_home(f_reservation_t *f_res)
{
    g_res.setup_done = true;
    g_res.phase = RES_PHASE_WAITING;
    g_res.appt_hour = f_res->appt_hour;
    g_res.appt_min = f_res->appt_min;
    g_res.heat_hour = f_res->heat_hour;
    g_res.heat_min = f_res->heat_min;
    g_res.temp_idx = f_res->temp_idx;
    if (sys_cb.flag_swithing) {
        return;
    }
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

void func_reservation_poll(void)
{
    tm_t tm;

#if ELUNCHBOX_PANEL_EN
    if (func_cb.sta == FUNC_HOME) {
        return;
    }
#endif
    if (!g_res.setup_done || g_res.phase != RES_PHASE_WAITING) {
        return;
    }

    tm = rtc_clock_get();
    if (tm.min == g_res.last_poll_min) {
        return;
    }
    g_res.last_poll_min = tm.min;

    if (tm.hour == g_res.appt_hour && tm.min == g_res.appt_min) {
        g_res.phase = RES_PHASE_HEATING;
    }
}

compo_form_t *func_reservation_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    txt = compo_textbox_create(frm, 32);
    compo_setid(txt, COMPO_ID_TXT_TITLE);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 40);
    compo_textbox_set(txt, "RESERVATION");

    txt = compo_textbox_create(frm, 48);
    compo_setid(txt, COMPO_ID_TXT_INFO);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_textbox_set(txt, "no home bin");

    return frm;
}

static void func_reservation_process(void)
{
    func_process();
}

static void func_reservation_message(size_msg_t msg)
{
    f_reservation_t *f_res = (f_reservation_t *)func_cb.f_cb;

    switch (msg) {
    case KU_BACK:
        if (f_res != NULL) {
            func_res_save_and_go_home(f_res);
        }
        break;

    case KU_RIGHT:
        func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        break;

    case KU_VOL_UP:
        if (f_res != NULL) {
            f_res->appt_min = (u8)((f_res->appt_min + 1) % 60);
            func_res_info_refresh(f_res);
        }
        break;

    case KU_VOL_DOWN:
        if (f_res != NULL) {
            f_res->appt_min = (u8)((f_res->appt_min + 59) % 60);
            func_res_info_refresh(f_res);
        }
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_reservation_enter(void)
{
    f_reservation_t *f_res;
    tm_t tm = rtc_clock_get();

    home_gpu_wait_idle();
    func_cb.f_cb = func_zalloc(sizeof(f_reservation_t));
    func_cb.frm_main = func_reservation_form_create();

    f_res = (f_reservation_t *)func_cb.f_cb;
    f_res->appt_hour = tm.hour;
    f_res->appt_min = (u8)((tm.min + 5) % 60);
    f_res->heat_hour = 1;
    f_res->heat_min = 0;
    f_res->temp_idx = 0;
    f_res->focus = 0;
    f_res->txt_info = compo_getobj_byid(COMPO_ID_TXT_INFO);
    func_res_info_refresh(f_res);
    home_gpu_wait_idle();
}

void func_reservation_exit(void)
{
    func_cb.last = FUNC_RESERVATION;
}

void func_reservation(void)
{
    printf("%s\n", __func__);
    func_reservation_enter();
    while (func_cb.sta == FUNC_RESERVATION) {
        func_reservation_process();
        func_reservation_message(msg_dequeue());
    }
    func_reservation_exit();
}
