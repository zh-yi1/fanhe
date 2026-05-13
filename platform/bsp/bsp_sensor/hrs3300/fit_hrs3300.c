#include <include.h>

#if (SENSOR_HR_SEL == SENSOR_HR_TYHX_HRS3300)
#include "hrs3300.h"
#include "fit_hrs3300.h"

static bool hrs3300_start_flag = 0;
static co_timer_t hrs3300;

void hrs3300_process_40ms_callback(co_timer_t *timer, void *param)
{
	u8 p = 0;
    heart_rate_meas_timeout_handler((void *)&p);
}

bool get_hrs3300_sta(void)
{
    return hrs3300_start_flag;
}

bool sensor_hrs3300_init(hrss_mode_t hrss_mode)
{
    if (!hrs3300_start_flag) {
        printf("%s mode:%d\n", __func__, hrss_mode);
        if (hrss_mode == 3 ) {
            hrss_mode = HRSS_BP_MODE;
        }
        Hrs3300_chip_init(hrss_mode);
        Hrs3300_chip_enable();
        Hrs3300_alg_open();
        hrs3300_start_flag = true;

        co_timer_set(&hrs3300, 40, TIMER_REPEAT, LEVEL_LOW_PRI, hrs3300_process_40ms_callback, NULL);
        co_timer_set_sleep(&hrs3300, true);
    }
    return hrs3300_start_flag;
}

bool sensor_hrs3300_stop(void)
{
    if (!hrs3300_start_flag) {
        return true;
    }

    printf("%s\n", __func__);
    Hrs3300_chip_disable();
    hrs3300_start_flag = false;

    co_timer_del(&hrs3300);

    return hrs3300_start_flag;
}

bool sensor_hx3300_wear_sta_get(void)
{
    hrs3300_results_t result = Hrs3300_alg_get_results();

    return result.alg_status == MSG_NO_TOUCH ? false : true;
}


#endif //SENSOR_HR_TYHX_HRS3300
