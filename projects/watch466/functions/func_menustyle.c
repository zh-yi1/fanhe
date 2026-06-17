#include "include.h"
#include "func.h"
#include "func_menu.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define TBL_ROTARY_ITEM_CNT                     ((int)(sizeof(tbl_rotary_item) / sizeof(tbl_rotary_item[0])))
#define ROTARY_RADIUS                           200                             //转盘半径

enum {
    COMPO_ID_ROTARY = 1,
};

typedef struct f_menustyle_t_ {
    compo_rotary_t *rotary;
} f_menustyle_t;

static const compo_rotary_item_t tbl_rotary_item[] = {
    // [MENU_STYLE_HONEYCOMB]          = {UI_BUF_MENUSTYLE_HONEYCOMB_BIN,              STR_STYLE_HONEYCOMB},
    // [MENU_STYLE_WATERFALL]          = {UI_BUF_MENUSTYLE_WATERFALL_BIN,              STR_STYLE_WATERFALL},
    // [MENU_STYLE_CUM_HONEYGRID]      = {UI_BUF_MENUSTYLE_HONEYCOMB_BIN,              STR_STYLE_HONEYCOMB},
    // [MENU_STYLE_LIST]               = {UI_BUF_MENUSTYLE_LIST_BIN,                   STR_STYLE_LIST_1},
    // [MENU_STYLE_FOOTBALL]           = {UI_BUF_MENUSTYLE_FOOTBALL_BIN,               STR_STYLE_FOOTBALL},
    // [MENU_STYLE_SUDOKU]             = {UI_BUF_MENUSTYLE_SUDOKU_BIN,                 STR_STYLE_SUDOKU_1},
    // [MENU_STYLE_SUDOKU_HRZ]         = {UI_BUF_MENUSTYLE_SUDOKU_BIN,                 STR_STYLE_SUDOKU_2},
    // [MENU_STYLE_GRID]               = {UI_BUF_MENUSTYLE_GRID_BIN,                   STR_STYLE_GRID_1},
    // [MENU_STYLE_DISK]               = {UI_BUF_MENUSTYLE_DISK_BIN,                   STR_STYLE_DISK},
    // [MENU_STYLE_RING]               = {UI_BUF_MENUSTYLE_RING_BIN,                   STR_STYLE_RING},
    // [MENU_STYLE_KALE]               = {UI_BUF_MENUSTYLE_KALEIDOSCOPE_BIN,           STR_STYLE_KALEIDOSCOPE},
    // [MENU_STYLE_SKYRER]             = {UI_BUF_MENUSTYLE_SKYRER_BIN,                 STR_STYLE_SKYRER},
    // [MENU_STYLE_CUM_SUDOKU]         = {UI_BUF_MENUSTYLE_CUM_SUDOKU_BIN,             STR_STYLE_SUDOKU_3},
    // [MENU_STYLE_CUM_GRID]           = {UI_BUF_MENUSTYLE_CUM_GRID_BIN,               STR_STYLE_GRID_2},
    // [MENU_STYLE_CUM_HEXAGON]        = {UI_BUF_MENUSTYLE_HEXAGON_BIN,                STR_STYLE_HEXAGON},
    // [MENU_STYLE_CUM_FOURGRID]       = {UI_BUF_MENUSTYLE_LIST_FOURGRID_BIN,          STR_STYLE_LIST_2},
};

//创建转盘，创建窗体中不要使用功能结构体 func_cb.f_cb
static compo_form_t *func_menustyle_rotary_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);

    //创建背景
    compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, make_color(24, 24, 32));
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    //创建转盘
    compo_rotary_t *rotary = compo_rotary_create(frm, tbl_rotary_item, TBL_ROTARY_ITEM_CNT, ROTARY_RADIUS);
    compo_setid(rotary, COMPO_ID_ROTARY);
    compo_rotary_update(rotary);
    return frm;
}

//主菜单样式列表事件处理
static void func_menustyle_process(void)
{
    f_menustyle_t *f_menustyle = (f_menustyle_t *)func_cb.f_cb;
    compo_rotary_t *rotary = f_menustyle->rotary;
    compo_rotary_move(rotary);
    func_process();
    if (compo_rotary_get_sta(rotary) == COMPO_ROTARY_STA_EXIT) {
        func_cb.sta = FUNC_MENU;
    }
}

//主菜单样式列表消息处理
static void func_menustyle_message(size_msg_t msg)
{
    f_menustyle_t *f_menustyle = (f_menustyle_t *)func_cb.f_cb;
    compo_rotary_t *rotary = f_menustyle->rotary;

    switch (msg) {
    case KU_BACK:
        func_cb.menu_style = compo_rotary_get_idx(rotary);
        break;

    default:
        break;
    }
    if (compo_rotary_message(rotary, msg)) {
        return;                                         //处理列表框信息
    }
    switch (msg) {
    case MSG_CTP_CLICK:
        //单击屏幕
        func_cb.menu_style = compo_rotary_get_idx(rotary);
        compo_rotary_move_control(rotary, COMPO_ROTARY_MOVE_CMD_EXITING);
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入主菜单样式列表
void func_menustyle_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_menustyle_t));
    func_cb.frm_main = func_menustyle_rotary_form_create();

    tft_set_temode(2);
    f_menustyle_t *f_menustyle = (f_menustyle_t *)func_cb.f_cb;
    f_menustyle->rotary = compo_getobj_byid(COMPO_ID_ROTARY);
    compo_rotary_t *rotary = f_menustyle->rotary;
    if (rotary->type != COMPO_TYPE_ROTARY) {
        halt(HALT_GUI_COMPO_ROTARY_TYPE);
    }
    compo_rotary_set_rotation_byidx(rotary, func_cb.menu_style);
    compo_rotary_update(rotary);
    compo_rotary_move_control(rotary, COMPO_ROTARY_MOVE_CMD_ENTERING);      //入场
}

//退出主菜单样式列表
void func_menustyle_exit(void)
{
    tft_set_temode(DEFAULT_TE_MODE);
}

//主菜单样式列表
void func_menustyle(void)
{
    printf("%s\n", __func__);
    func_menustyle_enter();
    while (func_cb.sta == FUNC_MENUSTYLE) {
        func_menustyle_process();
        func_menustyle_message(msg_dequeue());
    }
    func_menustyle_exit();
}
