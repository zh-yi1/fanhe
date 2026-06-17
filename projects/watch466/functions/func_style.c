#include "include.h"
#include "func.h"
#include "func_menu.h"

#define TRACE_EN    1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_style_t_ {

} f_style_t;

// char *style_str_list[MENU_STYLE_CNT] = {
//     "蜂窝",
//     "瀑布流",
//     "列表",
//     "足球",
//     "九宫格",
//     "九宫格(横向)",
//     "网格",
//     "圆盘",
//     "环形",
//     "万花筒",
//     "天圆地方",
//     "九宫格",
//     "网格",
//     "六边形",
//     "列表(四宫格)",
//     "网格蜂窝",
// };

char *style_str_list[MENU_STYLE_CNT] = {
    "honeycomb",
    "waterfall",
    "list",
    "football",
    "Sudoku",
    "Sudoku(landscape)",
    "grid",
    "disk",
    "circular",
    "Kaleidoscope",
    "square",
    "Sudokus(scale)",
    "grid",
    "hexagon",
    "four list",
    "grid honeycomb",
};

//创建菜单风格窗体
compo_form_t *func_style_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    // compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    // compo_form_set_title_center(frm, true);
    // compo_form_set_title(frm, i18n[STR_STYLE]);

	// //创建按键
    // compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_ICON_MENU_BIN);
    // compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 190);

	// compo_textbox_t *txt_style = compo_textbox_create(frm, 30);
	// compo_setid(txt_style, 8);
    // compo_textbox_set_pos(txt_style, GUI_SCREEN_CENTER_X, 310);
    // compo_textbox_set(txt_style, style_str_list[func_cb.menu_style]);

    // func_cb.menu_idx = 0;           //切换风格后进入回中心位置

    return frm;
}

//菜单风格功能事件处理
static void func_style_process(void)
{
    func_process();
}

//切换到设置菜单页面
static void func_set_sub_list_switch_to_menu(void)
{
    u8 func_sta = FUNC_MENU;
    compo_form_t *frm = func_create_form(func_sta);
    func_switching(FUNC_SWITCH_ZOOM_FADE_EXIT /*| FUNC_SWITCH_AUTO*/, NULL);
    compo_form_destroy(frm);
    func_cb.sta = func_sta;
}

//菜单风格功能消息处理
static void func_style_message(size_msg_t msg)
{
    compo_textbox_t *txt_style = compo_getobj_byid(8);
    switch (msg) {
    case MSG_CTP_CLICK:
        func_cb.menu_style++;
        if (func_cb.menu_style >= MENU_STYLE_CNT) {
            func_cb.menu_style = MENU_STYLE_HONEYCOMB;
        }
        TRACE("set_menu_style:%d\n", func_cb.menu_style);
        compo_textbox_set(txt_style, style_str_list[func_cb.menu_style]);
        break;

    case MSG_CTP_LONG:
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_QDEC_FORWARD:
        func_cb.menu_style++;
        if (func_cb.menu_style >= MENU_STYLE_CNT) {
            func_cb.menu_style = MENU_STYLE_HONEYCOMB;
        }
        TRACE("set_menu_style:%d\n", func_cb.menu_style);
        compo_textbox_set(txt_style, style_str_list[func_cb.menu_style]);
        break;

    case MSG_CTP_SHORT_DOWN:
    case MSG_QDEC_BACKWARD:
        if (func_cb.menu_style == 0) {
            func_cb.menu_style = MENU_STYLE_CNT - 1;
        } else {
            func_cb.menu_style--;
        }
        TRACE("set_menu_style:%d\n", func_cb.menu_style);
        compo_textbox_set(txt_style, style_str_list[func_cb.menu_style]);
        break;


    case MSG_CTP_SHORT_RIGHT:
        func_set_sub_list_switch_to_menu();
        break;


    default:
        func_message(msg);
        break;
    }
}

//进入菜单风格功能
void func_style_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_style_t));
    func_cb.frm_main = func_style_form_create();
}

//退出菜单风格功能
void func_style_exit(void)
{
    func_cb.last = FUNC_STYLE;
}

//菜单风格功能
void func_style(void)
{
    printf("%s\n", __func__);
    func_style_enter();
    while (func_cb.sta == FUNC_STYLE) {
        func_style_process();
        func_style_message(msg_dequeue());
    }
    func_style_exit();
}
