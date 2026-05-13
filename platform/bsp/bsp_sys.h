#ifndef _BSP_SYS_H
#define _BSP_SYS_H

#define VOL_MAX                         xcfg_cb.vol_max   //最大音量级数

//控制位汇总
#define SYS_CTL_BT                      0               //蓝牙开关
#define SYS_CTL_CALL_RING               1               //来电铃声
#define SYS_CTL_ACLOCK_MON              2               //闹钟设置星期一
#define SYS_CTL_ACLOCK_TUE              3               //闹钟设置星期二
#define SYS_CTL_ACLOCK_WED              4               //闹钟设置星期三
#define SYS_CTL_ACLOCK_THU              5               //闹钟设置星期四
#define SYS_CTL_ACLOCK_FRI              6               //闹钟设置星期五
#define SYS_CTL_ACLOCK_SAT              7               //闹钟设置星期六
#define SYS_CTL_ACLOCK_SUN              8               //闹钟设置星期日
#define SYS_CTL_ACLOCK1_ON              9               //闹钟1开关
#define SYS_CTL_ACLOCK2_ON              10              //闹钟2开关
#define SYS_CTL_ACLOCK3_ON              11              //闹钟3开关
#define SYS_CTL_FUNC_SPORT_ON           12              //运动功能界面选择
#define SYS_CTL_FUNC_SLEEP_ON           13              //睡眠功能界面选择
#define SYS_CTL_FUNC_ACTIVITY_ON        14              //活动数据功能界面选择
#define SYS_CTL_FUNC_HEART_ON           15              //心率功能界面选择
#define SYS_CTL_FUNC_SPO2_ON            16              //血氧功能界面选择
#define SYS_CTL_FUNC_HRV_ON             17              //血压功能界面选择b
#define SYS_CTL_FUNC_BT_CALL_ON         18              //电话功能界面选择
#define SYS_CTL_FUNC_WEATHER_ON         19              //天气功能界面选择
#define SYS_CTL_FUNC_MUSIC_ON           20              //音乐功能界面选择
#define SYS_CTL_FUNC_BREATHE_ON         21              //呼吸训练功能界面选择
#define SYS_CTL_FUNC_SMS_ON             22              //消息功能界面选择
#define SYS_CTL_FUNC_GAME_ON            23              //游戏功能界面选择
#define SYS_CTL_FUNC_CALCUL_ON          24              //计算器功能界面选择
#define SYS_CTL_FUNC_ALARM_ON           25              //闹钟功能界面选择
#define SYS_CTL_FUNC_TIMER_ON           26              //定时器功能界面选择
#define SYS_CTL_FUNC_STODWATCH_ON       27              //秒表功能界面选择
#define SYS_CTL_FUNC_SETTINGS_ON        28              //设置功能界面选择
#define SYS_CTL_FUNC_SPORT_RECORD_ON    29              //运动记录功能界面选择




#define SYS_CTL_TOTAL_BITS              256
#define SYS_CTL_TOTAL_BYTES             ((SYS_CTL_TOTAL_BITS + 7) / 8)      //系统控制位

//功能
#define ALARM_CLOCK_NUM_MAX             3               //闹钟最大数量，需要同步修改列表item
#define STOPWATCH_REC_NUM_MAX           10              //秒表最大数量，需要同步修改列表item
#define WEATHER_CNT                     8               //天气类型数量

typedef struct {
    //控制位
    u8 ctl_bits[SYS_CTL_TOTAL_BYTES];   //控制位汇总
    u8  play_mode;
    u8  vol;
    u8  hfp_vol;
    u8  eq_mode;
    u8  cur_dev;
    u8  lang_id;
    u8  lpwr_warning_cnt;
    u8  lpwr_warning_times;     //播报低电次数，0xff表示一直播
    u8  vbat_nor_cnt;
    s8  gain_offset;            //用于动态修改系统音量表
    u8  hfp2sys_mul;            //系统音量与HFP音量倍数，手机HFP音量只有16级。
    u8  lpwr_cnt;               //低电计数
#if DAC_DNR_EN
    u8  dnr_sta;                //动态降噪状态
#endif
    u16 vbat;                   //当前电压值(mv)
    u16 vbat_percent;           //电量百分比
    u16 kh_vol_msg;
	u32 vusb;
    u32 sleep_time;             //配置工具配置系统息屏时间
    u32 pwroff_time;            //配置工具配置自动关机时间
    u32 sleep_delay;            //系统休眠时间, 深度睡眠计时
    u32 guioff_delay;           //系统息屏时间, 浅度睡眠计时
    u32 pwroff_delay;
    u32 sleep_wakeup_time;
    u32 sleep_counter;
    u32 ms_ticks;               //ms为单位
    u32 rand_seed;

volatile u8  cm_times;
volatile u8  loudspeaker_mute;  //功放MUTE标志
volatile u8  charge_sta;        //0:充电关闭， 1：充电开启， 2：充满
volatile u8  charge_bled_flag;  //charge充满蓝灯常亮标志
volatile u8  ch_bled_cnt;       //charge充满蓝灯亮时间计数
volatile u8  poweron_flag;      //pwrkey开机标志
volatile u8  pwrdwn_hw_flag;    //模拟硬开关，关机标志
volatile u8  incall_flag;
volatile u8  gui_sleep_sta;
volatile u8  gui_need_wakeup;
volatile u8  chg_on;            //配合工作RTCCON8开个充电; 1打开状态，0关闭状态

    ///位变量不要用于需要在中断改值的变量。 请谨慎使用位变量，尽量少定义位变量。
    u8  rtc_first_pwron  		: 1,   //RTC是否第一次上电
        mute             		: 1,   //系统MUTE控制标志
        cm_factory       		: 1,   //是否第一次FLASH上电
        cm_vol_change    		: 1,   //音量级数是否需要更新到FLASH
        port2led_en      		: 1,   //1个IO推两个灯
        voice_evt_brk_en 		: 1;  //播放提示音时，U盘、SD卡、LINEIN等插入事件是否立即响应.

    u8  sleep_en         		: 1,   //用于控制是否进入sleep
		lowbat_flag      		: 1,
        bt_reconn_flag   		: 1,   //回连失败时候发起一键双连标志
        flag_sleep_ble_status 	: 1,   //用于未连接ble休眠后,当ble连接上后更新连接参数用
        flag_halt               : 1,   //蓝屏异常标志位
        flag_shipping_mode      : 1,   //船运模式标志位
        flag_pwroff             : 1;   //关机标志位
    bool mp3_res_playing;

    //运动
    u8 sport_idx;                                   //运动列表编号
    //设置
    u8 set_idx;                                     //设置菜单编号
	//调节亮度
    u8 light_level;                                 //亮度等级
    //调节音量
    u8 volume_level;                                //音量等级
	//调节振动强度
    u8 motor_level;
    //定时器
    u8 timer_sta;                                   //计时器工作状态（0:未开启 1:正在计时 2:暂停 3:结束 4:复位）
    u32 timer_total_sec;                            //当前计时器总时间 （单位：秒）
    u32 timer_left_sec;                             //当前计时器剩余时间 （单位：秒）
    u32 timer_custom_sec;                           //计时器自定义时间记忆值
    //密码锁
    u8 password_cnt;                                //密码有效长度
    u8 password_value[4];                           //密码有效值
    u8 password_change;                             //改密码锁
    //秒表
    u8 stopwatch_sta;                               //秒表工作状态（0:暂停/复位 1:正在计时）
    u32 stopwatch_total_msec;                       //当前秒表时间总计 （单位：毫秒）
    u32 stopwatch_rec_view[STOPWATCH_REC_NUM_MAX];  //秒表最大记录
    u8 stopwatch_rec_cnt;                           //秒表记录总数
    //闹钟
    u32 alarm_total_sec[ALARM_CLOCK_NUM_MAX];       //记录每个闹钟总秒数
    u8 alarm_week_sel[ALARM_CLOCK_NUM_MAX];         //记录每个闹钟的星期选择
    u8 alarm_idx;                                   //当前闹钟 idx:0,1,2
    u8 alarm_enable_cnt;                            //闹钟使能的个数 0:无闹钟
    u8 alarm_enable_sel;                            //使能闹钟选择
    //勿扰定时
    u32 disturd_start_time_sec;               //记录开始休眠的总秒数
    u32 disturd_end_time_sec;                 //记录结束休眠的总秒数
    u8 disturd_adl;                           //是否打开全天勿扰
    u8 disturd_tim;                           //是否打开定时勿扰
    u8 disturd_sel;                          //0-开始时间 1-结束时间
    //呼吸训练
    u32 breathe_duration;                           //训练时间 ms
    u8 breathe_mode;                                //训练模式 0:2,慢中快
    u8 dialplate_index;                             //表盘编号
    //天气
    u8 weather_idx;
    s8 temperature[2];                              //温度（0:最低 1:最高）
    //电量
    u8 charge_idx;
    
    //运动
    u16 step_cur;
    u16 step_goal;
    u8 distance_cur;
    u8 distance_goal;
    u8 kcal_cur;
    u8 kcal_goal;
    char outgoing_number[16];
    //运动记录
    u8 sport_record_cnt;
    u8 sport_record_btn_idx;
    //SCO 状态
    bool sco_state;
    //通话
    u32 reject_tick;                                //通话挂断防呆计时，解决部分手机挂断状态更新慢的问题
#if FUNC_MUSIC_EN
	bool local_music_sta;                           //本地音乐状态
	bool music_popup_en;                            //音乐页面插入跳转使能
#endif
    u8 music_src;                                   //音源

    //俄罗斯方块
    u32 max_score;
    u32 curr_score;

    //AI语音
#if ASR_SELECT
    bool asr_play;                                  //asr 指令响应状态
    u8   asr_gain;                                  //asr 备份增益
    u32  asr_mp3_addr;                              //asr MP3缓冲地址
    u32  asr_mp3_lens;                              //asr MP3缓冲长度
    bool asr_mp3_delay;                             //asr MP3延迟播放标志位
#endif
    u8 audio_path;

    bool gpu_exit_ever;
    bool frist_gui_sw_init;
    bool is_sleep_off_gui;                          //是否休眠只关GUI

    u8 sd_power_off_flag;
    u8 sd_enter_flag;
    u8 nav_index;
    bool flag_swithing;

///视频相机相关
    u8 sticker_idx;
    u8 image_sensor_idx;
    u8 datelabel;                                   //日期标签
    bool start_vedio_flag;
    u32 rfile_limit;                                  //录像限制时间
    //0x00:"VGA", 0x01:"2M", 0x02:"3M", 0x03:"5M", 0x04:"8M", 0x05:"10M", 0x06:"12M", 0x07:"15M"},
    u8 photosize_id;                                //图标大小选择
    u8 videosize_id;                                //摄像大小选择
    //0x00:"关闭", 0x01:"2s", 0x02:"5s", 0x03:"10s"
    u8 takephoto_time_id;                           //拍照定时器
    //日期
    u16 year;
    u8 mon;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
    u32 sign;                                       //固定值0xAA55CCDD
    u8 crc;                                         //判断数据区是否有用

    bool (*camera_take_photo_ble_send)(u8 *buf, u32 len);       //相机ble发送数据
    bool (*camera_video_record_ble_send)(u8 *buf, u32 len);       //相机ble发送数据

    u8 screenshot_flags;
    volatile bool ble_send_idle;                               //ble 发送空闲

} sys_cb_t;
extern sys_cb_t sys_cb;
extern volatile int micl2gnd_flag;
extern volatile u32 ticks_50ms;

typedef void (*isr_t)(void);
isr_t register_isr(int vector, isr_t isr);

void bsp_sys_init(void);
u8 bsp_sys_get_ctlbit(uint n);                      //获取系统控制位
void bsp_sys_set_ctlbit(uint n, u8 v);              //设置系统控制位
void bsp_sys_reverse_ctlbit(uint n);                //反转系统控制位
void bsp_sys_tm_update(void);           //更新tm结构体
void bsp_update_init(void);
void timer1_irq_init(void);
void bsp_sys_mute(void);
void bsp_sys_unmute(void);
void bsp_clr_mute_sta(void);
bool bsp_get_mute_sta(void);
void bsp_loudspeaker_mute(void);
void bsp_loudspeaker_unmute(void);
void uart0_mapping_sel(void);
void linein_detect(void);
bool linein_micl_is_online(void);
void get_usb_chk_sta_convert(void);
void sd_soft_cmd_detect(u32 check_ms);

/**
 * 是否使能船运模式,使能后关机不走时
 * en:true 打开，false 关闭
 */
void bsp_set_shipping_mode(bool en);

void huart_module_init(void);
void huart_wait_tx_finish(void);
void huart_putbuf(void *buf, u32 len);
void rtc_printf(void);
void exit_vid0v_process(void);
bool get_vio0v_func_en(void);
#endif // _BSP_SYS_H

