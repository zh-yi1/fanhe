#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//组件ID
enum{
    //按键
    COMPO_ID_BTN_YEAR_INC = 1,
    COMPO_ID_BTN_MON_INC,
    COMPO_ID_BTN_DAY_INC,
    COMPO_ID_BTN_YEAR_REDU,
    COMPO_ID_BTN_MON_REDU,
    COMPO_ID_BTN_DAY_REDU,
    COMPO_ID_BTN_NO,
    COMPO_ID_BTN_YES,

    //数字
	COMPO_ID_NUM_DISP_YEAR,
	COMPO_ID_NUM_DISP_MON,
	COMPO_ID_NUM_DISP_DAY,
};

typedef struct f_time_sub_custom_t_ {
    u16 year;
    u8 mon;
    u8 day;
} f_time_sub_custom_t;

#define TIME_SUB_CUSTOM_BTN_ITEM_CNT    ((int)(sizeof(tbl_time_sub_custom_btn_item) / sizeof(tbl_time_sub_custom_btn_item[0])))

typedef struct time_sub_custom_btn_item_t_ {
    u32 res_addr;
    u16 btn_id;
    s16 x;
    s16 y;
} time_sub_custom_btn_item_t;

//按钮item，创建时遍历一下
static const  time_sub_custom_btn_item_t tbl_time_sub_custom_btn_item[] = {
    // {UI_BUF_COMMON_INCREASE_BIN,             COMPO_ID_BTN_YEAR_INC,         62,     97},
    // {UI_BUF_COMMON_INCREASE_BIN,             COMPO_ID_BTN_MON_INC,          151,    97},
    // {UI_BUF_COMMON_INCREASE_BIN,             COMPO_ID_BTN_DAY_INC,          254,    97},
    // {UI_BUF_COMMON_REDUCE_BIN,               COMPO_ID_BTN_YEAR_REDU,        62,     236},
    // {UI_BUF_COMMON_REDUCE_BIN,               COMPO_ID_BTN_MON_REDU,         151,    236},
    // {UI_BUF_COMMON_REDUCE_BIN,               COMPO_ID_BTN_DAY_REDU,         254,    236},
    // {UI_BUF_COMMON_NO_BIN,                   COMPO_ID_BTN_NO,               62,     320},
   // {UI_BUF_ALARM_CLOCK_YES_BIN,             COMPO_ID_BTN_YES,              254,    320},
};

//创建日期调整显示窗体
compo_form_t *func_time_sub_custom_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //创建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 6);
    compo_textbox_set_pos(txt, 80, 34);
    compo_textbox_set(txt, "修改日期");
    compo_textbox_set_visible(txt, true);

    //创建按钮
    compo_button_t *btn;
    for (u8 idx_btn = 0; idx_btn < TIME_SUB_CUSTOM_BTN_ITEM_CNT; idx_btn++) {
        btn = compo_button_create_by_image(frm, tbl_time_sub_custom_btn_item[idx_btn].res_addr);
        compo_setid(btn, tbl_time_sub_custom_btn_item[idx_btn].btn_id);
        compo_button_set_pos(btn, tbl_time_sub_custom_btn_item[idx_btn].x, tbl_time_sub_custom_btn_item[idx_btn].y);
        if (idx_btn  <= 5) {
            compo_button_set_alpha(btn, UI_BTN_CLICK_EFFECT_ALPHA);
        }
    }

    //创建数字
    char str_buff[8];
    txt = compo_textbox_create(frm, 4);    //年
    compo_setid(txt, COMPO_ID_NUM_DISP_YEAR);
    compo_textbox_set_pos(txt, 62, 167);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_24_BIN);
    {
        tm_t tm_rtc = rtc_clock_get();

        snprintf(str_buff, sizeof(str_buff), "%04d", tm_rtc.year);
        compo_textbox_set(txt, str_buff);
        txt = compo_textbox_create(frm, 2);    //月
        compo_setid(txt, COMPO_ID_NUM_DISP_MON);
        compo_textbox_set_pos(txt, 152, 167);
        compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_24_BIN);
        snprintf(str_buff, sizeof(str_buff), "%02d", tm_rtc.mon);
        compo_textbox_set(txt, str_buff);
        txt = compo_textbox_create(frm, 2);    //日
        compo_setid(txt, COMPO_ID_NUM_DISP_DAY);
        compo_textbox_set_pos(txt, 252, 167);
        compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_24_BIN);
        snprintf(str_buff, sizeof(str_buff), "%02d", tm_rtc.day);
    }
    compo_textbox_set(txt, str_buff);

    return frm;
}

//日期调整功能事件处理
static void func_time_sub_custom_process(void)
{
    func_process();
}

//单击按钮
static void func_time_sub_custom_button_click(void)
{
    int id = compo_get_button_id();
    u16 year;
    u8 mon;
    u8 day;
    char str_buff[8];

    f_time_sub_custom_t *time = (f_time_sub_custom_t *)func_cb.f_cb;
    tm_t tm_rtc = rtc_clock_get();

    if (time->year == 0) {
        year = tm_rtc.year;
    } else {
        year = time->year;
    }

    if (time->mon == 0) {
        mon = tm_rtc.mon;
    } else {
        mon = time->mon;
    }

    if (time->day == 0) {
        day = tm_rtc.day;
    } else {
        day = time->day;
    }

    //获取数字组件的地址
    compo_textbox_t *num_year  = compo_getobj_byid(COMPO_ID_NUM_DISP_YEAR);
    compo_textbox_t *num_mon   = compo_getobj_byid(COMPO_ID_NUM_DISP_MON);
    compo_textbox_t *num_day   = compo_getobj_byid(COMPO_ID_NUM_DISP_DAY);

    switch (id) {
        case COMPO_ID_BTN_YEAR_INC:
            if(time->year < 9999 && year < 9999) {
                year++;
            }
            break;

        case COMPO_ID_BTN_MON_INC:
            if(time->mon < 12 && mon < 12) {
                mon++;
            }
            break;

        case COMPO_ID_BTN_DAY_INC:
            if(time->day < 31 && day < 31) {
                day++;
            }
            break;

        case COMPO_ID_BTN_YEAR_REDU:
            if( time->year >0 || year > 0) {
                year--;
                if(year == 0) {
                year = 9999;
                }
            }
            break;

        case COMPO_ID_BTN_MON_REDU:
            if(time->mon >0 || mon > 0) {
                mon--;
                if(mon == 0) {
                mon = 12;
                }
            }
            break;

        case COMPO_ID_BTN_DAY_REDU:
            if(time->day >0 || day > 0) {
                day--;
                if(day == 0) {
                day = 31;
                }
            }
            break;

        case COMPO_ID_BTN_NO:
            time->year = time->mon = time->day = 0;
            func_cb.sta = FUNC_SET_SUB_TIME;
            break;

        case COMPO_ID_BTN_YES:
            {
                tm_t tm_set = rtc_clock_get();

                tm_set.year = year;
                tm_set.mon = mon;
                tm_set.day = day;
                rtc_clock_set(tm_set);
                sys_cb.year = year;
                sys_cb.mon = mon;
                sys_cb.day = day;
            }
            time->year = time->mon = time->day = 0;
            func_cb.sta = FUNC_SET_SUB_TIME;
            break;

        default:
            break;
        }

        time->year = year;
        time->mon  = mon;
        time->day  = day;

        snprintf(str_buff, sizeof(str_buff), "%04d", time->year);
        compo_textbox_set(num_year, str_buff);
        snprintf(str_buff, sizeof(str_buff), "%02d", time->mon);
        compo_textbox_set(num_mon, str_buff);
        snprintf(str_buff, sizeof(str_buff), "%02d", time->day);
        compo_textbox_set(num_day, str_buff);
}

//日期调整功能消息处理
static void func_time_sub_custom_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_time_sub_custom_button_click();
        break;


    case MSG_QDEC_FORWARD:
    case MSG_QDEC_BACKWARD:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入日期调整功能
void func_time_sub_custom_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_time_sub_custom_t));
    func_cb.frm_main = func_time_sub_custom_form_create();
}

//退出日期调整功能
void func_time_sub_custom_exit(void)
{
    func_cb.last = FUNC_TIME_SUB_CUSTOM;
}

//日期调整功能
void func_time_sub_custom(void)
{
    printf("%s\n", __func__);
    func_time_sub_custom_enter();
    while (func_cb.sta == FUNC_TIME_SUB_CUSTOM) {
        func_time_sub_custom_process();
        func_time_sub_custom_message(msg_dequeue());
    }
    func_time_sub_custom_exit();
}
