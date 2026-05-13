#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_weather_t_ {

} f_weather_t;

//创建天气窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_weather_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_WEATHER]);

    sys_cb.temperature[0] = 18; //test
    sys_cb.temperature[1] = 29;

	//创建图片
	compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_WEATHER_WEATHER_LIST_BIN);
	compo_picturebox_cut(pic, sys_cb.weather_idx, WEATHER_CNT);
	compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, 193);
	compo_bonddata(pic, COMPO_BOND_WEATHER);

	//创建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 7);
    compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 330);
	compo_bonddata(txt, COMPO_BOND_TEMPERATURE);

	compo_textbox_t *txt2 = compo_textbox_create(frm, 6);
	compo_textbox_set_pos(txt2, GUI_SCREEN_CENTER_X, 375);
	compo_bonddata(txt2, COMPO_BOND_WEATHER);

    return frm;
}

//天气功能事件处理
static void func_weather_process(void)
{
    func_process();
}

//天气功能消息处理
static void func_weather_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        sys_cb.weather_idx = (sys_cb.weather_idx + 1) % WEATHER_CNT;    //test
        printf("sys_cb.weather_idx=%d\n", sys_cb.weather_idx);
        break;

    case MSG_CTP_SHORT_UP:
        break;

    case MSG_CTP_SHORT_DOWN:
        break;

    case MSG_CTP_LONG:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入天气功能
void func_weather_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_weather_t));
    func_cb.frm_main = func_weather_form_create();
}

//退出天气功能
void func_weather_exit(void)
{
    func_cb.last = FUNC_WEATHER;
}

//天气功能
void func_weather(void)
{
    printf("%s\n", __func__);
    func_weather_enter();
    while (func_cb.sta == FUNC_WEATHER) {
        func_weather_process();
        func_weather_message(msg_dequeue());
    }
    func_weather_exit();
}
