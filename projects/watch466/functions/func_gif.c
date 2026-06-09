#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_gif_t_ {

} f_gif_t;

//ID
enum {
    COMPO_ID_GIF = 1,
};

static u8 gif_obuf[GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*2] AT(.gif.obuf);
static u8 gif_ibuf[10240] AT(.gif.ibuf);

//创建GIF窗体
compo_form_t *func_gif_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    // compo_gif_t *gif = compo_gif_create(frm, UI_BUF_GIF_HEART_GIF, UI_LEN_GIF_HEART_GIF);
    // compo_gif_set_pos(gif, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    // compo_setid(gif, COMPO_ID_GIF);


    return frm;
}

//GIF功能事件处理
static void func_gif_process(void)
{
    func_process();
}

//GIF功能消息处理
static void func_gif_message(size_msg_t msg)
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

//进入GIF功能
void func_gif_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_gif_t));
    func_cb.frm_main = func_gif_form_create();

    compo_gif_t *gif = compo_getobj_byid(COMPO_ID_GIF);
    compo_gif_set_buf(gif, gif_obuf, gif_ibuf);
    compo_gif_play(gif);
    sleep_only_gui_off_en();
}

//退出GIF功能
void func_gif_exit(void)
{
    compo_gif_t *gif = compo_getobj_byid(COMPO_ID_GIF);
    compo_gif_exit(gif);
    func_cb.last = FUNC_GIF;
    sleep_only_gui_off_dis();
}

//GIF功能
void func_gif(void)
{
    printf("%s\n", __func__);
    func_gif_enter();
    while (func_cb.sta == FUNC_GIF) {
        func_gif_process();
        func_gif_message(msg_dequeue());
    }
    func_gif_exit();
}
