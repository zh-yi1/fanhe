#ifndef _FUNC_MENU_H
#define _FUNC_MENU_H

enum {
    MENU_CLARM,         //心率
    MENU_CLOCK,         //时钟表盘
    MENU_ACTIVITY,      //运动

};


//MENU STYLE
enum {
    MENU_STYLE_HONEYCOMB,               //蜂窝
    MENU_STYLE_WATERFALL,               //瀑布流
    MENU_STYLE_LIST,                    //菜单列表
    MENU_STYLE_FOOTBALL,                //足球
    MENU_STYLE_SUDOKU,                  //九宫格
    MENU_STYLE_SUDOKU_HRZ,              //横向九宫格
    MENU_STYLE_GRID,                    //网格
    MENU_STYLE_DISK,                    //圆盘
    MENU_STYLE_RING,                    //环形
    MENU_STYLE_KALE,                    //万花筒
    MENU_STYLE_SKYRER,                  //天圆地方

    //客户新加菜单样式
    MENU_STYLE_CUM_SUDOKU,              //九宫格(上下边缘图标自动缩放)
    MENU_STYLE_CUM_GRID,                //华为风格网格
    MENU_STYLE_CUM_HEXAGON,             //六边型菜单风格
    MENU_STYLE_CUM_FOURGRID,            //四宫格列表

    MENU_STYLE_CUM_HONEYGRID,           //网格蜂窝（华为风格）

    MENU_STYLE_CNT,
};

//创建不同菜单窗体
compo_form_t *func_menu_sub_football_form_create(void);
compo_form_t *func_menu_sub_honeycomb_form_create(void);
compo_form_t *func_menu_sub_waterfall_form_create(void);
compo_form_t *func_menu_sub_list_form_create(void);
compo_form_t *func_menu_sub_sudoku_form_create(void);
compo_form_t* func_menu_sub_grid_form_create(void);
compo_form_t *func_menu_sub_disk_form_create(void);
compo_form_t *func_menu_sub_ring_form_create(void);
compo_form_t *func_menu_sub_kale_form_create(void);
compo_form_t *func_menu_sub_skyrer_form_create(void);

compo_form_t *func_menu_sub_cum_sudoku_form_create(void);      //客户新加菜单风格
compo_form_t *func_menu_sub_hexagon_form_create(void);

//不同样式菜单入口
void func_menu_sub_football(void);
void func_menu_sub_honeycomb(void);
void func_menu_sub_waterfall(void);
void func_menu_sub_list(void);
void func_menu_sub_sudoku(void);
void func_menu_sub_grid(void);
void func_menu_sub_disk(void);
void func_menu_sub_ring(void);
void func_menu_sub_kale(void);
void func_menu_sub_skyrer(void);

void func_menu_sub_cum_sudoku(void);   //客户新加菜单风格
void func_menu_sub_hexagon(void);

//公用函数
void func_menu_sub_message(size_msg_t msg);
void func_menu_sub_exit(void);

#endif
