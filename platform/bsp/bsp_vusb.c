#include "include.h"

#if VUSB_DETECT_EN

#define VBG_VOLTAGE             sys_trim.vbg_volt
#define VBAT2_COEF              sys_trim.vbat_coef

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

AT(.com_text.port.vbat)
u16 bsp_vusb_get_voltage(void)
{
    if (!CHARGE_DC_IN()) {
        return 0;
    }
    return saradc_vusb_get_calc_value(adc_cb.vusb, adc_cb.bg);;
}

#endif
