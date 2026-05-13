#include "include.h"
#include "func.h"
#include "func_clock.h"

#if 0

#define TBL_ROTARY_ITEM_CNT                     ((int)(sizeof(tbl_rotary_item) / sizeof(tbl_rotary_item[0])))

enum {
    COMPO_ID_ROTARY = 40,
};

typedef struct f_clk_sub_window_t_ {
    u32 tick;
    compo_rotary_t *rotary;
    s32 last_dx;
    s16 test_angle;
    bool flag_drag;
} f_clk_sub_window_t;

static const u32 dialplate_base_addr[] = {
    UI_BUF_Z_PREV_DIAL_1_BIN,
    UI_BUF_Z_PREV_DIAL_2_BIN,
    UI_BUF_Z_PREV_DIAL_3_BIN,
    UI_BUF_Z_PREV_DIAL_4_BIN,
    UI_BUF_Z_PREV_DIAL_5_BIN,
    UI_BUF_Z_PREV_DIAL_6_BIN,
};


//创建转盘，创建窗体中不要使用功能结构体 func_cb.f_cb
static compo_form_t *func_clock_sub_rotary_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_rotary_t *rotary = compo_rotary_create(frm, dialplate_base_addr, sizeof(dialplate_base_addr)/sizeof(dialplate_base_addr[0]), 6);
    compo_setid(rotary, COMPO_ID_ROTARY);
    compo_rotary_update(rotary, true);
    return frm;
}

//时钟表盘主要事件流程处理
static void func_clock_sub_rotary_process(void)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    f_clk_sub_window_t *f_clk_sub_window = f_clk->sub_cb;
    s32 move_dx = 0;

    if (f_clk_sub_window->flag_drag) {
        s32 dx, dy;
        f_clk_sub_window->flag_drag = ctp_get_dxy(&dx, &dy);
        if (f_clk_sub_window->flag_drag) {
            move_dx = dx - f_clk_sub_window->last_dx;
            f_clk_sub_window->last_dx = dx;
            move_dx = -(10 * move_dx);
        }
    } else {
        f_clk_sub_window->last_dx = 0;
    }

    if (move_dx) {
        if ((f_clk_sub_window->test_angle + move_dx) < 0) {
            f_clk_sub_window->test_angle = 3600 + (f_clk_sub_window->test_angle + move_dx);
        } else {
            f_clk_sub_window->test_angle += move_dx;
        }
        if (f_clk_sub_window->test_angle >= 3600) {
            f_clk_sub_window->test_angle = 0;
        }
        compo_rotary_set_rotation(f_clk_sub_window->rotary, f_clk_sub_window->test_angle);
        compo_rotary_update(f_clk_sub_window->rotary, false);
    }

    func_clock_sub_process();
}

//时钟表盘转盘功能消息处理
static void func_clock_sub_rotary_message(size_msg_t msg)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    f_clk_sub_window_t *f_clk_sub_window = f_clk->sub_cb;

    switch (msg) {
    case MSG_CTP_TOUCH:
        f_clk_sub_window->flag_drag = true;
        break;

    case MSG_CTP_CLICK:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_LEFT:
        break;

    case KU_BACK:
        break;

    default:
        func_clock_sub_message(msg);
        break;
    }
}

//时钟表盘转盘进入处理
static void func_clock_sub_rotary_enter(void)
{
    tft_set_temode(2);
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    f_clk->sub_cb = func_zalloc(sizeof(f_clk_sub_window_t));
    f_clk_sub_window_t *f_clk_sub_window = f_clk->sub_cb;
    f_clk->sub_frm = func_clock_sub_rotary_form_create();
    f_clk_sub_window->rotary = compo_getobj_byid(COMPO_ID_ROTARY);
    compo_rotary_t *rotary = f_clk_sub_window->rotary;
    if (rotary->type != COMPO_TYPE_ROTARY) {
        halt(HALT_GUI_COMPO_ROTARY_TYPE);
    }

    compo_form_destroy(func_cb.frm_main);
    f_clk_sub_window->tick = tick_get();
    f_clk->sta = FUNC_CLOCK_SUB_ROTARY;                     //进入到转盘
}

//时钟表盘转盘退出处理
static void func_clock_sub_rotary_exit(void)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    compo_form_destroy(f_clk->sub_frm);
    func_free(f_clk->sub_cb);
    f_clk->sub_frm = NULL;
    f_clk->sub_cb = NULL;
    tft_set_temode(DEFAULT_TE_MODE);
}

//时钟表盘转盘
void func_clock_sub_rotary(void)
{
    printf("%s\n", __func__);
    func_clock_sub_rotary_enter();
    while (func_cb.sta == FUNC_CLOCK && ((f_clock_t *)func_cb.f_cb)->sta == FUNC_CLOCK_SUB_ROTARY) {
        func_clock_sub_rotary_process();
        func_clock_sub_rotary_message(msg_dequeue());
    }
    func_clock_sub_rotary_exit();
    printf("%s exit\n", __func__);
}
#endif
