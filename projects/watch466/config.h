/*****************************************************************************
 * Module    : Config
 * File      : config.h
 * Function  : SDK配置文件
 *****************************************************************************/

#ifndef USER_CONFIG_H
#define USER_CONFIG_H
#include "config_define.h"

#define UI_BTN_CLICK_EFFECT_ALPHA1    180
#define UI_BTN_CLICK_EFFECT_ALPHA     120
#define UI_BTN_NORMAL_EFFECT_ALPHA    255

/*****************************************************************************
 * Module    : Function选择相关配置
 *****************************************************************************/
#define MAX_FUNC_SORT_CNT               20  //最大支持左右快捷切换任务的个数

#define FUNC_BT_EN                      1   //是否打开蓝牙功能
#define FUNC_BT_DUT_EN                  0   //是否打开蓝牙的独立DUT测试模式
#define FUNC_MUSIC_EN                   0   //是否打开MUSIC功能
#define FUNC_FMRX_EN                    0   //是否打开FM收音功能
#define FUNC_RECORDER_EN                0   //是否打开录音机功能
#define FUNC_USBDEV_EN                  0   //是否打开USB DEVICE功能
#define FUNC_CAMERA_TRANS_EN            0   //是否打开相机传输功能,需要一张图片的RGB数据缓存
#define FUNC_IDLE_EN                    0   //是否打开IDLE功能
#define FUNC_GAME_TETRIS_EN             0   //是否打开俄罗斯方块游戏
#define FUNC_BLE_GATTS_EN               0   //是否打开BLE GATTS Demo功能
#define FUNC_LUNCHBOX_UART_EN           1   //是否打开智能盒饭串口协议功能

/******************************************************************************
*Module      :BT EMIT FUNCTION
*             BT 发射SDK 暂不支持TWS ，一拖二，BLE
*******************************************************************************/
#define BT_EMIT_EN                      0   //是否打开蓝牙发射控制功能,需同时打开MUSIC功能

/*****************************************************************************
 * Module    : 系统功能选择配置
 *****************************************************************************/
#define FPGA_EN                         0                           //FPGA调试(关闭部分与硬件相关的模拟配置)
#define BUCK_MODE_EN                    1                           //是否BUCK MODE
#define SYS_CLK_SEL                     SYS_48M                    //选择系统时钟
#define POWKEY_10S_RESET                xcfg_cb.powkey_10s_reset
#define SOFT_POWER_ON_OFF               1                           //是否使用软开关机功能
#define SOFT_POWER_VDDIO_EN             0                           //是否软关机开启VDDIO
#define PWRKEY_2_HW_PWRON               0                           //用PWRKEY模拟硬开关
#define LP_XOSC_CLOCK_EN                0                           //是否使能低功耗晶振用于RTC CLOCK，支持关机时钟功能。(单脚晶振不支持低功耗晶振)
#define RTC_CLOCK_PDN_EN                1                           //是否使能关机时钟功能, 使能后关机时会保持sniffrc，关机电流变大到9u左右。
#define GUI_AUTO_POWER_EN               1                           //是否使能刷图动态调节时钟，打开后系统时钟默认设置为SYS_CLK_SEL，刷图时调节为192M
#define VUSB4S_RESET_EN                 0                           //VUSB4s硬件复位使能

#define CHIP_PACKAGE_SELECT             CHIP_5790T                  //芯片封装选择 5790T
#define CHIP_PACKAGE_SUPPORT_PSRAM      1                           //芯片支持psram
#define CHIP_PACKAGE_SUPPORT_HFP        1                           //芯片支持HFP

#define UART0_PRINTF_SEL                PRINTF_PB3                  //日志总开关：PRINTF_NONE=关 printf；PRINTF_PB3=PB3 UART 115200
#define SYS_INIT_VOLUME                 xcfg_cb.sys_init_vol        //系统默认音量

#define TS_MODE_EN                      0                           //内部NTC模块是否开启
#define MODEM_CAT1_EN                   0                           //cat1测试功能
#define CALL_MGR_EN                     0                           //统一管理cat1与蓝牙通话（开了cat1需要打开）
#define CALL_MGR_DEFAULT                0                           //不需要默认过滤逻辑可以关闭
#define DONGLE_AUTH_EN					0						    //是否配置加密狗授权
#define HEAP_FUNC_SIZE                  4096                        //FUNC HEAP SIZE

/*****************************************************************************
 * Module    : FLASH MAP 说明（新加的功能请放在上面）
 *****************************************************************************
 **0x0
 *4K                           (BOOT1)
 *4K                           (BOOT2)
 **0x2000
 *FLASH_CODE_SIZE              (app.bin)
 *FLASH_UI_SIZE                (ui.bin)
 ***
 *FLASH_UPDATE_PACKAGE_SIZE    (压缩升级包存放区域)
 ***
 *FLASHDB_ADDR                 （FLASHDB_EN == 1 时，16K）
 ***
 *FLASH_AB_PARAM_ADDR          (若USE_APP_TYPE == USE_AB_APP, 则有app协议保存数据)
 ***
 *FLASH_CM_SIZE                (参数区)
 *1.程序使用空间从0x2000地址开始存放，使用大小为FLASH_CODE_SIZE
 *2.UI资源文件可自定义大小，起始地址要在code区域后，不能有地址重叠
 *3.如需压缩升级，升级包存放地址要在UI.bin资源之后
 *4.参数区起始地址为：FLASH_SIZE - FLASH_CM_SIZE
 *****************************************************************************/
/*****************************************************************************
 * Module    : FLASH配置
 *****************************************************************************/
#define FLASH_DISK_EN                   0                                                   //是否支持FLASH DISK 功能
#define FLASH_SIZE                      FSIZE_4M                                            //根据芯片信息配置实际FLASH SIZE
#define FLASH_CODE_BASE_SIZE            0x1C0000                                            //代码区(须<=FLASH_UI_BASE-0x2000)
#define FLASH_UI_BASE                   0x200000                                            //UI资源起始地址(最小值为FLASH_CODE_SIZE)
#define FLASH_UI_SIZE                   0x100000                                            //UI资源大小(ui.bin的大小)
#define FLASH_PKG_START                 0x300000                                            //升级压缩包存放起始地址
#define FLASH_PKG_SIZE                  0x060000                                            //升级压缩包大小
#define FLASH_DISK_START                FLASH_PKG_START                                     //FLASH DISK 功能与OTA升级复用
#define FLASH_DISK_LEN                  FLASH_PKG_SIZE                                      //FLASH DISK 功能与OTA升级复用, 0为关闭此功能
#define FLASH_CM_SIZE                   0x5000
#define FLASH_ERASE_4K                  1                                                   //是否支持4K擦除
#define FLASH_DUAL_READ                 0                                                   //是否支持2线模式
#define FLASH_QUAD_READ                 0                                                   //是否支持4线模式
#define SPIFLASH_SPEED_UP_EN            2                                                   //SPI FLASH提速: 0->1bit data, 1->2bit data, 2->4bit data
#define FLASH_ERASE_32K_64K             0                                                   //是否支持32k和64k擦除
#define FLASH_EXTERNAL_EN               0                                                   //是否支持外挂flash, 可存放UI资源
#define FLASH_MUL_EN                    0

/*****************************************************************************
 * Module    : NOC配置
 *****************************************************************************/
#define NOC_FLASH_EN                    0                       //是否支持NOC FLASH(CACHE)
#define NOC_PSRAM_EN                    0*CHIP_PACKAGE_SUPPORT_PSRAM                       //是否支持NOC FLASH(NO CACHE)
#define NOC_PSRAM_SIZE                  0 //FSIZE_4M                //PSRAM的大小

/*****************************************************************************
 * Module    : 屏幕驱动配置
 *****************************************************************************/
#define GUI_SELECT                      GUI_TFT_240_ST789_i80   // 饭盒 320x240 I8080

#if (GUI_SELECT == GUI_TFT_240_ST789_i80)
#define PORT_TFT_INT                    IO_PE9                      //TE
#define PORT_TFT_INT_VECTOR             PORT_INT2_VECTOR

#define PORT_LCD_DISPLAY                IO_NONE
#define PORT_LCD_VSYNC                  IO_NONE
#define PORT_LCD_HSYNC                  IO_NONE
#define PORT_LCD_DE                     IO_NONE

#define PORT_TFT_DC                     IO_PA3
#define PORT_TFT_CS                     IO_PA5
#define PORT_TFT_RST                    IO_PE8
#define PORT_TFT_LCD_SCL                IO_PA4

#define PORT_TFT_LCD_D0                 IO_PA2
#define PORT_TFT_LCD_D1                 IO_PA1
#define PORT_TFT_LCD_D2                 IO_PA0
#define PORT_TFT_LCD_D3                 IO_PE14
#define PORT_TFT_LCD_D4                 IO_PE13
#define PORT_TFT_LCD_D5                 IO_PE12
#define PORT_TFT_LCD_D6                 IO_PE11
#define PORT_TFT_LCD_D7                 IO_PE10
#define PORT_TFT_LCD_D8                 IO_NONE
#define PORT_TFT_LCD_D9                 IO_NONE
#define PORT_TFT_LCD_D10                IO_NONE
#define PORT_TFT_LCD_D11                IO_NONE
#define PORT_TFT_LCD_D12                IO_NONE
#define PORT_TFT_LCD_D13                IO_NONE
#define PORT_TFT_LCD_D14                IO_NONE
#define PORT_TFT_LCD_D15                IO_NONE
#else
#define PORT_TFT_INT                    IO_PA6                      //TE
#define PORT_TFT_INT_VECTOR             PORT_INT2_VECTOR

#define PORT_LCD_DISPLAY                IO_NONE
#define PORT_LCD_VSYNC                  IO_NONE
#define PORT_LCD_HSYNC                  IO_NONE
#define PORT_LCD_DE                     IO_NONE

#define PORT_TFT_DC                     IO_PA3
#define PORT_TFT_CS                     IO_PA5
#define PORT_TFT_RST                    IO_PA7
#define PORT_TFT_LCD_SCL                IO_PA4

#define PORT_TFT_LCD_D0                 IO_PA2
#define PORT_TFT_LCD_D1                 IO_PA3
#define PORT_TFT_LCD_D2                 IO_PA1
#define PORT_TFT_LCD_D3                 IO_PA0
#define PORT_TFT_LCD_D4                 IO_NONE
#define PORT_TFT_LCD_D5                 IO_NONE
#define PORT_TFT_LCD_D6                 IO_NONE
#define PORT_TFT_LCD_D7                 IO_NONE
#define PORT_TFT_LCD_D8                 IO_NONE
#define PORT_TFT_LCD_D9                 IO_NONE
#define PORT_TFT_LCD_D10                IO_NONE
#define PORT_TFT_LCD_D11                IO_NONE
#define PORT_TFT_LCD_D12                IO_NONE
#define PORT_TFT_LCD_D13                IO_NONE
#define PORT_TFT_LCD_D14                IO_NONE
#define PORT_TFT_LCD_D15                IO_NONE
#endif

#define PORT_TFT_BL                     PG_BL_TMR4                  //BL
#define LCD_BL_EN()                     led_pg_on()
#define LCD_BL_DIS()                    led_pg_off()

#define LCD_POWER_EN()                  lcd_pg_on()                 //屏幕供电
#define LCD_POWER_DIS()                 lcd_pg_off()

#define GUI_DEFAULT_BK                  100                         //默认背光亮度

/*****************************************************************************
 * Module    : 饭盒面板 (ELUNCHBOX) — 与手表默认配置冲突项在此覆盖
 *****************************************************************************/
#define ELUNCHBOX_PANEL_EN              1

#if ELUNCHBOX_PANEL_EN
#undef  SOFT_POWER_VDDIO_EN
#define SOFT_POWER_VDDIO_EN             1           /* 硬关机保持 VDDIO，PT8028/PE1 可唤醒开机 */
#define ELUNCHBOX_KEEP_AWAKE            1           /* 禁止深度休眠；允许 guioff 定时息屏 */
#define ELUNCHBOX_GUIOFF_TIME_SEC       300          /* 无按键自动息屏(秒)，正式版可改 300 */
#define ELUNCHBOX_GUIOFF_SLEEP_EN       1           /* 息屏后再进 BT 浅睡降功耗 */
#define ELUNCHBOX_GUIOFF_SLEEP_DELAY_SEC 30          /* 息屏后延迟多少秒进浅睡 */
#define FUNC_RESERVATION_UI_EN          1           /* 1=预约键(TCH7)可进预约页 */
/* 日志走全局 UART0_PRINTF_SEL（PRINTF_PB3 / PRINTF_NONE）；TRACE_EN 仅控各文件 TRACE() 宏 */
/* LEVEL_HIGH_PRI 定时器在 tmr 线程执行；饭盒须保持 BT_EMIT_EN=0，避免额外高优先级 co_timer */
/* 保持 FUNC_BT_EN=1，否则 libapp(rf.c) 缺 rfphy_* / modem_init 链接符号 */
#undef  BT_BACKSTAGE_EN
#define BT_BACKSTAGE_EN                 0
#undef  BT_BACKSTAGE_MUSIC_EN
#define BT_BACKSTAGE_MUSIC_EN           0
#undef  CTP_SELECT
#define CTP_SELECT                      CTP_NO      /* 无 CST 屏触摸，用 PT8028 */
#undef  CTP_SUPPORT_COVER
#define CTP_SUPPORT_COVER               0           /* 盖手息屏会 100ms 后休眠 */
#undef  USER_KEY_QDEC_EN
#define USER_KEY_QDEC_EN                0           /* G4=PE13/14 与 LCD D3/D4 冲突 */
#undef  MUSIC_SDCARD_EN
#define MUSIC_SDCARD_EN                 0           /* 无 SD 卡 */
#undef  SD_SOFT_DETECT_EN
#define SD_SOFT_DETECT_EN               0
#undef  SD0_MAPPING
#define SD0_MAPPING                     SD0MAP_NONE  /* 禁止 SD 占用 PB 等脚 */
#undef  SD1_MAPPING
#define SD1_MAPPING                     SD1MAP_NONE  /* SD1 G3 占 PE1~4，与 PT8028 冲突 */
#undef  FUNC_REC_TO_SD
#define FUNC_REC_TO_SD                  0           /* 无 SD 卡，避免 SD_SUPPORT_EN=1 */
#undef  PHOTO_VIEW_EN
#define PHOTO_VIEW_EN                   0           /* 照片浏览依赖 SD，饭盒无 SD */
#undef  VIDEO_RECODE_TAKE_PHOTO_EN
#define VIDEO_RECODE_TAKE_PHOTO_EN      0           /* 勿占 PE2/PE4(PT8028 D0/D2) */
#undef  IMG_SENSOR_BF03A2_EN
#define IMG_SENSOR_BF03A2_EN            0
#undef  DEFAULE_START_FUNC
#define DEFAULE_START_FUNC              FUNC_HOME   /* 饭盒上电默认进 Home 页 */
#else
#define ELUNCHBOX_KEEP_AWAKE            0
#define ELUNCHBOX_GUIOFF_SLEEP_EN       0
#endif

#ifndef FUNC_RESERVATION_UI_EN
#define FUNC_RESERVATION_UI_EN          1
#endif

/*****************************************************************************
 * Module    : 触摸驱动配置
 *****************************************************************************/
#if !ELUNCHBOX_PANEL_EN
#define CTP_SELECT                      CTP_CST8X                   //CTP Select
#endif
#define PORT_CTP_SCL                    IO_PA8
#define PORT_CTP_SDA                    IO_PA9
#define PORT_CTP_MAP_GPIO               2
#define PORT_CTP_IIC_HW                 1
#define PORT_CTP_INT                    IO_PA10
#define PORT_CTP_INT_VECTOR             PORT_INT3_VECTOR
#define PORT_CTP_RST                    IO_PA11
#define PORT_CTP_RST_H()                GPIOASET = BIT(11);
#define PORT_CTP_RST_L()                GPIOACLR = BIT(11);
#if !ELUNCHBOX_PANEL_EN
#define CTP_SUPPORT_COVER               1                           //是否支持盖手息屏功能，需要确认屏幕是否支持
#endif
#define CTP_DOUBLE_CLICK_EN             0                           //触摸双击检测使能

/*****************************************************************************
 * Module    : GUI相关配置
 *****************************************************************************/
#define COMPO_BUF_SIZE                  (3584)              	//组件BUF大小(2个BUF)
#define GUI_WGT_BUF_EXTRA               0                       //disp 96KB 已满，勿增大 widget 池
#define TFT_TE_CYCLE                    16.67                   //屏幕的刷新率TE周期时间 (ms)
#define TFT_TE_CYCLE_DELAY              (TFT_TE_CYCLE / 3)
#define DEFAULT_TE_MODE                 1                       //默认1 TE模式, 0为2 TE模式, 3为复杂界面专用模式
#define GUI_LINES_CNT                   30                      //单次推屏行数

#define GUI_FONT_W_SPACE                0                       //字的间距
#define GUI_FONT_H_SPACE                0                       //全局字的行间距,0:不设置, 其他:设置文本行最小间距
#define GUI_USE_ARC                     1                       //是否使用圆弧控件
#define GUI_USE_SCREENSHOOT             0                       //无PSRAM须关
#define GUI_USE_BLUR                    0                       //无PSRAM须关(config_extra会在BLUR=1时强开)
#define GUI_SPU_PSRAM                   0                       //使用PSRAM推屏
#ifndef DEFAULE_START_FUNC
#define DEFAULE_START_FUNC              FUNC_CLOCK           //默认启动页（手表：表盘）
#endif

/*****************************************************************************
 * Module    : UI场景相关配置
 *****************************************************************************/
#define GUI_SIDE_MENU_WIDTH             (GUI_SCREEN_WIDTH / 2)  //边菜单的宽度

#define FORM_TITLE_HEIGHT               50                      //窗体标题高度
#define FORM_TITLE_LEFT                 (GUI_SCREEN_WIDTH / 9)

#define UI_BUF_FONT_SYS                 UI_BUF_0FONT_FONT_BIN           //系统字体
#define UI_BUF_FONT_FORM_TIME           UI_BUF_0FONT_FONT_ASC_BIN       //窗体标题栏时间字体
#if defined(UI_BUF_0FONT_FONT_ASC_8_BIN)
#define UI_BUF_FONT_TIMEING_SUFFIX      UI_BUF_0FONT_FONT_ASC_8_BIN    //Time 页 H/Min 更小字
#elif defined(UI_BUF_0FONT_FONT_ASC_10_BIN)
#define UI_BUF_FONT_TIMEING_SUFFIX      UI_BUF_0FONT_FONT_ASC_10_BIN    //Time 页 H/Min 小字
#else
#define UI_BUF_FONT_TIMEING_SUFFIX      UI_BUF_0FONT_FONT_ASC_BIN       //缩放至设计尺寸
#endif

#define BOX_GUI_ROTATE_DISP             0

/*****************************************************************************
 * Module    : 蓝牙功能配置
 *****************************************************************************/
#define BT_BQB_RF_EN                    0   //BR/EDR DUT测试模式，为方便测试不自动回连（仅用于BQB RF测试）
#define BT_FCC_TEST_EN                  0   //蓝牙FCC测试使能，默认PB3 波特率1500000通信（仅用于FCC RF测试）
#define BT_LINK_INFO_PAGE1_EN           0   //是否使用PAGE1回连信息（打开后可以最多保存8个回连信息）
#define BT_POWER_UP_RECONNECT_TIMES     3   //上电回连次数
#define BT_TIME_OUT_RECONNECT_TIMES     20  //掉线回连次数
#define BT_SIMPLE_PAIR_EN               1   //是否打开蓝牙简易配对功能（关闭时需要手机端输入PIN码）
#define BT_DISCOVER_CTRL_EN             1   //是否使用按键打开可被发现（需自行添加配对键处理才能被连接配对）
#define BT_DISCOVER_TIMEOUT             100 //按键打开可被发现后，多久后仍无连接自动关闭，0不自动关闭，单位100ms
#define BT_ANTI_LOST_EN                 0   //是否打开蓝牙防丢报警
#define BT_DUT_MODE_EN                  0   //正常连接模式，是否使能DUT测试
#define BT_LOCAL_ADDR                   0   //蓝牙是否使用本地地址，0使用配置工具地址
#define BT_LOW_LATENCY_EN               0   //是否打开蓝牙低延时切换功能

#define BT_2ACL_EN                      0   //是否支持连接两部手机（TWS不支持）
#define BT_2ACL_AUTO_SWITCH             0   //连接两部手机时是否支持点击播放切换到对应的手机
#define BT_A2DP_EN                      0   //是否打开蓝牙音乐服务
#define BT_HFP_EN                       0*CHIP_PACKAGE_SUPPORT_HFP   //是否打开蓝牙通话服务
#define BT_HFP_GET_TIME_EN              0   //是否使用HFP获取设备时间
#define BT_HSP_EN                       0   //是否打开蓝牙HSP通话服务
#define BT_PBAP_EN                      0   //是否打开蓝牙电话簿服务
#define BT_MAP_EN                       0   //是否打开蓝牙短信服务(用于获取设备时间，支持IOS/Android)
#define BT_SPP_EN                       0   //是否打开蓝牙串口服务
#define BT_ID3_TAG_EN                   0   //是否打开蓝牙ID3功能
#define BT_PANU_EN                      0   //是否打开蓝牙个人区域网服务
#define BT_HID_EN                       0   //是否打开蓝牙HID服务
#define BT_HID_TYPE                     0   //选择HID服务类型: 0=自拍器(VOL+, 部分Android不能拍照), 1=自拍器(VOL+和ENTER, 影响IOS键盘使用), 2=游戏手柄
#define BT_HID_MANU_EN                  0   //蓝牙HID是否需要手动连接/断开
#define BT_HID_DISCON_DEFAULT_EN        0   //蓝牙HID服务默认不连接，需要手动进行连接。
#define BT_HID_VOL_CTRL_EN              0   //是否支持HID调手机音量功能（需同时打开BT_HID_EN和BT_A2DP_VOL_CTRL_EN）
#define BT_HFP_CALL_PRIVATE_SWITCH_EN   0   //是否打开按键切换私密接听与蓝牙接听功能
#define BT_HFP_CALL_PRIVATE_FORCE_EN    0   //是否强制使用私密接听（仅在手机接听，不通过蓝牙外放）
#define BT_HFP_RECORD_DEVICE_VOL_EN     0   //是否支持分别记录不同连接设备的通话音量
#define BT_HFP_RING_NUMBER_EN           0   //是否支持来电报号
#define BT_HFP_INBAND_RING_EN           0   //是否支持手机来电铃声（部分android不支持，默认用本地RING提示音）
#define BT_HFP_BAT_REPORT_EN            1   //是否支持电量显示
#define BT_HFP_MSBC_EN                  0   //是否打开宽带语音功能
#define BT_A2DP_AAC_AUDIO_EN            0   //是否支持蓝牙AAC音频格式
#define BT_HFP_3WAY_CTRL_EN             0   //是否使能三方通话管理
#define BT_HFP_SWITCH_EN                0   //是否使能通话切换功能，包括主动切换和哪边接听哪边出声
#define BT_VOIP_REJECT_EN               0   //网络电话不建立SCO功能使能,使用时需A2DP断开 (网络电话：微信通话，QQ通话等)
#define BT_A2DP_PROFILE_DEFAULT_EN      0   //蓝牙音频服务是否默认打开
#define BT_A2DP_VOL_CTRL_EN             0   //是否支持音量与手机同步，（默认使用AVRCP协议，打开BT_HID_VOL_CTRL_EN后使用HID协议）
#define BT_A2DP_RECORD_DEVICE_VOL_EN    0   //是否支持分别记录不同连接设备的音量，使用设备时恢复当前设备音量
#define BT_A2DP_VOL_REST_EN             0   //是否支持连接不支持同步音量手机时复位音量
#define BT_A2DP_AVRCP_PLAY_STATUS_EN    0   //是否支持手机播放状态同步，可加快播放暂停响应速度
#define BT_A2DP_RECON_EN                0   //是否支持A2DP控制键（播放/暂停、上下曲键）回连
#define BT_A2DP_SUPTO_RESTORE_PLAY_EN   0   //是否支持蓝牙超距回连恢复播放
#define BT_A2DP_EXCEPT_RESTORE_PLAY_EN  0   //是否支持异常复位后回连恢复播放
#define BT_AVDTP_DYN_LATENCY_EN         0   //是否支持根据信号环境动态调整延迟
#define BT_SCO_DBG_EN                   0   //是否打开无线调试通话参数功能
#define BT_CONNECT_REQ_FROM_WATCH_EN    0   //是否使能一键双连BT连接请求由手表发起
#define BT_RF_EXT_CTL_EN                0   //是否打开rf动态功耗调节
#define BT_HID_ONLY_FOR_IOS_EN          0   //是否使能安卓手机不建立HID
#define BT_HCI_DUMP                     0   //是否打开蓝牙HCI DUMP功能
#define BT_SINGLE_SLEEP_LPW_EN          0   //是否打开单模进休眠关bt省电
#if BT_HFP_EN
#define BT_HFP_KEY                      0xbeac5b78
#endif // BT_HFP_EN

/*****************************************************************************
 * Module    : BLE功能配置
 *****************************************************************************/
#define LE_EN                           1   //是否打开BLE功能
#define LE_PAIR_EN                      1   //是否使能BLE的加密配对
#define LE_SM_SC_EN                     1   //是否使能BLE的加密连接，需同时打开LE_PAIR_EN。一键双联需要打开此配置。
#define LE_ADV_POWERON_EN               1   //是否上电默认打开BLE广播
#define LE_BQB_RF_EN                    0   //BLE DUT测试模式，使用串口通信（仅用于BQB RFPHY测试）
#define LE_ALLOW_WKUP_EN                0   //休眠中ble断开/连接/传输是否需要退出休眠

//gatt 配置
#define LE_ATT_NUM                      35  //最大支持多少条gatt属性, att_handle 1 ~ LE_ATT_NUM (BlueFit + 128bit UUID img service + lunchbox BLE)

//APP 功能相关
#define USE_APP_TYPE                    APP_BLUE_FIT //选择手表应用app类型

//ANCS
#define LE_ANCS_CLIENT_EN               1   //是否打开ANCS Clients
#define LE_ANCS_MANUAL_EN               1   //是否需要手动打开ancs, 需要调用发起ancs连接的相关接口
//AMS
#define LE_AMS_CLIENT_EN                1   //是否打开AMS Clients

#define LE_ADV0_EN                      0   //是否打开无连接广播功能
#define LE_WIN10_POPUP                  0   //是否打开win10 swift pair快速配对

//FOTA功能配置
#define LE_AB_FOT_EN                    0   //是否打开BLE FOTA服务,需同时打开LE_AB_LINK_APP_EN
#define AB_FOT_TYPE_PACK                1   //FOTA压缩升级（代码做压缩处理，升级完成需做解压才可正常运行）
#define SW_VERSION		                "V0.0.1"   //只能使用数字0-9,ota需要转码
#define HW_VERSION		                "V0.0.1"   //只能使用数字0-9,ota需要转码
#define FOTA_UI_EN                      0          //是否支持UI升级，需要用一个批处理打包UI+FOT
#define FOTA_DOG_EN                     0          //是否支持升级狗升级

//HID功能配置
#define LE_HID_EN                       0   //BLE HID总开关
#define LE_HID_CONSUMER                 1   //BLE HID Consumer功能
#define LE_HID_DIGITIZER                1   //BLE HID Digitizer功能
#define LE_HID_MOUSE                    1   //BLE HID Mouse功能
#define LE_HID_KEYBOARD                 0   //BLE HID Keybarod功能
#define LE_SERVICE_CHANGED              0   //BLE service change通知功能
#define LE_HID_TEST                     0   //BLE HID测试

/*****************************************************************************
 * Module    : 通话功能配置
 *****************************************************************************/
//通话参数
#define BT_SCO_DUMP_EN                  0                           //是否打开上行降噪算法数据dump功能（双MIC优先用），dump:算法前主麦 + 算法前副麦 + 算法后
#define BT_SCO_FAR_DUMP_EN              0                           //是否打开通话下行数据dump功能，dump:算法前 + 算法后
#define BT_SCO_EQ_DUMP_EN               0                           //是否打开上行EQ的数据dump功能（单MIC优先用），dump:算法前主麦 + 算法后 + EQ后

#define BT_SCO_EQ_DRC_EN                0                           //DRC参数调试在 bt_mic_8k.drc //(msbc)bt_mic_16k.drc

#define BT_SCO_MAV_EN                   0                           //是否打开蓝牙通话变声功能

#define BT_SCO_AGC_EN                   0                           //是否打开AGC算法
#define BT_SCO_AGC_TARGET_DB            3                           //AGC均衡后目标值(-DB)
#define BT_SCO_AGC_COMPRESSION_DB       12                          //AGC最大抬升增益能力(DB)

#define BT_PLC_EN                       0
#define BT_ANL_GAIN                     3                           //MIC模拟增益(0~12DB)
#define BT_CALL_MAX_GAIN                xcfg_cb.bt_call_max_gain    //配置通话时DAC最大模拟增益

#define BT_AEC_EN                       0
#define BT_AEC_FF_MIC_REF_EN            0                           //如果aec的ff_mic回声比talk_mic回声大，可使能这功能，用于双mic降噪
#define BT_AEC_NLP_BYPASS_EN            0                           //是否打开nlp bypass
#define BT_ECHO_LEVEL                   xcfg_cb.bt_echo_level       //回声消除级别（级别越高，回声衰减越明显，但通话效果越差）(0~15)
#define BT_FAR_OFFSET                   xcfg_cb.bt_far_offset       //远端补偿值(0~255)

#define BT_ALC_EN                       0                           //是否使能ALC
#define BT_ALC_FADE_IN_DELAY            26                          //近端淡入延时(n*15ms)
#define BT_ALC_FADE_IN_STEP             1                           //近端淡入速度(64ms)
#define BT_ALC_FADE_OUT_DELAY           2                           //远端淡入延时(n*15ms)
#define BT_ALC_FADE_OUT_STEP            16                          //远端淡入速度(4ms)
#define BT_ALC_VOICE_THR                0x30000

//通话近端降噪算法(耳机MIC采集数据降噪, AINS4/DNN/DMDNN/AIAEC只能四选一)
#define BT_SCO_AINS4_EN					0	                        //是否打开MIC的AINS4降噪
#define BT_SCO_AINS4_LEVEL				xcfg_cb.bt_sco_nr_level	    //0-30级

#define BT_SCO_DNN_EN                   0                           //是否打开自研单麦DNN降噪算法
#define BT_SCO_DNN_LEVEL                xcfg_cb.bt_sco_nr_level     //降噪量：0-30级

#define BT_SCO_DMIC_AI_EN               0                           //是否打开自研双麦AI降噪算法
#define BT_SCO_DMNS_LEVEL               0                           //降噪量：0-20级
#define BT_SCO_DMNS_DISTANCE            23                          //双麦间距(出音孔之间的直线距离20~40mm)

#define BT_SCO_AIAEC_DNN_EN             0                           //是否打开自研AIAEC+单麦DNN降噪算法
#define BT_SCO_AIAEC_DNN_LEVEL          xcfg_cb.bt_sco_nr_level     //降噪量：0-30级
#define BT_SCO_AIAEC_NLP_LEVEL          xcfg_cb.bt_echo_level       //回声消除级别

#define BT_SCO_DMIC_AIAEC_EN            0                           //是否打开自研双麦+AIAEC降噪算法
#define BT_SCO_DMIC_AIAEC_NT_LEVEL      1
#define BT_SCO_DMIC_AIAEC_ECHO_LEVEL    1
#define BT_SCO_DMIC_AIAEC_DISTANCE      23                          //双麦间距(出音孔之间的直线距离20~40mm)
#define BT_SCO_DMIC_AIAEC_NLP_REF       0                           //0代表主mic，1代表副mic

//通话远端降噪算法(接收远端手机的通话数据降噪)
#define BT_SCO_FAR_NR_EN                0                           //是否打开远端降噪算法(Code: 2KB, Ram: 2.1KB)
#define BT_SCO_FAR_NR_LEVEL             5                           //强度: 0~5
#define BT_SCO_FAR_NOISE_THR            1                           //范围: 0~20
#define BT_SCO_FAR_VALUE_NS             3                           //范围: 0~50

//（注：量产版本一定要关掉该功能，仅用于调试，会影响稳定性，开启该功能要预留 1.9K Ram）
#define BT_SCO_APP_DBG_EN               0                           //是否打开通话算法APP调试功能，配合通话调试APP使用，配置工具需要打开蓝牙串口服务和EQ蓝牙串口调试

/*****************************************************************************
 * Module    : SD/UDISK音乐功能配置
 *****************************************************************************/
#define MUSIC_UDISK_EN                  0   //是否支持播放UDISK
#if !ELUNCHBOX_PANEL_EN
#define MUSIC_SDCARD_EN                 1   //是否支持播放SDCARD
#endif
#define USB_SD_UPDATE_EN                0   //是否支持UDISK/SD的离线升级

#define MUSIC_WAV_SUPPORT               1   //是否支持WAV格式解码
#define MUSIC_WMA_SUPPORT               0   //是否支持WMA格式解码
#define MUSIC_APE_SUPPORT               0   //是否支持APE格式解码
#define MUSIC_FLAC_SUPPORT              0   //是否支持FLAC格式解码
#define MUSIC_M4A_SUPPORT               0   //是否支持M4A格式解码
#define MUSIC_SBC_SUPPORT               0   //是否支持SBC格式解码(SD/UDISK的SBC歌曲, 此宏不影响蓝牙音乐)
#define MUSIC_AAC_SUPPORT               0   //仅用于AAC解码测试(测试用, 此宏不影响蓝牙音乐)
#define MUSIC_SBR_SUPPORT               0   //是否支持SBR解码(code: 26KB, ram: 55KB, 打开加强m4a兼容性)

#define MUSIC_FOLDER_SELECT_EN          1   //文件夹选择功能
#define MUSIC_AUTO_SWITCH_DEVICE        1   //双设备循环播放
#define MUSIC_BREAKPOINT_EN             1   //音乐断点记忆播放
#define MUSIC_QSKIP_EN                  1   //快进快退功能
#define MUSIC_PLAYMODE_NUM              4   //音乐播放模式总数
#define MUSIC_MODE_RETURN               1   //退出音乐模式之后是否返回原来的模式
#define MUSIC_PLAYDEV_BOX_EN            0   //是否显示“USB”, "SD"界面
#define MUSIC_ID3_TAG_EN                0   //是否获取MP3 ID3信息
#define MUSIC_REC_FILE_FILTER           0   //是否区分录音文件与非录音文件分别播放
#define MUSIC_LRC_EN                    0   //是否支持歌词显示
#define MUSIC_NAVIGATION_EN             0   //音乐文件导航功能(LCD点阵屏功能)
#define MUSIC_ENCRYPT_EN                0   //是否支持加密MP3文件播放(使用MusicEncrypt.exe工具进行MP3加密)
#define MUSIC_MP3_LOOPBACK_EN           0   //是否开启MP3音乐单曲无缝循环播放
#define MUSCI_BACKSTAGE_EN              0   //SD音乐是否后台播放(手表本地播放方案)

#define MUSIC_ENCRYPT_KEY               12345   //MusicEncrypt.exe工具上填的加密KEY

/*****************************************************************************
 * Module    : MIC相关配置
 *****************************************************************************/
#define MIC_DEFAULT_NCH                 ((xcfg_cb.mic0_en == true) + (xcfg_cb.mic1_en == true)) //默认多少个MIC采集音频

/*****************************************************************************
 * Module    : 录音功能配置
 *****************************************************************************/
#define FUNC_REC_EN                     0   //录音功能总开关

#define FUNC_REC_SPR                    SPR_8000            //录音的波特率
#define FUNC_REC_BITRATE                16000               //录音的码率
#define FUNC_REC_NCH                    MIC_DEFAULT_NCH     //录音的通道数, 硬件自行决定
#if !ELUNCHBOX_PANEL_EN
#define FUNC_REC_TO_SD                  1                   //录音到文件系统
#endif
#define FUNC_REC_OPUS_BT_MUSIC_EN       0                   //录音OPUS压缩同时支持后台音乐播放, 只支持OPUS压缩时可以使用后台音乐sbc传输播放, 开了之后只能录opus了
#undef  BT_A2DP_AAC_AUDIO_EN
#define BT_A2DP_AAC_AUDIO_EN            (!FUNC_REC_OPUS_BT_MUSIC_EN)
#define FUNC_REC_NR_EN                  1 * FUNC_REC_EN     //录音降噪 只支持 BT_SCO_DNN_EN WAV音频 降噪
#define FUNC_REC_DNN_LEVEL              6                   //录音DNN降噪量 0~30
#define FUNC_REC_OPUS_EN                1 * FUNC_REC_EN     //录音时支持OPUS压缩
#define FUNC_REC_OPUS_FRAME_LEN         80                  //默认没有连接ble时opus一帧数据长度
#define FUNC_REC_OPUS_BLE               1 * FUNC_REC_EN     //BLE发送压缩 opus 音频
#define FUNC_REC_AUTO_EN                1 * FUNC_REC_EN     //是否支持自动录音模式
#define FUNC_REC_OPUS_BLE_FIX           240                 //限制BLE实际OPUS发送数据量, 0就不限制

#define FUNC_REC_PCM2DAC_OUT            0   //录音时同时喇叭输出录音pcm
#define FUNC_REC_VOICED_CNT             5   //检测到有声次数
#define FUNC_REC_VOICED_POW_VALUE       500 //检测到有声的adc能量值参数
#define FUNC_REC_SILENT_CNT             20  //检测到无声此时
#define FUNC_REC_SILENT_POW_VALUE       200 //检测到无声的adc能量值参数
#define REC_FAST_PLAY                   0   //播卡播U下快速播放最新的录音文件(双击REC)

#define REC_MP3_SUPPORT                 1   //是否支持MP3音频格式录音
#define REC_WAV_SUPPORT                 1   //是否支持WAV音频格式录音
#define REC_ADPCM_SUPPORT               0   //是否支持ADPCM音频格式录音
#define REC_SBC_SUPPORT                 0   //是否支持SBC音频格式录音
#define REC_OPUS_SUPPORT                1   //是否支持OPUS压缩音频格式录音

/*****************************************************************************
* Module    : DAC配置控制
******************************************************************************/
#define DAC_CH_SEL                      xcfg_cb.dac_sel             //DAC_MONO ~ DAC_VCMBUF_DUAL
#define DAC_FAST_SETUP_EN               1                           //DAC快速上电，有噪声需要外部功放MUTE
#define DAC_MAX_GAIN                    xcfg_cb.dac_max_gain        //配置DAC最大模拟增益，默认设置为dac_vol_table[VOL_MAX]
#define DAC_OUT_SPR                     DAC_OUT_48K                 //dac out sample rate
#define DAC_24BITS_EN                   0
#define DAC_LDOH_SEL                    xcfg_cb.dacaud_ldo_sel
#define DACVDD_BYPASS_EN                xcfg_cb.dacaud_bypass_en    //DACVDD Bypass
#define DAC_PULL_DOWN_DELAY             80                          //控制DAC隔直电容的放电时间, 无电容时可设为0，减少开机时间。
#define DAC_DNR_EN                      0                           //是否使能动态降噪
#define DAC_DRC_EN                      1                           //是否使能DRC功能
#define DAC_AUTO_ONOFF_EN               1                           //是否DAC自动关闭模拟电源（不播放时自动关掉省电）

/*****************************************************************************
* Module    : 音频压缩算法配置
            ////16K采样率以下的  (20ms帧长)： 32kbps以下 的 单通道 压缩
******************************************************************************/
#define OPUS_ENC_EN                     0   //是否打开opus压缩算法

/*****************************************************************************
 * Module    : EQ相关配置
 *****************************************************************************/
#define EQ_MODE_EN                      0           //是否调节EQ MODE (POP, Rock, Jazz, Classic, Country)
#define EQ_DBG_IN_UART                  0           //是否使能UART在线调节EQ
#define EQ_DBG_IN_SPP                   0           //是否使能SPP在线调节EQ

/*****************************************************************************
 * Module    : PSRC相关配置 (硬件SRC)
 *****************************************************************************/
#define PSRC_HW_EN                      0           //是否支持硬件PSRC
#define PSRC_CH1_EN                     0           //是否打开第二路PSRC1

/*****************************************************************************
 * Module    : User按键配置 (可以同时选择多组按键)
 *****************************************************************************/
#if ELUNCHBOX_PANEL_EN
#define USER_PWRKEY                     0           /* 饭盒用 PT8028 TCH5，勿扫 PWRKEY 以免误重置息屏计时 */
#else
#define USER_PWRKEY                     1           //PWRKEY的使用，0为不使用
#endif
#define USER_ADKEY                      0           //ADKEY的使用， 0为不使用
#define USER_IOKEY                      0           //IOKEY的使用， 0为不使用
#define USER_PT8028_KEY                 1           //PT8028S 触摸键 BCD 接口
#define PT8028_FLAG_ACTIVE_LOW          1           //OUT_FLAG 低有效=有键（规格表2）
#define PT8028_KEY_DEBOUNCE_SCANS       1           //保留兼容；释放沿不再依赖此值
#define PT8028_KEY_LATCH_MS             80          //保留，兼容旧配置
#if ELUNCHBOX_PANEL_EN
#define PT8028_KEY_DEBUG                1           //边沿/按键日志经 pt8028_log_flush 主线程输出
#else
#define PT8028_KEY_DEBUG                1           //边沿/按键日志经 pt8028_log_flush 输出
#endif
#define PT8028_GPIO_MONITOR_EN          0           //1=主循环打印 GPIO(易刷屏/卡死，默认关)
#if ELUNCHBOX_PANEL_EN
#define PT8028_BCD_SAMPLE_CNT           8
#define PT8028_BCD_SAMPLE_US            100         /* vote 连采间隔(us) */
#define PT8028_BCD_STABLE_CNT           2
#define PT8028_PRESS_SETTLE_SCANS       8           /* FLAG 变 0 后等 BCD 更新 */
#define PT8028_TCH7_CONFIRM_SCANS       15          /* 111 且无 TCH0~6 才当预约 */
#define PT8028_RELEASE_HOLD_SCANS       4
#define PT8028_HOLD_READ_CNT            8           /* 释放沿 Hold 连采次数 */
#define PT8028_POLL_REINIT_MS           500         /* 主线程恢复 PE1~4，勿过频 */
/* D 线高阻：板载/PT8028 推挽；勿开 MCU 内部上拉以免干扰读 0 */
#else
#define PT8028_BCD_SAMPLE_CNT           5           //每次读 BCD 连采次数(多数表决)
#endif
#ifndef PT8028_BCD_STABLE_CNT
#define PT8028_BCD_STABLE_CNT           2           //释放/消息：连续 2 次 5ms 相同 BCD
#endif
#ifndef PT8028_PRESS_SETTLE_SCANS
#define PT8028_PRESS_SETTLE_SCANS       2           //OUT_FLAG 变 0 后约 10ms 再采 BCD
#endif
#define PT8028_RES_LONG_MS              2000        //预约键长按(ms)；短按/误读 BCD7 当模式键
#define PT8028_PWR_LONG_MS              3000        //开关键(TCH5)长按(ms)手动关机
#if ELUNCHBOX_PANEL_EN
#define PT8028_PWR_WAKE_MS              3000        //息屏/休眠后长按开关键亮屏(ms)
#endif
#define PT8028_LOCK_LONG_MS             3000        //锁键(TCH0)长按(ms)全局按键锁
#define PT8028_HEAT_LONG_MS             3000        //加热键(TCH1)长按(ms)进入加热页
#define KEY_LOCK_HINT_MS                3000        //锁定/误触：右上角锁图标显示(ms)
#define KEY_UNLOCK_HINT_MS              1500        //解锁图标显示(ms)

#define USER_PANEL_LED                  1           //面板白色 LED1~6（原理图 510Ω 到 GND）
#define PANEL_LED_ACTIVE_HIGH           1           //1=GPIO 高电平点亮 LED

#if !ELUNCHBOX_PANEL_EN
#define USER_KEY_QDEC_EN                1           //旋钮, 硬件正交解码, A,B输出分别接一个IO
#endif
#define USER_QDEC_MAPPING               QDEC_MAP_G4 //选择硬件正交解码的mapping, 每组map的IO固定，详见define处说明

#define USER_ADKEY_QDEC_EN              0           //旋钮, A,B串不同电阻接到同一个IO口上，软件ADC采集并解码
#define USER_ADKEY_QDEC_NO_STD          0           //是否使用非标准电平判断(适用编码器漏电平时)
#define USER_QDEC_ADCH                  ADCCH_PA0   //选择旋钮的ADC通道

#define USER_MULTI_PRESS_EN             1           //按键多击检测使能
#define USER_MULTI_KEY_TIME             4           //按键多击响应时间（单位100ms）
#define USER_PWRON_KEY_SEL              0           //定义为开关机的PWRKEY按键编号, 范围: 0 ~ 2
#define PWRON_PRESS_TIME                3000        //饭盒：长按开关键 3s 开机
#define PWROFF_PRESS_TIME               18          //长按PWRKEY多长时间关机 3: 1.5秒, 6: 2秒, 9: 2.5秒, 12: 3秒, 15: 3.5秒, 18: 4秒, 24: 5秒
#define ADKEY_CH                        ADCCH_PE7   //ADKEY的ADC通路选择
#define IS_PWRKEY_PRESS()			    (0 == (RTCCON & BIT(19)))

/*****************************************************************************
 * Module    : 电量检测及低电
 *****************************************************************************/
#define VBAT_DETECT_EN                  1           //电池电量检测功能
#define VUSB_DETECT_EN                  0           //充电电压检测功能
#define VBAT2_ADCCH                     ADCCH_VBAT  //ADCCH_VBAT为内部1/2电压通路，带升压应用需要外部ADC通路检测1/2电池电压
#define VBAT_FILTER_USE_PEAK            0           //电池检测滤波选则://0 取平均值.//1 取峰值(适用于播放音乐时,电池波动比较大的音箱方案).
#define VBAT_ADC_EN                     0           //是否是能VADC功能
#define LPWR_WARNING_VBAT               3500        //低电提醒电压   0：表示关闭此功能
#define LPWR_OFF_VBAT                   3100        //低电关机电压   0：表示关闭此功能
#define LOWPWR_REDUCE_VOL_EN            1           //低电时是否降低音量
#define LPWR_WARING_TIMES               0xff        //报低电次数
#define LPWR_WARNING_PERIOD             30          //低电播报周期(单位：秒)

/*****************************************************************************
 * Module    : 充电功能选择
 *****************************************************************************/
#define CHARGE_EN                       1                               //是否打开充电功能
#define CHARGE_TRICK_EN                 xcfg_cb.charge_trick_en         //是否打开涓流充电功能
#define CHARGE_DC_RESET                 xcfg_cb.charge_dc_reset         //是否打开DC插入复位功能
#define CHARGE_DC_NOT_PWRON             xcfg_cb.charge_dc_not_pwron     //DC插入，是否软开机。 1: DC IN时不能开机
#define CHARGE_DC_IN()                  charge_dc_detect()
#define CHARGE_INBOX()                  ((RTCCON >> 22) & 0x01)
#define CHARGE_STOP_CURR                xcfg_cb.charge_stop_curr        //电流范围0~37.5mA, 配置值范围: 0~15, 步进2.5mA
#define CHARGE_CONSTANT_CURR            xcfg_cb.charge_constant_curr    //电流范围0~320mA, 配置值范围: 0~63, 步进5mA
#define CHARGE_TRICKLE_CURR             xcfg_cb.charge_trickle_curr
#define CHARGE_STOP_VOLT                0                               //充电截止电压：0:4.2v；1:4.35v; 2:4.4v; 3:4.45v
#define CHARGE_STOP_VOLT_FIX            0                               //充电截止电压微调, 在充电截止电压基础上增加14mv/step
#define CHARGE_TRICK_STOP_VOLT          1                               //涓流截止电压：0:2.9v; 1:3v
#define CHARGE_VOLT_FOLLOW_EN           0                               //是否打开恒压跟随充电模式
#define CHARGE_VOLT_FOLLOW_SEL          1                               //恒压差值选择：0:187.5mV; 1:250mV; 2:312.5mV; 3:375mV
#define CHARGE_VOL_DYNAMIC_DET          0                               //是否打开充电时候，动态检测电池电压功能；打开后，充5s停10ms

//充电辅助设置项
#define CHARGE_USER_NTC_EN              0                               //充电是否使用NTC参数
#define CHARGE_VBAT_REFILL              4150                            //充满后停止充电，电池掉到指定电压后续充
#define CHARGE_NTC_ADC_MAX_TEMP         53                              //设置最小温度 摄氏度      53
#define CHARGE_NTC_ADC_MAX_RE_TEMP      48                              //设置恢复温度 摄氏度     48
#define CHARGE_NTC_ADC_MIN_TEMP         0                               //设置最高温度 摄氏度0
#define CHARGE_NTC_ADC_MIN_RE_TEMP      5                               //设置恢复温度 摄氏度5
#define CHARGE_VIO_0V_EN                0                               //0v充电功能使能，VBAT低于3v，插入DC系统上电工作（VDDBT下电，需要等到退出后才上电）

/*****************************************************************************
 * Module    : 硬件I2C配置
 *****************************************************************************/
#define I2C_HW_EN                       1           //是否使能硬件I2C功能

/*****************************************************************************
 * Module    : 软件I2C配置
 *****************************************************************************/
#define I2C_SW_EN                       0           //是否使能软件I2C功能

#define I2C_SCL_IN()                    {GPIOEDIR |= BIT(5); GPIOEPU  |= BIT(5);}
#define I2C_SCL_OUT()                   {GPIOEDE |= BIT(5); GPIOEDIR &= ~BIT(5);}
#define I2C_SCL_H()                     {GPIOESET = BIT(5);}
#define I2C_SCL_L()                     {GPIOECLR = BIT(5);}
#define I2C_SDA_IN()                    {GPIOEDIR |= BIT(7); GPIOEPU  |= BIT(7);}
#define I2C_SDA_OUT()                   {GPIOEDE |= BIT(7); GPIOEDIR &= ~BIT(7);}
#define I2C_SDA_H()                     {GPIOESET = BIT(7);}
#define I2C_SDA_L()                     {GPIOECLR = BIT(7);}
#define I2C_SDA_IS_H()                  (GPIOE & BIT(7))
#define I2C_SDA_SCL_OUT()               {I2C_SDA_OUT(); I2C_SCL_OUT();}
#define I2C_SDA_SCL_H()                 {I2C_SDA_H(); I2C_SCL_H();}

/*****************************************************************************
 * Module    : 传感器配置
 *****************************************************************************/
#define SENSOR_HUB_EN                   0

#define SENSOR_STEP_SEL                 SENSOR_STEP_NULL
#define SENSOR_STEP_IICX                0
#define SENSOR_STEP_SCL                 IO_PE8
#define SENSOR_STEP_SDA                 IO_PE7
#define SENSOR_STEP_MAP_GX              1
#define SENSOR_STEP_INT                 IO_PE9
#define SENSOR_STEP_INT_VECTOR          PORT_INT4_VECTOR

#define SENSOR_HR_SEL                   SENSOR_HR_NULL
#define SENSOR_HR_IICX                  0
#define SENSOR_HR_SCL                   IO_PE8
#define SENSOR_HR_SDA                   IO_PE7
#define SENSOR_HR_MAP_GX                1
#define SENSOR_HR_INT                   IO_PE10
#define SENSOR_HR_INT_VECTOR            PORT_INT5_VECTOR

#define SENSOR_GEO_SEL                  SENSOR_GEO_NULL
#define SENSOR_GEO_IICX                 1
#define SENSOR_GEO_SCL                  IO_PB9
#define SENSOR_GEO_SDA                  IO_PB8
#define SENSOR_GEO_MAP_GX               3

/*****************************************************************************
 * Module    : Loudspeaker mute检测配置
 *****************************************************************************/
#define LOUDSPEAKER_MUTE_EN             1           //是否使能功放MUTE
#define LOUDSPEAKER_MUTE_INIT()         loudspeaker_mute_init()
#define LOUDSPEAKER_MUTE_DIS()          loudspeaker_disable()
#define LOUDSPEAKER_MUTE()              loudspeaker_mute()
#define LOUDSPEAKER_UNMUTE()            loudspeaker_unmute()
#define LOUDSPEAKER_MUTE_PORT           IO_PE0
#define LOUDSPEAKER_HIGH_MUTE           0           //高电平为MUTE状态
#define LOUDSPEAKER_UNMUTE_DELAY        6           //UNMUTE延时配置，单位为5ms

#define AMP_CTRL_AB_D_EN                0           //功放AB/D类控制
#define AMP_CTRL_AB_D_PORT              IO_NONE     //控制IO
#define AMP_CTRL_AB_D_TYPE              0           //0:独立IO电平控制, 1:mute脉冲控制
#define AMPLIFIER_SEL_INIT()            amp_sel_cfg_init(AMP_CTRL_AB_D_PORT)
#define AMPLIFIER_SEL_D()               amp_sel_cfg_d()
#define AMPLIFIER_SEL_AB()              amp_sel_cfg_ab()

/*****************************************************************************
 * Module    : SD卡配置，MMC供电需要选择VDDIO为3.5V以上
 *****************************************************************************/
#define SD_DETECT_IO                    IO_NONE//IO_MUX_SDCLK
#define SD_DATA_BUS_4BIT_EN             0           //是否使用sd data bus 4bit width
#define SD_DATA_BUS_DDR_EN              0           //单线不支持DDR模式
#if !ELUNCHBOX_PANEL_EN
#define SD_SOFT_DETECT_EN               1           //是否使用软件检测 (SD发命令检测)
#endif

#define SD_IS_SOFT_DETECT()             SD_SOFT_DETECT_EN             //配置工具中选则63是软件检测.
#define SD_DETECT_INIT()                sdcard_detect_init()
#define SD_IS_ONLINE()                  sdcard_is_online()
#define IS_DET_SD_BUSY()                is_det_sdcard_busy()

#define SD0_LDO_EN()                    {nand_pg_on(); sys_cb.sd_power_off_flag = false;}
#define SD0_LDO_DIS()                   {nand_pg_off(); sys_cb.sd_power_off_flag = true;}

/*****************************************************************************
 * Module    : IO SD0配置，MMC供电需要选择VDDIO为3.5V以上
 *****************************************************************************/
#if !ELUNCHBOX_PANEL_EN
#define SD0_MAPPING                     SD0MAP_G2   //选择SD0 mapping
#endif
#define SD0_CLKGAT_EN()                 CLKGAT0 |= BIT(16);

/*****************************************************************************
 * Module    : IO SD1配置，MMC供电需要选择VDDIO为3.5V以上
 *****************************************************************************/
#define SD1_MAPPING                     SD1MAP_NONE    //选择SD0 mapping
#define SD1_CLKGAT_EN()                 CLKGAT0 |= BIT(21);

/*****************************************************************************
 * Module    : FMRX功能配置, 打开 FUNC_FMRX_EN 以下配置有效
 *****************************************************************************/
#define FMRX_HALF_SEEK_EN               0                       //是否打开半自动搜台
#define FMRX_THRESHOLD_VAL              xcfg_cb.fmrx_r_val      //内置FMRX搜台阈值(0~255), 值越小台越多，假台也可能增多。 // 128
#define FMRX_THRESHOLD_Z                xcfg_cb.fmrx_z_val      //该值越大台越多, 想减少很弱的台，可以适当改小            //1100
#define FMRX_THRESHOLD_FZ               xcfg_cb.fmrx_fz_val     //该值越大台越多, 想减少很弱的台，可以适当改小            //600
#define FMRX_THRESHOLD_D                xcfg_cb.fmrx_d_val      //3000
#define FMRX_AUDIO_CHANNEL              xcfg_cb.fmrx_audio_ch   //FM声道输出声道,一般单声道比双声道噪音会小些  //0 Mono  //1 Dual
#define FMRX_OPTIMIZE_TRY               0                       //FM 收台效果尝试优化,可以修改CLK控制等,需要实际样机去测试效果  //该功能未实际调试
#define FMRX_TEST_CHANNEL               0                       //FM 固定某些电台测试,可用于对比其它样机,定位到特定的一些台对比声音清晰度

/*****************************************************************************
 * Module    : usb device 功能选择
 *****************************************************************************/
#define UDE_STORAGE_EN                  1
#define UDE_SPEAKER_EN                  1
#define UDE_HID_EN                      1
#define UDE_MIC_EN                      1
#define UDE_MIC_SPR                     SPR_48000       //usb mic采样率
#define UDE_MIC_NCH                     2               //usb mic声道数
#define UDE_MIC_24BIT                   0               //usb mic adc 16bit or 24bit

/*****************************************************************************
 * Module    : 提示音 功能选择
 *****************************************************************************/
#define WARNING_TONE_EN                 1            //是否打开提示音功能, 总开关
#define WARING_MAXVOL_MP3               0            //最大音量提示音WAV或MP3选择， 播放WAV可以与MUSIC叠加播放。
#define WARNING_WAVRES_PLAY             0            //是否支持WAV提示音播放
#define WARNING_VOLUME                  xcfg_cb.warning_volume   //播放提示音的音量级数
#define LANG_SELECT                     LANG_EN      //提示音语言选择

#define WARNING_POWER_ON                1
#define WARNING_POWER_OFF               0
#if ELUNCHBOX_PANEL_EN
#undef  WARNING_POWER_ON
#define WARNING_POWER_ON                0           /* 饭盒：跳过开机 MP3，避免阻塞进 Home */
#endif
#define WARNING_BT_INCALL               1            //是否打开蓝牙来电提示音

/*****************************************************************************
 * Module    :外接加密芯片security_pay配置
 *****************************************************************************/
#define SECURITY_PAY_EN                 0                       //打开支付宝功能
#define SECURITY_TRANSITCODE_EN         0 * SECURITY_PAY_EN     //打开乘车码功能
#define SECURITY_PAY_SE3                1   //  0:SE2.0   1: SE3.0, 同步修改脚本参数，然后rebuild
                                            //project->build options->build steps需要同步修改脚本参数SE_VERSION_V2 SE_VERSION_V3
#define SECURITY_PAY_VENDOR             SECURITY_VENDOR_HS
#define SECURITY_PAY_IICX               0
#define SECURITY_PAY_SCL                IO_PE8
#define SECURITY_PAY_SDA                IO_PE7
#define SECURITY_PAY_MAP_GX             1
#define SECURITY_PAY_LOG                0   //打开支付宝log
#define OPEN_CSI_LOG                    0   //打开csi接口log
#define HS_TEST_EN                      0   //打开宏思测试程序，打开测试需要2k内存
#define HS_HARDWRAE_IIC                 0   //宏思使用硬件iic
#define HS_SLEEP_EN                     0   //使用睡眠不直接断电，目前不可用，是直接使用断电方式，不使用休眠模式。
#define SECURITY_MALLOC(size)           ab_zalloc(size)     //支付宝csi接口申请内存接口函数  ab_zalloc(size) / func_zalloc(size)
#define SECURITY_FREE(ptr)              ab_free(ptr)        //支付宝csi接口释放内存接口函数  ab_free(ptr) / func_free(ptr)

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
#define FILE_BASE_ADDR                  0x500000
#define FILE_NUM                        10
#define FILE_TOTAL_SIZE                 0x80000
#define FILE_INFO_ADDR                  (FILE_BASE_ADDR + FILE_TOTAL_SIZE)

#undef  sscanf
#undef  sprintf
#undef  EQ_DBG_IN_UART
#undef  EQ_DBG_IN_SPP

#elif SECURITY_PAY_EN
#undef  sscanf
#endif

/*****************************************************************************
 * Module    : 语音方案选择
 *****************************************************************************/
#define ASR_NULL                        0
#define ASR_WS                          1 //华镇
#define ASR_YJ                          2 //友杰
#define ASR_WS_AIR                      3 //华镇空调伴侣

#define ASR_SELECT                      ASR_NULL
#define ASR_FULL_SCENE                  1               //全场景模式
#define ASR_API_CHECK_TIME              1               //API执行时间检测
#define ASR_SAMPLE                      240             //MIC采样率
#define ASR_GAIN                        22              //MIC增益
#define ASR_DEAL_TYPE                   1               //事件处理方式 1:轮询; 0:消息
#define ASR_VOICE_BALL_ANIM             1               //悬浮球动画
#define ASR_USBKEY_PSD                  0               //加密狗
#define ASR_AND_SIRI_PARALLEL_EN        0               //语音SIRI融合功能，当siri开启时，mic数据同时送入siri和asr引擎
#define ASR_SIRI_AUTO_CLOSE             1               //自动关闭siri
#define ASR_SIRI_AUTO_CLOSE_COUNTDOWN   15              //多少秒后自动关闭siri
#define ASR_SIRI_SCO_DELAY_EN           0               //SIRI 延时输出
#define ASR_MIC_DATA_DUMP_EN            0               //语音识别，输出mic原始音频到bluetrum_voice_record

/*****************************************************************************
 * Module    : 高德地图
 *****************************************************************************/
#define AWK_MAP_EN                          0

/*****************************************************************************
 * Module    : AVI视频播放功能
 *****************************************************************************/
#define AVI_DIALPLATE_EN                    0*CHIP_PACKAGE_SUPPORT_PSRAM  //使用视频表盘功能
#define VIDEO_CLK_SEL                       SYS_192M                    //AVI选择的系统时钟
#define VIDEO_PLAY_EN                       0                           //AVI视频播放功能
#define AVI_USE_SD                          (1)*VIDEO_PLAY_EN           //AVI是否使用SD卡
#define AVI_USE_PSRAM                       (1)*VIDEO_PLAY_EN*CHIP_PACKAGE_SUPPORT_PSRAM           //是否使用PSRAM

/*****************************************************************************
 * Module    : JPEG显示解码功能
 *****************************************************************************/
#if !ELUNCHBOX_PANEL_EN
#define PHOTO_VIEW_EN                       1                           //jpeg照片解码显示功能
#endif
#define PHOTO_CLK_SEL                       SYS_192M                    //JPEG选择的系统时钟
#define PHOTO_USE_SD                        (1)*PHOTO_VIEW_EN           //JPEG显示是否使用SD卡
#define PHOTO_USE_PSRAM                     (1)*PHOTO_VIEW_EN*CHIP_PACKAGE_SUPPORT_PSRAM           //是否使用PSRAM

/*****************************************************************************
 * Module    : 摄像头拍照录像功能
 *****************************************************************************/
#define VIDEO_RECODE_TAKE_PHOTO_EN          0                               //摄像头录像与拍照功能
#define VIDEO_RECOED_TAKE_PHOTO_SCALE_EN    0                               //拍照和录像时是否缩放, 缩放要用3个摄像头分变率大小
#define WATER_MARK_EN                       (1)*VIDEO_RECODE_TAKE_PHOTO_EN  //摄像头录像拍照水印
#if ELUNCHBOX_PANEL_EN
#define CAMERA_POWER_ENABLE()               ((void)0)   /* PE2=PT8028 D0，禁止占用为 LDO 输出 */
#define CAMERA_POWER_DISABLE()              ((void)0)
#else
#define CAMERA_POWER_ENABLE()               {ldo1_enable(IO_PE2);}          //摄像头电源开
#define CAMERA_POWER_DISABLE()              {ldo1_disable(IO_PE2);}         //摄像头电源关
#endif
#define DVP_CLK_SEL                         SYS_192M                        //打开摄像头选择的系统时钟
#define CAMERA_USE_SD                       (1)*VIDEO_RECODE_TAKE_PHOTO_EN  //摄像头是否使用SD卡，录像必要SD卡
#define CAMERA_USE_PSRAM                    (1)*VIDEO_RECODE_TAKE_PHOTO_EN*CHIP_PACKAGE_SUPPORT_PSRAM  //是否使用psram内存区驱动摄像头

#define AVI_DVP_USE_CAMERA                  VIDEO_RECODE_TAKE_PHOTO_EN

#define IMAGE_SENSOR_WIDTH                  240                             //支持最大分辨率的摄像头宽
#define IMAGE_SENSOR_HEIGHT                 320                             //支持最大分辨率的摄像头高
#if CHIP_PACKAGE_SUPPORT_PSRAM
#define DVP_AUTO_RATIO                      1
#else
#define DVP_AUTO_RATIO                      2
#endif // CHIP_PACKAGE_SUPPORT_PSRAM
#define DVP_DISP_WIDTH                      (IMAGE_SENSOR_WIDTH/DVP_AUTO_RATIO)
#define DVP_DISP_HIGHT                      (IMAGE_SENSOR_HEIGHT/DVP_AUTO_RATIO)

#define DVP_DMA_LINE_NUM                    32      //多少行
#define DVP_DISP_ROTATE_90                  0       //是否旋转90度

#if AVI_DVP_USE_CAMERA
#define IMG_SENSOR_NO                       0
#define IMGA_AUTO_SELECT                    1       //自动读取ID选择
#define IMGA_MEMBERS_SELECT                 2       //再指定数组里选

//图像传感器IO分配,使用模拟iic
#define IMG_SENSOR_SELECT                   IMGA_MEMBERS_SELECT
#define IMG_SENSOR_NUM                      1               //前后摄像头选择2，单独一个选择1
#define PORT_IMAGE_SENSOR_CLK               IO_PA8
#define PORT_IMAGE_SENSOR_SDA               IO_PA9
#define PORT_IMAGE_SENSOR_RST1              IO_NONE
#define PORT_IMAGE_SENSOR_RST2              IO_NONE
#define PORT_IMAGE_SENSOR_PWDN1             IO_PA15
#define PORT_IMAGE_SENSOR_PWDN2             IO_NONE

#define PORT_IMAGE_SENSOR_DVP_D7            IO_NONE
#define PORT_IMAGE_SENSOR_DVP_D6            IO_NONE
#define PORT_IMAGE_SENSOR_DVP_D5            IO_NONE
#define PORT_IMAGE_SENSOR_DVP_D4            IO_NONE
#define PORT_IMAGE_SENSOR_DVP_D3            IO_NONE
#define PORT_IMAGE_SENSOR_DVP_D2            IO_NONE
#define PORT_IMAGE_SENSOR_DVP_D1            IO_NONE
#if ELUNCHBOX_PANEL_EN
#define PORT_IMAGE_SENSOR_DVP_D0            IO_NONE   /* PE4 = PT8028 D2 */
#else
#define PORT_IMAGE_SENSOR_DVP_D0            IO_PE4
#endif

#define PORT_IMAGE_SENSOR_HSYNC             IO_NONE
#define PORT_IMAGE_SENSOR_VSYNC             IO_NONE

#define PORT_IMAGE_SENSOR_VCLK              IO_PE5
#define PORT_IMAGE_SENSOR_MCLK              IO_PE6

#define IMG_SENSOR_BF03A2_EN                1       //是否打开BF03A2驱动  分辨率宽*高（240*320），SPI摄像头
#define IMG_SENSOR_GC0308_EN                0       //是否打开GC0308驱动  分辨率宽*高（640*480）, DVP摄像头
#define IMG_SENSOR_SPA0A39_EN               0       //是否打开SPA0A39驱动 分辨率宽*高（640*480），DVP摄像头
#define VIDEO_RECODE_FULL_SCREEN            0
#endif

/*****************************************************************************
 * Module    : 摄像头拍照录像、JPEG显示解码功能、AVI视频播放功能 界面事例
 *****************************************************************************/
#define AVI_DVP_DEMOLIST                    (PHOTO_VIEW_EN || VIDEO_RECODE_TAKE_PHOTO_EN || VIDEO_PLAY_EN)


/*****************************************************************************
 * Module    : FLASH MAP 说明（新加的功能请放在上面）
 *****************************************************************************
 **0x0
 *4K                           (BOOT1)
 *4K                           (BOOT2)
 **0x2000
 *FLASH_CODE_SIZE              (app.bin)
 *FLASH_UI_SIZE                (ui.bin)
 ***
 *FLASH_UPDATE_PACKAGE_SIZE    (压缩升级包存放区域)
 ***
 *FLASHDB_ADDR                 （FLASHDB_EN == 1 时，16K）
 ***
 *FLASH_AB_PARAM_ADDR          (若USE_APP_TYPE == USE_AB_APP, 则有app协议保存数据)
 ***
 *FLASH_CM_SIZE                (参数区)
 *1.程序使用空间从0x2000地址开始存放，使用大小为FLASH_CODE_SIZE
 *2.UI资源文件可自定义大小，起始地址要在code区域后，不能有地址重叠
 *3.如需压缩升级，升级包存放地址要在UI.bin资源之后
 *4.参数区起始地址为：FLASH_SIZE - FLASH_CM_SIZE
 *****************************************************************************/

/*****************************************************************************
 * Module    :线程大小配置
 *****************************************************************************/
///总堆栈
#define MEM_HEAP_SIZE                   18432                                               /*基础功能所需堆栈大小*/\
                                        + 1024*(OPUS_ENC_EN)                                /*OPUS堆栈大小*/\
                                        + 2048*(ASR_SELECT)                                 /*ASR堆栈大小*/\
                                        + 2560*(SECURITY_PAY_EN)                            /*支付宝堆栈大小*/\
                                        + 11264*(SECURITY_PAY_EN*SECURITY_TRANSITCODE_EN)   /*乘车码堆栈大小*/\
                                        + 1792*(VIDEO_PLAY_EN || PHOTO_VIEW_EN || VIDEO_RECODE_TAKE_PHOTO_EN)
///main线程堆栈
#define OS_THREAD_MAIN_STACK            4608                                                /*基础功能所需堆栈大小*/\
                                        + 2560*(SECURITY_PAY_EN)                            /*支付宝堆栈大小*/\
                                        + 11264*(SECURITY_PAY_EN*SECURITY_TRANSITCODE_EN)   /*乘车码堆栈大小*/\
///MUSIC线程堆栈
#define OS_THREAD_MUSIC_STACK           896                                                 /*基础功能所需堆栈大小*/\
                                        + (1024) * OPUS_ENC_EN                              /*OPUS堆栈大小*/\
///ASR线程堆栈
#define OS_THREAD_ASR_STACK             2048        //堆栈大小
#define OS_THREAD_ASR_TICK              -1
#define OS_THREAD_ASR_PRIORITY          28          //优先级

#include "config_extra.h"

/* config_extra 在 GUI_USE_BLUR=1 时会强开 SCREENSHOOT/NOC_PSRAM，但 NOC_PSRAM_SIZE=0 时无 psram 段，.psram_buf.lcd 会进 flash */
#if (PSRAM_SIZE == 0)
#undef GUI_USE_SCREENSHOOT
#undef GUI_USE_BLUR
#define GUI_USE_SCREENSHOOT             0
#define GUI_USE_BLUR                    0
#endif

#if !BT_PANU_EN
#undef FLASH_CODE_SIZE
#define FLASH_CODE_SIZE                 FLASH_CODE_BASE_SIZE                                /*基础功能所需FLASH*/\
                                        + 0x4000*(VIDEO_PLAY_EN)                            /*VIDEO PLAY*/\
                                        + 0x3000*(PHOTO_VIEW_EN||WATER_MARK_EN)             /*PHOTO VIEW 或 WATER MARK*/\
                                        + 0x4000*(VIDEO_RECODE_TAKE_PHOTO_EN)               /*VIDEO_RECODE_TAKE_PHOTO_EN*/\
                                        + 0x2000*(FUNC_MUSIC_EN)                            /*SD卡播放音乐功能界面*/\
                                        + 0x7000*(FUNC_RECORDER_EN)                         /*录音功能界面*/\
                                        + 0x7000*(OPUS_ENC_EN)                              /*OPUS压缩算法*/\
                                        + 0xFA000*(ASR_SELECT)                              /*ASR语音*/\
                                        + 0x15000*(SECURITY_PAY_EN)                         /*支付宝基础功能*/\
                                        + 0xA5000*(SECURITY_TRANSITCODE_EN)                 /*支付宝拓展乘车码功能*/
#endif

#endif // __CONFIG_WATCH_DEV_V1_0_H__
