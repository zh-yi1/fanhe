#include "include.h"
#include "func.h"

#if FUNC_GAME_TETRIS_EN

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define RESTART_X                       GUI_SCREEN_CENTER_X
#define RESTART_Y                       (GUI_SCREEN_CENTER_Y + 40)
#define RESTART_WID                     170
#define RESTART_HEI                     50

#define RETURN_X                        GUI_SCREEN_CENTER_X
#define RETURN_Y                        (GUI_SCREEN_CENTER_Y + 120)
#define RETURN_WID                      170
#define RETURN_HEI                      50


typedef struct f_game_tetris_over_t_ {

} f_game_tetris_over_t;

//创建海拔窗体
compo_form_t *func_game_tetris_over_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);
    compo_cardbox_t *cardbox;
    char str_buff[25];
    compo_picturebox_t *pic;

    //背景图
    pic = compo_picturebox_create(frm, UI_BUF_TETRIS_BJ_BIN);
    widget_set_location(pic->img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 320, 385);

    //创建游戏结束背景
    pic = compo_picturebox_create(frm, UI_BUF_TETRIS_GAMEOVER_BIN);
    widget_set_location(pic->img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 320, 385);

    //结束界面当前得分
    cardbox = compo_cardbox_create(frm, 0, 2, 4, 250, 150);
    compo_cardbox_set_pos(cardbox, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y+80);
    sprintf(str_buff,"%ld",sys_cb.curr_score);
    compo_cardbox_text_set(cardbox, 0, str_buff);
    compo_cardbox_text_set_location(cardbox, 0, 0, 0, 250, 35);
    compo_cardbox_set_location(cardbox, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 20, 160, 85);
    compo_cardbox_text_set_forecolor(cardbox, 0, COLOR_RED);

    return frm;
}

//海拔功能事件处理
static void func_game_tetris_over_process(void)
{
    func_process();
}

//海拔功能消息处理
static void func_game_tetris_over_message(size_msg_t msg)
{
    point_t pt = ctp_get_sxy();;
    switch (msg) {
    case MSG_CTP_CLICK:
        if (abs_s(pt.x - RESTART_X) * 2 <= RESTART_WID && abs_s(pt.y - RESTART_Y) * 2 <= RESTART_HEI) {
            func_switch_to(FUNC_GAME_TETRIS, FUNC_SWITCH_DIRECT);
        } else if(abs_s(pt.x - RETURN_X) * 2 <= RETURN_WID && abs_s(pt.y - RETURN_Y) * 2 <= RETURN_HEI) {
            func_switch_to(FUNC_GAME_TETRIS_START, FUNC_SWITCH_DIRECT);
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

//进入海拔功能
void func_game_tetris_over_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_game_tetris_over_t));
    func_cb.frm_main = func_game_tetris_over_form_create();
}

//退出海拔功能
void func_game_tetris_over_exit(void)
{
    func_cb.last = FUNC_GAME_TETRIS_OVER;
}

//海拔功能
void func_game_tetris_over(void)
{
    printf("%s\n", __func__);
    func_game_tetris_over_enter();
    while (func_cb.sta == FUNC_GAME_TETRIS_OVER) {
        func_game_tetris_over_process();
        func_game_tetris_over_message(msg_dequeue());
    }
    func_game_tetris_over_exit();
}

#endif // GAME_TETRIS_EN
