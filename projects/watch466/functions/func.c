#include "include.h"
#include "func_tbl.h"
#include "func.h"
#include "new_ui/ui.h"
//#include "func_reservation.h"
#if USER_PT8028_KEY
#include "bsp_pt8028_key.h"
#endif
#if USER_PANEL_LED
#include "func_led.h"
#endif

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

void home_gpu_wait_idle(void)
{
    os_gui_draw_w4_done();
    os_gui_draw_w4_done();
}

func_cb_t func_cb AT(.buf.func_cb);
#if ELUNCHBOX_PANEL_EN
u8 func_res_allow_switch;
#endif

bool gui_get_auto_power_en(void);

#if ELUNCHBOX_PANEL_EN
static bool func_elunchbox_res_key_page_ok(void)
{
    switch (func_cb.sta) {
    case FUNC_HOME:
    case FUNC_MODE:
    case FUNC_SETUP:
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
    func_res_allow_switch = 1;
    func_switch_to(FUNC_RESERVATION, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
    func_res_allow_switch = 0;
}

void func_elunchbox_res_key_poll(void)
{
#if USER_PT8028_KEY && FUNC_RESERVATION_UI_EN
    if (sys_cb.flag_swithing) {
        return;
    }
    if (pt8028_take_res_key_pending() && func_elunchbox_res_key_page_ok()) {
        func_elunchbox_switch_to_reservation();
    }
#endif
}
#endif

AT(.text.func.process)
void func_process(void)
{

   if (gui_get_auto_power_en()) {
        sys_clk_req(INDEX_GUI, SYS_192M);
   }

    WDT_CLR();

#if USER_PT8028_KEY
    pt8028_gpio_ensure_periodic();
#if ELUNCHBOX_PANEL_EN
    pt8028_key_scan();

#endif
    pt8028_log_flush();
    pt8028_poll_reinit();
#if PT8028_GPIO_MONITOR_EN
    pt8028_gpio_monitor();
#endif
#endif

#if (UART0_PRINTF_SEL != PRINTF_NONE)
    uart0_printf_ensure();
#endif

#if USER_PANEL_LED
    func_led_scan();                        /* 主线程刷新 LED，勿放 5ms 中断(易花屏) */
#endif

#if CPU_USAGE_MONITOT_EN
    cpu_trace_monitor();
#endif

#if (SD_SUPPORT_EN) && SD_SOFT_DETECT_EN
    sd_soft_cmd_detect(120);
#endif

    tft_bglight_frist_set_check();

    // gui 没有休眠才更新
	if (!sys_cb.gui_sleep_sta && !sys_cb.flag_halt) {
#if ELUNCHBOX_PANEL_EN
        bool gui_do_refresh = true;

        if (sys_cb.flag_swithing) {
            gui_do_refresh = false;
        }
        compo_update();
        if (gui_do_refresh) {
            gui_process();
        }
#if USER_PT8028_KEY && FUNC_RESERVATION_UI_EN
        func_elunchbox_res_key_poll();
#endif
#else
        compo_update();                                     //更新组件
        gui_process();                                      //刷新UI
#endif
    }

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
        bt_thread_check_trigger();
#if LE_EN
        ble_app_process();
#endif
    }

    /* 串口上报低电 → 切换到低电页面 */
    if (g_ui_sys.lowbat && func_cb.sta != FUNC_LOWBAT && !sys_cb.flag_swithing) {
        func_cb.sta = FUNC_LOWBAT;
    }

   if (gui_get_auto_power_en()) {
        sys_clk_free(INDEX_GUI);
   }

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
        return;
    }
#endif
    if (mode != FUNC_SWITCH_FADE_OUT) {
        frm = func_create_form(sta);                                  //创建下一个任务的窗体
    }

    bool res = func_switching(switch_mode, NULL);                     //切换动画

    if (frm) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }

    if (res) {
        if (sta == FUNC_CLOCK) {
            func_cb.flag_sort = false;
        }
        func_cb.sta = sta;
    }
}

//切换到下一个任务
void func_switch_next(bool flag_auto, bool flag_loop)
{
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
        return;
    }
#endif

    if (mode != FUNC_SWITCH_FADE_OUT) {
        frm = func_create_form(sta);                                  //创建下一个任务的窗体
    }

    bool res = func_switching(switch_mode, NULL);                     //切换动画

    if (frm) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }

    if (res) {
        if (sta == FUNC_CLOCK) {
            func_cb.flag_sort = false;
        } else {
            func_cb.flag_sort = true;
        }
        func_cb.sta = sta;
    }
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
#endif

    compo_form_t *frm = NULL;

    u8 mode = switch_mode & 0x7fff;
    if (mode != FUNC_SWITCH_FADE_OUT) {
        frm = func_create_form(sta);                                  //创建下一个任务的窗体
    }

    bool res = func_switching(switch_mode, NULL);         			  //切换动画

#if ELUNCHBOX_PANEL_EN
    if (res) {
        home_gpu_wait_idle();
    }
#endif

    if (frm) {
        compo_form_destroy(frm);                                      //切换完成或取消，销毁窗体
    }

    if (res) {
        func_cb.sta = sta;
    }
}

/* 手表表盘/菜单切换已移除：饭盒统一回 Home */
void func_switch_to_clock(void)
{
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
    func_cb.flag_sort = false;
}

void func_switch_to_menu(void)
{
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

void func_switch_to_football_menu(void)
{
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

void func_switching_to_menu(void)
{
    func_switch_to(FUNC_HOME, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
}

//页面滑动回退功能
void func_backing_to(void)
{
    if (func_cb.sta == FUNC_BT_RING || func_cb.sta == FUNC_BT_CALL) {
        return;
    }

    u8 stack_top = task_stack_pop();

    if (!stack_top || stack_top == FUNC_MENU || stack_top == FUNC_SMARTSTACK) {
        stack_top = FUNC_HOME;
    }

    func_switch_to(stack_top, func_get_switching_mode_byidx(sys_cb.nav_index, false));
    if (func_cb.sta != stack_top) {
        task_stack_push(func_cb.sta);
    }
}

//页面按键回退功能
void func_back_to(void)
{
    if (func_cb.sta == FUNC_BT_RING || func_cb.sta == FUNC_BT_CALL) {
        return;
    }

    u8 stack_top = task_stack_pop();

    if (!stack_top || stack_top == FUNC_MENU || stack_top == FUNC_SMARTSTACK) {
        stack_top = FUNC_HOME;
    }

    func_switch_to(stack_top, func_get_switching_mode_byidx(sys_cb.nav_index, false) | FUNC_SWITCH_AUTO);
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
                    bsp_sys_unmute();
                }
                bt_cb.music_playing = true;
            }
            break;

        case EVT_A2DP_MUSIC_STOP:
            if (!sbc_is_bypass()) {
                printf("EVT_A2DP_MUSIC_STOP\n");
                bsp_sys_mute();
            }
            bt_cb.music_playing = false;
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

//func common message process
void func_message(size_msg_t msg)
{
#if ELUNCHBOX_PANEL_EN
    /* new_ui pages handle navigation via func_key, not old-style msg queue */
    if (func_cb.sta == FUNC_HOME || func_cb.sta == FUNC_HOME_PAGE
        || func_cb.sta >= FUNC_NEW_HEAT_SET) {
        switch (msg) {
        case MSG_CTP_SHORT_LEFT:
        case MSG_CTP_SHORT_RIGHT:
        case KU_BACK:
        case KU_PREV:
        case MSG_QDEC_FORWARD:
        case MSG_QDEC_BACKWARD:
            return;
        default:
            break;
        }
    }
#endif
    switch (msg) {
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_QDEC_FORWARD:
    case MSG_QDEC_BACKWARD:
        /* 手表触控/编码器切任务已移除 */
        break;

    case MSG_CTP_COVER:
#if !ELUNCHBOX_KEEP_AWAKE
        sys_cb.sleep_delay = 1; //100ms后进入休眠
#endif
        break;

    case KU_PREV:
        if (func_cb.sta != FUNC_HEAT) {
            func_switch_to(FUNC_HEAT, FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO);
        }
        break;

    case KU_NEXT:
        /* 饭盒：预约 UI 由 pt8028_take_res_key_pending 专用入口进入 */
        break;

    case KU_BACK:
        if (func_cb.sta == FUNC_HOME) {
            break;
        }
        func_back_to();
        break;

#if SOFT_POWER_ON_OFF
        case KLH_BACK:
        case KLH_LEFT:
        case KLH_RIGHT:
            func_cb.sta = FUNC_PWROFF;
            break;
#endif

    case KU_MODE:
            if (func_cb.sta == FUNC_HOME) {
                break;
            }
            if (func_cb.sta != FUNC_MODE &&
                       func_cb.sta != FUNC_SETUP && func_cb.sta != FUNC_RESERVATION &&
                       func_cb.sta != FUNC_TIMEING && func_cb.sta != FUNC_LANGUAGEING &&
                       func_cb.sta != FUNC_VERINFO) {
                func_cb.sta = FUNC_NULL;
            }
            break;

        case MSG_SYS_500MS:
            break;

        case MSG_SYS_1S:
#if BT_HFP_BAT_REPORT_EN
            bt_hfp_report_bat();
#endif
            break;

        default:
            evt_message(msg);
            break;
    }

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
}

AT(.text.func)
void func_exit(void)
{
    //销毁窗体
    if (func_cb.frm_main != NULL) {
        home_gpu_wait_idle();
        compo_form_destroy(func_cb.frm_main);
    }
    //释放FUNC控制结构体
    if (func_cb.f_cb != NULL) {
        func_free(func_cb.f_cb);
    }
    func_cb.frm_main = NULL;
    func_cb.f_cb = NULL;
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
    func_cb.tbl_sort[0] = FUNC_HOME;
    func_cb.sort_cnt = 1;
    func_cb.flag_sort = false;
    func_cb.sta = FUNC_HOME;
    task_stack_init();  //任务堆栈
    latest_task_init(); //最近任务

    for (;;) {
        func_enter();
        for (int i = 0; i < FUNC_ENTRY_CNT; i++) {
            if (tbl_func_entry[i].func_idx == func_cb.sta) {
                task_stack_push(func_cb.sta);
                latest_task_add(func_cb.sta);
                func_entry = tbl_func_entry[i].func;
                func_entry();
                break;
            }
        }
        if (func_cb.sta == FUNC_PWROFF) {
            func_pwroff(1);
        }
        func_exit();
    }
}
