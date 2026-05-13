#ifndef _SWITCHING_H
#define _SWITCHING_H

#define FUNC_SWITCH_AUTO                0x8000                  //自动完成切换
#define FUNC_SWITCH_DOWN_BLUR           0x4000                  //下拉模糊
#define FUNC_SWITCH_DOWN_BG_BLUR        0x2000                  //背景模糊

enum ENUM_FLAG_POS {
    FLAG_POS_NORM,                          //中间转场过程
    FLAG_POS_START,                         //退回到起点
    FLAG_POS_END,                           //完成到结束
};

enum FUNC_SWITCH_MODE {
    FUNC_SWITCH_DIRECT,                                         //无切换/直接切换
    FUNC_SWITCH_CANCEL,                                         //取消切换

    //淡入淡出
    FUNC_SWITCH_FADE,
    FUNC_SWITCH_FADE_OUT,

    //功能切换
    FUNC_SWITCH_LR,
    FUNC_SWITCH_LR_LEFT = FUNC_SWITCH_LR,                       //平移切换
    FUNC_SWITCH_LR_RIGHT,
    FUNC_SWITCH_LR_ZOOM_LEFT,                                   //缩放切换
    FUNC_SWITCH_LR_ZOOM_RIGHT,

    FUNC_SWITCH_UD,
    FUNC_SWITCH_UD_UP = FUNC_SWITCH_UD,                       	//平移切换
    FUNC_SWITCH_UD_DOWN,
    FUNC_SWITCH_UD_ZOOM_UP,                                   	//缩放切换
    FUNC_SWITCH_UD_ZOOM_DOWN,

    //上下帘切换
    FUNC_SWITCH_MENU,
    FUNC_SWITCH_MENU_DROPDOWN_UP = FUNC_SWITCH_MENU,            //下拉菜单
    FUNC_SWITCH_MENU_DROPDOWN_DOWN,
    FUNC_SWITCH_MENU_PULLUP_UP,                                 //上拉菜单
    FUNC_SWITCH_MENU_PULLUP_DOWN,
    FUNC_SWITCH_MENU_SIDE_POP,                                  //边栏
    FUNC_SWITCH_MENU_SIDE_BACK,

    //进出从图标缩放切换
    FUNC_SWITCH_ZOOM,
    FUNC_SWITCH_ZOOM_ENTER = FUNC_SWITCH_ZOOM,                  //进入应用
    FUNC_SWITCH_ZOOM_EXIT,                                      //退出应用

    //进出淡入切换
    FUNC_SWITCH_ZOOM_FADE,
    FUNC_SWITCH_ZOOM_FADE_ENTER = FUNC_SWITCH_ZOOM_FADE,        //进入应用
    FUNC_SWITCH_ZOOM_FADE_EXIT,                                 //退出应用

    FUNC_SWITCH3D,                                              //3D切换扩展
};

enum {
    NAV_ZOOM,
    NAV_FADE,
    NAV_SHIFT,
    NAV_FLIP,
    NAV_GLASS_FLIP,
    NAV_STEREOSCOPIC_GLASS_FLIP,
    NAV_CUBE,
    NAV_GRID,
    NAV_ROTA,
    NAV_FOLD,
    NAV_DRIFT,
    NAV_NONE,
};

//菜单项定义
typedef struct func_switching_mode_t_ {
    u8 prev;
    u8 next;
} func_switching_mode_t;

bool func_switching(u16 switch_mode, void *param);              //转场动画
u8 func_get_switching_mode_byidx(u8 idx, bool flag_next);

#include "func_switching3d.h"

#endif
