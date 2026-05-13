#include "include.h"
#include "func.h"

#define TRACE_EN    1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define SEC_TO_HOUR(s)  (s / 3600)          //总秒数转换为时分秒（时）
#define SEC_TO_MIN(s)   ((s % 3600) / 60)   //总秒数转换为时分秒（分）
#define SEC_TO_SEC(s)   (s % 60)            //总秒数转换为时分秒（秒）

//显示页面
enum {
    TIMER_PAGE_SELECT,
    TIMER_PAGE_CUSTOM,
    TIMER_PAGE_COUNTDOWN,
};

//计时状态
enum {
    TIMER_STA_IDLE,
    TIMER_STA_WORKING,
    TIMER_STA_PAUSE,
    TIMER_STA_DONE,
    TIMER_STA_RESET,
};

//组件ID
enum {
    ///PAGE_SELECT
    //按键
	COMPO_ID_BTN_1MIN   = 1,
	COMPO_ID_BTN_2MIN   = 2,
	COMPO_ID_BTN_3MIN   = 3,
	COMPO_ID_BTN_5MIN   = 5,
	COMPO_ID_BTN_10MIN  = 10,
	COMPO_ID_BTN_30MIN  = 30,
	COMPO_ID_BTN_CUSTOM,

    ///PAGE_CUSTOM
    //按键
	COMPO_ID_BTN_HOUR_INC,      //时+
	COMPO_ID_BTN_MIN_INC,       //分+
	COMPO_ID_BTN_SEC_INC,       //秒+
	COMPO_ID_BTN_HOUR_RED,      //时-
	COMPO_ID_BTN_MIN_RED,       //分-
	COMPO_ID_BTN_SEC_RED,       //秒-
	COMPO_ID_BTN_OK,            //ok
    //数字
	COMPO_ID_NUM_CUSTOM,

    ///PAGE_COUNTDOWN
    //按键
	COMPO_ID_BTN_START,
	COMPO_ID_BTN_NO,
	//数字
	COMPO_ID_NUM_COUNTDOWN,
};

typedef struct f_timer_t_ {
    u8 page_disp;
    s8 hour;
    s8 min;
    s8 sec;
} f_timer_t;


//item列表，创建时遍历一下
typedef struct timer_btn_item_t_ {
    u32 res_addr;
    u16 btn_id;
    s16 x;
    s16 y;
    bool visible_en;
} timer_btn_item_t;

typedef struct timer_txt_item_t_ {
    const char *text;
    u16 max_word_cnt;
    s16 x;
    s16 y;
    bool visible_en;
} timer_txt_item_t;

typedef struct timer_pic_item_t_ {
    u32 res_addr;
    u16 pic_id;
    s16 x;
    s16 y;
    bool visible_en;
} timer_pic_item_t;

//PAGE_SELECT
#define TIMER_BTN_ITEM_CNT  ((int)(sizeof(tbl_timer_btn_item) / sizeof(tbl_timer_btn_item[0])))
static const timer_btn_item_t tbl_timer_btn_item[] = {
    {UI_BUF_TIMER_BG_BIN,           COMPO_ID_BTN_1MIN,      64,     128,    true},
    {UI_BUF_TIMER_BG_BIN,           COMPO_ID_BTN_2MIN,      160,    128,    true},
    {UI_BUF_TIMER_BG_BIN,           COMPO_ID_BTN_3MIN,      256,    128,    true},
    {UI_BUF_TIMER_BG_BIN,           COMPO_ID_BTN_5MIN,      64,     228,    true},
    {UI_BUF_TIMER_BG_BIN,           COMPO_ID_BTN_10MIN,     160,    228,    true},
    {UI_BUF_TIMER_BG_BIN,           COMPO_ID_BTN_30MIN,     256,    228,    true},
    {UI_BUF_COMMON_BUTTON2_BIN,      COMPO_ID_BTN_CUSTOM,    160,    336+60,    true},
};

#define TIMER_TXT_ITEM_CNT  ((int)(sizeof(tbl_timer_txt_item) / sizeof(tbl_timer_txt_item[0])))
static const timer_txt_item_t tbl_timer_txt_item[] = {
    {"1min",      4,    64,   128,    true},
    {"2min",      4,    160,  128,    true},
    {"3min",      4,    256,  128,    true},
    {"5min",      4,    64,   228,    true},
    {"10min",     5,    160,  228,    true},
    {"30min",     5,    256,  228,    true},
    {"custom",    6,    160,  380,    true},
};

//PAGE_CUSTOM
#define TIMER_CUSTOM_BTN_ITEM_CNT   ((int)(sizeof(tbl_timer_custom_btn_item) / sizeof(tbl_timer_custom_btn_item[0])))
static const timer_btn_item_t tbl_timer_custom_btn_item[] = {
    {UI_BUF_COMMON_INCREASE_BIN,        COMPO_ID_BTN_HOUR_INC,      65,     100,    true},
    {UI_BUF_COMMON_INCREASE_BIN,        COMPO_ID_BTN_MIN_INC,       160,    100,    true},
    {UI_BUF_COMMON_INCREASE_BIN,        COMPO_ID_BTN_SEC_INC,       255,    100,    true},
    {UI_BUF_COMMON_REDUCE_BIN,          COMPO_ID_BTN_HOUR_RED,      65,     239,    true},
    {UI_BUF_COMMON_REDUCE_BIN,          COMPO_ID_BTN_MIN_RED,       160,    239,    true},
    {UI_BUF_COMMON_REDUCE_BIN,          COMPO_ID_BTN_SEC_RED,       255,    239,    true},
    {UI_BUF_COMMON_BUTTON1_BIN,          COMPO_ID_BTN_OK,            160,    336+60,    true},
};

//创建定时器窗体，文件内调用
static compo_form_t *func_timer_form_create_by_type(u8 page_type)
{
    u8 hour, min, sec;
    u32 res_addr;
    char str_buff[24];
    compo_button_t *btn;
    compo_textbox_t *txt;

    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    // compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    // compo_form_set_title(frm, i18n[STR_TIMER]);
    txt = compo_textbox_create(frm, 10);
    compo_textbox_set(txt, i18n[STR_TIMER]);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 50);

    switch (page_type) {
    case TIMER_PAGE_SELECT:
        //创建按钮
        for (u8 idx = 0; idx < TIMER_BTN_ITEM_CNT; idx++) {
            btn = compo_button_create_by_image(frm, tbl_timer_btn_item[idx].res_addr);
            compo_setid(btn, tbl_timer_btn_item[idx].btn_id);
            compo_button_set_pos(btn, tbl_timer_btn_item[idx].x + 70, tbl_timer_btn_item[idx].y);
            compo_button_set_visible(btn, tbl_timer_btn_item[idx].visible_en);
        }
        //创建文本
        for (u8 idx = 0; idx < TIMER_TXT_ITEM_CNT; idx++) {
            txt = compo_textbox_create(frm, tbl_timer_txt_item[idx].max_word_cnt);
            compo_textbox_set_pos(txt, tbl_timer_txt_item[idx].x + 70, tbl_timer_txt_item[idx].y);
            compo_textbox_set(txt, tbl_timer_txt_item[idx].text);
            compo_textbox_set_visible(txt, tbl_timer_txt_item[idx].visible_en);
        }
        break;

    case TIMER_PAGE_CUSTOM:
        //创建按钮
        for (u8 idx = 0; idx < TIMER_CUSTOM_BTN_ITEM_CNT; idx++) {
            btn = compo_button_create_by_image(frm, tbl_timer_custom_btn_item[idx].res_addr);
            compo_setid(btn, tbl_timer_custom_btn_item[idx].btn_id);
            compo_button_set_pos(btn, tbl_timer_custom_btn_item[idx].x + 70, tbl_timer_custom_btn_item[idx].y);
            compo_button_set_visible(btn, tbl_timer_custom_btn_item[idx].visible_en);
        }
        //创建数字
        hour = SEC_TO_HOUR(sys_cb.timer_custom_sec);
        min = SEC_TO_MIN(sys_cb.timer_custom_sec);
        sec = SEC_TO_SEC(sys_cb.timer_custom_sec);
        txt = compo_textbox_create(frm, 12);
        compo_setid(txt, COMPO_ID_NUM_CUSTOM);
        compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 165);
        compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
        snprintf(str_buff, sizeof(str_buff), "%02d:%02d:%02d", hour, min, sec);
        compo_textbox_set(txt, str_buff);
        //创建文本
        txt = compo_textbox_create(frm, 3);
        compo_textbox_set_pos(txt, 160+70, 340+50);
        compo_textbox_set(txt, "OK");
        break;

    case TIMER_PAGE_COUNTDOWN:
        //新建按钮
        res_addr = sys_cb.timer_sta == TIMER_STA_WORKING ? UI_BUF_COMMON_PAUSE_BIN : (sys_cb.timer_sta == TIMER_STA_DONE ? UI_BUF_TIMER_AGAIN_BIN : UI_BUF_COMMON_START_BIN);
        btn = compo_button_create_by_image(frm, res_addr); //start/pause/again
        compo_setid(btn, COMPO_ID_BTN_START);
        compo_button_set_pos(btn, 233+70, 321);
        btn = compo_button_create_by_image(frm, UI_BUF_COMMON_NO_BIN);  //close
        compo_setid(btn, COMPO_ID_BTN_NO);
        compo_button_set_pos(btn, 87+70, 321);
        //新建数字
        hour = SEC_TO_HOUR(sys_cb.timer_left_sec);
        min = SEC_TO_MIN(sys_cb.timer_left_sec);
        sec = SEC_TO_SEC(sys_cb.timer_left_sec);
        txt = compo_textbox_create(frm, 12);
        compo_setid(txt, COMPO_ID_NUM_COUNTDOWN);
        compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 165);
        compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
        snprintf(str_buff, sizeof(str_buff), "%02d:%02d:%02d", hour, min, sec);
        compo_textbox_set(txt, str_buff);
        break;

    default:
        break;
    }

    return frm;
}

//创建定时器窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_timer_form_create(void)
{
    compo_form_t *frm = NULL;

    switch (sys_cb.timer_sta) {
    case TIMER_STA_IDLE:
        frm = func_timer_form_create_by_type(TIMER_PAGE_SELECT);
        break;

    case TIMER_STA_WORKING:
    case TIMER_STA_PAUSE:
    case TIMER_STA_DONE:
    case TIMER_STA_RESET:
        frm = func_timer_form_create_by_type(TIMER_PAGE_COUNTDOWN);
        break;

    default:
        break;
    }

    return frm;
}

//触摸按钮效果处理
static void func_timer_button_touch_handle(void)
{
    compo_button_t *btn;
    int id = compo_get_button_id();

    switch (id) {
    case COMPO_ID_BTN_START:
        btn = compo_getobj_byid(COMPO_ID_BTN_START);
        if (sys_cb.timer_sta == TIMER_STA_WORKING) {
            compo_button_set_bgimg(btn, UI_BUF_COMMON_PAUSE_BIN);
            compo_button_set_alpha(btn, UI_BTN_CLICK_EFFECT_ALPHA);
        } else if (sys_cb.timer_sta == TIMER_STA_DONE) {
            ;
        } else {
            compo_button_set_bgimg(btn, UI_BUF_COMMON_START_BIN);
            compo_button_set_alpha(btn, UI_BTN_CLICK_EFFECT_ALPHA);
        }
        break;

    case COMPO_ID_BTN_NO:
        btn = compo_getobj_byid(COMPO_ID_BTN_NO);

        compo_button_set_alpha(btn, UI_BTN_CLICK_EFFECT_ALPHA);
        break;

    default:
        break;
    }

}

//释放按钮效果处理
static void func_timer_button_release_handle(void)
{
    u32 res_addr;
    compo_button_t *btn;

    if ((btn = compo_getobj_byid(COMPO_ID_BTN_START)) != NULL) {
        res_addr = sys_cb.timer_sta == TIMER_STA_WORKING ? UI_BUF_COMMON_PAUSE_BIN : (sys_cb.timer_sta == TIMER_STA_DONE ? UI_BUF_TIMER_AGAIN_BIN : UI_BUF_COMMON_START_BIN);
        compo_button_set_bgimg(btn, res_addr);
        compo_button_set_alpha(btn, UI_BTN_NORMAL_EFFECT_ALPHA);
    }
    if ((btn = compo_getobj_byid(COMPO_ID_BTN_NO)) != NULL) {

       compo_button_set_alpha(btn, UI_BTN_NORMAL_EFFECT_ALPHA);
    }
}

//100ms计时器秒数刷新回调函数
static void timer_100ms_pro(co_timer_t *timer, void *param)
{
    u8 *count = NULL;
    bool done = false;
    if (param) {
        count = (u8 *)param;
    }
    if (count && sys_cb.timer_sta == TIMER_STA_WORKING && !sys_cb.gui_sleep_sta) {  //休眠不计时
        if (sys_cb.timer_left_sec == 0) {
            done = true;
        }
        if (++(*count) >= 10) { //1s
            *count = 0;
            if (--sys_cb.timer_left_sec == 0) {
                done = true;
            }
        }
    }
    if (done) {
        printf(">>>TIMER_DONE\n");
        sys_cb.timer_sta = TIMER_STA_DONE;
    }
}

//单击按钮
static void func_timer_button_click(void)
{
    static co_timer_t timer_timer;
    static u8 count_100ms;  //100ms定时器计数，10次为1s，减小暂停误差
    f_timer_t *f_timer = (f_timer_t*)func_cb.f_cb;
    u8 page_next = f_timer->page_disp;
    int id = compo_get_button_id();

    switch (f_timer->page_disp) {
    case TIMER_PAGE_SELECT:
        switch (id) {
        case COMPO_ID_BTN_1MIN:
        case COMPO_ID_BTN_2MIN:
        case COMPO_ID_BTN_3MIN:
        case COMPO_ID_BTN_5MIN:
        case COMPO_ID_BTN_10MIN:
        case COMPO_ID_BTN_30MIN:
            sys_cb.timer_left_sec = sys_cb.timer_total_sec = id * 60;
            sys_cb.timer_sta = TIMER_STA_RESET;
            page_next = TIMER_PAGE_COUNTDOWN;
            break;

        case COMPO_ID_BTN_CUSTOM:
            f_timer->hour = SEC_TO_HOUR(sys_cb.timer_custom_sec);
            f_timer->min = SEC_TO_MIN(sys_cb.timer_custom_sec);
            f_timer->sec = SEC_TO_SEC(sys_cb.timer_custom_sec);
            page_next = TIMER_PAGE_CUSTOM;
            break;

        default:
            break;
        }
        break;

    case TIMER_PAGE_CUSTOM:
        switch (id) {
        case COMPO_ID_BTN_HOUR_INC:
        case COMPO_ID_BTN_HOUR_RED:
            f_timer->hour = (f_timer->hour + (id == COMPO_ID_BTN_HOUR_INC ? 1 : -1)) % 24;
            f_timer->hour = f_timer->hour < 0 ? 23 : f_timer->hour;
            break;

        case COMPO_ID_BTN_MIN_INC:
        case COMPO_ID_BTN_MIN_RED:
            f_timer->min = (f_timer->min + (id == COMPO_ID_BTN_MIN_INC ? 1 : -1)) % 60;
            f_timer->min = f_timer->min < 0 ? 59 : f_timer->min;
            break;

        case COMPO_ID_BTN_SEC_INC:
        case COMPO_ID_BTN_SEC_RED:
            f_timer->sec = (f_timer->sec + (id == COMPO_ID_BTN_SEC_INC ? 1 : -1)) % 60;
            f_timer->sec = f_timer->sec < 0 ? 59 : f_timer->sec;
            break;

        case COMPO_ID_BTN_OK:
            sys_cb.timer_left_sec = sys_cb.timer_total_sec = sys_cb.timer_custom_sec = f_timer->hour * 3600 + f_timer->min * 60 + f_timer->sec;
            sys_cb.timer_sta = TIMER_STA_RESET;
            page_next = TIMER_PAGE_COUNTDOWN;
            break;

        default:
            break;
        }
        break;

    case TIMER_PAGE_COUNTDOWN:
        switch (id) {
        case COMPO_ID_BTN_START:
            if (sys_cb.timer_sta == TIMER_STA_WORKING) {
                sys_cb.timer_sta = TIMER_STA_PAUSE;
            } else if (sys_cb.timer_sta == TIMER_STA_DONE) {
                count_100ms = 0;
                sys_cb.timer_left_sec = sys_cb.timer_total_sec;
                sys_cb.timer_sta = TIMER_STA_RESET;
            } else {
                if (sys_cb.timer_sta == TIMER_STA_RESET) {
                    count_100ms = 0;
                    co_timer_set(&timer_timer, 100, TIMER_REPEAT, LEVEL_LOW_PRI, timer_100ms_pro, &count_100ms);
                }
                sys_cb.timer_sta = TIMER_STA_WORKING;
            }
            break;

        case COMPO_ID_BTN_NO:
            co_timer_del(&timer_timer);
            sys_cb.timer_sta = TIMER_STA_IDLE;
            sys_cb.timer_left_sec = sys_cb.timer_total_sec = 0;
            page_next = TIMER_PAGE_SELECT;
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    func_timer_button_release_handle();

    if (page_next != f_timer->page_disp) {
        f_timer->page_disp = page_next;
        compo_form_destroy(func_cb.frm_main);
        func_cb.frm_main = func_timer_form_create_by_type(f_timer->page_disp);
    }

//    TRACE("page[%d] sta[%d]\n", f_timer->page_disp, sys_cb.timer_sta);
}

//定时器功能事件处理
static void func_timer_process(void)
{
    u8 hour, min, sec;
    char str_buff[24];
    compo_textbox_t *txt_num;
    compo_button_t *btn;
    f_timer_t *f_timer = (f_timer_t*)func_cb.f_cb;

    switch (f_timer->page_disp) {
    case TIMER_PAGE_CUSTOM:
        txt_num = compo_getobj_byid(COMPO_ID_NUM_CUSTOM);
        snprintf(str_buff, sizeof(str_buff), "%02d:%02d:%02d", f_timer->hour, f_timer->min, f_timer->sec);
        compo_textbox_set(txt_num, str_buff);
        break;

    case TIMER_PAGE_COUNTDOWN:
        hour = SEC_TO_HOUR(sys_cb.timer_left_sec);
        min = SEC_TO_MIN(sys_cb.timer_left_sec);
        sec = SEC_TO_SEC(sys_cb.timer_left_sec);
        if (f_timer->sec != sec || f_timer->min != min || f_timer->hour != hour) {
            f_timer->sec = sec;
            f_timer->min = min;
            f_timer->hour = hour;
            txt_num = compo_getobj_byid(COMPO_ID_NUM_COUNTDOWN);
            snprintf(str_buff, sizeof(str_buff), "%02d:%02d:%02d", f_timer->hour, f_timer->min, f_timer->sec);
            compo_textbox_set(txt_num, str_buff);
        }
        if (sys_cb.timer_sta == TIMER_STA_DONE) {
            btn = compo_getobj_byid(COMPO_ID_BTN_START);
            compo_button_set_bgimg(btn, UI_BUF_TIMER_AGAIN_BIN);
        }
        break;

    default:
        break;
    }

    func_process();
}

//定时器功能消息处理
static void func_timer_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_TOUCH:
        func_timer_button_touch_handle();
        break;

    case MSG_CTP_CLICK:
        func_timer_button_click();
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_LONG:
        func_timer_button_release_handle();
        if (func_cb.flag_sort) {
            func_message(msg);
        }
        break;

    case MSG_CTP_SHORT_RIGHT:
        func_timer_button_release_handle();
        func_message(msg);
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入定时器功能
void func_timer_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_timer_t));
    func_cb.frm_main = func_timer_form_create();

    f_timer_t *f_timer = (f_timer_t*)func_cb.f_cb;
    f_timer->page_disp = sys_cb.timer_sta == TIMER_STA_IDLE ? TIMER_PAGE_SELECT : TIMER_PAGE_COUNTDOWN;
}

//退出定时器功能
void func_timer_exit(void)
{
}

//定时器功能
void func_timer(void)
{
    printf("%s\n", __func__);
    func_timer_enter();
    while (func_cb.sta == FUNC_TIMER) {
        func_timer_process();
        func_timer_message(msg_dequeue());
    }
    func_timer_exit();
}
