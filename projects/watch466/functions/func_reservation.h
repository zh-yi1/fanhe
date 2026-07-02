#ifndef _FUNC_RESERVATION_H
#define _FUNC_RESERVATION_H

/* 预约 UI 总开关见 config.h FUNC_RESERVATION_UI_EN；0 时 func_switch_to 拒绝进入预约页 */

#include "include.h"

#define FUNC_RES_APPT_HOUR_MAX          23

typedef enum {
    RES_PHASE_NONE = 0,
    RES_PHASE_WAITING,
    RES_PHASE_HEATING,
    RES_PHASE_FINISHED,
} reservation_phase_t;

void func_reservation_poll(void);
reservation_phase_t func_reservation_get_phase(void);
bool func_reservation_is_waiting(void);
bool func_reservation_is_active(void);
bool func_reservation_is_heating(void);
void func_reservation_marquee_text(char *buf, u16 buf_len);
void func_reservation_force_heating_enter(void);
void func_reservation_on_manual_shutdown(void);

/* func_new_reservation.c：滚轮时间设置 / 提交 / 返回 Home */
void func_reservation_new_ui_load_time(u8 *hour, u8 *min, u8 *sec);
void func_reservation_new_ui_submit_time(u8 hour, u8 min, u8 sec);
bool func_reservation_new_ui_go_home(void);

#endif
