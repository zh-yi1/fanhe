#include "include.h"
#include "func_menu.h"
#include "func_tbl.h"
#include "func.h"
#include "func_reservation.h"
#if ELUNCHBOX_PANEL_EN
#include "home_ui_shared.h"
/* ELUNCHBOX 模式：TE block 标志声明 */
extern volatile u8 elunchbox_te_block_flag;
#endif
#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#include "port_pt8028_key.h"
#endif
#if USER_PANEL_LED
#include "port_panel_led.h"
#endif

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

bool func_music_is_play(void);
void func_music_play(bool sta);
void func_call_mgr_process(void);
u8 func_menu_sub_skyrer_get_first_idx(void);
compo_form_t *func_clock_form_create_by_screenshoot(void);

func_cb_t func_cb AT(.buf.func_cb);

#if BT_BACKSTAGE_EN
AT(.text.func.process)
void func_watch_bt_process(void)
{
    uint disp_status = bsp_bt_disp_status();

    if (disp_status == BT_STA_OTA) {
        sfunc_bt_ota();
    }
#if !CALL_MGR_EN
    else if (sys_cb.reject_tick) {
        if (tick_check_expire(sys_cb.reject_tick, 3000) || disp_status < BT_STA_INCOMING) {
            sys_cb.reject_tick = 0;
        }
#if BT_VOIP_REJECT_EN
    } else if (bt_cb.disp_status == BT_STA_INCOMING && bt_cb.call_type == CALL_TYPE_PHONE) {
#else
    } else if (bt_cb.disp_status == BT_STA_INCOMING) {
#endif
        func_cb.sta = FUNC_BT_RING;

#if BT_VOIP_REJECT_EN
    } else if (bt_cb.disp_status >= BT_STA_OUTGOING && bt_cb.call_type == CALL_TYPE_PHONE) {
#else
    } else if (bt_cb.disp_status >= BT_STA_OUTGOING) {
#endif
        func_cb.sta = FUNC_BT_CALL;
    }
#endif
}
#endif // BT_BACKSTAGE_EN
bool gui_get_auto_power_en(void);

#if ELUNCHBOX_PANEL_EN
static bool func_elunchbox_res_key_page_ok(void)
{
    switch (func_cb.sta) {
    case FUNC_HOME:
    case FUNC_MODE:
    case FUNC_SETUP:
    case FUNC_HEAT:
    case FUNC_LANGUAGEING:
    case FUNC_TIMEING:
    case FUNC_VERINFO:
        return true;
    default:
        return false;
    }
}

void func_elunchbox_switch_to_reservation(void)
{
#if !FUNC_RESERVATION_UI_EN
    return;
#endif
    if (func_cb.sta == FUNC_RESERVATION) {
        return;
    }
    if (sys_cb.flag_swithing) {
        return;
    }
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif
    /* 与加热/模式/设置子页一致：FADE_OUT 切页 + func_exit 统一销毁源 frm。
     * 勿在 func_home_process 内抢先 destroy/free（易 use-after-free / 长时间阻塞触发 WDT）。
     */
    home_gpu_wait_idle();
    WDT_CLR();
    func_res_allow_switch = 1;
    func_switch_to(FUNC_RESERVATION, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
    func_res_allow_switch = 0;
}

void func_elunchbox_switch_to_heat(void)
{
    if (func_cb.sta == FUNC_HEAT) {
        return;
    }
    if (sys_cb.flag_swithing) {
        return;
    }
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
    func_home_drain_stale_key_msgs();
    pt8028_release_clear();
#endif
    home_gpu_wait_idle();
    WDT_CLR();
    func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

void func_elunchbox_res_key_poll(void)
{
#if USER_PT8028_KEY && FUNC_RESERVATION_UI_EN
    if (sys_cb.flag_swithing) {
        return;
    }
    if (pt8028_take_res_key_pending()) {
        if (func_key_lock_is_active()) {
            func_key_lock_notify_blocked();
            return;
        }
        if (func_elunchbox_res_key_page_ok()) {
            func_elunchbox_switch_to_reservation();
        }
    }
#endif
}
#endif /* ELUNCHBOX_PANEL_EN */

#if USER_PT8028_KEY && SOFT_POWER_ON_OFF
#if ELUNCHBOX_PANEL_EN
static bool elunchbox_pwr_gui_off;
static bool elunchbox_boot_power_sent;
static s32 elunchbox_guioff_sleep_delay = -1L;
static u8 elunchbox_guioff_sleep_mode;

void elunchbox_guioff_sleep_delay_reset(void)
{
#if ELUNCHBOX_GUIOFF_SLEEP_EN
    elunchbox_guioff_sleep_delay = (s32)ELUNCHBOX_GUIOFF_SLEEP_DELAY_SEC * 10;
#else
    elunchbox_guioff_sleep_delay = -1L;
#endif
}

void elunchbox_guioff_sleep_delay_tick(void)
{
    if (elunchbox_guioff_sleep_delay > 0) {
        elunchbox_guioff_sleep_delay--;
    }
}

bool elunchbox_guioff_sleep_ready(void)
{
#if ELUNCHBOX_GUIOFF_SLEEP_EN
    return elunchbox_guioff_sleep_delay == 0;
#else
    return false;
#endif
}

void elunchbox_guioff_sleep_delay_rearm(void)
{
#if ELUNCHBOX_GUIOFF_SLEEP_EN
    elunchbox_guioff_sleep_delay = 10;             /* 浅睡唤醒后 1s 再允许入睡 */
#endif
}

bool elunchbox_guioff_in_sleep_mode(void)
{
    return elunchbox_guioff_sleep_mode != 0;
}

void elunchbox_guioff_sleep_service(void)
{
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_uart_process();
    lunchbox_keep_warm_poll();
#endif
    func_reservation_poll();
}

void elunchbox_guioff_sleep_mode_enter(void)
{
    elunchbox_guioff_sleep_mode = 1;
}

static void elunchbox_pwr_gui_off_exit(void);

bool elunchbox_pwr_gui_off_is_on(void)
{
    return elunchbox_pwr_gui_off;
}

bool elunchbox_is_device_powered(void)
{
    return true;
}

void elunchbox_pwr_gui_off_activate(void)
{
    if (elunchbox_pwr_gui_off && sys_cb.gui_sleep_sta) {
        return;
    }
#if USER_PANEL_LED
    panel_led_all_off();
    panel_led_set_switch_latched(true);
    panel_led_set(PANEL_LED_ID_SWITCH, true);
#endif
    gui_sleep(false);
    elunchbox_pwr_gui_off = true;
    sys_cb.gui_need_wakeup = 0;
    elunchbox_guioff_sleep_delay_reset();
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_keep_warm_stop();
    lunchbox_power_off();
#endif
    elunchbox_boot_power_sent = false;
}

void elunchbox_guioff_sleep_post_wake(bool key_wake)
{
    elunchbox_guioff_sleep_mode = 0;
    pt8028_port_gpio_init();
    pt8028_key_scan();
    elunchbox_guioff_sleep_service();
    if (key_wake) {
        elunchbox_guioff_sleep_delay_reset();
    } else {
        elunchbox_guioff_sleep_delay_rearm();
    }
}

void elunchbox_pwr_gui_wake(void)
{
    if (!elunchbox_pwr_gui_off && !sys_cb.gui_sleep_sta) {
        return;
    }
    elunchbox_pwr_gui_off_exit();
}

static void elunchbox_screen_wake(void)
{
    elunchbox_pwr_gui_off = false;
    elunchbox_guioff_sleep_delay_reset();
    if (sys_cb.gui_sleep_sta) {
        gui_wakeup();
    }
    tft_bglight_force_on();
    reset_sleep_delay_all();
#if USER_PANEL_LED
    panel_led_scan();
#endif
}

static void elunchbox_pwr_gui_off_exit(void)
{
    elunchbox_screen_wake();
}

void elunchbox_panel_boot_power_on(void)
{
    if (elunchbox_boot_power_sent) {
        return;
    }
    elunchbox_boot_power_sent = true;
#if USER_PANEL_LED
    panel_led_set_switch_latched(true);
#endif
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_power_on();
#endif
}

static bool elunchbox_is_guioff(void)
{
    return sys_cb.gui_sleep_sta || elunchbox_pwr_gui_off;
}

static void elunchbox_guioff_idle_process(void)
{
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_uart_process();
    lunchbox_keep_warm_poll();
#endif
    func_reservation_poll();
}

static void func_elunchbox_pwr_long_poll(void)
{
    if (!pt8028_take_pwr_long_pending()) {
        return;
    }
    pt8028_pwr_long_consume();
    if (elunchbox_is_guioff()) {
        printf("elunchbox: TCH5 long -> wake\n");
        elunchbox_pwr_gui_wake();
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_power_on();
        elunchbox_boot_power_sent = true;
#endif
    } else {
        printf("elunchbox: TCH5 long -> guioff\n");
        elunchbox_pwr_gui_off_activate();
    }
}
#endif /* ELUNCHBOX_PANEL_EN */

#if USER_PT8028_KEY && SOFT_POWER_ON_OFF && !ELUNCHBOX_PANEL_EN
static void func_elunchbox_pwr_long_poll(void)
{
    if (!pt8028_take_pwr_long_pending()) {
        return;
    }
    func_cb.sta = FUNC_PWROFF;
}
#endif

#endif /* USER_PT8028_KEY && SOFT_POWER_ON_OFF */

#if USER_PT8028_KEY && FUNC_LUNCHBOX_UART_EN
static void func_elunchbox_key_notify_poll(void)
{
    u8 tch;
    u8 key_val;

    tch = pt8028_take_key_notify_tch();
    if (tch > PT8028_KEY_TCH7 || tch == PT8028_KEY_TCH5) {
        return;
    }
    key_val = pt8028_tch_to_lunchbox_key(tch);
    if (key_val != 0) {
        lunchbox_key_notify(key_val);
    }
}
#endif /* USER_PT8028_KEY && FUNC_LUNCHBOX_UART_EN */

AT(.text.func.process)
void func_process(void)
{
#if ELUNCHBOX_PANEL_EN
    bool guioff = elunchbox_is_guioff();
#else
    bool guioff = sys_cb.gui_sleep_sta;
#endif

    if (gui_get_auto_power_en() && !guioff) {
        sys_clk_req(INDEX_GUI, SYS_192M);
    }

    WDT_CLR();

#if CPU_USAGE_MONITOT_EN
    cpu_trace_monitor();
#endif

#if (SD_SUPPORT_EN) && SD_SOFT_DETECT_EN
    sd_soft_cmd_detect(120);
#endif

#if ELUNCHBOX_PANEL_EN
    if (!guioff || !sys_cb.gui_sleep_sta)
#endif
    tft_bglight_frist_set_check();

    // gui 没有休眠才更新
	#if FOTA_UI_EN
    if ((!guioff) && (func_cb.sta != FUNC_OTA_UI_MODE) && !sys_cb.flag_halt) {
	#else
	if (!guioff && !sys_cb.flag_halt) {
	#endif

#if ELUNCHBOX_PANEL_EN
        bool gui_do_refresh = true;

        if (sys_cb.flag_swithing) {
            gui_do_refresh = false;
        }
#if USER_PT8028_KEY
        /* Home 在 func_home_process 内扫键；子页（加热/模式/设置/预约等）须在此扫键 */
        if (func_cb.sta != FUNC_HOME) {
            pt8028_gpio_ensure_periodic();
            pt8028_key_scan();
        }
#if FUNC_LUNCHBOX_UART_EN
        func_elunchbox_key_notify_poll();
#endif
#endif
#if USER_PANEL_LED && USER_PT8028_KEY
        /* 按下对应 TCH 点亮 LED，松开全灭（原理图 LED1~6 -> PB0/PB1/PB2/PB5/PB6/PB7） */
        panel_led_scan();
#endif
        if (func_cb.frm_main != NULL) {
            compo_update();
            if (gui_do_refresh) {
                gui_process();
            }
        }
#if USER_PT8028_KEY && FUNC_RESERVATION_UI_EN
        func_elunchbox_res_key_poll();
#endif
#if USER_PT8028_KEY && ELUNCHBOX_PANEL_EN
        func_heat_key_poll();
#endif
        func_key_lock_poll();
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_keep_warm_poll();
#endif
        func_reservation_poll();
#else
        compo_update();                                     //更新组件

        gui_process();                                      //刷新UI
        func_reservation_poll();
#if FUNC_LUNCHBOX_UART_EN
        lunchbox_keep_warm_poll();
#endif
#endif

    } else if (guioff) {
#if ELUNCHBOX_PANEL_EN
        elunchbox_guioff_idle_process();
        pt8028_gpio_ensure_periodic();
        pt8028_key_scan();
#endif
    }

//#if OPUS_ENC_EN
//    if (bsp_opus_is_encode()) {
//        reset_sleep_delay_all();
//        u8 temp_buf[80];
//        memset(temp_buf, 0, sizeof(temp_buf));
//        if (bsp_opus_get_enc_frame(temp_buf, sizeof(temp_buf))) {
//            print_r(temp_buf, sizeof(temp_buf));
//        }
//    }
//#endif//OPUS_ENC_EN

    co_timer_pro(false);
    bsp_sensor_step_pro_isr();

    if (sys_cb.mp3_res_playing) {
        mp3_res_process();                                 //提示音后台处理
    }

    if (sleep_process(bt_is_allow_sleep)) {
        bt_cb.disp_status = 0xff;
    }

#if VBAT_DETECT_EN
    bsp_vbat_lpwr_process();
#endif

#if BT_BACKSTAGE_EN
    func_watch_bt_process();
#endif

#if CALL_MGR_EN
    func_call_mgr_process();
#endif
    //PWRKEY模拟硬开关关机处理
    if ((PWRKEY_2_HW_PWRON) && (sys_cb.pwrdwn_hw_flag)) {
        func_cb.sta = FUNC_PWROFF;
        sys_cb.pwrdwn_hw_flag = 0;
    }

#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_detect(1);
    }
#endif // CHARGE_EN

    if(bt_cb.bt_is_inited) {
        bt_thread_check_trigger(); //经典蓝牙线程
#if LE_EN
        ble_app_process();
#endif
#if LE_AB_FOT_EN
    	bsp_fot_process();
#endif
#if FUNC_LUNCHBOX_UART_EN
    	lb_ota_process();
#endif
    }

//#if MUSCI_BACKSTAGE_EN
//    bsp_music_process();
//#endif

#if ASR_SELECT
    bsp_asr_process();
#endif

#if SENSOR_HUB_EN
    bsp_sensorhub_process();
#endif

#if VBAT_ADC_EN
    static u32 ticks = 0;
    u32 vadc_process(void);
    if (tick_check_expire(ticks, 500)) {
        ticks = tick_get();
        /*u32 val = */vadc_process();
//        printf("vadc:%d uv, vbat:%d mv\n", val, sys_cb.vbat);
//        void vadc_test(void);
//        vadc_test();
    }
#endif

   if (gui_get_auto_power_en() && !guioff) {
        sys_clk_free(INDEX_GUI);
   }

#if FUNC_LUNCHBOX_UART_EN
    if (!guioff) {
        lunchbox_uart_process();
    }
#endif

#if USER_PT8028_KEY && SOFT_POWER_ON_OFF
    func_elunchbox_pwr_long_poll();
#endif
}

//根据任务名创建窗体。此处调用的创建窗体函数不要调用子任务的控制结构体
compo_form_t *func_create_form(u8 sta)
{
    compo_form_t *frm = NULL;
    compo_form_t *(*func_create)(void) = NULL;
    for (int i = 0; i < FUNC_CREATE_CNT; i++) {
        if (tbl_func_create[i].func_idx == sta) {
            func_create = tbl_func_create[i].func;
            if (func_create != NULL) {
                frm = func_create();
            }
            break;
        }
    }
    if (frm == NULL) {
        halt(HALT_FUNC_SORT);
    }
    return frm;

}

//获取当前任务顺序
static int func_get_order(u8 sta)
{
    int i;
    for (i=0; i<func_cb.sort_cnt; i++) {
        if (sta == func_cb.tbl_sort[i]) {
            return i;
        }
    }
    return -1;
}

//执行当前任务退出
void func_cur_sta_exit(void)
{
    void (*exit)(void) = NULL;
    for (int i = 0; i < FUNC_EXIT_CNT; i++) {
        if (tbl_func_exit[i].func_idx == func_cb.sta) {
            exit = tbl_func_exit[i].func;
            exit();
            break;
        }
    }
}


//切换到上一个任务
void func_switch_prev(bool flag_auto)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN

    compo_form_t *frm = NULL;
    u8 mode = func_get_switching_mode_byidx(sys_cb.nav_index, false);
    u16 switch_mode = flag_auto ? (mode | FUNC_SWITCH_AUTO) : mode;
    s8 idx = func_get_order(func_cb.sta);

    u8 sta;
    if (idx <= 0) {
        sta = task_stack_pop();
    } else {
        sta = func_cb.tbl_sort[idx - 1];
    }
#if !FUNC_RESERVATION_UI_EN
    if (sta == FUNC_RESERVATION) {
#if VIDEO_PLAY_EN
        compo_video_exit_unlock(video);
#endif
        return;
    }
#endif
#if GUI_USE_SCREENSHOOT
    compo_form_t *frm_cur = NULL;
    func_switching3d_form_create(switch_mode, sta, &frm_cur, &frm);
    bool res = func_switching(switch_mode, NULL);                     //切换动画
    if (frm != NULL) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }
    compo_form_destroy(frm_cur);                                      //切换完成或取消，销毁窗体

#else
    if (mode != FUNC_SWITCH_FADE_OUT) {
        frm = func_create_form(sta);                                  //创建下一个任务的窗体
    }

    bool res = func_switching(switch_mode, NULL);                     //切换动画

    if (frm) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }
#endif

#if GUI_USE_SCREENSHOOT
//    func_cb.frm_main = func_create_form(func_cb.sta);                 //换回真实的窗体
    void (*enter)(void) = NULL;
    for (int i = 0; i < FUNC_ENTER_CNT; i++) {
        if (tbl_func_enter[i].func_idx == func_cb.sta) {
            enter = tbl_func_enter[i].func;
            enter();
            break;
        }
    }
#endif

    if (res) {
        if (sta == FUNC_CLOCK) {
            func_cb.flag_sort = false;
        }
        func_cb.sta = sta;
    }

#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}

//切换到下一个任务
void func_switch_next(bool flag_auto, bool flag_loop)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN

    compo_form_t *frm = NULL;
    u8 sta;
    u8 mode = func_get_switching_mode_byidx(sys_cb.nav_index, true);
    u16 switch_mode = flag_auto ? (mode | FUNC_SWITCH_AUTO) : mode;
    u8 idx = func_get_order(func_cb.sta);
    if (idx < 0) {
        return ;
    }

    if (idx >= func_cb.sort_cnt - 1) {
        if (flag_loop) {
            sta = func_cb.tbl_sort[0];
        } else {
            return ;
        }
    } else {
        sta = func_cb.tbl_sort[idx + 1];
    }

#if !FUNC_RESERVATION_UI_EN
    if (sta == FUNC_RESERVATION) {
#if VIDEO_PLAY_EN
        compo_video_exit_unlock(video);
#endif
        return;
    }
#endif

#if GUI_USE_SCREENSHOOT
    compo_form_t *frm_cur = NULL;
    func_switching3d_form_create(switch_mode, sta, &frm_cur, &frm);
    bool res = func_switching(switch_mode, NULL);                     //切换动画
    if (NULL != frm) {
        compo_form_destroy(frm);                                      	  //切换完成或取消，销毁窗体
    }
    compo_form_destroy(frm_cur);                                      //切换完成或取消，销毁窗体
#else
    if (mode != FUNC_SWITCH_FADE_OUT) {
        frm = func_create_form(sta);                                  //创建下一个任务的窗体
    }

    bool res = func_switching(switch_mode, NULL);                     //切换动画

    if (frm) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }
#endif

#if GUI_USE_SCREENSHOOT
//    func_cb.frm_main = func_create_form(func_cb.sta);                 //换回真实的窗体
    void (*enter)(void) = NULL;
    for (int i = 0; i < FUNC_ENTER_CNT; i++) {
        if (tbl_func_enter[i].func_idx == func_cb.sta) {
            enter = tbl_func_enter[i].func;
            enter();
            break;
        }
    }
#endif

    if (res) {
        if (sta == FUNC_CLOCK) {
            func_cb.flag_sort = false;
        } else {
            func_cb.flag_sort = true;
        }
        func_cb.sta = sta;
    }

#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}


//切换
void func_switch_to(u8 sta, u16 switch_mode)
{
#if !FUNC_RESERVATION_UI_EN
    if (sta == FUNC_RESERVATION) {
        return;
    }
#endif
#if ELUNCHBOX_PANEL_EN
#if FUNC_RESERVATION_UI_EN
    if (sta == FUNC_RESERVATION && !func_res_allow_switch) {
        return;
    }
#endif
    if (sys_cb.flag_swithing) {
        return;
    }
    home_gpu_wait_idle();
    WDT_CLR();
#endif
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN

    compo_form_t *frm = NULL;

#if GUI_USE_SCREENSHOOT
    compo_form_t *frm_cur = NULL;
    if (sta == FUNC_SMARTSTACK) {
        frm = func_create_form(sta);
        switch_mode = (switch_mode & 0x8000) | FUNC_SWITCH_DIRECT;
    } else {
        func_switching3d_form_create(switch_mode, sta, &frm_cur, &frm);
    }
    bool res = func_switching(switch_mode, NULL);                     //切换动画
    if (frm) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }
    compo_form_destroy(frm_cur);                                      //切换完成或取消，销毁窗体
#else
    u8 mode = switch_mode & 0x7fff;
    /* ELUNCHBOX：DIRECT 和 FADE_OUT 都不在此预创建目标窗体。 */
    if (mode != FUNC_SWITCH_FADE_OUT && mode != FUNC_SWITCH_DIRECT) {
        frm = func_create_form(sta);
    }

    bool res = func_switching(switch_mode, NULL);

#if ELUNCHBOX_PANEL_EN
    if (res && mode != FUNC_SWITCH_DIRECT && mode != FUNC_SWITCH_FADE_OUT) {
        home_gpu_wait_idle();
    }
#endif

    if (frm) {
        compo_form_destroy(frm);
    }

#if ELUNCHBOX_PANEL_EN
    /* ELUNCHBOX DIRECT/FADE_OUT：切页成功后销毁源 frm，compos_init 清 GPU 池 */
    if ((mode == FUNC_SWITCH_DIRECT || mode == FUNC_SWITCH_FADE_OUT) && func_cb.frm_main != NULL) {
        home_gpu_wait_idle();
        WDT_CLR();
        compo_form_destroy(func_cb.frm_main);
        home_gpu_wait_idle();
        WDT_CLR();
        compos_init();
        home_gpu_wait_idle();
        WDT_CLR();
        func_cb.frm_main = NULL;
        if (res) {
            func_cb.sta = sta;
        }
    }
#endif

#endif

#if GUI_USE_SCREENSHOOT
//    func_cb.frm_main = func_create_form(func_cb.sta);                 //换回真实的窗体
    if (sta != FUNC_SMARTSTACK) {
        void (*enter)(void) = NULL;
        for (int i = 0; i < FUNC_ENTER_CNT; i++) {
            if (tbl_func_enter[i].func_idx == func_cb.sta) {
                enter = tbl_func_enter[i].func;
                enter();
                break;
            }
        }
    }
#endif

    if (res) {
        func_cb.sta = sta;
    }

#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}

//切换回主时钟
void func_switch_to_clock(void)
{
    func_switch_to(FUNC_CLOCK, FUNC_SWITCH_LR_ZOOM_RIGHT | FUNC_SWITCH_AUTO);
    func_cb.flag_sort = false;
}


//退回到主菜单
void func_switch_to_menu(void)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN
    u16 switch_mode;
    bool flag_frm_menu;                                                         //是否需要创建菜单窗体
    flag_frm_menu = true;
    if (func_cb.menu_style == MENU_STYLE_FOOTBALL) {
        switch_mode = FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO;;
        flag_frm_menu = false;
    } else if (func_cb.sta != FUNC_CLOCK || func_cb.menu_style == MENU_STYLE_HONEYCOMB) {
        switch_mode = FUNC_SWITCH_ZOOM_EXIT | FUNC_SWITCH_AUTO;
    } else if (func_cb.menu_style == MENU_STYLE_WATERFALL) {
        switch_mode = FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO;
        func_cb.flag_animation = true;                                          //淡出后进入入场动画
        flag_frm_menu = false;
    } else {
        switch_mode = FUNC_SWITCH_ZOOM_FADE_EXIT | FUNC_SWITCH_AUTO;
    }
    if (flag_frm_menu) {
        widget_icon_t *icon;
        compo_form_t *frm = func_create_form(FUNC_MENU);                        //创建下一个任务的窗体
#if (ASR_SELECT && ASR_VOICE_BALL_ANIM)
        component_t *compo = compo_get_next((component_t *)frm->anim);
#else
        component_t *compo = compo_get_next((component_t *)frm->title);
#endif

        if (compo->type == COMPO_TYPE_ICONLIST) {
            compo_iconlist_t *iconlist = (compo_iconlist_t *)compo;
            icon = compo_iconlist_select_byidx(iconlist, func_cb.menu_idx);
        } else if (compo->type == COMPO_TYPE_LISTBOX) {
            compo_listbox_t *listbox = (compo_listbox_t *)compo;
            icon = compo_listbox_select_byidx(listbox, func_cb.menu_idx);
        } else if (compo->type == COMPO_TYPE_DISKLIST) {
            compo_disklist_t *disklist = (compo_disklist_t *)compo;
            icon = compo_disklist_select_byidx(disklist, func_cb.menu_idx);
        } else if (compo->type == COMPO_TYPE_KALEIDOSCOPE) {
            compo_kaleidoscope_t *kale = (compo_kaleidoscope_t *)compo;
            icon = compo_kale_select_byidx(kale, func_cb.menu_idx);
        } else if (compo->type == COMPO_TYPE_RINGS) {
            compo_rings_t *rings = (compo_rings_t *)compo;
            icon = compo_rings_select_byidx(rings, func_cb.menu_idx);
            if (icon == NULL) {
                func_cb.menu_idx = func_menu_sub_skyrer_get_first_idx();
                icon = compo_rings_select_byidx(rings, func_cb.menu_idx);
            }
        } else {
//            printf("%s\n", __func__);
//            halt(HALT_GUI_COMPO_ICONLIST_TYPE);
//            return;
            icon = NULL;
        }
        if (icon == NULL) {
            switch_mode = FUNC_SWITCH_FADE_OUT;
        }
        func_switching(switch_mode, icon);                                      //退出动画
        compo_form_destroy(frm);                                                //切换完成或取消，销毁窗体
    } else {
        func_switching(switch_mode, NULL);                                      //退出动画
    }
    func_cb.sta = FUNC_MENU;
#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}

//上滑进入足球菜单
void func_switch_to_football_menu(void)
{
    func_cb.menu_style = MENU_STYLE_FOOTBALL;
    func_switch_to_menu();
}

//手动退回到主菜单
void func_switching_to_menu(void)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN

    widget_icon_t *icon;
    u16 switch_mode;
    compo_form_t *frm = func_create_form(FUNC_MENU);                            //创建下一个任务的窗体
#if (ASR_SELECT && ASR_VOICE_BALL_ANIM)
        component_t *compo = compo_get_next((component_t *)frm->anim);
#else
        component_t *compo = compo_get_next((component_t *)frm->title);
#endif
    if (compo->type == COMPO_TYPE_ICONLIST) {
        compo_iconlist_t *iconlist = (compo_iconlist_t *)compo;
        icon = compo_iconlist_select_byidx(iconlist, func_cb.menu_idx);
    } else if (compo->type == COMPO_TYPE_LISTBOX) {
        compo_listbox_t *listbox = (compo_listbox_t *)compo;
        icon = compo_listbox_select_byidx(listbox, func_cb.menu_idx);
    } else if (compo->type == COMPO_TYPE_DISKLIST) {
        compo_disklist_t *disklist = (compo_disklist_t *)compo;
        icon = compo_disklist_select_byidx(disklist, func_cb.menu_idx);
    } else if (compo->type == COMPO_TYPE_KALEIDOSCOPE) {
        compo_kaleidoscope_t *kale = (compo_kaleidoscope_t *)compo;
        icon = compo_kale_select_byidx(kale, func_cb.menu_idx);
    } else if (compo->type == COMPO_TYPE_RINGS) {
            compo_rings_t *rings = (compo_rings_t *)compo;
            icon = compo_rings_select_byidx(rings, func_cb.menu_idx);
            if (icon == NULL) {
                func_cb.menu_idx = func_menu_sub_skyrer_get_first_idx();
                icon = compo_rings_select_byidx(rings, func_cb.menu_idx);
            }
    } else {
//            printf("%s\n", __func__);
//            halt(HALT_GUI_COMPO_ICONLIST_TYPE);
//            return;
            icon = NULL;
    }
    if (func_cb.sta != FUNC_CLOCK || func_cb.menu_style == MENU_STYLE_HONEYCOMB) {
        switch_mode = FUNC_SWITCH_ZOOM_EXIT;
    } else {
        switch_mode = FUNC_SWITCH_ZOOM_FADE_EXIT;
    }

    if (icon == NULL) {
        switch_mode = FUNC_SWITCH_FADE_OUT;
    }

    bool res = func_switching(switch_mode, icon);                               //退出动画
    compo_form_destroy(frm);                                                    //切换完成或取消，销毁窗体
    if (res) {
        func_cb.sta = FUNC_MENU;
    }
#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}


//页面滑动回退功能
void func_backing_to(void)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN
    if (func_cb.sta == FUNC_BT_RING || func_cb.sta == FUNC_BT_CALL) {

        return;
    }

    u8 stack_top = task_stack_pop();

    if (!stack_top) {
#if ELUNCHBOX_PANEL_EN
        stack_top = FUNC_HOME;                                  //异常返回 Home
#else
        stack_top = FUNC_CLOCK;                                 //异常返回表盘
#endif
    }

    if (stack_top == FUNC_MENU
    ) {
        func_switching_to_menu();                               //右滑缓慢退出任务
    } else if (stack_top == FUNC_SMARTSTACK
    ) {
        func_switch_to(stack_top, FUNC_SWITCH_LR_ZOOM_RIGHT);   //返回上一个界面
    } else {
        func_switch_to(stack_top, func_get_switching_mode_byidx(sys_cb.nav_index, false));   //返回上一个界面
    }
    if (func_cb.sta != stack_top) {                             //如果页面没切换需要重新入栈
        task_stack_push(func_cb.sta);
    }
#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}

//页面按键回退功能
void func_back_to(void)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN
    if (func_cb.sta == FUNC_BT_RING || func_cb.sta == FUNC_BT_CALL) {
        return;
    }

    u8 stack_top = task_stack_pop();

    if (!stack_top) {
#if ELUNCHBOX_PANEL_EN
        stack_top = FUNC_HOME;                                  //异常返回 Home
#else
        stack_top = FUNC_CLOCK;                                 //异常返回表盘
#endif
    }

    if (stack_top == FUNC_MENU
    ) {
        func_switch_to_menu();                                  //返回主菜单
    } else if (stack_top == FUNC_SMARTSTACK
    ) {
        func_switch_to(stack_top, FUNC_SWITCH_LR_ZOOM_RIGHT | FUNC_SWITCH_AUTO);  //返回上一个界面
    }  else {
        func_switch_to(stack_top, func_get_switching_mode_byidx(sys_cb.nav_index, false) | FUNC_SWITCH_AUTO);    //返回上一个界面
    }
#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}

//页面直接回退,无动画效果
u8 func_directly_back_to(void)
{
    u8 stack_top = task_stack_pop();

    func_cb.sta = stack_top;

    return stack_top;
}

//static FATFS fat_fs;

void evt_message(size_msg_t msg)
{
    switch (msg) {
        case EVT_READY_EXCHANGE_MTU:
            ble_exchange_mtu_request();
            break;

#if BT_CONNECT_REQ_FROM_WATCH_EN
        case EVT_BT_CONNECT_ONCE:
            bt_connect();
            break;
#endif

#if MUSIC_UDISK_EN
        case EVT_UDISK_INSERT:
            if (dev_is_online(DEV_UDISK)) {
                if (dev_udisk_activation_try(1)) {
                    sys_cb.cur_dev = DEV_UDISK;
                    func_cb.sta = FUNC_MUSIC;
                }
            }
            break;
#endif // MUSIC_UDISK_EN

#if SD_SUPPORT_EN
        case EVT_SD_INSERT:
            if (dev_is_online(DEV_SDCARD)) {
                sys_cb.cur_dev = DEV_SDCARD;
                printf("EVT_SD_INSERT\n");
#if USB_SD_UPDATE_EN
                bsp_sd_disk_mount(24);
                func_update();                                  //尝试升级
#endif // USB_SD_UPDATE_EN

//                CLKGAT0 |= BIT(16);                   //sd0_clken
//                CLKGAT0 |= BIT(21);
//                SD0_LDO_EN();
//
//                sd0_set_speed_mhz(2);
//                FRESULT res = fs_mount (&fat_fs, "B:", 1);
//                printf("f_mount:%d\n", res);

//#if FUNC_MUSIC_EN
//				func_cb.sta = FUNC_MUSIC;
//#endif // FUNC_MUSIC_EN
                msg_enqueue(EVT_SD_CARD_INTER);
            }
            break;

        case EVT_SD_REMOVE:
            if (!dev_is_online(DEV_SDCARD)) {
                printf("EVT_SD_REMOVE\n");

//                FRESULT res = fs_unmount ("B:");
//                printf("fs_unmount:%d\n", res);

                msg_enqueue(EVT_SD_CARD_OUT);
            }
            break;
#endif // SD_SUPPORT_EN

#if FUNC_USBDEV_EN
        case EVT_PC_INSERT:
            if (dev_is_online(DEV_USBPC)) {
                func_cb.sta = FUNC_USBDEV;
            }
            break;
#endif // FUNC_USBDEV_EN

        case EVT_HFP_SET_VOL:
            if(sys_cb.incall_flag & INCALL_FLAG_SCO){
                bsp_change_volume(bsp_bt_get_hfp_vol(sys_cb.hfp_vol));
                printf("HFP SET VOL: %d\n", sys_cb.hfp_vol);
            }
            break;

        case EVT_A2DP_SET_VOL:
            if ((sys_cb.incall_flag & INCALL_FLAG_SCO) == 0) {
                printf("A2DP SET VOL: %d\n", sys_cb.vol);
                bsp_change_volume(sys_cb.vol);
                param_sys_vol_write();
                sys_cb.cm_times = 0;
                sys_cb.cm_vol_change = 1;
            }
            break;

        case EVT_A2DP_MUSIC_PLAY:
            if (func_cb.sta == FUNC_VIDEO_PLAY) {
                bt_audio_bypass();
            } else {
                if (!sbc_is_bypass()) {
                    printf("EVT_A2DP_MUSIC_PLAY\n");
    #if BT_EMIT_EN
                    music_control(MUSIC_MSG_BT_PLAY);
    #endif
                    bsp_sys_unmute();
                }
                bt_cb.music_playing = true;
            }
            break;

        case EVT_A2DP_MUSIC_STOP:
#if !BT_EMIT_EN
            if (!sbc_is_bypass()) {
                printf("EVT_A2DP_MUSIC_STOP\n");
                bsp_sys_mute();
            }
            bt_cb.music_playing = false;
#endif
            break;

        case EVT_BT_SCAN_START:
            if (bt_get_status() < BT_STA_SCANNING) {
                bt_scan_enable();
            }
            break;
#if EQ_DBG_IN_UART || EQ_DBG_IN_SPP
        case EVT_ONLINE_SET_EQ:
            eq_parse_cmd();
            break;
#endif

#if EQ_MODE_EN
        case EVT_BT_SET_EQ:
            music_set_eq_by_num(sys_cb.eq_mode);
            break;
#endif

#if BT_SCO_APP_DBG_EN
        case EVT_SCO_DBG_COMM_RSP:
        case EVT_SCO_DBG_TLV_RSP:
        case EVT_SCO_DBG_NOTIFY:
            sco_app_msg_deal(msg);
            break;
#endif // BT_SCO_APP_DBG_EN

        case EVT_HALT:
            func_cb.sta = FUNC_BT_UPDATE;		//出现异常后要处理事情
            break;

#if FUNC_REC_AUTO_EN
        case EVT_VOX_RECORD_EN: {
            u8 sta = get_bsp_record_sta();
//            printf("EVT_VOX_RECORD_EN: %d\n", sta);
            if (sta == REC_PAUSE) {
                bsp_record_continue();
            } else if (sta == REC_STOP) {
                bsp_record_start(false, bsp_record_get_type(), FUNC_REC_SPR, FUNC_REC_BITRATE);
            }
        } break;

        case EVT_VOX_RECORD_DIS: {
            u8 sta = get_bsp_record_sta();
//            printf("EVT_VOX_RECORD_DIS\n");
            if (sta == REC_RECORDING) {
                bsp_record_pause();
            }
        } break;
#endif // FUNC_REC_AUTO_EN
    }
}

/* 表盘子界面（下拉/侧边/转盘等）仍占用 func_cb.sta==FUNC_CLOCK，但 f_clk->sta!=MAIN。
 * 此时若仍走 func_switch_next / func_switch_to_menu 等，会与 sub_frm、转场状态机冲突，易 WDT。 */
static bool func_clock_subui_active(void)
{
    if (func_cb.sta != FUNC_CLOCK || func_cb.f_cb == NULL) {
        return false;
    }
    return ((f_clock_t *)func_cb.f_cb)->sta != FUNC_CLOCK_MAIN;
}

//func common message process
void func_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_SHORT_LEFT:
        if (func_cb.sta == FUNC_CLOCK) {
            if (!func_clock_subui_active()) {
                func_switch_next(false, true);                    //切到下一个任务
            }
        } else if (func_cb.flag_sort) {
            func_switch_next(false, true);
        }
        break;

    case MSG_CTP_SHORT_RIGHT:
        if (func_cb.sta == FUNC_CLOCK && func_clock_subui_active()) {
            break;
        }
        if (func_cb.flag_sort){
            func_switch_prev(false);                    //切到上一个任务
        } else if(func_cb.menu_style == MENU_STYLE_FOOTBALL) {
            func_back_to();
        } else /*if (ctp_get_sxy().x <= 32)*/ {
            func_backing_to();							//右滑缓慢退出任务
        }
        break;

    case MSG_CTP_COVER:
#if !ELUNCHBOX_KEEP_AWAKE
        sys_cb.sleep_delay = 1; //100ms后进入休眠
#endif
        break;

    case MSG_QDEC_FORWARD:
        if (func_cb.sta == FUNC_CLOCK) {
            if (!func_clock_subui_active()) {
                msg_queue_detach(MSG_QDEC_FORWARD, 0);  //防止不停滚动
                func_switch_next(true, true);                     //切到下一个任务
            }
        } else if (func_cb.flag_sort) {
            msg_queue_detach(MSG_QDEC_FORWARD, 0);
            func_switch_next(true, true);
        }
        break;

    case MSG_QDEC_BACKWARD:
        if (func_cb.sta == FUNC_CLOCK || func_cb.flag_sort) {
            msg_queue_detach(MSG_QDEC_BACKWARD, 0);
            if (func_cb.flag_sort != 0) {
                func_switch_prev(true);                     //切到下一个任务
            }
        }
        break;

    case KU_PREV:
        if (func_cb.sta != FUNC_HEAT) {
            func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
        break;

    case KU_NEXT:
#if !FUNC_RESERVATION_UI_EN
        break;
#elif ELUNCHBOX_PANEL_EN
        /* 饭盒：预约 UI 由 pt8028_take_res_key_pending 专用入口进入 */
        break;
#else
        if (func_cb.sta != FUNC_RESERVATION) {
            func_switch_to(FUNC_RESERVATION, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
#endif
        break;

    case KU_BACK:
#if ELUNCHBOX_PANEL_EN
        if (func_cb.sta == FUNC_HOME) {
            break;
        }
#endif
        if (func_cb.flag_sort) {
            func_switch_to_clock();                     //切换回主时钟
        } else if (func_cb.sta == FUNC_CLOCK) {
            if (!func_clock_subui_active()) {
                func_switch_to_menu();                  //退回到主菜单
            }
        } else {
            func_back_to();								//直接退出任务
        }
        break;

#if SOFT_POWER_ON_OFF
        case KLH_BACK:
        case KLH_LEFT:
        case KLH_RIGHT:
#if ELUNCHBOX_PANEL_EN
            /* 饭盒：开关键长按由 func_elunchbox_pwr_long_poll 软开/软关 */
            break;
#else
            func_cb.sta = FUNC_PWROFF;
            break;
#endif
#endif

    case KU_MODE:
#if ELUNCHBOX_PANEL_EN
            if (func_cb.sta == FUNC_HOME) {
                break;
            }
#endif
            if (func_cb.sta == FUNC_HOME) {
                func_home_mode_key();
            } else if (func_cb.sta != FUNC_HEAT && func_cb.sta != FUNC_MODE &&
                       func_cb.sta != FUNC_SETUP && func_cb.sta != FUNC_RESERVATION &&
                       func_cb.sta != FUNC_TIMEING && func_cb.sta != FUNC_LANGUAGEING &&
                       func_cb.sta != FUNC_VERINFO) {
                func_cb.sta = FUNC_NULL;
            }
            break;

        case KL_BACK:   //堆栈后台
            /* 下拉等子界面内勿切智能堆栈：避免 sub_frm 与切换栈交错导致异常 PC（如 0xfff9xxxx） */
            if (func_cb.sta == FUNC_CLOCK && func_clock_subui_active()) {
                break;
            }
            if (bt_cb.disp_status < BT_STA_INCOMING && func_cb.sta != FUNC_MENUSTYLE) {
                if (func_cb.sta == FUNC_CLOCK) {
                    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
                    /* 仅在主表盘态销毁 sub_frm。FUNC_CLOCK_SUB_* 子界面在自有 while 内仍持有 sub_frm，
                     * 若此处销毁，子界面 exit 会二次 compo_form_destroy -> 堆损坏 / WDT。 */
                    if (f_clk != NULL && f_clk->sta == FUNC_CLOCK_MAIN && f_clk->sub_frm != NULL) {
                        compo_form_destroy(f_clk->sub_frm);     //下拉界面存在双窗体
                    }
                }

                if (func_cb.menu_style == MENU_STYLE_FOOTBALL) {
                    func_switching(FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO, NULL);
                    func_cb.sta = FUNC_SMARTSTACK;
                } else {
                    func_switch_to(FUNC_SMARTSTACK, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);
                }
            }
            break;


//        case KU_LEFT:
//            ble_bt_connect();               //ios一键双连测试
//            printf("send sm req\n");
//            break;

//        case KU_LEFT:
//            bt_audio_enable();
//            printf("bt_audio_enable\n");
//            break;
//
//        case KU_RIGHT:
//            bt_audio_bypass();
//            printf("bt_audio_bypass\n");
//            break;

//#if EQ_MODE_EN
//        case KU_EQ:
//            sys_set_eq();
//            break;
//#endif // EQ_MODE_EN
//
//        case KU_MUTE:
//            if (sys_cb.mute) {
//                bsp_sys_unmute();
//            } else {
//                bsp_sys_mute();
//            }
//            break;
//
        case MSG_SYS_500MS:
            break;

        case MSG_SYS_1S:
#if CALL_MGR_EN
            bsp_bt_call_times_inc();
            bsp_modem_call_times_inc();
#endif
#if BT_HFP_BAT_REPORT_EN
            bt_hfp_report_bat();
#endif
            break;

        // case EVT_4G_INIT_DONE:
        //     modem_test_machine();
        //     break;
#if BT_EMIT_EN
        case EVT_BT_DISCONNECT:
            if (func_music_is_play()) {
                func_music_play(false);
            }
            break;
#endif // BT_EMIT_EN

        default:
            evt_message(msg);
            break;
    }

#if (ASR_SELECT == ASR_YJ && !ASR_DEAL_TYPE)
    third_func_message(msg);
#endif

    //调节音量，3秒后写入flash
    if ((sys_cb.cm_vol_change) && (sys_cb.cm_times >= 6)) {
        sys_cb.cm_vol_change = 0;
        cm_sync();
    }
}

///进入一个功能的总入口
extern u8 heap_func[HEAP_FUNC_SIZE] AT(.heap.func);

AT(.text.func)
void func_enter(void)
{
    //检查Func Heap
    u32 heap_size = func_heap_get_free_size();
    if (heap_size != HEAP_FUNC_SIZE) {
        printf("Func heap leak (%d -> %d): %d\n", func_cb.last, func_cb.sta, heap_size);
        printf("To Reset Func heap:%d\n", HEAP_FUNC_SIZE);
        func_heap_init(heap_func, HEAP_FUNC_SIZE);
//        halt(HALT_FUNC_HEAP);
    }

//    gui_box_clear();
    param_sync();
    reset_sleep_delay_all();
    reset_pwroff_delay();
    func_cb.mp3_res_play = mp3_res_play;
    func_cb.set_vol_callback = NULL;
//    bsp_clr_mute_sta();
//    sys_cb.voice_evt_brk_en = 1;    //播放提示音时，快速响应事件。
    AMPLIFIER_SEL_D();
#if ELUNCHBOX_PANEL_EN
    func_key_lock_on_page_change();
#endif
}

AT(.text.func)
void func_exit(void)
{
#if ASR_SELECT
    bsp_asr_voice_wake_sta_clr();
#endif
    //销毁窗体
    if (func_cb.frm_main != NULL) {
#if ELUNCHBOX_PANEL_EN
        home_gpu_wait_idle();
#endif
        compo_form_destroy(func_cb.frm_main);
#if ELUNCHBOX_PANEL_EN
        home_gpu_wait_idle();
        compos_init();
        home_gpu_wait_idle();
#endif
    }
    //释放FUNC控制结构体
    if (func_cb.f_cb != NULL) {
        func_free(func_cb.f_cb);
    }
    func_cb.frm_main = NULL;
    func_cb.f_cb = NULL;

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    if (func_cb.last == FUNC_ALIPAY) {
        void gui_sw_init(void);
        gui_sw_init();
        printf("gui_reinit\n");
    }
#endif

}

AT(.text.func)
void func_run(void)
{
#if CPU_USAGE_MONITOT_EN
    cpu_trace_monitor_init();
#endif

    void (*func_entry)(void) = NULL;
    printf("%s\n", __func__);
    memset(func_cb.tbl_sort, 0, sizeof(func_cb.tbl_sort));
#if ELUNCHBOX_PANEL_EN
    func_cb.tbl_sort[0] = FUNC_HOME;
    func_cb.sort_cnt = 1;
    func_cb.flag_sort = false;
    func_cb.sta = FUNC_HOME;
#else
    func_cb.tbl_sort[0] = FUNC_HOME;
    func_cb.tbl_sort[1] = FUNC_VIDEO_SHOWLIST;
    func_cb.tbl_sort[2] = FUNC_ACTIVITY;
    func_cb.tbl_sort[3] = FUNC_SLEEP;
    func_cb.tbl_sort[4] = FUNC_BLOOD_OXYGEN;
    func_cb.tbl_sort[5] = FUNC_BT;
    func_cb.tbl_sort[6] = FUNC_COMPO_SELECT;
    func_cb.sort_cnt = 7;
    func_cb.sta = DEFAULE_START_FUNC;
#endif
    task_stack_init();  //任务堆栈
    latest_task_init(); //最近任务
    // func.c
    
    for (;;) {
#if !ELUNCHBOX_PANEL_EN
        printf("func_enter <<\n");
#endif
        func_enter();
#if !ELUNCHBOX_PANEL_EN
        printf("pwrkey usage_id: %d\n", bsp_pwrkey_get_usage_id());
#endif
        for (int i = 0; i < FUNC_ENTRY_CNT; i++) {
            if (tbl_func_entry[i].func_idx == func_cb.sta) {
                task_stack_push(func_cb.sta);
                latest_task_add(func_cb.sta);
                func_entry = tbl_func_entry[i].func;
                func_entry();
                break;
            }
        }
#if !ELUNCHBOX_PANEL_EN
        printf("func_cb.sta:%d\n", func_cb.sta);
        if (func_cb.sta == FUNC_PWROFF) {
            printf("func_pwroff <<\n");
            func_pwroff(1);
            printf("func_pwroff >>\n");
        }
        printf("func_exit <<\n");
#endif
        if (func_cb.sta == FUNC_PWROFF) {
            func_pwroff(1);
        }
        func_exit();
#if !ELUNCHBOX_PANEL_EN
        printf("func_exit >>\n");
#endif
    }
}
