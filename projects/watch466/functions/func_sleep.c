#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


#define FUNC_SLEEP_SIMULATE_DATA_EN     1                           //假数据
#define TODAY_SLEEP_TIME_TXT_MAX        2//16

enum
{
    COMPO_ID_TEXT_DEEP_TIME_HOUR = 1,
    COMPO_ID_TEXT_DEEP_TIME_MIN,
    COMPO_ID_TEXT_LIGHT_TIME_HOUR,
    COMPO_ID_TEXT_LIGHT_TIME_MIN,

    COMPO_ID_CHART,
};

enum
{
    CHART_ANIM_STA_IDLE,
    CHART_ANIM_STA_START,
    CHART_ANIM_STA_END,

};

typedef struct f_sleep_t_ {
	page_tp_move_t ptm;

	u8 anim_sta;
	u16 data[7];
} f_sleep_t;


#if FUNC_SLEEP_SIMULATE_DATA_EN
static const uint16_t total_sleep_time[] = {8*60+30, 9*60+40, 7*60+20, 10*60+30, 5*60+10, 0, 8*60+30}; //min
#endif

//创建睡眠窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_sleep_form_create(void)
{
    component_t *compo;
    char buf[16];

    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);
	compo_form_add_image(frm, UI_BUF_SLEEP_SLEEP_BIN, GUI_SCREEN_CENTER_X, 173);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_SLEEP]);

    //今日深度睡眠时长
    compo_form_add_image(frm, UI_BUF_SLEEP_DEEP_ALEEP_BIN, 131, 282);
    compo = (component_t *)compo_textbox_create(frm, TODAY_SLEEP_TIME_TXT_MAX);
    compo_textbox_set_font((compo_textbox_t *)compo, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_setid(compo, COMPO_ID_TEXT_DEEP_TIME_HOUR);
    compo_textbox_set_pos((compo_textbox_t *)compo, 64, 344);
    snprintf(buf, sizeof(buf), "%d", total_sleep_time[0] / 60);
    compo_textbox_set((compo_textbox_t *)compo, buf);

    compo = (component_t *)compo_textbox_create(frm, TODAY_SLEEP_TIME_TXT_MAX-1);
    compo_textbox_set_pos((compo_textbox_t *)compo, 64+40, 344);
    snprintf(buf, sizeof(buf), "%s", i18n[STR_HOUR]);
    compo_textbox_set((compo_textbox_t *)compo, buf);

    compo = (component_t *)compo_textbox_create(frm, TODAY_SLEEP_TIME_TXT_MAX);
    compo_textbox_set_font((compo_textbox_t *)compo, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_setid(compo, COMPO_ID_TEXT_DEEP_TIME_MIN);
    compo_textbox_set_pos((compo_textbox_t *)compo, 64+80, 344);
    snprintf(buf, sizeof(buf), "%d", total_sleep_time[0] % 60);
    compo_textbox_set((compo_textbox_t *)compo, buf);

    compo = (component_t *)compo_textbox_create(frm, TODAY_SLEEP_TIME_TXT_MAX-1);
    compo_textbox_set_pos((compo_textbox_t *)compo, 64+120, 344);
    snprintf(buf, sizeof(buf), "%s", i18n[STR_MIN]);
    compo_textbox_set((compo_textbox_t *)compo, buf);

    //今日浅睡睡眠时长
    compo_form_add_image(frm, UI_BUF_SLEEP_LIGHT_SLEEP_BIN, 335, 282);
    compo = (component_t *)compo_textbox_create(frm, TODAY_SLEEP_TIME_TXT_MAX);
    compo_textbox_set_font((compo_textbox_t *)compo, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_setid(compo, COMPO_ID_TEXT_LIGHT_TIME_HOUR);
    compo_textbox_set_pos((compo_textbox_t *)compo, 267, 344);
    snprintf(buf, sizeof(buf), "%d", total_sleep_time[1] / 60);
    compo_textbox_set((compo_textbox_t *)compo, buf);

    compo = (component_t *)compo_textbox_create(frm, TODAY_SLEEP_TIME_TXT_MAX-1);
    compo_textbox_set_pos((compo_textbox_t *)compo, 267+40, 344);
    snprintf(buf, sizeof(buf), "%s", i18n[STR_HOUR]);
    compo_textbox_set((compo_textbox_t *)compo, buf);

    compo = (component_t *)compo_textbox_create(frm, TODAY_SLEEP_TIME_TXT_MAX);
    compo_textbox_set_font((compo_textbox_t *)compo, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_setid(compo, COMPO_ID_TEXT_LIGHT_TIME_MIN);
    compo_textbox_set_pos((compo_textbox_t *)compo, 267+80, 344);
    snprintf(buf, sizeof(buf), "%d", total_sleep_time[1] % 60);
    compo_textbox_set((compo_textbox_t *)compo, buf);

    compo = (component_t *)compo_textbox_create(frm, TODAY_SLEEP_TIME_TXT_MAX-1);
    compo_textbox_set_pos((compo_textbox_t *)compo, 267+120, 344);
    snprintf(buf, sizeof(buf), "%s", i18n[STR_MIN]);
    compo_textbox_set((compo_textbox_t *)compo, buf);

    //睡眠日总时长详情
    compo_form_add_image(frm, UI_BUF_SLEEP_CHART_BG_BIN, GUI_SCREEN_CENTER_X, 650);
#if (!FUNC_SLEEP_SIMULATE_DATA_EN)
    uint16_t total_sleep_time[2] = {0};
#endif

    //创建图表
    compo_chartbox_t *chart = compo_chartbox_create(frm, CHART_TYPE_BAR, 7);
    compo_chartbox_set_location(chart, 180 +72, 646, 230, 200);
    compo_chartbox_set_pixel(chart, 2);
    compo_setid(chart, COMPO_ID_CHART);

    return frm;
}

//刷新睡眠数据
static void func_sleep_data_refresh(void)
{
#if (!FUNC_SLEEP_SIMULATE_DATA_EN)
    uint16_t total_sleep_time[2] = {0};
#endif

    f_sleep_t *f_sleep = (f_sleep_t *)func_cb.f_cb;
    for (int i=0; i<7; i++) {
        f_sleep->data[i] = total_sleep_time[i] * 20 * 6 / (12 * 60) / 2;
    }

    //更新图表数据
//    compo_chartbox_t *chart = compo_getobj_byid(COMPO_ID_CHART);
//    u8 max_num = compo_chartbox_get_max_num(chart);
//
//    chart_t chart_info;
//    chart_info.y = 0;
//    chart_info.width = 5;   //5 * pixel(2) = 10像素点
//    for (int i=0; i<max_num; i++) {
//        chart_info.x = i*17 + 1;
//        chart_info.height = total_sleep_time[i] * 20 * 6 / (12 * 60) / 2;
//        compo_chartbox_set_value(chart, i, chart_info, COLOR_BLUE);
//    }

}

//睡眠图表动画
static void func_sleep_animation(void)
{
    f_sleep_t *f_sleep = (f_sleep_t *)func_cb.f_cb;
    if (f_sleep->anim_sta == CHART_ANIM_STA_END) {
        return;
    }
    compo_chartbox_t *chart = compo_getobj_byid(COMPO_ID_CHART);
    u8 max_num = compo_chartbox_get_max_num(chart);

    chart_t chart_info;
    chart_info.y = 0;
    chart_info.width = 5;   //5 * pixel(2) = 10像素点

    static u8 anim_cnt;
    static u32 ticks;
    if(tick_check_expire(ticks, 10)) {
        ticks = tick_get();

        switch (f_sleep->anim_sta) {
            case CHART_ANIM_STA_IDLE:
//                printf("sleep_get_offs:%d\n", compo_page_move_get_offset(&f_sleep->ptm));
                if (compo_page_move_get_offset(&f_sleep->ptm) < -300) {
                    anim_cnt = 0;
                    f_sleep->anim_sta = CHART_ANIM_STA_START;
                }
                break;

            case CHART_ANIM_STA_START:
                for (int i=0; i<max_num; i++) {
                    if (i == 0 && ++anim_cnt >= 255) {          //图表数据最大时退出动画
                        f_sleep->anim_sta = CHART_ANIM_STA_END;
                    }
                    if (anim_cnt > f_sleep->data[i]) {
                        continue;
                    }
                    chart_info.x = i*17 + 1;
                    chart_info.height = anim_cnt;
                    compo_chartbox_set_value(chart, i, chart_info, COLOR_BLUE);
                }
                break;
        }
    }
}

//睡眠功能消息处理
static void func_sleep_process(void)
{
    f_sleep_t *f_sleep = (f_sleep_t *)func_cb.f_cb;

    compo_page_move_process(&f_sleep->ptm);
    func_sleep_animation();
    func_process();
}

static void func_sleep_message(size_msg_t msg)
{
	f_sleep_t *f_sleep = (f_sleep_t *)func_cb.f_cb;

    switch (msg) {
    case MSG_CTP_TOUCH:
        compo_page_move_touch_handler(&f_sleep->ptm);
        break;

    case MSG_CTP_CLICK:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_LONG:
        break;

	case MSG_QDEC_BACKWARD:
        if (func_cb.flag_sort) {    //快捷应用状态下不滚动页面
            func_message(msg);
        } else {
            compo_page_move_set_by_pages(&f_sleep->ptm, -1);
        }
		break;

	case MSG_QDEC_FORWARD:
        if (func_cb.flag_sort) {
            func_message(msg);
        } else {
            compo_page_move_set_by_pages(&f_sleep->ptm, 1);
        }
		break;

    default:
        func_message(msg);
        break;
    }
}

//进入睡眠功能
void func_sleep_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_sleep_t));
    func_cb.frm_main = func_sleep_form_create();

    f_sleep_t *f_sleep = (f_sleep_t *)func_cb.f_cb;
    page_move_info_t info = {
        .title_used = true,
        .page_size = GUI_SCREEN_HEIGHT - (GUI_SCREEN_HEIGHT / 8),
        .page_count = 2,
        .jump_perc = 20,
        .quick_jump_perc = 100,
        .up_over_perc = 10,
        .down_over_perc = 10,
    };
    compo_page_move_init(&f_sleep->ptm, func_cb.frm_main->page_body, &info);

    func_sleep_data_refresh();
}

//退出睡眠功能
void func_sleep_exit(void)
{
    memset(func_cb.f_cb, 0, sizeof(f_sleep_t));
}

//睡眠功能
void func_sleep(void)
{
    printf("%s\n", __func__);
    func_sleep_enter();
    while (func_cb.sta == FUNC_SLEEP) {
        func_sleep_process();
        func_sleep_message(msg_dequeue());
    }
    func_sleep_exit();
}
