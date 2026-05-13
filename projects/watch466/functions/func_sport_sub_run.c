#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

u8 func_sport_get_str_idx();

typedef struct f_sport_sub_run_t_ {
    u8 sta;
    u8 min;                 //分
    u8 sec;                 //秒
    u16 msec;               //毫秒
    u32 total_msec;         //总毫秒
    s16 angle;
    s8 dir;
} f_sport_sub_run_t;

enum {
    COMPO_ID_NUM_SPORT_TIME = 1,
    COMPO_ID_NUM_BPM,
    COMPO_ID_NUM_KCAL,
    COMPO_ID_NUM_KM,
    COMPO_ID_PIC_BG,
};

//创建室内跑步窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_sport_sub_run_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_SPORT_EXERCISING_BG_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_X);

    compo_textbox_t *txt = compo_textbox_create(frm, 20);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 80);
    compo_textbox_set(txt, i18n[func_sport_get_str_idx()]);

    txt = compo_textbox_create(frm, 3);
    compo_textbox_set_forecolor(txt, COLOR_DIMGRAY);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 205);
    compo_textbox_set(txt, "bpm");

    txt = compo_textbox_create(frm, 4);
    compo_textbox_set_forecolor(txt, COLOR_DIMGRAY);
    compo_textbox_set_pos(txt, 120, 310);
    compo_textbox_set(txt, "kcal");

    txt = compo_textbox_create(frm, 2);
    compo_textbox_set_forecolor(txt, COLOR_DIMGRAY);
    compo_textbox_set_pos(txt, 340, 310);
    compo_textbox_set(txt, "km");

	//创建数字
	txt = compo_textbox_create(frm, 3);
	compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_setid(txt, COMPO_ID_NUM_BPM);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 150);
    compo_textbox_set(txt, "136");

    txt = compo_textbox_create(frm, 3);
	compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_setid(txt, COMPO_ID_NUM_KCAL);
    compo_textbox_set_pos(txt, 120, 260);
    compo_textbox_set(txt, "120");

    txt = compo_textbox_create(frm, 3);
	compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_setid(txt, COMPO_ID_NUM_KM);
    compo_textbox_set_pos(txt, 340, 260);
    compo_textbox_set(txt, "5.2");

    txt = compo_textbox_create(frm, 10);
	compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_setid(txt, COMPO_ID_NUM_SPORT_TIME);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 375);
    compo_textbox_set(txt, "00:00:00");

    pic = compo_picturebox_create(frm, UI_BUF_SPORT_EXERCISING_ARROW_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, 230);
    compo_picturebox_set_rotation_center(pic, 13, 222);
    compo_setid(pic, COMPO_ID_PIC_BG);

    return frm;
}

//室内跑步功能事件处理
static void func_sport_sub_run_process(void)
{
    f_sport_sub_run_t *f_sport_sub_run = (f_sport_sub_run_t *)func_cb.f_cb;
    compo_textbox_t *txt_time = compo_getobj_byid(COMPO_ID_NUM_SPORT_TIME);
    compo_picturebox_t *pic = compo_getobj_byid(COMPO_ID_PIC_BG);
    char buf_time[15];
    static u32 sport_ticks = 0, sport_ticks1= 0;

    if (tick_check_expire(sport_ticks, 10)) {
        sport_ticks = tick_get();

        f_sport_sub_run->total_msec += 10;
        f_sport_sub_run->min = ((f_sport_sub_run->total_msec / 1000) % 3600) / 60;
        f_sport_sub_run->sec = (f_sport_sub_run->total_msec / 1000) % 60;
        f_sport_sub_run->msec = f_sport_sub_run->total_msec % 1000;

        snprintf(buf_time, sizeof(buf_time), "%02d:%02d:%02d", f_sport_sub_run->min, f_sport_sub_run->sec, f_sport_sub_run->msec / 10);
        compo_textbox_set(txt_time, buf_time);
    }

    if (tick_check_expire(sport_ticks1, 20)) {
        sport_ticks1 = tick_get();

        f_sport_sub_run->angle += 10*f_sport_sub_run->dir;
        if (f_sport_sub_run->angle >= 1200) {
            f_sport_sub_run->dir = -1;
        } else if (f_sport_sub_run->angle <= -1200) {
            f_sport_sub_run->dir = 1;
        }
        compo_picturebox_set_rotation(pic, f_sport_sub_run->angle);
    }

    func_process();
}

//室内跑步功能消息处理
static void func_sport_sub_run_message(size_msg_t msg)
{

    switch (msg) {
    case MSG_CTP_CLICK:
        break;

    case MSG_CTP_SHORT_UP:
        break;

    case MSG_CTP_SHORT_DOWN:
        break;

    case MSG_CTP_LONG:
        break;

    case MSG_QDEC_FORWARD:
    case MSG_QDEC_BACKWARD:
        break;

    case MSG_CTP_SHORT_LEFT:
        break;


    default:
        func_message(msg);
        break;
    }
}

//进入室内跑步功能
void func_sport_sub_run_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_sport_sub_run_t));
    func_cb.frm_main = func_sport_sub_run_form_create();
    f_sport_sub_run_t *f_sport_sub_run = (f_sport_sub_run_t *)func_cb.f_cb;
    f_sport_sub_run->dir = 1;
}

//退出室内跑步功能
void func_sport_sub_run_exit(void)
{
    func_cb.last = FUNC_SPORT_SUB_RUN;
}

//室内跑步功能
void func_sport_sub_run(void)
{
    printf("%s\n", __func__);
    func_sport_sub_run_enter();
    while (func_cb.sta == FUNC_SPORT_SUB_RUN) {
        func_sport_sub_run_process();
        func_sport_sub_run_message(msg_dequeue());
    }
    func_sport_sub_run_exit();
}
