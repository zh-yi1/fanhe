#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_debug_info_t_ {

} f_debug_info_t;

//创建DEBUG窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_debug_info_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    char debug_info_str[300];
    compo_textbox_t *txt = compo_textbox_create(frm, 300);
    compo_textbox_set_multiline(txt, true);
    compo_textbox_set_location(txt, 160, 210, 320, 385);
    compo_textbox_set_multiline_drag(txt, true);
    print_r32(exception_debug_info_get(), 32);
    for (int i=0; i<32; i++) {
        sprintf(debug_info_str+(9*i), "%08x ", (int)(exception_debug_info_get()[i]));
    }
    compo_textbox_set(txt, debug_info_str);
    return frm;
}

//触摸某一个区域触发
void func_debug_info_check()
{
    static u32 ticks;
    static u8 cnt;
    static u8 clr;
    s32 sx, sy, x, y;
    bool press = ctp_get_cur_point(&sx, &sy, &x, &y);
    if(tick_check_expire(ticks, 100)) {
        ticks = tick_get();
        if (++clr > 30) {   //3s超时
            cnt = 0;
            clr = 0;
        }
        if (press && sx > GUI_SCREEN_WIDTH - 60 && sy > GUI_SCREEN_HEIGHT - 60) {   //右下角
            if(++cnt > 4){  //触摸次数
                cnt = 0;
                clr = 0;
                func_cb.sta = FUNC_DEBUG_INFO;
            }
        }
    }
}

//DEBUG功能事件处理
static void func_debug_info_process(void)
{
    func_process();
}

//DEBUG功能消息处理
static void func_debug_info_message(size_msg_t msg)
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

//进入DEBUG功能
void func_debug_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_debug_info_t));
    func_cb.frm_main = func_debug_info_form_create();
}

//退出DEBUG功能
void func_debug_info_exit(void)
{
    func_cb.last = FUNC_DEBUG_INFO;
}

//DEBUG功能
void func_debug_info(void)
{
    printf("%s\n", __func__);
    func_debug_enter();
    while (func_cb.sta == FUNC_DEBUG_INFO) {
        func_debug_info_process();
        func_debug_info_message(msg_dequeue());
    }
    func_debug_info_exit();
}
