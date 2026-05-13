#include <include.h>

#include "hx3605.h"
#include "fit_hx3605.h"

#if (SENSOR_HR_SEL == SENSOR_HR_TYHX_HX3605)

void hx3605_data_boot_clr(void);

static bool hx3605_40ms_timer = 0;
static bool hx3605_320ms_timer = 0;
static co_timer_t hx3605_40ms;
static co_timer_t hx3605_320ms;

#if HR_INT_MORE_SELECT
static bool hx_isr_kick = 0;

AT(.com_text.hx3605)
void hx3605_isr(void)
{
    if (!hx_isr_kick) {
        hx_isr_kick = true;
    }
}

void hx3605_gpioint_enable(void)
{
    GPIOEDE |= BIT(7);
    GPIOEDIR |= BIT(7);
    GPIOEFEN &= ~BIT(7);
    GPIOEPU |= BIT(7);
}

void hx3605_gpioint_disable(void)
{
    GPIOEDIR &= ~BIT(7);
}

void hx3605_isr_process(void)
{
//    printf("%s\n", __func__);
    if (hx_isr_kick) {
        hx3605_ppg_Int_handle();
        hx_isr_kick = false;
    }
}
#else

void hx3605_40ms_timer_set(bool en)
{
    hx3605_40ms_timer = en;
}

void hx3605_320ms_timer_set(bool en)
{
    hx3605_320ms_timer = en;
}

void hx3605_40ms_process_callback(co_timer_t *timer, void *param)
{
    // printf("%s : %d\n", __func__, hx3605_40ms_timer);
    if (hx3605_40ms_timer) {
        hx3605_agc_Int_handle();
    }
}

void hx3605_320ms_process_callback(co_timer_t *timer, void *param)
{
    // printf("%s : %d\n", __func__, hx3605_320ms_timer);
    u8 p = 0;
    if (hx3605_320ms_timer) {
        heart_rate_meas_timeout_handler((void *)&p);
    }
}
#endif // HR_INT_MORE_SELECT

bool sensor_hx3605_init(HX3605_MODE_T mode)
{
    if (hx3605_320ms_timer) {
        return false;
    }

    printf("%s, mode:%d\n", __func__, mode);
    hx3605_data_boot_clr();
    if (hx3605_init(mode) == true) {

#if HR_INT_MORE_SELECT
        hx_isr_kick = false;
#endif

        hx3605_40ms_timer = true;
        hx3605_320ms_timer = true;

        co_timer_set(&hx3605_40ms, 40, TIMER_REPEAT, LEVEL_LOW_PRI, hx3605_40ms_process_callback, NULL);
        co_timer_set_sleep(&hx3605_40ms, true);

        co_timer_set(&hx3605_320ms, 320, TIMER_REPEAT, LEVEL_LOW_PRI, hx3605_320ms_process_callback, NULL);
        co_timer_set_sleep(&hx3605_320ms, true);
    }
    return true;
}

bool sensor_hx3605_stop(void)
{
    if (!hx3605_320ms_timer) {
        return true;
    }

    printf("%s\n", __func__);
    hx3605_ppg_off();                   //芯片关闭, 进入休眠, 功耗 < 1uA
//    hx3605_chip_disable();
//    hx3605_hrs_disable();             //心率算法关闭, 同时关闭芯片

    hx3605_40ms_timer = false;
    hx3605_320ms_timer = false;

    co_timer_del(&hx3605_40ms);
    co_timer_del(&hx3605_320ms);

    return false;
}

bool sensor_hx3605_wear_sta_get(void) 
{
    bool sta = false;

    if (HRS_MODE == work_mode_flag && MSG_HRS_WEAR == hx3605_hrs_get_wear_status()) {
        sta = true;
    }

    if (SPO2_MODE == work_mode_flag && MSG_SPO2_WEAR == hx3605_spo2_get_wear_status()) {
        sta = true;
    }

    return sta;
}

int rand(void)                      //驱动静态库中调用了rand()
{
   return get_random(1000);
}

#endif //SENSOR_HR_TYHX_HX3605
