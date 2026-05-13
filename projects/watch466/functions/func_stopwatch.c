#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define NUM_REC_COLOR               make_color(2, 132, 253)
#define NUM_REC_COLOR_PRESS         make_color(1, 60, 117)

//组件ID
enum {
    //按键
    COMPO_ID_BTN_RECORD_VIEW = 1,       //查看记录
    COMPO_ID_BTN_RECORD,                //记录当前时间
    COMPO_ID_BTN_AFRESH,                //重新开始
    COMPO_ID_BTN_START_REC,             //开始计时
	//数字文本
	COMPO_ID_NUM_STOPWATCH_TIME,        //当前计时
	COMPO_ID_NUM_STOPWATCH_REC,         //记录数
};

typedef struct f_stopwatch_t_ {
    u8 min;                 //分
    u8 sec;                 //秒
    u16 msec;               //毫秒
} f_stopwatch_t;

typedef struct stopwatch_num_item_t_ {
    u32 res_addr;
    int num_cnt;
    u16 num_id;
    int val;
    s16 x;
    s16 y;
    bool zfill_en;
    bool visible_en;
} stopwatch_num_item_t;

//创建秒表窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_stopwatch_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_STOP_WATCH]);

	//新建按钮
    u32 res_addr;
    compo_button_t *btn;
    res_addr = sys_cb.stopwatch_sta ? UI_BUF_COMMON_PAUSE_BIN : UI_BUF_COMMON_START_BIN;    //开始/暂停
    btn = compo_button_create_by_image(frm, res_addr);
    compo_setid(btn, COMPO_ID_BTN_START_REC);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 404);
    btn = compo_button_create_by_image(frm, UI_BUF_STOPWATCH_AFRESH_BIN);    //复位
    compo_setid(btn, COMPO_ID_BTN_AFRESH);
    compo_button_set_pos(btn, 108, 363);
    compo_button_set_visible(btn, sys_cb.stopwatch_total_msec > 0);
    btn = compo_button_create_by_image(frm, UI_BUF_STOPWATCH_RECORD_BIN);    //计次
    compo_setid(btn, COMPO_ID_BTN_RECORD);
    compo_button_set_pos(btn, 358, 363);
    compo_button_set_visible(btn, sys_cb.stopwatch_sta != 0);
    btn = compo_button_create_by_image(frm, UI_BUF_STOPWATCH_RECORD1_BIN);    //计次详情
    compo_setid(btn, COMPO_ID_BTN_RECORD_VIEW);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 135);
    compo_button_set_visible(btn, sys_cb.stopwatch_rec_cnt > 0);

	//创建数字文本
	compo_textbox_t *txt_num;
	char str_buff[9];
    u8 min, sec;
    u16 msec;
    min = ((sys_cb.stopwatch_total_msec / 1000) % 3600) / 60;
    sec = (sys_cb.stopwatch_total_msec / 1000) % 60;
    msec = sys_cb.stopwatch_total_msec % 1000;
	txt_num = compo_textbox_create(frm, 8);     //当前计时
	compo_setid(txt_num, COMPO_ID_NUM_STOPWATCH_TIME);
	compo_textbox_set_location(txt_num, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 300, 60);
	compo_textbox_set_font(txt_num, UI_BUF_0FONT_FONT_NUM_46_BIN);
	snprintf(str_buff, sizeof(str_buff), "%02d:%02d.%02d", min, sec, msec / 10);
	compo_textbox_set(txt_num, str_buff);
	txt_num = compo_textbox_create(frm, 2);     //记录数
	compo_setid(txt_num, COMPO_ID_NUM_STOPWATCH_REC);
	compo_textbox_set_location(txt_num, 260, 135, 50, 60);
	compo_textbox_set_font(txt_num, UI_BUF_0FONT_FONT_NUM_24_BIN);
	snprintf(str_buff, sizeof(str_buff), "%02d", sys_cb.stopwatch_rec_cnt);
	compo_textbox_set(txt_num, str_buff);
	compo_textbox_set_forecolor(txt_num, NUM_REC_COLOR);
	compo_textbox_set_visible(txt_num, sys_cb.stopwatch_rec_cnt > 0);

    return frm;
}

//触摸按钮效果处理
static void func_stopwatch_button_touch_handle(void)
{
    u32 res_addr;
    int id = compo_get_button_id();

    //获取按钮组件的地址
    compo_button_t *btn_start = compo_getobj_byid(COMPO_ID_BTN_START_REC);
    compo_button_t *btn_afresh = compo_getobj_byid(COMPO_ID_BTN_AFRESH);
    compo_button_t *btn_record = compo_getobj_byid(COMPO_ID_BTN_RECORD);
    compo_button_t *btn_record1 = compo_getobj_byid(COMPO_ID_BTN_RECORD_VIEW);

    //获取数字组件的地址
    compo_textbox_t *num_rec = compo_getobj_byid(COMPO_ID_NUM_STOPWATCH_REC);
    switch (id) {
    case COMPO_ID_BTN_RECORD_VIEW:
        if (sys_cb.stopwatch_rec_cnt) {
            compo_button_set_alpha(btn_record1, UI_BTN_CLICK_EFFECT_ALPHA);
            compo_textbox_set_forecolor(num_rec, NUM_REC_COLOR_PRESS);
        }
        break;

    case COMPO_ID_BTN_RECORD:

        compo_button_set_alpha(btn_record, UI_BTN_CLICK_EFFECT_ALPHA);
        if (sys_cb.stopwatch_rec_cnt && sys_cb.stopwatch_sta) {
            compo_textbox_set_forecolor(num_rec, NUM_REC_COLOR_PRESS);
        }
        break;

    case COMPO_ID_BTN_AFRESH:

        compo_button_set_alpha(btn_afresh, UI_BTN_CLICK_EFFECT_ALPHA);
        break;

    case COMPO_ID_BTN_START_REC:
        res_addr = sys_cb.stopwatch_sta ? UI_BUF_COMMON_PAUSE_BIN : UI_BUF_COMMON_START_BIN;
        compo_button_set_bgimg(btn_start, res_addr);
        compo_button_set_alpha(btn_start, UI_BTN_CLICK_EFFECT_ALPHA);
        break;

    default:
        break;
    }
}

//释放按钮效果处理
static void func_stopwatch_button_release_handle(void)
{
    //获取按钮组件的地址
    compo_button_t *btn_start = compo_getobj_byid(COMPO_ID_BTN_START_REC);
    compo_button_t *btn_afresh = compo_getobj_byid(COMPO_ID_BTN_AFRESH);
    compo_button_t *btn_record = compo_getobj_byid(COMPO_ID_BTN_RECORD);
    compo_button_t *btn_record1 = compo_getobj_byid(COMPO_ID_BTN_RECORD_VIEW);

    u32 res_addr = sys_cb.stopwatch_sta ? UI_BUF_COMMON_PAUSE_BIN : UI_BUF_COMMON_START_BIN;
    compo_button_set_bgimg(btn_start, res_addr);
    compo_button_set_alpha(btn_start, UI_BTN_NORMAL_EFFECT_ALPHA);

    compo_button_set_bgimg(btn_afresh, UI_BUF_STOPWATCH_AFRESH_BIN);  //复位
    compo_button_set_alpha(btn_afresh, UI_BTN_NORMAL_EFFECT_ALPHA);
    compo_button_set_visible(btn_afresh, sys_cb.stopwatch_total_msec > 0 || sys_cb.stopwatch_sta);
    compo_button_set_bgimg(btn_record, UI_BUF_STOPWATCH_RECORD_BIN);  //计次
    compo_button_set_visible(btn_record, sys_cb.stopwatch_sta != 0);
    compo_button_set_alpha(btn_record, UI_BTN_NORMAL_EFFECT_ALPHA);
    compo_button_set_bgimg(btn_record1, UI_BUF_STOPWATCH_RECORD1_BIN);    //计次详情
    compo_button_set_visible(btn_record1, sys_cb.stopwatch_rec_cnt > 0);
    compo_button_set_alpha(btn_record1, UI_BTN_NORMAL_EFFECT_ALPHA);
    //获取数字组件的地址
    compo_textbox_t *num_rec = compo_getobj_byid(COMPO_ID_NUM_STOPWATCH_REC);

    compo_textbox_set_forecolor(num_rec, NUM_REC_COLOR);
    compo_textbox_set_visible(num_rec, sys_cb.stopwatch_rec_cnt > 0);
}

//50ms秒表毫秒刷新回调函数（10ms误差大）
static void stopwatch_50ms_pro(co_timer_t *timer, void *param)
{
    static u32 real_total_msec;
    if (sys_cb.stopwatch_sta && !sys_cb.gui_sleep_sta) {    //休眠不计时
        if (sys_cb.stopwatch_total_msec == 0) {
            real_total_msec = 0;
        }
        real_total_msec += 50;
        sys_cb.stopwatch_total_msec = real_total_msec + get_random(50);    //防止百分秒永远是5的倍数
    }
}

//单击按钮
static void func_stopwatch_button_click(void)
{
    static co_timer_t stopwatch_timer;
    int id = compo_get_button_id();
    f_stopwatch_t *f_stopwatch = (f_stopwatch_t *)func_cb.f_cb;
    char str_buff[9];

    //获取数字组件的地址
    compo_textbox_t *num_time = compo_getobj_byid(COMPO_ID_NUM_STOPWATCH_TIME);
    compo_textbox_t *num_rec = compo_getobj_byid(COMPO_ID_NUM_STOPWATCH_REC);

    switch (id) {
    case COMPO_ID_BTN_RECORD_VIEW:
        if (sys_cb.stopwatch_rec_cnt) {
            func_cb.sta = FUNC_STOPWATCH_SUB_RECORD;
        }
        break;

    case COMPO_ID_BTN_RECORD:
        //插入记录到头部
        if (sys_cb.stopwatch_sta) {
            sys_cb.stopwatch_rec_cnt += (sys_cb.stopwatch_rec_cnt < STOPWATCH_REC_NUM_MAX);
            for (u8 i = sys_cb.stopwatch_rec_cnt - 1; i > 0; i--)  {
                sys_cb.stopwatch_rec_view[i] = sys_cb.stopwatch_rec_view[i - 1];
            }
            sys_cb.stopwatch_rec_view[0] = sys_cb.stopwatch_total_msec;

            snprintf(str_buff, sizeof(str_buff), "%02d", sys_cb.stopwatch_rec_cnt);
            compo_textbox_set(num_rec, str_buff);
        }
        break;

    case COMPO_ID_BTN_AFRESH:
        if (sys_cb.stopwatch_total_msec > 0) {
            sys_cb.stopwatch_sta = 0;
            co_timer_del(&stopwatch_timer);

            f_stopwatch->min = 0;
            f_stopwatch->sec = 0;
            f_stopwatch->msec = 0;

            memset(sys_cb.stopwatch_rec_view, 0, sizeof(sys_cb.stopwatch_rec_view));
            sys_cb.stopwatch_rec_cnt = 0;
            sys_cb.stopwatch_rec_cnt = 0;
            sys_cb.stopwatch_total_msec = 0;

            compo_textbox_set(num_time, "00:00.00");
            compo_textbox_set(num_rec, "0");
        }
        break;

    case COMPO_ID_BTN_START_REC:
        sys_cb.stopwatch_sta = !sys_cb.stopwatch_sta;
        if (sys_cb.stopwatch_sta && sys_cb.stopwatch_total_msec == 0) {
            co_timer_set(&stopwatch_timer, 50, TIMER_REPEAT, LEVEL_LOW_PRI, stopwatch_50ms_pro, NULL);
        }
        break;
    }

    func_stopwatch_button_release_handle();
}

//秒表功能事件处理
static void func_stopwatch_process(void)
{
    if (sys_cb.stopwatch_sta) {
        u8 min = ((sys_cb.stopwatch_total_msec / 1000) % 3600) / 60;
        u8 sec = (sys_cb.stopwatch_total_msec / 1000) % 60;
        u16 msec = sys_cb.stopwatch_total_msec % 1000;
        char str_buff[9];
        f_stopwatch_t *f_stopwatch = (f_stopwatch_t *)func_cb.f_cb;

        //获取数字组件的地址
        compo_textbox_t *num_time = compo_getobj_byid(COMPO_ID_NUM_STOPWATCH_TIME);

        if (f_stopwatch->msec != msec || f_stopwatch->sec != sec || f_stopwatch->min != min) {
            f_stopwatch->min = min;
            f_stopwatch->sec = sec;
            f_stopwatch->msec = msec;

            snprintf(str_buff, sizeof(str_buff), "%02d:%02d.%02d", min, sec, msec / 10);
            compo_textbox_set(num_time, str_buff);
        }

    }

    func_process();
}

//秒表功能消息处理
static void func_stopwatch_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_TOUCH:
        func_stopwatch_button_touch_handle();
        break;

    case MSG_CTP_CLICK:
        func_stopwatch_button_click();
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_LONG:
        func_stopwatch_button_release_handle();
        if (func_cb.flag_sort) {
            func_message(msg);
        }
        break;

    case MSG_CTP_SHORT_RIGHT:
        func_stopwatch_button_release_handle();
        func_message(msg);
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入秒表功能
void func_stopwatch_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_stopwatch_t));
    func_cb.frm_main = func_stopwatch_form_create();
}

//退出秒表功能
void func_stopwatch_exit(void)
{
}

//秒表功能
void func_stopwatch(void)
{
    printf("%s\n", __func__);
    func_stopwatch_enter();
    while (func_cb.sta == FUNC_STOPWATCH) {
        func_stopwatch_process();
        func_stopwatch_message(msg_dequeue());
    }
    func_stopwatch_exit();
}
