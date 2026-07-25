#ifndef _FUNC_H
#define _FUNC_H

#include "common/msgbox.h"
#include "common/listbox.h"
#include "common/rotary.h"
#include "common/func_switching.h"
#include "func_clock.h"
#include "func_bt.h"
#include "common/func_idle.h"
#include "common/func_lowpwr.h"
#include "common/func_manage.h"
#include "common/func_update.h"
#if FUNC_BLE_GATTS_EN
#include "func_ble_gatts.h"
#endif
#if FUNC_LUNCHBOX_UART_EN
#include "func_lunchbox_uart.h"
#endif

#define TICK_IGNORE_KEY            700      //忽略700ms内的部分消息

//task number
enum {
    FUNC_NULL = 0,
    FUNC_MENU,                          //主菜单
    FUNC_MENUSTYLE,                     //主菜单样式选择
    FUNC_CLOCK,                         //时钟表盘
    FUNC_CLOCK_PREVIEW,                 //时钟表盘预览
    FUNC_SIDEBAR,                       //表盘右滑
    FUNC_CARD,                          //表盘上拉
    FUNC_HEARTRATE,                     //心率
    FUNC_BT,                            //蓝牙播放器(控制手机音乐)
	FUNC_COMPO_SELECT,                  //组件选择
    FUNC_COMPO_SELECT_SUB,              //组件选择子界面
    FUNC_BT_RING,                       //来电界面
    FUNC_BT_CALL,                       //通话界面
    FUNC_ALARM_CLOCK,			        //闹钟
    FUNC_ALARM_CLOCK_SUB_SET,			//闹钟--设置
    FUNC_ALARM_CLOCK_SUB_REPEAT,		//闹钟--重复
    FUNC_ALARM_CLOCK_SUB_EDIT,		    //闹钟--编辑
    FUNC_BLOOD_OXYGEN,			        //血氧
    FUNC_BLOODSUGAR,                    //血糖
    FUNC_BLOOD_PRESSURE,                //血压
    FUNC_BREATHE,				        //呼吸
    FUNC_CALCULATOR,			        //计算器
    FUNC_CAMERA,				        //相机
    FUNC_LIGHT,					        //亮度调节
    FUNC_TIMER,					        //定时器
    FUNC_SLEEP,					        //睡眠
    FUNC_STOPWATCH,				        //秒表
    FUNC_STOPWATCH_SUB_RECORD,			//秒表--秒表记录
    FUNC_WEATHER,				        //天气
    FUNC_GAME,                          //游戏
    FUNC_STYLE,                         //菜单风格
    FUNC_ALTITUDE,                      //海拔
    FUNC_MAP,                           //地图
    FUNC_MESSAGE,                       //消息
    FUNC_SCAN,                          //扫一扫
    FUNC_VOICE,                         //语音助手
#if SECURITY_PAY_EN
    FUNC_ALIPAY,                        //支付宝
#endif // SECURITY_PAY_EN
    FUNC_COMPASS,                       //指南针
    FUNC_ADDRESS_BOOK,                  //电话簿
    FUNC_CALL_SUB_LINKMAN = FUNC_ADDRESS_BOOK,
    FUNC_SPORT,				            //运动
    FUNC_SPORT_CONFIG,                  //运动配置
    FUNC_SPORT_SUB_RUN,                 //运动--室内跑步
    FUNC_SPORT_SWITCH,                  //运动开启
    FUNC_CALL,                          //电话
    FUNC_CALL_SUB_RECORD,               //电话-最近通话
    FUNC_CALL_SUB_DIAL,                 //电话-拨打电话
    FUNC_FINDPHONE,                     //寻找手机
    FUNC_CALENDAER,                     //日历
    FUNC_VOLUME,                        //音量
    FUNC_ACTIVITY,                      //活动记录
    FUNC_FLASHLIGHT,                    //手电筒
    FUNC_BRIGHTNESS,                    //亮度（足球菜单全屏图标）
    FUNC_SETTING,				        //设置
    FUNC_SET_SUB_DOUSING,               //设置--熄屏
    FUNC_SET_SUB_WRIST,                 //设置--抬腕
    FUNC_SET_SUB_DISTURD,               //设置--勿扰
    FUNC_DISTURD_SUB_SET,               //勿扰--时间设置
    FUNC_SET_SUB_SAV,                   //设置--声音与振动
    FUNC_SET_SUB_LANGUAGE,              //设置--语言
    FUNC_LANGUAGE = FUNC_SET_SUB_LANGUAGE,
    FUNC_SET_SUB_TIME,                  //设置--时间
    FUNC_SET_SUB_MENU_NAVIGATION,       //设置--菜单导航样式
    FUNC_TIME_SUB_CUSTOM,               //调整日期
    FUNC_SET_SUB_PASSWORD,              //设置--密码锁
    FUNC_PASSWORD_SUB_DISP,             //新密码锁设置
    FUNC_PASSWORD_SUB_SELECT,           //确认密码锁
    FUNC_SET_SUB_ABOUT,                 //设置--关于
    FUNC_SET_SUB_4G,                    //设置--4G
    FUNC_SET_SUB_RESTART,               //设置--重启
    FUNC_RESTART = FUNC_SET_SUB_RESTART,//重启
    FUNC_SET_SUB_RSTFY,                 //设置--恢复出厂
    FUNC_RSTFY = FUNC_SET_SUB_RSTFY,
    FUNC_SET_SUB_OFF,                   //设置--关机
    FUNC_OFF = FUNC_SET_SUB_OFF,        //关机
    FUNC_MUSIC_SRC,
#if FUNC_MUSIC_EN
    FUNC_MUSIC,
#endif
#if FUNC_FMRX_EN
    FUNC_FMRX,
#endif
    FUNC_BTHID,
#if FUNC_USBDEV_EN
    FUNC_USBDEV,
#endif
    FUNC_AUX,
    FUNC_SPDIF,
    FUNC_SPEAKER,
    FUNC_PWROFF,
    FUNC_SLEEPMODE,
    FUNC_I2S,
#if FUNC_RECORDER_EN
    FUNC_RECORDER,
#endif
    FUNC_BT_DUT,
    FUNC_BT_UPDATE,
    FUNC_IDLE,
    FUNC_CHARGE,
    FUNC_DEBUG_INFO,
    FUNC_SMARTSTACK,
    FUNC_MODEM_CALL,
    FUNC_MODEM_RING,
    FUNC_MESSAGE_REPLY,                  //消息发送
    FUNC_MIC_TEST,
    FUNC_EMIT_LIST,
#if FUNC_BLE_GATTS_EN
    FUNC_BLE_GATTS,                //BLE GATTS Demo
#endif
    FUNC_BIRD,
#if FUNC_GAME_TETRIS_EN
    FUNC_GAME_TETRIS,           //俄罗斯方块
    FUNC_GAME_TETRIS_START,
    FUNC_GAME_TETRIS_OVER,
#endif // FUNC_GAME_TETRIS_EN

    FUNC_VIDEO_PLAY,
    FUNC_PHOTO_VIEW,
    FUNC_VIDEO_SHOWLIST,
    FUNC_VIDEO_RECODE,
    FUNC_TAKE_PHOTO,
    FUNC_GIF,

#if LE_AB_FOT_EN
    FUNC_OTA_UI_MODE,
#endif
    FUNC_HEAT,                          //加热页
    FUNC_HOME,                          //默认主页
    FUNC_MODE,                          //模式页
    FUNC_NEW_HEAT,                      //新主页→加热页
    FUNC_NEW_WARM,                      //新主页→保温页
    FUNC_NEW_MODE,                      //新主页→模式页
    FUNC_NEW_SETUP,                     //新主页→设置页
    FUNC_NEW_LANGUAGE,                  //新主页→语言页
    FUNC_NEW_VERINFO,                   //新主页→版本信息页
    FUNC_NEW_TIME,                      //新主页→时间页
    FUNC_LID_CONFIRM,                   //上盖开启确认弹窗
    FUNC_RESERVATION,                   //预约页
    FUNC_SETUP,                         //设置页
    FUNC_TIMEING,                       //定时页
    FUNC_LANGUAGEING,                   //语言页
    FUNC_VERINFO,                       //版本信息页
    FUNC_LOWBAT,                        //低电模式（全屏 didian 图标）
    FUNC_HOME_PAGE,                     //新UI主页（new_ui）
    FUNC_MAX_NUM,           //用于计数

};

/** @brief 加热自然结束后：UART 开启保温(140°F)并进入模式页 Insulation 界面 */
void func_mode_keep_warm_enter(void);
bool func_mode_ui_is_heating(void);

//task control block
typedef struct {
    void *f_cb;                                     //当前任务控制指针
    compo_form_t *frm_main;                         //当前窗体
    void *msg_cb;                                   //对话框控制指针
    u32 enter_tick;                                 //记录进入func的tick
    u8 sta;                                         //cur working task number
    u8 last;                                        //lask task number
    u8 menu_style;                                  //菜单样式
    u8 menu_idx;                                    //菜单编号
    int32_t menu_idx_angle;                         //菜单编号角度
    u8 sta_break;                                   //被中断的任务
    u8 sort_cnt;                                    //快捷任务个数
    u8 tbl_sort[MAX_FUNC_SORT_CNT];                 //快捷任务表
    u8  flag_sort       : 1,                        //已进入快捷任务
        flag_animation  : 1;                        //入场动画

    void (*mp3_res_play)(u32 addr, u32 len);        //各任务的语音播报函数接口
    void (*set_vol_callback)(u8 dir);               //设置音量的回调函数，用于各任务的音量事件处理。
} func_cb_t;


extern func_cb_t func_cb;
extern const u8 func_sort_table[];     //任务切换排序table

ALWAYS_INLINE void func_mp3_res_play(u32 addr, u32 len)
{
    if (func_cb.mp3_res_play) {
        func_cb.mp3_res_play(addr, len);
    }
}

ALWAYS_INLINE void func_set_vol_callback(u8 dir)
{
    if (func_cb.set_vol_callback) {
        func_cb.set_vol_callback(dir);
    }
}

void func_run(void);
compo_form_t *func_create_form(u8 sta);     //根据任务名创建窗体

void func_cur_sta_exit(void);

void func_switch_prev(bool flag_auto);
void func_switch_next(bool flag_auto,bool flag_loop);

u8 get_funcs_total(void);
void func_process(void);
void func_message(size_msg_t msg);
void evt_message(size_msg_t msg);

void func_switch_to(u8 sta, u16 switch_mode);
void func_switch_to_football_menu(void);
void func_switching_to_menu(void);
void func_backing_to(void);                     //页面滑动回退功能
void func_back_to(void);                        //页面按键回退功能
u8 func_directly_back_to(void);                 //页面直接回退,无动画效果

bool func_video_allow_warning_tone(void);

#if ELUNCHBOX_PANEL_EN
void func_home_gui_mark_dirty(void);
bool func_home_gui_need_refresh(void);
extern u8 func_res_allow_switch;
void func_elunchbox_switch_to_reservation(void);
void func_elunchbox_res_key_poll(void);
void func_elunchbox_switch_to_heat(void);
/** BLE 0x04 加热指令：直达 func_heat_panel（参数已由 lb_mode_to_heat_set 预设） */
void func_elunchbox_switch_to_heat_panel(void);
/** BLE 0x04 保温指令：跳转 func_new_warm */
void func_elunchbox_switch_to_warm_panel(void);
/** BLE 停止加热：回 Home 主界面 */
void func_elunchbox_switch_to_home(void);
/** 取消 BLE 延后切页（停止加热时须调用） */
void func_elunchbox_ble_cancel_pending_switch(void);
/** UART/BLE 停止加热并回 Home（DP10=0 或 DP02=0） */
void func_elunchbox_uart_stop_and_home(void);
void func_heat_prepare_ble_stop(void);
/** 加热面板自然结束：UART 开保温 + 切 func_new_warm */
void func_elunchbox_enter_warm_from_heat(void);
/** 充电中开始/进行加热：切保温页并标记（拔电后退出指令回 Home） */
void func_elunchbox_enter_warm_from_charging(void);
bool func_elunchbox_warm_from_charging(void);
void func_elunchbox_warm_from_charging_set(bool on);
/** 充电中刷新保温页充电图标（不再主动切页，跳页由 UART DP 驱动） */
bool func_elunchbox_charging_redirect_warm(void);
/** 黑屏充电跑马灯页（关机态插电 / 亮屏充电后关机） */
bool elunchbox_charge_off_active(void);
void func_elunchbox_enter_charge_off_page(void);
void func_heat_ble_remote_restart(void);
void func_new_warm_ble_restart(void);
void func_heat_key_poll(void);
bool func_heat_ui_is_heating(void);
/** 充电进保温前加热页须就绪（避免切页未完成时改 sta 导致 GPU 崩溃） */
bool func_heat_panel_ready_for_charge_warm(void);
/** UART 侧判定加热结束是否可信（heat_live_ready 后才行，过滤开局残留 DP） */
bool func_heat_uart_finish_ok(void);
bool elunchbox_pwr_gui_off_is_on(void);
bool elunchbox_ui_is_live(void);
bool elunchbox_pwr_is_manual_off(void);
bool elunchbox_pwr_manual_off_wake_pressing(void);
/** 手动关机唤醒：TCH5 被按住时应阻止深度休眠，让 tick_get() 正常推进完成 2s 长按检测 */
bool elunchbox_pwr_manual_off_should_stay_awake(void);
/** 关机态 PB9/UART 门铃唤醒后，保持清醒数秒收充电包 */
void elunchbox_manual_off_uart_listen_arm(void);
bool elunchbox_manual_off_uart_listening(void);
void elunchbox_pwr_gui_off_activate(void);
bool elunchbox_is_device_powered(void);
void elunchbox_pwr_gui_wake(void);
void elunchbox_pwr_gui_wake_reason(const char *reason);
/** 蓝牙 0x04 总开关：关=息屏+加热模块断电(保持 BLE)；开=唤醒+上电 */
void elunchbox_pwr_ble_switch(bool on);
/** manual_off 时仅允许 intentional wake 调用 gui_wakeup */
bool elunchbox_pwr_manual_off_gui_wake_ok(void);
/** 设置 intentional_wake 标志，sfunc_sleep 内调 gui_wakeup 前须置 true */
void elunchbox_pwr_intentional_wake_set(bool on);
/** 关机进黑屏充电页：恢复 GPU，暂不开背光（切页后再亮） */
void elunchbox_pwr_wake_for_charge_off(void);
void elunchbox_manual_off_sleep_poll(void);
bool elunchbox_manual_wake_pending_take(void);
bool elunchbox_manual_wake_pending_peek(void);
void elunchbox_panel_boot_power_on(void);
void elunchbox_guioff_sleep_delay_reset(void);
void elunchbox_guioff_sleep_delay_tick(void);
bool elunchbox_guioff_sleep_ready(void);
void elunchbox_guioff_sleep_delay_rearm(void);
void elunchbox_guioff_sleep_service(void);
bool elunchbox_guioff_in_sleep_mode(void);
void elunchbox_guioff_sleep_mode_enter(void);
void elunchbox_guioff_sleep_post_wake(bool key_wake);
/** 任意按键操作：重置无操作息屏计时（亮屏状态下） */
void elunchbox_user_activity_reset(void);
/** 100ms tick：独立息屏倒计时（不受 bsp_key reset_sleep_delay_all 干扰） */
void elunchbox_guioff_idle_tick(void);
/** 独立倒计时是否已到 0（应息屏） */
bool elunchbox_guioff_idle_expired(void);
/** 加热/保温/倒计时进行中：禁止自动息屏 */
bool elunchbox_heating_blocks_idle(void);
/** guioff 唤醒后强制刷新 Home 的 tab/status/共享图标等绑定，避免 C24x */
void func_home_force_ui_refresh_after_wake(void);
void func_home_switch_to_reservation(void);
void func_home_drain_stale_key_msgs(void);
bool func_home_heating_countdown_active(void);
void func_mode_idle_preload_reset(void);
void func_mode_idle_preload_step(void);
#include "func_key_lock.h"
#endif

#endif // _FUNC_H
