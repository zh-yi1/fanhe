#include "include.h"
#include "func.h"

#if FUNC_GAME_TETRIS_EN

#define BULLET_NUM  10
#define ENEMY_NUM  5

static co_timer_t timer;

#define BLOCK_BEGIN_X                   (GUI_SCREEN_CENTER_X - 75)
#define BLOCK_BEGIN_Y                   (14)
#define LOWER_BEGIN_Y                   (14 + TETRIS_BLOCK_SIZE)
#define HIGHER_BEGIN_Y                  (14 - TETRIS_BLOCK_SIZE)

#define NEXT_BLOCK_BEGIN_X              (GUI_SCREEN_CENTER_X + 95)
#define NEXT_BLOCK_BEGIN_Y              (GUI_SCREEN_CENTER_Y - 32)

#define TETRIS_BLOCK_SIZE               21
#define TETRIS_NEXTBLOCK_SIZE           15

//边界
#define BOTTOM_Y                        295
#define LEFT_BORDER                     40
#define RIGHT_BORDER                    210
#define RIGHT_BORDER_ROTATE             230

//开始按钮的坐标信息
#define START_X                         (GUI_SCREEN_CENTER_X)
#define START_Y                         (GUI_SCREEN_HEIGHT - 70)
#define START_WID                       200
#define START_HEI                       50

#define CLOSE_X                         (GUI_SCREEN_CENTER_X + 115)
#define CLOSE_Y                         (GUI_SCREEN_CENTER_Y - 150)
#define CLOSE_WID                       65
#define CLOSE_HEI                       52

#define LEFT_X                           (GUI_SCREEN_CENTER_X - 90)
#define LEFT_Y                           (GUI_SCREEN_CENTER_Y + 170)
#define LEFT_WID                         65
#define LEFT_HEI                         52

#define DOWN_X                          (GUI_SCREEN_CENTER_X - 25)
#define DOWN_Y                          (GUI_SCREEN_CENTER_Y + 170)
#define DOWN_WID                        65
#define DOWN_HEI                        52

#define ROTATE_X                        (GUI_SCREEN_CENTER_X + 40)
#define ROTATE_Y                        (GUI_SCREEN_CENTER_Y + 170)
#define ROTATE_WID                      65
#define ROTATE_HEI                      52

#define RIGHT_X                         (GUI_SCREEN_CENTER_X + 110)
#define RIGHT_Y                         (GUI_SCREEN_CENTER_Y + 180)
#define RIGHT_WID                       65
#define RIGHT_HEI                       52

#define GAME_SCREEN__X                  (GUI_SCREEN_CENTER_X - 40)
#define GAME_SCREEN__Y                  (GUI_SCREEN_CENTER_Y - 40)
#define GAME_SCREEN__WID                (200)
#define GAME_SCREEN__HEI                (325)

#define EXIT_CANCEL_X                   (GUI_SCREEN_CENTER_X - 70)
#define EXIT_CANCEL_Y                   (GUI_SCREEN_CENTER_Y + 142)
#define EXIT_CANCEL_WID                 100
#define EXIT_CANCEL_HEI                 60

#define EXIT_RIGHT_X                    (GUI_SCREEN_CENTER_X + 80)
#define EXIT_RIGHT_Y                    (GUI_SCREEN_CENTER_Y + 142)
#define EXIT_RIGHT_WID                  100
#define EXIT_RIGHT_HEI                  60

enum
{
    COMPO_ID_GAME_FRM_PIC = 1,
    COMPO_ID_BACKGROUND_PIC,
    COMPO_ID_BACKGROUND_RUN_PIC,
    COMPO_ID_PIC_GAMEOVER,
    COMPO_ID_PIC_GAMEEXIT,
    COMPO_ID_GAME_SUSPEND_BUTTON,
    COMPO_ID_GAME_START_BUTTON,
    COMPO_ID_CARDBOX_TOTAL_SCORE,
    COMPO_ID_CARDBOX_LINE_SCORE,
    COMPO_ID_CARDBOX_MAXSCORE_SCORE,
    COMPO_ID_CARDBOX_GAMEOVER_SCORE,
};
enum
{
    GAME_STATUS_STOP,
    GAME_STATUS_WAIT_RUN,
    GAME_STATUS_RUNING,
    GAME_STATUS_GAMEOVER,
    GAME_STATUS_FINISH,
    GAME_STATUS_EXIT,
    GAME_STATUS_RESULT_WAIT,
};
//方块颜色
enum
{
    TETRIS_COLOR_YELLOW,
    TETRIS_COLOR_ORANGE,
    TETRIS_COLOR_RED,
    TETRIS_COLOR_PURPLE,
    TETRIS_COLOR_GREEN,
    TETRIS_COLOR_BLUE,
};
//方块形状
enum
{
    TETRIS_SHAPE_Z,
    TETRIS_SHAPE_S,
    TETRIS_SHAPE_L,
    TETRIS_SHAPE_J,
    TETRIS_SHAPE_I,
    TETRIS_SHAPE_T,
    TETRIS_SHAPE_O,
};

#define TEST_ELEMENT        0           //测试地图全占满方块需要多大内存

typedef struct game_object_t {
    s16                 x;
    s16                 y;
} oneblock_t;


typedef struct f_tetris_t_ {
    u8                   random_color;
    u8                   random_shape;
    u8                   random_status;
    u8                   cur_random_shape;
    u8                   cur_random_status;
    widget_image_t*      cur_image_block[4];            //下落的方块
    widget_image_t*      next_image_block[4];           //显示的下一方块
    widget_image_t*      recovery_block[300];           //方块回收数组
    oneblock_t           block[4];
    oneblock_t           nextblock[4];
    u32                  nextblock_resaddr;
    u16                  pos_x;
    u16                  pos_y;
    u16                  rotate_x;
    u16                  rotate_y;
    u32                  downtime;                      //方块下落时间tick
    u16                  downtime_interval;             //方块下落速度
    u8                   status;                        //游戏状态
    bool                 isbottom;                      //方块是否到达底部或者是否与其他方块重叠标志位
    bool                 isstart;                       //游戏进行标志位
    int                  map[15][10];                   //地图数组
    widget_image_t*      line_block_address[15][10];    //存放已存在方块的数组
    int                  next_free_index;               //方块回收index，表示回收数组里存放了几个方块待使用
    bool                 canMove;                       //移动标志位
    u16                  total_score;                   //总分
    u8                   clear_line_count_once;         //一次性消了多少行，不同行分数不同
    u16                  clear_line_score_once;         //每次下落方块或者消行得分
    bool                 isinto_timer;                  //是否进入定时器标志位
    bool                 isfail_clear;                  //失败清除标志位
    bool                 isexit;                        //是否处于退出界面
    bool                 issuspend;                     //是否按下暂停游戏键
    s16 reverse_x[4];                                   //方块下落y方向改变，实际对应到地图坐标轴是x在改变
    s16 reverse_y[4];
    int map_x;                                          //方块x改变对应到map地图
    int map_y;
} f_tetris_t;

//一维是方块种类，二维是方块旋转角度，三维是一个方块有四个小块 ，四维是每个方块的x，y坐标, 第四维中的第三个成员只是为了让方块产生位置不要超出地图第一列，需要具体调试,第四、五个成员是为了让nextblock的x、y保持在方框内
static int tetris_offsets[7][4][4][5] = {
    // TETRIS_SHAPE_Z
    {
        {{0, 0, -1, 1, -1}, {1, 0, -1, 1, -1}, {1, 1, -1, 1, -1}, {2, 1, -1, 1, -1}},
        {{1, 0, -2, 7, -9}, {0, 1, -2, 7, -9}, {1, 1, -2, 7, -9}, {0, 2, -2, 7, -9}},
        {{0, 0, -1, 1, -1}, {1, 0, -1, 1, -1}, {1, 1, -1, 1, -1}, {2, 1, -1, 1, -1}},
        {{1, 0, -2, 7, -9}, {0, 1, -2, 7, -9}, {1, 1, -2, 7, -9}, {0, 2, -2, 7, -9}},
    },
    // TETRIS_SHAPE_S
    {
        {{1, 0, -1, 0, -2}, {2, 0, -1, 0, -2}, {0, 1, -1, 0, -2}, {1, 1, -1, 0, -2}},
        {{0, 0, -2, 9, -10}, {0, 1, -2, 9, -10}, {1, 1, -2, 9, -10}, {1, 2, -2, 9, -10}},
        {{1, 0, -1, 0, -2}, {2, 0, -1, 0, -2}, {0, 1, -1, 0, -2}, {1, 1, -1, 0, -2}},
        {{0, 0, -2, 7, -8}, {0, 1, -2, 7, -8}, {1, 1, -2, 7, -8}, {1, 2, -2, 7, -8}},
    },
    // TETRIS_SHAPE_L
    {
        {{0, 0, -2, 9, -10}, {0, 1, -2, 9, -10}, {0, 2, -2, 9, -10}, {1, 2, -2, 9, -10}},
        {{0, 0, -1, 0, 0}, {1, 0, -1, 0, 0}, {2, 0, -1, 0, 0}, {0, 1, -1, 0, 0}},
        {{0, 0, -2, 5, -9}, {1, 0, -2, 5, -9}, {1, 1, -2, 5, -9}, {1, 2, -2, 5, -9}},
        {{2, 0, -1, 0, -5}, {0, 1, -1, 0, -5}, {1, 1, -1, 0, -5}, {2, 1, -1, 0, -5}}
    },
    // TETRIS_SHAPE_J
    {
        {{1, 0, -2, 5, -10}, {1, 1, -2, 5, -10}, {0, 2, -2, 5, -10}, {1, 2, -2, 5, -10}},
        {{0, 0, -1, 2, -2}, {0, 1, -1, 2, -2}, {1, 1, -1, 2, -2}, {2, 1, -1, 2, -2}},
        {{0, 0, -2, 9, -8}, {1, 0, -2, 9, -8}, {0, 1, -2, 9, -8}, {0, 2, -2, 9, -8}},
        {{0, 0, -1, 0, 0}, {1, 0, -1, 0, 0}, {2, 0, -1, 0, 0}, {2, 1, -1, 0, 0}}
    },
    // TETRIS_SHAPE_I
    {
        {{0, 1, -1, -4, -9}, {1, 1, -1, -4, -9}, {2, 1, -1, -4, -9}, {3, 1, -1, -4, -9}},
        {{0, 0, -3, 15, -19}, {0, 1, -3, 15, -19}, {0, 2, -3, 15, -19}, {0, 3, -3, 15, -19}},
        {{0, 1, -1, -4, -9}, {1, 1, -1, -4, -9}, {2, 1, -1, -4, -9}, {3, 1, -1, -4, -9}},
        {{0, 0, -3, 15, -19}, {0, 1, -3, 15, -19}, {0, 2, -3, 15, -19}, {0, 3, -3, 15, -19}},
    },
    // TETRIS_SHAPE_T
    {
        {{0, 0, -1, 0, 0}, {1, 0, -1, 0, 0}, {2, 0, -1, 0, 0}, {1, 1, -1, 0, 0}},
        {{1, 0, -2, 6, -10}, {0, 1, -2, 6, -10}, {1, 1, -2, 6, -10}, {1, 2, -2, 6, -10}},
        {{1, 0, -1, 0, -4}, {0, 1, -1, 0, -4}, {1, 1, -1, 0, -4}, {2, 1, -1, 0, -4}},
        {{0, 0, -2, 11, -10}, {0, 1, -2, 11, -10}, {1, 1, -2, 11, -10}, {0, 2, -2, 11, -10}}
    },
    // TETRIS_SHAPE_O
    {
        {{0, 0, -1, 8, -2}, {1, 0, -1, 8, -2}, {0, 1, -1, 8, -2}, {1, 1, -1, 8, -2}},
        {{0, 0, -1, 8, -2}, {1, 0, -1, 8, -2}, {0, 1, -1, 8, -2}, {1, 1, -1, 8, -2}},
        {{0, 0, -1, 8, -2}, {1, 0, -1, 8, -2}, {0, 1, -1, 8, -2}, {1, 1, -1, 8, -2}},
        {{0, 0, -1, 8, -2}, {1, 0, -1, 8, -2}, {0, 1, -1, 8, -2}, {1, 1, -1, 8, -2}},
    },

};

void block_pos_init(f_tetris_t *f_tetris)
{
    u8 shape_index = f_tetris->random_shape;
    u8 status_index = f_tetris->random_status;

    for (int i = 0; i < 4; i++)
    {
        f_tetris->block[i].x = BLOCK_BEGIN_X + tetris_offsets[shape_index][status_index][i][0] * TETRIS_BLOCK_SIZE;
        f_tetris->block[i].y = BLOCK_BEGIN_Y + tetris_offsets[shape_index][status_index][i][1] * TETRIS_BLOCK_SIZE + tetris_offsets[shape_index][status_index][i][2] * TETRIS_BLOCK_SIZE ;
    }//

    for (int i = 0; i < 4; ++i)
    {
        widget_set_location(f_tetris->cur_image_block[i], f_tetris->block[i].x, f_tetris->block[i].y, TETRIS_BLOCK_SIZE, TETRIS_BLOCK_SIZE);
    }
}

void block_pos_rotate_update(f_tetris_t *f_tetris)
{
    u8 shape_index = f_tetris->cur_random_shape;
    u8 status_index = f_tetris->cur_random_status;

    // 计算当前状态中最大x值
    int max_x_offset = 0;
    for (int i = 0; i < 4; i++)
    {
        if (tetris_offsets[shape_index][status_index][i][0] > max_x_offset) {
            max_x_offset = tetris_offsets[shape_index][status_index][i][0];
        }
    }

    // 检查是否可以旋转
    bool is_rotate_allow = true;
    int required_space = TETRIS_BLOCK_SIZE * max_x_offset;
    if (f_tetris->rotate_x + required_space > RIGHT_BORDER_ROTATE) {
        is_rotate_allow = false;
    }

    if (is_rotate_allow)
    {
        for (int i = 0; i < 4; i++)
        {
            f_tetris->block[i].x = f_tetris->rotate_x + tetris_offsets[shape_index][status_index][i][0] * TETRIS_BLOCK_SIZE;
            f_tetris->block[i].y = f_tetris->rotate_y + tetris_offsets[shape_index][status_index][i][1] * TETRIS_BLOCK_SIZE;
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        widget_set_location(f_tetris->cur_image_block[i], f_tetris->block[i].x, f_tetris->block[i].y, TETRIS_BLOCK_SIZE, TETRIS_BLOCK_SIZE);
    }
}

void next_block_pos_init(f_tetris_t *f_tetris)
{
    int shape_index = f_tetris->random_shape;
    int status_index = f_tetris->random_status;

    for (int i = 0; i < 4; i++)
    {
        f_tetris->nextblock[i].x = NEXT_BLOCK_BEGIN_X + tetris_offsets[shape_index][status_index][i][0] * TETRIS_NEXTBLOCK_SIZE + tetris_offsets[shape_index][status_index][i][3];
        f_tetris->nextblock[i].y = NEXT_BLOCK_BEGIN_Y + tetris_offsets[shape_index][status_index][i][1] * TETRIS_NEXTBLOCK_SIZE + tetris_offsets[shape_index][status_index][i][4];
    }

    for (int i = 0; i < 4; ++i)
    {
        widget_set_location(f_tetris->next_image_block[i], f_tetris->nextblock[i].x, f_tetris->nextblock[i].y, TETRIS_NEXTBLOCK_SIZE, TETRIS_NEXTBLOCK_SIZE);
    }
}


void block_pos_update(f_tetris_t *f_tetris)
{
    for(int i=0;i<4;i++)
    {
        f_tetris->block[i].x += f_tetris->pos_x;
        f_tetris->block[i].y += f_tetris->pos_y;
        f_tetris->reverse_x[i] = f_tetris->block[i].y;
        f_tetris->reverse_y[i] = f_tetris->block[i].x;

    }
    f_tetris->pos_x = 0;
    f_tetris->pos_y = 0;
}

void block_update(f_tetris_t *f_tetris)
{
    widget_set_location(f_tetris->cur_image_block[0], f_tetris->block[0].x, f_tetris->block[0].y, TETRIS_BLOCK_SIZE, TETRIS_BLOCK_SIZE);
    widget_set_location(f_tetris->cur_image_block[1], f_tetris->block[1].x, f_tetris->block[1].y, TETRIS_BLOCK_SIZE, TETRIS_BLOCK_SIZE);
    widget_set_location(f_tetris->cur_image_block[2], f_tetris->block[2].x, f_tetris->block[2].y, TETRIS_BLOCK_SIZE, TETRIS_BLOCK_SIZE);
    widget_set_location(f_tetris->cur_image_block[3], f_tetris->block[3].x, f_tetris->block[3].y, TETRIS_BLOCK_SIZE, TETRIS_BLOCK_SIZE);
}

int remove_first_four_elements(f_tetris_t *f_tetris)
{
    if (f_tetris->next_free_index < 4) {
        return -1;
    }

    for (int i = 0; i < f_tetris->next_free_index - 4; i++) {
        f_tetris->recovery_block[i] = f_tetris->recovery_block[i + 4];
    }

    for (int i = f_tetris->next_free_index - 4; i < f_tetris->next_free_index; i++) {
        f_tetris->recovery_block[i] = NULL;
    }
    f_tetris->next_free_index -= 4;
    return 0;
}
void display_nextblock(f_tetris_t *f_tetris)
{
    f_tetris->random_color = get_random(6) ;
    f_tetris->random_shape = get_random(7) ;
    f_tetris->random_status = get_random(4) ;

    //产生特定的方块，用于调试
    // f_tetris->random_shape = TETRIS_SHAPE_J;
    // f_tetris->random_status = 0;


    if (f_tetris->random_color == TETRIS_COLOR_YELLOW) {
        f_tetris->nextblock_resaddr = UI_BUF_TETRIS_FALL_1_BIN;
    } else if (f_tetris->random_color == TETRIS_COLOR_ORANGE) {
        f_tetris->nextblock_resaddr = UI_BUF_TETRIS_FALL_2_BIN;
    } else if (f_tetris->random_color == TETRIS_COLOR_RED) {
        f_tetris->nextblock_resaddr = UI_BUF_TETRIS_FALL_3_BIN;
    } else if (f_tetris->random_color == TETRIS_COLOR_PURPLE) {
        f_tetris->nextblock_resaddr = UI_BUF_TETRIS_FALL_4_BIN;
    } else if (f_tetris->random_color == TETRIS_COLOR_GREEN) {
        f_tetris->nextblock_resaddr = UI_BUF_TETRIS_FALL_5_BIN;
    } else if (f_tetris->random_color == TETRIS_COLOR_BLUE) {
        f_tetris->nextblock_resaddr = UI_BUF_TETRIS_FALL_6_BIN;
    }

    for (int i = 0; i < 4; i++)
    {
        widget_image_set(f_tetris->next_image_block[i], f_tetris->nextblock_resaddr);
    }


    next_block_pos_init(f_tetris);
}

void create_start_block(compo_form_t *frm, f_tetris_t *f_tetris)
{
    f_tetris->cur_random_shape = f_tetris->random_shape;
    f_tetris->cur_random_status = f_tetris->random_status;
    u32 res_addr = 0;
    if (f_tetris->random_color == TETRIS_COLOR_YELLOW) {
        res_addr = UI_BUF_TETRIS_16_1_BIN;
    } else if (f_tetris->random_color == TETRIS_COLOR_ORANGE) {
        res_addr = UI_BUF_TETRIS_16_2_BIN;
    } else if (f_tetris->random_color == TETRIS_COLOR_RED) {
        res_addr = UI_BUF_TETRIS_16_3_BIN;
    } else if (f_tetris->random_color == TETRIS_COLOR_PURPLE) {
        res_addr = UI_BUF_TETRIS_16_4_BIN;
    } else if (f_tetris->random_color == TETRIS_COLOR_GREEN) {
        res_addr = UI_BUF_TETRIS_16_5_BIN;
    } else if (f_tetris->random_color == TETRIS_COLOR_BLUE) {
        res_addr = UI_BUF_TETRIS_16_6_BIN;
    }

    if(f_tetris->next_free_index <= 4)
    {
        for(int i=0;i<4;i++)
        {
            widget_image_t *img = widget_image_create(frm->page_body, res_addr);
            widget_set_visible(img, true);
            f_tetris->cur_image_block[i] = img;
            widget_set_visible(f_tetris->cur_image_block[i], true);
        }
    }
    else
    {
        for (int i = 0; i < 4; i++)
        {
            widget_image_set(f_tetris->recovery_block[i], res_addr);
            f_tetris->cur_image_block[i] = f_tetris->recovery_block[i];
        }
        remove_first_four_elements(f_tetris);
    }


    f_tetris->downtime_interval = 1000;
    f_tetris->rotate_x = BLOCK_BEGIN_X;
    f_tetris->rotate_y = BLOCK_BEGIN_Y;

    f_tetris->isstart = 1;

    block_pos_init(f_tetris);
}

compo_form_t *func_game_tetris_form_create(void)
{
    f_tetris_t *f_tetris = (f_tetris_t *)func_cb.f_cb;
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);
    compo_setid(frm, COMPO_ID_GAME_FRM_PIC);
    component_t *compo;
    compo_cardbox_t *cardbox;
    char str_buff[30];
    compo_picturebox_t *pic;

#if !TEST_ELEMENT
    //背景图
    pic = compo_picturebox_create(frm, UI_BUF_TETRIS_BJ_BIN);
    compo_setid(pic, COMPO_ID_BACKGROUND_RUN_PIC);
    widget_set_location(pic->img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 320, 385);

    //suspend button
    compo = (component_t *)compo_button_create_by_image(frm, UI_BUF_TETRIS__S_SUSPEND_BIN);
    compo_setid(compo, COMPO_ID_GAME_SUSPEND_BUTTON);
    compo_button_set_pos((compo_button_t *)compo, GUI_SCREEN_CENTER_X + 82, GUI_SCREEN_CENTER_Y - 80);
    compo_button_set_visible((compo_button_t *)compo, false);

    //START button
    compo = (component_t *)compo_button_create_by_image(frm, UI_BUF_TETRIS__S_START_BIN);
    compo_setid(compo, COMPO_ID_GAME_START_BUTTON);
    compo_button_set_pos((compo_button_t *)compo, GUI_SCREEN_CENTER_X + 82, GUI_SCREEN_CENTER_Y - 80);
    compo_button_set_visible((compo_button_t *)compo, false);

    if(func_cb.sta == FUNC_GAME_TETRIS) {
        f_tetris->random_color = get_random(6) ;   //0 - 5
        f_tetris->random_shape = get_random(7) ;   //0 - 6
        f_tetris->random_status = get_random(4) ;   //0 - 3
        create_start_block(frm, f_tetris);
    }


    for(int i=0;i<4;i++)
    {
        if(func_cb.sta == FUNC_GAME_TETRIS) {
            widget_set_visible(f_tetris->cur_image_block[i], false);
            f_tetris->nextblock_resaddr = UI_BUF_TETRIS_FALL_1_BIN;
        }
        if(func_cb.sta == FUNC_GAME_TETRIS) {
            widget_image_t *img = widget_image_create(frm->page_body, f_tetris->nextblock_resaddr);
            widget_set_visible(img, false);
            f_tetris->next_image_block[i] = img;
        }
    }


    //总分数
    if(func_cb.sta == FUNC_GAME_TETRIS) {
        f_tetris->total_score = 0;
    }
    cardbox = compo_cardbox_create(frm, 0, 2, 4, 130, 30);
    compo_cardbox_set_pos(cardbox, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_setid(cardbox, COMPO_ID_CARDBOX_TOTAL_SCORE);
    if(func_cb.sta == FUNC_GAME_TETRIS) {
        sprintf(str_buff,"%d",f_tetris->total_score);
    } else {
        sprintf(str_buff," ");
    }
    compo_cardbox_text_set(cardbox, 0, str_buff);
    compo_cardbox_text_set_location(cardbox, 0, 0, 0, 130, 34);
    compo_cardbox_set_location(cardbox, GUI_SCREEN_CENTER_X+110, GUI_SCREEN_CENTER_Y+42, 84, 15);
    compo_cardbox_set_visible(cardbox,1);

    //消行加分
    if(func_cb.sta == FUNC_GAME_TETRIS) {
        f_tetris->clear_line_score_once = 0;
    }
    cardbox = compo_cardbox_create(frm, 0, 2, 4, 130, 30);
    compo_cardbox_set_pos(cardbox, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_setid(cardbox, COMPO_ID_CARDBOX_LINE_SCORE);
    if(func_cb.sta == FUNC_GAME_TETRIS) {
        sprintf(str_buff,"+%d",f_tetris->clear_line_score_once);
    } else {
        sprintf(str_buff," ");
    }
    compo_cardbox_text_set(cardbox, 0, str_buff);
    compo_cardbox_text_set_location(cardbox, 0, 0, 0, 130, 34);
    compo_cardbox_set_location(cardbox, GUI_SCREEN_CENTER_X+107, GUI_SCREEN_CENTER_Y+100, 84, 15);
    compo_cardbox_set_visible(cardbox,1);

    //创建游戏结束背景
    pic = compo_picturebox_create(frm, UI_BUF_TETRIS_GAMEOVER_BIN);
    widget_set_location(pic->img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 320, 385);
    compo_picturebox_set_visible(pic, false);
    compo_setid(pic, COMPO_ID_PIC_GAMEOVER);

    //退出游戏背景
    pic = compo_picturebox_create(frm, UI_BUF_TETRIS_GAME_TETRIS_EXITNOTICE_BIN);
    widget_set_location(pic->img, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 320, 385);
    compo_picturebox_set_visible(pic, false);
    compo_setid(pic, COMPO_ID_PIC_GAMEEXIT);

    //最高记录
    cardbox = compo_cardbox_create(frm, 0, 2, 4, 290, 150);
    compo_setid(cardbox, COMPO_ID_CARDBOX_MAXSCORE_SCORE);
    sprintf(str_buff,"最高记录 %ld分",sys_cb.max_score);
    compo_cardbox_text_set(cardbox, 0, str_buff);
    compo_cardbox_text_set_location(cardbox, 0, 0, 0, 290, 35);
    compo_cardbox_set_location(cardbox, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y+95, 160, 85);
    compo_cardbox_set_visible(cardbox,0);

    //结束界面当前得分
    cardbox = compo_cardbox_create(frm, 0, 2, 4, 250, 150);
    compo_cardbox_set_pos(cardbox, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y+80);
    compo_setid(cardbox, COMPO_ID_CARDBOX_GAMEOVER_SCORE);
    if(func_cb.sta == FUNC_GAME_TETRIS) {
        sprintf(str_buff,"%d",f_tetris->total_score);
    } else {
        sprintf(str_buff," ");
    }
    compo_cardbox_text_set(cardbox, 0, str_buff);
    compo_cardbox_text_set_location(cardbox, 0, 0, 0, 250, 35);
    compo_cardbox_set_location(cardbox, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 20, 160, 85);
    widget_text_set_color(cardbox->text[0], COLOR_RED);
    compo_cardbox_set_visible(cardbox,0);
#else
    widget_image_t *img;
    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 10; j++) {
            img = widget_image_create(frm->page_body, UI_BUF_TETRIS_16_1_BIN);
            widget_set_pos(img,
                BLOCK_BEGIN_X-50 + TETRIS_BLOCK_SIZE * i,  // X坐标
                BLOCK_BEGIN_Y+100 + TETRIS_BLOCK_SIZE * j    // Y坐标，加上块大小的偏移
            );
        }
    }
#endif
    return frm;
}

static void score_calculate_process(co_timer_t *timer, void *param)
{
    compo_cardbox_t *compo_total_score = compo_getobj_byid(COMPO_ID_CARDBOX_TOTAL_SCORE);
    f_tetris_t *f_tetris = (f_tetris_t *)func_cb.f_cb;
    char str_buff[16];
    f_tetris->clear_line_count_once = 0;
    co_timer_enable(timer, 0);
    f_tetris->isinto_timer = 0;
    f_tetris->total_score += f_tetris->clear_line_score_once;
    if(sys_cb.max_score < f_tetris->total_score)
    {
        sys_cb.max_score = f_tetris->total_score;
    }
    sprintf(str_buff,"%d",f_tetris->total_score);
    compo_cardbox_text_set(compo_total_score, 0, str_buff);


}

static void func_tetris_process(void)
{
    char str_buff[30];
    f_tetris_t *f_tetris = (f_tetris_t *)func_cb.f_cb;
    compo_form_t *frm = compo_getobj_byid(COMPO_ID_GAME_FRM_PIC);
    compo_button_t *compo_suspend_button = compo_getobj_byid(COMPO_ID_GAME_SUSPEND_BUTTON);
    compo_button_t *compo_start_button = compo_getobj_byid(COMPO_ID_GAME_START_BUTTON);
    compo_cardbox_t *compo_total_score = compo_getobj_byid(COMPO_ID_CARDBOX_TOTAL_SCORE);
    compo_cardbox_t *compo_line_score = compo_getobj_byid(COMPO_ID_CARDBOX_LINE_SCORE);

    if(f_tetris->issuspend == 0)
    {
        compo_button_set_visible(compo_suspend_button, true);
        compo_button_set_location(compo_suspend_button, GUI_SCREEN_CENTER_X + 107, GUI_SCREEN_CENTER_Y - 100, 65, 52);
    }
    else
    {
        compo_button_set_location(compo_start_button, GUI_SCREEN_CENTER_X + 107, GUI_SCREEN_CENTER_Y - 100, 65, 52);
        compo_button_set_visible(compo_start_button, true);
    }

    if(f_tetris->isstart && f_tetris->issuspend == 0)
    {
    if (f_tetris->isbottom == 0) {
        if (tick_check_expire(f_tetris->downtime, f_tetris->downtime_interval)) {
            f_tetris->downtime = tick_get();

            f_tetris->canMove = 1;

            for (int i = 0; i < 4; i++)
            {
                f_tetris->reverse_x[i] = f_tetris->block[i].y;
                f_tetris->reverse_y[i] = f_tetris->block[i].x;
                printf("i = %d , reverse_x[i] = %d , reverse_y[i] = %d\n",i, f_tetris->reverse_x[i], f_tetris->reverse_y[i]);
            }
            for (int i = 0; i < 4; i++) {
                //这里|| 后面的不是固定的，随着屏幕大小不同而不同，需要实际调试
                if (f_tetris->block[i].y  > BOTTOM_Y || f_tetris->map[((f_tetris->reverse_x[i] + TETRIS_BLOCK_SIZE)/ TETRIS_BLOCK_SIZE)][((f_tetris->reverse_y[i] - TETRIS_BLOCK_SIZE) / TETRIS_BLOCK_SIZE)] == 1) {
                    f_tetris->canMove = 0;
                    break;
                }
            }

            if (f_tetris->canMove)
            {
                for (int i = 0; i < 4; i++)
                {
                    f_tetris->block[i].y += TETRIS_BLOCK_SIZE;
                }
                f_tetris->rotate_y += TETRIS_BLOCK_SIZE;
                block_update(f_tetris);
            }
            else
            {
                f_tetris->isbottom = 1;
                for (int i = 0; i < 4; i++) {
                    f_tetris->map_x = f_tetris->reverse_x[i] / TETRIS_BLOCK_SIZE;
                    f_tetris->map_y = f_tetris->reverse_y[i] / TETRIS_BLOCK_SIZE - 1;    //后面这个-1不是固定的，随着不同屏幕尺寸大小图像大小不一样会导致不同，需要实际调试
                        f_tetris->map[f_tetris->map_x][f_tetris->map_y] = 1;

                }
            }

            // 遍历二维数组
            for (int i = 0; i < 15; i++) {
                for (int j = 0; j < 10; j++) {
                    printf("%d ", f_tetris->map[i][j]);
                }
                printf("\n");
            }
            printf("\n");

            if(f_tetris->canMove == 0)
            {
                int chosenblock = 0;
                for (int i = 0; i < 15; i++) {
                    for (int j = 0; j < 10; j++) {
                        if (f_tetris->map[i][j] != 0 && f_tetris->line_block_address[i][j] == NULL)
                        {
                            f_tetris->line_block_address[i][j] = f_tetris->cur_image_block[chosenblock];
                            if(chosenblock <= 2) chosenblock++;
                        }
                    }
                }
                printf("\n");
            }

            //打印已存在方块的地址
            for (int i = 0; i < 15; i++) {
                for (int j = 0; j < 10; j++) {
                    if(f_tetris->line_block_address[i][j] != NULL) {
                        // printf("f_tetris->line_block_address[%d][%d] = %p\n", i,j, f_tetris->line_block_address[i][j]);
                    }

                }
            }


            sprintf(str_buff,"+0");
            compo_cardbox_text_set(compo_line_score, 0, str_buff);
        }

}
else
{
    for (int col = 0; col < 10; col++) {
        if (f_tetris->map[0][col] == 1) {
            printf("game over!\n");
            sys_cb.curr_score = f_tetris->total_score;
            func_switch_to(FUNC_GAME_TETRIS_OVER, FUNC_SWITCH_DIRECT);
            // compo_picturebox_set_visible(pic_gameover, true);
            // //游戏结束，页面切换，方块回收
            // sprintf(str_buff,"%d分",f_tetris->total_score);
            // compo_cardbox_text_set(compo_gameover_score, 0, str_buff);
            // compo_cardbox_set_visible(compo_gameover_score, 1);
            // if(f_tetris->isfail_clear == 0)
            // {
            //     f_tetris->isfail_clear = 1;
            //     for(int i=0;i<15;i++)
            //     {
            //         for(int j=0;j<10;j++)
            //         {
            //             if(f_tetris->line_block_address[i][j] != NULL)
            //             {
            //                 widget_set_visible(f_tetris->line_block_address[i][j], FALSE);
            //             }

            //         }
            //     }
            //     for(int i=0;i<4;i++)
            //     {
            //         widget_set_visible(f_tetris->cur_image_block[i], FALSE);
            //     }
            // }

            // if(sys_cb.max_score < f_tetris->total_score)
            // {
            //     sys_cb.max_score = f_tetris->total_score;
            // }
            // sprintf(str_buff,"最高纪录 %ld分",sys_cb.max_score);

            // f_tetris->status = GAME_STATUS_RESULT_WAIT;

            break;  //只要有一列里面的第0行有方块就直接结束
        }
    }
    if(f_tetris->status == GAME_STATUS_RUNING)
    {
        f_tetris->clear_line_score_once = 1;
        sprintf(str_buff,"+%d",f_tetris->clear_line_score_once);
        compo_cardbox_text_set(compo_line_score, 0, str_buff);
        f_tetris->total_score++;
        sprintf(str_buff,"%d",f_tetris->total_score);
        compo_cardbox_text_set(compo_total_score, 0, str_buff);

        // 到达底部产生新的block
        create_start_block(frm, f_tetris);
        f_tetris->isbottom = 0;

        display_nextblock(f_tetris);
    }
}

//消行检测之判断哪一行满了
for (int i = 14; i >= 0; i--)
{
    int allOnes = 1;
    for (int j = 0; j < 10; j++)
    {
        if (f_tetris->map[i][j] != 1)
        {
            allOnes = 0;
            break;
        }

    }
    if (allOnes)
    {
        f_tetris->clear_line_count_once++;
        for(int j=0;j<10;j++)
        {
            widget_set_pos(f_tetris->line_block_address[i][j], -150,-150);
            widget_set_visible(f_tetris->line_block_address[i][j], true);
            if (f_tetris->recovery_block[f_tetris->next_free_index] == NULL)
            {
                f_tetris->recovery_block[f_tetris->next_free_index] = f_tetris->line_block_address[i][j];
                f_tetris->next_free_index++;
            }
            f_tetris->line_block_address[i][j] = NULL;
        }

        int dropSteps[10] = {0};

        for(int t=i-1;t>=0;t--)
        {
            for(int y=0;y<10;y++)
            {
                if (f_tetris->line_block_address[t][y] != NULL)
                {
                    rect_t rect = widget_get_location(f_tetris->line_block_address[t][y]);
                    int fall_step = 0;

                    while (t + fall_step + 1 < 15 && f_tetris->line_block_address[t + fall_step + 1][y] == NULL) //t = 12
                    {
                        fall_step++; // 增加下落步数
                    }

                    // 只有当 step > 0 时，才进行下落
                    if (fall_step > 0)
                    {
                        widget_set_pos(f_tetris->line_block_address[t][y], rect.x, rect.y + TETRIS_BLOCK_SIZE);
                        f_tetris->line_block_address[t + 1][y] = f_tetris->line_block_address[t][y];
                        f_tetris->line_block_address[t][y] = NULL;
                        dropSteps[y] = MAX(dropSteps[y], fall_step);
                    }
                }
            }
        }


        for (int row = i; row > 0 ; row--) {
            for (int col = 0; col < 10; col++) {
                f_tetris->map[row][col] = f_tetris->map[row - 1][col];

            }
        }


        for (int j = 0; j < 10; j++) {
            f_tetris->map[0][j] = 0;
        }


    }
    //消行加分
    if(i == 0 && f_tetris->clear_line_count_once)
    {
        if(f_tetris->clear_line_count_once == 1)
            f_tetris->clear_line_score_once = 10;
        else if(f_tetris->clear_line_count_once == 2)
            f_tetris->clear_line_score_once = 20;
        else if(f_tetris->clear_line_count_once == 3)
            f_tetris->clear_line_score_once = 40;
        else if(f_tetris->clear_line_count_once == 4)
            f_tetris->clear_line_score_once = 80;

        sprintf(str_buff,"+%d",f_tetris->clear_line_score_once);
        compo_cardbox_text_set(compo_line_score, 0, str_buff);

        if(f_tetris->isinto_timer == 0)
        {
            f_tetris->isinto_timer = 1;
            co_timer_enable(&timer, 1);

        }

    }
}
}
}

//游戏过程状态机
static void func_game_tetris_process(void)
{
   f_tetris_t *f_tetris = (f_tetris_t *)func_cb.f_cb;

#if !TEST_ELEMENT
    if(GAME_STATUS_RUNING == f_tetris->status)
    {
        func_tetris_process();
    }
#endif

    func_process();

}

static void func_game_tetris_click(void)
{
    int btn_id = compo_get_button_id();
    f_tetris_t *f_tetris = (f_tetris_t *)func_cb.f_cb;
    compo_button_t *compo_suspend_button = compo_getobj_byid(COMPO_ID_GAME_SUSPEND_BUTTON);
    compo_button_t *compo_start_button = compo_getobj_byid(COMPO_ID_GAME_START_BUTTON);
    compo_cardbox_t *compo_total_score = compo_getobj_byid(COMPO_ID_CARDBOX_TOTAL_SCORE);
    compo_cardbox_t *compo_line_score = compo_getobj_byid(COMPO_ID_CARDBOX_LINE_SCORE);
    compo_cardbox_t *compo_maxscore = compo_getobj_byid(COMPO_ID_CARDBOX_MAXSCORE_SCORE);
    compo_cardbox_t *compo_gameover_score = compo_getobj_byid(COMPO_ID_CARDBOX_GAMEOVER_SCORE);
    compo_picturebox_t *pic_gameover = compo_getobj_byid(COMPO_ID_PIC_GAMEOVER);
    compo_picturebox_t *pic_gameexit = compo_getobj_byid(COMPO_ID_PIC_GAMEEXIT);

    point_t pt;
    pt = ctp_get_sxy();

   if (GAME_STATUS_WAIT_RUN == f_tetris->status) {
        if (abs_s(pt.x - START_X) * 2 <= START_WID && abs_s(pt.y - START_Y) * 2 <= START_HEI)
        {
            compo_cardbox_set_visible(compo_gameover_score, 0);

            f_tetris->status = GAME_STATUS_RUNING;
            f_tetris->isstart = 1;
            for(int i=0;i<4;i++)
            {
                widget_set_visible(f_tetris->cur_image_block[i], true);
                widget_set_visible(f_tetris->next_image_block[i], true);
            }
            compo_cardbox_set_visible(compo_total_score, 1);
            compo_cardbox_set_visible(compo_line_score, 1);
            compo_cardbox_set_visible(compo_maxscore, 0);

            f_tetris->downtime_interval = 1000;
        }
   }
   else if (GAME_STATUS_FINISH == f_tetris->status) {
        if (abs_s(pt.x - START_X) * 2 <= START_WID && abs_s(pt.y - START_Y) * 2 <= START_HEI)
        {
            compo_cardbox_set_visible(compo_gameover_score, 0);

            f_tetris->status = GAME_STATUS_RUNING;
            f_tetris->isstart = 1;


            for(int i=0;i<4;i++)
            {
                widget_set_visible(f_tetris->next_image_block[i], true);
            }
            compo_cardbox_set_visible(compo_total_score, 1);
            compo_cardbox_set_visible(compo_line_score, 1);
            compo_cardbox_set_visible(compo_maxscore, 0);

        }
   }
   else if(GAME_STATUS_RUNING == f_tetris->status)
   {
        if(COMPO_ID_GAME_SUSPEND_BUTTON == btn_id && f_tetris->isexit == 0)
        {
            f_tetris->isstart = 0;
            f_tetris->issuspend = 1;
            compo_button_set_visible(compo_suspend_button, true);
            compo_button_set_visible(compo_start_button, true);
            compo_button_set_pos(compo_suspend_button, -200, -200);
            compo_button_set_pos(compo_start_button, GUI_SCREEN_CENTER_X + 82, GUI_SCREEN_CENTER_Y - 80);
        }
        else if(COMPO_ID_GAME_START_BUTTON == btn_id && f_tetris->isexit == 0)
        {
            f_tetris->isstart = 1;
            f_tetris->issuspend = 0;
            compo_button_set_visible(compo_suspend_button, true);
            compo_button_set_visible(compo_start_button, true);
            compo_button_set_pos(compo_start_button, -200, -200);
            compo_button_set_pos(compo_suspend_button, GUI_SCREEN_CENTER_X + 82, GUI_SCREEN_CENTER_Y - 80);
        }
        else if (abs_s(pt.x - CLOSE_X) * 2 <= CLOSE_WID && abs_s(pt.y - CLOSE_Y) * 2 <= CLOSE_HEI)
        {
            compo_picturebox_set_visible(pic_gameexit, true);
            compo_picturebox_set_visible(pic_gameover, false);

            f_tetris->isstart = 0;
            f_tetris->isexit = 1;
            for(int i=0;i<15;i++)
            {
                for(int j=0;j<10;j++)
                {
                    if(f_tetris->line_block_address[i][j] != NULL)
                    {
                        widget_set_visible(f_tetris->line_block_address[i][j], false);
                    }

                }
            }
            for(int i=0;i<4;i++)
            {
                widget_set_visible(f_tetris->cur_image_block[i], false);
            }

        }
        else if (abs_s(pt.x - GAME_SCREEN__X) * 2 <= GAME_SCREEN__WID && abs_s(pt.y - GAME_SCREEN__Y) * 2 <= GAME_SCREEN__HEI)
        {
            // printf("GAME_SCREEN__X CLICKED!!!\n");

            if(f_tetris->isstart)
            {
                u8 shape_index = f_tetris->cur_random_shape;
                u8 status_index = f_tetris->cur_random_status;
                s16 temp_x[4],temp_y[4];

                int x_rotate[4],y_rotate[4];
                for (int i = 0; i < 4; i++)
                {
                    temp_x[i] = f_tetris->rotate_x + tetris_offsets[shape_index][((status_index+1)%3)][i][0] * TETRIS_BLOCK_SIZE;
                    temp_y[i] = f_tetris->rotate_y + tetris_offsets[shape_index][((status_index+1)%3)][i][1] * TETRIS_BLOCK_SIZE;

                    f_tetris->reverse_x[i] = temp_y[i];
                    f_tetris->reverse_y[i] = temp_x[i];

                    x_rotate[i] = f_tetris->reverse_x[i] / TETRIS_BLOCK_SIZE + 1;
                    y_rotate[i] = f_tetris->reverse_y[i] / TETRIS_BLOCK_SIZE - 1;
                    // printf("x_rotate[%d] = %d , y_rotate[%d] = %d\n",i,x_rotate[i],i,y_rotate[i]);
                    if(x_rotate[i] < 0) x_rotate[i] = 0;
                    if(y_rotate[i] < 0) y_rotate[i] = 0;
                }

                bool canMove = true;


                for (int i = 0; i < 4; i++)
                {
                    if ((f_tetris->map[x_rotate[i]][y_rotate[i]] == 1) || (f_tetris->rotate_x + TETRIS_BLOCK_SIZE < LEFT_BORDER) || (f_tetris->rotate_x + TETRIS_BLOCK_SIZE > RIGHT_BORDER) || f_tetris->block[i].y >= BOTTOM_Y - TETRIS_BLOCK_SIZE)
                    {
                        // printf("f_tetris->map[x_rotate[i]][y_rotate[i]] = %d)\n",f_tetris->map[x_rotate[i]][y_rotate[i]]);
                        // printf("f_tetris->rotate_x + TETRIS_BLOCK_SIZE = %d)\n",f_tetris->rotate_x + TETRIS_BLOCK_SIZE);
                        canMove = false;
                        break;
                    }
                }
                printf("canMove = %d\n",canMove);
                if (canMove)
                {

                    f_tetris->cur_random_status += 1;
                    if(f_tetris->cur_random_status > 3) {
                        f_tetris->cur_random_status = 0;
                    }
                    block_pos_rotate_update(f_tetris);
                }
            }
        }
        else if (abs_s(pt.x - EXIT_CANCEL_X) * 2 <= EXIT_CANCEL_WID && abs_s(pt.y - EXIT_CANCEL_Y) * 2 <= EXIT_CANCEL_HEI && f_tetris->isexit == 1)
        {
            f_tetris->isstart = 1;
            f_tetris->isexit = 0;
            compo_picturebox_set_visible(pic_gameexit, false);
            compo_picturebox_set_visible(pic_gameover, false);
            for(int i=0;i<15;i++)
            {
                for(int j=0;j<10;j++)
                {
                    if(f_tetris->line_block_address[i][j] != NULL)
                    {
                        widget_set_visible(f_tetris->line_block_address[i][j], true);
                    }

                }
            }
            for(int i=0;i<4;i++)
            {
                widget_set_visible(f_tetris->cur_image_block[i], true);
            }

        }
        else if (abs_s(pt.x - EXIT_RIGHT_X) * 2 <= EXIT_RIGHT_WID && abs_s(pt.y - EXIT_RIGHT_Y) * 2 <= EXIT_RIGHT_HEI && f_tetris->isexit == 1)
        {
            func_cb.sta = FUNC_GAME_TETRIS_START;
        }


   }
}

static void func_game_tetris_message(size_msg_t msg)
{
#if !TEST_ELEMENT
    f_tetris_t *f_tetris = (f_tetris_t *)func_cb.f_cb;
    int x_left[4],y_left[4];
    int x_right[4],y_right[4];
    int x_down[4],y_down[4];

    for (int i = 0; i < 4; i++)
    {
        f_tetris->reverse_x[i] = f_tetris->block[i].y;
        f_tetris->reverse_y[i] = f_tetris->block[i].x;

        x_left[i] = f_tetris->reverse_x[i] / TETRIS_BLOCK_SIZE;
        y_left[i] = f_tetris->reverse_y[i] / TETRIS_BLOCK_SIZE - 2 ;

        x_right[i] = f_tetris->reverse_x[i] / TETRIS_BLOCK_SIZE + 0;
        y_right[i] = f_tetris->reverse_y[i] / TETRIS_BLOCK_SIZE + 0;

        x_down[i] = f_tetris->reverse_x[i] / TETRIS_BLOCK_SIZE + 1;
        y_down[i] = f_tetris->reverse_y[i] / TETRIS_BLOCK_SIZE - 1;

        if(x_left[i] < 0) x_left[i] = 0;
        if(y_left[i] < 0) y_left[i] = 0;
        if(x_right[i] < 0) x_right[i] = 0;
        if(y_right[i] < 0) y_right[i] = 0;
        if(x_down[i] < 0) x_down[i] = 0;
        if(y_down[i] < 0) y_down[i] = 0;

    }

    switch (msg) {

    case MSG_CTP_TOUCH:

    break;

    case MSG_CTP_CLICK:
        func_game_tetris_click();
        break;

    case MSG_CTP_SHORT_DOWN:
        if(f_tetris->status == GAME_STATUS_RUNING)
        {
            if(f_tetris->isstart)
                {
                    bool canMove = true;


                    for (int i = 0; i < 4; i++)
                    {
                        if ((f_tetris->map[x_down[i]][y_down[i]] == 1))
                        {
                            canMove = false;
                            break;
                        }
                    }

                    if (canMove)
                    {
                        f_tetris->downtime_interval = 100;
                    }
                }

        }
        break;

    case MSG_CTP_SHORT_LEFT:
        if(f_tetris->status == GAME_STATUS_RUNING)
        {
            if (f_tetris->isstart)
            {
                bool canMove = true;

                for (int i = 0; i < 4; i++)
                {
                    if ((f_tetris->block[i].y > BOTTOM_Y + TETRIS_BLOCK_SIZE || f_tetris->block[i].x < LEFT_BORDER || f_tetris->map[x_left[i]][y_left[i]] == 1))
                    {
                        canMove = false;
                        break;
                    }
                }

                if (canMove)
                {
                    f_tetris->pos_x -= TETRIS_BLOCK_SIZE;
                    f_tetris->rotate_x -= TETRIS_BLOCK_SIZE;
                    block_pos_update(f_tetris);
                    block_update(f_tetris);
                }
            }
        }
        else
        {
            func_message(msg);
        }
        break;
    case MSG_CTP_SHORT_RIGHT:
        if(f_tetris->status == GAME_STATUS_RUNING)
        {
            if(f_tetris->isstart)
            {
                bool canMove = true;

                for (int i = 0; i < 4; i++)
                {
                    if ((f_tetris->block[i].y > BOTTOM_Y + TETRIS_BLOCK_SIZE || f_tetris->block[i].x > RIGHT_BORDER || f_tetris->map[x_right[i]][y_right[i]] == 1))
                    {
                        canMove = false;
                        break;
                    }
                }

                if (canMove)
                {
                    f_tetris->pos_x += TETRIS_BLOCK_SIZE;
                    f_tetris->rotate_x += TETRIS_BLOCK_SIZE;
                    block_pos_update(f_tetris);
                    block_update(f_tetris);
                }

            }
        }
        else
        {
            func_message(msg);
        }
        break;

    case MSG_CTP_LONG:
        break;

    default:
        func_message(msg);
        break;
    }
#endif

}



void func_game_tetris_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_tetris_t));
    func_cb.frm_main = func_game_tetris_form_create();

#if !TEST_ELEMENT
    f_tetris_t *f_tetris = (f_tetris_t *)func_cb.f_cb;
    compo_cardbox_t *compo_total_score = compo_getobj_byid(COMPO_ID_CARDBOX_TOTAL_SCORE);
    compo_cardbox_t *compo_line_score = compo_getobj_byid(COMPO_ID_CARDBOX_LINE_SCORE);
    compo_cardbox_t *compo_maxscore = compo_getobj_byid(COMPO_ID_CARDBOX_MAXSCORE_SCORE);

    display_nextblock(f_tetris);

    f_tetris->status = GAME_STATUS_RUNING;  //进入该界面游戏直接就运行了的
    f_tetris->isstart = 1;
    for(int i=0;i<4;i++)
    {
        widget_set_visible(f_tetris->cur_image_block[i], true);
        widget_set_visible(f_tetris->next_image_block[i], true);
    }
    compo_cardbox_set_visible(compo_total_score, 1);
    compo_cardbox_set_visible(compo_line_score, 1);
    compo_cardbox_set_visible(compo_maxscore, 0);

    co_timer_set(&timer, 50, TIMER_REPEAT, LEVEL_LOW_PRI, score_calculate_process, NULL);
    co_timer_enable(&timer, 0);


#endif
}


void func_game_tetris_exit(void)
{
#if !TEST_ELEMENT
    co_timer_del(&timer);
#endif

    func_cb.last = FUNC_GAME_TETRIS;
}


void func_game_tetris(void)
{
    printf("%s\n", __func__);
    func_game_tetris_enter();
    while (func_cb.sta == FUNC_GAME_TETRIS) {
        func_game_tetris_process();
        func_game_tetris_message(msg_dequeue());
    }
    func_game_tetris_exit();
}
#endif // GAME_TETRIS_EN
