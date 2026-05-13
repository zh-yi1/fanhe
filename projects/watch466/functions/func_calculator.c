#include "include.h"
#include "func.h"
#include "func_calculator_alg.h"

#define TRACE_EN    1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

enum {
    COMPO_ID_NUM_DISP = 128,
};

typedef struct f_calculator_t_ {

} f_calculator_t;

#define CALCULATOR_DISP_BTN_ITEM_CNT    ((int)(sizeof(tbl_calculator_disp_btn_item) / sizeof(tbl_calculator_disp_btn_item[0])))
#define CALCULATOR_DISP_NUMBER_MAX      32                 //显示最大位数

typedef struct calculator_disp_btn_item_t_ {
    u32 res_addr;
    u16 btn_id;
    s16 x;
    s16 y;
} calculator_disp_btn_item_t;

//按钮item，创建时遍历一下
static const calculator_disp_btn_item_t tbl_calculator_disp_btn_item[] = {
    {UI_BUF_CALCULATOR_1_CLICK_BIN,             BTN_1,              43+33-2+1,     274+56+5+1},
    {UI_BUF_CALCULATOR_2_CLICK_BIN,             BTN_2,              121+33+5,    274+56+5+1},
    {UI_BUF_CALCULATOR_3_CLICK_BIN,             BTN_3,              199+33+5,    274+56+5+1},
    {UI_BUF_CALCULATOR_4_CLICK_BIN,             BTN_4,              43+33,     216+45+5+1},
    {UI_BUF_CALCULATOR_5_CLICK_BIN,             BTN_5,              121+33+5,    216+45+5+1},
    {UI_BUF_CALCULATOR_6_CLICK_BIN,             BTN_6,              199+33+5+1,    216+45+5+1},
    {UI_BUF_CALCULATOR_7_CLICK_BIN,             BTN_7,              43+33,     158+40},
    {UI_BUF_CALCULATOR_8_CLICK_BIN,             BTN_8,              121+33+5,    158+40},
    {UI_BUF_CALCULATOR_9_CLICK_BIN,             BTN_9,              199+33+5,    158+40},
    {UI_BUF_CALCULATOR_0_CLICK_BIN,             BTN_0,              121+45,    332+76},
    {UI_BUF_CALCULATOR_POINT_CLICK_BIN,         BTN_POINT,          199+45-2,    332+76},
    {UI_BUF_CALCULATOR_MINUS_CLICK_BIN,         BTN_OPPOSITE,       43+280,     332-132},
    {UI_BUF_CALCULATOR_CE_CLICK_BIN,            BTN_CE,             43+104+5+2,     100+30-1},
    {UI_BUF_CALCULATOR_C_CLICK_BIN,             BTN_C,              121-50+5,    100+30},
    {UI_BUF_CALCULATOR_DEL_CLICK_BIN,           BTN_DEL,            199+200+5,    100+102-3-1},
    {UI_BUF_CALCULATOR_DIVIDED_CLICK_BIN,       BTN_DIV,            277+135-5-2,    100+234+5+1},
    {UI_BUF_CALCULATOR_MULTIPLY_CLICK_BIN,      BTN_MUL,            277+135-5-2,    158+102+5+2},
    {UI_BUF_CALCULATOR_REDUCE_CLICK_BIN,        BTN_SUB,            277+45,    216+122},
    {UI_BUF_CALCULATOR_ADD_CLICK_BIN,           BTN_ADD,            277+45,    274-5-1},
    {UI_BUF_CALCULATOR_EQUAL_CLICK_BIN,         BTN_EQUAL,          277+45,    332+76},
};

//创建计算器窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_calculator_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_CALCULATOR_BG_BIN);
    compo_picturebox_set_pos(pic, 160+50+30, 228+40);

    //创建按钮
    compo_button_t *btn;
    for (u8 idx_btn = 0; idx_btn < CALCULATOR_DISP_BTN_ITEM_CNT; idx_btn++) {
        if (tbl_calculator_disp_btn_item[idx_btn].res_addr) {
            btn = compo_button_create_by_image(frm, tbl_calculator_disp_btn_item[idx_btn].res_addr);
            compo_button_set_pos(btn, tbl_calculator_disp_btn_item[idx_btn].x, tbl_calculator_disp_btn_item[idx_btn].y); //x+50
        } else {
            btn = compo_button_create(frm);
            compo_button_set_location(btn, tbl_calculator_disp_btn_item[idx_btn].x, tbl_calculator_disp_btn_item[idx_btn].y, 76, 56);//x+50
        }
        compo_setid(btn, tbl_calculator_disp_btn_item[idx_btn].btn_id);
        compo_button_set_visible(btn, false);
    }

    //创建文本
    compo_textbox_t *txt;
    txt = compo_textbox_create(frm, CALCULATOR_DISP_NUMBER_MAX);
    compo_setid(txt, COMPO_ID_NUM_DISP);
    compo_textbox_set_pos(txt, 160+50+150, 121);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set(txt, gcal_get_show_str());

    return frm;
}

//计算器功能事件处理
static void func_calculator_process(void)
{
    func_process();
}

//按钮触摸效果
static void func_calculator_button_press_handle(void)
{
    compo_button_t *btn = compo_getobj_byid(compo_get_button_id());
    if (btn) {
        compo_button_set_visible(btn, true);
    }
}

//按钮释放效果
static void func_calculator_button_release_handle(void)
{
    u16 hold_id = gcal_get_holding_operator();
    component_t *compo = (component_t *)compo_pool_get_top();
    while (compo != NULL) {
        if (compo->type == COMPO_TYPE_BUTTON && compo->id != hold_id) {
            compo_button_t *btn = (compo_button_t *)compo;
            compo_button_set_visible(btn, false);
        }
        compo = compo_get_next(compo);
    }
}

//单击按钮
static void func_calculator_button_click_handler(void)
{
    int id = compo_get_button_id();
    compo_textbox_t *txt = compo_getobj_byid(COMPO_ID_NUM_DISP);
    gcalc_btn_click_handler(id);
    compo_textbox_set(txt, gcal_get_show_str());

    func_calculator_button_release_handle();
}

//计算器功能消息处理
static void func_calculator_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_TOUCH:
        func_calculator_button_press_handle();
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_LONG:
        func_calculator_button_release_handle();
        if (func_cb.flag_sort) {
            func_message(msg);
        }
        break;

    case MSG_CTP_SHORT_RIGHT:
        func_calculator_button_release_handle();
        func_message(msg);
        break;

    case MSG_CTP_CLICK:
        func_calculator_button_click_handler();
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入计算器功能
void func_calculator_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_calculator_t));
    func_cb.frm_main = func_calculator_form_create();

    if (gcal_cb_init() == false) {
        halt(HALT_MALLOC);
    }
}

//退出计算器功能
void func_calculator_exit(void)
{
    gcal_cb_destroy();
}

//计算器功能
void func_calculator(void)
{
    printf("%s\n", __func__);
    func_calculator_enter();
    while (func_cb.sta == FUNC_CALCULATOR) {
        func_calculator_process();
        func_calculator_message(msg_dequeue());
    }
    func_calculator_exit();
}
