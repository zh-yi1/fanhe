#include "include.h"

u32 getcfg_vddbt_sel(void)
{
    return xcfg_cb.vddbt_sel;
}

u32 getcfg_vddio_sel(void)
{
    return xcfg_cb.vddio_sel;
}

AT(.com_text.getcfg.vddbt)
u32 getcfg_vddbt_capless_en(void)
{
    return xcfg_cb.vddbt_capless_en;
}

u32 getcfg_mic_bias_method(u8 mic_ch)
{
    if (mic_ch == MIC0) {
        return xcfg_cb.mic0_pwr_sel << 4 | xcfg_cb.mic0_bias_method;
    }
    if (mic_ch == MIC1) {
        return xcfg_cb.mic1_pwr_sel << 4 | xcfg_cb.mic1_bias_method;
    }
    return 0;
}

u32 getcfg_mic_save_rc(void)
{
    if (xcfg_cb.mic0_en && xcfg_cb.mic0_bias_method == 2) {
        return 1;
    } else if (xcfg_cb.mic1_en && xcfg_cb.mic1_bias_method == 2) {
        return 2;
    }
    return 0;
}

