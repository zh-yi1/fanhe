#ifndef __FUNC_LOWPWR_H
#define __FUNC_LOWPWR_H

typedef bool (*is_sleep_func)(void);

#define GUI_OFF_DELAY_TIME      sys_cb.sleep_time        //和配置的休眠时间一样
#define en_auto_pwroff()        sys_cb.pwroff_delay = sys_cb.pwroff_time;
#define dis_auto_pwroff()       sys_cb.pwroff_delay = -1L;
#define reset_pwroff_delay()    if(sys_cb.pwroff_delay != -1L) {sys_cb.pwroff_delay = sys_cb.pwroff_time;}
#define reset_sleep_delay()     sys_cb.sleep_delay = sys_cb.sleep_time;
#define reset_guioff_delay()    sys_cb.guioff_delay = sys_cb.sleep_time;
#define reset_sleep_delay_all() {sys_cb.sleep_delay = sys_cb.sleep_time; sys_cb.guioff_delay = sys_cb.sleep_time;}

#define sleep_only_gui_off_en()             {sys_cb.is_sleep_off_gui = true;}
#define sleep_only_gui_off_dis()            {sys_cb.is_sleep_off_gui = false;}
#define sleep_is_only_gui_off()             (sys_cb.is_sleep_off_gui)

extern u8 vddio_sleep_level;
bool sleep_process(is_sleep_func is_sleep);
void lowpwr_tout_ticks(void);
void sfunc_pwrdown(u8 vusb_wakeup_en);          //软开关方案，POWER键/低电时，进入关机状态
void func_pwroff(int pwroff_tone_en);           //进入低电或者关机状态
void sys_enter_sleep(u8 lpclk_type);
void sleep_set_sysclk(uint8_t sys_clk);         //休眠状态下设置系统时钟, 用于抬高主频跑算法，使用完需要设置为24M
void elunchbox_enter_sleep(void);               /* 饭盒: 原 sfunc_sleep, 进深睡 */
u8 sys_enter_sleep_vddio_level(void);

#endif // __FUNC_LOWPWR_H
