#include "include.h"
#include "func.h"
#include "func_menu.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//创建主菜单窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_menu_form_create(void)
{
    switch (func_cb.menu_style) {
    case MENU_STYLE_CUM_HONEYGRID:
    case MENU_STYLE_HONEYCOMB:
        return func_menu_sub_honeycomb_form_create();

    case MENU_STYLE_WATERFALL:
        return func_menu_sub_waterfall_form_create();

    case MENU_STYLE_FOOTBALL:
        return func_menu_sub_football_form_create();

    case MENU_STYLE_LIST:
    case MENU_STYLE_CUM_FOURGRID:
        return func_menu_sub_list_form_create();

    case MENU_STYLE_SUDOKU:
    case MENU_STYLE_SUDOKU_HRZ:
        return func_menu_sub_sudoku_form_create();

    case MENU_STYLE_GRID:
    case MENU_STYLE_CUM_GRID:
        return func_menu_sub_grid_form_create();

    case MENU_STYLE_DISK:
        return func_menu_sub_disk_form_create();

    case MENU_STYLE_RING:
        return func_menu_sub_ring_form_create();

    case MENU_STYLE_KALE:
        return func_menu_sub_kale_form_create();

    case MENU_STYLE_SKYRER:
        return func_menu_sub_skyrer_form_create();

    case MENU_STYLE_CUM_SUDOKU:
        return func_menu_sub_cum_sudoku_form_create();

    case MENU_STYLE_CUM_HEXAGON:
        return func_menu_sub_hexagon_form_create();

    default:
        halt(HALT_FUNC_MENU_SUBSTA);
        return NULL;
    }
}

////切换菜单样式
//static void func_menu_switch_style(void)
//{
//    func_cb.menu_idx = 0;
//    func_switching(FUNC_SWITCH_FADE_OUT | FUNC_SWITCH_AUTO, NULL);
//    func_cb.flag_animation = true;
//    switch (func_cb.menu_style) {
//    case MENU_STYLE_HONEYCOMB:
//        func_cb.menu_style = MENU_STYLE_WATERFALL;
//        break;
//
//    case MENU_STYLE_WATERFALL:
//        func_cb.menu_style = MENU_STYLE_SUDOKU;
//        break;
//
//    case MENU_STYLE_SUDOKU:
//        func_cb.menu_style = MENU_STYLE_LIST;
//        break;
//
//    case MENU_STYLE_LIST:
//        func_cb.menu_style = MENU_STYLE_SUDOKU_HRZ;
//        break;
//
//    case MENU_STYLE_SUDOKU_HRZ:
//        func_cb.menu_style = MENU_STYLE_GRID;
//        break;
//
//    case MENU_STYLE_GRID:
//        func_cb.menu_style = MENU_STYLE_DISK;
//        break;
//
//    case MENU_STYLE_DISK:
//        func_cb.menu_style = MENU_STYLE_RING;
//        break;
//
//    case MENU_STYLE_RING:
//        func_cb.menu_style = MENU_STYLE_KALE;
//        break;
//
//    case MENU_STYLE_KALE:
//        func_cb.menu_style = MENU_STYLE_SKYRER;
//        break;
//
//    case MENU_STYLE_SKYRER:
//        func_cb.menu_style = MENU_STYLE_CUM_SUDOKU;
//        break;
//
//    case MENU_STYLE_CUM_SUDOKU:
//        func_cb.menu_style = MENU_STYLE_CUM_GRID;
//        break;
//
//    case MENU_STYLE_CUM_GRID:
//        func_cb.menu_style = MENU_STYLE_CUM_HEXAGON;
//        break;
//
//    case MENU_STYLE_CUM_HEXAGON:
//        func_cb.menu_style = MENU_STYLE_CUM_FOURGRID;
//        break;
//
//    default:
//        func_cb.menu_style = MENU_STYLE_HONEYCOMB;
//        break;
//    }
//}

//菜单样式公用消息
void func_menu_sub_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case KU_BACK:
        //不响应
        break;

    case KD_BACK:
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_cb.sta = FUNC_MENUSTYLE;
            //func_menu_switch_style();
        }
        break;

    default:
        func_message(msg);
        break;
    }
}

//退出菜单样式
void func_menu_sub_exit(void)
{
    compo_form_destroy(func_cb.frm_main);
    func_free(func_cb.f_cb);
    func_cb.frm_main = NULL;
    func_cb.f_cb = NULL;
    tft_set_temode(1);		//2TE
}


//进入主菜单功能
void func_menu_enter(void)
{
    //func_cb.menu_style = MENU_STYLE_LIST;
}

//退出主菜单功能
void func_menu_exit(void)
{
    func_cb.last = FUNC_MENU;
}

//主菜单功能
void func_menu(void)
{
    printf("%s\n", __func__);
    func_menu_enter();
    while (func_cb.sta == FUNC_MENU) {
        switch (func_cb.menu_style) {
        case MENU_STYLE_CUM_HONEYGRID:
        case MENU_STYLE_HONEYCOMB:
            func_menu_sub_honeycomb();
            break;

        case MENU_STYLE_WATERFALL:
            func_menu_sub_waterfall();
            break;

        case MENU_STYLE_FOOTBALL:
            func_menu_sub_football();
            break;

        case MENU_STYLE_LIST:
        case MENU_STYLE_CUM_FOURGRID:
            func_menu_sub_list();
            break;

        case MENU_STYLE_SUDOKU:
        case MENU_STYLE_SUDOKU_HRZ:
            func_menu_sub_sudoku();
            break;

        case MENU_STYLE_GRID:
        case MENU_STYLE_CUM_GRID:
            func_menu_sub_grid();
            break;

        case MENU_STYLE_DISK:
            func_menu_sub_disk();
			break;

        case MENU_STYLE_RING:
            func_menu_sub_ring();
			break;

        case MENU_STYLE_KALE:
            func_menu_sub_kale();
            break;

        case MENU_STYLE_SKYRER:
            func_menu_sub_skyrer();
            break;

        case MENU_STYLE_CUM_SUDOKU:
            func_menu_sub_cum_sudoku();
            break;

        case MENU_STYLE_CUM_HEXAGON:
            func_menu_sub_hexagon();
            break;

        default:
            halt(HALT_FUNC_MENU_SUBSTA);
            break;
        }
    }
    func_menu_exit();
}
