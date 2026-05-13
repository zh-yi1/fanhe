#include <include.h>

#include "hx3602.h"
#include "hx3602_hrs_driv.h"
#include "tyhx_hrs_alg.h"
#include "fit_hx3602.h"

#if (SENSOR_HR_SEL == SENSOR_HR_TYHX_HX3602)

void hx3602_data_boot_clr(void);

static bool hx3602_40ms_timer = 0;
static bool hx3602_320ms_timer = 0;
//static co_timer_t hx3602_40ms;
//static co_timer_t hx3602_320ms;

#if HR_INT_MORE_SELECT
static bool hx_isr_kick = 0;

AT(.com_text.hx3602)
void hx3602_isr(void)
{
    if (!hx_isr_kick) {
        hx_isr_kick = true;
    }
}

void hx3602_gpioint_enable(void)
{
    GPIOEDE |= BIT(7);
    GPIOEDIR |= BIT(7);
    GPIOEFEN &= ~BIT(7);
    GPIOEPU |= BIT(7);
}

void hx3602_gpioint_disable(void)
{
    GPIOEDIR &= ~BIT(7);
}

void hx3602_isr_process(void)
{
//    printf("%s\n", __func__);
    if (hx_isr_kick) {
        hx3602_ppg_Int_handle();
        hx_isr_kick = false;
    }
}
#else

void hx3602_40ms_timer_set(bool en)
{
    hx3602_40ms_timer = en;
}

void hx3602_320ms_timer_set(bool en)
{
    hx3602_320ms_timer = en;
}

void hx3602_40ms_process_callback(void)
{
    // printf("%s : %d\n", __func__, hx3602_40ms_timer);
    if (hx3602_40ms_timer) {
        u8 p = 0;
        heart_rate_meas_timeout_handler((void *)&p);

    }
}

void hx3602_320ms_process_callback(co_timer_t *timer, void *param)
{
    // printf("%s : %d\n", __func__, hx3602_320ms_timer);
    u8 p = 0;
    if (hx3602_320ms_timer) {
        heart_rate_meas_timeout_handler((void *)&p);
    }
}
#endif // HR_INT_MORE_SELECT

bool sensor_hx3602_init(void)
{
    printf("%s\n", __func__);

    if (!Hrs3602_chip_init()) {
        return false;
    }
    Hrs3602_alg_config();
    Hrs3602_driv_init();
    tyhx_hrs_alg_open();

    hx3602_40ms_timer_set(true);

    return true;
}

bool sensor_hx3602_stop(void)
{
//    if (!hx3602_320ms_timer) {
//        return true;
//    }
//
    printf("%s\n", __func__);
//    hx3602_ppg_off();                   //芯片关闭, 进入休眠, 功耗 < 1uA
////    hx3602_chip_disable();
////    hx3602_hrs_disable();             //心率算法关闭, 同时关闭芯片
//
//    hx3602_40ms_timer = false;
//    hx3602_320ms_timer = false;
//
//    co_timer_del(&hx3602_40ms);
//    co_timer_del(&hx3602_320ms);

    return false;
}

bool sensor_hx3602_wear_sta_get(void)
{
    bool sta = false;
//
//    if (HRS_MODE == work_mode_flag && MSG_HRS_WEAR == hx3602_hrs_get_wear_status()) {
//        sta = true;
//    }
//
//    if (SPO2_MODE == work_mode_flag && MSG_SPO2_WEAR == hx3602_spo2_get_wear_status()) {
//        sta = true;
//    }

    return sta;
}

int rand(void)                      //驱动静态库中调用了rand()
{
   return get_random(1000);
}

#endif //SENSOR_HR_TYHX_HX3602
