#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if 1
#define WEEKS_LIST_CNT                       ((int)(sizeof(tbl_weeks_list) / sizeof(tbl_weeks_list[0])))

//组件ID
enum {
    //列表
    COMPO_ID_LISTBOX = 1,

    //按键
	COMPO_ID_BTN_REPETAT_NO,
	COMPO_ID_BTN_REPETAT_YES,

    //图像
	COMPO_ID_PIC_REPETAT_NO_CLICK,
	COMPO_ID_PIC_REPETAT_YES_CLICK,

};

typedef struct f_game_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;
} f_game_t;

static const compo_listbox_item_t tbl_weeks_list[] = {
    // {.res_addr = UI_BUF_GAME_GAME_BIRD_ICON_BIN, .str_idx = STR_GAME},
    // {.res_addr = UI_BUF_GAME_GAME_BIRD_ICON_BIN, .str_idx = STR_GAME},
    // {.res_addr = UI_BUF_GAME_GAME_BIRD_ICON_BIN, .str_idx = STR_GAME},
    // {.res_addr = UI_BUF_GAME_GAME_BIRD_ICON_BIN, .str_idx = STR_GAME},
    // {.res_addr = UI_BUF_GAME_GAME_BIRD_ICON_BIN, .str_idx = STR_GAME},
    // {.res_addr = UI_BUF_GAME_GAME_BIRD_ICON_BIN, .str_idx = STR_GAME},
    // {.res_addr = UI_BUF_GAME_GAME_BIRD_ICON_BIN, .str_idx = STR_GAME},
};

//创建闹钟--重复窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_game_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    //新建列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_MENU_CIRCLE);//COMPO_LISTBOX_STYLE_TITLE_NORMAL
    
    compo_listbox_set_rect_size(listbox, GUI_SCREEN_WIDTH, 110, 60);
    compo_listbox_set_rect_color(listbox, make_color(29, 29, 29));
    compo_listbox_set_rect_visible(listbox, true);
    compo_listbox_set(listbox, tbl_weeks_list, WEEKS_LIST_CNT);
    compo_setid(listbox, COMPO_ID_LISTBOX);
    compo_listbox_set_bithook(listbox, bsp_sys_get_ctlbit);
    compo_listbox_set_focus(listbox, 127);

    compo_listbox_update(listbox);
    return frm;
}

////触摸按钮效果处理
//static void func_game_button_touch_handle(void)
//{
//
//}

//释放按钮效果处理
static void func_game_button_release_handle(void)
{

}

//单击按钮
static void func_game_button_click(void)
{
    int id = compo_get_button_id();

    f_game_t *f_aclock = (f_game_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_aclock->listbox;
    printf("===>id [%d]\n", id);

        int aclock_sel = compo_listbox_select(listbox, ctp_get_sxy());
        if (aclock_sel >= 0) {
           printf("===>aclock_sel:%d\n",aclock_sel);
           func_switch_to(FUNC_BIRD, FUNC_SWITCH_DIRECT);
        }
}

static void func_game_process(void)
{
    f_game_t *f_aclock = (f_game_t *)func_cb.f_cb;
    compo_listbox_move(f_aclock->listbox);
    func_process();
}


static void func_game_message(size_msg_t msg)
{
    f_game_t *f_aclock = (f_game_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_aclock->listbox;

    if (compo_listbox_message(listbox, msg)) {
        func_game_button_release_handle();
        return;                                         //处理列表框信息
    }
    switch (msg) {
    case MSG_CTP_TOUCH:
        // func_game_button_touch_handle();
        break;

	case MSG_CTP_CLICK:
        func_game_button_click();
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_LONG:
        // func_game_button_release_handle();
        break;

    case MSG_CTP_SHORT_RIGHT:
        // func_game_button_release_handle();
        func_message(msg);
        break;



    default:
        func_message(msg);
        break;
    }
}

void func_game_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_game_t));
    func_cb.frm_main = func_game_form_create();

    f_game_t *f_aclock = (f_game_t *)func_cb.f_cb;
    f_aclock->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_aclock->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_aclock->mcb;        //建立移动控制块，退出时需要释放

    // s32 first_y = compo_listbox_gety_byidx(listbox, 2);
    s32 last_y = compo_listbox_gety_byidx(listbox, WEEKS_LIST_CNT - 2);
    compo_listbox_move_init_modify(listbox, listbox->ofs_y, last_y);

    func_cb.enter_tick = tick_get();

}

void func_game_exit(void)
{
    func_cb.last = FUNC_GAME;
}

void func_game(void)
{
    printf("%s\n", __func__);
    func_game_enter();
    while (func_cb.sta == FUNC_GAME) {
        func_game_process();
        func_game_message(msg_dequeue());
    }
    func_game_exit();
}

#endif

#if 0

#define GAME_NUM                    3

typedef struct f_game_t_ {
    compo_button_t* rect[GAME_NUM];
    compo_picturebox_t * pic[GAME_NUM];
    compo_textbox_t* text[GAME_NUM];

    bool drag_flag;
    s32 ofs_y;
    s32 focus_y;
    bool                flag_move_auto;
    s16                 focus_icon_idx;
    int                 moveto_idx;
    s32                 moveto_y;
    int                 line_height;
    uint32_t            tick;
    int                 line_center_y;
} f_game_t;

typedef struct {
    char name[20];
    u32 res_addr;
    bool cutflag;   //图片资源是否需要裁剪
} Style;

enum {
    GAME_ID_BTN_ICON_1 = 1,
    GAME_ID_BTN_ICON_2,
};



static Style game[GAME_NUM] = {
    {"飞扬的小鸟", UI_BUF_GAME_GAME_BIRD_BIN, 1},
#if FUNC_GAME_TETRIS_EN
    {"俄罗斯方块", UI_BUF_TETRIS_16_1_BIN, 0},
#endif // FUNC_GAME_TETRIS_EN
};

//创建海拔窗体
compo_form_t *func_game_form_create(void)
{
    f_game_t *f_game = (f_game_t *)func_cb.f_cb;
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);
	compo_button_t *btn;
    compo_textbox_t *txt;
    compo_picturebox_t * pic;

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, "游戏");

    for(int i=0;i<GAME_NUM;i++)
    {
        //矩形框
        btn = compo_create(frm, COMPO_TYPE_BUTTON);
        widget_icon_t *img_btn = widget_icon_create(frm->page_body, UI_BUF_COMMON_BG_BIN);
        btn->widget = img_btn;
        compo_setid(btn, GAME_ID_BTN_ICON_1 + i);
        compo_button_set_location(btn, 150, 100 + i*72, 280, 60);

        //文本
        txt = compo_textbox_create(frm, 15);
        compo_textbox_set_location(txt, 150, 100 + i*72, 180, 35);
        compo_textbox_set(txt, game[i].name);
        compo_textbox_set_visible(txt, 1);

        //图标
        pic = compo_picturebox_create(frm, game[i].res_addr);
        if(game[i].cutflag) {
            compo_picturebox_cut(pic, 0, 3);
        }
        compo_picturebox_set_pos(pic, 50, 100 + i*72);

        if(func_cb.sta == FUNC_GAME)
        {
            f_game->rect[i] = btn;
            f_game->text[i] = txt;
            f_game->pic[i]  = pic;
        }

    }

    return frm;
}

//海拔功能事件处理
static void func_game_process(void)
{
    f_game_t *f_game = (f_game_t *)func_cb.f_cb;
    f_game->line_height = 40;
    f_game->line_center_y = f_game->line_height / 2;
    if (f_game->drag_flag)
    {
        s32 dx, dy;

        f_game->drag_flag = ctp_get_dxy(&dx, &dy);

        if (f_game->drag_flag)
        {
            f_game->ofs_y = f_game->focus_y - dy;
            int iy = -45 - f_game->ofs_y;
            widget_page_set_client(func_cb.frm_main->page_body, 0, iy);

            int kidx;

            kidx = f_game->ofs_y / f_game->line_height;
            kidx -= f_game->ofs_y < 0 ? 1 : 0;
            f_game->focus_icon_idx = kidx;
        }
        else
        {
            s32 last_dy = ctp_get_last_dxy().y;
            f_game->flag_move_auto = true;
            f_game->focus_y = f_game->ofs_y;
            f_game->moveto_idx = f_game->focus_icon_idx - (last_dy * 80 / 466);

            if (f_game->moveto_idx > GAME_NUM - 4)
            {
                f_game->moveto_idx = GAME_NUM - 4;
            }
            if (f_game->moveto_idx < 0)
            {
                f_game->moveto_idx = 0;
            }

            f_game->moveto_y = f_game->line_center_y + f_game->line_height * f_game->moveto_idx;
            f_game->tick = tick_get();
        }
    }
    if (f_game->flag_move_auto)
    {
        int AUTO_STEP = 10;
        if (f_game->focus_y == f_game->moveto_y)
        {
            f_game->flag_move_auto = false;
            f_game->focus_icon_idx = f_game->ofs_y / f_game->line_height - (f_game->ofs_y < 0 ? 1 : 0);
            f_game->focus_y = f_game->ofs_y;
        }
        else if (tick_check_expire(f_game->tick, 10))
        {
            s32 dy;
            f_game->tick = tick_get();

            dy = f_game->moveto_y - f_game->focus_y;

            if (dy > 0) {
                if (dy > AUTO_STEP * 16) {
                    dy = dy / 16;
                } else if (dy > AUTO_STEP) {
                    dy = AUTO_STEP;
                } else {
                    dy = 1;
                }
            }
            else {
                if (dy < -AUTO_STEP * 16) {
                    dy = dy / 16;
                } else if (dy < -AUTO_STEP) {
                    dy = -AUTO_STEP;
                } else {
                    dy = -1;
                }
            }
            f_game->focus_y += dy;

            f_game->ofs_y = f_game->focus_y - dy;
            int iy = -45 - f_game->ofs_y;
            widget_page_set_client(func_cb.frm_main->page_body, 0, iy);
        }
    }
    func_process();
}

static void func_game_click(void)
{
    int id = compo_get_button_id();

    if(id == GAME_ID_BTN_ICON_1)
    {
        func_switch_to(FUNC_BIRD, FUNC_SWITCH_DIRECT);
    }

    else if(id == GAME_ID_BTN_ICON_2)
    {
#if FUNC_GAME_TETRIS_EN
        func_switch_to(FUNC_GAME_TETRIS_START, FUNC_SWITCH_DIRECT);
#endif // FUNC_GAME_TETRIS_EN
    }
}

//海拔功能消息处理
static void func_game_message(size_msg_t msg)
{
    f_game_t *f_game = (f_game_t *)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_TOUCH:
        f_game->drag_flag = true;
        f_game->flag_move_auto = false;
        f_game->focus_icon_idx = f_game->ofs_y / f_game->line_height - (f_game->ofs_y < 0 ? 1 : 0);
        f_game->focus_y = f_game->ofs_y;
        break;
    case MSG_CTP_CLICK:
        func_game_click();
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
void func_game_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_game_t));
    func_cb.frm_main = func_game_form_create();
}

//退出海拔功能
void func_game_exit(void)
{
    func_cb.last = FUNC_GAME;
}

//海拔功能
void func_game(void)
{
    printf("%s\n", __func__);
    func_game_enter();
    while (func_cb.sta == FUNC_GAME) {
        func_game_process();
        func_game_message(msg_dequeue());
    }
    func_game_exit();
}

#endif
