#ifndef _FUNC_RESERVATION_H
#define _FUNC_RESERVATION_H

#include "include.h"

typedef enum {
    RES_PHASE_NONE = 0,
    RES_PHASE_WAITING,
    RES_PHASE_HEATING,
    RES_PHASE_FINISHED,
} reservation_phase_t;

void func_reservation_poll(void);
reservation_phase_t func_reservation_get_phase(void);
bool func_reservation_is_waiting(void);
void func_reservation_marquee_text(char *buf, u16 buf_len);
void func_reservation_force_heating_enter(void);

#endif
