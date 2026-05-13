#include "include.h"

#if VBAT_DETECT_EN

#define VBG_VOLTAGE             sys_trim.vbg_volt
#define VBAT2_COEF              sys_trim.vbat_coef
#define VBG_LEVEL               sys_trim.vbg_lv

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

static vbat_cb_t vbat_cb;

//AT(.com_rodata.bat)
//const char bat_str[] = "VBAT: %d.%03dV\n";

AT(.com_text.port.vbat)
u16 bsp_vbat_get_voltage(u32 rst_flag)
{
    u32 vbat;
    u32 vbat2 = saradc_get_value10(VBAT2_ADCCH);
    vbat = saradc_vbat_get_calc_value(vbat2, adc_cb.bg, adc_cb.vrtc_val, adc_cb.vrtc_first);

    if (rst_flag) {
        adc_cb.vbat_bak = 0;
        adc_cb.vbat_val = vbat;
        adc_cb.vbat_total = adc_cb.vbat_val << 5;
    }

    //不同方案可能采用不同 vbat 滤波算法, 在方案对应的plugin.c中处理
    plugin_vbat_filter(&vbat);

    //默认的取平均值算法.
    adc_cb.vbat_total = adc_cb.vbat_total - adc_cb.vbat_val + vbat;
    adc_cb.vbat_val = adc_cb.vbat_total >> 5;           //平均值
    if (adc_cb.vbat_val > adc_cb.vbat_bak) {
        vbat = adc_cb.vbat_val - adc_cb.vbat_bak;
    } else {
        vbat = adc_cb.vbat_bak - adc_cb.vbat_val;
    }
    if (vbat >= 2) {   //偏差大于一定值则更新
        adc_cb.vbat_bak = adc_cb.vbat_val;
//        printf(bat_str, adc_cb.vbat_val/1000, adc_cb.vbat_val%1000);
    }
    //printf(bat_str, adc_cb.vbat_val/1000, adc_cb.vbat_val%1000);
    return adc_cb.vbat_bak;
}

void bsp_vbat_voltage_init(void)
{
    sys_cb.vbat = bsp_vbat_get_voltage(1);
    sys_cb.vbat_percent = 0xff;
}

int bsp_vbat_get_lpwr_status(void)
{
    if(CHARGE_DC_IN()){
        return 0;
    }

    if (sys_cb.vbat <= LPWR_OFF_VBAT) {
        if (LPWR_OFF_VBAT) {
            if (!sys_cb.lpwr_cnt) {
                sys_cb.lpwr_cnt = 1;
            } else if (sys_cb.lpwr_cnt >= 10) {
                return 2;       //VBAT低电关机
            }
        }
        return 0;               //VBAT低电不关机
    } else {
        sys_cb.lpwr_cnt = 0;
        if (sys_cb.vbat < LPWR_WARNING_VBAT) {
            return 1;           //VBAT低电提醒状态
        } else {
        }
        return 0;
    }
    sys_cb.lpwr_cnt = 0;
    return 0;
}

void bsp_vbat_lpwr_process(void)
{
    int vbat_sta = bsp_vbat_get_lpwr_status();

    if (vbat_sta == 2) {                            //达到低电关机状态
        func_cb.sta = FUNC_PWROFF;                  //低电，进入关机或省电模式
        return;
    } else if (vbat_sta == 1) {                     //低电提醒状态
        //低电提示音播放
        sys_cb.vbat_nor_cnt = 0;
        if (sys_cb.lpwr_warning_cnt > LPWR_WARNING_PERIOD) {
            sys_cb.lpwr_warning_cnt = 0;
            if (sys_cb.lpwr_warning_times) {        //低电语音提示次数
                sys_cb.lowbat_flag = 1;
                plugin_lowbat_vol_reduce();         //低电降低音乐音量
                if (sys_cb.lpwr_warning_times != 0xff) {
                    sys_cb.lpwr_warning_times--;
                }
            }
        }
    } else {
        if ((sys_cb.lowbat_flag) && (sys_cb.vbat > 3800)) {
            sys_cb.vbat_nor_cnt++;
            if (sys_cb.vbat_nor_cnt > 40) {
                sys_cb.lowbat_flag = 0;
                sys_cb.lpwr_warning_times = LPWR_WARING_TIMES;
                plugin_lowbat_vol_recover();        //离开低电, 恢复音乐音量
            }
        }
    }
}



/**
 * 电池电量对应百分百表
   不同电池修改这张表格，表格默认的是4.2v的电池
*/
static const u16 vbat_percent_table[11] = {
//   0%    10%   20%   30%   40%   50%   60%   70%   80%   90%   100%
    3450, 3500, 3550, 3600, 3650, 3700, 3750, 3800, 3850, 3902,  4050
};


/**
 * 电池电量标定回到，充电和放电都在这里处理了，10s一次回调
 */
static u16 vbat_percent_convert(u16 adc_value)
{
    u16 pervent = 0;
    u8 i = 0;

    if(adc_value >= vbat_percent_table[10]){
        return 100;
    }

    if(adc_value <= vbat_percent_table[0]){
        return 0;
    }

    for(i=0; i<=10; i++){
        if(adc_value < vbat_percent_table[i]){
            break;
        }
    }

    pervent = (adc_value - vbat_percent_table[i-1]) * 10 / (vbat_percent_table[i] - vbat_percent_table[i-1]) + (i-1)*10;
    return pervent;
}


/**
 * 电池电量标定回调执行，充电和放电都在这里处理了，10s执行一次（修改在bsp_vbat_percent_init函数）
 */
static void vbat_percent_process(co_timer_t *timer, void *param)
{
    if(vbat_cb.vbat_adc_last == 0){
        sys_cb.vbat_percent = vbat_percent_convert(adc_cb.vbat_val);
        vbat_cb.vbat_adc_last = adc_cb.vbat_val;
        vbat_cb.vbat_adc_curr = adc_cb.vbat_val;
        return;
    }

    vbat_cb.vbat_adc_curr = adc_cb.vbat_val;

    if(CHARGE_DC_IN()){     //充电状态
        if(sys_cb.charge_sta == 2){                                  //硬件判断充满状态
            if(adc_cb.vbat_val < vbat_percent_table[10]){               //如果硬件已经满了，对于个别一些烂电池可能始终没无法达到预设电压, 也认为其满了
                vbat_cb.vbat_adc_curr = vbat_percent_table[10];
            }
        }

        if(vbat_cb.vbat_adc_last < vbat_cb.vbat_adc_curr){
            vbat_cb.vbat_adc_last += VBAT_PERCENT_STEP*4;
        }
    }else{
        if(vbat_cb.vbat_adc_last > vbat_cb.vbat_adc_curr){
            vbat_cb.vbat_adc_last -= VBAT_PERCENT_STEP;
        }
    }

    sys_cb.vbat_percent = vbat_percent_convert(vbat_cb.vbat_adc_last);
    TRACE("vbat_adc_last:%d vbat_adc_curr:%d vbat_percent:%d\n", vbat_cb.vbat_adc_last, vbat_cb.vbat_adc_curr, sys_cb.vbat_percent);
}


/**
 * 电池百分比标定初始化
 */
void bsp_vbat_percent_init(void)
{
    static co_timer_t vbat_timer;

    memset(&vbat_cb, 0, sizeof(vbat_cb_t));
    vbat_percent_process(NULL, NULL);

	co_timer_set(&vbat_timer, 5000, TIMER_REPEAT, LEVEL_LOW_PRI, vbat_percent_process, NULL);
	co_timer_set_sleep(&vbat_timer, true);
}


/**
 * 获取电池电量百分比
 * 返回: 0~100
 */
u16 bsp_vbat_percent_get(void)
{
    return sys_cb.vbat_percent;
}


#endif  //VBAT_DETECT_EN
