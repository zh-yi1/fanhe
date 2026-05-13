#include "include.h"
#include "func.h"

#define TRACE_EN    0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define SIDEBAR_PAGE_HEIGHT 700     //长图总高度
#define QDEC_STEP_Y         150     //编码器步进距离

#define SIDEBAR_CARD_WIDTH_ORG      430
#define SIDEBAR_CARD_HEIGHT_ORG     170
#define SIDEBAR_CARD_Y_POS          126
#define SIDEBAR_CARD_Y_GAP          10

//卡片/按钮id
enum{
    SIDEBAR_CARD_ID_WEATHER = 1,
    SIDEBAR_CARD_ID_SLEEP,
    SIDEBAR_CARD_ID_HR,
    SIDEBAR_CARD_ID_CALCULATOR,
    SIDEBAR_CARD_ID_MUSIC,
    SIDEBAR_CARD_ID_CNT,
};

typedef struct f_sidebar_t_ {
	page_tp_move_t ptm;
    s8 m_time_min;
} f_sidebar_t;


//创建右滑菜单
compo_form_t * func_clock_sub_sidebar_form_create(void)
{
    compo_cardbox_t *cardbox;
    char str_buff[16];

    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //天气
    sys_cb.temperature[0] = 20;
    sys_cb.temperature[1] = 26;
    cardbox = compo_cardbox_create(frm, 0, 2, 2, SIDEBAR_CARD_WIDTH_ORG, SIDEBAR_CARD_HEIGHT_ORG);
    compo_cardbox_set_pos(cardbox, GUI_SCREEN_CENTER_X, SIDEBAR_CARD_Y_POS);
    compo_setid(cardbox, SIDEBAR_CARD_ID_WEATHER);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_WEATHER_BG_BIN);  //天气背景
    compo_cardbox_icon_set_location(cardbox, 0, 0, 0, SIDEBAR_CARD_WIDTH_ORG, SIDEBAR_CARD_HEIGHT_ORG);
    compo_cardbox_icon_set(cardbox, 1, UI_BUF_WEATHER_WEATHER_LIST_BIN);  //天气图标
    compo_cardbox_icon_set_location(cardbox, 1, -SIDEBAR_CARD_WIDTH_ORG/4, 0, 130, 130);
    compo_cardbox_icon_cut(cardbox, 1, sys_cb.weather_idx, WEATHER_CNT);
    compo_cardbox_text_set(cardbox, 0, i18n[STR_CLOUDY+sys_cb.weather_idx]);
    compo_cardbox_text_set_location(cardbox, 0, 90, 0-30, 200, 50);
    snprintf(str_buff, sizeof(str_buff), "%02d~%02d", sys_cb.temperature[0], sys_cb.temperature[1]);    //温度
    compo_cardbox_text_set(cardbox, 1, str_buff);
    compo_cardbox_text_set_location(cardbox, 1, 90, 0+30, 200, 50);
    //睡眠
    cardbox = compo_cardbox_create(frm, 0, 1, 2, SIDEBAR_CARD_WIDTH_ORG, SIDEBAR_CARD_HEIGHT_ORG);
    compo_cardbox_set_pos(cardbox, GUI_SCREEN_CENTER_X, SIDEBAR_CARD_Y_POS + (SIDEBAR_CARD_HEIGHT_ORG + SIDEBAR_CARD_Y_GAP)* 1);
    compo_setid(cardbox, SIDEBAR_CARD_ID_SLEEP);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_SLEEP_BIN);  //睡眠背景
    compo_cardbox_icon_set_location(cardbox, 0, 0, 0, SIDEBAR_CARD_WIDTH_ORG, SIDEBAR_CARD_HEIGHT_ORG);
    compo_cardbox_text_set(cardbox, 0, i18n[STR_SLEEP]);
    compo_cardbox_text_set_location(cardbox, 0, 90, 0-30, 200, 50);
    snprintf(str_buff, sizeof(str_buff), "%02dh%02dm", 8, 30);    //睡眠时间
    compo_cardbox_text_set(cardbox, 1, str_buff);
    compo_cardbox_text_set_location(cardbox, 1, 90, 0+30, 200, 50);

    //心率
    cardbox = compo_cardbox_create(frm, 0, 1, 2, SIDEBAR_CARD_WIDTH_ORG, SIDEBAR_CARD_HEIGHT_ORG);
    compo_cardbox_set_pos(cardbox, GUI_SCREEN_CENTER_X, SIDEBAR_CARD_Y_POS + (SIDEBAR_CARD_HEIGHT_ORG + SIDEBAR_CARD_Y_GAP)* 2);
    compo_setid(cardbox, SIDEBAR_CARD_ID_HR);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_HR_BIN);  //心率背景
    compo_cardbox_icon_set_location(cardbox, 0, 0, 0, SIDEBAR_CARD_WIDTH_ORG, SIDEBAR_CARD_HEIGHT_ORG);
    compo_cardbox_text_set_forecolor(cardbox, 0, COLOR_DARKGRAY);
    compo_cardbox_text_set(cardbox, 0, i18n[STR_HEART_RATE]);
    compo_cardbox_text_set_location(cardbox, 0, 90, 0-30, 200, 50);
    snprintf(str_buff, sizeof(str_buff), "%02dbpm", 85);    //心率
    compo_cardbox_text_set_forecolor(cardbox, 1, COLOR_DARKGRAY);
    compo_cardbox_text_set(cardbox, 1, str_buff);
    compo_cardbox_text_set_location(cardbox, 1, 90, 0+30, 200, 50);
    //计算器
    cardbox = compo_cardbox_create(frm, 0, 1, 0, 190, 180);
    compo_cardbox_set_pos(cardbox, 125, SIDEBAR_CARD_Y_POS + (SIDEBAR_CARD_HEIGHT_ORG + SIDEBAR_CARD_Y_GAP)* 3);
    compo_setid(cardbox, SIDEBAR_CARD_ID_CALCULATOR);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_CALCULATOR_BIN);  //图标
    compo_cardbox_icon_set_pos(cardbox, 0, 0, 0);
    //音乐
    cardbox = compo_cardbox_create(frm, 0, 1, 0, 190, 180);
    compo_cardbox_set_pos(cardbox, 340, SIDEBAR_CARD_Y_POS + (SIDEBAR_CARD_HEIGHT_ORG + SIDEBAR_CARD_Y_GAP)* 3);
    compo_setid(cardbox, SIDEBAR_CARD_ID_MUSIC);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_MUSIC_BIN);  //图标
    compo_cardbox_icon_set_pos(cardbox, 0, 0, 0);

    return frm;
}

//刷新右滑组件Widget（时间、温度、图标等）
void func_clock_sub_sidebar_update(void)
{
    f_sidebar_t *f_sidebar = (f_sidebar_t *)func_cb.f_cb;
    compo_cardbox_t *cardbox;
    char str_buff[16];
    //天气图标，温度
    if (f_sidebar->m_time_min != compo_cb.tm.min) {             //一分钟更新一次
        cardbox = compo_getobj_byid(SIDEBAR_CARD_ID_WEATHER);
        compo_cardbox_icon_cut(cardbox, 1, sys_cb.weather_idx, WEATHER_CNT);        //天气
        snprintf(str_buff, sizeof(str_buff), "%02d~%02d", sys_cb.temperature[0], sys_cb.temperature[1]);    //温度
        compo_cardbox_text_set(cardbox, 1, str_buff);

        f_sidebar->m_time_min = compo_cb.tm.min;
    }
}

//时钟表盘右滑菜单点击处理
static void func_clock_sub_sidebar_click_handler(void)
{
    u8 func_jump = FUNC_NULL;
    point_t pt = ctp_get_sxy();
    u16 id = 0;
    for(u8 i = SIDEBAR_CARD_ID_WEATHER; i < SIDEBAR_CARD_ID_CNT; i++) {
        if (compo_cardbox_btn_is(compo_getobj_byid(i), pt)) {
            id = i;
        }
    }
    TRACE("click id:%d\n", id);

    switch (id) {
    case SIDEBAR_CARD_ID_WEATHER:
        func_jump = FUNC_WEATHER;
        break;

    case SIDEBAR_CARD_ID_SLEEP:
        func_jump = FUNC_SLEEP;
        break;

    case SIDEBAR_CARD_ID_HR:
        func_jump = FUNC_HEARTRATE;
        break;

    case SIDEBAR_CARD_ID_CALCULATOR:
        if (pt.x <= GUI_SCREEN_CENTER_X) {
            func_jump = FUNC_CALCULATOR;
        }
        break;

    case SIDEBAR_CARD_ID_MUSIC:
        if (pt.x > GUI_SCREEN_CENTER_X) {
            func_jump = FUNC_BT;
        }
        break;

    default:
        break;
    }

    if (func_jump != FUNC_NULL) {
        func_switch_to(func_jump, func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO);  //切换动画
//        func_cb.sta = btn_info.func_jump;  //直接跳转
    }
}

//时钟表盘右滑菜单主要事件流程处理
static void func_clock_sub_sidebar_process(void)
{
    f_sidebar_t *f_sidebar = (f_sidebar_t *)func_cb.f_cb;

    compo_page_move_process(&f_sidebar->ptm);
    func_clock_sub_sidebar_update();
    func_process();
}

//时钟表盘右滑菜单功能消息处理
static void func_clock_sub_sidebar_message(size_msg_t msg)
{
    f_sidebar_t *f_sidebar = (f_sidebar_t *)func_cb.f_cb;

    switch (msg) {
    case MSG_CTP_TOUCH:
        compo_page_move_touch_handler(&f_sidebar->ptm);
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_LONG:
        break;

    case MSG_CTP_SHORT_LEFT:
        func_switch_to(FUNC_CLOCK, func_get_switching_mode_byidx(sys_cb.nav_index, true));
        break;

    case MSG_CTP_CLICK:
        func_clock_sub_sidebar_click_handler();
        break;

    case MSG_QDEC_FORWARD:
        compo_page_move_set(&f_sidebar->ptm, -QDEC_STEP_Y);
        break;

    case MSG_QDEC_BACKWARD:
        compo_page_move_set(&f_sidebar->ptm, QDEC_STEP_Y);
        break;

    case KU_BACK:
        func_switch_to(FUNC_CLOCK, func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO);
        break;

    default:
        func_message(msg);
        break;
    }
}

//时钟表盘右滑菜单进入处理
void func_clock_sub_sidebar_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_sidebar_t));
    func_cb.frm_main = func_clock_sub_sidebar_form_create();

    f_sidebar_t *f_sidebar = (f_sidebar_t *)func_cb.f_cb;
    page_move_info_t info = {
        .page_size = SIDEBAR_PAGE_HEIGHT,
        .page_count = 1,
        .quick_jump_perc = 50,
        .up_over_perc = 10,
        .down_over_perc = 10,
        .down_spring_perc = 15,
    };
    compo_page_move_init(&f_sidebar->ptm, func_cb.frm_main->page_body, &info);
}

//时钟表盘右滑菜单退出处理
void func_clock_sub_sidebar_exit(void)
{

}

//时钟表盘右滑菜单
void func_clock_sub_sidebar(void)
{
    printf("%s\n", __func__);
    func_clock_sub_sidebar_enter();
    while (func_cb.sta == FUNC_SIDEBAR) {
        func_clock_sub_sidebar_process();
        func_clock_sub_sidebar_message(msg_dequeue());
    }
    func_clock_sub_sidebar_exit();
}
