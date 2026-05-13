#include "include.h"
#include "func.h"

#if FUNC_GAME_TETRIS_EN

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//开始按钮的坐标信息
#define START_X                           (GUI_SCREEN_CENTER_X)
#define START_Y                           (GUI_SCREEN_HEIGHT - 25)
#define START_WID                         200
#define START_HEI                         50

enum
{
    COMPO_ID_BTN_START = 1,
};

typedef struct f_game_tetris_start_t_ {

} f_tetris_start_t;

//创建窗体
compo_form_t *func_game_tetris_start_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);
    widget_image_t * image;
    compo_cardbox_t *cardbox;
    char str_buff[30];

    image = widget_image_create(frm->page_body, UI_BUF_TETRIS_BJ_BIN);
    widget_set_location(image, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 320, 385);
    widget_set_visible(image, true);


    //新建深黑半透背景
    compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, COLOR_BLACK);
    compo_shape_set_alpha(rect, 200);
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_radius(rect, 80);


    //最高纪录
    cardbox = compo_cardbox_create(frm, 0, 2, 4, 290, 150);
    sprintf(str_buff,"最高记录 %ld分",sys_cb.max_score);
    compo_cardbox_text_set(cardbox, 0, str_buff);
    compo_cardbox_text_set_location(cardbox, 0, 0, 0, 290, 35);
    compo_cardbox_set_location(cardbox, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y+95, 160, 85);

    //新建按钮
    compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_TETRIS_START_BTN_BIN);
    compo_setid(btn, COMPO_ID_BTN_START);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - 50);

    return frm;
}

//事件处理
static void func_game_tetris_start_process(void)
{
    func_process();
}

//消息处理
static void func_game_tetris_start_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        if (compo_get_button_id() == COMPO_ID_BTN_START) {
            func_switch_to(FUNC_GAME_TETRIS, FUNC_SWITCH_DIRECT);
        }
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

//进入功能
void func_game_tetris_start_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_tetris_start_t));
    func_cb.frm_main = func_game_tetris_start_form_create();
}

//退出功能
void func_game_tetris_start_exit(void)
{
    func_cb.last = FUNC_GAME_TETRIS_START;
}

//功能
void func_game_tetris_start(void)
{
    printf("%s\n", __func__);
    func_game_tetris_start_enter();
    while (func_cb.sta == FUNC_GAME_TETRIS_START) {
        func_game_tetris_start_process();
        func_game_tetris_start_message(msg_dequeue());
    }
    func_game_tetris_start_exit();
}
#endif // GAME_TETRIS_EN
