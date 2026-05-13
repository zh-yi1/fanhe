#if 1
#include "include.h"
#include "func.h"
#include "func_clock.h"

#define TRACE_EN    0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


#define CARD_SCALE_START_POS            265                                                                     //卡片开始缩小位置
#define CARD_SCALE_STOP_POS             380                                                                     //卡片停止缩小位置
#define CARD_SCALE_STOP_Y               (CARD_SCALE_STOP_POS + 156 * 2)                                         //卡片停止缩小实际位置（越大缩小越慢，可见叠加越多）
#define CARD_WIDTH_MIN                  200                                                                     //卡片缩小的最小宽度
#define CARD_UNVISIBLE_POS_TOP          (-160)                                                                  //卡片超出屏幕顶部某位置后设为不可见
#define CARD_UNVISIBLE_POS_BOTTOM       (CARD_SCALE_STOP_Y + 156)                                               //卡片缩到最小后实际位置大于某值设为不可见
#define CARD_BG_GRAY_START              70                                                                      //卡片开始缩小位置处背景的灰度
#define CARD_BG_GRAY_STOP               20                                                                      //卡片停止缩小位置处背景的灰度
#define GRAY_LV(x)                      make_color(x, x, x)                                                     //生成灰度颜色值（0:黑 255:白）

#define SPRING_Y_MAX                    0                                                                       //回弹到顶部时的ofs_y
#define SPRING_Y_MIN                    (GUI_SCREEN_CENTER_Y - card_y_info[CARD_COUNT - 1])                     //回弹到底部时的ofs_y
#define DRAG_Y_MAX                      (SPRING_Y_MAX + 30)                                                     //拖动到顶部时的ofs_y
#define DRAG_Y_MIN                      (SPRING_Y_MIN - 78)                                                     //拖动到底部时的ofs_y

#define FOCUS_AUTO_TICK_EXPIRE          18                                                                      //松手后自动对齐焦点单位时间(ms)
#define FOCUS_AUTO_STEP                 16                                                                      //松手后自动对齐焦点单位时间步进
#define FOCUS_AUTO_STEP_DIV             6
#define DRAG_AUTO_SPEED                 16                                                                      //拖动松手后的速度(惯性)

#define EXIT_Y_MAX                      60                                                                      //下滑退出触摸点离屏幕顶部小于某值
#define TXT_CNT_MAX                     8                                                                       //文本框最大字符数
#define CARD_BTN_COUNT                  (COMPO_ID_CARD_POWEROFF_ASSISTANT - COMPO_ID_CARD_SPORT_COMPASS + 1)    //卡片（按钮）数量

///卡片、组件原始位置（相对于卡片中心点）
#define CARD_WIDTH_ORG                  GUI_SCREEN_WIDTH//308
//时钟、日期
#define CARD_CLOCK_X                    GUI_SCREEN_CENTER_X                     //中心点
#define CARD_CLOCK_Y                    80
#define CLOCK_BG_X                      (149 - CARD_CLOCK_X)                     //时钟背景
#define CLOCK_BG_Y                      0
#define CLOCK_BG_W                      120
#define CLOCK_BG_H                      120
#define CLOCK_POINTER_H_X               CLOCK_BG_X                              //时钟指针hour
#define CLOCK_POINTER_H_Y               CLOCK_BG_Y
#define CLOCK_POINTER_H_W               38
#define CLOCK_POINTER_H_H               6
#define CLOCK_POINTER_M_X               CLOCK_BG_X                              //时钟指针minute
#define CLOCK_POINTER_M_Y               CLOCK_BG_Y
#define CLOCK_POINTER_M_W               52
#define CLOCK_POINTER_M_H               6
#define DATE_X                          (297 - CARD_CLOCK_X)                    //日期
#define DATE_Y                          (65 - CARD_CLOCK_Y)
#define DATE_W                          100
#define DATE_H                          35
#define WEEKDAY_X                       DATE_X                                  //星期
#define WEEKDAY_Y                       (99 - CARD_CLOCK_Y)
#define WEEKDAY_W                       70
#define WEEKDAY_H                       DATE_H
//运动&指南针卡片
#define CARD_SPORT_COMPASS_X            GUI_SCREEN_CENTER_X                     //中心点
#define CARD_SPORT_COMPASS_Y            228
#define SPORT_BG_X                      (152 - CARD_SPORT_COMPASS_X)             //运动背景/按钮
#define SPORT_BG_Y                      0
#define SPORT_BG_W                      (GUI_SCREEN_WIDTH/2)
#define SPORT_BG_H                      146
#define SPORT_ICON_X                    (SPORT_BG_X - 20)                       //图标
#define SPORT_ICON_Y                    (SPORT_BG_Y - 20)
#define SPORT_ICON_W                    72
#define SPORT_ICON_H                    72
#define SPORT_TXT_X                     SPORT_BG_X                              //文本
#define SPORT_TXT_Y                     45
#define SPORT_TXT_W                     140
#define SPORT_TXT_H                     35
#define COMPASS_BG_X                    (314 - CARD_SPORT_COMPASS_X)            //指南针背景/按钮
#define COMPASS_BG_Y                    SPORT_BG_Y
#define COMPASS_BG_W                    SPORT_BG_W
#define COMPASS_BG_H                    SPORT_BG_H
#define COMPASS_ICON_X                  (COMPASS_BG_X - 20)                     //图标
#define COMPASS_ICON_Y                  SPORT_ICON_Y
#define COMPASS_ICON_W                  SPORT_ICON_W
#define COMPASS_ICON_H                  SPORT_ICON_H
#define COMPASS_TXT_X                   COMPASS_BG_X                            //文本
#define COMPASS_TXT_Y                   SPORT_TXT_Y
#define COMPASS_TXT_W                   (SPORT_TXT_W+10)
#define COMPASS_TXT_H                   SPORT_TXT_H
//运动记录卡片
#define CARD_ACTIVITY_X                 GUI_SCREEN_CENTER_X                     //中心点
#define CARD_ACTIVITY_Y                 393
#define ACTIVITY_BG_X                   0                                       //背景/按钮
#define ACTIVITY_BG_Y                   0
#define ACTIVITY_BG_W                   CARD_WIDTH_ORG
#define ACTIVITY_BG_H                   156
#define ACTIVITY_ICON_X                 SPORT_BG_X                              //图标
#define ACTIVITY_ICON_Y                 0
#define ACTIVITY_ICON_W                 120
#define ACTIVITY_ICON_H                 120
#define ACTIVITY_STEP_X                 COMPASS_BG_X                            //步数
#define ACTIVITY_STEP_Y                 -40
#define ACTIVITY_STEP_W                 (COMPASS_BG_W - 10)
#define ACTIVITY_STEP_H                 35
#define ACTIVITY_CALORIE_X              ACTIVITY_STEP_X                         //卡路里
#define ACTIVITY_CALORIE_Y              0
#define ACTIVITY_CALORIE_W              ACTIVITY_STEP_W
#define ACTIVITY_CALORIE_H              ACTIVITY_STEP_H
#define ACTIVITY_DISTANCE_X             ACTIVITY_STEP_X                         //距离
#define ACTIVITY_DISTANCE_Y             40
#define ACTIVITY_DISTANCE_W             ACTIVITY_STEP_W
#define ACTIVITY_DISTANCE_H             ACTIVITY_STEP_H
//睡眠卡片
#define CARD_SLEEP_X                    GUI_SCREEN_CENTER_X
#define CARD_SLEEP_Y                    563
#define SLEEP_BG_X                      ACTIVITY_BG_X                           //背景/按钮
#define SLEEP_BG_Y                      ACTIVITY_BG_Y
#define SLEEP_BG_W                      CARD_WIDTH_ORG
#define SLEEP_BG_H                      ACTIVITY_BG_H
#define SLEEP_ICON_DEEP_X               (119 - CARD_SLEEP_X)                     //深睡图标
#define SLEEP_ICON_DEEP_Y               (584 - CARD_SLEEP_Y)
#define SLEEP_ICON_DEEP_W               50
#define SLEEP_ICON_DEEP_H               50
#define SLEEP_ICON_LIGHT_X              (266 - CARD_SLEEP_X)                    //浅睡图标
#define SLEEP_ICON_LIGHT_Y              SLEEP_ICON_DEEP_Y
#define SLEEP_ICON_LIGHT_W              SLEEP_ICON_DEEP_W
#define SLEEP_ICON_LIGHT_H              SLEEP_ICON_DEEP_H
#define SLEEP_TOTAL_X                   0                                       //总睡眠时间
#define SLEEP_TOTAL_Y                   (520 - CARD_SLEEP_Y)
#define SLEEP_TOTAL_W                   110
#define SLEEP_TOTAL_H                   35
#define SLEEP_DEEP_H_X                  (186 - CARD_SLEEP_X)                    //深睡时间hour
#define SLEEP_DEEP_H_Y                  (565 - CARD_SLEEP_Y)
#define SLEEP_DEEP_H_W                  90
#define SLEEP_DEEP_H_H                  SLEEP_TOTAL_H
#define SLEEP_DEEP_M_X                  SLEEP_DEEP_H_X                          //深睡时间min
#define SLEEP_DEEP_M_Y                  (604 - CARD_SLEEP_Y)
#define SLEEP_DEEP_M_W                  SLEEP_DEEP_H_W
#define SLEEP_DEEP_M_H                  SLEEP_DEEP_H_H
#define SLEEP_LIGHT_H_X                 (334 - CARD_SLEEP_X)                    //浅睡时间hour
#define SLEEP_LIGHT_H_Y                 SLEEP_DEEP_H_Y
#define SLEEP_LIGHT_H_W                 SLEEP_DEEP_H_W
#define SLEEP_LIGHT_H_H                 SLEEP_DEEP_H_H
#define SLEEP_LIGHT_M_X                 SLEEP_LIGHT_H_X                         //浅睡时间min
#define SLEEP_LIGHT_M_Y                 SLEEP_DEEP_M_Y
#define SLEEP_LIGHT_M_W                 SLEEP_DEEP_M_W
#define SLEEP_LIGHT_M_H                 SLEEP_DEEP_M_H
//心率卡片
#define CARD_HEARTRATE_X                GUI_SCREEN_CENTER_X
#define CARD_HEARTRATE_Y                733
#define HEARTRATE_BG_X                  ACTIVITY_BG_X                           //背景/按钮
#define HEARTRATE_BG_Y                  ACTIVITY_BG_Y
#define HEARTRATE_BG_W                  CARD_WIDTH_ORG
#define HEARTRATE_BG_H                  ACTIVITY_BG_H
#define HEARTRATE_ICON_X                (121 - CARD_HEARTRATE_X)                 //心率图标
#define HEARTRATE_ICON_Y                (696 - CARD_HEARTRATE_Y)
#define HEARTRATE_ICON_W                32
#define HEARTRATE_ICON_H                32
#define HEARTRATE_LINE_X                HEARTRATE_BG_X                          //折线背景
#define HEARTRATE_LINE_Y                (756 - CARD_HEARTRATE_Y)
#define HEARTRATE_LINE_W                300
#define HEARTRATE_LINE_H                72
#define HEARTRATE_VALUE_X               (177 - CARD_HEARTRATE_X)                //心率值
#define HEARTRATE_VALUE_Y               HEARTRATE_ICON_Y
#define HEARTRATE_VALUE_W               60
#define HEARTRATE_VALUE_H               35
//音乐卡片
#define CARD_MUSIC_X                    GUI_SCREEN_CENTER_X
#define CARD_MUSIC_Y                    903
#define MUSIC_BG_X                      ACTIVITY_BG_X                           //背景/按钮
#define MUSIC_BG_Y                      ACTIVITY_BG_Y
#define MUSIC_BG_W                      CARD_WIDTH_ORG
#define MUSIC_BG_H                      ACTIVITY_BG_H
#define MUSIC_PREV_X                    (126 - CARD_MUSIC_X)                     //上一首
#define MUSIC_PREV_Y                    MUSIC_BG_Y
#define MUSIC_PREV_W                    70
#define MUSIC_PREV_H                    70
#define MUSIC_PP_X                      MUSIC_BG_X                             //播放暂停
#define MUSIC_PP_Y                      MUSIC_BG_Y
#define MUSIC_PP_W                      104
#define MUSIC_PP_H                      104
#define MUSIC_NEXT_X                    (340 - CARD_MUSIC_X)                    //下一首
#define MUSIC_NEXT_Y                    MUSIC_BG_Y
#define MUSIC_NEXT_W                    MUSIC_PREV_W
#define MUSIC_NEXT_H                    MUSIC_PREV_H
//关机&语音助手卡片
#define CARD_POWEROFF_ASSISTANT_X       GUI_SCREEN_CENTER_X                     //中心点
#define CARD_POWEROFF_ASSISTANT_Y       1068
#define POWEROFF_BG_X                   SPORT_BG_X                              //关机背景/按钮
#define POWEROFF_BG_Y                   SPORT_BG_Y
#define POWEROFF_BG_W                   SPORT_BG_W
#define POWEROFF_BG_H                   SPORT_BG_H
#define POWEROFF_ICON_X                 SPORT_ICON_X                            //图标
#define POWEROFF_ICON_Y                 SPORT_ICON_Y
#define POWEROFF_ICON_W                 SPORT_ICON_W
#define POWEROFF_ICON_H                 SPORT_ICON_H
#define POWEROFF_TXT_X                  SPORT_TXT_X                             //文本
#define POWEROFF_TXT_Y                  SPORT_TXT_Y
#define POWEROFF_TXT_W                  (SPORT_TXT_W+10)
#define POWEROFF_TXT_H                  SPORT_TXT_H
#define ASSISTANT_BG_X                  COMPASS_BG_X                            //语音助手背景/按钮
#define ASSISTANT_BG_Y                  COMPASS_BG_Y
#define ASSISTANT_BG_W                  COMPASS_BG_W
#define ASSISTANT_BG_H                  COMPASS_BG_H
#define ASSISTANT_ICON_X                COMPASS_ICON_X                          //图标
#define ASSISTANT_ICON_Y                COMPASS_ICON_Y
#define ASSISTANT_ICON_W                COMPASS_ICON_W
#define ASSISTANT_ICON_H                COMPASS_ICON_H
#define ASSISTANT_TXT_X                 COMPASS_TXT_X                           //文本
#define ASSISTANT_TXT_Y                 COMPASS_TXT_Y
#define ASSISTANT_TXT_W                 COMPASS_TXT_W
#define ASSISTANT_TXT_H                 COMPASS_TXT_H

enum {
    CARD_ID_CLOCK = 0,
    CARD_ID_SPORT_COMPASS,
    CARD_ID_ACTIVITY,
    CARD_ID_SLEEP,
    CARD_ID_HEARTRATE,
    CARD_ID_MUSIC,
    CARD_ID_POWEROFF_ASSISTANT,

    CARD_COUNT,
};

//卡片原始位置信息
static const s16 card_y_info[CARD_COUNT] = {
    [CARD_ID_CLOCK]                 = CARD_CLOCK_Y,
    [CARD_ID_SPORT_COMPASS]         = CARD_SPORT_COMPASS_Y,
    [CARD_ID_ACTIVITY]              = CARD_ACTIVITY_Y,
    [CARD_ID_SLEEP]                 = CARD_SLEEP_Y,
    [CARD_ID_HEARTRATE]             = CARD_HEARTRATE_Y,
    [CARD_ID_MUSIC]                 = CARD_MUSIC_Y,
    [CARD_ID_POWEROFF_ASSISTANT]    = CARD_POWEROFF_ASSISTANT_Y,
};

//组件ID
enum {
    COMPO_ID_CLOCK_BG = 256,
    COMPO_ID_CLOCK_H,
    COMPO_ID_CLOCK_M,
    COMPO_ID_DATE,
    COMPO_ID_WEEKDAY,

    COMPO_ID_CARD_SPORT_COMPASS,    //靠前的卡片优先扫描（按钮）
    COMPO_ID_CARD_ACTIVITY,
    COMPO_ID_CARD_SLEEP,
    COMPO_ID_CARD_HEARTRATE,
    COMPO_ID_CARD_MUSIC,
    COMPO_ID_CARD_POWEROFF_ASSISTANT,
};

//功能结构体
typedef struct f_card_t_ {
    s32 ofs_y;
    s32 ofs_y_drag;
    s8 focus_card;                  //靠近屏幕中央的卡片序号
    bool flag_drag;                 //开始拖动
    bool flag_move_auto;            //自动移到坐标
    s32 moveto_y;                   //设定自动移到的坐标
    s16 moveto_card;                //设定自动移到的卡片序号
    u32 tick;
    compo_picturebox_t *blur_pic;
} f_card_t;

extern void bt_control_play_pause(void); //蓝牙音乐播放暂停
extern void bt_control_prev(void);  //蓝牙音乐切换上一曲
extern void bt_control_next(void);  //蓝牙音乐切换下一曲
compo_form_t *func_clock_form_create_by_screenshoot(void);


static bool music_update_status = 0;
static bool music_control_status = 0;

static bool bt_music_status_get(void)
{
    bool status = false;

    if (bt_is_connected()) {
        status = a2dp_is_playing_fast();
    } else if (ble_ams_is_connected()) {
        //todo:
    }
    return status;
}

//创建组件
static void func_clock_sub_card_compo_create(compo_form_t *frm)
{
    compo_cardbox_t *cardbox;
    compo_picturebox_t *pic;
    compo_textbox_t *txt;
    compo_datetime_t *dt;

    //关机&语音助手
    cardbox = compo_cardbox_create(frm, 2, 2, 2, CARD_WIDTH_ORG, POWEROFF_BG_H);
    compo_setid(cardbox, COMPO_ID_CARD_POWEROFF_ASSISTANT);
    compo_cardbox_rect_set_location(cardbox, 0, ASSISTANT_BG_X, ASSISTANT_BG_Y, ASSISTANT_BG_W, ASSISTANT_BG_H, 20);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_ICON_VOICE_BIN);
    compo_cardbox_icon_set_location(cardbox, 0, ASSISTANT_ICON_X, ASSISTANT_ICON_Y, ASSISTANT_ICON_W, ASSISTANT_ICON_H);
    compo_cardbox_text_set(cardbox, 0, i18n[STR_VOICE]);
    compo_cardbox_text_set_location(cardbox, 0, ASSISTANT_TXT_X, ASSISTANT_TXT_Y, ASSISTANT_TXT_W, ASSISTANT_TXT_H);
    compo_cardbox_rect_set_location(cardbox, 1, POWEROFF_BG_X, POWEROFF_BG_Y, POWEROFF_BG_W, POWEROFF_BG_H, 20);
    compo_cardbox_icon_set(cardbox, 1, UI_BUF_ICON_OFF_BIN);
    compo_cardbox_icon_set_location(cardbox, 1, POWEROFF_ICON_X, POWEROFF_ICON_Y, POWEROFF_ICON_W, POWEROFF_ICON_H);
    compo_cardbox_text_set(cardbox, 1, i18n[STR_SETTING_OFF]);
    compo_cardbox_text_set_location(cardbox, 1, POWEROFF_TXT_X, POWEROFF_TXT_Y, POWEROFF_TXT_W, POWEROFF_TXT_H);
    //音乐
    cardbox = compo_cardbox_create(frm, 1, 3, 0, CARD_WIDTH_ORG, MUSIC_BG_H);
    compo_setid(cardbox, COMPO_ID_CARD_MUSIC);
    compo_cardbox_rect_set_location(cardbox, 0, MUSIC_BG_X, MUSIC_BG_Y, MUSIC_BG_W, MUSIC_BG_H, 20);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_MUSIC_PREV_BIN);
    widget_set_alpha(cardbox->icon[0], UI_BTN_CLICK_EFFECT_ALPHA1);
    compo_cardbox_icon_set_location(cardbox, 0, MUSIC_PREV_X, MUSIC_PREV_Y, MUSIC_PREV_W, MUSIC_PREV_H);
    compo_cardbox_icon_set(cardbox, 1, music_control_status ? UI_BUF_MUSIC_PAUSE_BIN : UI_BUF_MUSIC_PLAY_BIN);   //播放/暂停--------->>>todo
    compo_cardbox_icon_set_location(cardbox, 1, MUSIC_PP_X, MUSIC_PP_Y, MUSIC_PP_W, MUSIC_PP_H);
    compo_cardbox_icon_set(cardbox, 2, UI_BUF_MUSIC_NEXT_BIN);
    widget_set_alpha(cardbox->icon[2], UI_BTN_CLICK_EFFECT_ALPHA1);
    compo_cardbox_icon_set_location(cardbox, 2, MUSIC_NEXT_X, MUSIC_NEXT_Y, MUSIC_NEXT_W, MUSIC_NEXT_H);
    //心率
    cardbox = compo_cardbox_create(frm, 1, 2, 1, CARD_WIDTH_ORG, HEARTRATE_BG_H);
    compo_setid(cardbox, COMPO_ID_CARD_HEARTRATE);
    compo_cardbox_rect_set_location(cardbox, 0, HEARTRATE_BG_X, HEARTRATE_BG_Y, HEARTRATE_BG_W, HEARTRATE_BG_H, 20);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_HEART_RATE_HR_BG_BIN);
    compo_cardbox_icon_set_location(cardbox, 0, HEARTRATE_LINE_X, HEARTRATE_LINE_Y, HEARTRATE_LINE_W, HEARTRATE_LINE_H);
    compo_cardbox_icon_set(cardbox, 1, UI_BUF_ICON_HEART_RATE_BIN);
    compo_cardbox_icon_set_location(cardbox, 1, HEARTRATE_ICON_X, HEARTRATE_ICON_Y, HEARTRATE_ICON_W, HEARTRATE_ICON_H);
    compo_cardbox_text_set_font(cardbox, 0, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_cardbox_text_set(cardbox, 0, "72");   //心率值--------->>>todo
    compo_cardbox_text_set_location(cardbox, 0, HEARTRATE_VALUE_X, HEARTRATE_VALUE_Y, HEARTRATE_VALUE_W, HEARTRATE_VALUE_H);
    //睡眠
    cardbox = compo_cardbox_create(frm, 1, 2, 5, CARD_WIDTH_ORG, SLEEP_BG_H);
    compo_setid(cardbox, COMPO_ID_CARD_SLEEP);
    compo_cardbox_rect_set_location(cardbox, 0, SLEEP_BG_X, SLEEP_BG_Y, SLEEP_BG_W, SLEEP_BG_H, 20);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SLEEP_SLEEP_BIN);
    compo_cardbox_icon_set_location(cardbox, 0, SLEEP_ICON_DEEP_X, SLEEP_ICON_DEEP_Y, SLEEP_ICON_DEEP_W, SLEEP_ICON_DEEP_H);
    compo_cardbox_icon_set(cardbox, 1, UI_BUF_SLEEP_LIGHT_SLEEP_BIN);
    compo_cardbox_icon_set_location(cardbox, 1, SLEEP_ICON_LIGHT_X, SLEEP_ICON_LIGHT_Y, SLEEP_ICON_LIGHT_W, SLEEP_ICON_LIGHT_H);
    compo_cardbox_text_set_font(cardbox, 0, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_cardbox_text_set(cardbox, 0, "07:36");    //总时长--------->>>todo
    compo_cardbox_text_set_location(cardbox, 0, SLEEP_TOTAL_X, SLEEP_TOTAL_Y, SLEEP_TOTAL_W, SLEEP_TOTAL_H);
//    compo_cardbox_text_set_font(cardbox, 1, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_cardbox_text_set(cardbox, 1, "02h");      //深睡时长hour--------->>>todo
    compo_cardbox_text_set_location(cardbox, 1, SLEEP_DEEP_H_X, SLEEP_DEEP_H_Y, SLEEP_DEEP_H_W, SLEEP_DEEP_H_H);
//    compo_cardbox_text_set_font(cardbox, 2, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_cardbox_text_set(cardbox, 2, "07m");      //深睡时长min--------->>>todo
    compo_cardbox_text_set_location(cardbox, 2, SLEEP_DEEP_M_X, SLEEP_DEEP_M_Y, SLEEP_DEEP_M_W, SLEEP_DEEP_M_H);
//    compo_cardbox_text_set_font(cardbox, 3, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_cardbox_text_set(cardbox, 3, "05h");      //浅睡时长hour--------->>>todo
    compo_cardbox_text_set_location(cardbox, 3, SLEEP_LIGHT_H_X, SLEEP_LIGHT_H_Y, SLEEP_LIGHT_H_W, SLEEP_LIGHT_H_H);
//    compo_cardbox_text_set_font(cardbox, 4, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_cardbox_text_set(cardbox, 4, "29m");      //浅睡时长min--------->>>todo
    compo_cardbox_text_set_location(cardbox, 4, SLEEP_LIGHT_M_X, SLEEP_LIGHT_M_Y, SLEEP_LIGHT_M_W, SLEEP_LIGHT_M_H);
    //活动记录
    cardbox = compo_cardbox_create(frm, 1, 1, 3, CARD_WIDTH_ORG, ACTIVITY_BG_H);
    compo_setid(cardbox, COMPO_ID_CARD_ACTIVITY);
    compo_cardbox_rect_set_location(cardbox, 0, ACTIVITY_BG_X, ACTIVITY_BG_Y, ACTIVITY_BG_W, ACTIVITY_BG_H, 20);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_ICON_ACTIVITY_BIN);
    compo_cardbox_icon_set_location(cardbox, 0, ACTIVITY_ICON_X, ACTIVITY_ICON_Y, ACTIVITY_ICON_W, ACTIVITY_ICON_H);
    compo_cardbox_text_set_font(cardbox, 0, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_cardbox_text_set(cardbox, 0, "12000");    //步数--------->>>todo
    compo_cardbox_text_set_location(cardbox, 0, ACTIVITY_STEP_X, ACTIVITY_STEP_Y, ACTIVITY_STEP_W, ACTIVITY_STEP_H);
    compo_cardbox_text_set_font(cardbox, 1, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_cardbox_text_set(cardbox, 1, "512.7");    //卡路里--------->>>todo
    compo_cardbox_text_set_location(cardbox, 1, ACTIVITY_CALORIE_X, ACTIVITY_CALORIE_Y, ACTIVITY_CALORIE_W, ACTIVITY_CALORIE_H);
    compo_cardbox_text_set_font(cardbox, 2, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_cardbox_text_set(cardbox, 2, "8.0");      //距离--------->>>todo
    compo_cardbox_text_set_location(cardbox, 2, ACTIVITY_DISTANCE_X, ACTIVITY_DISTANCE_Y, ACTIVITY_DISTANCE_W, ACTIVITY_DISTANCE_H);
    //运动&指南针
    cardbox = compo_cardbox_create(frm, 2, 2, 2, CARD_WIDTH_ORG, SPORT_BG_H);
    compo_setid(cardbox, COMPO_ID_CARD_SPORT_COMPASS);
    compo_cardbox_rect_set_location(cardbox, 0, COMPASS_BG_X, COMPASS_BG_Y, COMPASS_BG_W, COMPASS_BG_H, 20);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_ICON_COMPASS_BIN);
    compo_cardbox_icon_set_location(cardbox, 0, COMPASS_ICON_X, COMPASS_ICON_Y, COMPASS_ICON_W, COMPASS_ICON_H);
    compo_cardbox_text_set(cardbox, 0, i18n[STR_COMPASS]);
    compo_cardbox_text_set_location(cardbox, 0, COMPASS_TXT_X, COMPASS_TXT_Y, COMPASS_TXT_W, COMPASS_TXT_H);
    compo_cardbox_rect_set_location(cardbox, 1, SPORT_BG_X, SPORT_BG_Y, SPORT_BG_W, SPORT_BG_H, 20);
    compo_cardbox_icon_set(cardbox, 1, UI_BUF_ICON_SPORT_BIN);
    compo_cardbox_icon_set_location(cardbox, 1, SPORT_ICON_X, SPORT_ICON_Y, SPORT_ICON_W, SPORT_ICON_H);
    compo_cardbox_text_set(cardbox, 1, i18n[STR_SPORTS]);
    compo_cardbox_text_set_location(cardbox, 1, SPORT_TXT_X, SPORT_TXT_Y, SPORT_TXT_W, SPORT_TXT_H);
    //时钟、日期
    pic = compo_picturebox_create(frm, UI_BUF_PULLUP_DIALPLATE_BG_BIN); //时钟背景
    compo_setid(pic, COMPO_ID_CLOCK_BG);
    dt = compo_datetime_create(frm, UI_BUF_PULLUP_HOUR_BIN);            //时针
    compo_setid(dt, COMPO_ID_CLOCK_H);
    compo_datetime_set_center(dt, 0, CLOCK_POINTER_H_H / 2);
    compo_datetime_set_start_angle(dt, 900);
    compo_bonddata(dt, COMPO_BOND_HOUR);
    dt = compo_datetime_create(frm, UI_BUF_PULLUP_MIN_BIN);             //分针
    compo_setid(dt, COMPO_ID_CLOCK_M);
    compo_datetime_set_center(dt, 0, CLOCK_POINTER_M_H /2);
    compo_datetime_set_start_angle(dt, 900);
    compo_bonddata(dt, COMPO_BOND_MINUTE);
    txt = compo_textbox_create(frm, 5);                                 //日期
    compo_setid(txt, COMPO_ID_DATE);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_bonddata(txt, COMPO_BOND_DATE);
    txt = compo_textbox_create(frm, TXT_CNT_MAX);                       //星期
    compo_setid(txt, COMPO_ID_WEEKDAY);
    compo_bonddata(txt, COMPO_BOND_WEEKDAY);
}

//更新组件位置和大小
static void func_clock_sub_card_compo_update(s32 ofs_y, bool creating)
{
    s16 x, y, w, h, card_y, card_w;
    u8 card_id = 0;
    u16 dc;  //当前卡片中心距屏幕中心距离
    u16 dc_min = GUI_SCREEN_CENTER_Y;
    component_t *compo = (component_t *)compo_pool_get_top();
    while (compo != NULL) {
        //获取初始值
        switch (compo->id) {
        case COMPO_ID_CLOCK_BG:                                 //时钟
            x = CLOCK_BG_X + CARD_CLOCK_X;
            y = CLOCK_BG_Y + CARD_CLOCK_Y;
            w = CLOCK_BG_W;
            h = CLOCK_BG_H;
            card_y = CARD_CLOCK_Y;
            card_id = CARD_ID_CLOCK;
            break;

        case COMPO_ID_CLOCK_H:
            x = CLOCK_POINTER_H_X + CARD_CLOCK_X;
            y = CLOCK_POINTER_H_Y + CARD_CLOCK_Y;
            w = CLOCK_POINTER_H_W;
            h = CLOCK_POINTER_H_H;
            card_y = CARD_CLOCK_Y;
            card_id = CARD_ID_CLOCK;
            break;

        case COMPO_ID_CLOCK_M:
            x = CLOCK_POINTER_M_X + CARD_CLOCK_X;
            y = CLOCK_POINTER_M_Y + CARD_CLOCK_Y;
            w = CLOCK_POINTER_M_W;
            h = CLOCK_POINTER_M_H;
            card_y = CARD_CLOCK_Y;
            card_id = CARD_ID_CLOCK;
            break;

        case COMPO_ID_DATE:
            x = DATE_X + CARD_CLOCK_X;
            y = DATE_Y + CARD_CLOCK_Y;
            w = DATE_W;
            h = DATE_H;
            card_y = CARD_CLOCK_Y;
            card_id = CARD_ID_CLOCK;
            break;

        case COMPO_ID_WEEKDAY:
            x = WEEKDAY_X + CARD_CLOCK_X;
            y = WEEKDAY_Y + CARD_CLOCK_Y;
            w = WEEKDAY_W;
            h = WEEKDAY_H;
            card_y = CARD_CLOCK_Y;
            card_id = CARD_ID_CLOCK;
            break;

        case COMPO_ID_CARD_SPORT_COMPASS:                       //运动&指南针
            x = CARD_SPORT_COMPASS_X;
            y = card_y = CARD_SPORT_COMPASS_Y;
            w = CARD_WIDTH_ORG;
            h = SPORT_BG_H;
            card_id = CARD_ID_SPORT_COMPASS;
            break;

        case COMPO_ID_CARD_ACTIVITY:                            //活动记录
            x = CARD_ACTIVITY_X;
            y = card_y = CARD_ACTIVITY_Y;
            w = ACTIVITY_BG_W;
            h = ACTIVITY_BG_H;
            card_id = CARD_ID_ACTIVITY;
            break;

        case COMPO_ID_CARD_SLEEP:                               //睡眠
            x = CARD_SLEEP_X;
            y = card_y = CARD_SLEEP_Y;
            w = SLEEP_BG_W;
            h = SLEEP_BG_H;
            card_id = CARD_ID_SLEEP;
            break;

        case COMPO_ID_CARD_HEARTRATE:                           //心率
            x = CARD_HEARTRATE_X;
            y = card_y = CARD_HEARTRATE_Y;
            w = HEARTRATE_BG_W;
            h = HEARTRATE_BG_H;
            card_id = CARD_ID_HEARTRATE;
            break;

        case COMPO_ID_CARD_MUSIC:                               //音乐
            x = CARD_MUSIC_X;
            y = card_y = CARD_MUSIC_Y;
            w = MUSIC_BG_W;
            h = MUSIC_BG_H;
            card_id = CARD_ID_MUSIC;
            break;

        case COMPO_ID_CARD_POWEROFF_ASSISTANT:                  //关机&语音助手
            x = CARD_POWEROFF_ASSISTANT_X;
            y = card_y = CARD_POWEROFF_ASSISTANT_Y;
            w = CARD_WIDTH_ORG;
            h = POWEROFF_BG_H;
            card_id = CARD_ID_POWEROFF_ASSISTANT;
            break;

        default:
            compo = compo_get_next(compo);
            continue;
            break;
        }

        //计算偏移、缩放
        card_w = CARD_WIDTH_ORG;
        card_y += ofs_y;
        y += ofs_y;
        dc = abs_s(card_y - GUI_SCREEN_CENTER_Y);
        if (creating == false && dc < dc_min) {  //焦点，创建时不使用f_cb
            f_card_t *f_card = (f_card_t *)func_cb.f_cb;
            f_card->focus_card = card_id;
            dc_min = dc;
//            TRACE("focus[%d]\n", card_id);
        }
        if (card_y > CARD_SCALE_START_POS && card_y < CARD_UNVISIBLE_POS_BOTTOM) {
            if (card_y > CARD_SCALE_STOP_Y) {  //缩到最小后保持一段时间
                y -= card_y - CARD_SCALE_STOP_Y;
                card_y = CARD_SCALE_STOP_Y;
            }
            if (card_w > CARD_WIDTH_MIN) {
                //缩放
                s16 card_w_org = card_w;
                card_w -= (card_w - CARD_WIDTH_MIN) * (card_y - CARD_SCALE_START_POS) / (CARD_SCALE_STOP_Y - CARD_SCALE_START_POS);
                w = w * card_w / card_w_org;
                h = h * card_w / card_w_org;
                x += (GUI_SCREEN_CENTER_X - x) * (card_w_org - card_w) / card_w_org;
                y += (card_y - y) * (card_w_org - card_w) / card_w_org;
                //偏移
                s16 card_y_org = card_y;
                card_y = CARD_SCALE_START_POS + (CARD_SCALE_STOP_POS - CARD_SCALE_START_POS) * (card_y - CARD_SCALE_START_POS) / (CARD_SCALE_STOP_Y - CARD_SCALE_START_POS);
                y += card_y - card_y_org;
            }
        }

        //设置位置和大小
        bool unvisible = card_y < CARD_UNVISIBLE_POS_TOP || card_y >= CARD_UNVISIBLE_POS_BOTTOM;
        if (compo->type == COMPO_TYPE_CARDBOX) {
            compo_cardbox_t *cardbox = (compo_cardbox_t *)compo;
            compo_cardbox_set_location(cardbox, x, y, w, h);
            u8 level = y > CARD_SCALE_START_POS ? \
                            (y < CARD_SCALE_STOP_POS ? \
                                (CARD_BG_GRAY_START - (CARD_BG_GRAY_START - CARD_BG_GRAY_STOP) * (y - CARD_SCALE_START_POS) / (CARD_SCALE_STOP_POS - CARD_SCALE_START_POS)) : \
                                CARD_BG_GRAY_STOP
                            ) : \
                        CARD_BG_GRAY_START;
//            TRACE("gray[%d]\n", level);
            compo_cardbox_rect_set_color(cardbox, 0, GRAY_LV(level));   //背景颜色
            compo_cardbox_rect_set_color(cardbox, 1, GRAY_LV(level));
            if (unvisible) {
                compo_cardbox_set_visible(cardbox, false);
            } else {
                compo_cardbox_set_visible(cardbox, true);
            }
        } else if (compo->type == COMPO_TYPE_PICTUREBOX) {
            compo_picturebox_t *pic = (compo_picturebox_t *)compo;
            compo_picturebox_set_pos(pic, x, y);
            compo_picturebox_set_size(pic, w, h);
            if (unvisible) {
                compo_picturebox_set_visible(pic, false);
            } else {
                compo_picturebox_set_visible(pic, true);
            }
        } else if (compo->type == COMPO_TYPE_TEXTBOX) {
            compo_textbox_t *txt = (compo_textbox_t *)compo;
            compo_textbox_set_location(txt, x, y, w, h);
            if (unvisible) {
                compo_textbox_set_visible(txt, false);
            } else {
                compo_textbox_set_visible(txt, true);
            }
        } else if (compo->type == COMPO_TYPE_DATETIME) {
            compo_datetime_t *dt = (compo_datetime_t *)compo;
            compo_datetime_set_pos(dt, x, y);
        }

        compo = compo_get_next(compo);          //遍历组件
    }
}

//创建上拉卡片菜单（文件内调用）
static compo_form_t *func_clock_sub_card_form_create_by_ofs(s16 ofs_y)
{
    compo_form_t *frm = compo_form_create(true);

    //创建遮罩层
    compo_shape_t *masklayer = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(masklayer, COLOR_BLACK);
    compo_shape_set_location(masklayer, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_alpha(masklayer, 140);
    //创建所有组件
    func_clock_sub_card_compo_create(frm);
    //更新组件位置和大小
    func_clock_sub_card_compo_update(ofs_y, true);

    return frm;
}

//创建上拉卡片菜单（供外部调用）
compo_form_t *func_clock_sub_card_form_create(void)
{
    return func_clock_sub_card_form_create_by_ofs(SPRING_Y_MAX);
}

//获取点击的卡片组件id
static u16 func_clock_sub_card_get_btn_id(point_t pt)
{
    u16 i, id;
    u16 ret = 0;
    rect_t rect;
    compo_cardbox_t *cardbox;
    for(i=0; i<CARD_BTN_COUNT; i++) {
        id = COMPO_ID_CARD_SPORT_COMPASS + i;
        cardbox = compo_getobj_byid(id);
        rect = compo_cardbox_get_absolute(cardbox);
        if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
            ret = id;
            break;
        }
    }
    return ret;
}

//表盘上拉卡片菜单点击处理
static void func_clock_sub_card_click_handler(void)
{
    compo_cardbox_t *cardbox;
    rect_t rect;
    u8 func_jump = FUNC_NULL;
    point_t pt = ctp_get_sxy();
    u16 compo_id = func_clock_sub_card_get_btn_id(pt);
    printf("click compo_id:%d\n", compo_id);

    switch (compo_id) {
    case COMPO_ID_CARD_SPORT_COMPASS:
        cardbox = compo_getobj_byid(COMPO_ID_CARD_SPORT_COMPASS);
        rect = compo_cardbox_get_rect_absolute(cardbox, 0); //指南针
        if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
            func_jump = FUNC_COMPASS;
        } else {
            rect = compo_cardbox_get_rect_absolute(cardbox, 1); //运动
            if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
                func_jump = FUNC_SPORT;
            }
        }
        break;

    case COMPO_ID_CARD_ACTIVITY:
        func_jump = FUNC_ACTIVITY;
        break;

    case COMPO_ID_CARD_SLEEP:
        func_jump = FUNC_SLEEP;
        break;

    case COMPO_ID_CARD_HEARTRATE:
        func_jump = FUNC_HEARTRATE;
        break;

    case COMPO_ID_CARD_MUSIC:
        cardbox = compo_getobj_byid(COMPO_ID_CARD_MUSIC);
        rect = compo_cardbox_get_icon_absolute(cardbox, 0); //上一首
        if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
            printf("music_prev?\n");
            bt_control_prev();
        } else {
            rect = compo_cardbox_get_icon_absolute(cardbox, 1); //播放/暂停
            if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
                bt_control_play_pause();
                music_control_status = !music_control_status;
            } else {
                rect = compo_cardbox_get_icon_absolute(cardbox, 2); //下一首
                if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
                    printf("music_next?\n");
                    bt_control_next();
                } else {
                    func_jump = FUNC_BT;
                }
            }
        }
        break;

    case COMPO_ID_CARD_POWEROFF_ASSISTANT:
        cardbox = compo_getobj_byid(COMPO_ID_CARD_POWEROFF_ASSISTANT);
        rect = compo_cardbox_get_rect_absolute(cardbox, 0); //语音助手
        if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
            func_jump = FUNC_VOICE;
        } else {
            rect = compo_cardbox_get_rect_absolute(cardbox, 1); //关机
            if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
                func_jump = FUNC_OFF;
            }
        }
        break;

    default:
        break;
    }

    if (func_jump != FUNC_NULL) {
        func_switch_to(func_jump, func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO);  //切换动画
//        func_cb.sta = func_jump;  //直接跳转
    }
}

//根据偏移位置把最近的卡片对齐到屏幕中央
static s16 func_clock_sub_card_align_y(s16 ofs_y)
{
    u8 i, nearest_id = 0;
    u16 dc_min = GUI_SCREEN_CENTER_Y;
    u16 dc;
    if (ofs_y >= DRAG_Y_MAX) {
        return DRAG_Y_MAX;  //回弹效果
    } else if (ofs_y <= DRAG_Y_MIN) {
        return DRAG_Y_MIN;
    }
    for (i=0; i<CARD_COUNT; i++) {
        dc = abs_s(card_y_info[i] + ofs_y - GUI_SCREEN_CENTER_Y);
        if (dc < dc_min) {
            nearest_id = i;
            dc_min = dc;
        }
    }
    return (GUI_SCREEN_CENTER_Y - card_y_info[nearest_id]);
}

//根据卡片序号对齐页面偏移位置到屏幕中央
static s16 func_clock_sub_card_align_by_idx(s8 card_id)
{
    if (card_id <= CARD_ID_CLOCK) {
        return DRAG_Y_MAX;
    } else if (card_id > (CARD_COUNT - 1)){
        return DRAG_Y_MIN;
    } else {
        return (GUI_SCREEN_CENTER_Y - card_y_info[card_id]);
    }
}

//限制滑动偏移位置
static s16 func_clock_sub_card_limit_y(s16 ofs_y)
{
    ofs_y = MIN(DRAG_Y_MAX, MAX(DRAG_Y_MIN, ofs_y));
    return ofs_y;
}

//设置页面偏移位置
static void func_clock_sub_card_set_offs(s16 ofs_y)
{
    f_card_t *f_card = (f_card_t *)func_cb.f_cb;

    f_card->ofs_y = ofs_y;
    func_clock_sub_card_compo_update(f_card->ofs_y, false);
}

//时钟表盘上拉菜单数据更新----------------------------------------->todo
static void func_clock_sub_card_data_update(void)
{
    compo_cardbox_t *cardbox;
    //活动记录
    cardbox = compo_getobj_byid(COMPO_ID_CARD_ACTIVITY);
    compo_cardbox_text_set(cardbox, 0, "12000");    //步数
    compo_cardbox_text_set(cardbox, 1, "512.7");    //卡路里
    compo_cardbox_text_set(cardbox, 2, "8.0");      //距离
    //睡眠
    cardbox = compo_getobj_byid(COMPO_ID_CARD_SLEEP);
    compo_cardbox_text_set(cardbox, 0, "07:36");    //总时长
    compo_cardbox_text_set(cardbox, 1, "02h");      //深睡h
    compo_cardbox_text_set(cardbox, 2, "07m");      //深睡m
    compo_cardbox_text_set(cardbox, 3, "05h");      //浅睡h
    compo_cardbox_text_set(cardbox, 4, "29m");      //浅睡m
    //心率
    cardbox = compo_getobj_byid(COMPO_ID_CARD_HEARTRATE);
    compo_cardbox_text_set(cardbox, 0, "72");       //心率值
    //音乐
    bool cur_music_sta = bt_music_status_get();
    if (music_update_status != cur_music_sta) {
        printf("cur_music_sta = %d\n", cur_music_sta);
        music_update_status = cur_music_sta;
        music_control_status = cur_music_sta;
    }
    cardbox = compo_getobj_byid(COMPO_ID_CARD_MUSIC);
    compo_cardbox_icon_set(cardbox, 1, music_control_status ? UI_BUF_MUSIC_PAUSE_BIN : UI_BUF_MUSIC_PLAY_BIN);  //播放/暂停
}

//时钟表盘上拉菜单主要事件流程处理
static void func_clock_sub_card_process(void)
{
    f_card_t *f_card = (f_card_t *)func_cb.f_cb;

	if (f_card->flag_drag) {
        s32 dx, dy;
        f_card->flag_drag = ctp_get_dxy(&dx, &dy);
        if (f_card->flag_drag) {
            //拖动页面
            f_card->ofs_y_drag = func_clock_sub_card_limit_y(f_card->ofs_y + dy);
            func_clock_sub_card_compo_update(f_card->ofs_y_drag, false);
        } else {
            //抬手
            f_card->ofs_y = f_card->ofs_y_drag;
            s16 last_dy = ctp_get_last_dxy().y;
            s16 to_y = f_card->ofs_y + (last_dy * DRAG_AUTO_SPEED);
            f_card->moveto_y = func_clock_sub_card_align_y(to_y);
            f_card->flag_move_auto = true;
            f_card->tick = tick_get();
        }
    } else if (f_card->flag_move_auto) {
        //自动移动
        if (f_card->ofs_y == f_card->moveto_y) {
            if (f_card->ofs_y < SPRING_Y_MIN) {
                f_card->moveto_y = SPRING_Y_MIN;
            } else if (f_card->ofs_y > SPRING_Y_MAX) {
                f_card->moveto_y = SPRING_Y_MAX;
            } else {
                f_card->flag_move_auto = false;            //移动完成
                f_card->moveto_card = f_card->focus_card;
            }
        } else if (tick_check_expire(f_card->tick, FOCUS_AUTO_TICK_EXPIRE)) {
            TRACE("[%d]--move_to->[%d]\n", f_card->ofs_y, f_card->moveto_y);
            s16 dy;
            f_card->tick = tick_get();
            dy = f_card->moveto_y - f_card->ofs_y;
            if (dy > 0) {
                if (dy > FOCUS_AUTO_STEP * FOCUS_AUTO_STEP_DIV) {
                    dy = dy / FOCUS_AUTO_STEP_DIV;
                } else if (dy > FOCUS_AUTO_STEP) {
                    dy = FOCUS_AUTO_STEP;
//                } else {
//                    dy = 1;
                }
            } else if (dy < 0) {
                if (dy < -FOCUS_AUTO_STEP * FOCUS_AUTO_STEP_DIV) {
                    dy = dy / FOCUS_AUTO_STEP_DIV;
                } else if (dy < -FOCUS_AUTO_STEP) {
                    dy = -FOCUS_AUTO_STEP;
//                } else {
//                    dy = -1;
                }
            }
            func_clock_sub_card_set_offs(f_card->ofs_y + dy);
        }
    }

    func_clock_sub_card_data_update();
    func_process();
}

//下拉返回表盘
static void func_clock_sub_card_switch_to_clock(bool auto_switch)
{
    f_card_t *f_card = (f_card_t *)func_cb.f_cb;

    u16 switch_mode = FUNC_SWITCH_MENU_PULLUP_DOWN | (auto_switch ? FUNC_SWITCH_AUTO : 0);
    compo_form_destroy(func_cb.frm_main);

#if GUI_USE_SCREENSHOOT && GUI_USE_BLUR
    switch_mode |= FUNC_SWITCH_DOWN_BG_BLUR;
    compo_form_t * frm_clock = func_clock_form_create_by_screenshoot();
#else
    compo_form_t * frm_clock = func_create_form(FUNC_CLOCK);
#endif

    compo_form_t * frm = func_clock_sub_card_form_create_by_ofs(f_card->ofs_y);
    func_cb.frm_main = frm;
    if (func_switching(switch_mode, NULL)) {
        func_cb.sta = FUNC_CLOCK;
    }
    compo_form_destroy(frm_clock);
}

//时钟表盘上拉菜单功能消息处理
static void func_clock_sub_card_message(size_msg_t msg)
{
    f_card_t *f_card = (f_card_t *)func_cb.f_cb;

    if (f_card->flag_drag) {
        evt_message(msg);  //拖动中，只处理部分消息
        return;
    } else if (f_card->flag_move_auto) {
        switch (msg) {
        case MSG_CTP_TOUCH:   //自动移动过程中，触屏停止
            f_card->flag_drag = true;
            f_card->flag_move_auto = false;
            break;

        case MSG_QDEC_FORWARD:  //向前滚动菜单
            if (++f_card->moveto_card > f_card->focus_card) {
                f_card->moveto_y = func_clock_sub_card_align_by_idx(f_card->moveto_card);
            }
            break;

        case MSG_QDEC_BACKWARD:
            if (--f_card->moveto_card < f_card->focus_card) {
                f_card->moveto_y = func_clock_sub_card_align_by_idx(f_card->moveto_card);
            }
            break;

        default:
            evt_message(msg);
            break;
        }
        return;
    }

    switch (msg) {
        case MSG_CTP_CLICK:
            func_clock_sub_card_click_handler();
            break;

        case MSG_CTP_SHORT_UP:
        case MSG_CTP_SHORT_DOWN:
            if (msg == MSG_CTP_SHORT_DOWN && ctp_get_sxy().y < EXIT_Y_MAX) {   //下滑返回到时钟主界面
                func_clock_sub_card_switch_to_clock(false);
            } else {
                f_card->flag_drag = true;
                f_card->flag_move_auto = false;
            }
            break;

        case MSG_CTP_SHORT_LEFT:
        case MSG_CTP_SHORT_RIGHT:
        case MSG_CTP_LONG:
            break;

        case MSG_QDEC_FORWARD:
            f_card->flag_move_auto = true;
            if (++f_card->moveto_card > f_card->focus_card) {
                f_card->moveto_y = func_clock_sub_card_align_by_idx(f_card->moveto_card);
            }
            TRACE("focus_card[%d] to_y[%d]\n", f_card->focus_card, f_card->moveto_y);
            break;

        case MSG_QDEC_BACKWARD:
            f_card->flag_move_auto = true;
            if (--f_card->moveto_card < f_card->focus_card) {
                f_card->moveto_y = func_clock_sub_card_align_by_idx(f_card->moveto_card);
            }
            TRACE("focus_card[%d] to_y[%d]\n", f_card->focus_card, f_card->moveto_y);
            break;

        case KU_BACK:
            func_clock_sub_card_switch_to_clock(true);  //单击BACK键返回到时钟主界面
            break;

        default:
            func_message(msg);
            break;
    }
}

//时钟表盘上拉菜单进入处理
void func_clock_sub_card_enter(void)
{
    music_update_status = bt_music_status_get();
    music_control_status = music_update_status;
    printf("music_control_status = %d\n", music_control_status);
    func_cb.f_cb = func_zalloc(sizeof(f_card_t));
    func_cb.frm_main = func_clock_sub_card_form_create();

    func_clock_sub_card_set_offs(SPRING_Y_MAX);
}

//时钟表盘上拉菜单退出处理
void func_clock_sub_card_exit(void)
{
}

//时钟表盘上拉菜单
void func_clock_sub_card(void)
{
    printf("%s\n", __func__);
    func_clock_sub_card_enter();
    while (func_cb.sta == FUNC_CARD) {
        func_clock_sub_card_process();
        func_clock_sub_card_message(msg_dequeue());
    }
    func_clock_sub_card_exit();
}
#endif
