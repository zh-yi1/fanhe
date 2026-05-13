#ifndef _FIT_HRS3300_H_
#define _FIT_HRS3300_H_

#include "include.h"

enum hrss_mode{
    HRSS_HRS_MODE,
    HRSS_SPO2_MODE,
    HRSS_BP_MODE,
};
typedef enum hrss_mode hrss_mode_t;

bool get_hrs3300_sta(void);
bool sensor_hrs3300_init(hrss_mode_t hrss_mode);
bool sensor_hrs3300_stop(void);
void hrs3300_process_40ms(void);
void hrs3300_kick(u8 select);
bool sensor_hx3300_wear_sta_get(void);
#endif // _FIT_HRS3300_H_
