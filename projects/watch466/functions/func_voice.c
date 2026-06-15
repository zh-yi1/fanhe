#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//组件ID
enum {
    //图像
	COMPO_ID_PIC_VOICE = 1,
};

typedef struct f_voice_t_ {

} f_voice_t;

//创建语音助手窗体
compo_form_t *func_voice_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    // compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    // compo_form_set_title_center(frm, true);
    // compo_form_set_title(frm, i18n[STR_VOICE]);

	// //创建动画
    // compo_animation_t *animation = compo_animation_create(frm, UI_BUF_VOICE_320_86_BIN);
    // compo_animation_set_pos(animation, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    // compo_animation_set_radix(animation, 2);
    // compo_animation_set_interval(animation, 1);
    // compo_setid(animation, COMPO_ID_PIC_VOICE);
    // compo_animation_set_roll(animation, UI_BUF_VOICE_320_86_BIN);
    return frm;
}

//动画滚动
static void func_voice_animation_roll(void)
{
    compo_animation_t *animation = compo_getobj_byid(COMPO_ID_PIC_VOICE);

    if (animation->interval == 0) {
        return;
    }

    static s32 dx;
    static u32 tick;
    if (tick_check_expire(tick, animation->interval)){
        tick = tick_get();
        dx--;
        if (dx < -(animation->wid * animation->radix)) {
            dx = 0;
        }
        widget_page_set_client(animation->page, dx, 0);
    }
}


//语音助手功能事件处理
static void func_voice_process(void)
{
    func_voice_animation_roll();
    func_process();
}

//语音助手功能消息处理
static void func_voice_message(size_msg_t msg)
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

    default:
        func_message(msg);
        break;
    }
}

//进入语音助手功能
void func_voice_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_voice_t));
    func_cb.frm_main = func_voice_form_create();
}

//退出语音助手功能
void func_voice_exit(void)
{
    func_cb.last = FUNC_VOICE;
}

//语音助手功能
void func_voice(void)
{
    printf("%s\n", __func__);
    func_voice_enter();
    while (func_cb.sta == FUNC_VOICE) {
        func_voice_process();
        func_voice_message(msg_dequeue());
    }
    func_voice_exit();
}
